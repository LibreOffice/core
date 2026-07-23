/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/*
 * window.L.Map.SlideShow is handling the slideShow action
 */

/* global _ sanitizeUrl app */
window.L.Map.mergeOptions({
	slideShow: true
});

window.L.Map.SlideShow = window.L.Handler.extend({

	_slideURL: '', // store the URL for svg
	_presentInWindow: false,
	_cypressSVGPresentationTest: false,
	_slideShowWindowProxy: null,

	initialize: function (map) {
		this._map = map;
		window.app.console.log('window.L.Map.SlideShow: Cypress in window: ' + ('Cypress' in window));
		this._cypressSVGPresentationTest =
			window.L.Browser.cypressTest && 'Cypress' in window
			&& window.Cypress.spec.name === 'impress/fullscreen_presentation_spec.js';
	},

	addHooks: function () {
		this._map.on('fullscreen', this._onFullScreen, this);
		this._map.on('presentinwindow', this._onPresentWindow, this);
		this._map.on('slidedownloadready', this._onSlideDownloadReady, this);
	},

	removeHooks: function () {
		this._map.off('fullscreen', this._onFullScreen, this);
		this._map.off('presentinwindow', this._onPresentWindow, this);
		this._map.off('slidedownloadready', this._onSlideDownloadReady, this);
	},

	_onFullScreen: function (e) {
		if (this._checkPresentationDisabled()) {
			this._notifyPresentationDisabled();
			return;
		}

		if (this._checkAlreadyPresenting()) {
			this._notifyAlreadyPresenting();
			return;
		}

		if (app.impress.areAllSlidesHidden()) {
			this._map.uiManager.showInfoModal('allslidehidden-modal', _('Empty Slide Show'),
					_('All slides are hidden!'), '', _('OK'), function () { }, false, 'allslidehidden-modal-response');
			return;
		}

		let doPresentation = function(that, e) {
			that._presentInWindow = false;

			that._startSlideNumber = 0; // Default: start from page 0
			if (typeof e.startSlideNumber !== 'undefined') {
				that._startSlideNumber = e.startSlideNumber;
			}
			that.fullscreen = !that._cypressSVGPresentationTest;
			that._map.downloadAs('slideshow.svg', 'svg', null, 'slideshow');

			window.L.DomEvent.on(document, 'fullscreenchange', that._onFullScreenChange, that);
		};

		let fallback = function(that, e) {
			// fallback to "open in new tab"
			if (that._slideShow) {
				window.L.DomUtil.remove(that._slideShow);
				that._slideShow = null;
			}

			doPresentation(that, e);
		};

		if (!(this._cypressSVGPresentationTest || this._map['wopi'].DownloadAsPostMessage)) {
			this._slideShow = window.L.DomUtil.create('iframe', 'leaflet-slideshow', this._map._container);
			if (this._slideShow.requestFullscreen) {

				let that = this;
				this._slideShow.requestFullscreen()
					.then(function () { doPresentation(that, e); })
					.catch(function () {
						fallback(that, e);
					});

				return;
			}
		}

		fallback(this, e);
	},

	_onPresentWindow: function (e) {
		if (this._checkPresentationDisabled()) {
			this._notifyPresentationDisabled();
			return;
		}

		if (this._checkAlreadyPresenting()) {
			this._notifyAlreadyPresenting();
			return;
		}

		this._presentInWindow = true;
		this._startSlideNumber = 0;
		if (e.startSlideNumber !== undefined) {
			this._startSlideNumber = e.startSlideNumber;
		}

		this._map.downloadAs('slideshow.svg', 'svg', null, 'slideshow');
	},

	_onFullScreenChange: function () {
		if (this._map['wopi'].DownloadAsPostMessage) {
			return;
		}

		this.fullscreen = document.fullscreenElement;
		if (!this.fullscreen) {
			this._stopFullScreen();
		}
	},

	_stopFullScreen: function () {
		window.L.DomUtil.remove(this._slideShow);
		this._slideShow = null;
		// #7102 on exit from fullscreen we don't get a 'focus' event
		// in Chrome so a later second attempt at launching a presentation
		// fails
		this._map.focus();
	},

	_onSlideDownloadReady: function (e) {
		if ('processCoolUrl' in window) {
			e.url = window.processCoolUrl({ url: e.url, type: 'slideshow' });
		}

		const embedURL = new URL(e.url);
		embedURL.searchParams.append('attachment', 0);

		this._slideURL = embedURL.toString();
		window.app.console.debug('slide file url : ', this._slideURL);

		this._startPlaying();
	},

	_startPlaying: function() {
		// Windowed Presentation
		if (this._presentInWindow) {

			this._slideShowWindowProxy = window.open('', '_blank', 'popup');

			if (!this._slideShowWindowProxy) {
				this._map.uiManager.showInfoModal('popup-blocked-modal',
					_('Windowed Presentation Blocked'),
					_('Presentation was blocked. Please allow pop-ups in your browser. This lets slide shows to be displayed in separated windows, allowing for easy screen sharing.'), '',
					_('OK'), null, false);
			}

			var popupTitle = _('Windowed Presentation: ') + this._map['wopi'].BaseFileName;
			this._slideShowWindowProxy.document.title = popupTitle;

			this._slideShowWindowProxy.document.body.style.margin = '0';
			this._slideShowWindowProxy.document.body.style.padding = '0';
			this._slideShowWindowProxy.document.body.style.height = '100%';
			this._slideShowWindowProxy.document.body.style.overflow = 'hidden'; // Prevent scrollbars.

			const iFrame = this._slideShowWindowProxy.document.createElement('iframe');
			iFrame.src = sanitizeUrl(this._slideURL);
			iFrame.style.width = '100%';
			iFrame.style.height = '100%';
			iFrame.style.border = 'none';
			this._slideShowWindowProxy.document.body.appendChild(iFrame);
			this._slideShow = iFrame;

			if ('processCoolUrl' in window) {
				this._processSlideshowLinks();
			}
			this._processSlideshowVideoForSafari();

			this._slideShowWindowProxy.document.close();
			this._slideShowWindowProxy.focus();

			this._slideShowWindowProxy.onload = this._handleSlideWindowLoaded.bind(this);

			// this event listener catches keypresses if the user somehow manages to unfocus the iframe
			this._slideShowWindowProxy.addEventListener('keydown', this._onSlideWindowKeyPress.bind(this));

			var slideShowWindow = this._slideShowWindowProxy;
			this._map.uiManager.showSnackbar(_('Presenting in another window'),
				_('Close Presentation'),
				function() {slideShowWindow.close();},
				-1, false, true);

			this._windowCloseInterval = setInterval(function() {
				if (slideShowWindow.closed) {
					clearInterval(this._windowCloseInterval);
					this._map.uiManager.closeSnackbar();
					this._slideShowWindowProxy = null;
					this._slideShow = null;
				}
			}.bind(this), 500);
			return;
		}

		if ('processCoolUrl' in window) {
			this._processSlideshowLinks();
		}
		this._processSlideshowVideoForSafari();

		// Cypress Presentation
		if (this._cypressSVGPresentationTest || !this._slideShow) {
			window.open(this._slideURL, '_self');
			return;
		}
		// Fullscreen Presentation

		this._map.uiManager.showSnackbar(_('Presenting in fullscreen'),
			_('End Presentation'),
			function() {this._stopFullScreen();},
			null, false, true);

		var separator = (this._slideURL.indexOf('?') === -1) ? '?' : '&';
		this._slideShow.src = this._slideURL + separator + 'StartSlideNumber=' + this._startSlideNumber;
		this._slideShow.contentWindow.focus();
	},

	_handleSlideWindowLoaded: function() {
		const iframe = this._slideShowWindowProxy.document.querySelector('iframe');

		if (iframe) {
			iframe.contentWindow.focus();
			iframe.contentWindow.addEventListener('keydown', this._onSlideWindowKeyPress.bind(this));
		}
	},

	_processSlideshowLinks: function() {
		this._slideShow.addEventListener('load', (function onLoadSlideshow() {
			var linkElements = [].slice.call(this._slideShow.contentDocument.querySelectorAll('a'))
				.filter(function(el) {
					return el.getAttribute('href') || el.getAttribute('xlink:href');
				})
				.map(function(el) {
					return {
						el: el,
						link: el.getAttribute('href') || el.getAttribute('xlink:href'),
						parent: el.parentNode
					};
				});

			var setAttributeNs = function(el, link) {
				link = window.processCoolUrl({ url: link, type: 'slide' });

				el.setAttributeNS('http://www.w3.org/1999/xlink', 'href', link);
				el.setAttribute('target', '_blank');
			};

			linkElements.forEach(function(item) {
				setAttributeNs(item.el, item.link);

				var parentLink = item.parent.getAttribute('xlink:href');

				if (parentLink) {
					setAttributeNs(item.parent, parentLink);
				}

				item.parent.parentNode.insertBefore(item.parent.cloneNode(true), item.parent.nextSibling);
				item.parent.parentNode.removeChild(item.parent);
			});
		}).bind(this));
	},

	_checkAlreadyPresenting: function() {
		if (this._slideShowWindowProxy && !this._slideShowWindowProxy.closed)
			return true;
		if (this._slideShow)
			return true;
		return false;
	},

	_notifyAlreadyPresenting: function() {
		this._map.uiManager.showInfoModal('already-presenting-modal',
			_('Already presenting'),
			_('You are already presenting this document'), '',
			_('OK'), null, false);
	},

	_checkPresentationDisabled: function() {
		return this._map['wopi'].DisablePresentation;
	},

	_notifyPresentationDisabled: function() {
		this._map.uiManager.showInfoModal('presentation-disabled-modal',
			_('Presentation disabled'),
			_('Presentation mode has been disabled for this document'), '',
			_('OK'), null, false);
	},

	_onSlideWindowKeyPress: function(e) {
		if (e.code === 'Escape') {
			this._slideShowWindowProxy.opener.focus();
			this._slideShowWindowProxy.close();
			this._map.uiManager.closeSnackbar();
		}
	},

	_processSlideshowVideoForSafari: function() {
		// There is an issue where Safari without LBSE renders the video in the wrong place, so we
		// must move it back into frame
		// GH#7399 fixed the same issue, but not in presentation mode
		if (!window.L.Browser.safari) {
			return;
		}

		if (!this._slideShow) {
			console.error('In Safari without fixing slideshow videos, this may cause videos to be offset');
			return;
		}

		this._slideShow.addEventListener('load', (function onLoadSlideshow() {
			var videos = this._slideShow.contentDocument.querySelectorAll('video');

			var fixSVGPos = function(video) {
				var videoContainer = video.parentNode;
				var foreignObject = videoContainer.parentNode;
				var svg = foreignObject.closest('svg');

				return function() {
					var widthRatio = svg.width.baseVal.value / svg.viewBox.baseVal.width;
					var heightRatio = svg.height.baseVal.value / svg.viewBox.baseVal.height;
					var minRatio = Math.min(widthRatio, heightRatio);

					var leftRightBorders = svg.width.baseVal.value - minRatio * svg.viewBox.baseVal.width;
					var topBottomBorders = svg.height.baseVal.value - minRatio * svg.viewBox.baseVal.height;

					// revert the scaling positioning to center (back to top-left) by subtracting at 1/2 of width
					var offsetX = -foreignObject.width.baseVal.value / 2.0;
					var offsetY = -foreignObject.height.baseVal.value / 2.0;

					// revert the object position
					offsetX = offsetX - foreignObject.x.baseVal.value + (leftRightBorders / 4.0);
					offsetY = offsetY - foreignObject.y.baseVal.value + (topBottomBorders / 4.0);

					// reapply the scaling positioning to center by adding 1/2 of the non-scaled width
					offsetX = offsetX + (foreignObject.width.baseVal.value * widthRatio / 2.0);
					offsetY = offsetY + (foreignObject.height.baseVal.value * heightRatio / 2.0);

					// reapply the object positioning
					offsetX = offsetX + foreignObject.x.baseVal.value * widthRatio;
					offsetY = offsetY + foreignObject.y.baseVal.value * heightRatio;

					var scaleString = 'scale(' + minRatio + ')';
					var translateString = 'translate(' + offsetX + 'px, ' + offsetY + 'px)';

					videoContainer.style.transform = translateString + ' ' + scaleString;
				};
			};

			for (var i = 0; i < videos.length; i++) {
				var fixThisVideoPos = fixSVGPos(videos[i]);

				fixThisVideoPos();
				var observer = new MutationObserver(fixThisVideoPos);

				observer.observe(this._slideShow, {
					attributes: true
				});

				this._slideShow.contentDocument.defaultView.addEventListener('resize', fixThisVideoPos);
			}
		}).bind(this));
	}
});

window.L.Map.addInitHook('addHandler', 'slideShow', window.L.Map.SlideShow);
