/* global describe it cy beforeEach require */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Manage Changes filter by date and time', function () {

	beforeEach(function () {
		helper.setupAndLoadDocument('calc/empty-selections.ods');
		cy.getFrameWindow().then(function (win) {
			this.win = win;
		}.bind(this));
	});

	function startRecordingChanges(win) {
		cy.getFrameWindow().then(function (frameWindow) {
			frameWindow.app.map.sendUnoCommand('.uno:TraceChangeMode');
		});
		helper.processToIdle(win);
		helper.waitForMapState('.uno:TraceChangeMode', 'true');
	}

	function openManageChanges(win) {
		cy.getFrameWindow().then(function (frameWindow) {
			frameWindow.app.map.sendUnoCommand('.uno:AcceptChanges');
		});
		cy.cGet('#calcchanges').should('exist');
		helper.processToIdle(win);
	}

	function filterByDate(win, condition) {
		cy.cGet('#tabcontrol-2').click();
		cy.cGet('#date-input').check();
		helper.processToIdle(win);
		cy.cGet('#datecond-input').should('not.be.disabled');
		cy.cGet('#datecond-input').select(condition);
		helper.processToIdle(win);
	}

	it('the clock button fills in the current time, not only the date', function () {
		startRecordingChanges(this.win);
		openManageChanges(this.win);
		filterByDate(this.win, 'between');

		cy.cGet('#endtime').should('not.be.disabled');
		cy.cGet('#endtime').should('have.value', '');
		cy.cGet('#endclock-button').should('not.be.disabled');
		cy.cGet('#endclock-button').click();
		helper.processToIdle(this.win);

		cy.cGet('#enddate .unolabel').invoke('text').should('not.be.empty');
		// The time uses the document locale, which has a 24 hour clock.
		cy.cGet('#endtime').invoke('val').should('match', /^\d\d:\d\d(:\d\d)?$/);
	});

	it('a typed start time filters the list of changes', function () {
		startRecordingChanges(this.win);
		helper.typeIntoDocument('x{enter}');
		helper.processToIdle(this.win);

		openManageChanges(this.win);
		cy.cGet('#calcchanges .ui-treeview-entry').should('have.length', 1);

		// The change was made today, before the end of the day.
		filterByDate(this.win, 'since');
		cy.cGet('#starttime').should('not.be.disabled');
		cy.cGet('#starttime').type('{selectall}{backspace}23:59:59');

		// Leaving the field sends the time, and the filter is applied when the list tab is shown
		// again.
		cy.cGet('#tabcontrol-1').click();
		helper.processToIdle(this.win);

		cy.cGet('#calcchanges').should('have.class', 'empty');
		cy.cGet('#calcchanges .ui-treeview-entry').should('not.exist');
	});

	it('text that is no time shows what core read and the dialog stays open', function () {
		startRecordingChanges(this.win);
		openManageChanges(this.win);
		filterByDate(this.win, 'since');

		cy.cGet('#starttime').should('not.be.disabled');
		cy.cGet('#starttime').type('{selectall}{backspace}abc{enter}');
		helper.processToIdle(this.win);

		// The time parser skips characters that are no digits, as on the desktop.
		cy.cGet('#AcceptRejectChangesDialog').should('be.visible');
		cy.cGet('#starttime').should('have.value', '00:00');
	});

	it('Enter shows the parsed time and keeps the dialog open', function () {
		startRecordingChanges(this.win);
		openManageChanges(this.win);
		filterByDate(this.win, 'since');

		cy.cGet('#starttime').should('not.be.disabled');
		cy.cGet('#starttime').type('{selectall}{backspace}7.5{enter}');
		helper.processToIdle(this.win);

		cy.cGet('#AcceptRejectChangesDialog').should('be.visible');
		cy.cGet('#starttime').should('have.value', '07:05');
		cy.cGet('#starttime').type('{enter}');
		helper.processToIdle(this.win);
		cy.cGet('#AcceptRejectChangesDialog').should('be.visible');
	});
});
