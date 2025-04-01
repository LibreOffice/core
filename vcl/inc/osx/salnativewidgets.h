/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

 20             // height of tab header in pixels
 12        // left/right margin of text within tab headers

 4  // width of focus ring in pixels

 10  // height of medium progress indicator in pixels
 16   // height of large progress indicator in pixels

 21  // height of normal push button without focus ring in pixels
 15   // height of small push button without focus ring in pixels

 14     // width/height of small radio button without focus ring in pixels
 3  // space between radio button and following text in pixels

 14     // width/height of checkbox without focus ring in pixels
 3  // space between checkbox and following text in pixels

 19   // width of slider in pixels
 18  // height of slider in pixels

 21       // height of editbox without focus ring in pixels
 1  // width of editbox border in pixels
 1  // width of left/right as well as top/bottom editbox margin in pixels

 20        // height of combobox without focus ring in pixels
 18  // width of combobox button without focus ring in pixels
 1   // width of combobox border in pixels
 1    // left/right margin of text in pixels

 20        // height of listbox without focus ring in pixels
 18  // width of listbox button without focus ring in pixels
 1   // width of listbox border in pixels
 1    // left/right margin of text in pixels

 13         // width of spin button without focus ring in pixels
 11  // height of upper spin button without focus ring in pixels
 11  // height of lower spin button without focus ring in pixels

// FIXME: spinboxes are positioned one pixel shifted to the right by VCL. As positioning as well as size should be equal to
// corresponding editboxes, comboboxes or listboxes, positioning of spinboxes should by equal too. Issue cannot be fixed within
// native widget drawing code. As a workaround, an offset is considered for spinboxes to align spinboxes correctly.

 1  // left offset for alignment with editboxes, comboboxes, and listboxes

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
