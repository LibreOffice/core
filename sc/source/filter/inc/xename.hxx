/*************************************************************************
 *
 *  $RCSfile: xename.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-14 12:09:52 $
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

#ifndef SC_XENAME_HXX
#define SC_XENAME_HXX

#ifndef SC_XLNAME_HXX
#include "xlname.hxx"
#endif
#ifndef SC_XEROOT_HXX
#include "xeroot.hxx"
#endif
#ifndef SC_XERECORD_HXX
#include "xerecord.hxx"
#endif

// ============================================================================

class XclExpNameManagerImpl;

/** Manager that stores all internal defined names (NAME records) of the document. */
class XclExpNameManager : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit            XclExpNameManager( const XclExpRoot& rRoot );
    virtual             ~XclExpNameManager();

    /** Creates NAME records for built-in and user defined names. */
    void                Initialize();

    /** Inserts the Calc name with the passed index and returns the Excel NAME index. */
    sal_uInt16          InsertName( USHORT nScNameIdx );
    /** Inserts the Calc database range with the passed index and returns the Excel NAME index. */
    sal_uInt16          InsertDBRange( USHORT nScDBRangeIdx );

    /** Inserts a new built-in defined name. */
    sal_uInt16          InsertBuiltInName( sal_Unicode cBuiltIn, XclExpTokenArrayRef xTokArr, SCTAB nScTab );
    /** Inserts a new built-in defined name, referring to the passed sheet range. */
    sal_uInt16          InsertBuiltInName( sal_Unicode cBuiltIn, const ScRange& rRange );
    /** Inserts a new built-in defined name, referring to the passed sheet range list. */
    sal_uInt16          InsertBuiltInName( sal_Unicode cBuiltIn, const ScRangeList& rRangeList );

    /** Inserts a new defined name. Sets another unused name, if rName already exists. */
    sal_uInt16          InsertUniqueName( const String& rName, XclExpTokenArrayRef xTokArr, SCTAB nScTab );
    /** Searches or inserts a defined name describing a macro name.
        @param bFunc  true = Macro function; false = Macro procedure. */
    sal_uInt16          InsertMacroCall( const String& rMacroName, bool bFunc );

    /** Returns the Calc sheet of a local defined name, or SCTAB_GLOBAL for global defined names. */
    const String&       GetOrigName( sal_uInt16 nNameIdx ) const;
    /** Returns the Calc sheet of a local defined name, or SCTAB_GLOBAL for global defined names. */
    SCTAB               GetScTab( sal_uInt16 nNameIdx ) const;
    /** Returns true, if the specified defined name is volatile. */
    bool                IsVolatile( sal_uInt16 nNameIdx ) const;

    /** Writes the entire list of NAME records. */
    virtual void        Save( XclExpStream& rStrm );

private:
    typedef ScfRef< XclExpNameManagerImpl > XclExpNameMgrImplRef;
    XclExpNameMgrImplRef mxImpl;
};

// ============================================================================

#endif

