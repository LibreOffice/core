/* -*- js-indent-level: 8 -*- */
/*
	This class is used for managing the accessibility keys of notebookbar control.
*/

/* global JSDialog app NotebookbarAccessibilityDefinitions _ */

/*
	This class relies on following id convention (example for "Home" tab):
		* Tab button id: "Home-etc..."
		* Tab content container id: "Home-container"
*/
var NotebookbarAccessibility = function() {

	this.initialized = false;

	this.activeTabPointers = {
		id: '',
		contentList: [],
		infoBoxList: []
	},

	this.definitions = new NotebookbarAccessibilityDefinitions();
	this.tabInfoList = null; // This is to be fetched from NotebookbarAccessibilityDefinitions class at initialization.
	this.combination = null;
	this.filteredItem = null;
	this.state = 0; // 0: User needs to select a tab. 1: User needs to either select an access key of tab content, or navigate by arrow keys.

	this.addInfoBox = function(anchorElement) {
		var visible = anchorElement.id.replace('-button', '');
		visible = document.querySelector('[id^="' + visible + '"]');

		if (visible)
			visible = visible.style.display !== 'none';

		if (visible) {
			var infoBox = document.createElement('div');
			infoBox.classList.add('accessibility-info-box');
			infoBox.textContent = anchorElement.accessKey;
			var rectangle = anchorElement.getBoundingClientRect();
			infoBox.style.top = (rectangle.bottom - 5) + 'px';
			infoBox.style.left = rectangle.left + 'px';
			document.body.appendChild(infoBox);

			return infoBox;
		}
		else {
			return null;
		}
	};

	this.setupAcceleratorsForCurrentTab = function(id) {
		if (id === undefined)
			id = this.activeTabPointers.id;

		this.removeAllInfoBoxes();

		this.activeTabPointers.id = id;
		this.activeTabPointers.contentList = this.tabInfoList[id].contentList;
		this.activeTabPointers.infoBoxList = [];

		for (var i = 0; i < this.activeTabPointers.contentList.length; i++) {
			var element = document.querySelector('[id^="' + this.activeTabPointers.contentList[i].id + '"]');
			if (element && element.offsetParent !== null) {
				element.accessKey = this.activeTabPointers.contentList[i].combination;
				this.activeTabPointers.infoBoxList.push(this.addInfoBox(element));
			}
			else if(!element) // element is null
				console.warn('NotebookbarAccessibility: Element with id ' + this.activeTabPointers.contentList[i].id + ' doesn\'t exist.');
		}
	};

	/*
		We want to show the accelerator info boxes if no JSDialog is open.
		When a JSDialog is open, we will underline the accelerator keys of the dialog.
	*/
	this.mayShowAcceleratorInfoBoxes = false;
	this.onDocumentKeyDown = function(event) {
			 if (this.initialized && (event.keyCode === 18 || (event.keyCode === 18 && event.shiftKey))) {
				this.mayShowAcceleratorInfoBoxes = true;
			}
	};

	this.onDocumentKeyUp = function(event) {
		if (this.initialized) {
			if (app.map && app.map.jsdialog && app.map.jsdialog.hasDialogOpened()) {
				if (event.keyCode === 18)
					document.body.classList.remove('activate-underlines');
			}
			else if (this.mayShowAcceleratorInfoBoxes && (event.keyCode === 18 || (event.keyCode === 18 && event.shiftKey))) { // 18: Alt key.
				this.resetState();
				this.setTabDescription(this.getCurrentSelectedTab());
				this.addTabAccelerators();
				this.accessibilityInputElement.focus();
			}
			else if (event.keyCode === 16) // ShiftLeft.
				return; // Ignore shift key.
			else {
				this.resetState();
			}
		}
	};

	this.onInputFocus = function() {
		this.addTabFocus();
		document.body.classList.add('activate-info-boxes');
	};

	this.onInputBlur = function() {
		document.body.classList.remove('activate-info-boxes');
		this.removeFocusFromTab();
		this.resetState();
	};

	this.isAllFilteredOut = function() {
		var count = document.querySelectorAll('.accessibility-info-box:not(.filtered_out)');
		count = count.length;
		return count === 0;
	};

	this.filterOutNonMatchingInfoBoxes = function() {
		var keyList = document.getElementsByClassName('accessibility-info-box');

		for (var i = 0; i < keyList.length; i++)
			keyList[i].classList.remove('filtered_out');

		if (this.combination !== null) {
			for (var i = 0; i < keyList.length; i++) {
				if (!keyList[i].textContent.startsWith(this.combination))
					keyList[i].classList.add('filtered_out');
			}
		}
	};

	this.checkTabAccelerators = function() {
		for (var tabId in this.tabInfoList) {
			if (Object.prototype.hasOwnProperty.call(this.tabInfoList, tabId)) {
				var element = document.querySelector('[id^="' + tabId + '"]');
				if (element && !element.classList.contains('hidden')) {
					if (this.tabInfoList[tabId].combination === this.combination) {
						this.filteredItem = this.tabInfoList[tabId];
						this.filteredItem.id = tabId;
						break;
					}
				}
			}
		}
	};

	this.checkContentAccelerators = function() {
		for (var i = 0; i < this.activeTabPointers.contentList.length; i++) {
			var item = this.activeTabPointers.contentList[i];
			if (this.combination === item.combination) {
				this.filteredItem = this.activeTabPointers.contentList[i];
				break;
			}
		}
	};

	this.checkCombinationAgainstAcccelerators = function() {
		this.filteredItem = null;

		if (this.state === 0)
			this.checkTabAccelerators();
		else if (this.state === 1)
			this.checkContentAccelerators();
	};

	this.clickOnFilteredItem = function() {
		var itemWasClicked = false;

		if (this.filteredItem !== null) {
			var element = document.querySelector('[id^="' + this.filteredItem.id + '"]');
			if (element) {
				// menu button & overflow button - prioritize dropdown arrow
				var dropdownArrow = element.querySelector('.arrowbackground');
				if (dropdownArrow) {
					element = dropdownArrow;
				}

				if (this.state === 0) {
					this.removeFocusFromTab();
					element.click();
					this.addTabFocus();
					this.setupAcceleratorsForCurrentTab(element.id);
					this.combination = null;
					this.accessibilityInputElement.value = '';
					this.setTabDescription(element);
					this.accessibilityInputElement.focus();
					this.state = 1;
				}
				else if (this.state === 1) {
					itemWasClicked = true;
					this.setTabItemDescription(element);
					var selectTarget = element.tagName === 'SELECT' ? element : element.querySelector('select');
					if (selectTarget) {
						selectTarget.focus();
						selectTarget.showPicker();
					} else {
						var clickTarget = element.querySelector('button.unobutton') || element;
						clickTarget.click();
						if (this.filteredItem && this.filteredItem.focusBack === true)
							this.focusToMap();
					}
				}
			}
			this.filteredItem = null;
		}
		else
			this.focusToMap();

		return itemWasClicked;
	};

	this.addTabFocus = function() {
		var element = this.getCurrentSelectedTab();
		if (element) {
			element.classList.add('add-focus-to-tab');
		}
	};

	this.removeFocusFromTab = function() {
		var element = this.getCurrentSelectedTab();
		if (element) {
			element.classList.remove('add-focus-to-tab');
		}
	};

	this.focusToMap = function () {
		app.map.focus();
		this.mayShowAcceleratorInfoBoxes = false;
		this.removeFocusFromTab();
	};

	this.getCurrentSelectedTab = function() {
		return document.querySelector('button.ui-tab.notebookbar.selected');
	};

	this.setTabDescription = function(tabElem) {
		var tabDescr = tabElem ? _('{0} tab selected').replace('{0}', tabElem.textContent) : '';
		this.accessibilityInputElement.setAttribute('aria-description', tabDescr);
	};

	this.setTabItemDescription = function(element) {
		var descr = '';
		if (element) {
			var button = element.hasAttribute('alt') ? element : element.querySelector('button[alt]');
			if (button) {
				descr = button.getAttribute('alt');
			}
		}
		this.accessibilityInputElement.setAttribute('aria-description', descr);
	};

	this.getCurrentSelectedTabPage = function() {
		return document.querySelector('div.ui-content.level-0.notebookbar:not(.hidden)');
	};

	this.resetState = function() {
		this.removeAllInfoBoxes();
		this.state = 0;
		this.accessibilityInputElement.value = '';
		this.accessibilityInputElement.setAttribute('aria-description', '');
		this.combination = null;
		this.mayShowAcceleratorInfoBoxes = false;
		this.filteredItem = null;
		for (var i = 0; i < this.activeTabPointers.contentList.length; i++) {
			const found = document.querySelector('[id^="' + this.activeTabPointers.contentList[i].id + '"]');
			if (found)
				found.accessKey = null;
			else
				console.warn('Accessibility - no element with id:' + this.activeTabPointers.contentList[i].id);
		}
	};

	this.onInputKeyDown = function(event) {
		if (event.ctrlKey) {
			this.resetState();
		}
	};

	this.onInputKeyUp = function(event) {
		var key = event.key.toUpperCase();
		event.preventDefault();
		event.stopPropagation();

		if (key === 'ESCAPE' || key === 'ALT') {
			if (this.combination === null)
				this.focusToMap();
			else {
				this.resetState();
			}
		}
		else if (event.keyCode === 16) // ShiftLeft.
			return; // Ignore shift key.
		else if (key === 'ARROWUP') {
			// Try to set focus on tab button.
			this.removeFocusFromTab();
			var currentSelectedTabButton = this.getCurrentSelectedTab();
			currentSelectedTabButton.focus();
		}
		else if (key === 'ARROWDOWN') {
			// Try to set focus on the first button of the tab content.
			var currentSelectedTabPage = this.getCurrentSelectedTabPage();
			var firstSelectableElement = JSDialog.FindFocusableElement(currentSelectedTabPage,'next');
			if (firstSelectableElement)
				firstSelectableElement.focus();
		}
		else if (key === 'ARROWRIGHT') {
			this.getNextTab('right');
		}
		else if (key === 'ARROWLEFT') {
			this.getNextTab('left');
		}
		else {
			if (this.combination === null) {
				this.combination = key;
				this.checkCombinationAgainstAcccelerators();
				this.filterOutNonMatchingInfoBoxes();
			}
			else {
				this.combination += key;
				this.checkCombinationAgainstAcccelerators();
				this.filterOutNonMatchingInfoBoxes();
			}
			// If item was clicked - don't focus the map to keep focus on dropdowns
			if (this.filteredItem !== null && this.clickOnFilteredItem())
				return;
			// So we checked the pressed key against available combinations. If there is no match, focus back to map.
			if (this.isAllFilteredOut() === true)
				this.focusToMap();
		}
	};

	this.getNextTab = function(move) {
		var currentSelectedTab = this.getCurrentSelectedTab();
		var isLeftMovement = move === 'left'? true : false;
		var tab = isLeftMovement ? currentSelectedTab.previousElementSibling : currentSelectedTab.nextElementSibling;
		if (!tab) {
			var tabs = document.querySelectorAll('.ui-tab.notebookbar:not(.hidden)');
			tab = isLeftMovement ? tabs[tabs.length - 1] : tabs[0];
		}
		while (tab) {
			if (!tab.classList.contains('hidden')) {
				// Found the next element without the "hidden" class
				break;
			}
			tab = isLeftMovement ? tab.previousElementSibling : tab.nextElementSibling;
		}
		this.removeFocusFromTab();
		tab.click();
		tab.focus();
	};

	this.removeAllInfoBoxes = function() {
		var infoBoxes = document.getElementsByClassName('accessibility-info-box');
		for (var i = infoBoxes.length - 1; i > -1; i--) {
			document.body.removeChild(infoBoxes[i]);
		}
	};

	this.addTabAccelerators = function() {
		// Remove all info boxes first.
		this.removeAllInfoBoxes();
		this.tabInfoList = this.definitions.getDefinitions();
		for (var tabId in this.tabInfoList) {
			if (Object.prototype.hasOwnProperty.call(this.tabInfoList, tabId)) {
				var element = document.querySelector('[id^="' + tabId + '"]');
				if (element && element.offsetParent !== null) {
					element.accessKey = this.tabInfoList[tabId].combination;
					this.addInfoBox(element);
				}
			}
		}
	};

	this.initTabListeners = function() {
		Object.keys(this.tabInfoList).forEach(function(tabId) {
			var element = document.querySelector('[id^="' + tabId + '"]');
			if (element) {
				element.addEventListener('keydown', function(event) {
					if (event.key === 'Alt') {
					  // focus back to document
					  this.focusToMap();
					}
				  }.bind(this));
			}
		}.bind(this));
	};

	this.initAccessibilityInputElement = function() {
		// Create an input element for catching the events and prevent document from catching them.
		this.accessibilityInputElement = document.createElement('input');
		// type = 'submit' prevents the screen reader to report something like: "editable blank",
		// when <alt> is pressed, since at start the input field is empty and obviously editable;
		// note that an input element with type 'submit' still receives keyboard events
		this.accessibilityInputElement.type = 'submit';
		// role = 'tablist' prevents the screen reader to report "Submit button" when <alt> is pressed
		// screen reader uses to report 'tablist' role as 'tab control'
		this.accessibilityInputElement.setAttribute('role', 'tablist');
		this.accessibilityInputElement.style.width = this.accessibilityInputElement.style.height = '0';
		this.accessibilityInputElement.id = 'accessibilityInputElement';
		this.accessibilityInputElement.onfocus = this.onInputFocus.bind(this);
		this.accessibilityInputElement.onblur = this.onInputBlur.bind(this);
		this.accessibilityInputElement.onkeyup = this.onInputKeyUp.bind(this);
		this.accessibilityInputElement.onkeydown = this.onInputKeyDown.bind(this);
		this.accessibilityInputElement.autocomplete = 'off';

		var container = document.createElement('div');
		container.style.width = container.style.height = '0';
		container.style.overflow = 'hidden';
		container.appendChild(this.accessibilityInputElement);

		document.body.insertBefore(container, document.body.firstChild);
	};

	this.initialize = function() {
		setTimeout(function() {
			if (window.mode.isDesktop() && !this.initialized) {
				if (document.body.dataset.userinterfacemode === 'notebookbar') {
					this.tabInfoList = this.definitions.getDefinitions();

					if (this.tabInfoList !== null) {
						this.initTabListeners();
						this.initAccessibilityInputElement();
						this.initialized = true;
					}
				}
			}
		}.bind(this), 3000);
	};
};

app.definitions.NotebookbarAccessibility = NotebookbarAccessibility;

app.UI.notebookbarAccessibility = new app.definitions.NotebookbarAccessibility();
