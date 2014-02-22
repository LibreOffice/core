/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "xetable.hxx"

#include <map>
#include <com/sun/star/i18n/ScriptType.hpp>
#include "scitems.hxx"
#include <svl/intitem.hxx>
#include "document.hxx"
#include "dociter.hxx"
#include "olinetab.hxx"
#include "formulacell.hxx"
#include "patattr.hxx"
#include "attrib.hxx"
#include "xehelper.hxx"
#include "xecontent.hxx"
#include "xeescher.hxx"
#include "xeextlst.hxx"
#include "tokenarray.hxx"

using namespace ::oox;


namespace ApiScriptType = ::com::sun::star::i18n::ScriptType;





XclExpStringRec::XclExpStringRec( const XclExpRoot& rRoot, const OUString& rResult ) :
    XclExpRecord( EXC_ID3_STRING ),
    mxResult( XclExpStringHelper::CreateString( rRoot, rResult ) )
{
    OSL_ENSURE( (rRoot.GetBiff() <= EXC_BIFF5) || (mxResult->Len() > 0),
        "XclExpStringRec::XclExpStringRec - empty result not allowed in BIFF8+" );
    SetRecSize( mxResult->GetSize() );
}

void XclExpStringRec::WriteBody( XclExpStream& rStrm )
{
    rStrm << *mxResult;
}



XclExpRangeFmlaBase::XclExpRangeFmlaBase(
        sal_uInt16 nRecId, sal_uInt32 nRecSize, const ScAddress& rScPos ) :
    XclExpRecord( nRecId, nRecSize ),
    maXclRange( ScAddress::UNINITIALIZED ),
    maBaseXclPos( ScAddress::UNINITIALIZED )
{
    maBaseXclPos.Set( static_cast< sal_uInt16 >( rScPos.Col() ), static_cast< sal_uInt16 >( rScPos.Row() ) );
    maXclRange.maFirst = maXclRange.maLast = maBaseXclPos;
}

XclExpRangeFmlaBase::XclExpRangeFmlaBase(
        sal_uInt16 nRecId, sal_uInt32 nRecSize, const ScRange& rScRange ) :
    XclExpRecord( nRecId, nRecSize ),
    maXclRange( ScAddress::UNINITIALIZED ),
    maBaseXclPos( ScAddress::UNINITIALIZED )
{
    maXclRange.Set(
        static_cast< sal_uInt16 >( rScRange.aStart.Col() ),
        static_cast< sal_uInt16 >( rScRange.aStart.Row() ),
        static_cast< sal_uInt16 >( rScRange.aEnd.Col() ),
        static_cast< sal_uInt16 >( rScRange.aEnd.Row() ) );
    maBaseXclPos = maXclRange.maFirst;
}

bool XclExpRangeFmlaBase::IsBasePos( sal_uInt16 nXclCol, sal_uInt32 nXclRow ) const
{
    return (maBaseXclPos.mnCol == nXclCol) && (maBaseXclPos.mnRow == nXclRow);
}

void XclExpRangeFmlaBase::Extend( const ScAddress& rScPos )
{
    sal_uInt16 nXclCol = static_cast< sal_uInt16 >( rScPos.Col() );
    sal_uInt32 nXclRow = static_cast< sal_uInt32 >( rScPos.Row() );
    maXclRange.maFirst.mnCol = ::std::min( maXclRange.maFirst.mnCol, nXclCol );
    maXclRange.maFirst.mnRow = ::std::min( maXclRange.maFirst.mnRow, nXclRow );
    maXclRange.maLast.mnCol  = ::std::max( maXclRange.maLast.mnCol,  nXclCol );
    maXclRange.maLast.mnRow  = ::std::max( maXclRange.maLast.mnRow,  nXclRow );
}

void XclExpRangeFmlaBase::WriteRangeAddress( XclExpStream& rStrm ) const
{
    maXclRange.Write( rStrm, false );
}



XclExpArray::XclExpArray( XclTokenArrayRef xTokArr, const ScRange& rScRange ) :
    XclExpRangeFmlaBase( EXC_ID3_ARRAY, 14 + xTokArr->GetSize(), rScRange ),
    mxTokArr( xTokArr )
{
}

XclTokenArrayRef XclExpArray::CreateCellTokenArray( const XclExpRoot& rRoot ) const
{
    return rRoot.GetFormulaCompiler().CreateSpecialRefFormula( EXC_TOKID_EXP, maBaseXclPos );
}

bool XclExpArray::IsVolatile() const
{
    return mxTokArr->IsVolatile();
}

void XclExpArray::WriteBody( XclExpStream& rStrm )
{
    WriteRangeAddress( rStrm );
    sal_uInt16 nFlags = EXC_ARRAY_DEFAULTFLAGS;
    ::set_flag( nFlags, EXC_ARRAY_RECALC_ALWAYS, IsVolatile() );
    rStrm << nFlags << sal_uInt32( 0 ) << *mxTokArr;
}



XclExpArrayBuffer::XclExpArrayBuffer( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot )
{
}

XclExpArrayRef XclExpArrayBuffer::CreateArray( const ScTokenArray& rScTokArr, const ScRange& rScRange )
{
    const ScAddress& rScPos = rScRange.aStart;
    XclTokenArrayRef xTokArr = GetFormulaCompiler().CreateFormula( EXC_FMLATYPE_MATRIX, rScTokArr, &rScPos );

    OSL_ENSURE( maRecMap.find( rScPos ) == maRecMap.end(), "XclExpArrayBuffer::CreateArray - array exists already" );
    XclExpArrayRef& rxRec = maRecMap[ rScPos ];
    rxRec.reset( new XclExpArray( xTokArr, rScRange ) );
    return rxRec;
}

XclExpArrayRef XclExpArrayBuffer::FindArray( const ScTokenArray& rScTokArr, const ScAddress& rBasePos ) const
{
    XclExpArrayRef xRec;
    
    if (rScTokArr.GetLen() != 1)
        
        return xRec;

    const formula::FormulaToken* pToken = rScTokArr.GetArray()[0];
    if (!pToken || pToken->GetOpCode() != ocMatRef)
        
        return xRec;

    const ScSingleRefData& rRef = static_cast<const ScToken*>(pToken)->GetSingleRef();
    ScAddress aAbsPos = rRef.toAbs(rBasePos);
    XclExpArrayMap::const_iterator it = maRecMap.find(aAbsPos);

    return (it == maRecMap.end()) ? xRec : xRec = it->second;
}



XclExpShrfmla::XclExpShrfmla( XclTokenArrayRef xTokArr, const ScAddress& rScPos ) :
    XclExpRangeFmlaBase( EXC_ID_SHRFMLA, 10 + xTokArr->GetSize(), rScPos ),
    mxTokArr( xTokArr ),
    mnUsedCount( 1 )
{
}

void XclExpShrfmla::ExtendRange( const ScAddress& rScPos )
{
    Extend( rScPos );
    ++mnUsedCount;
}

XclTokenArrayRef XclExpShrfmla::CreateCellTokenArray( const XclExpRoot& rRoot ) const
{
    return rRoot.GetFormulaCompiler().CreateSpecialRefFormula( EXC_TOKID_EXP, maBaseXclPos );
}

bool XclExpShrfmla::IsVolatile() const
{
    return mxTokArr->IsVolatile();
}

void XclExpShrfmla::WriteBody( XclExpStream& rStrm )
{
    WriteRangeAddress( rStrm );
    rStrm << sal_uInt8( 0 ) << mnUsedCount << *mxTokArr;
}



XclExpShrfmlaBuffer::XclExpShrfmlaBuffer( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot )
{
}

bool XclExpShrfmlaBuffer::IsValidTokenArray( const ScTokenArray& rArray ) const
{
    using namespace formula;

    FormulaToken** pTokens = rArray.GetArray();
    sal_uInt16 nLen = rArray.GetLen();
    for (sal_uInt16 i = 0; i < nLen; ++i)
    {
        const FormulaToken* p = pTokens[i];
        switch (p->GetType())
        {
            case svSingleRef:
            {
                const ScSingleRefData& rRefData = static_cast<const ScToken*>(p)->GetSingleRef();
                if (!GetFormulaCompiler().IsRef2D(rRefData))
                    
                    return false;
            }
            break;
            case svDoubleRef:
            {
                const ScComplexRefData& rRefData = static_cast<const ScToken*>(p)->GetDoubleRef();
                if (!GetFormulaCompiler().IsRef2D(rRefData))
                    
                    return false;
            }
            break;
            case svExternalSingleRef:
            case svExternalDoubleRef:
            case svExternalName:
                
                return false;
            default:
                ;
        }
    }
    return true;
}

XclExpShrfmlaRef XclExpShrfmlaBuffer::CreateOrExtendShrfmla(
    const ScFormulaCell& rScCell, const ScAddress& rScPos )
{
    XclExpShrfmlaRef xRec;
    const ScTokenArray* pShrdScTokArr = rScCell.GetSharedCode();
    if (!pShrdScTokArr)
        
        return xRec;

    
    if (maBadTokens.count(pShrdScTokArr) > 0)
        
        return xRec;

    if (!IsValidTokenArray(*pShrdScTokArr))
    {
        
        maBadTokens.insert(pShrdScTokArr);
        return xRec;
    }

    TokensType::iterator aIt = maRecMap.find(pShrdScTokArr);
    if( aIt == maRecMap.end() )
    {
        
        XclTokenArrayRef xTokArr = GetFormulaCompiler().CreateFormula( EXC_FMLATYPE_SHARED, *pShrdScTokArr, &rScPos );
        xRec.reset( new XclExpShrfmla( xTokArr, rScPos ) );
        maRecMap[ pShrdScTokArr ] = xRec;
    }
    else
    {
        
        OSL_ENSURE( aIt->second, "XclExpShrfmlaBuffer::CreateOrExtendShrfmla - missing record" );
        xRec = aIt->second;
        xRec->ExtendRange( rScPos );
    }

    return xRec;
}



XclExpTableop::XclExpTableop( const ScAddress& rScPos,
        const XclMultipleOpRefs& rRefs, sal_uInt8 nScMode ) :
    XclExpRangeFmlaBase( EXC_ID3_TABLEOP, 16, rScPos ),
    mnLastAppXclCol( static_cast< sal_uInt16 >( rScPos.Col() ) ),
    mnColInpXclCol( static_cast< sal_uInt16 >( rRefs.maColFirstScPos.Col() ) ),
    mnColInpXclRow( static_cast< sal_uInt16 >( rRefs.maColFirstScPos.Row() ) ),
    mnRowInpXclCol( static_cast< sal_uInt16 >( rRefs.maRowFirstScPos.Col() ) ),
    mnRowInpXclRow( static_cast< sal_uInt16 >( rRefs.maRowFirstScPos.Row() ) ),
    mnScMode( nScMode ),
    mbValid( false )
{
}

bool XclExpTableop::TryExtend( const ScAddress& rScPos, const XclMultipleOpRefs& rRefs )
{
    sal_uInt16 nXclCol = static_cast< sal_uInt16 >( rScPos.Col() );
    sal_uInt16 nXclRow = static_cast< sal_uInt16 >( rScPos.Row() );

    bool bOk = IsAppendable( nXclCol, nXclRow );
    if( bOk )
    {
        SCCOL nFirstScCol  = static_cast< SCCOL >( maXclRange.maFirst.mnCol );
        SCROW nFirstScRow  = static_cast< SCROW >( maXclRange.maFirst.mnRow );
        SCCOL nColInpScCol = static_cast< SCCOL >( mnColInpXclCol );
        SCROW nColInpScRow = static_cast< SCROW >( mnColInpXclRow );
        SCCOL nRowInpScCol = static_cast< SCCOL >( mnRowInpXclCol );
        SCROW nRowInpScRow = static_cast< SCROW >( mnRowInpXclRow );

        bOk =   ((mnScMode == 2) == rRefs.mbDblRefMode) &&
                (rScPos.Tab() == rRefs.maFmlaScPos.Tab()) &&
                (nColInpScCol == rRefs.maColFirstScPos.Col()) &&
                (nColInpScRow == rRefs.maColFirstScPos.Row()) &&
                (rScPos.Tab() == rRefs.maColFirstScPos.Tab()) &&
                (rScPos.Tab() == rRefs.maColRelScPos.Tab());

        if( bOk ) switch( mnScMode )
        {
            case 0:
                bOk =   (rScPos.Col() == rRefs.maFmlaScPos.Col()) &&
                        (nFirstScRow  == rRefs.maFmlaScPos.Row() + 1) &&
                        (nFirstScCol  == rRefs.maColRelScPos.Col() + 1) &&
                        (rScPos.Row() == rRefs.maColRelScPos.Row());
            break;
            case 1:
                bOk =   (nFirstScCol  == rRefs.maFmlaScPos.Col() + 1) &&
                        (rScPos.Row() == rRefs.maFmlaScPos.Row()) &&
                        (rScPos.Col() == rRefs.maColRelScPos.Col()) &&
                        (nFirstScRow  == rRefs.maColRelScPos.Row() + 1);
            break;
            case 2:
                bOk =   (nFirstScCol  == rRefs.maFmlaScPos.Col() + 1) &&
                        (nFirstScRow  == rRefs.maFmlaScPos.Row() + 1) &&
                        (nFirstScCol  == rRefs.maColRelScPos.Col() + 1) &&
                        (rScPos.Row() == rRefs.maColRelScPos.Row()) &&
                        (nRowInpScCol == rRefs.maRowFirstScPos.Col()) &&
                        (nRowInpScRow == rRefs.maRowFirstScPos.Row()) &&
                        (rScPos.Tab() == rRefs.maRowFirstScPos.Tab()) &&
                        (rScPos.Col() == rRefs.maRowRelScPos.Col()) &&
                        (nFirstScRow  == rRefs.maRowRelScPos.Row() + 1) &&
                        (rScPos.Tab() == rRefs.maRowRelScPos.Tab());
            break;
            default:
                bOk = false;
        }

        if( bOk )
        {
            
            OSL_ENSURE( IsAppendable( nXclCol, nXclRow ), "XclExpTableop::TryExtend - wrong cell address" );
            Extend( rScPos );
            mnLastAppXclCol = nXclCol;
        }
    }

    return bOk;
}

void XclExpTableop::Finalize()
{
    
    mbValid = maXclRange.maLast.mnCol == mnLastAppXclCol;
    
    if( !mbValid && (maXclRange.maFirst.mnRow < maXclRange.maLast.mnRow) )
    {
        --maXclRange.maLast.mnRow;
        mbValid = true;
    }

    
    if( mbValid ) switch( mnScMode )
    {
        case 0:
            mbValid =   (mnColInpXclCol + 1 < maXclRange.maFirst.mnCol) || (mnColInpXclCol > maXclRange.maLast.mnCol) ||
                        (mnColInpXclRow     < maXclRange.maFirst.mnRow) || (mnColInpXclRow > maXclRange.maLast.mnRow);
        break;
        case 1:
            mbValid =   (mnColInpXclCol     < maXclRange.maFirst.mnCol) || (mnColInpXclCol > maXclRange.maLast.mnCol) ||
                        (mnColInpXclRow + 1 < maXclRange.maFirst.mnRow) || (mnColInpXclRow > maXclRange.maLast.mnRow);
        break;
        case 2:
            mbValid =   ((mnColInpXclCol + 1 < maXclRange.maFirst.mnCol) || (mnColInpXclCol > maXclRange.maLast.mnCol) ||
                         (mnColInpXclRow + 1 < maXclRange.maFirst.mnRow) || (mnColInpXclRow > maXclRange.maLast.mnRow)) &&
                        ((mnRowInpXclCol + 1 < maXclRange.maFirst.mnCol) || (mnRowInpXclCol > maXclRange.maLast.mnCol) ||
                         (mnRowInpXclRow + 1 < maXclRange.maFirst.mnRow) || (mnRowInpXclRow > maXclRange.maLast.mnRow));
        break;
    }
}

XclTokenArrayRef XclExpTableop::CreateCellTokenArray( const XclExpRoot& rRoot ) const
{
    XclExpFormulaCompiler& rFmlaComp = rRoot.GetFormulaCompiler();
    return mbValid ?
        rFmlaComp.CreateSpecialRefFormula( EXC_TOKID_TBL, maBaseXclPos ) :
        rFmlaComp.CreateErrorFormula( EXC_ERR_NA );
}

bool XclExpTableop::IsVolatile() const
{
    return true;
}

void XclExpTableop::Save( XclExpStream& rStrm )
{
    if( mbValid )
        XclExpRangeFmlaBase::Save( rStrm );
}

bool XclExpTableop::IsAppendable( sal_uInt16 nXclCol, sal_uInt16 nXclRow ) const
{
    return  ((nXclCol == mnLastAppXclCol + 1) && (nXclRow == maXclRange.maFirst.mnRow)) ||
            ((nXclCol == mnLastAppXclCol + 1) && (nXclCol <= maXclRange.maLast.mnCol) && (nXclRow == maXclRange.maLast.mnRow)) ||
            ((mnLastAppXclCol == maXclRange.maLast.mnCol) && (nXclCol == maXclRange.maFirst.mnCol) && (nXclRow == maXclRange.maLast.mnRow + 1));
}

void XclExpTableop::WriteBody( XclExpStream& rStrm )
{
    sal_uInt16 nFlags = EXC_TABLEOP_DEFAULTFLAGS;
    ::set_flag( nFlags, EXC_TABLEOP_RECALC_ALWAYS, IsVolatile() );
    switch( mnScMode )
    {
        case 1: ::set_flag( nFlags, EXC_TABLEOP_ROW );  break;
        case 2: ::set_flag( nFlags, EXC_TABLEOP_BOTH ); break;
    }

    WriteRangeAddress( rStrm );
    rStrm << nFlags;
    if( mnScMode == 2 )
        rStrm << mnRowInpXclRow << mnRowInpXclCol << mnColInpXclRow << mnColInpXclCol;
    else
        rStrm << mnColInpXclRow << mnColInpXclCol << sal_uInt32( 0 );
}



XclExpTableopBuffer::XclExpTableopBuffer( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot )
{
}

XclExpTableopRef XclExpTableopBuffer::CreateOrExtendTableop(
        const ScTokenArray& rScTokArr, const ScAddress& rScPos )
{
    XclExpTableopRef xRec;

    
    XclMultipleOpRefs aRefs;
    if (XclTokenArrayHelper::GetMultipleOpRefs(aRefs, rScTokArr, rScPos))
    {
        
        for( size_t nPos = 0, nSize = maTableopList.GetSize(); !xRec && (nPos < nSize); ++nPos )
        {
            XclExpTableopRef xTempRec = maTableopList.GetRecord( nPos );
            if( xTempRec->TryExtend( rScPos, aRefs ) )
                xRec = xTempRec;
        }

        
        if( !xRec )
            xRec = TryCreate( rScPos, aRefs );
    }

    return xRec;
}

void XclExpTableopBuffer::Finalize()
{
    for( size_t nPos = 0, nSize = maTableopList.GetSize(); nPos < nSize; ++nPos )
        maTableopList.GetRecord( nPos )->Finalize();
}

XclExpTableopRef XclExpTableopBuffer::TryCreate( const ScAddress& rScPos, const XclMultipleOpRefs& rRefs )
{
    sal_uInt8 nScMode = 0;
    bool bOk =  (rScPos.Tab() == rRefs.maFmlaScPos.Tab()) &&
                (rScPos.Tab() == rRefs.maColFirstScPos.Tab()) &&
                (rScPos.Tab() == rRefs.maColRelScPos.Tab());

    if( bOk )
    {
        if( rRefs.mbDblRefMode )
        {
            nScMode = 2;
            bOk =   (rScPos.Col() == rRefs.maFmlaScPos.Col() + 1) &&
                    (rScPos.Row() == rRefs.maFmlaScPos.Row() + 1) &&
                    (rScPos.Col() == rRefs.maColRelScPos.Col() + 1) &&
                    (rScPos.Row() == rRefs.maColRelScPos.Row()) &&
                    (rScPos.Tab() == rRefs.maRowFirstScPos.Tab()) &&
                    (rScPos.Col() == rRefs.maRowRelScPos.Col()) &&
                    (rScPos.Row() == rRefs.maRowRelScPos.Row() + 1) &&
                    (rScPos.Tab() == rRefs.maRowRelScPos.Tab());
        }
        else if( (rScPos.Col() == rRefs.maFmlaScPos.Col()) &&
                 (rScPos.Row() == rRefs.maFmlaScPos.Row() + 1) &&
                 (rScPos.Col() == rRefs.maColRelScPos.Col() + 1) &&
                 (rScPos.Row() == rRefs.maColRelScPos.Row()) )
        {
            nScMode = 0;
        }
        else if( (rScPos.Col() == rRefs.maFmlaScPos.Col() + 1) &&
                 (rScPos.Row() == rRefs.maFmlaScPos.Row()) &&
                 (rScPos.Col() == rRefs.maColRelScPos.Col()) &&
                 (rScPos.Row() == rRefs.maColRelScPos.Row() + 1) )
        {
            nScMode = 1;
        }
        else
        {
            bOk = false;
        }
    }

    XclExpTableopRef xRec;
    if( bOk )
    {
        xRec.reset( new XclExpTableop( rScPos, rRefs, nScMode ) );
        maTableopList.AppendRecord( xRec );
    }

    return xRec;
}





XclExpCellBase::XclExpCellBase(
        sal_uInt16 nRecId, sal_Size nContSize, const XclAddress& rXclPos ) :
    XclExpRecord( nRecId, nContSize + 4 ),
    maXclPos( rXclPos )
{
}

bool XclExpCellBase::IsMultiLineText() const
{
    return false;
}

bool XclExpCellBase::TryMerge( const XclExpCellBase& /*rCell*/ )
{
    return false;
}

void XclExpCellBase::GetBlankXFIndexes( ScfUInt16Vec& /*rXFIndexes*/ ) const
{
    
}

void XclExpCellBase::RemoveUnusedBlankCells( const ScfUInt16Vec& /*rXFIndexes*/ )
{
    
}



XclExpSingleCellBase::XclExpSingleCellBase(
        sal_uInt16 nRecId, sal_Size nContSize, const XclAddress& rXclPos, sal_uInt32 nXFId ) :
    XclExpCellBase( nRecId, 2, rXclPos ),
    maXFId( nXFId ),
    mnContSize( nContSize )
{
}

XclExpSingleCellBase::XclExpSingleCellBase( const XclExpRoot& rRoot,
        sal_uInt16 nRecId, sal_Size nContSize, const XclAddress& rXclPos,
        const ScPatternAttr* pPattern, sal_Int16 nScript, sal_uInt32 nForcedXFId ) :
    XclExpCellBase( nRecId, 2, rXclPos ),
    maXFId( nForcedXFId ),
    mnContSize( nContSize )
{
    if( GetXFId() == EXC_XFID_NOTFOUND )
        SetXFId( rRoot.GetXFBuffer().Insert( pPattern, nScript ) );
}

sal_uInt16 XclExpSingleCellBase::GetLastXclCol() const
{
    return GetXclCol();
}

sal_uInt32 XclExpSingleCellBase::GetFirstXFId() const
{
    return GetXFId();
}

bool XclExpSingleCellBase::IsEmpty() const
{
    return false;
}

void XclExpSingleCellBase::ConvertXFIndexes( const XclExpRoot& rRoot )
{
    maXFId.ConvertXFIndex( rRoot );
}

void XclExpSingleCellBase::Save( XclExpStream& rStrm )
{
    OSL_ENSURE_BIFF( rStrm.GetRoot().GetBiff() >= EXC_BIFF3 );
    AddRecSize( mnContSize );
    XclExpCellBase::Save( rStrm );
}

void XclExpSingleCellBase::WriteBody( XclExpStream& rStrm )
{
    rStrm << static_cast<sal_uInt16> (GetXclRow()) << GetXclCol() << maXFId.mnXFIndex;
    WriteContents( rStrm );
}



IMPL_FIXEDMEMPOOL_NEWDEL( XclExpNumberCell )

XclExpNumberCell::XclExpNumberCell(
        const XclExpRoot& rRoot, const XclAddress& rXclPos,
        const ScPatternAttr* pPattern, sal_uInt32 nForcedXFId, double fValue ) :
    
    XclExpSingleCellBase( rRoot, EXC_ID3_NUMBER, 8, rXclPos, pPattern, ApiScriptType::LATIN, nForcedXFId ),
    mfValue( fValue )
{
}

static OString lcl_GetStyleId( XclExpXmlStream& rStrm, sal_uInt32 nXFIndex )
{
    return OString::number( rStrm.GetRoot().GetXFBuffer()
            .GetXmlCellIndex( nXFIndex ) );
}

static OString lcl_GetStyleId( XclExpXmlStream& rStrm, const XclExpCellBase& rCell )
{
    sal_uInt32 nXFId    = rCell.GetFirstXFId();
    sal_uInt16 nXFIndex = rStrm.GetRoot().GetXFBuffer().GetXFIndex( nXFId );
    return lcl_GetStyleId( rStrm, nXFIndex );
}

void XclExpNumberCell::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_c,
            XML_r,      XclXmlUtils::ToOString( GetXclPos() ).getStr(),
            XML_s,      lcl_GetStyleId( rStrm, *this ).getStr(),
            XML_t,      "n",
            
            FSEND );
    rWorksheet->startElement( XML_v, FSEND );
    rWorksheet->write( mfValue );
    rWorksheet->endElement( XML_v );
    rWorksheet->endElement( XML_c );
}

void XclExpNumberCell::WriteContents( XclExpStream& rStrm )
{
    rStrm << mfValue;
}



IMPL_FIXEDMEMPOOL_NEWDEL( XclExpBooleanCell )

XclExpBooleanCell::XclExpBooleanCell(
        const XclExpRoot rRoot, const XclAddress& rXclPos,
        const ScPatternAttr* pPattern, sal_uInt32 nForcedXFId, bool bValue ) :
    
    XclExpSingleCellBase( rRoot, EXC_ID3_BOOLERR, 2, rXclPos, pPattern, ApiScriptType::LATIN, nForcedXFId ),
    mbValue( bValue )
{
}

void XclExpBooleanCell::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_c,
            XML_r,      XclXmlUtils::ToOString( GetXclPos() ).getStr(),
            XML_s,      lcl_GetStyleId( rStrm, *this ).getStr(),
            XML_t,      "b",
            
            FSEND );
    rWorksheet->startElement( XML_v, FSEND );
    rWorksheet->write( mbValue ? "1" : "0" );
    rWorksheet->endElement( XML_v );
    rWorksheet->endElement( XML_c );
}

void XclExpBooleanCell::WriteContents( XclExpStream& rStrm )
{
    rStrm << sal_uInt16( mbValue ? 1 : 0 ) << EXC_BOOLERR_BOOL;
}

IMPL_FIXEDMEMPOOL_NEWDEL( XclExpLabelCell )

XclExpLabelCell::XclExpLabelCell(
        const XclExpRoot& rRoot, const XclAddress& rXclPos,
        const ScPatternAttr* pPattern, sal_uInt32 nForcedXFId, const OUString& rStr ) :
    XclExpSingleCellBase( EXC_ID3_LABEL, 0, rXclPos, nForcedXFId )
{
    sal_uInt16 nMaxLen = (rRoot.GetBiff() == EXC_BIFF8) ? EXC_STR_MAXLEN : EXC_LABEL_MAXLEN;
    XclExpStringRef xText = XclExpStringHelper::CreateCellString(
        rRoot, rStr, pPattern, EXC_STR_DEFAULT, nMaxLen);
    Init( rRoot, pPattern, xText );
}

XclExpLabelCell::XclExpLabelCell(
        const XclExpRoot& rRoot, const XclAddress& rXclPos,
        const ScPatternAttr* pPattern, sal_uInt32 nForcedXFId,
        const EditTextObject* pEditText, XclExpHyperlinkHelper& rLinkHelper ) :
    XclExpSingleCellBase( EXC_ID3_LABEL, 0, rXclPos, nForcedXFId )
{
    sal_uInt16 nMaxLen = (rRoot.GetBiff() == EXC_BIFF8) ? EXC_STR_MAXLEN : EXC_LABEL_MAXLEN;

    XclExpStringRef xText;
    if (pEditText)
        xText = XclExpStringHelper::CreateCellString(
            rRoot, *pEditText, pPattern, rLinkHelper, EXC_STR_DEFAULT, nMaxLen);
    else
        xText = XclExpStringHelper::CreateCellString(
            rRoot, EMPTY_OUSTRING, pPattern, EXC_STR_DEFAULT, nMaxLen);

    Init( rRoot, pPattern, xText );
}

bool XclExpLabelCell::IsMultiLineText() const
{
    return mbLineBreak || mxText->IsWrapped();
}

void XclExpLabelCell::Init( const XclExpRoot& rRoot,
        const ScPatternAttr* pPattern, XclExpStringRef xText )
{
    OSL_ENSURE( xText && xText->Len(), "XclExpLabelCell::XclExpLabelCell - empty string passed" );
    mxText = xText;
    mnSstIndex = 0;

    
    sal_uInt16 nXclFont = mxText->RemoveLeadingFont();
    if( GetXFId() == EXC_XFID_NOTFOUND )
    {
        OSL_ENSURE( nXclFont != EXC_FONT_NOTFOUND, "XclExpLabelCell::Init - leading font not found" );
        bool bForceLineBreak = mxText->IsWrapped();
        SetXFId( rRoot.GetXFBuffer().InsertWithFont( pPattern, ApiScriptType::WEAK, nXclFont, bForceLineBreak ) );
    }

    
    const XclExpXF* pXF = rRoot.GetXFBuffer().GetXFById( GetXFId() );
    mbLineBreak = pXF && pXF->GetAlignmentData().mbLineBreak;

    
    switch( rRoot.GetBiff() )
    {
        case EXC_BIFF5:
            
            OSL_ENSURE( mxText->Len() <= EXC_LABEL_MAXLEN, "XclExpLabelCell::XclExpLabelCell - string too long" );
            SetContSize( mxText->GetSize() );
            
            if( mxText->IsRich() )
            {
                OSL_ENSURE( mxText->GetFormatsCount() <= EXC_LABEL_MAXLEN, "XclExpLabelCell::WriteContents - too many formats" );
                mxText->LimitFormatCount( EXC_LABEL_MAXLEN );
                SetRecId( EXC_ID_RSTRING );
                SetContSize( GetContSize() + 1 + 2 * mxText->GetFormatsCount() );
            }
        break;
        case EXC_BIFF8:
            
            mnSstIndex = rRoot.GetSst().Insert( xText );
            SetRecId( EXC_ID_LABELSST );
            SetContSize( 4 );
        break;
        default:    DBG_ERROR_BIFF();
    }
}

void XclExpLabelCell::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_c,
            XML_r,      XclXmlUtils::ToOString( GetXclPos() ).getStr(),
            XML_s,      lcl_GetStyleId( rStrm, *this ).getStr(),
            XML_t,      "s",
            
            FSEND );
    rWorksheet->startElement( XML_v, FSEND );
    rWorksheet->write( (sal_Int32) mnSstIndex );
    rWorksheet->endElement( XML_v );
    rWorksheet->endElement( XML_c );
}

void XclExpLabelCell::WriteContents( XclExpStream& rStrm )
{
    switch( rStrm.GetRoot().GetBiff() )
    {
        case EXC_BIFF5:
            rStrm << *mxText;
            if( mxText->IsRich() )
            {
                rStrm << static_cast< sal_uInt8 >( mxText->GetFormatsCount() );
                mxText->WriteFormats( rStrm );
            }
        break;
        case EXC_BIFF8:
            rStrm << mnSstIndex;
        break;
        default:    DBG_ERROR_BIFF();
    }
}



IMPL_FIXEDMEMPOOL_NEWDEL( XclExpFormulaCell )

XclExpFormulaCell::XclExpFormulaCell(
        const XclExpRoot& rRoot, const XclAddress& rXclPos,
        const ScPatternAttr* pPattern, sal_uInt32 nForcedXFId,
        const ScFormulaCell& rScFmlaCell,
        XclExpArrayBuffer& rArrayBfr,
        XclExpShrfmlaBuffer& rShrfmlaBfr,
        XclExpTableopBuffer& rTableopBfr ) :
    XclExpSingleCellBase( EXC_ID2_FORMULA, 0, rXclPos, nForcedXFId ),
    mrScFmlaCell( const_cast< ScFormulaCell& >( rScFmlaCell ) )
{
    

    if( GetXFId() == EXC_XFID_NOTFOUND )
    {
        SvNumberFormatter& rFormatter = rRoot.GetFormatter();
        XclExpNumFmtBuffer& rNumFmtBfr = rRoot.GetNumFmtBuffer();

        
        sal_uLong nScNumFmt = pPattern ?
            GETITEMVALUE( pPattern->GetItemSet(), SfxUInt32Item, ATTR_VALUE_FORMAT, sal_uLong ) :
            rNumFmtBfr.GetStandardFormat();

        
        sal_uLong nAltScNumFmt = NUMBERFORMAT_ENTRY_NOT_FOUND;
        /*  Xcl doesn't know Boolean number formats, we write
            "TRUE";"FALSE" (language dependent). Don't do it for automatic
            formula formats, because Excel gets them right. */
        /*  #i8640# Don't set text format, if we have string results. */
        short nFormatType = mrScFmlaCell.GetFormatType();
        if( ((nScNumFmt % SV_COUNTRY_LANGUAGE_OFFSET) == 0) &&
                (nFormatType != NUMBERFORMAT_LOGICAL) &&
                (nFormatType != NUMBERFORMAT_TEXT) )
            nAltScNumFmt = nScNumFmt;
        /*  If cell number format is Boolean and automatic formula
            format is Boolean don't write that ugly special format. */
        else if( (nFormatType == NUMBERFORMAT_LOGICAL) &&
                (rFormatter.GetType( nScNumFmt ) == NUMBERFORMAT_LOGICAL) )
            nAltScNumFmt = rNumFmtBfr.GetStandardFormat();

        
        sal_Int16 nScript = ApiScriptType::LATIN;
        bool bForceLineBreak = false;
        if( nFormatType == NUMBERFORMAT_TEXT )
        {
            OUString aResult = mrScFmlaCell.GetString().getString();
            bForceLineBreak = mrScFmlaCell.IsMultilineResult();
            nScript = XclExpStringHelper::GetLeadingScriptType( rRoot, aResult );
        }
        SetXFId( rRoot.GetXFBuffer().InsertWithNumFmt( pPattern, nScript, nAltScNumFmt, bForceLineBreak ) );
    }

    

    ScAddress aScPos( static_cast< SCCOL >( rXclPos.mnCol ), static_cast< SCROW >( rXclPos.mnRow ), rRoot.GetCurrScTab() );
    const ScTokenArray& rScTokArr = *mrScFmlaCell.GetCode();

    
    mxAddRec = rTableopBfr.CreateOrExtendTableop( rScTokArr, aScPos );

    
    if( !mxAddRec ) switch( static_cast< ScMatrixMode >( mrScFmlaCell.GetMatrixFlag() ) )
    {
        case MM_FORMULA:
        {
            
            SCCOL nMatWidth;
            SCROW nMatHeight;
            mrScFmlaCell.GetMatColsRows( nMatWidth, nMatHeight );
            OSL_ENSURE( nMatWidth && nMatHeight, "XclExpFormulaCell::XclExpFormulaCell - empty matrix" );
            ScRange aMatScRange( aScPos );
            ScAddress& rMatEnd = aMatScRange.aEnd;
            rMatEnd.IncCol( static_cast< SCsCOL >( nMatWidth - 1 ) );
            rMatEnd.IncRow( static_cast< SCsROW >( nMatHeight - 1 ) );
            
            rRoot.GetAddressConverter().ValidateRange( aMatScRange, true );
            
            mxAddRec = rArrayBfr.CreateArray( rScTokArr, aMatScRange );
        }
        break;
        case MM_REFERENCE:
        {
            
            mxAddRec = rArrayBfr.FindArray(rScTokArr, aScPos);
            
            OSL_ENSURE( mxAddRec, "XclExpFormulaCell::XclExpFormulaCell - no matrix found" );
        }
        break;
        default:;
    }

    
    if( !mxAddRec )
        mxAddRec = rShrfmlaBfr.CreateOrExtendShrfmla(mrScFmlaCell, aScPos);

    
    if( !mxAddRec )
        mxTokArr = rRoot.GetFormulaCompiler().CreateFormula( EXC_FMLATYPE_CELL, rScTokArr, &aScPos );
}

void XclExpFormulaCell::Save( XclExpStream& rStrm )
{
    
    if( mxAddRec )
        mxTokArr = mxAddRec->CreateCellTokenArray( rStrm.GetRoot() );

    
    OSL_ENSURE( mxTokArr, "XclExpFormulaCell::Save - missing token array" );
    if( !mxTokArr )
        mxTokArr = rStrm.GetRoot().GetFormulaCompiler().CreateErrorFormula( EXC_ERR_NA );
    SetContSize( 16 + mxTokArr->GetSize() );
    XclExpSingleCellBase::Save( rStrm );

    
    if( mxAddRec && mxAddRec->IsBasePos( GetXclCol(), GetXclRow() ) )
        mxAddRec->Save( rStrm );

    
    if( mxStringRec )
        mxStringRec->Save( rStrm );
}

void XclExpFormulaCell::SaveXml( XclExpXmlStream& rStrm )
{
    const char* sType = NULL;
    OUString    sValue;

    XclXmlUtils::GetFormulaTypeAndValue( mrScFmlaCell, sType, sValue );
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_c,
            XML_r,      XclXmlUtils::ToOString( GetXclPos() ).getStr(),
            XML_s,      lcl_GetStyleId( rStrm, *this ).getStr(),
            XML_t,      sType,
            
            FSEND );

    rWorksheet->startElement( XML_f,
            
            XML_aca,    XclXmlUtils::ToPsz( (mxTokArr && mxTokArr->IsVolatile()) || (mxAddRec && mxAddRec->IsVolatile()) ),
            
            
            
            
            
            
            
            
            
            
            FSEND );
    rWorksheet->writeEscaped( XclXmlUtils::ToOUString( *mrScFmlaCell.GetDocument(), mrScFmlaCell.aPos,
                mrScFmlaCell.GetCode(), rStrm.GetRoot().GetOpCodeMap() ) );
    rWorksheet->endElement( XML_f );
    if( strcmp( sType, "inlineStr" ) == 0 )
    {
        rWorksheet->startElement( XML_is, FSEND );
        rWorksheet->startElement( XML_t, FSEND );
        rWorksheet->writeEscaped( sValue );
        rWorksheet->endElement( XML_t );
        rWorksheet->endElement( XML_is );
    }
    else
    {
        rWorksheet->startElement( XML_v, FSEND );
        rWorksheet->writeEscaped( sValue );
        rWorksheet->endElement( XML_v );
    }
    rWorksheet->endElement( XML_c );
}

void XclExpFormulaCell::WriteContents( XclExpStream& rStrm )
{
    sal_uInt16 nScErrCode = mrScFmlaCell.GetErrCode();
    if( nScErrCode )
    {
        rStrm << EXC_FORMULA_RES_ERROR << sal_uInt8( 0 )
            << XclTools::GetXclErrorCode( nScErrCode )
            << sal_uInt8( 0 ) << sal_uInt16( 0 )
            << sal_uInt16( 0xFFFF );
    }
    else
    {
        
        switch( mrScFmlaCell.GetFormatType() )
        {
            case NUMBERFORMAT_NUMBER:
                {
                    
                    rStrm << mrScFmlaCell.GetValue();
                }
                break;

            case NUMBERFORMAT_TEXT:
                {
                    OUString aResult = mrScFmlaCell.GetString().getString();
                    if( !aResult.isEmpty() || (rStrm.GetRoot().GetBiff() <= EXC_BIFF5) )
                    {
                        rStrm << EXC_FORMULA_RES_STRING;
                        mxStringRec.reset( new XclExpStringRec( rStrm.GetRoot(), aResult ) );
                    }
                    else
                        rStrm << EXC_FORMULA_RES_EMPTY;     
                    rStrm << sal_uInt8( 0 ) << sal_uInt32( 0 ) << sal_uInt16( 0xFFFF );
                }
                break;

            case NUMBERFORMAT_LOGICAL:
                {
                    sal_uInt8 nXclValue = (mrScFmlaCell.GetValue() == 0.0) ? 0 : 1;
                    rStrm << EXC_FORMULA_RES_BOOL << sal_uInt8( 0 )
                        << nXclValue << sal_uInt8( 0 ) << sal_uInt16( 0 )
                        << sal_uInt16( 0xFFFF );
                }
                break;

            default:
                rStrm << mrScFmlaCell.GetValue();
        }
    }

    
    sal_uInt16 nFlags = EXC_FORMULA_DEFAULTFLAGS;
    ::set_flag( nFlags, EXC_FORMULA_RECALC_ALWAYS, mxTokArr->IsVolatile() || (mxAddRec && mxAddRec->IsVolatile()) );
    ::set_flag( nFlags, EXC_FORMULA_SHARED, mxAddRec && (mxAddRec->GetRecId() == EXC_ID_SHRFMLA) );
    rStrm << nFlags << sal_uInt32( 0 ) << *mxTokArr;
}



XclExpMultiCellBase::XclExpMultiCellBase(
        sal_uInt16 nRecId, sal_uInt16 nMulRecId, sal_Size nContSize, const XclAddress& rXclPos ) :
    XclExpCellBase( nRecId, 0, rXclPos ),
    mnMulRecId( nMulRecId ),
    mnContSize( nContSize )
{
}

sal_uInt16 XclExpMultiCellBase::GetLastXclCol() const
{
    return GetXclCol() + GetCellCount() - 1;
}

sal_uInt32 XclExpMultiCellBase::GetFirstXFId() const
{
    return maXFIds.empty() ? XclExpXFBuffer::GetDefCellXFId() : maXFIds.front().mnXFId;
}

bool XclExpMultiCellBase::IsEmpty() const
{
    return maXFIds.empty();
}

void XclExpMultiCellBase::ConvertXFIndexes( const XclExpRoot& rRoot )
{
    for( XclExpMultiXFIdDeq::iterator aIt = maXFIds.begin(), aEnd = maXFIds.end(); aIt != aEnd; ++aIt )
        aIt->ConvertXFIndex( rRoot );
}

void XclExpMultiCellBase::Save( XclExpStream& rStrm )
{
    OSL_ENSURE_BIFF( rStrm.GetRoot().GetBiff() >= EXC_BIFF3 );

    XclExpMultiXFIdDeq::const_iterator aEnd = maXFIds.end();
    XclExpMultiXFIdDeq::const_iterator aRangeBeg = maXFIds.begin();
    XclExpMultiXFIdDeq::const_iterator aRangeEnd = aRangeBeg;
    sal_uInt16 nBegXclCol = GetXclCol();
    sal_uInt16 nEndXclCol = nBegXclCol;

    while( aRangeEnd != aEnd )
    {
        
        aRangeBeg = aRangeEnd;
        nBegXclCol = nEndXclCol;
        while( (aRangeBeg != aEnd) && (aRangeBeg->mnXFIndex == EXC_XF_NOTFOUND) )
        {
            nBegXclCol = nBegXclCol + aRangeBeg->mnCount;
            ++aRangeBeg;
        }
        
        aRangeEnd = aRangeBeg;
        nEndXclCol = nBegXclCol;
        while( (aRangeEnd != aEnd) && (aRangeEnd->mnXFIndex != EXC_XF_NOTFOUND) )
        {
            nEndXclCol = nEndXclCol + aRangeEnd->mnCount;
            ++aRangeEnd;
        }

        
        if( aRangeBeg != aRangeEnd )
        {
            sal_uInt16 nCount = nEndXclCol - nBegXclCol;
            bool bIsMulti = nCount > 1;
            sal_Size nTotalSize = GetRecSize() + (2 + mnContSize) * nCount;
            if( bIsMulti ) nTotalSize += 2;

            rStrm.StartRecord( bIsMulti ? mnMulRecId : GetRecId(), nTotalSize );
            rStrm << static_cast<sal_uInt16> (GetXclRow()) << nBegXclCol;

            sal_uInt16 nRelCol = nBegXclCol - GetXclCol();
            for( XclExpMultiXFIdDeq::const_iterator aIt = aRangeBeg; aIt != aRangeEnd; ++aIt )
            {
                for( sal_uInt16 nIdx = 0; nIdx < aIt->mnCount; ++nIdx )
                {
                    rStrm << aIt->mnXFIndex;
                    WriteContents( rStrm, nRelCol );
                    ++nRelCol;
                }
            }
            if( bIsMulti )
                rStrm << static_cast< sal_uInt16 >( nEndXclCol - 1 );
            rStrm.EndRecord();
        }
    }
}

void XclExpMultiCellBase::SaveXml( XclExpXmlStream& rStrm )
{
    XclExpMultiXFIdDeq::const_iterator aEnd = maXFIds.end();
    XclExpMultiXFIdDeq::const_iterator aRangeBeg = maXFIds.begin();
    XclExpMultiXFIdDeq::const_iterator aRangeEnd = aRangeBeg;
    sal_uInt16 nBegXclCol = GetXclCol();
    sal_uInt16 nEndXclCol = nBegXclCol;

    while( aRangeEnd != aEnd )
    {
        
        aRangeBeg = aRangeEnd;
        nBegXclCol = nEndXclCol;
        while( (aRangeBeg != aEnd) && (aRangeBeg->mnXFIndex == EXC_XF_NOTFOUND) )
        {
            nBegXclCol = nBegXclCol + aRangeBeg->mnCount;
            ++aRangeBeg;
        }
        
        aRangeEnd = aRangeBeg;
        nEndXclCol = nBegXclCol;
        while( (aRangeEnd != aEnd) && (aRangeEnd->mnXFIndex != EXC_XF_NOTFOUND) )
        {
            nEndXclCol = nEndXclCol + aRangeEnd->mnCount;
            ++aRangeEnd;
        }

        
        if( aRangeBeg != aRangeEnd )
        {
            sal_uInt16 nRelColIdx = nBegXclCol - GetXclCol();
            sal_Int32  nRelCol    = 0;
            for( XclExpMultiXFIdDeq::const_iterator aIt = aRangeBeg; aIt != aRangeEnd; ++aIt )
            {
                for( sal_uInt16 nIdx = 0; nIdx < aIt->mnCount; ++nIdx )
                {
                    WriteXmlContents(
                            rStrm,
                            XclAddress( static_cast<sal_uInt16>(nBegXclCol + nRelCol), GetXclRow() ),
                            aIt->mnXFIndex,
                            nRelColIdx );
                    ++nRelCol;
                    ++nRelColIdx;
                }
            }
        }
    }
}

sal_uInt16 XclExpMultiCellBase::GetCellCount() const
{
    sal_uInt16 nCount = 0;
    for( XclExpMultiXFIdDeq::const_iterator aIt = maXFIds.begin(), aEnd = maXFIds.end(); aIt != aEnd; ++aIt )
        nCount = nCount + aIt->mnCount;
    return nCount;
}

void XclExpMultiCellBase::AppendXFId( const XclExpMultiXFId& rXFId )
{
    if( maXFIds.empty() || (maXFIds.back().mnXFId != rXFId.mnXFId) )
        maXFIds.push_back( rXFId );
    else
        maXFIds.back().mnCount = maXFIds.back().mnCount + rXFId.mnCount;
}

void XclExpMultiCellBase::AppendXFId( const XclExpRoot& rRoot,
        const ScPatternAttr* pPattern, sal_uInt16 nScript, sal_uInt32 nForcedXFId, sal_uInt16 nCount )
{
    sal_uInt32 nXFId = (nForcedXFId == EXC_XFID_NOTFOUND) ?
        rRoot.GetXFBuffer().Insert( pPattern, nScript ) : nForcedXFId;
    AppendXFId( XclExpMultiXFId( nXFId, nCount ) );
}

bool XclExpMultiCellBase::TryMergeXFIds( const XclExpMultiCellBase& rCell )
{
    if( GetLastXclCol() + 1 == rCell.GetXclCol() )
    {
        maXFIds.insert( maXFIds.end(), rCell.maXFIds.begin(), rCell.maXFIds.end() );
        return true;
    }
    return false;
}

void XclExpMultiCellBase::GetXFIndexes( ScfUInt16Vec& rXFIndexes ) const
{
    OSL_ENSURE( GetLastXclCol() < rXFIndexes.size(), "XclExpMultiCellBase::GetXFIndexes - vector too small" );
    ScfUInt16Vec::iterator aDestIt = rXFIndexes.begin() + GetXclCol();
    for( XclExpMultiXFIdDeq::const_iterator aIt = maXFIds.begin(), aEnd = maXFIds.end(); aIt != aEnd; ++aIt )
    {
        ::std::fill( aDestIt, aDestIt + aIt->mnCount, aIt->mnXFIndex );
        aDestIt += aIt->mnCount;
    }
}

void XclExpMultiCellBase::RemoveUnusedXFIndexes( const ScfUInt16Vec& rXFIndexes )
{
    
    sal_uInt16 nLastXclCol = GetLastXclCol();
    OSL_ENSURE( nLastXclCol < rXFIndexes.size(), "XclExpMultiCellBase::RemoveUnusedXFIndexes - XF index vector too small" );

    
    maXFIds.clear();
    XclExpMultiXFId aXFId( 0 );
    for( ScfUInt16Vec::const_iterator aIt = rXFIndexes.begin() + GetXclCol(), aEnd = rXFIndexes.begin() + nLastXclCol + 1; aIt != aEnd; ++aIt )
    {
        
        aXFId.mnXFId = aXFId.mnXFIndex = *aIt;
        AppendXFId( aXFId );
    }

    
    if( !maXFIds.empty() && (maXFIds.front().mnXFIndex == EXC_XF_NOTFOUND) )
    {
        SetXclCol( GetXclCol() + maXFIds.front().mnCount );
        maXFIds.pop_front();
    }
    if( !maXFIds.empty() && (maXFIds.back().mnXFIndex == EXC_XF_NOTFOUND) )
        maXFIds.pop_back();

    
}



IMPL_FIXEDMEMPOOL_NEWDEL( XclExpBlankCell )

XclExpBlankCell::XclExpBlankCell( const XclAddress& rXclPos, const XclExpMultiXFId& rXFId ) :
    XclExpMultiCellBase( EXC_ID3_BLANK, EXC_ID_MULBLANK, 0, rXclPos )
{
    OSL_ENSURE( rXFId.mnCount > 0, "XclExpBlankCell::XclExpBlankCell - invalid count" );
    AppendXFId( rXFId );
}

XclExpBlankCell::XclExpBlankCell(
        const XclExpRoot& rRoot, const XclAddress& rXclPos, sal_uInt16 nLastXclCol,
        const ScPatternAttr* pPattern, sal_uInt32 nForcedXFId ) :
    XclExpMultiCellBase( EXC_ID3_BLANK, EXC_ID_MULBLANK, 0, rXclPos )
{
    OSL_ENSURE( rXclPos.mnCol <= nLastXclCol, "XclExpBlankCell::XclExpBlankCell - invalid column range" );
    
    AppendXFId( rRoot, pPattern, rRoot.GetDefApiScript(), nForcedXFId, nLastXclCol - rXclPos.mnCol + 1 );
}

bool XclExpBlankCell::TryMerge( const XclExpCellBase& rCell )
{
    const XclExpBlankCell* pBlankCell = dynamic_cast< const XclExpBlankCell* >( &rCell );
    return pBlankCell && TryMergeXFIds( *pBlankCell );
}

void XclExpBlankCell::GetBlankXFIndexes( ScfUInt16Vec& rXFIndexes ) const
{
    GetXFIndexes( rXFIndexes );
}

void XclExpBlankCell::RemoveUnusedBlankCells( const ScfUInt16Vec& rXFIndexes )
{
    RemoveUnusedXFIndexes( rXFIndexes );
}

void XclExpBlankCell::WriteContents( XclExpStream& /*rStrm*/, sal_uInt16 /*nRelCol*/ )
{
}

void XclExpBlankCell::WriteXmlContents( XclExpXmlStream& rStrm, const XclAddress& rAddress, sal_uInt32 nXFId, sal_uInt16 /* nRelCol */ )
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->singleElement( XML_c,
            XML_r,      XclXmlUtils::ToOString( rAddress ).getStr(),
            XML_s,      lcl_GetStyleId( rStrm, nXFId ).getStr(),
            FSEND );
}



IMPL_FIXEDMEMPOOL_NEWDEL( XclExpRkCell )

XclExpRkCell::XclExpRkCell(
        const XclExpRoot& rRoot, const XclAddress& rXclPos,
        const ScPatternAttr* pPattern, sal_uInt32 nForcedXFId, sal_Int32 nRkValue ) :
    XclExpMultiCellBase( EXC_ID_RK, EXC_ID_MULRK, 4, rXclPos )
{
    
    AppendXFId( rRoot, pPattern, ApiScriptType::LATIN, nForcedXFId );
    maRkValues.push_back( nRkValue );
}

bool XclExpRkCell::TryMerge( const XclExpCellBase& rCell )
{
    const XclExpRkCell* pRkCell = dynamic_cast< const XclExpRkCell* >( &rCell );
    if( pRkCell && TryMergeXFIds( *pRkCell ) )
    {
        maRkValues.insert( maRkValues.end(), pRkCell->maRkValues.begin(), pRkCell->maRkValues.end() );
        return true;
    }
    return false;
}

void XclExpRkCell::WriteXmlContents( XclExpXmlStream& rStrm, const XclAddress& rAddress, sal_uInt32 nXFId, sal_uInt16 nRelCol )
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_c,
            XML_r,      XclXmlUtils::ToOString( rAddress ).getStr(),
            XML_s,      lcl_GetStyleId( rStrm, nXFId ).getStr(),
            XML_t,      "n",
            
            FSEND );
    rWorksheet->startElement( XML_v, FSEND );
    rWorksheet->write( XclTools::GetDoubleFromRK( maRkValues[ nRelCol ] ) );
    rWorksheet->endElement( XML_v );
    rWorksheet->endElement( XML_c );
}

void XclExpRkCell::WriteContents( XclExpStream& rStrm, sal_uInt16 nRelCol )
{
    OSL_ENSURE( nRelCol < maRkValues.size(), "XclExpRkCell::WriteContents - overflow error" );
    rStrm << maRkValues[ nRelCol ];
}





XclExpOutlineBuffer::XclExpOutlineBuffer( const XclExpRoot& rRoot, bool bRows ) :
        mpScOLArray( 0 ),
        maLevelInfos( SC_OL_MAXDEPTH ),
        mnCurrLevel( 0 ),
        mbCurrCollapse( false )
{
    if( const ScOutlineTable* pOutlineTable = rRoot.GetDoc().GetOutlineTable( rRoot.GetCurrScTab() ) )
        mpScOLArray = bRows ? pOutlineTable->GetRowArray() : pOutlineTable->GetColArray();

    if( mpScOLArray )
        for( size_t nLevel = 0; nLevel < SC_OL_MAXDEPTH; ++nLevel )
            if( const ScOutlineEntry* pEntry = mpScOLArray->GetEntryByPos( nLevel, 0 ) )
                maLevelInfos[ nLevel ].mnScEndPos = pEntry->GetEnd();
}

void XclExpOutlineBuffer::UpdateColRow( SCCOLROW nScPos )
{
    if( mpScOLArray )
    {
        
        size_t nNewOpenScLevel = 0; 
        sal_uInt8 nNewLevel = 0;    

        if( mpScOLArray->FindTouchedLevel( nScPos, nScPos, nNewOpenScLevel ) )
            nNewLevel = static_cast< sal_uInt8 >( nNewOpenScLevel + 1 );
        

        mbCurrCollapse = false;
        if( nNewLevel >= mnCurrLevel )
        {
            
            for( sal_uInt16 nScLevel = 0; nScLevel <= nNewOpenScLevel; ++nScLevel )
            {
                /*  In each level: check if a new group is started (there may be
                    neighbored groups without gap - therefore check ALL levels). */
                if( maLevelInfos[ nScLevel ].mnScEndPos < nScPos )
                {
                    if( const ScOutlineEntry* pEntry = mpScOLArray->GetEntryByPos( nScLevel, nScPos ) )
                    {
                        maLevelInfos[ nScLevel ].mnScEndPos = pEntry->GetEnd();
                        maLevelInfos[ nScLevel ].mbHidden = pEntry->IsHidden();
                    }
                }
            }
        }
        else
        {
            
            
            sal_uInt16 nOldOpenScLevel = mnCurrLevel - 1;
            for( sal_uInt16 nScLevel = nNewOpenScLevel + 1; !mbCurrCollapse && (nScLevel <= nOldOpenScLevel); ++nScLevel )
                mbCurrCollapse = maLevelInfos[ nScLevel ].mbHidden;
        }

        
        mnCurrLevel = nNewLevel;
    }
}



XclExpGuts::XclExpGuts( const XclExpRoot& rRoot ) :
    XclExpRecord( EXC_ID_GUTS, 8 ),
    mnColLevels( 0 ),
    mnColWidth( 0 ),
    mnRowLevels( 0 ),
    mnRowWidth( 0 )
{
    if( const ScOutlineTable* pOutlineTable = rRoot.GetDoc().GetOutlineTable( rRoot.GetCurrScTab() ) )
    {
        
        if( const ScOutlineArray* pColArray = pOutlineTable->GetColArray() )
            mnColLevels = ulimit_cast< sal_uInt16 >( pColArray->GetDepth(), EXC_OUTLINE_MAX );
        if( mnColLevels )
        {
            ++mnColLevels;
            mnColWidth = 12 * mnColLevels + 5;
        }

        
        if( const ScOutlineArray* pRowArray = pOutlineTable->GetRowArray() )
            mnRowLevels = ulimit_cast< sal_uInt16 >( pRowArray->GetDepth(), EXC_OUTLINE_MAX );
        if( mnRowLevels )
        {
            ++mnRowLevels;
            mnRowWidth = 12 * mnRowLevels + 5;
        }
    }
}

void XclExpGuts::WriteBody( XclExpStream& rStrm )
{
    rStrm << mnRowWidth << mnColWidth << mnRowLevels << mnColLevels;
}



XclExpDimensions::XclExpDimensions( const XclExpRoot& rRoot ) :
    mnFirstUsedXclRow( 0 ),
    mnFirstFreeXclRow( 0 ),
    mnFirstUsedXclCol( 0 ),
    mnFirstFreeXclCol( 0 )
{
    switch( rRoot.GetBiff() )
    {
        case EXC_BIFF2: SetRecHeader( EXC_ID2_DIMENSIONS, 8 );  break;
        case EXC_BIFF3:
        case EXC_BIFF4:
        case EXC_BIFF5: SetRecHeader( EXC_ID3_DIMENSIONS, 10 ); break;
        case EXC_BIFF8: SetRecHeader( EXC_ID3_DIMENSIONS, 14 ); break;
        default:        DBG_ERROR_BIFF();
    }
}

void XclExpDimensions::SetDimensions(
        sal_uInt16 nFirstUsedXclCol, sal_uInt32 nFirstUsedXclRow,
        sal_uInt16 nFirstFreeXclCol, sal_uInt32 nFirstFreeXclRow )
{
    mnFirstUsedXclRow = nFirstUsedXclRow;
    mnFirstFreeXclRow = nFirstFreeXclRow;
    mnFirstUsedXclCol = nFirstUsedXclCol;
    mnFirstFreeXclCol = nFirstFreeXclCol;
}

void XclExpDimensions::SaveXml( XclExpXmlStream& rStrm )
{
    ScRange aRange;
    aRange.aStart.SetRow( (SCROW) mnFirstUsedXclRow );
    aRange.aStart.SetCol( (SCCOL) mnFirstUsedXclCol );

    if( mnFirstFreeXclRow != mnFirstUsedXclRow && mnFirstFreeXclCol != mnFirstUsedXclCol )
    {
        aRange.aEnd.SetRow( (SCROW) (mnFirstFreeXclRow-1) );
        aRange.aEnd.SetCol( (SCCOL) (mnFirstFreeXclCol-1) );
    }

    rStrm.GetCurrentStream()->singleElement( XML_dimension,
            XML_ref, XclXmlUtils::ToOString( aRange ).getStr(),
            FSEND );
}

void XclExpDimensions::WriteBody( XclExpStream& rStrm )
{
    XclBiff eBiff = rStrm.GetRoot().GetBiff();
    if( eBiff == EXC_BIFF8 )
        rStrm << mnFirstUsedXclRow << mnFirstFreeXclRow;
    else
        rStrm << static_cast< sal_uInt16 >( mnFirstUsedXclRow ) << static_cast< sal_uInt16 >( mnFirstFreeXclRow );
    rStrm << mnFirstUsedXclCol << mnFirstFreeXclCol;
    if( eBiff >= EXC_BIFF3 )
        rStrm << sal_uInt16( 0 );
}



namespace {

double lclGetCorrectedColWidth( const XclExpRoot& rRoot, sal_uInt16 nXclColWidth )
{
    long nFontHt = rRoot.GetFontBuffer().GetAppFontData().mnHeight;
    return nXclColWidth - XclTools::GetXclDefColWidthCorrection( nFontHt );
}

} 



XclExpDefcolwidth::XclExpDefcolwidth( const XclExpRoot& rRoot ) :
    XclExpUInt16Record( EXC_ID_DEFCOLWIDTH, EXC_DEFCOLWIDTH_DEF ),
    XclExpRoot( rRoot )
{
}

bool XclExpDefcolwidth::IsDefWidth( sal_uInt16 nXclColWidth ) const
{
    double fNewColWidth = lclGetCorrectedColWidth( GetRoot(), nXclColWidth );
    
    return std::abs( static_cast< long >( GetValue() * 256.0 - fNewColWidth + 0.5 ) ) < 16;
}

void XclExpDefcolwidth::SetDefWidth( sal_uInt16 nXclColWidth )
{
    double fNewColWidth = lclGetCorrectedColWidth( GetRoot(), nXclColWidth );
    SetValue( limit_cast< sal_uInt16 >( fNewColWidth / 256.0 + 0.5 ) );
}



XclExpColinfo::XclExpColinfo( const XclExpRoot& rRoot,
        SCCOL nScCol, SCROW nLastScRow, XclExpColOutlineBuffer& rOutlineBfr ) :
    XclExpRecord( EXC_ID_COLINFO, 12 ),
    XclExpRoot( rRoot ),
    mnWidth( 0 ),
    mnScWidth( 0 ),
    mnFlags( 0 ),
    mnFirstXclCol( static_cast< sal_uInt16 >( nScCol ) ),
    mnLastXclCol( static_cast< sal_uInt16 >( nScCol ) )
{
    ScDocument& rDoc = GetDoc();
    SCTAB nScTab = GetCurrScTab();

    
    maXFId.mnXFId = GetXFBuffer().Insert(
        rDoc.GetMostUsedPattern( nScCol, 0, nLastScRow, nScTab ), GetDefApiScript() );

    
    sal_uInt16 nScWidth = rDoc.GetColWidth( nScCol, nScTab );
    mnWidth = XclTools::GetXclColumnWidth( nScWidth, GetCharWidth() );
    mnScWidth =  sc::TwipsToHMM( nScWidth );
    
    ::set_flag( mnFlags, EXC_COLINFO_HIDDEN, rDoc.ColHidden(nScCol, nScTab) );

    
    rOutlineBfr.Update( nScCol );
    ::set_flag( mnFlags, EXC_COLINFO_COLLAPSED, rOutlineBfr.IsCollapsed() );
    ::insert_value( mnFlags, rOutlineBfr.GetLevel(), 8, 3 );
}

sal_uInt16 XclExpColinfo::ConvertXFIndexes()
{
    maXFId.ConvertXFIndex( GetRoot() );
    return maXFId.mnXFIndex;
}

bool XclExpColinfo::IsDefault( const XclExpDefcolwidth& rDefColWidth ) const
{
    return (maXFId.mnXFIndex == EXC_XF_DEFAULTCELL) && (mnFlags == 0) && rDefColWidth.IsDefWidth( mnWidth );
}

bool XclExpColinfo::TryMerge( const XclExpColinfo& rColInfo )
{
    if( (maXFId.mnXFIndex == rColInfo.maXFId.mnXFIndex) &&
        (mnWidth == rColInfo.mnWidth) &&
        (mnFlags == rColInfo.mnFlags) &&
        (mnLastXclCol + 1 == rColInfo.mnFirstXclCol) )
    {
        mnLastXclCol = rColInfo.mnLastXclCol;
        return true;
    }
    return false;
}

void XclExpColinfo::WriteBody( XclExpStream& rStrm )
{
    
    sal_uInt16 nLastXclCol = mnLastXclCol;
    if( nLastXclCol == static_cast< sal_uInt16 >( rStrm.GetRoot().GetMaxPos().Col() ) )
        ++nLastXclCol;

    rStrm   << mnFirstXclCol
            << nLastXclCol
            << mnWidth
            << maXFId.mnXFIndex
            << mnFlags
            << sal_uInt16( 0 );
}

void XclExpColinfo::SaveXml( XclExpXmlStream& rStrm )
{
    
    sal_uInt16 nLastXclCol = mnLastXclCol;
    if( nLastXclCol == static_cast< sal_uInt16 >( rStrm.GetRoot().GetMaxPos().Col() ) )
        ++nLastXclCol;

    rStrm.GetCurrentStream()->singleElement( XML_col,
            
            XML_collapsed,      XclXmlUtils::ToPsz( ::get_flag( mnFlags, EXC_COLINFO_COLLAPSED ) ),
            
            XML_hidden,         XclXmlUtils::ToPsz( ::get_flag( mnFlags, EXC_COLINFO_HIDDEN ) ),
            XML_max,            OString::number(  (nLastXclCol+1) ).getStr(),
            XML_min,            OString::number(  (mnFirstXclCol+1) ).getStr(),
            
            
            XML_style,          lcl_GetStyleId( rStrm, maXFId.mnXFIndex ).getStr(),
            XML_width,          OString::number( (double) (mnScWidth / (double)sc::TwipsToHMM( GetCharWidth() )) ).getStr(),
            FSEND );
}



XclExpColinfoBuffer::XclExpColinfoBuffer( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    maDefcolwidth( rRoot ),
    maOutlineBfr( rRoot )
{
}

void XclExpColinfoBuffer::Initialize( SCROW nLastScRow )
{

    for( sal_uInt16 nScCol = 0, nLastScCol = GetMaxPos().Col(); nScCol <= nLastScCol; ++nScCol )
        maColInfos.AppendNewRecord( new XclExpColinfo( GetRoot(), nScCol, nLastScRow, maOutlineBfr ) );
}

void XclExpColinfoBuffer::Finalize( ScfUInt16Vec& rXFIndexes )
{
    rXFIndexes.clear();
    rXFIndexes.reserve( maColInfos.GetSize() );

    size_t nPos, nSize;

    
    for( nPos = 0; nPos < maColInfos.GetSize(); ++nPos )
    {
        XclExpColinfoRef xRec = maColInfos.GetRecord( nPos );
        xRec->ConvertXFIndexes();

        
        if( nPos > 0 )
        {
            XclExpColinfoRef xPrevRec = maColInfos.GetRecord( nPos - 1 );
            if( xPrevRec->TryMerge( *xRec ) )
                
                maColInfos.RemoveRecord( nPos-- );
        }
    }

    
    typedef ::std::map< sal_uInt16, sal_uInt16 > XclExpWidthMap;
    XclExpWidthMap aWidthMap;
    sal_uInt16 nMaxColCount = 0;
    sal_uInt16 nMaxUsedWidth = 0;
    for( nPos = 0, nSize = maColInfos.GetSize(); nPos < nSize; ++nPos )
    {
        XclExpColinfoRef xRec = maColInfos.GetRecord( nPos );
        sal_uInt16 nColCount = xRec->GetColCount();

        
        rXFIndexes.resize( rXFIndexes.size() + nColCount, xRec->GetXFIndex() );

        
        sal_uInt16 nWidth = xRec->GetColWidth();
        sal_uInt16& rnMapCount = aWidthMap[ nWidth ];
        rnMapCount = rnMapCount + nColCount;
        if( rnMapCount > nMaxColCount )
        {
            nMaxColCount = rnMapCount;
            nMaxUsedWidth = nWidth;
        }
    }
    maDefcolwidth.SetDefWidth( nMaxUsedWidth );

    
    nPos = 0;
    while( nPos < maColInfos.GetSize() )
    {
        XclExpColinfoRef xRec = maColInfos.GetRecord( nPos );
        if( xRec->IsDefault( maDefcolwidth ) )
            maColInfos.RemoveRecord( nPos );
        else
            ++nPos;
    }
}

void XclExpColinfoBuffer::Save( XclExpStream& rStrm )
{
    
    maDefcolwidth.Save( rStrm );
    
    maColInfos.Save( rStrm );
}

void XclExpColinfoBuffer::SaveXml( XclExpXmlStream& rStrm )
{
    if( maColInfos.IsEmpty() )
        return;

    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_cols,
            FSEND );
    maColInfos.SaveXml( rStrm );
    rWorksheet->endElement( XML_cols );
}



XclExpDefaultRowData::XclExpDefaultRowData() :
    mnFlags( EXC_DEFROW_DEFAULTFLAGS ),
    mnHeight( EXC_DEFROW_DEFAULTHEIGHT )
{
}

XclExpDefaultRowData::XclExpDefaultRowData( const XclExpRow& rRow ) :
    mnFlags( EXC_DEFROW_DEFAULTFLAGS ),
    mnHeight( rRow.GetHeight() )
{
    ::set_flag( mnFlags, EXC_DEFROW_HIDDEN, rRow.IsHidden() );
    ::set_flag( mnFlags, EXC_DEFROW_UNSYNCED, rRow.IsUnsynced() );
}

bool operator<( const XclExpDefaultRowData& rLeft, const XclExpDefaultRowData& rRight )
{
    return (rLeft.mnHeight < rRight.mnHeight) ||
        ((rLeft.mnHeight == rRight.mnHeight) && (rLeft.mnFlags < rRight.mnFlags));
}



XclExpDefrowheight::XclExpDefrowheight() :
    XclExpRecord( EXC_ID3_DEFROWHEIGHT, 4 )
{
}

void XclExpDefrowheight::SetDefaultData( const XclExpDefaultRowData& rDefData )
{
    maDefData = rDefData;
}

void XclExpDefrowheight::WriteBody( XclExpStream& rStrm )
{
    OSL_ENSURE_BIFF( rStrm.GetRoot().GetBiff() >= EXC_BIFF3 );
    rStrm << maDefData.mnFlags << maDefData.mnHeight;
}



XclExpRow::XclExpRow( const XclExpRoot& rRoot, sal_uInt32 nXclRow,
        XclExpRowOutlineBuffer& rOutlineBfr, bool bAlwaysEmpty ) :
    XclExpRecord( EXC_ID3_ROW, 16 ),
    XclExpRoot( rRoot ),
    mnXclRow( nXclRow ),
    mnHeight( 0 ),
    mnFlags( EXC_ROW_DEFAULTFLAGS ),
    mnXFIndex( EXC_XF_DEFAULTCELL ),
    mnOutlineLevel( 0 ),
    mnXclRowRpt( 1 ),
    mnCurrentRow( nXclRow ),
    mbAlwaysEmpty( bAlwaysEmpty ),
    mbEnabled( true )
{
    SCTAB nScTab = GetCurrScTab();
    SCROW nScRow = static_cast< SCROW >( mnXclRow );

    

    sal_uInt8 nRowFlags = GetDoc().GetRowFlags( nScRow, nScTab );
    bool bUserHeight = ::get_flag< sal_uInt8 >( nRowFlags, CR_MANUALSIZE );
    bool bHidden = GetDoc().RowHidden(nScRow, nScTab);
    ::set_flag( mnFlags, EXC_ROW_UNSYNCED, bUserHeight );
    ::set_flag( mnFlags, EXC_ROW_HIDDEN, bHidden );

    

    
    

    mnHeight = GetDoc().GetRowHeight(nScRow, nScTab, false);

    

    rOutlineBfr.Update( nScRow );
    ::set_flag( mnFlags, EXC_ROW_COLLAPSED, rOutlineBfr.IsCollapsed() );
    ::insert_value( mnFlags, rOutlineBfr.GetLevel(), 0, 3 );
    mnOutlineLevel = rOutlineBfr.GetLevel();

    

    XclExpProgressBar& rProgress = GetProgressBar();
    rProgress.IncRowRecordCount();
    rProgress.Progress();
}

void XclExpRow::AppendCell( XclExpCellRef xCell, bool bIsMergedBase )
{
    OSL_ENSURE( !mbAlwaysEmpty, "XclExpRow::AppendCell - row is marked to be always empty" );
    
    InsertCell( xCell, maCellList.GetSize(), bIsMergedBase );
}

void XclExpRow::Finalize( const ScfUInt16Vec& rColXFIndexes )
{
    size_t nPos, nSize;

    

    
    size_t nColCount = GetMaxPos().Col() + 1;
    OSL_ENSURE( rColXFIndexes.size() == nColCount, "XclExpRow::Finalize - wrong column XF index count" );

    ScfUInt16Vec aXFIndexes( nColCount, EXC_XF_NOTFOUND );
    for( nPos = 0, nSize = maCellList.GetSize(); nPos < nSize; ++nPos )
    {
        XclExpCellRef xCell = maCellList.GetRecord( nPos );
        xCell->ConvertXFIndexes( GetRoot() );
        xCell->GetBlankXFIndexes( aXFIndexes );
    }

    

    /*  This is needed because nonexistant cells in Calc are not formatted at all,
        but in Excel they would have the column default format. Blank cells that
        are equal to the respective column default are removed later in this function. */
    if( !mbAlwaysEmpty )
    {
        
        XclExpMultiXFId aXFId( XclExpXFBuffer::GetDefCellXFId() );
        aXFId.ConvertXFIndex( GetRoot() );

        nPos = 0;
        while( nPos <= maCellList.GetSize() )  
        {
            
            sal_uInt16 nFirstFreeXclCol = (nPos > 0) ? (maCellList.GetRecord( nPos - 1 )->GetLastXclCol() + 1) : 0;
            
            sal_uInt16 nNextUsedXclCol = (nPos < maCellList.GetSize()) ? maCellList.GetRecord( nPos )->GetXclCol() : (GetMaxPos().Col() + 1);

            
            if( nFirstFreeXclCol < nNextUsedXclCol )
            {
                aXFId.mnCount = nNextUsedXclCol - nFirstFreeXclCol;
                XclExpCellRef xNewCell( new XclExpBlankCell( XclAddress( nFirstFreeXclCol, mnXclRow ), aXFId ) );
                
                InsertCell( xNewCell, nPos, false );
                
                ::std::fill( aXFIndexes.begin() + nFirstFreeXclCol,
                    aXFIndexes.begin() + nNextUsedXclCol, aXFId.mnXFIndex );
                
            }
            else
                ++nPos;
        }
    }

    

    ScfUInt16Vec::iterator aCellBeg = aXFIndexes.begin(), aCellEnd = aXFIndexes.end(), aCellIt;
    ScfUInt16Vec::const_iterator aColBeg = rColXFIndexes.begin(), aColIt;

    
    typedef ::std::map< sal_uInt16, size_t > XclExpXFIndexMap;
    XclExpXFIndexMap aIndexMap;
    sal_uInt16 nRowXFIndex = EXC_XF_DEFAULTCELL;
    size_t nMaxXFCount = 0;
    for( aCellIt = aCellBeg; aCellIt != aCellEnd; ++aCellIt )
    {
        if( *aCellIt != EXC_XF_NOTFOUND )
        {
            size_t& rnCount = aIndexMap[ *aCellIt ];
            ++rnCount;
            if( rnCount > nMaxXFCount )
            {
                nRowXFIndex = *aCellIt;
                nMaxXFCount = rnCount;
            }
        }
    }

    
    bool bUseColDefXFs = nRowXFIndex == EXC_XF_DEFAULTCELL;
    if( !bUseColDefXFs )
    {
        
        size_t nXFCountWithRowDefXF = 0;
        size_t nXFCountWithoutRowDefXF = 0;
        for( aCellIt = aCellBeg, aColIt = aColBeg; aCellIt != aCellEnd; ++aCellIt, ++aColIt )
        {
            sal_uInt16 nXFIndex = *aCellIt;
            if( nXFIndex != EXC_XF_NOTFOUND )
            {
                if( nXFIndex != nRowXFIndex )
                    ++nXFCountWithRowDefXF;     
                if( nXFIndex != *aColIt )
                    ++nXFCountWithoutRowDefXF;  
            }
        }

        
        bUseColDefXFs = nXFCountWithoutRowDefXF <= nXFCountWithRowDefXF;
    }

    

    if( bUseColDefXFs )
    {
        
        
        for( aCellIt = aCellBeg, aColIt = aColBeg; aCellIt != aCellEnd; ++aCellIt, ++aColIt )
            if( *aCellIt == *aColIt )
                *aCellIt = EXC_XF_NOTFOUND;
    }
    else
    {
        
        mnXFIndex = nRowXFIndex;
        ::set_flag( mnFlags, EXC_ROW_USEDEFXF );
        
        for( aCellIt = aCellBeg; aCellIt != aCellEnd; ++aCellIt )
            if( *aCellIt == nRowXFIndex )
                *aCellIt = EXC_XF_NOTFOUND;
    }

    
    nPos = 0;
    while( nPos < maCellList.GetSize() )   
    {
        XclExpCellRef xCell = maCellList.GetRecord( nPos );
        xCell->RemoveUnusedBlankCells( aXFIndexes );
        if( xCell->IsEmpty() )
            maCellList.RemoveRecord( nPos );
        else
            ++nPos;
    }

    
    GetProgressBar().Progress();
}

sal_uInt16 XclExpRow::GetFirstUsedXclCol() const
{
    return maCellList.IsEmpty() ? 0 : maCellList.GetFirstRecord()->GetXclCol();
}

sal_uInt16 XclExpRow::GetFirstFreeXclCol() const
{
    return maCellList.IsEmpty() ? 0 : (maCellList.GetLastRecord()->GetLastXclCol() + 1);
}

bool XclExpRow::IsDefaultable() const
{
    const sal_uInt16 nAllowedFlags = EXC_ROW_DEFAULTFLAGS | EXC_ROW_HIDDEN | EXC_ROW_UNSYNCED;
    return !::get_flag( mnFlags, static_cast< sal_uInt16 >( ~nAllowedFlags ) ) && IsEmpty();
}

void XclExpRow::DisableIfDefault( const XclExpDefaultRowData& rDefRowData )
{
    mbEnabled = !IsDefaultable() ||
        (mnHeight != rDefRowData.mnHeight) ||
        (IsHidden() != rDefRowData.IsHidden()) ||
        (IsUnsynced() != rDefRowData.IsUnsynced());
}

void XclExpRow::WriteCellList( XclExpStream& rStrm )
{
    OSL_ENSURE( mbEnabled || maCellList.IsEmpty(), "XclExpRow::WriteCellList - cells in disabled row" );
    maCellList.Save( rStrm );
}

void XclExpRow::Save( XclExpStream& rStrm )
{
    if( mbEnabled )
    {
        mnCurrentRow = mnXclRow;
        for ( sal_uInt32 i = 0; i < mnXclRowRpt; ++i, ++mnCurrentRow )
            XclExpRecord::Save( rStrm );
    }
}

void XclExpRow::InsertCell( XclExpCellRef xCell, size_t nPos, bool bIsMergedBase )
{
    OSL_ENSURE( xCell, "XclExpRow::InsertCell - missing cell" );

    /*  If we have a multi-line text in a merged cell, and the resulting
        row height has not been confirmed, we need to force the EXC_ROW_UNSYNCED
        flag to be true to ensure Excel works correctly. */
    if( bIsMergedBase && xCell->IsMultiLineText() )
        ::set_flag( mnFlags, EXC_ROW_UNSYNCED );

    
    XclExpCellRef xPrevCell = maCellList.GetRecord( nPos - 1 );
    if( xPrevCell && xPrevCell->TryMerge( *xCell ) )
        xCell = xPrevCell;
    else
        maCellList.InsertRecord( xCell, nPos++ );
    

    
    XclExpCellRef xNextCell = maCellList.GetRecord( nPos );
    if( xNextCell && xCell->TryMerge( *xNextCell ) )
        maCellList.RemoveRecord( nPos );
}

void XclExpRow::WriteBody( XclExpStream& rStrm )
{
    rStrm   << static_cast< sal_uInt16 >(mnCurrentRow)
            << GetFirstUsedXclCol()
            << GetFirstFreeXclCol()
            << mnHeight
            << sal_uInt32( 0 )
            << mnFlags
            << mnXFIndex;
}

void XclExpRow::SaveXml( XclExpXmlStream& rStrm )
{
    if( !mbEnabled )
        return;
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    bool haveFormat = ::get_flag( mnFlags, EXC_ROW_USEDEFXF );
    mnCurrentRow = mnXclRow + 1;
    for ( sal_uInt32 i=0; i<mnXclRowRpt; ++i )
    {
        rWorksheet->startElement( XML_row,
                XML_r,              OString::number(  (mnCurrentRow++) ).getStr(),
                
                XML_s,              haveFormat ? lcl_GetStyleId( rStrm, mnXFIndex ).getStr() : NULL,
                XML_customFormat,   XclXmlUtils::ToPsz( haveFormat ),
                XML_ht,             OString::number( (double) mnHeight / 20.0 ).getStr(),
                XML_hidden,         XclXmlUtils::ToPsz( ::get_flag( mnFlags, EXC_ROW_HIDDEN ) ),
                XML_customHeight,   XclXmlUtils::ToPsz( ::get_flag( mnFlags, EXC_ROW_UNSYNCED ) ),
                XML_outlineLevel,   OString::number(  mnOutlineLevel ).getStr(),
                XML_collapsed,      XclXmlUtils::ToPsz( ::get_flag( mnFlags, EXC_ROW_COLLAPSED ) ),
                
                
                
                FSEND );
        
        maCellList.SaveXml( rStrm );
        rWorksheet->endElement( XML_row );
    }
}



XclExpRowBuffer::XclExpRowBuffer( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    maOutlineBfr( rRoot ),
    maDimensions( rRoot )
{
}

void XclExpRowBuffer::AppendCell( XclExpCellRef xCell, bool bIsMergedBase )
{
    OSL_ENSURE( xCell, "XclExpRowBuffer::AppendCell - missing cell" );
    GetOrCreateRow( xCell->GetXclRow(), false ).AppendCell( xCell, bIsMergedBase );
}

void XclExpRowBuffer::CreateRows( SCROW nFirstFreeScRow )
{
    if( nFirstFreeScRow > 0 )
        GetOrCreateRow(  ::std::max ( nFirstFreeScRow - 1, GetMaxPos().Row() ), true );
}

void XclExpRowBuffer::Finalize( XclExpDefaultRowData& rDefRowData, const ScfUInt16Vec& rColXFIndexes )
{
    

    GetProgressBar().ActivateFinalRowsSegment();

    RowMap::iterator itr, itrBeg = maRowMap.begin(), itrEnd = maRowMap.end();
    for (itr = itrBeg; itr != itrEnd; ++itr)
        itr->second->Finalize(rColXFIndexes);

    

    typedef ::std::map< XclExpDefaultRowData, size_t > XclExpDefRowDataMap;
    XclExpDefRowDataMap aDefRowMap;

    XclExpDefaultRowData aMaxDefData;
    size_t nMaxDefCount = 0;
    
    XclExpRow* pPrev = NULL;
    typedef std::vector< XclExpRow* > XclRepeatedRows;
    XclRepeatedRows aRepeated;
    for (itr = itrBeg; itr != itrEnd; ++itr)
    {
        const RowRef& rRow = itr->second;
        if (rRow->IsDefaultable())
        {
            XclExpDefaultRowData aDefData( *rRow );
            size_t& rnDefCount = aDefRowMap[ aDefData ];
            ++rnDefCount;
            if( rnDefCount > nMaxDefCount )
            {
                nMaxDefCount = rnDefCount;
                aMaxDefData = aDefData;
            }
        }
        if ( pPrev )
        {
            if ( pPrev->IsDefaultable())
            {
                
                
                
                sal_uInt32 nRpt =  rRow->GetXclRow() - pPrev->GetXclRow();
                if ( nRpt > 1 )
                    aRepeated.push_back( pPrev );
                pPrev->SetXclRowRpt( nRpt );
                XclExpDefaultRowData aDefData( *pPrev );
                size_t& rnDefCount = aDefRowMap[ aDefData ];
                rnDefCount += ( pPrev->GetXclRowRpt() - 1 );
                if( rnDefCount > nMaxDefCount )
                {
                    nMaxDefCount = rnDefCount;
                    aMaxDefData = aDefData;
                }
            }
        }
        pPrev = rRow.get();
    }
    
    rDefRowData = aMaxDefData;

    
    
    for ( XclRepeatedRows::iterator it = aRepeated.begin(), it_end = aRepeated.end(); it != it_end; ++it)
    {
        if ( (*it)->GetXclRowRpt() > 1 && (*it)->GetHeight() == rDefRowData.mnHeight )
            (*it)->SetXclRowRpt( 1 );
    }

    

    sal_uInt16 nFirstUsedXclCol = SAL_MAX_UINT16;
    sal_uInt16 nFirstFreeXclCol = 0;
    sal_uInt32 nFirstUsedXclRow = SAL_MAX_UINT32;
    sal_uInt32 nFirstFreeXclRow = 0;

    for (itr = itrBeg; itr != itrEnd; ++itr)
    {
        const RowRef& rRow = itr->second;
        
        rRow->DisableIfDefault( aMaxDefData );

        
        if( !rRow->IsEmpty() )      
        {
            nFirstUsedXclCol = ::std::min( nFirstUsedXclCol, rRow->GetFirstUsedXclCol() );
            nFirstFreeXclCol = ::std::max( nFirstFreeXclCol, rRow->GetFirstFreeXclCol() );
        }

        
        if( rRow->IsEnabled() )
        {
            sal_uInt16 nXclRow = rRow->GetXclRow();
            nFirstUsedXclRow = ::std::min< sal_uInt32 >( nFirstUsedXclRow, nXclRow );
            nFirstFreeXclRow = ::std::max< sal_uInt32 >( nFirstFreeXclRow, nXclRow + 1 );
        }
    }

    
    nFirstUsedXclCol = ::std::min( nFirstUsedXclCol, nFirstFreeXclCol );
    nFirstUsedXclRow = ::std::min( nFirstUsedXclRow, nFirstFreeXclRow );

    
    maDimensions.SetDimensions(
        nFirstUsedXclCol, nFirstUsedXclRow, nFirstFreeXclCol, nFirstFreeXclRow );
}

void XclExpRowBuffer::Save( XclExpStream& rStrm )
{
    
    maDimensions.Save( rStrm );

    
    size_t nSize = maRowMap.size();
    RowMap::iterator itr, itrBeg = maRowMap.begin(), itrEnd = maRowMap.end();
    RowMap::iterator itrBlkStart = maRowMap.begin(), itrBlkEnd = maRowMap.begin();
    sal_uInt16 nStartXclRow = (nSize == 0) ? 0 : itrBeg->second->GetXclRow();


    for (itr = itrBeg; itr != itrEnd; ++itr)
    {
        
        while( (itrBlkEnd != itrEnd) && (itrBlkEnd->second->GetXclRow() - nStartXclRow < EXC_ROW_ROWBLOCKSIZE) )
            ++itrBlkEnd;

        
        RowMap::iterator itRow;
        for( itRow = itrBlkStart; itRow != itrBlkEnd; ++itRow )
            itRow->second->Save( rStrm );

        
        for( itRow = itrBlkStart; itRow != itrBlkEnd; ++itRow )
             itRow->second->WriteCellList( rStrm );

        itrBlkStart = (itrBlkEnd == itrEnd) ? itrBlkEnd : itrBlkEnd++;
        nStartXclRow += EXC_ROW_ROWBLOCKSIZE;
    }
}

void XclExpRowBuffer::SaveXml( XclExpXmlStream& rStrm )
{
    sal_Int32 nNonEmpty = 0;
    RowMap::iterator itr = maRowMap.begin(), itrEnd = maRowMap.end();
    for (; itr != itrEnd; ++itr)
        if (itr->second->IsEnabled())
            ++nNonEmpty;

    if (nNonEmpty == 0)
    {
        rStrm.GetCurrentStream()->singleElement( XML_sheetData, FSEND );
        return;
    }

    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_sheetData, FSEND );
    for (itr = maRowMap.begin(); itr != itrEnd; ++itr)
        itr->second->SaveXml(rStrm);
    rWorksheet->endElement( XML_sheetData );
}

XclExpDimensions* XclExpRowBuffer::GetDimensions()
{
    return &maDimensions;
}

XclExpRow& XclExpRowBuffer::GetOrCreateRow( sal_uInt32 nXclRow, bool bRowAlwaysEmpty )
{
    RowMap::iterator itr = maRowMap.begin();
    ScDocument& rDoc = GetRoot().GetDoc();
    SCTAB nScTab = GetRoot().GetCurrScTab();
    for ( size_t nFrom = maRowMap.size(); nFrom <= nXclRow; ++nFrom )
    {
        itr = maRowMap.find(nFrom);
        if ( itr == maRowMap.end() )
        {
            
            
            if ( !nFrom || ( nFrom == nXclRow ) || ( nFrom && ( rDoc.GetRowHeight(nFrom, nScTab, false) != rDoc.GetRowHeight(nFrom-1, nScTab, false) ) ) )
            {
                RowRef p(new XclExpRow(GetRoot(), nFrom, maOutlineBfr, bRowAlwaysEmpty));
                maRowMap.insert(RowMap::value_type(nFrom, p));
            }
        }
    }
    itr = maRowMap.find(nXclRow);
    return *itr->second;

}





XclExpCellTable::XclExpCellTable( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    maColInfoBfr( rRoot ),
    maRowBfr( rRoot ),
    maArrayBfr( rRoot ),
    maShrfmlaBfr( rRoot ),
    maTableopBfr( rRoot ),
    mxDefrowheight( new XclExpDefrowheight ),
    mxGuts( new XclExpGuts( rRoot ) ),
    mxNoteList( new XclExpNoteList ),
    mxMergedcells( new XclExpMergedcells( rRoot ) ),
    mxHyperlinkList( new XclExpHyperlinkList ),
    mxDval( new XclExpDval( rRoot ) ),
    mxExtLst( new XclExtLst( rRoot ) )
{
    ScDocument& rDoc = GetDoc();
    SCTAB nScTab = GetCurrScTab();
    SvNumberFormatter& rFormatter = GetFormatter();

    
    SCCOL nMaxScCol = GetMaxPos().Col();
    SCROW nMaxScRow = GetMaxPos().Row();

    
    SCCOL nLastUsedScCol;
    SCROW nLastUsedScRow;
    rDoc.GetTableArea( nScTab, nLastUsedScCol, nLastUsedScRow );

    if(nLastUsedScCol > nMaxScCol)
        nLastUsedScCol = nMaxScCol;

    if(nLastUsedScRow > nMaxScRow)
        nLastUsedScRow = nMaxScRow;

    ScRange aUsedRange( 0, 0, nScTab, nLastUsedScCol, nLastUsedScRow, nScTab );
    GetAddressConverter().ValidateRange( aUsedRange, true );
    nLastUsedScCol = aUsedRange.aEnd.Col();
    nLastUsedScRow = aUsedRange.aEnd.Row();

    
    SCROW nFirstUnflaggedScRow = rDoc.GetLastFlaggedRow( nScTab ) + 1;

    
    SCROW nFirstUngroupedScRow = 0;
    if( const ScOutlineTable* pOutlineTable = rDoc.GetOutlineTable( nScTab ) )
    {
        SCCOLROW nScStartPos, nScEndPos;
        if( const ScOutlineArray* pRowArray = pOutlineTable->GetRowArray() )
        {
            pRowArray->GetRange( nScStartPos, nScEndPos );
            
            nFirstUngroupedScRow = static_cast< SCROW >( nScEndPos + 2 );
        }
    }

    
    /*  #i30411# Files saved with SO7/OOo1.x with nonstandard default column
        formatting cause big Excel files, because all rows from row 1 to row
        32000 are exported. Now, if the used area goes exactly to row 32000,
        use this row as default and ignore all rows >32000.
        #i59220# Tolerance of +-128 rows for inserted/removed rows. */
    if( (31871 <= nLastUsedScRow) && (nLastUsedScRow <= 32127) && (nFirstUnflaggedScRow < nLastUsedScRow) && (nFirstUngroupedScRow <= nLastUsedScRow) )
        nMaxScRow = nLastUsedScRow;
    maColInfoBfr.Initialize( nMaxScRow );

    
    SCCOL nLastIterScCol = nMaxScCol;
    SCROW nLastIterScRow = ulimit_cast< SCROW >( nLastUsedScRow, nMaxScRow );
    ScUsedAreaIterator aIt( &rDoc, nScTab, 0, 0, nLastIterScCol, nLastIterScRow );

    
    GetProgressBar().ActivateCreateRowsSegment();

    for( bool bIt = aIt.GetNext(); bIt; bIt = aIt.GetNext() )
    {
        SCCOL nScCol = aIt.GetStartCol();
        SCROW nScRow = aIt.GetRow();
        SCCOL nLastScCol = aIt.GetEndCol();
        ScAddress aScPos( nScCol, nScRow, nScTab );

        XclAddress aXclPos( static_cast< sal_uInt16 >( nScCol ), static_cast< sal_uInt32 >( nScRow ) );
        sal_uInt16 nLastXclCol = static_cast< sal_uInt16 >( nLastScCol );

        const ScRefCellValue& rScCell = aIt.GetCell();
        XclExpCellRef xCell;

        const ScPatternAttr* pPattern = aIt.GetPattern();

        
        sal_uInt32 nMergeBaseXFId = EXC_XFID_NOTFOUND;
        bool bIsMergedBase = false;
        if( pPattern )
        {
            const SfxItemSet& rItemSet = pPattern->GetItemSet();
            
            const ScMergeAttr& rMergeItem = GETITEM( rItemSet, ScMergeAttr, ATTR_MERGE );
            bIsMergedBase = rMergeItem.IsMerged();
            /*  overlapped cell in a merged range; in Excel all merged cells
                must contain same XF index, for correct border */
            const ScMergeFlagAttr& rMergeFlagItem = GETITEM( rItemSet, ScMergeFlagAttr, ATTR_MERGE_FLAG );
            if( rMergeFlagItem.IsOverlapped() )
                nMergeBaseXFId = mxMergedcells->GetBaseXFId( aScPos );
        }

        OUString aAddNoteText;    

        switch (rScCell.meType)
        {
            case CELLTYPE_VALUE:
            {
                double fValue = rScCell.mfValue;

                
                if( pPattern && ((fValue == 0.0) || (fValue == 1.0)) )
                {
                    sal_uLong nScNumFmt = GETITEMVALUE( pPattern->GetItemSet(), SfxUInt32Item, ATTR_VALUE_FORMAT, sal_uLong );
                    if( rFormatter.GetType( nScNumFmt ) == NUMBERFORMAT_LOGICAL )
                        xCell.reset( new XclExpBooleanCell(
                            GetRoot(), aXclPos, pPattern, nMergeBaseXFId, fValue != 0.0 ) );
                }

                
                sal_Int32 nRkValue;
                if( !xCell && XclTools::GetRKFromDouble( nRkValue, fValue ) )
                    xCell.reset( new XclExpRkCell(
                        GetRoot(), aXclPos, pPattern, nMergeBaseXFId, nRkValue ) );

                
                if( !xCell )
                    xCell.reset( new XclExpNumberCell(
                        GetRoot(), aXclPos, pPattern, nMergeBaseXFId, fValue ) );
            }
            break;

            case CELLTYPE_STRING:
            {
                xCell.reset(new XclExpLabelCell(
                    GetRoot(), aXclPos, pPattern, nMergeBaseXFId, rScCell.mpString->getString()));
            }
            break;

            case CELLTYPE_EDIT:
            {
                XclExpHyperlinkHelper aLinkHelper( GetRoot(), aScPos );
                xCell.reset(new XclExpLabelCell(
                    GetRoot(), aXclPos, pPattern, nMergeBaseXFId, rScCell.mpEditText, aLinkHelper));

                
                if( aLinkHelper.HasLinkRecord() )
                    mxHyperlinkList->AppendRecord( aLinkHelper.GetLinkRecord() );
                
                if( aLinkHelper.HasMultipleUrls() )
                    aAddNoteText = ScGlobal::addToken( aAddNoteText, aLinkHelper.GetUrlList(), '\n', 2 );
            }
            break;

            case CELLTYPE_FORMULA:
            {
                xCell.reset(new XclExpFormulaCell(
                    GetRoot(), aXclPos, pPattern, nMergeBaseXFId,
                    *rScCell.mpFormula, maArrayBfr, maShrfmlaBfr, maTableopBfr));
            }
            break;

            default:
                OSL_FAIL( "XclExpCellTable::XclExpCellTable - unknown cell type" );
                
            case CELLTYPE_NONE:
            {
                xCell.reset( new XclExpBlankCell(
                    GetRoot(), aXclPos, nLastXclCol, pPattern, nMergeBaseXFId ) );
            }
            break;
        }

        
        if( xCell )
            maRowBfr.AppendCell( xCell, bIsMergedBase );

        if ( !aAddNoteText.isEmpty()  )
            mxNoteList->AppendNewRecord( new XclExpNote( GetRoot(), aScPos, NULL, aAddNoteText ) );

        
        if( pPattern )
        {
            const SfxItemSet& rItemSet = pPattern->GetItemSet();

            
            if( bIsMergedBase )
            {
                const ScMergeAttr& rMergeItem = GETITEM( rItemSet, ScMergeAttr, ATTR_MERGE );
                ScRange aScRange( aScPos );
                aScRange.aEnd.IncCol( rMergeItem.GetColMerge() - 1 );
                aScRange.aEnd.IncRow( rMergeItem.GetRowMerge() - 1 );
                sal_uInt32 nXFId = xCell ? xCell->GetFirstXFId() : EXC_XFID_NOTFOUND;
                
                OSL_ENSURE( (aScRange.aStart.Col() == aScRange.aEnd.Col()) || (nScCol == nLastScCol),
                    "XclExpCellTable::XclExpCellTable - invalid repeated blank merged cell" );
                for( SCCOL nIndex = nScCol; nIndex <= nLastScCol; ++nIndex )
                {
                    mxMergedcells->AppendRange( aScRange, nXFId );
                    aScRange.aStart.IncCol();
                    aScRange.aEnd.IncCol();
                }
            }

            
            if( ScfTools::CheckItem( rItemSet, ATTR_VALIDDATA, false ) )
            {
                sal_uLong nScHandle = GETITEMVALUE( rItemSet, SfxUInt32Item, ATTR_VALIDDATA, sal_uLong );
                ScRange aScRange( aScPos );
                aScRange.aEnd.SetCol( nLastScCol );
                mxDval->InsertCellRange( aScRange, nScHandle );
            }
        }
    }

    
    maRowBfr.CreateRows( ::std::max( nFirstUnflaggedScRow, nFirstUngroupedScRow ) );
}

void XclExpCellTable::Finalize()
{
    
    maTableopBfr.Finalize();

    /*  Finalize column buffer. This calculates column default XF indexes from
        the XF identifiers and fills a vector with these XF indexes. */
    ScfUInt16Vec aColXFIndexes;
    maColInfoBfr.Finalize( aColXFIndexes );

    /*  Finalize row buffer. This calculates all cell XF indexes from the XF
        identifiers. Then the XF index vector aColXFIndexes (filled above) is
        used to calculate the row default formats. With this, all unneeded blank
        cell records (equal to row default or column default) will be removed.
        The function returns the (most used) default row format in aDefRowData. */
    XclExpDefaultRowData aDefRowData;
    maRowBfr.Finalize( aDefRowData, aColXFIndexes );

    
    mxDefrowheight->SetDefaultData( aDefRowData );
}

XclExpRecordRef XclExpCellTable::CreateRecord( sal_uInt16 nRecId ) const
{
    XclExpRecordRef xRec;
    switch( nRecId )
    {
        case EXC_ID3_DIMENSIONS:    xRec.reset( new XclExpDelegatingRecord( const_cast<XclExpRowBuffer*>(&maRowBfr)->GetDimensions() ) );   break;
        case EXC_ID2_DEFROWHEIGHT:  xRec = mxDefrowheight;  break;
        case EXC_ID_GUTS:           xRec = mxGuts;          break;
        case EXC_ID_NOTE:           xRec = mxNoteList;      break;
        case EXC_ID_MERGEDCELLS:    xRec = mxMergedcells;   break;
        case EXC_ID_HLINK:          xRec = mxHyperlinkList; break;
        case EXC_ID_DVAL:           xRec = mxDval;          break;
        case EXC_ID_EXTLST:         xRec = mxExtLst;        break;
        default:    OSL_FAIL( "XclExpCellTable::CreateRecord - unknown record id" );
    }
    return xRec;
}

void XclExpCellTable::Save( XclExpStream& rStrm )
{
    
    maColInfoBfr.Save( rStrm );
    
    maRowBfr.Save( rStrm );
}

void XclExpCellTable::SaveXml( XclExpXmlStream& rStrm )
{
    
    XclExpDefaultRowData& rDefData = mxDefrowheight->GetDefaultData();
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_sheetFormatPr,
        XML_defaultRowHeight, OString::number( (double) rDefData.mnHeight / 20.0 ).getStr(), FSEND );
    rWorksheet->endElement( XML_sheetFormatPr );

    maColInfoBfr.SaveXml( rStrm );
    maRowBfr.SaveXml( rStrm );
    mxExtLst->SaveXml( rStrm );
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
