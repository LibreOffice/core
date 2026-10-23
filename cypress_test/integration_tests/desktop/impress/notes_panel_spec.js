/* global describe it cy beforeEach require expect NodeFilter */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

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

	// Every preview in the visible part of the slide list shows its thumbnail.
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

	it('the placeholder shows apart from typed text', function () {
		openNotesPane();

		var editEngine = '#notespanel-container .ui-editengine';
		cy.cGet(editEngine).should('have.class', 'ui-editengine-placeholder');

		cy.cGet(editEngine).click();
		cy.cGet(editEngine).should('not.have.class', 'ui-editengine-placeholder');

		leaveNotes();
		cy.cGet(editEngine).should('have.class', 'ui-editengine-placeholder');

		cy.cGet(editEngine).click();
		cy.cGet(editEngine).type('Remember the demo');
		leaveNotes();
		cy.cGet('#notespanel-container .ui-editengine-paragraph').should(
			'have.text',
			'Remember the demo'
		);
		cy.cGet(editEngine).should('not.have.class', 'ui-editengine-placeholder');
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

	// A thumbnail click moves the focus to the thumbnail, so the pane shows the
	// notes of the clicked slide with no caret in them. The document selection
	// leaves the pane together with the focus: WebKit draws a caret for a
	// selection in editable content whether or not that content has the focus.
	it('a thumbnail click takes the caret out of the notes', function () {
		openNotesPane();

		var editEngine = '#notespanel-container .ui-editengine';
		var paragraph = editEngine + ' .ui-editengine-paragraph';

		cy.cGet(editEngine).click();
		cy.cGet(editEngine).type('First slide');
		cy.cGet(paragraph).should('have.text', 'First slide');

		cy.cGet('#slide-sorter .preview-img').eq(1).click();

		cy.cGet(paragraph).should('contain.text', 'Click to add Notes');
		cy.cGet(editEngine).should('not.have.focus');
		cy.getFrameWindow().should(function (win) {
			var editor = win.document.querySelector(editEngine);
			expect(editor).to.not.equal(null);

			var selection = win.getSelection();
			if (selection.rangeCount === 0) return;
			var range = selection.getRangeAt(0);
			expect(editor.contains(range.startContainer)).to.equal(false);
			expect(editor.contains(range.endContainer)).to.equal(false);
		});
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
	// after the switch. A preview request that gets no reply stays counted as under way. The test
	// puts the view in that state at the moment the new page list arrives.
	it('the previews come back after a switch to the handout page', function () {
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

	// A preview request for a page the view does not list is answered, so it leaves the preview
	// queue at once. One request names a page that no page holds. The other names a slide, which
	// the handout view does not list.
	it('a preview of a page the view does not list is answered', function () {
		var slideParts;
		cy.getFrameWindow().then(function (win) {
			slideParts = win.app.impress.partList.map(function (page) {
				return page.part;
			});
			win.app.dispatcher.dispatch('notespanelhandout');
		});

		cy.getFrameWindow().its('app.impress.notesMode').should('eq', true);
		cy.getFrameWindow().should(function (win) {
			win.app.impress.partList.forEach(function (page) {
				expect(slideParts).not.to.include(page.part);
			});
			expectThumbnailsInView(win);
			expect(win.app.map._previewRequestsOnFly).to.equal(0);
		});

		var goneId;
		var answeredParts = [];
		function onPreviewGone(e) {
			answeredParts.push(e.part);
		}

		cy.getFrameWindow().then(function (win) {
			// The same identifier as the first slide, with its last hex digit changed.
			var slideId = slideParts[0];
			var lastDigit = slideId.search(/[0-9a-fA-F][^0-9a-fA-F]*$/);
			goneId = slideId.substring(0, lastDigit) +
				(slideId[lastDigit] === '0' ? '1' : '0') +
				slideId.substring(lastDigit + 1);
			expect(slideParts).not.to.include(goneId);

			win.app.map.on('tilepreviewgone', onPreviewGone);

			// The preview queue counts both requests as under way, as it does for the ones it
			// sends itself.
			win.app.map._previewRequestsOnFly += 2;
			win.app.map._timeToEmptyQueue = new Date();
			[goneId, slideId].forEach(function (part) {
				win.app.socket.sendMessage('tile nviewid=0 part=' + part + ' mode=0 ' +
					'width=180 height=135 tileposx=0 tileposy=0 ' +
					'tilewidth=15875 tileheight=11906 id=0');
			});
		});

		cy.getFrameWindow().should(function (win) {
			expect(answeredParts).to.have.members([goneId, slideParts[0]]);
			expect(win.app.map._previewRequestsOnFly).to.equal(0);
		});

		cy.getFrameWindow().then(function (win) {
			win.app.map.off('tilepreviewgone', onPreviewGone);
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

	// The notes are plain paragraphs, so Ctrl+Enter starts the next paragraph at the same level, as
	// Enter does.
	it('the notes scroll to keep the caret in view while new lines are typed', function () {
		var editEngine = '#notespanel-container .ui-editengine';
		var paragraph = editEngine + ' .ui-editengine-paragraph';

		openNotesPane();
		cy.cGet(editEngine).click();
		for (var i = 1; i <= 15; i++)
			cy.cGet(editEngine).type('Line ' + i + '{enter}');
		cy.cGet(paragraph).should('have.length.at.least', 16);

		cy.cGet(editEngine).should(function (editor) {
			expect(editor[0].scrollHeight).to.be.greaterThan(editor[0].clientHeight);
			var view = editor[0].getBoundingClientRect();
			var paragraphs = editor[0].querySelectorAll('.ui-editengine-paragraph');
			var last = paragraphs[paragraphs.length - 1].getBoundingClientRect();
			expect(last.top).to.be.at.least(view.top);
			expect(last.bottom).to.be.at.most(view.bottom + 1);
		});
	});

	it('Ctrl+Enter starts a new paragraph at the same level', function () {
		openNotesPane();

		var editEngine = '#notespanel-container .ui-editengine';
		var paragraph = editEngine + ' .ui-editengine-paragraph';
		cy.cGet(editEngine).click();
		cy.cGet(editEngine).type('first');
		cy.cGet(editEngine).type('{ctrl}{enter}');
		cy.cGet(editEngine).type('second');

		cy.cGet(paragraph).should('have.length', 2);
		cy.cGet(paragraph).eq(1).should('have.text', 'second');
		cy.cGet(paragraph).eq(1).should('not.have.attr', 'data-bullet');
		cy.cGet(paragraph).eq(1).should(function (element) {
			expect(element[0].style.paddingInlineStart).to.equal('');
		});
	});

	// Text that has no formatting of its own shows in the font of the notes style, the same font
	// as text that has.
	it('the notes show in the font they have in the document, 20pt as 14pt', function () {
		openNotesPane();

		var editEngine = '#notespanel-container .ui-editengine';
		var paragraph = editEngine + ' .ui-editengine-paragraph';
		cy.cGet(editEngine).click();
		cy.cGet(editEngine).type('Remember the demo');
		cy.cGet(paragraph).should('have.text', 'Remember the demo');

		// The paragraph names the family and the size of the notes style.
		cy.cGet(paragraph).should(function (element) {
			expect(element[0].style.fontFamily).to.not.equal('');
			var style = element[0].ownerDocument.defaultView.getComputedStyle(element[0]);
			var points = parseFloat(style.fontSize) * 0.75;
			expect(points).to.be.closeTo(14, 0.1);
		});
	});

	it('small notes text shows at 11pt at least', function () {
		openNotesPane();

		var editEngine = '#notespanel-container .ui-editengine';
		var paragraph = editEngine + ' .ui-editengine-paragraph';
		cy.cGet(editEngine).click();
		cy.cGet(editEngine).type('Small print');
		cy.cGet(paragraph).should('have.text', 'Small print');

		cy.getFrameWindow().then(function (win) {
			win.app.map.sendUnoCommand('.uno:SelectAll');
			helper.processToIdle(win);
			win.app.map.applyFontSize('8');
			helper.processToIdle(win);
		});

		cy.cGet(paragraph).contains('Small print').should(function (element) {
			var style = element[0].ownerDocument.defaultView.getComputedStyle(element[0]);
			var points = parseFloat(style.fontSize) * 0.75;
			expect(points).to.be.closeTo(11, 0.1);
		});
	});

	// Notes taller than the pane scroll, and each paragraph stays as tall as its text, so each
	// paragraph sits below the one before it.
	it('each paragraph of long notes keeps the height of its text', function () {
		openNotesPane();

		var editEngine = '#notespanel-container .ui-editengine';
		var paragraph = editEngine + ' .ui-editengine-paragraph';
		var line = 'Testing testing testing testing '.repeat(8);
		var lines = [];
		for (var i = 0; i < 6; ++i)
			lines.push(line);

		cy.cGet(editEngine).click();
		cy.cGet(editEngine).then(function (element) {
			var win = element[0].ownerDocument.defaultView;
			var data = new win.DataTransfer();
			data.setData('text/plain', lines.join('\n'));
			element[0].dispatchEvent(new win.ClipboardEvent('paste', {
				clipboardData: data, bubbles: true, cancelable: true
			}));
		});

		cy.cGet(paragraph).should('have.length', 6);
		cy.cGet(editEngine).should(function (element) {
			expect(element[0].scrollHeight).to.be.greaterThan(element[0].clientHeight);
		});
		cy.cGet(paragraph).should(function (elements) {
			elements.each(function (index, element) {
				expect(element.getBoundingClientRect().height)
					.to.be.at.least(element.scrollHeight - 1);
			});
		});
	});

	// While the notes have the focus, the Undo and Redo buttons act on what was typed there. The
	// slide added before stays, although the document can undo adding it.
	it('the Undo and Redo buttons act on the notes while they have the focus', function () {
		var editEngine = '#notespanel-container .ui-editengine';
		var paragraph = editEngine + ' .ui-editengine-paragraph';

		cy.getFrameWindow().then(function (win) {
			win.app.map.sendUnoCommand('.uno:InsertPage');
		});
		cy.cGet('#slide-sorter .preview-img').should('have.length', 3);

		openNotesPane();
		cy.cGet(editEngine).click();
		// One word, because each typed word is an undo step of its own.
		cy.cGet(editEngine).type('Demo');
		cy.cGet(paragraph).should('have.text', 'Demo');
		cy.getFrameWindow().then((win) => { helper.processToIdle(win); });

		desktopHelper.getNbItem('Undo').should('not.have.attr', 'disabled');
		desktopHelper.getNbIcon('Undo').click();
		cy.cGet(paragraph).should('have.text', '');
		cy.cGet(editEngine).should('have.focus');
		cy.getFrameWindow().then((win) => { helper.processToIdle(win); });
		cy.cGet('#slide-sorter .preview-img').should('have.length', 3);

		desktopHelper.getNbItem('Redo').should('not.have.attr', 'disabled');
		desktopHelper.getNbIcon('Redo').click();
		cy.cGet(paragraph).should('have.text', 'Demo');
	});

	// While the notes have the focus, Select All selects the notes text. After leaving the notes
	// it selects the slide content again.
	it('Select All acts on the notes while they have the focus', function () {
		var editEngine = '#notespanel-container .ui-editengine';
		var paragraph = editEngine + ' .ui-editengine-paragraph';

		openNotesPane();
		cy.cGet(editEngine).click();
		cy.cGet(editEngine).type('Demo');
		cy.cGet(paragraph).should('have.text', 'Demo');

		cy.getFrameWindow().then(function (win) {
			win.app.map.sendUnoCommand('.uno:SelectAll');
			helper.processToIdle(win);
		});
		cy.cGet(editEngine).type('X');
		cy.cGet(paragraph).should('have.text', 'X');

		leaveNotes();
		cy.cGet(paragraph).should('have.text', 'X');

		cy.getFrameWindow().then(function (win) {
			win.app.map.sendUnoCommand('.uno:SelectAll');
			helper.processToIdle(win);
		});
		cy.cGet('#document-container svg g.Page g').should('exist');
		cy.cGet(paragraph).should('have.text', 'X');
	});

	it('the selected notes text stays marked while its context menu is open', function () {
		var editEngine = '#notespanel-container .ui-editengine';
		var paragraph = editEngine + ' .ui-editengine-paragraph';

		// The marked text, or null when nothing is marked.
		function markedText(win) {
			var highlight = win.CSS.highlights.get('editengine-menu-selection');
			if (!highlight) return null;
			return Array.from(highlight.keys()).map(String).join('');
		}

		openNotesPane();
		cy.cGet(editEngine).click();
		cy.cGet(editEngine).type('Demo');
		cy.cGet(paragraph).should('have.text', 'Demo');

		cy.getFrameWindow().then(function (win) {
			win.app.map.sendUnoCommand('.uno:SelectAll');
			helper.processToIdle(win);
		});

		cy.cGet(editEngine).rightclick();
		helper.getContextMenuItem('Cut').should('be.visible');
		cy.getFrameWindow().then(function (win) {
			helper.processToIdle(win);
		});
		cy.getFrameWindow().should(function (win) {
			expect(markedText(win)).to.equal('Demo');
		});

		cy.cGet('#jsd-context-menu-dropdown-overlay').click();
		cy.cGet('#jsd-context-menu-dropdown-overlay').should('not.exist');
		cy.getFrameWindow().should(function (win) {
			expect(markedText(win)).to.equal(null);
		});
	});

	it('a right click on text selected over several lines opens the menu at the click', function () {
		var editEngine = '#notespanel-container .ui-editengine';
		var paragraph = editEngine + ' .ui-editengine-paragraph';

		openNotesPane();
		cy.cGet(editEngine).click();
		cy.cGet(editEngine).type('One{enter}Two');
		cy.cGet(paragraph).should('have.length', 2);

		cy.getFrameWindow().then(function (win) {
			win.app.map.sendUnoCommand('.uno:SelectAll');
			helper.processToIdle(win);
		});

		// The middle of the second line, away from the left edge of the pane.
		var clickX;
		cy.cGet(paragraph).last().then(function (element) {
			var rect = element[0].getBoundingClientRect();
			clickX = rect.left + rect.width / 2;
		});
		cy.cGet(paragraph).last().rightclick();
		helper.getContextMenuItem('Cut').should('be.visible');

		cy.cGet('#canvas-context-menupos').should(function (anchor) {
			var left = anchor[0].getBoundingClientRect().left;
			expect(left).to.be.closeTo(clickX, 2);
		});
	});

	// Bold applies to the notes text while the notes have the focus, and the Bold button shows
	// the attribute at the caret as the caret moves between a bold and a plain word.
	it('the Bold button follows the caret in the notes', function () {
		var editEngine = '#notespanel-container .ui-editengine';
		var paragraph = editEngine + ' .ui-editengine-paragraph';
		var boldButton = '#Home-container .unoBold';

		openNotesPane();
		cy.cGet(editEngine).click();
		cy.cGet(editEngine).type('Demo');
		cy.cGet(paragraph).should('have.text', 'Demo');

		cy.getFrameWindow().then(function (win) {
			win.app.map.sendUnoCommand('.uno:SelectAll');
			win.app.map.sendUnoCommand('.uno:Bold');
			helper.processToIdle(win);
		});
		cy.cGet(paragraph + ' span').should('have.css', 'font-weight', '700');
		cy.cGet(boldButton).should('have.class', 'selected');

		// The caret at the end of the bold word, Bold off again, then a plain word.
		cy.cGet(editEngine).type('{end}');
		cy.getFrameWindow().then(function (win) {
			win.app.map.sendUnoCommand('.uno:Bold');
			helper.processToIdle(win);
		});
		cy.cGet(editEngine).type(' plain');
		cy.cGet(paragraph).should('have.text', 'Demo plain');
		cy.cGet(boldButton).should('not.have.class', 'selected');

		cy.cGet(editEngine).type('{home}');
		cy.cGet(boldButton).should('have.class', 'selected');

		cy.cGet(editEngine).type('{end}');
		cy.cGet(boldButton).should('not.have.class', 'selected');
	});

	// After an undo in the notes, a text command still acts on the notes text.
	it('Bold acts on the notes after an undo', function () {
		var editEngine = '#notespanel-container .ui-editengine';
		var paragraph = editEngine + ' .ui-editengine-paragraph';

		openNotesPane();
		cy.cGet(editEngine).click();
		cy.cGet(editEngine).type('Demo');
		cy.cGet(paragraph).should('have.text', 'Demo');
		cy.getFrameWindow().then((win) => { helper.processToIdle(win); });

		desktopHelper.getNbItem('Undo').should('not.have.attr', 'disabled');
		desktopHelper.getNbIcon('Undo').click();
		cy.cGet(paragraph).should('have.text', '');
		cy.cGet(editEngine).should('have.focus');

		cy.cGet(editEngine).type('Again');
		cy.cGet(paragraph).should('have.text', 'Again');
		cy.getFrameWindow().then(function (win) {
			win.app.map.sendUnoCommand('.uno:SelectAll');
			win.app.map.sendUnoCommand('.uno:Bold');
			helper.processToIdle(win);
		});
		cy.cGet(paragraph + ' span').should('have.css', 'font-weight', '700');
	});

	// A pointer that passes over the slide leaves the notes in charge of the text commands.
	it('Bold acts on the notes after the mouse moved over the slide', function () {
		var editEngine = '#notespanel-container .ui-editengine';
		var paragraph = editEngine + ' .ui-editengine-paragraph';

		openNotesPane();
		cy.cGet(editEngine).click();
		cy.cGet(editEngine).type('Demo');
		cy.cGet(paragraph).should('have.text', 'Demo');

		cy.getFrameWindow().then(function (win) {
			win.app.map.sendUnoCommand('.uno:SelectAll');
			win.app.map.sendUnoCommand('.uno:Bold');
			helper.processToIdle(win);
		});
		cy.cGet(paragraph + ' span').should('have.css', 'font-weight', '700');

		cy.cGet('#document-container').then(function (items) {
			var rect = items[0].getBoundingClientRect();
			cy.cGet('body').trigger('mousemove', (rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2);
		});
		cy.getFrameWindow().then((win) => { helper.processToIdle(win); });

		cy.getFrameWindow().then(function (win) {
			win.app.map.sendUnoCommand('.uno:Bold');
			helper.processToIdle(win);
		});
		cy.cGet(paragraph + ' span').should('have.css', 'font-weight', '400');
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
	// The notes pane takes no selection of its own, so a shape selected on the
	// slide keeps its frame while the notes are typed.
	it('keeps the selection of a shape on the slide while the notes are typed', function () {
		// Insert a shape, which arrives selected.
		cy.getFrameWindow().then(function (win) {
			win.app.map.sendUnoCommand('.uno:BasicShapes.rectangle');
		});
		cy.cGet('#test-div-shapeHandlesSection').should('exist');
		cy.getFrameWindow().then((win) => { helper.processToIdle(win); });

		openNotesPane();

		// The client drops the handles section on an empty selection, so each
		// removal of its test div is one flicker.
		cy.getFrameWindow().then(function (win) {
			win._testHandlesRemoved = 0;

			var observer = new MutationObserver(function (mutations) {
				mutations.forEach(function (mutation) {
					mutation.removedNodes.forEach(function (node) {
						if (node.id === 'test-div-shapeHandlesSection')
							win._testHandlesRemoved++;
					});
				});
			});

			observer.observe(win.document.body, { childList: true });
			win._testHandlesObserver = observer;
		});

		var editEngine = '#notespanel-container .ui-editengine';
		cy.cGet(editEngine).click();
		cy.cGet(editEngine).type('Remember the demo');
		cy.cGet(editEngine + ' .ui-editengine-paragraph').should(
			'have.text',
			'Remember the demo'
		);

		cy.getFrameWindow().then((win) => { helper.processToIdle(win); });

		// The shape stays selected, and its frame was never taken away.
		cy.cGet('#test-div-shapeHandlesSection').should('exist');

		cy.getFrameWindow().then(function (win) {
			expect(win._testHandlesRemoved).to.equal(0);
			win._testHandlesObserver.disconnect();
		});
	});

	// Each time the context menu of the notes is closed with Escape, the notes are ready for typing
	// again, and the typed text replaces the selection the menu was opened on.
	it('typing goes on in the notes after the context menu is closed', function () {
		var editEngine = '#notespanel-container .ui-editengine';
		var paragraph = editEngine + ' .ui-editengine-paragraph';

		openNotesPane();
		cy.cGet(editEngine).click();
		cy.cGet(editEngine).type('Remember the demo');
		cy.cGet(paragraph).should('have.text', 'Remember the demo');

		cy.cGet(editEngine).type('{shift}{home}');
		cy.getFrameWindow().then((win) => { helper.processToIdle(win); });

		// A real right click on the selection keeps it, and real key presses go to whatever has
		// the focus. The second menu is opened after the first one was closed with Escape.
		[1, 2].forEach(function () {
			cy.cGet(paragraph).realClick({ button: 'right', x: 30, y: 10 });
			helper.getContextMenuItem('Paste').should('be.visible');

			cy.realPress('Escape');
			cy.cGet('#jsd-context-menu-dropdown-overlay').should('not.exist');
			cy.cGet(editEngine).should('have.focus');
		});

		cy.realType('show');
		cy.cGet(paragraph).should('have.text', 'show');
	});

	// The context menu of the notes offers Cut and Copy only when some text is selected.
	it('the context menu offers Cut and Copy only for selected text', function () {
		var editEngine = '#notespanel-container .ui-editengine';
		var paragraph = editEngine + ' .ui-editengine-paragraph';

		openNotesPane();
		cy.cGet(editEngine).click();
		cy.cGet(editEngine).type('Remember the demo');
		cy.cGet(paragraph).should('have.text', 'Remember the demo');
		cy.getFrameWindow().then((win) => { helper.processToIdle(win); });

		// The right click places the caret in the text and selects nothing.
		cy.cGet(paragraph).realClick({ button: 'right', x: 30, y: 10 });
		helper.getContextMenuItem('Paste').should('be.visible');
		helper.getContextMenuItemList().should('not.contain.text', 'Cut');
		helper.getContextMenuItemList().should('not.contain.text', 'Copy');

		cy.realPress('Escape');
		cy.cGet('#jsd-context-menu-dropdown-overlay').should('not.exist');
		cy.cGet(editEngine).should('have.focus');

		// A right click on selected text keeps the selection. The whole line is selected, so the
		// right click lands well inside the selection, away from its edge.
		cy.cGet(editEngine).type('{end}{shift}{home}');
		cy.getFrameWindow().then((win) => { helper.processToIdle(win); });
		cy.cGet(paragraph).realClick({ button: 'right', x: 30, y: 10 });
		helper.getContextMenuItem('Cut').should('be.visible');
		helper.getContextMenuItem('Copy').should('be.visible');
	});
});
