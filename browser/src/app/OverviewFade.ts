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
 * OverviewFade - switches to the Overview slide with a fade when a zoom gesture asks
 * for a level outside the allowed zoom range.
 *
 * The zoom range is clamped to 20% - 400%. On a normal slide, a zoom-out gesture at
 * 20% fades the view out, switches to the Overview slide (part 0) and fades back in.
 */

class OverviewFade {
	// overview-fade gives the document container its opacity transition and
	// overview-fade-out is the faded-out state. Both are defined in cool.css.
	private static readonly fadeClass = 'overview-fade';
	private static readonly fadeOutClass = 'overview-fade-out';

	// Longer than the opacity transition, so it only fires when transitionend got lost.
	private static readonly transitionFallbackMs = 600;
	// Covers a part switch confirmation that never arrives.
	private static readonly setPartFallbackMs = 2000;

	private static inProgress = false;

	// Handles a request for a zoom level outside the allowed range. The request
	// switches slide instead of zooming when the view already sits at the limit
	// being pushed: zooming out on a normal slide goes to the Overview slide.
	// Returns true when the request switched slide, false when the caller should
	// clamp and zoom as usual.
	public static handleZoomBeyondLimit(requestedZoom: number): boolean {
		if (OverviewFade.inProgress) return false;

		const map = app.map;
		if (!map || !map._docLayer) return false;

		const docLayer = map._docLayer;
		if (docLayer._docType !== 'presentation') return false;
		if (!app.impress.hasOverviewPage || app.file.fileBasedView) return false;

		const currentZoom = map.getZoom();

		if (requestedZoom < map.getMinZoom()) {
			if (currentZoom !== map.getMinZoom() || docLayer._selectedPart === 0)
				return false;
			OverviewFade.fadeToPart(0);
			return true;
		}

		return false;
	}

	private static fadeToPart(part: number): void {
		const map = app.map;
		const container = document.getElementById('document-container');

		// Cypress runs with animations disabled, so switch immediately.
		if (!container || window.L.Browser.cypressTest) {
			map.setPart(part);
			return;
		}

		OverviewFade.inProgress = true;
		container.classList.add(OverviewFade.fadeClass);

		OverviewFade.runOpacityTransition(container, true, () => {
			map.setPart(part);

			// Fade back in when the server confirms the switch. The timeout covers
			// a confirmation that never arrives.
			let fadedIn = false;
			const fadeIn = () => {
				if (fadedIn) return;
				fadedIn = true;
				OverviewFade.runOpacityTransition(container, false, () => {
					container.classList.remove(OverviewFade.fadeClass);
					OverviewFade.inProgress = false;
				});
			};
			map.once('setpart', fadeIn);
			setTimeout(fadeIn, OverviewFade.setPartFallbackMs);
		});
	}

	// Fades the container out (toHidden true) or back in, and runs whenDone when the
	// opacity transition finishes. A timeout stands in for transitionend when the
	// event gets lost, for example because the container is not rendered.
	private static runOpacityTransition(
		container: HTMLElement,
		toHidden: boolean,
		whenDone: () => void,
	): void {
		let finished = false;
		const finish = () => {
			if (finished) return;
			finished = true;
			container.removeEventListener('transitionend', onTransitionEnd);
			whenDone();
		};
		const onTransitionEnd = (e: TransitionEvent) => {
			if (e.target === container && e.propertyName === 'opacity') finish();
		};
		container.addEventListener('transitionend', onTransitionEnd);
		setTimeout(finish, OverviewFade.transitionFallbackMs);
		container.classList.toggle(OverviewFade.fadeOutClass, toHidden);
	}
}
