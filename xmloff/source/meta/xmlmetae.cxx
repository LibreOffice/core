/*************************************************************************
 *
 *  $RCSfile: xmlmetae.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:15:46 $
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
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#include <unotools/configmgr.hxx>

#include "xmlmetae.hxx"
#include "attrlist.hxx"
#include "nmspmap.hxx"

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
#include "xmlexp.hxx"
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif

using namespace com::sun::star;
using namespace ::xmloff::token;

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
const sal_Char *sOpenOfficeOrgProject ="OpenOffice.org_project";


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

rtl::OUString lcl_GetProductName()
{
    //  get the correct product name from the configuration

    rtl::OUStringBuffer aName;

    // First product: branded name + version
    // version is <product_versions>_<product_extension>$<platform>
    utl::ConfigManager* pMgr = utl::ConfigManager::GetConfigManager();
    if (pMgr)
    {
        sal_Bool bValid = sal_True;
        // plain product name
        rtl::OUString aValue;
        uno::Any aAny = pMgr->GetDirectConfigProperty(
                                            utl::ConfigManager::PRODUCTNAME);
        if ( (aAny >>= aValue) && aValue.getLength() )
        {
            aName.append( aValue.replace( ' ', '_' ) );
            aName.append( (sal_Unicode)'/' );

            aAny = pMgr->GetDirectConfigProperty(
                                        utl::ConfigManager::PRODUCTVERSION);
            if ( (aAny >>= aValue) && aValue.getLength() )
            {
                aName.append( aValue.replace( ' ', '_' ) );

                aAny = pMgr->GetDirectConfigProperty(
                                        utl::ConfigManager::PRODUCTEXTENSION);
                if ( (aAny >>= aValue) && aValue.getLength() )
                {
                    aName.append( (sal_Unicode)'_' );
                    aName.append( aValue.replace( ' ', '_' ) );
                }
            }

            aName.append( (sal_Unicode)'$' );
            aName.append( ::rtl::OUString::createFromAscii(
                                    TOOLS_INETDEF_OS ).replace( ' ', '_' ) );

            aName.append( (sal_Unicode)' ' );
        }
    }

    // second product: OpenOffice.org_project/<build_information>
    // build_information has '(' and '[' encoded as '$', ')' and ']' ignored
    // and ':' replaced by '-'
    {
        aName.appendAscii( sOpenOfficeOrgProject );
        aName.append( (sal_Unicode)'/' );
        ::rtl::OUString aDefault;
        ::rtl::OUString aBuildId( utl::Bootstrap::getBuildIdData( aDefault ) );
        for( sal_Int32 i=0; i < aBuildId.getLength(); i++ )
        {
            sal_Unicode c = aBuildId[i];
            switch( c )
            {
            case '(':
            case '[':
                aName.append( (sal_Unicode)'$' );
                break;
            case ')':
            case ']':
                break;
            case ':':
                aName.append( (sal_Unicode)'-' );
                break;
            default:
                aName.append( c );
                break;
            }
        }
    }


    return aName.makeStringAndClear();
}

void SfxXMLMetaExport::Export()
{
    if ( !xInfoProp.is() ) return;

    rtl::OUString sElem, sSubElem, sAttrName, sValue;
    uno::Any aPropVal;

    //  generator
    // first token: real product name + version
    sValue = lcl_GetProductName();
    {
        SvXMLElementExport aElem( rExport, XML_NAMESPACE_META, XML_GENERATOR,
                                  sal_True, sal_True );
        rExport.Characters( sValue );
    }

    //  document title
    SimpleStringElement( ::rtl::OUString::createFromAscii(PROP_TITLE),
                         XML_NAMESPACE_DC, XML_TITLE );

    //  description
    SimpleStringElement( ::rtl::OUString::createFromAscii(PROP_DESCRIPTION),
                         XML_NAMESPACE_DC, XML_DESCRIPTION );

    //  subject
    SimpleStringElement( ::rtl::OUString::createFromAscii(PROP_THEME),
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
        sValue = ConvertLanguageToIsoString( eLanguage, '-' );
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
    sal_Int32 nCycles;
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
    sal_Int32 nDurVal;
    if ( aPropVal >>= nDurVal )
    {
        Time aDurTime( nDurVal );
        sValue = GetISODurationString( aDurTime );

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
        sal_Int32 nSecs;
        if ( aPropVal >>= nSecs )
        {
            Time aTime;
            aTime.MakeTimeFromMS( nSecs * 1000 );
            rtl::OUString sReloadTime = GetISODurationString( aTime );

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
        rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ROLE, XML_TEMPLATE );

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
}



