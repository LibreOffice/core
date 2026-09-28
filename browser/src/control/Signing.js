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
 * Document Signing
 */

/* global _ app */

window.L.Map.include({
	onChangeSignStatus: function(signstatus) {
		// Have a non-empty status text by default, so a signatures -> no signatures
		// transition updates the status bar.
		var statusText = _('The document is not signed.');
		// Most documents don't have signatures, so have no disturbing icon for that case,
		// also don't have an empty icon, which would get the icon for the matching UNO
		// command.
		var statusIcon = ' ';
		// This is meant to be in sync with core.git
		// include/sfx2/signaturestate.hxx, SignatureState.
		switch (signstatus) {
		case '0': // SignatureState::NOSIGNATURES
			break;
		case '1': // SignatureState::OK
			statusText = _('This document is digitally signed and the signature is valid.');
			statusIcon = 'sign_ok';
			break;
		case '2': // SignatureState::BROKEN
			statusText = _('This document has an invalid signature.');
			statusIcon = 'sign_not_ok';
			break;
		case '3': // SignatureState::INVALID
			statusText = _('The signature was valid, but the document has been modified.');
			statusIcon = 'sign_not_ok';
			break;
		case '4': // SignatureState::NOTVALIDATED
			statusText = _('The signature is OK, but the certificate could not be validated.');
			statusIcon = 'sign_not_ok';
			break;
		case '5': // SignatureState::PARTIAL_OK
			statusText = _('The signature is OK, but the document is only partially signed.');
			statusIcon = 'sign_not_ok';
			break;
		case '6': // SignatureState::NOTVALIDATED_PARTIAL_OK
			statusText = _('The signature is OK, but the certificate could not be validated and the document is only partially signed.');
			statusIcon = 'sign_not_ok';
			break;
		}

		if (signstatus === '0') {
			let signstatusElement = document.querySelector('#signstatus');
			if (signstatusElement && signstatusElement.classList.contains('hidden')) {
				// Had no signatures and have no signatures: can skip the update.
				return;
			}
		}

		if (statusText) {
			if (window.mode.isInteractivePreview()) {
				return;
			}
			if (!window.mode.isSmallScreenDevice()) {
				app.map.statusBar.showSigningItem(statusIcon, statusText);

				// If requested, show the signatures, now that the sign status
				// changed.
				const eSignature = app.map.eSignature;
				if (eSignature && eSignature.showSignaturesOnNextUpdate) {
					eSignature.showSignaturesOnNextUpdate = false;
					app.map.sendUnoCommand('.uno:Signature');
				}
			} else if (app.map.mobileTopBar.showSigningItem)
				app.map.mobileTopBar.showSigningItem(statusIcon, statusText);
		}
	}
});
