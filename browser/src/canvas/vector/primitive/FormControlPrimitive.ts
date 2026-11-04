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
		kind: 'pushButton' | 'checkBox' | 'optionButton';
		matrix?: number[];
		label?: string;
		/// 0 not checked, 1 checked, 2 neither.
		state?: number;
		disabled?: boolean;
		defaultButton?: boolean;
		/// 0 left, 1 centred, 2 right.
		align?: number;
		textColor?: string;
		background?: string;
		font?: FormControlFont;
	}

	export namespace FormControlPrimitive {
		export const type = 'formControl';
	}
}
