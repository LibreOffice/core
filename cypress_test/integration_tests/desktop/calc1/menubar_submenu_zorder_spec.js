/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Menubar drop-down z-order', function () {
	var win;

	beforeEach(function () {
		helper.setupAndLoadDocument('calc/focus.ods');
		// The classic menubar (#menu-*) only exists in the compact UI; the
		// desktop default renders the notebookbar.
		desktopHelper.switchUIToCompact();
		cy.cGet('#menu-data').should('be.visible');
		cy.getFrameWindow().then(function (w) {
			win = w;
		});
		cy.then(function () {
			return helper.processToIdle(win);
		});
	});

	// A tall classic-menubar drop-down (e.g. Data, or its Statistics
	// sub-menu) had its lower entries painted behind the sheet-tab and status
	// bars. .main-nav is hoisted to <body> as a flex item (global.js), so its
	// z-index forms a stacking context; when it sat below the bottom bars
	// (z-index 1000) the trapped menu entries were hidden behind them.
	// The menu must paint above the bottom bars.
	it('tall Data menu entries are not hidden behind the bottom bars', function () {
		// A short viewport forces the (very tall) Data drop-down down into the
		// bottom bars, the same situation high browser zoom produces. Keep the
		// width wide so the menubar does not collapse to a hamburger.
		cy.viewport(1280, 500);

		cy.cGet('#menu-data').click();
		cy.cGet('#menu-data > ul').should('be.visible');

		cy.cGet('#menu-data > ul > li:visible').then(function ($items) {
			// Top edge of the highest visible bottom bar.
			var barTop = win.innerHeight;
			['spreadsheet-toolbar', 'toolbar-down'].forEach(function (id) {
				var bar = win.document.getElementById(id);
				if (bar && bar.offsetParent !== null)
					barTop = Math.min(barTop, bar.getBoundingClientRect().top);
			});
			expect(barTop, 'a bottom bar is visible').to.be.lessThan(win.innerHeight);

			// Pick a real menu entry (has an <a>) whose centre falls inside the
			// bottom-bar band and is on-screen - that is the entry that used to
			// be hidden behind the bar.
			var probe = null;
			for (var i = 0; i < $items.length; i++) {
				var item = $items[i];
				if (!item.querySelector('a'))
					continue;
				var r = item.getBoundingClientRect();
				var midY = r.top + r.height / 2;
				if (r.height > 0 && midY > barTop && midY < win.innerHeight) {
					probe = { x: r.left + r.width / 2, y: midY };
					break;
				}
			}
			expect(probe, 'a menu entry overlaps the bottom-bar region').to.not.be.null;

			// With the fix the menu paints above the bars, so the topmost
			// element at that point belongs to the menu rather than a bar.
			var top = win.document.elementFromPoint(probe.x, probe.y);
			expect(top, 'element at the probed point').to.not.be.null;
			expect(
				top.closest('#main-menu') !== null,
				'topmost element at the probed point belongs to #main-menu, not a bottom bar'
			).to.be.true;
		});
	});
});
