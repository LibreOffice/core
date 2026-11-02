/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var desktopHelper = require('../../common/desktop_helper');

// Counts the pixels of the document canvas that are not the plain white of an
// empty grid, over a fixed region at the top left of the sheet. Two sheets that
// carry different content give different counts, so the count says which sheet
// the grid is showing.
function readInk(win) {
	var anchor = win.app.sectionContainer.getDocumentAnchor();
	var canvas = win.document.getElementById('document-canvas');
	var data = canvas.getContext('2d')
		.getImageData(Math.round(anchor[0]), Math.round(anchor[1]), 500, 200).data;

	var ink = 0;
	for (var i = 0; i < data.length; i += 4) {
		if (data[i] < 200 || data[i + 1] < 200 || data[i + 2] < 200)
			ink++;
	}
	return ink;
}

// The cell cursor is drawn on the grid too, so it has to stand on the same cell
// in every measurement, or it counts as a difference between them.
function parkCursor() {
	cy.getFrameWindow().then(function(win) {
		calcHelper.enterCellAddressAndConfirm(win, 'A1');
	});
}

// Takes the ink count once the grid has stopped changing, so that every tile of
// the sheet on screen has arrived and been drawn.
function settledInk(name, counts) {
	cy.getFrameWindow().then(function(win) { helper.processToIdle(win); });

	var previous = -1;
	var stable = 0;
	function sample() {
		cy.getFrameWindow().then(function(win) {
			var ink = readInk(win);
			stable = (ink === previous) ? stable + 1 : 0;
			previous = ink;
		});
		cy.then(function() {
			if (stable < 3) {
				cy.wait(250);
				sample();
			} else {
				counts[name] = previous;
			}
		});
	}
	sample();
}

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Calc sheet view tests', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/sheetview.fods');
		desktopHelper.switchUIToNotebookbar();
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		}.bind(this));
	});

	it('A sheet view created a second time shows its own sheet', function() {
		var counts = {};

		// A second sheet, holding text of its own. It takes the sheet index that a
		// sheet view of the first sheet is created at.
		cy.cGet('#sheets-buttons-toolbox #insertsheet').click();
		cy.cGet('#spreadsheet-tab1').should('have.class', 'spreadsheet-tab-selected');

		parkCursor();
		helper.typeIntoDocument('ABC{enter}');
		parkCursor();
		settledInk('secondSheet', counts);

		// Back to the first sheet, the one the sheet views are made from.
		cy.cGet('#spreadsheet-tab0').click();
		cy.cGet('#spreadsheet-tab0').should('have.class', 'spreadsheet-tab-selected');
		parkCursor();
		settledInk('firstSheet', counts);

		cy.then(function() {
			// The two sheets have to look different, or the check below proves nothing.
			expect(counts.secondSheet).to.not.equal(counts.firstSheet);
		});

		// Create a sheet view, drop it again, then create one a second time.
		cy.getFrameWindow().then(function(win) {
			win.app.map.sendUnoCommand('.uno:NewSheetView');
			helper.processToIdle(win);
		});
		cy.getFrameWindow().then(function(win) {
			win.app.map.sendUnoCommand('.uno:RemoveSheetView');
			helper.processToIdle(win);
		});
		cy.getFrameWindow().then(function(win) {
			win.app.map.sendUnoCommand('.uno:NewSheetView');
			helper.processToIdle(win);
		});

		// The sheet view is a view of the first sheet, so the grid shows the first
		// sheet's table and not the second sheet's text.
		parkCursor();
		settledInk('sheetView', counts);
		cy.then(function() {
			expect(counts.sheetView).to.equal(counts.firstSheet);
		});
	});

	it('A comment added on a sheet view shows when the pointer is over its cell', function() {
		var win = this.win;

		function commentList() {
			return win.app.sectionContainer.getSectionWithName(win.app.CSections.CommentList.name)
				.sectionProperties.commentList;
		}

		// Adds a comment to the cell the way the Save button of a new comment does, then waits
		// until the comment has reached the comments of the selected sheet.
		function addComment(address, text) {
			calcHelper.enterCellAddressAndConfirm(win, address);
			cy.then(function() {
				win.app.map.sendUnoCommand('.uno:InsertAnnotation', {
					Text: { type: 'string', value: text }
				}, true);
				return helper.processToIdle(win);
			});
			cy.getFrameWindow().should(function(frameWindow) {
				var selectedPart = frameWindow.app.map._docLayer._selectedPart;
				var found = commentList().some(function(comment) {
					return comment.sectionProperties.data.text === text
						&& parseInt(comment.sectionProperties.data.tab) === selectedPart;
				});
				expect(found).to.equal(true);
			});
		}

		// Moves the pointer onto the cell, to the right of its middle, where the comment of the
		// cell takes the mouse.
		function hoverCell(address) {
			calcHelper.enterCellAddressAndConfirm(win, address);
			cy.cGet('#test-div-OwnCellCursor').then(function(items) {
				var rectangle = items[0].getBoundingClientRect();
				var x = Math.round(rectangle.left + rectangle.width * 0.7);
				var y = Math.round(rectangle.top + rectangle.height * 0.5);
				cy.cGet('body').realMouseMove(x, y);
			});
		}

		addComment('C1', 'Comment on C1');

		cy.then(function() {
			win.app.map.sendUnoCommand('.uno:NewSheetView');
			return helper.processToIdle(win);
		});
		cy.getFrameWindow().should(function(frameWindow) {
			var selectedPart = frameWindow.app.map._docLayer._selectedPart;
			expect(frameWindow.app.calc.isPartSheetView(selectedPart)).to.equal(true);
		});

		addComment('C3', 'Comment on C3');

		// The comment made before the sheet view existed pops up.
		hoverCell('C1');
		cy.cGet('.cool-annotation-content:visible').should('contain', 'Comment on C1');

		// The comment made on the sheet view pops up too, and replaces the first one.
		hoverCell('C3');
		cy.cGet('.cool-annotation-content:visible').should('contain', 'Comment on C3');
		cy.cGet('.cool-annotation-content:visible').should('not.contain', 'Comment on C1');
	});
});
