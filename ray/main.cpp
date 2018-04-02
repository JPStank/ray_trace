#include <iostream>
#include "sphere.h"
#include "hitable_list.h"
#include "float.h"
#include "camera.h"
#include "material.h"
#include "bvh.h"
#include "aarect.h"

#include <ctime>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"

vec3 color(const ray& r, hitable* world, int depth)
{
	hit_record rec;
	if (world->hit(r, 0.001f, FLT_MAX, rec))
	{
		ray scattered;
		vec3 attenuation;
		vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
		if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered))
			return emitted + attenuation*color(scattered, world, depth + 1);
		else
			return emitted;
	}
	else
		return vec3(0, 0, 0);
}

void random_scene(hitable** ret)
{
	int n = 500;
	hitable** list = new hitable*[n + 1];
	list[0] = new sphere(vec3(0.0f, -1000.0f, 0.0f), 1000.0f, new lambertian(new checker_texture(new constant_texture(vec3(0.2f, 0.3f, 0.1f)), new constant_texture(vec3(0.9f, 0.9f, 0.9f)))));
	int i = 1;

	for (int a = -11; a < 11; ++a)
	{
		for (int b = -11; b < 11; ++b)
		{
			float choose_mat = rand48();
			vec3 center(float(a) + 0.9f*rand48(), 0.2f, float(b) + 0.9f*rand48());

			if ((center - vec3(4.0f, 0.2f, 0.0f)).length() > 0.9f)
			{
				if (choose_mat < 0.8f) // diffuse
				{
					list[i++] = new moving_sphere(center, center+vec3(0.0f, 0.5f*rand48(), 0.0f),0.0f, 1.0f, 0.2f, new lambertian(new constant_texture(vec3(rand48()*rand48(), rand48()*rand48(), rand48()*rand48()))));
				}
				else if (choose_mat < 0.95f) // metal
				{
					list[i++] = new sphere(center, 0.2f, new metal(vec3(0.5f*(1.0f + rand48()), 0.5f*(1.0f + rand48()), 0.5f*(1.0f + rand48()))));
				}
				else // glass
				{
					list[i++] = new sphere(center, 0.2f, new dielectric(1.5f));
				}
			}
		}
	}

	list[i++] = new sphere(vec3(0.0f, 1.0f, 0.0f), 1.0f, new dielectric(1.5f));
	list[i++] = new sphere(vec3(-4.0f, 1.0f, 0.0f), 1.0f, new lambertian(new constant_texture(vec3(0.4f, 0.2f, 0.1f))));
	list[i++] = new sphere(vec3(4.0f, 1.0f, 0.0f), 1.0f, new metal(vec3(0.7f, 0.6f, 0.5f)));

	//*ret = new hitable_list(list, i);
	*ret = new bvh_node(list, i, 0.0f, 1.0f);
}

void two_spheres(hitable** ret)
{
	texture* checker = new checker_texture(new constant_texture(vec3(0.2f, 0.3f, 0.1f)), new constant_texture(vec3(0.9f, 0.9f, 0.9f)));
	//int n = 50;
	hitable** list = new hitable*[2];
	list[0] = new sphere(vec3(0.0f, -10.0f, 0.0f), 10.0f, new lambertian(checker));
	list[1] = new sphere(vec3(0.0f, 10.0f, 0.0f), 10.0f, new lambertian(checker));

	*ret = new hitable_list(list, 2);
}

void two_perlin_spheres(hitable** ret)
{
	texture* pertext = new noise_texture(4.0f);
	hitable** list = new hitable*[2];
	list[0] = new sphere(vec3(0.0f, -1000.0f, 0.0f), 1000.0f, new lambertian(pertext));
	list[1] = new sphere(vec3(0.0f, 2.0f, 0.0f), 2.0f, new lambertian(pertext));

	*ret = new hitable_list(list, 2);
}

void earth(hitable** ret)
{
	int nx, ny, nn;
	unsigned char* tex_data = stbi_load("earth_m.jpg", &nx, &ny, &nn, 0);
	material* mat = new lambertian(new image_texture(tex_data, nx, ny));

	*ret = new sphere(vec3(0.0f, 1.0f, 0.0f), 2.0f, mat);
}

void simple_light(hitable** ret)
{
	texture* pertext = new noise_texture(4.0f);
	hitable** list = new hitable*[4];
	list[0] = new sphere(vec3(0.0f, -1000.0f, 0.0f), 1000.0f, new lambertian(pertext));
	list[1] = new sphere(vec3(0.0f, 2.0f, 0.0f), 2.0f, new lambertian(pertext));
	list[2] = new sphere(vec3(0.0f, 7.0f, 0.0f), 2.0f, new diffuse_light(new constant_texture(vec3(4, 4, 4))));
	list[3] = new xy_rect(3, 5, 1, 3, -2, new diffuse_light(new constant_texture(vec3(4, 4, 4))));

	*ret = new hitable_list(list, 4);
}

void cornell_box(hitable** ret)
{
	hitable** list = new hitable*[6];
	int i = 0;
	material* red = new lambertian(new constant_texture(vec3(0.65f, 0.05f, 0.05f)));
	material* white = new lambertian(new constant_texture(vec3(0.73f, 0.73f, 0.73f)));
	material* green = new lambertian(new constant_texture(vec3(0.12f, 0.45f, 0.15f)));
	material* light = new diffuse_light(new constant_texture(vec3(15, 15, 15)));
	list[i++] = new flip_normals(new yz_rect(0, 555, 0, 555, 555, green));
	list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
	list[i++] = new xz_rect(213, 343, 227, 332, 554, light);
	list[i++] = new flip_normals(new xz_rect(0, 555, 0, 555, 555, white));
	list[i++] = new xz_rect(0, 555, 0, 555, 0, white);
	list[i++] = new flip_normals(new xy_rect(0, 555, 0, 555, 555, white));

	*ret = new hitable_list(list, i);
}

int main()
{
	srand((unsigned int)time(0));
#ifdef CUSTOM_RAND
	init_rand_table();
#endif
	//int nx = 1280;
	//int ny = 720;
	int ns = 100;
	int nx = 300;
	int ny = 300;
	std::cout << "P3\n" << nx << " " << ny << "\n255\n";

	//const int count = 5;
	//hitable* list[count];
	//list[0] = new sphere(vec3(0.0f, 0.0f, -1.0f), 0.75f, new lambertian(vec3(0.1f, 0.2f, 0.5f)));
	//list[1] = new sphere(vec3(0.0f, -100.75f, -1.0f), 100.0f, new lambertian(vec3(0.8f, 0.8f, 0.0f)));
	//list[2] = new sphere(vec3(1.5f, 0.0f, -1.0f), 0.75f, new metal(vec3(0.8f, 0.6f, 0.2f), 0.0f));
	//list[3] = new sphere(vec3(-1.5f, 0.0f, -1.0f), 0.75f, new dielectric(1.5f));
	//list[4] = new sphere(vec3(-1.5f, 0.0f, -1.0f), -0.7f, new dielectric(1.5f));
	hitable* world;// = new hitable_list(list, count);
	//random_scene(&world);// new hitable_list(list, count);
	//two_spheres(&world);
	//two_perlin_spheres(&world);
	//earth(&world);
	//simple_light(&world);
	cornell_box(&world);

	vec3 lookfrom(278, 278, -800);
	//vec3 lookfrom(478, 278, -600);
	vec3 lookat(278, 278, 0);
	float dist_to_focus = 10.0f;// (lookfrom - lookat).length();
	float aperature = 0.0f;
	float vfov = 40;
	camera cam(lookfrom, lookat, vec3(0.0f, 1.0f, 0.0f), vfov, float(nx)/float(ny), aperature, dist_to_focus, 0.0f, 1.0f);


	//int image_size = nx*ny * 3;
	//unsigned char *img = new unsigned char[image_size];
	//unsigned int img_idx = 0;

	for (int j = ny - 1; j >= 0; j--)
	{
		for (int i = 0; i < nx; i++)
		{
			vec3 col(0.0f, 0.0f, 0.0f);
			for (int s = 0; s < ns; ++s)
			{
				float u = float(i + rand48()) / float(nx);
				float v = float(j + rand48()) / float(ny);
				ray r = cam.get_ray(u, v);
				vec3 p = r.point_at_parameter(2.0f);
				col += color(r, world, 0);
			}

			col /= float(ns);
			col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
			int ir = int(255.99*col[0]);
			int ig = int(255.99*col[1]);
			int ib = int(255.99*col[2]);

			std::cout << ir << " " << ig << " " << ib << "\n";

			//img[img_idx++] = (unsigned char)(255.99f*col[0]);
			//img[img_idx++] = (unsigned char)(255.99f*col[1]);
			//img[img_idx++] = (unsigned char)(255.99f*col[2]);
		}
	}

	//stbi_write_png("test.png", nx, ny, 3, img, nx * 3);
	//delete[] img;

	delete world;
	return 0;
}