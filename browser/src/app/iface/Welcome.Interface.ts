/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

interface HandlerInterface {
	enable(): void;
	disable(): void;
	enabled(): boolean;
}

interface WelcomeInterface extends HandlerInterface {
	addHooks(): void;
	isGuest(): boolean;
	onUpdateList(): void;
	shouldWelcome(): boolean;
	showWelcomeDialog(): void;
	// Whether a welcome dialog is open: loading, shown, or waiting to be loaded again after a
	// failed load.
	isOpen(): boolean;
	// Removes the welcome dialog for good and fires 'welcomeclosed' when one was open.
	close(): void;
	removeHooks(): void;
	remove(): void;
	onMessage(e: { data: any }): void;
}
