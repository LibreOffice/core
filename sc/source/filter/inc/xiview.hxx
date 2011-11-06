/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SC_XIVIEW_HXX
#define SC_XIVIEW_HXX

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
    1)  When import filter starts reading a worksheet substream, inizialize an
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
    void                ReadTabBgColor( XclImpStream& rStrm, XclImpPalette& rPal );
    /** Sets the view settings at the current sheet or the extended sheet options object. */
    void                Finalize();

private:
    XclTabViewData      maData;         /// Sheet view settings data.
};

// ============================================================================

#endif

