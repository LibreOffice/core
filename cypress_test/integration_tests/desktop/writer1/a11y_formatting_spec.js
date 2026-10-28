/* global describe expect it cy before require */

const helper = require('../../common/helper');

describe(['tagdesktop'], 'Writer character formatting for the reader', { testIsolation: false }, function () {
	let win;

	before(function () {
		helper.setupAndLoadDocument('writer/a11y_formatting.fodt');

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

	// each formatted text of an element, as "text: formatting", from the style the browser applied
	function runs(element) {
		const result = [];
		const doc = element.ownerDocument;
		const walker = doc.createTreeWalker(element, doc.defaultView.NodeFilter.SHOW_TEXT);
		for (let node = walker.nextNode(); node; node = walker.nextNode()) {
			const parent = node.parentElement;
			const style = parent.ownerDocument.defaultView.getComputedStyle(parent);
			const kinds = [];
			if (parseInt(style.fontWeight) >= 600)
				kinds.push('bold');
			if (style.fontStyle === 'italic')
				kinds.push('italic');
			if (style.textDecorationLine.includes('underline'))
				kinds.push('underline');
			if (style.textDecorationLine.includes('line-through'))
				kinds.push('line-through');
			if (parent.closest('sup'))
				kinds.push('sup');
			if (parent.closest('sub'))
				kinds.push('sub');
			if (kinds.length)
				result.push(node.data + ': ' + kinds.join(' '));
		}
		return result;
	}

	// "bold" is two runs in two fonts, mirrored as one
	const expected = [
		'bold: bold',
		'italic: italic',
		'under: underline',
		'struck: line-through',
		'2: sup',
		'2: sub',
	];

	it('the paragraph after the caret has its formatting', function () {
		cy.cGet('#a11y-context-after').should(function ($after) {
			expect(runs($after[0].children[0]), 'the runs of the next paragraph').to.deep.equal(expected);
		});
	});

	it('the paragraph at the caret has its formatting, and its text is unchanged', function () {
		helper.typeIntoDocument('{downarrow}');
		cy.then(function () {
			return helper.processToIdle(win);
		});
		cy.cGet('#clipboard-area').should(function ($area) {
			expect($area.text(), 'the paragraph text').to.contain('A bold italic under struck x2 H2O end.');
			expect(runs($area[0]), 'the runs').to.deep.equal(expected);
		});
	});

	it('a run made bold from the keyboard is mirrored', function () {
		// the "A" at the start of the paragraph
		helper.typeIntoDocument('{home}{shift}{rightarrow}');
		cy.then(function () {
			return helper.processToIdle(win);
		});
		helper.typeIntoDocument('{ctrl}b');
		cy.then(function () {
			return helper.processToIdle(win);
		});
		cy.cGet('#clipboard-area').should(function ($area) {
			expect(runs($area[0])[0], 'the first run').to.equal('A: bold');
		});
	});
});
