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
 * Definitions.WindowId - Predefined special WindowId list used by JSDialog to
 *                        communicate with correct component on the core side.
 *                        Regular dialogs get automatically incremented positive,
 *                        non-zero values form the core. For special cases we use
 *                        0 - "main window"
 *                        negative numbers - converted by core into per-user ids
 */

// see core desktop/source/lib/init.cxx
enum WindowId {
	Sidebar = -1,
	Notebookbar = -2,
	Formulabar = -3,
	AddressInput = -4,
	QuickFind = -5,
	NotesPanel = -6,
}

enum AutoCompleteDialogId {
	Mention = 'mentionPopup',
	FormulaUsagePopup = 'formulausagePopup',
}
