/* global describe expect it cy before require */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');

// The dialog case is covered by a11y_tooltip_spec; these are the surfaces the
// auditor found it on instead. Two things are asked of content shown on hover:
// that it is drawn beside what it describes, and that it survives the pointer
// travelling to it.
describe(['tagdesktop'], 'Tooltip reach', { testIsolation: false }, function () {
	let win;

	// The furthest Control.Tooltip.position offsets a placement from its
	// trigger.
	const MAX_GAP = 12;

	before(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('calc/switch.ods', false, false, undefined,
			'tooltips=true');

		cy.getFrameWindow().then(function (frameWindow) { win = frameWindow; });
	});

	function tooltip() {
		return win.app.map.tooltip._container;
	}

	function shown() {
		return win.getComputedStyle(tooltip()).visibility === 'visible';
	}

	function triggerIn(selector) {
		return Array.from(win.document.querySelectorAll(selector + ' [data-cooltip]'))
			.filter(function (el) {
				return el.getAttribute('data-cooltip') &&
					el.checkVisibility({ visibilityProperty: true });
			})[0];
	}

	// mouseLeave clears the flag that a hover sets to hold the tooltip open, so
	// the pair takes it down whatever state the previous surface left behind.
	function clear() {
		cy.then(function () {
			win.app.map.tooltip.mouseLeave();
			win.app.map.tooltip.hide();
		});
		cy.wrap(null).should(function () {
			expect(shown(), 'no tooltip left over').to.be.false;
		});
	}

	function assertBesideItsTrigger(el, where) {
		const rect = el.getBoundingClientRect();
		const tip = tooltip().getBoundingClientRect();
		const gap = Math.max(
			tip.top - rect.bottom, rect.top - tip.bottom,
			tip.left - rect.right, rect.left - tip.right, 0);

		expect(gap, where + ': the tooltip is drawn this far from what it describes')
			.to.be.at.most(MAX_GAP);
	}

	function assertSurvivesTheCrossing(el, where) {
		const rect = el.getBoundingClientRect();
		const tip = tooltip().getBoundingClientRect();
		const samples = [];
		const x = Math.max(rect.left, Math.min(tip.left + tip.width / 2, rect.right));
		const y = tip.top > rect.bottom ? rect.bottom + 2 : rect.top - 2;

		cy.cGet('body').realMouseMove(x, y);

		for (let i = 0; i < 3; i++) {
			cy.wait(150);
			cy.then(function () {
				samples.push((i + 1) * 150 + 'ms: ' + (shown() ? 'shown' : 'gone'));
			});
		}

		cy.then(function () {
			expect(samples.join(', '), where + ': the tooltip while the pointer crosses to it')
				.to.not.contain('gone');
		});
	}

	/// A surface that carries no tooltip at all has nothing to ask this of, and
	/// which surfaces do differs between branches, so say so in the title
	/// rather than assert a widget that was never there.
	function checkSurface(ctx, selector, where) {
		let el = triggerIn(selector);

		if (!el) {
			ctx._runnable.title += ' (skipped: ' + where + ' carries no tooltip here)';
			ctx.skip();
		}

		clear();

		cy.then(function () {
			el = triggerIn(selector);
			expect(el, where + ' offers a widget with a tooltip').to.not.be.undefined;
		});

		cy.then(function () {
			cy.wrap(el).realHover();
			cy.then(function () { win.app.map.tooltip.show(el); });
		});

		cy.wrap(null).should(function () {
			expect(shown(), where + ': the tooltip opened').to.be.true;
		});

		cy.then(function () { assertBesideItsTrigger(el, where); });
		cy.then(function () { assertSurvivesTheCrossing(el, where); });
	}

	describe('in the notebookbar', function () {
		before(function () {
			desktopHelper.switchUIToNotebookbar();
			cy.cGet('#Home-container').should('be.visible');
		});

		it('a toolitem answers the pointer that comes for its tooltip', function () {
			checkSurface(this, '#Home-container', 'the Home tab');
		});
	});

	describe('in the sidebar', function () {
		before(function () {
			cy.cGet('#sidebar-dock-wrapper').should('be.visible');
		});

		it('a sidebar button answers the pointer that comes for its tooltip', function () {
			checkSurface(this, '#sidebar-dock-wrapper', 'the sidebar');
		});
	});


	describe('in the classic menubar', function () {
		before(function () {
			desktopHelper.switchUIToCompact();
			cy.cGet('#main-menu').should('be.visible');
		});

		it('a menu entry answers the pointer that comes for its tooltip', function () {
			checkSurface(this, '#main-menu', 'the menubar');
		});

		it('and so does a top toolbar button', function () {
			checkSurface(this, '#toolbar-up', 'the top toolbar');
		});
	});

	describe('in the Standard Filter dialog', function () {
		before(function () {
			cy.then(function () {
				win.app.map.sendUnoCommand('.uno:DataFilterStandardFilter');
			});
			cy.cGet('.ui-dialog[role="dialog"]').should('have.length', 1);
			cy.then(function () { return helper.processToIdle(win); });
		});

		it('a dialog button answers the pointer that comes for its tooltip', function () {
			checkSurface(this, '.ui-dialog', 'Standard Filter');
		});
	});
});
