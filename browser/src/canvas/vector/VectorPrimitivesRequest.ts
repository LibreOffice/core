/* -*- js-indent-level: 8; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

namespace cool {
	/// A request for the vector primitives of a part.
	export interface VectorPrimitivesRequest {
		/// The part asked for: the GUID of a page, or the index of a sheet.
		partId: string;
		/// The mode the part is asked in.
		mode: number;
		/// The number the answer names back, or undefined when the answer names none.
		requestId?: number;
		/// The blocks of a sheet asked for. Empty or undefined asks for no block.
		blocks?: string[];
		/// The epoch the versions of the client count in, or undefined before the client
		/// knows one.
		epoch?: number;
		/// The version the client holds. The answer is then the change since that version.
		/// Undefined asks for the whole part. It counts only with an epoch.
		since?: number;
	}

	/// The message that asks for the vector primitives of a part.
	export function vectorPrimitivesRequest(
		request: VectorPrimitivesRequest,
	): string {
		let message =
			'commandvalues command=.uno:VectorPrimitives?partid=' +
			request.partId +
			'&mode=' +
			String(request.mode);
		if (request.requestId !== undefined)
			message += '&requestid=' + String(request.requestId);
		if (request.blocks && request.blocks.length)
			message += '&blocks=' + request.blocks.join(',');
		if (request.epoch !== undefined) {
			if (request.since !== undefined)
				message += '&since=' + String(request.since);
			message += '&epoch=' + String(request.epoch);
		}
		return message;
	}
}
