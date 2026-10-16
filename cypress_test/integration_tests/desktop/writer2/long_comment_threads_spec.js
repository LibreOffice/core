/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

// Page 1 holds three long threads, "Test line 1" (comments 1 to 6), "Test line 2"
// (7 to 13) and "Test line 3" (14 to 18). They are far taller than the page.
describe(['tagdesktop'], 'Long Writer comment threads', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/long_comment_threads.odt');
		cy.viewport(1400, 900);
		desktopHelper.sidebarToggle();
		desktopHelper.selectZoomLevel('100', false);
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
		cy.cGet('#comment-container-18').should('exist');
		cy.then(() => {
			return helper.processToIdle(this.win);
		});
	});

	function commentSection(win) {
		return win.app.sectionContainer.getSectionWithName(win.app.CSections.CommentList.name);
	}

	it('Removing a reply does not select its thread', function() {
		// Core puts the text cursor where the removed reply was, which is inside
		// the text that "Test line 1" comments on. The call is the one that the
		// Remove entry of the comment menu makes.
		cy.then(() => {
			commentSection(this.win).remove('6');
		});
		cy.cGet('#comment-container-6').should('not.exist');
		cy.then(() => {
			return helper.processToIdle(this.win);
		});
		cy.then(() => {
			var selected = commentSection(this.win).sectionProperties.selectedComment;
			expect(selected ? selected.sectionProperties.data.id : null, 'selected thread').to.equal(null);
		});
	});
});
