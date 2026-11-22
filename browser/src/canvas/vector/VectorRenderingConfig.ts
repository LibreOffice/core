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

namespace cool {
	/// Return true if the vector rendering should be enabled
	export namespace VectorRenderingConfig {
		/// Return true if a document of the type starts out drawn from vector primitives.
		export function startsOn(docType?: string): boolean {
			// A page without document parameters, such as a unit test, uses tiles.
			if (!window.coolParams || !window.startsDrawnFromVectors) return false;
			return window.startsDrawnFromVectors(docType);
		}

		/// Return true if the client draws the slides of a presentation or a drawing from vector
		/// primitives. Before the document has a layer, its type is not known yet, and the answer
		/// is the one for a presentation. Once it has one, the render manager of its type answers.
		export function drawsSlidesFromVectors(): boolean {
			if (app.map && app.map._docLayer)
				return RenderManager.drawsSlidesFromVectors();
			return startsOn('presentation');
		}

		export function isEnabled(): boolean {
			// The debug panel's switch decides once there is a map to hold it. It starts out
			// at the default for the type of the document, so the default applies only before
			// the map exists.
			if (app.map && app.map._debug) return app.map._debug.vectorOn;
			return startsOn(app.map?._docLayer?._docType);
		}
	}
}
