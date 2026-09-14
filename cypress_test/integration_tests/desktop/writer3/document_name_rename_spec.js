/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

// A document name is one name, not a path. A host is free to take the name it is
// given literally, so a slash in it moves the document into a folder of that name
// instead of renaming it: the name field has to turn it down before it is sent.
describe(['tagdesktop'], 'Document rename', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/document_name_rename.odt',
			/* isMultiUser */ false, /* copy the .wopi.json next to it */ true);

		// Record what the name field puts on the wire.
		cy.getFrameWindow().then(function(win) {
			win.__sent = [];
			var send = win.app.socket.sendMessage.bind(win.app.socket);
			win.app.socket.sendMessage = function(msg) {
				win.__sent.push(msg);
				return send.apply(null, arguments);
			};
		});
	});

	function renameTo(name) {
		cy.cGet('#document-name-input').should('have.class', 'editable');
		cy.cGet('#document-name-input').clear();
		cy.cGet('#document-name-input').type(name + '{enter}');
	}

	it('a name with a slash is refused instead of being sent', function() {
		renameTo('Suivi test 20/05/2025');

		cy.cGet('#modal-dialog-invalid-document-name').should('exist');

		cy.getFrameWindow().then(function(win) {
			var sent = win.__sent.filter(function(msg) {
				return msg.startsWith('renamefile') || msg.startsWith('saveas');
			});
			expect(sent, 'rename requests').to.deep.equal([]);
		});
	});

	it('a name with a backslash is refused as well', function() {
		renameTo('Suivi test 20\\05\\2025');

		cy.cGet('#modal-dialog-invalid-document-name').should('exist');

		cy.getFrameWindow().then(function(win) {
			var sent = win.__sent.filter(function(msg) {
				return msg.startsWith('renamefile') || msg.startsWith('saveas');
			});
			expect(sent, 'rename requests').to.deep.equal([]);
		});
	});

	it('a name of just a dot is refused too', function() {
		renameTo('.');

		cy.cGet('#modal-dialog-invalid-document-name').should('exist');

		cy.getFrameWindow().then(function(win) {
			var sent = win.__sent.filter(function(msg) {
				return msg.startsWith('renamefile') || msg.startsWith('saveas');
			});
			expect(sent, 'rename requests').to.deep.equal([]);
		});
	});

	// The name field is not the only way a rename request reaches the server. A
	// request the server turns down is reported in the words the reader needs,
	// not as the command and the error kind the message carried.
	it('a rename the server refuses is reported in plain words', function() {
		cy.getFrameWindow().then(function(win) {
			win.app.map.renameFile('.');
		});

		cy.cGet('#modal-dialog-cool_alert').should('exist');
		cy.cGet('#modal-dialog-cool_alert')
			.should('contain.text', 'Please enter a valid document name.');
	});

	it('an ordinary name is still sent as a rename', function() {
		renameTo('Suivi test 20-05-2025');

		cy.cGet('#modal-dialog-invalid-document-name').should('not.exist');

		cy.getFrameWindow().then(function(win) {
			var sent = win.__sent.filter(function(msg) {
				return msg.startsWith('renamefile');
			});
			expect(sent.length, 'rename requests').to.equal(1);
		});
	});
});
