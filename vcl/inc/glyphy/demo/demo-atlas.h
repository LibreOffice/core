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
 * Google Author(s): Behdad Esfahbod, Maysum Panju
 */

#ifndef DEMO_ATLAS_H
#define DEMO_ATLAS_H

#include "demo-common.h"


typedef struct demo_atlas_t demo_atlas_t;

demo_atlas_t *
demo_atlas_create (unsigned int w,
		   unsigned int h,
		   unsigned int item_w,
		   unsigned int item_h_quantum);

demo_atlas_t *
demo_atlas_reference (demo_atlas_t *at);

void
demo_atlas_destroy (demo_atlas_t *at);


void
demo_atlas_alloc (demo_atlas_t  *at,
		  glyphy_rgba_t *data,
		  unsigned int   len,
		  unsigned int  *px,
		  unsigned int  *py);

void
demo_atlas_bind_texture (demo_atlas_t *at);

void
demo_atlas_set_uniforms (demo_atlas_t *at);


#endif /* DEMO_ATLAS_H */
