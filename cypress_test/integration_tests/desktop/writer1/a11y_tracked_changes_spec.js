/* global describe expect it cy before require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Tracked changes for the reader', { testIsolation: false }, function () {
	let win;

	before(function () {
		helper.setupAndLoadDocument('writer/a11y_tracked_changes.fodt');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			win.app.map.setAccessibilityState(true);
			return helper.processToIdle(win);
		});

		// into the paragraph with the changes
		helper.typeIntoDocument('{ctrl}{home}');
		helper.typeIntoDocument('{downarrow}');
		cy.then(function () {
			return helper.processToIdle(win);
		});
	});

	it('an insertion, a deletion and an attribute change are marked as such', function () {
		cy.cGet('#clipboard-area').should(function ($area) {
			expect($area.text(), 'the paragraph text')
				.to.contain('Keep fresh and lazy then warm end.');
			expect($area.find('ins').text(), 'the insertion').to.equal('fresh');
			expect($area.find('del').text(), 'the deletion').to.equal('lazy ');
			const $format = $area.find('span[aria-description="Formatting changed"]');
			expect($format.text(), 'the attribute change').to.equal('warm');
			expect($format.attr('role'), 'its role').to.be.undefined;
		});
		// the accessibility tree follows the DOM later
		cy.waitUntil(function () {
			return a11yHelper.getAXNodesWithin('#clipboard-area').then(function (nodes) {
				const shown = nodes.filter(function (node) { return !node.ignored; });
				const roles = shown.map(function (node) { return node.role; });
				const descriptions = shown.map(function (node) { return node.description; });
				return roles.includes('insertion') && roles.includes('deletion')
					&& descriptions.includes('Formatting changed');
			});
		});
	});

	it('the paragraph after the caret has its changes marked too', function () {
		helper.typeIntoDocument('{uparrow}');
		cy.then(function () {
			return helper.processToIdle(win);
		});
		cy.cGet('#a11y-context-after').should(function ($after) {
			const $paragraphs = $after.children();
			expect($paragraphs.eq(0).find('ins').text(), 'the insertion').to.equal('fresh');
			expect($paragraphs.eq(0).find('del').text(), 'the deletion').to.equal('lazy ');
			expect($paragraphs.eq(1).find('ins').text(), 'the paragraph inserted whole')
				.to.equal('Another verse');
		});
	});
});
