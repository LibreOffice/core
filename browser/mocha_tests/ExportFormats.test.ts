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

// The list of export formats used to be collected while the menubar and the
// notebookbar were built, so the Get_Export_Formats postMessage API answered
// with whatever the running user interface happened to have created. These
// tests run without any user interface at all.
describe('ExportFormats', function () {
	function setUp(hideExportOption: boolean, extraFormats?: string[]): void {
		(window as any).extraExportFormats = extraFormats ? extraFormats : [];
		app.map = { wopi: { HideExportOption: hideExportOption } } as any;
	}

	function formatsOf(docType: string): string[] {
		app.initExportFormats(docType);
		return app.exportFormats.map((entry) => entry.format);
	}

	it('lists the formats of the document type, without any UI', function () {
		setUp(false);

		nodeassert.deepStrictEqual(formatsOf('text'), [
			'pdf',
			'odt',
			'doc',
			'docx',
			'rtf',
			'epub',
			'html',
			'md',
		]);
		nodeassert.deepStrictEqual(formatsOf('spreadsheet'), [
			'pdf',
			'ods',
			'xls',
			'xlsx',
			'csv',
			'html',
		]);
		nodeassert.deepStrictEqual(formatsOf('drawing'), ['pdf', 'odg', 'png']);
	});

	it('reports formats which can be downloaded, not menu item ids', function () {
		setUp(false);
		app.initExportFormats('text');

		// 'exportdirectpdf' and 'exportepub' are the ids of the menu entries,
		// the integrator gets the format to hand back in Action_Export
		const pdf = app.exportFormats.find((entry) => entry.format === 'pdf');
		nodeassert.strictEqual(pdf?.downloadId, 'exportdirectpdf');
		nodeassert.ok(!app.exportFormats.some((entry) => entry.format === 'as'));
		nodeassert.ok(
			!app.exportFormats.some((entry) => entry.format === 'directpdf'),
		);
	});

	it('offers the extra slide formats only when the server enables them', function () {
		setUp(false);
		nodeassert.ok(!formatsOf('presentation').includes('png'));

		setUp(false, ['impress_png', 'impress_svg']);
		const formats = formatsOf('presentation');
		nodeassert.ok(formats.includes('png'));
		nodeassert.ok(formats.includes('svg'));
		nodeassert.ok(!formats.includes('tiff'));
	});

	it('keeps the formats for the integrator when the UI hides exporting', function () {
		setUp(true);
		app.initExportFormats('text');

		// the list the postMessage API answers with stays complete...
		nodeassert.strictEqual(app.exportFormats.length, 8);
		// ...while no entry of it may be shown
		nodeassert.ok(!app.exportFormats.some((entry) => entry.visible));
		nodeassert.strictEqual(app.isExportEntryVisible('downloadas-odt'), false);
		nodeassert.strictEqual(app.isExportEntryVisible('exportdirectpdf'), false);
		nodeassert.strictEqual(app.isExportEntryVisible('exportas'), false);
	});

	it('hides the entries of formats the document cannot be exported to', function () {
		setUp(false);
		app.initExportFormats('spreadsheet');

		nodeassert.strictEqual(app.isExportEntryVisible('downloadas-ods'), true);
		nodeassert.strictEqual(app.isExportEntryVisible('exportas-pdf'), true);
		nodeassert.strictEqual(app.isExportEntryVisible('downloadas-odt'), false);
		// entries which export nothing are none of our business
		nodeassert.strictEqual(app.isExportEntryVisible('.uno:Bold'), true);
		nodeassert.strictEqual(app.isExportEntryVisible('saveas-ods'), true);
	});
});
