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
 * JSDialog.Combobox - combobox widget with support for custom renders of entries
 *
 * Example JSON:
 * {
 *     id: 'id',
 *     type: 'combobox',
 *     text: 'some text',
 *     entries: [ 'A', 'B', 'C' ],
 *     customEntryRenderer: true
 * }
 *
 * customEntryRenderer - specifies if entries have custom content which is rendered by the core
 */

/* global JSDialog app _ $ */

JSDialog.comboboxEntry = function (parentContainer, data, builder) {
	var entry = window.L.DomUtil.create('div', 'ui-combobox-entry ' + builder.options.cssClass, parentContainer);
	entry.id = data.id;
	entry.setAttribute('role', 'option');
	entry.setAttribute('tabindex', '-1');
	entry.setAttribute('data-filter-text', data.text.toLowerCase());

	if (data.hasSubMenu)
		window.L.DomUtil.addClass(entry, 'ui-has-menu');

	if (data.w2icon) {
		// FIXME: DEPRECATED, this is legacy way to setup icon based on CSS class
		window.L.DomUtil.create('div', 'w2ui-icon ui-combobox-icon ' + data.w2icon, entry);
	}

	if (data.icon) {
		var icon = window.L.DomUtil.create('img', 'ui-combobox-icon', entry);
		icon.alt = '';
		builder._isStringCloseToURL(data.icon) ? icon.src = data.icon : app.LOUtil.setImage(icon,  app.LOUtil.getIconNameOfCommand(data.icon), builder.map, true);
	}

	if (data.hint) {
		entry.title = data.hint;
	}

	var content = window.L.DomUtil.create('span', '', entry);
	content.innerText = data.text;

    if (data.selected) {
        entry.setAttribute('aria-selected', 'true');
		window.L.DomUtil.addClass(entry, 'selected');
    } else {
        entry.setAttribute('aria-selected', 'false');
    }

	if (data.checked)
		window.L.DomUtil.addClass(entry, 'checked');
	else if (data.checked !== undefined)
		window.L.DomUtil.addClass(entry, 'notchecked');

	if (data.customRenderer)
		JSDialog.OnDemandRenderer(builder, data.comboboxId, 'combobox', data.pos, content, entry, data.text);

	var entryData = data.pos + ';' + data.text;

	var clickFunction = function () {
		builder.callback('combobox', 'selected', {id: data.comboboxId}, entryData, builder);
	};

	entry.addEventListener('click', clickFunction);
	entry.addEventListener('keypress', function (event) {
        if (event.key === 'Enter' || event.key === ' ') {
			clickFunction();
			event.preventDefault();
		}
	});

	entry.addEventListener('keydown', function (event) {
        if (event.key === 'Tab') {
			JSDialog.CloseDropdown(data.comboboxId);
			event.preventDefault();
		}
	});

	if (data.hasSubMenu) {
		entry.setAttribute('aria-haspopup', true);
		entry.setAttribute('aria-expanded', false);
		entry._onDropDown = function(open) {
			entry.setAttribute('aria-expanded', open);
		};

		entry.addEventListener('mouseover', function () {
			builder.callback('combobox', 'showsubmenu', {id: data.comboboxId}, entryData, builder);
		});
	}

	return false;
};

JSDialog.mobileComboboxEntry = function(parentContainer, data, builder) {
	var comboboxEntry = window.L.DomUtil.create('p', builder.options.cssClass, parentContainer);
	comboboxEntry.textContent = builder._cleanText(data.text);

	comboboxEntry.parent = data.parent;

	if (data.style && data.style.length)
		window.L.DomUtil.addClass(comboboxEntry, data.style);

	comboboxEntry.addEventListener('click', function () {
		builder.refreshSidebar = true;
		if (builder.wizard)
			builder.wizard.goLevelUp();
		builder.callback('combobox', 'selected', comboboxEntry.parent, data.pos + ';' + comboboxEntry.textContent, builder);
	});

	return false;
};

JSDialog.mobileCombobox = function (parentContainer, data, builder) {
	var container = window.L.DomUtil.create('div', 'ui-explorable-entry level-' + builder._currentDepth + ' ' + builder.options.cssClass + ' ui-widget', parentContainer);
	if (data && data.id)
		container.id = data.id;

	var sectionTitle = window.L.DomUtil.create('div', 'ui-header level-' + builder._currentDepth + ' ' + builder.options.cssClass + ' ui-widget', container);
	$(sectionTitle).css('justify-content', 'space-between');

	var leftDiv = window.L.DomUtil.create('div', 'ui-header-left combobox', sectionTitle);

	var editCallback = function(value) {
		builder.callback('combobox', 'change', data, value, builder);
	};
	builder._controlHandlers['edit'](leftDiv, data, builder, editCallback);

	var rightDiv = window.L.DomUtil.create('div', 'ui-header-right', sectionTitle);

	var arrowSpan = window.L.DomUtil.create('span', 'sub-menu-arrow', rightDiv);
	arrowSpan.textContent = '>';

	var contentDiv = window.L.DomUtil.create('div', 'ui-content level-' + builder._currentDepth + ' ' + builder.options.cssClass, container);
	contentDiv.title = data.text;

	var entries = [];
	if (data.entries) {
		for (var index in data.entries) {
			var style = 'ui-combobox-text';
			if ((data.selectedEntries && index == data.selectedEntries[0])
				|| data.entries[index] == data.text) {
				style += ' selected';
			}

			var entry = { type: 'comboboxentry', text: data.entries[index], pos: index, parent: data, style: style };
			entries.push(entry);
		}
	}

	var contentNode = {type: 'container', children: entries};

	builder._currentDepth++;
	builder.build(contentDiv, [contentNode]);
	builder._currentDepth--;

	if (!data.nosubmenu)
	{
		$(contentDiv).hide();
		if (builder.wizard) {
			$(container).click(function(event, data) {
				builder.wizard.goLevelDown(container, data);
				if (contentNode && contentNode.onshow)
					contentNode.onshow();
			});
		} else {
			window.app.console.debug('Builder used outside of mobile wizard: please implement the click handler');
		}
	}
	else
		$(container).hide();

	container.onSelect = function (pos) {
		console.error('Not implemented: select entry: ' + pos);
	};

	container.onSetText = function (text) {
		console.error('Not implemented: setText: ' + text);
	};
};

function _extractPos(selectCommandData) {
	return selectCommandData.substr(0, selectCommandData.indexOf(';'));
}

function _extractText(selectCommandData) {
	return selectCommandData.substr(selectCommandData.indexOf(';') + 1);
}

JSDialog.combobox = function (parentContainer, data, builder) {
	var container = window.L.DomUtil.create('div', 'ui-combobox ' + builder.options.cssClass, parentContainer);
	container.id = data.id;

	var content = window.L.DomUtil.create('input', 'ui-combobox-content ' + builder.options.cssClass, container);
	content.id = data.id + '-input-' + builder.options.suffix;
	content.value = data.text;
	content.role = 'combobox';
	content.setAttribute('autocomplete', 'off');
	content.setAttribute('aria-autocomplete', 'list');

	JSDialog.SetupA11yLabelForLabelableElement(parentContainer, content, data, builder);

	var dropDownId = JSDialog.CreateDropdownEntriesId(data.id);
	content.setAttribute('aria-expanded', false);

	var button = window.L.DomUtil.create('button', 'ui-combobox-button ' + builder.options.cssClass, container);
	button.setAttribute('aria-expanded', false);

	const dataAriaLabel = data.aria && data.aria.label ? data.aria.label : '';
	const buttonARIALabel = dataAriaLabel
		? _('Open ') + dataAriaLabel + _(' list')
		: _('Open list');
	const updatedAriaLabelData = {
		labelledBy: data.labelledBy,
		aria: {
			label: buttonARIALabel
		}
	}
	JSDialog.SetupA11yLabelForLabelableElement(parentContainer, button, updatedAriaLabelData, builder);

	var arrow = window.L.DomUtil.create('span', builder.options.cssClass + ' ui-listbox-arrow', button);
	arrow.id = 'listbox-arrow-' + data.id;

	container._onDropDown = function (open) {
		content.setAttribute('aria-expanded', open);
		button.setAttribute('aria-expanded', open);

		// Only set aria-controls when dropdown is open to avoid screen reader confusion
		if (open) {
			content.setAttribute('aria-controls', dropDownId);
			button.setAttribute('aria-controls', dropDownId);
		} else {
			content.removeAttribute('aria-controls');
			button.removeAttribute('aria-controls');
		}
	};

	if (data.selectedCount > 0)
		var selectedEntryPos = parseInt(data.selectedEntries[0]);

	// convert to dropdown entries
	var entries = [];
	for (var i in data.entries) {
		entries.push({
			text: data.entries[i].toString(),
			selected: parseInt(i) === selectedEntryPos,
			customRenderer: data.customEntryRenderer
		});
	}

	var resetSelection = function () {
		for (var i in entries) {
			entries[i].selected = false;
		}
	};

	if (data.enabled === false) {
		container.disabled = true;
		content.disabled = true;
	}

	JSDialog.SynchronizeDisabledState(container, [content]);

	// notebookbar a11y requires main element to have click handler for shortcuts to work
	container.addEventListener('click', function () { content.focus(); });

	content.addEventListener('keyup', function (event) {
		const shouldTriggerChange = data.changeOnEnterOnly ? event.key === 'Enter' : true;
		if (shouldTriggerChange) {
			builder.callback('combobox', 'change', data, this.value, builder);
			if (data.focusMapOnEnter && event.key === 'Enter') {
				builder.map.focus();
			}
		}

		resetSelection();
		for (var i in entries) {
			if (entries[i] == this.value || entries[i].text == this.value) {
				entries[i].selected = true;
				break;
			}
		}
		// check for drop down is in open state or not.
		// If open then we should make focus in entries field for Arrow key navigation
		if (event.key === 'ArrowDown' && builder.map.jsdialog.hasDropdownOpened()) {
			const comboboxEntries = JSDialog.GetDropdown(data.id);
			const selectedElement = comboboxEntries.querySelector(".selected");
			if (selectedElement) {
				selectedElement.focus();
				return;
			}
		}
	});

	var comboboxId = data.id;
	var clickFunction = function () {
		if (container.hasAttribute('disabled'))
			return;

		var parentBuilder = builder;
		var callback = function(objectType, eventType, object, data) {
			// send command with correct WindowId (from parent, not dropdown)
			let result;
			if (eventType !== 'close')
				result = parentBuilder.callback(objectType, eventType, object, data, parentBuilder);

			// close after selection
			if (eventType === 'selected') {
				container.onSelect(_extractPos(data));
				container.onSetText(_extractText(data));

				// Pass through if the parent callback has already set the focus
				// somewhere that shouldn't be changed by the CloseDropdown, e.g.
				// toolbar font name/size
				JSDialog.CloseDropdown(comboboxId, result === 'focusHandled');
			}

			return true;
		};

		JSDialog.OpenDropdown(data.id, container, entries, callback);

		if (entries.length > 0) {
			// Inject search field after dropdown DOM is created.
			// Double-nest so it runs after setupInitialFocus.
			app.layoutingService.appendLayoutingTask(function () {
				app.layoutingService.appendLayoutingTask(function () {
					var dropdownRoot = JSDialog.GetDropdown(data.id);
					if (!dropdownRoot)
						return;

					var grid = dropdownRoot.querySelector('.ui-grid[role="listbox"]');
					if (!grid || grid.querySelector('.ui-combobox-search-input'))
						return;

					var searchContainer = document.createElement('div');
					searchContainer.className = 'ui-combobox-search-container';

					var searchInput = document.createElement('input');
					searchInput.type = 'search';
					searchInput.className = 'jsdialog ui-edit ui-combobox-search-input';
					searchInput.setAttribute('placeholder', _('Search...'));
					searchInput.setAttribute('aria-label', _('Filter entries'));
					searchInput.setAttribute('autocomplete', 'off');
					searchContainer.appendChild(searchInput);

					grid.insertAdjacentElement('afterbegin', searchContainer);

					searchInput.addEventListener('input', function () {
						var filterText = searchInput.value.trim().toLowerCase();
						var allEntries = grid.querySelectorAll('.ui-combobox-entry');
						allEntries.forEach(function (entry) {
							var text = entry.getAttribute('data-filter-text') || '';
							if (filterText === '' || text.indexOf(filterText) >= 0) {
								window.L.DomUtil.removeClass(entry, 'hidden');
							} else {
								window.L.DomUtil.addClass(entry, 'hidden');
							}
						});
					});

					searchInput.addEventListener('keydown', function (event) {
						if (event.key === 'ArrowDown') {
							var firstVisible = grid.querySelector('.ui-combobox-entry:not(.hidden)');
							if (firstVisible) {
								firstVisible.focus();
								event.preventDefault();
							}
						} else if (event.key === 'Enter') {
							var firstVisible = grid.querySelector('.ui-combobox-entry:not(.hidden)');
							if (firstVisible)
								firstVisible.click();
							event.preventDefault();
						} else if (event.key === 'Escape') {
							JSDialog.CloseDropdown(comboboxId);
							content.focus();
							event.preventDefault();
						}
					});

					searchInput.focus();
				});
			});
		}
	};

	button.addEventListener('click', clickFunction);
	button.addEventListener('keypress', function (event) {
		if (event.key === 'Enter' || event.key === ' ')
			clickFunction();
	});

	container.updateRenders = function (pos) {
		var dropdownRoot = JSDialog.GetDropdown(data.id);
		if (!dropdownRoot)
			return;

		var dropdown = dropdownRoot.querySelectorAll('.ui-combobox-entry');
		if (dropdown[pos]) {
			dropdown[pos].replaceChildren();
			var img = window.L.DomUtil.create('img', '', dropdown[pos]);
			img.src = builder.rendersCache[data.id].images[pos];
			img.alt = entries[pos].text;
			img.title = entries[pos].text;
		}
	};

	container.onSelect = function (pos) {
		resetSelection();
		if (pos >= 0 && entries[pos])
			entries[pos].selected = true;
		else if (!entries[pos])
			console.warn('Cannot find entry with pos: "' + pos + '" in "' + data.id + '"');
	};

	container.onSetText = function (text) {
		if (document.activeElement === content)
			return;
		content.value = text;
	};

	container.updateEntries = function (newEntries) {
		entries = [];
		for (var i = 0; i < newEntries.length; i++) {
			entries.push({
				text: newEntries[i].toString(),
				selected: false,
				customRenderer: data.customEntryRenderer
			});
		}
		if (JSDialog.GetDropdown(data.id))
			JSDialog.CloseDropdown(data.id);
	};

	return false;
};
