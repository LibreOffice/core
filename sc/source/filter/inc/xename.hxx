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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XENAME_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XENAME_HXX

#include "xerecord.hxx"
#include "xlname.hxx"
#include "xlformula.hxx"
#include "xeroot.hxx"
#include <memory>

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
    sal_uInt16          InsertName( SCTAB nTab, sal_uInt16 nScNameIdx );

    /** Inserts a new built-in defined name, referring to the passed sheet range. */
    sal_uInt16          InsertBuiltInName( sal_Unicode cBuiltIn, const ScRange& rRange );
    /** Inserts a new built-in defined name, referring to the passed sheet range list. */
    sal_uInt16          InsertBuiltInName( sal_Unicode cBuiltIn, const ScRangeList& rRangeList );

    /** Inserts a new defined name. Sets another unused name, if rName already exists. */
    sal_uInt16          InsertUniqueName( const OUString& rName, XclTokenArrayRef xTokArr, SCTAB nScTab );
    /** Returns index of an existing name, or creates a name without definition. */
    sal_uInt16          InsertRawName( const OUString& rName );
    /** Searches or inserts a defined name describing a macro name.
        @param bVBasic  true = Visual Basic macro, false = Sheet macro.
        @param bFunc  true = Macro function; false = Macro procedure. */
    sal_uInt16          InsertMacroCall( const OUString& rMacroName, bool bVBasic, bool bFunc, bool bHidden = false );

    /** Returns the Calc sheet of a local defined name, or SCTAB_GLOBAL for global defined names. */
    const OUString&     GetOrigName( sal_uInt16 nNameIdx ) const;
    /** Returns the Calc sheet of a local defined name, or SCTAB_GLOBAL for global defined names. */
    SCTAB               GetScTab( sal_uInt16 nNameIdx ) const;
    /** Returns true, if the specified defined name is volatile. */
    bool                IsVolatile( sal_uInt16 nNameIdx ) const;

    /** Writes the entire list of NAME records. */
    virtual void        Save( XclExpStream& rStrm ) SAL_OVERRIDE;
    virtual void        SaveXml( XclExpXmlStream& rStrm ) SAL_OVERRIDE;

private:
    typedef std::shared_ptr< XclExpNameManagerImpl > XclExpNameMgrImplRef;
    XclExpNameMgrImplRef mxImpl;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
