/*************************************************************************
 *
 *  $RCSfile: ftransl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: tra $ $Date: 2001-03-09 08:48:02 $
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

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _DFLVCONT_HXX_
#include "ftransl.hxx"
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XMIMECONTENTTYPE_HPP_
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#endif

#ifndef _IMPLHELPER_HXX_
#include "..\misc\ImplHelper.hxx"
#endif

#include <shlobj.h>

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

#define IMPL_NAME  "com.sun.star.datatransfer.DataFormatTranslator"

#define MODULE_PRIVATE
#define CPPUTYPE_SEQSALINT8       getCppuType( (const Sequence< sal_Int8 >*) 0 )
#define CPPUTYPE_OUSTR            getCppuType( (const ::rtl::OUString*) 0 )
#define CPPUTYPE_SALINT32         getCppuType( ( sal_Int32 * ) 0 )
#define OUSTR( str )              OUString::createFromAscii( #str )
#define OUSTR_( str )             OUString::createFromAscii( str )
#define EMPTY_OUSTR               OUString::createFromAscii( "" )
#define _PARAM_WINFORMATNAME      "windows_formatname"

#define PRIVATE_OO                OUString::createFromAscii( "application/x-openoffice;" _PARAM_WINFORMATNAME "=" )
#define PARAM_WINFORMATNAME       OUString::createFromAscii( _PARAM_WINFORMATNAME )

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace rtl;
using namespace osl;
using namespace cppu;
using namespace std;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::container;

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace MODULE_PRIVATE
{
    Sequence< OUString > SAL_CALL DataFormatTranslator_getSupportedServiceNames( )
    {
        Sequence< OUString > aRet(1);
        aRet[0] = OUString::createFromAscii("com.sun.star.datatransfer.DataFormatTranslator");
        return aRet;
    }
}

//------------------------------------------------------------------------
// ctor
//------------------------------------------------------------------------

CDataFormatTranslator::CDataFormatTranslator( const Reference< XMultiServiceFactory >& rSrvMgr ) :
    m_SrvMgr( rSrvMgr )
{
    initTranslationTable( );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

Any SAL_CALL CDataFormatTranslator::getSystemDataTypeFromDataFlavor( const DataFlavor& aDataFlavor )
    throw( RuntimeException )
{
    Any aAny;

    try
    {
        Reference< XMimeContentTypeFactory > refXMimeCntFactory( m_SrvMgr->createInstance(
            OUSTR( com.sun.star.datatransfer.MimeContentTypeFactory ) ), UNO_QUERY );

        if ( !refXMimeCntFactory.is( ) )
            throw RuntimeException( );

        Reference< XMimeContentType >
            refXMimeCntType( refXMimeCntFactory->createMimeContentType( aDataFlavor.MimeType ) );

        OUString fullMediaType = refXMimeCntType->getFullMediaType( );
        if ( isTextPlainMediaType( fullMediaType ) )
        {
            // default is CF_TEXT
            aAny <<= static_cast< sal_Int32 >( CF_TEXT );

            if ( refXMimeCntType->hasParameter( OUSTR( charset ) ) )
            {
                // but maybe it is unicode text or oem text
                OUString charset = refXMimeCntType->getParameterValue( OUSTR( charset ) );
                findStandardFormatIdForCharset( charset, aAny );
            }
        }
        else
        {
            if ( refXMimeCntType->hasParameter( PARAM_WINFORMATNAME ) )
            {
                OUString winfmtname = refXMimeCntType->getParameterValue( PARAM_WINFORMATNAME );
                aAny <<= winfmtname;

                setStandardFormatIdForNativeFormatName( winfmtname, aAny );
            }
            else
                findStdFormatIdOrNativeFormatNameForFullMediaType( refXMimeCntFactory, fullMediaType, aAny );
        }
    }
    catch( IllegalArgumentException& )
    {
        OSL_ENSURE( sal_False, "Invalid content-type detected!" );
    }
    catch( NoSuchElementException& )
    {
        OSL_ENSURE( sal_False, "Illegal content-type parameter" );
    }
    catch( ... )
    {
        OSL_ENSURE( sal_False, "Unexpected error" );
        throw;
    }

    return aAny;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

DataFlavor SAL_CALL CDataFormatTranslator::getDataFlavorFromSystemDataType( const Any& aSysDataType )
    throw( RuntimeException )
{
    OSL_PRECOND( aSysDataType.hasValue( ), "Empty system data type delivered" );

    DataFlavor aFlavor = mkDataFlv( EMPTY_OUSTR, EMPTY_OUSTR, CPPUTYPE_SEQSALINT8 );

    if ( aSysDataType.getValueType( ) == CPPUTYPE_SALINT32 )
    {
        sal_Int32 clipformat;
        aSysDataType >>= clipformat;

        findDataFlavorForStandardFormatId( clipformat, aFlavor );
    }
    else if ( aSysDataType.getValueType( ) == CPPUTYPE_OUSTR )
    {
        OUString nativeFormatName;
        aSysDataType >>= nativeFormatName;

        findDataFlavorForNativeFormatName( nativeFormatName, aFlavor );
    }
    else
        OSL_ASSERT( false );

    return aFlavor;
}

// -------------------------------------------------
// XServiceInfo
// -------------------------------------------------

OUString SAL_CALL CDataFormatTranslator::getImplementationName(  )
    throw( RuntimeException )
{
    return OUString::createFromAscii( IMPL_NAME );
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

sal_Bool SAL_CALL CDataFormatTranslator::supportsService( const OUString& ServiceName )
    throw( RuntimeException )
{
    Sequence < OUString > SupportedServicesNames = DataFormatTranslator_getSupportedServiceNames();

    for ( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

Sequence< OUString > SAL_CALL CDataFormatTranslator::getSupportedServiceNames( )
    throw( RuntimeException )
{
    return DataFormatTranslator_getSupportedServiceNames( );
}

// -------------------------------------------------
// to optimize searching we add all entries with a
// standard clipboard format number first, in the
// table before the entries with CF_INVALID
// if we are searching for a standard clipboard
// format number we can stop if we find the first
// CF_INVALID
// -------------------------------------------------

void SAL_CALL CDataFormatTranslator::initTranslationTable( )
{
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(DIF), CF_DIF ) );//SOT_FORMATSTR_ID_DIF
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Bitmap), CF_DIB ) );// SOT_FORMAT_BITMAP
    m_TranslTable.push_back( mkPublicFormatEntry(  OUSTR(text/plain;charset=utf-16), EMPTY_OUSTR, CF_UNICODETEXT, OUSTR(Unicode-Text), CPPUTYPE_OUSTR) );
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Locale), CF_LOCALE ) );
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Image WMF), CF_METAFILEPICT ) );// SOT_FORMAT_WMF
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(GDIMetaFile) ) );// SOT_FORMAT_GDIMETAFILE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Private) ) );// SOT_FORMAT_PRIVATE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(FileName) ) );// SOT_FORMAT_FILE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(FileList) ) );// SOT_FORMAT_FILE_LIST
    m_TranslTable.push_back( mkPublicFormatEntry(  OUSTR(text/richtext), OUSTR(Rich Text Format) ) );// SOT_FORMAT_RTF
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Drawing Format) ) ); // SOT_FORMATSTR_ID_DRAWING
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(SVXB (StarView Bitmap/Animation)) ) );// SOT_FORMATSTR_ID_SVXB
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(SVIM (StarView ImageMap)) ) );// SOT_FORMATSTR_ID_SVIM
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(XFA (XOutDev FillAttr)) ) ); // SOT_FORMATSTR_ID_XFA
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(EditEngineFormat) ) ); // SOT_FORMATSTR_ID_EDITENGINE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StatusInfo vom SvxInternalLink) ) ); // SOT_FORMATSTR_ID_INTERNALLINK_STATE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(SOLK (StarOffice Link)) ) ); // SOT_FORMATSTR_ID_SOLK
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Netscape Bookmark) ) );// SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(SV_LBOX_DD_FORMAT) ) );// SOT_FORMATSTR_ID_TREELISTBOX
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Native) ) );// SOT_FORMATSTR_ID_NATIVE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(OwnerLink) ) );    // SOT_FORMATSTR_ID_OWNERLINK
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarServerFormat) ) );// SOT_FORMATSTR_ID_STARSERVER
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarObjectFormat) ) );// SOT_FORMATSTR_ID_STAROBJECT
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Applet Object) ) );// SOT_FORMATSTR_ID_APPLETOBJECT
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(PlugIn Object) ) );// SOT_FORMATSTR_ID_PLUGIN_OBJECT
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarWriter 3.0) ) );// SOT_FORMATSTR_ID_STARWRITER_30
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarWriter 4.0) ) );//SOT_FORMATSTR_ID_STARWRITER_40
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarWriter 5.0) ) );//SOT_FORMATSTR_ID_STARWRITER_50
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarWriter/Web 4.0) ) );//SOT_FORMATSTR_ID_STARWRITERWEB_40
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarWriter/Web 5.0) ) );//SOT_FORMATSTR_ID_STARWRITERWEB_50
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarWriter/Global 4.0) ) );//SOT_FORMATSTR_ID_STARWRITERGLOB_40
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarWriter/Global 5.0) ) );// SOT_FORMATSTR_ID_STARWRITERGLOB_50
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarDrawDocument) ) );//SOT_FORMATSTR_ID_STARDRAW
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarDrawDocument 4.0) ) );//SOT_FORMATSTR_ID_STARDRAW_40
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarImpress 5.0) ) );//SOT_FORMATSTR_ID_STARIMPRESS_50
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarDraw 5.0) ) );// SOT_FORMATSTR_ID_STARDRAW_50
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarCalcDocument) ) );//SOT_FORMATSTR_ID_STARCALC
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarCalc 4.0) ) );//SOT_FORMATSTR_ID_STARCALC_40
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarCalc 5.0) ) );// SOT_FORMATSTR_ID_STARCALC_50
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarChartDocument) ) );// SOT_FORMATSTR_ID_STARCHART
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarChartDocument) ) );    // SOT_FORMATSTR_ID_STARCHART_40
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarChart 5.0) ) );// SOT_FORMATSTR_ID_STARCHART_50
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarImageDocument) ) );//SOT_FORMATSTR_ID_STARIMAGE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarImageDocument 4.0) ) );//SOT_FORMATSTR_ID_STARIMAGE_40
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarImage 5.0) ) );    //SOT_FORMATSTR_ID_STARIMAGE_50
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarMath) ) );//SOT_FORMATSTR_ID_STARMATH
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarMathDocument 4.0) ) );//SOT_FORMATSTR_ID_STARMATH_40
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarMath 5.0) ) );//SOT_FORMATSTR_ID_STARMATH_50
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarObjectPaintDocument) ) );  //SOT_FORMATSTR_ID_STAROBJECT_PAINTDOC
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(FilledArea) ) );//SOT_FORMATSTR_ID_FILLED_AREA
    m_TranslTable.push_back( mkPublicFormatEntry(  OUSTR(text/html), OUSTR(HTML (HyperText Markup Language)) ) );//SOT_FORMATSTR_ID_HTML
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(HTML Format) ) );//SOT_FORMATSTR_ID_HTML_SIMPLE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(FORMAT_CHAOS) ) );//SOT_FORMATSTR_ID_CHAOS
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(CNT_MSGATTACHFILE_FORMAT) ) );//SOT_FORMATSTR_ID_CNT_MSGATTACHFILE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Biff5) ) );    //SOT_FORMATSTR_ID_BIFF_5
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Biff 5) ) );   //SOT_FORMATSTR_ID_BIFF__5
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Sylk) ) ); //SOT_FORMATSTR_ID_SYLK
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(SYLK) ) ); //SOT_FORMATSTR_ID_SYLK_BIGCAPS
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Link) ) );//SOT_FORMATSTR_ID_LINK
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarDraw TabBar) ) );  //SOT_FORMATSTR_ID_STARDRAW_TABBAR
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(SONLK (StarOffice Navi Link)) ) );//SOT_FORMATSTR_ID_SONLK
    m_TranslTable.push_back( mkPublicFormatEntry(  OUSTR(application/msword), OUSTR(MSWordDoc) ) ); //SOT_FORMATSTR_ID_MSWORD_DOC
    //m_TranslTable.push_back( mkPrivateFormatEntry(  OUSTR(MSWordDoc) ) ); //SOT_FORMATSTR_ID_MSWORD_DOC
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarFrameSetDocument) ) );//SOT_FORMATSTR_ID_STAR_FRAMESET_DOC
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(OfficeDocument) ) );//SOT_FORMATSTR_ID_OFFICE_DOC
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(NotesDocInfo) ) ); //SOT_FORMATSTR_ID_NOTES_DOCINFO
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(NoteshNote) ) );//SOT_FORMATSTR_ID_NOTES_HNOTE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Native) ) );       //SOT_FORMATSTR_ID_NOTES_NATIVE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(SfxDocument) ) );//SOT_FORMATSTR_ID_SFX_DOC
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(EVDF (Explorer View Dummy Format)) ) );//SOT_FORMATSTR_ID_EVDF
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(ESDF (Explorer Search Dummy Format) ) ) );//SOT_FORMATSTR_ID_ESDF
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(IDF (Iconview Dummy Format)) ) );//SOT_FORMATSTR_ID_IDF
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(EFTP (Explorer Ftp File) ) ) );    //SOT_FORMATSTR_ID_EFTP
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(EFD (Explorer Ftp Dir) ) ) );  //SOT_FORMATSTR_ID_EFD
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(SvxFormFieldExch) ) );//SOT_FORMATSTR_ID_SVX_FORMFIELDEXCH
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(ExtendedTabBar) ) );//SOT_FORMATSTR_ID_EXTENDED_TABBAR
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(SBA-DATAFORMAT) ) );//SOT_FORMATSTR_ID_SBA_DATAEXCHANGE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(SBA-FIELDFORMAT) ) );  //SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(SBA-PRIVATEURLFORMAT) ) );//SOT_FORMATSTR_ID_SBA_PRIVATE_URL
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Tabed) ) );    //SOT_FORMATSTR_ID_SBA_TABED
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Tabid) ) );    //SOT_FORMATSTR_ID_SBA_TABID
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(SBA-JOINFORMAT) ) );//SOT_FORMATSTR_ID_SBA_JOIN
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Star Object Descriptor) ) );//SOT_FORMATSTR_ID_OBJECTDESCRIPTOR
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Star Link Source Descriptor) ) );//SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Star Embed Source) ) );//SOT_FORMATSTR_ID_EMBED_SOURCE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Star Link Source) ) );//SOT_FORMATSTR_ID_LINK_SOURCE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Star Embedded Object) ) );//SOT_FORMATSTR_ID_EMBEDDED_OBJ
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR_(CFSTR_FILECONTENTS) ) );  //SOT_FORMATSTR_ID_FILECONTENT
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR_(CFSTR_FILEDESCRIPTOR) ) );//SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR_(CFSTR_FILENAME) ) );  //SOT_FORMATSTR_ID_FILENAME
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(SD-OLE) ) );//SOT_FORMATSTR_ID_SD_OLE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Embedded Object) ) );  //SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Embed Source) ) );//SOT_FORMATSTR_ID_EMBED_SOURCE_OLE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Object Descriptor) ) );    //SOT_FORMATSTR_ID_OBJECTDESCRIPTOR_OLE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Link Source Descriptor) ) );//SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR_OLE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Link Source) ) );  //SOT_FORMATSTR_ID_LINK_SOURCE_OLE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(SBA-CTRLFORMAT) ) );//SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(OutPlace Object) ) );  //SOT_FORMATSTR_ID_OUTPLACE_OBJ
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(CntOwnClipboard) ) );//SOT_FORMATSTR_ID_CNT_OWN_CLIP
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(SO-INet-Image) ) );//SOT_FORMATSTR_ID_INET_IMAGE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Netscape Image Format) ) );//SOT_FORMATSTR_ID_NETSCAPE_IMAGE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(SBA_FORMEXCHANGE) ) ); //SOT_FORMATSTR_ID_SBA_FORMEXCHANGE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(SBA_REPORTEXCHANGE) ) );//SOT_FORMATSTR_ID_SBA_REPORTEXCHANGE
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(UniformResourceLocator) ) );//SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(StarChartDocument 5.0) ) );//SOT_FORMATSTR_ID_STARCHARTDOCUMENT_50
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(Graphic Object) ) );   //SOT_FORMATSTR_ID_GRAPHOBJ
    m_TranslTable.push_back( mkPublicFormatEntry(  OUSTR(application/vnd.sun.xml.writer), OUSTR(Writer 6.0) ) );//SOT_FORMATSTR_ID_STARWRITER_60
    m_TranslTable.push_back( mkPublicFormatEntry(  OUSTR(application/vnd.sun.xml.writer.web), OUSTR(Writer/Web 6.0) ) );//SOT_FORMATSTR_ID_STARWRITERWEB_60
    m_TranslTable.push_back( mkPublicFormatEntry(  OUSTR(application/vnd.sun.xml.writer.global), OUSTR(Writer/Global 6.0) ) );//SOT_FORMATSTR_ID_STARWRITERGLOB_60
    m_TranslTable.push_back( mkPublicFormatEntry(  OUSTR(application/vnd.sun.xml.draw), OUSTR(Draw 6.0) ) );//SOT_FORMATSTR_ID_STARWDRAW_60
    m_TranslTable.push_back( mkPublicFormatEntry(  OUSTR(application/vnd.sun.xml.impress), OUSTR(Impress 6.0) ) );//SOT_FORMATSTR_ID_STARIMPRESS_60
    m_TranslTable.push_back( mkPublicFormatEntry(  OUSTR(application/vnd.sun.xml.calc), OUSTR(Calc 6.0) ) );//SOT_FORMATSTR_ID_STARCALC_60
    m_TranslTable.push_back( mkPublicFormatEntry(  OUSTR(application/vnd.sun.xml.chart), OUSTR(Chart 6.0) ) );//SOT_FORMATSTR_ID_STARCHART_60
    m_TranslTable.push_back( mkPublicFormatEntry(  OUSTR(application/vnd.sun.xml.math), OUSTR(Math 6.0) ) );//SOT_FORMATSTR_ID_STARMATH_60
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(SO_DUMMYFORMAT_3) ) );//SOT_FORMATSTR_ID_DUMMY3
    m_TranslTable.push_back( mkPrivateFormatEntry( OUSTR(SO_DUMMYFORMAT_4) ) );//SOT_FORMATSTR_ID_DUMMY4
}

// -------------------------------------------------
//
// -------------------------------------------------

void SAL_CALL CDataFormatTranslator::findDataFlavorForStandardFormatId( sal_Int32 aStandardFormatId, DataFlavor& aDataFlavor ) const
{
    /*
        we break the for loop if we find the first CF_INVALID
        because in the translation table the entries with a
        standard clipboard format id appear before the other
        entries with CF_INVALID
    */
    vector< FormatEntry >::const_iterator citer_end = m_TranslTable.end( );
    for ( vector< FormatEntry >::const_iterator citer = m_TranslTable.begin( ); citer != citer_end; ++citer )
    {
        sal_Int32 stdId = citer->aStandardFormatId;
        if ( aStandardFormatId == stdId )
        {
            aDataFlavor = citer->aDataFlavor;
            break;
        }
        else if ( stdId == CF_INVALID )
            break;
    }
}

// -------------------------------------------------
//
// -------------------------------------------------

void SAL_CALL CDataFormatTranslator::findDataFlavorForNativeFormatName( const OUString& aNativeFormatName, DataFlavor& aDataFlavor ) const
{
    vector< FormatEntry >::const_iterator citer_end = m_TranslTable.end( );
    for ( vector< FormatEntry >::const_iterator citer = m_TranslTable.begin( );
          citer != citer_end;
          ++citer )
    {
        if ( aNativeFormatName.equalsIgnoreCase( citer->aNativeFormatName ) )
        {
            aDataFlavor = citer->aDataFlavor;
            break;
        }
    }
}

// -------------------------------------------------
//
// -------------------------------------------------

void SAL_CALL CDataFormatTranslator::findStandardFormatIdForCharset( const OUString& aCharset, Any& aAny ) const
{
    if ( aCharset.equalsIgnoreCase( OUSTR( utf-16 ) ) )
        aAny <<= static_cast< sal_Int32 >( CF_UNICODETEXT );
    else
    {
        sal_Int32 wincp = getWinCPFromMimeCharset( aCharset );
        if ( IsOEMCP ( wincp ) )
            aAny <<= static_cast< sal_Int32 >( CF_OEMTEXT );
    }
}

// -------------------------------------------------
//
// -------------------------------------------------

void SAL_CALL CDataFormatTranslator::setStandardFormatIdForNativeFormatName( const OUString& aNativeFormatName, Any& aAny ) const
{
    vector< FormatEntry >::const_iterator citer_end = m_TranslTable.end( );
    for ( vector< FormatEntry >::const_iterator citer = m_TranslTable.begin( ); citer != citer_end; ++citer )
    {
        if ( aNativeFormatName.equalsIgnoreCase( citer->aNativeFormatName ) &&
             (CF_INVALID != citer->aStandardFormatId) )
        {
            aAny <<= citer->aStandardFormatId;
            break;
        }
    }
}

// -------------------------------------------------
//
// -------------------------------------------------

void SAL_CALL CDataFormatTranslator::findStdFormatIdOrNativeFormatNameForFullMediaType(
    const Reference< XMimeContentTypeFactory >& aRefXMimeFactory,
    const OUString& aFullMediaType,
    Any& aAny ) const
{
    vector< FormatEntry >::const_iterator citer_end = m_TranslTable.end( );
    for ( vector< FormatEntry >::const_iterator citer = m_TranslTable.begin( ); citer != citer_end; ++citer )
    {
        Reference< XMimeContentType >
        refXMime( aRefXMimeFactory->createMimeContentType( citer->aDataFlavor.MimeType ) );
        if ( aFullMediaType.equalsIgnoreCase( refXMime->getFullMediaType( ) ) )
        {
            sal_Int32 cf = citer->aStandardFormatId;
            if ( CF_INVALID != cf )
                aAny <<= cf;
            else
            {
                OSL_ENSURE( citer->aNativeFormatName.getLength( ),
                    "Invalid standard format id and empty native format name in translation table" );
                aAny <<= citer->aNativeFormatName;
            }
            break;
        }
    }
}

// -------------------------------------------------
//
// -------------------------------------------------

inline
sal_Bool CDataFormatTranslator::isTextPlainMediaType( const OUString& fullMediaType ) const
{
    return ( fullMediaType.equalsIgnoreCase( OUSTR( text/plain ) ) );
}

// -------------------------------------------------
//
// -------------------------------------------------

DataFlavor SAL_CALL CDataFormatTranslator::mkDataFlv( const OUString& cnttype, const OUString& hpname, Type dtype )
{
    DataFlavor dflv;

    dflv.MimeType             = cnttype;
    dflv.HumanPresentableName = hpname;
    dflv.DataType             = dtype;

    return dflv;
}

// -------------------------------------------------
//
// -------------------------------------------------

FormatEntry CDataFormatTranslator::mkFormatEntry( const DataFlavor& aDataFlavor, const OUString& aNativeFormatName, sal_Int32 aStandardFormatId )
{
    FormatEntry fmtDescr;

    fmtDescr.aDataFlavor        = aDataFlavor;
    fmtDescr.aNativeFormatName  = aNativeFormatName;
    fmtDescr.aStandardFormatId  = aStandardFormatId;

    return fmtDescr;
}

// -------------------------------------------------
//
// -------------------------------------------------

FormatEntry CDataFormatTranslator::mkPrivateFormatEntry( const OUString& aNativeFormatName, CLIPFORMAT aClipformat, const OUString& aHpName, Type aCppuType )
{
    DataFlavor aDFlv;
    OUString quotedFName = OUString::createFromAscii( "\"" ) + aNativeFormatName + OUString::createFromAscii( "\"" );

    if ( aHpName.getLength( ) )
        aDFlv = mkDataFlv( PRIVATE_OO + quotedFName, aHpName, aCppuType );
    else
        aDFlv = mkDataFlv( PRIVATE_OO + quotedFName, aNativeFormatName, aCppuType );

    FormatEntry fmtentry = mkFormatEntry( aDFlv, aNativeFormatName, aClipformat );

    return fmtentry;
}

// -------------------------------------------------
//
// -------------------------------------------------

FormatEntry CDataFormatTranslator::mkPublicFormatEntry( const OUString& aContentType, const OUString& aNativeFormatName, CLIPFORMAT aClipformat, const OUString& aHpName, Type aCppuType )
{
    DataFlavor aDFlv;
    OUString quotedFName = OUString::createFromAscii( "\"" ) + aNativeFormatName + OUString::createFromAscii( "\"" );

    if ( aHpName.getLength( ) )
        aDFlv = mkDataFlv( aContentType, aHpName, aCppuType );
    else
        aDFlv = mkDataFlv( aContentType, quotedFName, aCppuType );

    return mkFormatEntry( aDFlv, aNativeFormatName, aClipformat );
}
