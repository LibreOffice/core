/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <tools/debug.hxx>
#include <tools/time.hxx>
#include <tools/string.hxx>

#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>

#include "xmlmetai.hxx"
#include "xmlnmspe.hxx"
#include "xmlimp.hxx"
#include "nmspmap.hxx"

namespace binfilter {

using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

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

class SfxXMLMetaElementContext : public SvXMLImportContext
{
private:
    SfxXMLMetaContext&  rParent;
    sal_uInt16          nElementType;
    ::rtl::OUString       sContent;
    ::rtl::OUString       sFieldName;     // for <meta:user-defined>

public:
    SfxXMLMetaElementContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                const ::rtl::OUString& rLName,
                                const uno::Reference<
                                    xml::sax::XAttributeList>& xAttrList,
                                SfxXMLMetaContext& rParentContext,
                                sal_uInt16 nType );

    virtual ~SfxXMLMetaElementContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                 const ::rtl::OUString& rLName,
                                 const uno::Reference<
                                    xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
    virtual void Characters( const ::rtl::OUString& rChars );

    static BOOL ParseISODateTimeString( const ::rtl::OUString& rString,
                                        util::DateTime& rDateTime );
    static BOOL ParseISODurationString( const ::rtl::OUString& rString,
                                        Time& rTime );
};

//-------------------------------------------------------------------------

enum SfxXMLMetaElemTokens
{
    XML_TOK_META_TITLE,
    XML_TOK_META_DESCRIPTION,
    XML_TOK_META_SUBJECT,
    XML_TOK_META_KEYWORDS,
    XML_TOK_META_KEYWORD,
    XML_TOK_META_INITIALCREATOR,
    XML_TOK_META_CREATIONDATE,
    XML_TOK_META_CREATOR,
    XML_TOK_META_DATE,
    XML_TOK_META_PRINTEDBY,
    XML_TOK_META_PRINTDATE,
    XML_TOK_META_TEMPLATE,
    XML_TOK_META_AUTORELOAD,
    XML_TOK_META_HYPERLINKBEHAVIOUR,
    XML_TOK_META_LANGUAGE,
    XML_TOK_META_EDITINGCYCLES,
    XML_TOK_META_EDITINGDURATION,
    XML_TOK_META_USERDEFINED,
    XML_TOK_META_DOCUMENT_STATISTIC,
    XML_TOK_META_ELEM_END = XML_TOK_UNKNOWN
};

// XML_TOK_META_KEYWORD is not in map,
// handled in SfxXMLMetaElementContext::CreateChildContext

static __FAR_DATA SvXMLTokenMapEntry aMetaElemTokenMap[] =
{
    { XML_NAMESPACE_DC,     XML_TITLE,             XML_TOK_META_TITLE },
    { XML_NAMESPACE_DC,     XML_DESCRIPTION,       XML_TOK_META_DESCRIPTION },
    { XML_NAMESPACE_DC,     XML_SUBJECT,           XML_TOK_META_SUBJECT },
    { XML_NAMESPACE_META,   XML_KEYWORDS,          XML_TOK_META_KEYWORDS },
    { XML_NAMESPACE_META,   XML_INITIAL_CREATOR,   XML_TOK_META_INITIALCREATOR },
    { XML_NAMESPACE_META,   XML_CREATION_DATE,     XML_TOK_META_CREATIONDATE },
    { XML_NAMESPACE_DC,     XML_CREATOR,           XML_TOK_META_CREATOR },
    { XML_NAMESPACE_DC,     XML_DATE,              XML_TOK_META_DATE },
    { XML_NAMESPACE_META,   XML_PRINTED_BY,        XML_TOK_META_PRINTEDBY },
    { XML_NAMESPACE_META,   XML_PRINT_DATE,        XML_TOK_META_PRINTDATE },
    { XML_NAMESPACE_META,   XML_TEMPLATE,          XML_TOK_META_TEMPLATE },
    { XML_NAMESPACE_META,   XML_AUTO_RELOAD,       XML_TOK_META_AUTORELOAD },
    { XML_NAMESPACE_META,   XML_HYPERLINK_BEHAVIOUR,XML_TOK_META_HYPERLINKBEHAVIOUR },
    { XML_NAMESPACE_DC,     XML_LANGUAGE,          XML_TOK_META_LANGUAGE },
    { XML_NAMESPACE_META,   XML_EDITING_CYCLES,    XML_TOK_META_EDITINGCYCLES },
    { XML_NAMESPACE_META,   XML_EDITING_DURATION,  XML_TOK_META_EDITINGDURATION },
    { XML_NAMESPACE_META,   XML_USER_DEFINED,      XML_TOK_META_USERDEFINED },
    { XML_NAMESPACE_META,   XML_DOCUMENT_STATISTIC,XML_TOK_META_DOCUMENT_STATISTIC },
    XML_TOKEN_MAP_END
};

enum SfxXMLMetaTemplateTokens
{
    XML_TOK_META_TEMPLATE_HREF,
    XML_TOK_META_TEMPLATE_TITLE,
    XML_TOK_META_TEMPLATE_DATE,
    XML_TOK_META_TEMPLATE_END = XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aMetaTemplateTokenMap[] =
{
    { XML_NAMESPACE_XLINK,  XML_HREF,              XML_TOK_META_TEMPLATE_HREF },
    { XML_NAMESPACE_XLINK,  XML_TITLE,             XML_TOK_META_TEMPLATE_TITLE },
    { XML_NAMESPACE_META,   XML_DATE,              XML_TOK_META_TEMPLATE_DATE },
    XML_TOKEN_MAP_END
};

enum SfxXMLMetaReloadTokens
{
    XML_TOK_META_RELOAD_HREF,
    XML_TOK_META_RELOAD_DELAY,
    XML_TOK_META_RELOAD_END = XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aMetaReloadTokenMap[] =
{
    { XML_NAMESPACE_XLINK,  XML_HREF,              XML_TOK_META_RELOAD_HREF },
    { XML_NAMESPACE_META,   XML_DELAY,             XML_TOK_META_RELOAD_DELAY },
    XML_TOKEN_MAP_END
};

//-------------------------------------------------------------------------

sal_Bool lcl_GetNumber( const ::rtl::OUString& rString, sal_Int32& rValue,
                        sal_Int32 nMax )
{
    sal_Int32 nTemp = 0;

    ::rtl::OUString aTrimmed = rString.trim();
    sal_Int32 nLen = aTrimmed.getLength();
    sal_Int32 nPos = 0;
    sal_Unicode c;
    while( nPos < nLen && sal_Unicode('0') <= (c = aTrimmed[nPos]) &&
           sal_Unicode('9') >= c )
    {
        nTemp *= 10;
        nTemp += (c - sal_Unicode('0'));
        if ( nTemp > nMax )
            return sal_False;

        nPos++;
    }

//  if ( *pStr )
//      return FALSE;       // invalid characters

    rValue = nTemp;
    return sal_True;
}

// static
sal_Bool SfxXMLMetaElementContext::ParseISODateTimeString(
                                const ::rtl::OUString& rString,
                                util::DateTime& rDateTime )
{
    sal_Bool bSuccess = sal_True;

    ::rtl::OUString aDateStr, aTimeStr;
    sal_Int32 nPos = rString.indexOf( (sal_Unicode) 'T' );
    if ( nPos >= 0 )
    {
        aDateStr = rString.copy( 0, nPos );
        aTimeStr = rString.copy( nPos + 1 );
    }
    else
        aDateStr = rString;         // no separator: only date part

    sal_Int32 nYear  = 0;
    sal_Int32 nMonth = 1;
    sal_Int32 nDay   = 1;
    sal_Int32 nHour  = 0;
    sal_Int32 nMin   = 0;
    sal_Int32 nSec   = 0;

    const sal_Unicode* pStr = aDateStr.getStr();
    sal_Int32 nDateTokens = 1;
    while ( *pStr )
    {
        if ( *pStr == '-' )
            nDateTokens++;
        pStr++;
    }
    if ( nDateTokens > 3 || aDateStr.getLength() == 0 )
        bSuccess = sal_False;
    else
    {
        sal_Int32 n = 0;
        if ( !lcl_GetNumber( aDateStr.getToken( 0, '-', n ), nYear, 9999 ) )
            bSuccess = sal_False;
        if ( nDateTokens >= 2 )
            if ( !lcl_GetNumber( aDateStr.getToken( 0, '-', n ), nMonth, 12 ) )
                bSuccess = sal_False;
        if ( nDateTokens >= 3 )
            if ( !lcl_GetNumber( aDateStr.getToken( 0, '-', n ), nDay, 31 ) )
                bSuccess = sal_False;
    }

    if ( aTimeStr.getLength() > 0 )         // time is optional
    {
        pStr = aTimeStr.getStr();
        sal_Int32 nTimeTokens = 1;
        while ( *pStr )
        {
            if ( *pStr == ':' )
                nTimeTokens++;
            pStr++;
        }
        if ( nTimeTokens > 3 )
            bSuccess = sal_False;
        else
        {
            sal_Int32 n = 0;
            if ( !lcl_GetNumber( aTimeStr.getToken( 0, ':', n ), nHour, 23 ) )
                bSuccess = sal_False;
            if ( nTimeTokens >= 2 )
                if ( !lcl_GetNumber( aTimeStr.getToken( 0, ':', n ), nMin, 59 ) )
                    bSuccess = sal_False;
            if ( nTimeTokens >= 3 )
                if ( !lcl_GetNumber( aTimeStr.getToken( 0, ':', n ), nSec, 59 ) )
                    bSuccess = sal_False;
        }
    }

    if (bSuccess)
        rDateTime = util::DateTime( 0, nSec, nMin, nHour, nDay, nMonth, nYear );
    return bSuccess;
}

// static
sal_Bool SfxXMLMetaElementContext::ParseISODurationString(
                            const ::rtl::OUString& rString, Time& rTime )
{
    ::rtl::OUString aTrimmed = rString.trim().toAsciiUpperCase();
    const sal_Unicode* pStr = aTrimmed.getStr();

    if ( *(pStr++) != sal_Unicode('P') )            // duration must start with "P"
        return sal_False;

    sal_Bool bSuccess = TRUE;
    sal_Bool bDone = sal_False;
    sal_Bool bTimePart = sal_False;
    sal_Int32 nDays  = 0;
    sal_Int32 nHours = 0;
    sal_Int32 nMins  = 0;
    sal_Int32 nSecs  = 0;
    sal_Int32 nTemp = 0;

    while ( bSuccess && !bDone )
    {
        sal_Unicode c = *(pStr++);
        if ( !c )                               // end
            bDone = sal_True;
        else if ( sal_Unicode('0') <= c && sal_Unicode('9') >= c )
        {
            if ( nTemp >= SAL_MAX_INT32 / 10 )
                bSuccess = sal_False;
            else
            {
                nTemp *= 10;
                nTemp += (c - sal_Unicode('0'));
            }
        }
        else if ( bTimePart )
        {
            if ( c == sal_Unicode('H') )
            {
                nHours = nTemp;
                nTemp = 0;
            }
            else if ( c == sal_Unicode('M') )
            {
                nMins = nTemp;
                nTemp = 0;
            }
            else if ( c == sal_Unicode('S') )
            {
                nSecs = nTemp;
                nTemp = 0;
            }
            else
                bSuccess = sal_False;               // invalid characted
        }
        else
        {
            if ( c == sal_Unicode('T') )            // "T" starts time part
                bTimePart = TRUE;
            else if ( c == sal_Unicode('D') )
            {
                nDays = nTemp;
                nTemp = 0;
            }
            else if ( c == sal_Unicode('Y') || c == sal_Unicode('M') )
            {
                //! how many days is a year or month?

                DBG_ERROR("years or months in duration: not implemented");
                bSuccess = sal_False;
            }
            else
                bSuccess = sal_False;               // invalid characted
        }
    }

    if ( bSuccess )
    {
        if ( nDays )
            nHours += nDays * 24;               // add the days to the hours part
        rTime = Time( nHours, nMins, nSecs );
    }
    return bSuccess;
}

//-------------------------------------------------------------------------

SfxXMLMetaElementContext::SfxXMLMetaElementContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                    const ::rtl::OUString& rLName,
                                    const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                    SfxXMLMetaContext& rParentContext, sal_uInt16 nType ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rParent( rParentContext ),
    nElementType( nType )
{
    rParent.AddRef();
    uno::Any aPropAny;

    if ( nElementType == XML_TOK_META_TEMPLATE )
    {
        //  <meta:template>: everything is in the attributes

        uno::Reference<beans::XPropertySet> xInfoProp = rParent.GetInfoProp();
        if ( xInfoProp.is() )
        {
            sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
            for( sal_Int16 i=0; i < nAttrCount; i++ )
            {
                ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
                ::rtl::OUString aLocalName;
                sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName(
                                                    sAttrName, &aLocalName );
                ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

                SvXMLTokenMap aTokenMap( aMetaTemplateTokenMap );
                switch( aTokenMap.Get( nPrefix, aLocalName ) )
                {
                case XML_TOK_META_TEMPLATE_HREF:
                    aPropAny <<= GetImport().GetAbsoluteReference(sValue);
                    xInfoProp->setPropertyValue(
                        ::rtl::OUString::createFromAscii(PROP_TEMPLATEURL),
                        aPropAny );
                    break;
                case XML_TOK_META_TEMPLATE_TITLE:
                    aPropAny <<= sValue;
                    xInfoProp->setPropertyValue(
                        ::rtl::OUString::createFromAscii(PROP_TEMPLATENAME),
                        aPropAny );
                    break;
                case XML_TOK_META_TEMPLATE_DATE:
                    {
                        util::DateTime aDateTime;
                        if ( ParseISODateTimeString( sValue, aDateTime ) )
                        {
                            aPropAny <<= aDateTime;
                            xInfoProp->setPropertyValue(
                                ::rtl::OUString::createFromAscii(
                                            PROP_TEMPLATEDATE), aPropAny );
                        }
                    }
                    break;
                }
            }
        }
    }
    else if ( nElementType == XML_TOK_META_AUTORELOAD )
    {
        //  <meta:auto-reload>: everything is in the attributes

        uno::Reference<beans::XPropertySet> xInfoProp = rParent.GetInfoProp();
        if ( xInfoProp.is() )
        {
            //  AutoloadEnabled is implicit
            sal_Bool bReload = sal_True;
            aPropAny.setValue( &bReload, getBooleanCppuType() );
            xInfoProp->setPropertyValue(
                    ::rtl::OUString::createFromAscii(PROP_RELOADENABLED),
                    aPropAny );

            sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
            for( sal_Int16 i=0; i < nAttrCount; i++ )
            {
                ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
                ::rtl::OUString aLocalName;
                sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName(
                                                    sAttrName, &aLocalName );
                ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

                SvXMLTokenMap aTokenMap( aMetaReloadTokenMap );
                uno::Any aAny;
                switch( aTokenMap.Get( nPrefix, aLocalName ) )
                {
                    case XML_TOK_META_RELOAD_HREF:
                        aPropAny <<= GetImport().GetAbsoluteReference(sValue);
                        xInfoProp->setPropertyValue(
                            ::rtl::OUString::createFromAscii(PROP_RELOADURL),
                            aPropAny );
                        break;
                    case XML_TOK_META_RELOAD_DELAY:
                        {
                            Time aTime;
                            if ( ParseISODurationString( sValue, aTime ) )
                            {
                                sal_Int32 nSecs = aTime.GetMSFromTime() / 1000;
                                aPropAny <<= nSecs;
                                xInfoProp->setPropertyValue(
                                    ::rtl::OUString::createFromAscii(
                                                PROP_RELOADSECS), aPropAny );
                            }
                        }
                        break;
                }
            }
        }
    }
    else if ( nElementType == XML_TOK_META_HYPERLINKBEHAVIOUR )
    {
        //  <meta:hyperlink-behaviour>: everything is in the attributes

        uno::Reference<beans::XPropertySet> xInfoProp = rParent.GetInfoProp();
        if ( xInfoProp.is() )
        {
            sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
            for( sal_Int16 i=0; i < nAttrCount; i++ )
            {
                ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
                ::rtl::OUString aLocalName;
                sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName(
                                                    sAttrName, &aLocalName );
                if ( nPrefix == XML_NAMESPACE_OFFICE &&
                     IsXMLToken(aLocalName, XML_TARGET_FRAME_NAME) )
                {
                    ::rtl::OUString sValue = xAttrList->getValueByIndex( i );
                    aPropAny <<= sValue;
                    xInfoProp->setPropertyValue(
                        ::rtl::OUString::createFromAscii(PROP_DEFAULTTARGET),
                        aPropAny );
                }
            }
        }
    }
    else if ( nElementType == XML_TOK_META_USERDEFINED )
    {
        //  <meta:user-defined>: get field name from attributes

        INT16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for( INT16 i=0; i < nAttrCount; i++ )
        {
            ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
            ::rtl::OUString aLocalName;
            sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName(
                                                sAttrName, &aLocalName );
            if ( nPrefix == XML_NAMESPACE_META &&
                 IsXMLToken(aLocalName, XML_NAME) )
            {
                sFieldName = xAttrList->getValueByIndex( i );
            }
        }
    }
    else if ( nElementType == XML_TOK_META_DOCUMENT_STATISTIC )
        GetImport().SetStatisticAttributes(xAttrList);
}

SfxXMLMetaElementContext::~SfxXMLMetaElementContext()
{
    rParent.ReleaseRef();
}

SvXMLImportContext* SfxXMLMetaElementContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLName,
                                     const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    if ( nElementType == XML_TOK_META_KEYWORDS &&
         nPrefix == XML_NAMESPACE_META &&
         IsXMLToken(rLName, XML_KEYWORD) )
    {
        //  <office:keyword> inside of <office:keywords>
        pContext = new SfxXMLMetaElementContext( GetImport(), nPrefix, rLName,
                                    xAttrList, rParent, XML_TOK_META_KEYWORD );
    }

    if ( !pContext )
    {
        //  default context to ignore unknown elements
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );
    }
    return pContext;
}

void SfxXMLMetaElementContext::EndElement()
{
    uno::Reference<beans::XPropertySet> xInfoProp = rParent.GetInfoProp();
    if ( !xInfoProp.is() )
        return;

    uno::Any aPropAny;
    util::DateTime aDateTime;
    Time aTime;
    sal_Int32 nValue;
    switch ( nElementType )
    {
        // simple strings
        case XML_TOK_META_TITLE:
            aPropAny <<= sContent;
            xInfoProp->setPropertyValue( ::rtl::OUString::createFromAscii(PROP_TITLE), aPropAny );
            break;
        case XML_TOK_META_DESCRIPTION:
            aPropAny <<= sContent;
            xInfoProp->setPropertyValue( ::rtl::OUString::createFromAscii(PROP_DESCRIPTION), aPropAny );
            break;
        case XML_TOK_META_SUBJECT:
            aPropAny <<= sContent;
            xInfoProp->setPropertyValue( ::rtl::OUString::createFromAscii(PROP_THEME), aPropAny );
            break;
        case XML_TOK_META_INITIALCREATOR:
            aPropAny <<= sContent;
            xInfoProp->setPropertyValue( ::rtl::OUString::createFromAscii(PROP_AUTHOR), aPropAny );
            break;
        case XML_TOK_META_CREATOR:
            aPropAny <<= sContent;
            xInfoProp->setPropertyValue( ::rtl::OUString::createFromAscii(PROP_MODIFIEDBY), aPropAny );
            break;
        case XML_TOK_META_PRINTEDBY:
            aPropAny <<= sContent;
            xInfoProp->setPropertyValue( ::rtl::OUString::createFromAscii(PROP_PRINTEDBY), aPropAny );
            break;
        // date/time
        case XML_TOK_META_CREATIONDATE:
            if ( ParseISODateTimeString( sContent, aDateTime ) )
            {
                aPropAny <<= aDateTime;
                xInfoProp->setPropertyValue( ::rtl::OUString::createFromAscii(PROP_CREATIONDATE), aPropAny );
            }
            break;
        case XML_TOK_META_DATE:
            if ( ParseISODateTimeString( sContent, aDateTime ) )
            {
                aPropAny <<= aDateTime;
                xInfoProp->setPropertyValue( ::rtl::OUString::createFromAscii(PROP_MODIFYDATE), aPropAny );
            }
            break;
        case XML_TOK_META_PRINTDATE:
            if ( ParseISODateTimeString( sContent, aDateTime ) )
            {
                aPropAny <<= aDateTime;
                xInfoProp->setPropertyValue( ::rtl::OUString::createFromAscii(PROP_PRINTDATE), aPropAny );
            }
            break;
        // others
        case XML_TOK_META_LANGUAGE:
            {
                uno::Reference<beans::XPropertySet> xDocProp = rParent.GetDocProp();
                if ( xDocProp.is() )
                {
                    //  set document language in document properties
                    //  (not available for all document types)

                    lang::Locale aLocale;
                    String aString( sContent );
                    xub_StrLen  nSepPos = aString.Search( (sal_Unicode)'-' );
                    if ( nSepPos != STRING_NOTFOUND )
                    {
                        aLocale.Language = aString.Copy( 0, nSepPos );
                        aLocale.Country = aString.Copy( nSepPos+1 );
                    }
                    else
                        aLocale.Language = aString;

                    aPropAny <<= aLocale;
                    try
                    {
                        xDocProp->setPropertyValue(
                            ::rtl::OUString::createFromAscii( PROP_CHARLOCALE ),
                            aPropAny );
                    }
                    catch (beans::UnknownPropertyException&)
                    {
                        // no error
                    }
                }
            }
            break;
        case XML_TOK_META_EDITINGCYCLES:
            if ( lcl_GetNumber( sContent, nValue, USHRT_MAX ) )
            {
                aPropAny <<= (sal_Int16) nValue;
                xInfoProp->setPropertyValue( ::rtl::OUString::createFromAscii(PROP_EDITINGCYCLES), aPropAny );
            }
            break;
        case XML_TOK_META_EDITINGDURATION:
            //  property is a int32 with the Time::GetTime value
            if ( ParseISODurationString( sContent, aTime ) )
            {
                aPropAny <<= (sal_Int32) aTime.GetTime();
                xInfoProp->setPropertyValue( ::rtl::OUString::createFromAscii(PROP_EDITINGDURATION), aPropAny );
            }
            break;
        case XML_TOK_META_KEYWORDS:
            //  nothing to do, keywords are handled in XML_TOK_META_KEYWORD
            break;
        case XML_TOK_META_TEMPLATE:
        case XML_TOK_META_AUTORELOAD:
        case XML_TOK_META_HYPERLINKBEHAVIOUR:
            //  nothing to do, everything is handled in attributes
            break;
        case XML_TOK_META_KEYWORD:
            rParent.AddKeyword( sContent );
            break;
        case XML_TOK_META_USERDEFINED:
            rParent.AddUserField( sFieldName, sContent );
            break;
        case XML_TOK_META_DOCUMENT_STATISTIC:
            break;
        default:
            DBG_ERROR("wrong element");
    }
}

void SfxXMLMetaElementContext::Characters( const ::rtl::OUString& rChars )
{
    sContent += rChars;
}

//-------------------------------------------------------------------------
//
//  context for <office:meta> element
//

SfxXMLMetaContext::SfxXMLMetaContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                    const ::rtl::OUString& rLName,
                                    const uno::Reference<frame::XModel>& rDocModel ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xDocProp( rDocModel, uno::UNO_QUERY ),
    pTokenMap( NULL ),
    nUserKeys( 0 )
{
    uno::Reference<document::XDocumentInfoSupplier> xSupp( rDocModel, uno::UNO_QUERY );
    if ( xSupp.is() )
    {
        xDocInfo = xSupp->getDocumentInfo();
        xInfoProp = uno::Reference<beans::XPropertySet>( xDocInfo, uno::UNO_QUERY );
    }
#ifndef SVX_LIGHT
    // this service is not available for player
    DBG_ASSERT( xInfoProp.is(), "no document info properties" );
#endif
}

SfxXMLMetaContext::SfxXMLMetaContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                    const ::rtl::OUString& rLName,
                                    const uno::Reference<document::XDocumentInfo>&	rDocInfo ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xDocInfo( rDocInfo ),
    xInfoProp( rDocInfo, uno::UNO_QUERY ),
    pTokenMap ( NULL ),
    nUserKeys ( 0 )
{
#ifndef SVX_LIGHT
    // this service is not available for player
    DBG_ASSERT( xInfoProp.is(), "no document info properties" );
#endif
}

SfxXMLMetaContext::~SfxXMLMetaContext()
{
    delete pTokenMap;
}

SvXMLImportContext* SfxXMLMetaContext::CreateChildContext( sal_uInt16 nPrefix,
                                    const ::rtl::OUString& rLName,
                                    const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    if (!pTokenMap)
        pTokenMap = new SvXMLTokenMap( aMetaElemTokenMap );

    sal_uInt16 nToken = pTokenMap->Get( nPrefix, rLName );
    if ( nToken != XML_TOK_UNKNOWN )
        pContext = new SfxXMLMetaElementContext( GetImport(),
                                nPrefix, rLName, xAttrList, *this, nToken );

    if ( !pContext )
    {
        //  default context to ignore unknown elements
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );
    }
    return pContext;
}

void SfxXMLMetaContext::EndElement()
{
    if ( sKeywords.getLength() && xInfoProp.is() )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString(sKeywords);
        xInfoProp->setPropertyValue( ::rtl::OUString::createFromAscii(PROP_KEYWORDS), aAny );
    }
}

void SfxXMLMetaContext::AddKeyword( const ::rtl::OUString& rKW )
{
    if ( sKeywords.getLength() )
    {
        // comma separated
        sKeywords.append(sal_Unicode(','));
        sKeywords.append(sal_Unicode(' '));
    }
    sKeywords.append( rKW );
}

void SfxXMLMetaContext::AddUserField( const ::rtl::OUString& rName, const ::rtl::OUString& rContent )
{
    if ( xDocInfo.is() && nUserKeys < xDocInfo->getUserFieldCount() )
    {
        //  keep default name if none is there
        if ( rName.getLength() )
            xDocInfo->setUserFieldName( nUserKeys, rName );
        xDocInfo->setUserFieldValue( nUserKeys, rContent );
        ++nUserKeys;
    }
}


}//end of namespace binfilter
