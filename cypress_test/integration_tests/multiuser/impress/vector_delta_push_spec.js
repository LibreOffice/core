/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

describe(['tagmultiuser'], 'Multiuser vector delta push', function() {
	var win1, win2;

	// The stock load checks wait for bitmap tiles, which a vector
	// view never paints, so wait for the document connection directly.
	function waitForVectorView(win) {
		cy.wrap(null, {timeout: 60000}).should(function() {
			expect(win.app).to.exist;
			expect(win.app.map).to.exist;
			expect(win.app.map._docLayer).to.exist;
			expect(win.app.impress.partList).to.have.length.greaterThan(0);
		});
	}

	beforeEach(function() {
		var newFilePath = helper.setupDocument('impress/slide_operations.odp');
		helper.loadDocument(newFilePath, true, true, undefined, 'vector=true');
		cy.getFrameWindow('#iframe1').then(function(win) { win1 = win; });
		cy.getFrameWindow('#iframe2').then(function(win) { win2 = win; });
	});

	it('an edit in one view reaches the other view as a pushed delta', function() {
		cy.cSetActiveFrame('#iframe1');
		waitForVectorView(win1);
		helper.processToIdle(win1);
		cy.cSetActiveFrame('#iframe2');
		waitForVectorView(win2);
		helper.processToIdle(win2);

		// Record every delta User B receives and every message B sends
		// from here on. A pushed delta arrives through the command
		// values path once the binary frame is decompressed.
		cy.then(function() {
			var docLayer = win2.app.map._docLayer;
			win2.__deltas = [];
			var originalOnCommandValues = docLayer._onCommandValuesMsg;
			docLayer._onCommandValuesMsg = function(textMsg) {
				originalOnCommandValues.call(docLayer, textMsg);
				if (textMsg.indexOf('vectorprimitivesdelta') >= 0)
					win2.__deltas.push(textMsg);
			};

			win2.__sent = [];
			var originalSendMessage = win2.app.socket.sendMessage;
			win2.app.socket.sendMessage = function(msg) {
				win2.__sent.push(String(msg));
				return originalSendMessage.apply(this, arguments);
			};
		});

		// User A inserts a rectangle, which changes the slide.
		cy.cSetActiveFrame('#iframe1');
		cy.then(function() {
			win1.app.map.sendUnoCommand('.uno:BasicShapes.rectangle');
		});
		helper.processToIdle(win1);

		// The change reaches User B as a pushed delta, without B asking
		// for anything.
		cy.cSetActiveFrame('#iframe2');
		helper.processToIdle(win2);
		cy.wrap(null).should(function() {
			expect(win2.__deltas.length).to.be.greaterThan(0);
		});
		cy.then(function() {
			var vectorRequests = win2.__sent.filter(function(msg) {
				return msg.indexOf('.uno:VectorPrimitives') >= 0;
			});
			expect(vectorRequests).to.be.empty;
		});
	});
});
