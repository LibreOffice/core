/*************************************************************************
 *
 *  $RCSfile: xecontent.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-19 11:35:54 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

// ============================================================================

#ifndef SC_XECONTENT_HXX
#include "xecontent.hxx"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XAREALINKS_HPP_
#include <com/sun/star/sheet/XAreaLinks.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XAREALINK_HPP_
#include <com/sun/star/sheet/XAreaLink.hpp>
#endif

#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif
#define ITEMID_FIELD EE_FEATURE_FIELD
#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_FLDITEM_HXX
#include <svx/flditem.hxx>
#endif

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_VALIDAT_HXX
#include "validat.hxx"
#endif
#ifndef SC_UNONAMES_HXX
#include "unonames.hxx"
#endif
#ifndef SC_CONVUNO_HXX
#include "convuno.hxx"
#endif
#ifndef SC_RANGENAM_HXX
#include "rangenam.hxx"
#endif

#ifndef SC_FAPIHELPER_HXX
#include "fapihelper.hxx"
#endif
#ifndef SC_XLFORMULA_HXX
#include "xlformula.hxx"
#endif

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::table::CellRangeAddress;
using ::com::sun::star::sheet::XAreaLinks;
using ::com::sun::star::sheet::XAreaLink;

#include "excupn.hxx"
#include "excrecds.hxx"


// Shared string table ========================================================

XclExpSst::~XclExpSst()
{
}

sal_uInt32 XclExpSst::Insert( XclExpString* pString )
{
    //! TODO search for the strings to prevent doubles
    maStringList.Append( pString );
    return maStringList.Count() - 1;
}

void XclExpSst::Save( XclExpStream& rStrm )
{
    if( maStringList.Empty() )
        return;

    SvMemoryStream aExtSst( 8192 );

    sal_uInt32 nCount = maStringList.Count();

    sal_uInt32 nBucket = nCount;
    while( nBucket > 0x0100 )
        nBucket /= 2;

    sal_uInt16 nPerBucket = static_cast< sal_uInt16 >( ::std::max( 8UL, nBucket ) );
    sal_uInt16 nBucketIndex = 0;

    // *** write the SST record ***

    rStrm.StartRecord( EXC_ID_SST, 8 );

    rStrm << nCount << nCount;
    for( const XclExpString* pString = maStringList.First(); pString; pString = maStringList.Next() )
    {
        if( !nBucketIndex )
        {
            // write bucket info before string to get correct record position
            sal_uInt32 nStrmPos = rStrm.GetStreamPos();
            sal_uInt16 nRecPos = static_cast< sal_uInt16 >( rStrm.GetRecPos() + 4 );
            aExtSst << nStrmPos             // stream position
                    << nRecPos              // position from start of SST or CONTINUE
                    << sal_uInt16( 0 );     // reserved
        }

        rStrm << *pString;

        if( ++nBucketIndex == nPerBucket )
            nBucketIndex = 0;
    }

    rStrm.EndRecord();

    // *** write the EXTSST record ***

    rStrm.StartRecord( EXC_ID_EXTSST, 0 );

    rStrm << nPerBucket;
    rStrm.SetSliceSize( 8 );    // size of one bucket info
    aExtSst.Seek( STREAM_SEEK_TO_BEGIN );
    rStrm.CopyFromStream( aExtSst );

    rStrm.EndRecord();
}


// Background bitmap ==========================================================

inline XclExpStream& operator<<( XclExpStream& rStrm, const BitmapColor& rBmpColor )
{
    return rStrm << rBmpColor.GetBlue() << rBmpColor.GetGreen() << rBmpColor.GetRed();
}


// ----------------------------------------------------------------------------

XclExpBitmap::XclExpBitmap( const XclExpRoot& rRoot ) :
    XclExpRecord( EXC_ID_BITMAP ),
    mpGraphic( NULL )
{
    SfxStyleSheet* pStyleSh = rRoot.mpRD->pStyleSheet;
    if( pStyleSh )
        mpGraphic = ((const SvxBrushItem&) rRoot.mpRD->pStyleSheetItemSet->Get( ATTR_BACKGROUND )).GetGraphic();
}

void XclExpBitmap::Save( XclExpStream& rStrm )
{
    if( !mpGraphic ) return;

    Bitmap aBmp( mpGraphic->GetBitmap() );
    if( aBmp.GetBitCount() != 24 )
        aBmp.Convert( BMP_CONVERSION_24BIT );

    BitmapReadAccess* pAccess = aBmp.AcquireReadAccess();
    if( !pAccess ) return;

    sal_Int32 nWidth = ::std::min( pAccess->Width(), 0xFFFFL );
    sal_Int32 nHeight = ::std::min( pAccess->Height(), 0xFFFFL );
    if( (nWidth > 0) && (nHeight > 0) )
    {
        rStrm.StartRecord( EXC_ID_BITMAP, 0 );
        rStrm.SetMaxRecSize( EXC_BITMAP_MAXREC );
        rStrm.SetMaxContSize( EXC_BITMAP_MAXCONT );

        sal_uInt8 nPadding = static_cast< sal_uInt8 >( nWidth & 0x03 );
        sal_uInt32 nTmpSize = (nWidth * 3UL + nPadding) * nHeight + 12;

        rStrm   << EXC_BITMAP_UNKNOWNID
                << nTmpSize                             // size after _this_ field
                << sal_uInt32( 12 )                     // unknown
                << static_cast< sal_uInt16 >( nWidth )  // width
                << static_cast< sal_uInt16 >( nHeight ) // height
                << sal_uInt16( 1 )                      // planes
                << sal_uInt16( 24 );                    // bits per pixel

        for( sal_Int32 nY = nHeight - 1; nY >= 0; --nY )
        {
            for( sal_Int32 nX = 0; nX < nWidth; ++nX )
                rStrm << pAccess->GetPixel( nY, nX );
            rStrm.WriteZeroBytes( nPadding );
        }

        rStrm.EndRecord();
    }
    aBmp.ReleaseAccess( pAccess );
}


// Hyperlinks =================================================================

XclExpHyperlink::XclExpHyperlink( const XclExpRoot& rRoot, const SvxURLField& rUrlField ) :
    XclExpRecord( EXC_ID_HLINK ),
    mpVarData( new SvMemoryStream ),
    mnFlags( 0 )
{
    const String& rUrl = rUrlField.GetURL();
    const String& rRepr = rUrlField.GetRepresentation();
    INetURLObject aUrlObj( rUrl );
    const INetProtocol eProtocol = aUrlObj.GetProtocol();
    bool bWithRepr = rRepr.Len() > 0;
    XclExpStream aXclStrm( *mpVarData, rRoot );         // using in raw write mode.
    ::std::auto_ptr< XclExpString > pTextMark;

    // description
    if( bWithRepr )
    {
        XclExpString aDescr( rRepr, EXC_STR_FORCEUNICODE, 255 );
        aXclStrm << sal_uInt32( aDescr.Len() + 1 );     // string length + 1 trailing zero word
        aDescr.WriteBuffer( aXclStrm );                 // NO flags
        aXclStrm << sal_uInt16( 0 );

        mnFlags |= EXC_HLINK_DESCR;
        mpRepr.reset( new String( rRepr ) );
    }

    // file link or URL
    if( eProtocol == INET_PROT_FILE )
    {
        String aFileName;
        sal_uInt16 nLevel;
        bool bRel;
        BuildFileName( aFileName, nLevel, bRel, aUrlObj, rRoot );

        if( !bRel )
            mnFlags |= EXC_HLINK_ABS;
        mnFlags |= EXC_HLINK_BODY;

        ByteString aAsciiLink( aFileName, rRoot.GetCharSet() );
        XclExpString aLink( aFileName, EXC_STR_FORCEUNICODE, 255 );
        aXclStrm    << XclTools::maGuidFileMoniker
                    << nLevel
                    << sal_uInt32( aAsciiLink.Len() + 1 );      // string length + 1 trailing zero byte
        aXclStrm.Write( aAsciiLink.GetBuffer(), aAsciiLink.Len() );
        aXclStrm    << sal_uInt8( 0 )
                    << sal_uInt32( 0xDEADFFFF );
        aXclStrm.WriteZeroBytes( 20 );
        aXclStrm    << sal_uInt32( aLink.GetBufferSize() + 6 )
                    << sal_uInt32( aLink.GetBufferSize() )      // byte count, not string length
                    << sal_uInt16( 0x0003 );
        aLink.WriteBuffer( aXclStrm );                          // NO flags

        if( !mpRepr.get() )
            mpRepr.reset( new String( aFileName ) );
    }
    else if( eProtocol != INET_PROT_NOT_VALID )
    {
        XclExpString aUrl( aUrlObj.GetURLNoMark(), EXC_STR_FORCEUNICODE, 255 );
        aXclStrm    << XclTools::maGuidUrlMoniker
                    << sal_uInt32( aUrl.GetBufferSize() + 2 );  // byte count + 1 trailing zero word
        aUrl.WriteBuffer( aXclStrm );                           // NO flags
        aXclStrm    << sal_uInt16( 0 );

        mnFlags |= EXC_HLINK_BODY | EXC_HLINK_ABS;
        if( !mpRepr.get() )
            mpRepr.reset( new String( rUrl ) );
    }
    else if( rUrl.GetChar( 0 ) == '#' )     // hack for #89066#
    {
        String aTextMark( rUrl.Copy( 1 ) );
        aTextMark.SearchAndReplace( '.', '!' );
        pTextMark.reset( new XclExpString( aTextMark, EXC_STR_FORCEUNICODE, 255 ) );
    }

    // text mark
    if( !pTextMark.get() && aUrlObj.HasMark() )
        pTextMark.reset( new XclExpString( aUrlObj.GetMark(), EXC_STR_FORCEUNICODE, 255 ) );

    if( pTextMark.get() )
    {
        aXclStrm    << sal_uInt32( pTextMark->Len() + 1 );  // string length + 1 trailing zero word
        pTextMark->WriteBuffer( aXclStrm );                 // NO flags
        aXclStrm    << sal_uInt16( 0 );

        mnFlags |= EXC_HLINK_MARK;
    }

    SetRecSize( 32 + mpVarData->Tell() );
}

XclExpHyperlink::~XclExpHyperlink()
{
}

void XclExpHyperlink::BuildFileName(
        String& rName, sal_uInt16& rnLevel, bool& rbRel,
        const INetURLObject& rUrlObj, const XclExpRoot& rRoot ) const
{
    rName = rUrlObj.getFSysPath( INetURLObject::FSYS_DOS );
    rnLevel = 0;
    rbRel = rRoot.IsRelUrl();

    if( rbRel )
    {
        String aTmpName( rName );
        rName = rUrlObj.GetRelURL( rRoot.GetBasePath(), rName,
            INetURLObject::WAS_ENCODED, INetURLObject::DECODE_WITH_CHARSET );

        if( rName.SearchAscii( INET_FILE_SCHEME ) == 0 )    // not converted to rel -> make abs
        {
            rName = aTmpName;
            rbRel = false;
        }
        else if( rName.SearchAscii( "./" ) == 0 )
            rName.Erase( 0, 2 );
        else
        {
            while( rName.SearchAndReplaceAscii( "../", EMPTY_STRING ) != STRING_NOTFOUND )
                ++rnLevel;
        }
    }
}

void XclExpHyperlink::WriteBody( XclExpStream& rStrm )
{
    sal_uInt16 nCol = static_cast< sal_uInt16 >( maPos.Col() );
    sal_uInt16 nRow = static_cast< sal_uInt16 >( maPos.Row() );
    mpVarData->Seek( STREAM_SEEK_TO_BEGIN );

    rStrm   << nRow << nRow << nCol << nCol
            << XclTools::maGuidStdLink
            << sal_uInt32( 2 )
            << mnFlags;
    rStrm.CopyFromStream( *mpVarData );
}


// Label ranges ===============================================================

XclExpLabelranges::XclExpLabelranges( const XclExpRoot& rRoot ) :
    XclExpRecord( EXC_ID_LABELRANGES )
{
    sal_uInt16 nTab = rRoot.GetScTab();

    // row label ranges
    FillRangeList( maRowRanges, rRoot.GetDoc().GetRowNameRangesRef(), nTab );
    // row labels only over 1 column (restriction of Excel97/2000/XP)
    for( ScRange* pRange = maRowRanges.First(); pRange; pRange = maRowRanges.Next() )
        if( pRange->aStart.Col() != pRange->aEnd.Col() )
            pRange->aEnd.SetCol( pRange->aStart.Col() );
    rRoot.CheckCellRangeList( maRowRanges );

    // col label ranges
    FillRangeList( maColRanges, rRoot.GetDoc().GetColNameRangesRef(), nTab );
    rRoot.CheckCellRangeList( maColRanges );

    SetRecSize( 4 + 8 * (maRowRanges.Count() + maColRanges.Count()) );
}

void XclExpLabelranges::FillRangeList( ScRangeList& rRanges, ScRangePairListRef xLabelRangesRef, sal_uInt16 nTab )
{
    for( const ScRangePair* pRangePair = xLabelRangesRef->First(); pRangePair; pRangePair = xLabelRangesRef->Next() )
    {
        const ScRange& rRange = pRangePair->GetRange( 0 );
        if( rRange.aStart.Tab() == nTab )
            rRanges.Append( rRange );
    }
}

void XclExpLabelranges::Save( XclExpStream& rStrm )
{
    if( maRowRanges.Count() || maColRanges.Count() )
        XclExpRecord::Save( rStrm );
}

void XclExpLabelranges::WriteBody( XclExpStream& rStrm )
{
    rStrm << maRowRanges << maColRanges;
}


// Validation =================================================================

/** Writes a formula for the DV record. */
void lcl_xecontent_WriteDvFormula( XclExpStream& rStrm, const ExcUPN* pXclTokArr )
{
    sal_uInt16 nSize = pXclTokArr ? pXclTokArr->GetLen() : 0;
    const sal_Char* pData = pXclTokArr ? pXclTokArr->GetData() : NULL;

    rStrm << nSize << sal_uInt16( 0 );
    rStrm.Write( pData, nSize );
}

/** Writes a formula for the DV record, based on a single string. */
void lcl_xecontent_WriteDvFormula( XclExpStream& rStrm, const XclExpString& rString )
{
    // fake a formula with a single tStr token
    rStrm   << static_cast< sal_uInt16 >( rString.GetSize() + 1 )
            << sal_uInt16( 0 )
            << sal_uInt8( 0x17 )    // TODO: define tStr
            << rString;
}


// ----------------------------------------------------------------------------

XclExpDv::XclExpDv( const XclExpRoot& rRoot, sal_uInt32 nHandle ) :
    XclExpRecord( EXC_ID_DV ),
    XclExpRoot( rRoot ),
    mpValData( rRoot.GetDoc().GetValidationEntry( nHandle ) ),
    mnHandle( nHandle )
{
}

void XclExpDv::InsertCellRange( const ScRange& rRange )
{
    maRanges.Join( rRange );
}

bool XclExpDv::CheckWriteRecord()
{
    CheckCellRangeList( maRanges );
    return mpValData && maRanges.Count();
}

void XclExpDv::WriteBody( XclExpStream& rStrm )
{
    DBG_ASSERT( mpValData, "XclExpDv::WriteBody - missing core data" );
    if( !mpValData ) return;

    // prompt box - empty string represented by single NUL character
    String aTitle, aText;
    bool bShowPrompt = (mpValData->GetInput( aTitle, aText ) == TRUE);
    XclExpString aPromptTitle( aTitle );
    if( !aTitle.Len() )
        aPromptTitle.Assign( '\0' );
    XclExpString aPromptText( aText );
    if( !aText.Len() )
        aPromptText.Assign( '\0' );

    // error box - empty string represented by single NUL character
    ScValidErrorStyle eScErrorStyle;
    bool bShowError = (mpValData->GetErrMsg( aTitle, aText, eScErrorStyle ) == TRUE);
    XclExpString aErrorTitle( aTitle );
    if( !aTitle.Len() )
        aErrorTitle.Assign( '\0' );
    XclExpString aErrorText( aText );
    if( !aText.Len() )
        aErrorText.Assign( '\0' );

    // flags
    sal_uInt32 nFlags = 0;
    switch( mpValData->GetDataMode() )
    {
        case SC_VALID_ANY:      nFlags |= EXC_DV_MODE_ANY;      break;
        case SC_VALID_WHOLE:    nFlags |= EXC_DV_MODE_WHOLE;    break;
        case SC_VALID_DECIMAL:  nFlags |= EXC_DV_MODE_DECIMAL;  break;
        case SC_VALID_LIST:     nFlags |= EXC_DV_MODE_LIST;     break;
        case SC_VALID_DATE:     nFlags |= EXC_DV_MODE_DATE;     break;
        case SC_VALID_TIME:     nFlags |= EXC_DV_MODE_TIME;     break;
        case SC_VALID_TEXTLEN:  nFlags |= EXC_DV_MODE_TEXTLEN;  break;
        case SC_VALID_CUSTOM:   nFlags |= EXC_DV_MODE_CUSTOM;   break;
        default:                DBG_ERRORFILE( "XclExpDv::SaveCont - unknown mode" );
    }
    switch( mpValData->GetOperation() )
    {
        case SC_COND_NONE:
        case SC_COND_EQUAL:     nFlags |= EXC_DV_COND_EQUAL;        break;
        case SC_COND_LESS:      nFlags |= EXC_DV_COND_LESS;         break;
        case SC_COND_GREATER:   nFlags |= EXC_DV_COND_GREATER;      break;
        case SC_COND_EQLESS:    nFlags |= EXC_DV_COND_EQLESS;       break;
        case SC_COND_EQGREATER: nFlags |= EXC_DV_COND_EQGREATER;    break;
        case SC_COND_NOTEQUAL:  nFlags |= EXC_DV_COND_NOTEQUAL;     break;
        case SC_COND_BETWEEN:   nFlags |= EXC_DV_COND_BETWEEN;      break;
        case SC_COND_NOTBETWEEN:nFlags |= EXC_DV_COND_NOTBETWEEN;   break;
        default:                DBG_ERRORFILE( "XclExpDv::SaveCont - unknown condition" );
    }
    switch( eScErrorStyle )
    {
        case SC_VALERR_STOP:    nFlags |= EXC_DV_ERROR_STOP;    break;
        case SC_VALERR_WARNING: nFlags |= EXC_DV_ERROR_WARNING; break;
        case SC_VALERR_INFO:    nFlags |= EXC_DV_ERROR_INFO;    break;
        case SC_VALERR_MACRO:   bShowError = false;             break;
        default:                DBG_ERRORFILE( "XclExpDv::SaveCont - unknown error style" );
    }
    if( mpValData->IsIgnoreBlank() )
        nFlags |= EXC_DV_IGNOREBLANK;
    if( mpValData->GetListType() == ValidListType::INVISIBLE )
        nFlags |= EXC_DV_SUPPRESSDROPDOWN;
    if( bShowPrompt )
        nFlags |= EXC_DV_SHOWPROMPT;
    if( bShowError )
        nFlags |= EXC_DV_SHOWERROR;

    // formulas
    ::std::auto_ptr< ScTokenArray > pScTokArr;
    EC_Codetype eDummy;

    // first formula
    ::std::auto_ptr< ExcUPN > pXclTokArr1;
    ::std::auto_ptr< XclExpString > pXclString;
    pScTokArr.reset( mpValData->CreateTokenArry( 0 ) );
    if( pScTokArr.get() )
    {
        String aString;
        if( (mpValData->GetDataMode() == SC_VALID_LIST) &&
            XclTokenArrayHelper::GetStringList( aString, *pScTokArr, '\n' ) )
        {
            /*  Formula is a list of string tokens -> build the Excel string.
                Data validity is BIFF8 only (important for the XclExpString object).
                Excel uses the NUL character as string list separator. */
            pXclString.reset( new XclExpString( EXC_STR_8BITLENGTH ) );
            xub_StrLen nTokenCnt = aString.GetTokenCount( '\n' );
            xub_StrLen nStringIx = 0;
            for( xub_StrLen nToken = 0; nToken < nTokenCnt; ++nToken )
            {
                String aToken( aString.GetToken( 0, '\n', nStringIx ) );
                if( nToken > 0 )
                    pXclString->Append( '\0' );
                pXclString->Append( aToken );
            }
            nFlags |= EXC_DV_STRINGLIST;
        }
        else
        {
            // no list validation -> convert the formula
            pXclTokArr1.reset( new ExcUPN( mpRD, *pScTokArr, eDummy, NULL, true ) );
        }

        /*  All formulas are stored like conditional formatting formulas (with
            tRefN/tAreaN tokens as value or array class). But NOT the cell references
            and defined names in list validation - they are stored as reference class
            tokens... Example:
            1) Cell must be equal to A1 -> formula is =A1 -> writes tRefNV token
            2) List is taken from A1    -> formula is =A1 -> writes tRefNR token

            Following a VERY dirty hack that looks into the Excel token array. If there
            is a leading tRefN*, tAreaN*, or tName* token, it is converted to reference
            class. This is because the formula compiler is already too obscure, so
            adding this special case will surely break anything else there.
            TODO: Remove this mess when the formula compiler is cleaned up! */
        if( (mpValData->GetDataMode() == SC_VALID_LIST) && pXclTokArr1.get() )
        {
            sal_Char* pData = const_cast< sal_Char* >( pXclTokArr1->GetData() );
            if( pData && pXclTokArr1->GetLen() )
            {
                if( (*pData == 0x43) || (*pData == 0x63) ||     // tNameV, tNameA
                    (*pData == 0x4C) || (*pData == 0x6C) ||     // tRefNV, tRefNA
                    (*pData == 0x4D) || (*pData == 0x6D) )      // tAreaNV, tAreaNA
                    // remove any token class and add reference token class
                    (*pData &= 0x1F) |= 0x20;
            }
        }
    }

    // second formula
    ::std::auto_ptr< ExcUPN > pXclTokArr2;
    pScTokArr.reset( mpValData->CreateTokenArry( 1 ) );
    if( pScTokArr.get() )
    {
        EC_Codetype eDummy;
        pXclTokArr2.reset( new ExcUPN( mpRD, *pScTokArr, eDummy, NULL, true ) );
    }

    // export the record
    rStrm << nFlags << aPromptTitle << aErrorTitle << aPromptText << aErrorText;

    if( pXclString.get() )
        lcl_xecontent_WriteDvFormula( rStrm, *pXclString );
    else
        lcl_xecontent_WriteDvFormula( rStrm, pXclTokArr1.get() );
    lcl_xecontent_WriteDvFormula( rStrm, pXclTokArr2.get() );

    rStrm << maRanges;
}


// ----------------------------------------------------------------------------

XclExpDval::XclExpDval( const XclExpRoot& rRoot ) :
    XclExpRecord( EXC_ID_DVAL, 18 ),
    XclExpRoot( rRoot ),
    mpLastFoundDv( NULL )
{
}

XclExpDv& XclExpDval::SearchOrCreateDv( sal_uInt32 nHandle )
{
    // test last found record
    if( mpLastFoundDv && (mpLastFoundDv->GetHandle() == nHandle) )
        return *mpLastFoundDv;

    // binary search
    sal_uInt32 nCurrIndex = 0;
    if( !maDvList.Empty() )
    {
        sal_uInt32 nFirst = 0;
        sal_uInt32 nLast = maDvList.Count() - 1;
        bool bLoop = true;
        sal_uInt32 nCurrHandle;
        while( (nFirst <= nLast) && bLoop )
        {
            nCurrIndex = (nFirst + nLast) / 2;
            mpLastFoundDv = maDvList.GetObject( nCurrIndex );
            nCurrHandle = mpLastFoundDv->GetHandle();
            if( nCurrHandle == nHandle )
                bLoop = false;
            else if( nCurrHandle < nHandle )
                nFirst = nCurrIndex + 1;
            else if( nCurrIndex )
                nLast = nCurrIndex - 1;
            else    // special case for nLast = -1
                bLoop = false;
        }
        if( nCurrHandle == nHandle )
            return *mpLastFoundDv;
        else if( nCurrHandle < nHandle )
            ++nCurrIndex;
    }

    // create new DV record
    mpLastFoundDv = new XclExpDv( *this, nHandle );
    maDvList.Insert( mpLastFoundDv, nCurrIndex );
    return *mpLastFoundDv;
}

void XclExpDval::InsertCellRange( const ScRange& rRange, sal_uInt32 nHandle )
{
    XclExpDv& rDvRec = SearchOrCreateDv( nHandle );
    rDvRec.InsertCellRange( rRange );
}

void XclExpDval::Save( XclExpStream& rStrm )
{
    // check all records
    sal_uInt32 nIndex = maDvList.Count();
    while( nIndex )
    {
        --nIndex;   // backwards to keep nIndex valid
        XclExpDv* pDvRec = maDvList.GetObject( nIndex );
        if( !pDvRec->CheckWriteRecord() )
            maDvList.Delete( nIndex );
    }

    // write the DVAL and the DV's
    if( !maDvList.Empty() )
    {
        XclExpRecord::Save( rStrm );
        maDvList.Save( rStrm );
    }
}

void XclExpDval::WriteBody( XclExpStream& rStrm )
{
    rStrm.WriteZeroBytes( 10 );
    rStrm << EXC_DVAL_NOOBJ << maDvList.Count();
}


// Web Queries ================================================================

XclExpWebQuery::XclExpWebQuery(
        const String& rRangeName,
        const String& rUrl,
        const String& rSource,
        sal_Int32 nRefrSecs ) :
    maDestRange( rRangeName ),
    maUrl( rUrl ),
    mpQryTables( NULL ),
    // refresh delay time: seconds -> minutes
    mnRefresh( static_cast< sal_Int16 >( ::std::min( (nRefrSecs + 59L) / 60L, 0x7FFFL ) ) ),
    mbEntireDoc( false )
{
    // comma separated list of HTML table names or indexes
    xub_StrLen nTokenCnt = rSource.GetTokenCount( ';' );
    String aNewTables, aAppendTable;
    xub_StrLen nStringIx = 0;
    bool bExitLoop = false;
    for( xub_StrLen nToken = 0; (nToken < nTokenCnt) && !bExitLoop; ++nToken )
    {
        String aToken( rSource.GetToken( 0, ';', nStringIx ) );
        mbEntireDoc = ScfTools::IsHTMLDocName( aToken );
        bExitLoop = mbEntireDoc || ScfTools::IsHTMLTablesName( aToken );
        if( !bExitLoop && ScfTools::GetHTMLNameFromName( aToken, aAppendTable ) )
            ScGlobal::AddToken( aNewTables, aAppendTable, ',' );
    }

    if( !bExitLoop )    // neither HTML_all nor HTML_tables found
    {
        if( aNewTables.Len() )
            mpQryTables = new XclExpString( aNewTables );
        else
            mbEntireDoc = true;
    }
}

XclExpWebQuery::~XclExpWebQuery()
{
    delete mpQryTables;
}

void XclExpWebQuery::Save( XclExpStream& rStrm )
{
    DBG_ASSERT( !mbEntireDoc || !mpQryTables, "XclExpWebQuery::Save - illegal mode" );
    sal_uInt16 nFlags;

    // QSI record
    rStrm.StartRecord( EXC_ID_QSI, 10 + maDestRange.GetSize() );
    rStrm   << EXC_QSI_DEFAULTFLAGS
            << sal_uInt16( 0x0010 )
            << sal_uInt16( 0x0012 )
            << sal_uInt32( 0x00000000 )
            << maDestRange;
    rStrm.EndRecord();

    // PARAMQRY record
    nFlags = EXC_PQRY_DEFAULTFLAGS | (mbEntireDoc ? EXC_PQRY_DOC : EXC_PQRY_TABLES);
    rStrm.StartRecord( EXC_ID_PQRY, 12 );
    rStrm   << nFlags
            << sal_uInt16( 0x0000 )
            << sal_uInt16( 0x0001 );
    rStrm.WriteZeroBytes( 6 );
    rStrm.EndRecord();

    // SXSTRING record
    rStrm.StartRecord( EXC_ID_SXSTRING, maUrl.GetSize() );
    rStrm << maUrl;
    rStrm.EndRecord();

    // unknown record 0x0802
    rStrm.StartRecord( EXC_ID_0802, 16 + maDestRange.GetSize() );
    rStrm   << EXC_ID_0802;             // repeated record id ?!?
    rStrm.WriteZeroBytes( 6 );
    rStrm   << sal_uInt16( 0x0003 )
            << sal_uInt32( 0x00000000 )
            << sal_uInt16( 0x0010 )
            << maDestRange;
    rStrm.EndRecord();

    // WEBQRYSETTINGS record
    nFlags = mpQryTables ? EXC_WQSETT_SPECTABLES : EXC_WQSETT_ALL;
    rStrm.StartRecord( EXC_ID_WQSETT, 28 );
    rStrm   << EXC_ID_WQSETT            // repeated record id ?!?
            << sal_uInt16( 0x0000 )
            << sal_uInt16( 0x0004 )
            << sal_uInt16( 0x0000 )
            << EXC_WQSETT_DEFAULTFLAGS
            << nFlags;
    rStrm.WriteZeroBytes( 10 );
    rStrm   << mnRefresh                // refresh delay in minutes
            << EXC_WQSETT_FORMATFULL
            << sal_uInt16( 0x0000 );
    rStrm.EndRecord();

    // WEBQRYTABLES record
    if( mpQryTables )
    {
        rStrm.StartRecord( EXC_ID_WQTABLES, 4 + mpQryTables->GetSize() );
        rStrm   << EXC_ID_WQTABLES          // repeated record id ?!?
                << sal_uInt16( 0x0000 )
                << *mpQryTables;            // comma separated list of source tables
        rStrm.EndRecord();
    }
}


// ----------------------------------------------------------------------------

XclExpWebQueryBuffer::XclExpWebQueryBuffer( const XclExpRoot& rRoot )
{
    sal_uInt16 nTab = rRoot.GetScTab();
    ScDocument& rDoc = rRoot.GetDoc();
    SfxObjectShell* pShell = rRoot.GetDocShell();
    if( !pShell ) return;
    Reference< XPropertySet > xPropSet( pShell->GetModel(), UNO_QUERY );
    if( !xPropSet.is() ) return;

    Reference< XAreaLinks > xAreaLinks;
    ::getPropValue( xAreaLinks, xPropSet, OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_AREALINKS ) ) );
    Reference< XIndexAccess > xLinksIA( xAreaLinks, UNO_QUERY );
    if( !xLinksIA.is() ) return;

    const OUString aPropFilter( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_FILTER ) );
//  const OUString aPropFilterOpt( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_FILTOPT ) );
    const OUString aPropUrl( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_LINKURL ) );
    const OUString aPropRefresh( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_REFDELAY ) );
    OUString aFilter, /*aFilterOpt,*/ aUrl;
    const OUString aWebQueryFilter( RTL_CONSTASCII_USTRINGPARAM( EXC_WEBQRY_FILTER ) );
    String aRangeName;
    sal_Int32 nRefresh;

    sal_Int32 nCount = xLinksIA->getCount();
    for( sal_Int32 nIndex = 0; nIndex < nCount; ++nIndex )
    {
        Reference< XAreaLink > xAreaLink;
        Any aLinkAny( xLinksIA->getByIndex( nIndex ) );
        if( aLinkAny >>= xAreaLink )
        {
            CellRangeAddress aDestRange( xAreaLink->getDestArea() );
            if( aDestRange.Sheet == nTab )
            {
                Reference< XPropertySet > xLinkProp( xAreaLink, UNO_QUERY );
                if( xLinkProp.is() && ::getPropValue( aFilter, xLinkProp, aPropFilter ) && (aFilter == aWebQueryFilter) )
                {
                    // get properties
//                  ::getPropValue( aFilterOpt, xLinkProp, aPropFilterOpt );
                    ::getPropValue( aUrl, xLinkProp, aPropUrl );
                    ::getPropValue( nRefresh, xLinkProp, aPropRefresh );

                    String aAbsDoc( ScGlobal::GetAbsDocName( aUrl, pShell ) );
                    INetURLObject aUrlObj( aAbsDoc );
                    String aWebQueryUrl( aUrlObj.getFSysPath( INetURLObject::FSYS_DOS ) );
                    if( !aWebQueryUrl.Len() )
                        aWebQueryUrl = aAbsDoc;

                    // find range or create a new range
                    ScRange aScDestRange;
                    ScUnoConversion::FillScRange( aScDestRange, aDestRange );
                    ScRangeData* pRangeData = rDoc.GetRangeName()->GetRangeAtBlock( aScDestRange );
                    if( pRangeData )
                        aRangeName = pRangeData->GetName();
                    else
                    {
                        ExcName* pExcName = new ExcName( *rRoot.mpRD, aScDestRange, aUrlObj.getBase() );
                        aRangeName = pExcName->GetName();
                        rRoot.mpRD->pNameList->InsertSorted( *rRoot.mpRD, pExcName, nTab );
                    }

                    // create and store the web query record
                    Append( new XclExpWebQuery( aRangeName, aWebQueryUrl, xAreaLink->getSourceArea(), nRefresh ) );
                }
            }
        }
    }
}


// ============================================================================

