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
 * app.exportFormats - the formats the loaded document can be exported to.
 *
 * The list is built once the document type is known, so it does not depend on
 * any part of the user interface being created: the menubar, the notebookbar
 * and the Get_Export_Formats postMessage API all take the formats from here.
 *
 * HideExportOption only clears the visible flag: the integrator keeps the full
 * list, so it can offer export buttons of its own around the iframe.
 */

interface ExportFormat {
	format: string; // format name, as used by downloadas and by the postMessage API
	label: string; // translated name of the format
	downloadId: string; // id of the user interface entry downloading this format
	storage: boolean; // format can also be exported to the storage ('Export as')
	visible: boolean; // the user interface may offer this format
}

function makeExportFormat(
	format: string,
	label: string,
	storage?: boolean,
	downloadId?: string,
): ExportFormat {
	return {
		format: format,
		label: label,
		downloadId: downloadId ? downloadId : 'downloadas-' + format,
		storage: storage === true,
		visible: true,
	};
}

app.getExportFormats = function (docType: string): ExportFormat[] {
	// the mobile apps have no export dialogs, they download the file instead
	const pdf = makeExportFormat(
		'pdf',
		_('PDF Document (.pdf)'),
		true,
		window.ThisIsAMobileApp ? undefined : 'exportdirectpdf',
	);
	const epub = makeExportFormat(
		'epub',
		_('EPUB (.epub)'),
		true,
		window.ThisIsAMobileApp ? undefined : 'exportepub',
	);

	if (docType === 'text')
		return [
			pdf,
			makeExportFormat('odt', _('ODF text document (.odt)')),
			makeExportFormat('doc', _('Word 2003 Document (.doc)')),
			makeExportFormat('docx', _('Word Document (.docx)')),
			makeExportFormat('rtf', _('Rich Text (.rtf)')),
			epub,
			makeExportFormat('html', _('HTML file (.html)')),
			makeExportFormat('md', _('Markdown (.md)'), true),
		];

	if (docType === 'spreadsheet')
		return [
			pdf,
			makeExportFormat('ods', _('ODF spreadsheet (.ods)')),
			makeExportFormat('xls', _('Excel 2003 Spreadsheet (.xls)')),
			makeExportFormat('xlsx', _('Excel Spreadsheet (.xlsx)')),
			makeExportFormat('csv', _('CSV file (.csv)')),
			makeExportFormat('html', _('HTML file (.html)')),
		];

	if (docType === 'presentation') {
		const formats = [
			pdf,
			makeExportFormat('odp', _('ODF presentation (.odp)')),
			makeExportFormat('ppt', _('PowerPoint 2003 Presentation (.ppt)')),
			makeExportFormat('pptx', _('PowerPoint Presentation (.pptx)')),
			makeExportFormat('odg', _('ODF Drawing (.odg)')),
			makeExportFormat('html', _('HTML Document (.html)')),
		];

		// these are only offered when the server was configured to show them
		const extras = [
			makeExportFormat('svg', _('Scalable Vector Graphics (.svg)')),
			makeExportFormat('bmp', _('Current slide as Bitmap (.bmp)')),
			makeExportFormat(
				'gif',
				_('Current slide as Graphics Interchange Format (.gif)'),
			),
			makeExportFormat(
				'png',
				_('Current slide as Portable Network Graphics (.png)'),
			),
			makeExportFormat(
				'tiff',
				_('Current slide as Tag Image File Format (.tiff)'),
			),
		];
		extras.forEach((entry) => {
			if (window.extraExportFormats.includes('impress_' + entry.format))
				formats.push(entry);
		});

		return formats;
	}

	if (docType === 'drawing')
		return [
			pdf,
			makeExportFormat('odg', _('ODF Drawing (.odg)')),
			makeExportFormat('png', _('Image (.png)')),
		];

	return [];
};

app.initExportFormats = function (docType: string): void {
	app.exportFormats = app.getExportFormats(docType);
	app.updateExportFormatsVisibility();
};

app.updateExportFormatsVisibility = function (): void {
	const hidden = !!(app.map && app.map.wopi && app.map.wopi.HideExportOption);
	app.exportFormats.forEach((entry: ExportFormat) => {
		entry.visible = !hidden;
	});
};

// Which format a 'Download as' or 'Export as' user interface entry exports.
function exportEntryFormat(id: string): string | null {
	if (id.startsWith('downloadas-')) return id.substring('downloadas-'.length);
	if (id.startsWith('exportas-')) return id.substring('exportas-'.length);
	if (id === 'exportdirectpdf' || id === 'exportpdf') return 'pdf';
	if (id === 'exportepub') return 'epub';
	return null;
}

/// Whether the user interface may show the entry with the given id. Entries
/// which export nothing are left alone, so this can be asked about any id.
app.isExportEntryVisible = function (id: string): boolean {
	if (id === 'downloadas' || id === 'exportas')
		return app.exportFormats.some((entry: ExportFormat) => entry.visible);

	const format = exportEntryFormat(id);
	if (format === null) return true;

	const entry = app.exportFormats.find(
		(entry: ExportFormat) => entry.format === format,
	);
	return entry !== undefined && entry.visible;
};
