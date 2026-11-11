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

	// a range of the paragraph text in bold, italic, underline, strikethrough, superscript or
	// subscript; only the flags that are set
	export interface InlineFormat {
		start: MessageNumber;
		end: MessageNumber;
		bold?: boolean | string;
		italic?: boolean | string;
		underline?: boolean | string;
		strikethrough?: boolean | string;
		escapement?: 'super' | 'sub';
	}

	// a range of the paragraph text inserted, deleted or with its attributes changed while
	// changes were recorded
	export interface InlineChange {
		start: MessageNumber;
		end: MessageNumber;
		type: 'insertion' | 'deletion' | 'attributes';
		author?: string;
		dateTime?: string; // ISO 8601 with no zone, a local time
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

	// the range of a content control's content, without the characters marking its ends
	export interface InlineContentControl {
		start: MessageNumber;
		end: MessageNumber;
		type:
			| 'richText'
			| 'plainText'
			| 'checkbox'
			| 'dropDown'
			| 'comboBox'
			| 'date'
			| 'picture';
		alias?: string;
		checked?: boolean | string; // a checkbox only
		placeholder?: boolean | string; // the content is the placeholder text
	}

	// a range of the paragraph text the spell or grammar checker marked
	export interface InlineError {
		start: MessageNumber;
		end: MessageNumber;
		kind: 'spelling' | 'grammar';
	}

	// the elements of one paragraph, each kind in order
	export interface InlineElements {
		links?: InlineLink[];
		formats?: InlineFormat[];
		changes?: InlineChange[];
		images?: InlineImage[];
		comments?: InlineComment[];
		contentControls?: InlineContentControl[];
		errors?: InlineError[];
	}

	// one list per paragraph, an empty string for a paragraph without any or for no paragraphs
	type PerParagraph<T> = Array<T[] | string> | string;

	// the elements of the paragraphs before or after the caret
	export interface ContextInlineElements {
		links?: PerParagraph<InlineLink>;
		formats?: PerParagraph<InlineFormat>;
		changes?: PerParagraph<InlineChange>;
		images?: PerParagraph<InlineImage>;
		comments?: PerParagraph<InlineComment>;
		contentControls?: PerParagraph<InlineContentControl>;
		errors?: PerParagraph<InlineError>;
	}

	export interface ParagraphInlineElements {
		paragraph: InlineElements;
		before: ContextInlineElements;
		after: ContextInlineElements;
	}

	// the fields of a message that carry them
	export interface InlineElementsMessage {
		links?: InlineLink[];
		formats?: InlineFormat[];
		changes?: InlineChange[];
		images?: InlineImage[];
		comments?: InlineComment[];
		contentControls?: InlineContentControl[];
		errors?: InlineError[];
		beforeLinks?: PerParagraph<InlineLink>;
		afterLinks?: PerParagraph<InlineLink>;
		beforeFormats?: PerParagraph<InlineFormat>;
		afterFormats?: PerParagraph<InlineFormat>;
		beforeChanges?: PerParagraph<InlineChange>;
		afterChanges?: PerParagraph<InlineChange>;
		beforeImages?: PerParagraph<InlineImage>;
		afterImages?: PerParagraph<InlineImage>;
		beforeComments?: PerParagraph<InlineComment>;
		afterComments?: PerParagraph<InlineComment>;
		beforeContentControls?: PerParagraph<InlineContentControl>;
		afterContentControls?: PerParagraph<InlineContentControl>;
		beforeErrors?: PerParagraph<InlineError>;
		afterErrors?: PerParagraph<InlineError>;
	}

	export function inlineElements(
		message: InlineElementsMessage,
	): ParagraphInlineElements {
		return {
			paragraph: {
				links: message.links,
				formats: message.formats,
				changes: message.changes,
				images: message.images,
				comments: message.comments,
				contentControls: message.contentControls,
				errors: message.errors,
			},
			before: {
				links: message.beforeLinks,
				formats: message.beforeFormats,
				changes: message.beforeChanges,
				images: message.beforeImages,
				comments: message.beforeComments,
				contentControls: message.beforeContentControls,
				errors: message.beforeErrors,
			},
			after: {
				links: message.afterLinks,
				formats: message.afterFormats,
				changes: message.afterChanges,
				images: message.afterImages,
				comments: message.afterComments,
				contentControls: message.afterContentControls,
				errors: message.afterErrors,
			},
		};
	}
}
