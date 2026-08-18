/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var aichatHelper = require('../../common/aichat_helper');

// The AI assistant always writes formulas in US English syntax: English function
// names and a comma between the arguments. A German session reads a formula with a
// semicolon between the arguments and shows the German function names, so the
// assistant's formula only lands if the command it writes through reads English.

function setCellFormula(win, address, formula) {
	win.app.map.sendUnoCommand('.uno:SetCellFormula', {
		Cell: { type: 'string', value: address },
		Formula: { type: 'string', value: formula },
	});
}

function cellValue(win, address, expected) {
	calcHelper.enterCellAddressAndConfirm(win, address);
	helper.copy();
	cy.cGet('#copy-paste-container table td')
		.should('not.contain.text', 'Err')
		.should('not.contain.text', '#NAME?')
		.invoke('text')
		.should('match', expected);
}

describe(['tagdesktop'], 'Assistant formulas in a comma-decimal language', function() {
	beforeEach(function() {
		// A1 holds 2 and A2 holds 3.
		helper.setupAndLoadDocument('calc/aichat_formula.fods', false, false, 'de-DE');
		helper.setDummyClipboardForCopy();
		cy.getFrameWindow().then((win) => { this.win = win; });
	});

	it('a formula with English names and comma arguments computes', function() {
		setCellFormula(this.win, 'B1', '=AVERAGE(A1,A2)');
		cellValue(this.win, 'B1', /^2[.,]5$/);
	});

	it('several cells in a row each get their formula', function() {
		setCellFormula(this.win, 'B1', '=SUM(A1,A2)');
		setCellFormula(this.win, 'B2', '=MAX(A1,A2)');
		cellValue(this.win, 'B1', /^5$/);
		cellValue(this.win, 'B2', /^3$/);
	});

	it('a period in a plain number is the decimal point', function() {
		setCellFormula(this.win, 'B1', '1.5');
		cellValue(this.win, 'B1', /^1[.,]5$/);
	});

	it('text is written as text', function() {
		setCellFormula(this.win, 'B1', 'Durchschnitt');
		cellValue(this.win, 'B1', /^Durchschnitt$/);
	});
});

describe(['tagdesktop'], 'Assistant formulas in a dot-decimal language', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/aichat_formula.fods', false, false, 'en-US');
		helper.setDummyClipboardForCopy();
		cy.getFrameWindow().then((win) => { this.win = win; });
	});

	it('the same formula computes the same way', function() {
		setCellFormula(this.win, 'B1', '=AVERAGE(A1,A2)');
		cellValue(this.win, 'B1', /^2\.5$/);
	});
});

describe(['tagdesktop'], 'Assistant formula approval', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/aichat_formula.fods');
		cy.getFrameWindow().then((win) => { this.win = win; });
	});

	it('the card says the spreadsheet is about to change', function() {
		aichatHelper.enableAIWithCaptureSocket(this.win, {
			approvalToolName: 'set_cell_formula',
		});
		aichatHelper.openAIChat();
		aichatHelper.typeIntoAIInput('Average column A');
		aichatHelper.clickSend();

		cy.cGet('#aichat-messages-list').should(
			'contain.text', 'wants to set a formula in your spreadsheet');
	});

	it('approving the card answers approve', function() {
		aichatHelper.enableAIWithCaptureSocket(this.win, {
			approvalToolName: 'set_cell_formula',
		});
		aichatHelper.openAIChat();
		aichatHelper.typeIntoAIInput('Average column A');
		aichatHelper.clickSend();

		cy.cGet('.aichat-approve-btn').click();
		cy.getFrameWindow().should(function(win) {
			expect(win.__aichatApprovePayloads).to.have.length(1);
			expect(win.__aichatApprovePayloads[0].action).to.eq('approve');
		});
	});

	it('rejecting the card answers reject', function() {
		aichatHelper.enableAIWithCaptureSocket(this.win, {
			approvalToolName: 'set_cell_formula',
		});
		aichatHelper.openAIChat();
		aichatHelper.typeIntoAIInput('Average column A');
		aichatHelper.clickSend();

		cy.cGet('.aichat-reject-btn').click();
		cy.getFrameWindow().should(function(win) {
			expect(win.__aichatApprovePayloads).to.have.length(1);
			expect(win.__aichatApprovePayloads[0].action).to.eq('reject');
		});
	});
});
