/* global describe it cy beforeEach require Cypress expect */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagdesktop'], 'Calc keyboard access for the bars', function () {
	var FOCUSABLE = 'button:not([disabled]):not([tabindex="-1"]), ' +
		'input:not([disabled]):not([tabindex="-1"]), [tabindex="0"]';

	beforeEach(function () {
		helper.setupAndLoadDocument('calc/focus.ods');
	});

	// Wait one Chrome repaint cycle (two requestAnimationFrame) so a focus
	// change is applied before we sample it - no server round-trip.
	function waitAFrame() {
		cy.getFrameWindow().then(function (win) {
			return new Cypress.Promise(function (resolve) {
				win.requestAnimationFrame(function () {
					win.requestAnimationFrame(resolve);
				});
			});
		});
	}

	// Press F6 until keyboard focus lands somewhere inside the container.
	function focusRegionWithF6(containerSelector, remainingTries) {
		cy.realPress('F6');
		waitAFrame();
		cy.cGet(containerSelector).then(function (found) {
			var container = found[0];
			if (container.contains(container.ownerDocument.activeElement))
				return;
			if (remainingTries <= 1)
				throw new Error(containerSelector + ' did not get focus after cycling F6');
			focusRegionWithF6(containerSelector, remainingTries - 1);
		});
	}

	// Tab onto each focusable element in turn, asserting the focus landed on
	// it, until focus leaves the container (the input field redirects DOM
	// focus to a hidden input and marks its wrapper 'focused').
	function tabThroughElements(containerSelector, elements, i) {
		if (i >= elements.length)
			return;

		// F6 already focused the first element; Tab to reach the following ones
		if (i > 0) {
			cy.realPress('Tab');
			waitAFrame();
		}

		var element = elements[i];
		var name = element.id || element.getAttribute('aria-label');
		cy.cGet(containerSelector).should(function (found) {
			var active = found[0].ownerDocument.activeElement;
			expect(element.contains(active) || element.closest('.focused') !== null,
				name).to.equal(true);
		});

		cy.cGet(containerSelector).then(function (found) {
			if (found[0].contains(found[0].ownerDocument.activeElement))
				tabThroughElements(containerSelector, elements, i + 1);
		});
	}

	// Shift+Tab back until the given element has keyboard focus, in case F6
	// did not land on the first focusable of the container.
	function focusFirstElement(containerSelector, first, remainingTries) {
		cy.cGet(containerSelector).then(function (found) {
			if (found[0].ownerDocument.activeElement === first)
				return;
			if (remainingTries <= 1)
				throw new Error('could not reach the first focusable with Shift+Tab');
			cy.realPress(['Shift', 'Tab']);
			waitAFrame();
			focusFirstElement(containerSelector, first, remainingTries - 1);
		});
	}

	function assertTabVisitsFocusables(containerSelector) {
		cy.cGet(containerSelector).find(FOCUSABLE).filter(':visible').then(function (found) {
			var elements = found.toArray();
			expect(elements.length, containerSelector + ' focusable elements').to.be.greaterThan(1);
			focusFirstElement(containerSelector, elements[0], elements.length);
			tabThroughElements(containerSelector, elements, 0);
		});
	}

	it('F6 then Tab visits every focusable item of the formula bar row', function () {
		calcHelper.clickOnFirstCell();
		focusRegionWithF6('#formulabar-row', 10);
		assertTabVisitsFocusables('#formulabar-row');
	});

	it('F6 then Tab visits every focusable item of the status bar', function () {
		calcHelper.clickOnFirstCell();
		focusRegionWithF6('#toolbar-down', 10);
		assertTabVisitsFocusables('#toolbar-down');
	});

	// The sheet-name tabs are a roving-tabindex tablist (only the selected tab is
	// a tab stop; Arrow keys move between tabs), so Tab visits the bar's tab stops
	// - the insert-sheet button and the selected tab - not every sheet tab.
	it('F6 then Tab visits every focusable item of the sheet-name tabs bar', function () {
		calcHelper.clickOnFirstCell();
		focusRegionWithF6('#spreadsheet-toolbar', 10);
		assertTabVisitsFocusables('#spreadsheet-toolbar');
	});
});
