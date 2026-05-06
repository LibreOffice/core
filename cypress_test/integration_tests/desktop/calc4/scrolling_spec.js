/* global describe it cy beforeEach expect require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Scroll through document', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/scrolling.ods');
		desktopHelper.switchUIToCompact();
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Scrolling to bottom/top', function() {
		desktopHelper.assertScrollbarPosition('vertical', 25, 40);
		desktopHelper.pressKey(3,'pagedown');
		desktopHelper.assertScrollbarPosition('vertical', 110, 130);
		desktopHelper.pressKey(3,'pageup');
		desktopHelper.assertScrollbarPosition('vertical', 50, 105);
		desktopHelper.pressKey(3,'downArrow');
		desktopHelper.assertScrollbarPosition('vertical', 25, 40);
	});

	it('Scrolling to left/right', function() {
		desktopHelper.selectZoomLevel('200');
		helper.typeIntoDocument('{home}');
		desktopHelper.assertScrollbarPosition('horizontal', 48, 60);
		helper.typeIntoDocument('{end}');
		helper.processToIdle(this.win);
		desktopHelper.assertScrollbarPosition('horizontal', 180, 300);
	});

	it('Scroll while selecting vertically', function() {
		desktopHelper.assertScrollbarPosition('vertical', 25, 40);
		desktopHelper.assertScrollbarPosition('horizontal', 48, 50);

		// Click on a cell near the edge of the view
		cy.cGet('#map')
		.then(function(items) {
			expect(items).to.have.lengthOf(1);
			var XPos = items[0].getBoundingClientRect().right - 280;
			var YPos = items[0].getBoundingClientRect().bottom - 60;
			cy.cGet('body').click(XPos, YPos);
		});

		// Select cells downwards with shift + arrow
		for (let i = 0; i < 10; ++i) {
			helper.typeIntoDocument('{shift}{downArrow}');
		}

		// Document should scroll
		desktopHelper.assertScrollbarPosition('vertical', 180, 300);
		// Document should not scroll horizontally
		desktopHelper.assertScrollbarPosition('horizontal', 48, 50);
	});

	it('Scroll while selecting horizontally', function() {
		desktopHelper.assertScrollbarPosition('horizontal', 48, 60);

		// Click on a cell near the edge of the view
		cy.cGet('#map')
		.then(function(items) {
			expect(items).to.have.lengthOf(1);
			var XPos = items[0].getBoundingClientRect().right - 280;
			var YPos = items[0].getBoundingClientRect().bottom - 60;
			cy.cGet('body').click(XPos, YPos);
		});

		// Select cells to the right with shift + arrow
		for (let i = 0; i < 10; ++i) {
			helper.typeIntoDocument('{shift}{rightArrow}');
		}

		// Document should scroll
		desktopHelper.assertScrollbarPosition('horizontal', 80, 155);
	});

	it('Scroll while selecting with mouse', function () {
		cy.cGet(helper.addressInputSelector).should('have.value', 'A2');

		// Click on the bottom left cell and hold
		cy.cGet('#document-container')
			.then(function (items) {
				expect(items).to.have.lengthOf(1);
				var yPos = items[0].getBoundingClientRect().height - 60;
				cy.cGet('#document-container').realMouseDown({ pointer: 'mouse', button: 'left', x: 30, y: yPos, scrollBehavior: false });
			});
		// Drag some cells to the right
		cy.cGet('#document-container').realMouseMove(-280, -60, { position: 'bottomRight', scrollBehavior: false });
		// Drag to the bottom edge
		cy.cGet('#document-container').realMouseMove(-280, 0, { position: 'bottomRight', scrollBehavior: false });
		helper.processToIdle(this.win);
		// Wait for autoscroll and lift the button
		helper.waitForTimers(this.win, 'autoscroll');
		cy.cGet('#document-container').realMouseUp({ pointer: 'mouse', button: 'left' });

		// Allow the change to propagate to core and it to update the addressInputSelector
		helper.processToIdle(this.win);
		// Without the fix, the selected range is of the form A17:A22, instead of A17:D22
		// It's better not to check the exact range because it can easily change in different executions
		cy.cGet(helper.addressInputSelector).invoke('val').should('contain', 'D');
	});

	it('Scroll while selecting with mouse - outside the canvas', function () {
		cy.cGet(helper.addressInputSelector).should('have.value', 'A2');

		// Click on the bottom left cell and hold
		cy.cGet('#document-container')
			.then(function (items) {
				expect(items).to.have.lengthOf(1);
				const right = items[0].getBoundingClientRect().right;
				const bottom = items[0].getBoundingClientRect().bottom;
				const horizontalCenter = Math.round(right * 0.5);

				helper.processToIdle(this.win);
				cy.cGet('body').realMouseDown(horizontalCenter, Math.round(bottom * 0.5));

				cy.cGet('body').realMouseMove(horizontalCenter, bottom - 50);

				// We should have initiated a selection by now. Move the mouse to where the horizontal scroll bar must be (this tests a fix).
				cy.cGet('body').realMouseMove(horizontalCenter, bottom - 5);

				// Wait for autoscroll to start at the edge before moving outside
				helper.processToIdle(this.win);

				// Move the mouse pointer outside the document. It should be widening the selection (mouse button is pressed and being held).
				for (let i = 0; i < 10; i++) {
					cy.cGet('body').realMouseMove(horizontalCenter, bottom + 20);
					helper.processToIdle(this.win);
					cy.cGet('body').realMouseMove(horizontalCenter, bottom + 30);
				}

				// Release the mouse button to stop autoscroll
				cy.cGet('body').realMouseUp();
				helper.processToIdle(this.win);
				helper.waitForTimers(this.win, 'autoscroll');

				// Click on the ~center of the window.
				cy.cGet('body').click(horizontalCenter, Math.round(right * 0.5));
				cy.cGet(helper.addressInputSelector).then((item) => {
					const addressInput = item[0];
					const rowNumber = parseInt(addressInput.value.substring(1, addressInput.value.length));
					cy.expect(rowNumber).to.be.greaterThan(22);
				});
			}
		);
	});
});
