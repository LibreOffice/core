/* global describe expect it cy before require */

const helper = require('../../common/helper');
const calcHelper = require('../../common/calc_helper');

// What content shown on hover owes the pointer: it can be reached, it can be
// dismissed, and it stays while the pointer is on it. The three buttons beside
// Format code are the subject: a dialog does not rebuild itself under the
// pointer, which is what makes the same check flaky on the sidebar.
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

	it('stays while the pointer crosses the gap to it', function () {
		const samples = [];
		let gap;

		pointerAway();
		hover('add');

		cy.then(function () {
			const rect = widget('add').getBoundingClientRect();
			const tip = tooltip().getBoundingClientRect();

			gap = Math.max(tip.top - rect.bottom, rect.top - tip.bottom, 0);
			expect(gap, 'the tooltip is drawn away from the button').to.be.greaterThan(0);

			cy.cGet('body').realMouseMove(rect.left + rect.width / 2,
				tip.top > rect.bottom ? rect.bottom + 2 : rect.top - 2);
		});

		for (let i = 0; i < 3; i++) {
			cy.wait(150);
			cy.then(function () {
				samples.push((i + 1) * 150 + 'ms: ' + (shown() ? 'shown' : 'gone'));
			});
		}

		cy.then(function () {
			expect(samples.join(', '),
				'gap of ' + gap + 'px, the tooltip while the pointer crosses it')
				.to.not.contain('gone');
		});
	});

	it('stays while the pointer rests on it', function () {
		pointerAway();

		// Leaving the widget arms the hide; arriving on the tooltip cancels it.
		cy.then(function () {
			const tip = win.app.map.tooltip;

			tip.show(widget('add'));
			tip.beginHide();
			tip.mouseEnter();
		});

		cy.wait(900);

		cy.then(function () {
			expect(shown(), 'the tooltip under the pointer, past the grace period')
				.to.be.true;
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

	it('hides only for the widget that armed the hide', function () {
		pointerAway();

		// Add arms its hide, Edit opens before that hide falls due.
		cy.then(function () {
			const tip = win.app.map.tooltip;

			tip.show(widget('add'));
			tip.beginHide();
			tip.show(widget('edit'));
		});

		// Wait for that hide to fall due.
		helper.waitForTimers(win, 'tooltip');

		cy.then(function () {
			expect(shown(), 'the tooltip Edit opened').to.be.true;
			expect(label(), 'and it still says what Edit carries')
				.to.equal(widget('edit').getAttribute('data-cooltip'));
		});
	});

	it('hides after the pointer leaves across a neighbour it never showed', function () {
		pointerAway();

		// Delete is crossed too fast to show, so the hide it arms is the only
		// one left to take Add's tooltip down.
		cy.then(function () {
			const tip = win.app.map.tooltip;

			tip.show(widget('add'));
			tip.beginHide();
			tip.beginShow(widget('delete'));
			tip.beginHide();
		});

		cy.wrap(null).should(function () {
			expect(shown(), 'the tooltip Add opened').to.be.false;
		});
	});
});
