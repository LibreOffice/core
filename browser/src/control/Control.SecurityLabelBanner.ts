// @ts-strict-ignore
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
 * SecurityLabelBanner - a full-width strip at the top of the document area
 * showing the document's security classification marking.
 *
 * It owns a single DOM element inserted into the body column directly above
 * #main-document-content (see css/cool.css), so the document area reflows below
 * it rather than being covered; when there is no label the banner is hidden and
 * reserves no space. Styling is themeable via CSS custom properties
 * (--security-banner-*, defaulting to the doc-type theme colors, as the compare
 * banner does). The marking comes from getCommandValues(".uno:SecurityLabel");
 * update() is re-callable so the banner tracks labels added/removed while editing.
 */

class SecurityLabelBanner {
	private banner: HTMLDivElement | null = null;

	private ensureBanner(): HTMLDivElement | null {
		if (this.banner) {
			return this.banner;
		}
		// Insert as a flex item in the body column, directly above the document
		// area (#main-document-content, flex: 1), so the document reflows below the
		// banner rather than being overlapped -- and the app can't override it the
		// way it manages #map's absolute geometry.
		const docContent = document.getElementById('main-document-content');
		if (!docContent || !docContent.parentNode) {
			return null;
		}
		this.banner = document.createElement('div');
		this.banner.id = 'security-label-banner';
		// Announce classification changes to assistive technology.
		this.banner.setAttribute('aria-live', 'polite');
		docContent.parentNode.insertBefore(this.banner, docContent);
		return this.banner;
	}

	// Show the marking, or hide the banner (reclaiming its space) when empty.
	// Showing/hiding it changes the height of #main-document-content, and so of
	// the #document-container inside it; the ResizeObserver on that container
	// (see DocEvents) fires app 'resize' and the active ViewLayout resizes the
	// canvas from there. Nothing to do here beyond the DOM change.
	public update(marking: string): void {
		const banner = this.ensureBanner();
		if (!banner) {
			return;
		}
		banner.textContent = marking || '';
		banner.style.display = marking ? '' : 'none';
	}
}
