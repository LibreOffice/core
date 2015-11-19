/*
 * Copyright 2012 Google, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Google Author(s): Behdad Esfahbod
 */

#ifndef DEMO_SHADERS_H
#define DEMO_SHADERS_H

#include "demo-common.h"
#include "demo-font.h"


struct glyph_vertex_t {
  /* Position */
  GLfloat x;
  GLfloat y;
  /* Glyph info */
  GLfloat g16hi;
  GLfloat g16lo;
};

void
demo_shader_add_glyph_vertices (const glyphy_point_t        &p,
				double                       font_size,
				glyph_info_t                *gi,
				std::vector<glyph_vertex_t> *vertices,
				glyphy_extents_t            *extents);


GLuint
demo_shader_create_program (void);


#endif /* DEMO_SHADERS_H */
