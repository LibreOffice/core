/* -*- js-indent-level: 8 -*- */
/* global describe it cy require expect beforeEach */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');
var desktopHelper = require('../../common/desktop_helper');

var deck = '#cleanup-deck';

// Every check is listed at all times. A check holding findings stands as the
// disclosure over them.
var groupIds = {
	images: 'cleanup-group-image',
	cropped: 'cleanup-group-cropped-image',
	hidden: 'cleanup-group-hidden-slide',
	masters: 'cleanup-group-unused-master',
	notes: 'cleanup-group-notes',
	ole: 'cleanup-group-embedded-object',
};

// A check holding no findings stands as a flat row that says what it has to
// report.
var checkIds = {
	images: 'cleanup-check-image',
	cropped: 'cleanup-check-cropped-image',
	hidden: 'cleanup-check-hidden-slide',
	masters: 'cleanup-check-unused-master',
	notes: 'cleanup-check-notes',
	ole: 'cleanup-check-embedded-object',
};

function group(name) {
	return deck + ' #' + groupIds[name];
}

function check(name) {
	return deck + ' #' + checkIds[name];
}

// Every finding gets a row of its own inside its check.
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

	cy.cGet(deck + ' #cleanup-options-heading-button').should('be.visible');
}

// The values a scan is run with stand behind one row in the footer. The row
// opens with the deck, so a test that reads or sets a value makes sure the row
// is open rather than pressing it.
function openOptions() {
	cy.cGet(deck + ' #cleanup-options-heading-button').should('not.be.disabled')
		.then(function($heading) {
			if ($heading.attr('aria-expanded') !== 'true')
				cy.wrap($heading).click();
		});
	cy.cGet(deck + ' #cleanup-options-body').should('be.visible');
}

// The row that holds the values a scan is run with folds them away.
function closeOptions() {
	cy.cGet(deck + ' #cleanup-options-heading-button')
		.should('have.attr', 'aria-expanded', 'true').click();
	cy.cGet(deck + ' #cleanup-options-heading-button')
		.should('have.attr', 'aria-expanded', 'false');
	cy.cGet(deck + ' #cleanup-options-body').should('not.be.visible');
}

function closeCleanupDeck(win) {
	cy.then(function() {
		win.app.dispatcher.dispatch('cleanupdeck');
	});
	helper.processToIdle(win);

	cy.cGet(deck + ':visible').should('not.exist');
}

// The panel of a deck that has just opened holds no list: it says what the deck
// is for, every check reports that no scan has covered it yet, there is
// nothing for the band that adds a list up to say, and the values a scan is
// run with are in view.
function assertPanelIsEmpty() {
	cy.cGet(deck + ' #cleanup-message').should('be.visible')
		.and('contain.text', 'Find what makes this presentation large');
	cy.cGet(deck + ' .cleanup-check-row:visible .cleanup-check-state')
		.should('have.length', 6)
		.each(function($state) {
			expect($state.text(), 'what a check reports').to.equal('Not checked');
		});
	cy.cGet(deck + ' .cleanup-row:visible').should('not.exist');
	cy.cGet(deck + ' #cleanup-summary:visible').should('not.exist');
	cy.cGet(deck + ' #cleanup-options-heading-button')
		.should('have.attr', 'aria-expanded', 'true');
	cy.cGet(deck + ' #cleanup-options-body').should('be.visible');
}

// Every size the panel writes is one of three forms: whole bytes under a
// kilobyte, whole kilobytes under a megabyte, and megabytes to one decimal
// place.
const byteSize = '(?:\\d+ bytes?|\\d+ KB|\\d+\\.\\d MB)';
const savingText = new RegExp('^saves ' + byteSize + '$');
const totalText = new RegExp('^(?:' + byteSize + ' \\(\\d+%\\) can be cleaned up'
	+ '|' + byteSize + ' can be cleaned up'
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

	// The drawing that puts the status line up comes some frames after the press.
	cy.wrap(run).should(function() {
		expect(run.started, 'the run the press set going').to.be.true;
	});
	cy.then(function() {
		run.stopWatching();
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
	cy.cGet(deck + ' #cleanup-scan').should('have.text', 'Scan presentation');
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

// The footer stands at the foot of the dock, so its bottom edge is the bottom
// edge of the dock itself.
function assertFooterSitsAtBottomOfDock() {
	cy.cGet('#sidebar-dock-wrapper').then(function($dock) {
		const dockBottom = $dock[0].getBoundingClientRect().bottom;

		cy.cGet(deck + ' .cleanup-footer').should(function($footer) {
			const footerBottom = $footer[0].getBoundingClientRect().bottom;
			expect(Math.abs(footerBottom - dockBottom),
				'how far the footer is off the bottom of the dock').to.be.lessThan(2);
		});
	});
}

// A check with findings stands as the disclosure that holds them, in the place
// of the flat row.
function assertGroupListed(name) {
	cy.cGet(group(name)).should('be.visible');
	cy.cGet(check(name) + ':visible').should('not.exist');
}

// A check with no findings keeps its place in the panel: the disclosure is off
// the screen and a flat row stands there saying what the check has to report.
function assertCheckReads(name, state) {
	cy.cGet(group(name) + ':visible').should('not.exist');
	cy.cGet(check(name)).should('be.visible')
		.find('.cleanup-check-state').should('have.text', state);
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

		// Until a scan has run the panel says what the deck is for, and there is
		// nothing for the band that adds the list up to say.
		cy.cGet(deck + ' #cleanup-message').should('be.visible')
			.and('contain.text', 'Find what makes this presentation large');
		cy.cGet(deck + ' #cleanup-summary:visible').should('not.exist');

		scanPresentation(win);

		// The list takes the place of that line.
		cy.cGet(deck + ' #cleanup-message:visible').should('not.exist');

		assertGroupListed('images');
		assertGroupListed('hidden');
		assertGroupListed('masters');

		// The speaker notes and the embedded objects are not looked at while
		// the deck is not being prepared to hand out, and both checks say so
		// rather than going off the screen.
		assertCheckReads('notes', 'Not checked');
		assertCheckReads('ole', 'Not checked');

		// A heading says what a check looks for and the count beside it says how
		// many findings it holds.
		cy.cGet(group('images') + ' .ui-expander-label')
			.should('have.text', 'High-resolution images');
		cy.cGet(group('images') + ' .ui-expander-secondary').should('have.text', '2');
		cy.cGet(group('hidden') + ' .ui-expander-label')
			.should('have.text', 'Hidden slides');
		cy.cGet(group('hidden') + ' .ui-expander-secondary').should('have.text', '1');
		cy.cGet(group('masters') + ' .ui-expander-label')
			.should('have.text', 'Unused master slides');
		cy.cGet(group('masters') + ' .ui-expander-secondary').should('have.text', '1');

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

		// The figure and the button that deals with the whole list stand in the
		// footer, which keeps to the foot of the deck while the list scrolls, so
		// a short window does not push them out of sight below the list. The
		// band leads the footer, then the row the options stand behind, then the
		// one action of the panel.
		cy.cGet(deck + ' #cleanup-summary').should('be.visible');
		cy.cGet(deck + ' #cleanup-summary #cleanup-total').should('be.visible');
		cy.cGet(deck + ' #cleanup-summary #cleanup-fix-all').should('be.visible');
		cy.cGet(deck + ' .cleanup-footer').then(($footer) => {
			const order = ['cleanup-summary', 'cleanup-options', 'cleanup-run'];
			const places = order.map((name) =>
				Array.from($footer[0].children)
					.findIndex((child) => child.classList.contains(name)));
			expect(places[0], 'the summary band').to.not.equal(-1);
			expect(places[1], 'the options row').to.not.equal(-1);
			expect(places[0], 'place of the summary band').to.be.lessThan(places[1]);
			expect(places[1], 'place of the options row').to.be.lessThan(places[2]);
		});
	});

	it('the footer stands at the foot of the dock however little the panel lists', function() {
		const win = this.win;

		// The panel of a deck that has just opened lists nothing, and the band
		// that adds a list up and the one action of the panel are at the foot of
		// the dock.
		assertFooterSitsAtBottomOfDock();

		scanPresentation(win);
		assertGroupListed('images');

		// Folding every check that holds findings leaves the panel short again,
		// and the footer has not moved.
		cy.cGet(deck + ' .cleanup-group:visible .ui-expander-btn')
			.then(function($headings) {
				$headings.each(function(index, heading) {
					cy.wrap(heading).click();
				});
			});
		cy.cGet(deck + ' .cleanup-group:visible .ui-expander-btn[aria-expanded="true"]')
			.should('not.exist');

		assertFooterSitsAtBottomOfDock();
	});

	it('the name of the images check says what counts as too much detail', function() {
		const win = this.win;

		// The check says what it looks for in a line the reader can rest on,
		// which carries the resolution the scan measures against, so the name
		// itself does not have to.
		const explanation
			= 'Images stored at more than 150 DPI, more detail than a slide can show';

		cy.cGet(check('images') + ' .cleanup-check-title')
			.should('have.text', 'High-resolution images')
			.and('have.attr', 'data-cooltip', explanation);

		scanPresentation(win);

		// The heading of the disclosure that stands over the findings says the
		// same thing.
		cy.cGet(group('images') + ' .ui-expander-label')
			.should('have.text', 'High-resolution images');
		cy.cGet(group('images') + ' .ui-expander-btn')
			.should('have.attr', 'data-cooltip', explanation);
		cy.cGet(group('images')).should('not.have.attr', 'data-cooltip');
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

	it('lists the picture that is drawn with part of it hidden', function() {
		const win = this.win;

		scanPresentation(win);

		assertGroupListed('cropped');

		// A heading says what the check looks for and the count beside it says
		// how many pictures the deck draws with part of them hidden.
		cy.cGet(group('cropped') + ' .ui-expander-label')
			.should('have.text', 'Cropped images');
		cy.cGet(group('cropped') + ' .ui-expander-secondary').should('have.text', '1');

		// The row names where the picture sits the same way an image row does.
		cy.cGet(rowsOf('cropped') + ':visible ' + rowText).first().invoke('text').then((text) => {
			expect(text.trim(), 'text of the cropped image row').to.match(/^Slide \d+(, image \d+)?$/);
		});

		// How many bytes the picture takes and how much of it never reaches
		// the slide read as a detail under that line.
		cy.cGet(rowsOf('cropped') + ':visible .cleanup-row-detail').first()
			.should('be.visible')
			.invoke('text').then((text) => {
				expect(text.trim(), 'detail of the cropped image row')
					.to.match(/^\d+(\.\d)? (KB|MB), \d+% cropped away$/);
			});

		// The row says how much smaller trimming the picture leaves the
		// document.
		assertRowSavings('cropped', 1);
	});

	it('removing a hidden slide can be undone', function() {
		const win = this.win;

		scanPresentation(win);
		impressHelper.assertSlidePreviewCountAfterIdle(win, 3);

		dealWithRowOf('hidden', win);

		// The slide is gone and the panel has nothing left to say about hidden
		// slides, without being asked to look again.
		impressHelper.assertSlidePreviewCountAfterIdle(win, 2);
		assertCheckReads('hidden', 'None');

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

	it('trimming a cropped image can be undone', function() {
		const win = this.win;

		scanPresentation(win);

		cy.cGet(rowsOf('cropped') + ':visible ' + rowText).first().invoke('text').then((text) => {
			const croppedRow = text.trim();
			expect(croppedRow, 'text of the cropped image row').to.not.be.empty;

			dealWithRowOf('cropped', win);

			// The picture now holds only the part the slide shows, so the
			// check is left with nothing to report and the disclosure goes
			// with its only row.
			assertCheckReads('cropped', 'None');

			undo(win);

			// The picture is back with the part that was hidden, and the panel
			// says so without being asked to look again: the very same row
			// returns along with its figure.
			waitForRunToEnd();
			assertGroupListed('cropped');
			cy.cGet(rowsOf('cropped') + ':visible ' + rowText).should('have.length', 1)
				.invoke('text').then((restored) => {
					expect(restored.trim(), 'the cropped image row on screen').to.equal(croppedRow);
				});
			assertRowSavings('cropped', 1);

			// Trimming it over again takes the row off the list once more,
			// again without being asked.
			redo(win);

			waitForRunToEnd();
			assertCheckReads('cropped', 'None');
		});
	});

	it('dealing with everything at once can be taken back in one step', function() {
		const win = this.win;
		let undoStepsBefore = 0;

		openOptions();
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

		assertCheckReads('hidden', 'None');
		assertCheckReads('masters', 'None');
		assertCheckReads('notes', 'None');

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

		openOptions();
		cy.cGet(deck + ' #cleanup-resolution-input').select('Leave images alone');
		helper.processToIdle(win);

		scanPresentation(win);

		// Cropped pictures are measured against the same resolution, so asking
		// for images to be left alone leaves them alone too, and both checks
		// say they were not looked at rather than that they found nothing.
		assertCheckReads('images', 'Not checked');
		assertCheckReads('cropped', 'Not checked');
		assertGroupListed('hidden');
	});

	it('a check a scan covered and found nothing in says so', function() {
		const win = this.win;

		// The deck holds no embedded object, so a scan that looks for them
		// leaves that check reporting that it found nothing, next to the
		// speaker notes the same scan did find.
		openOptions();
		cy.cGet(deck + ' #cleanup-sharing-input').check();

		scanPresentation(win);

		assertGroupListed('notes');
		assertCheckReads('ole', 'None');
	});

	it('the options stand behind a row that names what a scan would be run with', function() {
		// The values a scan is run with stand together at the end of the panel,
		// so the checks above them read as one list of what the presentation
		// holds.
		cy.cGet(deck + ' .cleanup-footer .cleanup-options').should('exist');
		cy.cGet(deck + ' .cleanup-tool .cleanup-options').should('not.exist');

		// The fields are in view from the start, and while they are on show they
		// say what they hold, so the row says nothing of its own.
		cy.cGet(deck + ' #cleanup-options-heading-button')
			.should('have.attr', 'aria-expanded', 'true');
		cy.cGet(deck + ' #cleanup-options-body').should('be.visible');
		cy.cGet(deck + ' #cleanup-options-heading-secondary').should('not.be.visible');

		// Pressing the row puts the fields away, and the line says what they
		// hold.
		closeOptions();
		cy.cGet(deck + ' #cleanup-options-heading-secondary').should('be.visible')
			.and('have.text', '150 DPI, quality 80 %');

		openOptions();
		cy.cGet(deck + ' #cleanup-options-heading-secondary').should('not.be.visible');
		cy.cGet(deck + ' #cleanup-sharing-input').check();
		cy.cGet(deck + ' #cleanup-resolution-input').select('Leave images alone');

		// The line follows what the fields now hold.
		closeOptions();
		cy.cGet(deck + ' #cleanup-options-heading-secondary').should('be.visible')
			.and('have.text', 'images left alone');
	});

	it('the options take a value again once a run is over', function() {
		const win = this.win;

		// A run turns the fields off. Once it is over they are live again, and
		// they still hold what they held.
		cy.cGet(deck + ' #cleanup-quality-input').should('have.value', '80');
		scanPresentation(win);

		cy.cGet(deck + ' #cleanup-resolution-input').should('not.be.disabled');
		cy.cGet(deck + ' #cleanup-quality-input').should('not.be.disabled')
			.and('have.value', '80');
		cy.cGet(deck + ' #cleanup-sharing-input').should('not.be.disabled').check()
			.should('be.checked');
		cy.cGet(deck + ' #cleanup-resolution-input').select('Leave images alone')
			.should('have.value', '0');

		// The values are the ones the next scan is run with: a scan that leaves
		// the images alone covers neither image check.
		scanPresentation(win);
		assertCheckReads('images', 'Not checked');
		assertCheckReads('cropped', 'Not checked');
	});

	it('the choice that asks for a presentation to be readied for sharing stands with the other options', function() {
		// The choice decides what a scan looks for, as the two figures above it
		// do, so it stands behind the same row and folds away with them.
		cy.cGet(deck + ' .cleanup-tool #cleanup-sharing-input').should('not.exist');
		closeOptions();
		cy.cGet(deck + ' #cleanup-sharing-input').should('not.be.visible');

		openOptions();

		cy.cGet(deck + ' #cleanup-options-body #cleanup-sharing-input').should('be.visible');
		cy.cGet(deck + ' #cleanup-options-body').then(($body) => {
			const ids = Array.from($body[0].children).map((child) => child.id);
			expect(ids, 'the resolution').to.include('cleanup-resolution');
			expect(ids, 'the quality').to.include('cleanup-quality');
			expect(ids.indexOf('cleanup-sharing'), 'place of the choice')
				.to.equal(ids.length - 1);
		});
	});

	it('a notes row offers to remove the notes and an image row offers to fix the image', function() {
		const win = this.win;

		openOptions();
		cy.cGet(deck + ' #cleanup-sharing-input').check();
		scanPresentation(win);

		assertGroupListed('notes');

		// Dealing with a speaker notes row takes the notes off the slide they
		// are on, and dealing with an image row leaves the image where it is
		// and works on what it holds, so the two rows say different words.
		cy.cGet(rowsOf('notes') + ':visible ' + rowFix).first()
			.should('have.text', 'Remove');
		cy.cGet(rowsOf('images') + ':visible ' + rowFix).first()
			.should('have.text', 'Fix');
	});

	it('changing an option leaves the list and the scan button as they were', function() {
		const win = this.win;

		scanPresentation(win);
		assertGroupListed('images');

		openOptions();
		cy.cGet(deck + ' #cleanup-resolution-input').select('300 DPI');

		// The row says what the next scan would be run with. Nothing else
		// moves: the list is still the one the last scan found, its heading
		// still stands over it, and the button still offers to scan the
		// presentation.
		cy.cGet(deck + ' #cleanup-options-heading-secondary')
			.should('have.text', '300 DPI, quality 80 %');
		cy.cGet(deck + ' #cleanup-scan').should('have.text', 'Scan presentation');
		cy.cGet(group('images') + ' .ui-expander-label')
			.should('have.text', 'High-resolution images');
		cy.cGet(rowsOf('images') + ':visible').should('have.length', 2);
	});

	it('the keyboard moves to Stop when a run starts from the options row', function() {
		const win = this.win;

		// The options go dead for as long as a run lasts, so the row they stand
		// behind hands the keyboard on rather than dropping it. The run is set
		// going without pressing the scan button, which would take the keyboard
		// off the options row first.
		cy.cGet(deck + ' #cleanup-options-heading-button').focus();
		helper.assertFocus('id', 'cleanup-options-heading-button');

		// The focus goes to Stop first and moves on when the run ends, which on a short run
		// can happen before the focus is checked. So the first element to take the focus is
		// noted, and that must be Stop.
		const focus = {};
		cy.cGet(deck).then(function($panel) {
			$panel[0].addEventListener('focusin', function(event) {
				focus.first = event.target.id;
			}, { once: true });
		});

		cy.cGet(deck + ' #cleanup-scan-button').then(function($scan) {
			$scan[0].dispatchEvent(new win.MouseEvent('click', { bubbles: true }));
		});

		cy.wrap(focus).its('first').should('equal', 'cleanup-stop-button');

		helper.processToIdle(win);
		waitForRunToEnd();
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

	it('the row link of a cropped image moves the view to its slide', function() {
		const win = this.win;

		scanPresentation(win);

		// The view is on the first slide of the deck.
		cy.cGet('#preview-img-part-0').should('have.class', 'preview-img-currentpart');

		// The cropped picture is on the third slide, and pressing its row takes
		// the reader there to look at it.
		cy.cGet(rowsOf('cropped') + ':visible ' + rowLink).first()
			.should('contain.text', 'Slide 3').click();

		cy.cGet('#preview-img-part-2').should('have.class', 'preview-img-currentpart');
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

	it('the keyboard reaches a row, the button of a row, the options and the scan button', function() {
		const win = this.win;

		scanPresentation(win);

		function describeFocused() {
			const element = win.document.activeElement;
			if (!element) return 'nothing';
			if (element.id) return '#' + element.id;
			return '.' + String(element.className).split(' ').join('.');
		}

		// Tab walks on from the heading of the first check and every widget it
		// lands on is noted, so what is asserted is what the reader reaches
		// rather than the widgets that happen to lie in between.
		const visited = [];

		// The button inside the heading is what the keyboard lands on, and it
		// takes the focus from the document rather than from the runner's own
		// focus command.
		cy.cGet(group('images') + ' .ui-expander-btn').then(function($heading) {
			$heading[0].focus();
		});

		for (let step = 0; step < 6; step++) {
			cy.realPress('Tab');
			cy.then(function() {
				visited.push(describeFocused());
			});
		}

		cy.then(function() {
			expect(visited.some(function(widget) {
				return widget.indexOf('-goto') !== -1;
			}), 'the link of a row, in ' + visited.join(' ')).to.be.true;
			expect(visited.some(function(widget) {
				return widget.indexOf('-fix-button') !== -1;
			}), 'the button of a row, in ' + visited.join(' ')).to.be.true;

			// A check with nothing to report holds nothing to press, so the
			// keyboard walks past it rather than stopping on it.
			expect(visited.some(function(widget) {
				return widget.indexOf('cleanup-check-row') !== -1;
			}), 'a check with nothing to report, in ' + visited.join(' ')).to.be.false;
		});

		// The footer ends the walk: the button that deals with the whole list,
		// the row the options stand behind, each field in the order it stands
		// in, and the one action of the panel.
		const fields = [];

		cy.cGet(deck + ' #cleanup-fix-all-button').focus();

		for (let step = 0; step < 5; step++) {
			cy.realPress('Tab');
			cy.then(function() {
				fields.push(describeFocused());
			});
		}

		cy.then(function() {
			expect(fields, 'the walk through the open options')
				.to.deep.equal(['#cleanup-options-heading-button', '#cleanup-resolution-input',
					'#cleanup-quality-input', '#cleanup-sharing-input', '#cleanup-scan-button']);
		});

		// With the fields folded away the keyboard walks past them, from the
		// row they stand behind straight to the one action of the panel.
		const footer = [];

		closeOptions();
		cy.cGet(deck + ' #cleanup-fix-all-button').focus();

		for (let step = 0; step < 2; step++) {
			cy.realPress('Tab');
			cy.then(function() {
				footer.push(describeFocused());
			});
		}

		cy.then(function() {
			expect(footer, 'the walk through the folded options')
				.to.deep.equal(['#cleanup-options-heading-button', '#cleanup-scan-button']);
		});
	});

	it('pressing Enter on the button of a row deals with that row', function() {
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

		// Each tool is headed by a heading and each check under it by one a
		// level down, so a reader can move through the panel by heading.
		cy.cGet(deck + ' #cleanup-tool-images-title')
			.should('have.attr', 'role', 'heading');
		cy.cGet(group('images') + ' .ui-expander-heading').should(function($heading) {
			expect($heading[0].tagName, 'the tag of a check heading').to.equal('H2');
		});

		// The button of an image row says only Fix, so its label names the row
		// it deals with as well.
		cy.cGet(rowsOf('images') + ':visible ' + rowLink).first().invoke('text')
			.then(function(text) {
				cy.cGet(rowsOf('images') + ':visible ' + rowFix).first()
					.should('have.attr', 'aria-label', 'Fix ' + text.trim());
			});
	});

	it('opening the deck again starts from an empty panel', function() {
		const win = this.win;

		scanPresentation(win);
		assertGroupListed('images');
		openOptions();

		closeCleanupDeck(win);
		openCleanupDeck(win);

		// Closing the deck told the kit to let the run go, so the panel comes
		// back asking for a scan rather than showing what the last one found.
		// The fields are out of the way again too, so what the reader opens is
		// the panel at its shortest however they left it.
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
		cy.cGet(deck + ' #cleanup-options-heading-button').should('be.visible');

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

	it('the keyboard moves to the button of the next row once a row is dealt with', function() {
		const win = this.win;

		scanPresentation(win);

		cy.cGet(rowsOf('images') + ':visible').should('have.length', 2);

		cy.cGet(rowsOf('images') + ':visible ' + rowFix).first().focus();
		cy.realPress('Enter');

		helper.processToIdle(win);
		waitForRunToEnd();

		// The row that was dealt with took the keyboard with it, so the keyboard
		// stands on the button of the row that followed it and the next press
		// acts on that row.
		cy.cGet(rowsOf('images') + ':visible ' + rowFix).should(function($fix) {
			expect($fix.length, 'row buttons left on screen').to.equal(1);
			expect($fix[0].ownerDocument.activeElement,
				'the widget the keyboard is on').to.equal($fix[0]);
		});
	});
});
