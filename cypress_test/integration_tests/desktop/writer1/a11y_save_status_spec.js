/* global describe expect it cy before require */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');
const a11yHelper = require('../../common/a11y_helper');

// Text rewritten on a timer ahead of the document moves a reader's offsets, and NVDA's say
// all then starts a line a character late.
describe(['tagdesktop'], 'Save status accessibility', { testIsolation: false }, function () {
	let win;

	before(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('writer/copy_paste.odt');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		desktopHelper.switchUIToNotebookbar();
		cy.cGet('.notebookbar-tabs-container').should('be.visible');
		cy.cGet('#save-status').should(function ($status) {
			expect($status.text(), 'the save status the map writes').to.not.equal('');
		});
	});

	it('the save status is the Save button\'s description, not text of the page', function () {
		if (!a11yHelper.axTreeAvailable())
			return;

		// The status is rewritten on a timer, so the tree and the DOM are read
		// together and compared again when a tick falls between them.
		function check(attempt) {
			return a11yHelper.getAXNodes().then(function (nodes) {
				const status = win.document.getElementById('save-status').textContent;
				const described = nodes.filter(function (node) {
					return !node.ignored && node.role === 'button' && node.description !== '';
				});
				const saveButton = described.find(function (node) {
					return node.description === status;
				});
				if (!saveButton && attempt < 3)
					return check(attempt + 1);

				expect(saveButton, 'a button described as "' + status + '"').to.not.equal(undefined);
				expect(saveButton.name, 'the button it describes').to.match(/Save$/);

				const asText = nodes.filter(function (node) {
					return !node.ignored && node.role !== 'button' && node.name.indexOf(status) !== -1;
				}).map(function (node) {
					return node.role + ' "' + node.name + '"';
				});
				expect(asText.join(', '), 'the status read as text of the page').to.equal('');
			});
		}

		cy.then(function () {
			return check(1);
		});
	});
});
