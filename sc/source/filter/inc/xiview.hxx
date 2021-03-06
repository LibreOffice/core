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

#include "xlview.hxx"
#include "xiroot.hxx"

// Document view settings =====================================================

/** Contains document view settings (WINDOW1 record). */
class XclImpDocViewSettings : protected XclImpRoot
{
public:
    explicit            XclImpDocViewSettings( const XclImpRoot& rRoot );

    /** Reads a WINDOW1 record. */
    void                ReadWindow1( XclImpStream& rStrm );

    /** Returns the Calc index of the displayed sheet. */
    SCTAB               GetDisplScTab() const;

    /** Sets the view settings at the document. */
    void                Finalize();

private:
    XclDocViewData      maData;         /// Document view settings data.
};

// Sheet view settings ========================================================

/** Contains all view settings for a single sheet.

    Usage:
    1)  When import filter starts reading a worksheet substream, initialize an
        instance of this class with the Initialize() function. This will set
        all view options to Excel default values.
    2)  Read all view related records using the Read*() functions.
    3)  When import filter ends reading a worksheet substream, call Finalize()
        to set all view settings to the current sheet of the Calc document.
 */
class XclImpTabViewSettings : protected XclImpRoot
{
public:
    explicit            XclImpTabViewSettings( const XclImpRoot& rRoot );

    /** Initializes the object to be used for a new sheet. */
    void                Initialize();

    /** Reads a WINDOW2 record. */
    void                ReadWindow2( XclImpStream& rStrm, bool bChart );
    /** Reads an SCL record. */
    void                ReadScl( XclImpStream& rStrm );
    /** Reads a PANE record. */
    void                ReadPane( XclImpStream& rStrm );
    /** Reads a SELECTION record. */
    void                ReadSelection( XclImpStream& rStrm );
    /** Reads a SHEETEXT record (Tab Color). */
    void                ReadTabBgColor( XclImpStream& rStrm, const XclImpPalette& rPal );
    /** Sets the view settings at the current sheet or the extended sheet options object. */
    void                Finalize();

private:
    XclTabViewData      maData;         /// Sheet view settings data.
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
