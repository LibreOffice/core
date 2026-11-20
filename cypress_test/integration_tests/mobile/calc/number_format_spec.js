/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var mobileHelper = require('../../common/mobile_helper');

describe.skip(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Apply number formatting.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/number_format.ods', false, false, undefined, 'vector=false');

		// Click on edit button
		mobileHelper.enableEditingMobile();

		calcHelper.clickOnFirstCell();

		mobileHelper.openMobileWizard();
		cy.cGet('#ScNumberFormatPropertyPanel').click();
		cy.cGet('#numberformatcombobox > .ui-header').should('be.visible');
	});

	function selectFormatting(formattingString) {
		// Select formatting list
		mobileHelper.selectListBoxItem2('#numberformatcombobox > .ui-header', formattingString);
	}

	it('Select percent format from list.', function() {
		cy.cGet('#numberformatcombobox > .level-2').contains('Number').click();
		cy.cGet('.ui-content.level-1.mobile-wizard').contains('Percent').click();

		cy.cGet('.unoNumberFormatPercent').should('have.class', 'selected');

		// Decimal and leading zeros are changed.
		cy.cGet('#decimalplaces input').should('have.value', '2');
		cy.cGet('#leadingzeroes input').should('have.value', '1');

		calcHelper.selectEntireSheet();

		var regex = new RegExp(';0;0.00%$');
		cy.cGet('#copy-paste-container table td').should('have.attr', 'sdnum').should('match', regex);
		cy.cGet('#copy-paste-container table td').should('have.text', '100000.00%');
	});

	it('Push percent button.', function() {
		cy.cGet('#NumberFormatPercentimg').click();

		cy.cGet('.unoNumberFormatPercent').should('have.class', 'selected');
		cy.cGet('#numberformatcombobox .ui-header-left').should('have.text', 'Percent');

		// Decimal and leading zeros are changed.
		cy.cGet('#decimalplaces input').should('have.value', '2');
		cy.cGet('#leadingzeroes input').should('have.value', '1');
		calcHelper.selectEntireSheet();
		var regex = new RegExp(';0;0.00%$');
		cy.cGet('#copy-paste-container table td').should('have.attr', 'sdnum').should('match', regex);

		cy.cGet('#copy-paste-container table td').should('have.text', '100000.00%');
	});

	it('Select currency format from list.', function() {
		cy.cGet('#numberformatcombobox > .level-2').contains('Number').click();
		cy.cGet('.ui-content.level-1.mobile-wizard').contains('Currency').click();

		cy.cGet('.unoNumberFormatCurrency').should('have.class', 'selected');

		// Decimal and leading zeros are changed.
		cy.cGet('#decimalplaces input').should('have.value', '2');
		cy.cGet('#leadingzeroes input').should('have.value', '1');
		calcHelper.selectEntireSheet();

		var regex = new RegExp(';\\[\\$\\$-409]#,##0.00;\\[RED]-\\[\\$\\$-409]#,##0.00$');
		cy.cGet('#copy-paste-container table td').should('have.attr', 'sdnum').should('match', regex);
		cy.cGet('#copy-paste-container table td').should('have.text', '$1,000.00');
	});

	it('Push currency button.', function() {
		cy.cGet('#NumberFormatCurrencyimg').click();
		cy.cGet('.unoNumberFormatCurrency').should('have.class', 'selected');
		cy.cGet('#numberformatcombobox .ui-header-left').should('have.text', 'Currency');

		// Decimal and leading zeros are changed.
		cy.cGet('#decimalplaces input').should('have.value', '2');
		cy.cGet('#leadingzeroes input').should('have.value', '1');

		calcHelper.selectEntireSheet();

		var regex = new RegExp(';\\[\\$\\$-409]#,##0.00;\\[RED]-\\[\\$\\$-409]#,##0.00$');
		cy.cGet('#copy-paste-container table td').should('have.attr', 'sdnum').should('match', regex);
		cy.cGet('#copy-paste-container table td').should('have.text', '$1,000.00');
	});

	it('Push number button.', function() {
		// Change to currency first
		cy.cGet('#NumberFormatCurrencyimg').click();
		cy.cGet('.unoNumberFormatCurrency').should('have.class', 'selected');

		// Decimal and leading zeros are changed.
		cy.cGet('#decimalplaces input').should('have.value', '2');
		cy.cGet('#leadingzeroes input').should('have.value', '1');
		calcHelper.selectEntireSheet();

		var regex = new RegExp(';\\[\\$\\$-409]#,##0.00;\\[RED]-\\[\\$\\$-409]#,##0.00$');

		cy.cGet('#copy-paste-container table td').should('have.attr', 'sdnum').should('match', regex);
		cy.cGet('#copy-paste-container table td').should('have.text', '$1,000.00');

		calcHelper.clickOnFirstCell();
		mobileHelper.openMobileWizard();

		cy.cGet('#ScNumberFormatPropertyPanel').click();
		cy.cGet('.unoNumberFormatDecimal').should('be.visible');

		// Change to number formatting
		cy.cGet('.unoNumberFormatDecimal').click();

		cy.cGet('.unoNumberFormatDecimal').should('have.class', 'selected');
		cy.cGet('#numberformatcombobox .ui-header-left').should('have.text', 'Number');
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table td').should('have.text', '1,000.00');
	});

	it('Select date format from list.', function() {
		cy.cGet('#numberformatcombobox > .level-2').contains('Number').click();
		cy.cGet('.ui-content.level-1.mobile-wizard').contains('Date').click();

		// Combobox entry contains the selected format
		cy.cGet('#numberformatcombobox .ui-header-left')
			.should('have.text', 'Date ');

		// Decimal and leading zeros are changed.
		cy.cGet('#decimalplaces input')
			.should('have.value', '0');

		cy.cGet('#leadingzeroes input')
			.should('have.value', '0');

		calcHelper.selectEntireSheet();

		var regex = new RegExp(';MM/DD/YY$');
		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'sdnum')
			.should('match', regex);

		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'sdval', '1000');

		cy.cGet('#copy-paste-container table td')
			.should('have.text', '09/26/02');
	});

	it('Select time format from list.', function() {
		cy.cGet('#numberformatcombobox > .level-2').contains('Number').click();
		cy.cGet('.ui-content.level-1.mobile-wizard').contains('Time').click();

		// Decimal and leading zeros are changed.
		cy.cGet('#decimalplaces input')
			.should('have.value', '0');

		cy.cGet('#leadingzeroes input')
			.should('have.value', '0');

		calcHelper.selectEntireSheet();

		var regex = new RegExp(';HH:MM:SS AM/PM$');
		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'sdnum')
			.should('match', regex);

		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'sdval', '1000');

		cy.cGet('#copy-paste-container table td')
			.should('have.text', '12:00:00 AM');
	});

	it('Select scientific format from list.', function() {
		cy.cGet('#numberformatcombobox > .level-2').contains('Number').click();
		cy.cGet('.ui-content.level-1.mobile-wizard').contains('Scientific').click();

		// Decimal and leading zeros are changed.
		cy.cGet('#decimalplaces input')
			.should('have.value', '2');

		cy.cGet('#leadingzeroes input')
			.should('have.value', '1');

		calcHelper.selectEntireSheet();

		var regex = new RegExp(';0\\.00E\\+00$');
		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'sdnum')
			.should('match', regex);

		cy.cGet('#copy-paste-container table td')
			.should('have.text', '1.00E+03');
	});

	it.skip('Select fraction format from list.', function() {
		selectFormatting('Fraction');

		// Decimal and leading zeros are changed.
		cy.cGet('#decimalplaces input')
			.should('have.value', '1');

		cy.cGet('#leadingzeroes input')
			.should('have.value', '0');

		calcHelper.selectEntireSheet();

		var regex = new RegExp(';# \\?/\\?$');
		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'sdnum')
			.should('match', regex);

		cy.cGet('#copy-paste-container table td')
			.should('have.text', '1000    ');
	});

	it('Select boolean format from list.', function() {
		cy.cGet('#numberformatcombobox > .level-2').contains('Number').click();
		cy.cGet('.ui-content.level-1.mobile-wizard').contains('Boolean Value').click();

		// Decimal and leading zeros are changed.
		cy.cGet('#decimalplaces input')
			.should('have.value', '0');

		cy.cGet('#leadingzeroes input')
			.should('have.value', '0');

		calcHelper.selectEntireSheet();

		var regex = new RegExp(';BOOLEAN$');
		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'sdnum')
			.should('match', regex);

		cy.cGet('#copy-paste-container table td')
			.should('have.text', 'TRUE');
	});

	it('Select text format from list.', function() {
		cy.cGet('#numberformatcombobox > .level-2').contains('Number').click();
		cy.cGet('.ui-content.level-1.mobile-wizard').contains('Text').click();

		// Decimal and leading zeros are changed.
		cy.cGet('#decimalplaces input')
			.should('have.value', '0');

		cy.cGet('#leadingzeroes input')
			.should('have.value', '0');

		calcHelper.selectEntireSheet();

		var regex = new RegExp(';@$');
		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'sdnum')
			.should('match', regex);

		cy.cGet('#copy-paste-container table td')
			.should('have.text', '1000');
	});

	it('Change decimal places.', function() {
		// Check default value
		cy.cGet('#decimalplaces input')
			.should('have.value', '0');

		// Type in a new value
		helper.typeIntoInputField('#decimalplaces input', '2', true, false);

		calcHelper.selectEntireSheet();

		var regex = new RegExp(';0\\.00$');
		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'sdnum')
			.should('match', regex);

		cy.cGet('#copy-paste-container table td')
			.should('have.text', '1000.00');
	});

	it('Change leading zeros.', function() {
		// Check default value
		cy.cGet('#leadingzeroes input')
			.should('have.value', '1');

		// Type in a new value
		helper.typeIntoInputField('#leadingzeroes input', '6', true, false);

		calcHelper.selectEntireSheet();

		var regex = new RegExp(';000000$');
		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'sdnum')
			.should('match', regex);

		cy.cGet('#copy-paste-container table td')
			.should('have.text', '001000');
	});

	it('Apply red color for negative numbers.', function() {
		// Check default value
		cy.cGet('#negativenumbersred input')
			.should('not.have.prop', 'checked', true);

		// Change the option
		cy.cGet('#negativenumbersred input').click();

		cy.cGet('#negativenumbersred input')
			.should('have.prop', 'checked', true);

		calcHelper.selectEntireSheet();

		var regex = new RegExp(';0;\\[RED]-0$');
		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'sdnum')
			.should('match', regex);

		cy.cGet('#copy-paste-container table td')
			.should('have.text', '1000');
	});

	it('Add thousands separator.', function() {
		// Check default value
		cy.cGet('#thousandseparator input')
			.should('not.have.prop', 'checked', true);

		// Change the option
		cy.cGet('#thousandseparator input').click();

		cy.cGet('#thousandseparator input')
			.should('have.prop', 'checked', true);

		calcHelper.selectEntireSheet();

		var regex = new RegExp(';#,##0$');
		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'sdnum')
			.should('match', regex);

		cy.cGet('#copy-paste-container table td')
			.should('have.text', '1,000');
	});
});
