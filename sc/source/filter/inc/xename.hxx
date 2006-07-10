/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xename.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 13:55:20 $
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

class ScRangeList;
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
    sal_uInt16          InsertBuiltInName( sal_Unicode cBuiltIn, XclTokenArrayRef xTokArr, SCTAB nScTab );
    /** Inserts a new built-in defined name, referring to the passed sheet range. */
    sal_uInt16          InsertBuiltInName( sal_Unicode cBuiltIn, const ScRange& rRange );
    /** Inserts a new built-in defined name, referring to the passed sheet range list. */
    sal_uInt16          InsertBuiltInName( sal_Unicode cBuiltIn, const ScRangeList& rRangeList );

    /** Inserts a new defined name. Sets another unused name, if rName already exists. */
    sal_uInt16          InsertUniqueName( const String& rName, XclTokenArrayRef xTokArr, SCTAB nScTab );
    /** Returns index of an existing name, or creates a name without definition. */
    sal_uInt16          InsertRawName( const String& rName );
    /** Searches or inserts a defined name describing a macro name.
        @param bVBasic  true = Visual Basic macro, false = Sheet macro.
        @param bFunc  true = Macro function; false = Macro procedure. */
    sal_uInt16          InsertMacroCall( const String& rMacroName, bool bVBasic, bool bFunc, bool bHidden = false );

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

