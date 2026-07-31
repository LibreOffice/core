/* -*- js-indent-level: 8 -*- */

/* global describe it cy require beforeEach expect */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Presenter Console.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/slide-bitmap-background.odp');
	});

	it('Presenter Console class', function() {
		cy.getFrameWindow().then(function(win) {
			var elem;
			var fakeMap = win.L.Evented.extend({
				getPreview: function() {
					return {width: 50, height: 50};
				}
			});

			var map = new fakeMap();

			class FakeWindowProxy {
				addEventListener() {}
				close() {}
			}

			class FakeCompositor {
				computeLayerResolution(width, height) {
					return [width, height];
				}
				computeLayerSize(width, height) {
					return [width, height];
				}
			};

			class FakePresenter {
				constructor() {
					this._slideCompositor = new FakeCompositor();
					// fake window
					this._slideShowWindowProxy = new FakeWindowProxy();
				}

				_getSlidesCount() {
					return 2;
				}

				getVisibleSlidesCount() {
					this._getSlidesCount();
				}

				_getRepeatDuration() {
					return 0;
				}

				isSlideHidden() {
					return false;
				}

				getNextVisibleSlide(slideNumber) {
					return slideNumber + 1;
				}

				getNotes() {
					return "test notes";
				}

				slideshowWindowCleanUp() {
					// empty body
				}

				endPresentation() {

				}
			}

			var fakePresenter = new FakePresenter();
			var fakeConsole = new win.SlideShow.PresenterConsole(map, fakePresenter);

			// create popup window
			map.fire('newpresentinconsole');

			expect(fakeConsole._proxyPresenter).to.not.equal(null);

			// start presentation
			map.fire('presentationinfo');

			expect(fakeConsole._previews).to.have.lengthOf(2);

			{
				// create a frame image
				var width = 50;
				var height = 50;
				const offscreen = new OffscreenCanvas(width, height);
				const ctx = offscreen.getContext('2d');
				ctx.fillStyle = 'blue';
				ctx.fillRect(0, 0, width, height);

				map.fire('newslideshowframe', { frame: offscreen.transferToImageBitmap() });

				elem = fakeConsole._proxyPresenter.document.querySelector('#current-presentation');
				expect(elem).to.not.equal(null);
				expect(elem).to.not.equal(undefined);
			}

			map.fire('transitionend', { slide: 0 });

			elem = fakeConsole._proxyPresenter.document.querySelector('#next-presentation');
			expect(elem).to.not.equal(null);
			expect(elem).to.not.equal(undefined);

			elem = fakeConsole._proxyPresenter.document.querySelector('#notes');
			expect(elem).to.not.equal(null);
			expect(elem).to.not.equal(undefined);

			var smile = win.document.querySelector('meta[name="previewSmile"]').content;
			map.fire('tilepreview', {
				id : '2000',
				partIndex : 1,
				tile: { src: smile }
			});

			expect(fakeConsole._previews[1]).to.equal(smile);

			fakeConsole._proxyPresenter.close();
		});
	});
});
