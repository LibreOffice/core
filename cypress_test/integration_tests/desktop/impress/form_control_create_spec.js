/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var desktopHelper = require('../../common/desktop_helper');
var formHelper = require('../../common/form_controls_helper');

// Creating a form control on a vector slide by dragging out its rectangle. The controls of the
// test document lie in the upper part of the slide; the part below them is empty.
describe(['tagdesktop'], 'Dragging out a form control on a vector slide', function() {

	beforeEach(function() {
		formHelper.loadFormDocument(function(win) {
			this.win = win;
		}.bind(this));
	});

	// The section that keeps the geometry of the objects the client holds.
	function geometry(win) {
		return win.app.sectionContainer.getSectionWithName(
			win.app.CSections.RenderGeometry.name).constructor;
	}

	it('A control of the Form tab is dragged out where it is to go and selected.', function() {
		var win = this.win;
		var before;
		cy.wrap(null).should(function() {
			before = formHelper.formObjects(win);
			expect(before, 'the controls of the slide').to.have.length(14);
		});
		formHelper.switchDesignMode(win, true);

		// in design mode the button of a control starts the drag, and nothing is made yet
		formHelper.clickFormTabButton(win, 'Pushbutton');
		cy.wrap(null).should(function() {
			var mode = win.eval('Interaction.modeOn()');
			expect(mode && mode.constructor.name, 'the mode').to.equal('ClassicFormControlCreateContext');
			expect(formHelper.formObjects(win), 'the controls of the slide').to.have.length(14);
		});

		// the rectangle the client asks for, as the drag gave it
		var sent = null;
		cy.then(function() {
			var sendUnoCommand = win.app.map.sendUnoCommand;
			win.app.map.sendUnoCommand = function(command, args) {
				if (command === '.uno:CreateControl')
					sent = args;
				return sendUnoCommand.apply(this, arguments);
			};
		});

		// a drag in the empty part below the controls, from 3 cm by 11 cm to 8 cm by 12 cm
		formHelper.dragOnCanvas(win, { x: 1701, y: 6236 }, { x: 4535, y: 6803 });
		cy.wrap(null).should(function() {
			expect(sent, 'the rectangle sent').to.not.equal(null);
			var left = Number(sent.X.value);
			var top = Number(sent.Y.value);
			var right = left + Number(sent.Width.value);
			var bottom = top + Number(sent.Height.value);
			// the drag snaps as for a shape, so the rectangle lies near the places dragged
			expect(Math.abs(left - 1701), 'the left edge sent').to.be.below(300);
			expect(Math.abs(bottom - 6803), 'the bottom edge sent').to.be.below(300);

			var made = formHelper.formObjects(win).filter(function(objectId) {
				return before.indexOf(objectId) < 0;
			});
			expect(made, 'the controls made').to.have.length(1);
			// the engine makes the control in that rectangle, which it holds in its own units
			var box = geometry(win).boxOf(made[0]);
			expect(Math.abs(box.minX - left), 'the left edge').to.be.below(5);
			expect(Math.abs(box.minY - top), 'the top edge').to.be.below(5);
			expect(Math.abs(box.maxX - right), 'the right edge').to.be.below(5);
			expect(Math.abs(box.maxY - bottom), 'the bottom edge').to.be.below(5);
			expect(formHelper.selectedObject(win), 'the selected object').to.equal(made[0]);
			expect(win.eval('Interaction.modeOn()'), 'the mode').to.equal(null);
		});

		// out of design mode the tab offers no control to make
		formHelper.switchDesignMode(win, false);
		desktopHelper.getNbItem('Pushbutton', 'Form').should('have.attr', 'disabled');
	});
});
