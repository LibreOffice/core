/* global describe expect it cy before require */

const helper = require('../../common/helper');
const calcHelper = require('../../common/calc_helper');

// What a dialog's tooltips owe the pointer in a real browser: the text they
// carry, a placement that does not cover what they describe, and Escape that
// takes the tooltip without taking the dialog. When one opens and how long it
// waits are in the Tooltip mocha tests. The three buttons beside Format code
// are the subject.
describe(['tagdesktop'], 'Tooltip', { testIsolation: false }, function () {
	let win;

	before(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('calc/switch.ods', false, false, undefined,
			'tooltips=true');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		calcHelper.dblClickOnFirstCell();
		cy.then(function () { win.app.map.sendUnoCommand('.uno:FormatCellDialog'); });
		cy.cGet('.ui-dialog[role="dialog"]').should('have.length', 1);
		cy.then(function () { return helper.processToIdle(win); });
		cy.cGet('.ui-dialog #add[data-cooltip]').should('be.visible');
	});

	function tooltip() {
		return win.app.map.tooltip._container;
	}

	function shown() {
		return win.getComputedStyle(tooltip()).visibility === 'visible';
	}

	function label() {
		return tooltip().textContent.trim();
	}

	function widget(id) {
		return win.document.querySelector('.ui-dialog #' + id + '[data-cooltip]');
	}

	/// The harness moves the pointer in jumps, which do not always fire the
	/// leave, so the control's own off/on pair is what clears the tooltip.
	///
	/// A jump scrolls the page and it springs back a frame later, so a widget
	/// the pointer lands on fires a leave after the command has returned, and
	/// that leave arms a hide against whatever is showing by then. The format
	/// code entry carries no tooltip of its own.
	function pointerAway() {
		cy.cGet('.ui-dialog #formatted').realHover();
		cy.wrap(null).should(function () {
			expect(win.document.querySelector('.ui-dialog [data-cooltip]:hover'),
				'a widget still under the pointer').to.be.null;
		});
		cy.then(function () {
			win.app.map.tooltip.disable();
			win.app.map.tooltip.enable();
		});
		cy.wrap(null).should(function () {
			expect(shown(), 'no tooltip left over').to.be.false;
		});
	}

	/// The pointer has to be on the widget for the leave to fire later; show()
	/// opens the tooltip because the harness's jumps do not always enter.
	function hover(id) {
		cy.cGet('.ui-dialog #' + id + '[data-cooltip]').realHover();
		cy.then(function () { win.app.map.tooltip.show(widget(id)); });
		cy.wrap(null).should(function () {
			expect(shown(), 'the tooltip of ' + id + ' opened').to.be.true;
		});
	}

	it('shows the text the widget it describes carries', function () {
		pointerAway();
		hover('add');

		cy.then(function () {
			expect(label(), 'the tooltip of the Add button')
				.to.equal(widget('add').getAttribute('data-cooltip'));
		});
	});

	it('hides on Escape, without taking the dialog with it', function () {
		pointerAway();
		hover('add');

		cy.realPress('Escape');

		cy.wrap(null).should(function () {
			expect(shown(), 'Escape took the tooltip down').to.be.false;
		});

		cy.cGet('.ui-dialog[role="dialog"]').should('have.length', 1);
	});

	it('does not stand in the way of what it covers', function () {
		pointerAway();
		hover('add');

		cy.then(function () {
			const box = tooltip().getBoundingClientRect();
			const at = win.document.elementFromPoint(
				box.left + box.width / 2, box.top + box.height / 2);

			expect(at && at.closest('#cooltip'),
				'what owns the point the tooltip is drawn on').to.equal(null);
		});
	});
});
