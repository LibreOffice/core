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
    rScRanges.RemoveAll();

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

