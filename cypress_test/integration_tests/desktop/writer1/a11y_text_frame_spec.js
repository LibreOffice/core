/* global describe expect it cy before require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Writer text frames for the reader', { testIsolation: false }, function () {
	let win;

	before(function () {
		helper.setupAndLoadDocument('writer/a11y_text_frame.fodt');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			win.app.map.setAccessibilityState(true);
			return helper.processToIdle(win);
		});

		helper.typeIntoDocument('{ctrl}{home}');
		cy.then(function () {
			return helper.processToIdle(win);
		});
	});

	it('a text frame in the text is read by its title, as a frame', function () {
		helper.typeIntoDocument('{downarrow}');
		cy.then(function () {
			return helper.processToIdle(win);
		});
		cy.cGet('#clipboard-area [role="img"]').should(function ($frames) {
			expect($frames, 'the frames').to.have.length(1);
			expect($frames.attr('aria-label'), 'the title').to.equal('Note box');
			expect($frames.attr('aria-roledescription'), 'the role').to.equal('Frame');
			expect($frames.attr('aria-description'), 'the description').to.equal('A box beside the text');
		});
		cy.then(function () {
			return a11yHelper.getAXNodesWithin('#clipboard-area').then(function (nodes) {
				const frames = nodes.filter(function (node) {
					return !node.ignored && (node.role === 'image' || node.role === 'img');
				});
				expect(frames.map(function (node) { return node.name; }), 'the frames read')
					.to.deep.equal(['Note box']);
				expect(frames.map(function (node) { return node.properties.roledescription; }),
					'the roles read').to.deep.equal(['Frame']);
			});
		});
	});
});
