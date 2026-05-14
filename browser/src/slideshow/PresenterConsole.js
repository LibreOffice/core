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
 * PresenterConsole
 */

/* global app SlideShow _ LOUtil */

class PresenterConsole {
	constructor(map, presenter) {
		this._map = map;
		this._presenter = presenter;
		this._active = false;
		this._map.on('presentationinfo', this._onPresentationInfo, this);
		this._map.on('newpresentinconsole', this._onPresentInConsole, this);
	}

	_generateHtml(title) {
		this.labels = {
			currentSlide: _('Current Slide'),
			nextSlide: _('Next Slide'),
			previous: _('Previous'),
			next: _('Next'),
			notes: _('Notes'),
			slides: _('Slides'),
			exchange: _('Exchange'),
			pause: _('Pause'),
			restart: _('Restart'),
			resume: _('Resume'),
			presentToAll: _('Present to all'),
			goBack: _('Go Back'),
			zoomIn: _('Zoom In'),
			zoomOut: _('Zoom Out'),
		};
		let sanitizer = document.createElement('div');
		sanitizer.innerText = title;

		let sanitizedTitle = sanitizer.innerHTML;
		return `
			<!DOCTYPE html>
			<html lang="en">
			<head>
				<meta charset="UTF-8">
				<meta name="viewport" content="width=device-width, initial-scale=1">
				<title>${sanitizedTitle}</title>
			</head>
			<body>
                                <header>
                                </header>
                                <main id="main-content">
								  <div id="toolbar">
									<button type="button" id="close-slides" data-cooltip="${this.labels.goBack}" aria-label="${this.labels.goBack}">
										<img src="${LOUtil.getImageURL('presenterscreen-ArrowBack.svg')}">
									</button>
                                  </div>
                                  <div id="presentation-content">
                                    <div id="first-presentation">
										<div id="timer-container">
											<div id="timer"></div>
											 <div id="timer-controls">
												<button type="button" id="pause" data-cooltip="${this.labels.pause}" aria-label="${this.labels.pause}">
													<img src="${LOUtil.getImageURL('presenterscreen-ButtonPauseTimerNormal.svg')}">
												</button>
												<button type="button" id="restart" data-cooltip="${this.labels.restart}" aria-label="${this.labels.restart}">
													<img src="${LOUtil.getImageURL('presenterscreen-ButtonRestartTimerNormal.svg')}">
												</button>
												<button type="button" id="presentToAll" data-cooltip="${this.labels.presentToAll}" aria-label="${this.labels.presentToAll}">
													<img src="${LOUtil.getImageURL('presenterscreen-ButtonFollowMeNormal.svg')}">
												</button>
											 </div>
											<div id="today"></div>
										</div>
                                        <div id='current-slide-container'>
                                            <canvas id="current-presentation"></canvas>
											<div id="slideshow-control-container">
											<div id="navigation-container">
												<button type="button" id="prev" data-cooltip="${this.labels.previous}" aria-label="${this.labels.previous}">
													<img src="${LOUtil.getImageURL('presenterscreen-ButtonSlidePreviousSelected.svg')}">
												</button>
												<div id="title-current">${this.labels.currentSlide}</div>
												<button type="button" id="next" data-cooltip="${this.labels.next}" aria-label="${this.labels.next}">
													<img src="${LOUtil.getImageURL('presenterscreen-ButtonEffectNextSelected.svg')}">
												</button>
											</div>
											<div id="action-buttons-container">
												<button type="button" id="notes" data-cooltip="${this.labels.notes}" aria-label=${this.labels.notes}">
													<img src="${LOUtil.getImageURL('presenterscreen-ButtonNotesNormal.svg')}">
												</button>
												<button type="button" id="slides" data-cooltip="${this.labels.slides}" aria-label="${this.labels.slides}">
													<img src="${LOUtil.getImageURL('presenterscreen-ButtonSlideSorterNormal.svg')}">
												</button>
												<button type="button" id="exchange" data-cooltip="${this.labels.exchange}" aria-label="${this.labels.exchange}">
													<img src="${LOUtil.getImageURL('presenterscreen-ButtonSwitchMonitorNormal.svg')}">
												</button>
											</div>
										</div>
                                        </div>
									</div>
									<div id="notes-separator"></div>
                                     <div id="second-presentation">
                                         <div id="title-next">${this.labels.nextSlide}</div>
                                         <div id='next-slide-container'>
                                            <img id="next-presentation"></img>
                                         </div>
                                    </div>
                                  </div>
                                </main>
                                <footer>
                                </footer>
			</body>
			</html>
			`;
	}

	_getSlidesCount() {
		return this._presenter._getSlidesCount();
	}

	_getRepeatDuration() {
		return this._presenter._getRepeatDuration();
	}

	_isSlideHidden(slideNumber) {
		return this._presenter.isSlideHidden(slideNumber);
	}

	_getNextVisibleSlide(slideNumber) {
		return this._presenter.getNextVisibleSlide(slideNumber);
	}

	_getVisibleIndex(slideNumber) {
		return this._presenter.getVisibleIndex(slideNumber);
	}

	_onPresentationInfo() {
		if (!this._proxyPresenter) {
			return;
		}

		this._map.on('newslideshowframe', this._onNextFrame, this);
		this._map.on('transitionstart', this._onTransitionStart, this);
		this._map.on('transitionend', this._onTransitionEnd, this);
		this._map.on('tilepreview', this._onTilePreview, this);
		this._boundOnWindowClose = this._onWindowClose.bind(this);
		this._map.on('presentinwindowclose', this._boundOnWindowClose, this);

		// safe check for current-presentation element
		const currentPresentationCanvas =
			this._proxyPresenter.document.querySelector('#current-presentation');
		if (!currentPresentationCanvas) return;
		this._computeCanvas(currentPresentationCanvas);

		currentPresentationCanvas.addEventListener(
			'click',
			this._onPresenterCanvasClick.bind(this),
		);
		this._timer = this._proxyPresenter.setInterval(
			window.L.bind(this._onTimer, this),
			1000,
		);
		this._ticks = 0;
		this._pause = false;

		this._visibleSlidesCount = this._presenter.getVisibleSlidesCount();
		this._previews = new Array(this._getSlidesCount());

		this._disableButton(this._prevButton); // On start by default we should disable the prev button

		if (this._slides) {
			let img;
			let elem = this._slides.querySelector('#slides-preview');
			for (let index = 0; index < this._getSlidesCount(); index++) {
				if (this._isSlideHidden(index)) continue;
				img = this._proxyPresenter.document.createElement('img');
				img.id = `preview-slide-${index}`;
				img.src = document.querySelector('meta[name="previewImg"]').content;
				img.alt = _('Preview Slide {1}').replace('{1}', index + 1);
				img.style.marginLeft = '10px';
				img.style.marginRight = '10px';
				img.style.marginTop = '10px';
				img.style.marginBottom = '10px';
				img.style.border = '3px solid transparent';
				img.style.padding = '1px';
				img.style.borderRadius = '3px';
				img.width = 100;
				img.height = 100;
				img._index = index;
				img.tabIndex = 0;
				elem.append(img);
			}
		}
	}

	_onPresenterCanvasClick(event) {
		const canvas = event.target;
		const rect = canvas.getBoundingClientRect();
		const relativeX = (event.clientX - rect.left) / canvas.clientWidth;
		const relativeY = (event.clientY - rect.top) / canvas.clientHeight;

		this._map.fire('presentercanvasclick', {
			relativeX: relativeX,
			relativeY: relativeY,
			originalEvent: event,
		});
	}

	_onImpressModeChanged(e) {
		if (this._waitForExitingNotesMode && e.mode === 0) {
			this._waitForExitingNotesMode = false;
			this._map.off('impressmodechanged', this._onImpressModeChanged, this);
			setTimeout(this._onPresentInConsole.bind(this), 500);
		}
	}

	_openPresenterWindow() {
		const windowopen =
			window.mode.isCODesktop() && window.origOpen
				? window.origOpen
				: window.open;
		return windowopen(
			'',
			'_blank',
			'toolbar=0,scrollbars=0,location=0,statusbar=0,menubar=0,resizable=1,popup=true',
		);
	}

	_onPresentInConsole() {
		if (app.impress.notesMode) {
			app.console.debug(
				'PresenterConsole._onPresentInConsole: notes mode is enabled, exiting',
			);
			// exit notes view mode and wait for status update notification
			// so we're sure that impress mode is changed
			// finally skip next partsupdate event,
			// since it's only due to the mode change
			this._presenter._skipNextSlideShowInfoChangedMsg = true;
			this._waitForExitingNotesMode = true;
			this._map.on('impressmodechanged', this._onImpressModeChanged, this);
			app.map.sendUnoCommand('.uno:NormalMultiPaneGUI');
			return;
		}

		this._active = true;
		this._map.fire('newpresentinwindow');
		if (!this._presenter._slideShowWindowProxy) {
			this._active = false;
			return;
		}

		this._proxyPresenter = this._openPresenterWindow();
		if (!this._proxyPresenter) {
			this._active = false;
			this._presenter._notifyBlockedPresenting();
			return;
		}

		this._proxyPresenter.document.open();
		this._proxyPresenter.document.write(
			this._generateHtml(_('Presenter Console')),
		);
		this._proxyPresenter.document.close();

		this._currentSlideCanvas = this._proxyPresenter.document.querySelector(
			'#current-presentation',
		);
		this._currentSlideContext =
			this._currentSlideCanvas.getContext('bitmaprenderer');

		this._boundOnResize = this._onResize.bind(this);
		this._proxyPresenter.addEventListener('resize', this._boundOnResize);

		if (this._presenter._slideShowWindowProxy) {
			this._presenter._slideShowWindowProxy.addEventListener(
				'unload',
				this._boundOnWindowClose,
			);
			window.addEventListener('beforeunload', this._boundOnWindowClose);
		}
		this._proxyPresenter.addEventListener(
			'unload',
			window.L.bind(this._onConsoleClose, this),
		);
		this._boundOnKeyDown = this._onKeyDown.bind(this);
		this._proxyPresenter.addEventListener('keydown', this._boundOnKeyDown);

		// Declare some basic elements that we will use often in next function calls
		this._prevButton = this._proxyPresenter.document.querySelector('#prev');
		this._nextButton = this._proxyPresenter.document.querySelector('#next');

		this._proxyPresenter.document.body.style.margin = '0';
		this._proxyPresenter.document.body.style.padding = '0';
		this._proxyPresenter.document.body.style.overflowX = 'hidden';

		this._proxyPresenter.document.body.style.display = 'flex';
		this._proxyPresenter.document.body.style.flexDirection = 'column';
		this._proxyPresenter.document.body.style.minHeight = '100vh';
		this._proxyPresenter.document.body.style.minWidth = '100vw';

		let elem;
		let mainContentContainer =
			this._proxyPresenter.document.querySelector('#main-content');
		let slideShowBGColor = window
			.getComputedStyle(document.documentElement)
			.getPropertyValue('--color-background-slideshow');
		this.slideShowColor = window
			.getComputedStyle(document.documentElement)
			.getPropertyValue('--color-slideshow');
		let slideShowFontFamily = window
			.getComputedStyle(document.documentElement)
			.getPropertyValue('--cool-font');
		this.slideSelectionColor = window
			.getComputedStyle(document.documentElement)
			.getPropertyValue('--orange1-txt-primary-color');
		this.PresenterConsoleBtnHoverColor = window
			.getComputedStyle(document.documentElement)
			.getPropertyValue('--color-presenter-console-btn-hover');
		this.PresenterConsoleBtnRadius = window
			.getComputedStyle(document.documentElement)
			.getPropertyValue('--border-radius');

		mainContentContainer.style.backgroundColor = slideShowBGColor;
		mainContentContainer.style.color = this.slideShowColor;
		mainContentContainer.style.fontFamily = slideShowFontFamily;
		mainContentContainer.style.display = 'flex';
		mainContentContainer.style.flexDirection = 'column';
		mainContentContainer.style.minWidth = '100vw';
		mainContentContainer.style.minHeight = '100vh';

		// Disable text selection
		mainContentContainer.style.userSelect = 'none'; //Firefox, Chrome etc.

		this._proxyPresenter.document.body.style.backgroundColor = slideShowBGColor;

		elem = this._proxyPresenter.document.querySelector('#presentation-content');
		elem.style.display = 'flex';
		elem.style.flexWrap = 'wrap';
		elem.style.gap = '3vw';
		elem.style.marginBottom = '10px';

		this._first = elem = this._proxyPresenter.document.querySelector(
			'#first-presentation',
		);
		elem.style.display = 'flex';
		elem.style.flexDirection = 'column';
		elem.style.flex = '1';
		// consistent gap between label and presentation preview
		elem.style.gap = '1vh';
		elem.style.marginTop = '4vh';
		elem.style.marginLeft = '2vw';

		// Apply common button style to every button in Current slide division
		let currentSlideActionButtons = this._first.querySelectorAll('button');
		currentSlideActionButtons.forEach((button) => {
			button.style.display = 'flex';
			button.style.flexDirection = 'column';
			button.style.alignItems = 'center';
			button.style.backgroundColor = 'transparent';
			button.style.color = this.slideShowColor;
			button.style.border = 'none';
			button.style.borderRadius = this.PresenterConsoleBtnRadius;
		});

		elem = this._proxyPresenter.document.querySelector('#title-current');
		elem.style.display = 'flex';
		elem.style.flexDirection = 'column';
		elem.style.justifyContent = 'center';
		elem.style.alignItems = 'center';
		elem.style.backgroundColor = 'transparent';
		elem.style.color = this.slideShowColor;
		elem.style.fontSize = '22px';

		elem = this._proxyPresenter.document.querySelector(
			'#current-slide-container',
		);
		// this will handle the responsiveness on resize for current-presentation window
		elem.style.width = '56vw';
		elem.style.height = '67vh';

		// slideshow-control-container
		let slideshowControlContainer = this._proxyPresenter.document.querySelector(
			'#slideshow-control-container',
		);
		slideshowControlContainer.style.display = 'flex';
		slideshowControlContainer.style.gap = '2vw';
		slideshowControlContainer.style.alignItems = 'center';
		slideshowControlContainer.style.marginTop = '1vh';

		// Select the parent container by its ID
		let navigationContainer = this._proxyPresenter.document.getElementById(
			'navigation-container',
		);

		// Add the necessary styles to make elements appear in a row
		navigationContainer.style.display = 'flex';
		navigationContainer.style.width = 'max-content';
		navigationContainer.style.alignItems = 'center';
		navigationContainer.style.gap = '0.5vw'; // Adjust gap as needed

		// Select all button elements inside #navigation-container
		let navigationButtons = this._proxyPresenter.document.querySelectorAll(
			'#navigation-container button',
		);

		// Apply additional style for navigation button
		navigationButtons.forEach((button) => {
			button.style.border = 'none';
			button.style.justifyContent = 'center';
		});

		// slideshow-control-container
		let actionBtnContainer = this._proxyPresenter.document.querySelector(
			'#action-buttons-container',
		);
		actionBtnContainer.style.display = 'flex';
		actionBtnContainer.style.gap = '1vw';

		this._first.addEventListener(
			'click',
			window.L.bind(this._onToolbarClick, this),
		);

		let notesSeparator =
			this._proxyPresenter.document.querySelector('#notes-separator');
		notesSeparator.style.backgroundColor = 'transparent';
		notesSeparator.style.color = 'transparent';
		notesSeparator.style.border = '1px solid';
		notesSeparator.style.margin = '2vh 0vw';
		notesSeparator.style.height = '85vh';

		this._second = elem = this._proxyPresenter.document.querySelector(
			'#second-presentation',
		);
		elem.style.display = 'flex';
		elem.style.flexDirection = 'column';
		elem.style.flex = '1';
		// consistent gap between label and presentation preview
		elem.style.gap = '1vh';
		elem.style.marginTop = '4vh';

		elem = this._proxyPresenter.document.querySelector('#title-next');
		elem.style.display = 'flex';
		elem.style.flexDirection = 'column';
		elem.style.backgroundColor = 'transparent';
		elem.style.color = this.slideShowColor;
		elem.style.height = '35px';
		elem.style.fontSize = '22px';
		elem.style.justifyContent = 'center';

		let nextSlideContainer = this._proxyPresenter.document.querySelector(
			'#next-slide-container',
		);
		nextSlideContainer.style.width = '25vw';
		nextSlideContainer.style.height = '80vh';
		nextSlideContainer.style.display = 'flex';
		nextSlideContainer.style.flexDirection = 'column';
		nextSlideContainer.style.gap = '2vw';

		elem = this._proxyPresenter.document.querySelector('#next-presentation');
		elem.addEventListener('click', window.L.bind(this._onClickPreview, this));

		this._notes = this._proxyPresenter.document.createElement('div');
		this._notes.style.height = '45vh';
		this._notes.style.width = '25vw';
		this._notes.style.paddingTop = '10px';
		this._notes.style.borderTop = '2px solid transparent';
		this._notes.style.fontSize = '24px';
		this._notes.style.overflowX = 'hidden';

		elem = this._proxyPresenter.document.createElement('div');
		elem.id = 'notes';
		elem.style.height = '100%';
		elem.style.width = '100%';
		elem.style.userSelect = 'text'; // Enables text selection

		this._notes.appendChild(elem);

		// Append the container div to the notes section
		nextSlideContainer.appendChild(this._createTextScalerContainer());

		this._slides = this._proxyPresenter.document.createElement('div');
		this._slides.style.height = '100%';
		this._slides.style.width = '100%';
		this._slides.style.display = 'flex';
		this._slides.style.flexDirection = 'column';
		this._slides.style.gap = '2vh';

		elem = this._proxyPresenter.document.createElement('div');
		elem.id = 'slides-preview';
		elem.style.overflow = 'auto';
		elem.style.height = '90vh';
		elem.style.width = '100%';
		elem.style.display = 'flex';
		elem.style.flexWrap = 'wrap';
		elem.style.justifyContent = 'center';
		elem.style.columnGap = '5vw';

		this._slides.appendChild(elem);
		this._slides.addEventListener(
			'click',
			window.L.bind(this._onClickSlides, this),
		);
		this._slides.addEventListener(
			'keydown',
			function (event) {
				if (event.code !== 'Enter') return;
				this._onClickSlides(event);
			}.bind(this),
		);

		elem = this._proxyPresenter.document.querySelector('#toolbar');
		elem.style.display = 'flex';
		elem.style.alignItems = 'center';
		elem.style.backgroundColor = slideShowBGColor;
		elem.style.overflow = 'hidden';
		elem.style.width = '100%';
		elem.style.gap = '1vw';
		elem.style.margin = '1vh 0vw';
		elem.style.height = '6vh';
		elem.addEventListener('click', window.L.bind(this._onToolbarClick, this));

		let list =
			this._proxyPresenter.document.querySelectorAll('#toolbar button');
		for (elem = 0; elem < list.length; elem++) {
			list[elem].style.display = 'flex';
			list[elem].style.flexDirection = 'column';
			list[elem].style.justifyContent = 'center';
			list[elem].style.alignItems = 'center';
			list[elem].style.backgroundColor = 'transparent';
			list[elem].style.color = this.slideShowColor;
			list[elem].style.padding = '10px';
			list[elem].style.height = '6vh';
			list[elem].style.border = 'none';
		}

		// By default we will hide the Back button to jump from Slides view to Normal view
		let closeSlideButton =
			this._proxyPresenter.document.querySelector('#close-slides');
		closeSlideButton.style.display = 'none';
		closeSlideButton.style.marginLeft = '10px';

		elem = this._proxyPresenter.document.querySelector('#timer-container');
		elem.style.display = 'flex';
		elem.style.alignItems = 'center';
		elem.style.gap = '15px';

		elem = this._proxyPresenter.document.querySelector('#timer');
		elem.style.fontSize = '22px';
		elem.style.width = '85px';
		elem.style.color = this.slideShowColor;

		let timeControlElem =
			this._proxyPresenter.document.querySelector('#timer-controls');
		timeControlElem.style.display = 'flex';
		timeControlElem.style.alignItems = 'center';
		timeControlElem.style.gap = '5px';

		timeControlElem.addEventListener(
			'click',
			window.L.bind(this._onToolbarClick, this),
		);

		// Style buttons in slideshow control container
		const buttons = timeControlElem.querySelectorAll('button');
		buttons.forEach(
			function (button) {
				button.style.display = 'flex';
				button.style.flexDirection = 'column';
				button.style.alignItems = 'center';
				button.style.justifyContent = 'center';
				button.style.backgroundColor = 'transparent';
				button.style.borderColor = 'none';

				const img = button.querySelector('img');
				if (img) {
					img.style.width = '32px';
					img.style.height = '32px';
				}
			}.bind(this),
		);

		// By default hide the exchange monitors button
		if (!window.mode.isCODesktop()) {
			let exchangeMonitorsButton =
				this._proxyPresenter.document.querySelector('#exchange');
			exchangeMonitorsButton.style.display = 'none';
		}

		elem = this._proxyPresenter.document.querySelector('#today');
		elem.style.textAlign = 'right';
		elem.style.fontSize = '22px';
		elem.style.marginLeft = 'auto';
		elem.style.color = this.slideShowColor;

		this._ticks = 0;
		this._onTimer();

		// initialize tooltip division
		this.tooltip = this._proxyPresenter.document.createElement('div');
		this.tooltip.style.position = 'absolute';
		this.tooltip.style.backgroundColor = '#262626';
		this.tooltip.style.color = '#e8e8e8';
		this.tooltip.style.padding = '7px 9px';
		this.tooltip.style.borderRadius = '6px';
		this.tooltip.style.fontSize = '14px';
		this.tooltip.style.fontFamily = slideShowFontFamily;
		this.tooltip.style.whiteSpace = 'nowrap';
		this.tooltip.style.pointerEvents = 'none';
		this.tooltip.style.zIndex = '2147483647';
		this.tooltip.style.boxShadow = 'rgba(77, 77, 77, 0.5) 0px 0px 4px 0px';
		this.tooltip.style.display = 'none'; // Initially hidden

		mainContentContainer.append(this.tooltip);

		this._tooltip = window.L.control.tooltip({
			window: this._proxyPresenter,
			container: this.tooltip,
		});

		let allPresenterConsoleButtons =
			this._proxyPresenter.document.querySelectorAll('button');
		this._attachTooltips(allPresenterConsoleButtons);

		// enable notes by default on start of present console
		this._onShowNotes();
		// simulate resize to Firefox
		this._onResize();

		this._proxyPresenter.addEventListener('load', () => {
			const pauseButton = this._proxyPresenter.document.querySelector('#pause');
			if (pauseButton) {
				pauseButton.focus();
			}
		});
	}

	// Show the tooltip
	_showTooltip(button, text) {
		// Show the tooltip
		this.tooltip.style.display = 'block';

		// Set the tooltip text
		this.tooltip.textContent = text;

		// Get the button's position and tooltip dimensions
		const rect = button.getBoundingClientRect();
		const tooltipRect = this.tooltip.getBoundingClientRect();

		// Calculate initial position
		let left = rect.left + this._proxyPresenter.scrollX;
		let top = rect.bottom + this._proxyPresenter.scrollY + 5;

		// Adjust if tooltip goes off the right edge of the screen
		if (left + tooltipRect.width > this._proxyPresenter.innerWidth) {
			left = this._proxyPresenter.innerWidth - tooltipRect.width - 10; // Add some padding
		}

		// Adjust if tooltip goes off the left edge of the screen
		if (left < 0) {
			left = 10; // Add some padding
		}

		// Adjust if tooltip goes off the bottom edge of the screen
		if (top + tooltipRect.height > this._proxyPresenter.innerHeight) {
			top = rect.top + this._proxyPresenter.scrollY - tooltipRect.height - 5; // Position above the button
		}

		// Adjust if tooltip goes off the top edge of the screen
		if (top < 0) {
			top = rect.bottom + this._proxyPresenter.scrollY + 5; // Revert to below the button
		}

		// Apply the adjusted position
		this.tooltip.style.left = `${left}px`;
		this.tooltip.style.top = `${top}px`;
	}

	// Hide the tooltip
	_hideTooltip() {
		this.tooltip.style.display = 'none';
	}

	// Attach tooltips to buttons
	_attachTooltips(buttons) {
		buttons.forEach(
			function (button) {
				button.addEventListener(
					'mouseenter',
					function () {
						// Add hover effect for enabled button only
						if (!button.disable)
							button.style.backgroundColor = this.PresenterConsoleBtnHoverColor;

						button.style.borderRadius = this.PresenterConsoleBtnRadius;
						const tooltipText = button.getAttribute('data-cooltip') || 'Button'; // Default text if no attribute
						this._showTooltip(button, tooltipText);
					}.bind(this),
				);

				const hideTooltip = this._hideTooltip.bind(this);
				button.addEventListener(
					'mouseleave',
					function () {
						// Remove hover effect
						if (!button.disable) button.style.backgroundColor = 'transparent';

						// Hide tooltip
						this._hideTooltip();
					}.bind(this),
				);
				// for slides view change element on screen to show all the slides in that case tooltip should be hidden
				if (button.getAttribute('data-cooltip') === this.labels.slides)
					button.addEventListener('click', hideTooltip);
			}.bind(this),
		);
	}

	_adjustFontSize(increment) {
		// Define the font size bounds
		const MIN_FONT_SIZE = 12;
		const MAX_FONT_SIZE = 64;

		let currentFontSize = parseInt(this._notes.style.fontSize);
		let newFontSize = currentFontSize + increment;
		let zoomInBtn = this._proxyPresenter.document.querySelector('#increase');
		let zoomOutBtn = this._proxyPresenter.document.querySelector('#decrease');

		// Ensure the font size stays within bounds
		if (newFontSize >= MIN_FONT_SIZE && newFontSize <= MAX_FONT_SIZE) {
			this._notes.style.fontSize = `${newFontSize}px`;
			this._enableButton(zoomInBtn);
			this._enableButton(zoomOutBtn);
		}

		if (newFontSize <= MIN_FONT_SIZE) {
			this._disableButton(zoomOutBtn);
		} else if (newFontSize >= MAX_FONT_SIZE) {
			this._disableButton(zoomInBtn);
		}
	}

	_createTextScalerContainer() {
		// Create the main container div
		let fontChangeContainer =
			this._proxyPresenter.document.createElement('div');
		fontChangeContainer.id = 'textScaler';
		fontChangeContainer.style.display = 'none';

		// Create the plus button
		let plusButton = this._proxyPresenter.document.createElement('button');
		plusButton.id = 'increase';
		plusButton.setAttribute('data-cooltip', this.labels.zoomIn); // Set the tooltip text

		// Create the image for the plus button
		let plusImage = this._proxyPresenter.document.createElement('img');
		plusImage.src = LOUtil.getImageURL('presenterscreen-ButtonPlusNormal.svg');
		plusImage.alt = 'Increase Font'; // Optional: Add alt text for accessibility
		// Add the image inside the plus button
		plusButton.appendChild(plusImage);

		// Create the minus button
		let minusButton = this._proxyPresenter.document.createElement('button');
		minusButton.id = 'decrease';
		minusButton.setAttribute('data-cooltip', this.labels.zoomOut); // Set the tooltip text
		// Create the image for the minus button
		let minusImage = this._proxyPresenter.document.createElement('img');
		minusImage.src = LOUtil.getImageURL(
			'presenterscreen-ButtonMinusNormal.svg',
		);
		minusImage.alt = 'Decrease Font'; // Optional: Add alt text for accessibility

		// Add the image inside the minus button
		minusButton.appendChild(minusImage);

		// Add buttons to the container div
		fontChangeContainer.appendChild(plusButton);
		fontChangeContainer.appendChild(minusButton);

		// common button settings
		let fontScalerButtons = fontChangeContainer.querySelectorAll('button');
		fontScalerButtons.forEach(
			function (button) {
				button.style.display = 'flex';
				button.style.flexDirection = 'column';
				button.style.alignItems = 'center';
				button.style.backgroundColor = 'transparent';
				button.style.border = 'none';
				button.style.margin = '0 5px'; // Add some spacing between buttons
			}.bind(this),
		);

		// font change button action listener
		fontChangeContainer.addEventListener(
			'click',
			window.L.bind(this._onToolbarClick, this),
		);

		return fontChangeContainer;
	}

	_pauseButton() {
		// Update the image source
		let pauseBtn = this._proxyPresenter.document.querySelector('#pause');
		let imgElem = this._proxyPresenter.document.querySelector('#pause>img');
		if (this._pause) {
			imgElem.src = 'images/presenterscreen-ButtonResumeTimerNormal.svg';
			pauseBtn.setAttribute('data-cooltip', this.labels.resume); // Set the tooltip text
		} else {
			imgElem.src = 'images/presenterscreen-ButtonPauseTimerNormal.svg';
			pauseBtn.setAttribute('data-cooltip', this.labels.pause); // Set the tooltip text
		}

		// Kind of special case, on restart we will nor show tooltip on play/pause button
		if (!this._timerReset) {
			this._showTooltip(pauseBtn, pauseBtn.getAttribute('data-cooltip'));
			return;
		}
		this._timerReset = false;
	}

	_onKeyDown(e) {
		this._presenter.getNavigator().onKeyDown(e);
	}

	_onClickPreview(e) {
		this._presenter.getNavigator().onClick(e);
	}

	_onToolbarClick(e) {
		let target = e.target;
		if (!target) {
			return;
		}

		if (target.localName !== 'button') {
			target = target.parentElement;
		}

		if (target.localName !== 'button') {
			return;
		}

		const isLastSlide = this._currentIndex + 1 == this._visibleSlidesCount;
		switch (target.id) {
			case 'prev': {
				this._presenter.getNavigator().rewindEffect();
				break;
			}
			case 'next': {
				this._presenter.getNavigator().dispatchEffect();
				// if repeat after sec is set then do not close on last slide
				if (isLastSlide && !this._presenter._presentationInfo.isEndless) {
					this._onWindowClose();
					break;
				}
				break;
			}
			case 'pause':
				this._pause = !this._pause;
				this._pauseButton();
				break;
			case 'restart':
				this._pause = false;
				this._ticks = 0;
				this._drawClock();
				this._timerReset = true;
				this._pauseButton();
				this._proxyPresenter.clearInterval(this._timer);
				this._timer = this._proxyPresenter.setInterval(
					window.L.bind(this._onTimer, this),
					1000,
				);
				break;
			case 'presentToAll':
				this._onPresentToAll();
				break;
			case 'help':
				// TODO. add help.collaboraonline.com
				window.open('https://collaboraonline.com', '_blank', 'noopener');
				break;
			case 'notes':
				if (this._proxyPresenter.document.contains(this._notes)) {
					this._onHideNotes();
				} else {
					this._onShowNotes();
				}
				break;
			case 'slides':
				this._onShowSlides();
				break;
			case 'exchange':
				this._exchangeMonitors();
				break;
			case 'close-slides':
				this._onHideSlides();
				break;
			case 'increase':
				this._adjustFontSize(2);
				break;
			case 'decrease':
				this._adjustFontSize(-2);
				break;
		}

		e.stopPropagation();
	}

	_resizePreviews(width, height) {
		let preview;
		let previews = this._slides.querySelector('#slides-preview');
		let size = this._map.getPreview(2000, 0, width, height, {
			fetchThumbnail: false,
			autoUpdate: false,
		});

		for (let index = 0; index < this._visibleSlidesCount; index++) {
			preview = previews.children.item(index);
			if (
				preview.width !== size.width ||
				preview.height !== size.height ||
				width !== size.width ||
				height !== size.height
			) {
				this._map.getPreview(2000, preview._index, size.width, size.height, {
					autoUpdate: false,
					slideshow: true,
				});
			}
		}
	}

	_onShowSlides() {
		let elem = this._proxyPresenter.document.querySelector('#slides');
		this.toggleButtonState(elem, true);

		// Show Back button to go into previous page (Current slides page)
		let closeSlideButton =
			this._proxyPresenter.document.querySelector('#close-slides');
		closeSlideButton.style.display = 'block';

		elem = this._proxyPresenter.document.querySelector('#next-presentation');
		let rect = elem.getBoundingClientRect();
		let notesSeparator =
			this._proxyPresenter.document.querySelector('#notes-separator');
		notesSeparator.style.display = 'none';
		this._first.style.display = 'none';
		this._second.style.display = 'none';

		elem = this._proxyPresenter.document.querySelector('#presentation-content');
		elem.appendChild(this._slides);

		if (this._selectedImg) {
			this._selectedImg.scrollIntoView();
		}

		this._proxyPresenter.setTimeout(
			window.L.bind(this._resizePreviews, this, rect.width, rect.height),
			0,
		);
	}

	_onHideSlides() {
		let elem = this._proxyPresenter.document.querySelector(
			'#presentation-content',
		);

		this._slides.remove();
		let notesSeparator =
			this._proxyPresenter.document.querySelector('#notes-separator');
		notesSeparator.style.display = 'block';

		this._first.style.display = 'flex';
		this._second.style.display = 'flex';

		elem = this._proxyPresenter.document.querySelector('#slides');
		this.toggleButtonState(elem, false);

		// Hide back button on normal view
		let closeSlideButton =
			this._proxyPresenter.document.querySelector('#close-slides');
		closeSlideButton.style.display = 'none';

		this._onResize();
	}

	_exchangeMonitors() {
		window.postMobileMessage('EXCHANGEMONITORS');
	}

	_selectImg(img) {
		if (this._selectedImg) {
			this._selectedImg.style.border = '3px solid transparent';
			this._selectedImg = null;
		}

		if (img) {
			this._selectedImg = img;
			this._selectedImg.style.border =
				'3px solid rgb(' + this.slideSelectionColor + ')';
		}
	}

	_onClickSlides(e) {
		if (e.target && e.target.localName === 'img') {
			this._selectImg(e.target);

			if (this._selectedImg && this._proxyPresenter) {
				this._proxyPresenter.requestAnimationFrame(
					function (navigator, index) {
						navigator.displaySlide(index, true);
					}.bind(
						null,
						this._presenter.getNavigator(),
						this._selectedImg._index,
					),
				);
			}
		}

		e.stopPropagation();
	}

	_onShowNotes() {
		let elem = this._proxyPresenter.document.querySelector('#notes');
		this.toggleButtonState(elem, true);

		let container = this._proxyPresenter.document.querySelector(
			'#next-slide-container',
		);
		this._notes.style.borderTopColor = this.slideShowColor;

		let notesSeparator =
			this._proxyPresenter.document.querySelector('#notes-separator');
		notesSeparator.style.color = this.slideShowColor;

		// show font scaler container on show notes
		let textScaler = this._proxyPresenter.document.querySelector('#textScaler');
		textScaler.style.display = 'flex';

		// Insert _notes before textScaler
		container.insertBefore(this._notes, textScaler);
	}

	_onHideNotes(e) {
		let notesSeparator =
			this._proxyPresenter.document.querySelector('#notes-separator');
		notesSeparator.style.color = 'transparent';

		// hide font scaler container on hide notes
		let textScaler = this._proxyPresenter.document.querySelector('#textScaler');
		textScaler.style.display = 'none';
		this._notes.style.borderTopColor = 'transparent';

		this._notes.remove();

		let elem = this._proxyPresenter.document.querySelector('#notes');
		this.toggleButtonState(elem, false);

		if (e) {
			e.stopPropagation();
		}
	}

	_onPresentToAll() {
		app.map.slideShowPresenter.setLeader(true);
		app.map.slideShowPresenter._presentToAllInProgress = true;
		app.map.fire('newpresentinwindow');
	}

	_disableButton(elem) {
		elem.disabled = true;
		elem.style.opacity = '0.5';
	}

	_enableButton(elem) {
		elem.disabled = false;
		elem.style.opacity = '1';
	}

	toggleButtonState(elem, toggleOn) {
		if (toggleOn) {
			// Apply the 'selected' styles on show notes to display toggle effect on button
			elem.style.filter = 'brightness(1.4)';
			elem.style.backgroundColor = 'black';
			elem.disable = true;
		} else {
			elem.style.filter = '';
			elem.style.backgroundColor = 'transparent';
			elem.disable = false;
		}
	}

	_onTimer() {
		if (!this._proxyPresenter) {
			return;
		}

		if (!this._pause) {
			++this._ticks;
		}

		this._proxyPresenter.requestAnimationFrame(this._drawClock.bind(this));
	}

	_drawClock() {
		if (!this._proxyPresenter || !this._proxyPresenter.document) {
			return;
		}

		let sec, min, hour, elem;
		if (!this._pause) {
			sec = this._ticks % 60;
			min = Math.floor(this._ticks / 60);
			hour = Math.floor(min / 60);
			min = min % 60;

			elem = this._proxyPresenter.document.querySelector('#timer');
			if (elem) {
				elem.innerText =
					String(hour).padStart(2, '0') +
					':' +
					String(min).padStart(2, '0') +
					':' +
					String(sec).padStart(2, '0');
			}
		}

		let dateTime = new Date();
		elem = this._proxyPresenter.document.querySelector('#today');
		if (elem) {
			elem.innerText = dateTime.toLocaleTimeString(String.Locale, {
				hour: '2-digit',
				minute: '2-digit',
				second: '2-digit',
			});
		}

		let next =
			this._proxyPresenter.document.querySelector('#next-presentation');
		if (
			this._ticks % 2 === 0 &&
			typeof this._lastIndex !== 'undefined' &&
			next
		) {
			const nextIndex = this._getNextVisibleSlide(this._lastIndex);
			this._proxyPresenter.setTimeout(
				window.L.bind(this._fetchPreview, this, nextIndex, next),
				0,
			);
		}
	}

	_fetchPreview(index, elem) {
		if (index >= this._getSlidesCount()) {
			return;
		}

		let rect = elem.getBoundingClientRect();
		if (rect.width === 0 && rect.height === 0) {
			return;
		}

		let preview = this._previews[index];
		if (preview) {
			this._lastIndex = index;
			return;
		}

		let size = this._map.getPreview(2000, 0, rect.width, rect.height, {
			fetchThumbnail: false,
			autoUpdate: false,
		});

		this._map.getPreview(2000, index, size.width, size.height, {
			autoUpdate: false,
			slideshow: true,
		});
	}

	_onWindowClose() {
		if (this._proxyPresenter && !this._proxyPresenter.closed)
			this._proxyPresenter.close();

		this._active = false;
		app.map.slideShowPresenter._presentToAllInProgress = false;
		window.removeEventListener('beforeunload', this._boundOnWindowClose);

		this._presenter.endPresentation(true);
	}

	_onConsoleClose() {
		if (this._presenter._slideShowWindowProxy) {
			this._presenter.endPresentation(true);
		}

		this._proxyPresenter.removeEventListener('resize', this._boundOnResize);
		this._proxyPresenter.removeEventListener('keydown', this._boundOnKeyDown);
		this._proxyPresenter.clearInterval(this._timer);
		this._proxyPresenter.close();
		this._active = false;
		app.map.slideShowPresenter._presentToAllInProgress = false;

		delete this._proxyPresenter;
		delete this._currentIndex;
		delete this._lastIndex;
		delete this._previews;

		this._map.off('newslideshowframe', this._onNextFrame, this);
		this._map.off('transitionstart', this._onTransitionStart, this);
		this._map.off('transitionend', this._onTransitionEnd, this);
		this._map.off('tilepreview', this._onTilePreview, this);
	}

	_resizeSlideView(viewContainerId, slideViewId) {
		let container = this._proxyPresenter.document.querySelector(
			'#' + viewContainerId,
		);
		if (!container) {
			return;
		}
		let rect = container.getBoundingClientRect();
		let size = this._map.getPreview(2000, 0, rect.width, rect.height, {
			fetchThumbnail: false,
			autoUpdate: false,
		});
		let slideView = this._proxyPresenter.document.querySelector(
			'#' + slideViewId,
		);
		if (slideView) {
			slideView.style.width = size.width + 'px';
			slideView.style.height = size.height + 'px';
		}
	}

	_onResize() {
		if (!this._proxyPresenter) {
			return;
		}

		this._proxyPresenter.clearTimeout(this._resizeTimeout);
		this._resizeSlideView('current-slide-container', 'current-presentation');
		this._resizeSlideView('next-slide-container', 'next-presentation');

		// timeControlContainer should also maintain it's width based on current-slide-container width, better for responsive view
		let timeControlContainer =
			this._proxyPresenter.document.querySelector('#timer-container');
		if (timeControlContainer) {
			timeControlContainer.style.width = this._currentSlideCanvas.style.width;
		}

		this._resizeTimeout = this._proxyPresenter.setTimeout(
			function () {
				let previews =
					this._proxyPresenter.document.querySelector('#slides-preview');
				if (previews && typeof this._currentIndex !== 'undefined') {
					let preview = this._proxyPresenter.document.querySelector(
						`#preview-slide-${this._currentIndex}`,
					);
					// 80vh
					let height = this._proxyPresenter.innerHeight * 0.8;
					// 25vw
					let width = this._proxyPresenter.innerWidth * 0.25;
					if (
						preview.width >= this._proxyPresenter.innerWidth ||
						preview.height >= this._proxyPresenter.innerHeight ||
						preview.width < width
					) {
						this._resizePreviews(width, height);
					}
				}
			}.bind(this),
			800,
		);
	}

	_onTransitionStart(e) {
		if (!this._proxyPresenter) {
			return;
		}

		this._currentIndex = e.slide;
		const isFirstSlide = this._currentIndex == 0;
		let elem = this._proxyPresenter.document.querySelector('#title-current');
		if (elem) {
			elem.innerText = _('Slide {0} of {1}')
				.replace('{0}', this._getVisibleIndex(e.slide) + 1)
				.replace('{1}', this._visibleSlidesCount);
		}
		if (isFirstSlide) this._disableButton(this._prevButton);
		else this._enableButton(this._prevButton);

		elem = this._proxyPresenter.document.querySelector('#next-presentation');
		if (elem) {
			const nextIndex = this._getNextVisibleSlide(this._currentIndex);
			this._fetchPreview(nextIndex, elem);
		}
	}

	_onTransitionEnd(e) {
		if (!this._proxyPresenter) {
			return;
		}

		this._currentIndex = e.slide;

		if (this._notes) {
			let notes = this._presenter.getNotes(e.slide);
			let notesContentElem = this._notes.querySelector('#notes');
			notesContentElem.innerText = _('No Notes');
			if (notes && notes.toLowerCase() !== 'click to add notes'.toLowerCase())
				notesContentElem.innerText = notes;
		}

		let img = this._slides.querySelector(
			`#preview-slide-${this._currentIndex}`,
		);
		if (img) {
			this._selectImg(img);
		}

		let next =
			this._proxyPresenter.document.querySelector('#next-presentation');
		this.drawNext(next);
	}

	drawNext(elem) {
		if (!this._proxyPresenter) {
			return;
		}

		if (!elem) {
			return;
		}

		if (this._currentIndex === undefined) {
			return;
		}

		let rect = elem.getBoundingClientRect();
		if (rect.width === 0 && rect.height === 0) {
			this._proxyPresenter.requestAnimationFrame(
				this.drawNext.bind(this, elem),
			);
			return;
		}

		let size = this._map.getPreview(2000, 0, rect.width, rect.height, {
			fetchThumbnail: false,
			autoUpdate: false,
		});

		const nextSlideIndex = this._getNextVisibleSlide(this._currentIndex);
		if (nextSlideIndex >= this._getSlidesCount()) {
			this.drawEnd(size, this._getRepeatDuration()).then(function (blob) {
				var reader = new FileReader();
				reader.onload = function (e) {
					elem.src = e.target.result;
				};
				reader.readAsDataURL(blob);
			});
			return;
		}

		let preview = this._previews[nextSlideIndex];
		if (!preview) {
			elem.src = document.querySelector('meta[name="previewImg"]').content;
		} else {
			elem.src = preview;
		}

		if (!preview || rect.width !== size.width || rect.height !== size.height) {
			this._map.getPreview(2000, nextSlideIndex, size.width, size.height, {
				autoUpdate: false,
				slideshow: true,
			});
			elem.style.width = size.width + 'px';
			elem.style.height = size.height + 'px';
		}
	}

	drawEnd(size, repeat) {
		const width = size.width;
		const height = size.height;
		const offscreen = new OffscreenCanvas(width, height);
		const ctx = offscreen.getContext('2d');

		ctx.fillStyle = 'black';
		ctx.fillRect(0, 0, width, height);

		ctx.fillStyle = 'white';
		ctx.font = '20px sans-serif';
		ctx.textAlign = 'center';
		ctx.textBaseline = 'middle';

		if (repeat > 0) {
			ctx.fillText(
				_('Repeat in {0} seconds').replace('{0}', repeat),
				width / 2,
				height / 2,
			);
		} else {
			ctx.fillText(_('Click to exit presentation...'), width / 2, height / 2);
		}

		return offscreen.convertToBlob({ type: 'image/png' });
	}

	_onNextFrame(e) {
		const bitmap = e.frame;
		if (!bitmap) {
			return;
		}

		// We need to resize the frame to the current slide canvas size explicitly.
		// In fact, in Firefox transferFromImageBitmap does not resize it
		// automatically to the set canvas size as it occurs in Chrome.
		// According to Firefox version we can have 2 different behavior:
		// on older versions (like 115) the frame is cropped wrt. the canvas size
		// on newer versions (like 121) the canvas is automatically resized to
		// frame size, the latter case can lead to worse performance.
		createImageBitmap(bitmap, {
			resizeWidth: this._currentSlideCanvas.width,
			resizeHeight: this._currentSlideCanvas.height,
		}).then(
			function (image) {
				if (this._proxyPresenter) {
					this._proxyPresenter.requestAnimationFrame(
						function (context, image) {
							context.transferFromImageBitmap(image);
						}.bind(null, this._currentSlideContext, image),
					);
				}
			}.bind(this),
		);
	}

	_onTilePreview(e) {
		if (!this._proxyPresenter) {
			return;
		}

		if (this._currentIndex === undefined) {
			return;
		}

		if (e.id !== '2000') {
			return;
		}

		const nextSlideIndex = this._getNextVisibleSlide(this._currentIndex);
		if (nextSlideIndex === e.part) {
			let next =
				this._proxyPresenter.document.querySelector('#next-presentation');
			if (next) {
				next.src = e.tile.src;
			}
		}

		this._previews[e.part] = e.tile.src;
		this._lastIndex = e.part;

		let img = this._slides.querySelector(`#preview-slide-${e.part}`);
		if (img) {
			img.src = e.tile.src;
			img.width = e.width;
			img.height = e.height;
		}
	}

	_computeCanvas(canvas) {
		let rect = canvas.getBoundingClientRect();
		let size = this._presenter._slideCompositor.computeLayerResolution(
			rect.width,
			rect.height,
		);
		size = this._presenter._slideCompositor.computeLayerSize(size[0], size[1]);
		canvas.width = size[0];
		canvas.height = size[1];
	}
}

SlideShow.PresenterConsole = PresenterConsole;
