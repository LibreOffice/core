/* global describe it cy beforeEach expect require */

var helper = require("../../common/helper");

describe(["tagdesktop"], "Header/footer boundary indicator.", function () {
	beforeEach(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument("writer/header_footer_boundary.odt");
	});

	it("Shows the header boundary in edit mode and hides it on leaving.", function () {
		// (200, 50) lands in the header band of page 1, entering header edit mode.
		cy.cGet("#document-container").click(200, 50);
		cy.getFrameWindow().should(function (win) {
			var section = win.app.sectionContainer.getSectionWithName(win.app.CSections.HeaderFooterBoundary.name);
			expect(section, "boundary section was created").to.exist;
			expect(section.sectionProperties.headerRectangle, "header boundary present").to.not.be.null;
		});

		// Click into the body to leave header edit mode; engine sends {hide:true}.
		cy.cGet("#document-container").click(200, 400);
		cy.getFrameWindow().should(function (win) {
			var section = win.app.sectionContainer.getSectionWithName(win.app.CSections.HeaderFooterBoundary.name);
			expect(section.sectionProperties.headerRectangle, "header boundary cleared").to.be.null;
			expect(section.size[0], "overlay width zeroed").to.eq(0);
			expect(section.size[1], "overlay height zeroed").to.eq(0);
		});
	});
});
