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

#ifndef DEMO_BUFFER_H
#define DEMO_BUFFER_H

#include "demo-common.h"
#include "demo-font.h"
#include "demo-shader.h"

typedef struct demo_buffer_t demo_buffer_t;

demo_buffer_t *
demo_buffer_create (void);

demo_buffer_t *
demo_buffer_reference (demo_buffer_t *buffer);

void
demo_buffer_destroy (demo_buffer_t *buffer);


void
demo_buffer_clear (demo_buffer_t *buffer);

void
demo_buffer_extents (demo_buffer_t    *buffer,
		     glyphy_extents_t *ink_extents,
		     glyphy_extents_t *logical_extents);

void
demo_buffer_move_to (demo_buffer_t        *buffer,
		     const glyphy_point_t *p);

void
demo_buffer_current_point (demo_buffer_t  *buffer,
			   glyphy_point_t *p);

void
demo_buffer_add_text (demo_buffer_t        *buffer,
		      const char           *utf8,
		      demo_font_t          *font,
		      double                font_size);

void
demo_buffer_draw (demo_buffer_t *buffer);


#endif /* DEMO_BUFFER_H */
