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
 * The inline elements of the paragraphs in an accessibility message: the paragraph at the caret,
 * and the paragraphs before and after it.
 */

namespace cool {
	// numbers arrive as strings in the message
	type MessageNumber = number | string;

	// a range of the paragraph text
	export interface InlineLink {
		start: MessageNumber;
		end: MessageNumber;
		url: string;
	}

	// a U+FFFC (OBJECT REPLACEMENT CHARACTER) standing for an image, or a shape, an object or a
	// text frame read like one
	export interface InlineImage {
		position: MessageNumber;
		name: string;
		description: string;
		kind?: 'shape' | 'formula' | 'chart' | 'object' | 'frame';
		shapeType?: string; // as the engine sends it, e.g. "Rectangle"
	}

	// a range of the paragraph text, ending after the U+FFFC of its anchor
	export interface InlineComment {
		start: MessageNumber;
		end: MessageNumber;
		id: MessageNumber;
		author: string;
		text: string;
	}

	// the elements of one paragraph, each kind in order
	export interface InlineElements {
		links?: InlineLink[];
		images?: InlineImage[];
		comments?: InlineComment[];
	}

	// one list per paragraph, an empty string for a paragraph without any or for no paragraphs
	type PerParagraph<T> = Array<T[] | string> | string;

	// the elements of the paragraphs before or after the caret
	export interface ContextInlineElements {
		links?: PerParagraph<InlineLink>;
		images?: PerParagraph<InlineImage>;
		comments?: PerParagraph<InlineComment>;
	}

	export interface ParagraphInlineElements {
		paragraph: InlineElements;
		before: ContextInlineElements;
		after: ContextInlineElements;
	}

	// the fields of a message that carry them
	export interface InlineElementsMessage {
		links?: InlineLink[];
		images?: InlineImage[];
		comments?: InlineComment[];
		beforeLinks?: PerParagraph<InlineLink>;
		afterLinks?: PerParagraph<InlineLink>;
		beforeImages?: PerParagraph<InlineImage>;
		afterImages?: PerParagraph<InlineImage>;
		beforeComments?: PerParagraph<InlineComment>;
		afterComments?: PerParagraph<InlineComment>;
	}

	export function inlineElements(
		message: InlineElementsMessage,
	): ParagraphInlineElements {
		return {
			paragraph: {
				links: message.links,
				images: message.images,
				comments: message.comments,
			},
			before: {
				links: message.beforeLinks,
				images: message.beforeImages,
				comments: message.beforeComments,
			},
			after: {
				links: message.afterLinks,
				images: message.afterImages,
				comments: message.afterComments,
			},
		};
	}
}
