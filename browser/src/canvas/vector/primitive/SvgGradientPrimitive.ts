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

namespace cool {
	/// A gradient stop. offset and opacity run from 0 to 1, and opacity is absent for opaque.
	export interface SvgGradientStop {
		offset: number;
		color: string;
		opacity?: number;
	}

	/// A polygon filled with an SVG gradient, its stops sorted by offset.
	/// unitToObject maps the unit gradient to the parent space. It is
	/// absent for a single stop, which is a plain fill, and for a gradient
	/// that draws nothing. Past the stops, "pad" keeps the end colors,
	/// "repeat" starts the stops over and "reflect" runs them back and forth.
	export interface SvgGradientPrimitive extends Primitive {
		path?: string;
		bounds?: number[]; // [minX, minY, maxX, maxY] in twips
		unitToObject?: number[];
		spreadMethod?: 'pad' | 'reflect' | 'repeat';
		colorStops?: SvgGradientStop[];
	}

	/// The unit gradient runs along the x axis from 0 to 1.
	export interface SvgLinearGradientPrimitive extends SvgGradientPrimitive {
		type: typeof SvgLinearGradientPrimitive.type;
	}

	export namespace SvgLinearGradientPrimitive {
		export const type = 'svgLinearGradient';
	}

	/// The unit gradient is the circle of radius 1 around the origin.
	/// focalStep, when present, centers the circle of radius r at
	/// focalStep * (reach - r), so the smallest circle is nearest the focus.
	export interface SvgRadialGradientPrimitive extends SvgGradientPrimitive {
		type: typeof SvgRadialGradientPrimitive.type;
		focalStep?: number[];
	}

	export namespace SvgRadialGradientPrimitive {
		export const type = 'svgRadialGradient';
	}
}
