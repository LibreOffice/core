/*************************************************************************
 *
 *  $RCSfile: eppt.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: sj $ $Date: 2001-04-04 15:56:11 $
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

#ifndef _EPPT_HXX_
#include <eppt.hxx>
#endif
#ifndef _EPPT_DEF_HXX
#include "epptdef.hxx"
#endif
#ifndef _GLOBNAME_HXX
#include <tools/globname.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif
#ifndef _SV_POLY_HXX
#include <vcl/poly.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif
#ifndef _SV_GRADIENT_HXX
#include <vcl/gradient.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _FLTCALL_HXX
#include <svtools/fltcall.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif
#ifndef _DINFOS2_HXX_
#include <dinfos2.hxx>
#endif
#ifndef _COM_SUN_STAR_VIEW_PAPERORIENTATION_HPP_
#include <com/sun/star/view/PaperOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_PAPERFORMAT_HPP_
#include <com/sun/star/view/PaperFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif
#ifndef _ZCODEC_HXX
#include <tools/zcodec.hxx>
#endif
#ifndef _IPOBJ_HXX
#include <so3/ipobj.hxx>
#endif
#ifndef _SVX_SVXENUM_HXX
#include <svx/svxenum.hxx>
#endif
#ifndef _SOT_STORINFO_HXX
#include <sot/storinfo.hxx>
#endif
#ifndef _MSOLEEXP_HXX
#include <svx/msoleexp.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif
#include <svtools/wmf.hxx>

#define PPT_TRANSITION_TYPE_NONE            0
#define PPT_TRANSITION_TYPE_RANDOM          1
#define PPT_TRANSITION_TYPE_BLINDS          2
#define PPT_TRANSITION_TYPE_CHECKER         3
#define PPT_TRANSITION_TYPE_COVER           4
#define PPT_TRANSITION_TYPE_DISSOLVE        5
#define PPT_TRANSITION_TYPE_FADE            6
#define PPT_TRANSITION_TYPE_PULL            7
#define PPT_TRANSITION_TYPE_RANDOM_BARS     8
#define PPT_TRANSITION_TYPE_STRIPS          9
#define PPT_TRANSITION_TYPE_WIPE           10
#define PPT_TRANSITION_TYPE_ZOOM           11
#define PPT_TRANSITION_TYPE_SPLIT          13


//============================ PPTWriter ==================================

PPTWriter::PPTWriter( SvStorageRef& rSvStorage, SvStorageRef& xOleSource,
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & rXModel,
            ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > & rXStatInd,
            SvMemoryStream* pVBA, sal_uInt32 nCnvrtFlags ) :
    mbStatus                ( FALSE ),
    mXModel                 ( rXModel ),
    mXStatusIndicator       ( rXStatInd ),
    mnLatestStatValue       ( 0 ),
    mbStatusIndicator       ( FALSE ),
    mpCurUserStrm           ( NULL ),
    mpStrm                  ( NULL ),
    mpPicStrm               ( NULL ),
    mpPptEscherEx           ( NULL ),
    maFraction              ( 1, 576 ),
    maMapModeSrc            ( MAP_100TH_MM ),
    maMapModeDest           ( MAP_INCH, Point(), maFraction, maFraction ),
    meLatestPageType        ( NORMAL ),
    mnTxId                  ( 0x7a2f64 ),
    mnFillColor             ( 0xffffff ),
    mnFillBackColor         ( 0x000000 ),
    mnPagesWritten          ( 0 ),
    mnMasterTitleIndex      ( 0xffffffff ),
    mnMasterBodyIndex       ( 0xffffffff ),
    mpStyleSheet            ( NULL ),
    mnVBAOleOfs             ( 0 ),
    mnExEmbed               ( 0 ),
    mpVBA                   ( pVBA ),
    mpExEmbed               ( new SvMemoryStream )
{
    sal_uInt32 i;
    if ( !ImplInitSOIface() )
        return;

    FontCollectionEntry aDefaultFontDesc( String( RTL_CONSTASCII_USTRINGPARAM( "Times New Roman" ) ),
                                            ::com::sun::star::awt::FontFamily::ROMAN,
                                                ::com::sun::star::awt::FontPitch::VARIABLE,
                                                    RTL_TEXTENCODING_MS_1252 );
    maFontCollection.GetId( aDefaultFontDesc ); // default is always times new roman

    if ( !ImplGetPageByIndex( 0, NOTICE ) )
        return;
    INT32 nWidth = 21000;
    if ( ImplGetPropertyValue( mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM(  "Width" ) ) ) )
        mAny >>= nWidth;
    INT32 nHeight = 29700;
    if ( ImplGetPropertyValue( mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Height" ) ) ) )
        mAny >>= nHeight;

    maNotesPageSize = ImplMapSize( ::com::sun::star::awt::Size( nWidth, nHeight ) );

    if ( !ImplGetPageByIndex( 0, MASTER ) )
        return;
    nWidth = 28000;
    if ( ImplGetPropertyValue( mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Width" ) ) ) )
        mAny >>= nWidth;
    nHeight = 21000;
    if ( ImplGetPropertyValue( mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Height" ) ) ) )
        mAny >>= nHeight;
    maDestPageSize = ImplMapSize( ::com::sun::star::awt::Size( nWidth, nHeight ) );

    mrStg = rSvStorage;
    if ( !mrStg.Is() )
        return;

    mXSource = xOleSource;
    if ( !mXSource.Is() )
        return;

    // MasterPages + Slides und Notizen + NotesMasterPage
    mnDrawings = mnMasterPages + ( mnPages << 1 ) + 1;

    if ( mXStatusIndicator.is() )
    {
        mbStatusIndicator = TRUE;
        mnStatMaxValue = ( mnPages + mnMasterPages ) * 5;
        mXStatusIndicator->start( String( RTL_CONSTASCII_USTRINGPARAM( "PowerPoint Export" ) ),
                                    mnStatMaxValue + ( mnStatMaxValue >> 3 ) );
    }

    SvGlobalName aGName( 0x64818d10L, 0x4f9b, 0x11cf, 0x86, 0xea, 0x00, 0xaa, 0x00, 0xb9, 0x29, 0xe8 );
    mrStg->SetClass( aGName, 0, String( RTL_CONSTASCII_USTRINGPARAM( "MS PowerPoint 97" ) ) );

    if ( !ImplCreateCurrentUserStream() )
        return;

    if ( !ImplCreateSummaryInformation() )
        return;

    mpStrm = mrStg->OpenStream( String( RTL_CONSTASCII_USTRINGPARAM( "PowerPoint Document" ) ) );
    if ( !mpStrm )
        return;

    if ( !mpPicStrm )
        mpPicStrm = mrStg->OpenStream( String( RTL_CONSTASCII_USTRINGPARAM( "Pictures" ) ) );

    mpPptEscherEx = new PptEscherEx( *mpStrm, mnDrawings );

    ImplGetMasterTitleAndBody();
    if ( !ImplGetStyleSheets() )
        return;

    if ( !ImplCreateDocument() )
        return;

    if ( !ImplCreateMainMaster() )
        return;

    if ( !ImplCreateMainNotes() )
        return;

    for ( i = 1; i < mnMasterPages; i++ )
    {
        if ( !ImplCreateTitleMasterPage( i ) )
            return;
    }
    maTextRuleList.First();                         // rewind list, so we can get the current or next entry without
                                                    // searching, all entrys are sorted#
    for ( i = 0; i < mnPages; i++ )
    {
        if ( !ImplCreateSlide( i ) )
            return;
    }
    for ( i = 0; i < mnPages; i++ )
    {
        if ( !ImplCreateNotes( i ) )
            return;
    }
    if ( !ImplCloseDocument() )
        return;

    if ( mbStatusIndicator )
    {
        mXStatusIndicator->setText( String( RTL_CONSTASCII_USTRINGPARAM( "PowerPoint Export" ) ) );
        sal_uInt32 nValue = mnStatMaxValue + ( mnStatMaxValue >> 3 );
        if ( nValue > mnLatestStatValue )
        {
            mXStatusIndicator->setValue( nValue );
            mnLatestStatValue = nValue;
        }
    }

//  mpPptEscherEx->Flush();         // interne _Escher Daten jetzt einfuegen, damit die PersistTable rausgeschrieben werden kann


    ImplWriteOLE( nCnvrtFlags );

    ImplWriteVBA( pVBA );

    if ( !ImplWriteAtomEnding() )
        return;

    if ( !ImplCreateDocumentSummaryInformation() )
        return;

    mbStatus = TRUE;
};


// ---------------------------------------------------------------------------------------------

PPTWriter::~PPTWriter()
{
    void*  pPtr;
    delete mpExEmbed;
    delete mpPptEscherEx;
    delete mpStyleSheet;

    for ( pPtr = maTextRuleList.First(); pPtr; pPtr = maTextRuleList.Next() )
        delete (TextRuleEntry*)pPtr;
    for ( pPtr = maSlideNameList.First(); pPtr; pPtr = maSlideNameList.Next() )
        delete (::rtl::OUString*)pPtr;
    for ( pPtr = maHyperlink.First(); pPtr; pPtr = maHyperlink.Next() )
        delete (EPPTHyperlink*)pPtr;
    for ( pPtr = maExOleObj.First(); pPtr; pPtr = maExOleObj.Next() )
        delete (PPTExOleObjEntry*)pPtr;

    if ( mbStatusIndicator )
        mXStatusIndicator->end();
}

// ---------------------------------------------------------------------------------------------

static inline sal_uInt32 PPTtoEMU( INT32 nPPT )
{
    return (sal_uInt32)( (double)nPPT * 1587.5 );
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateCurrentUserStream()
{
    mpCurUserStrm = mrStg->OpenStream( String( RTL_CONSTASCII_USTRINGPARAM( "Current User" ) ) );
    if ( !mpCurUserStrm )
        return FALSE;
    char pUserName[] = "Current User";
    sal_uInt32 nLenOfUserName = strlen( pUserName );
    sal_uInt32 nSizeOfRecord = 0x14 + ( ( nLenOfUserName + 4 ) & ~ 3 );

    *mpCurUserStrm << (sal_uInt16)0 << (sal_uInt16)EPP_CurrentUserAtom << nSizeOfRecord;
    *mpCurUserStrm << (sal_uInt32)0x14                  // Len
                   << (sal_uInt32)0xe391c05f;           // Magic

    sal_uInt32 nEditPos = mpCurUserStrm->Tell();
    *mpCurUserStrm << (sal_uInt32)0x0                   // OffsetToCurrentEdit;
                   << (sal_uInt16)nLenOfUserName        //
                   << (sal_uInt16)0x3f4                 // DocFileVersion
                   << (sal_uInt8)3                      // MajorVersion
                   << (sal_uInt8)0                      // MinorVersion
                   << (sal_uInt16)0;                    // Pad Word
    pUserName[ nLenOfUserName ] = 8;
    mpCurUserStrm->Write( pUserName, nLenOfUserName + 1 );
    for ( sal_uInt32 i = 0x15 + nLenOfUserName; i < nSizeOfRecord; i++ )
    {
        *mpCurUserStrm << (sal_uInt8)0;                 // pad bytes
    };
    mpCurUserStrm->Seek( nEditPos );
    return TRUE;
};

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateSummaryInformation()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentInfoSupplier >
        aXDocumentInfoSupplier( mXModel, ::com::sun::star::uno::UNO_QUERY );
    if ( aXDocumentInfoSupplier.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentInfo >
            aXDocumentInfo( aXDocumentInfoSupplier->getDocumentInfo() );
        if ( aXDocumentInfo.is() )
        {
            DInfo aDInfo( *mrStg, String( RTL_CONSTASCII_USTRINGPARAM( "\005SummaryInformation" ) ) );

            if ( aDInfo.IsValid() )
            {
                sal_uInt8 aPropSetGUID[ 16 ] =
                {
                    0xe0, 0x85, 0x9f, 0xf2, 0xf9, 0x4f, 0x68, 0x10, 0xab, 0x91, 0x08, 0x00, 0x2b, 0x27, 0xb3, 0xd9
                };
                Section aPropSet( aPropSetGUID );

                PropItem aPropItem;

                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    aXDocInfoPropSet( aXDocumentInfo, ::com::sun::star::uno::UNO_QUERY );                       ;
                if ( aXDocInfoPropSet.is() )
                {
                    if ( ImplGetPropertyValue( aXDocInfoPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Author" ) ) ) )
                    {
                        if ( (*(::rtl::OUString*)mAny.getValue() ).len() )
                        {
                            aPropItem.Clear();
                            aPropItem.Write( String( *(::rtl::OUString*)mAny.getValue() ) );
                            aPropSet.AddProperty( PID_AUTHOR, aPropItem );
                        }
                    }
                    if ( ImplGetPropertyValue( aXDocInfoPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "ModifiedBy" ) ) ) )
                    {
                        if ( (*(::rtl::OUString*)mAny.getValue() ).len() )
                        {
                            aPropItem.Clear();
                            aPropItem.Write( String( *(::rtl::OUString*)mAny.getValue() ) );
                            aPropSet.AddProperty( PID_LASTAUTHOR, aPropItem );
                        }
                    }
                    if ( ImplGetPropertyValue( aXDocInfoPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Theme" ) ) ) )
                    {
                        if ( (*(::rtl::OUString*)mAny.getValue() ).len() )
                        {
                            aPropItem.Clear();
                            aPropItem.Write( String( *(::rtl::OUString*)mAny.getValue() ) );
                            aPropSet.AddProperty( PID_SUBJECT, aPropItem );
                        }
                    }
                    if ( ImplGetPropertyValue( aXDocInfoPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ) ) )
                    {
                        if ( (*(::rtl::OUString*)mAny.getValue() ).len() )
                        {
                            aPropItem.Clear();
                            aPropItem.Write( String( *(::rtl::OUString*)mAny.getValue() ) );
                            aPropSet.AddProperty( PID_TITLE, aPropItem );
                        }
                    }
                    if ( ImplGetPropertyValue( aXDocInfoPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Keywords" ) ) ) )
                    {
                        if ( (*(::rtl::OUString*)mAny.getValue() ).len() )
                        {
                            aPropItem.Clear();
                            aPropItem.Write( String( *(::rtl::OUString*)mAny.getValue() ) );
                            aPropSet.AddProperty( PID_KEYWORDS, aPropItem );
                        }
                    }
                    if ( ImplGetPropertyValue( aXDocInfoPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Description" ) ) ) )
                    {
                        if ( (*(::rtl::OUString*)mAny.getValue() ).len() )
                        {
                            aPropItem.Clear();
                            aPropItem.Write( String( *(::rtl::OUString*)mAny.getValue() ) );
                            aPropSet.AddProperty( PID_COMMENTS, aPropItem );
                        }
                    }
                    if ( ImplGetPropertyValue( aXDocInfoPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "CreationDate" ) ) ) )
                    {
                        DateTime aDateTime( *(DateTime*)mAny.getValue() );
                        if ( aDateTime.IsValid() )
                        {
                            aPropItem.Clear();
                            aPropItem.Write( aDateTime );
                            aPropSet.AddProperty( PID_CREATE_DTM, aPropItem );
                        }
                    }
                    if ( ImplGetPropertyValue( aXDocInfoPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "PrintDate" ) ) ) )
                    {
                        DateTime aDateTime( *(DateTime*)mAny.getValue() );
                        if ( aDateTime.IsValid() )
                        {
                            aPropItem.Clear();
                            aPropItem.Write( aDateTime );
                            aPropSet.AddProperty( PID_LASTPRINTED_DTM, aPropItem );
                        }
                    }
                }

                if ( ImplGetPageByIndex( 0, NORMAL ) && ImplGetPropertyValue( mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Preview" ) ) ) )
                {
                    sal_uInt16 nWidth = 4233;
                    sal_uInt16 nHeight = ( (sal_uInt16)( (double)4233.0 /
                                                (double)maDestPageSize.Width *
                                                    (double)maDestPageSize.Height ) );
                    aPropItem.Clear();
                    aPropItem << (UINT32)VT_CF
                              << (UINT32)0;

                    aPropItem << (sal_uInt32)0xfffffff
                              << (sal_uInt32)3
                              << (sal_uInt16)8
                              << nWidth
                              << nHeight
                              << (sal_uInt16)0;

                    ::com::sun::star::uno::Sequence<sal_uInt8> aSeq;
                    aSeq = *(::com::sun::star::uno::Sequence<sal_uInt8>*)mAny.getValue();

                    const sal_uInt8* pAry = aSeq.getArray();
                    sal_uInt32 nAryLen = aSeq.getLength();

                    if ( pAry && nAryLen )
                    {
                        if ( nAryLen < 0x20000 )    // we will not generate preview pics greater than 128kb
                        {
                            aPropItem.Write( pAry, nAryLen );
                            sal_uInt32 nSize = aPropItem.Tell() - 8;
                            aPropItem.Seek( 4 );
                            aPropItem << nSize;
                            aPropSet.AddProperty( PID_PREVIEW, aPropItem );
                            aDInfo.AddSection( aPropSet );
                        }
                    }
                }
                aDInfo.Write();
            }
        }
    }
    return TRUE;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateDocumentSummaryInformation()
{
    DInfo aDInfo( *mrStg, String( RTL_CONSTASCII_USTRINGPARAM( "\005DocumentSummaryInformation" ) ) );

    if ( aDInfo.IsValid() )
    {
        // first section

        sal_uInt8 aPropSetGUID[ 16 ] =
        {
            0x02, 0xd5, 0xcd, 0xd5, 0x9c, 0x2e, 0x1b, 0x10, 0x93, 0x97, 0x08, 0x00, 0x2b, 0x2c, 0xf9, 0xae
        };
        Section aPropSet( aPropSetGUID );
        aDInfo.AddSection( aPropSet );

        // second section

        sal_uInt32 nNextId = 2;
        sal_uInt8 aUserPropSetGUID[ 16 ] =
        {
            0x05, 0xd5, 0xcd, 0xd5, 0x9c, 0x2e, 0x1b, 0x10, 0x93, 0x97, 0x08, 0x00, 0x2b, 0x2c, 0xf9, 0xae
        };
        Section aUserPropSet( aUserPropSetGUID );

        Dictionary aDict;

        // code page indicator

        sal_uInt8 aCodePageIndicator[ 8 ] =
        {
            0x02, 0x00, 0x00, 0x00, 0xe4, 0x04, 0x00, 0x00
        };
        aUserPropSet.AddProperty( 1, aCodePageIndicator, 8 );   // 1 is always the code page indicator

        // guid

        sal_uInt8 aGuid[ 0x56 ] =
        {
            0x41, 0x00, 0x00, 0x00,
            0x4e, 0x00, 0x00, 0x00,
            '{',0,'D',0,'B',0,'1',0,'A',0,'C',0,'9',0,'6',0,'4',0,'-',0,
            'E',0,'3',0,'9',0,'C',0,'-',0,'1',0,'1',0,'D',0,'2',0,'-',0,
            'A',0,'1',0,'E',0,'F',0,'-',0,'0',0,'0',0,'6',0,'0',0,'9',0,
            '7',0,'D',0,'A',0,'5',0,'6',0,'8',0,'9',0,'}',0
        };
        aDict.AddProperty( nNextId, String( RTL_CONSTASCII_USTRINGPARAM( "_PID_GUID" ) ) );
        aUserPropSet.AddProperty( nNextId++, aGuid, 0x56 );

        // hyperlinks

        if ( maHyperlink.Count() )
        {
            SvMemoryStream  aHyperBlob;
            ImplCreateHyperBlob( aHyperBlob );
            aDict.AddProperty( nNextId, String( RTL_CONSTASCII_USTRINGPARAM( "_PID_HLINKS" ) ) );
            aUserPropSet.AddProperty( nNextId++, aHyperBlob );
        }

        // special propertys

        ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentInfoSupplier >
            aXDocumentInfoSupplier( mXModel, ::com::sun::star::uno::UNO_QUERY );

        if ( aXDocumentInfoSupplier.is() )
        {

            ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentInfo >
                    aXDocumentInfo( aXDocumentInfoSupplier->getDocumentInfo() );
            if ( aXDocumentInfo.is() )
            {
                INT32 nUserFieldCount = aXDocumentInfo->getUserFieldCount();
                PropItem aPropItem;
                for ( int i = 0; i < nUserFieldCount; i++ )
                {
                    aPropItem.Clear();
                    ::rtl::OUString aUValue( aXDocumentInfo->getUserFieldValue( i ) );
                    if ( aUValue.len() )
                    {
                        aPropItem.Write( String( aUValue ) );
                        aDict.AddProperty( nNextId, aXDocumentInfo->getUserFieldName( i ) );
                        aUserPropSet.AddProperty( nNextId++, aPropItem );
                    }
                }
            }
        }

        aUserPropSet.AddDictionary( aDict );
        aDInfo.AddSection( aUserPropSet );
        aDInfo.Write();
    }
    return TRUE;
}

// ---------------------------------------------------------------------------------------------

void PPTWriter::ImplWriteExtParaHeader( SvMemoryStream& rSt, sal_uInt32 nRef, sal_uInt32 nInstance, sal_uInt32 nSlideId )
{
    if ( rSt.Tell() )
    {
        aBuExOutlineStream << (sal_uInt32)( ( EPP_PST_ExtendedParagraphHeaderAtom << 16 )
                                | ( nRef << 4 ) )
                            << (sal_uInt32)8
                            << (sal_uInt32)nSlideId
                            << (sal_uInt32)nInstance;
        aBuExOutlineStream.Write( rSt.GetData(), rSt.Tell() );
    }
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateDocument()
{
    sal_uInt32 i;
    sal_uInt16 nSlideType = EPP_SLIDESIZE_TYPECUSTOM;

    sal_uInt32 nWidth = maDestPageSize.Width;
    sal_uInt32 nHeight = maDestPageSize.Height;

    if ( ( nWidth == 0x1680 ) && ( nHeight == 0x10e0 ) )
        nSlideType = EPP_SLIDESIZE_TYPEONSCREEN;
    else if ( ( nWidth == 0x1200 ) && ( nHeight == 0x240 ) )
        nSlideType = EPP_SLIDESIZE_TYPEBANNER;
    else if ( ( nWidth == 0x1950 ) && ( nHeight == 0x10e0 ) )
        nSlideType = EPP_SLIDESIZE_TYPE35MM;
    else if ( ( nWidth == 0x1860 ) && ( nHeight == 0x10e0 ) )
        nSlideType = EPP_SLIDESIZE_TYPEA4PAPER;

    mpPptEscherEx->OpenContainer( EPP_Document );
    // CREATE DOCUMENT ATOM
    mpPptEscherEx->AddAtom( 40, EPP_DocumentAtom, 1 );
    *mpStrm << nWidth                           // Slide Size in Master coordinates X
            << nHeight                          //   "     "   "    "        "      Y
            << (INT32)maNotesPageSize.Width     // Notes Page Size                  X
            << (INT32)maNotesPageSize.Height    //   "     "   "                    Y
            << (INT32)1 << (INT32)2;            // the scale used when the Powerpoint document is embedded. the default is 1:2
    mpPptEscherEx->InsertPersistOffset( EPP_MAINNOTESMASTER_PERSIST_KEY, mpStrm->Tell() );
    *mpStrm << (sal_uInt32)0                        // Reference to NotesMaster ( 0 if none );
            << (sal_uInt32)0                        // Reference to HandoutMaster ( 0 if none );
            << (sal_Int16)1                         // Number of the first slide;
            << nSlideType                           // Size of the document slides ( default: EPP_SLIDESIZETYPEONSCREEN )
            << (sal_uInt8)0                         // bool1 indicates if document was saved with embedded true type fonts
            << (sal_uInt8)0                         // bool1 indicates if the placeholders on the title slide are omitted
            << (sal_uInt8)0                         // bool1 right to left ( flag for Bidi version )
            << (sal_uInt8)1;                            // bool1 visibility of comments shapes

    mpPptEscherEx->PtInsert( EPP_Persist_Document, mpStrm->Tell() );

    mpPptEscherEx->OpenContainer( EPP_SlideListWithText );      // Animation info fuer die Slides

    sal_uInt32  nShapes;
    sal_Bool    bOtherThanPlaceHolders;

    for ( i = 0; i < mnPages; i++ )
    {
        sal_uInt32  nPOffset, nPObjects;
        sal_Bool    bOutliner, bTitle;

        bOtherThanPlaceHolders = bOutliner = bTitle = FALSE;
        nPObjects = 0;

        mpPptEscherEx->AddAtom( 20, EPP_SlidePersistAtom );
        mpPptEscherEx->InsertPersistOffset( EPP_MAINSLIDE_PERSIST_KEY | i, mpStrm->Tell() );
        *mpStrm << (sal_uInt32)0;                               // psrReference - logical reference to the slide persist object ( EPP_MAINSLIDE_PERSIST_KEY )
        nPOffset = mpStrm->Tell();
        *mpStrm << (sal_uInt32)0                                // flags - only bit 3 used, if set then slide contains shapes other than placeholders
                << (INT32)0                                     // numberTexts - number of placeholder texts stored with the persist object.  Allows to display outline view without loading the slide persist objects
                << (INT32)i + 0x100                             // slideId - Unique slide identifier, used for OLE link monikers for example
                << (sal_uInt32)0;                               // reserved, usualy 0

        if ( !ImplGetPageByIndex( i, NORMAL ) )                 // sehr aufregend: noch einmal ueber alle seiten
            return FALSE;

        const PHLayout& rLayout = ImplGetLayout( mXPagePropSet );

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed >
            aXName( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );

        if ( aXName.is() )
        {
            ::rtl::OUString aStr = aXName->getName();
            ::rtl::OUString *pUStr = new ::rtl::OUString( aStr );
            maSlideNameList.Insert( pUStr, LIST_APPEND );
        }
        else
            maSlideNameList.Insert( new ::rtl::OUString(), LIST_APPEND );

        nShapes = mXShapes->getCount();

        sal_Bool bSecOutl = FALSE;
        if ( nShapes && ( rLayout.bTitlePossible || rLayout.bOutlinerPossible ) )
        {
            for ( sal_uInt32 nIndex = 0; nIndex < nShapes; nIndex++ )
            {
                if ( !ImplGetShapeByIndex( nIndex ) )
                    continue;

                if ( mbPresObj && ( ( mType == "presentation.Outliner" ) || ( mType == "presentation.Subtitle" ) ) )
                {
                    if ( bOutliner == FALSE )
                    {
                        bOutliner = TRUE;
                        mnTextStyle = EPP_TEXTSTYLE_BODY;
                        sal_uInt32 nTextType = EPP_TEXTTYPE_Body;
                        if ( bSecOutl )
                            nTextType = EPP_TEXTTYPE_HalfBody;
                        else if ( mType == "presentation.Subtitle" )
                            nTextType = EPP_TEXTTYPE_CenterBody;

                        TextRuleEntry* pRule = new TextRuleEntry( i );
                        SvMemoryStream aExtBu( 0x200, 0x200 );
                        if ( !mbEmptyPresObj )
                            ImplGetText();
                        ImplWriteTextStyleAtom( *mpStrm, nTextType, nPObjects, pRule, aExtBu );
                        ImplWriteExtParaHeader( aExtBu, nPObjects++, nTextType, i + 0x100 );
                        maTextRuleList.Insert( (void*)pRule, LIST_APPEND );
                        if ( rLayout.bSecOutlinerPossible )
                        {
                            if ( ( nIndex + 1 ) < nShapes )
                            {
                                if ( ImplGetShapeByIndex( nIndex + 1 ) && mType == "presentation.Outliner" )
                                {
                                    bSecOutl = TRUE;
                                    TextRuleEntry* pRule = new TextRuleEntry( i );
                                    SvMemoryStream aExtBu( 0x200, 0x200 );
                                    if ( !mbEmptyPresObj )
                                        ImplGetText();
                                    ImplWriteTextStyleAtom( *mpStrm, nTextType, nPObjects, pRule, aExtBu );
                                    ImplWriteExtParaHeader( aExtBu, nPObjects++, nTextType, i + 0x100 );
                                    maTextRuleList.Insert( (void*)pRule, LIST_APPEND );
                                }
                            }
                        }
                    }
                }
                else if ( rLayout.bTitlePossible && ( mType == "presentation.TitleText" ) )
                {
                    if ( bTitle == FALSE )
                    {
                        bTitle = TRUE;
                        mnTextStyle = EPP_TEXTSTYLE_TITLE;
                        TextRuleEntry* pRule = new TextRuleEntry( i );
                        SvMemoryStream aExtBu( 0x200, 0x200 );
                        if ( !mbEmptyPresObj )
                            ImplGetText();
                        ImplWriteTextStyleAtom( *mpStrm, EPP_TEXTTYPE_Title, nPObjects, pRule, aExtBu );
                        ImplWriteExtParaHeader( aExtBu, nPObjects++, EPP_TEXTTYPE_Title, i + 0x100 );
                        maTextRuleList.Insert( (void*)pRule, LIST_APPEND );
                    }
                }
                else
                {
                    if ( mbEmptyPresObj )
                        nPObjects++;
                    else
                        bOtherThanPlaceHolders = TRUE;  // muss noch auf background und leeren Title/outliner geprueft werden !!!
                }
                if ( bOutliner && bTitle && bOtherThanPlaceHolders )
                    break;
            }
        }
        if ( nPObjects )
        {
            sal_uInt32 nOldPos = mpStrm->Tell();
            mpStrm->Seek( nPOffset );
            *mpStrm << (sal_uInt32)( ( bOtherThanPlaceHolders ) ? 4 : 0 );
            *mpStrm << nPObjects;
            mpStrm->Seek( nOldPos );
        }
    }
    mpPptEscherEx->CloseContainer();    // EPP_SlideListWithText

    mpPptEscherEx->OpenContainer( EPP_SlideListWithText, 2 );   // Animation info fuer die notes
    for( i = 0; i < mnPages; i++ )
    {
        if ( !ImplGetPageByIndex( i, NOTICE ) )
            return FALSE;

        nShapes = mXShapes->getCount();

        bOtherThanPlaceHolders = FALSE;
        if ( nShapes )
        {
            for ( sal_uInt32 nIndex = 0; ( nIndex < nShapes ) && ( bOtherThanPlaceHolders == FALSE ); nIndex++ )
            {
                 if ( ImplGetShapeByIndex( nIndex ) && ( mType != "drawing.Page" ) )
                    bOtherThanPlaceHolders = TRUE;
            }
        }
        mpPptEscherEx->AddAtom( 20, EPP_SlidePersistAtom );
        mpPptEscherEx->InsertPersistOffset( EPP_MAINNOTES_PERSIST_KEY | i, mpStrm->Tell() );
        *mpStrm << (sal_uInt32)0
                << (sal_uInt32)( ( bOtherThanPlaceHolders ) ? 4 : 0 )
                << (INT32)0
                << (INT32)i + 0x100
                << (sal_uInt32)0;
    }
    mpPptEscherEx->CloseContainer();        // EPP_SlideListWithText

    ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XPresentationSupplier >
        aXPresSupplier( mXModel, ::com::sun::star::uno::UNO_QUERY );            ;
    if ( aXPresSupplier.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XPresentation >
            aXPresentation( aXPresSupplier->getPresentation() );
        if ( aXPresentation.is() )
        {
            mXPropSet = ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet >
                    ( aXPresentation, ::com::sun::star::uno::UNO_QUERY );
            if ( mXPropSet.is() )
            {
                ::rtl::OUString aCustomShow;
                sal_uInt32  nPenColor = 0x1000000;
                INT32   nRestartTime = 0x7fffffff;
                sal_Int16   nStartSlide = 0;
                sal_Int16   nEndSlide = 0;
                sal_uInt32  nFlags = 0;             // Bit 0:   Auto advance
                                                    // Bit 1    Skip builds ( do not allow slide effects )
                                                    // Bit 2    Use slide range
                                                    // Bit 3    Use named show
                                                    // Bit 4    Browse mode on
                                                    // Bit 5    Kiosk mode on
                                                    // Bit 7    loop continously
                                                    // Bit ?    show scrollbar

                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CustomShow" ) ) ) )
                {
                    aCustomShow = ( *(::rtl::OUString*)mAny.getValue() );
                    if ( aCustomShow.len() )
                    {
                        nFlags |= 8;
                    }
                }
                if ( ( nFlags & 8 ) == 0 )
                {
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "FirstPage" ) ) ) )
                    {
                        ::rtl::OUString aSlideName( *(::rtl::OUString*)mAny.getValue() );
                        for ( ::rtl::OUString* pStr = (::rtl::OUString*)maSlideNameList.First(); pStr;
                                    pStr = (::rtl::OUString*)maSlideNameList.Next(), nStartSlide++ )
                        {
                            if ( *pStr == aSlideName )
                            {
                                nStartSlide++;
                                nFlags |= 4;
                                nEndSlide = (sal_uInt16)mnPages;
                                break;
                            }
                        }
                        if ( !pStr )
                            nStartSlide = 0;
                    }
                }

//              if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "DiaName" ) ) ) )
//              {
//              }
//              if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsAlwaysOnTop" ) ) ) )
//              {
//              }
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsAutomatic" ) ) ) )
                {
                    sal_Bool bBool;
                    mAny >>= bBool;
                    if ( !bBool )
                        nFlags |= 1;
                }

                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsEndless" ) ) ) ) // muesste eigendlich heissen IsNotEndless !=)"§()&
                {
                    sal_Bool bBool;
                    mAny >>= bBool;
                    if ( bBool )
                        nFlags |= 0x80;
                }
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsFullScreen" ) ) ) )
                {
                    sal_Bool bBool;
                    mAny >>= bBool;
                    if ( !bBool )
                        nFlags |= 0x11;
                }
//              if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsLivePresentation" ) ) ) )
//              {
//              }
//              if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsMouseVisible" ) ) ) )
//              {
//              }
//              if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "PageRange" ) ) ) )
//              {
//              }
//              if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "StartWithNavigator" ) ) ) )
//              {
//              }
//              if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "UsePen" ) ) ) )
//              {
//              }
                mpPptEscherEx->AddAtom( 80, EPP_SSDocInfoAtom, 1 );
                *mpStrm << nPenColor << nRestartTime << nStartSlide << nEndSlide;

                sal_uInt32 nCustomShowNameLen = aCustomShow.len();
                if ( nCustomShowNameLen > 31 )
                    nCustomShowNameLen = 31;
                if ( nCustomShowNameLen )       // named show identifier
                {
                    const sal_Unicode* pCustomShow = aCustomShow.getStr();
                    for ( i = 0; i < nCustomShowNameLen; i++ )
                    {
                        *mpStrm << (sal_uInt16)( pCustomShow[ i ] );
                    }
                }
                for ( i = nCustomShowNameLen; i < 32; i++, *mpStrm << (sal_uInt16)0 );

                *mpStrm << nFlags;
                ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XCustomPresentationSupplier >
                    aXCPSup( mXModel, ::com::sun::star::uno::UNO_QUERY );
                if ( aXCPSup.is() )
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                        aXCont( aXCPSup->getCustomPresentations() );
                    if ( aXCont.is() )
                    {
                        ::com::sun::star::uno::Sequence< ::rtl::OUString> aNameSeq( aXCont->getElementNames() );
                        const ::rtl::OUString* pUString = aNameSeq.getArray();
                        sal_Int16 nCount = (sal_Int16)aNameSeq.getLength();
                        if ( nCount )
                        {
                            mpPptEscherEx->OpenContainer( EPP_NamedShows );
                            sal_uInt32 nCustomShowIndex = 0;
                            for ( sal_Int16 i = 0; i < nCount; i++ )        // Anzahl der Custom Shows
                            {
                                if ( pUString[ i ].len() )
                                {
                                    mpPptEscherEx->OpenContainer( EPP_NamedShow, nCustomShowIndex++ );

                                    sal_uInt32 nNamedShowLen = pUString[ i ].len();
                                    if ( nNamedShowLen > 31 )
                                        nNamedShowLen = 31;
                                    mpPptEscherEx->AddAtom( nNamedShowLen << 1, EPP_CString );
                                    const sal_Unicode* pCustomShowName = pUString[ i ].getStr();
                                    for ( sal_uInt32 k = 0; k < nNamedShowLen; *mpStrm << (sal_uInt16)( pCustomShowName[ k++ ] ) );
                                    mAny = aXCont->getByName( pUString[ i ] );
                                    if ( mAny.getValue() )
                                    {

                                        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > aXIC;
                                        if ( mAny >>= aXIC )
                                        {
                                            mpPptEscherEx->BeginAtom();

                                            INT32 nSlideCount = aXIC->getCount();
                                            for ( INT32 j = 0; j < nSlideCount; j++ )   // Anzahl der Slides
                                            {
                                                mAny = aXIC->getByIndex( j );
                                                if ( mAny.getValue() )
                                                {
                                                    ::com::sun::star::uno::Reference<
                                                        ::com::sun::star::drawing::XDrawPage > aXDrawPage;
                                                    if ( mAny >>= aXDrawPage )
                                                    {
                                                        ::com::sun::star::uno::Reference<
                                                            ::com::sun::star::container::XNamed >
                                                            aXName( aXDrawPage, ::com::sun::star::uno::UNO_QUERY );
                                                        if ( aXName.is() )
                                                        {
                                                            ::rtl::OUString aSlideName( aXName->getName() );
                                                            sal_uInt32 nPageNumber = 0;
                                                            for ( ::rtl::OUString* pSlideName = (::rtl::OUString*)maSlideNameList.First();
                                                                pSlideName;
                                                                pSlideName = (::rtl::OUString*)maSlideNameList.Next(), nPageNumber++ )
                                                            {
                                                                if ( *pSlideName == aSlideName )
                                                                {
                                                                    *mpStrm << (sal_uInt32)( nPageNumber + 0x100 ); // unique slide id
                                                                    break;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            mpPptEscherEx->EndAtom( EPP_NamedShowSlides );
                                        }
                                    }
                                    mpPptEscherEx->CloseContainer();            // EPP_NamedShow
                                }
                            }
                            mpPptEscherEx->CloseContainer();                // EPP_NamedShows
                        }
                    }
                }
            }
        }
    }
    mpPptEscherEx->AddAtom( 0, EPP_EndDocument );
    mpPptEscherEx->CloseContainer();    // EPP_Document
    return TRUE;
};

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateHyperBlob( SvMemoryStream& rStrm )
{
    sal_uInt32 nCurrentOfs, nParaOfs, nParaCount = 0;
    rStrm << (sal_uInt32)0x41;      // property type VT_BLOB
    nParaOfs = rStrm.Tell();
    rStrm << (sal_uInt32)0;         // property size
    rStrm << (sal_uInt32)0;         // property count

    for ( EPPTHyperlink* pLink = (EPPTHyperlink*)maHyperlink.First(); pLink; pLink = (EPPTHyperlink*)maHyperlink.Next() )
    {
        nParaCount += 6;
        rStrm   << (sal_uInt32)3    // Type VT_I4
                << (sal_uInt32)7    // (VTI4 - Private1)
                << (sal_uInt32)3    // Type VT_I4
                << (sal_uInt32)6    // (VTI4 - Private2)
                << (sal_uInt32)3    // Type VT_I4
                << (sal_uInt32)0;   // (VTI4 - Private3)

        // INFO
        // HIWORD:  = 0 : do not change anything
        //          = 1 : replace the hyperlink with the target and subadress in the following two VTLPWSTR
        //          = 2 : delete the hyperlink
        // LOWORD:  = 0 : graphic shown as background (link)
        //          = 1 : graphic shown as shape (link)
        //          = 2 : graphic is used to fill a shape
        //          = 3 : graphic used to fill a shape outline (future use)
        //          = 4 : hyperlink attached to a shape
        //          = 5 :    "         "      " " (Word) field
        //          = 6 :    "         "      " " (Excel) range
        //          = 7 :    "         "      " " (PPT) text range
        //          = 8 :    "         "      " " (Project) task

        sal_uInt32 nUrlLen = pLink->aURL.Len();
        const sal_Unicode* pUrl = pLink->aURL.GetBuffer();

        sal_uInt32 nInfo = 7;

        rStrm   << (sal_uInt32)3    // Type VT_I4
                << nInfo;       // Info

        switch( pLink->nType & 0xff )
        {
            case 1 :        // click action to slidenumber
            {
                rStrm << (sal_uInt32)0x1f << (sal_uInt32)1 << (sal_uInt32)0;    // path
                rStrm << (sal_uInt32)0x1f << (sal_uInt32)( nUrlLen + 1 );
                for ( sal_uInt32 i = 0; i < nUrlLen; i++ )
                {
                    rStrm << pUrl[ i ];
                }
                rStrm << (sal_uInt16)0;
            }
            break;
            case 2 :
            {
                rStrm   << (sal_uInt32)0x1f
                        << (sal_uInt32)( nUrlLen + 1 );
                for ( sal_uInt32 i = 0; i < nUrlLen; i++ )
                {
                    rStrm << pUrl[ i ];
                }
                if ( ! ( i & 1 ) )
                    rStrm << (sal_uInt16)0;
                rStrm   << (sal_uInt16)0
                        << (sal_uInt32)0x1f
                        << (sal_uInt32)1
                        << (sal_uInt32)0;
            }
            break;
        }
    }
    nCurrentOfs = rStrm.Tell();
    rStrm.Seek( nParaOfs );
    rStrm << (sal_uInt32)( nCurrentOfs - ( nParaOfs + 4 ) );
    rStrm << nParaCount;
    rStrm.Seek( nCurrentOfs );
    return TRUE;
}

PHLayout& PPTWriter::ImplGetLayout(  const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rXPropSet ) const
{
    ::com::sun::star::uno::Any aAny;
    sal_Int16 nLayout = 20;
    if ( GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Layout" ) ) ), sal_True )
        aAny >>= nLayout;

    if ( ( nLayout >= 21 ) && ( nLayout <= 26 ) )   // NOTES _> HANDOUT6
        nLayout = 20;
    if ( ( nLayout >= 27 ) && ( nLayout <= 30 ) )   // VERTICAL LAYOUT
        nLayout -= 6;
    else if ( nLayout > 30 )
        nLayout = 20;
    return pPHLayout[ nLayout ];
}


// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateMainMaster()
{
    if ( !ImplGetPageByIndex( 0, MASTER ) )
        return FALSE;

    if ( !ImplGetPropertyValue( mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Background" ) ) ) )                // Backgroundshape laden
        return FALSE;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > aXBackgroundPropSet;
    if ( !( mAny >>= aXBackgroundPropSet ) )
        return FALSE;

    mnFillColor = 0xffffff;
    mnFillBackColor = 0x000000;

    ::com::sun::star::drawing::FillStyle aFS = ::com::sun::star::drawing::FillStyle_NONE;
    if ( ImplGetPropertyValue( aXBackgroundPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillStyle" ) ) ) )
        mAny >>= aFS;
    switch ( aFS )
    {
        case ::com::sun::star::drawing::FillStyle_GRADIENT :
        {
            if ( ImplGetPropertyValue( aXBackgroundPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillGradient" ) ) ) )
            {
                mnFillColor = EscherPropertyContainer::GetGradientColor( (::com::sun::star::awt::Gradient*)mAny.getValue(), 0 );
                mnFillBackColor = EscherPropertyContainer::GetGradientColor( (::com::sun::star::awt::Gradient*)mAny.getValue(), 1 );
            }
        }
        break;

        case ::com::sun::star::drawing::FillStyle_SOLID :
        {
            if ( ImplGetPropertyValue( aXBackgroundPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillColor" ) ) ) )
            {
                mnFillColor = mpPptEscherEx->GetColor( *((sal_uInt32*)mAny.getValue()) );
                mnFillBackColor = mnFillColor ^ 0xffffff;
            }
        }
    }

    mpPptEscherEx->PtReplaceOrInsert( EPP_Persist_MainMaster, mpStrm->Tell() );
    mpPptEscherEx->OpenContainer( EPP_MainMaster );
    mpPptEscherEx->AddAtom( 24, EPP_SlideAtom, 2 );
    *mpStrm << (INT32)EPP_LAYOUT_TITLEANDBODYSLIDE  // slide layout -> title and body slide
            << (sal_uInt8)1 << (sal_uInt8)2 << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0     // placeholderID
            << (sal_uInt32)0        // master ID ( ist gleich null bei einer masterpage )
            << (sal_uInt32)0        // notes ID ( ist gleich null wenn keine notizen vorhanden )
            << (sal_uInt16)0        // Bit 1: Follow master objects, Bit 2: Follow master scheme, Bit 3: Follow master background
            << (sal_uInt16)0;       // padword

    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0x99cc00 << (sal_uInt32)0xcc3333 << (sal_uInt32)0xffcccc << (sal_uInt32)0xb2b2b2;
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    *mpStrm << (sal_uInt32)0xff0000 << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x00ffff << (sal_uInt32)0x0099ff << (sal_uInt32)0xffff00 << (sal_uInt32)0x0000ff << (sal_uInt32)0x969696;
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    *mpStrm << (sal_uInt32)0xccffff << (sal_uInt32)0x000000 << (sal_uInt32)0x336666 << (sal_uInt32)0x008080 << (sal_uInt32)0x339933 << (sal_uInt32)0x000080 << (sal_uInt32)0xcc3300 << (sal_uInt32)0x66ccff;
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x333333 << (sal_uInt32)0x000000 << (sal_uInt32)0xdddddd << (sal_uInt32)0x808080 << (sal_uInt32)0x4d4d4d << (sal_uInt32)0xeaeaea;
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0x66ccff << (sal_uInt32)0xff0000 << (sal_uInt32)0xcc00cc << (sal_uInt32)0xc0c0c0;
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0xc0c0c0 << (sal_uInt32)0xff6600 << (sal_uInt32)0x0000ff << (sal_uInt32)0x009900;
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 6 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0xff9933 << (sal_uInt32)0xccff99 << (sal_uInt32)0xcc00cc << (sal_uInt32)0xb2b2b2;

    mpStyleSheet->Write( *mpStrm ,mpPptEscherEx );

    EscherSolverContainer aSolverContainer;

    mpPptEscherEx->OpenContainer( EPP_PPDrawing );
    mpPptEscherEx->OpenContainer( ESCHER_DgContainer );
    mpPptEscherEx->EnterGroup();

    ImplWritePage( pPHLayout[ 0 ], aSolverContainer, MASTER, TRUE );    // Die Shapes der Seite werden im PPT Dok. erzeugt

//*******************************
//** DEFAULT MASTER TITLE AREA **
//*******************************

    if ( mnMasterTitleIndex + 1 )
    {
        if ( !ImplGetShapeByIndex( mnMasterTitleIndex ) )
            return FALSE;

        if ( ImplGetText() )
        {
            ::rtl::OUString aUString( mXText->getString() );
            sal_uInt16 nChar;

            mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
            mnShapeMasterTitle = mpPptEscherEx->GetShapeID();
            mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle, 0xa00, mnShapeMasterTitle );// Flags: HaveAnchor | HasSpt
            EscherPropertyContainer aPropOpt;
            aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x50001 );
            aPropOpt.AddOpt( ESCHER_Prop_lTxid, mnTxId += 0x60 );
            aPropOpt.AddOpt( ESCHER_Prop_AnchorText, ESCHER_AnchorMiddle );
            aPropOpt.AddOpt( ESCHER_Prop_fillColor, mnFillColor );
            aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, mnFillBackColor );
            aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x110001 );
            aPropOpt.AddOpt( ESCHER_Prop_lineColor, 0x8000001 );
            aPropOpt.AddOpt( ESCHER_Prop_shadowColor, 0x8000002 );
            aPropOpt.CreateFillProperties( mXPropSet, sal_True );
            sal_uInt32 nLineFlags = 0x90001;
            if ( aPropOpt.GetOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags ) )
                nLineFlags |= 0x10001;  // draw dashed line if no line
            aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags );
            ImplWriteTextBundle( aPropOpt );
            aPropOpt.Commit( *mpStrm );
            mpPptEscherEx->AddAtom( 8, ESCHER_ClientAnchor );
            *mpStrm << (sal_Int16)maRect.Top() << (sal_Int16)maRect.Left() << (sal_Int16)maRect.Right() << (sal_Int16)maRect.Bottom();      // oben, links, rechts, unten ????
            mpPptEscherEx->OpenContainer( ESCHER_ClientData );
            mpPptEscherEx->AddAtom( 8, EPP_OEPlaceholderAtom );
            *mpStrm << (sal_uInt32)0                                                        // PlacementID
                    << (sal_uInt8)EPP_PLACEHOLDER_MASTERTITLE                               // PlaceHolderID
                    << (sal_uInt8)0                                                         // Size of PlaceHolder ( 0 = FULL, 1 = HALF, 2 = QUARTER )
                    << (sal_uInt16)0;                                                       // padword
            mpPptEscherEx->CloseContainer();    // ESCHER_ClientData
            mpPptEscherEx->OpenContainer( ESCHER_ClientTextbox );
            mpPptEscherEx->AddAtom( 4, EPP_TextHeaderAtom );
            *mpStrm << (sal_uInt32)EPP_TEXTTYPE_Title;
            mpPptEscherEx->AddAtom( mnTextSize << 1, EPP_TextCharsAtom );
            const sal_Unicode* pString = aUString;
            for ( sal_uInt32 i = 0; i < mnTextSize; i++ )
            {
                nChar = pString[ i ];       // 0xa -> 0xb weicher Zeilenumbruch
                if ( nChar == 0xa )
                    nChar++;                // 0xd -> 0xd harter Zeilenumbruch
                *mpStrm << nChar;
            }
            mpPptEscherEx->AddAtom( 6, EPP_BaseTextPropAtom );
            *mpStrm << (sal_uInt32)( mnTextSize + 1 ) << (sal_uInt16)0;
            mpPptEscherEx->AddAtom( 10, EPP_TextSpecInfoAtom );
            *mpStrm << (sal_uInt32)( mnTextSize + 1 ) << (sal_uInt32)1 << (sal_uInt16)0;
            mpPptEscherEx->CloseContainer();    // ESCHER_ClientTextBox
            mpPptEscherEx->CloseContainer();    // ESCHER_SpContainer
        }
    }

//********************************
//** DEFAULT MASTER OBJECT AREA **
//********************************

    if ( mnMasterBodyIndex + 1 )
    {
        if ( !ImplGetShapeByIndex( mnMasterBodyIndex ) )
            return FALSE;

        if ( ImplGetText() )
        {
            mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
            mnShapeMasterBody = mpPptEscherEx->GetShapeID();
            mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle, 0xa00, mnShapeMasterBody );  // Flags: HaveAnchor | HasSpt
            EscherPropertyContainer aPropOpt;
            aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x50001 );
            aPropOpt.AddOpt( ESCHER_Prop_lTxid, mnTxId += 0x60 );
            aPropOpt.AddOpt( ESCHER_Prop_fillColor, mnFillColor );
            aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, mnFillBackColor );
            aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x110001 );
            aPropOpt.AddOpt( ESCHER_Prop_lineColor, 0x8000001 );
            aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90001 );
            aPropOpt.AddOpt( ESCHER_Prop_shadowColor, 0x8000002 );
            aPropOpt.CreateFillProperties( mXPropSet, sal_True );
            sal_uInt32 nLineFlags = 0x90001;
            if ( aPropOpt.GetOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags ) )
                nLineFlags |= 0x10001;  // draw dashed line if no line
            aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags );
            ImplWriteTextBundle( aPropOpt );
            aPropOpt.Commit( *mpStrm );
            mpPptEscherEx->AddAtom( 8, ESCHER_ClientAnchor );
            *mpStrm << (sal_Int16)maRect.Top() << (sal_Int16)maRect.Left() << (sal_Int16)maRect.Right() << (sal_Int16)maRect.Bottom();  // oben, links, rechts, unten ????
            mpPptEscherEx->OpenContainer( ESCHER_ClientData );
            mpPptEscherEx->AddAtom( 8, EPP_OEPlaceholderAtom );
            *mpStrm << (sal_uInt32)1                                                        // PlacementID
                    << (sal_uInt8)EPP_PLACEHOLDER_MASTERBODY                                    // PlaceHolderID
                    << (sal_uInt8)0                                                         // Size of PlaceHolder ( 0 = FULL, 1 = HALF, 2 = QUARTER )
                    << (sal_uInt16)0;                                                       // padword
            mpPptEscherEx->CloseContainer();    // ESCHER_ClientData
            mpPptEscherEx->OpenContainer( ESCHER_ClientTextbox );       // printf
            mpPptEscherEx->AddAtom( 4, EPP_TextHeaderAtom );
            *mpStrm << (sal_uInt32)EPP_TEXTTYPE_Body;

            TextObj aTextObj( mXText, EPP_TEXTTYPE_Body, maFontCollection, (PPTExBulletProvider&)*this );
            mnTextSize = aTextObj.Count();
            aTextObj.Write( mpStrm );
            mpPptEscherEx->BeginAtom();
            for ( ParagraphObj* pPara = aTextObj.First() ; pPara; pPara = aTextObj.Next() )
            {
                sal_uInt32 nCharCount = pPara->Count();
                sal_uInt16 nDepth = pPara->nDepth;
                if ( nDepth > 4)
                    nDepth = 4;

                *mpStrm << nCharCount
                        << nDepth;
            }
            mpPptEscherEx->EndAtom( EPP_BaseTextPropAtom );
            mpPptEscherEx->AddAtom( 10, EPP_TextSpecInfoAtom );
            *mpStrm << (sal_uInt32)( mnTextSize ) << (sal_uInt32)1 << (sal_uInt16)0;

            mpPptEscherEx->CloseContainer();    // ESCHER_ClientTextBox
            mpPptEscherEx->CloseContainer();    // ESCHER_SpContainer
        }
    }
    mpPptEscherEx->LeaveGroup();

    ImplWriteBackground( aXBackgroundPropSet );

    aSolverContainer.WriteSolver( *mpStrm );

    mpPptEscherEx->CloseContainer();    // ESCHER_DgContainer
    mpPptEscherEx->CloseContainer();    // EPP_Drawing
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 1 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0x99cc00 << (sal_uInt32)0xcc3333 << (sal_uInt32)0xffcccc << (sal_uInt32)0xb2b2b2;

    if ( aBuExMasterStream.Tell() )
    {
        ImplProgTagContainer( mpStrm, &aBuExMasterStream );
    }

    mpPptEscherEx->CloseContainer();    // EPP_MainMaster
    return TRUE;
};

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateMainNotes()
{
    if ( !ImplGetPageByIndex( 0, NOTICE ) )
        return FALSE;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XMasterPageTarget >
        aXMasterPageTarget( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );

    if ( !aXMasterPageTarget.is() )
        return FALSE;

    mXDrawPage = aXMasterPageTarget->getMasterPage();
    if ( !mXDrawPage.is() )
        return FALSE;

    mXPropSet = ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
            ( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );
    if ( !mXPropSet.is() )
        return FALSE;

    mXShapes = ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShapes >
            ( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );
    if ( !mXShapes.is() )
        return FALSE;

    EscherSolverContainer aSolverContainer;

    mpPptEscherEx->PtReplaceOrInsert( EPP_Persist_MainNotes, mpStrm->Tell() );
    mpPptEscherEx->OpenContainer( EPP_Notes );
    mpPptEscherEx->AddAtom( 8, EPP_NotesAtom, 1 );
    *mpStrm << (sal_uInt32)0x80000000                                               // Number that identifies this slide
            << (sal_uInt32)0;                                                       // follow nothing
    mpPptEscherEx->OpenContainer( EPP_PPDrawing );
    mpPptEscherEx->OpenContainer( ESCHER_DgContainer );
    mpPptEscherEx->EnterGroup();

    ImplWritePage( pPHLayout[ 20 ], aSolverContainer, NOTICE, TRUE );

    mpPptEscherEx->LeaveGroup();
    mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
    mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle, 0xc00 );
    EscherPropertyContainer aPropOpt;
    aPropOpt.AddOpt( ESCHER_Prop_fillColor, 0xffffff );                 // stock valued fill color
    aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0 );
    aPropOpt.AddOpt( ESCHER_Prop_fillRectRight, 0x68bdde );
    aPropOpt.AddOpt( ESCHER_Prop_fillRectBottom, 0x8b9f8e );
    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x120012 );
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0 );
    aPropOpt.AddOpt( ESCHER_Prop_bWMode, ESCHER_wDontShow );
    aPropOpt.AddOpt( ESCHER_Prop_fBackground, 0x10001 );                        // if true, this is the background shape
    aPropOpt.Commit( *mpStrm );
    mpPptEscherEx->CloseContainer();    // ESCHER_SpContainer

    aSolverContainer.WriteSolver( *mpStrm );

    mpPptEscherEx->CloseContainer();    // ESCHER_DgContainer
    mpPptEscherEx->CloseContainer();    // EPP_Drawing
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 1 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0x99cc00 << (sal_uInt32)0xcc3333 << (sal_uInt32)0xffcccc << (sal_uInt32)0xb2b2b2;
    mpPptEscherEx->CloseContainer();    // EPP_Notes
    return TRUE;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateTitleMasterPage( int nPageNum )
{
    if ( !ImplGetPageByIndex( nPageNum, MASTER ) )
        return FALSE;

    if ( !ImplGetPropertyValue( mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Background" ) ) ) )    // Backgroundshape laden
        return FALSE;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > aXBackgroundPropSet;
    if ( ! ( mAny >>= aXBackgroundPropSet ) )
        return FALSE;

    mpPptEscherEx->PtReplaceOrInsert( EPP_Persist_MainMaster | nPageNum, mpStrm->Tell() );
    mpPptEscherEx->OpenContainer( EPP_Slide );
    mpPptEscherEx->AddAtom( 24, EPP_SlideAtom, 2 );
    *mpStrm << (INT32)EPP_LAYOUT_TITLEMASTERSLIDE
            << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0     // placeholderID
            << (sal_uInt32)0x80000000       // master ID ( ist gleich null bei einer masterpage )
            << (sal_uInt32)0                // notes ID ( ist gleich null wenn keine notizen vorhanden )
            << (sal_uInt16)0                // Bit 1: Follow master objects, Bit 2: Follow master scheme, Bit 3: Follow master background
            << (sal_uInt16)0;               // padword

    EscherSolverContainer aSolverContainer;

    mpPptEscherEx->OpenContainer( EPP_PPDrawing );
    mpPptEscherEx->OpenContainer( ESCHER_DgContainer );
    mpPptEscherEx->EnterGroup();

    ImplWritePage( pPHLayout[ 0 ], aSolverContainer, MASTER, TRUE );    // Die Shapes der Seite werden im PPT Dok. erzeugt

    mpPptEscherEx->LeaveGroup();

    ImplWriteBackground( aXBackgroundPropSet );

    aSolverContainer.WriteSolver( *mpStrm );

    mpPptEscherEx->CloseContainer();    // ESCHER_DgContainer
    mpPptEscherEx->CloseContainer();    // EPP_Drawing
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 1 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0x99cc00 << (sal_uInt32)0xcc3333 << (sal_uInt32)0xffcccc << (sal_uInt32)0xb2b2b2;
    mpPptEscherEx->CloseContainer();    // EPP_MasterSlide
    return TRUE;
};

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateSlide( int nPageNum )
{
    ::com::sun::star::uno::Any aAny;

    if ( !ImplGetPageByIndex( nPageNum, NORMAL ) )
        return FALSE;

    sal_uInt32 nMasterID = 0x80000000;

    nMasterID |= ImplGetMasterIndex( NORMAL );

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > aXBackgroundPropSet;
    sal_Bool bHasBackground = GetPropertyValue( aAny, mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Background" ) ) );
    if ( bHasBackground )
        bHasBackground = ( aAny >>= aXBackgroundPropSet );
    sal_uInt16 nMode = 3;   // Bit 1: Follow master objects, Bit 2: Follow master scheme, Bit 3: Follow master background
    if ( !bHasBackground )
        nMode |= 4;

    const PHLayout& rLayout = ImplGetLayout( mXPagePropSet );
    mpPptEscherEx->PtReplaceOrInsert( EPP_Persist_Slide | nPageNum, mpStrm->Tell() );
    mpPptEscherEx->OpenContainer( EPP_Slide );
    mpPptEscherEx->AddAtom( 24, EPP_SlideAtom, 2 );
    *mpStrm << rLayout.nLayout;
    mpStrm->Write( rLayout.nPlaceHolder, 8 );       // placeholderIDs ( 8Stueck )
    *mpStrm << (sal_uInt32)nMasterID                // master ID ( ist gleich 0x80000000 bei einer masterpage   )
            << (sal_uInt32)nPageNum + 0x100         // notes ID ( ist gleich null wenn keine notizen vorhanden )
            << nMode
            << (sal_uInt16)0;                       // padword


    mnDiaMode = 0;
    sal_Bool bVisible = sal_True;
    ::com::sun::star::presentation::FadeEffect eFe = ::com::sun::star::presentation::FadeEffect_NONE;

    if ( GetPropertyValue( aAny, mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Visible" ) ) ) )
        aAny >>= bVisible;
    if ( GetPropertyValue( aAny, mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Change" ) ) ) )
    {
        switch ( *(INT32*)aAny.getValue() )
        {
            case 1 :        // automatisch
                mnDiaMode++;
            case 2 :        // halbautomatisch
                mnDiaMode++;
            default :
            case 0 :        // manuell
            break;
        }
    }
    if ( GetPropertyValue( aAny, mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Effect" ) ) ) )
        aAny >>= eFe;

    sal_Bool bNeedsSSSlideInfoAtom = ( bVisible == FALSE )
                                    || ( mnDiaMode == 2 )
                                    || ( eFe != ::com::sun::star::presentation::FadeEffect_NONE );
    if ( bNeedsSSSlideInfoAtom )
    {
        sal_uInt8   nDirection = 0;
        sal_uInt8   nTransitionType = 0;
        sal_uInt16  nBuildFlags = 1;        // advange by mouseclick
        sal_uInt8   nSoundRef = 0;
        INT32       nSlideTime = 0;         // muss noch !!!
        sal_uInt8   nSpeed = 1;

        if ( GetPropertyValue( aAny, mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Speed" ) ) ) )
        {
            ::com::sun::star::presentation::AnimationSpeed aAs;
            aAny >>= aAs;
            nSpeed = (sal_uInt8)aAs;
        }
        switch ( eFe )
        {
            default :
            case ::com::sun::star::presentation::FadeEffect_RANDOM :
                nTransitionType = PPT_TRANSITION_TYPE_RANDOM;
            break;

            case ::com::sun::star::presentation::FadeEffect_HORIZONTAL_STRIPES :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_VERTICAL_STRIPES :
                nTransitionType = PPT_TRANSITION_TYPE_BLINDS;
            break;

            case ::com::sun::star::presentation::FadeEffect_VERTICAL_CHECKERBOARD :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_HORIZONTAL_CHECKERBOARD :
                nTransitionType = PPT_TRANSITION_TYPE_CHECKER;
            break;

            case ::com::sun::star::presentation::FadeEffect_MOVE_FROM_UPPERLEFT :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_MOVE_FROM_UPPERRIGHT :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_MOVE_FROM_LOWERLEFT :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_MOVE_FROM_LOWERRIGHT :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_MOVE_FROM_TOP :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_MOVE_FROM_LEFT :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_MOVE_FROM_BOTTOM :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_MOVE_FROM_RIGHT :
                nTransitionType = PPT_TRANSITION_TYPE_COVER;
            break;

            case ::com::sun::star::presentation::FadeEffect_DISSOLVE :
                nTransitionType = PPT_TRANSITION_TYPE_DISSOLVE;
            break;

            case ::com::sun::star::presentation::FadeEffect_VERTICAL_LINES :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_HORIZONTAL_LINES :
                nTransitionType = PPT_TRANSITION_TYPE_RANDOM_BARS;
            break;

            case ::com::sun::star::presentation::FadeEffect_CLOSE_HORIZONTAL :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_OPEN_HORIZONTAL :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_CLOSE_VERTICAL :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_OPEN_VERTICAL :
                nTransitionType = PPT_TRANSITION_TYPE_SPLIT;
            break;

            case ::com::sun::star::presentation::FadeEffect_FADE_FROM_UPPERLEFT :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_FADE_FROM_UPPERRIGHT :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_FADE_FROM_LOWERLEFT :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_FADE_FROM_LOWERRIGHT :
                nDirection += 4;
                nTransitionType = PPT_TRANSITION_TYPE_STRIPS;
            break;

            case ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_LOWERRIGHT :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_LOWERLEFT :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_UPPERRIGHT :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_UPPERLEFT :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_TOP :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_LEFT :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_BOTTOM :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_UNCOVER_TO_RIGHT :
                nTransitionType = PPT_TRANSITION_TYPE_PULL;
            break;

            case ::com::sun::star::presentation::FadeEffect_FADE_FROM_TOP :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_FADE_FROM_LEFT :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_FADE_FROM_BOTTOM :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_FADE_FROM_RIGHT :
                nTransitionType = PPT_TRANSITION_TYPE_WIPE;
            break;

            case ::com::sun::star::presentation::FadeEffect_ROLL_FROM_TOP :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_ROLL_FROM_LEFT :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_ROLL_FROM_BOTTOM :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_ROLL_FROM_RIGHT :
                nTransitionType = PPT_TRANSITION_TYPE_WIPE;
            break;

            case ::com::sun::star::presentation::FadeEffect_FADE_TO_CENTER :
                nDirection++;
            case ::com::sun::star::presentation::FadeEffect_FADE_FROM_CENTER :
                nTransitionType = PPT_TRANSITION_TYPE_ZOOM;
            break;

            case ::com::sun::star::presentation::FadeEffect_NONE :
                nDirection = 2;
            break;
        }
        if ( mnDiaMode == 2 )                                   // automatic ?
            nBuildFlags |= 0x400;
        if ( bVisible == FALSE )
            nBuildFlags |= 4;

        if ( GetPropertyValue( aAny, mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Duration" ) ) ) )// duration of this slide
            nSlideTime = *(INT32*)aAny.getValue() << 10;        // in ticks


        mpPptEscherEx->AddAtom( 16, EPP_SSSlideInfoAtom );
        *mpStrm << (sal_uInt32)nSlideTime                       // standtime in ticks
                << (sal_uInt32)0
                << (sal_uInt8)nDirection
                << (sal_uInt8)nTransitionType
                << (sal_uInt16)nBuildFlags
                << (sal_uInt8)nSpeed
                << (sal_uInt8)nSoundRef << (sal_uInt8)0 << (sal_uInt8)0;
    }

    EscherSolverContainer aSolverContainer;
    mpPptEscherEx->OpenContainer( EPP_PPDrawing );
    mpPptEscherEx->OpenContainer( ESCHER_DgContainer );
    mpPptEscherEx->EnterGroup();
    ImplWritePage( rLayout, aSolverContainer, NORMAL, FALSE, nPageNum );    // Die Shapes der Seite werden im PPT Dok. erzeugt
    mpPptEscherEx->LeaveGroup();

    if ( bHasBackground )
        ImplWriteBackground( aXBackgroundPropSet );
    else
    {
        mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
        mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle, 0xc00 );             // Flags: Connector | Background | HasSpt
        EscherPropertyContainer aPropOpt;
        aPropOpt.AddOpt( ESCHER_Prop_fillColor, mnFillColor );              // stock valued fill color
        aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, mnFillBackColor );
        aPropOpt.AddOpt( ESCHER_Prop_fillRectRight, PPTtoEMU( maDestPageSize.Width ) );
        aPropOpt.AddOpt( ESCHER_Prop_fillRectBottom, PPTtoEMU( maDestPageSize.Width ) );
        aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x120012 );
        aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x80000 );
        aPropOpt.AddOpt( ESCHER_Prop_bWMode, ESCHER_wDontShow );
        aPropOpt.AddOpt( ESCHER_Prop_fBackground, 0x10001 );                // if true, this is the background shape
        aPropOpt.Commit( *mpStrm );
        mpPptEscherEx->CloseContainer();    // ESCHER_SpContainer
    }

    aSolverContainer.WriteSolver( *mpStrm );

    mpPptEscherEx->CloseContainer();    // ESCHER_DgContainer
    mpPptEscherEx->CloseContainer();    // EPP_Drawing
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 1 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0x99cc00 << (sal_uInt32)0xcc3333 << (sal_uInt32)0xffcccc << (sal_uInt32)0xb2b2b2;
    mpPptEscherEx->CloseContainer();    // EPP_Slide
    return TRUE;
};

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreateNotes( int nPageNum )
{
    if ( !ImplGetPageByIndex( nPageNum, NOTICE ) )
        return FALSE;

    mpPptEscherEx->PtReplaceOrInsert( EPP_Persist_Notes | nPageNum, mpStrm->Tell() );
    mpPptEscherEx->OpenContainer( EPP_Notes );
    mpPptEscherEx->AddAtom( 8, EPP_NotesAtom, 1 );
    *mpStrm << (sal_uInt32)nPageNum + 0x100
            << (sal_uInt16)3                                        // follow master ....
            << (sal_uInt16)0;

    EscherSolverContainer aSolverContainer;

    mpPptEscherEx->OpenContainer( EPP_PPDrawing );
    mpPptEscherEx->OpenContainer( ESCHER_DgContainer );
    mpPptEscherEx->EnterGroup();

    ImplWritePage( pPHLayout[ 20 ], aSolverContainer, NOTICE, FALSE );  // Die Shapes der Seite werden im PPT Dok. erzeugt

    mpPptEscherEx->LeaveGroup();
    mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
    mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle, 0xc00 ); // Flags: Connector | Background | HasSpt
    EscherPropertyContainer aPropOpt;
    aPropOpt.AddOpt( ESCHER_Prop_fillColor, 0xffffff );     // stock valued fill color
    aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0 );
    aPropOpt.AddOpt( ESCHER_Prop_fillRectRight, 0x8b9f8e );
    aPropOpt.AddOpt( ESCHER_Prop_fillRectBottom, 0x68bdde );
    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x120012 );
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x80000 );
    aPropOpt.AddOpt( ESCHER_Prop_bWMode, ESCHER_wDontShow );
    aPropOpt.AddOpt( ESCHER_Prop_fBackground, 0x10001 );
    aPropOpt.Commit( *mpStrm );
    mpPptEscherEx->CloseContainer();    // ESCHER_SpContainer

    aSolverContainer.WriteSolver( *mpStrm );

    mpPptEscherEx->CloseContainer();    // ESCHER_DgContainer
    mpPptEscherEx->CloseContainer();    // EPP_Drawing
    mpPptEscherEx->AddAtom( 32, EPP_ColorSchemeAtom, 0, 1 );
    *mpStrm << (sal_uInt32)0xffffff << (sal_uInt32)0x000000 << (sal_uInt32)0x808080 << (sal_uInt32)0x000000 << (sal_uInt32)0x99cc00 << (sal_uInt32)0xcc3333 << (sal_uInt32)0xffcccc << (sal_uInt32)0xb2b2b2;
    mpPptEscherEx->CloseContainer();    // EPP_Notes
    return TRUE;
};

void PPTWriter::ImplWriteBackground( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet )
{
    //************************ ******
    //** DEFAULT BACKGROUND SHAPE **
    //******************************

    sal_uInt32 nFillColor = 0xffffff;
    sal_uInt32 nFillBackColor = 0;

    mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
    mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle, 0xc00 );                     // Flags: Connector | Background | HasSpt
    Point aEmptyPoint = Point();
    Rectangle aRect( aEmptyPoint, Size( 28000, 21000 ) );
    EscherPropertyContainer aPropOpt( (EscherGraphicProvider&)*mpPptEscherEx, mpPicStrm, aRect );
    aPropOpt.AddOpt( ESCHER_Prop_fillType, ESCHER_FillSolid );
    ::com::sun::star::drawing::FillStyle aFS( ::com::sun::star::drawing::FillStyle_NONE );
    if ( ImplGetPropertyValue( rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillStyle" ) ) ) )
        mAny >>= aFS;

    switch( aFS )
    {
        case ::com::sun::star::drawing::FillStyle_GRADIENT :
        {
            aPropOpt.CreateGradientProperties( rXPropSet );
            aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x1f001e );
            aPropOpt.GetOpt( ESCHER_Prop_fillColor, nFillColor );
            aPropOpt.GetOpt( ESCHER_Prop_fillBackColor, nFillBackColor );
        }
        break;

        case ::com::sun::star::drawing::FillStyle_BITMAP :
            aPropOpt.CreateGraphicProperties( rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillBitmapURL" ) ), sal_True );
        break;

        case ::com::sun::star::drawing::FillStyle_HATCH :
        case ::com::sun::star::drawing::FillStyle_SOLID :
        {
            if ( ImplGetPropertyValue( rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillColor" ) ) ) )
            {
                nFillColor = mpPptEscherEx->GetColor( *((sal_uInt32*)mAny.getValue()) );
                nFillBackColor = nFillColor ^ 0xffffff;
            }
        }
        case ::com::sun::star::drawing::FillStyle_NONE :
        default:
            aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x120012 );
        break;
    }
    aPropOpt.AddOpt( ESCHER_Prop_fillColor, nFillColor );
    aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, nFillBackColor );
    aPropOpt.AddOpt( ESCHER_Prop_fillRectRight, PPTtoEMU( maDestPageSize.Width ) );
    aPropOpt.AddOpt( ESCHER_Prop_fillRectBottom, PPTtoEMU( maDestPageSize.Height ) );
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x80000 );
    aPropOpt.AddOpt( ESCHER_Prop_bWMode, ESCHER_bwWhite );
    aPropOpt.AddOpt( ESCHER_Prop_fBackground, 0x10001 );
    aPropOpt.Commit( *mpStrm );
    mpPptEscherEx->CloseContainer();    // ESCHER_SpContainer
}

void PPTWriter::ImplWriteCString( SvStream& rSt, const String& rString, sal_uInt32 nInstance )
{
    sal_uInt32 i, nLen = rString.Len();
    if ( nLen )
    {
        rSt << (sal_uInt32)( ( nInstance << 4 ) | ( EPP_CString << 16 ) )
            << (sal_uInt32)( nLen << 1 );
        for ( i = 0; i < nLen; i++ )
            rSt << rString.GetChar( (sal_uInt16)i );
    }
}

void PPTWriter::ImplWriteVBA( SvMemoryStream* pVBA )
{
    if ( pVBA )
    {
        pVBA->Seek( STREAM_SEEK_TO_END );
        sal_uInt32 nLen = pVBA->Tell();
        if ( nLen > 8 )
        {
            nLen -= 8;
            mnVBAOleOfs = mpStrm->Tell();
            mpPptEscherEx->BeginAtom();
            mpStrm->Write( (sal_Int8*)pVBA->GetData() + 8, nLen );
            mpPptEscherEx->EndAtom( EPP_ExOleObjStg, 0, 1 );
        }
    }
}

// ---------------------------------------------------------------------------------------------

void PPTWriter::ImplWriteOLE( sal_uInt32 nCnvrtFlags )
{
    PPTExOleObjEntry* pPtr;

    SvxMSExportOLEObjects aOleExport( nCnvrtFlags );

    for ( pPtr = (PPTExOleObjEntry*)maExOleObj.First(); pPtr;
        pPtr = (PPTExOleObjEntry*)maExOleObj.Next() )
    {
        SvMemoryStream* pStrm = NULL;
        pPtr->nOfsB = mpStrm->Tell();
        switch ( pPtr->eType )
        {
            case NORMAL_OLE_OBJECT :
            {
                SvStorageRef xSrcStor = mXSource->OpenStorage( pPtr->aObject, STREAM_READWRITE | STREAM_SHARE_DENYALL );
                if ( xSrcStor.Is() )
                {
                    SvInPlaceObjectRef  xInplaceObj( ((SvFactory*)SvInPlaceObject::
                                            ClassFactory())->CreateAndLoad( xSrcStor ) );
                    if( xInplaceObj.Is() )
                    {
                        SvStorageRef xTempStorage( new SvStorage( new SvMemoryStream(), TRUE ) );
                        aOleExport.ExportOLEObject( *xInplaceObj, *xTempStorage );
                        pStrm = xTempStorage->CreateMemoryStream();
                        xInplaceObj.Clear();
                    }
                }
            }
            break;

            case OCX_CONTROL :
            {
                if ( pPtr->xControlModel.is() )
                {
                    String aName;
                    ::com::sun::star::awt::Size aSize;
                    SvStorageRef xDest( new SvStorage( new SvMemoryStream(), TRUE ) );
                    sal_Bool bOk = SvxMSConvertOCXControls::WriteOCXStream( xDest, pPtr->xControlModel, aSize, aName );
                    if ( bOk )
                        pStrm = xDest->CreateMemoryStream();
                }
            }
        }
        if ( pStrm )
        {
            mpPptEscherEx->BeginAtom();
            pStrm->Seek( STREAM_SEEK_TO_END );
            *mpStrm << (sal_uInt32)pStrm->Tell();           // uncompressed size
            pStrm->Seek( 0 );
            ZCodec aZCodec( 0x8000, 0x8000 );
            aZCodec.BeginCompression();
            aZCodec.Compress( *pStrm, *mpStrm );
            aZCodec.EndCompression();
            delete pStrm;
            mpPptEscherEx->EndAtom( EPP_ExOleObjStg, 0, 1 );
        }
    }
}

// ---------------------------------------------------------------------------------------------
// PersistantTable und UserEditAtom schreiben

sal_Bool PPTWriter::ImplWriteAtomEnding()
{

#define EPP_LastViewTypeNone        0
#define EPP_LastViewTypeSlideView   1
#define EPP_LastViewTypeOutlineView 2
#define EPP_LastViewTypeNotes       3


    sal_uInt32  i, nPos, nOfs, nPersistOfs = mpStrm->Tell();
    sal_uInt32  nPersistEntrys = 0;
    *mpStrm << (sal_uInt32)0 << (sal_uInt32)0 << (sal_uInt32)0;         // Record Header und ersten Eintrag ueberspringen

    // Document pesist schreiben
        nPersistEntrys++;
        *mpStrm << (sal_uInt32)0;
    // MasterPages persists schreiben
    for ( i = 0; i < mnMasterPages; i++ )
    {
        nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_MainMaster | i );
        if ( nOfs )
        {
            *mpStrm << nOfs;
            mpPptEscherEx->InsertAtPersistOffset( EPP_MAINMASTER_PERSIST_KEY | i, ++nPersistEntrys );
        }
    }
    // MainNotesMaster persist schreiben
    nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_MainNotes );
    if ( nOfs )
    {
        *mpStrm << nOfs;
        mpPptEscherEx->InsertAtPersistOffset( EPP_MAINNOTESMASTER_PERSIST_KEY, ++nPersistEntrys );
    }
    // Slide persists schreiben -> es gilt hier auch den EPP_SlidePersistAtome mit einem gueltigen wert zu beschreiben
    for ( i = 0; i < mnPages; i++ )
    {
        nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_Slide | i );
        if ( nOfs )
        {
            *mpStrm << nOfs;
            mpPptEscherEx->InsertAtPersistOffset( EPP_MAINSLIDE_PERSIST_KEY | i, ++nPersistEntrys );
        }
    }
    // Notes persists schreiben
    for ( i = 0; i < mnPages; i++ )
    {
        nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_Notes | i );
        if ( nOfs )
        {
            *mpStrm << nOfs;
            mpPptEscherEx->InsertAtPersistOffset( EPP_MAINNOTES_PERSIST_KEY | i, ++nPersistEntrys );
        }
    }
    // Ole persists
    PPTExOleObjEntry* pPtr;
    for ( pPtr = (PPTExOleObjEntry*)maExOleObj.First(); pPtr; pPtr = (PPTExOleObjEntry*)maExOleObj.Next() )
    {
        nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_ExObj );
        if ( nOfs )
        {
            nPersistEntrys++;
            *mpStrm << pPtr->nOfsB;
            sal_uInt32 nOldPos, nPersOfs = nOfs + pPtr->nOfsA + 16 + 8;     // 8 bytes atom header, +16 to the persist entry
            nOldPos = mpStrm->Tell();
            mpStrm->Seek( nPersOfs );
            *mpStrm << nPersistEntrys;
            mpStrm->Seek( nOldPos );
        }
    }
    // VB persist
    if ( mnVBAOleOfs && mpVBA )
    {
        nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_VBAInfoAtom );
        if ( nOfs )
        {
            nPersistEntrys++;
            sal_uInt32 n1, n2;

            mpVBA->Seek( 0 );
            *mpVBA >> n1
                   >> n2;

            *mpStrm << mnVBAOleOfs;
            sal_uInt32 nOldPos = mpStrm->Tell();
            mpStrm->Seek( nOfs );               // Fill the VBAInfoAtom with the correct index to the persisttable
            *mpStrm << nPersistEntrys
                    << n1
                    << 2;
            mpStrm->Seek( nOldPos );

        }
    }
    nPos = mpStrm->Tell();
    mpStrm->Seek( nPersistOfs );
    mpPptEscherEx->AddAtom( ( nPersistEntrys + 1 ) << 2, EPP_PersistPtrIncrementalBlock );      // Record Header eintragen
    *mpStrm << (sal_uInt32)( ( nPersistEntrys << 20 ) | 1 );
    mpStrm->Seek( nPos );

    *mpCurUserStrm << (sal_uInt32)nPos;             // offset to current edit setzen
    mpPptEscherEx->AddAtom( 28, EPP_UserEditAtom );
    *mpStrm << (INT32)0x100                     // last slide ID
            << (sal_uInt32)0x03000dbc               // minor and major app version that did the save
            << (sal_uInt32)0                        // offset last save, 0 after a full save
            << nPersistOfs                      // File offset to persist pointers for this save operation
            << (sal_uInt32)1                        // Persist reference to the document persist object
            << (sal_uInt32)nPersistEntrys           // max persists written, Seed value for persist object id management
            << (sal_Int16)EPP_LastViewTypeSlideView // last view type
            << (sal_Int16)0x12;                     // padword

    return TRUE;
}

// ---------------------------------------------------------------------------------------------

PPTExCharSheet::PPTExCharSheet( int nInstance )
{
    sal_uInt16 nFontHeight;

    for ( int nDepth = 0; nDepth < 5; nDepth++ )
    {
        PPTExCharLevel& rLev = maCharLevel[ nDepth ];
        switch ( nInstance )
        {
            case EPP_TEXTTYPE_Title :
            case EPP_TEXTTYPE_CenterTitle :
                nFontHeight = 44;
            break;
            case EPP_TEXTTYPE_Body :
            case EPP_TEXTTYPE_CenterBody :
            case EPP_TEXTTYPE_HalfBody :
            case EPP_TEXTTYPE_QuarterBody :
            {
                switch ( nDepth )
                {
                    case 0 : nFontHeight = 32; break;
                    case 1 : nFontHeight = 28; break;
                    case 2 : nFontHeight = 24; break;
                    default :nFontHeight = 20; break;
                }
            }
            break;
            case EPP_TEXTTYPE_Notes :
                nFontHeight = 12;
            break;
            case EPP_TEXTTYPE_notUsed :
            case EPP_TEXTTYPE_Other :
                nFontHeight = 24;
            break;
        }
        rLev.mnFlags = 0;
        rLev.mnFont = 0;
        rLev.mnAsianOrComplexFont = 0xffff;
        rLev.mnFontHeight = nFontHeight;
        rLev.mnFontColor = 0xfe000000;
        rLev.mnEscapement = 0;
    }
}


void PPTExCharSheet::SetStyleSheet( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
                                    FontCollection& rFontCollection, int nLevel )
{
    PortionObj  aPortionObj( rXPropSet, rFontCollection );

    PPTExCharLevel& rLev = maCharLevel[ nLevel ];

    if ( aPortionObj.meCharColor == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        rLev.mnFontColor = aPortionObj.mnCharColor;
    if ( aPortionObj.meCharEscapement == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        rLev.mnEscapement = aPortionObj.mnCharEscapement;
    if ( aPortionObj.meCharHeight == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        rLev.mnFontHeight = aPortionObj.mnCharHeight;
    if ( aPortionObj.meFontName == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        rLev.mnFont = aPortionObj.mnFont;
    if ( aPortionObj.meAsianOrComplexFont == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        rLev.mnAsianOrComplexFont = aPortionObj.mnAsianOrComplexFont;
    rLev.mnFlags = aPortionObj.mnCharAttr;
}

void PPTExCharSheet::Write( SvStream& rSt, PptEscherEx* pEx, sal_uInt16 nLev, sal_Bool bFirst, sal_Bool bSimpleText )
{
    const PPTExCharLevel& rLev = maCharLevel[ nLev ];

    sal_uInt32 nCharFlags = 0xefffff;
    if ( bSimpleText )
        nCharFlags = 0x7ffff;

    rSt << nCharFlags
        << rLev.mnFlags
        << rLev.mnFont;

    if ( bSimpleText )
    {
        rSt << rLev.mnFontHeight
            << rLev.mnFontColor;
    }
    else
    {
        rSt << rLev.mnAsianOrComplexFont
            << (sal_uInt16)0xffff       // unbekannt
            << (sal_uInt16)0xffff       // unbekannt
            << rLev.mnFontHeight
            << rLev.mnFontColor
            << rLev.mnEscapement;
    }
}

PPTExParaSheet::PPTExParaSheet( int nInstance, sal_uInt16 nDefaultTab, PPTExBulletProvider& rProv ) :
    rBuProv     ( rProv ),
    mnInstance  ( nInstance )
{
    sal_Bool bHasBullet = FALSE;

    sal_uInt16 nUpperDist = 0;
    sal_uInt16 nBulletChar;
    sal_uInt16 nBulletOfs;
    sal_uInt16 nTextOfs;

    for ( int nDepth = 0; nDepth < 5; nDepth++ )
    {
        PPTExParaLevel& rLev = maParaLevel[ nDepth ];
        switch ( nInstance )
        {
            case EPP_TEXTTYPE_Title :
            case EPP_TEXTTYPE_CenterTitle :
            break;
            case EPP_TEXTTYPE_Body :
            case EPP_TEXTTYPE_CenterBody :
            case EPP_TEXTTYPE_HalfBody :
            case EPP_TEXTTYPE_QuarterBody :
            {
                bHasBullet = TRUE;
                nUpperDist = 0x14;
            }
            break;
            case EPP_TEXTTYPE_Notes :
                nUpperDist = 0x1e;
            break;

//          default :
//          case EPP_TEXTTYPE_notUsed :
//          case EPP_TEXTTYPE_Other :
//          break;
        }
        switch ( nDepth )
        {
            case 0 :
            {
                nBulletChar = 0x2022;
                nBulletOfs = 0;
                nTextOfs = ( bHasBullet ) ? 0xd8 : 0;
            }
            break;
            case 1 :
            {
                nBulletChar = 0x2013;
                nBulletOfs = 0x120;
                nTextOfs = 0x1d4;
            }
            break;
            case 2 :
            {
                nBulletChar = 0x2022;
                nBulletOfs = 0x240;
                nTextOfs = 0x2d0;
            }
            break;
            case 3 :
            {
                nBulletChar = 0x2013;
                nBulletOfs = 0x360;
                nTextOfs = 0x3f0;
            }
            break;
            case 4 :
            {
                nBulletChar = 0xbb;
                nBulletOfs = 0x480;
                nTextOfs = 0x510;
            }
            break;
        }
        rLev.mbIsBullet = bHasBullet;
        rLev.mnBulletChar = nBulletChar;
        rLev.mnBulletFont = 0;
        rLev.mnBulletHeight = 100;
        rLev.mnBulletColor = 0xfe000000;
        rLev.mnAdjust = 0;
        rLev.mnLineFeed = 100;
        rLev.mnLowerDist = 0;
        rLev.mnUpperDist = nUpperDist;
        rLev.mnTextOfs = nTextOfs;
        rLev.mnBulletOfs = nBulletOfs;
        rLev.mnDefaultTab = nDefaultTab;

        rLev.mbExtendedBulletsUsed = FALSE;
        rLev.mnBulletId = 0xffff;
        rLev.mnBulletStart = 0;
        rLev.mnMappedNumType = 0;
        rLev.mnNumberingType = 0;
    }
}

void PPTExParaSheet::SetStyleSheet( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
                                        FontCollection& rFontCollection, int nLevel, sal_uInt16 nCharacterHeight )
{
    ParagraphObj aParagraphObj( rXPropSet, rBuProv );
    aParagraphObj.CalculateGraphicBulletSize( nCharacterHeight );
    PPTExParaLevel& rLev = maParaLevel[ nLevel ];

    if ( aParagraphObj.meTextAdjust == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        rLev.mnAdjust = aParagraphObj.mnTextAdjust;
    if ( aParagraphObj.meLineSpacing == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        rLev.mnLineFeed = aParagraphObj.mnLineSpacing;
    if ( aParagraphObj.meLineSpacingBottom == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        rLev.mnLowerDist = aParagraphObj.mnLineSpacingBottom;
    if ( aParagraphObj.meLineSpacingTop == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        rLev.mnUpperDist = aParagraphObj.mnLineSpacingTop;

    rLev.mbIsBullet = aParagraphObj.mbIsBullet; //( ( aParagraphObj.nBulletFlags & 1 ) != 0 );

    if ( !nLevel )
    {
        if ( ( aParagraphObj.meBullet ==  ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
                    && aParagraphObj.bExtendedParameters )
        {
            for ( int i = 0; i < 5; i++ )
            {
                PPTExParaLevel& rLev = maParaLevel[ i ];
                if ( i )
                    aParagraphObj.ImplGetNumberingLevel( rBuProv, i, FALSE );
//              rLev.mbIsBullet = ( ( aParagraphObj.nBulletFlags & 1 ) != 0 );
                rLev.mnTextOfs = aParagraphObj.nTextOfs;
                rLev.mnBulletOfs = (sal_uInt16)aParagraphObj.nBulletOfs;
                rLev.mnBulletChar = aParagraphObj.cBulletId;
                FontCollectionEntry aFontDescEntry( aParagraphObj.aFontDesc.Name, aParagraphObj.aFontDesc.Family,
                                                        aParagraphObj.aFontDesc.Pitch, aParagraphObj.aFontDesc.CharSet );
                rLev.mnBulletFont = (sal_uInt16)rFontCollection.GetId( aFontDescEntry );
                rLev.mnBulletHeight = aParagraphObj.nBulletRealSize;
                rLev.mnBulletColor = aParagraphObj.nBulletColor;

                rLev.mbExtendedBulletsUsed = aParagraphObj.bExtendedBulletsUsed;
                rLev.mnBulletId = aParagraphObj.nBulletId;
                rLev.mnNumberingType = aParagraphObj.nNumberingType;
                rLev.mnBulletStart = aParagraphObj.nStartWith;
                rLev.mnMappedNumType = aParagraphObj.nMappedNumType;
            }
        }
    }
}

void PPTExParaSheet::Write( SvStream& rSt, PptEscherEx* pEx, sal_uInt16 nLev, sal_Bool bFirst, sal_Bool bSimpleText )
{
    const PPTExParaLevel& rLev = maParaLevel[ nLev ];

    if ( maParaLevel[ 0 ].mbExtendedBulletsUsed || maParaLevel[ 1 ].mbExtendedBulletsUsed ||
            maParaLevel[ 2 ].mbExtendedBulletsUsed || maParaLevel[ 3 ].mbExtendedBulletsUsed ||
                maParaLevel[ 4 ].mbExtendedBulletsUsed )
    {
        SvStream& rOut = rBuProv.aBuExMasterStream;
        if ( !nLev )
        {
            rOut << (sal_uInt32)( ( EPP_PST_ExtendedParagraphMasterAtom << 16 ) | ( mnInstance << 4 ) )
                 << (sal_uInt32)( 5 * 16 + 2 )
                 << (sal_uInt16)5;              // depth
        }
        sal_uInt16 nBulletId = rLev.mnBulletId;
        if ( rLev.mnNumberingType != SVX_NUM_BITMAP )
            nBulletId = 0xffff;
        rOut << (sal_uInt32)0x03800000
             << (sal_uInt16)nBulletId
             << (sal_uInt32)rLev.mnMappedNumType
             << (sal_uInt16)rLev.mnBulletStart
             << (sal_uInt32)0;
    }

    sal_uInt32 nParaFlags = 0x3ffdff;
    sal_uInt16 nBulletFlags = ( rLev.mbIsBullet ) ? 0xf : 0xe;

    if ( nLev || bSimpleText )
        nParaFlags &= 0x7fff;

    rSt << nParaFlags
        << nBulletFlags
        << rLev.mnBulletChar
        << rLev.mnBulletFont
        << rLev.mnBulletHeight
        << rLev.mnBulletColor
        << rLev.mnAdjust
        << rLev.mnLineFeed
        << rLev.mnUpperDist
        << rLev.mnLowerDist
        << rLev.mnTextOfs
        << rLev.mnBulletOfs;

    if ( bSimpleText || nLev )
        return;

    rSt << rLev.mnDefaultTab
        << (sal_uInt16)0
        << (sal_uInt16)0
        << (sal_uInt16)2
        << (sal_uInt16)0;
}


PPTExStyleSheet::PPTExStyleSheet( sal_uInt16 nDefaultTab, PPTExBulletProvider& rBuProv )
{
    for ( int nInstance = EPP_TEXTTYPE_Title; nInstance <= EPP_TEXTTYPE_QuarterBody; nInstance++ )
    {
        mpParaSheet[ nInstance ] = ( nInstance == EPP_TEXTTYPE_notUsed ) ? NULL : new PPTExParaSheet( nInstance, nDefaultTab, rBuProv );
        mpCharSheet[ nInstance ] = ( nInstance == EPP_TEXTTYPE_notUsed ) ? NULL : new PPTExCharSheet( nInstance );
    }
}

PPTExStyleSheet::~PPTExStyleSheet()
{
    for ( int nInstance = EPP_TEXTTYPE_Title; nInstance <= EPP_TEXTTYPE_QuarterBody; nInstance++ )
    {
        if ( nInstance == EPP_TEXTTYPE_notUsed )
            continue;

        delete mpParaSheet[ nInstance ];
        delete mpCharSheet[ nInstance ];
    }
}

void PPTExStyleSheet::SetStyleSheet( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
                                        FontCollection& rFontCollection, int nInstance, int nLevel )
{
    if ( nInstance == EPP_TEXTTYPE_notUsed )
        return;

    mpCharSheet[ nInstance ]->SetStyleSheet( rXPropSet, rFontCollection, nLevel );
    mpParaSheet[ nInstance ]->SetStyleSheet( rXPropSet, rFontCollection, nLevel, mpCharSheet[ nInstance ]->maCharLevel[ nLevel ].mnFontHeight );
}

sal_Bool PPTExStyleSheet::IsHardAttribute( sal_uInt32 nInstance, sal_uInt32 nLevel, PPTExTextAttr eAttr, sal_uInt32 nValue )
{
    const PPTExParaLevel& rPara = mpParaSheet[ nInstance ]->maParaLevel[ nLevel ];
    const PPTExCharLevel& rChar = mpCharSheet[ nInstance ]->maCharLevel[ nLevel ];

    sal_uInt32 nFlag = 0;

    switch ( eAttr )
    {
        case ParaAttr_BulletOn : return ( rPara.mbIsBullet ) ? ( nValue ) ? FALSE : TRUE : ( nValue ) ? TRUE : FALSE;
        case ParaAttr_BuHardFont :
        case ParaAttr_BulletFont : return ( rPara.mnBulletFont != nValue );
        case ParaAttr_BuHardColor :
        case ParaAttr_BulletColor : return ( rPara.mnBulletColor != nValue );
        case ParaAttr_BuHardHeight :
        case ParaAttr_BulletHeight : return ( rPara.mnBulletHeight != nValue );
        case ParaAttr_BulletChar : return ( rPara.mnBulletChar != nValue );
        case ParaAttr_Adjust : return ( rPara.mnAdjust != nValue );
        case ParaAttr_LineFeed : return ( rPara.mnLineFeed != nValue );
        case ParaAttr_UpperDist : return ( rPara.mnUpperDist != nValue );
        case ParaAttr_LowerDist : return ( rPara.mnLowerDist != nValue );
        case ParaAttr_TextOfs : return ( rPara.mnTextOfs != nValue );
        case ParaAttr_BulletOfs : return ( rPara.mnBulletOfs != nValue );
        case ParaAttr_DefaultTab : return ( rPara.mnDefaultTab != nValue );
        case CharAttr_Bold : nFlag = 1; break;
        case CharAttr_Italic : nFlag = 2; break;
        case CharAttr_Underline : nFlag = 4; break;
        case CharAttr_Shadow : nFlag = 16; break;
        case CharAttr_Strikeout : nFlag = 256; break;
        case CharAttr_Embossed : nFlag = 512; break;
        case CharAttr_Font : return ( rChar.mnFont != nValue );
        case CharAttr_AsianOrComplexFont : return ( rChar.mnAsianOrComplexFont != nValue );
        case CharAttr_Symbol : return TRUE;
        case CharAttr_FontHeight : return ( rChar.mnFontHeight != nValue );
        case CharAttr_FontColor : return ( rChar.mnFontColor != nValue );
        case CharAttr_Escapement : return ( rChar.mnEscapement != nValue );
    };
    if ( nFlag )
    {
        if ( rChar.mnFlags & nFlag )
            return ( ( nValue & nFlag ) == 0 );
        else
            return ( ( nValue & nFlag ) != 0 );
    }
    return TRUE;
}

void PPTExStyleSheet::Write( SvStream& rSt, PptEscherEx* pEx )
{
    for ( int nInstance = EPP_TEXTTYPE_Title; nInstance <= EPP_TEXTTYPE_QuarterBody; nInstance++ )
    {
        if ( nInstance == EPP_TEXTTYPE_notUsed )
            continue;

        pEx->BeginAtom();

        sal_Bool bFirst = TRUE;
        sal_Bool bSimpleText = FALSE;

        rSt << (sal_uInt16)5;                           // paragraph count

        for ( sal_uInt16 nLev = 0; nLev < 5; nLev++ )
        {
            if ( nInstance >= EPP_TEXTTYPE_CenterBody )
            {
                bFirst = FALSE;
                bSimpleText = TRUE;
                rSt << nLev;
            }
            mpParaSheet[ nInstance ]->Write( rSt, pEx, nLev, bFirst, bSimpleText );
            mpCharSheet[ nInstance ]->Write( rSt, pEx, nLev, bFirst, bSimpleText );
            bFirst = FALSE;
        }
        pEx->EndAtom( EPP_TxMasterStyleAtom, 0, nInstance );
    }
}

sal_uInt32 PPTExStyleSheet::SizeOfTxCFStyleAtom() const
{
    return 24;
}

// the TxCFStyleAtom stores the text properties that are used
// when creating new objects in PowerPoint.

void PPTExStyleSheet::WriteTxCFStyleAtom( SvStream& rSt )
{
    const PPTExCharLevel& rCharStyle = mpCharSheet[ EPP_TEXTTYPE_Other ]->maCharLevel[ 0 ];
    const PPTExParaLevel& rParaStyle = mpParaSheet[ EPP_TEXTTYPE_Other ]->maParaLevel[ 0 ];

    sal_uInt16 nFlags = 0x60        // ??
                      | 0x02        // fontsize;
                      | 0x04;       // fontcolor

    sal_uInt32 nCharFlags = rCharStyle.mnFlags;
    nCharFlags &= CharAttr_Italic | CharAttr_Bold | CharAttr_Underline | CharAttr_Shadow;

    rSt << (sal_uInt32)( EPP_TxCFStyleAtom << 16 )  // recordheader
        << SizeOfTxCFStyleAtom() - 8
        << (sal_uInt16)( 0x80 | nCharFlags )
        << (sal_uInt16)nFlags
        << (sal_uInt16)nCharFlags
        << (sal_Int32)-1                            // ?
        << rCharStyle.mnFontHeight
        << rCharStyle.mnFontColor;
}


// ---------------------------------------------------------------------------------------------

// ---------------------
// - exported function -
// ---------------------

extern "C" BOOL __LOADONCALLAPI ExportPPT( SvStorageRef& rSvStorage, SvStorageRef& xOleSource,
                    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & rXModel,
                        ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > & rXStatInd,
                            SvMemoryStream* pVBA, sal_uInt32 nCnvrtFlags )
{
    PPTWriter*  pPPTWriter;
    BOOL bStatus = FALSE;

    pPPTWriter = new PPTWriter( rSvStorage, xOleSource, rXModel, rXStatInd, pVBA, nCnvrtFlags );
    if ( pPPTWriter )
    {
        bStatus = ( pPPTWriter->IsValid() == TRUE );
        delete pPPTWriter;
    }

    return bStatus;
}

