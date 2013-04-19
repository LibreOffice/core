/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SC_DRAGDATA_HXX__
#define __SC_DRAGDATA_HXX__

#include "rtl/ustring.hxx"

class ScTransferObj;
class ScDrawTransferObj;
class ScDocument;

struct ScDragData
{
    ScTransferObj*      pCellTransfer;
    ScDrawTransferObj*  pDrawTransfer;
    ScDocument*         pJumpLocalDoc;

    OUString       aLinkDoc;
    OUString       aLinkTable;
    OUString       aLinkArea;
    OUString       aJumpTarget;
    OUString       aJumpText;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
