/* global describe it cy beforeEach require expect NodeFilter */

var helper = require('../../common/helper');

// The speaker notes pane below the slide renders the notes outliner as an
// editengine custom widget, so what is typed there ends up on the slide's notes
// page rather than on the slide itself.
describe(['tagdesktop'], 'Impress speaker notes pane', function () {
	let newFileName;

	beforeEach(function () {
		newFileName = helper.setupAndLoadDocument('impress/empty-placeholder.fodp');
	});

	function openNotesPane() {
		cy.getFrameWindow().then(function (win) {
			win.app.dispatcher.dispatch('notespanel');
		});

		cy.cGet('#notespanel-dock-wrapper').should('be.visible');
		cy.cGet('#notespanel-container .ui-editengine').should('exist');
	}

	// Cypress sees the document iframe as the focused element, so its blur()
	// refuses to act on the editor; blur the element itself instead.
	function leaveNotes() {
		cy.cGet('#notespanel-container .ui-editengine').then(function ($editor) {
			$editor[0].blur();
		});
	}

	it('opens on request and shows an editable notes area', function () {
		cy.cGet('#notespanel-dock-wrapper').should('not.be.visible');

		openNotesPane();

		cy.cGet('#notespanel-container .ui-editengine').should(
			'have.attr',
			'contenteditable',
			'true'
		);
	});

	it('clicking into empty notes replaces the placeholder with the typed text', function () {
		openNotesPane();

		var editEngine = '#notespanel-container .ui-editengine';
		var paragraph = editEngine + ' .ui-editengine-paragraph';
		cy.cGet(paragraph).should('contain.text', 'Click to add Notes');

		cy.cGet(editEngine).click();
		cy.cGet(paragraph).should('not.contain.text', 'Click to add Notes');

		// Leaving the notes empty brings the placeholder back. The blur is called on the element
		// itself, because cy.blur() looks for the focus in the top window, where the frame holds it.
		cy.cGet(editEngine).then(function (element) {
			element[0].blur();
		});
		cy.cGet(paragraph).should('contain.text', 'Click to add Notes');

		cy.cGet(editEngine).click();
		cy.cGet(paragraph).should('not.contain.text', 'Click to add Notes');
		cy.cGet(editEngine).type('Remember the demo');
		cy.cGet(paragraph).should('have.text', 'Remember the demo');
	});

	// Once the slide has notes, a click only places the caret.
	it('a click into existing notes keeps the text', function () {
		openNotesPane();

		cy.cGet('#notespanel-container .ui-editengine').click();
		cy.cGet('#notespanel-container .ui-editengine').type('Remember the demo');
		leaveNotes();
		cy.cGet('#notespanel-container .ui-editengine-paragraph').should(
			'have.text',
			'Remember the demo'
		);

		cy.cGet('#notespanel-container .ui-editengine').click();
		cy.cGet('#notespanel-container .ui-editengine-paragraph').should(
			'have.text',
			'Remember the demo'
		);
	});

	// Adding a slide moves the pane to the notes of the new slide while the
	// editor keeps the focus. Those notes are entered right away, so their
	// placeholder goes too, and the notes typed before stay with their slide.
	it('a slide added while typing starts with empty notes as well', function () {
		openNotesPane();

		cy.cGet('#notespanel-container .ui-editengine').click();
		cy.cGet('#notespanel-container .ui-editengine').type('First slide');
		cy.cGet('#notespanel-container .ui-editengine-paragraph').should(
			'have.text',
			'First slide'
		);

		cy.getFrameWindow().then(function (win) {
			win.app.map.sendUnoCommand('.uno:InsertPage');
		});
		cy.cGet('#slide-sorter .preview-img').should('have.length', 3);

		cy.cGet('#notespanel-container .ui-editengine-paragraph').should(
			'have.text',
			''
		);
		cy.cGet('#notespanel-container .ui-editengine').type('Second slide');
		cy.cGet('#notespanel-container .ui-editengine-paragraph').should(
			'have.text',
			'Second slide'
		);

		cy.getFrameWindow().then(function (win) {
			win.app.map.setPart(0);
		});
		cy.cGet('#notespanel-container .ui-editengine-paragraph').should(
			'have.text',
			'First slide'
		);
	});

	// A click on a slide thumbnail puts the slide sorter in charge of the
	// keys. Clicking into the notes afterwards has to take them back, so
	// that everything typed there lands in the notes.
	it('keys typed after a thumbnail click stay in the notes', function () {
		openNotesPane();

		cy.cGet('#slide-sorter .preview-img').eq(1).click();

		cy.cGet('#notespanel-container .ui-editengine').click();
		cy.cGet('#notespanel-container .ui-editengine').type('abc');
		cy.cGet('#notespanel-container .ui-editengine-paragraph').should(
			'have.text',
			'abc'
		);
		cy.cGet('#notespanel-container .ui-editengine').should('have.focus');
	});

	it('typed text reaches the notes of the current slide', function () {
		openNotesPane();

		cy.cGet('#notespanel-container .ui-editengine').click();
		cy.cGet('#notespanel-container .ui-editengine').type('Remember the demo');

		cy.cGet('#notespanel-container .ui-editengine-paragraph').should(
			'contain.text',
			'Remember the demo'
		);

		cy.getFrameWindow().then((win) => { helper.processToIdle(win); });
		helper.reloadDocument(newFileName);
		openNotesPane();

		cy.cGet('#notespanel-container .ui-editengine-paragraph').should(
			'contain.text',
			'Remember the demo'
		);
	});

	// The three notes views are mutually exclusive, so switching the pane on
	// from the status bar has to leave the handout page behind and bring back
	// a pane that holds the notes of the slide and still takes typing.
	it('the status bar button swaps the handout page for a working pane', function () {
		cy.viewport(1920, 1080);

		openNotesPane();
		cy.cGet('#notespanel-container .ui-editengine').click();
		cy.cGet('#notespanel-container .ui-editengine').type('Remember the demo');
		cy.cGet('#notespanel-container .ui-editengine-paragraph').should(
			'contain.text',
			'Remember the demo'
		);

		cy.getFrameWindow().then(function (win) {
			win.app.dispatcher.dispatch('notespanelhandout');
		});

		cy.getFrameWindow().its('app.impress.notesMode').should('eq', true);
		cy.cGet('#notespanel-dock-wrapper').should('not.be.visible');

		cy.cGet('#toolbar-down #notespanel').click();

		cy.getFrameWindow().its('app.impress.notesMode').should('eq', false);
		cy.cGet('#notespanel-dock-wrapper').should('be.visible');

		// The notes of the slide are there, and they stay: the engine sends a
		// late update for every page it passes through on the way out of the
		// handout page, and an empty notes placeholder reads "Click to add
		// Notes".
		cy.getFrameWindow().then((win) => { helper.processToIdle(win); });
		cy.cGet('#notespanel-container .ui-editengine-paragraph').should(
			'contain.text',
			'Remember the demo'
		);

		// Typing still reaches the notes editor.
		cy.cGet('#notespanel-container .ui-editengine').click();
		cy.cGet('#notespanel-container .ui-editengine').type(' again');
		cy.cGet('#notespanel-container .ui-editengine-paragraph').should(
			'contain.text',
			'again'
		);
	});

	// The handout page lists other pages than the slides, so every preview is fetched again
	// after the switch. The kit sends no reply for a preview of a page the view no longer lists,
	// and such a request stays counted as under way. The test puts the view in that state at
	// the moment the new page list arrives.
	it('the previews come back after a switch to the handout page', function () {
		function expectThumbnailsInView(win) {
			var preview = win.app.map._docLayer._preview;
			var previews = preview._previewTiles.filter(function (img, index) {
				return preview._isPreviewVisible(index);
			});
			expect(previews).to.have.length.greaterThan(0);
			previews.forEach(function (img) {
				expect(img.placeholderName).to.equal(null);
			});
		}

		var slideParts;
		cy.getFrameWindow().should(function (win) {
			expectThumbnailsInView(win);
			slideParts = win.app.impress.partList.map(function (page) {
				return page.part;
			});
		});

		cy.getFrameWindow().then(function (win) {
			win.app.map.once('updateparts', function () {
				// Three preview requests went out just now, and none of them gets a reply.
				win.app.map._previewRequestsOnFly = 3;
				win.app.map._timeToEmptyQueue = new Date();
			});
			win.app.dispatcher.dispatch('notespanelhandout');
		});

		cy.getFrameWindow().its('app.impress.notesMode').should('eq', true);

		// The preview queue waits for the requests that get no reply, then goes on by itself.
		cy.getFrameWindow().then(function (win) {
			cy.waitUntil(function () {
				return win.app.timerRegistry.hasActive('previewqueue');
			}, { interval: 50 });
			helper.waitForTimers(win, 'previewqueue');
		});

		// Every preview in view holds a page of the handout list and shows its thumbnail,
		// without a scroll.
		cy.getFrameWindow().should(function (win) {
			var preview = win.app.map._docLayer._preview;
			var handoutParts = win.app.impress.partList.map(function (page) {
				return page.part;
			});
			handoutParts.forEach(function (part) {
				expect(slideParts).not.to.include(part);
			});
			expect(preview._previewTiles.map(function (img) {
				return img._part;
			})).to.deep.equal(handoutParts);
			expectThumbnailsInView(win);
		});
	});

	it('Deleting a selection dragged out of the notes pane removes all of it', function () {
		openNotesPane();

		var editEngine = '#notespanel-container .ui-editengine';
		var paragraph = editEngine + ' .ui-editengine-paragraph';
		cy.cGet(editEngine).click();
		cy.cGet(editEngine).type('ONE TWO');
		cy.cGet(paragraph).should('contain.text', 'ONE TWO');
		cy.getFrameWindow().then((win) => { helper.processToIdle(win); });

		// Press in the notes, select "ONE " and release the mouse outside the notes pane.
		var textBefore;
		cy.cGet(editEngine).trigger('mousedown');
		cy.cGet(paragraph).then(function (element) {
			textBefore = element[0].textContent;
			// The paragraph can hold other text before the typed words, and the words can sit
			// directly in the paragraph or inside a formatting run.
			var walker = element[0].ownerDocument.createTreeWalker(element[0], NodeFilter.SHOW_TEXT);
			var text = walker.nextNode();
			while (text && text.data.indexOf('ONE ') === -1)
				text = walker.nextNode();
			var start = text.data.indexOf('ONE ');
			element[0].ownerDocument.getSelection().setBaseAndExtent(text, start + 4, text, start);
		});
		cy.cGet('body').trigger('mouseup');

		cy.getFrameWindow().then((win) => { helper.processToIdle(win); });
		cy.cGet(editEngine).type('{backspace}');

		cy.cGet(paragraph).should(function (element) {
			expect(element[0].textContent).to.equal(textBefore.replace('ONE ', ''));
		});
	});

	it('Enter starts a new paragraph', function () {
		openNotesPane();

		cy.cGet('#notespanel-container .ui-editengine').click();
		cy.cGet('#notespanel-container .ui-editengine').type('first{enter}second');

		cy.cGet('#notespanel-container .ui-editengine-paragraph').should(
			'have.length',
			2
		);
	});

	it('Ctrl+V works while the notes panel has focus', function () {
		// Given an open notes panel with its editengine widget focused:
		openNotesPane();
		cy.cGet('#notespanel-container .ui-editengine').click();

		// When simulating Ctrl+V on the focused widget and then a paste event
		// carrying external text:
		cy.getFrameWindow().then(function (win) {
			const editable = win.document.querySelector(
				'#notespanel-container .ui-editengine'
			);
			editable.focus();
			const keyEvent = new win.KeyboardEvent('keydown', {
				key: 'v', code: 'KeyV', ctrlKey: true,
				bubbles: true, cancelable: true
			});
			editable.dispatchEvent(keyEvent);

			// Then the content is pasted to the notes pane:
			// Without the accompanying fix in place, this test would have failed with:
			// assert expected false to equal **true**
			// i.e. the paste went to the main document content.
			expect(win.L.Map.THIS._clip._isAnyInputFieldSelected()).to.equal(true);
			expect(keyEvent.defaultPrevented).to.equal(false);
			// Cypress does not produce a native paste event from a synthetic
			// Ctrl+V keydown, so call it directly.
			const dt = new win.DataTransfer();
			dt.setData('text/plain', 'external text');
			const pasteEvent = new win.ClipboardEvent('paste', {
				clipboardData: dt, bubbles: true, cancelable: true
			});
			editable.dispatchEvent(pasteEvent);
		});
		cy.cGet('#notespanel-container .ui-editengine-paragraph').should(
			'contain.text',
			'external text'
		);
	});
});
