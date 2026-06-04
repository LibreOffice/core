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

declare var JSDialog;

interface MenuCommandsInterface {
	allowlist: {
		general: string[];
		tracking: string[];
		text: string[];
		spreadsheet: string[];
		presentation: string[];
		drawing: string[];
	};
	mobileDenylist: string[];
}

const MenuCommands: MenuCommandsInterface = {
	/*
	 * Enter UNO commands that should appear in the context menu.
	 * Entering a UNO command under `general' would enable it for all types
	 * of documents. If you do not want that, allowlist it in document specific filter.
	 *
	 * UNOCOMMANDS_EXTRACT_START <- don't remove this line, it's used by unocommands.py
	 */
	allowlist: {
		/*
		 * UNO commands for menus are not available sometimes. Presence of Menu commands
		 * in following list is just for reference and ease of locating uno command
		 * from context menu structure.
		 */
		general: ['Cut', 'Copy', 'Paste', 'PasteSpecial', 'Delete',
				  'FormatPaintbrush', 'ResetAttributes',
				  'NumberingStart', 'ContinueNumbering', 'IncrementLevel', 'DecrementLevel',
				  'OpenHyperlinkOnCursor', 'InsertHyperlink', 'EditHyperlink', 'CopyHyperlinkLocation', 'RemoveHyperlink',
				  'AnchorMenu', 'SetAnchorToPage', 'SetAnchorToPara', 'SetAnchorAtChar',
				  'SetAnchorToChar', 'SetAnchorToFrame', 'Crop',
				  'WrapMenu', 'WrapOff', 'WrapOn', 'WrapIdeal', 'WrapLeft', 'WrapRight', 'WrapThrough',
				  'WrapThroughTransparencyToggle', 'WrapContour', 'WrapAnchorOnly',
				  'ConvertMenu', 'ChangeBezier',
				  'DistributeHorzCenter', 'DistributeHorzDistance','DistributeHorzLeft','DistributeHorzRight',
				  'DistributeVertBottom', 'DistributeVertCenter', 'DistributeVertDistance', 'DistributeVertTop',
				  'ArrangeFrameMenu', 'ArrangeMenu', 'BringToFront', 'ObjectForwardOne', 'ObjectBackOne', 'SendToBack',
				  'RotateMenu', 'RotateLeft', 'RotateRight', 'TransformDialog', 'FormatLine', 'FormatArea',
				  'FormatChartArea', 'InsertTitles', 'InsertRemoveAxes',
				  'DeleteLegend', 'DiagramType', 'DataRanges', 'DiagramData', 'View3D', 'ManageThemes',
				  'FormatWall', 'FormatFloor', 'FormatLegend', 'FormatTitle', 'FormatDataSeries',
				  'FormatAxis', 'FormatMajorGrid', 'FormatMinorGrid', 'FormatDataLabels',
				  'FormatDataLabel', 'FormatDataPoint', 'FormatMeanValue', 'FormatXErrorBars', 'FormatYErrorBars',
				  'FormatTrendline', 'FormatTrendlineEquation', 'FormatSelection', 'FormatStockLoss',
				  'FormatStockGain', 'InsertDataLabel', 'InsertDataLabels' , 'DeleteDataLabel', 'DeleteDataLabels', 'ResetDataPoint',
				  'InsertTrendline', 'InsertMeanValue', 'InsertXErrorBars' , 'InsertYErrorBars', 'ResetAllDataPoints' , 'DeleteAxis',
				  'InsertAxisTitle', 'InsertMinorGrid', 'InsertMajorGrid' , 'InsertAxis', 'DeleteMajorGrid' , 'DeleteMinorGrid',
				  'SpellCheckIgnoreAll', 'LanguageStatus', 'SpellCheckApplySuggestion', 'PageDialog',
				  'CompressGraphic', 'SaveGraphic', 'GraphicDialog', 'InsertCaptionDialog',
				  'AnimationEffects', 'ExecuteAnimationEffect',
				  'InsertAnnotation', 'FormatGroup', 'FormatUngroup'],

		tracking: ['NextTrackedChange', 'PreviousTrackedChange', 'RejectTrackedChange', 'AcceptTrackedChange', 'ReinstateTrackedChange'],

		text: ['TableInsertMenu',
			   'InsertRowsBefore', 'InsertRowsAfter', 'InsertColumnsBefore', 'InsertColumnsAfter',
			   'TableDeleteMenu', 'SetObjectToBackground', 'SetObjectToForeground',
			   'DeleteRows', 'DeleteColumns', 'DeleteTable', 'EditCurrentRegion',
			   'MergeCells', 'SetOptimalColumnWidth', 'SetOptimalRowHeight',
			   'SetMinimalColumnWidth', 'SetMinimalRowHeight', 'DistributeRows', 'DistributeColumns',
			   'UpdateCurIndex','RemoveTableOf','EditCurIndex',
			   'ReplyComment', 'DeleteComment', 'DeleteAuthor', 'DeleteAllNotes',
			   'SpellingAndGrammarDialog', 'FontDialog', 'FontDialogForParagraph', 'TableDialog',
			   'SpellCheckIgnore', 'FrameDialog', 'UnfloatFrame', 'ContentControlProperties', 'DeleteContentControl',
			   'AddToWordbook'],

		spreadsheet: ['MergeCells', 'SplitCell', 'InsertCell', 'DeleteCell',
				  'RecalcPivotTable', 'DataDataPilotRun', 'DeletePivotTable', 'CalculatedFieldRun',
				  'InsertCalcTable', 'RemoveCalcTable',
				  'DatabaseSettings', 'FormatCellDialog', 'DeleteNote', 'SetAnchorToCell', 'SetAnchorToCellResize',
				  'FormatSparklineMenu', 'InsertSparkline', 'DeleteSparkline', 'DeleteSparklineGroup',
				  'EditSparklineGroup', 'EditSparkline', 'GroupSparklines', 'UngroupSparklines', 'AutoFill'],

		presentation: ['SetDefault', 'TableInsertMenu',
				  'InsertRowsBefore', 'InsertRowsAfter', 'InsertColumnsBefore', 'InsertColumnsAfter',
				  'TableDeleteMenu', 'DeleteRows', 'DeleteColumns', 'DeleteTable',
				  'SetMinimalRowHeight', 'SetOptimalRowHeight', 'SetMinimalColumnWidth', 'SetOptimalColumnWidth',
				  'DistributeRows', 'DistributeColumns'],
		drawing: []
	},
	// UNOCOMMANDS_EXTRACT_END <- don't remove this line, it's used by unocommands.py

	// This denylist contains those menu items which should be disabled on mobile
	// phones even if they are allowed in general. We need to have only those items here
	// which are also part of the allowlist, otherwise the menu items are not visible
	// anyway.

	// For clarity, please keep this list in sections that are sorted in the same order
	// as the items appear in the allowlist arrays above. Also keep items on separate
	// lines as in the arrays above.

	mobileDenylist: [
		// general
		'PasteSpecial',
		'TransformDialog', 'FormatLine', 'FormatArea',
		'InsertTitles', 'InsertRemoveAxes',
		'DiagramType', 'DataRanges',
		'FormatWall', 'FormatDataSeries', 'FormatXErrorBars', 'FormatYErrorBars',
		'FormatDataPoint', 'FormatAxis', 'FormatMajorGrid', 'FormatMinorGrid',
		'InsertTrendline', 'InsertXErrorBars' , 'InsertYErrorBars', 'FormatChartArea',
		'FormatMeanValue', 'DiagramData', 'FormatLegend', 'FormatTrendline',
		'FormatTrendlineEquation', 'FormatStockLoss', 'FormatStockGain', 'LanguageStatus',
		'PageDialog',
		// text
		'SpellingAndGrammarDialog', 'FontDialog', 'FontDialogForParagraph',
		// spreadsheet
		'FormatCellDialog', 'DataDataPilotRun', 'CalculatedFieldRun', 'InsertCalcTable',
		'GroupSparklines', 'UngroupSparklines', 'AutoFill'
	],
};

JSDialog.MenuCommands = MenuCommands;
