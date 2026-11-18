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

// Opaque type stubs for UI classes that appear only in type signatures
// (e.g. MapInterface members). Keeping them as `any` avoids pulling the
// entire UI tree into the mocha_tests compile. When a test needs to
// actually exercise one of these classes, replace the stub with a real
// reference to its source file.

type UIManager = any;
type AddressInputField = any;
type Menubar = any;
type UserList = any;
type Sidebar = any;
type ContextToolbar = any;
type NavigatorPanel = any;
type MobileSearchBar = any;
type ServerAuditDialog = any;
type SlideShowPresenter = any;
type SidebarFromNotebookbarPanel = any;
type MobileTopBar = any;
type ContextMenuControl = any;
type CSelections = any;
type Cursor = any;
type Bounds = cool.Bounds;

declare namespace cool {
	type SplitPanesContext = any;
	type ESignature = any;
}

declare const brandProductName: string | undefined;
