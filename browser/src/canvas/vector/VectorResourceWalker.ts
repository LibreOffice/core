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
	/// Walk a JSON primitive tree and collect the checksum of every
	/// image-bearing primitive and the font id of every text portion
	/// seen. Recurses into the children container.
	export class VectorResourceWalker {
		private _checksums: Set<number>;
		private _fontIds: Set<string>;

		constructor(checksums: Set<number>, fontIds: Set<string>) {
			this._checksums = checksums;
			this._fontIds = fontIds;
		}

		walkObjects(objects: SlideObject[]): void {
			for (const obj of objects) {
				if (obj.primitives) this.walkPrimitives(obj.primitives);
			}
		}

		walkPrimitives(primitives: Primitive[]): void {
			for (const p of primitives) this._walkPrimitive(p);
		}

		private _walkPrimitive(primitive: Primitive): void {
			if (GraphicResource.is(primitive))
				this._checksums.add(primitive.checksum);
			const fontId = (primitive as TextSimplePortionPrimitive).fontId;
			if (typeof fontId === 'string') this._fontIds.add(fontId);
			if (primitive.children) this.walkPrimitives(primitive.children);
		}
	}
}
