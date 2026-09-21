/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
interface ControlInterface {
	addTo(map: MapInterface): ControlInterface;
	remove(): void;
}

interface ControlLayerInterface extends ControlInterface {
	_container: HTMLDivElement;
	_addLayer(layer: BaseClass, name: string, overlay: boolean): void;
	_update(): void;
}

interface ControlsInterface {
	[name: string]: ControlLayerInterface;
}

interface OverlaysInterface {
	[name: string]: string;
}

type UnoCommand = string | { textCommand: string; objectCommand: string };

// deduced from _extractCommand()
type ControlCommand =
	| string
	| {
			uno?: UnoCommand;
			lockUno?: UnoCommand;
			command?: string;
			id?: string;
	  };
