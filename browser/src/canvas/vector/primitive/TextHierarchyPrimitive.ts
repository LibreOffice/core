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

/*
 * The structure of a text. Each of these groups part of a text, and its children are everything
 * it draws.
 */
namespace cool {
	/// A text field. fieldType is "url" for a hyperlink and "page" for a
	/// page number. Other kinds of field have no fieldType.
	export interface TextHierarchyFieldPrimitive extends Primitive {
		type: typeof TextHierarchyFieldPrimitive.type;
		fieldType?: 'url' | 'page';
	}

	export namespace TextHierarchyFieldPrimitive {
		export const type = 'textHierarchyField';
	}

	/// One line of a paragraph.
	export interface TextHierarchyLinePrimitive extends Primitive {
		type: typeof TextHierarchyLinePrimitive.type;
	}

	export namespace TextHierarchyLinePrimitive {
		export const type = 'textHierarchyLine';
	}

	/// One paragraph of a text, with its lines and portions as children.
	/// bounds is its rectangle as minX, minY, maxX and maxY, in slide twips.
	export interface TextHierarchyParagraphPrimitive extends Primitive {
		type: typeof TextHierarchyParagraphPrimitive.type;
		bounds?: [number, number, number, number];
	}

	export namespace TextHierarchyParagraphPrimitive {
		export const type = 'textHierarchyParagraph';

		/// The paragraph nodes of the object's own text, in text order, or an
		/// empty list. The copy in a shadow, hidden geometry and edit-view
		/// content are passed over.
		export function collect(
			primitives: Primitive[] | undefined,
		): TextHierarchyParagraphPrimitive[] {
			const block = findBlock(primitives);
			if (!block?.children) return [];
			return block.children.filter(
				(child) => child.type === type,
			) as TextHierarchyParagraphPrimitive[];
		}

		function findBlock(
			primitives: Primitive[] | undefined,
		): TextHierarchyBlockPrimitive | undefined {
			if (!primitives) return undefined;
			for (const primitive of primitives) {
				if (primitive.type === TextHierarchyBlockPrimitive.type)
					return primitive as TextHierarchyBlockPrimitive;
				if (
					primitive.type === ShadowPrimitive.type ||
					primitive.type === HiddenGeometryPrimitive.type ||
					primitive.type === ExclusiveEditViewPrimitive.type
				)
					continue;
				const found = findBlock(primitive.children);
				if (found) return found;
			}
			return undefined;
		}
	}

	/// The whole text of an object, one paragraph node per paragraph, empty
	/// ones included, in text order. A shadow holds a copy of the block.
	export interface TextHierarchyBlockPrimitive extends Primitive {
		type: typeof TextHierarchyBlockPrimitive.type;
	}

	export namespace TextHierarchyBlockPrimitive {
		export const type = 'textHierarchyBlock';
	}

	/// The bullet or the numbering label of a paragraph.
	export interface TextHierarchyBulletPrimitive extends Primitive {
		type: typeof TextHierarchyBulletPrimitive.type;
	}

	export namespace TextHierarchyBulletPrimitive {
		export const type = 'textHierarchyBullet';
	}

	/// The emphasis marks over or under a text portion.
	export interface TextHierarchyEmphasisMarkPrimitive extends Primitive {
		type: typeof TextHierarchyEmphasisMarkPrimitive.type;
	}

	export namespace TextHierarchyEmphasisMarkPrimitive {
		export const type = 'textHierarchyEmphasisMark';
	}
}
