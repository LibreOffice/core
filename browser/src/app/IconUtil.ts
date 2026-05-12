/* -*- js-indent-level: 8 -*- */
/* global app */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// IconUtil contains various LO related utility functions used

class IconUtil {
	public static commandsWithIcons: string[] = [
		'Cut',
		'Copy',
		'Paste',
		'Delete',
		'CompressGraphic',
		'SaveGraphic',
		'saveimagetowopi',
		'Crop',
		'FormatPaintbrush',
		'ResetAttributes',
		'NextTrackedChange',
		'PreviousTrackedChange',
		'PageDialog',
		'MergeCells',
		'DeleteCell',
		'InsertCell',
		'InsertRowsBefore',
		'InsertRowsAfter',
		'InsertColumnsBefore',
		'InsertColumnsAfter',
		'InsertColumnsMenu',
		'DeleteRows',
		'DeleteColumns',
		'DeleteTable',
		'SetOptimalColumnWidth',
		'TableDialog',
		'GraphicDialog',
		'InsertAnnotation',
		'InsertThreadedComment',
		'SpellingAndGrammarDialog',
		'DecrementLevel',
		'TransformDialog',
		'FormatLine',
		'FormatArea',
		'SetAnchorToPara',
		'SetAnchorAtChar',
		'SetAnchorToChar',
		'WrapOff',
		'WrapOn',
		'WrapIdeal',
		'WrapLeft',
		'WrapRight',
		'WrapThrough',
		'WrapThroughTransparencyToggle',
		'WrapAnchorOnly',
		'WrapContour',
		'BringToFront',
		'ObjectForwardOne',
		'ObjectBackOne',
		'SendToBack',
		'SetObjectToForeground',
		'SetObjectToBackground',
		'InsertCaptionDialog',
		'OpenHyperlinkOnCursor',
		'EditHyperlink',
		'CopyHyperlinkLocation',
		'RemoveHyperlink',
		'RotateRight',
		'RotateLeft',
		'SetAnchorToCell',
		'SetAnchorToCellResize',
		'SetAnchorToPage',
		'FormatCellDialog',
		'InsertSparkline',
		'AcceptTrackedChange',
		'RejectTrackedChange',
		'ReinstateTrackedChange',
		'SetDefault',
		'AnimationEffects',
		'ChangeBezier',
		'InsertHyperlink',
		'FreezePanes',
		'InsertSheetBefore',
		'InsertSheetAfter',
		'Remove',
		'Protect',
		'SetTabBgColor',
		'ResetTabBgColor',
		'Show',
		'Hide',
		'MoveSheetLeft',
		'MoveSheetRight',
		'RowHeight',
		'SetOptimalRowHeight',
		'HideRow',
		'ShowRow',
		'InsertPage',
		'DuplicatePage',
		'DeletePage',
		'PageSetup',
		'ShowSlide',
		'Hideslide',
		'ColumnWidth',
		'HideColumn',
		'ShowColumn',
		'GroupSparklines',
		'UngroupSparklines',
	];

	public static toSnakeCase(text: string) {
		return text
			.replace(/[ _]/gi, '-')
			.replace(/([a-z])([A-Z])/g, '$1-$2')
			.toLowerCase();
	}

	public static createMenuItemLink(name: string, iconCommand: string): string {
		let iconHtml = '<span class="context-menu-icon-spacer"></span>';

		if (IconUtil.commandsWithIcons.includes(iconCommand)) {
			const iconName = LOUtil.getIconNameOfCommand(iconCommand);
			const iconUrl = LOUtil.getImageURL(iconName);
			iconHtml = `<img class="context-menu-icon" src="${iconUrl}" alt>`;
		}

		return LOUtil.sanitize(
			`<a class="context-menu-link ${IconUtil.toSnakeCase(iconCommand)}" href="#"> ${iconHtml} ${name} </a>`,
		);
	}
}

app.IconUtil = IconUtil;
