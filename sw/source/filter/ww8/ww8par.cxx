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

#include <com/sun/star/embed/ElementModes.hpp>

#include <i18nlangtag/languagetag.hxx>

#include <unotools/ucbstreamhelper.hxx>
#include <rtl/random.h>
#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"

#include <sfx2/docinf.hxx>
#include <sfx2/request.hxx>
#include <sfx2/frame.hxx>
#include <tools/urlobj.hxx>
#include <unotools/tempfile.hxx>

#include <comphelper/docpasswordrequest.hxx>
#include <comphelper/string.hxx>

#include <editeng/brushitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <editeng/fontitem.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svxerr.hxx>
#include <filter/msfilter/mscodec.hxx>
#include <svx/svdmodel.hxx>
#include <svx/xflclit.hxx>

#include <unotools/fltrcfg.hxx>
#include <fmtfld.hxx>
#include <fmturl.hxx>
#include <fmtinfmt.hxx>
#include <reffld.hxx>
#include <fmthdft.hxx>
#include <fmtcntnt.hxx>
#include <fmtcnct.hxx>
#include <fmtanchr.hxx>
#include <fmtpdsc.hxx>
#include <ftninfo.hxx>
#include <fmtftn.hxx>
#include <txtftn.hxx>
#include <ndtxt.hxx>
#include <pagedesc.hxx>
#include <paratr.hxx>
#include <fmtclbl.hxx>
#include <section.hxx>
#include <docsh.hxx>
#include <docufld.hxx>
#include <swfltopt.hxx>
#include <viewsh.hxx>
#include <shellres.hxx>
#include <mdiexp.hxx>
#include <statstr.hrc>
#include <swerror.h>
#include <swtable.hxx>
#include <fchrfmt.hxx>
#include <charfmt.hxx>

#include <comphelper/extract.hxx>
#include <fltini.hxx>

#include "writerwordglue.hxx"

#include "ndgrf.hxx"
#include <editeng/editids.hrc>
#include <txtflcnt.hxx>
#include <fmtflcnt.hxx>
#include <txatbase.hxx>

#include "ww8par2.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/document/IndexedPropertyValues.hpp>
#include <svl/itemiter.hxx>

#include <comphelper/processfactory.hxx>
#include <basic/basmgr.hxx>

#include "ww8toolbar.hxx"
#include <osl/file.hxx>

#include <breakit.hxx>

#if OSL_DEBUG_LEVEL > 1
#include <iostream>
#include <dbgoutsw.hxx>
#endif
#include <unotools/localfilehelper.hxx>

#include <svx/hlnkitem.hxx>
#include "WW8Sttbf.hxx"
#include "WW8FibData.hxx"

using namespace ::com::sun::star;
using namespace sw::util;
using namespace sw::types;
using namespace nsHdFtFlags;

#include <com/sun/star/i18n/ScriptType.hpp>
#include <unotools/pathoptions.hxx>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>

#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <comphelper/sequenceashashmap.hxx>
#include <oox/ole/vbaproject.hxx>
#include <oox/ole/olestorage.hxx>







/** Expands to a pointer behind the last element of a STATIC data array (like STL end()). */

/** Expands to the size of a STATIC data array. */


SwMacroInfo* GetMacroInfo( SdrObject* pObj, sal_Bool bCreate )             
{
    if ( pObj )
    {
        sal_uInt16 nCount = pObj->GetUserDataCount();
        for( sal_uInt16 i = 0; i < nCount; i++ )
        {
            SdrObjUserData* pData = pObj->GetUserData( i );
            if( pData && pData->GetInventor() == SW_DRAWLAYER
                && pData->GetId() == SW_UD_IMAPDATA)
            {
                return dynamic_cast<SwMacroInfo*>(pData);
            }
        }
        if ( bCreate )
        {
            SwMacroInfo* pData = new SwMacroInfo;
            pObj->AppendUserData(pData);
            return pData;
        }
    }

    return 0;
};

void lclGetAbsPath(OUString& rPath, sal_uInt16 nLevel, SwDocShell* pDocShell)
{
    OUString aTmpStr;
    while( nLevel )
    {
        aTmpStr += "../";
        --nLevel;
    }
    if (!aTmpStr.isEmpty())
        aTmpStr += rPath;
    else
        aTmpStr = rPath;

    if (!aTmpStr.isEmpty())
    {
        bool bWasAbs = false;
        rPath = pDocShell->GetMedium()->GetURLObject().smartRel2Abs( aTmpStr, bWasAbs ).GetMainURL( INetURLObject::NO_DECODE );
        
    }
}

void lclIgnoreString32( SvMemoryStream& rStrm, bool b16Bit )
{
    sal_uInt32 nChars(0);
    rStrm.ReadUInt32( nChars );
    if( b16Bit )
        nChars *= 2;
    rStrm.SeekRel( nChars );
}

OUString SwWW8ImplReader::ReadRawUniString(SvMemoryStream& rStrm, sal_uInt16 nChars, bool b16Bit)
{
    
    const sal_uInt8 WW8_NUL_C                   = '\x00';       
    const sal_uInt16 WW8_NUL                    = WW8_NUL_C;    
    sal_Unicode         mcNulSubst = '\0';

    sal_uInt16 nCharsLeft = nChars;
    sal_Unicode* pcBuffer = new sal_Unicode[ nCharsLeft + 1 ];

    sal_Unicode* pcUniChar = pcBuffer;
    sal_Unicode* pcEndChar = pcBuffer + nCharsLeft;

    if( b16Bit )
    {
        sal_uInt16 nReadChar;
        for( ;  (pcUniChar < pcEndChar); ++pcUniChar )
        {
            rStrm.ReadUInt16( nReadChar );
            (*pcUniChar) = (nReadChar == WW8_NUL) ? mcNulSubst : static_cast< sal_Unicode >( nReadChar );
        }
    }
    else
    {
        sal_uInt8 nReadChar;
        for( ; (pcUniChar < pcEndChar); ++pcUniChar )
        {
            rStrm.ReadUChar( nReadChar ) ;
            (*pcUniChar) = (nReadChar == WW8_NUL_C) ? mcNulSubst : static_cast< sal_Unicode >( nReadChar );
        }
    }

    *pcEndChar = '\0';
    OUString aRet(pcBuffer);
    delete[] pcBuffer;
    return aRet;
}

void lclAppendString32(OUString& rString, SvMemoryStream& rStrm, sal_uInt32 nChars, bool b16Bit)
{
    sal_uInt16 nReadChars = ulimit_cast< sal_uInt16 >( nChars );
    OUString urlStr = SwWW8ImplReader::ReadRawUniString( rStrm, nReadChars, b16Bit );
    rString += urlStr;
}

void lclAppendString32(OUString& rString, SvMemoryStream& rStrm, bool b16Bit)
{
    sal_uInt32 nValue(0);
    rStrm.ReadUInt32( nValue );
    lclAppendString32(rString, rStrm, nValue, b16Bit);
}

void SwWW8ImplReader::ReadEmbeddedData( SvMemoryStream& rStrm, SwDocShell* pDocShell, struct HyperLinksTable& hlStr)
{
    
    
    const sal_uInt32 WW8_HLINK_BODY             = 0x00000001;   
    const sal_uInt32 WW8_HLINK_ABS              = 0x00000002;   
    const sal_uInt32 WW8_HLINK_DESCR            = 0x00000014;   
    const sal_uInt32 WW8_HLINK_MARK             = 0x00000008;   
    const sal_uInt32 WW8_HLINK_FRAME            = 0x00000080;   
    const sal_uInt32 WW8_HLINK_UNC              = 0x00000100;   

    
    

    sal_uInt8 maGuidUrlMoniker[ 16 ] = {
        0xE0, 0xC9, 0xEA, 0x79, 0xF9, 0xBA, 0xCE, 0x11, 0x8C, 0x82, 0x00, 0xAA, 0x00, 0x4B, 0xA9, 0x0B };

    sal_uInt8 maGuidFileMoniker[ 16 ] = {
        0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 };

    sal_uInt8 aGuid[16];
    sal_uInt32 nFlags(0);


    rStrm.Read(aGuid, 16);
    rStrm.SeekRel( 4 );
    rStrm.ReadUInt32( nFlags );

    sal_uInt16 nLevel = 0;                  
    boost::scoped_ptr< OUString > xLongName;    
    boost::scoped_ptr< OUString > xShortName;   
    boost::scoped_ptr< OUString > xTextMark;    

    
    if( ::get_flag( nFlags, WW8_HLINK_DESCR ) )
        lclIgnoreString32( rStrm, true );

    
    if( ::get_flag( nFlags, WW8_HLINK_FRAME ) )
    {
        OUString sFrmName;
        lclAppendString32(sFrmName, rStrm, true);
        hlStr.tarFrm = sFrmName;
    }

        
    if( ::get_flag( nFlags, WW8_HLINK_UNC ) )
    {
        xLongName.reset( new OUString );
        lclAppendString32( *xLongName, rStrm, true );
        lclGetAbsPath( *xLongName, 0 , pDocShell);
    }
    
    else if( ::get_flag( nFlags, WW8_HLINK_BODY ) )
    {
        rStrm.Read( aGuid, 16);

        if( (memcmp(aGuid, maGuidFileMoniker, 16) == 0) )
        {
            rStrm.ReadUInt16( nLevel );
            xShortName.reset( new OUString );
            lclAppendString32( *xShortName,rStrm, false );
            rStrm.SeekRel( 24 );

            sal_uInt32 nStrLen(0);
            rStrm.ReadUInt32( nStrLen );
            if( nStrLen )
            {
                nStrLen = 0;
                rStrm.ReadUInt32( nStrLen );
                nStrLen /= 2;
                rStrm.SeekRel( 2 );
                xLongName.reset( new OUString );
                lclAppendString32( *xLongName, rStrm,nStrLen, true );
                lclGetAbsPath( *xLongName, nLevel, pDocShell);
            }
            else
                lclGetAbsPath( *xShortName, nLevel, pDocShell);
        }
        else if( (memcmp(aGuid, maGuidUrlMoniker, 16) == 0) )
        {
            sal_uInt32 nStrLen(0);
            rStrm.ReadUInt32( nStrLen );
            nStrLen /= 2;
            xLongName.reset( new OUString );
            lclAppendString32( *xLongName,rStrm, nStrLen, true );
            if( !::get_flag( nFlags, WW8_HLINK_ABS ) )
                lclGetAbsPath( *xLongName, 0 ,pDocShell);
        }
        else
        {
            DBG_ERRORFILE( "WW8Hyperlink::ReadEmbeddedData - unknown content GUID" );
        }
    }

    
    if( ::get_flag( nFlags, WW8_HLINK_MARK ) )
    {
        xTextMark.reset( new OUString );
        lclAppendString32( *xTextMark, rStrm, true );
    }

    if( !xLongName.get() && xShortName.get() )
    {
        xLongName.reset( new OUString );
        *xLongName = xLongName->concat(*xShortName);
    }
    else if( !xLongName.get() && xTextMark.get() )
        xLongName.reset( new OUString );

    if( xLongName.get() )
    {
        if( xTextMark.get() )
        {
            if (xLongName->isEmpty())
                *xTextMark = xTextMark->replace('!', '.');
            *xLongName = xLongName->concat("#");
            *xLongName = xLongName->concat(*xTextMark);
        }
        hlStr.hLinkAddr = *xLongName;
    }
}

class BasicProjImportHelper
{
    SwDocShell& mrDocShell;
    uno::Reference< uno::XComponentContext > mxCtx;
public:
    BasicProjImportHelper( SwDocShell& rShell ) : mrDocShell( rShell )
    {
        mxCtx = comphelper::getProcessComponentContext();
    }
    bool import( const uno::Reference< io::XInputStream >& rxIn );
    OUString getProjectName();
};

bool BasicProjImportHelper::import( const uno::Reference< io::XInputStream >& rxIn )
{
    bool bRet = false;
    try
    {
        oox::ole::OleStorage root( mxCtx, rxIn, false );
        oox::StorageRef vbaStg = root.openSubStorage( "Macros" , false );
        if ( vbaStg.get() )
        {
            oox::ole::VbaProject aVbaPrj( mxCtx, mrDocShell.GetModel(), OUString("Writer") );
            bRet = aVbaPrj.importVbaProject( *vbaStg );
        }
    }
    catch( const uno::Exception& )
    {
        bRet = false;
    }
    return bRet;
}

OUString BasicProjImportHelper::getProjectName()
{
    OUString sProjName( "Standard" );
    uno::Reference< beans::XPropertySet > xProps( mrDocShell.GetModel(), uno::UNO_QUERY );
    if ( xProps.is() )
    {
        try
        {
            uno::Reference< script::vba::XVBACompatibility > xVBA( xProps->getPropertyValue( "BasicLibraries" ), uno::UNO_QUERY_THROW  );
            sProjName = xVBA->getProjectName();

        }
        catch( const uno::Exception& )
        {
        }
    }
    return sProjName;
}

class Sttb : TBBase
{
struct SBBItem
{
    sal_uInt16 cchData;
    OUString data;
    SBBItem() : cchData(0){}
};
    sal_uInt16 fExtend;
    sal_uInt16 cData;
    sal_uInt16 cbExtra;

    std::vector< SBBItem > dataItems;

    Sttb(const Sttb&);
    Sttb& operator = ( const Sttb&);
public:
    Sttb();
    ~Sttb();
    bool Read(SvStream &rS);
    void Print( FILE* fp );
    OUString getStringAtIndex( sal_uInt32 );
};

Sttb::Sttb() : fExtend( 0 )
,cData( 0 )
,cbExtra( 0 )
{
}

Sttb::~Sttb()
{
}

bool Sttb::Read( SvStream& rS )
{
    OSL_TRACE("Sttb::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    rS.ReadUInt16( fExtend ).ReadUInt16( cData ).ReadUInt16( cbExtra );
    if ( cData )
    {
        for ( sal_Int32 index = 0; index < cData; ++index )
        {
            SBBItem aItem;
            rS.ReadUInt16( aItem.cchData );
            aItem.data = read_uInt16s_ToOUString(rS, aItem.cchData);
            dataItems.push_back( aItem );
        }
    }
    return true;
}

void Sttb::Print( FILE* fp )
{
    fprintf( fp, "[ 0x%" SAL_PRIxUINT32 " ] Sttb - dump\n", nOffSet);
    fprintf( fp, " fExtend 0x%x [expected 0xFFFF ]\n", fExtend );
    fprintf( fp, " cData no. or string data items %d (0x%x)\n", cData, cData );

    if ( cData )
    {
        for ( sal_Int32 index = 0; index < cData; ++index )
            fprintf(fp,"   string dataItem[ %d(0x%x) ] has name %s\n", static_cast< int >( index ), static_cast< unsigned int >( index ), OUStringToOString( dataItems[ index ].data, RTL_TEXTENCODING_UTF8 ).getStr() );
    }
}

OUString
Sttb::getStringAtIndex( sal_uInt32 index )
{
    OUString aRet;
    if ( index < dataItems.size() )
        aRet = dataItems[ index ].data;
    return aRet;

}

SwMSDffManager::SwMSDffManager( SwWW8ImplReader& rRdr )
    : SvxMSDffManager(*rRdr.pTableStream, rRdr.GetBaseURL(), rRdr.pWwFib->fcDggInfo,
        rRdr.pDataStream, 0, 0, COL_WHITE, 12, rRdr.pStrm),
    rReader(rRdr), pFallbackStream(0)
{
    SetSvxMSDffSettings( GetSvxMSDffSettings() );
    nSvxMSDffOLEConvFlags = SwMSDffManager::GetFilterFlags();
}

sal_uInt32 SwMSDffManager::GetFilterFlags()
{
    sal_uInt32 nFlags(0);
    const SvtFilterOptions& rOpt = SvtFilterOptions::Get();
    if (rOpt.IsMathType2Math())
        nFlags |= OLE_MATHTYPE_2_STARMATH;
    if (rOpt.IsExcel2Calc())
        nFlags |= OLE_EXCEL_2_STARCALC;
    if (rOpt.IsPowerPoint2Impress())
        nFlags |= OLE_POWERPOINT_2_STARIMPRESS;
    if (rOpt.IsWinWord2Writer())
        nFlags |= OLE_WINWORD_2_STARWRITER;
    return nFlags;
}

/*
 * I would like to override the default OLE importing to add a test
 * and conversion of OCX controls from their native OLE type into our
 * native nonOLE Form Control Objects.
 *
 * cmc
 */

SdrObject* SwMSDffManager::ImportOLE( long nOLEId,
                                      const Graphic& rGrf,
                                      const Rectangle& rBoundRect,
                                      const Rectangle& rVisArea,
                                      const int _nCalledByGroup,
                                      sal_Int64 nAspect ) const
{
    
    
    
    if ( _nCalledByGroup > 0 )
    {
        return 0L;
    }

    SdrObject* pRet = 0;
    OUString sStorageName;
    SotStorageRef xSrcStg;
    uno::Reference < embed::XStorage > xDstStg;
    if( GetOLEStorageName( nOLEId, sStorageName, xSrcStg, xDstStg ))
    {
        SvStorageRef xSrc = xSrcStg->OpenSotStorage( sStorageName,
            STREAM_READWRITE| STREAM_SHARE_DENYALL );
        OSL_ENSURE(rReader.pFormImpl, "No Form Implementation!");
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape;
        if ( (!(rReader.bIsHeader || rReader.bIsFooter)) &&
            rReader.pFormImpl->ReadOCXStream(xSrc,&xShape,true))
        {
            pRet = GetSdrObjectFromXShape(xShape);
        }
        else
        {
            ErrCode nError = ERRCODE_NONE;
            pRet = CreateSdrOLEFromStorage( sStorageName, xSrcStg, xDstStg,
                rGrf, rBoundRect, rVisArea, pStData, nError, nSvxMSDffOLEConvFlags, nAspect );
        }
    }
    return pRet;
}

void SwMSDffManager::DisableFallbackStream()
{
    OSL_ENSURE(!pFallbackStream,
        "if you're recursive, you're broken");
    pFallbackStream = pStData2;
    aOldEscherBlipCache = aEscherBlipCache;
    aEscherBlipCache.clear();
    pStData2 = 0;
}

void SwMSDffManager::EnableFallbackStream()
{
    pStData2 = pFallbackStream;
    aEscherBlipCache = aOldEscherBlipCache;
    aOldEscherBlipCache.clear();
    pFallbackStream = 0;
}

sal_uInt16 SwWW8ImplReader::GetToggleAttrFlags() const
{
    return pCtrlStck ? pCtrlStck->GetToggleAttrFlags() : 0;
}

sal_uInt16 SwWW8ImplReader::GetToggleBiDiAttrFlags() const
{
    return pCtrlStck ? pCtrlStck->GetToggleBiDiAttrFlags() : 0;
}

void SwWW8ImplReader::SetToggleAttrFlags(sal_uInt16 nFlags)
{
    if (pCtrlStck)
        pCtrlStck->SetToggleAttrFlags(nFlags);
}

void SwWW8ImplReader::SetToggleBiDiAttrFlags(sal_uInt16 nFlags)
{
    if (pCtrlStck)
        pCtrlStck->SetToggleBiDiAttrFlags(nFlags);
}

SdrObject* SwMSDffManager::ProcessObj(SvStream& rSt,
                                       DffObjData& rObjData,
                                       void* pData,
                                       Rectangle& rTextRect,
                                       SdrObject* pObj
                                       )
{
    if( !rTextRect.IsEmpty() )
    {
        SvxMSDffImportData& rImportData = *(SvxMSDffImportData*)pData;
        SvxMSDffImportRec* pImpRec = new SvxMSDffImportRec;

        
        pImpRec->nShapeId   = rObjData.nShapeId;
        pImpRec->eShapeType = rObjData.eShapeType;

        rObjData.bClientAnchor = maShapeRecords.SeekToContent( rSt,
                                            DFF_msofbtClientAnchor,
                                            SEEK_FROM_CURRENT_AND_RESTART );
        if( rObjData.bClientAnchor )
            ProcessClientAnchor( rSt,
                    maShapeRecords.Current()->nRecLen,
                    pImpRec->pClientAnchorBuffer, pImpRec->nClientAnchorLen );

        rObjData.bClientData = maShapeRecords.SeekToContent( rSt,
                                            DFF_msofbtClientData,
                                            SEEK_FROM_CURRENT_AND_RESTART );
        if( rObjData.bClientData )
            ProcessClientData( rSt,
                    maShapeRecords.Current()->nRecLen,
                    pImpRec->pClientDataBuffer, pImpRec->nClientDataLen );


        
        
        pImpRec->nLayoutInTableCell = 0xFFFFFFFF;

        if(    maShapeRecords.SeekToContent( rSt,
                                             DFF_msofbtUDefProp,
                                             SEEK_FROM_CURRENT_AND_RESTART )
            && maShapeRecords.Current()->nRecLen )
        {
            sal_uInt32  nBytesLeft = maShapeRecords.Current()->nRecLen;
            sal_uInt32  nUDData;
            sal_uInt16  nPID;
            while( 5 < nBytesLeft )
            {
                rSt.ReadUInt16( nPID );
                if ( rSt.GetError() != 0 )
                    break;
                rSt.ReadUInt32( nUDData );
                switch( nPID )
                {
                    case 0x038F: pImpRec->nXAlign = nUDData; break;
                    case 0x0390:
                        delete pImpRec->pXRelTo;
                        pImpRec->pXRelTo = new sal_uInt32;
                        *(pImpRec->pXRelTo) = nUDData;
                        break;
                    case 0x0391: pImpRec->nYAlign = nUDData; break;
                    case 0x0392:
                        delete pImpRec->pYRelTo;
                        pImpRec->pYRelTo = new sal_uInt32;
                        *(pImpRec->pYRelTo) = nUDData;
                        break;
                    case 0x03BF: pImpRec->nLayoutInTableCell = nUDData; break;
                    case 0x0393:
                    
                    
                    
                        pImpRec->relativeHorizontalWidth = nUDData;
                        break;
                    case 0x0394:
                    
                    
                    
                        pImpRec->isHorizontalRule = true;
                        break;
                }
                if ( rSt.GetError() != 0 )
                    break;
                pImpRec->bHasUDefProp = sal_True;
                nBytesLeft  -= 6;
            }
        }

        
        sal_uInt32 nTextId = GetPropertyValue( DFF_Prop_lTxid, 0 );
        if( nTextId )
        {
            SfxItemSet aSet( pSdrModel->GetItemPool() );

            
            
            
            
            bool bIsSimpleDrawingTextBox = (pImpRec->eShapeType == mso_sptTextBox);
            if (!bIsSimpleDrawingTextBox)
            {
                
                
                
                bIsSimpleDrawingTextBox =
                (
                    (pImpRec->eShapeType == mso_sptTextSimple) ||
                    (
                        (pImpRec->eShapeType == mso_sptRectangle)
                        && ShapeHasText(pImpRec->nShapeId, rObjData.rSpHd.GetRecBegFilePos() )
                    )
                );
            }

            
            sal_Int32 nTextLeft = GetPropertyValue( DFF_Prop_dxTextLeft, 91440L);
            sal_Int32 nTextRight = GetPropertyValue( DFF_Prop_dxTextRight, 91440L );
            sal_Int32 nTextTop = GetPropertyValue( DFF_Prop_dyTextTop, 45720L  );
            sal_Int32 nTextBottom = GetPropertyValue( DFF_Prop_dyTextBottom, 45720L );

            ScaleEmu( nTextLeft );
            ScaleEmu( nTextRight );
            ScaleEmu( nTextTop );
            ScaleEmu( nTextBottom );

            sal_Int32 nTextRotationAngle=0;
            bool bVerticalText = false;
            if ( IsProperty( DFF_Prop_txflTextFlow ) )
            {
                MSO_TextFlow eTextFlow = (MSO_TextFlow)(GetPropertyValue(
                    DFF_Prop_txflTextFlow) & 0xFFFF);
                switch( eTextFlow )
                {
                    case mso_txflBtoT:
                        nTextRotationAngle = 9000;
                    break;
                    case mso_txflVertN:
                    case mso_txflTtoBN:
                        nTextRotationAngle = 27000;
                        break;
                    case mso_txflTtoBA:
                        bVerticalText = true;
                    break;
                    case mso_txflHorzA:
                        bVerticalText = true;
                        nTextRotationAngle = 9000;
                    case mso_txflHorzN:
                    default :
                        break;
                }
            }

            if (nTextRotationAngle)
            {
                while (nTextRotationAngle > 360000)
                    nTextRotationAngle-=9000;
                switch (nTextRotationAngle)
                {
                    case 9000:
                        {
                            long nWidth = rTextRect.GetWidth();
                            rTextRect.Right() = rTextRect.Left() + rTextRect.GetHeight();
                            rTextRect.Bottom() = rTextRect.Top() + nWidth;

                            sal_Int32 nOldTextLeft = nTextLeft;
                            sal_Int32 nOldTextRight = nTextRight;
                            sal_Int32 nOldTextTop = nTextTop;
                            sal_Int32 nOldTextBottom = nTextBottom;

                            nTextLeft = nOldTextBottom;
                            nTextRight = nOldTextTop;
                            nTextTop = nOldTextLeft;
                            nTextBottom = nOldTextRight;
                        }
                        break;
                    case 27000:
                        {
                            long nWidth = rTextRect.GetWidth();
                            rTextRect.Right() = rTextRect.Left() + rTextRect.GetHeight();
                            rTextRect.Bottom() = rTextRect.Top() + nWidth;

                            sal_Int32 nOldTextLeft = nTextLeft;
                            sal_Int32 nOldTextRight = nTextRight;
                            sal_Int32 nOldTextTop = nTextTop;
                            sal_Int32 nOldTextBottom = nTextBottom;

                            nTextLeft = nOldTextTop;
                            nTextRight = nOldTextBottom;
                            nTextTop = nOldTextRight;
                            nTextBottom = nOldTextLeft;
                        }
                        break;
                    default:
                        break;
                }
            }

            if (bIsSimpleDrawingTextBox)
            {
                SdrObject::Free( pObj );
                pObj = new SdrRectObj(OBJ_TEXT, rTextRect);
            }

            
            
            Rectangle aNewRect(rTextRect);
            aNewRect.Bottom() -= nTextTop + nTextBottom;
            aNewRect.Right() -= nTextLeft + nTextRight;

            
            
            if( bIsSimpleDrawingTextBox )
            {
                ::boost::shared_ptr<SvxMSDffShapeInfo> const pTmpRec(
                        new SvxMSDffShapeInfo(0, pImpRec->nShapeId));

                SvxMSDffShapeInfos_ById::const_iterator const it =
                    GetShapeInfos()->find(pTmpRec);
                if (it != GetShapeInfos()->end())
                {
                    SvxMSDffShapeInfo& rInfo = **it;
                    pImpRec->bReplaceByFly   = rInfo.bReplaceByFly;
                    pImpRec->bLastBoxInChain = rInfo.bLastBoxInChain;
                }
            }

            if( bIsSimpleDrawingTextBox )
                ApplyAttributes( rSt, aSet, rObjData );

            if (GetPropertyValue(DFF_Prop_FitTextToShape) & 2)
            {
                aSet.Put( SdrTextAutoGrowHeightItem( sal_True ) );
                aSet.Put( SdrTextMinFrameHeightItem(
                    aNewRect.Bottom() - aNewRect.Top() ) );
                aSet.Put( SdrTextMinFrameWidthItem(
                    aNewRect.Right() - aNewRect.Left() ) );
            }
            else
            {
                aSet.Put( SdrTextAutoGrowHeightItem( sal_False ) );
                aSet.Put( SdrTextAutoGrowWidthItem( sal_False ) );
            }

            switch ( (MSO_WrapMode)
                GetPropertyValue( DFF_Prop_WrapText, mso_wrapSquare ) )
            {
                case mso_wrapNone :
                    aSet.Put( SdrTextAutoGrowWidthItem( sal_True ) );
                    pImpRec->bAutoWidth = true;
                break;
                case mso_wrapByPoints :
                    aSet.Put( SdrTextContourFrameItem( sal_True ) );
                break;
                default:
                    ;
            }

            
            aSet.Put( SdrTextLeftDistItem( nTextLeft ) );
            aSet.Put( SdrTextRightDistItem( nTextRight ) );
            aSet.Put( SdrTextUpperDistItem( nTextTop ) );
            aSet.Put( SdrTextLowerDistItem( nTextBottom ) );
            pImpRec->nDxTextLeft    = nTextLeft;
            pImpRec->nDyTextTop     = nTextTop;
            pImpRec->nDxTextRight   = nTextRight;
            pImpRec->nDyTextBottom  = nTextBottom;

            
            MSO_Anchor eTextAnchor =
                (MSO_Anchor)GetPropertyValue( DFF_Prop_anchorText, mso_anchorTop );

            SdrTextVertAdjust eTVA = bVerticalText
                                     ? SDRTEXTVERTADJUST_BLOCK
                                     : SDRTEXTVERTADJUST_CENTER;
            SdrTextHorzAdjust eTHA = bVerticalText
                                     ? SDRTEXTHORZADJUST_CENTER
                                     : SDRTEXTHORZADJUST_BLOCK;

            switch( eTextAnchor )
            {
                case mso_anchorTop:
                {
                    if ( bVerticalText )
                        eTHA = SDRTEXTHORZADJUST_RIGHT;
                    else
                        eTVA = SDRTEXTVERTADJUST_TOP;
                }
                break;
                case mso_anchorTopCentered:
                {
                    if ( bVerticalText )
                        eTHA = SDRTEXTHORZADJUST_RIGHT;
                    else
                        eTVA = SDRTEXTVERTADJUST_TOP;
                }
                break;
                case mso_anchorMiddle:
                break;
                case mso_anchorMiddleCentered:
                break;
                case mso_anchorBottom:
                {
                    if ( bVerticalText )
                        eTHA = SDRTEXTHORZADJUST_LEFT;
                    else
                        eTVA = SDRTEXTVERTADJUST_BOTTOM;
                }
                break;
                case mso_anchorBottomCentered:
                {
                    if ( bVerticalText )
                        eTHA = SDRTEXTHORZADJUST_LEFT;
                    else
                        eTVA = SDRTEXTVERTADJUST_BOTTOM;
                }
                break;
                default:
                    ;
            }

            aSet.Put( SdrTextVertAdjustItem( eTVA ) );
            aSet.Put( SdrTextHorzAdjustItem( eTHA ) );

            if (pObj != NULL)
            {
                pObj->SetMergedItemSet(aSet);
                pObj->SetModel(pSdrModel);

                if (bVerticalText)
                {
                    SdrTextObj *pTextObj = dynamic_cast< SdrTextObj* >(pObj);
                    if (pTextObj)
                        pTextObj->SetVerticalWriting(true);
                }

                if ( bIsSimpleDrawingTextBox )
                {
                    if ( nTextRotationAngle )
                    {
                        long nMinWH = rTextRect.GetWidth() < rTextRect.GetHeight() ?
                            rTextRect.GetWidth() : rTextRect.GetHeight();
                        nMinWH /= 2;
                        Point aPivot(rTextRect.TopLeft());
                        aPivot.X() += nMinWH;
                        aPivot.Y() += nMinWH;
                        double a = nTextRotationAngle * nPi180;
                        pObj->NbcRotate(aPivot, nTextRotationAngle, sin(a), cos(a));
                    }
                }

                if ( ( ( rObjData.nSpFlags & SP_FFLIPV ) || mnFix16Angle || nTextRotationAngle ) && dynamic_cast< SdrObjCustomShape* >( pObj ) )
                {
                    SdrObjCustomShape* pCustomShape = dynamic_cast< SdrObjCustomShape* >( pObj );

                    double fExtraTextRotation = 0.0;
                    if ( mnFix16Angle && !( GetPropertyValue( DFF_Prop_FitTextToShape ) & 4 ) )
                    {   
                        fExtraTextRotation = -mnFix16Angle;
                    }
                    if ( rObjData.nSpFlags & SP_FFLIPV )    
                    {                                       
                        fExtraTextRotation += 18000.0;      
                    }
                    fExtraTextRotation += nTextRotationAngle;
                    if ( !::basegfx::fTools::equalZero( fExtraTextRotation ) )
                    {
                        fExtraTextRotation /= 100.0;
                        SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pCustomShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
                        const OUString sTextRotateAngle( "TextRotateAngle" );
                        com::sun::star::beans::PropertyValue aPropVal;
                        aPropVal.Name = sTextRotateAngle;
                        aPropVal.Value <<= fExtraTextRotation;
                        aGeometryItem.SetPropertyValue( aPropVal );
                        pCustomShape->SetMergedItem( aGeometryItem );
                    }
                }
                else if ( mnFix16Angle )
                {
                    
                    double a = mnFix16Angle * nPi180;
                    pObj->NbcRotate( rObjData.aBoundRect.Center(), mnFix16Angle,
                                     sin( a ), cos( a ) );
                }
            }
        }
        else if( !pObj )
        {
            
            
            
            pObj = new SdrRectObj(rTextRect);
            pObj->SetModel( pSdrModel );
            SfxItemSet aSet( pSdrModel->GetItemPool() );
            ApplyAttributes( rSt, aSet, rObjData );

            const SfxPoolItem* pPoolItem=NULL;
            SfxItemState eState = aSet.GetItemState( XATTR_FILLCOLOR,
                                                     false, &pPoolItem );
            if( SFX_ITEM_DEFAULT == eState )
                aSet.Put( XFillColorItem( OUString(),
                          Color( mnDefaultColor ) ) );
            pObj->SetMergedItemSet(aSet);
        }

        
        if (GetPropertyValue(DFF_Prop_fPrint) & 0x20)
            pImpRec->bDrawHell = sal_True;
        else
            pImpRec->bDrawHell = sal_False;
        if (GetPropertyValue(DFF_Prop_fPrint) & 0x02)
            pImpRec->bHidden = sal_True;
        pImpRec->nNextShapeId   = GetPropertyValue( DFF_Prop_hspNext, 0 );

        if ( nTextId )
        {
            pImpRec->aTextId.nTxBxS = (sal_uInt16)( nTextId >> 16 );
            pImpRec->aTextId.nSequence = (sal_uInt16)nTextId;
        }

        pImpRec->nDxWrapDistLeft = GetPropertyValue(
                                    DFF_Prop_dxWrapDistLeft, 114935L ) / 635L;
        pImpRec->nDyWrapDistTop = GetPropertyValue(
                                    DFF_Prop_dyWrapDistTop, 0 ) / 635L;
        pImpRec->nDxWrapDistRight = GetPropertyValue(
                                    DFF_Prop_dxWrapDistRight, 114935L ) / 635L;
        pImpRec->nDyWrapDistBottom = GetPropertyValue(
                                    DFF_Prop_dyWrapDistBottom, 0 ) / 635L;
        

        if (SeekToContent(DFF_Prop_pWrapPolygonVertices, rSt))
        {
            delete pImpRec->pWrapPolygon;
            pImpRec->pWrapPolygon = NULL;

            sal_uInt16 nNumElemVert, nNumElemMemVert, nElemSizeVert;
            rSt.ReadUInt16( nNumElemVert ).ReadUInt16( nNumElemMemVert ).ReadUInt16( nElemSizeVert );
            if (nNumElemVert && ((nElemSizeVert == 8) || (nElemSizeVert == 4)))
            {
                pImpRec->pWrapPolygon = new Polygon(nNumElemVert);
                for (sal_uInt16 i = 0; i < nNumElemVert; ++i)
                {
                    sal_Int32 nX, nY;
                    if (nElemSizeVert == 8)
                        rSt.ReadInt32( nX ).ReadInt32( nY );
                    else
                    {
                        sal_Int16 nSmallX, nSmallY;
                        rSt.ReadInt16( nSmallX ).ReadInt16( nSmallY );
                        nX = nSmallX;
                        nY = nSmallY;
                    }
                    (*(pImpRec->pWrapPolygon))[i].X() = nX;
                    (*(pImpRec->pWrapPolygon))[i].Y() = nY;
                }
            }
        }

        pImpRec->nCropFromTop = GetPropertyValue(
                                    DFF_Prop_cropFromTop, 0 );
        pImpRec->nCropFromBottom = GetPropertyValue(
                                    DFF_Prop_cropFromBottom, 0 );
        pImpRec->nCropFromLeft = GetPropertyValue(
                                    DFF_Prop_cropFromLeft, 0 );
        pImpRec->nCropFromRight = GetPropertyValue(
                                    DFF_Prop_cropFromRight, 0 );

        sal_uInt32 nLineFlags = GetPropertyValue( DFF_Prop_fNoLineDrawDash );

        if ( !IsHardAttribute( DFF_Prop_fLine ) &&
             pImpRec->eShapeType == mso_sptPictureFrame )
        {
            nLineFlags &= ~0x08;
        }

        pImpRec->eLineStyle = (nLineFlags & 8)
                              ? (MSO_LineStyle)GetPropertyValue(
                                                    DFF_Prop_lineStyle,
                                                    mso_lineSimple )
                              : (MSO_LineStyle)USHRT_MAX;
        pImpRec->eLineDashing = (MSO_LineDashing)GetPropertyValue(
                                        DFF_Prop_lineDashing, mso_lineSolid );

        pImpRec->nFlags = rObjData.nSpFlags;

        if( pImpRec->nShapeId )
        {
            
            pImpRec->pObj = pObj;
            rImportData.aRecords.insert( pImpRec );

            
            
            if( ( rObjData.nCalledByGroup == 0 )
                ||
                ( (rObjData.nSpFlags & SP_FGROUP)
                 && (rObjData.nCalledByGroup < 2) )
              )
                StoreShapeOrder( pImpRec->nShapeId,
                                ( ( (sal_uLong)pImpRec->aTextId.nTxBxS ) << 16 )
                                    + pImpRec->aTextId.nSequence, pObj );
        }
        else
            delete pImpRec;
    }

    sal_uInt32 nBufferSize = GetPropertyValue( DFF_Prop_pihlShape );
     if( (0 < nBufferSize) && (nBufferSize <= 0xFFFF) && SeekToContent( DFF_Prop_pihlShape, rSt ) )
    {
        SvMemoryStream aMemStream;
        struct HyperLinksTable hlStr;
        sal_uInt16 mnRawRecId,mnRawRecSize;
        aMemStream.WriteUInt16( sal_uInt16( 0 ) ).WriteUInt16( static_cast< sal_uInt16 >( nBufferSize ) );

        
        ::std::vector< sal_uInt8 > aBuffer( nBufferSize );
        sal_uInt8* pnData = &aBuffer.front();
        sal_uInt8 mnStreamSize;
        if( pnData && rSt.Read( pnData, nBufferSize ) == nBufferSize )
        {
            aMemStream.Write( pnData, nBufferSize );
            aMemStream.Seek( STREAM_SEEK_TO_END );
            mnStreamSize = aMemStream.Tell();
            aMemStream.Seek( STREAM_SEEK_TO_BEGIN );
            bool bRet =  4 <= mnStreamSize;
            if( bRet )
                aMemStream.ReadUInt16( mnRawRecId ).ReadUInt16( mnRawRecSize );
            SwDocShell* pDocShell = rReader.mpDocShell;
            if(pDocShell)
            {
                rReader.ReadEmbeddedData( aMemStream, pDocShell, hlStr);
            }
        }

        if (pObj && !hlStr.hLinkAddr.isEmpty())
        {
            SwMacroInfo* pInfo = GetMacroInfo( pObj, true );
            if( pInfo )
            {
                pInfo->SetShapeId( rObjData.nShapeId );
                pInfo->SetHlink( hlStr.hLinkAddr );
                if (!hlStr.tarFrm.isEmpty())
                    pInfo->SetTarFrm( hlStr.tarFrm );
                OUString aNameStr = GetPropertyString( DFF_Prop_wzName, rSt );
                if (!aNameStr.isEmpty())
                    pInfo->SetName( aNameStr );
            }
        }
    }

    return pObj;
}

/**
 * Special FastSave - Attributes
 */
void SwWW8ImplReader::Read_StyleCode( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if (nLen < 0)
    {
        bCpxStyle = false;
        return;
    }
    sal_uInt16 nColl = 0;
    if (pWwFib->GetFIBVersion() <= ww::eWW2)
        nColl = *pData;
    else
        nColl = SVBT16ToShort(pData);
    if (nColl < vColl.size())
    {
        SetTxtFmtCollAndListLevel( *pPaM, vColl[nColl] );
        bCpxStyle = true;
    }
}

/**
 * Read_Majority is for Majority (103) and Majority50 (108)
 */
void SwWW8ImplReader::Read_Majority( sal_uInt16, const sal_uInt8* , short )
{
}

/**
 * Stack
 */
void SwWW8FltControlStack::NewAttr(const SwPosition& rPos,
    const SfxPoolItem& rAttr)
{
    OSL_ENSURE(RES_TXTATR_FIELD != rAttr.Which(), "probably don't want to put"
        "fields into the control stack");
    OSL_ENSURE(RES_TXTATR_INPUTFIELD != rAttr.Which(), "probably don't want to put"
        "input fields into the control stack");
    OSL_ENSURE(RES_TXTATR_ANNOTATION != rAttr.Which(), "probably don't want to put"
        "annotations into the control stack");
    OSL_ENSURE(RES_FLTR_REDLINE != rAttr.Which(), "probably don't want to put"
        "redlines into the control stack");
    SwFltControlStack::NewAttr(rPos, rAttr);
}

SwFltStackEntry* SwWW8FltControlStack::SetAttr(const SwPosition& rPos, sal_uInt16 nAttrId,
    sal_Bool bTstEnde, long nHand, sal_Bool )
{
    SwFltStackEntry *pRet = NULL;
    
    
    
    if (rReader.pPlcxMan && rReader.pPlcxMan->GetDoingDrawTextBox())
    {
        size_t nCnt = size();
        for (size_t i=0; i < nCnt; ++i)
        {
            SwFltStackEntry& rEntry = (*this)[i];
            if (nAttrId == rEntry.pAttr->Which())
            {
                DeleteAndDestroy(i--);
                --nCnt;
            }
        }
    }
    else 
        pRet = SwFltControlStack::SetAttr(rPos, nAttrId, bTstEnde, nHand);
    return pRet;
}

long GetListFirstLineIndent(const SwNumFmt &rFmt)
{
    OSL_ENSURE( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION,
            "<GetListFirstLineIndent> - misusage: position-and-space-mode does not equal LABEL_WIDTH_AND_POSITION" );

    SvxAdjust eAdj = rFmt.GetNumAdjust();
    long nReverseListIndented;
    if (eAdj == SVX_ADJUST_RIGHT)
        nReverseListIndented = -rFmt.GetCharTextDistance();
    else if (eAdj == SVX_ADJUST_CENTER)
        nReverseListIndented = rFmt.GetFirstLineOffset()/2;
    else
        nReverseListIndented = rFmt.GetFirstLineOffset();
    return nReverseListIndented;
}

static long lcl_GetTrueMargin(const SvxLRSpaceItem &rLR, const SwNumFmt &rFmt,
    long &rFirstLinePos)
{
    OSL_ENSURE( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION,
            "<lcl_GetTrueMargin> - misusage: position-and-space-mode does not equal LABEL_WIDTH_AND_POSITION" );

    const long nBodyIndent = rLR.GetTxtLeft();
    const long nFirstLineDiff = rLR.GetTxtFirstLineOfst();
    rFirstLinePos = nBodyIndent + nFirstLineDiff;

    const long nPseudoListBodyIndent = rFmt.GetAbsLSpace();
    const long nReverseListIndented = GetListFirstLineIndent(rFmt);
    long nExtraListIndent = nPseudoListBodyIndent + nReverseListIndented;

    return nExtraListIndent > 0 ? nExtraListIndent : 0;
}



void SyncIndentWithList( SvxLRSpaceItem &rLR,
                         const SwNumFmt &rFmt,
                         const bool bFirstLineOfstSet,
                         const bool bLeftIndentSet )
{
    if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
    {
        long nWantedFirstLinePos;
        long nExtraListIndent = lcl_GetTrueMargin(rLR, rFmt, nWantedFirstLinePos);
        rLR.SetTxtLeft(nWantedFirstLinePos - nExtraListIndent);
        rLR.SetTxtFirstLineOfst(0);
    }
    else if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
    {
        if ( !bFirstLineOfstSet && bLeftIndentSet &&
             rFmt.GetFirstLineIndent() != 0 )
        {
            rLR.SetTxtFirstLineOfst( rFmt.GetFirstLineIndent() );
        }
        else if ( bFirstLineOfstSet && !bLeftIndentSet &&
                  rFmt.GetIndentAt() != 0 )
        {
            rLR.SetTxtLeft( rFmt.GetIndentAt() );
        }
        else if (!bFirstLineOfstSet && !bLeftIndentSet )
        {
            if ( rFmt.GetFirstLineIndent() != 0 )
            {
                rLR.SetTxtFirstLineOfst( rFmt.GetFirstLineIndent() );
            }
            if ( rFmt.GetIndentAt() != 0 )
            {
                rLR.SetTxtLeft( rFmt.GetIndentAt() );
            }
        }
    }
}

const SwNumFmt* SwWW8FltControlStack::GetNumFmtFromStack(const SwPosition &rPos,
    const SwTxtNode &rTxtNode)
{
    const SwNumFmt *pRet = 0;
    const SfxPoolItem *pItem = GetStackAttr(rPos, RES_FLTR_NUMRULE);
    if (pItem && rTxtNode.GetNumRule())
    {
        if (rTxtNode.IsCountedInList())
        {
            OUString sName(((SfxStringItem*)pItem)->GetValue());
            const SwNumRule *pRule = pDoc->FindNumRulePtr(sName);
            if (pRule)
                pRet = GetNumFmtFromSwNumRuleLevel(*pRule, rTxtNode.GetActualListLevel());
        }
    }
    return pRet;
}

sal_Int32 SwWW8FltControlStack::GetCurrAttrCP() const
{
    return rReader.GetCurrAttrCP();
}

bool SwWW8FltControlStack::IsParaEndInCPs(sal_Int32 nStart,sal_Int32 nEnd,bool bSdOD) const
{
    return rReader.IsParaEndInCPs(nStart,nEnd,bSdOD);
}

/**
 * Clear the para end position recorded in reader intermittently
 * for the least impact on loading performance.
 */
void SwWW8FltControlStack::ClearParaEndPosition()
{
    if ( !empty() )
        return;

    rReader.ClearParaEndPosition();
}

bool SwWW8FltControlStack::CheckSdOD(sal_Int32 nStart,sal_Int32 nEnd)
{
    return rReader.IsParaEndInCPs(nStart,nEnd);
}

void SwWW8ReferencedFltEndStack::SetAttrInDoc( const SwPosition& rTmpPos,
                                               SwFltStackEntry& rEntry )
{
    switch( rEntry.pAttr->Which() )
    {
    case RES_FLTR_BOOKMARK:
        {
            
            
            bool bInsertBookmarkIntoDoc = true;

            SwFltBookmark* pFltBookmark = dynamic_cast<SwFltBookmark*>(rEntry.pAttr);
            if ( pFltBookmark != 0 && pFltBookmark->IsTOCBookmark() )
            {
                const OUString& rName = pFltBookmark->GetName();
                ::std::set< OUString, SwWW8::ltstr >::const_iterator aResult = aReferencedTOCBookmarks.find(rName);
                if ( aResult == aReferencedTOCBookmarks.end() )
                {
                    bInsertBookmarkIntoDoc = false;
                }
            }
            if ( bInsertBookmarkIntoDoc )
            {
                SwFltEndStack::SetAttrInDoc( rTmpPos, rEntry );
            }
            break;
        }
    default:
        SwFltEndStack::SetAttrInDoc( rTmpPos, rEntry );
        break;
    }

}

void SwWW8FltControlStack::SetAttrInDoc(const SwPosition& rTmpPos,
    SwFltStackEntry& rEntry)
{
    switch (rEntry.pAttr->Which())
    {
        case RES_LR_SPACE:
            {
                /*
                 Loop over the affected nodes and
                 a) convert the word style absolute indent to indent relative
                    to any numbering indent active on the nodes
                 b) adjust the writer style tabstops relative to the old
                    paragraph indent to be relative to the new paragraph indent
                */
                using namespace sw::util;
                SwPaM aRegion(rTmpPos);
                if (rEntry.MakeRegion(pDoc, aRegion, false))
                {
                    SvxLRSpaceItem aNewLR( *(SvxLRSpaceItem*)rEntry.pAttr );
                    sal_uLong nStart = aRegion.Start()->nNode.GetIndex();
                    sal_uLong nEnd   = aRegion.End()->nNode.GetIndex();
                    for(; nStart <= nEnd; ++nStart)
                    {
                        SwNode* pNode = pDoc->GetNodes()[ nStart ];
                        if (!pNode || !pNode->IsTxtNode())
                            continue;

                        SwCntntNode* pNd = (SwCntntNode*)pNode;
                        SvxLRSpaceItem aOldLR = (const SvxLRSpaceItem&)pNd->GetAttr(RES_LR_SPACE);

                        SwTxtNode *pTxtNode = (SwTxtNode*)pNode;

                        const SwNumFmt *pNum = 0;
                        pNum = GetNumFmtFromStack( *aRegion.GetPoint(), *pTxtNode );
                        if (!pNum)
                        {
                            pNum = GetNumFmtFromTxtNode(*pTxtNode);
                        }

                        if ( pNum )
                        {
                            
                            const bool bFirstLineIndentSet =
                                ( rReader.maTxtNodesHavingFirstLineOfstSet.end() !=
                                    rReader.maTxtNodesHavingFirstLineOfstSet.find( pNode ) );
                            
                            const bool bLeftIndentSet =
                                (  rReader.maTxtNodesHavingLeftIndentSet.end() !=
                                    rReader.maTxtNodesHavingLeftIndentSet.find( pNode ) );
                            SyncIndentWithList( aNewLR, *pNum,
                                                bFirstLineIndentSet,
                                                bLeftIndentSet );
                        }

                        if (aNewLR == aOldLR)
                            continue;

                        pNd->SetAttr(aNewLR);

                    }
                }
            }
            break;

        case RES_TXTATR_FIELD:
            OSL_ENSURE(!this, "What is a field doing in the control stack,"
                "probably should have been in the endstack");
            break;

        case RES_TXTATR_ANNOTATION:
            OSL_ENSURE(!this, "What is a annotation doing in the control stack,"
                "probably should have been in the endstack");
            break;

        case RES_TXTATR_INPUTFIELD:
            OSL_ENSURE(!this, "What is a input field doing in the control stack,"
                "probably should have been in the endstack");
            break;

        case RES_TXTATR_INETFMT:
            {
                SwPaM aRegion(rTmpPos);
                if (rEntry.MakeRegion(pDoc, aRegion, false))
                {
                    SwFrmFmt *pFrm;
                    
                    
                    
                    if (0 != (pFrm = rReader.ContainsSingleInlineGraphic(aRegion)))
                    {
                        const SwFmtINetFmt *pAttr = (const SwFmtINetFmt *)
                            rEntry.pAttr;
                        SwFmtURL aURL;
                        aURL.SetURL(pAttr->GetValue(), false);
                        aURL.SetTargetFrameName(pAttr->GetTargetFrame());
                        pFrm->SetFmtAttr(aURL);
                    }
                    else
                    {
                        pDoc->InsertPoolItem(aRegion, *rEntry.pAttr, 0);
                    }
                }
            }
            break;
        default:
            SwFltControlStack::SetAttrInDoc(rTmpPos, rEntry);
            break;
    }
}

const SfxPoolItem* SwWW8FltControlStack::GetFmtAttr(const SwPosition& rPos,
    sal_uInt16 nWhich)
{
    const SfxPoolItem *pItem = GetStackAttr(rPos, nWhich);
    if (!pItem)
    {
        SwCntntNode const*const pNd = rPos.nNode.GetNode().GetCntntNode();
        if (!pNd)
            pItem = &pDoc->GetAttrPool().GetDefaultItem(nWhich);
        else
        {
            /*
            If we're hunting for the indent on a paragraph and need to use the
            parent style indent, then return the indent in msword format, and
            not writer format, because that's the style that the filter works
            in (naturally)
            */
            if (nWhich == RES_LR_SPACE)
            {
                SfxItemState eState = SFX_ITEM_DEFAULT;
                if (const SfxItemSet *pSet = pNd->GetpSwAttrSet())
                    eState = pSet->GetItemState(RES_LR_SPACE, false);
                if (eState != SFX_ITEM_SET && rReader.nAktColl < rReader.vColl.size())
                    pItem = &(rReader.vColl[rReader.nAktColl].maWordLR);
            }

            /*
            If we're hunting for a character property, try and exact position
            within the text node for lookup
            */
            if (pNd->IsTxtNode())
            {
                const sal_Int32 nPos = rPos.nContent.GetIndex();
                SfxItemSet aSet(pDoc->GetAttrPool(), nWhich, nWhich);
                if (static_cast<const SwTxtNode*>(pNd)->GetAttr(aSet, nPos, nPos))
                    pItem = aSet.GetItem(nWhich);
            }

            if (!pItem)
                pItem = &pNd->GetAttr(nWhich);
        }
    }
    return pItem;
}

const SfxPoolItem* SwWW8FltControlStack::GetStackAttr(const SwPosition& rPos,
    sal_uInt16 nWhich)
{
    SwFltPosition aFltPos(rPos);

    size_t nSize = size();
    while (nSize)
    {
        const SwFltStackEntry& rEntry = (*this)[ --nSize ];
        if (rEntry.pAttr->Which() == nWhich)
        {
            if ( (rEntry.bOpen) ||
                 (
                  (rEntry.m_aMkPos.m_nNode <= aFltPos.m_nNode) &&
                  (rEntry.m_aPtPos.m_nNode >= aFltPos.m_nNode) &&
                  (rEntry.m_aMkPos.m_nCntnt <= aFltPos.m_nCntnt) &&
                  (rEntry.m_aPtPos.m_nCntnt > aFltPos.m_nCntnt)
                 )
               )
                /*
                 * e.g. half-open range [0-3) so asking for properties at 3
                 * means props that end at 3 are not included
                 */
            {
                return rEntry.pAttr;
            }
        }
    }
    return 0;
}

bool SwWW8FltRefStack::IsFtnEdnBkmField(
    const SwFmtFld& rFmtFld,
    sal_uInt16& rBkmNo)
{
    const SwField* pFld = rFmtFld.GetField();
    sal_uInt16 nSubType;
    if(pFld && (RES_GETREFFLD == pFld->Which())
        && ((REF_FOOTNOTE == (nSubType = pFld->GetSubType())) || (REF_ENDNOTE  == nSubType))
        && !((SwGetRefField*)pFld)->GetSetRefName().isEmpty())
    {
        const IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
        IDocumentMarkAccess::const_iterator_t ppBkmk =
            pMarkAccess->findMark( ((SwGetRefField*)pFld)->GetSetRefName() );
        if(ppBkmk != pMarkAccess->getAllMarksEnd())
        {
            
            rBkmNo = ppBkmk - pMarkAccess->getAllMarksBegin();
            return true;
        }
    }
    return false;
}

void SwWW8FltRefStack::SetAttrInDoc(const SwPosition& rTmpPos,
    SwFltStackEntry& rEntry)
{
    switch (rEntry.pAttr->Which())
    {
        /*
        Look up these in our lists of bookmarks that were changed to
        variables, and replace the ref field with a var field, otherwise
        do normal (?) strange stuff
        */
        case RES_TXTATR_FIELD:
        case RES_TXTATR_ANNOTATION:
        case RES_TXTATR_INPUTFIELD:
        {
            SwNodeIndex aIdx(rEntry.m_aMkPos.m_nNode, 1);
            SwPaM aPaM(aIdx, rEntry.m_aMkPos.m_nCntnt);

            SwFmtFld& rFmtFld   = *(SwFmtFld*)rEntry.pAttr;
            SwField* pFld = rFmtFld.GetField();

            if (!RefToVar(pFld, rEntry))
            {
                sal_uInt16 nBkmNo;
                if( IsFtnEdnBkmField(rFmtFld, nBkmNo) )
                {
                    ::sw::mark::IMark const * const pMark = (pDoc->getIDocumentMarkAccess()->getAllMarksBegin() + nBkmNo)->get();

                    const SwPosition& rBkMrkPos = pMark->GetMarkPos();

                    SwTxtNode* pTxt = rBkMrkPos.nNode.GetNode().GetTxtNode();
                    if( pTxt && rBkMrkPos.nContent.GetIndex() )
                    {
                        SwTxtAttr* const pFtn = pTxt->GetTxtAttrForCharAt(
                            rBkMrkPos.nContent.GetIndex()-1, RES_TXTATR_FTN );
                        if( pFtn )
                        {
                            sal_uInt16 nRefNo = ((SwTxtFtn*)pFtn)->GetSeqRefNo();

                            ((SwGetRefField*)pFld)->SetSeqNo( nRefNo );

                            if( pFtn->GetFtn().IsEndNote() )
                                ((SwGetRefField*)pFld)->SetSubType(REF_ENDNOTE);
                        }
                    }
                }
            }

            pDoc->InsertPoolItem(aPaM, *rEntry.pAttr, 0);
            MoveAttrs(*aPaM.GetPoint());
        }
        break;
        case RES_FLTR_TOX:
            SwFltEndStack::SetAttrInDoc(rTmpPos, rEntry);
            break;
        default:
        case RES_FLTR_BOOKMARK:
            OSL_ENSURE(!this, "EndStck used with non field, not what we want");
            SwFltEndStack::SetAttrInDoc(rTmpPos, rEntry);
            break;
    }
}

/*
 For styles we will do our tabstop arithmetic in word style and adjust them to
 writer style after all the styles have been finished and the dust settles as
 to what affects what.

 For explicit attributes we turn the adjusted writer tabstops back into 0 based
 word indexes and we'll turn them back into writer indexes when setting them
 into the document. If explicit left indent exist which affects them, then this
 is handled when the explicit left indent is set into the document
*/
void SwWW8ImplReader::Read_Tab(sal_uInt16 , const sal_uInt8* pData, short nLen)
{
    if (nLen < 0)
    {
        pCtrlStck->SetAttr(*pPaM->GetPoint(), RES_PARATR_TABSTOP);
        return;
    }

    sal_uInt8 nDel = (nLen > 0) ? pData[0] : 0;
    const sal_uInt8* pDel = pData + 1;                   

    sal_uInt8 nIns = (nLen > nDel*2+1) ? pData[nDel*2+1] : 0;
    const sal_uInt8* pIns = pData + 2*nDel + 2;          

    short nRequiredLength = 2 + 2*nDel + 2*nIns + 1*nIns;
    if (nRequiredLength > nLen)
    {
        
        
        nIns = 0;
        nDel = 0;
    }

    WW8_TBD* pTyp = (WW8_TBD*)(pData + 2*nDel + 2*nIns + 2); 

    SvxTabStopItem aAttr(0, 0, SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP);

    const SwTxtFmtColl* pSty = 0;
    sal_uInt16 nTabBase;
    if (pAktColl && nAktColl < vColl.size()) 
    {
        nTabBase = vColl[nAktColl].nBase;
        if (nTabBase < vColl.size())  
            pSty = (const SwTxtFmtColl*)vColl[nTabBase].pFmt;
    }
    else
    { 
        nTabBase = nAktColl;
        if (nAktColl < vColl.size())
            pSty = (const SwTxtFmtColl*)vColl[nAktColl].pFmt;
        
    }

    bool bFound = false;
    ::boost::unordered_set<size_t> aLoopWatch;
    while (pSty && !bFound)
    {
        const SfxPoolItem* pTabs;
        bFound = pSty->GetAttrSet().GetItemState(RES_PARATR_TABSTOP, false,
            &pTabs) == SFX_ITEM_SET;
        if( bFound )
            aAttr = *((const SvxTabStopItem*)pTabs);
        else
        {
            sal_uInt16 nOldTabBase = nTabBase;
            
            if (nTabBase < vColl.size())
                nTabBase = vColl[nTabBase].nBase;

            if (
                    nTabBase < vColl.size() &&
                    nOldTabBase != nTabBase &&
                    nTabBase != ww::stiNil
               )
            {
                
                
                aLoopWatch.insert(reinterpret_cast<size_t>(pSty));
                if (nTabBase < vColl.size())
                    pSty = (const SwTxtFmtColl*)vColl[nTabBase].pFmt;
                

                if (aLoopWatch.find(reinterpret_cast<size_t>(pSty)) !=
                    aLoopWatch.end())
                    pSty = 0;
            }
            else
                pSty = 0; 
        }
    }

    SvxTabStop aTabStop;
    for (short i=0; i < nDel; ++i)
    {
        sal_uInt16 nPos = aAttr.GetPos(SVBT16ToShort(pDel + i*2));
        if( nPos != SVX_TAB_NOTFOUND )
            aAttr.Remove( nPos, 1 );
    }

    for (short i=0; i < nIns; ++i)
    {
        short nPos = SVBT16ToShort(pIns + i*2);
        aTabStop.GetTabPos() = nPos;
        switch( pTyp[i].aBits1 & 0x7 ) 
        {
            case 0:
                aTabStop.GetAdjustment() = SVX_TAB_ADJUST_LEFT;
                break;
            case 1:
                aTabStop.GetAdjustment() = SVX_TAB_ADJUST_CENTER;
                break;
            case 2:
                aTabStop.GetAdjustment() = SVX_TAB_ADJUST_RIGHT;
                break;
            case 3:
                aTabStop.GetAdjustment() = SVX_TAB_ADJUST_DECIMAL;
                break;
            case 4:
                continue; 
        }

        switch( pTyp[i].aBits1 >> 3 & 0x7 )
        {
            case 0:
                aTabStop.GetFill() = ' ';
                break;
            case 1:
                aTabStop.GetFill() = '.';
                break;
            case 2:
                aTabStop.GetFill() = '-';
                break;
            case 3:
            case 4:
                aTabStop.GetFill() = '_';
                break;
        }

        sal_uInt16 nPos2 = aAttr.GetPos( nPos );
        if (nPos2 != SVX_TAB_NOTFOUND)
            aAttr.Remove(nPos2, 1); 
        aAttr.Insert(aTabStop);
    }

    if (nIns || nDel)
        NewAttr(aAttr);
    else
    {
        
        
        
        
        
        
        
        if (!pAktColl) 
        {
            using namespace sw::util;
            SvxTabStopItem aOrig = pSty ?
            ItemGet<SvxTabStopItem>(*pSty, RES_PARATR_TABSTOP) :
            DefaultItemGet<SvxTabStopItem>(rDoc, RES_PARATR_TABSTOP);
            NewAttr(aOrig);
        }
    }
}

/**
 * DOP
*/
void SwWW8ImplReader::ImportDop()
{
    
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
        mpDocShell->GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocuProps(
        xDPS->getDocumentProperties());
    OSL_ENSURE(xDocuProps.is(), "DocumentProperties is null");
    if (xDocuProps.is())
    {
        DateTime aLastPrinted(
            msfilter::util::DTTM2DateTime(pWDop->dttmLastPrint));
       ::util::DateTime uDT(aLastPrinted.GetNanoSec(),
            aLastPrinted.GetSec(), aLastPrinted.GetMin(),
            aLastPrinted.GetHour(), aLastPrinted.GetDay(),
            aLastPrinted.GetMonth(), aLastPrinted.GetYear(), false);
        xDocuProps->setPrintDate(uDT);
    }

    //
    
    //

    
    
    rDoc.Setn32DummyCompatabilityOptions1( pWDop->GetCompatabilityOptions());
    rDoc.Setn32DummyCompatabilityOptions2( pWDop->GetCompatabilityOptions2());

    
    
    rDoc.set(IDocumentSettingAccess::PARA_SPACE_MAX, pWDop->fDontUseHTMLAutoSpacing);
    rDoc.set(IDocumentSettingAccess::PARA_SPACE_MAX_AT_PAGES, true );
    
    rDoc.set(IDocumentSettingAccess::TAB_COMPAT, true);
    
    rDoc.set(IDocumentSettingAccess::TABS_RELATIVE_TO_INDENT, false);

    
    long nDefTabSiz = pWDop->dxaTab;
    if( nDefTabSiz < 56 )
        nDefTabSiz = 709;

    
    SvxTabStopItem aNewTab( 1, sal_uInt16(nDefTabSiz), SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP );
    ((SvxTabStop&)aNewTab[0]).GetAdjustment() = SVX_TAB_ADJUST_DEFAULT;

    rDoc.GetAttrPool().SetPoolDefaultItem( aNewTab );

    
    if (pWDop->wScaleSaved)
    {
        uno::Sequence<beans::PropertyValue> aViewProps(3);
        aViewProps[0].Name = "ZoomFactor";
        aViewProps[0].Value <<= sal_Int16(pWDop->wScaleSaved);
        aViewProps[1].Name = "VisibleBottom";
        aViewProps[1].Value <<= sal_Int32(0);
        aViewProps[2].Name = "ZoomType";
        aViewProps[2].Value <<= sal_Int16(0);

        uno::Reference< uno::XComponentContext > xComponentContext(comphelper::getProcessComponentContext());
        uno::Reference<container::XIndexContainer> xBox = document::IndexedPropertyValues::create(xComponentContext);
        xBox->insertByIndex(sal_Int32(0), uno::makeAny(aViewProps));
        uno::Reference<container::XIndexAccess> xIndexAccess(xBox, uno::UNO_QUERY);
        uno::Reference<document::XViewDataSupplier> xViewDataSupplier(mpDocShell->GetModel(), uno::UNO_QUERY);
        xViewDataSupplier->setViewData(xIndexAccess);
    }

    rDoc.set(IDocumentSettingAccess::USE_VIRTUAL_DEVICE, !pWDop->fUsePrinterMetrics);
    rDoc.set(IDocumentSettingAccess::USE_HIRES_VIRTUAL_DEVICE, true);
    rDoc.set(IDocumentSettingAccess::ADD_FLY_OFFSETS, true );
    rDoc.set(IDocumentSettingAccess::ADD_EXT_LEADING, !pWDop->fNoLeading);
    rDoc.set(IDocumentSettingAccess::OLD_NUMBERING, false);
    rDoc.set(IDocumentSettingAccess::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING, false); 
    rDoc.set(IDocumentSettingAccess::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK, !pWDop->fExpShRtn); 
    rDoc.set(IDocumentSettingAccess::DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT, false);  
    rDoc.set(IDocumentSettingAccess::OLD_LINE_SPACING, false);

    
    
    rDoc.set(IDocumentSettingAccess::ADD_PARA_SPACING_TO_TABLE_CELLS, true);

    
    
    rDoc.set(IDocumentSettingAccess::USE_FORMER_OBJECT_POS, false);

    
    
    rDoc.set(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION, true);

    rDoc.set(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING, false); 

    rDoc.set(IDocumentSettingAccess::TABLE_ROW_KEEP, true); 

    rDoc.set(IDocumentSettingAccess::IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION, true); 

    rDoc.set(IDocumentSettingAccess::INVERT_BORDER_SPACING, true);
    rDoc.set(IDocumentSettingAccess::COLLAPSE_EMPTY_CELL_PARA, true);
    rDoc.set(IDocumentSettingAccess::TAB_OVERFLOW, true);
    rDoc.set(IDocumentSettingAccess::UNBREAKABLE_NUMBERINGS, true);
    rDoc.set(IDocumentSettingAccess::CLIPPED_PICTURES, true);
    rDoc.set(IDocumentSettingAccess::TAB_OVER_MARGIN, true);
    rDoc.set(IDocumentSettingAccess::SURROUND_TEXT_WRAP_SMALL, true);

    //
    
    //

    
    if (pWwFib->nFib > 105)
        ImportDopTypography(pWDop->doptypography);

    
    
    {
        using namespace com::sun::star;

        uno::Reference<lang::XComponent> xModelComp(mpDocShell->GetModel(),
           uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xDocProps(xModelComp,
           uno::UNO_QUERY);
        if (xDocProps.is())
        {
            uno::Reference<beans::XPropertySetInfo> xInfo =
                xDocProps->getPropertySetInfo();
            sal_Bool bValue = false;
            if (xInfo.is())
            {
                if (xInfo->hasPropertyByName("ApplyFormDesignMode"))
                {
                    xDocProps->setPropertyValue("ApplyFormDesignMode", cppu::bool2any(bValue));
                }
            }
        }
    }

    mpDocShell->SetModifyPasswordHash(pWDop->lKeyProtDoc);

    const SvtFilterOptions& rOpt = SvtFilterOptions::Get();
    if (rOpt.IsUseEnhancedFields())
        rDoc.set(IDocumentSettingAccess::PROTECT_FORM, pWDop->fProtEnabled );
}

void SwWW8ImplReader::ImportDopTypography(const WW8DopTypography &rTypo)
{
    using namespace com::sun::star;
    switch (rTypo.iLevelOfKinsoku)
    {
        case 2: 
            {
                i18n::ForbiddenCharacters aForbidden(rTypo.rgxchFPunct,
                    rTypo.rgxchLPunct);
                rDoc.setForbiddenCharacters(rTypo.GetConvertedLang(),
                        aForbidden);
                
                
                if (rTypo.GetConvertedLang() == LANGUAGE_JAPANESE)
                    return;
            }
            break;
        default:
            break;
    }

    /*
    This MS hack means that level 2 of japanese is not in operation, so we put
    in what we know are the MS defaults, there is a complementary reverse
    hack in the writer. Its our default as well, but we can set it anyway
    as a flag for later.
    */
    if (!rTypo.reserved2)
    {
        i18n::ForbiddenCharacters aForbidden(rTypo.GetJapanNotBeginLevel1(),
            rTypo.GetJapanNotEndLevel1());
        rDoc.setForbiddenCharacters(LANGUAGE_JAPANESE,aForbidden);
    }

    rDoc.set(IDocumentSettingAccess::KERN_ASIAN_PUNCTUATION, rTypo.fKerningPunct);
    rDoc.setCharacterCompressionType(static_cast<SwCharCompressType>(rTypo.iJustification));
}

/**
 * Footnotes and Endnotes
 */
WW8ReaderSave::WW8ReaderSave(SwWW8ImplReader* pRdr ,WW8_CP nStartCp) :
    maTmpPos(*pRdr->pPaM->GetPoint()),
    mpOldStck(pRdr->pCtrlStck),
    mpOldAnchorStck(pRdr->pAnchorStck),
    mpOldRedlines(pRdr->mpRedlineStack),
    mpOldPlcxMan(pRdr->pPlcxMan),
    mpWFlyPara(pRdr->pWFlyPara),
    mpSFlyPara(pRdr->pSFlyPara),
    mpPreviousNumPaM(pRdr->pPreviousNumPaM),
    mpPrevNumRule(pRdr->pPrevNumRule),
    mpTableDesc(pRdr->pTableDesc),
    mnInTable(pRdr->nInTable),
    mnAktColl(pRdr->nAktColl),
    mcSymbol(pRdr->cSymbol),
    mbIgnoreText(pRdr->bIgnoreText),
    mbSymbol(pRdr->bSymbol),
    mbHdFtFtnEdn(pRdr->bHdFtFtnEdn),
    mbTxbxFlySection(pRdr->bTxbxFlySection),
    mbAnl(pRdr->bAnl),
    mbInHyperlink(pRdr->bInHyperlink),
    mbPgSecBreak(pRdr->bPgSecBreak),
    mbWasParaEnd(pRdr->bWasParaEnd),
    mbHasBorder(pRdr->bHasBorder),
    mbFirstPara(pRdr->bFirstPara)
{
    pRdr->bSymbol = false;
    pRdr->bHdFtFtnEdn = true;
    pRdr->bTxbxFlySection = pRdr->bAnl = pRdr->bPgSecBreak = pRdr->bWasParaEnd
        = pRdr->bHasBorder = false;
    pRdr->bFirstPara = true;
    pRdr->nInTable = 0;
    pRdr->pWFlyPara = 0;
    pRdr->pSFlyPara = 0;
    pRdr->pPreviousNumPaM = 0;
    pRdr->pPrevNumRule = 0;
    pRdr->pTableDesc = 0;
    pRdr->nAktColl = 0;


    pRdr->pCtrlStck = new SwWW8FltControlStack(&pRdr->rDoc, pRdr->nFieldFlags,
        *pRdr);

    pRdr->mpRedlineStack = new sw::util::RedlineStack(pRdr->rDoc);

    pRdr->pAnchorStck = new SwWW8FltAnchorStack(&pRdr->rDoc, pRdr->nFieldFlags);

    
    
    if (pRdr->pPlcxMan)
        pRdr->pPlcxMan->SaveAllPLCFx(maPLCFxSave);

    if (nStartCp != -1)
    {
        pRdr->pPlcxMan = new WW8PLCFMan(pRdr->pSBase,
            mpOldPlcxMan->GetManType(), nStartCp);
    }

    maOldApos.push_back(false);
    maOldApos.swap(pRdr->maApos);
    maOldFieldStack.swap(pRdr->maFieldStack);
}

void WW8ReaderSave::Restore( SwWW8ImplReader* pRdr )
{
    pRdr->pWFlyPara = mpWFlyPara;
    pRdr->pSFlyPara = mpSFlyPara;
    pRdr->pPreviousNumPaM = mpPreviousNumPaM;
    pRdr->pPrevNumRule = mpPrevNumRule;
    pRdr->pTableDesc = mpTableDesc;
    pRdr->cSymbol = mcSymbol;
    pRdr->bSymbol = mbSymbol;
    pRdr->bIgnoreText = mbIgnoreText;
    pRdr->bHdFtFtnEdn = mbHdFtFtnEdn;
    pRdr->bTxbxFlySection = mbTxbxFlySection;
    pRdr->nInTable = mnInTable;
    pRdr->bAnl = mbAnl;
    pRdr->bInHyperlink = mbInHyperlink;
    pRdr->bWasParaEnd = mbWasParaEnd;
    pRdr->bPgSecBreak = mbPgSecBreak;
    pRdr->nAktColl = mnAktColl;
    pRdr->bHasBorder = mbHasBorder;
    pRdr->bFirstPara = mbFirstPara;

    
    pRdr->DeleteCtrlStk();
    pRdr->pCtrlStck = mpOldStck;

    pRdr->mpRedlineStack->closeall(*pRdr->pPaM->GetPoint());
    delete pRdr->mpRedlineStack;
    pRdr->mpRedlineStack = mpOldRedlines;

    pRdr->DeleteAnchorStk();
    pRdr->pAnchorStck = mpOldAnchorStck;

    *pRdr->pPaM->GetPoint() = maTmpPos;

    if (mpOldPlcxMan != pRdr->pPlcxMan)
    {
        delete pRdr->pPlcxMan;
        pRdr->pPlcxMan = mpOldPlcxMan;
    }
    if (pRdr->pPlcxMan)
        pRdr->pPlcxMan->RestoreAllPLCFx(maPLCFxSave);
    pRdr->maApos.swap(maOldApos);
    pRdr->maFieldStack.swap(maOldFieldStack);
}

void SwWW8ImplReader::Read_HdFtFtnText( const SwNodeIndex* pSttIdx,
    long nStartCp, long nLen, ManTypes nType )
{
    
    WW8ReaderSave aSave( this );

    pPaM->GetPoint()->nNode = pSttIdx->GetIndex() + 1;
    pPaM->GetPoint()->nContent.Assign( pPaM->GetCntntNode(), 0 );

    
    ReadText( nStartCp, nLen, nType ); 
    aSave.Restore( this );
}

/**
 * Use authornames, if not available fall back to initials.
 */
long SwWW8ImplReader::Read_And(WW8PLCFManResult* pRes)
{
    WW8PLCFx_SubDoc* pSD = pPlcxMan->GetAtn();
    if( !pSD )
        return 0;

    OUString sAuthor;
    OUString sInitials;
    if( bVer67 )
    {
        const WW67_ATRD* pDescri = (const WW67_ATRD*)pSD->GetData();
        const OUString* pA = GetAnnotationAuthor(SVBT16ToShort(pDescri->ibst));
        if (pA)
            sAuthor = *pA;
        else
            sAuthor = OUString(pDescri->xstUsrInitl + 1, pDescri->xstUsrInitl[0],
                RTL_TEXTENCODING_MS_1252);
    }
    else
    {
        const WW8_ATRD* pDescri = (const WW8_ATRD*)pSD->GetData();

        {
            const sal_uInt16 nLen = SVBT16ToShort(pDescri->xstUsrInitl[0]);
            OUStringBuffer aBuf;
            aBuf.setLength(nLen);
            for(sal_uInt16 nIdx = 1; nIdx <= nLen; ++nIdx)
                aBuf[nIdx-1] = SVBT16ToShort(pDescri->xstUsrInitl[nIdx]);
            sInitials = aBuf.makeStringAndClear();
        }

        if (const OUString* pA = GetAnnotationAuthor(SVBT16ToShort(pDescri->ibst)))
            sAuthor = *pA;
        else
            sAuthor = sInitials;

        
        sal_uInt32 nTagBkmk = SVBT32ToUInt32(pDescri->ITagBkmk);
        if (nTagBkmk != 0xFFFFFFFF)
        {
            int nAtnIndex = GetAnnotationIndex(nTagBkmk);
            if (nAtnIndex != -1)
            {
                WW8_CP nStart = GetAnnotationStart(nAtnIndex);
                WW8_CP nEnd = GetAnnotationEnd(GetAnnotationEndIndex(nAtnIndex));
                sal_Int32 nLen = nEnd - nStart;
                if( nLen )
                 {
                    if (pPaM->GetPoint()->nContent.GetIndex() >= nLen)
                    {
                        pPaM->SetMark();
                        pPaM->GetPoint()->nContent -= nLen;
                    }
                    else if (pPaM->GetPoint()->nNode.GetNode().IsTxtNode() )
                    {
                        pPaM->SetMark();
                        nLen -= pPaM->GetPoint()->nContent.GetIndex();

                        SwTxtNode* pTxtNode = 0;
                        
                        while( pPaM->GetPoint()->nNode >= 0 )
                        {
                            SwNode* pNode = 0;
                            
                            do
                            {
                                pPaM->GetPoint()->nNode--;
                                nLen--; 
                                pNode = &pPaM->GetPoint()->nNode.GetNode();
                            }
                            while( !pNode->IsTxtNode() && pPaM->GetPoint()->nNode >= 0 );

                            
                            if( pNode->IsTxtNode() )
                            {
                                pTxtNode = pNode->GetTxtNode();
                                if( nLen < pTxtNode->Len() )
                                    break;
                                else
                                    nLen -= pTxtNode->Len();
                            }
                        }

                        
                        if( pTxtNode )
                        {
                            pTxtNode->MakeStartIndex(&pPaM->GetPoint()->nContent);
                            pPaM->GetPoint()->nContent += pTxtNode->Len() - nLen;
                        }
                    }
                }
            }
        }
    }

    sal_uInt32 nDateTime = 0;

    if (sal_uInt8 * pExtended = pPlcxMan->GetExtendedAtrds()) 
    {
        sal_uLong nIndex = pSD->GetIdx() & 0xFFFF; 
        if (pWwFib->lcbAtrdExtra/18 > nIndex)
            nDateTime = SVBT32ToUInt32(*(SVBT32*)(pExtended+(nIndex*18)));
    }

    DateTime aDate = msfilter::util::DTTM2DateTime(nDateTime);

    OUString sTxt;
    OutlinerParaObject *pOutliner = ImportAsOutliner( sTxt, pRes->nCp2OrIdx,
        pRes->nCp2OrIdx + pRes->nMemLen, MAN_AND );

    this->pFmtOfJustInsertedApo = 0;
    SwPostItField aPostIt(
        (SwPostItFieldType*)rDoc.GetSysFldType(RES_POSTITFLD), sAuthor,
        sTxt, sInitials, OUString(), aDate );
    aPostIt.SetTextObject(pOutliner);

    SwPaM aEnd(*pPaM->End(), *pPaM->End());
    pCtrlStck->NewAttr(*aEnd.GetPoint(), SvxCharHiddenItem(false, RES_CHRATR_HIDDEN));
    rDoc.InsertPoolItem(aEnd, SwFmtFld(aPostIt), 0);
    pCtrlStck->SetAttr(*aEnd.GetPoint(), RES_CHRATR_HIDDEN);

    
    if (pPaM->HasMark())
    {
        IDocumentMarkAccess* pMarksAccess = rDoc.getIDocumentMarkAccess();
        pMarksAccess->makeAnnotationMark(*pPaM, aPostIt.GetName());
        pPaM->Exchange();
        pPaM->DeleteMark();
    }

    return 0;
}

void SwWW8ImplReader::Read_HdFtTextAsHackedFrame(long nStart, long nLen,
    SwFrmFmt &rHdFtFmt, sal_uInt16 nPageWidth)
{
    const SwNodeIndex* pSttIdx = rHdFtFmt.GetCntnt().GetCntntIdx();
    OSL_ENSURE(pSttIdx, "impossible");
    if (!pSttIdx)
        return;

    SwPosition aTmpPos(*pPaM->GetPoint());

    pPaM->GetPoint()->nNode = pSttIdx->GetIndex() + 1;
    pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(), 0);

    SwFlyFrmFmt *pFrame = rDoc.MakeFlySection(FLY_AT_PARA, pPaM->GetPoint());

    SwFmtAnchor aAnch( pFrame->GetAnchor() );
    aAnch.SetType( FLY_AT_PARA );
    pFrame->SetFmtAttr( aAnch );
    SwFmtFrmSize aSz(ATT_MIN_SIZE, nPageWidth, MINLAY);
    SwFrmSize eFrmSize = ATT_MIN_SIZE;
    if( eFrmSize != aSz.GetWidthSizeType() )
        aSz.SetWidthSizeType( eFrmSize );
    pFrame->SetFmtAttr(aSz);
    pFrame->SetFmtAttr(SwFmtSurround(SURROUND_THROUGHT));
    pFrame->SetFmtAttr(SwFmtHoriOrient(0, text::HoriOrientation::LEFT)); 

    
    pFrame->SetFmtAttr( SvxOpaqueItem( RES_OPAQUE, false ) );
    SdrObject* pFrmObj = CreateContactObject( pFrame );
    OSL_ENSURE( pFrmObj,
            "<SwWW8ImplReader::Read_HdFtTextAsHackedFrame(..)> - missing SdrObject instance" );
    if ( pFrmObj )
    {
        pFrmObj->SetOrdNum( 0L );
    }
    MoveInsideFly(pFrame);

    const SwNodeIndex* pHackIdx = pFrame->GetCntnt().GetCntntIdx();

    Read_HdFtFtnText(pHackIdx, nStart, nLen - 1, MAN_HDFT);

    MoveOutsideFly(pFrame, aTmpPos);
}

void SwWW8ImplReader::Read_HdFtText(long nStart, long nLen, SwFrmFmt* pHdFtFmt)
{
    const SwNodeIndex* pSttIdx = pHdFtFmt->GetCntnt().GetCntntIdx();
    if (!pSttIdx)
        return;

    SwPosition aTmpPos( *pPaM->GetPoint() ); 

    Read_HdFtFtnText(pSttIdx, nStart, nLen - 1, MAN_HDFT);

    *pPaM->GetPoint() = aTmpPos;
}

bool SwWW8ImplReader::isValid_HdFt_CP(WW8_CP nHeaderCP) const
{
    
    return (nHeaderCP < pWwFib->ccpHdr) ? true : false;
}

bool SwWW8ImplReader::HasOwnHeaderFooter(sal_uInt8 nWhichItems, sal_uInt8 grpfIhdt,
    int nSect)
{
    if (pHdFt)
    {
        WW8_CP start;
        long nLen;
        sal_uInt8 nNumber = 5;

        for( sal_uInt8 nI = 0x20; nI; nI >>= 1, nNumber-- )
        {
            if (nI & nWhichItems)
            {
                bool bOk = true;
                if( bVer67 )
                    bOk = ( pHdFt->GetTextPos(grpfIhdt, nI, start, nLen ) && nLen >= 2 );
                else
                {
                    pHdFt->GetTextPosExact( static_cast< short >(nNumber + (nSect+1)*6), start, nLen);
                    bOk = ( 2 <= nLen ) && isValid_HdFt_CP(start);
                }

                if (bOk)
                    return true;
            }
        }
    }
    return false;
}

void SwWW8ImplReader::Read_HdFt(int nSect, const SwPageDesc *pPrev,
    const wwSection &rSection)
{
    sal_uInt8 grpfIhdt = rSection.maSep.grpfIhdt;
    SwPageDesc *pPD = rSection.mpPage;

    if( pHdFt )
    {
        WW8_CP start;
        long nLen;
        sal_uInt8 nNumber = 5;

        for( sal_uInt8 nI = 0x20; nI; nI >>= 1, nNumber-- )
        {
            if (nI & grpfIhdt)
            {
                bool bOk = true;
                if( bVer67 )
                    bOk = ( pHdFt->GetTextPos(grpfIhdt, nI, start, nLen ) && nLen >= 2 );
                else
                {
                    pHdFt->GetTextPosExact( static_cast< short >(nNumber + (nSect+1)*6), start, nLen);
                    bOk = ( 2 <= nLen ) && isValid_HdFt_CP(start);
                }

                bool bUseLeft
                    = (nI & ( WW8_HEADER_EVEN | WW8_FOOTER_EVEN )) ? true: false;
                bool bUseFirst
                    = (nI & ( WW8_HEADER_FIRST | WW8_FOOTER_FIRST )) ? true: false;
                bool bFooter
                    = (nI & ( WW8_FOOTER_EVEN | WW8_FOOTER_ODD | WW8_FOOTER_FIRST )) ? true: false;

                SwFrmFmt& rFmt = bUseLeft ? pPD->GetLeft()
                    : bUseFirst ? pPD->GetFirstMaster()
                    : pPD->GetMaster();

                SwFrmFmt* pHdFtFmt;
                
                bool bNoFirst = !(grpfIhdt & WW8_HEADER_FIRST) && !(grpfIhdt & WW8_FOOTER_FIRST);
                if (bFooter)
                {
                    bIsFooter = true;
                    
                    if (!pPD->GetMaster().GetFooter().GetFooterFmt())
                        pPD->GetMaster().SetFmtAttr(SwFmtFooter(true));
                    if (bUseLeft)
                        pPD->GetLeft().SetFmtAttr(SwFmtFooter(true));
                    if (bUseFirst || (rSection.maSep.fTitlePage && bNoFirst))
                        pPD->GetFirstMaster().SetFmtAttr(SwFmtFooter(true));
                    pHdFtFmt = const_cast<SwFrmFmt*>(rFmt.GetFooter().GetFooterFmt());
                }
                else
                {
                    bIsHeader = true;
                    
                    if (!pPD->GetMaster().GetHeader().GetHeaderFmt())
                        pPD->GetMaster().SetFmtAttr(SwFmtHeader(true));
                    if (bUseLeft)
                        pPD->GetLeft().SetFmtAttr(SwFmtHeader(true));
                    if (bUseFirst || (rSection.maSep.fTitlePage && bNoFirst))
                        pPD->GetFirstMaster().SetFmtAttr(SwFmtHeader(true));
                    pHdFtFmt = const_cast<SwFrmFmt*>(rFmt.GetHeader().GetHeaderFmt());
                }

                if (bOk)
                {
                    bool bHackRequired = false;
                    if (bIsHeader && rSection.IsFixedHeightHeader())
                        bHackRequired = true;
                    else if (bIsFooter && rSection.IsFixedHeightFooter())
                        bHackRequired = true;

                    if (bHackRequired)
                    {
                        Read_HdFtTextAsHackedFrame(start, nLen, *pHdFtFmt,
                            static_cast< sal_uInt16 >(rSection.GetTextAreaWidth()) );
                    }
                    else
                        Read_HdFtText(start, nLen, pHdFtFmt);
                }
                else if (!bOk && pPrev)
                    CopyPageDescHdFt(pPrev, pPD, nI);

                bIsHeader = bIsFooter = false;
            }
        }
    }
}

bool wwSectionManager::SectionIsProtected(const wwSection &rSection) const
{
    return (mrReader.pWwFib->fReadOnlyRecommended && !rSection.IsNotProtected());
}

void wwSectionManager::SetHdFt(wwSection &rSection, int nSect,
    const wwSection *pPrevious)
{
    
    if (!rSection.maSep.grpfIhdt)
        return;

    OSL_ENSURE(rSection.mpPage, "makes no sense to call with a main page");
    if (rSection.mpPage)
    {
        mrReader.Read_HdFt(nSect, pPrevious ? pPrevious->mpPage : 0,
                rSection);
    }

    
    
    if (mrReader.pHdFt)
        mrReader.pHdFt->UpdateIndex(rSection.maSep.grpfIhdt);

}

void SwWW8ImplReader::AppendTxtNode(SwPosition& rPos)
{
    SwTxtNode* pTxt = pPaM->GetNode()->GetTxtNode();

    const SwNumRule* pRule = NULL;

    if (pTxt != NULL)
        pRule = sw::util::GetNumRuleFromTxtNode(*pTxt);

    if (
         pRule && !pWDop->fDontUseHTMLAutoSpacing &&
         (bParaAutoBefore || bParaAutoAfter)
       )
    {
        
        if (bParaAutoAfter)
            SetLowerSpacing(*pPaM, 0);

        
        
        if(pPrevNumRule && bParaAutoBefore)
            SetUpperSpacing(*pPaM, 0);

        
        
        if((pRule != pPrevNumRule) && pPreviousNumPaM)
            SetLowerSpacing(*pPreviousNumPaM, GetParagraphAutoSpace(pWDop->fDontUseHTMLAutoSpacing));

        
        if(pPreviousNumPaM)
            delete pPreviousNumPaM, pPreviousNumPaM = 0;

        pPreviousNumPaM = new SwPaM(*pPaM);
        pPrevNumRule = pRule;
    }
    else if(!pRule && pPreviousNumPaM)
    {
        
        
        SetLowerSpacing(*pPreviousNumPaM, GetParagraphAutoSpace(pWDop->fDontUseHTMLAutoSpacing));
        delete pPreviousNumPaM, pPreviousNumPaM = 0;
        pPrevNumRule = 0;
    }
    else
    {
        
        if(pPreviousNumPaM)
            delete pPreviousNumPaM, pPreviousNumPaM = 0;
        pPrevNumRule = pRule;
    }

    
    
    
    if(bParaAutoBefore && bFirstPara && !pWDop->fDontUseHTMLAutoSpacing)
        SetUpperSpacing(*pPaM, 0);

    bFirstPara = false;

    rDoc.AppendTxtNode(rPos);

    
    pAnchorStck->Flush();
}

bool SwWW8ImplReader::SetSpacing(SwPaM &rMyPam, int nSpace, bool bIsUpper )
{
        bool bRet = false;
        const SwPosition* pSpacingPos = rMyPam.GetPoint();

        const SvxULSpaceItem* pULSpaceItem = (const SvxULSpaceItem*)pCtrlStck->GetFmtAttr(*pSpacingPos, RES_UL_SPACE);

        if(pULSpaceItem != 0)
        {
            SvxULSpaceItem aUL(*pULSpaceItem);

            if(bIsUpper)
                aUL.SetUpper( static_cast< sal_uInt16 >(nSpace) );
            else
                aUL.SetLower( static_cast< sal_uInt16 >(nSpace) );

            const sal_Int32 nEnd = pSpacingPos->nContent.GetIndex();
            rMyPam.GetPoint()->nContent.Assign(rMyPam.GetCntntNode(), 0);
            pCtrlStck->NewAttr(*pSpacingPos, aUL);
            rMyPam.GetPoint()->nContent.Assign(rMyPam.GetCntntNode(), nEnd);
            pCtrlStck->SetAttr(*pSpacingPos, RES_UL_SPACE);
            bRet = true;
        }
        return bRet;
}

bool SwWW8ImplReader::SetLowerSpacing(SwPaM &rMyPam, int nSpace)
{
    return SetSpacing(rMyPam, nSpace, false);
}

bool SwWW8ImplReader::SetUpperSpacing(SwPaM &rMyPam, int nSpace)
{
    return SetSpacing(rMyPam, nSpace, true);
}

sal_uInt16 SwWW8ImplReader::TabRowSprm(int nLevel) const
{
    if (bVer67)
        return 25;
    return nLevel ? 0x244C : 0x2417;
}

void SwWW8ImplReader::EndSpecial()
{
    
    if (bAnl)
        StopAllAnl(); 

    while(maApos.size() > 1)
    {
        StopTable();
        maApos.pop_back();
        --nInTable;
        if (maApos[nInTable] == true)
            StopApo();
    }

    if (maApos[0] == true)
        StopApo();

    OSL_ENSURE(!nInTable, "unclosed table!");
}

bool SwWW8ImplReader::ProcessSpecial(bool &rbReSync, WW8_CP nStartCp)
{
    
    if (bInHyperlink)
        return false;

    rbReSync = false;

    OSL_ENSURE(nInTable >= 0,"nInTable < 0!");

    
    bool bTableRowEnd = (pPlcxMan->HasParaSprm(bVer67 ? 25 : 0x2417) != 0 );






//


//





//

//









    sal_uInt8 nCellLevel = 0;

    if (bVer67)
        nCellLevel = 0 != pPlcxMan->HasParaSprm(24);
    else
    {
        nCellLevel = 0 != pPlcxMan->HasParaSprm(0x2416);
        if (!nCellLevel)
            nCellLevel = 0 != pPlcxMan->HasParaSprm(0x244B);
    }
    do
    {
        WW8_TablePos *pTabPos=0;
        WW8_TablePos aTabPos;
        if(nCellLevel && !bVer67)
        {
            WW8PLCFxSave1 aSave;
            pPlcxMan->GetPap()->Save( aSave );
            rbReSync = true;
            WW8PLCFx_Cp_FKP* pPap = pPlcxMan->GetPapPLCF();
            WW8_CP nMyStartCp=nStartCp;

            if (const sal_uInt8 *pLevel = pPlcxMan->HasParaSprm(0x6649))
                nCellLevel = *pLevel;

            bool bHasRowEnd = SearchRowEnd(pPap, nMyStartCp, (nInTable<nCellLevel?nInTable:nCellLevel-1));

            
            if (!bHasRowEnd)
                nCellLevel = static_cast< sal_uInt8 >(nInTable);

            if (bHasRowEnd && ParseTabPos(&aTabPos,pPap))
                pTabPos = &aTabPos;

            pPlcxMan->GetPap()->Restore( aSave );
        }

        

        ApoTestResults aApo = TestApo(nCellLevel, bTableRowEnd, pTabPos);

        
        bool bStartTab = (nInTable < nCellLevel) && !bFtnEdn;

        bool bStopTab = bWasTabRowEnd && (nInTable > nCellLevel) && !bFtnEdn;

        bWasTabRowEnd = false;  
                                

        if (nInTable && !bTableRowEnd && !bStopTab && (nInTable == nCellLevel && aApo.HasStartStop()))
            bStopTab = bStartTab = true; 

        
        if( bAnl && !bTableRowEnd )
        {
            const sal_uInt8* pSprm13 = pPlcxMan->HasParaSprm( 13 );
            if( pSprm13 )
            {   
                sal_uInt8 nT = static_cast< sal_uInt8 >(GetNumType( *pSprm13 ));
                if( ( nT != WW8_Pause && nT != nWwNumType ) 
                    || aApo.HasStartStop()                  
                    || bStopTab || bStartTab )
                {
                    StopAnlToRestart(nT);  
                }
                else
                {
                    NextAnlLine( pSprm13 ); 
                }
            }
            else
            {   
                StopAllAnl(); 
            }
        }
        if (bStopTab)
        {
            StopTable();
            maApos.pop_back();
            --nInTable;
        }
        if (aApo.mbStopApo)
        {
            StopApo();
            maApos[nInTable] = false;
        }

        if (aApo.mbStartApo)
        {
            maApos[nInTable] = StartApo(aApo, pTabPos);
            
            
            rbReSync = true;
        }
        if (bStartTab)
        {
            WW8PLCFxSave1 aSave;
            pPlcxMan->GetPap()->Save( aSave );

           
           if (bAnl)
               StopAllAnl();

            if(nInTable < nCellLevel)
            {
                if (StartTable(nStartCp))
                    ++nInTable;
                else
                    break;
                maApos.push_back(false);
            }

            if(nInTable >= nCellLevel)
            {
                
                
                rbReSync = true;
                pPlcxMan->GetPap()->Restore( aSave );
            }
        }
    } while (!bFtnEdn && (nInTable < nCellLevel));
    return bTableRowEnd;
}

rtl_TextEncoding SwWW8ImplReader::GetCurrentCharSet()
{
    /*
    #i2015
    If the hard charset is set use it, if not see if there is an open
    character run that has set the charset, if not then fallback to the
    current underlying paragraph style.
    */
    rtl_TextEncoding eSrcCharSet = eHardCharSet;
    if (eSrcCharSet == RTL_TEXTENCODING_DONTKNOW)
    {
        if (!maFontSrcCharSets.empty())
            eSrcCharSet = maFontSrcCharSets.top();
        if ((eSrcCharSet == RTL_TEXTENCODING_DONTKNOW) && nCharFmt >= 0 && (size_t)nCharFmt < vColl.size() )
            eSrcCharSet = vColl[nCharFmt].GetCharSet();
        if ((eSrcCharSet == RTL_TEXTENCODING_DONTKNOW) && StyleExists(nAktColl) && nAktColl < vColl.size())
            eSrcCharSet = vColl[nAktColl].GetCharSet();
        if (eSrcCharSet == RTL_TEXTENCODING_DONTKNOW)
        {
            /*
             #i22206#/#i52786#
             The (default) character set used for a run of text is the default
             character set for the version of Word that last saved the document.

             This is a bit tentative, more might be required if the concept is correct.
             When later version of word write older 6/95 documents the charset is
             correctly set in the character runs involved, so its hard to reproduce
             documents that require this to be sure of the process involved.
            */
            const SvxLanguageItem *pLang = (const SvxLanguageItem*)GetFmtAttr(RES_CHRATR_LANGUAGE);
            LanguageType eLang = pLang ? pLang->GetLanguage() : LANGUAGE_SYSTEM;
            ::com::sun::star::lang::Locale aLocale(LanguageTag::convertToLocale(eLang));
            eSrcCharSet = msfilter::util::getBestTextEncodingFromLocale(aLocale);
        }
    }
    return eSrcCharSet;
}


rtl_TextEncoding SwWW8ImplReader::GetCurrentCJKCharSet()
{
    /*
    #i2015
    If the hard charset is set use it, if not see if there is an open
    character run that has set the charset, if not then fallback to the
    current underlying paragraph style.
    */
    rtl_TextEncoding eSrcCharSet = eHardCharSet;
    if (eSrcCharSet == RTL_TEXTENCODING_DONTKNOW)
    {
        if (!maFontSrcCJKCharSets.empty())
            eSrcCharSet = maFontSrcCJKCharSets.top();
        if (!vColl.empty())
        {
            if ((eSrcCharSet == RTL_TEXTENCODING_DONTKNOW) && nCharFmt >= 0 && (size_t)nCharFmt < vColl.size() )
                eSrcCharSet = vColl[nCharFmt].GetCJKCharSet();
            if (eSrcCharSet == RTL_TEXTENCODING_DONTKNOW && nAktColl < vColl.size())
                eSrcCharSet = vColl[nAktColl].GetCJKCharSet();
        }
        if (eSrcCharSet == RTL_TEXTENCODING_DONTKNOW)
        { 
            /*
             #i22206#/#i52786#
             The (default) character set used for a run of text is the default
             character set for the version of Word that last saved the document.

             This is a bit tentative, more might be required if the concept is correct.
             When later version of word write older 6/95 documents the charset is
             correctly set in the character runs involved, so its hard to reproduce
             documents that require this to be sure of the process involved.
            */
            const SvxLanguageItem *pLang =
                (const SvxLanguageItem*)GetFmtAttr(RES_CHRATR_LANGUAGE);
            if (pLang)
            {
                switch (pLang->GetLanguage())
                {
                    case LANGUAGE_CZECH:
                        eSrcCharSet = RTL_TEXTENCODING_MS_1250;
                        break;
                    default:
                        eSrcCharSet = RTL_TEXTENCODING_MS_1252;
                        break;
                }
            }
        }
    }
    return eSrcCharSet;
}

void SwWW8ImplReader::PostProcessAttrs()
{
    if (mpPostProcessAttrsInfo != NULL)
    {
        SfxItemIter aIter(mpPostProcessAttrsInfo->mItemSet);

        const SfxPoolItem * pItem = aIter.GetCurItem();
        if (pItem != NULL)
        {
            do
            {
                pCtrlStck->NewAttr(*mpPostProcessAttrsInfo->mPaM.GetPoint(),
                                   *pItem);
                pCtrlStck->SetAttr(*mpPostProcessAttrsInfo->mPaM.GetMark(),
                                   pItem->Which(), true);
            }
            while (!aIter.IsAtEnd() && 0 != (pItem = aIter.NextItem()));
        }

        delete mpPostProcessAttrsInfo;
        mpPostProcessAttrsInfo = NULL;
    }
}

/*
 #i9241#
 It appears that some documents that are in a baltic 8 bit encoding which has
 some undefined characters can have use made of those characters, in which
 case they default to CP1252. If not then its perhaps that the font encoding
 is only in use for 6/7 and for 8+ if we are in 8bit mode then the encoding
 is always 1252.

 So a encoding converter that on an undefined character attempts to
 convert from 1252 on the undefined character
*/
sal_Size Custom8BitToUnicode(rtl_TextToUnicodeConverter hConverter,
    sal_Char *pIn, sal_Size nInLen, sal_Unicode *pOut, sal_Size nOutLen)
{
    const sal_uInt32 nFlags =
        RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR |
        RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR |
        RTL_TEXTTOUNICODE_FLAGS_INVALID_IGNORE |
        RTL_TEXTTOUNICODE_FLAGS_FLUSH;

    const sal_uInt32 nFlags2 =
        RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE |
        RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_IGNORE |
        RTL_TEXTTOUNICODE_FLAGS_INVALID_IGNORE |
        RTL_TEXTTOUNICODE_FLAGS_FLUSH;

    sal_Size nDestChars=0;
    sal_Size nConverted=0;

    do
    {
        sal_uInt32 nInfo = 0;
        sal_Size nThisConverted=0;

        nDestChars += rtl_convertTextToUnicode(hConverter, 0,
            pIn+nConverted, nInLen-nConverted,
            pOut+nDestChars, nOutLen-nDestChars,
            nFlags, &nInfo, &nThisConverted);

        OSL_ENSURE(nInfo == 0, "A character conversion failed!");

        nConverted += nThisConverted;

        if (
            nInfo & RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR ||
            nInfo & RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
           )
        {
            sal_Size nOtherConverted;
            rtl_TextToUnicodeConverter hCP1252Converter =
                rtl_createTextToUnicodeConverter(RTL_TEXTENCODING_MS_1252);
            nDestChars += rtl_convertTextToUnicode(hCP1252Converter, 0,
                pIn+nConverted, 1,
                pOut+nDestChars, nOutLen-nDestChars,
                nFlags2, &nInfo, &nOtherConverted);
            rtl_destroyTextToUnicodeConverter(hCP1252Converter);
            nConverted+=1;
        }
    } while (nConverted < nInLen);

    return nDestChars;
}

bool SwWW8ImplReader::LangUsesHindiNumbers(sal_uInt16 nLang)
{
    bool bResult = false;

    switch (nLang)
    {
        case 0x1401: 
        case 0x3c01: 
        case 0xc01: 
        case 0x801: 
        case 0x2c01: 
        case 0x3401: 
        case 0x3001: 
        case 0x1001: 
        case 0x1801: 
        case 0x2001: 
        case 0x4001: 
        case 0x401: 
        case 0x2801: 
        case 0x1c01: 
        case 0x3801: 
        case 0x2401: 
            bResult = true;
            break;
        default:
            break;
    }

    return bResult;
}

sal_Unicode SwWW8ImplReader::TranslateToHindiNumbers(sal_Unicode nChar)
{
    if (nChar >= 0x0030 && nChar <= 0x0039)
        return nChar + 0x0630;

    return nChar;
}

/**
 * Return value: true for non special chars
 */
bool SwWW8ImplReader::ReadPlainChars(WW8_CP& rPos, sal_Int32 nEnd, sal_Int32 nCpOfs)
{
    sal_Int32 nRequestedStrLen = nEnd - rPos;

    OSL_ENSURE(nRequestedStrLen, "String is 0");
    if (nRequestedStrLen <= 0)
        return true;

    sal_Int32 nRequestedPos = pSBase->WW8Cp2Fc(nCpOfs+rPos, &bIsUnicode);
    bool bValidPos = checkSeek(*pStrm, nRequestedPos);
    OSL_ENSURE(bValidPos, "Document claimed to have more text than available");
    if (!bValidPos)
    {
        
        rPos+=nRequestedStrLen;
        return true;
    }

    sal_Size nAvailableStrLen = pStrm->remainingSize() / (bIsUnicode ? 2 : 1);
    OSL_ENSURE(nAvailableStrLen, "Document claimed to have more text than available");
    if (!nAvailableStrLen)
    {
        
        rPos+=nRequestedStrLen;
        return true;
    }

    sal_Int32 nValidStrLen = std::min<sal_Size>(nRequestedStrLen, nAvailableStrLen);

    
    
    
    const sal_Int32 nStrLen = std::min(nValidStrLen, SAL_MAX_INT32-1);

    const rtl_TextEncoding eSrcCharSet = bVer67 ? GetCurrentCharSet() :
        RTL_TEXTENCODING_MS_1252;
    const rtl_TextEncoding eSrcCJKCharSet = bVer67 ? GetCurrentCJKCharSet() :
        RTL_TEXTENCODING_MS_1252;

    
    rtl_uString *pStr = rtl_uString_alloc(nStrLen);
    sal_Unicode* pBuffer = pStr->buffer;
    sal_Unicode* pWork = pBuffer;

    sal_Char* p8Bits = NULL;

    rtl_TextToUnicodeConverter hConverter = 0;
    if (!bIsUnicode || bVer67)
        hConverter = rtl_createTextToUnicodeConverter(eSrcCharSet);

    if (!bIsUnicode)
        p8Bits = new sal_Char[nStrLen];

    
    sal_uInt8   nBCode = 0;
    sal_uInt16 nUCode;

    sal_uInt16 nCTLLang = 0;
    const SfxPoolItem * pItem = GetFmtAttr(RES_CHRATR_CTL_LANGUAGE);
    if (pItem != NULL)
        nCTLLang = static_cast<const SvxLanguageItem *>(pItem)->GetLanguage();

    sal_Int32 nL2;
    for( nL2 = 0; nL2 < nStrLen; ++nL2, ++pWork )
    {
        if (bIsUnicode)
            pStrm->ReadUInt16( nUCode ); 
        else
        {
            pStrm->ReadUChar( nBCode ); 
            nUCode = nBCode;
        }

        if (pStrm->GetError())
        {
            rPos = WW8_CP_MAX-10; 
            rtl_freeMemory(pStr);
            delete [] p8Bits;
            return true;
        }

        if ((32 > nUCode) || (0xa0 == nUCode))
        {
            pStrm->SeekRel( bIsUnicode ? -2 : -1 );
            break; 
        }

        if (bIsUnicode)
        {
            if (!bVer67)
                *pWork = nUCode;
            else
            {
                if (nUCode >= 0x3000) 
                {
                    sal_Char aTest[2];
                    aTest[0] = static_cast< sal_Char >((nUCode & 0xFF00) >> 8);
                    aTest[1] = static_cast< sal_Char >(nUCode & 0x00FF);
                    OUString aTemp(aTest, 2, eSrcCJKCharSet);
                    OSL_ENSURE(aTemp.getLength() == 1, "so much for that theory");
                    *pWork = aTemp[0];
                }
                else
                {
                    sal_Char cTest = static_cast< sal_Char >(nUCode & 0x00FF);
                    Custom8BitToUnicode(hConverter, &cTest, 1, pWork, 1);
                }
            }
        }
        else
            p8Bits[nL2] = nBCode;
    }

    if (nL2)
    {
        const sal_Int32 nEndUsed = !bIsUnicode
            ? Custom8BitToUnicode(hConverter, p8Bits, nL2, pBuffer, nStrLen)
            : nL2;

        for( sal_Int32 nI = 0; nI < nStrLen; ++nI, ++pBuffer )
            if (m_bRegardHindiDigits && bBidi && LangUsesHindiNumbers(nCTLLang))
                *pBuffer = TranslateToHindiNumbers(*pBuffer);

        pStr->buffer[nEndUsed] = 0;
        pStr->length = nEndUsed;

        emulateMSWordAddTextToParagraph(OUString(pStr, SAL_NO_ACQUIRE));
        pStr = NULL;
        rPos += nL2;
        if (!maApos.back()) 
            bWasParaEnd = false; 
    }

    if (hConverter)
        rtl_destroyTextToUnicodeConverter(hConverter);
    if (pStr)
        rtl_uString_release(pStr);
    delete [] p8Bits;
    return nL2 >= nStrLen;
}

#define MSASCII SAL_MAX_INT16

namespace
{
    
    sal_Int16 lcl_getScriptType(
        const uno::Reference<i18n::XBreakIterator>& rBI,
        const OUString &rString, sal_Int32 nPos)
    {
        sal_Int16 nScript = rBI->getScriptType(rString, nPos);
        if (nScript == i18n::ScriptType::WEAK && rString[nPos] >= 0x0020 && rString[nPos] <= 0x007F)
            nScript = MSASCII;
        return nScript;
    }

    
    
    sal_Int32 lcl_endOfScript(
        const uno::Reference<i18n::XBreakIterator>& rBI,
        const OUString &rString, sal_Int32 nPos, sal_Int16 nScript)
    {
        while (nPos < rString.getLength())
        {
            sal_Int16 nNewScript = lcl_getScriptType(rBI, rString, nPos);
            if (nScript != nNewScript)
                break;
            ++nPos;
        }
        return nPos;
    }

    sal_Int32 lcl_getWriterScriptType(
        const uno::Reference<i18n::XBreakIterator>& rBI,
        const OUString &rString, sal_Int32 nPos)
    {
        sal_Int16 nScript = i18n::ScriptType::WEAK;

        if (rString.isEmpty())
            return nScript;

        while (nPos >= 0)
        {
            nScript = rBI->getScriptType(rString, nPos);
            if (nScript != i18n::ScriptType::WEAK)
                break;
            --nPos;
        }

        return nScript;
    }

    bool samePitchIgnoreUnknown(FontPitch eA, FontPitch eB)
    {
        return (eA == eB || eA == PITCH_DONTKNOW || eB == PITCH_DONTKNOW);
    }

    bool sameFontIgnoringIrrelevantFields(const SvxFontItem &rA, const SvxFontItem &rB)
    {
        
        return rA.GetFamilyName() == rB.GetFamilyName() &&
            rA.GetStyleName() == rB.GetStyleName() &&
            rA.GetFamily() == rB.GetFamily() &&
            samePitchIgnoreUnknown(rA.GetPitch(), rB.GetPitch());
    }
}





//





//



//






//

void SwWW8ImplReader::emulateMSWordAddTextToParagraph(const OUString& rAddString)
{
    if (rAddString.isEmpty())
        return;

    uno::Reference<i18n::XBreakIterator> xBI(g_pBreakIt->GetBreakIter());
    if (!xBI.is())
    {
        simpleAddTextToParagraph(rAddString);
        return;
    }

    sal_Int16 nScript = lcl_getScriptType(xBI, rAddString, 0);
    sal_Int32 nLen = rAddString.getLength();

    OUString sParagraphText;
    const SwCntntNode *pCntNd = pPaM->GetCntntNode();
    const SwTxtNode* pNd = pCntNd ? pCntNd->GetTxtNode() : NULL;
    if (pNd)
        sParagraphText = pNd->GetTxt();
    sal_Int32 nParaOffset = sParagraphText.getLength();
    sParagraphText = sParagraphText + rAddString;

    sal_Int32 nPos = 0;
    while (nPos < nLen)
    {
        sal_Int32 nEnd = lcl_endOfScript(xBI, rAddString, nPos, nScript);
        if (nEnd < 0)
            break;

        OUString sChunk(rAddString.copy(nPos, nEnd-nPos));
        const sal_uInt16 aIds[] = {RES_CHRATR_FONT, RES_CHRATR_CJK_FONT, RES_CHRATR_CTL_FONT};
        const SvxFontItem *pOverriddenItems[] = {NULL, NULL, NULL};
        bool aForced[] = {false, false, false};

        int nLclIdctHint = 0xFF;
        if (nScript == i18n::ScriptType::WEAK)
            nLclIdctHint = nIdctHint;
        else if (nScript == MSASCII) 
            nLclIdctHint = 0;

        sal_uInt16 nForceFromFontId = 0;
        if (nLclIdctHint != 0xFF)
        {
            switch (nLclIdctHint)
            {
                case 0:
                    nForceFromFontId = RES_CHRATR_FONT;
                    break;
                case 1:
                    nForceFromFontId = RES_CHRATR_CJK_FONT;
                    break;
                case 2:
                    nForceFromFontId = RES_CHRATR_CTL_FONT;
                    break;
                default:
                    break;
            }
        }

        if (nForceFromFontId != 0)
        {
            
            

            sal_Int32 nWriterScript = lcl_getWriterScriptType(xBI, sParagraphText,
                nPos + nParaOffset);

            bool bWriterWillUseSameFontAsWordAutomatically = false;

            if (nWriterScript != i18n::ScriptType::WEAK)
            {
                if (
                     (nWriterScript == i18n::ScriptType::ASIAN && nForceFromFontId == RES_CHRATR_CJK_FONT) ||
                     (nWriterScript == i18n::ScriptType::COMPLEX && nForceFromFontId == RES_CHRATR_CTL_FONT) ||
                     (nWriterScript == i18n::ScriptType::LATIN && nForceFromFontId == RES_CHRATR_FONT)
                   )
                {
                    bWriterWillUseSameFontAsWordAutomatically = true;
                }
                else
                {
                    const SvxFontItem *pSourceFont = (const SvxFontItem*)GetFmtAttr(nForceFromFontId);
                    sal_uInt16 nDestId = aIds[nWriterScript-1];
                    const SvxFontItem *pDestFont = (const SvxFontItem*)GetFmtAttr(nDestId);
                    bWriterWillUseSameFontAsWordAutomatically = sameFontIgnoringIrrelevantFields(*pSourceFont, *pDestFont);
                }
            }

            
            if (!bWriterWillUseSameFontAsWordAutomatically)
            {
                const SvxFontItem *pSourceFont = (const SvxFontItem*)GetFmtAttr(nForceFromFontId);

                for (size_t i = 0; i < SAL_N_ELEMENTS(aIds); ++i)
                {
                    const SvxFontItem *pDestFont = (const SvxFontItem*)GetFmtAttr(aIds[i]);
                    aForced[i] = aIds[i] != nForceFromFontId && *pSourceFont != *pDestFont;
                    if (aForced[i])
                    {
                        pOverriddenItems[i] =
                            (const SvxFontItem*)pCtrlStck->GetStackAttr(*pPaM->GetPoint(), aIds[i]);

                        SvxFontItem aForceFont(*pSourceFont);
                        aForceFont.SetWhich(aIds[i]);
                        pCtrlStck->NewAttr(*pPaM->GetPoint(), aForceFont);
                    }
                }
            }
        }

        simpleAddTextToParagraph(sChunk);

        for (size_t i = 0; i < SAL_N_ELEMENTS(aIds); ++i)
        {
            if (aForced[i])
            {
                pCtrlStck->SetAttr(*pPaM->GetPoint(), aIds[i]);
                if (pOverriddenItems[i])
                    pCtrlStck->NewAttr(*pPaM->GetPoint(), *(pOverriddenItems[i]));
            }
        }

        nPos = nEnd;
        if (nPos < nLen)
            nScript = lcl_getScriptType(xBI, rAddString, nPos);
    }
}

void SwWW8ImplReader::simpleAddTextToParagraph(const OUString& rAddString)
{
    if (rAddString.isEmpty())
        return;

#if OSL_DEBUG_LEVEL > 1
        {
            OString sText(OUStringToOString(rAddString, RTL_TEXTENCODING_UTF8));
            SAL_INFO("sw.ww8", "<addTextToParagraph>" << sText.getStr() << "</addTextToParagraph>");
        }
#endif

    const SwCntntNode *pCntNd = pPaM->GetCntntNode();
    const SwTxtNode* pNd = pCntNd ? pCntNd->GetTxtNode() : NULL;

    OSL_ENSURE(pNd, "What the hell, where's my text node");

    if (!pNd)
        return;

    const sal_Int32 nCharsLeft = SAL_MAX_INT32 - pNd->GetTxt().getLength();
    if (nCharsLeft > 0)
    {
        if (rAddString.getLength() <= nCharsLeft)
        {
            rDoc.InsertString(*pPaM, rAddString);
        }
        else
        {
            rDoc.InsertString(*pPaM, rAddString.copy(0, nCharsLeft));
            AppendTxtNode(*pPaM->GetPoint());
            rDoc.InsertString(*pPaM, rAddString.copy(nCharsLeft));
        }
    }
    else
    {
        AppendTxtNode(*pPaM->GetPoint());
        rDoc.InsertString(*pPaM, rAddString);
    }

    bReadTable = false;
}

/**
 * Return value: true for para end
 */
bool SwWW8ImplReader::ReadChars(WW8_CP& rPos, WW8_CP nNextAttr, long nTextEnd,
    long nCpOfs)
{
    long nEnd = ( nNextAttr < nTextEnd ) ? nNextAttr : nTextEnd;

    if (bSymbol || bIgnoreText)
    {
        if( bSymbol ) 
        {
            for(sal_uInt16 nCh = 0; nCh < nEnd - rPos; ++nCh)
            {
                rDoc.InsertString( *pPaM, OUString(cSymbol) );
            }
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_FONT );
        }
        pStrm->SeekRel( nEnd- rPos );
        rPos = nEnd; 
        return false;
    }

    while (true)
    {
        if (ReadPlainChars(rPos, nEnd, nCpOfs))
            return false; 

        bool bStartLine = ReadChar(rPos, nCpOfs);
        rPos++;
        if (bPgSecBreak || bStartLine || rPos == nEnd) 
        {
            return bStartLine;
        }
    }
}

bool SwWW8ImplReader::HandlePageBreakChar()
{
    bool bParaEndAdded = false;
    
    
    if (!nInTable)
    {
        bool IsTemp=true;
        SwTxtNode* pTemp = pPaM->GetNode()->GetTxtNode();
        if (pTemp && pTemp->GetTxt().isEmpty()
                && (bFirstPara || bFirstParaOfPage))
        {
            IsTemp = false;
            AppendTxtNode(*pPaM->GetPoint());
            pTemp->SetAttr(*GetDfltAttr(RES_PARATR_NUMRULE));
        }

        bPgSecBreak = true;
        pCtrlStck->KillUnlockedAttrs(*pPaM->GetPoint());
        /*
        If it's a 0x0c without a paragraph end before it, act like a
        paragraph end, but nevertheless, numbering (and perhaps other
        similar constructs) do not exist on the para.
        */
        if (!bWasParaEnd && IsTemp)
        {
            bParaEndAdded = true;
            if (0 >= pPaM->GetPoint()->nContent.GetIndex())
            {
                if (SwTxtNode* pTxtNode = pPaM->GetNode()->GetTxtNode())
                {
                    pTxtNode->SetAttr(
                        *GetDfltAttr(RES_PARATR_NUMRULE));
                }
            }
        }
    }
    return bParaEndAdded;
}

bool SwWW8ImplReader::ReadChar(long nPosCp, long nCpOfs)
{
    bool bNewParaEnd = false;
    
    
    
    sal_Size nRequestedPos = pSBase->WW8Cp2Fc(nCpOfs+nPosCp, &bIsUnicode);
    if (!checkSeek(*pStrm, nRequestedPos))
        return false;

    sal_uInt8 nBCode(0);
    sal_uInt16 nWCharVal(0);
    if( bIsUnicode )
        pStrm->ReadUInt16( nWCharVal ); 
    else
    {
        pStrm -> ReadUChar( nBCode ); 
        nWCharVal = nBCode;
    }

    sal_Unicode cInsert = '\x0';
    bool bRet = false;

    if ( 0xc != nWCharVal )
        bFirstParaOfPage = false;

    switch (nWCharVal)
    {
        case 0:
            {
                
                SwPageNumberField aFld(
                    (SwPageNumberFieldType*)rDoc.GetSysFldType(
                    RES_PAGENUMBERFLD ), PG_RANDOM, SVX_NUM_ARABIC);
                rDoc.InsertPoolItem(*pPaM, SwFmtFld(aFld), 0);
            }
            break;
        case 0xe:
            
            if (maSectionManager.CurrentSectionColCount() < 2)
                bRet = HandlePageBreakChar();
            else if (!nInTable)
            {
                
                SwCntntNode *pCntNd=pPaM->GetCntntNode();
                if (pCntNd!=NULL && pCntNd->Len()>0) 
                    AppendTxtNode(*pPaM->GetPoint());
                rDoc.InsertPoolItem(*pPaM, SvxFmtBreakItem(SVX_BREAK_COLUMN_BEFORE, RES_BREAK), 0);
            }
            break;
        case 0x7:
            bNewParaEnd = true;
            TabCellEnd();       
            break;
        case 0xf:
            if( !bSpec )        
                cInsert = '\xa4';
            break;
        case 0x14:
            if( !bSpec )        
                cInsert = '\xb5';
            break;
        case 0x15:
            if( !bSpec )        
            {
                cp_set::iterator aItr = maTOXEndCps.find((WW8_CP)nPosCp);
                if (aItr == maTOXEndCps.end())
                    cInsert = '\xa7';
                else
                    maTOXEndCps.erase(aItr);
            }
            break;
        case 0x9:
            cInsert = '\x9';    
            break;
        case 0xb:
            cInsert = '\xa';    
            break;
        case 0xc:
            bRet = HandlePageBreakChar();
            break;
        case 0x1e:              
            rDoc.InsertString( *pPaM, OUString(CHAR_HARDHYPHEN) );
            break;
        case 0x1f:              
            rDoc.InsertString( *pPaM, OUString(CHAR_SOFTHYPHEN) );
            break;
        case 0xa0:              
            rDoc.InsertString( *pPaM, OUString(CHAR_HARDBLANK)  );
            break;
        case 0x1:
            /*
            Current thinking is that if bObj is set then we have a
            straightforward "traditional" ole object, otherwise we have a
            graphic preview of an associated ole2 object (or a simple
            graphic of course)

            normally in the canvas field, the code is 0x8 0x1.
            in a special case, the code is 0x1 0x1, which yields a simple picture
            */
            {
                bool bReadObj = IsInlineEscherHack();
                if( bReadObj )
                {
                    long nCurPos = pStrm->Tell();
                    sal_uInt16 nWordCode(0);

                    if( bIsUnicode )
                        pStrm->ReadUInt16( nWordCode );
                    else
                    {
                        sal_uInt8 nByteCode(0);
                        pStrm->ReadUChar( nByteCode );
                        nWordCode = nByteCode;
                    }
                    if( nWordCode == 0x1 )
                        bReadObj = false;
                    pStrm->Seek( nCurPos );
                }
                if( !bReadObj )
                {
                    SwFrmFmt *pResult = 0;
                    if (bObj)
                        pResult = ImportOle();
                    else if (bSpec)
                        pResult = ImportGraf();

                    
                    if (!pResult)
                    {
                        cInsert = ' ';
                        OSL_ENSURE(!bObj && !bEmbeddObj && !nObjLocFc,
                            "WW8: Please report this document, it may have a "
                            "missing graphic");
                    }
                    else
                    {
                        
                        bObj = bEmbeddObj = false;
                        nObjLocFc = 0;
                    }
                }
            }
            break;
        case 0x8:
            if( !bObj )
                Read_GrafLayer( nPosCp );
            break;
        case 0xd:
            bNewParaEnd = bRet = true;
            if (nInTable > 1)
            {
                /*
                #i9666#/#i23161#
                Yes complex, if there is an entry in the undocumented PLCF
                which I believe to be a record of cell and row boundaries
                see if the magic bit which I believe to mean cell end is
                set. I also think btw that the third byte of the 4 byte
                value is the level of the cell
                */
                WW8PLCFspecial* pTest = pPlcxMan->GetMagicTables();
                if (pTest && pTest->SeekPosExact(nPosCp+1+nCpOfs) &&
                    pTest->Where() == nPosCp+1+nCpOfs)
                {
                    WW8_FC nPos;
                    void *pData;
                    pTest->Get(nPos, pData);
                    sal_uInt32 nData = SVBT32ToUInt32(*(SVBT32*)pData);
                    if (nData & 0x2) 
                    {
                        TabCellEnd();
                        bRet = false;
                    }
                }
                else if (bWasTabCellEnd)
                {
                    TabCellEnd();
                    bRet = false;
                }
            }

            bWasTabCellEnd = false;

            break;              
        case 0x5:               
        case 0x13:
            break;
        case 0x2:               
            if (!maFtnStack.empty())
                cInsert = 0x2;
            break;
        default:
            SAL_INFO( "sw.ww8.level2", "<unknownValue val=\"" << nWCharVal << "\">" );
            break;
    }

    if( '\x0' != cInsert )
    {
        OUString sInsert(cInsert);
        emulateMSWordAddTextToParagraph(sInsert);
    }
    if (!maApos.back()) 
        bWasParaEnd = bNewParaEnd;
    return bRet;
}

void SwWW8ImplReader::ProcessAktCollChange(WW8PLCFManResult& rRes,
    bool* pStartAttr, bool bCallProcessSpecial)
{
    sal_uInt16 nOldColl = nAktColl;
    nAktColl = pPlcxMan->GetColl();

    
    if (nAktColl >= vColl.size() || !vColl[nAktColl].pFmt || !vColl[nAktColl].bColl)
    {
        nAktColl = 0;
        bParaAutoBefore = false;
        bParaAutoAfter = false;
    }
    else
    {
        bParaAutoBefore = vColl[nAktColl].bParaAutoBefore;
        bParaAutoAfter = vColl[nAktColl].bParaAutoAfter;
    }

    if (nOldColl >= vColl.size())
        nOldColl = 0; 

    bool bTabRowEnd = false;
    if( pStartAttr && bCallProcessSpecial && !bInHyperlink )
    {
        bool bReSync;
        
        bTabRowEnd = ProcessSpecial(bReSync, rRes.nAktCp+pPlcxMan->GetCpOfs());
        if( bReSync )
            *pStartAttr = pPlcxMan->Get( &rRes ); 
    }

    if (!bTabRowEnd && StyleExists(nAktColl))
    {
        SetTxtFmtCollAndListLevel( *pPaM, vColl[ nAktColl ]);
        ChkToggleAttr(vColl[ nOldColl ].n81Flags, vColl[ nAktColl ].n81Flags);
        ChkToggleBiDiAttr(vColl[nOldColl].n81BiDiFlags,
            vColl[nAktColl].n81BiDiFlags);
    }
}

long SwWW8ImplReader::ReadTextAttr(WW8_CP& rTxtPos, bool& rbStartLine)
{
    long nSkipChars = 0;
    WW8PLCFManResult aRes;

    OSL_ENSURE(pPaM->GetNode()->GetTxtNode(), "Missing txtnode");
    bool bStartAttr = pPlcxMan->Get(&aRes); 
    aRes.nAktCp = rTxtPos;                  

    bool bNewSection = (aRes.nFlags & MAN_MASK_NEW_SEP) && !bIgnoreText;
    if ( bNewSection ) 
    {
        OSL_ENSURE(pPaM->GetNode()->GetTxtNode(), "Missing txtnode");
        
        maSectionManager.CreateSep(rTxtPos, bPgSecBreak);
        
        
        bPgSecBreak = false;
        OSL_ENSURE(pPaM->GetNode()->GetTxtNode(), "Missing txtnode");
    }

    
    if ( (aRes.nFlags & MAN_MASK_NEW_PAP)|| rbStartLine )
    {
        ProcessAktCollChange( aRes, &bStartAttr,
            MAN_MASK_NEW_PAP == (aRes.nFlags & MAN_MASK_NEW_PAP) &&
            !bIgnoreText );
        rbStartLine = false;
    }

    
    long nSkipPos = -1;

    if( 0 < aRes.nSprmId ) 
    {
        if( ( eFTN > aRes.nSprmId ) || ( 0x0800 <= aRes.nSprmId ) )
        {
            if( bStartAttr ) 
            {
                if( aRes.nMemLen >= 0 )
                    ImportSprm(aRes.pMemPos, aRes.nSprmId);
            }
            else
                EndSprm( aRes.nSprmId ); 
        }
        else if( aRes.nSprmId < 0x800 ) 
        {
            if (bStartAttr)
            {
                nSkipChars = ImportExtSprm(&aRes);
                if (
                    (aRes.nSprmId == eFTN) || (aRes.nSprmId == eEDN) ||
                    (aRes.nSprmId == eFLD) || (aRes.nSprmId == eAND)
                   )
                {
                    
                    rTxtPos += nSkipChars;
                    nSkipPos = rTxtPos-1;
                }
            }
            else
                EndExtSprm( aRes.nSprmId );
        }
    }

    pStrm->Seek(pSBase->WW8Cp2Fc( pPlcxMan->GetCpOfs() + rTxtPos, &bIsUnicode));

    
    if (nSkipChars && !bIgnoreText)
        pCtrlStck->MarkAllAttrsOld();
    bool bOldIgnoreText = bIgnoreText;
    bIgnoreText = true;
    sal_uInt16 nOldColl = nAktColl;
    bool bDoPlcxManPlusPLus = true;
    long nNext;
    do
    {
        if( bDoPlcxManPlusPLus )
            pPlcxMan->advance();
        nNext = pPlcxMan->Where();

        if (mpPostProcessAttrsInfo &&
            mpPostProcessAttrsInfo->mnCpStart == nNext)
        {
            mpPostProcessAttrsInfo->mbCopy = true;
        }

        if( (0 <= nNext) && (nSkipPos >= nNext) )
        {
            nNext = ReadTextAttr( rTxtPos, rbStartLine );
            bDoPlcxManPlusPLus = false;
            bIgnoreText = true;
        }

        if (mpPostProcessAttrsInfo &&
            nNext > mpPostProcessAttrsInfo->mnCpEnd)
        {
            mpPostProcessAttrsInfo->mbCopy = false;
        }
    }
    while( nSkipPos >= nNext );
    bIgnoreText    = bOldIgnoreText;
    if( nSkipChars )
    {
        pCtrlStck->KillUnlockedAttrs( *pPaM->GetPoint() );
        if( nOldColl != pPlcxMan->GetColl() )
            ProcessAktCollChange(aRes, 0, false);
    }

    return nNext;
}


bool SwWW8ImplReader::IsParaEndInCPs(sal_Int32 nStart, sal_Int32 nEnd,bool bSdOD) const
{
    
    if (nStart == -1 || nEnd == -1 || nEnd < nStart )
        return false;

    for (cp_vector::const_reverse_iterator aItr = maEndParaPos.rbegin(); aItr!= maEndParaPos.rend(); ++aItr)
    {
        
        
        
        if ( bSdOD && ( (nStart < *aItr && nEnd > *aItr) || ( nStart == nEnd && *aItr == nStart)) )
            return true;
        else if ( !bSdOD &&  (nStart < *aItr && nEnd >= *aItr) )
            return true;
    }

    return false;
}


void SwWW8ImplReader::ClearParaEndPosition()
{
    if ( maEndParaPos.size() > 0 )
        maEndParaPos.clear();
}

void SwWW8ImplReader::ReadAttrs(WW8_CP& rNext, WW8_CP& rTxtPos, bool& rbStartLine)
{
    
    if( rTxtPos >= rNext )
    {
        do
        {
            maCurrAttrCP = rTxtPos;
            rNext = ReadTextAttr( rTxtPos, rbStartLine );
        }
        while( rTxtPos >= rNext );

    }
    else if ( rbStartLine )
    {
    /* No attributes, but still a new line.
     * If a line ends with a line break and paragraph attributes or paragraph templates
     * do NOT change the line end was not added to the Plcx.Fkp.papx i.e. (nFlags & MAN_MASK_NEW_PAP)
     * is false.
     * Due to this we need to set the template here as a kind of special treatment.
     */
    if (!bCpxStyle && nAktColl < vColl.size())
            SetTxtFmtCollAndListLevel(*pPaM, vColl[nAktColl]);
        rbStartLine = false;
    }
}

/**
 * CloseAttrEnds to only read the attribute ends at the end of a text or a
 * text area (Header, Footnote, ...).
 * We ignore attribute starts and fields.
 */
void SwWW8ImplReader::CloseAttrEnds()
{
    
    
    std::stack<sal_uInt16> aStack;
    pPlcxMan->TransferOpenSprms(aStack);

    while (!aStack.empty())
    {
        sal_uInt16 nSprmId = aStack.top();
        if ((0 < nSprmId) && (( eFTN > nSprmId) || (0x0800 <= nSprmId)))
            EndSprm(nSprmId);
        aStack.pop();
    }

    EndSpecial();
}

bool SwWW8ImplReader::ReadText(long nStartCp, long nTextLen, ManTypes nType)
{
    bool bJoined=false;

    bool bStartLine = true;
    short nCrCount = 0;
    short nDistance = 0;

    bWasParaEnd = false;
    nAktColl    =  0;
    pAktItemSet =  0;
    nCharFmt    = -1;
    bSpec = false;
    bPgSecBreak = false;

    pPlcxMan = new WW8PLCFMan( pSBase, nType, nStartCp );
    long nCpOfs = pPlcxMan->GetCpOfs(); 

    WW8_CP nNext = pPlcxMan->Where();
    SwTxtNode* pPreviousNode = 0;
    sal_uInt8 nDropLines = 0;
    SwCharFmt* pNewSwCharFmt = 0;
    const SwCharFmt* pFmt = 0;
    pStrm->Seek( pSBase->WW8Cp2Fc( nStartCp + nCpOfs, &bIsUnicode ) );

    WW8_CP l = nStartCp;
    while ( l<nStartCp+nTextLen )
    {
        ReadAttrs( nNext, l, bStartLine );
        OSL_ENSURE(pPaM->GetNode()->GetTxtNode(), "Missing txtnode");

        if (mpPostProcessAttrsInfo != NULL)
            PostProcessAttrs();

        if( l>= nStartCp + nTextLen )
            break;

        bStartLine = ReadChars(l, nNext, nStartCp+nTextLen, nCpOfs);

        
        
        if (bStartLine && !pPreviousNode) 
        {
            bool bSplit = true;
            if (mbCareFirstParaEndInToc)
            {
                mbCareFirstParaEndInToc = false;
                if (pPaM->End() && pPaM->End()->nNode.GetNode().GetTxtNode() &&  pPaM->End()->nNode.GetNode().GetTxtNode()->Len() == 0)
                    bSplit = false;
            }
            if (mbCareLastParaEndInToc)
            {
                mbCareLastParaEndInToc = false;
                if (pPaM->End() && pPaM->End()->nNode.GetNode().GetTxtNode() &&  pPaM->End()->nNode.GetNode().GetTxtNode()->Len() == 0)
                    bSplit = false;
            }
            if (bSplit)
            {
                
                if (mbOnLoadingMain)
                    maEndParaPos.push_back(l-1);
                AppendTxtNode(*pPaM->GetPoint());
            }
        }

        if (pPreviousNode && bStartLine)
        {
            SwTxtNode* pEndNd = pPaM->GetNode()->GetTxtNode();
            const sal_Int32 nDropCapLen = pPreviousNode->GetTxt().getLength();

            
            {
                SwPaM aTmp(*pEndNd, 0, *pEndNd, nDropCapLen+1);
                pCtrlStck->Delete(aTmp);
            }

            
            const SwFmtDrop* defaultDrop =
                (const SwFmtDrop*) GetFmtAttr(RES_PARATR_DROP);
            SwFmtDrop aDrop(*defaultDrop);

            aDrop.GetLines() = nDropLines;
            aDrop.GetDistance() = nDistance;
            aDrop.GetChars() = writer_cast<sal_uInt8>(nDropCapLen);
            
            aDrop.GetWholeWord() = false;

            if (pFmt)
                aDrop.SetCharFmt(const_cast<SwCharFmt*>(pFmt));
            else if(pNewSwCharFmt)
                aDrop.SetCharFmt(const_cast<SwCharFmt*>(pNewSwCharFmt));

            SwPosition aStart(*pEndNd);
            pCtrlStck->NewAttr(aStart, aDrop);
            pCtrlStck->SetAttr(*pPaM->GetPoint(), RES_PARATR_DROP);
            pPreviousNode = 0;
        }
        else if (bDropCap)
        {
            
            pPreviousNode = pPaM->GetNode()->GetTxtNode();

            const sal_uInt8 *pDCS;

            if (bVer67)
                pDCS = pPlcxMan->GetPapPLCF()->HasSprm(46);
            else
                pDCS = pPlcxMan->GetPapPLCF()->HasSprm(0x442C);

            if (pDCS)
                nDropLines = (*pDCS) >> 3;
            else    
                pPreviousNode = 0;

            if (const sal_uInt8 *pDistance = pPlcxMan->GetPapPLCF()->HasSprm(0x842F))
                nDistance = SVBT16ToShort( pDistance );
            else
                nDistance = 0;

            const SwFmtCharFmt *pSwFmtCharFmt = 0;

            if(pAktItemSet)
                pSwFmtCharFmt = &(ItemGet<SwFmtCharFmt>(*pAktItemSet, RES_TXTATR_CHARFMT));

            if(pSwFmtCharFmt)
                pFmt = pSwFmtCharFmt->GetCharFmt();

            if(pAktItemSet && !pFmt)
            {
                OUString sPrefix(OUStringBuffer("WW8Dropcap").append(nDropCap++).makeStringAndClear());
                pNewSwCharFmt = rDoc.MakeCharFmt(sPrefix, (SwCharFmt*)rDoc.GetDfltCharFmt());
                 pAktItemSet->ClearItem(RES_CHRATR_ESCAPEMENT);
                pNewSwCharFmt->SetFmtAttr( *pAktItemSet );
            }

            delete pAktItemSet;
            pAktItemSet = 0;
            bDropCap=false;
        }

        if (bStartLine || bWasTabRowEnd)
        {
            
            if ((nCrCount++ & 0x40) == 0 && nType == MAN_MAINTEXT)
            {
                nProgress = (sal_uInt16)( l * 100 / nTextLen );
                ::SetProgressState(nProgress, mpDocShell); 
            }
        }

        
        
        
        
        if (bPgSecBreak)
        {
            
            
            
            WW8PLCFxDesc aTemp;
            aTemp.nStartPos = aTemp.nEndPos = WW8_CP_MAX;
            if (pPlcxMan->GetSepPLCF())
                pPlcxMan->GetSepPLCF()->GetSprms(&aTemp);
            if ((aTemp.nStartPos != l) && (aTemp.nEndPos != l))
            {
                
                
                
                
                if (!bStartLine && !pAnchorStck->empty())
                {
                    AppendTxtNode(*pPaM->GetPoint());
                }
                rDoc.InsertPoolItem(*pPaM,
                    SvxFmtBreakItem(SVX_BREAK_PAGE_BEFORE, RES_BREAK), 0);
                bFirstParaOfPage = true;
                bPgSecBreak = false;
            }
        }
    }

    if (pPaM->GetPoint()->nContent.GetIndex())
        AppendTxtNode(*pPaM->GetPoint());

    if (!bInHyperlink)
        bJoined = JoinNode(*pPaM);

    CloseAttrEnds();

    delete pPlcxMan, pPlcxMan = 0;
    return bJoined;
}

/**
 * class SwWW8ImplReader
 */
SwWW8ImplReader::SwWW8ImplReader(sal_uInt8 nVersionPara, SvStorage* pStorage,
    SvStream* pSt, SwDoc& rD, const OUString& rBaseURL, bool bNewDoc)
    : mpDocShell(rD.GetDocShell())
    , pStg(pStorage)
    , pStrm(pSt)
    , pTableStream(0)
    , pDataStream(0)
    , rDoc(rD)
    , maSectionManager(*this)
    , m_aExtraneousParas(rD)
    , maInsertedTables(rD)
    , maSectionNameGenerator(rD, OUString("WW"))
    , maGrfNameGenerator(bNewDoc, OUString('G'))
    , maParaStyleMapper(rD)
    , maCharStyleMapper(rD)
    , maTxtNodesHavingFirstLineOfstSet()
    , maTxtNodesHavingLeftIndentSet()
    , pMSDffManager(0)
    , mpAtnNames(0)
    , sBaseURL(rBaseURL)
    , m_bRegardHindiDigits( false )
    , mbNewDoc(bNewDoc)
    , nDropCap(0)
    , nIdctHint(0)
    , bBidi(false)
    , bReadTable(false)
    , mbLoadingTOCCache(false)
    , mbLoadingTOCHyperlink(false)
    , mpPosAfterTOC(0)
    , mbCareFirstParaEndInToc(false)
    , mbCareLastParaEndInToc(false)
    , maTOXEndCps()
    , maCurrAttrCP(-1)
    , mbOnLoadingMain(false)
{
    pStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    nWantedVersion = nVersionPara;
    pCtrlStck   = 0;
    mpRedlineStack = 0;
    pReffedStck = 0;
    pReffingStck = 0;
    pAnchorStck = 0;
    pFonts = 0;
    pSBase = 0;
    pPlcxMan = 0;
    pStyles = 0;
    pAktColl = 0;
    pLstManager = 0;
    pAktItemSet = 0;
    pDfltTxtFmtColl = 0;
    pStandardFmtColl = 0;
    pHdFt = 0;
    pWFlyPara = 0;
    pSFlyPara = 0;
    pFlyFmtOfJustInsertedGraphic   = 0;
    pFmtOfJustInsertedApo = 0;
    pPreviousNumPaM = 0;
    pPrevNumRule = 0;
    nAktColl = 0;
    nObjLocFc = nPicLocFc = 0;
    nInTable=0;
    bReadNoTbl = bPgSecBreak = bSpec = bObj = bTxbxFlySection
               = bHasBorder = bSymbol = bIgnoreText
               = bWasTabRowEnd = bWasTabCellEnd = false;
    bShdTxtCol = bCharShdTxtCol = bAnl = bHdFtFtnEdn = bFtnEdn
               = bIsHeader = bIsFooter = bIsUnicode = bCpxStyle = bStyNormal =
                 bWWBugNormal  = false;

    mpPostProcessAttrsInfo = 0;

    bNoAttrImport = bEmbeddObj = false;
    bAktAND_fNumberAcross = false;
    bNoLnNumYet = true;
    bInHyperlink = false;
    bWasParaEnd = false;
    bDropCap = false;
    bFirstPara = true;
    bFirstParaOfPage = false;
    bParaAutoBefore = false;
    bParaAutoAfter = false;
    nProgress = 0;
    nSwNumLevel = nWwNumType = 0xff;
    pTableDesc = 0;
    pNumOlst = 0;
    pNode_FLY_AT_PARA = 0;
    pDrawModel = 0;
    pDrawPg = 0;
    mpDrawEditEngine = 0;
    pWWZOrder = 0;
    pFormImpl = 0;
    mpChosenOutlineNumRule = 0;
    pNumFldType = 0;
    nFldNum = 0;

    nLFOPosition = USHRT_MAX;
    nListLevel = WW8ListManager::nMaxLevel;
    eHardCharSet = RTL_TEXTENCODING_DONTKNOW;

    nPgChpDelim = nPgChpLevel = 0;

    maApos.push_back(false);
}

void SwWW8ImplReader::DeleteStk(SwFltControlStack* pStck)
{
    if( pStck )
    {
        pStck->SetAttr( *pPaM->GetPoint(), 0, false);
        pStck->SetAttr( *pPaM->GetPoint(), 0, false);
        delete pStck;
    }
    else
    {
        OSL_ENSURE( !this, "WW-Stack bereits geloescht" );
    }
}

void wwSectionManager::SetSegmentToPageDesc(const wwSection &rSection,
    bool bIgnoreCols)
{
    SwPageDesc &rPage = *rSection.mpPage;

    SetNumberingType(rSection, rPage);

    SwFrmFmt &rFmt = rPage.GetMaster();

    if(mrReader.pWDop->fUseBackGroundInAllmodes) 
        mrReader.GrafikCtor();


    if (mrReader.pWDop->fUseBackGroundInAllmodes && mrReader.pMSDffManager)
    {
        Rectangle aRect(0, 0, 100, 100); 
        SvxMSDffImportData aData(aRect);
        SdrObject* pObject = 0;
        if (mrReader.pMSDffManager->GetShape(0x401, pObject, aData))
        {
            
            if ((aData.begin()->nFlags & 0x400) != 0)
            {
                SfxItemSet aSet(rFmt.GetAttrSet());
                mrReader.MatchSdrItemsIntoFlySet(pObject, aSet, mso_lineSimple,
                                                 mso_lineSolid, mso_sptRectangle, aRect);
                rFmt.SetFmtAttr(aSet.Get(RES_BACKGROUND));
            }
        }
    }
    wwULSpaceData aULData;
    GetPageULData(rSection, aULData);
    SetPageULSpaceItems(rFmt, aULData, rSection);

    SetPage(rPage, rFmt, rSection, bIgnoreCols);

    if (!(rSection.maSep.pgbApplyTo & 1))
        mrReader.SetPageBorder(rFmt, rSection);
    if (!(rSection.maSep.pgbApplyTo & 2))
        mrReader.SetPageBorder(rPage.GetFirstMaster(), rSection);

    mrReader.SetDocumentGrid(rFmt, rSection);
}

void wwSectionManager::SetUseOn(wwSection &rSection)
{
    bool bEven = (rSection.maSep.grpfIhdt & (WW8_HEADER_EVEN|WW8_FOOTER_EVEN)) ?
        true : false;

    bool bMirror = mrReader.pWDop->fMirrorMargins ||
        mrReader.pWDop->doptypography.f2on1;

    UseOnPage eUseBase = bMirror ? nsUseOnPage::PD_MIRROR : nsUseOnPage::PD_ALL;
    UseOnPage eUse = eUseBase;
    if (!bEven)
        eUse = (UseOnPage)(eUse | nsUseOnPage::PD_HEADERSHARE | nsUseOnPage::PD_FOOTERSHARE);
    if (!rSection.HasTitlePage())
        eUse = (UseOnPage)(eUse | nsUseOnPage::PD_FIRSTSHARE);

    OSL_ENSURE(rSection.mpPage, "Makes no sense to call me with no pages to set");
    if (rSection.mpPage)
        rSection.mpPage->WriteUseOn(eUse);
}

/**
 * Set the page descriptor on this node, handle the different cases for a text
 * node or a table
 */
void GiveNodePageDesc(SwNodeIndex &rIdx, const SwFmtPageDesc &rPgDesc,
    SwDoc &rDoc)
{
    /*
    If it's a table here, apply the pagebreak to the table
    properties, otherwise we add it to the para at this
    position
    */
    if (rIdx.GetNode().IsTableNode())
    {
        SwTable& rTable =
            rIdx.GetNode().GetTableNode()->GetTable();
        SwFrmFmt* pApply = rTable.GetFrmFmt();
        OSL_ENSURE(pApply, "impossible");
        if (pApply)
            pApply->SetFmtAttr(rPgDesc);
    }
    else
    {
        SwPosition aPamStart(rIdx);
        aPamStart.nContent.Assign(
            rIdx.GetNode().GetCntntNode(), 0);
        SwPaM aPage(aPamStart);

        rDoc.InsertPoolItem(aPage, rPgDesc, 0);
    }
}

/**
 * Map a word section to a writer page descriptor
 */
SwFmtPageDesc wwSectionManager::SetSwFmtPageDesc(mySegIter &rIter,
    mySegIter &rStart, bool bIgnoreCols)
{
    if (IsNewDoc() && rIter == rStart)
    {
        rIter->mpPage =
            mrReader.rDoc.GetPageDescFromPool(RES_POOLPAGE_STANDARD);
    }
    else
    {
        sal_uInt16 nPos = mrReader.rDoc.MakePageDesc(
            SwViewShell::GetShellRes()->GetPageDescName(mnDesc, ShellResource::NORMAL_PAGE),
            0, false);
        rIter->mpPage = &mrReader.rDoc.GetPageDesc(nPos);
    }
    OSL_ENSURE(rIter->mpPage, "no page!");
    if (!rIter->mpPage)
        return SwFmtPageDesc();

    
    const wwSection *pPrevious = 0;
    if (rIter != rStart)
        pPrevious = &(*(rIter-1));
    SetHdFt(*rIter, std::distance(rStart, rIter), pPrevious);
    SetUseOn(*rIter);

    
    SetSegmentToPageDesc(*rIter, bIgnoreCols);

    SwFmtPageDesc aRet(rIter->mpPage);

    rIter->mpPage->SetFollow(rIter->mpPage);

    if (rIter->PageRestartNo())
        aRet.SetNumOffset(rIter->PageStartAt());

    ++mnDesc;
    return aRet;
}

bool wwSectionManager::IsNewDoc() const
{
    return mrReader.mbNewDoc;
}

void wwSectionManager::InsertSegments()
{
    const SvtFilterOptions& rOpt = SvtFilterOptions::Get();
    sal_Bool bUseEnhFields = rOpt.IsUseEnhancedFields();
    mySegIter aEnd = maSegments.end();
    mySegIter aStart = maSegments.begin();
    for (mySegIter aIter = aStart; aIter != aEnd; ++aIter)
    {
        
        
        
        if ( aIter->maSep.bkc == 1 && aIter->maSep.ccolM1 > 0 )
        {
            SwPaM start( aIter->maStart );
            mrReader.rDoc.InsertPoolItem( start, SvxFmtBreakItem(SVX_BREAK_COLUMN_BEFORE, RES_BREAK), 0);
            continue;
        }

        mySegIter aNext = aIter+1;
        mySegIter aPrev = (aIter == aStart) ? aIter : aIter-1;

        
        
        bool bThisAndPreviousAreCompatible = ((aIter->GetPageWidth() == aPrev->GetPageWidth()) &&
            (aIter->GetPageHeight() == aPrev->GetPageHeight()) && (aIter->IsLandScape() == aPrev->IsLandScape()));

        bool bInsertSection = (aIter != aStart) ? (aIter->IsContinuous() &&  bThisAndPreviousAreCompatible): false;
        bool bInsertPageDesc = !bInsertSection;
        bool bProtected = SectionIsProtected(*aIter); 
        if (bUseEnhFields && mrReader.pWDop->fProtEnabled && aIter->IsNotProtected())
        {
            
            
            mrReader.rDoc.set(IDocumentSettingAccess::PROTECT_FORM, false );
        }

        if (bInsertPageDesc)
        {
            /*
             If a cont section follows this section then we won't be
             creating a page desc with 2+ cols as we cannot host a one
             col section in a 2+ col pagedesc and make it look like
             word. But if the current section actually has columns then
             we are forced to insert a section here as well as a page
             descriptor.
            */

            bool bIgnoreCols = false;
            bool bThisAndNextAreCompatible = (aNext != aEnd) ? ((aIter->GetPageWidth() == aNext->GetPageWidth()) &&
                (aIter->GetPageHeight() == aNext->GetPageHeight()) && (aIter->IsLandScape() == aNext->IsLandScape())) : true;

            if (((aNext != aEnd && aNext->IsContinuous() && bThisAndNextAreCompatible) || bProtected))
            {
                bIgnoreCols = true;
                if ((aIter->NoCols() > 1) || bProtected)
                    bInsertSection = true;
            }

            SwFmtPageDesc aDesc(SetSwFmtPageDesc(aIter, aStart, bIgnoreCols));
            if (!aDesc.GetPageDesc())
                continue;

            
            
            
            
            
            if ( aIter->maSep.bkc == 3 || aIter->maSep.bkc == 4 )
            {
                
                
                
                
                
                wwSection aTmpSection = *aIter;
                
                SwFmtPageDesc aFollow(SetSwFmtPageDesc(aIter, aStart, bIgnoreCols));
                
                *aIter = aTmpSection;

                
                UseOnPage eUseOnPage = nsUseOnPage::PD_LEFT;
                if ( aIter->maSep.bkc == 4 ) 
                    eUseOnPage = nsUseOnPage::PD_RIGHT;

                aDesc.GetPageDesc()->WriteUseOn( eUseOnPage );
                aDesc.GetPageDesc()->SetFollow( aFollow.GetPageDesc() );
            }

            GiveNodePageDesc(aIter->maStart, aDesc, mrReader.rDoc);
        }

        SwTxtNode* pTxtNd = 0;
        if (bInsertSection)
        {
            
            SwPaM aSectPaM(*mrReader.pPaM);
            SwNodeIndex aAnchor(aSectPaM.GetPoint()->nNode);
            if (aNext != aEnd)
            {
                aAnchor = aNext->maStart;
                aSectPaM.GetPoint()->nNode = aAnchor;
                aSectPaM.GetPoint()->nContent.Assign(
                    aNext->maStart.GetNode().GetCntntNode(), 0);
                aSectPaM.Move(fnMoveBackward);
            }

            const SwPosition* pPos  = aSectPaM.GetPoint();
            SwTxtNode const*const pSttNd = pPos->nNode.GetNode().GetTxtNode();
            const SwTableNode* pTableNd = pSttNd ? pSttNd->FindTableNode() : 0;
            if (pTableNd)
            {
                pTxtNd =
                    mrReader.rDoc.GetNodes().MakeTxtNode(aAnchor,
                    mrReader.rDoc.GetTxtCollFromPool( RES_POOLCOLL_TEXT ));

                aSectPaM.GetPoint()->nNode = SwNodeIndex(*pTxtNd);
                aSectPaM.GetPoint()->nContent.Assign(
                    aSectPaM.GetCntntNode(), 0);
            }

            aSectPaM.SetMark();

            aSectPaM.GetPoint()->nNode = aIter->maStart;
            aSectPaM.GetPoint()->nContent.Assign(
                aSectPaM.GetCntntNode(), 0);

            
            SwSectionFmt *pRet = InsertSection(aSectPaM, *aIter);
            
            if (pRet)
            {
                
                if (mrReader.pWDop->fNoColumnBalance)
                    pRet->SetFmtAttr(SwFmtNoBalancedColumns(true));
                else
                {
                    
                    
                    
                    if (aNext == aEnd || !aNext->IsContinuous())
                        pRet->SetFmtAttr(SwFmtNoBalancedColumns(true));
                }
            }

            bool bHasOwnHdFt = false;
            /*
             In this nightmare scenario the continuous section has its own
             headers and footers so we will try and find a hard page break
             between here and the end of the section and put the headers and
             footers there.
            */
            if (!bInsertPageDesc)
            {
               bHasOwnHdFt =
                mrReader.HasOwnHeaderFooter(
                 aIter->maSep.grpfIhdt & ~(WW8_HEADER_FIRST | WW8_FOOTER_FIRST),
                 aIter->maSep.grpfIhdt, std::distance(aStart, aIter)
                );
            }
            if (bHasOwnHdFt)
            {
                
                
                SwPageDesc *pOrig = aIter->mpPage;
                bool bFailed = true;
                SwFmtPageDesc aDesc(SetSwFmtPageDesc(aIter, aStart, true));
                if (aDesc.GetPageDesc())
                {
                    sal_uLong nStart = aSectPaM.Start()->nNode.GetIndex();
                    sal_uLong nEnd   = aSectPaM.End()->nNode.GetIndex();
                    for(; nStart <= nEnd; ++nStart)
                    {
                        SwNode* pNode = mrReader.rDoc.GetNodes()[nStart];
                        if (!pNode)
                            continue;
                        if (sw::util::HasPageBreak(*pNode))
                        {
                            SwNodeIndex aIdx(*pNode);
                            GiveNodePageDesc(aIdx, aDesc, mrReader.rDoc);
                            bFailed = false;
                            break;
                        }
                    }
                }
                if(bFailed)
                {
                    aIter->mpPage = pOrig;
                }
            }
        }

        if (pTxtNd)
        {
            SwNodeIndex aIdx(*pTxtNd);
            SwPaM aTest(aIdx);
            mrReader.rDoc.DelFullPara(aTest);
            pTxtNd = 0;
        }
    }
}

void wwExtraneousParas::delete_all_from_doc()
{
    typedef std::vector<SwTxtNode*>::iterator myParaIter;
    myParaIter aEnd = m_aTxtNodes.end();
    for (myParaIter aI = m_aTxtNodes.begin(); aI != aEnd; ++aI)
    {
        SwTxtNode *pTxtNode = *aI;
        SwNodeIndex aIdx(*pTxtNode);
        SwPaM aTest(aIdx);
        m_rDoc.DelFullPara(aTest);
    }
    m_aTxtNodes.clear();
}

void SwWW8ImplReader::StoreMacroCmds()
{
    if (pWwFib->lcbCmds)
    {
        pTableStream->Seek(pWwFib->fcCmds);

        uno::Reference < embed::XStorage > xRoot(mpDocShell->GetStorage());

        if (!xRoot.is())
            return;

        try
        {
            uno::Reference < io::XStream > xStream =
                    xRoot->openStreamElement( OUString(SL::aMSMacroCmds), embed::ElementModes::READWRITE );
            SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( xStream );

            sal_uInt8 *pBuffer = new sal_uInt8[pWwFib->lcbCmds];
            pWwFib->lcbCmds = pTableStream->Read(pBuffer, pWwFib->lcbCmds);
            pStream->Write(pBuffer, pWwFib->lcbCmds);
            delete[] pBuffer;
            delete pStream;
        }
        catch ( const uno::Exception& )
        {
        }
    }
}

void SwWW8ImplReader::ReadDocVars()
{
    std::vector<OUString> aDocVarStrings;
    std::vector<ww::bytes> aDocVarStringIds;
    std::vector<OUString> aDocValueStrings;
    WW8ReadSTTBF(!bVer67, *pTableStream, pWwFib->fcStwUser,
        pWwFib->lcbStwUser, bVer67 ? 2 : 0, eStructCharSet,
        aDocVarStrings, &aDocVarStringIds, &aDocValueStrings);
    if (!bVer67) {
        using namespace ::com::sun::star;

        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            mpDocShell->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps(
            xDPS->getDocumentProperties());
        OSL_ENSURE(xDocProps.is(), "DocumentProperties is null");
        uno::Reference<beans::XPropertyContainer> xUserDefinedProps =
            xDocProps->getUserDefinedProperties();
        OSL_ENSURE(xUserDefinedProps.is(), "UserDefinedProperties is null");

        for(size_t i=0; i<aDocVarStrings.size(); i++)
        {
            const OUString &rName = aDocVarStrings[i];
            uno::Any aValue;
            aValue <<= OUString(aDocValueStrings[i]);
            try {
                xUserDefinedProps->addProperty( rName,
                    beans::PropertyAttribute::REMOVABLE,
                    aValue );
            } catch (const uno::Exception &) {
                
            }
        }
    }
}

/**
 * Document Info
 */
void SwWW8ImplReader::ReadDocInfo()
{
    if( pStg )
    {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            mpDocShell->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps(
            xDPS->getDocumentProperties());
        OSL_ENSURE(xDocProps.is(), "DocumentProperties is null");

        if (xDocProps.is()) {
            if ( pWwFib->fDot )
            {
                OUString sTemplateURL;
                SfxMedium* pMedium = mpDocShell->GetMedium();
                if ( pMedium )
                {
                    OUString aName = pMedium->GetName();
                    INetURLObject aURL( aName );
                    sTemplateURL = aURL.GetMainURL(INetURLObject::DECODE_TO_IURI);
                    if ( !sTemplateURL.isEmpty() )
                        xDocProps->setTemplateURL( sTemplateURL );
                }
            }
            else if (pWwFib->lcbSttbfAssoc) 
            {
                long nCur = pTableStream->Tell();
                Sttb aSttb;
                pTableStream->Seek( pWwFib->fcSttbfAssoc ); 
                if (!aSttb.Read( *pTableStream ) )
                    OSL_TRACE("** Read of SttbAssoc data failed!!!! ");
                pTableStream->Seek( nCur ); 
#if OSL_DEBUG_LEVEL > 1
                aSttb.Print( stderr );
#endif
                OUString sPath = aSttb.getStringAtIndex( 0x1 );
                OUString aURL;
                
                if ( !sPath.isEmpty() )
                    ::utl::LocalFileHelper::ConvertPhysicalNameToURL( sPath, aURL );
                if (aURL.isEmpty())
                    xDocProps->setTemplateURL( aURL );
                else
                    xDocProps->setTemplateURL( sPath );

            }
            sfx2::LoadOlePropertySet(xDocProps, pStg);
        }
    }
}

static void lcl_createTemplateToProjectEntry( const uno::Reference< container::XNameContainer >& xPrjNameCache, const OUString& sTemplatePathOrURL, const OUString& sVBAProjName )
{
    if ( xPrjNameCache.is() )
    {
        INetURLObject aObj;
        aObj.SetURL( sTemplatePathOrURL );
        bool bIsURL = aObj.GetProtocol() != INET_PROT_NOT_VALID;
        OUString aURL;
        if ( bIsURL )
            aURL = sTemplatePathOrURL;
        else
        {
            osl::FileBase::getFileURLFromSystemPath( sTemplatePathOrURL, aURL );
            aObj.SetURL( aURL );
        }
        try
        {
            OUString templateNameWithExt = aObj.GetLastName();
            OUString templateName;
            sal_Int32 nIndex =  templateNameWithExt.lastIndexOf( '.' );
            if ( nIndex != -1 )
            {
                templateName = templateNameWithExt.copy( 0, nIndex );
                xPrjNameCache->insertByName( templateName, uno::makeAny( sVBAProjName ) );
            }
        }
        catch( const uno::Exception& )
        {
        }
    }
}

class WW8Customizations
{
    SvStream* mpTableStream;
    WW8Fib mWw8Fib;
public:
    WW8Customizations( SvStream*, WW8Fib& );
    bool  Import( SwDocShell* pShell );
};

WW8Customizations::WW8Customizations( SvStream* pTableStream, WW8Fib& rFib ) : mpTableStream(pTableStream), mWw8Fib( rFib )
{
}

bool WW8Customizations::Import( SwDocShell* pShell )
{
    if ( mWw8Fib.lcbCmds == 0 || !IsEightPlus(mWw8Fib.GetFIBVersion()) )
        return false;
    try
    {
        Tcg aTCG;
        long nCur = mpTableStream->Tell();
        mpTableStream->Seek( mWw8Fib.fcCmds ); 
        bool bReadResult = aTCG.Read( *mpTableStream );
        mpTableStream->Seek( nCur ); 
        if ( !bReadResult )
        {
            SAL_WARN("sw.ww8", "** Read of Customization data failed!!!! ");
            return false;
        }
#if OSL_DEBUG_LEVEL > 1
        aTCG.Print( stderr );
#endif
        return aTCG.ImportCustomToolBar( *pShell );
    }
    catch(...)
    {
        SAL_WARN("sw.ww8", "** Read of Customization data failed!!!! epically");
        return false;
    }
}

bool SwWW8ImplReader::ReadGlobalTemplateSettings( const OUString& sCreatedFrom, const uno::Reference< container::XNameContainer >& xPrjNameCache )
{
    SvtPathOptions aPathOpt;
    OUString aAddinPath = aPathOpt.GetAddinPath();
    uno::Sequence< OUString > sGlobalTemplates;

    
    uno::Reference<ucb::XSimpleFileAccess3> xSFA(ucb::SimpleFileAccess::create(::comphelper::getProcessComponentContext()));

    if( xSFA->isFolder( aAddinPath ) )
        sGlobalTemplates = xSFA->getFolderContents( aAddinPath, sal_False );

    sal_Int32 nEntries = sGlobalTemplates.getLength();
    bool bRes = true;
    for ( sal_Int32 i=0; i<nEntries; ++i )
    {
        INetURLObject aObj;
        aObj.SetURL( sGlobalTemplates[ i ] );
        bool bIsURL = aObj.GetProtocol() != INET_PROT_NOT_VALID;
        OUString aURL;
        if ( bIsURL )
                aURL = sGlobalTemplates[ i ];
        else
                osl::FileBase::getFileURLFromSystemPath( sGlobalTemplates[ i ], aURL );
        if ( !aURL.endsWithIgnoreAsciiCase( ".dot" ) || ( !sCreatedFrom.isEmpty() && sCreatedFrom.equals( aURL ) ) )
            continue; 

        SotStorageRef rRoot = new SotStorage( aURL, STREAM_STD_READWRITE, STORAGE_TRANSACTED );

        BasicProjImportHelper aBasicImporter( *mpDocShell );
        
        aBasicImporter.import( mpDocShell->GetMedium()->GetInputStream() );
        lcl_createTemplateToProjectEntry( xPrjNameCache, aURL, aBasicImporter.getProjectName() );
        
        SvStorageStreamRef refMainStream = rRoot->OpenSotStream( OUString( "WordDocument" ));
        refMainStream->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
        WW8Fib aWwFib( *refMainStream, 8 );
        SvStorageStreamRef xTableStream = rRoot->OpenSotStream(OUString::createFromAscii( aWwFib.fWhichTblStm ? SL::a1Table : SL::a0Table), STREAM_STD_READ);

        if (xTableStream.Is() && SVSTREAM_OK == xTableStream->GetError())
        {
            xTableStream->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
            WW8Customizations aGblCustomisations( xTableStream, aWwFib );
            aGblCustomisations.Import( mpDocShell );
        }
    }
    return bRes;
}

sal_uLong SwWW8ImplReader::CoreLoad(WW8Glossary *pGloss, const SwPosition &rPos)
{
    sal_uLong nErrRet = 0;

    if (mbNewDoc && pStg && !pGloss)
        ReadDocInfo();

    ::ww8::WW8FibData * pFibData = new ::ww8::WW8FibData();

    if (pWwFib->fReadOnlyRecommended)
        pFibData->setReadOnlyRecommended(true);
    else
        pFibData->setReadOnlyRecommended(false);

    if (pWwFib->fWriteReservation)
        pFibData->setWriteReservation(true);
    else
        pFibData->setWriteReservation(false);

    ::sw::tExternalDataPointer pExternalFibData(pFibData);

    rDoc.setExternalData(::sw::FIB, pExternalFibData);

    ::sw::tExternalDataPointer pSttbfAsoc
          (new ::ww8::WW8Sttb<ww8::WW8Struct>(*pTableStream, pWwFib->fcSttbfAssoc, pWwFib->lcbSttbfAssoc));

    rDoc.setExternalData(::sw::STTBF_ASSOC, pSttbfAsoc);

    if (pWwFib->fWriteReservation || pWwFib->fReadOnlyRecommended)
    {
        SwDocShell * pDocShell = rDoc.GetDocShell();
        if (pDocShell)
            pDocShell->SetReadOnlyUI(sal_True);
    }

    pPaM = new SwPaM(rPos);

    pCtrlStck = new SwWW8FltControlStack( &rDoc, nFieldFlags, *this );

    mpRedlineStack = new sw::util::RedlineStack(rDoc);

    /*
        RefFldStck: Keeps track of bookmarks which may be inserted as
        variables intstead.
    */
    pReffedStck = new SwWW8ReferencedFltEndStack(&rDoc, nFieldFlags);
    pReffingStck = new SwWW8FltRefStack(&rDoc, nFieldFlags);

    pAnchorStck = new SwWW8FltAnchorStack(&rDoc, nFieldFlags);

    sal_uInt16 nPageDescOffset = rDoc.GetPageDescCnt();

    SwNodeIndex aSttNdIdx( rDoc.GetNodes() );
    SwRelNumRuleSpaces aRelNumRule(rDoc, mbNewDoc);

    sal_uInt16 eMode = nsRedlineMode_t::REDLINE_SHOW_INSERT;

    mpSprmParser = new wwSprmParser(pWwFib->GetFIBVersion());

    
    bVer6  = (6 == pWwFib->nVersion);
    bVer7  = (7 == pWwFib->nVersion);
    bVer67 = bVer6 || bVer7;
    bVer8  = (8 == pWwFib->nVersion);

    eTextCharSet = WW8Fib::GetFIBCharset(pWwFib->chse);
    eStructCharSet = WW8Fib::GetFIBCharset(pWwFib->chseTables);

    bWWBugNormal = pWwFib->nProduct == 0xc03d;

    if (!mbNewDoc)
        aSttNdIdx = pPaM->GetPoint()->nNode;

    ::StartProgress(STR_STATSTR_W4WREAD, 0, 100, mpDocShell);

    
    pFonts = new WW8Fonts( *pTableStream, *pWwFib );

    
    pWDop = new WW8Dop( *pTableStream, pWwFib->nFib, pWwFib->fcDop,
        pWwFib->lcbDop );

    if (mbNewDoc)
        ImportDop();

    /*
        Import revisioning data: author names
    */
    if( pWwFib->lcbSttbfRMark )
    {
        ReadRevMarkAuthorStrTabl( *pTableStream,
                                    pWwFib->fcSttbfRMark,
                                    pWwFib->lcbSttbfRMark, rDoc );
    }

    
    std::vector<OUString> aLinkStrings;
    std::vector<ww::bytes> aStringIds;

    WW8ReadSTTBF(!bVer67, *pTableStream, pWwFib->fcSttbFnm,
        pWwFib->lcbSttbFnm, bVer67 ? 2 : 0, eStructCharSet,
        aLinkStrings, &aStringIds);

    for (size_t i=0; i < aLinkStrings.size() && i < aStringIds.size(); ++i)
    {
        ww::bytes stringId = aStringIds[i];
        WW8_STRINGID *stringIdStruct = (WW8_STRINGID*)(&stringId[0]);
        aLinkStringMap[SVBT16ToShort(stringIdStruct->nStringId)] =
            aLinkStrings[i];
    }

    ReadDocVars(); 

    ::SetProgressState(nProgress, mpDocShell);    

    pLstManager = new WW8ListManager( *pTableStream, *this );

    /*
        zuerst(!) alle Styles importieren   (siehe WW8PAR2.CXX)
            VOR dem Import der Listen !!
    */
    ::SetProgressState(nProgress, mpDocShell);    
    pStyles = new WW8RStyle( *pWwFib, this );     
    pStyles->Import();

    /*
        In the end: (also see WW8PAR3.CXX)

        Go through all Styles and attach respective List Format
        AFTER we imported the Styles and AFTER we imported the Lists!
    */
    ::SetProgressState(nProgress, mpDocShell); 
    pStyles->PostProcessStyles();

    if (!vColl.empty())
        SetOutLineStyles();

    pSBase = new WW8ScannerBase(pStrm,pTableStream,pDataStream,pWwFib);

    static const SvxExtNumType eNumTA[16] =
    {
        SVX_NUM_ARABIC, SVX_NUM_ROMAN_UPPER, SVX_NUM_ROMAN_LOWER,
        SVX_NUM_CHARS_UPPER_LETTER_N, SVX_NUM_CHARS_LOWER_LETTER_N,
        SVX_NUM_ARABIC, SVX_NUM_ARABIC, SVX_NUM_ARABIC,
        SVX_NUM_ARABIC, SVX_NUM_ARABIC, SVX_NUM_ARABIC,
        SVX_NUM_ARABIC, SVX_NUM_ARABIC, SVX_NUM_ARABIC,
        SVX_NUM_ARABIC, SVX_NUM_ARABIC
    };

    if (pSBase->AreThereFootnotes())
    {
        static const SwFtnNum eNumA[4] =
        {
            FTNNUM_DOC, FTNNUM_CHAPTER, FTNNUM_PAGE, FTNNUM_DOC
        };

        SwFtnInfo aInfo;
        aInfo = rDoc.GetFtnInfo(); 

        aInfo.ePos = FTNPOS_PAGE;
        aInfo.eNum = eNumA[pWDop->rncFtn];
        aInfo.aFmt.SetNumberingType( static_cast< sal_uInt16 >(eNumTA[pWDop->nfcFtnRef]) );
        if( pWDop->nFtn )
            aInfo.nFtnOffset = pWDop->nFtn - 1;
        rDoc.SetFtnInfo( aInfo );
    }
    if( pSBase->AreThereEndnotes() )
    {
        SwEndNoteInfo aInfo;
        aInfo = rDoc.GetEndNoteInfo(); 

        aInfo.aFmt.SetNumberingType( static_cast< sal_uInt16 >(eNumTA[pWDop->nfcEdnRef]) );
        if( pWDop->nEdn )
            aInfo.nFtnOffset = pWDop->nEdn - 1;
        rDoc.SetEndNoteInfo( aInfo );
    }

    if( pWwFib->lcbPlcfhdd )
        pHdFt = new WW8PLCF_HdFt( pTableStream, *pWwFib, *pWDop );

    if (!mbNewDoc)
    {
        
        
        
        const SwPosition* pPos = pPaM->GetPoint();

        
        rDoc.SplitNode( *pPos, false );

        
        SwTxtNode const*const pTxtNd = pPos->nNode.GetNode().GetTxtNode();
        if ( pTxtNd->GetTxt().getLength() )
        {
            rDoc.SplitNode( *pPos, false );
            
            pPaM->Move( fnMoveBackward );
        }

        
        const sal_uLong nNd = pPos->nNode.GetIndex();
        bReadNoTbl = ( nNd < rDoc.GetNodes().GetEndOfInserts().GetIndex() &&
                       rDoc.GetNodes().GetEndOfInserts().StartOfSectionIndex() < nNd );
    }

    ::SetProgressState(nProgress, mpDocShell); 

    
    if (pGloss)
    {
        WW8PLCF aPlc(*pTableStream, pWwFib->fcPlcfglsy, pWwFib->lcbPlcfglsy, 0);

        WW8_CP nStart, nEnd;
        void* pDummy;

        for (int i = 0; i < pGloss->GetNoStrings(); ++i, aPlc.advance())
        {
            SwNodeIndex aIdx( rDoc.GetNodes().GetEndOfContent());
            SwTxtFmtColl* pColl =
                rDoc.GetTxtCollFromPool(RES_POOLCOLL_STANDARD,
                false);
            SwStartNode *pNode =
                rDoc.GetNodes().MakeTextSection(aIdx,
                SwNormalStartNode,pColl);
            pPaM->GetPoint()->nNode = pNode->GetIndex()+1;
            pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(),0);
            aPlc.Get( nStart, nEnd, pDummy );
            ReadText(nStart,nEnd-nStart-1,MAN_MAINTEXT);
        }
    }
    else 
    {
        if (mbNewDoc && pStg && !pGloss) /*meaningless for a glossary, cmc*/
        {
            mpDocShell->SetIsTemplate( pWwFib->fDot ); 
            uno::Reference<document::XDocumentPropertiesSupplier> const
                xDocPropSupp(mpDocShell->GetModel(), uno::UNO_QUERY_THROW);
            uno::Reference< document::XDocumentProperties > xDocProps( xDocPropSupp->getDocumentProperties(), uno::UNO_QUERY_THROW );

            OUString sCreatedFrom = xDocProps->getTemplateURL();
            uno::Reference< container::XNameContainer > xPrjNameCache;
            uno::Reference< lang::XMultiServiceFactory> xSF(mpDocShell->GetModel(), uno::UNO_QUERY);
            if ( xSF.is() )
                xPrjNameCache.set( xSF->createInstance( "ooo.vba.VBAProjectNameProvider" ), uno::UNO_QUERY );

            
            ReadGlobalTemplateSettings( sCreatedFrom, xPrjNameCache );

            
            uno::Any aGlobs;
            uno::Sequence< uno::Any > aArgs(1);
            aArgs[ 0 ] <<= mpDocShell->GetModel();
            aGlobs <<= ::comphelper::getProcessServiceFactory()->createInstanceWithArguments( "ooo.vba.word.Globals", aArgs );

#ifndef DISABLE_SCRIPTING
            BasicManager *pBasicMan = mpDocShell->GetBasicManager();
            if (pBasicMan)
                pBasicMan->SetGlobalUNOConstant( "VBAGlobals", aGlobs );
#endif
            BasicProjImportHelper aBasicImporter( *mpDocShell );
            
            bool bRet = aBasicImporter.import( mpDocShell->GetMedium()->GetInputStream() );

            lcl_createTemplateToProjectEntry( xPrjNameCache, sCreatedFrom, aBasicImporter.getProjectName() );
            WW8Customizations aCustomisations( pTableStream, *pWwFib );
            aCustomisations.Import( mpDocShell );

            if( bRet )
                rDoc.SetContainsMSVBasic(true);

            StoreMacroCmds();
        }
        mbOnLoadingMain = true;
        ReadText(0, pWwFib->ccpText, MAN_MAINTEXT);
        mbOnLoadingMain = false;
    }

    ::SetProgressState(nProgress, mpDocShell); 

    if (pDrawPg && pMSDffManager && pMSDffManager->GetShapeOrders())
    {
        
        SvxMSDffShapeTxBxSort aTxBxSort;

        
        sal_uInt16 nShapeCount = pMSDffManager->GetShapeOrders()->size();

        for (sal_uInt16 nShapeNum=0; nShapeNum < nShapeCount; nShapeNum++)
        {
            SvxMSDffShapeOrder *pOrder =
                (*pMSDffManager->GetShapeOrders())[nShapeNum];
            
            if (pOrder->nTxBxComp && pOrder->pFly)
                aTxBxSort.insert(pOrder);
        }
        
        if( !aTxBxSort.empty() )
        {
            SwFmtChain aChain;
            for( SvxMSDffShapeTxBxSort::iterator it = aTxBxSort.begin(); it != aTxBxSort.end(); ++it )
            {
                SvxMSDffShapeOrder *pOrder = *it;

                
                SwFlyFrmFmt* pFlyFmt     = pOrder->pFly;
                SwFlyFrmFmt* pNextFlyFmt = 0;
                SwFlyFrmFmt* pPrevFlyFmt = 0;

                
                SvxMSDffShapeTxBxSort::iterator tmpIter1 = it;
                ++tmpIter1;
                if( tmpIter1 != aTxBxSort.end() )
                {
                    SvxMSDffShapeOrder *pNextOrder = *tmpIter1;
                    if ((0xFFFF0000 & pOrder->nTxBxComp)
                           == (0xFFFF0000 & pNextOrder->nTxBxComp))
                        pNextFlyFmt = pNextOrder->pFly;
                }
                
                if( it != aTxBxSort.begin() )
                {
                    SvxMSDffShapeTxBxSort::iterator tmpIter2 = it;
                    --tmpIter2;
                    SvxMSDffShapeOrder *pPrevOrder = *tmpIter2;
                    if ((0xFFFF0000 & pOrder->nTxBxComp)
                           == (0xFFFF0000 & pPrevOrder->nTxBxComp))
                        pPrevFlyFmt = pPrevOrder->pFly;
                }
                
                
                if (pNextFlyFmt || pPrevFlyFmt)
                {
                    aChain.SetNext( pNextFlyFmt );
                    aChain.SetPrev( pPrevFlyFmt );
                    pFlyFmt->SetFmtAttr( aChain );
                }
            }
        }
    }

    if (mbNewDoc)
    {
        if( pWDop->fRevMarking )
            eMode |= nsRedlineMode_t::REDLINE_ON;
        if( pWDop->fRMView )
            eMode |= nsRedlineMode_t::REDLINE_SHOW_DELETE;
    }

    maInsertedTables.DelAndMakeTblFrms();
    maSectionManager.InsertSegments();

    vColl.clear();

    DELETEZ( pStyles );

    if( pFormImpl )
        DeleteFormImpl();
    GrafikDtor();
    DELETEZ( pMSDffManager );
    DELETEZ( pHdFt );
    DELETEZ( pSBase );
    delete pWDop;
    DELETEZ( pFonts );
    delete mpAtnNames;
    delete mpSprmParser;
    ::EndProgress(mpDocShell);

    pDataStream = 0;
    pTableStream = 0;

    DeleteCtrlStk();
    mpRedlineStack->closeall(*pPaM->GetPoint());
    delete mpRedlineStack;
    DeleteAnchorStk();
    DeleteRefStks();

    
    {
        std::vector<const SwGrfNode*> vecBulletGrf;
        std::vector<SwFrmFmt*> vecFrmFmt;

        IDocumentMarkAccess* const pMarkAccess = rDoc.getIDocumentMarkAccess();
        if ( pMarkAccess )
        {
            IDocumentMarkAccess::const_iterator_t ppBkmk = pMarkAccess->findBookmark( "_PictureBullets" );
            if ( ppBkmk != pMarkAccess->getBookmarksEnd() &&
                       IDocumentMarkAccess::GetType( *(ppBkmk->get()) ) == IDocumentMarkAccess::BOOKMARK )
            {
                SwTxtNode* pTxtNode = ppBkmk->get()->GetMarkStart().nNode.GetNode().GetTxtNode();

                if ( pTxtNode )
                {
                    const SwpHints* pHints = pTxtNode->GetpSwpHints();
                    for( sal_uInt16 nHintPos = 0; pHints && nHintPos < pHints->Count(); ++nHintPos)
                    {
                        const SwTxtAttr *pHt = (*pHints)[nHintPos];
                        const sal_Int32 st = *(pHt->GetStart());
                        if( pHt
                            && pHt->Which() == RES_TXTATR_FLYCNT
                            && (st >= ppBkmk->get()->GetMarkStart().nContent.GetIndex()) )
                        {
                            SwFrmFmt* pFrmFmt = pHt->GetFlyCnt().GetFrmFmt();
                            vecFrmFmt.push_back(pFrmFmt);
                            const SwNodeIndex* pNdIdx = pFrmFmt->GetCntnt().GetCntntIdx();
                            const SwNodes* pNodesArray = (pNdIdx != NULL)
                                                         ? &(pNdIdx->GetNodes())
                                                         : NULL;
                            const SwGrfNode *pGrf = (pNodesArray != NULL)
                                                    ? dynamic_cast<const SwGrfNode*>((*pNodesArray)[pNdIdx->GetIndex() + 1])
                                                    : NULL;
                            vecBulletGrf.push_back(pGrf);
                        }
                    }
                    
                    size_t nCount = pLstManager->GetWW8LSTInfoNum();
                    for (size_t i = 0; i < nCount; ++i)
                    {
                        SwNumRule* pRule = pLstManager->GetNumRule(i);
                        for (sal_uInt16 j = 0; j < MAXLEVEL; ++j)
                        {
                            SwNumFmt aNumFmt(pRule->Get(j));
                            const sal_Int16 nType = aNumFmt.GetNumberingType();
                            const sal_uInt16 nGrfBulletCP = aNumFmt.GetGrfBulletCP();
                            if ( nType == SVX_NUM_BITMAP
                                 && vecBulletGrf.size() > nGrfBulletCP
                                 && vecBulletGrf[nGrfBulletCP] != NULL )
                            {
                                Graphic aGraphic = vecBulletGrf[nGrfBulletCP]->GetGrf();
                                SvxBrushItem aBrush(aGraphic, GPOS_AREA, SID_ATTR_BRUSH);
                                Font aFont = numfunc::GetDefBulletFont();
                                int nHeight = aFont.GetHeight() * 12;
                                Size aPrefSize( aGraphic.GetPrefSize());
                                if (aPrefSize.Height() * aPrefSize.Width() != 0 )
                                {
                                    int nWidth = (nHeight * aPrefSize.Width()) / aPrefSize.Height();
                                    Size aSize(nWidth, nHeight);
                                    aNumFmt.SetGraphicBrush(&aBrush, &aSize);
                                }
                                else
                                {
                                    aNumFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
                                    aNumFmt.SetBulletChar(0x2190);
                                }
                                pRule->Set( j, aNumFmt );
                            }
                        }
                    }
                    
                    for (sal_uInt16 i = 0; i < vecFrmFmt.size(); ++i)
                    {
                        rDoc.DelLayoutFmt(vecFrmFmt[i]);
                    }
                }
            }
        }
        DELETEZ( pLstManager );
    }

    
    
    
    m_aExtraneousParas.delete_all_from_doc();

    UpdateFields();

    
    if (mbNewDoc)
      rDoc.SetRedlineMode((RedlineMode_t)( eMode ));

    UpdatePageDescs(rDoc, nPageDescOffset);

    delete pPaM, pPaM = 0;
    return nErrRet;
}

sal_uLong SwWW8ImplReader::SetSubStreams(SvStorageStreamRef &rTableStream,
    SvStorageStreamRef &rDataStream)
{
    sal_uLong nErrRet = 0;
    
    switch (pWwFib->nVersion)
    {
        case 6:
        case 7:
            pTableStream = pStrm;
            pDataStream = pStrm;
            break;
        case 8:
            if(!pStg)
            {
                OSL_ENSURE( pStg, "Version 8 always needs to have a Storage!!" );
                nErrRet = ERR_SWG_READ_ERROR;
                break;
            }

            rTableStream = pStg->OpenSotStream( OUString::createFromAscii(
                pWwFib->fWhichTblStm ? SL::a1Table : SL::a0Table),
                STREAM_STD_READ);

            pTableStream = &rTableStream;
            pTableStream->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

            rDataStream = pStg->OpenSotStream(OUString(SL::aData),
                STREAM_STD_READ | STREAM_NOCREATE );

            if (rDataStream.Is() && SVSTREAM_OK == rDataStream->GetError())
            {
                pDataStream = &rDataStream;
                pDataStream->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
            }
            else
                pDataStream = pStrm;
            break;
        default:
            
            OSL_ENSURE( !this, "We forgot to encode nVersion!" );
            nErrRet = ERR_SWG_READ_ERROR;
            break;
    }
    return nErrRet;
}

namespace
{
    utl::TempFile *MakeTemp(SvFileStream &rSt)
    {
        utl::TempFile *pT = new utl::TempFile;
        pT->EnableKillingFile();
        rSt.Open(pT->GetFileName(), STREAM_READWRITE | STREAM_SHARE_DENYWRITE);
        return pT;
    }

#define WW_BLOCKSIZE 0x200

    void DecryptRC4(msfilter::MSCodec_Std97& rCtx, SvStream &rIn, SvStream &rOut)
    {
        rIn.Seek(STREAM_SEEK_TO_END);
        const sal_Size nLen = rIn.Tell();
        rIn.Seek(0);

        sal_uInt8 in[WW_BLOCKSIZE];
        for (sal_Size nI = 0, nBlock = 0; nI < nLen; nI += WW_BLOCKSIZE, ++nBlock)
        {
            sal_Size nBS = (nLen - nI > WW_BLOCKSIZE) ? WW_BLOCKSIZE : nLen - nI;
            nBS = rIn.Read(in, nBS);
            rCtx.InitCipher(nBlock);
            rCtx.Decode(in, nBS, in, nBS);
            rOut.Write(in, nBS);
        }
    }

    void DecryptXOR(msfilter::MSCodec_XorWord95 &rCtx, SvStream &rIn, SvStream &rOut)
    {
        sal_Size nSt = rIn.Tell();
        rIn.Seek(STREAM_SEEK_TO_END);
        sal_Size nLen = rIn.Tell();
        rIn.Seek(nSt);

        rCtx.InitCipher();
        rCtx.Skip(nSt);

        sal_uInt8 in[0x4096];
        for (sal_Size nI = nSt; nI < nLen; nI += 0x4096)
        {
            sal_Size nBS = (nLen - nI > 0x4096 ) ? 0x4096 : nLen - nI;
            nBS = rIn.Read(in, nBS);
            rCtx.Decode(in, nBS);
            rOut.Write(in, nBS);
        }
    }

    
    OUString QueryPasswordForMedium(SfxMedium& rMedium)
    {
        OUString aPassw;

        using namespace com::sun::star;

        const SfxItemSet* pSet = rMedium.GetItemSet();
        const SfxPoolItem *pPasswordItem;

        if(pSet && SFX_ITEM_SET == pSet->GetItemState(SID_PASSWORD, true, &pPasswordItem))
            aPassw = ((const SfxStringItem *)pPasswordItem)->GetValue();
        else
        {
            try
            {
                uno::Reference< task::XInteractionHandler > xHandler( rMedium.GetInteractionHandler() );
                if( xHandler.is() )
                {
                    ::comphelper::DocPasswordRequest* pRequest = new ::comphelper::DocPasswordRequest(
                        ::comphelper::DocPasswordRequestType_MS, task::PasswordRequestMode_PASSWORD_ENTER,
                        INetURLObject( rMedium.GetOrigURL() ).GetName( INetURLObject::DECODE_WITH_CHARSET ) );
                    uno::Reference< task::XInteractionRequest > xRequest( pRequest );

                    xHandler->handle( xRequest );

                    if( pRequest->isPassword() )
                        aPassw = pRequest->getPassword();
                }
            }
            catch( const uno::Exception& )
            {
            }
        }

        return aPassw;
    }

    uno::Sequence< beans::NamedValue > InitXorWord95Codec( ::msfilter::MSCodec_XorWord95& rCodec, SfxMedium& rMedium, WW8Fib* pWwFib )
    {
        uno::Sequence< beans::NamedValue > aEncryptionData;
        SFX_ITEMSET_ARG( rMedium.GetItemSet(), pEncryptionData, SfxUnoAnyItem, SID_ENCRYPTIONDATA, false );
        if ( pEncryptionData && ( pEncryptionData->GetValue() >>= aEncryptionData ) && !rCodec.InitCodec( aEncryptionData ) )
            aEncryptionData.realloc( 0 );

        if ( !aEncryptionData.getLength() )
        {
            OUString sUniPassword = QueryPasswordForMedium( rMedium );

            OString sPassword(OUStringToOString(sUniPassword,
                WW8Fib::GetFIBCharset(pWwFib->chseTables)));

            sal_Int32 nLen = sPassword.getLength();
            if( nLen <= 15 )
            {
                sal_uInt8 pPassword[16];
                memcpy(pPassword, sPassword.getStr(), nLen);
                memset(pPassword+nLen, 0, sizeof(pPassword)-nLen);

                rCodec.InitKey( pPassword );
                aEncryptionData = rCodec.GetEncryptionData();

                
                
                
                
                
                ::msfilter::MSCodec_Std97 aCodec97;

                
                TimeValue aTime;
                osl_getSystemTime( &aTime );
                rtlRandomPool aRandomPool = rtl_random_createPool();
                rtl_random_addBytes ( aRandomPool, &aTime, 8 );

                sal_uInt8 pDocId[ 16 ];
                rtl_random_getBytes( aRandomPool, pDocId, 16 );

                rtl_random_destroyPool( aRandomPool );

                sal_uInt16 pStd97Pass[16];
                memset( pStd97Pass, 0, sizeof( pStd97Pass ) );
                for( sal_Int32 nChar = 0; nChar < nLen; ++nChar )
                    pStd97Pass[nChar] = sUniPassword[nChar];

                aCodec97.InitKey( pStd97Pass, pDocId );

                
                ::comphelper::SequenceAsHashMap aEncryptionHash( aEncryptionData );
                aEncryptionHash.update( ::comphelper::SequenceAsHashMap( aCodec97.GetEncryptionData() ) );
                aEncryptionHash >> aEncryptionData;
            }
        }

        return aEncryptionData;
    }

    uno::Sequence< beans::NamedValue > InitStd97Codec( ::msfilter::MSCodec_Std97& rCodec, sal_uInt8 pDocId[16], SfxMedium& rMedium )
    {
        uno::Sequence< beans::NamedValue > aEncryptionData;
        SFX_ITEMSET_ARG( rMedium.GetItemSet(), pEncryptionData, SfxUnoAnyItem, SID_ENCRYPTIONDATA, false );
        if ( pEncryptionData && ( pEncryptionData->GetValue() >>= aEncryptionData ) && !rCodec.InitCodec( aEncryptionData ) )
            aEncryptionData.realloc( 0 );

        if ( !aEncryptionData.getLength() )
        {
            OUString sUniPassword = QueryPasswordForMedium( rMedium );

            sal_Int32 nLen = sUniPassword.getLength();
            if ( nLen <= 15 )
            {
                sal_Unicode pPassword[16];
                memset( pPassword, 0, sizeof( pPassword ) );
                for( sal_Int32 nChar = 0; nChar < nLen; ++nChar )
                    pPassword[nChar] = sUniPassword[nChar];

                rCodec.InitKey( pPassword, pDocId );
                aEncryptionData = rCodec.GetEncryptionData();
            }
        }

        return aEncryptionData;
    }
}

sal_uLong SwWW8ImplReader::LoadThroughDecryption(SwPaM& rPaM ,WW8Glossary *pGloss)
{
    sal_uLong nErrRet = 0;
    if (pGloss)
        pWwFib = pGloss->GetFib();
    else
        pWwFib = new WW8Fib(*pStrm, nWantedVersion);

    if (pWwFib->nFibError)
        nErrRet = ERR_SWG_READ_ERROR;

    SvStorageStreamRef xTableStream, xDataStream;

    if (!nErrRet)
        nErrRet = SetSubStreams(xTableStream, xDataStream);

    utl::TempFile *pTempMain = 0;
    utl::TempFile *pTempTable = 0;
    utl::TempFile *pTempData = 0;
    SvFileStream aDecryptMain;
    SvFileStream aDecryptTable;
    SvFileStream aDecryptData;

    bool bDecrypt = false;
    enum {RC4, XOR, Other} eAlgo = Other;
    if (pWwFib->fEncrypted && !nErrRet)
    {
        if (!pGloss)
        {
            bDecrypt = true;
            if (8 != pWwFib->nVersion)
                eAlgo = XOR;
            else
            {
                if (pWwFib->nKey != 0)
                    eAlgo = XOR;
                else
                {
                    pTableStream->Seek(0);
                    sal_uInt32 nEncType;
                    pTableStream->ReadUInt32( nEncType );
                    if (nEncType == 0x10001)
                        eAlgo = RC4;
                }
            }
        }
    }

    if (bDecrypt)
    {
        nErrRet = ERRCODE_SVX_WRONGPASS;
        SfxMedium* pMedium = mpDocShell->GetMedium();

        if ( pMedium )
        {
            switch (eAlgo)
            {
                default:
                    nErrRet = ERRCODE_SVX_READ_FILTER_CRYPT;
                    break;
                case XOR:
                {
                    msfilter::MSCodec_XorWord95 aCtx;
                    uno::Sequence< beans::NamedValue > aEncryptionData = InitXorWord95Codec( aCtx, *pMedium, pWwFib );

                    
                    if ( aEncryptionData.getLength() && aCtx.VerifyKey( pWwFib->nKey, pWwFib->nHash ) )
                    {
                        nErrRet = 0;
                        pTempMain = MakeTemp(aDecryptMain);

                        pStrm->Seek(0);
                        size_t nUnencryptedHdr =
                            (8 == pWwFib->nVersion) ? 0x44 : 0x34;
                        sal_uInt8 *pIn = new sal_uInt8[nUnencryptedHdr];
                        nUnencryptedHdr = pStrm->Read(pIn, nUnencryptedHdr);
                        aDecryptMain.Write(pIn, nUnencryptedHdr);
                        delete [] pIn;

                        DecryptXOR(aCtx, *pStrm, aDecryptMain);

                        if (!pTableStream || pTableStream == pStrm)
                            pTableStream = &aDecryptMain;
                        else
                        {
                            pTempTable = MakeTemp(aDecryptTable);
                            DecryptXOR(aCtx, *pTableStream, aDecryptTable);
                            pTableStream = &aDecryptTable;
                        }

                        if (!pDataStream || pDataStream == pStrm)
                            pDataStream = &aDecryptMain;
                        else
                        {
                            pTempData = MakeTemp(aDecryptData);
                            DecryptXOR(aCtx, *pDataStream, aDecryptData);
                            pDataStream = &aDecryptData;
                        }

                        pMedium->GetItemSet()->ClearItem( SID_PASSWORD );
                        pMedium->GetItemSet()->Put( SfxUnoAnyItem( SID_ENCRYPTIONDATA, uno::makeAny( aEncryptionData ) ) );
                    }
                }
                break;
                case RC4:
                {
                    sal_uInt8 aDocId[ 16 ];
                    sal_uInt8 aSaltData[ 16 ];
                    sal_uInt8 aSaltHash[ 16 ];

                    bool bCouldReadHeaders =
                        checkRead(*pTableStream, aDocId, 16) &&
                        checkRead(*pTableStream, aSaltData, 16) &&
                        checkRead(*pTableStream, aSaltHash, 16);

                    msfilter::MSCodec_Std97 aCtx;
                    
                    uno::Sequence< beans::NamedValue > aEncryptionData;
                    if (bCouldReadHeaders)
                        aEncryptionData = InitStd97Codec( aCtx, aDocId, *pMedium );
                    if ( aEncryptionData.getLength() && aCtx.VerifyKey( aSaltData, aSaltHash ) )
                    {
                        nErrRet = 0;

                        pTempMain = MakeTemp(aDecryptMain);

                        pStrm->Seek(0);
                        sal_Size nUnencryptedHdr = 0x44;
                        sal_uInt8 *pIn = new sal_uInt8[nUnencryptedHdr];
                        nUnencryptedHdr = pStrm->Read(pIn, nUnencryptedHdr);

                        DecryptRC4(aCtx, *pStrm, aDecryptMain);

                        aDecryptMain.Seek(0);
                        aDecryptMain.Write(pIn, nUnencryptedHdr);
                        delete [] pIn;


                        pTempTable = MakeTemp(aDecryptTable);
                        DecryptRC4(aCtx, *pTableStream, aDecryptTable);
                        pTableStream = &aDecryptTable;

                        if (!pDataStream || pDataStream == pStrm)
                            pDataStream = &aDecryptMain;
                        else
                        {
                            pTempData = MakeTemp(aDecryptData);
                            DecryptRC4(aCtx, *pDataStream, aDecryptData);
                            pDataStream = &aDecryptData;
                        }

                        pMedium->GetItemSet()->ClearItem( SID_PASSWORD );
                        pMedium->GetItemSet()->Put( SfxUnoAnyItem( SID_ENCRYPTIONDATA, uno::makeAny( aEncryptionData ) ) );
                    }
                }
                break;
            }
        }

        if (nErrRet == 0)
        {
            pStrm = &aDecryptMain;

            delete pWwFib;
            pWwFib = new WW8Fib(*pStrm, nWantedVersion);
            if (pWwFib->nFibError)
                nErrRet = ERR_SWG_READ_ERROR;
        }
    }

    if (!nErrRet)
        nErrRet = CoreLoad(pGloss, *rPaM.GetPoint());

    delete pTempMain;
    delete pTempTable;
    delete pTempData;

    if (!pGloss)
        delete pWwFib;
    return nErrRet;
}

class outlineeq : public std::unary_function<const SwTxtFmtColl*, bool>
{
private:
    sal_uInt8 mnNum;
public:
    outlineeq(sal_uInt8 nNum) : mnNum(nNum) {}
    bool operator()(const SwTxtFmtColl *pTest) const
    {
        return pTest->IsAssignedToListLevelOfOutlineStyle() && pTest->GetAssignedOutlineStyleLevel() == mnNum;
    }
};

void SwWW8ImplReader::SetOutLineStyles()
{
    /*
    #i3674# - Load new document and insert document cases.
    */
    SwNumRule aOutlineRule(*rDoc.GetOutlineNumRule());
    
    
    
    
    
    mpChosenOutlineNumRule = &aOutlineRule;

    sw::ParaStyles aOutLined(sw::util::GetParaStyles(rDoc));
    
    sw::util::SortByAssignedOutlineStyleListLevel(aOutLined);

    typedef sw::ParaStyleIter myParaStyleIter;
    /*
    If we are inserted into a document then don't clobber existing existing
    levels.
    */
    sal_uInt16 nFlagsStyleOutlLevel = 0;
    if (!mbNewDoc)
    {
        
        sw::ParaStyles::reverse_iterator aEnd = aOutLined.rend();
        for ( sw::ParaStyles::reverse_iterator aIter = aOutLined.rbegin(); aIter < aEnd; ++aIter)
        {
            if ((*aIter)->IsAssignedToListLevelOfOutlineStyle())
                nFlagsStyleOutlLevel |= 1 << (*aIter)->GetAssignedOutlineStyleLevel();
            else
                break;
        }
    }
    else
    {
        /*
        Only import *one* of the possible multiple outline numbering rules, so
        pick the one that affects most styles. If we're not importing a new
        document, we got to stick with what is already there.
        */
        
        
        
        
        
        std::map<sal_uInt16, int>aRuleMap;
        typedef std::map<sal_uInt16, int>::iterator myIter;
        for (sal_uInt16 nI = 0; nI < vColl.size(); ++nI)
        {
            SwWW8StyInf& rSI = vColl[ nI ];
            if (
                (MAXLEVEL > rSI.nOutlineLevel) && rSI.pOutlineNumrule &&
                rSI.pFmt
               )
            {
                myIter aIter = aRuleMap.find(nI);
                if (aIter == aRuleMap.end())
                {
                    aRuleMap[nI] = 1;
                }
                else
                    ++(aIter->second);
            }
        }

        int nMax = 0;
        myIter aEnd2 = aRuleMap.end();
        for (myIter aIter = aRuleMap.begin(); aIter != aEnd2; ++aIter)
        {
            if (aIter->second > nMax)
            {
                nMax = aIter->second;
                if(aIter->first < vColl.size())
                    mpChosenOutlineNumRule = vColl[ aIter->first ].pOutlineNumrule;
                else
                    mpChosenOutlineNumRule = 0; 
            }
        }

        OSL_ENSURE(mpChosenOutlineNumRule, "Impossible");
        if (mpChosenOutlineNumRule)
            aOutlineRule = *mpChosenOutlineNumRule;

        if (mpChosenOutlineNumRule != &aOutlineRule)
        {
            
            sw::ParaStyles::reverse_iterator aEnd = aOutLined.rend();
            for ( sw::ParaStyles::reverse_iterator aIter = aOutLined.rbegin(); aIter < aEnd; ++aIter)
            {
                if((*aIter)->IsAssignedToListLevelOfOutlineStyle())
                    (*aIter)->DeleteAssignmentToListLevelOfOutlineStyle();

                else
                    break;
            }
        }
    }

    sal_uInt16 nOldFlags = nFlagsStyleOutlLevel;

    for (sal_uInt16 nI = 0; nI < vColl.size(); ++nI)
    {
        SwWW8StyInf& rSI = vColl[nI];

        if (rSI.IsOutlineNumbered())
        {
            sal_uInt16 nAktFlags = 1 << rSI.nOutlineLevel;
            if (
                 (nAktFlags & nFlagsStyleOutlLevel) ||
                 (rSI.pOutlineNumrule != mpChosenOutlineNumRule)
               )
            {
                /*
                If our spot is already taken by something we can't replace
                then don't insert and remove our outline level.
                */
                rSI.pFmt->SetFmtAttr(
                        SwNumRuleItem( rSI.pOutlineNumrule->GetName() ) );
                ((SwTxtFmtColl*)rSI.pFmt)->DeleteAssignmentToListLevelOfOutlineStyle(); 
            }
            else
            {
                /*
                If there is a style already set for this outline
                numbering level and its not a style set by us already
                then we can remove it outline numbering.
                (its one of the default headings in a new document
                so we can clobber it)
                Of course if we are being inserted into a document that
                already has some set we can't do this, thats covered by
                the list of level in nFlagsStyleOutlLevel to ignore.
                */
                outlineeq aCmp(rSI.nOutlineLevel);
                myParaStyleIter aResult = std::find_if(aOutLined.begin(),
                    aOutLined.end(), aCmp);

                myParaStyleIter aEnd = aOutLined.end();
                while (aResult != aEnd  && aCmp(*aResult))
                {
                    (*aResult)->DeleteAssignmentToListLevelOfOutlineStyle();
                    ++aResult;
                }

                /*
                #i1886#
                I believe that when a list is registered onto a winword
                style which is an outline numbering style (i.e.
                nOutlineLevel is set) that the style of numbering is for
                the level is indexed by the *list* level that was
                registered on that style, and not the outlinenumbering
                level, which is probably a logical sequencing, and not a
                physical mapping into the list style reged on that outline
                style.
                */
                sal_uInt8 nFromLevel = rSI.nListLevel;
                sal_uInt8 nToLevel = rSI.nOutlineLevel;
                const SwNumFmt& rRule=rSI.pOutlineNumrule->Get(nFromLevel);
                aOutlineRule.Set(nToLevel, rRule);
                ((SwTxtFmtColl*)rSI.pFmt)->AssignToListLevelOfOutlineStyle(nToLevel);
                
                nFlagsStyleOutlLevel |= nAktFlags;
            }
        }
    }
    if (nOldFlags != nFlagsStyleOutlLevel)
        rDoc.SetOutlineNumRule(aOutlineRule);
    
    if ( mpChosenOutlineNumRule == &aOutlineRule )
    {
        mpChosenOutlineNumRule = rDoc.GetOutlineNumRule();
    }
}

const OUString* SwWW8ImplReader::GetAnnotationAuthor(sal_uInt16 nIdx)
{
    if (!mpAtnNames && pWwFib->lcbGrpStAtnOwners)
    {
        
        mpAtnNames = new ::std::vector<OUString>;
        SvStream& rStrm = *pTableStream;

        long nOldPos = rStrm.Tell();
        rStrm.Seek( pWwFib->fcGrpStAtnOwners );

        long nRead = 0, nCount = pWwFib->lcbGrpStAtnOwners;
        while (nRead < nCount)
        {
            if( bVer67 )
            {
                mpAtnNames->push_back(read_uInt8_PascalString(rStrm,
                    RTL_TEXTENCODING_MS_1252));
                nRead += mpAtnNames->rbegin()->getLength() + 1; 
            }
            else
            {
                mpAtnNames->push_back(read_uInt16_PascalString(rStrm));
                
                nRead += (mpAtnNames->rbegin()->getLength() + 1)*2;
            }
        }
        rStrm.Seek( nOldPos );
    }

    const OUString *pRet = 0;
    if (mpAtnNames && nIdx < mpAtnNames->size())
        pRet = &((*mpAtnNames)[nIdx]);
    return pRet;
}

int SwWW8ImplReader::GetAnnotationIndex(sal_uInt32 nTag)
{
    if (!mpAtnIndexes.get() && pWwFib->lcbSttbfAtnbkmk)
    {
        mpAtnIndexes.reset(new std::map<sal_uInt32, int>());
        std::vector<OUString> aStrings;
        std::vector<ww::bytes> aEntries;
        WW8ReadSTTBF(!bVer67, *pTableStream, pWwFib->fcSttbfAtnbkmk, pWwFib->lcbSttbfAtnbkmk, sizeof(struct WW8_ATNBE), eStructCharSet, aStrings, &aEntries);
        for (size_t i = 0; i < aStrings.size() && i < aEntries.size(); ++i)
        {
            ww::bytes aEntry = aEntries[i];
            WW8_ATNBE* pAtnbeStruct = (WW8_ATNBE*)(&aEntry[0]);
            mpAtnIndexes->insert(std::pair<sal_uInt32, int>(SVBT32ToUInt32(pAtnbeStruct->nTag), i));
        }
    }
    if (mpAtnIndexes.get())
    {
        std::map<sal_uInt32, int>::iterator it = mpAtnIndexes->find(nTag);
        if (it != mpAtnIndexes->end())
            return it->second;
    }
    return -1;
}

sal_uInt16 SwWW8ImplReader::GetAnnotationEndIndex(sal_uInt16 nStart)
{
    WW8_CP nStartAkt;
    void* p;
    if (mpAtnStarts->GetData(nStart, nStartAkt, p) && p)
    {
        
        return SVBT16ToShort(*((SVBT16*)p));
    }
    return nStart;
}

WW8_CP SwWW8ImplReader::GetAnnotationStart(int nIndex)
{
    if (!mpAtnStarts.get() && pWwFib->lcbPlcfAtnbkf)
        
        mpAtnStarts.reset(new WW8PLCFspecial(pTableStream, pWwFib->fcPlcfAtnbkf, pWwFib->lcbPlcfAtnbkf, 4));

    if (mpAtnStarts.get())
        return mpAtnStarts->GetPos(nIndex);
    else
        return SAL_MAX_INT32;
}

WW8_CP SwWW8ImplReader::GetAnnotationEnd(int nIndex)
{
    if (!mpAtnEnds.get() && pWwFib->lcbPlcfAtnbkl)
        
        mpAtnEnds.reset(new WW8PLCFspecial(pTableStream, pWwFib->fcPlcfAtnbkl, pWwFib->lcbPlcfAtnbkl, 0));

    if (mpAtnEnds.get())
        return mpAtnEnds->GetPos(nIndex);
    else
        return SAL_MAX_INT32;
}

sal_uLong SwWW8ImplReader::LoadDoc( SwPaM& rPaM,WW8Glossary *pGloss)
{
    sal_uLong nErrRet = 0;

    {
        static const sal_Char* aNames[ 13 ] = {
            "WinWord/WW", "WinWord/WW8", "WinWord/WWFT",
            "WinWord/WWFLX", "WinWord/WWFLY",
            "WinWord/WWF",
            "WinWord/WWFA0", "WinWord/WWFA1", "WinWord/WWFA2",
            "WinWord/WWFB0", "WinWord/WWFB1", "WinWord/WWFB2",
            "WinWord/RegardHindiDigits"
        };
        sal_uInt32 aVal[ 13 ];

        SwFilterOptions aOpt( 13, aNames, aVal );

        nIniFlags = aVal[ 0 ];
        nIniFlags1= aVal[ 1 ];
        
        nIniFlyDx = aVal[ 3 ];
        nIniFlyDy = aVal[ 4 ];

        nFieldFlags = aVal[ 5 ];
        nFieldTagAlways[0] = aVal[ 6 ];
        nFieldTagAlways[1] = aVal[ 7 ];
        nFieldTagAlways[2] = aVal[ 8 ];
        nFieldTagBad[0] = aVal[ 9 ];
        nFieldTagBad[1] = aVal[ 10 ];
        nFieldTagBad[2] = aVal[ 11 ];
        m_bRegardHindiDigits = aVal[ 12 ] > 0;
    }

    sal_uInt16 nMagic(0);
    pStrm->ReadUInt16( nMagic );

    
    switch (nWantedVersion)
    {
        case 6:
        case 7:
            if (
                (0xa5dc != nMagic && 0xa5db != nMagic) &&
                (nMagic < 0xa697 || nMagic > 0xa699)
               )
            {
                
                if (pStg && 0xa5ec == nMagic)
                {
                    sal_uLong nCurPos = pStrm->Tell();
                    if (pStrm->Seek(nCurPos + 22))
                    {
                        sal_uInt32 nfcMin;
                        pStrm->ReadUInt32( nfcMin );
                        if (0x300 != nfcMin)
                            nErrRet = ERR_WW6_NO_WW6_FILE_ERR;
                    }
                    pStrm->Seek( nCurPos );
                }
                else
                    nErrRet = ERR_WW6_NO_WW6_FILE_ERR;
            }
            break;
        case 8:
            if (0xa5ec != nMagic)
                nErrRet = ERR_WW8_NO_WW8_FILE_ERR;
            break;
        default:
            nErrRet = ERR_WW8_NO_WW8_FILE_ERR;
            OSL_ENSURE( !this, "We forgot to encode nVersion!" );
            break;
    }

    if (!nErrRet)
        nErrRet = LoadThroughDecryption(rPaM ,pGloss);

    rDoc.PropagateOutlineRule();

    return nErrRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT Reader* SAL_CALL ImportDOC()
{
    return new WW8Reader();
}

sal_uLong WW8Reader::OpenMainStream( SvStorageStreamRef& rRef, sal_uInt16& rBuffSize )
{
    sal_uLong nRet = ERR_SWG_READ_ERROR;
    OSL_ENSURE( pStg, "Where is my Storage?" );
    rRef = pStg->OpenSotStream( OUString("WordDocument"), STREAM_READ | STREAM_SHARE_DENYALL);

    if( rRef.Is() )
    {
        if( SVSTREAM_OK == rRef->GetError() )
        {
            sal_uInt16 nOld = rRef->GetBufferSize();
            rRef->SetBufferSize( rBuffSize );
            rBuffSize = nOld;
            nRet = 0;
        }
        else
            nRet = rRef->GetError();
    }
    return nRet;
}

sal_uLong WW8Reader::Read(SwDoc &rDoc, const OUString& rBaseURL, SwPaM &rPam, const OUString & /* FileName */)
{
    sal_uInt16 nOldBuffSize = 32768;
    bool bNew = !bInsertMode; 

    SvStorageStreamRef refStrm; 
    SvStream* pIn = pStrm;

    sal_uLong nRet = 0;
    sal_uInt8 nVersion = 8;

    const OUString sFltName = GetFltName();
    if ( sFltName=="WW6" )
    {
        if (pStrm)
            nVersion = 6;
        else
        {
            OSL_ENSURE(!this, "WinWord 95 Reader-Read without Stream");
            nRet = ERR_SWG_READ_ERROR;
        }
    }
    else
    {
        if ( sFltName=="CWW6" )
            nVersion = 6;
        else if ( sFltName=="CWW7" )
            nVersion = 7;

        if( pStg )
        {
            nRet = OpenMainStream( refStrm, nOldBuffSize );
            pIn = &refStrm;
        }
        else
        {
            OSL_ENSURE(!this, "WinWord 95/97 Reader-Read without Storage");
            nRet = ERR_SWG_READ_ERROR;
        }
    }

    if( !nRet )
    {
        if (bNew)
        {
            
            Reader::ResetFrmFmts( rDoc );
        }
        SwWW8ImplReader* pRdr = new SwWW8ImplReader(nVersion, pStg, pIn, rDoc,
            rBaseURL, bNew);
        try
        {
            nRet = pRdr->LoadDoc( rPam );
        }
        catch( const std::exception& )
        {
            nRet = ERR_WW8_NO_WW8_FILE_ERR;
        }
        delete pRdr;

        if( refStrm.Is() )
        {
            refStrm->SetBufferSize( nOldBuffSize );
            refStrm.Clear();
        }
        else
        {
            pIn->ResetError();
        }

    }
    return nRet;
}

int WW8Reader::GetReaderType()
{
    return SW_STORAGE_READER | SW_STREAM_READER;
}

sal_Bool WW8Reader::HasGlossaries() const
{
    return true;
}

sal_Bool WW8Reader::ReadGlossaries(SwTextBlocks& rBlocks, sal_Bool bSaveRelFiles) const
{
    bool bRet=false;

    WW8Reader *pThis = const_cast<WW8Reader *>(this);

    sal_uInt16 nOldBuffSize = 32768;
    SvStorageStreamRef refStrm;
    if (!pThis->OpenMainStream(refStrm, nOldBuffSize))
    {
        WW8Glossary aGloss( refStrm, 8, pStg );
        bRet = aGloss.Load( rBlocks, bSaveRelFiles ? true : false);
    }
    return bRet ? true : false;
}

sal_Bool SwMSDffManager::GetOLEStorageName(long nOLEId, OUString& rStorageName,
    SvStorageRef& rSrcStorage, uno::Reference < embed::XStorage >& rDestStorage) const
{
    bool bRet = false;

    sal_Int32 nPictureId = 0;
    if (rReader.pStg)
    {
        
        
        
        
        long nOldPos = rReader.pStrm->Tell();
        {
            
            
            
            
            
            WW8_CP nStartCp, nEndCp;
            if ( rReader.GetTxbxTextSttEndCp(nStartCp, nEndCp,
                            static_cast<sal_uInt16>((nOLEId >> 16) & 0xFFFF),
                            static_cast<sal_uInt16>(nOLEId & 0xFFFF)) )
            {
                WW8PLCFxSaveAll aSave;
                memset( &aSave, 0, sizeof( aSave ) );
                rReader.pPlcxMan->SaveAllPLCFx( aSave );

                nStartCp += rReader.nDrawCpO;
                nEndCp   += rReader.nDrawCpO;
                WW8PLCFx_Cp_FKP* pChp = rReader.pPlcxMan->GetChpPLCF();
                wwSprmParser aSprmParser(rReader.pWwFib->GetFIBVersion());
                while (nStartCp <= nEndCp && !nPictureId)
                {
                    WW8PLCFxDesc aDesc;
                    pChp->SeekPos( nStartCp );
                    pChp->GetSprms( &aDesc );

                    if (aDesc.nSprmsLen && aDesc.pMemPos) 
                    {
                        long nLen = aDesc.nSprmsLen;
                        const sal_uInt8* pSprm = aDesc.pMemPos;

                        while (nLen >= 2 && !nPictureId)
                        {
                            sal_uInt16 nId = aSprmParser.GetSprmId(pSprm);
                            sal_uInt16 nSL = aSprmParser.GetSprmSize(nId, pSprm);

                            if( nLen < nSL )
                                break; 

                            if( 0x6A03 == nId && 0 < nLen )
                            {
                                nPictureId = SVBT32ToUInt32(pSprm +
                                    aSprmParser.DistanceToData(nId));
                                bRet = true;
                            }
                            pSprm += nSL;
                            nLen -= nSL;
                        }
                    }
                    nStartCp = aDesc.nEndPos;
                }

                rReader.pPlcxMan->RestoreAllPLCFx( aSave );
            }
        }
        rReader.pStrm->Seek( nOldPos );
    }

    if( bRet )
    {
        rStorageName = OUString('_');
        rStorageName += OUString::number(nPictureId);
        rSrcStorage = rReader.pStg->OpenSotStorage(OUString(
            SL::aObjectPool));
        if (!rReader.mpDocShell)
            bRet=false;
        else
            rDestStorage = rReader.mpDocShell->GetStorage();
    }
    return bRet;
}

/**
 * When reading a single Box (which possibly is part of a group), we do
 * not yet have enough information to decide whether we need it as a TextField
 * or not.
 * So convert all of them as a precaution.
 * FIXME: Actually implement this!
 */
sal_Bool SwMSDffManager::ShapeHasText(sal_uLong, sal_uLong) const
{
    return true;
}

bool SwWW8ImplReader::InEqualOrHigherApo(int nLvl) const
{
    if (nLvl)
        --nLvl;
    
    if ( sal::static_int_cast< sal_Int32>(nLvl) >= sal::static_int_cast< sal_Int32>(maApos.size()) )
    {
        return false;
    }
    mycApoIter aIter = std::find(maApos.begin() + nLvl, maApos.end(), true);
    if (aIter != maApos.end())
        return true;
    else
        return false;
}

bool SwWW8ImplReader::InEqualApo(int nLvl) const
{
    
    if (nLvl)
        --nLvl;
    if (nLvl < 0 || static_cast<size_t>(nLvl) >= maApos.size())
        return false;
    return maApos[nLvl];
}

namespace sw
{
    namespace hack
    {
        Position::Position(const SwPosition &rPos)
            : maPtNode(rPos.nNode), mnPtCntnt(rPos.nContent.GetIndex())
        {
        }

        Position::Position(const Position &rPos)
            : maPtNode(rPos.maPtNode), mnPtCntnt(rPos.mnPtCntnt)
        {
        }

        Position::operator SwPosition() const
        {
            SwPosition aRet(maPtNode);
            aRet.nContent.Assign(maPtNode.GetNode().GetCntntNode(), mnPtCntnt);
            return aRet;
        }
    }
}

SwMacroInfo::SwMacroInfo()
    : SdrObjUserData( SW_DRAWLAYER, SW_UD_IMAPDATA, 0 )
    , mnShapeId(-1)

{
}

SwMacroInfo::~SwMacroInfo()
{
}

SdrObjUserData* SwMacroInfo::Clone( SdrObject* /*pObj*/ ) const
{
   return new SwMacroInfo( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
