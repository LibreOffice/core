/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'macOS Option/Command + Shift + Arrow text selection', function () {
	beforeEach(function () {
		helper.setupAndLoadDocument('writer/testfile.fodt');

		cy.cGet('#document-container').click();
		helper.clearAllText();
		helper.typeIntoDocument('hello');
		// A plain space typed through a simulated keydown takes a different
		// wire path (a raw "key" message instead of "textinput") and, in this
		// test environment, leaves keyboard selection tracking broken for the
		// rest of the test. Sending the space as a "textinput" message
		// directly avoids that keydown path entirely.
		cy.getFrameWindow().then(function (win) {
			win.app.socket.sendMessage('textinput id=0 text= ');
		});
		helper.typeIntoDocument('world{end}');

		cy.getFrameWindow().then(function (win) {
			this.win = win;
		}.bind(this));
	});

	// The active view's own selection overlay is the "*-text-selections"
	// section that holds polygons once a selection exists (see
	// calc1/split_selection_spec.js for the same pattern on Calc); reading
	// those polygons - not the clipboard, and not TextSelectionMiddleware's
	// TextSelections.isActive(), which tracks only the mouse-drag touch
	// handles, not keyboard-driven selection - reflects a live keyboard
	// selection made via Shift+Arrow.
	function getSelectionPolygons(win) {
		var section = win.app.sectionContainer.sections.find(function (s) {
			return s.name.endsWith('-text-selections');
		});
		return section ? section.polygons : [];
	}

	function assertSameSelection(win, baselineKeys, macKeys) {
		helper.typeIntoDocument(baselineKeys);
		cy.wrap(null, { timeout: 10000 }).should(function () {
			expect(getSelectionPolygons(win), 'baseline selection polygons').to.not.be.empty;
		});
		cy.then(function () {
			var baselinePolygons = getSelectionPolygons(win);

			helper.typeIntoDocument('{end}'); // collapse, back to the same starting point
			cy.wrap(null, { timeout: 10000 }).should(function () {
				expect(getSelectionPolygons(win), 'selection collapsed by End').to.be.empty;
			});

			cy.then(function () {
				win.L.Browser.mac = true;
				helper.typeIntoDocument(macKeys);
			});

			cy.wrap(null, { timeout: 10000 }).should(function () {
				expect(getSelectionPolygons(win), 'a fresh selection').to.deep.equal(baselinePolygons);
			});
		});
	}

	// Before the fix, Option+Shift+Left fell through CanvasTileLayer
	// .postKeyboardEvent's macOS rewrite unchanged and reached core as plain
	// Alt+Shift+Left, which core does not bind to anything, so nothing was
	// selected.
	it('Option+Shift+LeftArrow selects the same span as Ctrl+Shift+LeftArrow (previous word)', function () {
		assertSameSelection(this.win, '{ctrl}{shift}{leftarrow}', '{alt}{shift}{leftarrow}');
	});

	// Before the fix, Command+Shift+Left fell through unchanged and reached
	// core as plain Ctrl+Shift+Left (word selection, the Windows/Linux
	// convention for that chord) instead of Shift+Home (line selection).
	it('Command+Shift+LeftArrow selects the same span as Shift+Home (start of line)', function () {
		assertSameSelection(this.win, '{shift}{home}', '{meta}{shift}{leftarrow}');
	});
});
