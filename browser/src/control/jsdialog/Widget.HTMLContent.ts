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
 * Util.HTMLContent - widgets created from plain HTML
 */

declare var JSDialog: any;

interface StatusbarItemMetadata {
	cooltip: string;
	configLabel?: string;
}

// Statusbar HTML item strings.
// cooltip is shown as data-cooltip on the element.
// configLabel is the menu entry text in the status bar config menu
function getStatusbarItemMetadata(id: string): StatusbarItemMetadata {
	switch (id) {
		case 'statepagenumber':
			return {
				cooltip: _('Number of Pages. Click to open the Go to Page dialog box.'),
				configLabel: _('Number of Pages'),
			};
		case 'statewordcount':
			return { cooltip: _('Word Counter'), configLabel: _('Word Counter') };
		case 'statusdocpos':
			return {
				cooltip: _('Number of Sheets'),
				configLabel: _('Number of Sheets'),
			};
		case 'rowcolselcount':
			return {
				cooltip: _('Selected range of cells'),
				configLabel: _('Cell Selection'),
			};
		case 'insertmode':
			return {
				cooltip: _('Entering text mode'),
				configLabel: _('Insert Mode'),
			};
		case 'showcomments':
			return { cooltip: _('Show Comments'), configLabel: _('Comments') };
		case 'statusselectionmode':
			return { cooltip: _('Selection Mode') };
		case 'slidestatus':
			return {
				cooltip: _('Number of Slides'),
				configLabel: _('Number of Slides'),
			};
		case 'pagestatus':
			return {
				cooltip: _('Number of Pages'),
				configLabel: _('Number of Pages'),
			};
		case 'statetablecell':
			return {
				cooltip: _('Choice of functions'),
				configLabel: _('Table Cell Functions'),
			};
		case 'prevpage': {
			const docType = app.map && app.map.getDocType ? app.map.getDocType() : '';
			const label =
				docType === 'spreadsheet'
					? _('Sheet Navigation')
					: docType === 'presentation'
						? _('Slide Navigation')
						: _('Page Navigation');
			return { cooltip: '', configLabel: label };
		}
		case 'permissionmode':
			return { cooltip: _('Permission Mode') };
		case 'documentstatus':
			return { cooltip: _('Your changes have been saved') };
	}
	return { cooltip: '' };
}

function getStatusbarItemConfigLabel(id: string): string | null {
	return getStatusbarItemMetadata(id).configLabel || null;
}

function sanitizeString(text: string): string {
	const sanitizer = document.createElement('div');
	sanitizer.innerText = text;
	return sanitizer.innerHTML;
}

function getPermissionModeElements(
	isReadOnlyMode: boolean,
	canUserWrite: boolean,
	map: any,
) {
	const permissionModeDiv = document.createElement('div');
	permissionModeDiv.className = 'jsdialog ui-badge';

	const cooltip = getStatusbarItemMetadata('permissionmode').cooltip;

	if (isReadOnlyMode && !canUserWrite) {
		permissionModeDiv.classList.add('status-readonly-mode');
		permissionModeDiv.textContent = _('Read-only');
		permissionModeDiv.setAttribute('data-cooltip', cooltip);
		window.L.control.attachTooltipEventListener(permissionModeDiv, map);
	} else if (isReadOnlyMode && canUserWrite) {
		permissionModeDiv.classList.add('status-readonly-transient-mode');
		permissionModeDiv.style.display = 'none';
	} else {
		permissionModeDiv.classList.add('status-edit-mode');
		permissionModeDiv.textContent = _('Edit mode');
		permissionModeDiv.setAttribute('data-cooltip', cooltip);
		window.L.control.attachTooltipEventListener(permissionModeDiv, map);
	}

	return permissionModeDiv;
}

function getStatusbarItemElement(
	id: string,
	title: string,
	text: string,
	builder: any,
	renderAsButton = false,
) {
	const element = document.createElement(renderAsButton ? 'button' : 'div');
	element.id = id;
	element.className =
		'jsdialog ui-badge' + (renderAsButton ? ' unobutton' : '');
	element.textContent = text;
	element.setAttribute('data-cooltip', title);
	window.L.control.attachTooltipEventListener(element, builder.map);

	return element;
}

function getPageNumberElements(text: string, builder: any) {
	const button = getStatusbarItemElement(
		'StatePageNumber',
		getStatusbarItemMetadata('statepagenumber').cooltip,
		text,
		builder,
		true,
	);
	button.onclick = () => app.map.sendUnoCommand('.uno:GotoPage');
	return button;
}

function getWordCountElements(text: string, builder: any) {
	return getStatusbarItemElement(
		'StateWordCount',
		getStatusbarItemMetadata('statewordcount').cooltip,
		text,
		builder,
	);
}

function getStatusDocPosElements(text: string, builder: any) {
	const button = getStatusbarItemElement(
		'StatusDocPos',
		getStatusbarItemMetadata('statusdocpos').cooltip,
		text,
		builder,
		true,
	);
	button.onclick = () => app.map.sendUnoCommand('.uno:JumpToTable');
	return button;
}

function getInsertModeElements(text: string, builder: any) {
	return getStatusbarItemElement(
		'InsertMode',
		getStatusbarItemMetadata('insertmode').cooltip,
		text,
		builder,
	);
}

function getSelectionModeElements(text: string, builder: any) {
	return getStatusbarItemElement(
		'StatusSelectionMode',
		getStatusbarItemMetadata('statusselectionmode').cooltip,
		text,
		builder,
	);
}

function getRowColSelCountElements(text: string, builder: any) {
	return getStatusbarItemElement(
		'RowColSelCount',
		getStatusbarItemMetadata('rowcolselcount').cooltip,
		text,
		builder,
	);
}

function getStateTableCellElements(text: string, builder: any) {
	return getStatusbarItemElement(
		'StateTableCell',
		getStatusbarItemMetadata('statetablecell').cooltip,
		text,
		builder,
	);
}

function getSlideStatusElements(text: string, builder: any) {
	const button = getStatusbarItemElement(
		'SlideStatus',
		getStatusbarItemMetadata('slidestatus').cooltip,
		text,
		builder,
		true,
	);
	button.onclick = () => app.map.sendUnoCommand('.uno:GotoPage');
	return button;
}

function getPageStatusElements(text: string, builder: any) {
	const button = getStatusbarItemElement(
		'PageStatus',
		getStatusbarItemMetadata('pagestatus').cooltip,
		text,
		builder,
		true,
	);
	button.onclick = () => app.map.sendUnoCommand('.uno:GotoPage');
	return button;
}

function getDocumentStatusElements(text: string, builder: any) {
	const cooltip = getStatusbarItemMetadata('documentstatus').cooltip;
	const docstat = getStatusbarItemElement(
		'DocumentStatus',
		cooltip,
		'',
		builder,
	);

	if (text === 'SAVING') docstat.textContent = _('Saving...');
	else if (text === 'SAVED') {
		const lastSaved = document.createElement('span');
		lastSaved.id = 'last-saved';
		lastSaved.textContent = '';
		lastSaved.setAttribute('data-cooltip', cooltip + '.');
		window.L.control.attachTooltipEventListener(lastSaved, builder.map);
		docstat.appendChild(lastSaved);
	}

	return docstat;
}

function getShowCommentsStatusElements(text: string, builder: any) {
	return getStatusbarItemElement(
		'ShowComments',
		getStatusbarItemMetadata('showcomments').cooltip,
		text,
		builder,
	);
}

var getElementsFromId = function (
	id: string,
	closeCallback: EventListenerOrEventListenerObject,
	data: HtmlContentJson,
	builder: any,
) {
	if (id === 'iconset')
		return (window as any).getConditionalFormatMenuElements(
			'iconsetoverlay',
			true,
		);
	else if (id === 'scaleset')
		return (window as any).getConditionalColorScaleMenuElements(
			'iconsetoverlay',
			true,
		);
	else if (id === 'databarset')
		return (window as any).getConditionalDataBarMenuElements(
			'iconsetoverlay',
			true,
		);
	else if (id === 'inserttablepopup')
		return (window as any).getInsertTablePopupElements(closeCallback);
	else if (id === 'insertshapespopup')
		return (window as any).getShapesPopupElements(closeCallback);
	else if (id === 'insertconnectorspopup')
		return (window as any).getConnectorsPopupElements(closeCallback);
	else if (id === 'userslistpopup')
		return window.L.control.createUserListWidget();
	else if (id === 'permissionmode')
		return getPermissionModeElements(
			data.isReadOnlyMode,
			data.canUserWrite,
			builder.map,
		);
	else if (id === 'statepagenumber')
		return getPageNumberElements(data.text, builder);
	else if (id === 'statewordcount')
		return getWordCountElements(data.text, builder);
	else if (id === 'showcomments')
		return getShowCommentsStatusElements(data.text, builder);
	else if (id === 'statusdocpos')
		return getStatusDocPosElements(data.text, builder);
	else if (id === 'insertmode')
		return getInsertModeElements(data.text, builder);
	else if (id === 'statusselectionmode')
		return getSelectionModeElements(data.text, builder);
	else if (id === 'rowcolselcount')
		return getRowColSelCountElements(data.text, builder);
	else if (id === 'statetablecell')
		return getStateTableCellElements(data.text, builder);
	else if (id === 'slidestatus')
		return getSlideStatusElements(data.text, builder);
	else if (id === 'pagestatus')
		return getPageStatusElements(data.text, builder);
	else if (id === 'documentstatus')
		return getDocumentStatusElements(data.text, builder);
};

function htmlContent(
	parentContainer: Element,
	data: HtmlContentJson,
	builder: JSBuilder,
) {
	parentContainer.replaceChildren();

	const elements = getElementsFromId(
		data.htmlId,
		data.closeCallback,
		data,
		builder,
	);

	parentContainer.appendChild(elements);

	// TODO: remove this and create real widget for userslistpopup
	if (data.htmlId === 'userslistpopup')
		setTimeout(() => builder.map.userList.renderAll(), 0);

	if (data.enabled === false && parentContainer.firstChild)
		(parentContainer.firstChild as HTMLElement).setAttribute(
			'disabled',
			'true',
		);
}

JSDialog.htmlContent = htmlContent;
JSDialog.getStatusbarItemConfigLabel = getStatusbarItemConfigLabel;
