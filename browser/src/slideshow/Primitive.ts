/* -*- js-indent-level: 8 -*- */

/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* This file for handling Primitive */

declare var SlideShow: any;

interface Vertex {
	position: vec3;
	normal: vec3;
	texCoord: vec2;
}

class Primitive {
	public vertices: Vertex[] = [];
	public operations: Operation[] = [];

	static cloneDeep(oldPrimitive: Primitive): Primitive {
		const newPrimitive = new Primitive();

		newPrimitive.vertices = [...oldPrimitive.vertices];

		newPrimitive.operations = oldPrimitive.operations.map(
			(operation: Operation) => {
				return operation;
			},
		);

		return newPrimitive;
	}

	public pushTriangle(
		slideLocation0: vec2,
		slideLocation1: vec2,
		slideLocation2: vec2,
	): void {
		let verts: vec3[] = [];
		const texs: vec2[] = [];

		verts.push([2 * slideLocation0[0] - 1, -2 * slideLocation0[1] + 1, 0.0]);
		verts.push([2 * slideLocation1[0] - 1, -2 * slideLocation1[1] + 1, 0.0]);
		verts.push([2 * slideLocation2[0] - 1, -2 * slideLocation2[1] + 1, 0.0]);

		const normal = this.cross(
			this.subtract(verts[0], verts[1]),
			this.subtract(verts[1], verts[2]),
		);
		const isFacingUs = normal[2] >= 0.0;

		if (isFacingUs) {
			texs.push(slideLocation0);
			texs.push(slideLocation1);
			texs.push(slideLocation2);
		} else {
			texs.push(slideLocation0);
			texs.push(slideLocation2);
			texs.push(slideLocation1);
			verts = [];
			verts.push([2 * slideLocation0[0] - 1, -2 * slideLocation0[1] + 1, 0.0]);
			verts.push([2 * slideLocation2[0] - 1, -2 * slideLocation2[1] + 1, 0.0]);
			verts.push([2 * slideLocation1[0] - 1, -2 * slideLocation1[1] + 1, 0.0]);
		}

		this.vertices.push(
			{ position: verts[0], normal: [0, 0, 1], texCoord: texs[0] },
			{ position: verts[1], normal: [0, 0, 1], texCoord: texs[1] },
			{ position: verts[2], normal: [0, 0, 1], texCoord: texs[2] },
		);
	}

	public getVertex(n: number): vec3 {
		return this.vertices[n].position;
	}

	private subtract(a: vec3, b: vec3): vec3 {
		return [a[0] - b[0], a[1] - b[1], a[2] - b[2]];
	}

	private cross(a: vec3, b: vec3): vec3 {
		return [
			a[1] * b[2] - a[2] * b[1],
			a[2] * b[0] - a[0] * b[2],
			a[0] * b[1] - a[1] * b[0],
		];
	}

	public clear(): void {
		this.operations = [];
	}
}

SlideShow.Primitive = Primitive;
