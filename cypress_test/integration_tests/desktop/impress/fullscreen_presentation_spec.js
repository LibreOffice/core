/* global describe it cy require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe.skip(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Fullscreen Presentation.', function() {

	function getSlideShowContent() {
		return cy.cGet().find('.leaflet-slideshow').then(($iframe) =>{
				cy.wrap($iframe.contents());
			});
	}

	function before(filePath) {
		helper.setupAndLoadDocument(filePath, false, false, undefined, 'vector=false');

		desktopHelper.hideSidebar();

		cy.cGet('#menu-slide > a').click();
		cy.cGet('#menu-fullscreen-presentation > a').click();
	}

	it('Text fields.', function() {
		before('impress/text_fields.odp');

		cy.wait(3000);

		getSlideShowContent().find('#id1').should('have.class', 'Slide');

		getSlideShowContent().find('#tf2 > g > text > tspan > tspan > tspan')
			.should('have.class', 'PlaceholderText')
			.should('contain', '1');

		getSlideShowContent().find('#tf6 > g > text > tspan > tspan > tspan')
			.should('have.class', 'PlaceholderText')
			.should('contain', '1');

		getSlideShowContent().find('#tf5 > g > text > tspan > tspan > tspan')
			.should('have.class', 'PlaceholderText')
			.should('contain', 'SlideOne');

		// go to second slide
		getSlideShowContent().find('#id1').click();

		getSlideShowContent().find('#id2').should('have.class', 'Slide');

		getSlideShowContent().find('#tf7 > g > text > tspan > tspan > tspan')
			.should('have.class', 'PlaceholderText')
			.should('contain', '2');

		getSlideShowContent().find('#tf9 > g > text > tspan > tspan > tspan')
			.should('have.class', 'PlaceholderText')
			.should('contain', '2');

		getSlideShowContent().find('#tf8 > g > text > tspan > tspan > tspan')
			.should('have.class', 'PlaceholderText')
			.should('contain', 'SlideHello');
	});

	it('Custom background.', function() {
		before('impress/slide-bitmap-background.odp');

		cy.wait(3000);

		getSlideShowContent().find('#id1')
			.should('have.class', 'Slide');
		getSlideShowContent().find('#id1 > g').its('0')
			.should('have.class', 'MasterPageView');
		getSlideShowContent().find('#id1 > g > use')
			.should('have.class', 'Background')
			.should('have.attr', 'href', '#bg-id1');

		getSlideShowContent().find('#id1 > .Page > .SlideBackground > .Background')
			.should('have.attr', 'id', 'bg-id1');
	});

	it.skip('Leading spaces shorter than a text line.', function() {
		before('impress/white-spaces.odp');

		cy.wait(3000);

		getSlideShowContent().find('#id8 > .SVGTextShape > .TextParagraph > .TextPosition')
			.as('textLines')
			.should('contain', '2');

		getSlideShowContent().get('@textLines').eq(0).find('tspan').first()
			.should('have.text', '    ')
			.then((spaces) => {
				const spacesLength = spaces.prop('textLength').baseVal.valueAsString;
				getSlideShowContent().get('@textLines').eq(1).find('tspan').first()
					.should('have.text', '1234')
					.then((template) => {
						const templateLength = template.prop('textLength').baseVal.valueAsString;
						expect(spacesLength).to.eq(templateLength);
					});
			});
	});

	it('Leading spaces as long as a text line.', function() {
		before('impress/white-spaces.odp');

		cy.wait(3000);

		getSlideShowContent().find('#id10 > .SVGTextShape > .TextParagraph > .TextPosition')
			.as('textLines')
			.should('contain', '2');

		getSlideShowContent().get('@textLines').eq(0).find('tspan').first()
			.then((spaces) => {
				expect(spaces).to.have.prop('textContent').match(/ +/);
				const spacesLength = spaces.prop('textLength').baseVal.value;
				getSlideShowContent().get('@textLines').eq(1).find('tspan').first()
					.should('have.text', '1234567890')
					.then((template) => {
						const templateLength = template.prop('textLength').baseVal.value;
						expect(spacesLength).to.be.gte(templateLength);
						const textX = parseFloat(this.textLines[1].getAttribute('x'));
						expect(template.get(0).getBBox().x).to.be.closeTo(textX, 0.001);
					});
			});
	});

	it('Leading spaces longer than a text line.', function() {
		before('impress/white-spaces.odp');

		cy.wait(3000);

		getSlideShowContent().find('#id12 > .SVGTextShape > .TextParagraph > .TextPosition')
			.as('textLines')
			.should('contain', '3');

		getSlideShowContent().get('@textLines').eq(1).find('tspan').first()
			.should('have.text', '    ')
			.then((spaces) => {
				const spacesLength = spaces.prop('textLength').baseVal.valueAsString;
				getSlideShowContent().get('@textLines').eq(2).find('tspan').first()
					.should('have.text', '1234')
					.then((template) => {
						const templateLength = template.prop('textLength').baseVal.valueAsString;
						expect(spacesLength).to.eq(templateLength);
					});
			});
	});

	it('Internal spaces up to the end of the line.', function() {
		before('impress/white-spaces.odp');

		cy.wait(3000);

		getSlideShowContent().find('#id14 > .SVGTextShape > .TextParagraph > .TextPosition')
			.as('textLines')
			.should('contain', '2');

		getSlideShowContent().get('@textLines').eq(0).find('tspan').first()
			.then((spaces) => {
				expect(spaces).to.have.prop('textContent').match(/1 +/);
				getSlideShowContent().get('@textLines').eq(1).find('tspan').first()
					.should('have.text', '1234567890')
					.then((template) => {
						const textX = parseFloat(this.textLines[1].getAttribute('x'));
						expect(template.get(0).getBBox().x).to.be.closeTo(textX, 0.001);
					});
			});
	});

	it('Internal spaces crossing two lines.', function() {
		before('impress/white-spaces.odp');

		cy.wait(3000);

		getSlideShowContent().find('#id16 > .SVGTextShape > .TextParagraph > .TextPosition')
			.as('textLines')
			.should('contain', '3');

		getSlideShowContent().get('@textLines').eq(1).find('tspan').first()
			.should('have.text', '    567890')
			.then((spaces) => {
				const spacesLength = spaces.prop('textLength').baseVal.valueAsString;
				getSlideShowContent().get('@textLines').eq(2).find('tspan').first()
					.should('have.text', '1234567890')
					.then((template) => {
						const templateLength = template.prop('textLength').baseVal.valueAsString;
						expect(spacesLength).to.eq(templateLength);
					});
			});
	});

	it('Animation: Emphasis: Spin.', function() {
		before('impress/anim-spin.odp');

		cy.wait(3000);

		getSlideShowContent().find('#id1')
			.should('have.class', 'Slide');
		getSlideShowContent().find('#id1 > g').its('1')
			.should('have.class', 'Page');
		getSlideShowContent().find('#id3')
			.then((shape) => {
				const matrix = shape.prop('transform').baseVal.getItem(0).matrix;
				expect(matrix.a).to.eq(1);
				expect(matrix.b).to.eq(0);
				expect(matrix.c).to.eq(0);
				expect(matrix.d).to.eq(1);
				expect(matrix.e).to.eq(0);
				expect(matrix.f).to.eq(0);
			})
			.click();

		cy.wait(3000);
		getSlideShowContent().find('#id3')
			.then((shape) => {
				const matrix = shape.prop('transform').baseVal.getItem(0).matrix;
				expect(Math.round(matrix.a)).to.eq(0);
				expect(matrix.b).to.eq(1);
				expect(matrix.c).to.eq(-1);
				expect(Math.round(matrix.d)).to.eq(0);
				expect(matrix.e).to.eq(22501);
				expect(matrix.f).to.eq(-5500);
			});
	});

	it.skip('Animation: Emphasis: Grow and Shrink.', function() {
		before('impress/anim-grow-and-shrink.odp');

		cy.wait(3000);

		getSlideShowContent().find('#id1')
			.should('have.class', 'Slide');
		getSlideShowContent().find('#id1 > g').its('1')
			.should('have.class', 'Page');
		getSlideShowContent().find('#id3')
			.then((shape) => {
				const matrix = shape.prop('transform').baseVal.getItem(0).matrix;
				expect(matrix.a).to.eq(1);
				expect(matrix.b).to.eq(0);
				expect(matrix.c).to.eq(0);
				expect(matrix.d).to.eq(1);
				expect(matrix.e).to.eq(0);
				expect(matrix.f).to.eq(0);
			})
			.click();

		cy.wait(3000);
		getSlideShowContent().find('#id3')
			.then((shape) => {
				const matrix = shape.prop('transform').baseVal.getItem(0).matrix;
				expect(matrix.a).to.eq(2.5);
				expect(matrix.b).to.eq(0);
				expect(matrix.c).to.eq(0);
				expect(matrix.d).to.eq(2.5);
				expect(matrix.e).to.eq(-21000.75);
				expect(matrix.f).to.eq(-12750.75);
			});
	});

});
