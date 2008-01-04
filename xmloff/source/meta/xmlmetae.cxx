/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlmetae.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-04 16:12:38 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#include <tools/debug.hxx>
#include <tools/inetdef.hxx>
#ifndef INCLUDED_I18NPOOL_MSLANGID_HXX
#include <i18npool/mslangid.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#include <unotools/configmgr.hxx>

#include <xmloff/xmlmetae.hxx>
#include <xmloff/attrlist.hxx>
#include <xmloff/nmspmap.hxx>

#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _TOOLS_TIME_HXX
#include <tools/time.hxx>
#endif

#ifndef _UTL_BOOTSTRAP_HXX
#include <unotools/bootstrap.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include <xmloff/xmlexp.hxx>
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif

#include <unotools/docinfohelper.hxx>

using namespace com::sun::star;
using namespace ::xmloff::token;

//-------------------------------------------------------------------------

#define PROP_TITLE          "Title"
#define PROP_DESCRIPTION    "Description"
#define PROP_SUBJECT          "Subject"
#define PROP_KEYWORDS       "Keywords"
#define PROP_AUTHOR         "Author"
#define PROP_CREATIONDATE   "CreationDate"
#define PROP_MODIFIEDBY     "ModifiedBy"
#define PROP_MODIFYDATE     "ModifyDate"
#define PROP_PRINTEDBY      "PrintedBy"
#define PROP_PRINTDATE      "PrintDate"
#define PROP_TEMPLATEURL    "TemplateFileName"
#define PROP_TEMPLATENAME   "Template"
#define PROP_TEMPLATEDATE   "TemplateDate"
#define PROP_RELOADENABLED  "AutoloadEnabled"
#define PROP_RELOADURL      "AutoloadURL"
#define PROP_RELOADSECS     "AutoloadSecs"
#define PROP_DEFAULTTARGET  "DefaultTarget"
#define PROP_EDITINGCYCLES  "EditingCycles"
#define PROP_EDITINGDURATION "EditingDuration"

#define PROP_CHARLOCALE     "Language"
#define PROP_DOCSTATISTIC   "DocumentStatistic"


//-------------------------------------------------------------------------

void lcl_AddTwoDigits( rtl::OUStringBuffer& rStr, sal_Int32 nVal )
{
    if ( nVal < 10 )
        rStr.append( sal_Unicode('0') );
    rStr.append( nVal );
}

// static
rtl::OUString SfxXMLMetaExport::GetISODateTimeString( const util::DateTime& rDateTime )
{
    //  return ISO date string "YYYY-MM-DDThh:mm:ss"

    rtl::OUStringBuffer sTmp;
    sTmp.append( (sal_Int32) rDateTime.Year );
    sTmp.append( sal_Unicode('-') );
    lcl_AddTwoDigits( sTmp, rDateTime.Month );
    sTmp.append( sal_Unicode('-') );
    lcl_AddTwoDigits( sTmp, rDateTime.Day );
    sTmp.append( sal_Unicode('T') );
    lcl_AddTwoDigits( sTmp, rDateTime.Hours );
    sTmp.append( sal_Unicode(':') );
    lcl_AddTwoDigits( sTmp, rDateTime.Minutes );
    sTmp.append( sal_Unicode(':') );
    lcl_AddTwoDigits( sTmp, rDateTime.Seconds );

    return sTmp.makeStringAndClear();
}

//-------------------------------------------------------------------------

SfxXMLMetaExport::SfxXMLMetaExport(
        SvXMLExport& rExp,
        const uno::Reference<frame::XModel>& rDocModel ) :
    rExport( rExp )
{
    uno::Reference<document::XDocumentInfoSupplier> xSupp( rDocModel, uno::UNO_QUERY );
    if ( xSupp.is() )
    {
        xDocInfo = xSupp->getDocumentInfo();
        xInfoProp = uno::Reference<beans::XPropertySet>( xDocInfo, uno::UNO_QUERY );
    }

    if ( xInfoProp.is() )
    {
        //  get document language from document properties
        //  (not available for all document types)
        try
        {
            uno::Any aLocAny = xInfoProp->getPropertyValue(
                        rtl::OUString::createFromAscii( PROP_CHARLOCALE ) );
            aLocAny >>= aLocale;
        }
        catch (beans::UnknownPropertyException&)
        {
            // no error
        }
    }

    // for Image etc. there is no XModel and no document info
//  DBG_ASSERT( xInfoProp.is(), "no document info properties" );
}

SfxXMLMetaExport::SfxXMLMetaExport(
        SvXMLExport& rExp,
        const uno::Reference<document::XDocumentInfo>& rDocInfo ) :
    rExport( rExp ),
    xDocInfo( rDocInfo )
{
    xInfoProp = uno::Reference<beans::XPropertySet>( xDocInfo, uno::UNO_QUERY );

    if ( xInfoProp.is() )
    {
        //  get document language from document info
        //  (not available for all document types)

        try
        {
            xInfoProp->getPropertyValue( rtl::OUString::createFromAscii( PROP_CHARLOCALE ) ) >>= aLocale;
        }
        catch (beans::UnknownPropertyException&)
        {
            // no error
        }

        try
        {
            // the document statistic is requested in this way only if the exporter is not based on model
            // but on document info directly
            xInfoProp->getPropertyValue( rtl::OUString::createFromAscii( PROP_DOCSTATISTIC ) ) >>= aDocStatistic;
        }
        catch (beans::UnknownPropertyException&)
        {
            // no error
        }
    }
}


SfxXMLMetaExport::~SfxXMLMetaExport()
{
}

void SfxXMLMetaExport::SimpleStringElement( const rtl::OUString& rPropertyName,
                                            sal_uInt16 nNamespace,
                                            enum XMLTokenEnum eElementName )
{
    uno::Any aAny = xInfoProp->getPropertyValue( rPropertyName );
    rtl::OUString sValue;
    if ( aAny >>= sValue )
    {
        if ( sValue.getLength() )
        {
            SvXMLElementExport aElem( rExport, nNamespace, eElementName,
                                      sal_True, sal_False );
            rExport.Characters( sValue );

        }
    }
}

void SfxXMLMetaExport::SimpleDateTimeElement(
        const rtl::OUString& rPropertyName, sal_uInt16 nNamespace,
        enum XMLTokenEnum eElementName )
{
    uno::Any aAny = xInfoProp->getPropertyValue( rPropertyName );
    util::DateTime aDateTime;
    if ( aAny >>= aDateTime )
    {
        rtl::OUString sValue = GetISODateTimeString( aDateTime );

        SvXMLElementExport aElem( rExport, nNamespace, eElementName,
                                  sal_True, sal_False );
        rExport.Characters( sValue );
    }
}

void SfxXMLMetaExport::Export()
{
    // BM: #i60323# export generator even if xInfoProp is empty (which is the
    // case for charts). The generator does not depend on xInfoProp
    rtl::OUString sElem, sSubElem, sAttrName, sValue;
    uno::Any aPropVal;

    //  generator
    // first token: real product name + version
    sValue = ::utl::DocInfoHelper::GetGeneratorString();
    {
        SvXMLElementExport aElem( rExport, XML_NAMESPACE_META, XML_GENERATOR,
                                  sal_True, sal_True );
        rExport.Characters( sValue );
    }

    if ( !xInfoProp.is() ) return;

    //  document title
    SimpleStringElement( ::rtl::OUString::createFromAscii(PROP_TITLE),
                         XML_NAMESPACE_DC, XML_TITLE );

    //  description
    SimpleStringElement( ::rtl::OUString::createFromAscii(PROP_DESCRIPTION),
                         XML_NAMESPACE_DC, XML_DESCRIPTION );

    //  subject
    SimpleStringElement( ::rtl::OUString::createFromAscii(PROP_SUBJECT),
                         XML_NAMESPACE_DC, XML_SUBJECT );

    //  created...
    SimpleStringElement( ::rtl::OUString::createFromAscii(PROP_AUTHOR),
                         XML_NAMESPACE_META, XML_INITIAL_CREATOR );
    SimpleDateTimeElement( ::rtl::OUString::createFromAscii(PROP_CREATIONDATE),
                           XML_NAMESPACE_META, XML_CREATION_DATE );

    //  modified...
    SimpleStringElement( ::rtl::OUString::createFromAscii(PROP_MODIFIEDBY),
                         XML_NAMESPACE_DC, XML_CREATOR );
    SimpleDateTimeElement( ::rtl::OUString::createFromAscii(PROP_MODIFYDATE),
                           XML_NAMESPACE_DC, XML_DATE );

    //  printed...
    SimpleStringElement( ::rtl::OUString::createFromAscii(PROP_PRINTEDBY),
                         XML_NAMESPACE_META, XML_PRINTED_BY );
    SimpleDateTimeElement( ::rtl::OUString::createFromAscii(PROP_PRINTDATE),
                           XML_NAMESPACE_META, XML_PRINT_DATE );

    //  keywords
    // service DocumentInfo contains keywords in a single string, comma separated.
    aPropVal = xInfoProp->getPropertyValue(
                    ::rtl::OUString::createFromAscii(PROP_KEYWORDS) );
    rtl::OUString sKeywords;
    aPropVal >>= sKeywords;
    if ( sKeywords.getLength() )
    {
        sal_Int32 nTokenIndex = 0;
        do
        {
            rtl::OUString sKeyword = sKeywords.getToken( 0, ',', nTokenIndex ).trim();

            SvXMLElementExport aKeywElem( rExport,
                                          XML_NAMESPACE_META, XML_KEYWORD,
                                          sal_True, sal_False );
            rExport.Characters( sKeyword );
        }
        while ( nTokenIndex >= 0 );
    }

    //  document language
#if 0
    if ( eLanguage != LANGUAGE_SYSTEM )
    {
        sValue = MsLangId::convertLanguageToIsoString( eLanguage, '-' );
        if ( sValue.getLength() )
        {
            SvXMLElementExport aElem( rExport, XML_NAMESPACE_DC, XML_LANGUAGE,
                                      sal_True, sal_False );
            aElem->Characters( sValue );
        }
    }
#endif
    if ( aLocale.Language.getLength() )
    {
        sValue = aLocale.Language;
        if ( aLocale.Country.getLength() )
        {
            sValue += rtl::OUString::valueOf((sal_Unicode)'-');
            sValue += aLocale.Country;
        }
        SvXMLElementExport aElem( rExport, XML_NAMESPACE_DC, XML_LANGUAGE,
                                  sal_True, sal_False );
        rExport.Characters( sValue );
    }

    //  editing cycles
    aPropVal = xInfoProp->getPropertyValue(
                    ::rtl::OUString::createFromAscii(PROP_EDITINGCYCLES) );
    sal_Int32 nCycles = 0;
    if ( aPropVal >>= nCycles )
    {
        sValue = rtl::OUString::valueOf( nCycles );

        SvXMLElementExport aElem( rExport,
                                  XML_NAMESPACE_META, XML_EDITING_CYCLES,
                                  sal_True, sal_False );
        rExport.Characters( sValue );
    }

    //  editing duration
    //  property is a int32 with the Time::GetTime value
    aPropVal = xInfoProp->getPropertyValue(
                    ::rtl::OUString::createFromAscii(PROP_EDITINGDURATION) );
    sal_Int32 nDurVal = 0;
    if ( aPropVal >>= nDurVal )
    {
        Time aDurTime( nDurVal );
        sValue = SvXMLUnitConverter::convertTimeDuration( aDurTime );

        SvXMLElementExport aElem( rExport,
                                  XML_NAMESPACE_META, XML_EDITING_DURATION,
                                  sal_True, sal_False );
        rExport.Characters( sValue );
    }

    //  default target
    aPropVal = xInfoProp->getPropertyValue(
                        ::rtl::OUString::createFromAscii(PROP_DEFAULTTARGET) );
    rtl::OUString sDefTarget;
    aPropVal >>= sDefTarget;
    if ( sDefTarget.getLength() )
    {
        rExport.AddAttribute( XML_NAMESPACE_OFFICE, XML_TARGET_FRAME_NAME,
                              sDefTarget );

        //! define strings for xlink:show values
        XMLTokenEnum eShow =
            sDefTarget.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("_blank"))
                ? XML_NEW : XML_REPLACE;
        rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, eShow );

        SvXMLElementExport aElem( rExport,
                                  XML_NAMESPACE_META,XML_HYPERLINK_BEHAVIOUR,
                                  sal_True, sal_False );
    }

    //  auto-reload
    aPropVal = xInfoProp->getPropertyValue(
                        ::rtl::OUString::createFromAscii(PROP_RELOADENABLED) );
    BOOL bAutoReload = FALSE;
    if ( aPropVal.getValueTypeClass() == uno::TypeClass_BOOLEAN )
        bAutoReload = *(sal_Bool*)aPropVal.getValue();
    if ( bAutoReload )
    {
        aPropVal = xInfoProp->getPropertyValue(
                            ::rtl::OUString::createFromAscii(PROP_RELOADURL) );
        rtl::OUString sReloadURL;
        aPropVal >>= sReloadURL;
        if ( sReloadURL.getLength() )
        {
            rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF,
                                  rExport.GetRelativeReference( sReloadURL) );
        }

        aPropVal = xInfoProp->getPropertyValue(
                            ::rtl::OUString::createFromAscii(PROP_RELOADSECS) );
        sal_Int32 nSecs = 0;
        if ( aPropVal >>= nSecs )
        {
            Time aTime;
            aTime.MakeTimeFromMS( nSecs * 1000 );
            rtl::OUString sReloadTime = SvXMLUnitConverter::convertTimeDuration( aTime );

            rExport.AddAttribute( XML_NAMESPACE_META, XML_DELAY, sReloadTime );
        }


        SvXMLElementExport aElem( rExport, XML_NAMESPACE_META, XML_AUTO_RELOAD,
                                  sal_True, sal_False );
    }

    //  template
    aPropVal = xInfoProp->getPropertyValue(
                        ::rtl::OUString::createFromAscii(PROP_TEMPLATEURL) );
    rtl::OUString sTplPath;
    aPropVal >>= sTplPath;
    if ( sTplPath.getLength() )
    {
        rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONREQUEST );

        //  template URL
        rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF,
                              rExport.GetRelativeReference(sTplPath) );

        //  template name
        aPropVal = xInfoProp->getPropertyValue(
                        ::rtl::OUString::createFromAscii(PROP_TEMPLATENAME) );
        rtl::OUString sTplName;
        aPropVal >>= sTplName;
        if ( sTplName.getLength() )
        {
            rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TITLE, sTplName );
        }

        //  template date
        aPropVal = xInfoProp->getPropertyValue(
                        ::rtl::OUString::createFromAscii(PROP_TEMPLATEDATE) );
        util::DateTime aDateTime;
        if ( aPropVal >>= aDateTime )
        {
            rtl::OUString sTplDate = GetISODateTimeString( aDateTime );

            rExport.AddAttribute( XML_NAMESPACE_META, XML_DATE, sTplDate );
        }

        SvXMLElementExport aElem( rExport, XML_NAMESPACE_META, XML_TEMPLATE,
                                  sal_True, sal_False );
    }

    //  user defined fields
    sal_Int16 nUFCount = xDocInfo->getUserFieldCount();
    if ( nUFCount )
    {
        for (sal_Int16 nUF=0; nUF<nUFCount; nUF++)
        {
            rtl::OUString aUFName = xDocInfo->getUserFieldName( nUF );
            rtl::OUString aUFValue = xDocInfo->getUserFieldValue( nUF );

            rExport.AddAttribute( XML_NAMESPACE_META, XML_NAME, aUFName );

            SvXMLElementExport aElem( rExport, XML_NAMESPACE_META,
                                      XML_USER_DEFINED, sal_True, sal_False );
            rExport.Characters( aUFValue );
        }
    }

    // extended user fields (type safe)
          uno::Reference< beans::XPropertySetInfo > xSetInfo = xInfoProp->getPropertySetInfo();
    const uno::Sequence< beans::Property >          lProps   = xSetInfo->getProperties();
    const beans::Property*                          pProps   = lProps.getConstArray();
          sal_Int32                                 c        = lProps.getLength();
          sal_Int32                                 i        = 0;
    for (i=0; i<c; ++i)
    {
        // "fix" property ? => ignore it !
        if (pProps[i].Handle >= 0)
            continue;

        // "dynamic" prop => export it
        uno::Any              aValue = xInfoProp->getPropertyValue(pProps[i].Name);
        ::rtl::OUStringBuffer sValueBuffer;
        ::rtl::OUStringBuffer sType ;

        if (!SvXMLUnitConverter::convertAny(sValueBuffer, sType, aValue))
            continue;

        rExport.AddAttribute( XML_NAMESPACE_META, XML_NAME, pProps[i].Name );
        rExport.AddAttribute( XML_NAMESPACE_META, XML_VALUE_TYPE, sType.makeStringAndClear() );
        SvXMLElementExport aElem( rExport, XML_NAMESPACE_META,
                                  XML_USER_DEFINED, sal_True, sal_False );
        rExport.Characters( sValueBuffer.makeStringAndClear() );
    }

    // write document statistic if there is any provided
    if ( aDocStatistic.getLength() )
    {
        for ( sal_Int32 nInd = 0; nInd < aDocStatistic.getLength(); nInd++ )
        {
            sal_Int32 nValue = 0;
            if ( aDocStatistic[nInd].Value >>= nValue )
            {
                ::rtl::OUString aValue = rtl::OUString::valueOf( nValue );

                if ( aDocStatistic[nInd].Name.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TableCount" ) ) ) )
                    rExport.AddAttribute( XML_NAMESPACE_META, XML_TABLE_COUNT, aValue );
                else if ( aDocStatistic[nInd].Name.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ObjectCount" ) ) ) )
                    rExport.AddAttribute( XML_NAMESPACE_META, XML_OBJECT_COUNT, aValue );
                else if ( aDocStatistic[nInd].Name.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ImageCount" ) ) ) )
                    rExport.AddAttribute( XML_NAMESPACE_META, XML_IMAGE_COUNT, aValue );
                else if ( aDocStatistic[nInd].Name.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PageCount" ) ) ) )
                    rExport.AddAttribute( XML_NAMESPACE_META, XML_PAGE_COUNT, aValue );
                else if ( aDocStatistic[nInd].Name.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ParagraphCount" ) ) ) )
                    rExport.AddAttribute( XML_NAMESPACE_META, XML_PARAGRAPH_COUNT, aValue );
                else if ( aDocStatistic[nInd].Name.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "WordCount" ) ) ) )
                    rExport.AddAttribute( XML_NAMESPACE_META, XML_WORD_COUNT, aValue );
                else if ( aDocStatistic[nInd].Name.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharacterCount" ) ) ) )
                    rExport.AddAttribute( XML_NAMESPACE_META, XML_CHARACTER_COUNT, aValue );
                else if ( aDocStatistic[nInd].Name.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CellCount" ) ) ) )
                    rExport.AddAttribute( XML_NAMESPACE_META, XML_CELL_COUNT, aValue );
                else
                {
                    DBG_ASSERT( sal_False, "Unknown statistic value!\n" );
                }
            }
        }
        SvXMLElementExport aElem( rExport, XML_NAMESPACE_META, XML_DOCUMENT_STATISTIC, sal_True, sal_True );
    }
}



