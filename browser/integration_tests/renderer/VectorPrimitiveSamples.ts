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

/// The connection that the samples come over, opened on the first request. It loads a document
/// because the kit of a document answers the command.
let vectorPrimitiveSampleClient: Promise<any> | undefined;

/// The answer of the coolwsd at COOLWSD_URL for the primitive sample with the given name, such as
/// "backgroundColor". Its "primitives" hold the JSON that the engine writes for the sample.
async function vectorPrimitiveSample(name: string): Promise<any> {
	if (!vectorPrimitiveSampleClient) {
		// The bundle is in the renderer folder of the build, next to the compiled helpers.
		const { SocketClient } = require('../helper/SocketClient');
		const { BASE_URL, sampleUrl } = require('../helper/Environment');
		vectorPrimitiveSampleClient = SocketClient.connectAndLoad(
			BASE_URL,
			sampleUrl('impress.fodp'),
		);
	}
	const client = await vectorPrimitiveSampleClient;
	return client.primitiveSample(name);
}

after(async function () {
	if (vectorPrimitiveSampleClient) (await vectorPrimitiveSampleClient).close();
});
