/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/*
 * JSControl is a base class for implementing jsdialog based controls. Handles positioning.
 */

interface JSControlOptions {
	position: string;
	allowlist?: typeof MenuCommands.allowlist;
	mobileDenylist?: typeof MenuCommands.mobileDenylist;
	[name: string]: any;
}

class JSControl extends BaseClass {
	protected options: JSControlOptions = {
		position: 'topright',
	};

	protected _map: MapInterface | null = null;
	private _container: HTMLElement | null = null;
	protected onAdded?: (map: MapInterface) => void;
	protected onRemove?: (map: MapInterface) => void;

	protected mergeOptions(options: Record<string, any>) {
		for (const [name, value] of Object.entries(options)) {
			this.options[name] = value;
		}
	}

	constructor(options?: Record<string, any>) {
		super();
		if (options) {
			this.mergeOptions(options);
		}
	}

	public getPosition(): string {
		return this.options.position;
	}

	public setPosition(position: string): JSControl {
		const map = this._map;

		if (map) {
			map.removeControl(this);
		}

		this.options.position = position;

		if (map) {
			map.addControl(this);
		}

		return this;
	}

	public getContainer(): HTMLElement | null {
		return this._container;
	}

	public onAdd(map: MapInterface): HTMLElement | null {
		return null;
	}

	public addTo(map: MapInterface): JSControl {
		this.remove();
		this._map = map;

		const container = (this._container = this.onAdd(map));
		const pos = this.getPosition();
		const corner: Node = map._controlCorners[pos];

		window.L.DomUtil.addClass(container, 'leaflet-control');

		if (pos.indexOf('bottom') !== -1) {
			corner.insertBefore(container as Node, corner.firstChild);
		} else {
			corner.appendChild(container as Node);
		}

		if (this.onAdded) {
			this.onAdded(this._map);
		}

		return this;
	}

	public remove(): JSControl {
		if (!this._map) {
			return this;
		}

		window.L.DomUtil.remove(this._container);

		if (this.onRemove) {
			this.onRemove(this._map);
		}

		this._map = null;
		return this;
	}

	public isVisible(): boolean {
		if (!this._map) {
			return false;
		}

		const corner = this._map._controlCorners[this.options.position];
		return corner.hasChildNodes();
	}
}
