/*************************************************************************
 *
 *  $RCSfile: xiview.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2005-02-21 13:46:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

