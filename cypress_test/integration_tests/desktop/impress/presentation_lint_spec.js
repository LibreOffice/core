/* -*- js-indent-level: 8 -*- */
/* global describe it cy require expect beforeEach */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');
var desktopHelper = require('../../common/desktop_helper');

var deck = '#cleanup-deck';

// The panel keeps one group per kind of finding. A group with nothing to report
// is taken out of the panel.
var groupIds = {
	images: 'cleanup-group-image',
	hidden: 'cleanup-group-hidden-slide',
	masters: 'cleanup-group-unused-master',
	notes: 'cleanup-group-notes',
	ole: 'cleanup-group-embedded-object',
};

function group(name) {
	return deck + ' #' + groupIds[name];
}

// A group folds shut under a heading that says its name and how many rows it holds.
function groupTitle(name) {
	return group(name) + ' .ui-expander-label';
}

// Every finding gets a row of its own inside its group.
function rowsOf(name) {
	return group(name) + ' .cleanup-row';
}

// The line that names a finding. It is a link when the view can be moved to what it names and
// plain text when it cannot, so the text of a row is read through the first of these and a row
// is pressed through the second.
var rowText = '.cleanup-row-text';
var rowLink = 'button.cleanup-row-text';

// A push button is drawn as a wrapper around the button that takes the press, and it is the
// button that carries the label, the disabled state and the keyboard.
var rowFix = '[id^="cleanup-row-"][id$="-fix-button"]';
var scanButton = deck + ' #cleanup-scan-button';
var fixAllButton = deck + ' #cleanup-fix-all-button';

// Whether the panel is showing the widget named, asked of the element the panel holds now.
function showing(panel, selector) {
	const element = panel.querySelector(selector);
	return !!element && element.checkVisibility();
}

function openCleanupDeck(win) {
	cy.then(function() {
		win.app.dispatcher.dispatch('cleanupdeck');
	});
	helper.processToIdle(win);

	cy.cGet(deck + ' #cleanup-options').should('be.visible');
}

function closeCleanupDeck(win) {
	cy.then(function() {
		win.app.dispatcher.dispatch('cleanupdeck');
	});
	helper.processToIdle(win);

	cy.cGet(deck + ':visible').should('not.exist');
}

// The panel of a deck that has just opened holds no list: it says what the scan
// button is for and there is nothing for the band that adds a list up to say.
function assertPanelIsEmpty() {
	cy.cGet(deck + ' #cleanup-message').should('be.visible')
		.and('contain.text', 'press Scan');
	cy.cGet(deck + ' .cleanup-row:visible').should('not.exist');
	cy.cGet(deck + ' #cleanup-summary:visible').should('not.exist');
}

// Every size the panel writes is one of three forms: whole bytes under a
// kilobyte, whole kilobytes under a megabyte, and megabytes to one decimal
// place.
const byteSize = '(?:\\d+ bytes?|\\d+ KB|\\d+\\.\\d MB)';
const savingText = new RegExp('^saves ' + byteSize + '$');
const totalText = new RegExp('^(?:Estimated saving: ' + byteSize
	+ '|Saved ' + byteSize + '|\\d+ things? to clean up)$');

// Press a control that sets a run going, and see that a run did follow. The status line stands
// in the place of the scan button for exactly as long as the run lasts, and a run over a short
// list can begin and end inside one turn of the client, sooner than a frame is drawn. The panel
// hands every region it draws again to the layouting service, which puts them on the screen over
// the frames that follow, so one watch can be handed several states of the panel at once. What
// the panel showed as the run began is therefore built up from the drawings in the order they
// arrive, and the reading is taken at the drawing that puts the status line up.
function pressToStartRun(getButton) {
	const run = { started: false, atStart: null };

	cy.cGet(deck).then(function($panel) {
		const panel = $panel[0];
		const view = panel.ownerDocument.defaultView;

		// Whether a widget of one drawing is in sight: neither it nor anything around it
		// inside that drawing carries the class that takes a widget off the screen. Null
		// when the drawing does not carry the widget at all. A drawing that a later one
		// replaces hands its id over to that rebuild, so a widget that stands as a region
		// of its own is looked for by the class it carries as well as by its id.
		function drawnShowing(drawing, id, cssClass) {
			if (drawing.nodeType !== view.Node.ELEMENT_NODE) return null;

			let widget = drawing.id === id ? drawing : drawing.querySelector('#' + id);
			if (!widget && cssClass && drawing.classList.contains(cssClass))
				widget = drawing;
			if (!widget) return null;

			for (let node = widget; node; node = node.parentElement) {
				if (node.classList.contains('hidden')) return false;
				if (node === drawing) break;
			}
			return true;
		}

		// A widget that no drawing has carried yet stands as it stood when the press
		// was made.
		const showingNow = {
			statusLine: false,
			progressBar: showing(panel, '#cleanup-progress'),
			scanButton: showing(panel, '#cleanup-scan'),
			summaryBand: showing(panel, '#cleanup-summary'),
		};
		let bandRead = false;

		const observer = new view.MutationObserver(function(records) {
			const drawings = [];
			for (const record of records) drawings.push(...record.addedNodes);

			for (const drawing of drawings) {
				// The panel draws the band again in the same breath as the status
				// line, just after it, so the band of a run is the first one drawn
				// once the line is up. A later one belongs to the end of the run.
				if (!bandRead) {
					const band = drawnShowing(
						drawing, 'cleanup-summary', 'cleanup-summary');
					if (band !== null) {
						showingNow.summaryBand = band;
						bandRead = run.started;
					}
				}

				if (run.started) continue;

				const bar = drawnShowing(drawing, 'cleanup-progress');
				if (bar !== null) showingNow.progressBar = bar;

				const button = drawnShowing(drawing, 'cleanup-scan');
				if (button !== null) showingNow.scanButton = button;

				if (drawnShowing(drawing, 'cleanup-status') !== true) continue;

				showingNow.statusLine = true;
				run.started = true;
				run.atStart = showingNow;
			}
		});

		observer.observe(panel, { childList: true, subtree: true });

		run.stopWatching = function() {
			observer.disconnect();
		};
	});

	getButton().should('be.visible').and('not.be.disabled').click();

	cy.then(function() {
		run.stopWatching();
		expect(run.started, 'the run the press set going').to.be.true;
	});

	return run;
}

// Wait out the run: the status line goes back out of sight and the button that
// starts a run stands in its place again, ready for the next press.
function waitForRunToEnd() {
	cy.cGet(deck + ' #cleanup-status').should('not.be.visible');
	cy.cGet(scanButton).should('be.visible').and('not.be.disabled');
}

function scanPresentation(win) {
	pressToStartRun(function() {
		return cy.cGet(scanButton);
	});

	// Waiting for the client to go quiet lets the answers to the press arrive
	// before the wait below reads what the panel now says.
	helper.processToIdle(win);

	waitForRunToEnd();

	// The button asks to be pressed again once the scan is over, under the label
	// it carried all along.
	cy.cGet(scanButton).should('have.text', 'Scan');
}

// Every row that offers a saving says what it comes to as a size.
function assertRowSavings(name, count) {
	cy.cGet(rowsOf(name) + ':visible .cleanup-row-saving')
		.should('have.length', count)
		.and('be.visible')
		.each(function($saving) {
			expect($saving.text(), 'the saving a row offers').to.match(savingText);
		});
}

// The band over the list says what the whole list comes to, as a size or as a
// count of the things there are to do.
function assertTotalFigure() {
	cy.cGet(deck + ' #cleanup-total').should('be.visible')
		.invoke('text').should('match', totalText);
}

function assertGroupListed(name) {
	cy.cGet(group(name)).should('be.visible');
}

// A group the scan found nothing for is not on screen, and the panel is free to
// leave it out of the deck altogether.
function assertGroupNotListed(name) {
	cy.cGet(group(name) + ':visible').should('not.exist');
}

// Carry out the cleanup the first row of a group offers. The kit works out what the
// cleanup needs in idle turns of its own, and a cleanup that reaches beyond the
// finding it is about brings a fresh scan with it, so the wait runs to the end of
// everything the press set going.
function dealWithRowOf(name, win) {
	pressToStartRun(function() {
		return cy.cGet(rowsOf(name) + ':visible ' + rowFix).first();
	});
	helper.processToIdle(win);

	waitForRunToEnd();
}

function undo(win) {
	cy.then(function() {
		win.app.map.sendUnoCommand('.uno:Undo');
	});
	helper.processToIdle(win);
}

function redo(win) {
	cy.then(function() {
		win.app.map.sendUnoCommand('.uno:Redo');
	});
	helper.processToIdle(win);
}

// Load the deck the tests read and open the cleanup panel over it. The window is
// kept on the test so that every test reaches the same frame.
function loadPresentationAndOpenDeck(test) {
	// A deck with something to say in every group fills more than the height the
	// sidebar gets in the default test window, and what is past the bottom of
	// the sidebar counts as off screen.
	cy.viewport(1920, 1080);

	helper.setupAndLoadDocument('impress/presentation_lint.fodp');

	cy.getFrameWindow().then(function(win) {
		test.win = win;
		helper.processToIdle(win);
		openCleanupDeck(win);
	});
}

describe(['tagdesktop'], 'Presentation cleanup suggestions', function() {

	beforeEach(function() {
		loadPresentationAndOpenDeck(this);
	});

	it('lists what the presentation has to clean up', function() {
		const win = this.win;

		// Until a scan has run the panel says what the scan button is for, and
		// there is nothing for the band that adds the list up to say.
		cy.cGet(deck + ' #cleanup-message').should('be.visible')
			.and('contain.text', 'press Scan');
		cy.cGet(deck + ' #cleanup-summary:visible').should('not.exist');

		scanPresentation(win);

		// The list takes the place of that invitation.
		cy.cGet(deck + ' #cleanup-message:visible').should('not.exist');

		assertGroupListed('images');
		assertGroupListed('hidden');
		assertGroupListed('masters');

		// Neither the speaker notes group nor the embedded objects group is
		// listed while the deck is not being prepared to hand out.
		assertGroupNotListed('notes');
		assertGroupNotListed('ole');

		// A heading says how many rows its group holds, and the one over the
		// image rows carries the resolution the scan measured against as well,
		// so no row has to repeat it.
		cy.cGet(groupTitle('images')).should('have.text', 'Images over 150 DPI (2)');
		cy.cGet(groupTitle('hidden')).should('have.text', 'Hidden slides (1)');
		cy.cGet(groupTitle('masters')).should('have.text', 'Unused master slides (1)');

		// A row names where the image sits, and which image on that slide it is
		// when the slide has more than one, and nothing else.
		cy.cGet(rowsOf('images') + ':visible ' + rowText).first().invoke('text').then((text) => {
			expect(text.trim(), 'text of the image row').to.match(/^Slide \d+(, image \d+)?$/);
		});

		// How many bytes the image takes and what it is stored at read as a
		// detail under that line.
		cy.cGet(rowsOf('images') + ':visible .cleanup-row-detail').first()
			.should('be.visible')
			.invoke('text').then((text) => {
				expect(text.trim(), 'detail of the image row').to.match(/^\d+ KB at \d+ DPI$/);
			});

		// The image row says how much smaller dealing with it leaves the
		// document, and the panel adds the rows up.
		assertRowSavings('images', 2);
		assertTotalFigure();

		// At least one row offers a cleanup, so the whole list can be dealt
		// with in one go.
		cy.cGet(deck + ' #cleanup-fix-all').should('be.visible');

		// The button that cancels a run is part of the panel, and waits out of
		// sight for as long as nothing is running.
		cy.cGet(deck + ' #cleanup-stop').should('exist').and('not.be.visible');

		// The figure and the button that deals with the whole list stand over
		// the groups, next to the scan button, so a short window does not push
		// them out of sight below the list.
		cy.cGet(deck + ' #cleanup-summary').should('be.visible');
		cy.cGet(deck + ' #cleanup-summary #cleanup-total').should('be.visible');
		cy.cGet(deck + ' #cleanup-summary #cleanup-fix-all').should('be.visible');
		cy.cGet(deck).then(($deck) => {
			const ids = Array.from($deck[0].querySelectorAll('[id]'))
				.map((element) => element.id);
			expect(ids, 'the summary band').to.include('cleanup-summary');
			expect(ids.indexOf('cleanup-summary'), 'place of the summary band')
				.to.be.lessThan(ids.indexOf(groupIds.images));
		});
	});

	it('an image with too little to gain is off the list by the end of the run', function() {
		const win = this.win;

		scanPresentation(win);

		// The image on slide 3 is stored at about the size it is drawn at. The
		// scan lists it and the measurement finds too little in it to be worth a
		// row, so the row is gone from the panel by the time the run is over,
		// and the rows the reader is left with are the ones worth acting on.
		cy.cGet(rowsOf('images') + ':visible').should('have.length', 2)
			.invoke('text').then((text) => {
				expect(text, 'the image rows on screen').to.not.contain('Slide 3');
			});

		// Every row still on screen says how much dealing with it would free up.
		assertRowSavings('images', 2);
	});

	it('each image of a slide is named with its own number once the run has ended', function() {
		const win = this.win;

		scanPresentation(win);

		// Slide 1 draws two different images, so each of the two rows says which
		// of them it is. The numbers are worked out again over whatever the
		// measurement left on the list, so it is the text at the end of the run
		// that has to read right.
		cy.cGet(rowsOf('images') + ':visible ' + rowText).should('have.length', 2)
			.then(($links) => {
				const texts = $links.toArray().map((link) => link.innerText.trim());
				expect(texts[0], 'first image row of the slide').to.equal('Slide 1, image 1');
				expect(texts[1], 'second image row of the slide').to.equal('Slide 1, image 2');
			});
	});

	it('removing a hidden slide can be undone', function() {
		const win = this.win;

		scanPresentation(win);
		impressHelper.assertSlidePreviewCountAfterIdle(win, 3);

		dealWithRowOf('hidden', win);

		// The slide is gone and the panel has nothing left to say about hidden
		// slides, without being asked to look again.
		impressHelper.assertSlidePreviewCountAfterIdle(win, 2);
		assertGroupNotListed('hidden');

		undo(win);

		impressHelper.assertSlidePreviewCountAfterIdle(win, 3);
	});

	it('compressing an image can be undone', function() {
		const win = this.win;

		scanPresentation(win);

		cy.cGet(rowsOf('images') + ':visible ' + rowText).first().invoke('text').then((text) => {
			const imageRow = text.trim();
			expect(imageRow, 'text of the image row').to.not.be.empty;

			dealWithRowOf('images', win);

			// The image now holds the pixels the largest drawing of it needs,
			// so its row is gone. The other image of the slide is untouched and
			// keeps its row and its figure, and it is the only image left on
			// that slide, so it reads without a number.
			cy.cGet(rowsOf('images') + ':visible ' + rowText).should('have.length', 1)
				.invoke('text').then((remaining) => {
					expect(remaining.trim(), 'the image row left on screen').to.not.equal(imageRow);
					expect(remaining.trim(), 'the image row left on screen')
						.to.match(/^Slide \d+$/);
				});

			assertRowSavings('images', 1);

			undo(win);

			// The image is back the size it was, and the panel says so without
			// being asked to look again: the very same row returns along with
			// its figure, next to the row that was left alone.
			waitForRunToEnd();
			cy.cGet(rowsOf('images') + ':visible ' + rowText).should('have.length', 2)
				.invoke('text').then((text) => {
					expect(text, 'the image rows on screen').to.contain(imageRow);
				});
			assertRowSavings('images', 2);

			// Doing the cleanup over again takes that row off the list once
			// more, again without being asked.
			redo(win);

			waitForRunToEnd();
			cy.cGet(rowsOf('images') + ':visible ' + rowText).should('have.length', 1)
				.invoke('text').then((text) => {
					expect(text, 'the image rows on screen').to.not.contain(imageRow);
				});
		});
	});

	it('dealing with everything at once can be taken back in one step', function() {
		const win = this.win;
		let undoStepsBefore = 0;

		cy.cGet(deck + ' #cleanup-sharing-input').check();
		scanPresentation(win);

		assertGroupListed('hidden');
		assertGroupListed('masters');
		assertGroupListed('notes');

		desktopHelper.getUndoCount(win).then((count) => {
			undoStepsBefore = count;
		});

		cy.cGet(fixAllButton).should('be.visible').and('not.be.disabled').click();
		helper.processToIdle(win);

		waitForRunToEnd();

		assertGroupNotListed('hidden');
		assertGroupNotListed('masters');
		assertGroupNotListed('notes');

		// Having dealt with the whole list, the panel reports what it freed up
		// rather than what is still there to gain.
		cy.cGet(deck + ' #cleanup-total').should('be.visible').and('contain.text', 'Saved');

		desktopHelper.getUndoCount(win).then((count) => {
			expect(count - undoStepsBefore, 'undo steps the whole batch added').to.equal(1);
		});

		undo(win);
		scanPresentation(win);

		// One undo brought the whole presentation back, so every group has
		// something to report again.
		impressHelper.assertSlidePreviewCountAfterIdle(win, 3);
		assertGroupListed('hidden');
		assertGroupListed('masters');
		assertGroupListed('notes');
	});

	it('a run stands in the place of the scan button and shows how far it has come', function() {
		const win = this.win;

		// The bar the run fills up is part of the panel and waits inside the
		// status line for as long as nothing is running.
		cy.cGet(deck + ' #cleanup-progress').should('exist').and('not.be.visible');

		const run = pressToStartRun(function() {
			return cy.cGet(scanButton);
		});

		// The status line takes the place of the button that started the run and
		// carries the bar, and the band that adds the list up waits for the run
		// to end rather than reporting a figure that is still moving.
		cy.then(function() {
			expect(run.atStart.statusLine, 'the status line during the run').to.be.true;
			expect(run.atStart.progressBar, 'the progress bar during the run').to.be.true;
			expect(run.atStart.scanButton, 'the scan button during the run').to.be.false;
			expect(run.atStart.summaryBand, 'the summary band during the run').to.be.false;
		});

		helper.processToIdle(win);
		waitForRunToEnd();

		// The run is over, so the button is back in its place, the bar is out of
		// the panel and the band says what the list comes to.
		cy.cGet(deck + ' #cleanup-progress').should('not.be.visible');
		cy.cGet(deck + ' #cleanup-summary').should('be.visible');
		assertTotalFigure();
	});

	it('asking for images to be left alone reports none of them', function() {
		const win = this.win;

		cy.cGet(deck + ' #cleanup-resolution-input').select('Leave images alone');
		helper.processToIdle(win);

		scanPresentation(win);

		assertGroupNotListed('images');
		assertGroupListed('hidden');
	});

	it('a row link moves the view to the slide it names', function() {
		const win = this.win;

		scanPresentation(win);

		// The view is on the first slide of the deck.
		cy.cGet('#preview-img-part-0').should('have.class', 'preview-img-currentpart');

		// The hidden slide is the second one, and pressing its row takes the
		// reader there to look at it.
		cy.cGet(rowsOf('hidden') + ':visible ' + rowLink).first()
			.should('contain.text', 'Slide 2').click();

		cy.cGet('#preview-img-part-1').should('have.class', 'preview-img-currentpart');
	});

	it('a row link moves the view while a run is going', function() {
		const win = this.win;

		scanPresentation(win);

		cy.cGet('#preview-img-part-0').should('have.class', 'preview-img-currentpart');

		// A cleanup of one image stays inside that image, so the rest of the
		// list is still there to be read and pressed while the run lasts.
		pressToStartRun(function() {
			return cy.cGet(rowsOf('images') + ':visible ' + rowFix).first();
		});

		cy.cGet(rowsOf('hidden') + ':visible ' + rowLink).first().click();

		cy.cGet('#preview-img-part-1').should('have.class', 'preview-img-currentpart');

		helper.processToIdle(win);
		waitForRunToEnd();

		// The cleanup still finished: the image it dealt with has left the list.
		cy.cGet(rowsOf('images') + ':visible').should('have.length', 1);
	});

	it('the keyboard reaches the scan button, a row and the Fix button of a row', function() {
		const win = this.win;

		scanPresentation(win);

		// Tab walks on from the last of the options and every widget it lands
		// on is noted, so what is asserted is what the reader reaches rather
		// than the widgets that happen to lie in between.
		const visited = [];

		function describeFocused() {
			const element = win.document.activeElement;
			if (!element) return 'nothing';
			if (element.id) return '#' + element.id;
			return '.' + String(element.className).split(' ').join('.');
		}

		cy.cGet(deck + ' #cleanup-sharing-input').focus();
		helper.assertFocus('id', 'cleanup-sharing-input');

		for (let step = 0; step < 8; step++) {
			cy.realPress('Tab');
			cy.then(function() {
				visited.push(describeFocused());
			});
		}

		cy.then(function() {
			expect(visited, 'the button that starts a run')
				.to.include('#cleanup-scan-button');
			expect(visited.some(function(widget) {
				return widget.indexOf('-goto') !== -1;
			}), 'the link of a row, in ' + visited.join(' ')).to.be.true;
			expect(visited.some(function(widget) {
				return widget.indexOf('-fix-button') !== -1;
			}), 'the Fix button of a row, in ' + visited.join(' ')).to.be.true;
		});
	});

	it('pressing Enter on a Fix button deals with that row', function() {
		const win = this.win;

		scanPresentation(win);

		cy.cGet(rowsOf('images') + ':visible').should('have.length', 2);

		cy.cGet(rowsOf('images') + ':visible ' + rowText).first().invoke('text')
			.then(function(text) {
				const pressedRow = text.trim();

				cy.cGet(rowsOf('images') + ':visible ' + rowFix).first().focus();
				cy.realPress('Enter');

				helper.processToIdle(win);
				waitForRunToEnd();

				// The row the press was on has left the list, and the row that
				// was left alone is still there.
				cy.cGet(rowsOf('images') + ':visible ' + rowText)
					.should('have.length', 1)
					.invoke('text').then(function(remaining) {
						expect(remaining.trim(), 'the image row left on screen')
							.to.not.equal(pressedRow);
					});
			});
	});

	it('the panel tells a screen reader what the run is doing and how the list is built', function() {
		const win = this.win;

		// The line that says what the run is doing and the line that stands in
		// for an empty list are both read out as they change. The status line is
		// a live region as a whole, so the text and the Stop button beside it are
		// read out together.
		cy.cGet(deck + ' #cleanup-status-line').should('have.attr', 'aria-live', 'polite');
		cy.cGet(deck + ' #cleanup-message').should('have.attr', 'aria-live', 'polite');

		scanPresentation(win);

		// So is the band the figure the whole list comes to sits in.
		cy.cGet(deck + ' #cleanup-summary').should('have.attr', 'aria-live', 'polite');

		// Each group is headed by a heading, so a reader can move from group to
		// group by heading.
		cy.cGet(group('images') + ' .ui-expander-heading').should(function($heading) {
			expect($heading[0].tagName, 'the tag of a group heading').to.equal('H2');
		});

		// The Fix button of a row says which row it deals with, since its own
		// label says only Fix.
		cy.cGet(rowsOf('images') + ':visible ' + rowText).first().invoke('text')
			.then(function(text) {
				cy.cGet(rowsOf('images') + ':visible ' + rowFix).first()
					.should('have.attr', 'aria-label', 'Fix ' + text.trim());
			});
	});

	it('opening the deck again starts from an empty panel', function() {
		const win = this.win;

		scanPresentation(win);
		assertGroupListed('images');

		closeCleanupDeck(win);
		openCleanupDeck(win);

		// Closing the deck told the kit to let the run go, so the panel comes
		// back asking for a scan rather than showing what the last one found.
		assertPanelIsEmpty();

		// A scan of its own fills it again.
		scanPresentation(win);
		assertGroupListed('images');
		assertRowSavings('images', 2);
	});

	it('another deck taking the dock leaves nothing behind in the panel', function() {
		const win = this.win;

		scanPresentation(win);
		assertGroupListed('images');

		cy.then(function() {
			win.app.map.sendUnoCommand('.uno:SidebarDeck.PropertyDeck');
		});
		helper.processToIdle(win);

		cy.cGet(deck + ':visible').should('not.exist');
		cy.cGet('#sidebar-dock-wrapper').should('be.visible');

		openCleanupDeck(win);

		// The deck went away when the other one took the dock, so the run went
		// with it and the panel starts over.
		assertPanelIsEmpty();
	});

	it('taking a cleanup back puts the estimate back in the place of what was saved', function() {
		const win = this.win;

		scanPresentation(win);

		cy.cGet(deck + ' #cleanup-total').invoke('text').then(function(text) {
			const beforeCleanup = text.trim();
			expect(beforeCleanup, 'the figure before the cleanup').to.match(totalText);

			pressToStartRun(function() {
				return cy.cGet(fixAllButton);
			});
			helper.processToIdle(win);
			waitForRunToEnd();

			// Having dealt with the whole list, the panel reports what it freed
			// up rather than what is still there to gain.
			cy.cGet(deck + ' #cleanup-total').invoke('text')
				.should('match', new RegExp('^Saved ' + byteSize + '$'));

			undo(win);

			// The document is back as it was, so the figure is the one the kit
			// works out over the list it gathers again, and nothing is said
			// about a saving any more.
			cy.cGet(deck + ' #cleanup-total').should('be.visible')
				.invoke('text').should('equal', beforeCleanup);
		});
	});
});

describe(['tagdesktop'], 'Presentation cleanup in the tabbed view', function() {

	// The tool item dispatches an action of the client rather than a real UNO
	// command, so it keeps the id the tab gives it.
	const button = '#review-cleanup';

	beforeEach(function() {
		cy.viewport(1920, 1080);

		helper.setupAndLoadDocument('impress/presentation_lint.fodp');
		desktopHelper.switchUIToNotebookbar();
	});

	it('the Review tab opens and closes the cleanup deck', function() {
		cy.cGet('#Review-tab-label').click();

		cy.cGet(button).should('be.visible').click();
		cy.cGet(deck + ' #cleanup-options').should('be.visible');

		// The button reads as pressed for as long as the deck is up.
		cy.cGet(button).should('have.class', 'selected');

		cy.cGet(button).click();
		cy.cGet(deck + ':visible').should('not.exist');
	});
});

// The panel hands the keyboard on when the row it was on leaves the list.
describe(['tagdesktop'], 'Presentation cleanup and the keyboard', function() {

	beforeEach(function() {
		loadPresentationAndOpenDeck(this);
	});

	it('the keyboard moves to the Fix button of the next row once a row is dealt with', function() {
		const win = this.win;

		scanPresentation(win);

		cy.cGet(rowsOf('images') + ':visible').should('have.length', 2);

		cy.cGet(rowsOf('images') + ':visible ' + rowFix).first().focus();
		cy.realPress('Enter');

		helper.processToIdle(win);
		waitForRunToEnd();

		// The row that was dealt with took the keyboard with it, so the keyboard
		// stands on the Fix button of the row that followed it and the next
		// press acts on that row.
		cy.cGet(rowsOf('images') + ':visible ' + rowFix).should(function($fix) {
			expect($fix.length, 'Fix buttons left on screen').to.equal(1);
			expect($fix[0].ownerDocument.activeElement,
				'the widget the keyboard is on').to.equal($fix[0]);
		});
	});
});
