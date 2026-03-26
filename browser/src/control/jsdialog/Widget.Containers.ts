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
 * JSDialog.Containers - various container widgets
 */

declare var JSDialog: any;

function _getGridChild(
	children: Array<WidgetJSON>,
	row: number,
	col: number,
): WidgetJSON {
	for (var index in children) {
		if (
			parseInt(children[index].top) == row &&
			parseInt(children[index].left) == col
		)
			return children[index];
	}
	return null;
}

JSDialog.container = function (
	parentContainer: Element,
	data: ContainerWidgetJSON | GridWidgetJSON,
	builder: JSBuilder,
) {
	if ((data as GridWidgetJSON).cols && (data as GridWidgetJSON).rows)
		return JSDialog.grid(parentContainer, data, builder);

	let id = data.id;
	if ((!id || id === '') && builder)
		id = JSDialog.MakeIdUnique('unnamed-container');

	if (parentContainer && !parentContainer.id) parentContainer.id = id;

	// ARIA support for non-grid containers
	if (parentContainer) {
		if (data.allyRole) parentContainer.setAttribute('role', data.allyRole);
		if (data.aria?.label)
			parentContainer.setAttribute('aria-label', data.aria.label);
		if (data.ariaLive) parentContainer.setAttribute('aria-live', data.ariaLive);
	}

	return true;
};

JSDialog.grid = function (
	parentContainer: Element,
	data: GridWidgetJSON,
	builder: JSBuilder,
) {
	const rows = builder._getGridRows(data.children);
	const cols = builder._getGridColumns(data.children);

	const processedChildren = [];
	const isSingleChild = data.children && data.children.length === 1;

	const table = window.L.DomUtil.create(
		'div',
		builder.options.cssClass +
			' ui-grid' +
			(isSingleChild ? ' single-child-grid' : ''),
		parentContainer,
	);
	table.id = data.id;

	if (data.allyRole) {
		table.role = data.allyRole;

		if (data.allyRole === 'listbox' && data.initialSelectedId)
			table.setAttribute('aria-activedescendant', data.initialSelectedId);
	}

	if (data.tabIndex !== undefined)
		table.setAttribute('tabindex', data.tabIndex);

	const gridRowColStyle =
		'grid-template-rows: repeat(' +
		rows +
		', auto); \
		grid-template-columns: repeat(' +
		cols +
		', 1fr);' +
		(data.columnSpacing > 0 ? ' column-gap:' + data.columnSpacing + 'px;' : '');

	table.style = gridRowColStyle;

	for (let row = 0; row < rows; row++) {
		let prevChild = null;

		for (let col = 0; col < cols; col++) {
			const child = _getGridChild(data.children, row, col);
			const isMergedCell =
				prevChild &&
				prevChild.width &&
				parseInt(prevChild.left) + parseInt(prevChild.width) > col;

			if (child) {
				if (!child.id || child.id === '')
					// required for postprocess...
					child.id = table.id + '-cell-' + row + '-' + col;

				const sandbox = window.L.DomUtil.create('div');
				builder.build(sandbox, [child], false);

				const control = sandbox.firstChild;
				if (control) {
					window.L.DomUtil.addClass(control, 'ui-grid-cell');
					table.appendChild(control);
				}

				processedChildren.push(child);
				prevChild = child;
			} else if (!isMergedCell) {
				// empty placeholder to keep correct layout in some cases.
				window.L.DomUtil.create('div', 'ui-grid-cell', table);
			}
		}
	}

	for (let i = 0; i < (data.children || []).length; i++) {
		const child = data.children[i];
		if (processedChildren.indexOf(child) === -1) {
			const sandbox = window.L.DomUtil.create('div');
			builder.build(sandbox, [child], false);

			// For listbox role, process all children from sandbox
			// all children will be of role="option"
			if (data.allyRole === 'listbox') {
				const controls = Array.from(sandbox.children);
				for (let j = 0; j < controls.length; j++) {
					const control = controls[j];
					if (control) {
						window.L.DomUtil.addClass(control, 'ui-grid-cell');
						table.appendChild(control);
					}
				}
			} else {
				const control = sandbox.firstChild;
				if (control) {
					window.L.DomUtil.addClass(control, 'ui-grid-cell');
					table.appendChild(control);
				}
			}

			processedChildren.push(child);
		}
	}

	return false;
};

JSDialog.toolbox = function (
	parentContainer: Element,
	data: ToolboxWidgetJSON,
	builder: JSBuilder,
) {
	const levelClass =
		builder._currentDepth !== undefined
			? ' level-' + builder._currentDepth
			: '';
	const toolbox = window.L.DomUtil.create(
		'div',
		builder.options.cssClass + ' horizontal toolbox' + levelClass,
		parentContainer,
	);
	toolbox.id = data.id;

	if (data.enabled === false) {
		toolbox.disabled = true;
		for (const index in data.children) {
			data.children[index].enabled = false;
		}
	}

	if (data.aria?.role) {
		toolbox.setAttribute('role', data.aria.role);
	}

	const enabledCallback = function (enable: boolean) {
		for (const j in data.children) {
			const childId = data.children[j].id;
			const toolboxChild = toolbox.querySelector('#' + childId);
			if (!toolboxChild) continue;
			if (enable) {
				toolboxChild.removeAttribute('disabled');
				toolboxChild.classList.remove('disabled');
			} else {
				toolboxChild.disabled = true;
				toolboxChild.classList.add('disabled');
			}
		}
	};
	JSDialog.OnStateChange(toolbox, enabledCallback);

	// builder modifiers
	const noLabels = builder.options.noLabelsForUnoButtons;
	builder.options.noLabelsForUnoButtons = true;

	const inlineLabels = builder.options.useInLineLabelsForUnoButtons;
	if (data.hasVerticalParent === true && data.children.length === 1)
		builder.options.useInLineLabelsForUnoButtons = true;

	if (data.children.length === 1) {
		if (!data.children[0].labelledBy && data.labelledBy)
			data.children[0].labelledBy = data.labelledBy;
		else if (!data.children[0].aria && data.aria)
			data.children[0].aria = data.aria;
	} else {
		JSDialog.SetupA11yLabelForNonLabelableElement(toolbox, data, builder);
	}

	builder.build(toolbox, data.children, false);

	// reset modifiers
	builder.options.useInLineLabelsForUnoButtons = inlineLabels;
	builder.options.noLabelsForUnoButtons = noLabels;

	return false;
};

JSDialog.spacer = function (
	parentContainer: Element,
	data: WidgetJSON,
	builder: JSBuilder,
) {
	const spacer = window.L.DomUtil.create(
		'div',
		builder.options.cssClass + ' ui-spacer',
		parentContainer,
	);
	spacer.id = data.id;

	return false;
};
