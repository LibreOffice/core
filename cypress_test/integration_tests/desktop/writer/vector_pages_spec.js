/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

const helper = require('../../common/helper');

// A text document opened with the vector rendering draws its pages from the primitives the
// server sends for each page, and typing reaches the page as a pushed change, with no tiles.
describe(['tagdesktop'], 'Writer pages drawn from vector primitives', function() {
	let win;
	// The text messages sent to the server since the document loaded, oldest first.
	let sent;

	beforeEach(function() {
		const path = helper.setupDocument('writer/undo_redo.odt');
		helper.loadDocument(path, true, undefined, undefined, 'vector=true');
		cy.getFrameWindow().then(function(w) {
			win = w;
			// The standard load checks wait for tiles, and a vector view has none.
			cy.wrap(null).should(function() {
				expect(win.app.file.writer.pageIds).to.have.length.greaterThan(0);
			});
			helper.processToIdle(win);
			cy.then(function() {
				sent = [];
				const socket = w.app.socket;
				const doSend = socket._doSend;
				socket._doSend = function(message) {
					if (typeof message === 'string')
						sent.push(message);
					return doSend.apply(this, arguments);
				};
			});
		});
	});

	// The requests for tiles sent since the document loaded.
	function tileRequests() {
		return sent.filter(function(message) {
			return message.indexOf('tilecombine') === 0;
		});
	}

	// The cached content of the first page, once it arrived.
	function firstPage() {
		return cy.wrap(null).should(function() {
			const pageId = win.app.file.writer.pageIds[0];
			expect(win.RenderManager.writerVectorManager().requestTextPage(pageId)).to.not.be.undefined;
		}).then(function() {
			return win.RenderManager.writerVectorManager().requestTextPage(win.app.file.writer.pageIds[0]);
		});
	}

	// The text that the text portions of an entry show, one after the other.
	function textOf(primitives) {
		let text = '';
		(primitives || []).forEach(function(primitive) {
			if (typeof primitive.text === 'string') {
				const start = primitive.textPosition || 0;
				const length = primitive.textLength === undefined
					? primitive.text.length : primitive.textLength;
				text += primitive.text.substring(start, start + length);
			}
			text += textOf(primitive.children);
		});
		return text;
	}

	it('Draws the first page from its entries', function() {
		cy.wrap(null).should(function() {
			expect(win.RenderManager.drawsFromPrimitives()).to.be.true;
			// A text document leaves the editing of its objects to the server.
			expect(win.RenderManager.isVectorRendering()).to.be.false;
			// The section that draws the pages from their entries is in place.
			expect(win.app.sectionContainer.doesSectionExist(
				win.app.CSections.VectorContent.name)).to.be.true;
		});
		firstPage().then(function(page) {
			const pageEntry = page.objects.get(page.order[0]);
			expect(pageEntry.kind).to.equal('page');
			const kinds = page.order.map(function(id) { return page.objects.get(id).kind; });
			expect(kinds).to.include('body');
			expect(kinds).to.include('textframe');
		});
	});

	it('Shows typed text after the server pushes the change', function() {
		let versionBefore;
		firstPage().then(function(page) {
			versionBefore = page.version;
		});

		helper.typeIntoDocument('vectortyped');

		cy.wrap(null).should(function() {
			// Text shown before the server answers it waits as an edit typed here, so once none
			// waits the page shows what the server sent.
			expect(win.RenderManager._instance._typedEdits.has()).to.be.false;
			const page = win.RenderManager.writerVectorManager().requestTextPage(win.app.file.writer.pageIds[0]);
			expect(page.version).to.be.greaterThan(versionBefore);
			const text = page.order.map(function(id) {
				return textOf(page.objects.get(id).primitives);
			}).join('');
			expect(text).to.contain('vectortyped');
			expect(tileRequests()).to.deep.equal([]);
		});
	});
});
