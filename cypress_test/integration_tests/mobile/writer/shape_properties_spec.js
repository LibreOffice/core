/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Change shape properties via mobile wizard.', function() {
	const defaultStartPoint = [2198, 5559];
	const defaultBase = 5992;
	const defaultAltitude = 5992;
	const unitScale = 2540.37;

	class TriangleCoordinatesMatcher {
		/**
		 * @param {number} start
		 * @param {number} base
		 * @param {number} altitude
		 * @param {boolean} horizontalMirrored
		 * @param {boolean} verticalMirrored
		 */
		constructor(start, base, altitude, horizontalMirrored, verticalMirrored, delta) {
			// FIXME: This is probably a bug in core side. On flipping horizontally the base length changes.
			base = horizontalMirrored ? base + 54 : base;

			this.xStart = start[0] + (horizontalMirrored ? base : 0);
			this.xEnd = start[0] + (horizontalMirrored ? 0 : base);
			this.yStart = start[1] + (verticalMirrored ? altitude : 0);
			this.yEnd = start[1] + (verticalMirrored ? 0 : altitude);
			this.delta = delta || 30;
		}

		/**
		 * Checks the correctness of triangle svg path based on coordinates.
		 * @param {string} pathCommandStr is the value of the attribute 'd' of the triangle shape's svg path.
		 */
		match(pathCommandStr) {
			// M 1953,10839 L 7945,4847 1953,4847 1953,10839 1953,10839 Z
			// Filter empty strings to handle SVG path data with or without trailing whitespace
			const pathCmdSplit = pathCommandStr.split(' ').filter(s => s.length > 0);
			expect(pathCmdSplit).to.have.length(8);
			TriangleCoordinatesMatcher.pointMatch(pathCmdSplit[1], this.xStart, this.yStart, this.delta, 'top of hypotenuse');
			TriangleCoordinatesMatcher.pointMatch(pathCmdSplit[3], this.xEnd, this.yEnd, this.delta, 'bottom of hypotenuse');
			TriangleCoordinatesMatcher.pointMatch(pathCmdSplit[4], this.xStart, this.yEnd, this.delta, 'left end of base');
		}

		/**
		 * Does approximate matching of a point with the given expected values and error margin.
		 * @param {string} pointStr
		 * @param {number} expectedX
		 * @param {number} expectedY
		 * @param {number} delta
		 * @param {string} contextString
		 */
		static pointMatch(pointStr, expectedX, expectedY, delta, contextString) {
			const pointParts = pointStr.split(',');
			expect(pointParts).to.have.length(2);
			const x = parseInt(pointParts[0]);
			const y = parseInt(pointParts[1]);
			expect(x).to.be.closeTo(expectedX, delta, contextString + ' x ');
			expect(y).to.be.closeTo(expectedY, delta, contextString + ' y ');
		}
	}

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/shape_properties.odt');
		mobileHelper.enableEditingMobile();
		helper.moveCursor('end');
		helper.moveCursor('home');
		mobileHelper.openInsertionWizard();
		// Do insertion
		cy.cGet('body').contains('.menu-entry-with-icon', 'Shape').click();
		cy.cGet('.basicshapes_right-triangle').click();
		// Check that the shape is there
		cy.cGet('#document-container svg g').should('have.class', 'com.sun.star.drawing.CustomShape');
	});

	function triggerNewSVG() {
		mobileHelper.closeMobileWizard();
		// Change width
		openPosSizePanel();
		cy.cGet('#selectwidth .plus').should('be.visible');
		cy.cGet('#selectwidth .plus').click();
		cy.cGet('#selectwidth .plus').click();
		mobileHelper.closeMobileWizard();
	}

	function openPosSizePanel() {
		mobileHelper.openMobileWizard();
		cy.cGet('#PosSizePropertyPanel').click();
		cy.cGet('#selectwidth').should('be.visible');
	}

	function openLinePropertyPanel() {
		mobileHelper.openMobileWizard();
		cy.cGet('#LinePropertyPanel').click();
		cy.cGet('#linestyle').should('be.visible');
	}

	function openAreaPanel() {
		mobileHelper.openMobileWizard();
		cy.cGet('#AreaPropertyPanel').click();
		cy.cGet('#fillstylearea').should('be.visible');
	}

	it('Check default shape geometry.', function() {
		// Geometry
		const matcher = new TriangleCoordinatesMatcher(defaultStartPoint, defaultBase, defaultAltitude);
		cy.cGet('#canvas-container svg svg g g g path').invoke('attr', 'd').should(matcher.match.bind(matcher));
		// Fill color
		cy.cGet('#canvas-container svg svg g g g path').should('have.attr', 'fill', 'rgb(114,159,207)');
	});

	it('Change shape width.', function() {
		openPosSizePanel();
		helper.typeIntoInputField('#mobile-wizard #selectwidth .spinfield', '4.2', true, false);
		const matcher = new TriangleCoordinatesMatcher(defaultStartPoint, Math.floor(4.2 * unitScale) /* new base */, defaultAltitude);
		cy.cGet('#canvas-container svg svg g g g path').invoke('attr', 'd').should(matcher.match.bind(matcher));
	});

	it('Change shape height.', function() {
		openPosSizePanel();
		helper.typeIntoInputField('#mobile-wizard #selectheight .spinfield', '5.2', true, false);
		const matcher = new TriangleCoordinatesMatcher(defaultStartPoint, defaultBase, Math.ceil(5.2 * unitScale) /* new altitude */);
		cy.cGet('#canvas-container svg svg g g g path').invoke('attr', 'd').should(matcher.match.bind(matcher));
	});

	it('Change size with keep ratio enabled.', function() {
		openPosSizePanel();
		// Enable keep ratio
		cy.cGet('#mobile-wizard #ratio #ratio').click();
		cy.cGet('#mobile-wizard #ratio #ratio').should('have.prop', 'checked', true);
		// Change height
		helper.typeIntoInputField('#selectheight .spinfield', '5.2');
		const matcher = new TriangleCoordinatesMatcher(defaultStartPoint, Math.floor(5.2 * unitScale), Math.ceil(5.2 * unitScale));
		cy.cGet('#canvas-container svg svg g g g path').invoke('attr', 'd').should(matcher.match.bind(matcher));
	});

	it('Vertical mirroring', function() {
		openPosSizePanel();
		cy.cGet('#mobile-wizard .unoFlipVertical').click();
		const matcher = new TriangleCoordinatesMatcher(defaultStartPoint, defaultBase, defaultAltitude, false /* horiz mirroring */, true /* vert mirroring */);
		cy.cGet('#canvas-container svg svg g g g path').invoke('attr', 'd').should(matcher.match.bind(matcher));
	});

	it('Horizontal mirroring', function() {
		openPosSizePanel();
		cy.cGet('#mobile-wizard .unoFlipHorizontal').click();
		triggerNewSVG();
		const matcher = new TriangleCoordinatesMatcher(defaultStartPoint, defaultBase, defaultAltitude, true /* horiz mirroring */, false /* vert mirroring */);
		cy.cGet('#canvas-container svg svg g g g path').invoke('attr', 'd').should(matcher.match.bind(matcher));
	});

	it('Trigger moving backward / forward', function() {
		openPosSizePanel();
		// We can't test the result, so we just trigger
		// the events to catch crashes, consoler errors.
		cy.cGet('#mobile-wizard .unoBringToFront').click();
		cy.wait(300);
		cy.cGet('#mobile-wizard .unoObjectForwardOne').click();
		cy.wait(300);
		cy.cGet('#mobile-wizard .unoObjectBackOne').click();
		cy.wait(300);
		cy.cGet('#mobile-wizard .unoSendToBack').click();
		cy.wait(300);
	});

	it.skip('Change line color', function() {
		openLinePropertyPanel();
		cy.cGet('#mobile-wizard .unoXLineColor').click();
		cy.cGet('.ui-content[title="Line Color"] .color-sample-small[style="background-color: rgb(152, 0, 0);"]').click();
		triggerNewSVG();
		cy.cGet('#canvas-container svg svg g g g path[fill="none"]').should('have.attr', 'stroke', 'rgb(152,0,0)');
	});

	it.skip('Change line style', function() {
		openLinePropertyPanel();
		mobileHelper.selectListBoxItem2('#linestyle', 'Ultrafine Dashed');
		triggerNewSVG();
		cy.cGet('#canvas-container svg svg g g g path[fill="none"]').should('have.length.greaterThan', 12);
	});

	it.skip('Change line width', function() {
		openLinePropertyPanel();
		cy.cGet('#linewidth .spinfield').should('have.attr', 'readonly', 'readonly');
		cy.cGet('#linewidth .plus').click();
		triggerNewSVG();
		cy.cGet('#canvas-container svg svg g g g path[fill="none"]').should('have.attr', 'stroke-width', '141');
		openLinePropertyPanel();
		cy.cGet('#linewidth .minus').click();
		triggerNewSVG();
		cy.cGet('#canvas-container svg svg g g g path[fill="none"]').should('have.attr', 'stroke-width', '88');
	});

	it.skip('Change line transparency', function() {
		openLinePropertyPanel();
		helper.typeIntoInputField('#linetransparency .spinfield', '20', true, false);
		triggerNewSVG();
		cy.cGet('#document-container svg g svg g g g defs mask linearGradient').should('exist');
	});

	it.skip('Arrow style items are hidden.', function() {
		openLinePropertyPanel();
		cy.cGet('#mobile-wizard #linestyle').should('be.visible');
		cy.cGet('#mobile-wizard #beginarrowstyle').should('not.exist');
		cy.cGet('#mobile-wizard #endarrowstyle').should('not.exist');

	});

	it.skip('Apply gradient fill', function() {
		cy.cGet('#document-container svg g svg g.Page g g#id1 defs pattern').should('not.exist');
		openAreaPanel();
		cy.cGet('#mobile-wizard #fillstylearea .ui-header-left').should('have.text', 'Color');
		mobileHelper.selectListBoxItem2('#fillstylearea', 'Gradient');
		// Select type
		cy.cGet('#mobile-wizard #gradientstyle .ui-header-left').should('have.text', 'Linear');
		mobileHelper.selectListBoxItem2('#gradientstyle', 'Square');
		// Select From color
		cy.cGet('#mobile-wizard #fillgrad1').click();
		mobileHelper.selectFromColorPalette(0, 2);
		// Set gradient angle
		helper.typeIntoInputField('#mobile-wizard #gradangle .spinfield', '100');
		cy.cGet('#mobile-wizard #gradangle .spinfield').should('have.value', '100');
		// Select To color
		cy.cGet('#mobile-wizard #fillgrad2').click();
		mobileHelper.selectFromColorPalette(1, 7);
		triggerNewSVG();
		cy.cGet('#document-container svg g svg g.Page g g#id1 defs pattern').should('exist');
	});

	it.skip('Apply hatching fill', function() {
		cy.cGet('#document-container svg g svg g.Page g g#id1 defs pattern').should('not.exist');
		openAreaPanel();
		cy.cGet('#mobile-wizard #fillstylearea .ui-header-left').should('have.text', 'Color');
		mobileHelper.selectListBoxItem2('#fillstylearea', 'Hatching');
		cy.cGet('#mobile-wizard #fillattrhb .ui-header-left').should('have.text', 'Black 0 Degrees');
		mobileHelper.selectListBoxItem2('#fillattrhb', 'Black 45 Degrees');
		triggerNewSVG();
		cy.cGet('#document-container svg g svg g.Page g g#id1 defs pattern').should('exist');
	});

	it.skip('Apply bitmap fill', function() {
		cy.cGet('#document-container svg g svg g.Page g g#id1 defs clipPath').should('not.exist');
		openAreaPanel();
		cy.cGet('#mobile-wizard #fillstylearea .ui-header-left').should('have.text', 'Color');
		mobileHelper.selectListBoxItem2('#fillstylearea', 'Bitmap');
		cy.cGet('#mobile-wizard #fillattrhb .ui-header-left').should('have.text', 'Painted White');
		mobileHelper.selectListBoxItem2('#fillattrhb', 'Paper Graph');
		triggerNewSVG();
		cy.cGet('#document-container svg g svg g.Page g g#id1 defs clipPath').should('exist');
	});

	it.skip('Apply pattern fill', function() {
		cy.cGet('#document-container svg g svg g.Page g g#id1 defs clipPath').should('not.exist');
		openAreaPanel();
		cy.cGet('#mobile-wizard #fillstylearea .ui-header-left').should('have.text', 'Color');
		mobileHelper.selectListBoxItem2('#fillstylearea', 'Pattern');
		cy.cGet('#mobile-wizard #fillattrhb .ui-header-left').should('have.text', '5 Percent');
		mobileHelper.selectListBoxItem2('#fillattrhb', '20 Percent');
		triggerNewSVG();
		cy.cGet('#document-container svg g svg g.Page g g#id1 defs clipPath').should('exist');
	});

	it.skip('Change fill color', function() {
		cy.cGet('#document-container svg g svg g.Page g g#id1 path:nth-of-type(1)').should('have.attr', 'fill', 'rgb(114,159,207)');
		openAreaPanel();
		cy.cGet('#mobile-wizard #FillColor .color-sample-selected').should('have.attr', 'style', 'background-color: rgb(114, 159, 207);');
		cy.cGet('.unoFillColor').click();
		mobileHelper.selectFromColorPalette(0, 2, 0, 2);
		cy.cGet('#mobile-wizard #FillColor .color-sample-selected').should('have.attr', 'style', 'background-color: rgb(204, 0, 0);');
		triggerNewSVG();
		cy.cGet('#document-container svg g svg g.Page g g#id1 path:nth-of-type(1)').should('have.attr', 'fill', 'rgb(204,0,0)');
	});

	it.skip('Change fill transparency type', function() {
		cy.cGet('#document-container svg g svg g.Page g g#id1 linearGradient').should('not.exist');
		openAreaPanel();
		cy.cGet('#mobile-wizard #transtype .ui-header-left').should('have.text', 'None');
		mobileHelper.selectListBoxItem2('#transtype', 'Linear');
		// TODO: implement show/hide
		//cy.get('#settransparency .spinfield')
		//	.should('not.exist');
		triggerNewSVG();
		cy.cGet('#document-container svg g svg g.Page g g#id1 linearGradient').should('exist');
	});

	it.skip('Change fill transparency', function() {
		cy.cGet('#document-container svg g svg g.Page g g#id1 path:nth-of-type(1)').should('not.have.attr', 'fill-opacity');
		openAreaPanel();
		cy.cGet('#mobile-wizard #transtype .ui-header-left').should('have.text', 'None');
		helper.typeIntoInputField('#settransparency .spinfield', '50');
		cy.cGet('#mobile-wizard #settransparency .spinfield').should('have.value', '50');
		cy.cGet('#mobile-wizard #transtype .ui-header-left').should('have.text', 'Solid');
		triggerNewSVG();
		cy.cGet('#document-container svg g svg g.Page g g#id1 path:nth-of-type(1)').should('have.attr', 'fill-opacity', '0.502');
	});
});
