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

namespace cool {
	/// The font a form control's model gives its text. The height is in points.
	export interface FormControlFont {
		name?: string;
		height?: number;
		bold?: boolean;
		italic?: boolean;
	}

	/// A form control of a kind the client draws itself, as the values its model holds. The
	/// matrix maps the unit square to the control's place on the slide, in twips.
	export interface FormControlPrimitive extends Primitive {
		type: typeof FormControlPrimitive.type;
		kind:
			| 'pushButton'
			| 'checkBox'
			| 'optionButton'
			| 'label'
			| 'groupBox'
			| 'textBox'
			| 'formattedField'
			| 'numericField'
			| 'currencyField'
			| 'dateField'
			| 'timeField'
			| 'patternField'
			| 'fileSelection'
			| 'comboBox'
			| 'listBox'
			| 'imageButton'
			| 'imageControl'
			| 'scrollBar'
			| 'spinButton'
			| 'tableControl'
			| 'navigationBar';
		matrix?: number[];
		label?: string;
		/// The text as the control shows it, formatted as the office formats it.
		text?: string;
		/// The date of a date field as YYYY-MM-DD, where it holds one.
		date?: string;
		/// The time of a time field as HH:MM:SS, where it holds one.
		time?: string;
		/// Whether the format of a time field shows the seconds.
		seconds?: boolean;
		/// Whether a time field holds a span of time, which can pass 24 hours.
		duration?: boolean;
		/// The edit mask of a pattern field, one character class per position: L a fixed
		/// character, a a letter, A a letter in upper case, c a letter or a digit, C the same in
		/// upper case, N a digit, n a digit or a space, x any character, X the same in upper case.
		editMask?: string;
		/// What each position of a pattern field shows while it is empty, and the fixed
		/// characters at the L positions of the edit mask.
		literalMask?: string;
		/// Whether a field takes only what its mask or format allows.
		strictFormat?: boolean;
		/// The checksum of the picture of an image control or an image button, which comes
		/// from the bitmap cache.
		checksum?: number;
		/// How the picture fits the control: 0 at its own size, 1 as large as fits with its
		/// proportions kept, 2 stretched over the whole control.
		scaleMode?: number;
		/// 0 not checked, 1 checked, 2 neither.
		state?: number;
		disabled?: boolean;
		/// A control that shows its value as an enabled one does and takes no change of it.
		readOnly?: boolean;
		/// The place of the control in the order Tab goes through the form, from 1. The controls
		/// with a place come first, the lowest first, and the others follow in the order of the
		/// page.
		tabIndex?: number;
		/// Whether Tab stops at the control. Where it is absent, the kind of the control decides.
		tabStop?: boolean;
		defaultButton?: boolean;
		/// 0 left, 1 centred, 2 right.
		align?: number;
		textColor?: string;
		background?: string;
		/// A field with a button that opens a list or a calendar.
		dropdown?: boolean;
		/// A field with buttons that step its value.
		spin?: boolean;
		multiLine?: boolean;
		/// A list of which several entries can be selected at once.
		multiSelection?: boolean;
		/// 0 for a control without a border. Absent for one with a border.
		border?: number;
		/// The entries of a list or a combo box, and which of them are selected.
		items?: string[];
		selected?: number[];
		/// A scroll bar or spin button that runs down instead of across.
		vertical?: boolean;
		/// Where the value of a scroll bar or spin button stands, between min and max; visible is
		/// the share a scroll bar's thumb stands for.
		value?: number;
		min?: number;
		max?: number;
		visible?: number;
		/// How far one step of an arrow goes, and for a scroll bar how far a click on its track.
		step?: number;
		pageStep?: number;
		/// The labels of the columns of a table control.
		columns?: string[];
		font?: FormControlFont;
	}

	export namespace FormControlPrimitive {
		export const type = 'formControl';

		/// How the picture of an image control or an image button fits the control, as the
		/// scaleMode values say. Where the model names no way, an image control stretches the
		/// picture, as its model starts out doing, and an image button shows it at its own size.
		export function imageScaleMode(primitive: FormControlPrimitive): number {
			return primitive.scaleMode ?? (primitive.kind === 'imageControl' ? 2 : 0);
		}
	}
}
