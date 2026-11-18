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

/// The part of the ws API that the tests call. ws ships without types, and @types/ws is not in
/// node_shrinkpack.
declare module 'ws' {
	import { EventEmitter } from 'events';

	class WebSocket extends EventEmitter {
		constructor(address: string, options?: { origin?: string });
		send(data: string): void;
		close(): void;
	}

	export = WebSocket;
}
