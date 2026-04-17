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

class DocumentBase {
	public readonly type: string = 'DocumentBase';
	public activeLayout: ViewLayoutBase;
	public tableMiddleware: TableMiddleware;
	public selectionMiddleware: ImpressSelectionMiddleware | null;
	public mouseControl: MouseControl | null = null;
	private _activeModes: number[] = [0];
	protected views: Map<number, DocumentViewBase> = new Map<
		number,
		DocumentViewBase
	>();
	protected activeViewID: number;
	public activeView: DocumentViewBase;
	private activeViewSelectionColor = 'lightblue'; // Overwritten in constructor.

	public partHasComments: boolean | undefined = undefined;
	protected _fileSize: cool.SimplePoint;

	constructor() {
		if (!app.activeDocument) app.activeDocument = this;

		if (app.map._docLayer._docType === 'text') {
			this.activeLayout = new ViewLayoutWriter();
		} else {
			this.activeLayout = new ViewLayoutBase();
		}
		this._fileSize = new cool.SimplePoint(0, 0);
		this.tableMiddleware = new TableMiddleware();

		this.tableMiddleware.setupTableOverlay();

		if (app.map._docLayer._docType === 'presentation')
			this.selectionMiddleware = new ImpressSelectionMiddleware();
		else this.selectionMiddleware = null;

		this.addSections();
		this.activeViewID = 0;
		this.activeView = new DocumentViewBase(this.activeViewID);

		const dummyDiv = document.createElement('div');
		dummyDiv.className = 'selections-data';
		document.body.appendChild(dummyDiv);
		this.activeViewSelectionColor =
			getComputedStyle(dummyDiv).getPropertyValue('background-color');
		this.activeView.setColor(this.activeViewSelectionColor);
		dummyDiv.remove();
	}

	public setActiveViewID(activeViewID: number) {
		if (this.activeViewID !== activeViewID) {
			this.activeViewID = activeViewID;
			this.activeView.clearTextSelection();
			// Remove the old active view's section before creating a new one.
			app.sectionContainer.removeSection(this.activeView.selectionSection.name);
			this.activeView = new DocumentViewBase(this.activeViewID);
			this.activeView.setColor(this.activeViewSelectionColor);
		}
	}

	private addSections() {
		this.mouseControl = new MouseControl(app.CSections.MouseControl.name);

		if (app.sectionContainer.doesSectionExist(this.mouseControl.name))
			app.sectionContainer.removeSection(this.mouseControl.name);

		app.sectionContainer.addSection(this.mouseControl);

		if (
			app.map._docLayer._docType === 'text' &&
			app.map.uiManager?.isRulerVisible()
		) {
			app.sectionContainer.addSection(new cool.RulerSpacerSection());
		}
	}

	public get fileSize(): cool.SimplePoint {
		return this._fileSize;
	}

	public set fileSize(value: cool.SimplePoint) {
		this._fileSize = value;
	}

	public removeView(viewID: number) {
		if (this.views.has(viewID)) {
			app.sectionContainer.removeSection(
				(this.views.get(viewID) as DocumentViewBase).selectionSection.name,
			);
			this.views.delete(viewID);
		}
	}

	// This shouldn't create views if not found. But it will happen when we use only this class for views.
	public getView(viewID: number): DocumentViewBase {
		if (this.views.has(viewID))
			return this.views.get(viewID) as DocumentViewBase;
		else {
			this.views.set(viewID, new DocumentViewBase(viewID));
			return this.views.get(viewID) as DocumentViewBase;
		}
	}

	public set activeModes(modes: number[]) {
		this._activeModes = modes.slice();
	}

	public get activeModes() {
		return this._activeModes.slice();
	}

	public isModeActive(mode: number): boolean {
		return this._activeModes.includes(mode);
	}
}
