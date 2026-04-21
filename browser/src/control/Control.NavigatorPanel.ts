// @ts-strict-ignore
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
 * JSDialog.NavigatorPanel
 */

/* global app */
class NavigatorPanel extends SidebarBase {
	navigationPanel: HTMLElement;
	floatingNavIcon: HTMLElement;
	presentationControlsWrapper: HTMLElement;
	navigatorDockWrapper: HTMLElement;
	quickFindWrapper: HTMLElement;
	closeNavButton: HTMLElement;

	highlightTerm: string;
	focusQuickFind: boolean;
	dirtyWidth: boolean = true;
	currentWidth: number = 0;

	constructor(map: any) {
		super(map, SidebarType.Navigator);
	}

	onAdd(map: ReturnType<typeof window.L.map>) {
		super.onAdd(map);
		this.dirtyWidth = true;
		this.map.on('navigator', this.onNavigator, this);
		this.map.on('doclayerinit', this.onDocLayerInit, this);
		this.map.on('focussearch', this.focusSearch, this);
		this.navigationPanel = document.getElementById(`navigation-sidebar`);
		this.navigationPanel.setAttribute('aria-label', _('Navigation Panel'));
		this.navigationPanel.setAttribute('tabindex', '-1');

		this.floatingNavIcon = document.getElementById(`navigator-floating-icon`);
		this.presentationControlsWrapper = this.navigationPanel.querySelector(
			'#presentation-controls-wrapper',
		);
		this.navigatorDockWrapper = this.navigationPanel.querySelector(
			'#navigator-dock-wrapper',
		);
		this.quickFindWrapper = this.navigationPanel.querySelector(
			'#quickfind-dock-wrapper',
		);
		this.map.on(
			'zoomend',
			this.handleFloatingButtonVisibilityOnZoomChange,
			this,
		);
	}

	onRemove() {
		super.onRemove();
		this.map.off('navigator');
		this.map.off('zoomend');
		this.map.off('doclayerinit');
		this.dirtyWidth = true;
	}

	onDocLayerInit() {
		const allowedDocTypes = ['presentation', 'drawing'];
		// for presentation show slide sorter navigation panel by default
		if (
			allowedDocTypes.includes(app.map.getDocType()) &&
			!window.mode.isSmallScreenDevice()
		) {
			// Navigator panel should be visible and by default we should open slide sorter in case of impress/draw
			this.showNavigationPanel(false);
		}
	}

	initializeNavigator(docType: string) {
		app.layoutingService.appendLayoutingTask(() => {
			this.initializeImpl(docType);
		});
	}

	initializeImpl(docType: string) {
		// Create navigation container
		const navContainer = window.L.DomUtil.create(
			'div',
			'navigation-options-container',
		);
		navContainer.id = 'navigation-options-wrapper';

		// For calc we do not need to add floating icon
		if (docType !== 'spreadsheet') {
			// Create floating navigation button
			this.createFloatingNavigatorBtn();

			// Insert floatingNavIcon right after navigatorPanel
			this.navigationPanel.insertAdjacentElement(
				'afterend',
				this.floatingNavIcon,
			);
		}

		// Create header section
		var navHeader = window.L.DomUtil.create(
			'div',
			'navigation-header',
			navContainer,
		);

		var navTitle = window.L.DomUtil.create('h2', 'navigation-title', navHeader);
		navTitle.textContent = _('Navigation');

		// Create wrapper for search
		const navSearchWrapper = window.L.DomUtil.create(
			'div',
			'navigation-search-wrapper',
		);

		// Create a wrapper div
		const closeNavWrapper = window.L.DomUtil.create(
			'div',
			'close-navigation-wrapper',
			navHeader,
		);

		// Create the close button inside the div
		this.closeNavButton = window.L.DomUtil.create(
			'button',
			'close-navigation-button',
			closeNavWrapper,
		);
		const closeNavigationText = _('Close Navigation');
		this.closeNavButton.setAttribute('aria-label', closeNavigationText);
		this.closeNavButton.setAttribute('data-cooltip', closeNavigationText);
		window.L.control.attachTooltipEventListener(this.closeNavButton, this.map);
		this.closeNavButton.setAttribute('tabindex', '0');

		const clickFunction = function () {
			this.closeNavigation();
			if (app.showNavigator) {
				app.map.sendUnoCommand('.uno:Navigator');
			}
			app.map.focus();
		}.bind(this);

		this.closeNavButton.addEventListener('click', clickFunction);

		this.navigationPanel.addEventListener(
			'keydown',
			function (e: KeyboardEvent) {
				if (e.code === 'Escape') {
					const contextMenu = app.map.uiManager.isAnyContextMenuOpened();
					if (contextMenu) {
						// Do not close navigator if any context menu is open
						return;
					}
					clickFunction();
					e.preventDefault();
					e.stopPropagation();
				}
			},
		);

		const contentDivs = [];

		// build tabs if we are in an environment that supports them
		// e.g. in writer we have navigator and quickfind tabs
		// in impress/draw we have navigator and slide sorter tabs
		if (this.map.isPresentationOrDrawing() || this.map.isText()) {
			var navigationTabs = [];

			var navOptions = window.L.DomUtil.create(
				'div',
				'navigation-tabs',
				navContainer,
			);
			navOptions.id = 'navigation-options';
			navOptions.setAttribute('role', 'tablist');

			if (this.map.isPresentationOrDrawing()) {
				// Create Slide Sorter tab
				var slideSorterTab = window.L.DomUtil.create(
					'button',
					'tab',
					navOptions,
				);
				slideSorterTab.id = 'tab-slide-sorter';
				slideSorterTab.textContent = _('Slides');

				if (this.presentationControlsWrapper) {
					slideSorterTab.setAttribute(
						'aria-controls',
						'presentation-controls-wrapper',
					);

					this.presentationControlsWrapper.setAttribute('role', 'tabpanel');
					this.presentationControlsWrapper.setAttribute(
						'aria-labelledby',
						'tab-slide-sorter',
					);
					contentDivs.push(this.presentationControlsWrapper);
				}
				navigationTabs.push(slideSorterTab);
			}

			// Create Navigator tab
			var navigatorTab = window.L.DomUtil.create('button', 'tab', navOptions);
			navigatorTab.id = 'tab-navigator';
			navigatorTab.textContent = _('Outline');
			if (this.navigatorDockWrapper) {
				navigatorTab.setAttribute('aria-controls', 'navigator-dock-wrapper');

				this.navigatorDockWrapper.setAttribute('role', 'tabpanel');
				this.navigatorDockWrapper.setAttribute(
					'aria-labelledby',
					'tab-navigator',
				);
				contentDivs.push(this.navigatorDockWrapper);
			}
			navigationTabs.push(navigatorTab);

			if (this.map.isText()) {
				// Create Quick Find tab
				var quickFindTab = window.L.DomUtil.create('button', 'tab', navOptions);
				quickFindTab.id = 'tab-quick-find';
				quickFindTab.textContent = _('Results');

				if (this.quickFindWrapper) {
					quickFindTab.setAttribute('aria-controls', 'quickfind-dock-wrapper');

					this.quickFindWrapper.setAttribute('role', 'tabpanel');
					this.quickFindWrapper.setAttribute(
						'aria-labelledby',
						'tab-quick-find',
					);
					contentDivs.push(this.quickFindWrapper);
				}
				navigationTabs.push(quickFindTab);
			}

			if (navigationTabs.length > 0) {
				// Set up ARIA attributes for tabs
				navigationTabs.forEach((tab, index) => {
					tab.setAttribute('role', 'tab');
					// Only first tab initially selected
					if (index === 0) {
						tab.setAttribute('aria-selected', 'true');
						tab.classList.add('selected');
					} else {
						tab.setAttribute('aria-selected', 'false');
						tab.setAttribute('tabindex', '-1');
					}
				});

				// Tab Click Event Listener
				navigationTabs.forEach((tab) => {
					tab.addEventListener(
						'click',
						function () {
							this.switchNavigationTab(tab.id);
						}.bind(this),
					);
				});

				// Initialize keyboard navigation
				JSDialog.KeyboardTabNavigation(navigationTabs, contentDivs);
			}
		}

		if (this.navigationPanel) {
			// Insert navigation container as the first child & navHeader as next-child of navigator-panel
			this.navigationPanel.prepend(navContainer);
			if (this.map.isText()) {
				this.navigationPanel.prepend(navSearchWrapper);
				this.createSearchBar(navSearchWrapper);
			}
			this.navigationPanel.prepend(navHeader);
		}

		this.dirtyWidth = true;
	}

	createFloatingNavigatorBtn() {
		// Get or create the main wrapper div
		this.floatingNavIcon.className =
			'notebookbar unoNavigator unospan-view-navigator unotoolbutton visible';
		this.floatingNavIcon.setAttribute('tabindex', '-1');
		const navigatorText = _('Navigator');
		this.floatingNavIcon.setAttribute('data-cooltip', navigatorText);
		window.L.control.attachTooltipEventListener(this.floatingNavIcon, this.map);

		// Create the button wrapper (square container)
		const buttonWrapper = document.createElement('div');
		buttonWrapper.className = 'navigator-btn-wrapper'; // Class for styling

		// Create the button
		const button = document.createElement('button');
		button.className = 'ui-content unobutton';
		button.id = 'floating-navigator';
		button.accessKey = 'ZN';
		button.setAttribute('aria-label', navigatorText);

		// Create the image inside the button
		const img = document.createElement('img');
		img.alt = ''; // empty alt for accessibility
		app.LOUtil.setImage(img, 'lc_navigator.svg', this.map);

		// Append elements
		button.appendChild(img);
		buttonWrapper.appendChild(button);
		this.floatingNavIcon.appendChild(buttonWrapper);

		// Click event
		this.floatingNavIcon.addEventListener(
			'click',
			function () {
				this.showNavigationPanel(true);
				if (app.map.isPresentationOrDrawing()) {
					this.switchNavigationTab('tab-slide-sorter');
				} else {
					app.map.sendUnoCommand('.uno:Navigator');
				}
				this.focusSearch();
			}.bind(this),
		);
	}

	onNavigator(data: FireEvent) {
		var navigatorData = data.data;
		this.builder.setWindowId(navigatorData.id);
		this.container.innerHTML = '';

		if (navigatorData.action === 'close') {
			this.closeSidebar();
		} else if (navigatorData.children) {
			if (navigatorData.children.length) {
				this.onResize();
			}

			this.markNavigatorTreeView(navigatorData);

			app.layoutingService.appendLayoutingTask(() => {
				this.builder.build(this.container, [navigatorData], false);
			});

			// There is case where user can directly click navigator from notebookbar view option
			// in that case we first show the navigation panel and then switch to tab view
			this.showNavigationPanel(false);
			this.navigatorDockWrapper.style.display = '';
			app.showNavigator = true;
			if (
				app.map.isPresentationOrDrawing() &&
				!this.isNavigationPanelVisible()
			) {
				this.switchNavigationTab('tab-slide-sorter');
			} else {
				this.switchNavigationTab('tab-navigator');
			}
			if (this.focusQuickFind) {
				this.switchNavigationTab('tab-quick-find');
				this.focusSearch();
				this.focusQuickFind = false;
			}
		} else {
			this.closeSidebar();
		}

		this.dirtyWidth = true;
	}

	onJSUpdate(e: FireEvent) {
		if (this.highlightTerm && this.highlightTerm.trim().length > 0) {
			e.data.control.highlightTerm = this.highlightTerm;
		}
		return super.onJSUpdate(e);
	}

	closeSidebar() {
		this.closeNavigation();
		app.showNavigator = false;
		super.closeSidebar();
	}

	// Function to handle tab click
	switchNavigationTab(tabId: string) {
		// Remove 'selected' class from all tabs
		this.navigationPanel
			.querySelectorAll('.navigation-tabs .tab')
			.forEach((t) => {
				t.classList.remove('selected');
				t.setAttribute('aria-selected', 'false');
				t.setAttribute('tabindex', '-1');
			});

		// Add 'selected' class to the clicked tab
		// In Calc we don't have tabs so far
		const tab = this.navigationPanel.querySelector('#' + tabId);
		if (tab) {
			tab.classList.add('selected');
			tab.setAttribute('aria-selected', 'true');
			tab.removeAttribute('tabindex');
		}

		// Toggle visibility based on tabId
		if (tabId === 'tab-slide-sorter') {
			// todo: must be a better way to handle this
			this.presentationControlsWrapper.style.display = 'block';
			this.navigatorDockWrapper.style.display = 'none';
			this.quickFindWrapper.style.display = 'none';
		} else if (tabId === 'tab-navigator') {
			if (!app.showNavigator) app.map.sendUnoCommand('.uno:Navigator');
			this.presentationControlsWrapper.style.display = 'none';
			this.navigatorDockWrapper.style.display = 'block';
			this.quickFindWrapper.style.display = 'none';
		} else if (tabId === 'tab-quick-find') {
			if (!app.showQuickFind) app.map.sendUnoCommand('.uno:QuickFind'); // todo add showQuickFind to other areas of app
			this.presentationControlsWrapper.style.display = 'none';
			this.navigatorDockWrapper.style.display = 'none';
			this.quickFindWrapper.style.display = 'block';
		}
	}

	handleFloatingButtonVisibilityOnZoomChange() {
		// Handle special case for impress as the view there is landscape so better to hide Floating Nav ICON on lower zoom compare to other app
		if (
			this.map.getZoom() > 14 ||
			(this.map.getZoom() >= 13 && this.map.getDocType() === 'presentation')
		) {
			this.floatingNavIcon.classList.remove('visible');
		} else if (!this.navigationPanel.classList.contains('visible')) {
			this.floatingNavIcon.classList.add('visible');
		}
	}

	getCurrentWidth() {
		if (this.dirtyWidth) {
			// Consider navigations sidebar width to place marker at correct position
			const presentationControlsWrapper: HTMLDivElement =
				document.querySelector('#navigation-sidebar');
			let presentationControlsWrapperWidth: number = 0;

			if (presentationControlsWrapper)
				presentationControlsWrapperWidth =
					presentationControlsWrapper.getBoundingClientRect().width;

			this.currentWidth = presentationControlsWrapperWidth;
			this.dirtyWidth = false;
		}

		return this.currentWidth;
	}

	requestShow() {
		app.socket.sendMessage('uno .uno:Navigator');
	}

	showNavigationPanel(setFocus: boolean) {
		app.layoutingService.appendLayoutingTask(() => {
			this.navigationPanel.classList.add('visible');
			this.floatingNavIcon.classList.remove('visible');
			// this will update the indentation marks for elements like ruler
			app.map.fire('fixruleroffset');

			if (setFocus) this.navigationPanel.focus();
		});
	}

	isNavigationPanelVisible(): boolean {
		return this.navigationPanel.classList.contains('visible');
	}

	closeNavigation() {
		app.layoutingService.appendLayoutingTask(() => {
			this.navigationPanel.classList.remove('visible');
			this.floatingNavIcon.classList.add('visible');
			this.handleFloatingButtonVisibilityOnZoomChange(); // on close panel we should check if we can display nav icon or not based on zoom level
		});
	}

	preFocusQuickFind() {
		this.focusQuickFind = true;
	}

	focusSearch() {
		const searchInput = document.getElementById(
			'navigator-search-input',
		) as HTMLInputElement;
		if (!searchInput) return;

		app.layoutingService.appendLayoutingTask(() => {
			searchInput.select();
			searchInput.focus();
		});
	}

	createSearchBar(wrapper: HTMLElement) {
		var data = {
			id: '',
			type: 'container',
			children: [
				{
					id: 'navigator-search',
					type: 'edit',
					placeholder: _('Search...'),
					text: '',
				} as EditWidgetJSON,
				{
					id: '',
					type: 'toolbox',
					text: '',
					children: [
						{
							id: 'navigator-search-button',
							type: 'pushbutton',
							text: '',
							image: 'lc_recsearch.svg',
							aria: {
								label: _('Search'),
							},
						},
					],
				},
			],
		} as WidgetJSON;

		this.builder.build(wrapper, [data], false);
	}

	useDefaultCallback(
		objectType: string,
		eventType: string,
		object: any,
		data: any,
		builder: JSBuilder,
	) {
		super.callback(objectType, eventType, object, data, builder);
	}

	getSearchTerm(): string | null {
		const searchInput = document.getElementById(
			'navigator-search-input',
		) as HTMLInputElement;

		if (searchInput) return searchInput.value;
		else return null;
	}

	useSearchCallback(
		objectType: string,
		eventType: string,
		object: any,
		builder: JSBuilder,
	) {
		// Switch to "Results tab" first.
		if (eventType === 'activate') {
			const resultsTab = this.navigationPanel.querySelector(
				'#tab-quick-find:not(.selected)',
			) as HTMLElement;
			if (resultsTab) resultsTab.click();
		}

		const nextButton = this.navigationPanel.querySelector(
			'#findnext button',
		) as HTMLElement;
		const nextButtonVisible =
			nextButton && (nextButton as any).checkVisibility();
		const searchTerm = this.getSearchTerm();

		if (!searchTerm) return; // There is something wrong. If search input doesn't exist, nothing to do below.

		const termChanged = searchTerm !== this.highlightTerm;
		this.highlightTerm = searchTerm;
		const newSearch = termChanged || !nextButtonVisible;

		if (newSearch) {
			if (object.id === 'navigator-search-button')
				super.callback('edit', 'activate', { id: 'Find' }, searchTerm, builder);
			else
				super.callback(
					objectType,
					eventType,
					{ id: 'Find' },
					searchTerm,
					builder,
				);
		} else if (nextButton) nextButton.click();

		// Update outline highlighting
		// Note: only update on 'activate' or button pressed events to be consistent with results tab
		if (eventType === 'activate') {
			var treeContainer = document.getElementById('contenttree') as any;
			if (treeContainer) treeContainer.highlightEntries(searchTerm);
		}
	}

	override callback(
		objectType: string,
		eventType: string,
		object: any,
		data: any,
		builder: JSBuilder,
	): void {
		if (!['navigator-search-button', 'navigator-search'].includes(object.id)) {
			this.useDefaultCallback(objectType, eventType, object, data, builder);
			return;
		} else {
			this.useSearchCallback(objectType, eventType, object, builder);
		}
	}
}

JSDialog.NavigatorPanel = function (map: any) {
	return new NavigatorPanel(map);
};
