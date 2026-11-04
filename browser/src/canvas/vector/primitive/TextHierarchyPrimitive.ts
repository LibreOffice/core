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

	/// One paragraph of a text.
	export interface TextHierarchyParagraphPrimitive extends Primitive {
		type: typeof TextHierarchyParagraphPrimitive.type;
	}

	export namespace TextHierarchyParagraphPrimitive {
		export const type = 'textHierarchyParagraph';
	}

	/// The whole text of an object.
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
