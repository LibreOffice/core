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

#include "xiformula.hxx"
#include "rangelst.hxx"
#include "xistream.hxx"

#include "excform.hxx"

// Formula compiler ===========================================================

/** Implementation class of the export formula compiler. */
class XclImpFmlaCompImpl : protected XclImpRoot, protected XclTokenArrayHelper
{
public:
    explicit            XclImpFmlaCompImpl( const XclImpRoot& rRoot );

    /** Creates a range list from the passed Excel token array. */
    void                CreateRangeList(
                            ScRangeList& rScRanges, XclFormulaType eType,
                            const XclTokenArray& rXclTokArr, XclImpStream& rStrm );

    const ScTokenArray* CreateFormula( XclFormulaType eType, const XclTokenArray& rXclTokArr );

private:
    XclFunctionProvider maFuncProv;     /// Excel function data provider.
};

XclImpFmlaCompImpl::XclImpFmlaCompImpl( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    maFuncProv( rRoot )
{
}

void XclImpFmlaCompImpl::CreateRangeList(
        ScRangeList& rScRanges, XclFormulaType /*eType*/,
        const XclTokenArray& rXclTokArr, XclImpStream& /*rStrm*/ )
{
    rScRanges.RemoveAll();

    //FIXME: evil hack, using old formula import :-)
    if( !rXclTokArr.Empty() )
    {
        SvMemoryStream aMemStrm;
        aMemStrm.WriteUInt16( EXC_ID_EOF ).WriteUInt16( rXclTokArr.GetSize() );
        aMemStrm.Write( rXclTokArr.GetData(), rXclTokArr.GetSize() );
        XclImpStream aFmlaStrm( aMemStrm, GetRoot() );
        aFmlaStrm.StartNextRecord();
        GetOldFmlaConverter().GetAbsRefs( rScRanges, aFmlaStrm, aFmlaStrm.GetRecSize() );
    }
}

const ScTokenArray* XclImpFmlaCompImpl::CreateFormula(
        XclFormulaType /*eType*/, const XclTokenArray& rXclTokArr )
{
    if (rXclTokArr.Empty())
        return nullptr;

    // evil hack!  are we trying to phase out the old style formula converter ?
    SvMemoryStream aMemStrm;
    aMemStrm.WriteUInt16( EXC_ID_EOF ).WriteUInt16( rXclTokArr.GetSize() );
    aMemStrm.Write( rXclTokArr.GetData(), rXclTokArr.GetSize() );
    XclImpStream aFmlaStrm( aMemStrm, GetRoot() );
    aFmlaStrm.StartNextRecord();
    const ScTokenArray* pArray = nullptr;
    GetOldFmlaConverter().Reset();
    GetOldFmlaConverter().Convert(pArray, aFmlaStrm, aFmlaStrm.GetRecSize(), true);
    return pArray;
}

XclImpFormulaCompiler::XclImpFormulaCompiler( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    mxImpl( new XclImpFmlaCompImpl( rRoot ) )
{
}

XclImpFormulaCompiler::~XclImpFormulaCompiler()
{
}

void XclImpFormulaCompiler::CreateRangeList(
        ScRangeList& rScRanges, XclFormulaType eType,
        const XclTokenArray& rXclTokArr, XclImpStream& rStrm )
{
    mxImpl->CreateRangeList( rScRanges, eType, rXclTokArr, rStrm );
}

const ScTokenArray* XclImpFormulaCompiler::CreateFormula(
        XclFormulaType eType, const XclTokenArray& rXclTokArr )
{
    return mxImpl->CreateFormula(eType, rXclTokArr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
