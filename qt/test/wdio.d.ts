/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Multiremote browser shape exposed by WDIO in every spec file.
declare const browser: {
	webEngine: WebdriverIO.Browser & {
		waitForCondition(
			predicate: () => boolean,
			opts?: {
				timeout?: number;
				interval?: number;
				timeoutMsg?: string;
			},
		): Promise<boolean>;
	};
	native: WebdriverIO.Browser;
};

// Globals available inside browser.execute() callbacks (browser-side).
interface Window {
	postMobileMessage(msg: string): void;
}
declare const app: {
	map: {
		_docLoaded: boolean;
		getDocType(): string;
		// part is the part index, how is 0 deselect / 1 select / 2 toggle.
		selectPart(part: number, how: number, external: boolean): void;
		setPart(part: number): void;
		deselectAll(): void;
		isEditMode(): boolean;
		_enterEditMode(permission: string): void;
		_clip: {
			setTextSelectionType(selectionType: string): void;
			_execCopyCutPaste(operation: string, cmd?: string): void;
		};
		_docLayer: {
			// Authoritative slide count, set from the server's partscount.
			_parts: number;
			_preview: {
				_pasteSlide(nPos?: number): Promise<void>;
			};
		};
		backstageView?: {
			show(): void;
			hide(): void;
			toggle(): void;
		};
	};
	impress: {
		getSelectedSlidesCount(): number;
	};
	dispatcher: {
		dispatch(action: string): void;
	};
	activeDocument?: {
		activeLayout?: {
			type: string;
			documentRectangles?: Array<unknown>;
			viewedRectangle?: { pWidth: number; pHeight: number };
		};
	};
	layoutingService: {
		hasTasksPending(): boolean;
	};
};
