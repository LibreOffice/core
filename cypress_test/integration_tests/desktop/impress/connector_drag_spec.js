/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Connector endpoint drag preview.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/connector_drag.fodp');
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		});
	});

	function getHandlesSection(win) {
		return win.app.sectionContainer.getSectionWithName(
			win.app.CSections.ShapeHandlesSection.name);
	}

	// Regression test for: https://github.com/CollaboraOnline/online/issues/11953
	// Bug: Dragging a connector endpoint gave no visual feedback until the
	// mouse button was released. While the endpoint is dragged, core reports
	// the line the connector would get on drop and that polyline is drawn
	// as a preview following the mouse.
	it('Dragging an endpoint shows the routed preview.', function() {
		// The connector crosses the center of the slide.
		impressHelper.clickCenterOfSlide({});

		// Let the selection settle so the handle geometry is final and the
		// canvas sub-section that receives the drag is positioned before the
		// mouse press reads the handle's screen position.
		helper.processToIdle(this.win);

		// The two endpoints of the connector are poly handles.
		cy.cGet('#test-div-shape-handle-0').should('exist');
		cy.cGet('#test-div-shape-handle-1').should('exist');

		// Connectors have no rectangle handles, so no rotation handle either.
		cy.cGet('#test-div-shape-handle-rotation').should('not.exist');

		cy.cGet('#test-div-shape-handle-1').then(($handle) => {
			var rect = $handle[0].getBoundingClientRect();
			var startX = rect.left + rect.width / 2;
			var startY = rect.top + rect.height / 2;
			var endX = startX + 60;
			var endY = startY - 40;

			cy.cGet('body').realMouseDown({ x: startX, y: startY });
			cy.cGet('body').realMouseMove(startX + 30, startY - 20);
			cy.cGet('body').realMouseMove(endX, endY);

			// While dragging, the preview polyline from core is shown.
			cy.waitUntil(() => {
				var section = getHandlesSection(this.win);
				var polygons = section && section.sectionProperties.shapeDragPreviewPolygons;
				return Boolean(polygons && polygons.length > 0 && polygons[0].length > 1);
			}, {
				errorMsg: 'The drag preview polyline did not arrive.'
			});

			cy.cGet('body').realMouseUp({ x: endX, y: endY });

			// After dropping, the preview is removed.
			cy.waitUntil(() => {
				var section = getHandlesSection(this.win);
				return Boolean(section && !section.sectionProperties.shapeDragPreviewPolygons);
			}, {
				errorMsg: 'The drag preview polyline was not removed after the drop.'
			});

			// The drop has moved the endpoint, so its handle follows.
			cy.cGet('#test-div-shape-handle-1').should(function($movedHandle) {
				var movedRect = $movedHandle[0].getBoundingClientRect();
				var movedX = movedRect.left + movedRect.width / 2;
				var movedY = movedRect.top + movedRect.height / 2;
				expect(Math.abs(movedX - endX)).to.be.lessThan(10);
				expect(Math.abs(movedY - endY)).to.be.lessThan(10);
			});
		});
	});
});
