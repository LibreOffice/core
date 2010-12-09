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

#include "precompiled_sc.hxx"
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

    // ------------------------------------------------------------------------
private:
    XclFunctionProvider maFuncProv;     /// Excel function data provider.
    const XclBiff       meBiff;         /// Cached BIFF version to save GetBiff() calls.
};

// ----------------------------------------------------------------------------

XclImpFmlaCompImpl::XclImpFmlaCompImpl( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    maFuncProv( rRoot ),
    meBiff( rRoot.GetBiff() )
{
}

void XclImpFmlaCompImpl::CreateRangeList(
        ScRangeList& rScRanges, XclFormulaType /*eType*/,
        const XclTokenArray& rXclTokArr, XclImpStream& /*rStrm*/ )
{
    rScRanges.clear();

    //! evil hack, using old formula import :-)
    if( !rXclTokArr.Empty() )
    {
        SvMemoryStream aMemStrm;
        aMemStrm << EXC_ID_EOF << rXclTokArr.GetSize();
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
        return NULL;

    // evil hack!  are we trying to phase out the old style formula converter ?
    SvMemoryStream aMemStrm;
    aMemStrm << EXC_ID_EOF << rXclTokArr.GetSize();
    aMemStrm.Write( rXclTokArr.GetData(), rXclTokArr.GetSize() );
    XclImpStream aFmlaStrm( aMemStrm, GetRoot() );
    aFmlaStrm.StartNextRecord();
    const ScTokenArray* pArray = NULL;
    GetOldFmlaConverter().Reset();
    GetOldFmlaConverter().Convert(pArray, aFmlaStrm, aFmlaStrm.GetRecSize(), true);
    return pArray;
}

// ----------------------------------------------------------------------------

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

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
