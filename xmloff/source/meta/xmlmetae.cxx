/*************************************************************************
 *
 *  $RCSfile: xmlmetae.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2001-02-23 19:15:11 $
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

#include <tools/debug.hxx>
#include <tools/inetdef.hxx>
#include <tools/isolang.hxx>
#include <unotools/configmgr.hxx>

#include "xmlmetae.hxx"
#include "xmlkywd.hxx"
#include "attrlist.hxx"
#include "nmspmap.hxx"
#include "rscrev.hxx"

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _TOOLS_TIME_HXX
#include <tools/time.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif

using namespace com::sun::star;

//-------------------------------------------------------------------------

#define PROP_TITLE          "Title"
#define PROP_DESCRIPTION    "Description"
#define PROP_THEME          "Theme"
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

#define PROP_CHARLOCALE     "CharLocale"


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

// static
rtl::OUString SfxXMLMetaExport::GetISODurationString( const Time& rTime )
{
    //  return ISO time period string

    rtl::OUStringBuffer sTmp;
    sTmp.append( sal_Unicode('P') );                // "period"

    sal_uInt16 nHours = rTime.GetHour();
    sal_Bool bHasHours = ( nHours > 0 );
    if ( nHours >= 24 )
    {
        //  add days

        sal_uInt16 nDays = nHours / 24;
        sTmp.append( (sal_Int32) nDays );
        sTmp.append( sal_Unicode('D') );            // "days"

        nHours -= nDays * 24;
    }
    sTmp.append( sal_Unicode('T') );                // "time"

    if ( bHasHours )
    {
        sTmp.append( (sal_Int32) nHours );
        sTmp.append( sal_Unicode('H') );            // "hours"
    }
    sal_uInt16 nMinutes = rTime.GetMin();
    if ( bHasHours || nMinutes > 0 )
    {
        sTmp.append( (sal_Int32) nMinutes );
        sTmp.append( sal_Unicode('M') );            // "minutes"
    }
    sal_uInt16 nSeconds = rTime.GetSec();
    sTmp.append( (sal_Int32) nSeconds );
    sTmp.append( sal_Unicode('S') );                // "seconds"

    return sTmp.makeStringAndClear();
}

//-------------------------------------------------------------------------

SfxXMLMetaExport::SfxXMLMetaExport(
        const uno::Reference<xml::sax::XDocumentHandler>& rHdl,
        const uno::Reference<frame::XModel>& rDocModel ) :
    xHandler( rHdl ),
    pNamespaceMap( NULL ),
    sCDATA( ::rtl::OUString::createFromAscii(sXML_CDATA) ),
    sWS( ::rtl::OUString::createFromAscii(sXML_WS) )
{
    uno::Reference<document::XDocumentInfoSupplier> xSupp( rDocModel, uno::UNO_QUERY );
    if ( xSupp.is() )
    {
        xDocInfo = xSupp->getDocumentInfo();
        xInfoProp = uno::Reference<beans::XPropertySet>( xDocInfo, uno::UNO_QUERY );
    }

    uno::Reference<beans::XPropertySet> xDocProp( rDocModel, uno::UNO_QUERY );
    if ( xDocProp.is() )
    {
        //  get document language from document properties
        //  (not available for all document types)

        try
        {
            uno::Any aLocAny = xDocProp->getPropertyValue(
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

    pAttrList = new SvXMLAttributeList;
    xAttrList = pAttrList;
}

SfxXMLMetaExport::~SfxXMLMetaExport()
{
}

void SfxXMLMetaExport::SimpleStringElement( const rtl::OUString& rPropertyName,
                                            sal_uInt16 nNamespace,
                                            const sal_Char* pElementName )
{
    uno::Any aAny = xInfoProp->getPropertyValue( rPropertyName );
    rtl::OUString sValue;
    if ( aAny >>= sValue )
    {
        if ( sValue.getLength() )
        {
            rtl::OUString sElem = pNamespaceMap->GetQNameByKey( nNamespace,
                            ::rtl::OUString::createFromAscii(pElementName) );
            xHandler->ignorableWhitespace( sWS );
            xHandler->startElement( sElem, xAttrList );
            xHandler->characters( sValue );
            xHandler->endElement( sElem );
        }
    }
}

void SfxXMLMetaExport::SimpleDateTimeElement(
        const rtl::OUString& rPropertyName, sal_uInt16 nNamespace,
        const sal_Char* pElementName )
{
    uno::Any aAny = xInfoProp->getPropertyValue( rPropertyName );
    util::DateTime aDateTime;
    if ( aAny >>= aDateTime )
    {
        rtl::OUString sValue = GetISODateTimeString( aDateTime );

        rtl::OUString sElem = pNamespaceMap->GetQNameByKey( nNamespace,
                ::rtl::OUString::createFromAscii(pElementName) );
        xHandler->ignorableWhitespace( sWS );
        xHandler->startElement( sElem, xAttrList );
        xHandler->characters( sValue );
        xHandler->endElement( sElem );
    }
}

rtl::OUString lcl_GetProductName()
{
    //  get the correct product name from the configuration

    rtl::OUStringBuffer aName;
    utl::ConfigManager* pMgr = utl::ConfigManager::GetConfigManager();
    if (pMgr)
    {
        rtl::OUString aValue;
        uno::Any aAny = pMgr->GetDirectConfigProperty(utl::ConfigManager::PRODUCTNAME);
        if ( (aAny >>= aValue) && aValue.getLength() )
            aName.append( aValue ).append( (sal_Unicode)' ' );

        aAny = pMgr->GetDirectConfigProperty(utl::ConfigManager::PRODUCTVERSION);
        if ( (aAny >>= aValue) && aValue.getLength() )
            aName.append( aValue ).append( (sal_Unicode)' ' );

        aAny = pMgr->GetDirectConfigProperty(utl::ConfigManager::PRODUCTEXTENSION);
        if ( (aAny >>= aValue) && aValue.getLength() )
            aName.append( aValue ).append( (sal_Unicode)' ' );
    }
    aName.append( (sal_Unicode)'(' );
    aName.appendAscii( TOOLS_INETDEF_OS );
    aName.append( (sal_Unicode)')' );

    return aName.makeStringAndClear();
}

void SfxXMLMetaExport::Export( const SvXMLNamespaceMap& rNamespaceMap )
{
    pNamespaceMap = &rNamespaceMap;
    if ( !xInfoProp.is() ) return;

    rtl::OUString sElem, sSubElem, sAttrName, sValue;
    uno::Any aPropVal;

    //  generator (exported only)
    sValue = lcl_GetProductName();
    sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_META,
                        ::rtl::OUString::createFromAscii(sXML_generator) );
    xHandler->ignorableWhitespace( sWS );
    xHandler->startElement( sElem, xAttrList );
    xHandler->characters( sValue );
    xHandler->endElement( sElem );

    //  build-id as comment
    uno::Reference<xml::sax::XExtendedDocumentHandler> xExt( xHandler,
                                uno::UNO_QUERY );   //! pass in ctor?
    if (xExt.is())
    {
        sValue = String::CreateFromAscii( RSCUPDVER );
        xExt->comment( sValue );
    }

    //  document title
    SimpleStringElement( ::rtl::OUString::createFromAscii(PROP_TITLE),
                         XML_NAMESPACE_DC, sXML_title );

    //  description
    SimpleStringElement( ::rtl::OUString::createFromAscii(PROP_DESCRIPTION),
                         XML_NAMESPACE_DC, sXML_description );

    //  subject
    SimpleStringElement( ::rtl::OUString::createFromAscii(PROP_THEME),
                         XML_NAMESPACE_DC, sXML_subject );

    //  created...
    SimpleStringElement( ::rtl::OUString::createFromAscii(PROP_AUTHOR),
                         XML_NAMESPACE_META, sXML_initial_creator );
    SimpleDateTimeElement( ::rtl::OUString::createFromAscii(PROP_CREATIONDATE),
                            XML_NAMESPACE_META, sXML_creation_date );

    //  modified...
    SimpleStringElement( ::rtl::OUString::createFromAscii(PROP_MODIFIEDBY),
                         XML_NAMESPACE_DC, sXML_creator );
    SimpleDateTimeElement( ::rtl::OUString::createFromAscii(PROP_MODIFYDATE),
                           XML_NAMESPACE_DC, sXML_date );

    //  printed...
    SimpleStringElement( ::rtl::OUString::createFromAscii(PROP_PRINTEDBY),
                         XML_NAMESPACE_META, sXML_printed_by );
    SimpleDateTimeElement( ::rtl::OUString::createFromAscii(PROP_PRINTDATE),
                            XML_NAMESPACE_META, sXML_print_date );

    //  keywords
    // service DocumentInfo contains keywords in a single string, comma separated.
    aPropVal = xInfoProp->getPropertyValue(
                    ::rtl::OUString::createFromAscii(PROP_KEYWORDS) );
    rtl::OUString sKeywords;
    aPropVal >>= sKeywords;
    sal_Int32 nKWCount = sKeywords.getTokenCount( ',' );
    if ( sKeywords.getLength() && nKWCount )
    {
        sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_META,
                            ::rtl::OUString::createFromAscii(sXML_keywords) );
        xHandler->ignorableWhitespace( sWS );
        xHandler->startElement( sElem, xAttrList );
        for (sal_Int32 nKW=0; nKW<nKWCount; nKW++)
        {
            rtl::OUString sKeyword = sKeywords.getToken( nKW, ',' ).trim();

            sSubElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_META,
                            ::rtl::OUString::createFromAscii(sXML_keyword) );
            xHandler->ignorableWhitespace( sWS );
            xHandler->startElement( sSubElem, xAttrList );
            xHandler->characters( sKeyword );
            xHandler->endElement( sSubElem );
        }
        xHandler->ignorableWhitespace( sWS );
        xHandler->endElement( sElem );
    }

    //  document language
#if 0
    if ( eLanguage != LANGUAGE_SYSTEM )
    {
        sValue = ConvertLanguageToIsoString( eLanguage, '-' );
        if ( sValue.getLength() )
        {
            sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_DC,
                        ::rtl::OUString::createFromAscii(sXML_language) );
            xHandler->ignorableWhitespace( sWS );
            xHandler->startElement( sElem, xAttrList );
            xHandler->characters( sValue );
            xHandler->endElement( sElem );
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
        sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_DC,
                    ::rtl::OUString::createFromAscii(sXML_language) );
        xHandler->ignorableWhitespace( sWS );
        xHandler->startElement( sElem, xAttrList );
        xHandler->characters( sValue );
        xHandler->endElement( sElem );
    }

    //  editing cycles
    aPropVal = xInfoProp->getPropertyValue(
                    ::rtl::OUString::createFromAscii(PROP_EDITINGCYCLES) );
    sal_Int32 nCycles;
    if ( aPropVal >>= nCycles )
    {
        sValue = rtl::OUString::valueOf( nCycles );

        sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_META,
                    ::rtl::OUString::createFromAscii(sXML_editing_cycles) );
        xHandler->ignorableWhitespace( sWS );
        xHandler->startElement( sElem, xAttrList );
        xHandler->characters( sValue );
        xHandler->endElement( sElem );
    }

    //  editing duration
    //  property is a int32 with the Time::GetTime value
    aPropVal = xInfoProp->getPropertyValue(
                    ::rtl::OUString::createFromAscii(PROP_EDITINGDURATION) );
    sal_Int32 nDurVal;
    if ( aPropVal >>= nDurVal )
    {
        Time aDurTime( nDurVal );
        sValue = GetISODurationString( aDurTime );

        sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_META,
                    ::rtl::OUString::createFromAscii(sXML_editing_duration) );
        xHandler->ignorableWhitespace( sWS );
        xHandler->startElement( sElem, xAttrList );
        xHandler->characters( sValue );
        xHandler->endElement( sElem );
    }

    //  default target
    aPropVal = xInfoProp->getPropertyValue(
                        ::rtl::OUString::createFromAscii(PROP_DEFAULTTARGET) );
    rtl::OUString sDefTarget;
    aPropVal >>= sDefTarget;
    if ( sDefTarget.len() )
    {
        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_OFFICE,
                    ::rtl::OUString::createFromAscii(sXML_target_frame_name) );
        pAttrList->AddAttribute( sAttrName, sCDATA, sDefTarget );

        //! define strings for xlink:show values
        rtl::OUString sShow = ::rtl::OUString::createFromAscii(
            sDefTarget.compareToAscii( "_blank" ) == 0 ? sXML_new
                                                       : sXML_replace );

        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_XLINK,
                                ::rtl::OUString::createFromAscii(sXML_show) );
        pAttrList->AddAttribute( sAttrName, sCDATA, sShow );

        sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_META,
                ::rtl::OUString::createFromAscii(sXML_hyperlink_behaviour) );

        xHandler->ignorableWhitespace( sWS );
        xHandler->startElement( sElem, xAttrList );
        xHandler->endElement( sElem );

        pAttrList->Clear();
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
        if ( sReloadURL.len() )
        {
            sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_XLINK,
                                ::rtl::OUString::createFromAscii(sXML_href) );
            pAttrList->AddAttribute( sAttrName, sCDATA, sReloadURL );
        }

        aPropVal = xInfoProp->getPropertyValue(
                            ::rtl::OUString::createFromAscii(PROP_RELOADSECS) );
        sal_Int32 nSecs;
        if ( aPropVal >>= nSecs )
        {
            Time aTime;
            aTime.MakeTimeFromMS( nSecs * 1000 );
            rtl::OUString sReloadTime = GetISODurationString( aTime );

            sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_META,
                                ::rtl::OUString::createFromAscii(sXML_delay) );
            pAttrList->AddAttribute( sAttrName, sCDATA, sReloadTime );
        }

        sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_META,
                        ::rtl::OUString::createFromAscii(sXML_auto_reload) );

        xHandler->ignorableWhitespace( sWS );
        xHandler->startElement( sElem, xAttrList );
        xHandler->endElement( sElem );

        pAttrList->Clear();
    }

    //  template
    aPropVal = xInfoProp->getPropertyValue(
                        ::rtl::OUString::createFromAscii(PROP_TEMPLATEURL) );
    rtl::OUString sTplPath;
    aPropVal >>= sTplPath;
    if ( sTplPath.len() )
    {
        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_XLINK,
                                  ::rtl::OUString::createFromAscii(sXML_type) );
        pAttrList->AddAttribute( sAttrName, sCDATA,
                                 ::rtl::OUString::createFromAscii(sXML_simple) );
        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_XLINK,
                              ::rtl::OUString::createFromAscii(sXML_actuate) );
        pAttrList->AddAttribute( sAttrName, sCDATA,
                            ::rtl::OUString::createFromAscii(sXML_onRequest) );

        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_XLINK,
                              ::rtl::OUString::createFromAscii(sXML_role) );
        pAttrList->AddAttribute( sAttrName, sCDATA,
                      pNamespaceMap->GetQNameByKey( XML_NAMESPACE_OFFICE,
                            ::rtl::OUString::createFromAscii(sXML_template) ) );

        //  template URL
        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_XLINK,
                                ::rtl::OUString::createFromAscii(sXML_href) );
        pAttrList->AddAttribute( sAttrName, sCDATA, sTplPath );

        //  template name
        aPropVal = xInfoProp->getPropertyValue(
                        ::rtl::OUString::createFromAscii(PROP_TEMPLATENAME) );
        rtl::OUString sTplName;
        aPropVal >>= sTplName;
        if ( sTplName.len() )
        {
            sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_XLINK,
                            ::rtl::OUString::createFromAscii(sXML_title) );
            pAttrList->AddAttribute( sAttrName, sCDATA, sTplName );
        }

        //  template date
        aPropVal = xInfoProp->getPropertyValue(
                        ::rtl::OUString::createFromAscii(PROP_TEMPLATEDATE) );
        util::DateTime aDateTime;
        if ( aPropVal >>= aDateTime )
        {
            rtl::OUString sTplDate = GetISODateTimeString( aDateTime );

            sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_META,
                                ::rtl::OUString::createFromAscii(sXML_date) );
            pAttrList->AddAttribute( sAttrName, sCDATA, sTplDate );
        }

        sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_META,
                            ::rtl::OUString::createFromAscii(sXML_template) );

        xHandler->ignorableWhitespace( sWS );
        xHandler->startElement( sElem, xAttrList );
        xHandler->endElement( sElem );

        pAttrList->Clear();
    }

    //  user defined fields
    sal_Int16 nUFCount = xDocInfo->getUserFieldCount();
    if ( nUFCount )
    {
        sSubElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_META,
                        ::rtl::OUString::createFromAscii(sXML_user_defined) );
        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_META,
                        ::rtl::OUString::createFromAscii(sXML_name) );

        for (sal_Int16 nUF=0; nUF<nUFCount; nUF++)
        {
            rtl::OUString aUFName = xDocInfo->getUserFieldName( nUF );
            rtl::OUString aUFValue = xDocInfo->getUserFieldValue( nUF );

            pAttrList->AddAttribute( sAttrName, sCDATA, aUFName );

            xHandler->ignorableWhitespace( sWS );
            xHandler->startElement( sSubElem, xAttrList );
            xHandler->characters( aUFValue );
            xHandler->endElement( sSubElem );

            pAttrList->Clear();
        }
    }

    pNamespaceMap = NULL;
}



