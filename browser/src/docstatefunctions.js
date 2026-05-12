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
 * Doc state functions.
 * This file is meant to be used for setting and getting the document states.
 */

/* global app _ cool */

window.addEventListener('load', function () {
	app.calc.cellCursorRectangle = new cool.SimpleRectangle(0, 0, 0, 0);
	app.calc.cellAddress = new cool.SimplePoint(0, 0);
	app.calc.splitCoordinate = new cool.SimplePoint(0, 0);
	app.file.textCursor.rectangle = new cool.SimpleRectangle(0, 0, 0, 0);
	app.tile.size = new cool.SimplePoint(0, 0);
	app.pixelsToTwips = 15;
	app.twipsToPixels = 1 / app.pixelsToTwips;
	app.tile.size.pX = app.tile.size.pY = 256;
});

app.getViewRectangles = function () {
	if (app.map._docLayer._splitPanesContext)
		return app.map._docLayer._splitPanesContext.getViewRectangles();
	else return [app.activeDocument.activeLayout.viewedRectangle.clone()];
};

// ToDo: _splitPanesContext should be an app variable.
app.isPointVisibleInTheDisplayedArea = function (twipsArray /* x, y */) {
	if (app.map._docLayer._splitPanesContext) {
		let rectangles = app.map._docLayer._splitPanesContext.getViewRectangles();
		for (let i = 0; i < rectangles.length; i++) {
			if (rectangles[i].containsPoint(twipsArray)) return true;
		}
		return false;
	} else {
		return app.activeDocument.activeLayout.viewedRectangle.containsPoint(
			twipsArray,
		);
	}
};

app.isXVisibleInTheDisplayedArea = function (twipsX) {
	if (app.map._docLayer._splitPanesContext) {
		let rectangles = app.map._docLayer._splitPanesContext.getViewRectangles();
		for (let i = 0; i < rectangles.length; i++) {
			if (rectangles[i].containsX(twipsX)) return true;
		}
		return false;
	} else {
		return app.activeDocument.activeLayout.viewedRectangle.containsX(twipsX);
	}
};

app.isYVisibleInTheDisplayedArea = function (twipsY) {
	if (app.map._docLayer._splitPanesContext) {
		let rectangles = app.map._docLayer._splitPanesContext.getViewRectangles();
		for (let i = 0; i < rectangles.length; i++) {
			if (rectangles[i].containsY(twipsY)) return true;
		}
		return false;
	} else {
		return app.activeDocument.activeLayout.viewedRectangle.containsY(twipsY);
	}
};

app.isRectangleVisibleInTheDisplayedArea = function (
	twipsArray /* x, y, width, height */,
) {
	if (app.map._docLayer._splitPanesContext) {
		let rectangles = app.map._docLayer._splitPanesContext.getViewRectangles();
		for (let i = 0; i < rectangles.length; i++) {
			if (rectangles[i].intersectsRectangle(twipsArray)) return true;
		}
		return false;
	} else {
		return app.activeDocument.activeLayout.viewedRectangle.intersectsRectangle(
			twipsArray,
		);
	}
};

app.isXOrdinateInFrozenPane = function (pixelX) {
	if (app.map._docLayer._splitPanesContext) {
		const splitPos = app.map._docLayer._splitPanesContext.getSplitPos();

		if (pixelX < splitPos.x && pixelX >= 0) return true;
		else return false;
	} else return false;
};

app.isYOrdinateInFrozenPane = function (pixelY) {
	if (app.map._docLayer._splitPanesContext) {
		const splitPos = app.map._docLayer._splitPanesContext.getSplitPos();

		if (pixelY < splitPos.y && pixelY >= 0) return true;
		else return false;
	} else return false;
};

app.isReadOnly = function () {
	return app.file.readOnly;
};

app.isViewModeExtension = function (extension) {
	if (app.file.viewModeExtensions && extension) {
		var extensionList = app.file.viewModeExtensions.split('|');
		return extensionList.indexOf(extension.toLowerCase()) !== -1;
	}
	return false;
};

app.getScale = function () {
	return ((app.tile.size.pX / app.tile.size.x) * 15) / app.dpiScale;
};

app.setCursorVisibility = function (visible) {
	app.file.textCursor.visible = visible;
};

app.isCommentEditingAllowed = function () {
	return app.file.editComment;
};

app.isRedlineManagementAllowed = function () {
	return app.file.allowManageRedlines;
};

app.setPermission = function (permission) {
	app.file.permission = permission;
	if (permission === 'edit') {
		app.file.readOnly = false;
		app.file.editComment = true;
		app.file.allowManageRedlines = true;
	} else if (permission === 'readonly') {
		app.file.readOnly = true;
		app.file.editComment = false;
		app.file.allowManageRedlines = false;
	}
};

app.setCommentEditingPermission = function (isAllowed) {
	app.file.editComment = isAllowed;
};

app.setRedlineManagementAllowed = function (isAllowed) {
	app.file.allowManageRedlines = isAllowed;
};

app.getPermission = function () {
	return app.file.permission;
};

app.registerExportFormat = function (label, format) {
	var duplicate = false;
	for (var i = 0; i < app.file.exportFormats.length; i++) {
		if (
			app.file.exportFormats[i].label == label &&
			app.file.exportFormats[i].format == format
		) {
			duplicate = true;
			break;
		}
	}

	if (duplicate == false) {
		app.file.exportFormats.push({ label: label, format: format });
	}
};

app.getFollowedViewId = function () {
	return app.following.viewId;
};

app.setFollowingOff = function () {
	app.console.debug('user following: OFF');
	app.following.mode = 'none';
	app.following.viewId = -1;
};

app.setFollowingUser = function (viewId) {
	app.console.debug('user following: USER - ' + viewId);
	app.following.mode = 'user';
	app.following.viewId = viewId;
};

app.setFollowingEditor = function (viewId = -1) {
	app.console.debug('user following: EDITOR - ' + viewId);
	app.following.mode = 'editor';
	app.following.viewId = viewId;
};

app.isFollowingOff = function () {
	return app.following.mode === 'none';
};

app.isFollowingUser = function () {
	return app.following.mode === 'user';
};

app.isFollowingEditor = function () {
	return app.following.mode === 'editor';
};

app.updateFollowingUsers = function () {
	app.console.debug('user following: update');
	var isCellCursorVisible = app.calc.cellCursorVisible;
	var isTextCursorVisible = app.file.textCursor.visible;

	if (isCellCursorVisible || isTextCursorVisible) {
		let twipsArray = [];
		if (isCellCursorVisible)
			twipsArray = [
				app.calc.cellCursorRectangle.x2,
				app.calc.cellCursorRectangle.y2,
			];
		else
			twipsArray = [
				app.file.textCursor.rectangle.x2,
				app.file.textCursor.rectangle.y2,
			];

		const cursorPositionInView =
			app.isPointVisibleInTheDisplayedArea(twipsArray);

		if (
			parseInt(app.getFollowedViewId()) ===
				parseInt(app.map._docLayer._viewId) &&
			!cursorPositionInView
		) {
			app.setFollowingOff();
		} else if (
			parseInt(app.getFollowedViewId()) === -1 &&
			cursorPositionInView
		) {
			app.setFollowingUser(parseInt(app.map._docLayer._viewId));
		}
	}
};

app.showAsyncDownloadError = function (response, initialMsg) {
	const reader = new FileReader();
	const timeout = 10000;
	reader.onload = function () {
		if (reader.result === 'wrong server') {
			initialMsg += _(', cluster configuration error: mis-matching serverid');
			app.map.uiManager.showSnackbar(initialMsg, '', null, timeout);
		} else {
			app.map.uiManager.showSnackbar(initialMsg, '', null, timeout);
		}
	};
	reader.readAsText(response);
};

app.calc.isRTL = function () {
	if (!app.map._docLayer || !app.map._docLayer._lastStatusJSON) return false;

	const part =
		app.map._docLayer._lastStatusJSON.parts[app.map._docLayer._selectedPart];

	if (part) return part.rtllayout !== 0;
	else return false;
};

app.setServerAuditFromCore = function (entries) {
	app.serverAudit = entries;
	app.map.fire('receivedserveraudit');
};

app.isExperimentalMode = function () {
	if (app.socket && app.socket.WSDServer && app.socket.WSDServer.Options)
		return app.socket.WSDServer.Options.indexOf('E') !== -1;
	return false;
};

app.calc.isPartHidden = function (part) {
	if (!app.map._docLayer || !app.map._docLayer._lastStatusJSON) return false;
	if (part >= app.map._docLayer._lastStatusJSON.parts.length) return false;

	return app.map._docLayer._lastStatusJSON.parts[part].visible === 0; // ToDo: Move _lastStatusJSON into docstate.ts
};

app.calc.isPartProtected = function (part) {
	if (!app.map._docLayer || !app.map._docLayer._lastStatusJSON) return false;
	if (part >= app.map._docLayer._lastStatusJSON.parts.length) return false;

	return app.map._docLayer._lastStatusJSON.parts[part].protected === 1;
};

app.calc.isPartSheetView = function (part) {
	if (!app.map._docLayer || !app.map._docLayer._lastStatusJSON) return false;
	if (part >= app.map._docLayer._lastStatusJSON.parts.length) return false;

	return app.map._docLayer._lastStatusJSON.parts[part].sheetviewid >= 0;
};

app.calc.getPartColor = function (part) {
	if (!app.map._docLayer || !app.map._docLayer._lastStatusJSON) return null;
	const parts = app.map._docLayer._lastStatusJSON.parts;
	if (part >= parts.length) return null;
	return parts[part].color || null;
};

app.calc.isSelectedPartSheetView = function () {
	if (!app.map._docLayer || !app.map._docLayer._lastStatusJSON) return false;

	return app.calc.isPartSheetView(app.map._docLayer._selectedPart);
};

// Checks if the given part is the default (base) sheet of the currently selected sheet view.
app.calc.isDefaultPartOfSelectedSheetView = function (part) {
	if (!app.map._docLayer || !app.map._docLayer._lastStatusJSON) return false;
	var selectedPart = app.map._docLayer._selectedPart;
	if (!app.calc.isPartSheetView(selectedPart)) return false;

	var parts = app.map._docLayer._lastStatusJSON.parts;
	if (part >= parts.length) return false;

	var selectedPartData = parts[selectedPart];
	return (
		selectedPartData.defaultviewhash &&
		selectedPartData.defaultviewhash === parts[part].hash
	);
};

// Returns the sheet view part index that corresponds to the given default (base) part, or -1 if none.
app.calc.getSheetViewPartForDefaultPart = function (part) {
	if (!app.map._docLayer || !app.map._docLayer._lastStatusJSON) return -1;
	var parts = app.map._docLayer._lastStatusJSON.parts;
	if (part >= parts.length) return -1;

	var partHash = parts[part].hash;
	if (!partHash) return -1;

	for (var i = 0; i < parts.length; i++) {
		if (parts[i].sheetviewid >= 0 && parts[i].defaultviewhash === partHash) {
			return i;
		}
	}

	return -1;
};

// Updates the active sheet view part based on the selected part.
app.calc.updateActiveSheetView = function (selectedPart) {
	if (!app.map._docLayer || !app.map._docLayer._lastStatusJSON) return;

	if (app.calc.isPartSheetView(selectedPart)) {
		var parts = app.map._docLayer._lastStatusJSON.parts;
		var defaultViewHash = parts[selectedPart].defaultviewhash;
		if (defaultViewHash) {
			for (var i = 0; i < parts.length; i++) {
				if (parts[i].hash === defaultViewHash) {
					app.calc.partWithActiveSheetView = i;
					return;
				}
			}
		}
	}

	// Only clear when navigating back to the default part itself.
	if (selectedPart === app.calc.partWithActiveSheetView)
		app.calc.partWithActiveSheetView = -1;
};

// Checks if the given part has an active sheet view.
app.calc.hasActiveSheetView = function (part) {
	return app.calc.partWithActiveSheetView === part;
};

app.calc.isAnyPartHidden = function () {
	if (!app.map._docLayer || !app.map._docLayer._lastStatusJSON) return false;

	for (let i = 0; i < app.map._docLayer._lastStatusJSON.parts.length; i++) {
		if (app.map._docLayer._lastStatusJSON.parts[i].visible === 0) return true;
	}
	return false;
};

app.calc.getHiddenPartCount = function () {
	if (!app.map._docLayer || !app.map._docLayer._lastStatusJSON) return 0;

	let count = 0;

	for (let i = 0; i < app.map._docLayer._lastStatusJSON.parts.length; i++) {
		if (app.map._docLayer._lastStatusJSON.parts[i].visible === 0) count++;
	}

	return count;
};

app.calc.getVisiblePartCount = function () {
	if (!app.map._docLayer || !app.map._docLayer._lastStatusJSON) return 0;

	let count = 0;

	for (let i = 0; i < app.map._docLayer._lastStatusJSON.parts.length; i++) {
		if (app.map._docLayer._lastStatusJSON.parts[i].visible === 1) count++;
	}

	return count;
};

app.calc.getHiddenPartNameArray = function () {
	if (!app.map._docLayer || !app.map._docLayer._lastStatusJSON) return [];

	let array = [];

	for (let i = 0; i < app.map._docLayer._lastStatusJSON.parts.length; i++) {
		let part = app.map._docLayer._lastStatusJSON.parts[i];
		if (part.visible === 0) array.push(part.name);
	}

	return array;
};

app.impress.isSlideHidden = function (index) {
	if (app.impress.partList) {
		if (app.impress.partList.length > index)
			return !app.impress.partList[index].visible;
		else {
			app.console.warn(
				'Index is bigger than the part count (isSlideHidden): ' + index,
			);
			return true;
		}
	} else return false;
};

app.impress.areAllSlidesHidden = function () {
	if (app.impress.partList) {
		for (let i = 0; i < app.impress.partList.length; i++) {
			if (app.impress.partList[i].visible === 1) return false;
		}
		return true;
	} else return false;
};

app.impress.getSelectedSlidesCount = function () {
	let count = 0;

	if (app.impress.partList) {
		for (let i = 0; i < app.impress.partList.length; i++) {
			if (app.impress.partList[i].selected === 1) count++;
		}
	}

	return count;
};

app.impress.getIndexFromSlideHash = function (hash) {
	if (app.impress.partList) {
		for (let i = 0; i < app.impress.partList.length; i++) {
			if (app.impress.partList[i].hash === hash) return i;
		}

		app.console.warn('No part with hash (getIndexFromSlideHash): ' + hash);

		return 0;
	} else return 0;
};

app.impress.isSlideSelected = function (index) {
	if (
		app.impress.partList &&
		index >= 0 &&
		index < app.impress.partList.length
	) {
		return app.impress.partList[index].selected === 1;
	} else return false;
};

app.enterRAF = function () {
	if (app.map._debug) app.map._debug.enterRAF();
};

app.exitRAF = function () {
	if (app.map._debug) app.map._debug.exitRAF();
};
