/* -*- js-indent-level: 8 -*- */
/* global cy require expect Cypress */

var helper = require('./helper');
var desktopHelper = require('./desktop_helper');

// The form controls of impress/form_controls.fodp, by their name in the form, with the middle of
// each on the slide in centimetres.
var places = {
	agree: { x: 4.5, y: 2 },
	name: { x: 4.5, y: 3.5 },
	amount: { x: 4.5, y: 5 },
	yes: { x: 4.5, y: 6.5 },
	colour: { x: 4.5, y: 8 },
	count: { x: 11.5, y: 3 },
	press: { x: 11.5, y: 5.5 },
	day: { x: 11.5, y: 7 },
	hour: { x: 11.5, y: 8.5 },
	fixed: { x: 18.5, y: 2 },
	off: { x: 18.5, y: 3.5 },
	caption: { x: 18.5, y: 5 },
	phone: { x: 18.5, y: 6.5 },
	size: { x: 18.5, y: 8 },
};

// The kind of object a form control is.
var formControlKind = 32;

// The messages sent to the server, oldest first, for each window that records them.
var sentBy = new WeakMap();

/*
	Opens impress/form_controls.fodp in a view drawn from vector primitives, and records the
	messages sent to the server from then on. Calls back with the frame window. The document opens
	for editing with the notebookbar, or read-only when readOnly is true.
*/
function loadFormDocument(onWindow, readOnly) {
	cy.viewport(1920, 1080);
	var path = helper.setupDocument('impress/form_controls.fodp');
	helper.loadDocument(path, true, undefined, undefined,
		'vector=true' + (readOnly ? '&permission=readonly' : ''));
	cy.getFrameWindow().then(function(win) {
		// The usual load checks wait for tiles, which a vector view does not have.
		cy.wrap(null).should(function() {
			expect(win.app.impress.partList).to.have.length.greaterThan(0);
		});
		cy.then(function() {
			var sent = [];
			sentBy.set(win, sent);
			var socket = win.app.socket;
			var doSend = socket._doSend;
			socket._doSend = function(message) {
				if (typeof message === 'string')
					sent.push(message);
				return doSend.apply(this, arguments);
			};
		});
		cy.then(function() {
			return helper.processToIdle(win);
		});
		cy.then(function() {
			onWindow(win);
		});
	});
	if (!readOnly)
		desktopHelper.switchUIToNotebookbar();
}

// The section that keeps the geometry of the objects the client holds.
function geometry(win) {
	return win.app.sectionContainer.getSectionWithName(
		win.app.CSections.RenderGeometry.name).constructor;
}

// The ids of the form controls on the slide.
function formObjects(win) {
	return geometry(win).heldByThePage().filter(function(objectId) {
		var object = geometry(win).objectOf(objectId);
		return object && object.objectKind === formControlKind;
	});
}

// The id of the selected object, or undefined when not exactly one is selected.
function selectedObject(win) {
	var section = win.app.sectionContainer.sections.find(function(section) {
		return section.name.indexOf('selected-object-') === 0;
	});
	var selected = section ? section.selectedObjects() : [];
	return selected.length === 1 ? selected[0] : undefined;
}

// Whether that object is a form control.
function isFormControl(win, objectId) {
	var object = geometry(win).objectOf(objectId);
	return !!object && object.objectKind === formControlKind;
}

// A place on the page in twips, in CSS pixels from the top left corner of the document canvas.
function twipsOnCanvas(win, x, y) {
	var visible = win.app.activeDocument.activeLayout.viewedRectangle;
	var scale = win.app.twipsToPixels / win.app.dpiScale;
	return {
		x: x * scale - visible.pX1 / win.app.dpiScale,
		y: y * scale - visible.pY1 / win.app.dpiScale,
	};
}

// The place of a form control on the document canvas, in CSS pixels from its top left corner.
function onCanvas(win, name) {
	var twipsPerCentimetre = 1440 / 2.54;
	return twipsOnCanvas(win, places[name].x * twipsPerCentimetre,
		places[name].y * twipsPerCentimetre);
}

// Drags from one place to another, in twips on the page, in steps.
function dragOnCanvas(win, from, to) {
	cy.then(function() {
		var start = twipsOnCanvas(win, from.x, from.y);
		var drawing = cy.cGet('#document-canvas').trigger('mousedown', start.x, start.y,
			{ button: 0, buttons: 1 });
		for (var step = 1; step <= 10; ++step) {
			var place = twipsOnCanvas(win, from.x + step / 10 * (to.x - from.x),
				from.y + step / 10 * (to.y - from.y));
			drawing = drawing.trigger('mousemove', place.x, place.y, { button: 0, buttons: 1 });
		}
		var end = twipsOnCanvas(win, to.x, to.y);
		drawing.trigger('mouseup', end.x, end.y, { button: 0, buttons: 0 });
	});
	cy.then(function() {
		return helper.processToIdle(win);
	});
}

// The element over the page that works as that form control, or null where none lies there.
function controlAt(win, name) {
	var place = onCanvas(win, name);
	var canvas = win.document.getElementById('document-canvas').getBoundingClientRect();
	var element = win.document.elementFromPoint(canvas.left + place.x, canvas.top + place.y);
	return element ? element.closest('.vector-form-control') : null;
}

// Yields the element of that form control once it is there.
function getControl(win, name) {
	return cy.wrap(null).should(function() {
		expect(controlAt(win, name), 'the element of ' + name).to.not.equal(null);
	}).then(function() {
		return controlAt(win, name);
	});
}

// Yields the part of that form control that matches the selector, once it is there.
function getPart(win, name, selector) {
	return cy.wrap(null).should(function() {
		var control = controlAt(win, name);
		expect(control, 'the element of ' + name).to.not.equal(null);
		expect(control.querySelector(selector), selector + ' of ' + name).to.not.equal(null);
	}).then(function() {
		return controlAt(win, name).querySelector(selector);
	});
}

/*
	Types into the field of that form control with the keyboard, as a person does: a click puts
	the focus in the field, and Ctrl+A first selects the text there when the text is to be
	replaced.
*/
function typeInto(win, name, text, replace) {
	getPart(win, name, '.vector-form-field').click();
	if (replace)
		cy.realPress(['Control', 'a']);
	cy.realType(text);
}

// Leaves the field of that form control, so it sends what was typed.
function leaveField(win, name) {
	getPart(win, name, '.vector-form-field').then(function(field) {
		Cypress.$(field)[0].blur();
	});
}

// Clicks that form control on the canvas, where it is an object of the slide.
function clickOnCanvas(win, name) {
	cy.then(function() {
		var place = onCanvas(win, name);
		cy.cGet('#document-canvas').click(place.x, place.y);
	});
	cy.then(function() {
		return helper.waitForTimers(win, 'clicktimer');
	});
	cy.then(function() {
		return helper.processToIdle(win);
	});
}

// Forgets the messages sent so far.
function forgetSent(win) {
	cy.then(function() {
		sentBy.get(win).length = 0;
	});
}

// Waits for a form control message with that action and value, and yields the id of the control
// it was sent for. The value is the text sent, or a regular expression for it.
function expectSent(win, action, value) {
	var valuePattern = value instanceof RegExp ? value.source
		: encodeURIComponent(value).replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
	var pattern = new RegExp('^formcontrol id=(\\d+) action=' + action + ' value=' +
		valuePattern + '$');
	var found;
	return cy.wrap(null).should(function() {
		found = sentBy.get(win).map(function(message) {
			return pattern.exec(message);
		}).find(Boolean);
		expect(found, 'formcontrol action=' + action + ' value=' + value).to.not.equal(undefined);
	}).then(function() {
		return Number(found[1]);
	});
}

// Waits for the server to finish its work, and checks that no form control message was sent.
function expectNoFormControlSent(win) {
	cy.then(function() {
		return helper.processToIdle(win);
	});
	cy.then(function() {
		var sent = sentBy.get(win).filter(function(message) {
			return message.indexOf('formcontrol ') === 0;
		});
		expect(sent, 'form control messages').to.deep.equal([]);
	});
}

// Waits for the design mode of the form controls to be on or off, as the server reports it.
function waitForDesignMode(on) {
	cy.getFrameWindow().should(function(win) {
		var state = win.app.map['stateChangeHandler'].getItemValue('.uno:SwitchControlDesignMode');
		expect(String(state), 'the design mode').to.equal(on ? 'true' : 'false');
	});
}

// Clicks a button of the Form tab and waits for the server.
function clickFormTabButton(win, command) {
	desktopHelper.selectNotebookbarTab('Form');
	desktopHelper.getNbItem(command, 'Form').should('not.have.attr', 'disabled');
	desktopHelper.getNbIcon(command, 'Form').click();
	cy.then(function() {
		return helper.processToIdle(win);
	});
}

// Switches the design mode of the form controls on or off with the button of the Form tab.
function switchDesignMode(win, on) {
	waitForDesignMode(!on);
	clickFormTabButton(win, 'SwitchControlDesignMode');
	waitForDesignMode(on);
}

// The element that has the focus in the document frame.
function focused(win) {
	return win.document.activeElement;
}

// Waits for the focus to be in the element of that form control.
function expectFocusIn(win, name) {
	cy.wrap(null).should(function() {
		var control = controlAt(win, name);
		expect(control, 'the element of ' + name).to.not.equal(null);
		expect(control.contains(focused(win)), 'the focus in ' + name).to.equal(true);
	});
}

module.exports.loadFormDocument = loadFormDocument;
module.exports.formObjects = formObjects;
module.exports.selectedObject = selectedObject;
module.exports.isFormControl = isFormControl;
module.exports.controlAt = controlAt;
module.exports.getControl = getControl;
module.exports.getPart = getPart;
module.exports.typeInto = typeInto;
module.exports.leaveField = leaveField;
module.exports.clickOnCanvas = clickOnCanvas;
module.exports.dragOnCanvas = dragOnCanvas;
module.exports.forgetSent = forgetSent;
module.exports.expectSent = expectSent;
module.exports.expectNoFormControlSent = expectNoFormControlSent;
module.exports.waitForDesignMode = waitForDesignMode;
module.exports.clickFormTabButton = clickFormTabButton;
module.exports.switchDesignMode = switchDesignMode;
module.exports.focused = focused;
module.exports.expectFocusIn = expectFocusIn;
