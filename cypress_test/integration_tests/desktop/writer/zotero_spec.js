/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

// Tests for the Zotero citation control (browser/src/control/Control.Zotero.js).
//
// The control normally talks to api.zotero.org and www.zotero.org. To test it
// without a Zotero account or network access we instantiate the control, force
// it into the "enabled" state, and pre-set the citation-format settings that
// would otherwise come from a fetched CSL style. Command-emission tests then
// stub sendUnoCommand and assert the exact command and parameters the control
// sends to core for each of the three storage backends: ReferenceMark (ODF),
// Field (DOCX form field) and Bookmark. The delete test drives real editing
// and reads the caret from app.file.textCursor.rectangle.

// Instantiate app.map.zotero (if absent) and put it in a usable, enabled state.
function enableZotero(win, options) {
	options = options || {};
	var zotero = win.app.map.zotero;
	if (!zotero) {
		zotero = win.L.control.zotero(win.app.map);
		win.app.map.zotero = zotero;
		win.app.map.addControl(zotero);
	}

	zotero.apiKey = 'test-api-key';
	zotero.userID = '12345';
	zotero.enable = true;
	// The unreachable-items check needs the fetched group list; skip it.
	zotero.showUnsupportedWarning = false;

	zotero.settings.style = 'test-style';
	zotero.settings.locale = 'en-US';
	zotero.settings.citationFormat = options.citationFormat || 'numeric';
	zotero.settings.layout = { prefix: '', suffix: '', delimiter: '; ' };
	zotero.settings.group = { prefix: '', suffix: '', delimiter: ', ' };
	zotero.settings.hasBibliography = '1';
	zotero.settings.bibliographyStyleHasBeenSet = '0';

	if (options.fieldType)
		zotero.settings.fieldType = options.fieldType;

	// The list refresh fires a commandvalues request over the socket. The
	// command-emission tests keep it stubbed out; the citations-list tests
	// pass stubSocket: false to let the real round-trip through.
	if (options.stubSocket !== false)
		cy.stub(win.app.socket, 'sendMessage').callsFake(function () {});

	return zotero;
}

// A mock library item in the shape insertCitation()/getCitationJSONString() consume.
function mockCitationItem(key, label) {
	return {
		key: key,
		citation: '<span>' + label + '</span>',
		csljson: { id: key, type: 'document', title: 'Title ' + key },
		links: { self: { href: 'https://www.zotero.org/testuser/items/' + key } },
	};
}

// Parameters passed to the sendUnoCommand call for the given command.
function paramsFor(win, command) {
	var call = win.app.map.sendUnoCommand.getCalls().find(function(c) {
		return c.args[0] === command;
	});
	expect(call, 'sendUnoCommand("' + command + '") was called').to.not.be.undefined;
	return call.args[1];
}

function caretY(win) {
	return Math.round(win.app.file.textCursor.rectangle.y1);
}

describe(['tagdesktop'], 'Zotero citations', function() {

	describe('Insert citation', function() {
		beforeEach(function() {
			cy.viewport(1400, 1000);
			helper.setupAndLoadDocument('writer/copy_paste_simple.odt');
			cy.getFrameWindow().then((win) => { this.win = win; });
		});

		it('as reference mark', function() {
			const win = this.win;
			cy.then(() => {
				const z = enableZotero(win, { fieldType: 'ReferenceMark' });
				cy.stub(win.app.map, 'sendUnoCommand');
				z.insertNewCitation = true;
				z.insertCitation([mockCitationItem('KEY1', '[1]')]);
			});
			cy.then(() => {
				const p = paramsFor(win, '.uno:InsertField');
				expect(p.TypeName.value).to.equal('SetRef');
				expect(p.Name.value).to.match(/^ZOTERO_ITEM CSL_CITATION /);
				expect(p.NeverExpand.value).to.equal(true);
				expect(p.Content.value).to.contain('1');
			});
		});

		it('as form field', function() {
			const win = this.win;
			cy.then(() => {
				const z = enableZotero(win, { fieldType: 'Field' });
				cy.stub(win.app.map, 'sendUnoCommand');
				z.insertNewCitation = true;
				z.insertCitation([mockCitationItem('KEY1', '[1]')]);
			});
			cy.then(() => {
				const p = paramsFor(win, '.uno:TextFormField');
				expect(p.FieldType.value).to.equal('vnd.oasis.opendocument.field.UNHANDLED');
				expect(p.FieldCommand.value).to.match(/^ADDIN ZOTERO_ITEM CSL_CITATION /);
				expect(p.FieldResult.value).to.contain('1');
			});
		});

		it('as bookmark', function() {
			const win = this.win;
			cy.then(() => {
				const z = enableZotero(win, { fieldType: 'Bookmark' });
				cy.stub(win.app.map, 'sendUnoCommand');
				z.insertNewCitation = true;
				z.insertCitation([mockCitationItem('KEY1', '[1]')]);
			});
			cy.then(() => {
				const p = paramsFor(win, '.uno:InsertBookmark');
				expect(p.Bookmark.value).to.match(/^ZOTERO_BREF_/);
				expect(p.BookmarkText.value).to.contain('1');
			});
		});

		it('update existing reference mark', function() {
			const win = this.win;
			cy.then(() => {
				const z = enableZotero(win, { fieldType: 'ReferenceMark' });
				cy.stub(win.app.map, 'sendUnoCommand');
				z.insertNewCitation = false;
				z.insertCitation([mockCitationItem('KEY1', '[1]')]);
			});
			cy.then(() => {
				paramsFor(win, '.uno:UpdateField');
			});
		});
	});

	describe('Insert bibliography', function() {
		const BIB_HTML = '<div class="csl-entry">A reference</div>';

		beforeEach(function() {
			cy.viewport(1400, 1000);
			helper.setupAndLoadDocument('writer/copy_paste_simple.odt');
			cy.getFrameWindow().then((win) => { this.win = win; });
		});

		it('as section', function() {
			const win = this.win;
			cy.then(() => {
				const z = enableZotero(win, { fieldType: 'ReferenceMark' });
				cy.stub(win.app.map, 'sendUnoCommand');
				z.sendInsertBibCommand(BIB_HTML);
			});
			cy.then(() => {
				const p = paramsFor(win, '.uno:InsertSection');
				expect(p.RegionName.value).to.match(/^ZOTERO_BIBL .* CSL_BIBLIOGRAPHY/);
				expect(p.Content.value).to.equal(BIB_HTML);
			});
		});

		it('as form field', function() {
			const win = this.win;
			cy.then(() => {
				const z = enableZotero(win, { fieldType: 'Field' });
				cy.stub(win.app.map, 'sendUnoCommand');
				z.sendInsertBibCommand(BIB_HTML);
			});
			cy.then(() => {
				const p = paramsFor(win, '.uno:TextFormField');
				expect(p.FieldCommand.value).to.match(/^ADDIN ZOTERO_BIBL .* CSL_BIBLIOGRAPHY$/);
				expect(p.FieldResult.value).to.equal(BIB_HTML);
			});
		});

		it('as bookmark', function() {
			const win = this.win;
			cy.then(() => {
				const z = enableZotero(win, { fieldType: 'Bookmark' });
				cy.stub(win.app.map, 'sendUnoCommand');
				z.sendInsertBibCommand(BIB_HTML);
			});
			cy.then(() => {
				const p = paramsFor(win, '.uno:InsertBookmark');
				expect(p.Bookmark.value).to.match(/^ZOTERO_BREF_/);
				expect(p.BookmarkText.value).to.equal(BIB_HTML);
			});
		});
	});

	describe('Unlink citations', function() {
		beforeEach(function() {
			cy.viewport(1400, 1000);
			helper.setupAndLoadDocument('writer/copy_paste_simple.odt');
			cy.getFrameWindow().then((win) => { this.win = win; });
		});

		it('deletes reference marks and sections', function() {
			const win = this.win;
			cy.then(() => {
				const z = enableZotero(win, { fieldType: 'ReferenceMark' });
				cy.stub(win.app.map, 'sendUnoCommand');
				z.handleUnlinkCitations();
			});
			cy.then(() => {
				const del = paramsFor(win, '.uno:DeleteFields');
				expect(del.TypeName.value).to.equal('SetRef');
				expect(del.NamePrefix.value).to.equal('ZOTERO_ITEM CSL_CITATION');
				const sec = paramsFor(win, '.uno:DeleteSections');
				expect(sec.SectionNamePrefix.value).to.equal('ZOTERO_BIBL');
			});
		});

		it('deletes form fields', function() {
			const win = this.win;
			cy.then(() => {
				const z = enableZotero(win, { fieldType: 'Field' });
				cy.stub(win.app.map, 'sendUnoCommand');
				z.handleUnlinkCitations();
			});
			cy.then(() => {
				const p = paramsFor(win, '.uno:DeleteTextFormFields');
				expect(p.FieldCommandPrefix.value).to.equal('ADDIN ZOTERO_');
			});
		});

		it('deletes bookmarks', function() {
			const win = this.win;
			cy.then(() => {
				const z = enableZotero(win, { fieldType: 'Bookmark' });
				cy.stub(win.app.map, 'sendUnoCommand');
				z.handleUnlinkCitations();
			});
			cy.then(() => {
				const p = paramsFor(win, '.uno:DeleteBookmarks');
				expect(p.BookmarkNamePrefix.value).to.equal('ZOTERO_BREF_');
			});
		});

		it('asks for confirmation', function() {
			const win = this.win;
			cy.then(() => {
				enableZotero(win, { fieldType: 'ReferenceMark' });
				win.app.map.zotero.unlinkCitations();
			});
			cy.cGet('#zoterounlinkwarn').should('exist');
		});
	});

	describe('Delete citation', function() {
		// zotero.odt has three paragraphs: "[1]" (a citation reference mark),
		// "sdfsdf", and "[2]" (a second citation, the last line).
		beforeEach(function() {
			cy.viewport(1400, 1000);
			helper.setupAndLoadDocument('writer/zotero.odt');
			cy.getFrameWindow().then((win) => { this.win = win; });
		});

		it('caret stays put after refresh', function() {
			const win = this.win;
			const data = {};

			// Reference Y for the first citation's line and the last citation's line.
			helper.typeIntoDocument('{ctrl}{home}');
			cy.then(() => helper.processToIdle(win));
			cy.then(() => { data.firstLineY = caretY(win); });
			helper.typeIntoDocument('{ctrl}{end}');
			cy.then(() => helper.processToIdle(win));
			cy.then(() => { data.lastLineY = caretY(win); });

			// Delete the citation "[2]" on the last line.
			cy.then(() => helper.typeIntoDocument('{backspace}{backspace}{backspace}'));
			cy.then(() => helper.processToIdle(win));
			cy.then(() => { data.afterDeleteY = caretY(win); });

			// Refresh the remaining citations, as the Zotero-enabled client does
			// automatically after a citation is deleted.
			cy.then(() => win.app.map.sendUnoCommand('.uno:UpdateFields', {
				'TypeName': { 'type': 'string', 'value': 'SetRef' },
				'NamePrefix': { 'type': 'string', 'value': 'ZOTERO_ITEM CSL_CITATION' },
				'NeverExpand': { 'type': 'boolean', 'value': true },
				'Fields': {
					'type': '[][]com.sun.star.beans.PropertyValue',
					'value': [
						{
							'Name': { 'type': 'string', 'value': 'ZOTERO_ITEM CSL_CITATION {"citationID":"LhYhRckguI"} RNDnDmW9RDFck' },
							'Content': { 'type': 'string', 'value': '[1]' },
						},
					],
				},
			}, true));
			cy.then(() => helper.processToIdle(win));
			cy.then(() => { data.afterRefreshY = caretY(win); });

			cy.then(() => {
				// The two citations are on different lines, so a jump is detectable.
				expect(data.lastLineY - data.firstLineY, 'last line below first line')
					.to.be.greaterThan(50);
				// Deleting the citation leaves the caret on the last line.
				expect(Math.abs(data.afterDeleteY - data.lastLineY), 'caret on last line after delete')
					.to.be.lessThan(30);
				// The refresh must not move the caret up to the first citation.
				expect(Math.abs(data.afterRefreshY - data.lastLineY), 'caret still on last line after refresh')
					.to.be.lessThan(30);
			});
		});
	});

	describe('Citations list', function() {
		// The citations map is rebuilt from the field list core reports, so it
		// must reflect the document after an insert or a delete. zotero.odt
		// holds two citations with item keys V5D7Z3EL ("[1]") and EVCF32P3
		// ("[2]").
		beforeEach(function() {
			cy.viewport(1400, 1000);
			helper.setupAndLoadDocument('writer/zotero.odt');
			cy.getFrameWindow().then((win) => { this.win = win; });
		});

		function setupListTest(win) {
			const z = enableZotero(win, { fieldType: 'ReferenceMark', stubSocket: false });
			// The citation update would call api.zotero.org; the list itself
			// is built from the field names, so keep the update out.
			cy.stub(z, 'updateCitations').callsFake(function () {});
			return z;
		}

		function citationKeys(win) {
			return Object.keys(win.app.map.zotero.citations);
		}

		it('lists the citations in the document', function() {
			const win = this.win;
			cy.then(() => { setupListTest(win).updateFieldsList(); });
			cy.wrap(null).should(() => {
				expect(citationKeys(win)).to.have.members(['V5D7Z3EL', 'EVCF32P3']);
			});
		});

		it('grows after insert', function() {
			const win = this.win;
			cy.then(() => { setupListTest(win).updateFieldsList(); });
			cy.wrap(null).should(() => {
				expect(citationKeys(win)).to.have.length(2);
			});

			helper.typeIntoDocument('{ctrl}{end}');
			cy.then(() => helper.processToIdle(win));
			// Insert a third citation; this refreshes the field list itself.
			cy.then(() => {
				const z = win.app.map.zotero;
				z.insertNewCitation = true;
				z.insertCitation([mockCitationItem('KEY3', '[3]')]);
			});
			cy.wrap(null).should(() => {
				expect(citationKeys(win)).to.have.members(['V5D7Z3EL', 'EVCF32P3', 'KEY3']);
			});
		});

		it('shrinks after delete', function() {
			const win = this.win;
			cy.then(() => { setupListTest(win).updateFieldsList(); });
			cy.wrap(null).should(() => {
				expect(citationKeys(win)).to.have.length(2);
			});

			// Delete the citation "[2]" at the end of the document.
			helper.typeIntoDocument('{ctrl}{end}');
			cy.then(() => helper.processToIdle(win));
			cy.then(() => helper.typeIntoDocument('{backspace}{backspace}{backspace}'));
			cy.then(() => helper.processToIdle(win));

			cy.then(() => { win.app.map.zotero.updateFieldsList(); });
			cy.wrap(null).should(() => {
				expect(citationKeys(win)).to.have.members(['V5D7Z3EL']);
			});
		});
	});
});
