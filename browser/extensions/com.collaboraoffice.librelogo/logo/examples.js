/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Ready-made programs for the Examples menu. The sources use English
// keywords, which every language accepts, so one text works everywhere; the
// panel translates only the menu labels (see Panel.exampleName). Keep this in
// sync with that switch.

export const EXAMPLES = [
	{
		id: 'star',
		source: `; A five-pointed star
PENCOLOR "GOLD"
FILLCOLOR "GOLD"
PENSIZE 4
REPEAT 5 [
    FORWARD 220
    RIGHT 144
]
FILL
`,
	},
	{
		id: 'spiral',
		source: `; A square spiral in changing colours
PENSIZE 2
REPEAT 100 [
    PENCOLOR ANY
    FORWARD REPCOUNT * 3
    RIGHT 89
]
`,
	},
	{
		id: 'rosette',
		source: `; A pinwheel of coloured petals
HIDETURTLE
PENSIZE 2
PENCOLOR "GRAY"
REPEAT 12 [
    FILLCOLOR ANY
    PENDOWN
    CIRCLE 150
    PENUP
    RIGHT 30
    FORWARD 55
]
`,
	},
	{
		id: 'tree',
		source: `; A recursive tree
TO tree size
    IF size < 8 [ STOP ]
    PENSIZE size / 12
    FORWARD size
    LEFT 28
    tree size * 0.72
    RIGHT 56
    tree size * 0.72
    LEFT 28
    BACK size
END
PENCOLOR "BROWN"
PENUP
POSITION [300, 720]
HEADING 0
PENDOWN
tree 170
`,
	},
	{
		id: 'animation',
		source: `; A flower that blooms: SLEEP paces the drawing, so the
; preview animates petal by petal. Wrap the REPEAT in
; PICTURE "flower.svg" [ ... ] to save it as an animated SVG.
HIDETURTLE
PENSIZE 2
PENCOLOR "PURPLE"
REPEAT 18 [
    FILLCOLOR ANY
    ELLIPSE [50, 150]
    RIGHT 20
    SLEEP 150
]
`,
	},
	{
		id: 'beaver',
		source: `; A friendly beaver head

TO at x y
    PENUP
    POSITION [x, y]
    HEADING 0
    PENDOWN
END

HIDETURTLE
PENCAP "round"

; --- ears (behind the head) ---
PENSIZE 3
PENCOLOR [90, 55, 33]
FILLCOLOR [120, 74, 45]
at 205 290  CIRCLE 95
at 395 290  CIRCLE 95
FILLCOLOR [170, 120, 80]
at 205 300  CIRCLE 48
at 395 300  CIRCLE 48

; --- head ---
PENSIZE 4
PENCOLOR [90, 55, 33]
FILLCOLOR [150, 96, 58]
at 300 400  ELLIPSE [320, 285]

; --- muzzle / lower face ---
FILLCOLOR [212, 180, 134]
at 300 470  ELLIPSE [235, 175]

; --- eyes ---
PENSIZE 3
PENCOLOR "BLACK"
FILLCOLOR "WHITE"
at 240 360  CIRCLE 66
at 360 360  CIRCLE 66
FILLCOLOR "BLACK"
at 247 366  CIRCLE 30
at 353 366  CIRCLE 30
PENCOLOR "WHITE"
FILLCOLOR "WHITE"
at 239 358  CIRCLE 13
at 345 358  CIRCLE 13

; --- nose ---
PENSIZE 2
PENCOLOR [30, 20, 18]
FILLCOLOR [50, 35, 32]
at 300 440  ELLIPSE [60, 44]

; --- front teeth ---
PENSIZE 3
PENCOLOR [110, 110, 110]
FILLCOLOR [255, 252, 235]
at 286 500  RECTANGLE [26, 62]
at 314 500  RECTANGLE [26, 62]

; --- whiskers ---
PENSIZE 2
PENCOLOR [70, 50, 40]
at 216 462  POSITION [120, 442]
at 216 474  POSITION [112, 476]
at 216 486  POSITION [122, 508]
at 384 462  POSITION [480, 442]
at 384 474  POSITION [488, 476]
at 384 486  POSITION [478, 508]
`,
	},
];
