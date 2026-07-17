/* global describe it cy require expect beforeEach */

var desktopHelper = require('../../common/desktop_helper');
var helper = require('../../common/helper');

describe(['tagmultiuser'], 'Multiuser Annotation Tests', function() {

	beforeEach(function() {

		helper.setupAndLoadDocument('impress/comment_switching.odp',true);
		cy.viewport(2600, 800);
		desktopHelper.switchUIToNotebookbar();

		cy.cSetActiveFrame('#iframe1');
		desktopHelper.closeNavigatorSidebar();
		desktopHelper.hideSidebarImpress();
		desktopHelper.selectZoomLevel('50', false);

		cy.cSetActiveFrame('#iframe2');
		desktopHelper.closeNavigatorSidebar();
		desktopHelper.hideSidebarImpress();
		desktopHelper.selectZoomLevel('50', false);
	});

	it('Insert', function() {
		cy.cSetActiveFrame('#iframe1');
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('exist');
		cy.cGet('.cool-annotation-content > div').should('contain','some text');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.annotation-marker').should('exist');
		cy.cGet('.cool-annotation-content > div').should('contain','some text');
	});

	it('Modify', function() {
		cy.cSetActiveFrame('#iframe1');
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('exist');
		cy.cGet('[id^=annotation-content-area-]').should('contain','some text0');
		cy.cGet('.cool-annotation-content-wrapper:visible .cool-annotation-menu').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Modify').click();
		cy.cGet('[id^=annotation-modify-textarea-]').type('{home}');
		cy.cGet('[id^=annotation-modify-textarea-]').type('some other text, ');
		cy.cGet('[id^=annotation-save-]').click();
		cy.cGet('[id^=annotation-content-area-]').should('contain','some other text, some text0');
		cy.cGet('.annotation-marker').should('exist');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('[id^=annotation-content-area-]').should('contain','some other text, some text0');
		cy.cGet('.annotation-marker').should('exist');
	});

	it('Remove',function() {
		cy.cSetActiveFrame('#iframe1');
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('exist');
		cy.cGet('.cool-annotation-content > div').should('contain','some text');
		cy.cGet('.cool-annotation-content-wrapper:visible .cool-annotation-menu').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Remove').click();
		cy.cGet('.annotation-marker').should('not.exist');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.annotation-marker').should('not.exist');
	});

	// Skipping reply tests in Impress since reply functionality is temporarily disabled.
	it.skip('Reply',function() {
		cy.cSetActiveFrame('#iframe1');
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('exist');
		cy.cGet('.cool-annotation-content > div').should('contain','some text');
		cy.cGet('.cool-annotation-content-wrapper:visible .cool-annotation-menu').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Reply').click();
		cy.cGet('[id^=annotation-reply-textarea-]').type('some reply text');
		cy.cGet('[id^=annotation-reply-].button-primary').click();
		cy.cGet('.cool-annotation-content > div').should('include.text','some reply text');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.cool-annotation-content > div').should('include.text','some reply text');
	});
});

describe(['tagmultiuser'], 'Multiuser Collapsed Annotation Tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/comment_switching.odp',true);
		cy.viewport(2400, 800);
		desktopHelper.switchUIToNotebookbar();

		cy.cSetActiveFrame('#iframe1');
		desktopHelper.closeNavigatorSidebar();
		desktopHelper.hideSidebarImpress();
		desktopHelper.selectZoomLevel('50', false);

		cy.cSetActiveFrame('#iframe2');
		desktopHelper.closeNavigatorSidebar();
		desktopHelper.hideSidebarImpress();
		desktopHelper.selectZoomLevel('50', false);
	});

	it('Insert', function() {
		cy.cSetActiveFrame('#iframe1');
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('exist');
		cy.cGet('.cool-annotation-content > div').should('contain','some text');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.annotation-marker').should('exist');
		cy.cGet('.cool-annotation-content > div').should('contain','some text');
	});

	it('Modify', function() {
		cy.cSetActiveFrame('#iframe1');
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('exist');
		cy.cGet('[id^=annotation-content-area-]').should('contain','some text0');
		cy.cGet('.cool-annotation-table .avatar-img').click();
		cy.cGet('.cool-annotation-menu').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Modify').click();
		cy.cGet('[id^=annotation-modify-textarea-]').type('{home}');
		cy.cGet('[id^=annotation-modify-textarea-]').type('some other text, ');
		cy.cGet('[id^=annotation-save-]').click();
		cy.cGet('[id^=annotation-content-area-]').should('contain','some other text, some text0');
		cy.cGet('.annotation-marker').should('exist');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('[id^=annotation-content-area-]').should('contain','some other text, some text0');
		cy.cGet('.annotation-marker').should('exist');
	});

	it('Remove',function() {
		cy.cSetActiveFrame('#iframe1');
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('exist');
		cy.cGet('.cool-annotation-content > div').should('contain','some text');
		cy.cGet('.cool-annotation-table .avatar-img').click();
		cy.cGet('.cool-annotation-menu').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Remove').click();
		cy.cGet('.annotation-marker').should('not.exist');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.annotation-marker').should('not.exist');
	});

	// Skipping reply tests in Impress since reply functionality is temporarily disabled.
	it.skip('Reply',function() {
		cy.cSetActiveFrame('#iframe1');
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('exist');
		cy.cGet('.cool-annotation-content > div').should('contain','some text');
		cy.cGet('.cool-annotation-table .avatar-img').click();
		cy.cGet('.cool-annotation-menu').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Reply').click();
		cy.cGet('[id^=annotation-reply-textarea-]').type('some reply text');
		cy.cGet('[id^=annotation-reply-].button-primary').click();
		cy.cGet('.cool-annotation-content > div').should('include.text','some reply text');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.cool-annotation-content > div').should('include.text','some reply text');
	});
});

describe(['tagmultiuser'], 'Multiuser Annotation Autosave Tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/comment_switching.odp',true);
		cy.viewport(2600, 800);
		desktopHelper.switchUIToNotebookbar();

		cy.cSetActiveFrame('#iframe1');
		desktopHelper.closeNavigatorSidebar();
		desktopHelper.hideSidebarImpress();
		desktopHelper.selectZoomLevel('50', false);

		cy.cSetActiveFrame('#iframe2');
		desktopHelper.closeNavigatorSidebar();
		desktopHelper.hideSidebarImpress();
		desktopHelper.selectZoomLevel('50', false);
	});

	it('Insert autosave', function() {
		cy.cSetActiveFrame('#iframe1');
		desktopHelper.insertComment(undefined, false);
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('.cool-annotation-edit.modify-annotation').should('be.visible');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.annotation-marker').should('exist');
		cy.cGet('.cool-annotation-content > div').should('have.text','some text0');
	});

	it('Insert autosave save', function() {
		cy.cSetActiveFrame('#iframe1');
		desktopHelper.insertComment(undefined, false);
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('.cool-annotation-edit.modify-annotation').should('be.visible');
		cy.cGet('[id^=annotation-save-]').click();
		cy.cGet('.annotation-button-autosaved').should('be.not.visible');
		cy.cGet('.annotation-button-delete').should('be.not.visible');
		cy.cGet('.cool-annotation-edit.modify-annotation').should('be.not.visible');
		cy.cGet('.annotation-marker').should('exist');
		cy.cGet('.cool-annotation-content > div').should('have.text','some text0');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.annotation-marker').should('exist');
		cy.cGet('.cool-annotation-content > div').should('have.text','some text0');
	});

	it('Insert autosave cancel', function() {
		cy.cSetActiveFrame('#iframe1');
		desktopHelper.insertComment(undefined, false);
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('.cool-annotation-edit.modify-annotation').should('be.visible');
		cy.cGet('.modify-annotation [id^=annotation-cancel-]').click();
		cy.cGet('.annotation-button-autosaved').should('not.exist');
		cy.cGet('.annotation-button-delete').should('not.exist');
		cy.cGet('.cool-annotation-edit.modify-annotation').should('not.exist');
		cy.cGet('.annotation-marker').should('not.exist');
		cy.cGet('.cool-annotation-content > div').should('not.exist');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.annotation-marker').should('not.exist');
		cy.cGet('.cool-annotation-content > div').should('not.exist');
	});

	it('Modify autosave', function() {
		cy.cSetActiveFrame('#iframe1');
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('exist');
		cy.cGet('[id^=annotation-content-area-]').should('have.text','some text0');
		cy.cGet('.cool-annotation-content-wrapper:visible .cool-annotation-menu').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Modify').click();
		cy.cGet('[id^=annotation-modify-textarea-]').type('{home}');
		cy.cGet('[id^=annotation-modify-textarea-]').type('some other text, ');
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('.cool-annotation-edit.modify-annotation').should('be.visible');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.annotation-marker').should('exist');
		cy.cGet('.cool-annotation-content > div').should('have.text','some other text, some text0');
	});

	it('Modify autosave save', function() {
		cy.cSetActiveFrame('#iframe1');
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('exist');
		cy.cGet('[id^=annotation-content-area-]').should('have.text','some text0');
		cy.cGet('.cool-annotation-content-wrapper:visible .cool-annotation-menu').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Modify').click();
		cy.cGet('[id^=annotation-modify-textarea-]').type('{home}');
		cy.cGet('[id^=annotation-modify-textarea-]').type('some other text, ');
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('.cool-annotation-edit.modify-annotation').should('be.visible');
		cy.cGet('[id^=annotation-save-]').click();
		cy.cGet('[id^=annotation-content-area-]').should('have.text','some other text, some text0');
		cy.cGet('.annotation-marker').should('exist');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.annotation-marker').should('exist');
		cy.cGet('.cool-annotation-content > div').should('have.text','some other text, some text0');
	});

	it('Modify autosave cancel', function() {
		cy.cSetActiveFrame('#iframe1');
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('exist');
		cy.cGet('[id^=annotation-content-area-]').should('have.text','some text0');
		cy.cGet('.cool-annotation-content-wrapper:visible .cool-annotation-menu').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Modify').click();
		cy.cGet('[id^=annotation-modify-textarea-]').type('{home}');
		cy.cGet('[id^=annotation-modify-textarea-]').type('some other text, ');
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('.cool-annotation-edit.modify-annotation').should('be.visible');
		cy.cGet('.modify-annotation [id^=annotation-cancel-]').click();
		cy.cGet('[id^=annotation-content-area-]').should('have.text','some text0');
		cy.cGet('.annotation-marker').should('exist');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.annotation-marker').should('exist');
		cy.cGet('.cool-annotation-content > div').should('have.text','some text0');
	});

	// Skipping reply tests in Impress since reply functionality is temporarily disabled.
	it.skip('Reply autosave',function() {
		cy.cSetActiveFrame('#iframe1');
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('exist');
		cy.cGet('.cool-annotation-content > div').should('have.text','some text0');
		cy.cGet('.cool-annotation-content-wrapper:visible .cool-annotation-menu').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Reply').click();
		cy.cGet('[id^=annotation-reply-textarea-]').type('some reply text');
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('[id^=annotation-modify-textarea-]').should('be.visible');
		cy.cGet('[id^=annotation-modify-textarea-]').should('include.text', 'some text0');
		cy.cGet('[id^=annotation-modify-textarea-]').should('include.text', 'some reply text');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.cool-annotation-edit.reply-annotation').should('be.not.visible');
		cy.cGet('.cool-annotation-content > div').should('include.text','some reply text');
	});

	it.skip('Reply autosave save',function() {
		cy.cSetActiveFrame('#iframe1');
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('exist');
		cy.cGet('.cool-annotation-content > div').should('have.text','some text0');
		cy.cGet('.cool-annotation-content-wrapper:visible .cool-annotation-menu').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Reply').click();
		cy.cGet('[id^=annotation-reply-textarea-]').type('some reply text');
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('[id^=annotation-modify-textarea-]').should('be.visible');
		cy.cGet('[id^=annotation-modify-textarea-]').should('include.text', 'some text0');
		cy.cGet('[id^=annotation-modify-textarea-]').should('include.text', 'some reply text');
		cy.cGet('[id^=annotation-save-]').click();
		cy.cGet('.annotation-button-autosaved').should('be.not.visible');
		cy.cGet('.annotation-button-delete').should('be.not.visible');
		cy.cGet('.cool-annotation-edit.reply-annotation').should('be.not.visible');
		cy.cGet('.cool-annotation-content > div').should('include.text','some text0');
		cy.cGet('.cool-annotation-content > div').should('include.text','some reply text');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.cool-annotation-edit.reply-annotation').should('be.not.visible');
		cy.cGet('.cool-annotation-content > div').should('include.text','some reply text');
	});

	it.skip('Reply autosave cancel',function() {
		cy.cSetActiveFrame('#iframe1');
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('exist');
		cy.cGet('.cool-annotation-content > div').should('have.text','some text0');
		cy.cGet('.cool-annotation-content-wrapper:visible .cool-annotation-menu').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Reply').click();
		cy.cGet('[id^=annotation-reply-textarea-]').type('some reply text');
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('[id^=annotation-modify-textarea-]').should('be.visible');
		cy.cGet('[id^=annotation-modify-textarea-]').should('include.text', 'some text0');
		cy.cGet('[id^=annotation-modify-textarea-]').should('include.text', 'some reply text');
		cy.cGet('.modify-annotation [id^=annotation-cancel-]').click();
		cy.cGet('.annotation-button-autosaved').should('be.not.visible');
		cy.cGet('.annotation-button-delete').should('be.not.visible');
		cy.cGet('.cool-annotation-edit.reply-annotation').should('be.not.visible');
		cy.cGet('.cool-annotation-content > div').should('have.text','some text0');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.cool-annotation-edit.reply-annotation').should('be.not.visible');
		cy.cGet('.cool-annotation-content > div').should('have.text','some text0');
	});
});

// The document has two comments on slide 2. The first view stays on slide 1
// and inserts comments there; the second view watches slide 2. The comments
// shown on slide 2 must not move or flicker while the other view comments on
// a different slide.
describe(['tagmultiuser'], 'Comment inserted on another slide', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/annotation_on_other_slide.odp', true);
		cy.viewport(2600, 800);
		desktopHelper.switchUIToNotebookbar();

		cy.cSetActiveFrame('#iframe1');
		desktopHelper.closeNavigatorSidebar();
		desktopHelper.hideSidebarImpress();

		cy.cSetActiveFrame('#iframe2');
		desktopHelper.closeNavigatorSidebar();
		desktopHelper.hideSidebarImpress();
	});

	function commentPositions(win) {
		var section = win.app.sectionContainer.getSectionWithName(win.app.CSections.CommentList.name);
		var positions = {};
		section.sectionProperties.commentList.forEach(function(c) {
			positions[c.sectionProperties.data.id] = {
				part: c.sectionProperties.partIndex,
				left: c.sectionProperties.container.style.left,
				top: c.sectionProperties.container.style.top,
				display: c.sectionProperties.container.style.display
			};
		});
		return positions;
	}

	it('does not move the comments shown on the current slide', function() {
		// The second view looks at slide 2, where the document's comments live.
		cy.cSetActiveFrame('#iframe2');
		cy.getFrameWindow().then(function(win) {
			win.app.map.setPart(1);
			return helper.processToIdle(win);
		});
		// Both comments of slide 2 have containers placed by the layout.
		cy.cGet('.cool-annotation').should('have.length', 2);
		cy.cGet('.cool-annotation').each(function($el) {
			expect($el[0].style.left).to.not.equal('');
		});
		// Let the layouting task and the position transition settle.
		cy.wait(1000);

		var baseline = null;
		cy.getFrameWindow().then(function(win) {
			baseline = commentPositions(win);
		});

		// The first view inserts two comments on slide 1, one after the other.
		// Type through the helper but save by hand: after the save the comment
		// list can be rebuilt in slide order, which puts the slide 2 comments
		// after the new one, so the saved comment is found by its text rather
		// than by the position of its container.
		for (var round = 0; round < 2; round++) {
			cy.cSetActiveFrame('#iframe1');
			var text = 'comment ' + round + ' on slide one';
			desktopHelper.insertComment(text, false);
			cy.cGet('.cool-annotation').last({log: false}).find('[value="Save"]').click();
			cy.cGet('body').contains('.cool-annotation-content', text).should('exist');
			helper.typeIntoDocument('{esc}');
			cy.getFrameWindow().then(function(win) {
				return helper.processToIdle(win);
			});

			// The new comment reaches the second view but shows nothing there,
			// and the slide 2 comments stay exactly where they were. The wait
			// gives a buggy relayout and its position transition time to move
			// things before the position check.
			cy.cSetActiveFrame('#iframe2');
			cy.cGet('.cool-annotation').should('have.length', 3 + round);
			cy.wait(1000);
			cy.getFrameWindow().then(function(win) {
				var current = commentPositions(win);
				Object.keys(baseline).forEach(function(id) {
					expect(current[id].left, 'left of comment ' + id).to.equal(baseline[id].left);
					expect(current[id].top, 'top of comment ' + id).to.equal(baseline[id].top);
				});
				Object.keys(current).forEach(function(id) {
					if (!(id in baseline))
						expect(current[id].display, 'slide 1 comment stays hidden').to.equal('none');
				});
			});
		}
	});
});
