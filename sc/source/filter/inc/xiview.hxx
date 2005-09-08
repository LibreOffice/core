/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xiview.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:34:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef SC_XIVIEW_HXX
#define SC_XIVIEW_HXX

#ifndef SC_XLVIEW_HXX
#include "xlview.hxx"
#endif
#ifndef SC_XIROOT_HXX
#include "xiroot.hxx"
#endif

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
    void                ReadWindow2( XclImpStream& rStrm );
    /** Reads an SCL record. */
    void                ReadScl( XclImpStream& rStrm );
    /** Reads a PANE record. */
    void                ReadPane( XclImpStream& rStrm );
    /** Reads a SELECTION record. */
    void                ReadSelection( XclImpStream& rStrm );

    /** Sets the view settings at the current sheet or the extended sheet options object. */
    void                Finalize();

private:
    XclTabViewData      maData;         /// Sheet view settings data.
};

// ============================================================================

#endif

