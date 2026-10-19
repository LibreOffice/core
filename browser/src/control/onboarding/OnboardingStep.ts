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
 * One stop of the onboarding tour: the words it shows, the element it points
 * at, and the documents and user interface modes it belongs to.
 */

type OnboardingDocType = 'text' | 'spreadsheet' | 'presentation' | 'drawing';

type OnboardingUiMode = 'classic' | 'notebookbar';

interface OnboardingStep {
	// A short stable name for the step, made of lower case letters and hyphens.
	// No two steps in the list share one.
	id: string;

	// The heading of the card, already translated.
	title: string;

	// The body of the card, already translated.
	text: string;

	// The element the step points at, either as a CSS selector or as a function
	// that returns the element. A step without a target has no element of its
	// own and speaks about the document as a whole. The element must be able to
	// carry an ::after pseudo-element, so not an input, and must be positioned,
	// or get a position: relative rule in onboarding.css.
	target?: string | (() => HTMLElement | null);

	// Where the card sits next to the target, as a JSDialog popup anchor:
	// - '' or absent: below the target;
	// - 'above': over the target;
	// - 'top end': beside the target on its end side, level with its top;
	// - 'top start': beside the target on its start side, level with its top.
	anchor?: string;

	// The document types this step belongs to. Absent means every document type.
	docTypes?: OnboardingDocType[];

	// Whether this step applies in the current environment. Absent means it
	// does.
	isAvailable?: () => boolean;

	// The user interface modes this step belongs to. Absent means both modes.
	uiModes?: OnboardingUiMode[];

	// Work to do just before the step appears, such as opening the tab that
	// holds the element.
	beforeShow?: () => void;
}
