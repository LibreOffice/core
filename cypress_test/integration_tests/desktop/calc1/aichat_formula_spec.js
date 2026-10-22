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

describe(['tagdesktop'], 'Assistant suggestions in a spreadsheet', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/aichat_formula.fods');
		cy.getFrameWindow().then((win) => {
			this.win = win;
			aichatHelper.enableAIAndStubSocket(win, {});
		});
		aichatHelper.openAIChat();
	});

	it('four suggestions show, and See more shows the rest', function() {
		cy.cGet('#aichat-empty-state').should('contain.text', 'this spreadsheet');
		cy.cGet('#aichat-chip-formula-diagnosis').should('exist');
		cy.cGet('#aichat-chip-sanity-check').should('exist');
		cy.cGet('#aichat-chip-create-formula').should('exist');
		cy.cGet('#aichat-chip-0').should('exist');
		cy.cGet('#aichat-chip-3').should('not.exist');
		cy.cGet('#aichat-see-more button').click();
		helper.waitUntilLayoutingIsIdle(this.win);
		cy.cGet('#aichat-chip-3').should('exist');
		cy.cGet('#aichat-see-more').should('contain.text', 'See less');
	});

	it('See less hides them again', function() {
		cy.cGet('#aichat-see-more button').click();
		helper.waitUntilLayoutingIsIdle(this.win);
		cy.cGet('#aichat-chip-3').should('exist');
		cy.cGet('#aichat-see-more button').click();
		helper.waitUntilLayoutingIsIdle(this.win);
		cy.cGet('#aichat-chip-3').should('not.exist');
		cy.cGet('#aichat-see-more').should('contain.text', 'See more');
	});
});

describe(['tagdesktop'], 'Cell references in an assistant reply', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/aichat_formula.fods');
		cy.getFrameWindow().then((win) => {
			this.win = win;
			aichatHelper.enableAIAndStubSocket(win, {
				content: 'The lookup reads [Sheet1.A1:A2](cell://Sheet1.A1:A2).' +
					' The totals are in [Budget!A1:A2](cell://Budget!A1:A2).' +
					' The old range was Sheet1.B1:B2.',
			});
		});
		aichatHelper.openAIChat();
	});

	it('a link to a range on a named sheet selects that range', function() {
		aichatHelper.typeIntoAIInput('Where does the lookup read?');
		aichatHelper.clickSend();
		helper.waitUntilLayoutingIsIdle(this.win);

		cy.cGet('#aichat-msg-1 .aichat-cell-ref[data-cell="Sheet1.A1:A2"]').click();
		calcHelper.assertAddressAfterIdle(this.win, 'A1:A2');
	});

	it('a link to a range on another sheet shows that sheet and selects the range', function() {
		aichatHelper.typeIntoAIInput('Where are the totals?');
		aichatHelper.clickSend();
		helper.waitUntilLayoutingIsIdle(this.win);

		cy.cGet('.spreadsheet-tab.spreadsheet-tab-selected').should('have.text', 'Sheet1');
		cy.cGet('#aichat-msg-1 .aichat-cell-ref[data-cell="Budget!A1:A2"]').click();
		helper.processToIdle(this.win);
		cy.cGet('.spreadsheet-tab.spreadsheet-tab-selected').should('have.text', 'Budget');
		calcHelper.assertAddressAfterIdle(this.win, 'A1:A2');
	});

	it('a range written without a link stays plain text', function() {
		aichatHelper.typeIntoAIInput('Where does the lookup read?');
		aichatHelper.clickSend();
		helper.waitUntilLayoutingIsIdle(this.win);

		cy.cGet('#aichat-msg-1').should('contain.text', 'Sheet1.B1:B2');
		cy.cGet('#aichat-msg-1 a').should('not.exist');
	});
});

describe(['tagdesktop'], 'The cell under the cursor as context', function() {
	beforeEach(function() {
		// A1 holds 2 and A2 holds 3.
		helper.setupAndLoadDocument('calc/aichat_formula.fods');
		cy.getFrameWindow().then((win) => {
			this.win = win;
			setCellFormula(win, 'A3', '=SUM(A1,A2)');
			aichatHelper.enableAIWithCaptureSocket(win, {});
		});
		aichatHelper.openAIChat();
	});

	function lastUserContent(win) {
		var messages = win.__aichatPayloads[0].messages;
		return messages[messages.length - 1].content;
	}

	it('a formula cell sends its value and its formula', function() {
		calcHelper.enterCellAddressAndConfirm(this.win, 'A3');
		cy.getFrameWindow().should(function(win) {
			expect(win.app.map._docLayer._lastFormula).to.eq('=SUM(A1,A2)');
		});

		aichatHelper.typeIntoAIInput('Explain the formula');
		aichatHelper.clickSend();

		cy.getFrameWindow().should(function(win) {
			expect(win.__aichatPayloads).to.have.length(1);
			// The cell's value, 5, comes first and its formula follows.
			expect(lastUserContent(win)).to.match(
				/```\n\s*5\s+Formulas:\n- A3: =SUM\(A1,A2\)\n/);
		});
	});

	it('Explain Formula on a formula cell sends the cell', function() {
		calcHelper.enterCellAddressAndConfirm(this.win, 'A3');
		cy.getFrameWindow().should(function(win) {
			expect(win.app.map._docLayer._lastFormula).to.eq('=SUM(A1,A2)');
		});

		cy.getFrameWindow().then(function(win) {
			win.app.dispatcher.dispatch('aichatquick-explainformula');
		});

		cy.cGet('#aichat-panel').should('not.contain.text', 'Select the content you want');
		cy.getFrameWindow().should(function(win) {
			expect(win.__aichatPayloads).to.have.length(1);
			expect(lastUserContent(win)).to.contain('- A3: =SUM(A1,A2)');
		});
	});

	it('a cell with a plain value sends no cell', function() {
		calcHelper.enterCellAddressAndConfirm(this.win, 'A1');
		cy.getFrameWindow().should(function(win) {
			expect(win.app.map._docLayer._lastFormula).to.eq('2');
		});

		aichatHelper.typeIntoAIInput('What can you do?');
		aichatHelper.clickSend();

		cy.getFrameWindow().should(function(win) {
			expect(win.__aichatPayloads).to.have.length(1);
			expect(lastUserContent(win)).to.eq('What can you do?');
		});
	});
});

describe(['tagdesktop'], 'Formula error diagnosis', function() {
	beforeEach(function() {
		// A1 holds 2 and A2 holds 3, so B1 divides by zero.
		helper.setupAndLoadDocument('calc/aichat_formula.fods');
		cy.getFrameWindow().then((win) => {
			this.win = win;
			setCellFormula(win, 'B1', '=A1/(A2-3)');
			aichatHelper.enableAIWithCaptureSocket(win, {});
		});
		aichatHelper.openAIChat();
	});

	function diagnoseB1(win) {
		calcHelper.enterCellAddressAndConfirm(win, 'B1');
		cy.getFrameWindow().then(function(win) {
			win.app.dispatcher.dispatch('helpfixformulaerror');
		});
		// The request comes first and the card that asks about it follows.
		cy.cGet('#aichat-msg-0').should('contain.text', 'Diagnose formula error in B1');
		cy.cGet('#aichat-msg-1')
			.should('contain.text', 'these cells will be sent')
			.should('contain.text', 'Sheet1: A1, A2');
	}

	it('the cells are listed first and go out only after Send', function() {
		diagnoseB1(this.win);
		cy.getFrameWindow().should(function(win) {
			expect(win.__aichatPayloads).to.have.length(0);
		});

		cy.cGet('.aichat-approve-btn').click();

		cy.getFrameWindow().should(function(win) {
			expect(win.__aichatPayloads).to.have.length(1);
			var messages = win.__aichatPayloads[0].messages;
			expect(messages[messages.length - 1].content).to.contain('A2');
		});
	});

	it('Cancel sends nothing', function() {
		diagnoseB1(this.win);

		cy.cGet('.aichat-reject-btn').click();

		cy.cGet('#aichat-hint').should('contain.text', 'Nothing was sent.');
		cy.cGet('.aichat-approve-btn').should('not.exist');
		cy.getFrameWindow().should(function(win) {
			expect(win.__aichatPayloads).to.have.length(0);
		});
	});

	it('a cancelled diagnosis stays out of the next request', function() {
		diagnoseB1(this.win);
		cy.cGet('.aichat-reject-btn').click();

		// A1 holds a plain value, so the typed message carries no cell.
		calcHelper.enterCellAddressAndConfirm(this.win, 'A1');
		cy.getFrameWindow().should(function(win) {
			expect(win.app.map._docLayer._lastFormula).to.eq('2');
		});
		aichatHelper.typeIntoAIInput('What can you do?');
		aichatHelper.clickSend();

		cy.getFrameWindow().should(function(win) {
			expect(win.__aichatPayloads).to.have.length(1);
			var messages = win.__aichatPayloads[0].messages;
			expect(messages).to.have.length(1);
			expect(messages[0].content).to.eq('What can you do?');
		});
	});
});

describe(['tagdesktop'], 'A waiting diagnosis and another request', function() {
	beforeEach(function() {
		// A1 holds 2 and A2 holds 3, so B1 divides by zero.
		helper.setupAndLoadDocument('calc/aichat_formula.fods');
		cy.getFrameWindow().then((win) => {
			this.win = win;
			setCellFormula(win, 'B1', '=A1/(A2-3)');
			// Each request asks to inspect the document before it answers.
			aichatHelper.enableAIWithCaptureSocket(win, {
				approvalToolName: 'extract_document_structure',
			});
		});
		aichatHelper.openAIChat();
	});

	it('a sanity check withdraws the diagnosis and keeps its own approval', function() {
		calcHelper.enterCellAddressAndConfirm(this.win, 'B1');
		cy.getFrameWindow().then(function(win) {
			win.app.dispatcher.dispatch('helpfixformulaerror');
		});
		cy.cGet('#aichat-msg-1 .aichat-approve-btn').should('exist');

		calcHelper.enterCellAddressAndConfirm(this.win, 'A1:A2');
		cy.getFrameWindow().then(function(win) {
			win.app.dispatcher.dispatch('aichatcleanupdata');
		});

		// The diagnosis card no longer offers Send, and the approval the sanity
		// check asks for still has its buttons.
		cy.cGet('#aichat-msg-3').should('contain.text', 'Mock change');
		cy.cGet('#aichat-msg-1 .aichat-approve-btn').should('not.exist');
		cy.cGet('#aichat-msg-3 .aichat-reject-btn').click();

		cy.getFrameWindow().should(function(win) {
			expect(win.__aichatApprovePayloads).to.have.length(1);
			expect(win.__aichatPayloads).to.have.length(1);
			var messages = win.__aichatPayloads[0].messages;
			expect(messages).to.have.length(1);
			expect(messages[0].content).to.not.contain('formula error');
		});
	});
});
