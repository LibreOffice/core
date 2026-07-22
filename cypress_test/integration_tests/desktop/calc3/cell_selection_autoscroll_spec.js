/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Calc drag-selection autoscroll', function () {

	beforeEach(function () {
		helper.setupAndLoadDocument('calc/autofill.fods');
	});

	// While a cell selection is dragged with the mouse button held, the document
	// scrolls only when the pointer is near a viewport edge, and it starts while
	// the pointer is still just inside that edge, not only after it leaves the
	// document. A pointer resting away from every edge does not scroll. The scroll
	// decision follows the live pointer, not the selection that comes back from the
	// server. app.map.isAutoScrolling tells whether that edge autoscroll is running.
	it('autoscrolls only when the dragged pointer is near an edge', function () {
		cy.getFrameWindow().then(function (win) {
			helper.processToIdle(win);

			var mouseControl = win.app.sectionContainer.getSectionWithName('mouse-control');
			expect(mouseControl, 'mouse-control section').to.exist;

			// Call the handlers directly. Cypress synthetic DOM events are rejected
			// by the canvas container, so report a drag as in progress for the test.
			cy.stub(win.app.sectionContainer, 'isDraggingSomething').returns(true);

			var dpi = win.app.dpiScale;
			var size = win.app.map._size;
			var origin = mouseControl.position;

			// A section-local point that lands at the given css position in the
			// viewport once the drag branch adds the section origin back.
			var atViewport = function (cssX, cssY) {
				return win.cool.SimplePoint.fromCorePixels([
					cssX * dpi - origin[0],
					cssY * dpi - origin[1],
				]);
			};

			var mouseEvent = function (type) {
				return new win.MouseEvent(type, { buttons: 1 });
			};

			mouseControl.onMouseDown(
				atViewport(size.x / 2, size.y / 2),
				mouseEvent('mousedown')
			);

			// Pointer in the middle: no autoscroll.
			mouseControl.onMouseMove(
				atViewport(size.x / 2, size.y / 2),
				[0, 0],
				mouseEvent('mousemove')
			);
			expect(win.app.map.isAutoScrolling, 'pointer in the middle')
				.to.not.equal(true);

			// Pointer just inside the bottom edge: autoscroll starts.
			mouseControl.onMouseMove(
				atViewport(size.x / 2, size.y - 10),
				[0, 1000],
				mouseEvent('mousemove')
			);
			expect(win.app.map.isAutoScrolling, 'pointer near the bottom edge')
				.to.equal(true);

			// Releasing the button stops the autoscroll.
			mouseControl.onMouseUp(
				atViewport(size.x / 2, size.y - 10),
				mouseEvent('mouseup')
			);
			expect(win.app.map.isAutoScrolling, 'after releasing the button')
				.to.not.equal(true);
		});
	});

	// Dragging the autofill marker near a viewport edge scrolls the document the
	// same way a cell-selection drag does: the marker forwards the drag to the
	// mouse-control section. A touch drag must fill, not pan the view, so the
	// marker hands the event to mouse-control as a mouse event; if that stopped
	// working the drag would pan and autoscroll would never start.
	it('autofill marker drag near an edge autoscrolls, including touch', function () {
		cy.getFrameWindow().then(function (win) {
			helper.processToIdle(win);

			var marker = win.app.sectionContainer.getSectionWithName('auto fill marker');
			var mouseControl = win.app.sectionContainer.getSectionWithName('mouse-control');
			expect(marker, 'auto fill marker section').to.exist;

			// The marker only forwards a drag while its fill area is shown.
			win.app.map._docLayer._cellAutoFillAreaPixels =
				win.app.LOUtil.createRectangle(0, 0, 10, 10);
			cy.stub(win.app.sectionContainer, 'isDraggingSomething').returns(true);

			var dpi = win.app.dpiScale;
			var size = win.app.map._size;
			var docAnchor = win.app.sectionContainer.getDocumentAnchor();

			// A marker-local point that lands at the given css position in the
			// viewport after the marker maps it into mouse-control's frame and
			// mouse-control adds its own origin back.
			var atViewport = function (cssX, cssY) {
				return win.cool.SimplePoint.fromCorePixels([
					cssX * dpi - marker.myTopLeft[0] + docAnchor[0] - mouseControl.position[0],
					cssY * dpi - marker.myTopLeft[1] + docAnchor[1] - mouseControl.position[1],
				]);
			};

			var touchEvent = function (type) {
				return new win.MouseEvent(type, { buttons: 1 });
			};

			marker.onMouseDown(
				atViewport(size.x / 2, size.y / 2),
				touchEvent('touchstart')
			);

			// Pointer in the middle: no autoscroll.
			marker.onMouseMove(
				atViewport(size.x / 2, size.y / 2),
				[0, 0],
				touchEvent('touchmove')
			);
			expect(win.app.map.isAutoScrolling, 'pointer in the middle')
				.to.not.equal(true);

			// Pointer just inside the bottom edge: autoscroll starts.
			marker.onMouseMove(
				atViewport(size.x / 2, size.y - 10),
				[0, 1000],
				touchEvent('touchmove')
			);
			expect(win.app.map.isAutoScrolling, 'pointer near the bottom edge')
				.to.equal(true);

			// Releasing the button stops the autoscroll.
			marker.onMouseUp(
				atViewport(size.x / 2, size.y - 10),
				touchEvent('touchend')
			);
			expect(win.app.map.isAutoScrolling, 'after releasing the button')
				.to.not.equal(true);
		});
	});
});
