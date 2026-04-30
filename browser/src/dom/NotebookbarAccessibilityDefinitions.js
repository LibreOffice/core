/* -*- js-indent-level: 8 -*- */
/*
	This class is used as the notebookbar accessibility configuration provider for the current document type.
*/

/* global app */

/* eslint-disable-next-line */
var NotebookbarAccessibilityDefinitions = function() {
	this.cleanMenuName = function(id) {
		if (!id) {
			console.debug('No id found for Notebookbar Accesibility item');
			return id;
		}
		var separatorPos = id.indexOf(':');
		if (separatorPos === -1)
			return id;
		return id.substr(0, separatorPos);
	};

	this.getContentListRecursive = function(rawList, list, language) {
		if (Array.isArray(rawList)) {
			for (var i = 0; i < rawList.length; i++) {
				if (rawList[i].accessibility) {
					var combination = language && rawList[i].accessibility[language] ? rawList[i].accessibility[language]: rawList[i].accessibility.combination;
					var id = this.cleanMenuName(rawList[i].id);

					var overflow = document.querySelector('#' + id + '.ui-overflow-group');
					var arrow = document.querySelector('#' + id + ' .arrowbackground');
					var element = document.getElementById(id + '-button');

					// If standard lookups failed and item has a UNO command, try
					// finding by modelid. This handles WeldedToolbar items where
					// core assigns command-based ids (e.g. NumberFormatCurrency)
					// instead of the JS definition ids (e.g. home-number-format-currency).
					if (!overflow && !arrow && !element && rawList[i].command) {
						var commandId = rawList[i].command.replace('.uno:', '');
						var byModelId = document.querySelector('[modelid="' + commandId + '"]');
						if (byModelId) {
							id = commandId;
							arrow = byModelId.querySelector('.arrowbackground');
						}
					}

					if (overflow) {
						// overflow button
						if (typeof overflow.isCollapsed === 'function' && overflow.isCollapsed()) {
							list.push({ id: id, focusBack: rawList[i].accessibility.focusBack, combination: combination });
						} else if (rawList[i].children) {
							this.getContentListRecursive(rawList[i].children, list, language);
							if(rawList[i].more && rawList[i].more.accessibility) {
								const moreAccessibility = rawList[i].more.accessibility;
								combination = language && moreAccessibility[language] ? moreAccessibility[language]: moreAccessibility.combination;
								list.push({ id: id + '-more-button', focusBack: moreAccessibility.focusBack, combination: combination });
							}
						}
					} else if (arrow) {
						// menu button
						list.push({ id: id, focusBack: rawList[i].accessibility.focusBack, combination: combination });
					} else if (element) {
						// regular uno button
						list.push({ id: id + '-button', focusBack: rawList[i].accessibility.focusBack, combination: combination });
					} else if (document.getElementById(id + '-input')) {
						// checkbox
						list.push({ id: id + '-input', focusBack: rawList[i].accessibility.focusBack, combination: combination });
					} else if (rawList[i].type === 'iconviewlist') {
						// iconviewlist
						list.push({ id: id + '-expand-button', focusBack:  rawList[i].accessibility.focusBack, combination: combination });
					} else {
						// other
						list.push({ id: id, focusBack: rawList[i].accessibility.focusBack, combination: combination });
					}
				}
				else if (rawList[i].children && Array.isArray(rawList[i].children) && rawList[i].children.length > 0) {
					this.getContentListRecursive(rawList[i].children, list, language);
				}
			}
		}
		else if (rawList.children && Array.isArray(rawList.children) && rawList.children.length > 0) {
			this.getContentListRecursive(rawList.children, list, language);
		}
		else
			return;
	};

	this.getTabsAndContents = function() {
		var tabs = app.map.uiManager.notebookbar.getTabs();
		var rawDefinitions = app.map.uiManager.notebookbar.getFullJSON();
		var contextContainer = null;

		while (rawDefinitions.children && Array.isArray(rawDefinitions.children) && rawDefinitions.children[0] && contextContainer === null) {
			if (rawDefinitions.children[0].id === 'ContextContainer')
				contextContainer = rawDefinitions.children[0]; // We get the tab pages array here.
			else
				rawDefinitions = rawDefinitions.children[0];
		}

		if (!contextContainer || !Array.isArray(tabs))
			return;
		else {
			for (var i = 0; i < tabs.length; i++) {
				var tabName = tabs[i].id.split('-')[0];
				tabs[i].rawContentList = [];

				for (var j = 0; j < contextContainer.children.length; j++) {
					if (contextContainer.children[j].children[0].id === tabName + '-container') {
						tabs[i].rawContentList = contextContainer.children[j].children[0].children;
						break;
					}
				}
			}
		}

		var defs = {};
		var language = this.getLanguage();
		for (i = 0; i < tabs.length; i++) {
			if (tabs[i].accessibility && tabs[i].accessibility.focusBack !== undefined) {
				defs[tabs[i].id] = tabs[i];
				defs[tabs[i].id].focusBack = tabs[i].accessibility.focusBack;
				defs[tabs[i].id].combination = language && tabs[i].accessibility[language] ? tabs[i].accessibility[language]: tabs[i].accessibility.combination;
				defs[tabs[i].id].contentList = [];
				this.getContentListRecursive(defs[tabs[i].id].rawContentList, defs[tabs[i].id].contentList, language);
				delete defs[tabs[i].id].rawContentList;
			}
		}

		return defs;
	};

	this.getLanguage = function() {
		if (app.UI.language.fromURL && app.UI.language.fromURL !== '') {
			return app.UI.language.fromURL;
		}
		else
			return null; // Default.
	};

	this.checkIntegratorButtons = function(selectedDefinitions) {
		// The list of containers of the buttons which are added by integrations (via insertbutton post message etc).
		var containerList = ['save', 'userListHeader', 'shortcutstoolbox', 'closebuttonwrapper', 'navigator-floating-icon', 'viewModeDropdownButton'];
		for (var i = 0; i < containerList.length; i++) {
			var container = document.getElementById(containerList[i]);

			if (container) {
				// All the buttons inside the container which are visible on screen.
				var buttonList = Array.from(container.querySelectorAll('button'));
				if (buttonList.length) {
					for (var j = 0; j < buttonList.length; j++) {
						var button = buttonList[j];
						if (button.accessKey && button.id) {
							if (selectedDefinitions[button.id] === undefined) {
								selectedDefinitions[button.id] = {
									focusBack: true,
									combination: button.accessKey,
									contentList: []
								};
							}
						}
					}
				}
			}
		}
	};

	this.optionsToolButtons = function(selectedDefinitions) {
		// add optionstoolbox accesskey information to selected definitions

		const optionsToolSectionData = app.map.uiManager.notebookbar.getDefaultToolItems();
		const language = this.getLanguage();

		for (let option = 0; option < optionsToolSectionData.length; option++) {
			let toolOption = optionsToolSectionData[option];
			toolOption.id = (toolOption.id == null ? toolOption.command.replace('.uno:', '') : toolOption.id);

			let element = document.getElementById(toolOption.id + '-button');
			if (!element)
			{
				const button = document.querySelector('[modelid="'+ toolOption.id + '"] .unobutton');
				toolOption.id = button ? button.id : toolOption.id + '-button';
			}
			else
				toolOption.id += '-button';

			selectedDefinitions[toolOption.id] = {
				focusBack : toolOption.accessibility.focusBack,
				combination : language && toolOption.accessibility[language] ? toolOption.accessibility[language]: toolOption.accessibility.combination,
				contentList: []
			};
		}
	}

	this.getDefinitions = function() {
		var selectedDefinitions = this.getTabsAndContents();
		this.checkIntegratorButtons(selectedDefinitions);
		this.optionsToolButtons(selectedDefinitions);

		return selectedDefinitions;
	};
};
