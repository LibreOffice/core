/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Accelerator info box z-order', function () {
	var win;

	beforeEach(function () {
		helper.setupAndLoadDocument('calc/focus.ods');
		desktopHelper.switchUIToNotebookbar();
		cy.cGet('#Home-tab-label').should('be.visible');
		cy.getFrameWindow().then(function (w) {
			win = w;
		});
		cy.then(function () {
			return helper.processToIdle(win);
		});
		// NotebookbarAccessibility initializes on a timer after the notebookbar
		// is shown, and ignores Alt until then.
		cy.wrap(null).should(function () {
			expect(win.app.UI.notebookbarAccessibility.initialized,
				'notebookbar accessibility initialized').to.be.true;
		});
	});

	// The yellow accelerator boxes are appended to <body>, next to the hoisted
	// .main-nav flex item. .main-nav forms a stacking context, so once it was
	// raised above the bottom bars the notebookbar painted over the boxes.
	it('accelerator boxes are not hidden behind the notebookbar', function () {
		cy.then(function () {
			// Alt down then up is what shows the boxes.
			win.app.UI.notebookbarAccessibility.onDocumentKeyDown({ keyCode: 18 });
			win.app.UI.notebookbarAccessibility.onDocumentKeyUp({ keyCode: 18 });
		});

		cy.cGet('body').should('have.class', 'activate-info-boxes');
		cy.cGet('.accessibility-info-box').should('be.visible');

		cy.cGet('.accessibility-info-box:visible').then(function ($boxes) {
			expect($boxes.length, 'visible accelerator boxes').to.be.greaterThan(0);

			var probed = 0;
			for (var i = 0; i < $boxes.length; i++) {
				var r = $boxes[i].getBoundingClientRect();
				if (r.width === 0 || r.height === 0)
					continue;

				var top = win.document.elementFromPoint(
					r.left + r.width / 2, r.top + r.height / 2);
				expect(top, 'element at the accelerator box centre').to.not.be.null;
				expect(
					top.classList.contains('accessibility-info-box'),
					'topmost element over box "' + $boxes[i].textContent + '" is the box itself'
				).to.be.true;
				probed++;
			}
			expect(probed, 'probed accelerator boxes').to.be.greaterThan(0);
		});
	});
});
