/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_XENAME_HXX
#define SC_XENAME_HXX

#include "xerecord.hxx"
#include "xlname.hxx"
#include "xlformula.hxx"
#include "xeroot.hxx"
#include <boost/shared_ptr.hpp>

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
    sal_uInt16          InsertName( SCTAB nTab, USHORT nScNameIdx );
    /** Inserts the Calc database range with the passed index and returns the Excel NAME index. */
    sal_uInt16          InsertDBRange( USHORT nScDBRangeIdx );

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
    virtual void        SaveXml( XclExpXmlStream& rStrm );

private:
    typedef boost::shared_ptr< XclExpNameManagerImpl > XclExpNameMgrImplRef;
    XclExpNameMgrImplRef mxImpl;
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
