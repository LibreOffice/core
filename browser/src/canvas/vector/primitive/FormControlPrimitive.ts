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
		/// 0 not checked, 1 checked, 2 neither.
		state?: number;
		disabled?: boolean;
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
		/// The labels of the columns of a table control.
		columns?: string[];
		font?: FormControlFont;
	}

	export namespace FormControlPrimitive {
		export const type = 'formControl';
	}
}
