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
	removeHooks(): void;
	remove(): void;
	onMessage(e: { data: any }): void;
}
