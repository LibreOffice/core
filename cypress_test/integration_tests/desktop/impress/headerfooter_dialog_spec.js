/* global describe it cy beforeEach expect require */

const helper = require('../../common/helper');

function recordWidgets(node, widgets, nested, insideDisabled) {
	if (!node || typeof node !== 'object')
		return;

	if (node.id) {
		widgets.set(node.id, node);
		if (insideDisabled)
			nested.add(node.id);
		else
			nested.delete(node.id);
	}

	if (!node.children)
		return;

	const inside = insideDisabled || node.enabled === false;
	for (let i = 0; i < node.children.length; i++)
		recordWidgets(node.children[i], widgets, nested, inside);
}

function dialogRecorder(state) {
	return (e) => {
		if (e.data.jsontype !== 'dialog')
			return;
		if (state.dialogId === null)
			state.dialogId = e.data.id;
		if (e.data.id !== state.dialogId)
			return;

		recordWidgets(e.data.control || e.data, state.widgets, state.nested, false);
	};
}

function isRendered(element) {
	return !!element && element.getClientRects().length > 0;
}

function colorOf(element) {
	return element.ownerDocument.defaultView.getComputedStyle(element).color;
}

function liveLabelColor(widgets, doc) {
	let color = null;

	widgets.forEach(function(widget, id) {
		if (color !== null || widget.type !== 'fixedtext' || widget.enabled === false)
			return;

		const label = doc.getElementById(id);
		if (isRendered(label))
			color = colorOf(label);
	});

	return color;
}

describe(['tagdesktop'], 'Impress Header and Footer dialog', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('impress/help_dialog.odp');

		cy.getFrameWindow().then((win) => {
			this.win = win;
		});

		// Core has to go quiet before the recorder is installed: the sidebar and
		// the busy popup of the load arrive through the same events, and the
		// recorder would latch onto one of those instead of the dialog.
		cy.then(() => {
			return helper.processToIdle(this.win);
		});

		cy.then(() => {
			this.widgets = new Map();
			this.nested = new Set();
			this.dialogId = null;

			const record = dialogRecorder(this);

			this.win.app.map.on('jsdialog', record);
			this.win.app.map.on('jsdialogupdate', record);

			this.win.app.map.sendUnoCommand('.uno:HeaderAndFooter');
		});

		cy.cGet('.jsdialog-window').should('exist');
		cy.then(() => {
			return helper.processToIdle(this.win);
		});

		cy.then(() => {
			cy.cGet('[id="' + this.dialogId + '"] .ui-dialog-title')
				.should('have.text', 'Header and Footer');
		});
	});

	it('opens with the label of a disabled field disabled too', function() {
		cy.then(() => {
			const dialog = this.win.document.getElementById(this.dialogId);
			expect(dialog, 'the dialog core built, id ' + this.dialogId).to.not.be.null;

			const doc = dialog.ownerDocument;
			const live = liveLabelColor(this.widgets, doc);

			this.widgets.forEach((field, id) => {
				if (field.enabled !== false || !field.labelledBy)
					return;

				const labelData = this.widgets.get(field.labelledBy);
				if (!labelData)
					return;

				const which = 'label "' + labelData.text + '" (' + field.labelledBy +
				              ') of the disabled ' + id;

				expect(labelData.enabled, which).to.equal(false);

				if (this.nested.has(field.labelledBy))
					return;

				const label = doc.getElementById(field.labelledBy);
				const control = doc.getElementById(id);
				if (!isRendered(label) || !isRendered(control))
					return;

				expect(colorOf(label), which + ', against the field itself')
					.to.equal(colorOf(control));
				expect(colorOf(label), which + ', against a label still in use')
					.to.not.equal(live);
			});
		});
	});
});
