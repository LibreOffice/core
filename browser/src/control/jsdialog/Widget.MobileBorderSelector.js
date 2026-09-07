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
 * JSDialog.MobileBorderSelector - widget for selecting border style on mobile
 *
 * Example JSON:
 * {
 *     id: 'id',
 *     type: 'borderstyle'
 * }
 */

/* global _ JSDialog $ app */

function _getCurrentBorderNumber(builder) {
	var outer = builder.map['stateChangeHandler'].getItemValue('.uno:BorderOuter');
	var inner = builder.map['stateChangeHandler'].getItemValue('.uno:BorderInner');

	if (!outer || !inner)
		return 0;

	var left = outer.left === 'true';
	var right = outer.right === 'true';
	var bottom = outer.bottom === 'true';
	var top = outer.top === 'true';
	var horiz = inner.horizontal === 'true';
	var vert = inner.vertical === 'true';

	if (left && !right && !bottom && !top && !horiz && !vert) {
		return 2;
	} else if (!left && right && !bottom && !top && !horiz && !vert) {
		return 3;
	} else if (left && right && !bottom && !top && !horiz && !vert) {
		return 4;
	} else if (!left && !right && !bottom && top && !horiz && !vert) {
		return 5;
	} else if (!left && !right && bottom && !top && !horiz && !vert) {
		return 6;
	} else if (!left && !right && bottom && top && !horiz && !vert) {
		return 7;
	} else if (left && right && bottom && top && !horiz && !vert) {
		return 8;
	} else if (!left && !right && bottom && top && horiz && !vert) {
		return 9;
	} else if (left && right && bottom && top && horiz && !vert) {
		return 10;
	} else if (left && right && bottom && top && !horiz && vert) {
		return 11;
	} else if (left && right && bottom && top && horiz && vert) {
		return 12;
	}

	return 1;
}

// Returns the color for a border preset as a '#rrggbb' string. When the selection already has
// borders this is their color. Otherwise it is the color last chosen in the line color picker,
// falling back to black.
function _getBorderLineColor(item, builder) {
	const stateColor = builder.map['stateChangeHandler'].getItemValue(item.command);
	if (parseInt(stateColor) >= 0)
		return JSDialog.getCurrentColor(item, builder);

	const lastColor = app.colorLastSelection[item.command];
	if (typeof lastColor === 'string' && /^#?[0-9a-fA-F]{6}$/.test(lastColor))
		return lastColor.startsWith('#') ? lastColor : '#' + lastColor;

	return JSDialog.getCurrentColor(item, builder);
}

function _borderControlItem(parentContainer, data, builder, i, selected) {
	var button = null;

	var div = builder._createIdentifiable('div', 'unotoolbutton mobile-wizard ui-content unospan', parentContainer, data);

	var buttonId = 'border-' + i;
	button = window.L.DomUtil.create('img', 'ui-content borderbutton', div);
	app.LOUtil.setImage(button, 'lc_fr0' + i + '.svg', builder.map);
	button.id = buttonId;
	if (selected)
		$(button).addClass('selected');

	$(div).click(function () {
		var color;
		// Find our associated color picker
		var item = app.LOUtil.findItemWithAttributeRecursive(data.parent, 'command', '.uno:FrameLineColor');
		if (item)
			color = _getBorderLineColor(item, builder);
		window.setBorderStyle(i, color);
	});
}

JSDialog.mobileBorderSelector = function (parentContainer, data, builder) {
	var mainContainer = window.L.DomUtil.create('div', builder.options.cssClass + ' ui-mobileborderselector', parentContainer);
	mainContainer.id = data.id;

	if (data.enabled === false) {
		mainContainer.disabled = true;
	}

	var bordercontrollabel = window.L.DomUtil.create('label', builder.options.cssClass + ' ui-text', mainContainer);
	bordercontrollabel.textContent = _('Cell borders');
	bordercontrollabel.id = data.id + 'label';
	var current = _getCurrentBorderNumber(builder);
	for (var i = 1; i < 13; ++i)
		_borderControlItem(mainContainer, data, builder, i, i === current);

	var updateFunction = function() {
		var current = _getCurrentBorderNumber(builder);
		for (var i = 1; i < 13; ++i) {
			if (i !== current)
				$('#border-' + i).removeClass('selected');
			else
				$('#border-' + i).addClass('selected');
		}
	};

	builder.map.on('commandstatechanged', function(e) {
		if (e.commandName === '.uno:BorderOuter' || e.commandName === '.uno:BorderInner')
			updateFunction();
	}, this);

	return false;
};
