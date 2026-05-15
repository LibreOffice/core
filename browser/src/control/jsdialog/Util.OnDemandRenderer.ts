// @ts-strict-ignore
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

/*
 * Util.OnDemandRenderer - helper for rendering entries on demand (when visible)
 */

declare var JSDialog: any;

function onDemandRenderer(
	builder: JSBuilder,
	controlId: string,
	controlType: string,
	entryId: number,
	placeholder: Element,
	parentContainer: Element,
	entryText: string | undefined,
) {
	const setupOnDemandRenderer = () => {
		// avoid races, might be already updated
		if (!parentContainer.contains(placeholder)) return;

		const cachedComboboxEntries = builder.rendersCache[controlId];
		let requestRender = true;

		if (cachedComboboxEntries && cachedComboboxEntries.images[entryId]) {
			const originalClass = placeholder.classList;
			window.L.DomUtil.remove(placeholder);
			placeholder = window.L.DomUtil.create('img', '', parentContainer);
			const placeholderImg = placeholder as HTMLImageElement;
			placeholderImg.src = cachedComboboxEntries.images[entryId];
			placeholderImg.alt = entryText;
			placeholderImg.title = entryText;
			originalClass.forEach((className: string) =>
				placeholderImg.classList.add(className),
			);
			requestRender = !cachedComboboxEntries.persistent;
		}

		if (requestRender) {
			// render on demand
			const pendingKey = controlId + ':' + entryId;
			var onIntersection = (entries: any) => {
				entries.forEach((entry: any) => {
					if (entry.isIntersecting) {
						// Already rendered for this (controlId,
						// entryId): rendersCache holds the image.
						// Skipping avoids the redundant round-trip
						// that core would dedupe and never reply to,
						// which would leak the counter.
						const cached = builder.rendersCache[controlId];
						if (cached && cached.images[entryId]) return;

						// Increment at most once per pair. On rebuild,
						// observers for the new placeholder may fire
						// while the original request is still in
						// flight - allow the retry send to go out
						// (core may have dropped the first request)
						// but do not double-count.
						if (!app.pendingOnDemandRenderRequests.has(pendingKey)) {
							app.pendingOnDemandRenderRequests.add(pendingKey);
							app.pendingOnDemandRenders++;
						}
						builder.callback(
							controlType,
							'render_entry',
							{ id: controlId },
							entryId +
								';' +
								Math.floor(100 * window.devicePixelRatio) +
								';' +
								Math.floor(100 * window.devicePixelRatio),
							builder,
						);
					}
				});
			};

			var observer = new IntersectionObserver(onIntersection, {
				root: null,
				threshold: 0.01, // percentage of visible area
			});

			observer.observe(placeholder);
		}
	};

	// If no first tile yet, delay sending the render request.
	TileManager.appendAfterFirstTileTask(setupOnDemandRenderer);
}

JSDialog.OnDemandRenderer = onDemandRenderer;
