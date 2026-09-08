/* -*- js-indent-level: 8 -*- */

/* global describe it cy require beforeEach expect */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Presenter Console.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/slide-bitmap-background.odp');
	});

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
	}

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
			return "test notes\nbullet one";
		}

		getNotesHtml() {
			return "<p>test notes</p>\r\n<ul>\r\n<li>bullet one</li>\r\n</ul>\r\n";
		}

		slideshowWindowCleanUp() {
			// empty body
		}

		endPresentation() {

		}
	}

	function fixedSizePreview() {
		return { width: 50, height: 50 };
	}

	// Returns the largest slide of the given shape that fits in the box.
	function fittedPreview(ratio) {
		return function(id, part, maxWidth, maxHeight) {
			if (maxWidth > maxHeight * ratio) {
				return { width: Math.round(maxHeight * ratio), height: maxHeight };
			}
			return { width: maxWidth, height: Math.round(maxWidth / ratio) };
		};
	}

	// The buttons under the current slide are the last thing in the window, so the
	// whole console is on screen when they are.
	function expectConsoleOnScreen(fakeConsole) {
		const consoleWindow = fakeConsole._proxyPresenter;
		const doc = consoleWindow.document;
		const controls = doc.querySelector('#slideshow-control-container');

		expect(controls.getBoundingClientRect().bottom).to.be.at.most(consoleWindow.innerHeight);

		// Nothing sticks out below the window, so the window does not scroll.
		expect(doc.documentElement.scrollHeight).to.be.at.most(consoleWindow.innerHeight);
	}

	// One line of text is about 1.2 times the font size tall, so anything under 1.5
	// times has not wrapped.
	function expectOneLine(elem) {
		const style = elem.ownerDocument.defaultView.getComputedStyle(elem);
		expect(elem.getBoundingClientRect().height)
			.to.be.at.most(parseFloat(style.fontSize) * 1.5);
	}

	// This opens the console on a two slide presentation. It returns the map the
	// console listens to along with the console itself.
	function openConsole(win, getPreview) {
		const FakeMap = win.L.Evented.extend({ getPreview: getPreview });
		const map = new FakeMap();
		const presenterConsole = new win.SlideShow.PresenterConsole(map, new FakePresenter());

		// create popup window
		map.fire('newpresentinconsole');
		expect(presenterConsole._proxyPresenter).to.not.equal(null);

		// start presentation
		map.fire('presentationinfo');

		return { map: map, presenterConsole: presenterConsole };
	}

	it('Presenter Console class', function() {
		cy.getFrameWindow().then(function(win) {
			var elem;
			const opened = openConsole(win, fixedSizePreview);
			const map = opened.map;
			const fakeConsole = opened.presenterConsole;

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

			// The notes keep their bullet list, and the markup carries no attributes.
			elem = fakeConsole._notes.querySelector('#notes');
			expect(elem.querySelectorAll('li')).to.have.lengthOf(1);
			expect(elem.querySelector('li').textContent).to.equal('bullet one');
			expect(elem.querySelector('li').attributes).to.have.lengthOf(0);

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

	it('Console fits the window it opens in', function() {
		cy.getFrameWindow().then(function(win) {
			const fakeConsole = openConsole(win, fittedPreview(16 / 9)).presenterConsole;

			// The timer and the clock are drawn on an animation frame, and they are the
			// tallest thing above the slide once they carry text.
			fakeConsole._drawClock();

			// The console measures itself while the window is still coming up. This
			// lays it out again now that the window has its real size.
			fakeConsole._onResize();

			expectConsoleOnScreen(fakeConsole);

			fakeConsole._proxyPresenter.close();
		});
	});

	it('Console fits the window beside a slide taller than it is wide', function() {
		cy.getFrameWindow().then(function(win) {
			// A portrait slide leaves the timer row a narrow width to sit in.
			const fakeConsole = openConsole(win, fittedPreview(9 / 16)).presenterConsole;
			const doc = fakeConsole._proxyPresenter.document;

			fakeConsole._drawClock();
			fakeConsole._onResize();

			// The timer and the clock stay on one line, so the row keeps its height.
			expectOneLine(doc.querySelector('#timer'));
			expectOneLine(doc.querySelector('#today'));

			expectConsoleOnScreen(fakeConsole);

			fakeConsole._proxyPresenter.close();
		});
	});
});
