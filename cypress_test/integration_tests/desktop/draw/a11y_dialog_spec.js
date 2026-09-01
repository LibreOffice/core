/* global describe expect it cy before after afterEach require */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');
const a11yHelper = require('../../common/a11y_helper');

// The three .ui files SdrawDialogList names, with the command that opens each.
const allDrawDialogs = [
	'.uno:FontDialog',      // modules/sdraw/ui/drawchardialog.ui
	'.uno:PageSetup',       // modules/sdraw/ui/drawpagedialog.ui
	'.uno:ParagraphDialog', // modules/sdraw/ui/drawparadialog.ui
];

// 'common' dialogs draw does not have
const excludedCommonDialogs = [
	'.uno:AcceptTrackedChanges',
	// writer-only command, not dispatched in draw
	'.uno:SecurityLabel',
	// handleDialog clicks an existing signature row; no draw fixture is signed
	'.uno:Signature',
	'.uno:SpellingAndGrammarDialog', // does not open in draw, SpellDialog is the equivalent
	'.uno:StyleNewByExample', // command dispatches but does not surface a dialog in draw
	// handleDialog clicks #durationbutton-button, hidden without a Duration
	// custom property
	'.uno:SetDocumentProperties',
	'.uno:SplitCell', // needs a table cell selected, which draw has no fixture for
];

describe(['tagdesktop'], 'Accessibility Draw Dialog Tests', { testIsolation: false }, function () {
	let win;
	let hasLinguisticData = false;

	before(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('draw/to_curve.fodg', /*isMultiUser=*/false, /*copyCertificates=*/true);

		desktopHelper.switchUIToNotebookbar();

		helper.setDummyClipboardForCopy();

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
			a11yHelper.enableUICoverage(win);
		});

		cy.cGet('.jsdialog-window').should('not.exist');

		cy.then(() => {
			return helper.processToIdle(win);
		});

		cy.then(() => {
			const thesaurusState = win.app.map.stateChangeHandler.getItemValue('.uno:ThesaurusDialog');
			hasLinguisticData = (thesaurusState === 'enabled');
		});

		// Several common dialogs need a text insertion point.
		enterShapeTextEditing();
	});

	// Tab walks objects in document order; a click point moves with the viewport.
	function enterShapeTextEditing() {
		helper.typeIntoDocument('{esc}');
		// Closing a dialog gives the keyboard focus back to the document
		// asynchronously. Wait for the focus to arrive, so the Tab below is
		// delivered to the document rather than to whatever held the focus
		// meanwhile.
		helper.assertFocus('className', 'clipboard');
		cy.realPress('Tab');
		cy.cGet('#test-div-shapeHandlesSection').should('exist');
		helper.typeIntoDocument('{enter}');
		cy.then(function () {
			return helper.processToIdle(win);
		});
	}

	after(function () {
		a11yHelper.reportUICoverage(win, hasLinguisticData);

		cy.get('@uicoverageResult').then(result => {
			expect(result.used, 'used .ui files').to.not.be.empty;
			expect(result.CompleteDrawDialogCoverage,
				`complete draw dialog coverage; missing: ${JSON.stringify(result.MissingDrawDialogCoverage)}`).to.be.true;
			// The common list is not asserted: most of what it still reports
			// is reachable only through the dialogs excluded above.
		});
	});

	afterEach(function () {
		cy.cGet('body').then($body => {
			const dialogs = $body.find('.jsdialog-window .ui-dialog-titlebar-close');
			if (dialogs.length > 0) {
				for (let i = dialogs.length - 1; i >= 0; i--) {
					cy.wrap(dialogs[i]).click({ force: true });
				}
			}
		});
		cy.cGet('.jsdialog-window:not(.ui-overflow-group-popup):not(.snackbar)').should('not.exist');

		a11yHelper.resetState();

		// put the cursor back in the shape text for the next dialog
		helper.typeIntoDocument('{esc}{esc}');
		enterShapeTextEditing();
	});

	a11yHelper.allCommonDialogs.forEach(function (commandSpec) {
		const command = typeof commandSpec === 'string' ? commandSpec : commandSpec.command;
		if (excludedCommonDialogs.includes(command)) {
			// silently skip the common dialogs that draw doesn't have
			return;
		} else {
			it(`Common Dialog ${command}`, function () {
				if (!hasLinguisticData && a11yHelper.needsLinguisticData(command)) {
					this._runnable.title += ' (skipped: missing linguistic data)';
					this.skip();
				}
				a11yHelper.testDialog(win, commandSpec);
			});
		}
	});

	it('Transform dialog', function () {
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:BasicShapes.octagon');
		});
		cy.cGet('#test-div-shapeHandlesSection').should('exist');

		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:TransformDialog');
		});
		a11yHelper.handleDialog(win, 1, '.uno:TransformDialog');

		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:FormatArea');
		});
		a11yHelper.handleDialog(win, 1, '.uno:FormatArea');

		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:FormatLine');
		});
		a11yHelper.handleDialog(win, 1, '.uno:FormatLine');

		helper.typeIntoDocument('{esc}');
	});

	it('Object dialog', function () {
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:BasicShapes.octagon');
		});
		cy.cGet('#test-div-shapeHandlesSection').should('exist');

		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:NameGroup');
		});
		a11yHelper.handleDialog(win, 1, '.uno:NameGroup');

		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:ObjectTitleDescription');
		});
		a11yHelper.handleDialog(win, 1, '.uno:ObjectTitleDescription');

		helper.typeIntoDocument('{esc}');
	});

	it('Graphic dialog', function () {
		// Leave the shape text editing and drop the shape selection, and wait
		// for its selection overlay to go away. insertImage waits for the
		// selection overlay of the inserted image, so an overlay left over
		// from the shape would satisfy that wait before the image exists.
		helper.typeIntoDocument('{esc}{esc}');
		cy.cGet('#document-container svg g').should('not.exist');

		desktopHelper.insertImage();

		cy.then(() => {
			return helper.processToIdle(win);
		});

		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:CompressGraphic');
		});
		a11yHelper.handleDialog(win, 1, '.uno:CompressGraphic');

		helper.typeIntoDocument('{esc}');
		desktopHelper.selectZoomLevel('100', false);
	});

	allDrawDialogs.forEach(function (commandSpec) {
		const command = typeof commandSpec === 'string' ? commandSpec : commandSpec.command;
		it(`Draw Dialog ${command}`, function () {
			a11yHelper.testDialog(win, commandSpec);
		});
	});
});
