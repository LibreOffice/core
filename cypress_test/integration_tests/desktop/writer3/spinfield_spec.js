/* global describe it cy require beforeEach expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Spinfield unit and button tests', function () {
	var win;

	beforeEach(function () {
		helper.setupAndLoadDocument('writer/help_dialog.odt');
		cy.getFrameWindow().then(function (w) {
			win = w;
		});
	});

	function getUnit(val, decimal) {
		var pattern = new RegExp('[\\d\\-\\' + decimal + ']', 'g');
		return val.replace(pattern, '').trim();
	}

	function openDialogAndSwitchToBorder() {
		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:FontDialog');
		});

		cy.cGet('.ui-dialog[role="dialog"]').should('have.length', 1);
		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.cGet('button#borders.ui-tab').click();

		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.cGet('#leftmf-input').should('be.visible');
	}

	function parseLocaleNumber(val, decimal) {
		if (decimal !== '.')
			val = val.replace(decimal, '.');
		return parseFloat(val);
	}

	function testUnitPersistence(decimal, expectedUnit) {
		openDialogAndSwitchToBorder();

		// Unit is correct and persists after clicking up button
		cy.cGet('#leftmf-input').invoke('val').then(function (initialVal) {
			var initialUnit = getUnit(initialVal, decimal);
			expect(initialUnit).to.equal(expectedUnit);

			cy.cGet('#leftmf .spinfieldbutton-up').click();

			cy.cGet('#leftmf-input').should('be.visible');
			cy.cGet('#leftmf-input').should(function ($el) {
				expect(getUnit($el.val(), decimal)).to.equal(expectedUnit);
			});
		});

		// Unit persists after arrow key change
		cy.cGet('#leftmf-input').invoke('val').then(function (val) {
			cy.cGet('#leftmf-input').focus();
			cy.cGet('#leftmf-input').type('{uparrow}');

			cy.cGet('#leftmf-input').should('be.visible');
			cy.cGet('#leftmf-input').should(function ($el) {
				expect(getUnit($el.val(), decimal)).to.equal(expectedUnit);
			});
		});

		// Typing a value with the locale's decimal separator is accepted
		cy.cGet('#leftmf-input').focus();
		cy.cGet('#leftmf-input').clear();
		cy.cGet('#leftmf-input').type('0' + decimal + '5');
		cy.cGet('#leftmf-input').should(function ($el) {
			expect(parseLocaleNumber($el.val(), decimal)).to.equal(0.5);
		});
	}

	it('Unit persists after button click and arrow key', function () {
		testUnitPersistence('.', '\u2033');
	});

	it('Space before unit in displayed value', function () {
		helper.setupAndLoadDocument('writer/help_dialog.odt', false, false, 'de-DE');
		cy.getFrameWindow().then(function (w) {
			win = w;
		});

		openDialogAndSwitchToBorder();

		// cm unit should have a space before it
		cy.cGet('#leftmf-input').invoke('val').then(function (val) {
			expect(val).to.match(/\d\scm$/);
		});

		// After incrementing, the space should persist
		cy.cGet('#leftmf .spinfieldbutton-up').click();
		cy.cGet('#leftmf-input').invoke('val').then(function (val) {
			expect(val).to.match(/\d\scm$/);
		});
	});

	it('Unit persists with German locale', function () {
		helper.setupAndLoadDocument('writer/help_dialog.odt', false, false, 'de-DE');
		cy.getFrameWindow().then(function (w) {
			win = w;
		});

		testUnitPersistence(',', 'cm');
	});

	function testButtonsAndArrowKeys(decimal, altDecimal) {
		openDialogAndSwitchToBorder();

		var input = '#leftmf-input';
		var upBtn = '#leftmf .spinfieldbutton-up';
		var downBtn = '#leftmf .spinfieldbutton-down';

		// Up button increments and displays locale decimal separator
		cy.cGet(input).invoke('val').then(function (val) {
			var num = parseLocaleNumber(val, decimal);

			cy.cGet(upBtn).click();

			cy.cGet(input).should(function ($el) {
				var newVal = $el.val();
				var newNum = parseLocaleNumber(newVal, decimal);
				expect(newNum).to.be.greaterThan(num);
				// Fractional value must use the locale's decimal separator
				if (newNum !== Math.floor(newNum)) {
					expect(newVal).to.contain(decimal);
					expect(newVal).to.not.contain(altDecimal);
				}
			});
		});

		// Down button decrements
		cy.cGet(input).invoke('val').then(function (val) {
			var num = parseLocaleNumber(val, decimal);

			cy.cGet(downBtn).click();

			cy.cGet(input).should(function ($el) {
				expect(parseLocaleNumber($el.val(), decimal)).to.be.lessThan(num);
			});
		});

		// Arrow up key increments
		cy.cGet(input).invoke('val').then(function (val) {
			var num = parseLocaleNumber(val, decimal);

			cy.cGet(input).focus();
			cy.cGet(input).type('{uparrow}');

			cy.cGet(input).should(function ($el) {
				expect(parseLocaleNumber($el.val(), decimal)).to.be.greaterThan(num);
			});
		});

		// Arrow down key decrements
		cy.cGet(input).invoke('val').then(function (val) {
			var num = parseLocaleNumber(val, decimal);

			cy.cGet(input).type('{downarrow}');

			cy.cGet(input).should(function ($el) {
				expect(parseLocaleNumber($el.val(), decimal)).to.be.lessThan(num);
			});
		});
	}

	it('Buttons and arrow keys increment and decrement the value', function () {
		testButtonsAndArrowKeys('.', ',');
	});

	it('Buttons and arrow keys work with German locale', function () {
		helper.setupAndLoadDocument('writer/help_dialog.odt', false, false, 'de-DE');
		cy.getFrameWindow().then(function (w) {
			win = w;
		});

		testButtonsAndArrowKeys(',', '.');
	});

	it('Ctrl+Home sets minimum value', function () {
		openDialogAndSwitchToBorder();

		var input = '#leftmf-input';

		cy.cGet(input).focus();
		cy.cGet(input).type('{ctrl}{home}');

		cy.cGet(input).should(function ($el) {
			var min = parseFloat($el.closest('.spinfieldcontainer')[0]._min);
			var num = parseFloat($el.val());
			expect(num).to.equal(min);
		});
	});

	it('Ctrl+End sets maximum value', function () {
		openDialogAndSwitchToBorder();

		var input = '#leftmf-input';

		cy.cGet(input).focus();
		cy.cGet(input).type('{ctrl}{end}');

		cy.cGet(input).should(function ($el) {
			var max = parseFloat($el.closest('.spinfieldcontainer')[0]._max);
			var num = parseFloat($el.val());
			expect(num).to.equal(max);
		});
	});

	it('Kerning spinfield is labelled by Custom Value label', function () {
		desktopHelper.switchUIToNotebookbar();
		cy.cGet('#sidebar-dock-wrapper').should('be.visible');

		// Click the dropdown arrow on the Spacing toolbar button
		// in the sidebar to open the TextCharacterSpacingControl popover
		cy.cGet('#sidebar-dock-wrapper .unoSpacing .arrowbackground').click();

		cy.cGet('.jsdialog-window.modalpopup').should('exist');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		// The kerning spinfield should be labeled by a <label> element
		// with text "Custom Value", not have a stale aria-label
		// containing the current numeric value.
		cy.cGet('label[for="kerning-input"]').should('exist')
			.should('contain.text', 'Custom Value');
		cy.cGet('#kerning-input').should('not.have.attr', 'aria-label');

		// Change the value and verify the label still says "Custom Value"
		// and no aria-label with the old value reappears.
		cy.cGet('#kerning-input').clear().type('1.5');
		cy.cGet('#kerning-input').should('not.have.attr', 'aria-label');
		cy.cGet('label[for="kerning-input"]').should('contain.text', 'Custom Value');
	});

	it('Buttons enabled after re-enabling spinfield in columns dialog', function () {
		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:FormatColumns');
		});

		cy.cGet('.ui-dialog[role="dialog"]').should('have.length', 1);
		cy.then(function () {
			return helper.processToIdle(win);
		});

		// Set columns to 2
		cy.cGet('#colsnf-input').should('be.visible');
		cy.cGet('#colsnf .spinfieldbutton-up').click();
		cy.then(function () {
			return helper.processToIdle(win);
		});

		// Select a separator line style via the menubutton popup
		cy.cGet('#linestylelb').should('be.visible');
		cy.cGet('#linestylelb').click();
		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.cGet('#iconview_lines .ui-iconview-entry').first().click();
		cy.then(function () {
			return helper.processToIdle(win);
		});

		// The line width spinfield and its buttons should be enabled
		cy.cGet('#linewidthmf-input').should('be.visible');
		cy.cGet('#linewidthmf-input').should('not.have.attr', 'disabled');
		cy.cGet('#linewidthmf .spinfieldbutton-up').should('not.have.attr', 'disabled');
		cy.cGet('#linewidthmf .spinfieldbutton-down').should('not.have.attr', 'disabled');

		cy.cGet('#linewidthmf-input').invoke('val').then(function (initialVal) {
			var initialNum = parseFloat(initialVal);

			cy.cGet('#linewidthmf .spinfieldbutton-up').click();

			cy.cGet('#linewidthmf-input').should(function ($el) {
				var newNum = parseFloat($el.val());
				expect(newNum).to.be.greaterThan(initialNum);
			});
		});
	});
});
