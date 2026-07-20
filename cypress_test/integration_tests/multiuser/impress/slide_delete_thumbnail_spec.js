/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

describe(['tagmultiuser'], 'Multiuser slide delete thumbnails', function() {
	var win1, win2;

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/impress-edit.fodp', true);
		cy.getFrameWindow('#iframe1').then(function(win) { win1 = win; });
		cy.getFrameWindow('#iframe2').then(function(win) { win2 = win; });
	});

	// Map each slide's identity to the thumbnail image it currently shows,
	// skipping slides whose preview is still a placeholder (not yet fetched).
	function thumbnailsByHash(win) {
		var preview = win.app.map._docLayer._preview;
		var ph = win.document.querySelector('meta[name="previewSmile"]').content;
		var ph2 = win.document.querySelector('meta[name="previewImg"]').content;
		var map = {};
		preview._previewTiles.forEach(function(img) {
			var src = img.src || '';
			if (src !== ph && src !== ph2)
				map[img.hash] = src;
		});
		return map;
	}

	it('another view deleting a slide does not misplace the thumbnails', function() {
		cy.cSetActiveFrame('#iframe1');
		cy.wrap(null).should(function() {
			expect(win1.app.impress.partList.length).to.be.greaterThan(11);
		});
		cy.cSetActiveFrame('#iframe2');
		cy.wrap(null).should(function() {
			expect(win2.app.impress.partList.length).to.be.greaterThan(11);
		});

		// User A goes to slide 12 (index 11) and stays there.
		cy.cSetActiveFrame('#iframe1');
		cy.cGet('#preview-img-part-11').scrollIntoView().click();
		helper.processToIdle(win1);
		cy.wrap(null).should(function() {
			expect(win1.app.map.getCurrentPartNumber()).to.equal(11);
		});
		// Give the visible thumbnails time to render.
		cy.wait(2000);
		cy.then(function() {
			win1.__before = thumbnailsByHash(win1);
			win1.__deletedHash = String(win1.app.impress.partList[1].hash);
			// The test is only meaningful if A actually has some real thumbnails.
			expect(Object.keys(win1.__before).length).to.be.greaterThan(0);
		});

		// User B deletes slide 2 (index 1).
		cy.cSetActiveFrame('#iframe2');
		cy.cGet('#preview-img-part-1').scrollIntoView().click();
		cy.wrap(null).should(function() {
			expect(win2.app.map.getCurrentPartNumber()).to.equal(1);
		});
		cy.then(function() {
			win2.__countBefore = win2.app.impress.partList.length;
			win2.app.map.deletePage();
		});
		helper.processToIdle(win2);
		cy.wrap(null).should(function() {
			expect(win2.app.impress.partList.length).to.equal(win2.__countBefore - 1);
		});

		// Let User A finish reacting, and let any preview refetch settle.
		cy.cSetActiveFrame('#iframe1');
		helper.processToIdle(win1);
		cy.wait(3000);

		cy.wrap(null).should(function() {
			var before = win1.__before;
			var after = thumbnailsByHash(win1);

			// The deleted slide's thumbnail is gone.
			expect(after).to.not.have.property(win1.__deletedHash);

			// Every slide that had a thumbnail still has one: none went blank.
			// The renumbering shifts each slide's own number, so its picture may
			// change; the test allows that but requires the slide to keep a
			// thumbnail of its own.
			Object.keys(before).forEach(function(hash) {
				if (hash === win1.__deletedHash)
					return;
				expect(after, 'slide ' + hash + ' still has a thumbnail').to.have.property(hash);
			});

			// No slide shows another slide's picture. The bug placed the current
			// slide's image on a neighbour and a stale image on the current slide,
			// so a surviving slide displaying any other slide's original thumbnail
			// is the failure we guard against.
			Object.keys(after).forEach(function(shownHash) {
				Object.keys(before).forEach(function(otherHash) {
					if (shownHash === otherHash)
						return;
					expect(after[shownHash],
						'slide ' + shownHash + ' must not show slide ' + otherHash + ' image')
						.to.not.equal(before[otherHash]);
				});
			});
		});
	});
});
