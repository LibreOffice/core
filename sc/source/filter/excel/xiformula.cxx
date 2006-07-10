/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xiformula.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 13:41:02 $
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

#ifndef SC_XIFORMULA_HXX
#include "xiformula.hxx"
#endif

#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"
#endif

#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif

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
        ScRangeList& rScRanges, XclFormulaType eType,
        const XclTokenArray& rXclTokArr, XclImpStream& rStrm )
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

// ============================================================================

