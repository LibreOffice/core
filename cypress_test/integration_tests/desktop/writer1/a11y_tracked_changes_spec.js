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

	it('the element with the caret stays until the caret is in the new paragraph', function () {
		helper.typeIntoDocument('{downarrow}{downarrow}');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		// Orca, when the element with the caret goes away on Up, moves out of the editable
		const early = [];
		let removed = 0;
		let observer;
		cy.cGet('#clipboard-area ins').should('have.text', 'Another verse').then(function ($ins) {
			const area = win.document.getElementById('clipboard-area');
			let shown = false;
			observer = new win.MutationObserver(function () {
				if (!shown && area.textContent.includes('Keep fresh')) {
					shown = true;
					if (!$ins[0].isConnected)
						early.push('removed before the new paragraph was shown');
				}
				if ($ins[0].isConnected || removed)
					return;
				++removed;
				const caret = win.getSelection().focusNode;
				if (!area.textContent.includes('Keep fresh and lazy then warm end.')
					|| !area.contains(caret))
					early.push(area.textContent);
			});
			observer.observe(area, { childList: true, subtree: true });
		});
		// the spacer stays last while it is kept
		let spacerObserver;
		cy.cGet('#clipboard-area').then(function ($area) {
			const area = $area[0];
			spacerObserver = new win.MutationObserver(function () {
				if (area.lastChild && area.lastChild.id !== 'post-space')
					early.push('after the post-space: ' + area.lastChild.nodeName);
			});
			spacerObserver.observe(area, { childList: true });
		});

		helper.typeIntoDocument('{uparrow}');
		cy.wrap(null).should(function () {
			expect(removed, 'the old insertion removed').to.be.greaterThan(0);
		});
		cy.then(function () {
			observer.disconnect();
			spacerObserver.disconnect();
			expect(early, 'removed before the caret was in the new paragraph').to.be.empty;
		});
	});

	it('the type, the author and the time of a change are its details', function () {
		// back in the paragraph with the changes
		cy.cGet('#clipboard-area').should(function ($area) {
			const check = function (selector, start, author) {
				const id = $area.find(selector).attr('aria-details');
				expect(id, 'the details of ' + selector).to.match(/^a11y-change-/);
				const target = win.document.getElementById(id);
				expect(target, 'the target of ' + selector).to.exist;
				expect(target.textContent, 'the details of ' + selector)
					.to.match(new RegExp('^' + start + ', ' + author + ', .*2026'));
			};
			check('ins', 'Insertion', 'Ann');
			check('del', 'Deletion', 'Ben');
			check('span[aria-description="Formatting changed"]', 'Formatting changed', 'Ann');
		});
	});

	it('a move to the next or the previous change announces the change reached', function () {
		helper.typeIntoDocument('{ctrl}{home}');
		cy.then(function () {
			return helper.processToIdle(win);
		});
		const expectStatus = function (pattern) {
			cy.cGet('#a11y-jump-status').should(function ($status) {
				expect($status.text(), 'the status message').to.match(pattern);
			});
		};
		helper.typeIntoDocument('{ctrl}{alt}{shift}{pagedown}');
		expectStatus(/^Insertion, Ann, .*2026.*: fresh$/);
		helper.typeIntoDocument('{ctrl}{alt}{shift}{pagedown}');
		expectStatus(/^Deletion, Ben, .*2026.*: lazy $/);
		helper.typeIntoDocument('{ctrl}{alt}{shift}{pageup}');
		expectStatus(/^Insertion, Ann, .*2026.*: fresh$/);

		// the same change reached again
		cy.then(function () {
			win.document.getElementById('a11y-jump-status').textContent = '';
		});
		helper.typeIntoDocument('{home}');
		helper.typeIntoDocument('{ctrl}{alt}{shift}{pagedown}');
		expectStatus(/^Insertion, Ann, .*2026.*: fresh$/);

		// a change in the next paragraph
		helper.typeIntoDocument('{ctrl}{alt}{shift}{pagedown}');
		helper.typeIntoDocument('{ctrl}{alt}{shift}{pagedown}');
		expectStatus(/^Formatting changed, Ann, .*2026.*: warm$/);
		helper.typeIntoDocument('{ctrl}{alt}{shift}{pagedown}');
		expectStatus(/^Insertion, Ben, .*2026.*: Another verse$/);
	});

	it('an empty paragraph reached by Down keeps a line for the caret', function () {
		helper.typeIntoDocument('{ctrl}{home}');
		helper.typeIntoDocument('{downarrow}{downarrow}');
		cy.cGet('#clipboard-area ins').should('have.text', 'Another verse');

		// Orca, with no line in the editable, leaves focus mode as the insertion goes away
		helper.typeIntoDocument('{downarrow}');
		cy.cGet('#clipboard-area').should(function ($area) {
			expect($area.text(), 'the empty paragraph').to.equal('');
			// Without the fix, there were only the two spacers.
			expect($area.children('br'), 'the line').to.have.length(1);
			const selection = win.getSelection();
			expect(selection.focusNode, 'the caret in the editable').to.equal($area[0]);
			expect(selection.focusOffset, 'before the line').to.equal(1);
		});

		// And Delete joins the next paragraph, as in any empty paragraph
		helper.typeIntoDocument('{del}');
		cy.cGet('#clipboard-area').should('have.text', 'Last paragraph');
	});
});
