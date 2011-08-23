/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SVSTDARR_LONGS_DECL
#define _SVSTDARR_LONGS
#include <bf_svtools/svstdarr.hxx>
#endif


#include <vector>



#include <com/sun/star/container/XEnumerationAccess.hpp>


#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>

#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>

#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>


#include "txtexppr.hxx"
#include "xmluconv.hxx"
#include "XMLAnchorTypePropHdl.hxx"
#include "xexptran.hxx"

#include "nmspmap.hxx"
#include "xmlexp.hxx"
#include "txtflde.hxx"
#include "txtprmap.hxx"
#include "XMLImageMapExport.hxx"

#include "XMLTextNumRuleInfo.hxx"
#include "XMLTextListAutoStylePool.hxx"
#include "XMLSectionExport.hxx"
#include "XMLIndexMarkExport.hxx"
#include "XMLEventExport.hxx"
#include "XMLRedlineExport.hxx"
#include "MultiPropertySetHelper.hxx"
#include "XMLTextCharStyleNamesElementExport.hxx"
namespace binfilter {


using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::frame;
using namespace ::binfilter::xmloff::token;

using rtl::OUString;
using rtl::OUStringBuffer;


typedef OUString *OUStringPtr;
SV_DECL_PTRARR_DEL( OUStrings_Impl, OUStringPtr, 20, 10 )
SV_IMPL_PTRARR( OUStrings_Impl, OUStringPtr )

SV_DECL_PTRARR_SORT_DEL( OUStringsSort_Impl, OUStringPtr, 20, 10 )
SV_IMPL_OP_PTRARR_SORT( OUStringsSort_Impl, OUStringPtr )

#ifdef DBG_UTIL
static int txtparae_bContainsIllegalCharacters = sal_False;
#endif

// The following map shows which property values are required:
//
// property						auto style pass		export
// --------------------------------------------------------
// ParaStyleName				if style exists		always
// ParaConditionalStyleName		if style exists		always
// NumberingRules				if style exists		always
// TextSection					always				always
// ParaChapterNumberingLevel	never				always

// The conclusion is that for auto styles the first three properties
// should be queried using a multi property set if, and only if, an
// auto style needs to be exported. TextSection should be queried by
// an individual call to getPropertyvalue, because this seems to be
// less expensive than querying the first three properties if they aren't
// required.

// For the export pass all properties can be queried using a multi property
// set.

static const sal_Char* aParagraphPropertyNamesAuto[] =
{
    "NumberingRules",
    "ParaConditionalStyleName",
    "ParaStyleName",
    NULL
};

enum eParagraphPropertyNamesEnumAuto
{
    NUMBERING_RULES_AUTO = 0,
    PARA_CONDITIONAL_STYLE_NAME_AUTO = 1,
    PARA_STYLE_NAME_AUTO = 2
};

static const sal_Char* aParagraphPropertyNames[] =
{
    "ParaChapterNumberingLevel",
    "ParaConditionalStyleName",
    "ParaStyleName",
    "TextSection",
    NULL
};

enum eParagraphPropertyNamesEnum
{
    PARA_CHAPTER_NUMERBING_LEVEL = 0,
    PARA_CONDITIONAL_STYLE_NAME = 1,
    PARA_STYLE_NAME = 2,
    TEXT_SECTION = 3
};



void XMLTextParagraphExport::Add( sal_uInt16 nFamily,
                                  const Reference < XPropertySet > & rPropSet,
                                  const XMLPropertyState** ppAddStates)
{
    UniReference < SvXMLExportPropertyMapper > xPropMapper;
    switch( nFamily )
    {
    case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
        xPropMapper = GetParaPropMapper();
        break;
    case XML_STYLE_FAMILY_TEXT_TEXT:
        xPropMapper = GetTextPropMapper();
        break;
    case XML_STYLE_FAMILY_TEXT_FRAME:
        xPropMapper = GetAutoFramePropMapper();
        break;
    case XML_STYLE_FAMILY_TEXT_SECTION:
        xPropMapper = GetSectionPropMapper();
        break;
    case XML_STYLE_FAMILY_TEXT_RUBY:
        xPropMapper = GetRubyPropMapper();
        break;
    }
    DBG_ASSERT( xPropMapper.is(), "There is the property mapper?" );

    vector< XMLPropertyState > xPropStates =
            xPropMapper->Filter( rPropSet );
    if( ppAddStates )
    {
        while( *ppAddStates )
        {
            xPropStates.push_back( **ppAddStates );
            ppAddStates++;
        }
    }

    if( xPropStates.size() > 0L )
    {
        Reference< XPropertySetInfo > xPropSetInfo =
            rPropSet->getPropertySetInfo();
        OUString sParent, sCondParent;
        Any aAny;
        sal_uInt16 nIgnoreProps = 0;
        switch( nFamily )
        {
        case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
            if( xPropSetInfo->hasPropertyByName( sParaStyleName ) )
            {
                aAny = rPropSet->getPropertyValue( sParaStyleName );
                aAny >>= sParent;
            }
            if( xPropSetInfo->hasPropertyByName( sParaConditionalStyleName ) )
            {
                aAny = rPropSet->getPropertyValue( sParaConditionalStyleName );
                aAny >>= sCondParent;
            }
            if( xPropSetInfo->hasPropertyByName( sNumberingRules ) )
            {
                aAny = rPropSet->getPropertyValue( sNumberingRules );
                Reference < XIndexReplace > xNumRule;
                aAny >>= xNumRule;
                if( xNumRule.is() && xNumRule->getCount() )
                {
                    Reference < XNamed > xNamed( xNumRule, UNO_QUERY );
                    OUString sName;
                    if( xNamed.is() )
                        sName = xNamed->getName();
                    sal_Bool bAdd = !sName.getLength();
                    if( !bAdd )
                    {
                        Reference < XPropertySet > xNumPropSet( xNumRule,
                                                                UNO_QUERY );
                        OUString sIsAutomatic( RTL_CONSTASCII_USTRINGPARAM( "IsAutomatic" ) );
                        if( xNumPropSet.is() &&
                            xNumPropSet->getPropertySetInfo()
                                       ->hasPropertyByName( sIsAutomatic ) )
                        {
                            aAny = xNumPropSet->getPropertyValue( sIsAutomatic );
                            bAdd = *(sal_Bool *)aAny.getValue();
                        }
                        else
                        {
                            bAdd = sal_True;
                        }
                    }
                    if( bAdd )
                        pListAutoPool->Add( xNumRule );
                }
            }
            break;
        case XML_STYLE_FAMILY_TEXT_TEXT:
            {
                // Get parent and remove hyperlinks (they aren't of interest)
                UniReference< XMLPropertySetMapper > xPM =
                    xPropMapper->getPropertySetMapper();
                for( ::std::vector< XMLPropertyState >::iterator i
                           = xPropStates.begin();
                      nIgnoreProps < 2 && i != xPropStates.end();
                      i++ )
                {
                    switch( xPM->GetEntryContextId(i->mnIndex) )
                    {
                    case CTF_CHAR_STYLE_NAME:
                        i->maValue >>= sParent;
                        i->mnIndex = -1;
                        nIgnoreProps++;
                        break;
                    case CTF_HYPERLINK_URL:
                        i->mnIndex = -1;
                        nIgnoreProps++;
                        break;
                    }
                }
            }
            break;
        case XML_STYLE_FAMILY_TEXT_FRAME:
            if( xPropSetInfo->hasPropertyByName( sFrameStyleName ) )
            {
                aAny = rPropSet->getPropertyValue( sFrameStyleName );
                aAny >>= sParent;
            }
            break;
        case XML_STYLE_FAMILY_TEXT_SECTION:
        case XML_STYLE_FAMILY_TEXT_RUBY:
            ; // section styles have no parents
            break;
        }
        if( (xPropStates.size() - nIgnoreProps) > 0 )
        {
            GetAutoStylePool().Add( nFamily, sParent, xPropStates );
            if( sCondParent.getLength() && sParent != sCondParent )
                GetAutoStylePool().Add( nFamily, sCondParent, xPropStates );
        }
    }
}

void XMLTextParagraphExport::Add( sal_uInt16 nFamily,
                                  MultiPropertySetHelper& rPropSetHelper,
                                  const Reference < XPropertySet > & rPropSet,
                                  const XMLPropertyState** ppAddStates)
{
    UniReference < SvXMLExportPropertyMapper > xPropMapper;
    switch( nFamily )
    {
    case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
        xPropMapper = GetParaPropMapper();
        break;
    }
    DBG_ASSERT( xPropMapper.is(), "There is the property mapper?" );

    vector< XMLPropertyState > xPropStates =
            xPropMapper->Filter( rPropSet );
    if( ppAddStates )
    {
        while( *ppAddStates )
        {
            xPropStates.push_back( **ppAddStates );
            ppAddStates++;
        }
    }

    if( xPropStates.size() > 0L )
    {
        OUString sParent, sCondParent;
        Any aAny;
        switch( nFamily )
        {
        case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
            if( rPropSetHelper.hasProperty( PARA_STYLE_NAME_AUTO ) )
            {
                aAny = rPropSetHelper.getValue( PARA_STYLE_NAME_AUTO, rPropSet,
                                                sal_True );
                aAny >>= sParent;
            }
            if( rPropSetHelper.hasProperty( PARA_CONDITIONAL_STYLE_NAME_AUTO ) )
            {
                aAny = rPropSetHelper.getValue( PARA_CONDITIONAL_STYLE_NAME_AUTO,
                                                 rPropSet, sal_True );
                aAny >>= sCondParent;
            }
            if( rPropSetHelper.hasProperty( NUMBERING_RULES_AUTO ) )
            {
                aAny = rPropSetHelper.getValue( NUMBERING_RULES_AUTO,
                                                 rPropSet, sal_True );
                Reference < XIndexReplace > xNumRule;
                aAny >>= xNumRule;
                if( xNumRule.is() && xNumRule->getCount() )
                {
                    Reference < XNamed > xNamed( xNumRule, UNO_QUERY );
                    OUString sName;
                    if( xNamed.is() )
                        sName = xNamed->getName();
                    sal_Bool bAdd = !sName.getLength();
                    if( !bAdd )
                    {
                        Reference < XPropertySet > xNumPropSet( xNumRule,
                                                                UNO_QUERY );
                        OUString sIsAutomatic( RTL_CONSTASCII_USTRINGPARAM( "IsAutomatic" ) );
                        if( xNumPropSet.is() &&
                            xNumPropSet->getPropertySetInfo()
                                       ->hasPropertyByName( sIsAutomatic ) )
                        {
                            aAny = xNumPropSet->getPropertyValue( sIsAutomatic );
                            bAdd = *(sal_Bool *)aAny.getValue();
                        }
                        else
                        {
                            bAdd = sal_True;
                        }
                    }
                    if( bAdd )
                        pListAutoPool->Add( xNumRule );
                }
            }
            break;
        }
        if( xPropStates.size() > 0 )
        {
            GetAutoStylePool().Add( nFamily, sParent, xPropStates );
            if( sCondParent.getLength() && sParent != sCondParent )
                GetAutoStylePool().Add( nFamily, sCondParent, xPropStates );
        }
    }
}

OUString XMLTextParagraphExport::Find(
        sal_uInt16 nFamily,
           const Reference < XPropertySet > & rPropSet,
        const OUString& rParent,
        const XMLPropertyState** ppAddStates) const
{
    OUString sName( rParent );
    UniReference < SvXMLExportPropertyMapper > xPropMapper;
    switch( nFamily )
    {
    case XML_STYLE_FAMILY_TEXT_PARAGRAPH:
        xPropMapper = GetParaPropMapper();
        break;
    case XML_STYLE_FAMILY_TEXT_FRAME:
        xPropMapper = GetAutoFramePropMapper();
        break;
    case XML_STYLE_FAMILY_TEXT_SECTION:
        xPropMapper = GetSectionPropMapper();
        break;
    case XML_STYLE_FAMILY_TEXT_RUBY:
        xPropMapper = GetRubyPropMapper();
        break;
    }
    DBG_ASSERT( xPropMapper.is(), "There is the property mapper?" );
    if( !xPropMapper.is() )
        return sName;
    vector< XMLPropertyState > xPropStates =
            xPropMapper->Filter( rPropSet );
    if( ppAddStates )
    {
        while( *ppAddStates )
        {
            xPropStates.push_back( **ppAddStates );
            ppAddStates++;
        }
    }

    if( xPropStates.size() > 0L )
        sName = GetAutoStylePool().Find( nFamily, sName, xPropStates );

    return sName;
}

OUString XMLTextParagraphExport::FindTextStyleAndHyperlink(
           const Reference < XPropertySet > & rPropSet,
        sal_Bool& rHyperlink,
        sal_Bool& rHasCharStyle,
        const XMLPropertyState** ppAddStates ) const
{
    UniReference < SvXMLExportPropertyMapper > xPropMapper
        = GetTextPropMapper();
    vector< XMLPropertyState > xPropStates =
            xPropMapper->Filter( rPropSet );

    // Get parent and remove hyperlinks (they aren't of interest)
    OUString sName;
    rHyperlink = rHasCharStyle = sal_False;
    sal_uInt16 nIgnoreProps = 0;
    UniReference< XMLPropertySetMapper > xPM =
        xPropMapper->getPropertySetMapper();
    for( ::std::vector< XMLPropertyState >::iterator
            i = xPropStates.begin();
         nIgnoreProps < 2 && i != xPropStates.end();
         i++ )
    {
        switch( xPM->GetEntryContextId(i->mnIndex) )
        {
        case CTF_CHAR_STYLE_NAME:
            i->maValue >>= sName;
            i->mnIndex = -1;
            rHasCharStyle = sName.getLength() > 0;
            nIgnoreProps++;
            break;
        case CTF_HYPERLINK_URL:
            rHyperlink = sal_True;
            i->mnIndex = -1;
            nIgnoreProps++;
            break;
        }
    }
    if( ppAddStates )
    {
        while( *ppAddStates )
        {
            xPropStates.push_back( **ppAddStates );
            ppAddStates++;
        }
    }
    if( (xPropStates.size() - nIgnoreProps) > 0L )
        sName = GetAutoStylePool().Find( XML_STYLE_FAMILY_TEXT_TEXT, sName, xPropStates );

    return sName;
}

OUString XMLTextParagraphExport::FindTextStyle(
           const Reference < XPropertySet > & rPropSet,
        sal_Bool& rHasCharStyle ) const
{
    sal_Bool bDummy;
    return FindTextStyleAndHyperlink( rPropSet, bDummy, rHasCharStyle );
}


void XMLTextParagraphExport::exportListChange(
        const XMLTextNumRuleInfo& rPrevInfo,
        const XMLTextNumRuleInfo& rNextInfo )
{
    // end a list
    if( rPrevInfo.GetLevel() > 0 &&
        ( !rNextInfo.HasSameNumRules( rPrevInfo ) ||
          rNextInfo.GetLevel() < rPrevInfo.GetLevel() ||
             rNextInfo.IsRestart()	) )
    {
        sal_Int16 nPrevLevel = rPrevInfo.GetLevel();
        sal_Int16 nNextLevel =
            ( !rNextInfo.HasSameNumRules( rPrevInfo ) ||
              rNextInfo.IsRestart() ) ?  0 : rNextInfo.GetLevel();

        DBG_ASSERT( pListElements &&
                pListElements->Count() >= 2*(nNextLevel-nPrevLevel),
                "SwXMLExport::ExportListChange: list elements missing" );

        for( sal_Int16 i=nPrevLevel; i > nNextLevel; i-- )
        {
            for( sal_uInt16 j=0; j<2; j++ )
            {
                OUString *pElem = (*pListElements)[pListElements->Count()-1];
                pListElements->Remove( pListElements->Count()-1 );

                GetExport().EndElement( *pElem, sal_True );

                delete pElem;
            }
        }
    }

    // start a new list
    if( rNextInfo.GetLevel() > 0 &&
        ( !rPrevInfo.HasSameNumRules( rNextInfo ) ||
          rPrevInfo.GetLevel() < rNextInfo.GetLevel() ||
             rNextInfo.IsRestart() ) )
    {
        sal_Int16 nPrevLevel =
            ( !rNextInfo.HasSameNumRules( rPrevInfo ) ||
              rNextInfo.IsRestart() ) ? 0 : rPrevInfo.GetLevel();
        sal_Int16 nNextLevel = rNextInfo.GetLevel();

        // Find out whether this is the first application of the list or not.
        // For named lists, we use the internal name. For unnamed lists, we
        // use the generated name. This works well, because there are either
        // unnamed or either named lists only.
        sal_Bool bListExported = sal_False;
        OUString sName;
        if( rNextInfo.IsNamed() )
            sName = rNextInfo.GetName();
        else
            sName = pListAutoPool->Find( rNextInfo.GetNumRules() );
        DBG_ASSERT( sName.getLength(), "list without a name" );
        if( sName.getLength() )
        {
            bListExported = pExportedLists &&
                             pExportedLists->Seek_Entry( (OUString *)&sName );
            if( !bListExported )
            {
                if( !pExportedLists )
                    pExportedLists = new OUStringsSort_Impl;
                pExportedLists->Insert( new OUString(sName) );
            }
        }
        sal_Bool bContinue = !rNextInfo.IsRestart() && bListExported &&
                             !rPrevInfo.HasSameNumRules( rNextInfo );

        for( sal_Int16 i=nPrevLevel; i < nNextLevel; i++)
        {
            // <text:ordered-list> or <text:unordered-list>
            GetExport().CheckAttrList();
            if( 0 == i )
            {
                // For named list, the name might be the name of an automatic
                // rule, so we have to take a look into the style pool.
                // For unnamed lists, we have done this already.
                if( rNextInfo.IsNamed() )
                {
                    OUString sTmp( pListAutoPool->Find(
                                            rNextInfo.GetNumRules() ) );
                    if( sTmp.getLength() )
                        sName = sTmp;
                }
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                                          sName );
            }
            if( bContinue && rNextInfo.IsOrdered() )
                GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                          XML_CONTINUE_NUMBERING, XML_TRUE );

            enum XMLTokenEnum eLName =
                    rNextInfo.IsOrdered() ? XML_ORDERED_LIST
                                          : XML_UNORDERED_LIST;

            OUString *pElem = new OUString(
                    GetExport().GetNamespaceMap().GetQNameByKey(
                                        XML_NAMESPACE_TEXT,
                                        GetXMLToken(eLName) ) );
            GetExport().IgnorableWhitespace();
            GetExport().StartElement( *pElem, sal_False );

            if( !pListElements )
                pListElements = new OUStrings_Impl;
            pListElements->Insert( pElem, pListElements->Count() );

            // <text:list-header> or <text:list-item>
            GetExport().CheckAttrList();
            if( rNextInfo.HasStartValue() )
            {
                OUStringBuffer aBuffer;
                aBuffer.append( (sal_Int32)rNextInfo.GetStartValue() );
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_START_VALUE,
                              aBuffer.makeStringAndClear() );
            }
            eLName = (rNextInfo.IsNumbered() || i+1 < nNextLevel)
                        ? XML_LIST_ITEM
                        : XML_LIST_HEADER;
            pElem = new OUString(  GetExport().GetNamespaceMap().GetQNameByKey(
                                        XML_NAMESPACE_TEXT,
                                        GetXMLToken(eLName) ) );
            GetExport().IgnorableWhitespace();
            GetExport().StartElement( *pElem, sal_False );

            pListElements->Insert( pElem, pListElements->Count() );
        }
    }

    if( rNextInfo.GetLevel() > 0 && rNextInfo.IsNumbered() &&
        rPrevInfo.HasSameNumRules( rNextInfo ) &&
        rPrevInfo.GetLevel() >= rNextInfo.GetLevel() &&
        !rNextInfo.IsRestart() )
    {
        // </text:list-item> or </text:list-header>
        DBG_ASSERT( pListElements && pListElements->Count() >= 2,
                "SwXMLExport::ExportListChange: list elements missing" );

        OUString *pElem = (*pListElements)[pListElements->Count()-1];
        GetExport().EndElement( *pElem, sal_True );

        pListElements->Remove( pListElements->Count()-1 );
        delete pElem;

        // <text:list-item>
        GetExport().CheckAttrList();
        if( rNextInfo.HasStartValue() )
        {
            OUStringBuffer aBuffer;
            aBuffer.append( (sal_Int32)rNextInfo.GetStartValue() );
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_START_VALUE,
                              aBuffer.makeStringAndClear() );
        }
        pElem = new OUString( GetExport().GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_TEXT,
                                GetXMLToken(XML_LIST_ITEM) ) );
        GetExport().IgnorableWhitespace();
        GetExport().StartElement( *pElem, sal_False );

        pListElements->Insert( pElem, pListElements->Count() );
    }
}


XMLTextParagraphExport::XMLTextParagraphExport(
        SvXMLExport& rExp,
        SvXMLAutoStylePoolP & rASP
        ) :
    XMLStyleExport( rExp, OUString(), &rASP ),
//	rExport( rExp ),
    rAutoStylePool( rASP ),
    pListElements( 0 ),
    pExportedLists( 0 ),
    pListAutoPool( new XMLTextListAutoStylePool( this->GetExport() ) ),
    pFieldExport( 0 ),
    pPageTextFrameIdxs( 0 ),
    pPageGraphicIdxs( 0 ),
    pPageEmbeddedIdxs( 0 ),
    pPageShapeIdxs( 0 ),
    pFrameTextFrameIdxs( 0 ),
    pFrameGraphicIdxs( 0 ),
    pFrameEmbeddedIdxs( 0 ),
    pSectionExport( NULL ),
    pIndexMarkExport( NULL ),
    pRedlineExport( NULL ),
    pFrameShapeIdxs( 0 ),
    bBlock( sal_False ),
    bOpenRuby( sal_False ),
    sParagraphService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.Paragraph")),
    sTableService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextTable")),
    sTextFieldService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextField")),
    sTextFrameService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextFrame")),
    sTextGraphicService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextGraphicObject")),
    sTextEmbeddedService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextEmbeddedObject")),
    sTextEndnoteService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.Endnote")),
    sTextContentService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextContent")),
    sShapeService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Shape")),
    sParaStyleName(RTL_CONSTASCII_USTRINGPARAM("ParaStyleName")),
    sParaConditionalStyleName(RTL_CONSTASCII_USTRINGPARAM("ParaConditionalStyleName")),
    sParaChapterNumberingLevel(RTL_CONSTASCII_USTRINGPARAM("ParaChapterNumberingLevel")),
    sCharStyleName(RTL_CONSTASCII_USTRINGPARAM("CharStyleName")),
    sCharStyleNames(RTL_CONSTASCII_USTRINGPARAM("CharStyleNames")),
    sFrameStyleName(RTL_CONSTASCII_USTRINGPARAM("FrameStyleName")),
    sTextField(RTL_CONSTASCII_USTRINGPARAM("TextField")),
    sText(RTL_CONSTASCII_USTRINGPARAM("Text")),
    sFrame(RTL_CONSTASCII_USTRINGPARAM("Frame")),
    sCategory(RTL_CONSTASCII_USTRINGPARAM("Category")),
    sNumberingRules(RTL_CONSTASCII_USTRINGPARAM("NumberingRules")),
    sTextPortionType(RTL_CONSTASCII_USTRINGPARAM("TextPortionType")),
    sFootnote(RTL_CONSTASCII_USTRINGPARAM("Footnote")),
    sBookmark(RTL_CONSTASCII_USTRINGPARAM("Bookmark")),
    sReferenceMark(RTL_CONSTASCII_USTRINGPARAM("ReferenceMark")),
    sIsCollapsed(RTL_CONSTASCII_USTRINGPARAM("IsCollapsed")),
    sIsStart(RTL_CONSTASCII_USTRINGPARAM("IsStart")),
    sReferenceId(RTL_CONSTASCII_USTRINGPARAM("ReferenceId")),
    sNumberingType(RTL_CONSTASCII_USTRINGPARAM("NumberingType")),
    sPageStyleName(RTL_CONSTASCII_USTRINGPARAM("PageStyleName")),
    sPageDescName(RTL_CONSTASCII_USTRINGPARAM("PageDescName")),
    sPrefix(RTL_CONSTASCII_USTRINGPARAM("Prefix")),
    sStartAt(RTL_CONSTASCII_USTRINGPARAM("StartAt")),
    sSuffix(RTL_CONSTASCII_USTRINGPARAM("Suffix")),
    sPositionEndOfDoc(RTL_CONSTASCII_USTRINGPARAM("PositionEndOfDoc")),
    sFootnoteCounting(RTL_CONSTASCII_USTRINGPARAM("FootnoteCounting")),
    sEndNotice(RTL_CONSTASCII_USTRINGPARAM("EndNotice")),
    sBeginNotice(RTL_CONSTASCII_USTRINGPARAM("BeginNotice")),
    sFrameWidthAbsolute(RTL_CONSTASCII_USTRINGPARAM("FrameWidthAbsolute")),
    sFrameWidthPercent(RTL_CONSTASCII_USTRINGPARAM("FrameWidthPercent")),
    sFrameHeightAbsolute(RTL_CONSTASCII_USTRINGPARAM("FrameHeightAbsolute")),
    sFrameHeightPercent(RTL_CONSTASCII_USTRINGPARAM("FrameHeightPercent")),
    sWidth(RTL_CONSTASCII_USTRINGPARAM("Width")),
    sRelativeWidth(RTL_CONSTASCII_USTRINGPARAM("RelativeWidth")),
    sHeight(RTL_CONSTASCII_USTRINGPARAM("Height")),
    sRelativeHeight(RTL_CONSTASCII_USTRINGPARAM("RelativeHeight")),
    sSizeType(RTL_CONSTASCII_USTRINGPARAM("SizeType")),
    sIsSyncWidthToHeight(RTL_CONSTASCII_USTRINGPARAM("IsSyncWidthToHeight")),
    sIsSyncHeightToWidth(RTL_CONSTASCII_USTRINGPARAM("IsSyncHeightToWidth")),
    sHoriOrient(RTL_CONSTASCII_USTRINGPARAM("HoriOrient")),
    sHoriOrientPosition(RTL_CONSTASCII_USTRINGPARAM("HoriOrientPosition")),
    sVertOrient(RTL_CONSTASCII_USTRINGPARAM("VertOrient")),
    sVertOrientPosition(RTL_CONSTASCII_USTRINGPARAM("VertOrientPosition")),
    sChainNextName(RTL_CONSTASCII_USTRINGPARAM("ChainNextName")),
    sAnchorType(RTL_CONSTASCII_USTRINGPARAM("AnchorType")),
    sAnchorPageNo(RTL_CONSTASCII_USTRINGPARAM("AnchorPageNo")),
    sGraphicURL(RTL_CONSTASCII_USTRINGPARAM("GraphicURL")),
    sGraphicFilter(RTL_CONSTASCII_USTRINGPARAM("GraphicFilter")),
    sGraphicRotation(RTL_CONSTASCII_USTRINGPARAM("GraphicRotation")),
    sAlternativeText(RTL_CONSTASCII_USTRINGPARAM("AlternativeText")),
    sHyperLinkURL(RTL_CONSTASCII_USTRINGPARAM("HyperLinkURL")),
    sHyperLinkName(RTL_CONSTASCII_USTRINGPARAM("HyperLinkName")),
    sHyperLinkTarget(RTL_CONSTASCII_USTRINGPARAM("HyperLinkTarget")),
    sUnvisitedCharStyleName(RTL_CONSTASCII_USTRINGPARAM("UnvisitedCharStyleName")),
    sVisitedCharStyleName(RTL_CONSTASCII_USTRINGPARAM("VisitedCharStyleName")),
    sTextSection(RTL_CONSTASCII_USTRINGPARAM("TextSection")),
    sDocumentIndex(RTL_CONSTASCII_USTRINGPARAM("DocumentIndex")),
    sDocumentIndexMark(RTL_CONSTASCII_USTRINGPARAM("DocumentIndexMark")),
    sActualSize(RTL_CONSTASCII_USTRINGPARAM("ActualSize")),
    sContourPolyPolygon(RTL_CONSTASCII_USTRINGPARAM("ContourPolyPolygon")),
    sIsPixelContour(RTL_CONSTASCII_USTRINGPARAM("IsPixelContour")),
    sIsAutomaticContour(RTL_CONSTASCII_USTRINGPARAM("IsAutomaticContour")),
    sAnchorCharStyleName(RTL_CONSTASCII_USTRINGPARAM("AnchorCharStyleName")),
    sServerMap(RTL_CONSTASCII_USTRINGPARAM("ServerMap")),
    sRedline(RTL_CONSTASCII_USTRINGPARAM("Redline")),
    sRuby(RTL_CONSTASCII_USTRINGPARAM("Ruby")),
    sRubyText(RTL_CONSTASCII_USTRINGPARAM("RubyText")),
    sRubyAdjust(RTL_CONSTASCII_USTRINGPARAM("RubyAdjust")),
    sRubyCharStyleName(RTL_CONSTASCII_USTRINGPARAM("RubyCharStyleName")),
    bProgress( sal_False ),
    aCharStyleNamesPropInfoCache( sCharStyleNames )
{
    UniReference < XMLPropertySetMapper > xPropMapper =
        new XMLTextPropertySetMapper( TEXT_PROP_MAP_PARA );
    xParaPropMapper = new XMLTextExportPropertySetMapper( xPropMapper,
                                                             GetExport() );

    OUString sFamily( GetXMLToken(XML_PARAGRAPH) );
    OUString sPrefix( 'P' );
    rAutoStylePool.AddFamily( XML_STYLE_FAMILY_TEXT_PARAGRAPH, sFamily,
                              xParaPropMapper, sPrefix );

    xPropMapper = new XMLTextPropertySetMapper( TEXT_PROP_MAP_TEXT );
    xTextPropMapper = new XMLTextExportPropertySetMapper( xPropMapper,
                                                             GetExport() );
    sFamily = OUString( GetXMLToken(XML_TEXT) );
    sPrefix = OUString( 'T' );
    rAutoStylePool.AddFamily( XML_STYLE_FAMILY_TEXT_TEXT, sFamily,
                              xTextPropMapper, sPrefix );

    xPropMapper = new XMLTextPropertySetMapper( TEXT_PROP_MAP_AUTO_FRAME );
    xAutoFramePropMapper = new XMLTextExportPropertySetMapper( xPropMapper,
                                                                  GetExport() );
    sFamily = OUString( RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_GRAPHICS_NAME) );
    sPrefix = OUString( RTL_CONSTASCII_USTRINGPARAM( "fr" ) );
    rAutoStylePool.AddFamily( XML_STYLE_FAMILY_TEXT_FRAME, sFamily,
                              xAutoFramePropMapper, sPrefix );

    xPropMapper = new XMLTextPropertySetMapper( TEXT_PROP_MAP_SECTION );
    xSectionPropMapper = new XMLTextExportPropertySetMapper( xPropMapper,
                                                             GetExport() );
    sFamily = OUString( GetXMLToken( XML_SECTION ) );
    sPrefix = OUString( RTL_CONSTASCII_USTRINGPARAM( "Sect" ) );
    rAutoStylePool.AddFamily( XML_STYLE_FAMILY_TEXT_SECTION, sFamily,
                              xSectionPropMapper, sPrefix );

    xPropMapper = new XMLTextPropertySetMapper( TEXT_PROP_MAP_RUBY );
    xRubyPropMapper = new SvXMLExportPropertyMapper( xPropMapper );
    sFamily = OUString( GetXMLToken( XML_RUBY ) );
    sPrefix = OUString( RTL_CONSTASCII_USTRINGPARAM( "Ru" ) );
    rAutoStylePool.AddFamily( XML_STYLE_FAMILY_TEXT_RUBY, sFamily,
                              xRubyPropMapper, sPrefix );

    xPropMapper = new XMLTextPropertySetMapper( TEXT_PROP_MAP_FRAME );
    xFramePropMapper = new XMLTextExportPropertySetMapper( xPropMapper,
                                                              GetExport() );
    pSectionExport = new XMLSectionExport( rExp, *this );
    pIndexMarkExport = new XMLIndexMarkExport( rExp, *this );
    pRedlineExport = IsBlockMode() ? NULL : new XMLRedlineExport( rExp );

    // The text field helper needs a pre-constructed XMLPropertyState
    // to export the combined characters field. We construct that
    // here, because we need the text property mapper to do it.

    // construct Any value, then find index
    Any aAny;
    sal_Bool bTmp = sal_True;
    aAny.setValue(&bTmp, ::getBooleanCppuType());
    sal_Int32 nIndex = xTextPropMapper->getPropertySetMapper()->FindEntryIndex(
                                "", XML_NAMESPACE_STYLE,
                                GetXMLToken(XML_TEXT_COMBINE));
    pFieldExport = new XMLTextFieldExport( rExp, new XMLPropertyState( nIndex, aAny ) );
}

XMLTextParagraphExport::~XMLTextParagraphExport()
{
    delete pRedlineExport;
    delete pIndexMarkExport;
    delete pSectionExport;
    delete pFieldExport;
    delete pListElements;
    delete pExportedLists;
    delete pListAutoPool;
    delete pPageTextFrameIdxs;
    delete pPageGraphicIdxs;
    delete pPageEmbeddedIdxs;
    delete pPageShapeIdxs;
    delete pFrameTextFrameIdxs;
    delete pFrameGraphicIdxs;
    delete pFrameEmbeddedIdxs;
    delete pFrameShapeIdxs;
#ifdef DBG_UTIL
    txtparae_bContainsIllegalCharacters = sal_False;
#endif
}

SvXMLExportPropertyMapper *XMLTextParagraphExport::CreateShapeExtPropMapper(
        SvXMLExport& rExport )
{
    UniReference < XMLPropertySetMapper > xPropMapper =
        new XMLTextPropertySetMapper( TEXT_PROP_MAP_SHAPE );
    return new XMLTextExportPropertySetMapper( xPropMapper, rExport );
}

SvXMLExportPropertyMapper *XMLTextParagraphExport::CreateCharExtPropMapper(
        SvXMLExport& rExport)
{
    XMLPropertySetMapper *pPropMapper =
        new XMLTextPropertySetMapper( TEXT_PROP_MAP_TEXT );
    return new XMLTextExportPropertySetMapper( pPropMapper, rExport );
}

SvXMLExportPropertyMapper *XMLTextParagraphExport::CreateParaExtPropMapper(
        SvXMLExport& rExport)
{
    XMLPropertySetMapper *pPropMapper =
        new XMLTextPropertySetMapper( TEXT_PROP_MAP_SHAPE_PARA );
    return new XMLTextExportPropertySetMapper( pPropMapper, rExport );
}

void XMLTextParagraphExport::collectFrames( sal_Bool bBoundToFrameOnly )
{
    Reference < XTextFramesSupplier > xTFS( GetExport().GetModel(), UNO_QUERY );
    if( xTFS.is() )
    {
        xTextFrames = Reference < XIndexAccess >( xTFS->getTextFrames(),
                                                  UNO_QUERY );
        sal_Int32 nCount =  xTextFrames->getCount();
        for( sal_Int32 i = 0; i < nCount; i++ )
        {
            Any aAny = xTextFrames->getByIndex( i );
            Reference < XTextFrame > xTxtFrame;
            aAny >>= xTxtFrame;
            Reference < XPropertySet > xPropSet( xTxtFrame, UNO_QUERY );

            aAny = xPropSet->getPropertyValue( sAnchorType );
            TextContentAnchorType eAnchor;
            aAny >>= eAnchor;

            switch( eAnchor )
            {
            case TextContentAnchorType_AT_PAGE:
                if( !bBoundToFrameOnly )
                {
                    if( !pPageTextFrameIdxs )
                        pPageTextFrameIdxs = new SvLongs;
                    pPageTextFrameIdxs->Insert( i, pPageTextFrameIdxs->Count() );
                }
                break;
            case TextContentAnchorType_AT_FRAME:
                if( !pFrameTextFrameIdxs )
                    pFrameTextFrameIdxs = new SvLongs;
                pFrameTextFrameIdxs->Insert( i, pFrameTextFrameIdxs->Count() );
                break;
            }
        }
    }

    Reference < XTextGraphicObjectsSupplier > xTGOS( GetExport().GetModel(),
                                                    UNO_QUERY );
    if( xTGOS.is() )
    {
        xGraphics = Reference < XIndexAccess >( xTGOS->getGraphicObjects(),
                                                  UNO_QUERY );
        sal_Int32 nCount =  xGraphics->getCount();
        for( sal_Int32 i = 0; i < nCount; i++ )
        {
            Any aAny = xGraphics->getByIndex( i );
            Reference < XTextContent > xTxtCntnt;
            aAny >>= xTxtCntnt;
            Reference < XPropertySet > xPropSet( xTxtCntnt, UNO_QUERY );

            aAny = xPropSet->getPropertyValue( sAnchorType );
            TextContentAnchorType eAnchor;
            aAny >>= eAnchor;

            switch( eAnchor )
            {
            case TextContentAnchorType_AT_PAGE:
                if( !bBoundToFrameOnly )
                {
                    if( !pPageGraphicIdxs )
                        pPageGraphicIdxs = new SvLongs;
                    pPageGraphicIdxs->Insert( i, pPageGraphicIdxs->Count() );
                }
                break;
            case TextContentAnchorType_AT_FRAME:
                if( !pFrameGraphicIdxs )
                    pFrameGraphicIdxs = new SvLongs;
                pFrameGraphicIdxs->Insert( i, pFrameGraphicIdxs->Count() );
                break;
            }
        }
    }

    Reference < XTextEmbeddedObjectsSupplier > xTEOS( GetExport().GetModel(),
                                                    UNO_QUERY );
    if( xTEOS.is() )
    {
        xEmbeddeds = Reference < XIndexAccess >( xTEOS->getEmbeddedObjects(),
                                                  UNO_QUERY );
        sal_Int32 nCount =  xEmbeddeds->getCount();
        for( sal_Int32 i = 0; i < nCount; i++ )
        {
            Any aAny = xEmbeddeds->getByIndex( i );
            Reference < XTextContent > xTxtCntnt;
            aAny >>= xTxtCntnt;
            Reference < XPropertySet > xPropSet( xTxtCntnt, UNO_QUERY );

            aAny = xPropSet->getPropertyValue( sAnchorType );
            TextContentAnchorType eAnchor;
            aAny >>= eAnchor;

            switch( eAnchor )
            {
            case TextContentAnchorType_AT_PAGE:
                if( !bBoundToFrameOnly )
                {
                    if( !pPageEmbeddedIdxs )
                        pPageEmbeddedIdxs = new SvLongs;
                    pPageEmbeddedIdxs->Insert( i, pPageEmbeddedIdxs->Count() );
                }
                break;
            case TextContentAnchorType_AT_FRAME:
                if( !pFrameEmbeddedIdxs )
                    pFrameEmbeddedIdxs = new SvLongs;
                pFrameEmbeddedIdxs->Insert( i, pFrameEmbeddedIdxs->Count() );
                break;
            }
        }
    }

    Reference < XDrawPageSupplier > xDPS( GetExport().GetModel(),
                                                    UNO_QUERY );
    if( xDPS.is() )
    {
        xShapes = Reference < XIndexAccess >( xDPS->getDrawPage(),
                                                  UNO_QUERY );
        sal_Int32 nCount =  xShapes->getCount();
        for( sal_Int32 i = 0; i < nCount; i++ )
        {
            Any aAny = xShapes->getByIndex( i );
            Reference < XShape > xShape;
            aAny >>= xShape;
            if( !xShape.is() )
                continue;

            Reference < XPropertySet > xPropSet( xShape, UNO_QUERY );

            aAny = xPropSet->getPropertyValue( sAnchorType );
            TextContentAnchorType eAnchor;
            aAny >>= eAnchor;

            if( (TextContentAnchorType_AT_PAGE != eAnchor &&
                 TextContentAnchorType_AT_FRAME != eAnchor) ||
                 (TextContentAnchorType_AT_PAGE == eAnchor &&
                 bBoundToFrameOnly ) )
                continue;

            Reference<XServiceInfo> xServiceInfo( xShape,
                                                  UNO_QUERY );
            if( xServiceInfo->supportsService( sTextFrameService ) ||
                  xServiceInfo->supportsService( sTextGraphicService ) ||
                xServiceInfo->supportsService( sTextEmbeddedService ) )
                continue;

            if( TextContentAnchorType_AT_PAGE == eAnchor )
            {
                if( !pPageShapeIdxs )
                    pPageShapeIdxs = new SvLongs;
                pPageShapeIdxs->Insert( i, pPageShapeIdxs->Count() );
            }
            else
            {
                if( !pFrameShapeIdxs )
                    pFrameShapeIdxs = new SvLongs;
                pFrameShapeIdxs->Insert( i, pFrameShapeIdxs->Count() );
            }
        }
    }
}

void XMLTextParagraphExport::exportPageFrames( sal_Bool bAutoStyles,
                                               sal_Bool bProgress )
{
    if( pPageTextFrameIdxs )
    {
        for( sal_uInt16 i = 0; i < pPageTextFrameIdxs->Count(); i++ )
        {
            Any aAny = xTextFrames->getByIndex( (*pPageTextFrameIdxs)[i] );
            Reference < XTextFrame > xTxtFrame;
            aAny >>= xTxtFrame;
            Reference < XTextContent > xTxtCntnt( xTxtFrame, UNO_QUERY );
            exportTextFrame( xTxtCntnt, bAutoStyles, bProgress );
        }
    }
    if( pPageGraphicIdxs )
    {
        for( sal_uInt16 i = 0; i < pPageGraphicIdxs->Count(); i++ )
        {
            Any aAny = xGraphics->getByIndex( (*pPageGraphicIdxs)[i] );
            Reference < XTextContent > xTxtCntnt;
            aAny >>= xTxtCntnt;
            exportTextGraphic( xTxtCntnt, bAutoStyles );
        }
    }
    if( pPageEmbeddedIdxs )
    {
        for( sal_uInt16 i = 0; i < pPageEmbeddedIdxs->Count(); i++ )
        {
            Any aAny = xEmbeddeds->getByIndex( (*pPageEmbeddedIdxs)[i] );
            Reference < XTextContent > xTxtCntnt;
            aAny >>= xTxtCntnt;
            exportTextEmbedded( xTxtCntnt, bAutoStyles );
        }
    }
    if( pPageShapeIdxs )
    {
        for( sal_uInt16 i = 0; i < pPageShapeIdxs->Count(); i++ )
        {
            Any aAny = xShapes->getByIndex( (*pPageShapeIdxs)[i] );
            Reference < XShape > xShape;
            aAny >>= xShape;
            Reference < XTextContent > xTxtCntnt( xShape, UNO_QUERY );
            exportShape( xTxtCntnt, bAutoStyles );
        }
    }
}

sal_Bool lcl_txtpara_isFrameAnchor(
        const Reference < XPropertySet > rPropSet,
        const Reference < XTextFrame >& rParentTxtFrame )
{
    Any aAny = rPropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("AnchorFrame") ) );
    Reference < XTextFrame > xAnchorTxtFrame;
    aAny >>= xAnchorTxtFrame;

    return xAnchorTxtFrame == rParentTxtFrame;
}

void XMLTextParagraphExport::exportFrameFrames(
        sal_Bool bAutoStyles,
        sal_Bool bProgress,
        const Reference < XTextFrame > *pParentTxtFrame )
{
    if( pFrameTextFrameIdxs && pFrameTextFrameIdxs->Count() )
    {
        Any aAny;
        sal_uInt16 i = 0;
        while( i < pFrameTextFrameIdxs->Count() )
        {
            aAny = xTextFrames->getByIndex( (*pFrameTextFrameIdxs)[i] );
            Reference < XTextFrame > xTxtFrame;
            aAny >>= xTxtFrame;
            Reference < XPropertySet > xPropSet( xTxtFrame, UNO_QUERY );
            if( lcl_txtpara_isFrameAnchor( xPropSet, *pParentTxtFrame ) )
            {
                if( !bAutoStyles )
                    pFrameTextFrameIdxs->Remove( i );
                sal_uInt16 nOldCount = pFrameTextFrameIdxs->Count();
                Reference < XTextContent > xTxtCntnt( xTxtFrame, UNO_QUERY );
                exportTextFrame( xTxtCntnt, bAutoStyles, bProgress );
                if( bAutoStyles )
                    i++;
                else if( pFrameTextFrameIdxs->Count() != nOldCount )
                    i = 0;
            }
            else
                i++;
        }
    }
    if( pFrameGraphicIdxs && pFrameGraphicIdxs->Count() )
    {
        Any aAny;
        sal_uInt16 i = 0;
        while( i < pFrameGraphicIdxs->Count() )
        {
            aAny = xGraphics->getByIndex( (*pFrameGraphicIdxs)[i] );
            Reference < XTextContent > xTxtCntnt;
            aAny >>= xTxtCntnt;
            Reference < XPropertySet > xPropSet( xTxtCntnt, UNO_QUERY );
            if( lcl_txtpara_isFrameAnchor( xPropSet, *pParentTxtFrame ) )
            {
                if( !bAutoStyles )
                    pFrameGraphicIdxs->Remove( i );
                sal_uInt16 nOldCount = pFrameGraphicIdxs->Count();
                exportTextGraphic( xTxtCntnt, bAutoStyles );
                if( bAutoStyles )
                    i++;
                else if( pFrameGraphicIdxs->Count() != nOldCount )
                    i = 0;
            }
            else
                i++;
        }
    }
    if( pFrameEmbeddedIdxs && pFrameEmbeddedIdxs->Count() )
    {
        Any aAny;
        sal_uInt16 i = 0;
        while( i < pFrameEmbeddedIdxs->Count() )
        {
            aAny = xEmbeddeds->getByIndex( (*pFrameEmbeddedIdxs)[i] );
            Reference < XEmbeddedObjectSupplier > xEOS;
            aAny >>= xEOS;
            Reference < XPropertySet > xPropSet( xEOS, UNO_QUERY );
            if( lcl_txtpara_isFrameAnchor( xPropSet, *pParentTxtFrame ) )
            {
                if( !bAutoStyles )
                    pFrameEmbeddedIdxs->Remove( i );
                sal_uInt16 nOldCount = pFrameEmbeddedIdxs->Count();
                Reference < XTextContent > xTxtCntnt( xEOS, UNO_QUERY );
                exportTextEmbedded( xTxtCntnt, bAutoStyles );
                if( bAutoStyles )
                    i++;
                else if( pFrameEmbeddedIdxs->Count() != nOldCount )
                    i = 0;
            }
            else
                i++;
        }
    }
    if( pFrameShapeIdxs && pFrameShapeIdxs->Count() )
    {
        Any aAny;
        sal_uInt16 i = 0;
        while( i < pFrameShapeIdxs->Count() )
        {
            aAny = xShapes->getByIndex( (*pFrameShapeIdxs)[i] );
            Reference < XShape > xShape;
            aAny >>= xShape;
            Reference < XPropertySet > xPropSet( xShape, UNO_QUERY );
            if( lcl_txtpara_isFrameAnchor( xPropSet, *pParentTxtFrame ) )
            {
                if( !bAutoStyles )
                    pFrameShapeIdxs->Remove( i );
                sal_uInt16 nOldCount = pFrameShapeIdxs->Count();
                Reference < XTextContent > xTxtCntnt( xShape, UNO_QUERY );
                exportShape( xTxtCntnt, bAutoStyles );
                if( bAutoStyles )
                    i++;
                else if( pFrameShapeIdxs->Count() != nOldCount )
                    i = 0;
            }
            else
                i++;
        }
    }
}

void XMLTextParagraphExport::exportText(
        const Reference < XText > & rText,
        sal_Bool bAutoStyles,
        sal_Bool bProgress,
        sal_Bool bExportParagraph )
{
    if( bAutoStyles )
        GetExport().GetShapeExport(); // make sure the graphics styles family
                                      // is added
    Reference < XEnumerationAccess > xEA( rText, UNO_QUERY );
    Reference < XEnumeration > xParaEnum = xEA->createEnumeration();
    Reference < XPropertySet > xPropertySet( rText, UNO_QUERY );
    Reference < XTextSection > xBaseSection;

    // #97718# footnotes don't supply paragraph enumerations in some cases
    // This is always a bug, but at least we don't want to crash.
    DBG_ASSERT( xParaEnum.is(), "We need a paragraph enumeration" );
    if( ! xParaEnum.is() )
        return;

    sal_Bool bExportLevels = sal_True;

    if (xPropertySet.is())
    {
        Reference < XPropertySetInfo > xInfo ( xPropertySet->getPropertySetInfo() );

        if( xInfo.is() )
        {
            if (xInfo->hasPropertyByName( sTextSection ))
            {
                xPropertySet->getPropertyValue(sTextSection) >>= xBaseSection ;
            }

            // for applications that use the outliner we need to check if
            // the current text object needs the level information exported
            if( !bAutoStyles )
            {
                // fixme: move string to class member, couldn't do now because
                //		  of no incompatible build
                OUString sHasLevels( RTL_CONSTASCII_USTRINGPARAM("HasLevels") );
                if (xInfo->hasPropertyByName( sHasLevels ) )
                {
                    xPropertySet->getPropertyValue(sHasLevels) >>= bExportLevels;
                }
            }
        }
    }

    // #96530# Export redlines at start & end of XText before & after
    // exporting the text content enumeration
    if( !bAutoStyles && (pRedlineExport != NULL) )
        pRedlineExport->ExportStartOrEndRedline( xPropertySet, sal_True );
    exportTextContentEnumeration( xParaEnum, bAutoStyles, xBaseSection,
                                  bProgress, bExportParagraph, 0, bExportLevels );
    if( !bAutoStyles && (pRedlineExport != NULL) )
        pRedlineExport->ExportStartOrEndRedline( xPropertySet, sal_False );
}

void XMLTextParagraphExport::exportText(
        const Reference < XText > & rText,
        const Reference < XTextSection > & rBaseSection,
        sal_Bool bAutoStyles,
        sal_Bool bProgress,
        sal_Bool bExportParagraph )
{
    if( bAutoStyles )
        GetExport().GetShapeExport(); // make sure the graphics styles family
                                      // is added
    Reference < XEnumerationAccess > xEA( rText, UNO_QUERY );
    Reference < XEnumeration > xParaEnum = xEA->createEnumeration();

    // #98165# don't continue without a paragraph enumeration
    if( ! xParaEnum.is() )
        return;

    // #96530# Export redlines at start & end of XText before & after
    // exporting the text content enumeration
    Reference<XPropertySet> xPropertySet;
    if( !bAutoStyles && (pRedlineExport != NULL) )
    {
        xPropertySet = Reference<XPropertySet>::query( rText );
        pRedlineExport->ExportStartOrEndRedline( xPropertySet, sal_True );
    }
    exportTextContentEnumeration( xParaEnum, bAutoStyles, rBaseSection,
                                  bProgress, bExportParagraph );
    if( !bAutoStyles && (pRedlineExport != NULL) )
        pRedlineExport->ExportStartOrEndRedline( xPropertySet, sal_False );
}

sal_Bool XMLTextParagraphExport::exportTextContentEnumeration(
        const Reference < XEnumeration > & rContEnum,
        sal_Bool bAutoStyles,
        const Reference < XTextSection > & rBaseSection,
        sal_Bool bProgress,
        sal_Bool bExportParagraph,
        const Reference < XPropertySet > *pRangePropSet,
        sal_Bool bExportLevels)
{
    DBG_ASSERT( rContEnum.is(), "No enumeration to export!" );
    sal_Bool bHasMoreElements = rContEnum->hasMoreElements();
    if( !bHasMoreElements )
        return sal_False;

    XMLTextNumRuleInfo aPrevNumInfo;
    XMLTextNumRuleInfo aNextNumInfo;

    sal_Bool bHasContent sal_False;
    Reference<XTextSection> xCurrentTextSection = rBaseSection;

    MultiPropertySetHelper aPropSetHelper(
                               bAutoStyles ? aParagraphPropertyNamesAuto :
                                          aParagraphPropertyNames );

    Any aAny;
    sal_Bool bHoldElement = sal_False;
    Reference < XTextContent > xTxtCntnt;
    while( bHoldElement || bHasMoreElements )
    {
        if (bHoldElement)
        {
            bHoldElement = sal_False;
        }
        else
        {
            aAny = rContEnum->nextElement();
            aAny >>= xTxtCntnt;

            aPropSetHelper.resetValues();

        }

        Reference<XServiceInfo> xServiceInfo( xTxtCntnt, UNO_QUERY );
        if( xServiceInfo->supportsService( sParagraphService ) )
        {
            if( bExportLevels )
            {
                if( bAutoStyles )
                {
                    exportListAndSectionChange( xCurrentTextSection, xTxtCntnt,
                                                aPrevNumInfo, aNextNumInfo,
                                                bAutoStyles );
                }
                else
                {
                    aNextNumInfo.Set( xTxtCntnt );

                    exportListAndSectionChange( xCurrentTextSection, aPropSetHelper,
                                                TEXT_SECTION, xTxtCntnt,
                                                aPrevNumInfo, aNextNumInfo,
                                                bAutoStyles );
                }
            }

            // if we found a mute section: skip all section content
            if (pSectionExport->IsMuteSection(xCurrentTextSection))
            {
                // Make sure headings are exported anyway.
                if( !bAutoStyles )
                    pSectionExport->ExportMasterDocHeadingDummies();

                while (rContEnum->hasMoreElements() &&
                       pSectionExport->IsInSection( xCurrentTextSection,
                                                    xTxtCntnt, sal_True ))
                {
                    aAny = rContEnum->nextElement();
                    aAny >>= xTxtCntnt;
                    aPropSetHelper.resetValues();
                    aNextNumInfo.Reset();
                }
                // the first non-mute element still needs to be processed
                bHoldElement =
                    ! pSectionExport->IsInSection( xCurrentTextSection,
                                                   xTxtCntnt, sal_False );
            }
            else
                exportParagraph( xTxtCntnt, bAutoStyles, bProgress,
                                 bExportParagraph, aPropSetHelper );
            bHasContent = sal_True;
        }
        else if( xServiceInfo->supportsService( sTableService ) )
        {
            if( !bAutoStyles )
            {
                aNextNumInfo.Reset();
            }

            exportListAndSectionChange( xCurrentTextSection, xTxtCntnt,
                                        aPrevNumInfo, aNextNumInfo,
                                        bAutoStyles );

            if (! pSectionExport->IsMuteSection(xCurrentTextSection))
            {
                // export start + end redlines (for wholly redlined tables)
                if ((! bAutoStyles) && (NULL != pRedlineExport))
                    pRedlineExport->ExportStartOrEndRedline(xTxtCntnt, sal_True);

                exportTable( xTxtCntnt, bAutoStyles, bProgress  );

                if ((! bAutoStyles) && (NULL != pRedlineExport))
                    pRedlineExport->ExportStartOrEndRedline(xTxtCntnt, sal_False);
            }
            else if( !bAutoStyles )
            {
                // Make sure headings are exported anyway.
                pSectionExport->ExportMasterDocHeadingDummies();
            }

            bHasContent = sal_True;
        }
        else if( xServiceInfo->supportsService( sTextFrameService ) )
        {
            exportTextFrame( xTxtCntnt, bAutoStyles, bProgress, pRangePropSet );
        }
        else if( xServiceInfo->supportsService( sTextGraphicService ) )
        {
            exportTextGraphic( xTxtCntnt, bAutoStyles, pRangePropSet );
        }
        else if( xServiceInfo->supportsService( sTextEmbeddedService ) )
        {
            exportTextEmbedded( xTxtCntnt, bAutoStyles, pRangePropSet );
        }
        else if( xServiceInfo->supportsService( sShapeService ) )
        {
            exportShape( xTxtCntnt, bAutoStyles, pRangePropSet );
        }
        else
        {
            DBG_ASSERT( !xTxtCntnt.is(), "unknown text content" );
        }

        if( !bAutoStyles )
        {
            aPrevNumInfo = aNextNumInfo;
        }

        bHasMoreElements = rContEnum->hasMoreElements();
    }

    if( bExportLevels && bHasContent && !bAutoStyles )
    {
        aNextNumInfo.Reset();

        // close open lists and sections; no new styles
        exportListAndSectionChange( xCurrentTextSection, rBaseSection,
                                    aPrevNumInfo, aNextNumInfo,
                                    bAutoStyles );
    }

    return sal_True;
}

void XMLTextParagraphExport::exportParagraph(
        const Reference < XTextContent > & rTextContent,
        sal_Bool bAutoStyles, sal_Bool bProgress, sal_Bool bExportParagraph,
        MultiPropertySetHelper& rPropSetHelper)
{
    sal_Int8 nOutlineLevel = -1;

    if( bProgress )
    {
        ProgressBarHelper *pProgress = GetExport().GetProgressBarHelper();
            pProgress->SetValue( pProgress->GetValue()+1 );
    }

    // get property set or multi property set and initialize helper
    Reference<XMultiPropertySet> xMultiPropSet( rTextContent, UNO_QUERY );
    Reference<XPropertySet> xPropSet( rTextContent, UNO_QUERY );

    // check for supported properties
    if( !rPropSetHelper.checkedProperties() )
        rPropSetHelper.hasProperties( xPropSet->getPropertySetInfo() );

//	if( xMultiPropSet.is() )
//		rPropSetHelper.getValues( xMultiPropSet );
//	else
//		rPropSetHelper.getValues( xPropSet );

    Any aAny;

    if( bExportParagraph )
    {
        if( bAutoStyles )
        {
            Add( XML_STYLE_FAMILY_TEXT_PARAGRAPH, rPropSetHelper, xPropSet );
        }
        else
        {
            OUString sStyle;
            if( rPropSetHelper.hasProperty( PARA_STYLE_NAME ) )
            {
                if( xMultiPropSet.is() )
                    aAny = rPropSetHelper.getValue( PARA_STYLE_NAME,
                                                    xMultiPropSet );
                else
                    aAny = rPropSetHelper.getValue( PARA_STYLE_NAME,
                                                    xPropSet );
                aAny >>= sStyle;
            }

            OUString sAutoStyle( sStyle );
            sAutoStyle = Find( XML_STYLE_FAMILY_TEXT_PARAGRAPH, xPropSet, sStyle );
            if( sAutoStyle.getLength() )
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                                          sAutoStyle );

            if( rPropSetHelper.hasProperty( PARA_CONDITIONAL_STYLE_NAME ) )
            {
                OUString sCondStyle;
                if( xMultiPropSet.is() )
                    aAny = rPropSetHelper.getValue( PARA_CONDITIONAL_STYLE_NAME,
                                                     xMultiPropSet );
                else
                    aAny = rPropSetHelper.getValue( PARA_CONDITIONAL_STYLE_NAME,
                                                     xPropSet );
                aAny >>= sCondStyle;
                if( sCondStyle != sStyle )
                {
                    sCondStyle = Find( XML_STYLE_FAMILY_TEXT_PARAGRAPH, xPropSet,
                                          sCondStyle );
                    if( sCondStyle.getLength() )
                        GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                                  XML_COND_STYLE_NAME,
                                                  sCondStyle );
                }
            }

            if( rPropSetHelper.hasProperty( PARA_CHAPTER_NUMERBING_LEVEL ) )
            {
                if( xMultiPropSet.is() )
                    aAny = rPropSetHelper.getValue( PARA_CHAPTER_NUMERBING_LEVEL,
                                                     xMultiPropSet );
                else
                    aAny = rPropSetHelper.getValue( PARA_CHAPTER_NUMERBING_LEVEL,
                                                     xPropSet );

                aAny >>= nOutlineLevel;
                if( -1 != nOutlineLevel )
                {
                    OUStringBuffer sTmp;
                    sTmp.append( (sal_Int32)nOutlineLevel+1L );
                    GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_LEVEL,
                                  sTmp.makeStringAndClear() );
                }
            }
        }
    }

    Reference < XEnumerationAccess > xEA( rTextContent, UNO_QUERY );
    Reference < XEnumeration > xTextEnum = xEA->createEnumeration();

    Reference < XEnumeration> xContentEnum;
    Reference < XContentEnumerationAccess > xCEA( rTextContent, UNO_QUERY );
    if( xCEA.is() )
        xContentEnum = xCEA->createContentEnumeration( sTextContentService );
    sal_Bool bHasContentEnum = xContentEnum.is() &&
                                  xContentEnum->hasMoreElements();

    Reference < XTextSection > xSection;
    if( bHasContentEnum )
    {
        // For the auto styles, the multi property set helper is only used
        // if hard attributes are existing. Therfor, it seems to be a better
        // strategy to have the TextSection property seperate, because otherwise
        // we always retrieve the style names even if they are not required.
        if( bAutoStyles )
        {
            if( xPropSet->getPropertySetInfo()->hasPropertyByName( sTextSection ) )
            {
                aAny = xPropSet->getPropertyValue( sTextSection );
                aAny >>= xSection;
            }
        }
        else
        {
            if( rPropSetHelper.hasProperty( TEXT_SECTION ) )
            {
                aAny = rPropSetHelper.getValue( TEXT_SECTION );
                aAny >>= xSection;
            }
        }
    }

    if( bAutoStyles )
    {
        sal_Bool bPrevCharIsSpace = sal_True;
        if( bHasContentEnum )
            bPrevCharIsSpace = !exportTextContentEnumeration(
                                    xContentEnum, bAutoStyles, xSection,
                                    bProgress );
        exportTextRangeEnumeration( xTextEnum, bAutoStyles, bProgress );
    }
    else
    {
        sal_Bool bPrevCharIsSpace = sal_True;
        enum XMLTokenEnum eElem =
            -1 == nOutlineLevel ? XML_P : XML_H;
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT, eElem,
                                  sal_True, sal_False );
        if( bHasContentEnum )
            bPrevCharIsSpace = !exportTextContentEnumeration(
                                    xContentEnum, bAutoStyles, xSection,
                                    bProgress );
        exportTextRangeEnumeration( xTextEnum, bAutoStyles, bProgress,
                                     bPrevCharIsSpace );
    }
}

// bookmarks, reference marks (and TOC marks) are the same except for the
// element names. We use the same method for export and it an array with
// the proper element names
static const enum XMLTokenEnum lcl_XmlReferenceElements[] = {
    XML_REFERENCE_MARK, XML_REFERENCE_MARK_START, XML_REFERENCE_MARK_END };
static const enum XMLTokenEnum lcl_XmlBookmarkElements[] = {
    XML_BOOKMARK, XML_BOOKMARK_START, XML_BOOKMARK_END };


void XMLTextParagraphExport::exportTextRangeEnumeration(
        const Reference < XEnumeration > & rTextEnum,
        sal_Bool bAutoStyles, sal_Bool bProgress,
        sal_Bool bPrvChrIsSpc )
{
    sal_Bool bPrevCharIsSpace = bPrvChrIsSpc;

    Any aAny;
    while( rTextEnum->hasMoreElements() )
    {
        aAny = rTextEnum->nextElement();
        Reference < XTextRange > xTxtRange;
        aAny >>= xTxtRange;

        Reference<XPropertySet> xPropSet(xTxtRange, UNO_QUERY);
        Reference<XPropertySetInfo> xPropInfo =
            xPropSet->getPropertySetInfo();

        if (xPropInfo->hasPropertyByName(sTextPortionType))
        {
            aAny = xPropSet->getPropertyValue(sTextPortionType);
            OUString sType;
            aAny >>= sType;

            if( sType.equals(sText))
            {
                exportTextRange( xTxtRange, bAutoStyles,
                                 bPrevCharIsSpace );
            }
            else if( sType.equals(sTextField))
            {
                exportTextField( xTxtRange, bAutoStyles );
                bPrevCharIsSpace = sal_False;
            }
            else if( sType.equals( sFrame ) )
            {
                Reference < XEnumeration> xContentEnum;
                Reference < XContentEnumerationAccess > xCEA( xTxtRange,
                                                              UNO_QUERY );
                if( xCEA.is() )
                    xContentEnum = xCEA->createContentEnumeration(
                                                    sTextContentService );
                // frames are never in sections
                Reference<XTextSection> xSection;
                if( xContentEnum.is() )
                    exportTextContentEnumeration( xContentEnum,
                                                    bAutoStyles,
                                                    xSection, bProgress, sal_True,
                                                     &xPropSet	);

                bPrevCharIsSpace = sal_False;
            }
            else if (sType.equals(sFootnote))
            {
                exportTextFootnote(xPropSet,
                                   xTxtRange->getString(),
                                   bAutoStyles, bProgress );
                bPrevCharIsSpace = sal_False;
            }
            else if (sType.equals(sBookmark))
            {
                exportTextMark(xPropSet,
                               sBookmark,
                               lcl_XmlBookmarkElements,
                               bAutoStyles);
            }
            else if (sType.equals(sReferenceMark))
            {
                exportTextMark(xPropSet,
                               sReferenceMark,
                               lcl_XmlReferenceElements,
                               bAutoStyles);
            }
            else if (sType.equals(sDocumentIndexMark))
            {
                pIndexMarkExport->ExportIndexMark(xPropSet,	bAutoStyles);
            }
            else if (sType.equals(sRedline))
            {
                if (NULL != pRedlineExport)
                    pRedlineExport->ExportChange(xPropSet, bAutoStyles);
            }
            else if (sType.equals(sRuby))
            {
                exportRuby(xPropSet, bAutoStyles);
            }
            else
                DBG_ERROR("unknown text portion type");
        }
        else
        {
            Reference<XServiceInfo> xServiceInfo( xTxtRange, UNO_QUERY );
            if( xServiceInfo->supportsService( sTextFieldService ) )
            {
                exportTextField( xTxtRange, bAutoStyles );
                bPrevCharIsSpace = sal_False;
            }
            else
            {
                // no TextPortionType property -> non-Writer app -> text
                exportTextRange( xTxtRange, bAutoStyles, bPrevCharIsSpace );
            }
        }
    }

    DBG_ASSERT( !bOpenRuby, "Red Alert: Ruby still open!" );
}

void XMLTextParagraphExport::exportTable(
        const Reference < XTextContent > & rTextContent,
        sal_Bool bAutoStyles, sal_Bool bProgress )
{
}

void XMLTextParagraphExport::exportTextField(
        const Reference < XTextRange > & rTextRange,
        sal_Bool bAutoStyles )
{
    Reference < XPropertySet > xPropSet( rTextRange, UNO_QUERY );
    // non-Writer apps need not support Property TextField, so test first
    if (xPropSet->getPropertySetInfo()->hasPropertyByName( sTextField ))
    {
        Any aAny = xPropSet->getPropertyValue( sTextField );

        Reference < XTextField > xTxtFld;
        aAny >>= xTxtFld;
        DBG_ASSERT( xTxtFld.is(), "text field missing" );
        if( xTxtFld.is() )
        {
            if( bAutoStyles )
            {
                pFieldExport->ExportFieldAutoStyle( xTxtFld );
            }
            else
            {
                pFieldExport->ExportField( xTxtFld );
            }
        }
        else
        {
            // write only characters
            GetExport().Characters(rTextRange->getString());
        }

    }
}

void XMLTextParagraphExport::exportTextMark(
    const Reference<XPropertySet> & rPropSet,
    const OUString sProperty,
    const enum XMLTokenEnum pElements[],
    sal_Bool bAutoStyles)
{
    // mib said: "Hau wech!"
    //
    // (Originally, I'd export a span element in case the (book|reference)mark
    //	was formatted. This actually makes a difference in case some pervert
    //	sets a point reference mark in the document and, say, formats it bold.
    //  This basically meaningless formatting will now been thrown away
    //	(aka cleaned up), since mib said: ...					dvo

     if (!bAutoStyles)
    {
        // get reference (as text content)
        Any aAny;
        aAny = rPropSet->getPropertyValue(sProperty);
        Reference<XTextContent> xRefMark;
        aAny >>= xRefMark;

        // name element
        Reference<XNamed> xName(xRefMark, UNO_QUERY);
        GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_NAME,
                                 xName->getName());

        // start, end, or point-reference?
        sal_Int8 nElement;
        aAny = rPropSet->getPropertyValue(sIsCollapsed);
        if( *(sal_Bool *)aAny.getValue() )
        {
            nElement = 0;
        }
        else
        {
            aAny = rPropSet->getPropertyValue(sIsStart);
            nElement = *(sal_Bool *)aAny.getValue() ? 1 : 2;
        }

        // export element
        DBG_ASSERT(pElements != NULL, "illegal element array");
        DBG_ASSERT(nElement >= 0, "illegal element number");
        DBG_ASSERT(nElement <= 2, "illegal element number");
        SvXMLElementExport aElem(GetExport(),
                                 XML_NAMESPACE_TEXT, pElements[nElement],
                                 sal_False, sal_False);
    }
    // else: no styles. (see above)
}

sal_Bool lcl_txtpara_isBoundAsChar(
        const Reference < XPropertySet > & rPropSet,
        const Reference < XPropertySetInfo > & rPropSetInfo )
{
    sal_Bool bIsBoundAsChar = sal_False;
    OUString sAnchorType( RTL_CONSTASCII_USTRINGPARAM( "AnchorType" ) );
    if( rPropSetInfo->hasPropertyByName( sAnchorType ) )
    {
        Any aAny =
            rPropSet->getPropertyValue( sAnchorType );
        TextContentAnchorType eAnchor;
        aAny >>= eAnchor;
        bIsBoundAsChar = TextContentAnchorType_AS_CHARACTER == eAnchor;
    }

    return bIsBoundAsChar;
}

sal_Int32 XMLTextParagraphExport::addTextFrameAttributes(
    const Reference < XPropertySet >& rPropSet,
    sal_Bool bShape )
{
    sal_Int32 nShapeFeatures = SEF_DEFAULT;

    // draw:name (#97662#: not for shapes, since those names will be
    // treated in the shape export)
    if( !bShape )
    {
        Reference < XNamed > xNamed( rPropSet, UNO_QUERY );
        if( xNamed.is() )
        {
            OUString sName( xNamed->getName() );
            if( sName.getLength() )
                GetExport().AddAttribute( XML_NAMESPACE_DRAW, XML_NAME,
                                          xNamed->getName() );
        }
    }

    Any aAny;
    OUStringBuffer sValue;

    // text:anchor-type
    aAny = rPropSet->getPropertyValue( sAnchorType );
    TextContentAnchorType eAnchor = TextContentAnchorType_AT_PARAGRAPH;
    aAny >>= eAnchor;
    {
        XMLAnchorTypePropHdl aAnchorTypeHdl;
        OUString sTmp;
        aAnchorTypeHdl.exportXML( sTmp, aAny,
                                  GetExport().GetMM100UnitConverter() );
        GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_ANCHOR_TYPE, sTmp );
    }

    // text:anchor-page-number
    if( TextContentAnchorType_AT_PAGE == eAnchor )
    {
        aAny = rPropSet->getPropertyValue( sAnchorPageNo );
        sal_Int16 nPage=0;
        aAny >>= nPage;
        GetExport().GetMM100UnitConverter().convertNumber( sValue,
                                                           (sal_Int32)nPage );
        GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_ANCHOR_PAGE_NUMBER,
                                  sValue.makeStringAndClear() );
    }
    else
    {
        // #92210#
        nShapeFeatures |= SEF_EXPORT_NO_WS;
    }

    if( !bShape )
    {
        // svg:x
        sal_Int16 nHoriOrient =  HoriOrientation::NONE;
        aAny = rPropSet->getPropertyValue( sHoriOrient );
        aAny >>= nHoriOrient;
        if( HoriOrientation::NONE == nHoriOrient )
        {
            sal_Int32 nPos = 0;
            Any aAny = rPropSet->getPropertyValue( sHoriOrientPosition );
            aAny >>= nPos;
            GetExport().GetMM100UnitConverter().convertMeasure( sValue, nPos );
            GetExport().AddAttribute( XML_NAMESPACE_SVG, XML_X,
                                      sValue.makeStringAndClear() );
        }
    }
    else if( TextContentAnchorType_AS_CHARACTER == eAnchor )
        nShapeFeatures = (nShapeFeatures & ~SEF_EXPORT_X);

    if( !bShape || TextContentAnchorType_AS_CHARACTER == eAnchor  )
    {
        // svg:y
        sal_Int16 nVertOrient =  VertOrientation::NONE;
        aAny = rPropSet->getPropertyValue( sVertOrient );
        aAny >>= nVertOrient;
        if( VertOrientation::NONE == nVertOrient )
        {
            sal_Int32 nPos = 0;
            Any aAny = rPropSet->getPropertyValue( sVertOrientPosition );
            aAny >>= nPos;
            GetExport().GetMM100UnitConverter().convertMeasure( sValue, nPos );
            GetExport().AddAttribute( XML_NAMESPACE_SVG, XML_Y,
                                      sValue.makeStringAndClear() );
        }
        if( bShape )
            nShapeFeatures = (nShapeFeatures & ~SEF_EXPORT_Y);
    }


    Reference< XPropertySetInfo > xPropSetInfo = rPropSet->getPropertySetInfo();

    // svg:width
    if( xPropSetInfo->hasPropertyByName( sWidth ) )
    {
        sal_Int32 nWidth =  0;
        aAny = rPropSet->getPropertyValue( sWidth );
        aAny >>= nWidth;
        GetExport().GetMM100UnitConverter().convertMeasure( sValue, nWidth );
        GetExport().AddAttribute( XML_NAMESPACE_SVG, XML_WIDTH,
                                  sValue.makeStringAndClear() );
    }
    sal_Bool bSyncWidth = sal_False;
    if( xPropSetInfo->hasPropertyByName( sIsSyncWidthToHeight ) )
    {
        aAny = rPropSet->getPropertyValue( sIsSyncWidthToHeight );
        bSyncWidth = *(sal_Bool *)aAny.getValue();
        if( bSyncWidth )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_REL_WIDTH,
                                      XML_SCALE );
    }
    if( !bSyncWidth && xPropSetInfo->hasPropertyByName( sRelativeWidth ) )
    {
        sal_Int16 nRelWidth =  0;
        aAny = rPropSet->getPropertyValue( sRelativeWidth );
        aAny >>= nRelWidth;
        DBG_ASSERT( nRelWidth >= 0 && nRelWidth <= 254,
                    "Got illegal relative width from API" );
        if( nRelWidth > 0 )
        {
            GetExport().GetMM100UnitConverter().convertPercent( sValue,
                                                                nRelWidth );
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_REL_WIDTH,
                                      sValue.makeStringAndClear() );
        }
    }

    // svg:height, fo:min-height or style:rel-height
    sal_Int16 nSizeType = SizeType::FIX;
    if( xPropSetInfo->hasPropertyByName( sSizeType ) )
    {
        aAny = rPropSet->getPropertyValue( sSizeType );
        aAny >>= nSizeType;
    }
    sal_Bool bSyncHeight = sal_False;
    if( xPropSetInfo->hasPropertyByName( sIsSyncHeightToWidth ) )
    {
        aAny = rPropSet->getPropertyValue( sIsSyncHeightToWidth );
        bSyncHeight = *(sal_Bool *)aAny.getValue();
    }
    sal_Int16 nRelHeight =  0;
    if( !bSyncHeight && xPropSetInfo->hasPropertyByName( sRelativeHeight ) )
    {
        aAny = rPropSet->getPropertyValue( sRelativeHeight );
        aAny >>= nRelHeight;
    }
    if( xPropSetInfo->hasPropertyByName( sHeight ) )
    {
        sal_Int32 nHeight =  0;
        aAny = rPropSet->getPropertyValue( sHeight );
        aAny >>= nHeight;
        GetExport().GetMM100UnitConverter().convertMeasure( sValue,
                                                            nHeight );
        if( SizeType::FIX != nSizeType && 0==nRelHeight && !bSyncHeight )
            GetExport().AddAttribute( XML_NAMESPACE_FO, XML_MIN_HEIGHT,
                                      sValue.makeStringAndClear() );
        else
            GetExport().AddAttribute( XML_NAMESPACE_SVG, XML_HEIGHT,
                                      sValue.makeStringAndClear() );
    }
    if( bSyncHeight )
    {
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_REL_HEIGHT,
                SizeType::MIN == nSizeType ? XML_SCALE_MIN : XML_SCALE );

    }
    else if( nRelHeight > 0 )
    {
        GetExport().GetMM100UnitConverter().convertPercent( sValue,
                                                            nRelHeight );
        if( SizeType::MIN == nSizeType )
            GetExport().AddAttribute( XML_NAMESPACE_FO, XML_MIN_HEIGHT,
                                      sValue.makeStringAndClear() );
        else
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_REL_HEIGHT,
                                      sValue.makeStringAndClear() );
    }

    OUString sZOrder( RTL_CONSTASCII_USTRINGPARAM( "ZOrder" ) );
    if( xPropSetInfo->hasPropertyByName( sZOrder ) )
    {
        sal_Int32 nZIndex = 0;
        aAny = rPropSet->getPropertyValue( sZOrder );
        aAny >>= nZIndex;
        if( -1 != nZIndex )
        {
            GetExport().GetMM100UnitConverter().convertNumber( sValue,
                                                                nZIndex );
            GetExport().AddAttribute( XML_NAMESPACE_DRAW, XML_ZINDEX,
                                      sValue.makeStringAndClear() );
        }
    }

    return nShapeFeatures;
}

void XMLTextParagraphExport::exportAnyTextFrame(
        const Reference < XTextContent > & rTxtCntnt,
        FrameType eType,
        sal_Bool bAutoStyles,
        sal_Bool bProgress,
        const Reference < XPropertySet > *pRangePropSet)
{
    Reference < XPropertySet > xPropSet( rTxtCntnt, UNO_QUERY );

    if( bAutoStyles )
    {
        if( FT_EMBEDDED == eType )
            _collectTextEmbeddedAutoStyles( xPropSet );
        else
            Add( XML_STYLE_FAMILY_TEXT_FRAME, xPropSet );

        if( pRangePropSet && lcl_txtpara_isBoundAsChar( xPropSet,
                                            xPropSet->getPropertySetInfo() ) )
            Add( XML_STYLE_FAMILY_TEXT_TEXT, *pRangePropSet );

        switch( eType )
        {
        case FT_TEXT:
            {
                // frame bound frames
                Reference < XTextFrame > xTxtFrame( rTxtCntnt, UNO_QUERY );
                Reference < XText > xTxt = xTxtFrame->getText();
                collectFramesBoundToFrameAutoStyles( xTxtFrame, bProgress );

                exportText( xTxt, bAutoStyles, bProgress, sal_True );
            }
            break;
        case FT_SHAPE:
            {
                Reference < XShape > xShape( rTxtCntnt, UNO_QUERY );
                GetExport().GetShapeExport()->collectShapeAutoStyles( xShape );
            }
            break;
        }
    }
    else
    {
        Reference< XPropertySetInfo > xPropSetInfo =
            xPropSet->getPropertySetInfo();
        Reference< XPropertyState > xPropState( xPropSet, UNO_QUERY );
        {
            sal_Bool bAddCharStyles = pRangePropSet &&
                lcl_txtpara_isBoundAsChar( xPropSet, xPropSetInfo );
            sal_Bool bIsUICharStyle;
            OUString sStyle;
            if( bAddCharStyles )
                   sStyle = FindTextStyle( *pRangePropSet, bIsUICharStyle );
            else
                bIsUICharStyle = sal_False;

            XMLTextCharStyleNamesElementExport aCharStylesExport(
                GetExport(), bIsUICharStyle &&
                             aCharStyleNamesPropInfoCache.hasProperty(
                                            *pRangePropSet ),
                *pRangePropSet, sCharStyleNames );

            if( sStyle.getLength() )
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                                          sStyle );
            {
                SvXMLElementExport aElem( GetExport(), sStyle.getLength() > 0,
                    XML_NAMESPACE_TEXT, XML_SPAN, sal_False, sal_False );
                {
                    SvXMLElementExport aElem( GetExport(),
                        FT_SHAPE != eType &&
                        addHyperlinkAttributes( xPropSet,
                                                xPropState,xPropSetInfo ),
                        XML_NAMESPACE_DRAW, XML_A, sal_False, sal_False );
                    switch( eType )
                    {
                    case FT_TEXT:
                        _exportTextFrame( xPropSet, xPropSetInfo, bProgress );
                        break;
                    case FT_GRAPHIC:
                        _exportTextGraphic( xPropSet, xPropSetInfo );
                        break;
                    case FT_EMBEDDED:
                        _exportTextEmbedded( xPropSet, xPropSetInfo );
                        break;
                    case FT_SHAPE:
                        {
                            Reference < XShape > xShape( rTxtCntnt, UNO_QUERY );
                            sal_Int32 nFeatures =
                                addTextFrameAttributes( xPropSet, sal_True );
                            GetExport().GetShapeExport()
                                ->exportShape( xShape, nFeatures );
                        }
                        break;
                    }
                }
            }
        }
    }
}

void XMLTextParagraphExport::_exportTextFrame(
        const Reference < XPropertySet > & rPropSet,
        const Reference < XPropertySetInfo > & rPropSetInfo,
        sal_Bool bProgress )
{
    Reference < XTextFrame > xTxtFrame( rPropSet, UNO_QUERY );
    Reference < XText > xTxt = xTxtFrame->getText();

    OUString sStyle;
    Any aAny;
    if( rPropSetInfo->hasPropertyByName( sFrameStyleName ) )
    {
        aAny = rPropSet->getPropertyValue( sFrameStyleName );
        aAny >>= sStyle;
    }

    OUString sAutoStyle( sStyle );
    sAutoStyle = Find( XML_STYLE_FAMILY_TEXT_FRAME, rPropSet, sStyle );
    if( sAutoStyle.getLength() )
        GetExport().AddAttribute( XML_NAMESPACE_DRAW, XML_STYLE_NAME,
                                  sAutoStyle );
    addTextFrameAttributes( rPropSet, sal_False );

    // draw:chain-next-name
    if( rPropSetInfo->hasPropertyByName( sChainNextName ) )
    {
        OUString sNext;
        aAny = rPropSet->getPropertyValue( sChainNextName );
        if( (aAny >>= sNext) && sNext.getLength() > 0 )
            GetExport().AddAttribute( XML_NAMESPACE_DRAW,
                                      XML_CHAIN_NEXT_NAME,
                                      sNext );
    }

    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_DRAW,
                              XML_TEXT_BOX, sal_False, sal_True );

    // frame bound frames
    exportFramesBoundToFrame( xTxtFrame, bProgress );

    // script:events
    Reference<XEventsSupplier> xEventsSupp( xTxtFrame, UNO_QUERY );
    GetExport().GetEventExport().Export(xEventsSupp);

    // image map
    GetExport().GetImageMapExport().Export( rPropSet );

    exportText( xTxt, sal_False, bProgress, sal_True );
}

void XMLTextParagraphExport::exportContour(
        const Reference < XPropertySet > & rPropSet,
        const Reference < XPropertySetInfo > & rPropSetInfo )
{
    if( !rPropSetInfo->hasPropertyByName( sContourPolyPolygon ) )
        return;

    Any aAny = rPropSet->getPropertyValue( sContourPolyPolygon );

    PointSequenceSequence aSourcePolyPolygon;
    aAny >>= aSourcePolyPolygon;

    if( !aSourcePolyPolygon.getLength() )
        return;

    awt::Point aPoint( 0, 0 );
    awt::Size aSize( 0, 0 );
    sal_Int32 nPolygons = aSourcePolyPolygon.getLength();
    const PointSequence *pPolygons = aSourcePolyPolygon.getConstArray();
    while( nPolygons-- )
    {
        sal_Int32 nPoints = pPolygons->getLength();
        const awt::Point *pPoints = pPolygons->getConstArray();
        while( nPoints-- )
        {
            if( aSize.Width < pPoints->X )
                aSize.Width = pPoints->X;
            if( aSize.Height < pPoints->Y )
                aSize.Height = pPoints->Y;
            pPoints++;
        }
        pPolygons++;
    }

    sal_Bool bPixel = sal_False;
    if( rPropSetInfo->hasPropertyByName( sIsPixelContour ) )
    {
        aAny = rPropSet->getPropertyValue( sIsPixelContour );
        bPixel = *(sal_Bool *)aAny.getValue();
    }

    // svg: width
    OUStringBuffer aStringBuffer( 10 );
    if( bPixel )
        GetExport().GetMM100UnitConverter().convertMeasurePx(aStringBuffer, aSize.Width);
    else
        GetExport().GetMM100UnitConverter().convertMeasure(aStringBuffer, aSize.Width);
    GetExport().AddAttribute( XML_NAMESPACE_SVG, XML_WIDTH,
                              aStringBuffer.makeStringAndClear() );

    // svg: height
    if( bPixel )
        GetExport().GetMM100UnitConverter().convertMeasurePx(aStringBuffer, aSize.Height);
    else
        GetExport().GetMM100UnitConverter().convertMeasure(aStringBuffer, aSize.Height);
    GetExport().AddAttribute( XML_NAMESPACE_SVG, XML_HEIGHT,
                              aStringBuffer.makeStringAndClear() );

    // svg:viewbox
    SdXMLImExViewBox aViewBox(0, 0, aSize.Width, aSize.Height);
    GetExport().AddAttribute(XML_NAMESPACE_SVG, XML_VIEWBOX,
                aViewBox.GetExportString(GetExport().GetMM100UnitConverter()));

    sal_Int32 nOuterCnt( aSourcePolyPolygon.getLength() );

    enum XMLTokenEnum eElem = XML_TOKEN_INVALID;
    if( 1L == nOuterCnt )
    {
        // simple polygon shape, can be written as svg:points sequence
        /*const*/ PointSequence* pSequence =
                            (PointSequence*)aSourcePolyPolygon.getConstArray();

        SdXMLImExPointsElement aPoints( pSequence, aViewBox, aPoint,
                                aSize, GetExport().GetMM100UnitConverter() );

        // write point array
        GetExport().AddAttribute( XML_NAMESPACE_DRAW, XML_POINTS,
                                      aPoints.GetExportString());
        eElem = XML_CONTOUR_POLYGON;
    }
    else
    {
        // polypolygon, needs to be written as a svg:path sequence
        /*const*/ PointSequence* pOuterSequence =
                        (PointSequence*)aSourcePolyPolygon.getConstArray();
        if(pOuterSequence)
        {
            // prepare svx:d element export
            SdXMLImExSvgDElement aSvgDElement( aViewBox );

            for(sal_Int32 a(0L); a < nOuterCnt; a++)
            {
                /*const*/ PointSequence* pSequence = pOuterSequence++;
                if(pSequence)
                {
                    aSvgDElement.AddPolygon(pSequence, 0L, aPoint,
                        aSize, GetExport().GetMM100UnitConverter(),
                        sal_True );
                }
            }

            // write point array
            GetExport().AddAttribute( XML_NAMESPACE_SVG, XML_D,
                                      aSvgDElement.GetExportString());
            eElem = XML_CONTOUR_PATH;
        }
    }

    if( rPropSetInfo->hasPropertyByName( sIsAutomaticContour ) )
    {
        aAny = rPropSet->getPropertyValue( sIsAutomaticContour );
        if( *(sal_Bool *)aAny.getValue() )
            GetExport().AddAttribute( XML_NAMESPACE_DRAW,
                                      XML_RECREATE_ON_EDIT, XML_TRUE );
    }

    // write object now
    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_DRAW, eElem,
                              sal_True, sal_True );
}

void XMLTextParagraphExport::_exportTextGraphic(
        const Reference < XPropertySet > & rPropSet,
        const Reference < XPropertySetInfo > & rPropSetInfo )
{
    OUString sStyle;
    Any aAny;
    if( rPropSetInfo->hasPropertyByName( sFrameStyleName ) )
    {
        aAny = rPropSet->getPropertyValue( sFrameStyleName );
        aAny >>= sStyle;
    }

    OUString sAutoStyle( sStyle );
    sAutoStyle = Find( XML_STYLE_FAMILY_TEXT_FRAME, rPropSet, sStyle );
    if( sAutoStyle.getLength() )
        GetExport().AddAttribute( XML_NAMESPACE_DRAW, XML_STYLE_NAME,
                                  sAutoStyle );
    addTextFrameAttributes( rPropSet, sal_False );

    // xlink:href
    OUString sOrigURL;
    aAny = rPropSet->getPropertyValue( sGraphicURL );
    aAny >>= sOrigURL;
    OUString sURL = GetExport().AddEmbeddedGraphicObject( sOrigURL );
    setTextEmbeddedGraphicURL( rPropSet, sURL );

    // If there still is no url, then then graphic is empty
    if( sURL.getLength() )
    {
        GetExport().AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, sURL );
        GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
        GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, 
                                                       XML_ONLOAD );
    }

    // draw:filter-name
    OUString sGrfFilter;
    aAny = rPropSet->getPropertyValue( sGraphicFilter );
    aAny >>= sGrfFilter;
    if( sGrfFilter.getLength() )
        GetExport().AddAttribute( XML_NAMESPACE_DRAW, XML_FILTER_NAME,
                                  sGrfFilter );

    // svg:transform
    aAny = rPropSet->getPropertyValue( sGraphicRotation );
    sal_Int16 nVal;
    aAny >>= nVal;
    if( nVal != 0 )
    {
        OUStringBuffer sRet( GetXMLToken(XML_ROTATE).getLength()+4 );
        sRet.append( GetXMLToken(XML_ROTATE));
        sRet.append( (sal_Unicode)'(' );
        GetExport().GetMM100UnitConverter().convertNumber( sRet, (sal_Int32)nVal );
        sRet.append( (sal_Unicode)')' );
        GetExport().AddAttribute( XML_NAMESPACE_SVG, XML_TRANSFORM,
                                  sRet.makeStringAndClear() );
    }

    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_DRAW,
                              XML_IMAGE, sal_False, sal_True );

    // optional office:binary-data
    GetExport().AddEmbeddedGraphicObjectAsBase64( sOrigURL );

    // script:events
    Reference<XEventsSupplier> xEventsSupp( rPropSet, UNO_QUERY );
    GetExport().GetEventExport().Export(xEventsSupp);

    // svg:desc
    exportAlternativeText( rPropSet, rPropSetInfo );
    /*
    OUString sAltText;
    aAny = rPropSet->getPropertyValue( sAlternativeText );
    aAny >>= sAltText;
    if( sAltText.getLength() )
    {
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_SVG,
                                  XML_DESC, sal_True, sal_False );
        GetExport().Characters( sAltText );
    }
    */

    // image map
    GetExport().GetImageMapExport().Export( rPropSet );

    // draw:contour
    exportContour( rPropSet, rPropSetInfo );
}

void XMLTextParagraphExport::_collectTextEmbeddedAutoStyles(
        const Reference < XPropertySet > & rPropSet )
{
    DBG_ASSERT( !this, "no API implementation avialable" );
}

void XMLTextParagraphExport::_exportTextEmbedded(
        const Reference < XPropertySet > & rPropSet,
        const Reference < XPropertySetInfo > & rPropSetInfo )
{
    DBG_ASSERT( !this, "no API implementation avialable" );
}

void XMLTextParagraphExport::exportEvents( const Reference < XPropertySet > & rPropSet )
{
    // script:events
    Reference<XEventsSupplier> xEventsSupp( rPropSet, UNO_QUERY );
    GetExport().GetEventExport().Export(xEventsSupp);

    // image map
    OUString sImageMap(RTL_CONSTASCII_USTRINGPARAM("ImageMap"));
    if (rPropSet->getPropertySetInfo()->hasPropertyByName(sImageMap))
        GetExport().GetImageMapExport().Export( rPropSet );
}
void XMLTextParagraphExport::exportAlternativeText( 
        const Reference < XPropertySet > & rPropSet,
        const Reference < XPropertySetInfo > & rPropSetInfo )
{
    // svg:desc
    if( rPropSetInfo->hasPropertyByName( sAlternativeText  ) )
    {
        OUString sAltText;
        Any aAny = rPropSet->getPropertyValue( sAlternativeText );
        aAny >>= sAltText;
        if( sAltText.getLength() )
        {
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_SVG,
                                      XML_DESC, sal_True, sal_False );
            GetExport().Characters( sAltText );
        }
    }
}

void XMLTextParagraphExport::setTextEmbeddedGraphicURL(
    const Reference < XPropertySet >& rPropSet,
    OUString& rStreamName ) const
{
}

sal_Bool XMLTextParagraphExport::addHyperlinkAttributes(
        const Reference < XPropertySet > & rPropSet,
        const Reference < XPropertyState > & rPropState,
        const Reference < XPropertySetInfo > & rPropSetInfo )
{
    sal_Bool bExport = sal_False;
    Any aAny;
    OUString sHRef, sName, sTargetFrame, sUStyleName, sVStyleName;
    sal_Bool bServerMap = sal_False;

    if( rPropSetInfo->hasPropertyByName( sHyperLinkURL ) &&
        ( !rPropState.is() || PropertyState_DIRECT_VALUE ==
                    rPropState->getPropertyState( sHyperLinkURL ) ) )
    {
        aAny= rPropSet->getPropertyValue( sHyperLinkURL );
        aAny >>= sHRef;

        if( sHRef.getLength() > 0 )
            bExport = sal_True;
    }

    if( rPropSetInfo->hasPropertyByName( sHyperLinkName ) &&
        ( !rPropState.is() || PropertyState_DIRECT_VALUE ==
                    rPropState->getPropertyState( sHyperLinkName ) ) )
    {
        aAny = rPropSet->getPropertyValue( sHyperLinkName );
        aAny >>= sName;
        if( sName.getLength() > 0 )
            bExport = sal_True;
    }

    if( rPropSetInfo->hasPropertyByName( sHyperLinkTarget ) &&
        ( !rPropState.is() || PropertyState_DIRECT_VALUE ==
                    rPropState->getPropertyState( sHyperLinkTarget ) ) )
    {
        aAny = rPropSet->getPropertyValue( sHyperLinkTarget );
        aAny >>= sTargetFrame;
        if( sTargetFrame.getLength() )
            bExport = sal_True;
    }

    if( rPropSetInfo->hasPropertyByName( sServerMap ) &&
        ( !rPropState.is() || PropertyState_DIRECT_VALUE ==
                    rPropState->getPropertyState( sServerMap ) ) )
    {
        aAny = rPropSet->getPropertyValue( sServerMap );
        bServerMap = *(sal_Bool *)aAny.getValue();
        if( bServerMap  )
            bExport = sal_True;
    }

    if( rPropSetInfo->hasPropertyByName( sUnvisitedCharStyleName ) &&
        ( !rPropState.is() || PropertyState_DIRECT_VALUE ==
            rPropState->getPropertyState( sUnvisitedCharStyleName ) ) )
    {
        aAny = rPropSet->getPropertyValue( sUnvisitedCharStyleName );
        aAny >>= sUStyleName;
        if( sUStyleName.getLength() )
            bExport = sal_True;
    }

    if( rPropSetInfo->hasPropertyByName( sVisitedCharStyleName ) &&
        ( !rPropState.is() || PropertyState_DIRECT_VALUE ==
            rPropState->getPropertyState( sVisitedCharStyleName ) ) )
    {
        aAny = rPropSet->getPropertyValue( sVisitedCharStyleName );
        aAny >>= sVStyleName;
        if( sVStyleName.getLength() )
            bExport = sal_True;
    }

    if( bExport )
    {
        GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, GetExport().GetRelativeReference( sHRef ) );

        if( sName.getLength() > 0 )
            GetExport().AddAttribute( XML_NAMESPACE_OFFICE, XML_NAME, sName );

        if( sTargetFrame.getLength() )
        {
            GetExport().AddAttribute( XML_NAMESPACE_OFFICE,
                                      XML_TARGET_FRAME_NAME, sTargetFrame );
            enum XMLTokenEnum eTok =
                sTargetFrame.equalsAsciiL( "_blank", sizeof("_blank")-1 )
                    ? XML_NEW : XML_REPLACE;
            GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, eTok );
        }

        if( bServerMap  )
            GetExport().AddAttribute( XML_NAMESPACE_OFFICE,
                                      XML_SERVER_MAP, XML_TRUE );

        if( sUStyleName.getLength() )
            GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                      XML_STYLE_NAME, sUStyleName );

        if( sVStyleName.getLength() )
            GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                      XML_VISITED_STYLE_NAME, sVStyleName );
    }

    return bExport;
}

void XMLTextParagraphExport::exportTextRange(
        const Reference < XTextRange > & rTextRange,
        sal_Bool bAutoStyles,
        sal_Bool& rPrevCharIsSpace )
{
    Reference < XPropertySet > xPropSet( rTextRange, UNO_QUERY );
    if( bAutoStyles )
    {
        Add( XML_STYLE_FAMILY_TEXT_TEXT, xPropSet );
    }
    else
    {
        sal_Bool bHyperlink = sal_False, bIsUICharStyle = sal_False;
        OUString sStyle = FindTextStyleAndHyperlink( xPropSet, bHyperlink,
                                                        bIsUICharStyle );
        Reference < XPropertySetInfo > xPropSetInfo;
        if( bHyperlink )
        {
            Reference< XPropertyState > xPropState( xPropSet, UNO_QUERY );
            xPropSetInfo = xPropSet->getPropertySetInfo();
            bHyperlink = addHyperlinkAttributes( xPropSet, xPropState, xPropSetInfo );
        }
        SvXMLElementExport aElem( GetExport(), bHyperlink, XML_NAMESPACE_TEXT,
                                  XML_A, sal_False, sal_False );
        if( bHyperlink )
        {
            // export events (if supported)
            OUString sHyperLinkEvents(RTL_CONSTASCII_USTRINGPARAM(
                "HyperLinkEvents"));
            if (xPropSetInfo->hasPropertyByName(sHyperLinkEvents))
            {
                Any aAny = xPropSet->getPropertyValue(sHyperLinkEvents);
                Reference<XNameReplace> xName;
                aAny >>= xName;
                GetExport().GetEventExport().Export(xName, sal_False);
            }
        }

        {
            XMLTextCharStyleNamesElementExport aCharStylesExport(
                GetExport(), bIsUICharStyle && 
                             aCharStyleNamesPropInfoCache.hasProperty(
                                                    xPropSet, xPropSetInfo ),
                xPropSet, sCharStyleNames );

            OUString sText = rTextRange->getString();
            if( sStyle.getLength() )
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                                          sStyle );
            {
                // in a block to make sure it is destroyed before the text:a element
                SvXMLElementExport aElem( GetExport(), sStyle.getLength() > 0,
                                          XML_NAMESPACE_TEXT, XML_SPAN, sal_False,
                                          sal_False );
                exportText( sText, rPrevCharIsSpace );
            }
        }
    }
}

void XMLTextParagraphExport::exportText( const OUString& rText,
                                           sal_Bool& rPrevCharIsSpace )
{
    sal_Int32 nExpStartPos = 0L;
    sal_Int32 nEndPos = rText.getLength();
    sal_Int32 nSpaceChars = 0;
    for( sal_Int32 nPos = 0; nPos < nEndPos; nPos++ )
    {
        sal_Unicode cChar = rText[nPos];
        sal_Bool bExpCharAsText = sal_True;
        sal_Bool bExpCharAsElement = sal_False;
        sal_Bool bCurrCharIsSpace = sal_False;
        switch( cChar )
        {
        case 0x0009:	// Tab
        case 0x000A:	// LF
            // These characters are exported as text.
            bExpCharAsElement = sal_True;
            bExpCharAsText = sal_False;
            break;
        case 0x000D:
            break;	// legal character
        case 0x0020:	// Blank
            if( rPrevCharIsSpace )
            {
                // If the previous character is a space character,
                // too, export a special space element.
                bExpCharAsText = sal_False;
            }
            bCurrCharIsSpace = sal_True;
            break;
        default:
            if( cChar < 0x0020 )
            {
#ifdef DBG_UTIL
                OSL_ENSURE( txtparae_bContainsIllegalCharacters ||
                            cChar >= 0x0020,
                            "illegal character in text content" );
                txtparae_bContainsIllegalCharacters = sal_True;
#endif
                bExpCharAsText = sal_False;
            }
            break;
        }

        // If the current character is not exported as text
           // the text that has not been exported by now has to be exported now.
        if( nPos > nExpStartPos && !bExpCharAsText )
        {
            DBG_ASSERT( 0==nSpaceChars, "pending spaces" );
            OUString sExp( rText.copy( nExpStartPos, nPos - nExpStartPos ) );
            GetExport().Characters( sExp );
            nExpStartPos = nPos;
        }

        // If there are spaces left that have not been exported and the
        // current chracter is not a space , the pending spaces have to be
        // exported now.
        if( nSpaceChars > 0 && !bCurrCharIsSpace )
        {
            DBG_ASSERT( nExpStartPos == nPos, " pending characters" );

            if( nSpaceChars > 1 )
            {
                OUStringBuffer sTmp;
                sTmp.append( (sal_Int32)nSpaceChars );
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_C,
                              sTmp.makeStringAndClear() );
            }

            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                      XML_S, sal_False, sal_False );

            nSpaceChars = 0;
        }

        // If the current character has to be exported as a special
        // element, the elemnt will be exported now.
        if( bExpCharAsElement )
        {
            switch( cChar )
            {
            case 0x0009:	// Tab
                {
                    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                              XML_TAB_STOP, sal_False,
                                              sal_False );
                }
                break;
            case 0x000A:	// LF
                {
                    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                              XML_LINE_BREAK, sal_False,
                                              sal_False );
                }
                break;
            }
        }

        // If the current character is a space, and the previous one
        // is a space, too, the number of pending spaces is incremented
        // only.
        if( bCurrCharIsSpace && rPrevCharIsSpace )
            nSpaceChars++;
        rPrevCharIsSpace = bCurrCharIsSpace;

        // If the currect character is not exported as text, the start
        // position for text is the position behind the current position.
        if( !bExpCharAsText )
        {
            DBG_ASSERT( nExpStartPos == nPos, "wrong export start pos" );
            nExpStartPos = nPos+1;
        }
    }

    if( nExpStartPos < nEndPos )
    {
        DBG_ASSERT( 0==nSpaceChars, " pending spaces " );
        OUString sExp( rText.copy( nExpStartPos, nEndPos - nExpStartPos ) );
        GetExport().Characters( sExp );
    }

    // If there are some spaces left, they have to be exported now.
    if( nSpaceChars > 0 )
    {
        if( nSpaceChars > 1 )
        {
            OUStringBuffer sTmp;
            sTmp.append( (sal_Int32)nSpaceChars );
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_C,
                          sTmp.makeStringAndClear() );
        }

        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT, XML_S,
                                  sal_False, sal_False );
    }
}

void XMLTextParagraphExport::exportTextDeclarations()
{
    pFieldExport->ExportFieldDeclarations();

    // get XPropertySet from the document and ask for AutoMarkFileURL. 
    // If it exists, export the auto-mark-file element.
    Reference<XPropertySet> xPropertySet( GetExport().GetModel(), UNO_QUERY );
    if (xPropertySet.is())
    {
        OUString sUrl;
        OUString sIndexAutoMarkFileURL(
            RTL_CONSTASCII_USTRINGPARAM("IndexAutoMarkFileURL"));
        if (xPropertySet->getPropertySetInfo()->hasPropertyByName(
            sIndexAutoMarkFileURL))
        {
            Any aAny = xPropertySet->getPropertyValue(sIndexAutoMarkFileURL);
            aAny >>= sUrl;
            if (sUrl.getLength() > 0)
            {
                GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, 
                                          GetExport().GetRelativeReference(sUrl) );
                SvXMLElementExport aAutoMarkElement( 
                    GetExport(), XML_NAMESPACE_TEXT, 
                    XML_ALPHABETICAL_INDEX_AUTO_MARK_FILE,
                    sal_True, sal_True );
            }
        }
    }
}

void XMLTextParagraphExport::exportTextDeclarations(
    const Reference<XText> & rText )
{
    pFieldExport->ExportFieldDeclarations(rText);
}

void XMLTextParagraphExport::exportUsedDeclarations( sal_Bool bOnlyUsed )
{
    pFieldExport->SetExportOnlyUsedFieldDeclarations( bOnlyUsed );
}


void XMLTextParagraphExport::exportTrackedChanges(sal_Bool bAutoStyles)
{
    if (NULL != pRedlineExport)
        pRedlineExport->ExportChangesList( bAutoStyles );
}

void XMLTextParagraphExport::exportTrackedChanges(
    const Reference<XText> & rText,
    sal_Bool bAutoStyle)
{
    if (NULL != pRedlineExport)
        pRedlineExport->ExportChangesList(rText, bAutoStyle);
}

void XMLTextParagraphExport::recordTrackedChangesForXText(
    const Reference<XText> & rText )
{
    if (NULL != pRedlineExport)
        pRedlineExport->SetCurrentXText(rText);
}

void XMLTextParagraphExport::recordTrackedChangesNoXText()
{
    if (NULL != pRedlineExport)
        pRedlineExport->SetCurrentXText();
}


void XMLTextParagraphExport::exportTextAutoStyles()
{
    GetAutoStylePool().exportXML( XML_STYLE_FAMILY_TEXT_PARAGRAPH,
                                   GetExport().GetDocHandler(),
                                   GetExport().GetMM100UnitConverter(),
                                   GetExport().GetNamespaceMap() );

    GetAutoStylePool().exportXML( XML_STYLE_FAMILY_TEXT_TEXT,
                                   GetExport().GetDocHandler(),
                                   GetExport().GetMM100UnitConverter(),
                                   GetExport().GetNamespaceMap() );

    GetAutoStylePool().exportXML( XML_STYLE_FAMILY_TEXT_FRAME,
                                   GetExport().GetDocHandler(),
                                   GetExport().GetMM100UnitConverter(),
                                   GetExport().GetNamespaceMap() );

    GetAutoStylePool().exportXML( XML_STYLE_FAMILY_TEXT_SECTION,
                                  GetExport().GetDocHandler(),
                                  GetExport().GetMM100UnitConverter(),
                                  GetExport().GetNamespaceMap() );

    GetAutoStylePool().exportXML( XML_STYLE_FAMILY_TEXT_RUBY,
                                  GetExport().GetDocHandler(),
                                  GetExport().GetMM100UnitConverter(),
                                  GetExport().GetNamespaceMap() );

    pListAutoPool->exportXML();
}

void XMLTextParagraphExport::exportRuby(
    const Reference<XPropertySet> & rPropSet,
    sal_Bool bAutoStyles )
{
    // early out: a collapsed ruby makes no sense
    Any aAny = rPropSet->getPropertyValue(sIsCollapsed);
    if (*(sal_Bool*)aAny.getValue())
        return;

    // start value ?
    aAny = rPropSet->getPropertyValue(sIsStart);
    sal_Bool bStart = (*(sal_Bool*)aAny.getValue());

    if (bAutoStyles)
    {
        // ruby auto styles
        if (bStart)
            Add( XML_STYLE_FAMILY_TEXT_RUBY, rPropSet );
    }
    else
    {
        // prepare element names
        OUString sRuby(GetXMLToken(XML_RUBY));
        OUString sTextRuby(GetExport().GetNamespaceMap().
                           GetQNameByKey(XML_NAMESPACE_TEXT, sRuby));
        OUString sRubyBase(GetXMLToken(XML_RUBY_BASE));
        OUString sTextRubyBase(GetExport().GetNamespaceMap().
                           GetQNameByKey(XML_NAMESPACE_TEXT, sRubyBase));

        if (bStart)
        {
            // ruby start

            // we can only start a ruby if none is open
            DBG_ASSERT(! bOpenRuby, "Can't open a ruby inside of ruby!");
            if( bOpenRuby )
                return;

            // save ruby text + ruby char style
            aAny = rPropSet->getPropertyValue(sRubyText);
            aAny >>= sOpenRubyText;
            aAny = rPropSet->getPropertyValue(sRubyCharStyleName);
            aAny >>= sOpenRubyCharStyle;

            // ruby style
            GetExport().CheckAttrList();
            OUString sEmpty;
            OUString sStyleName = Find( XML_STYLE_FAMILY_TEXT_RUBY, rPropSet, 
                                        sEmpty );
            DBG_ASSERT(sStyleName.getLength() > 0, "I can't find the style!");
            GetExport().AddAttribute(XML_NAMESPACE_TEXT, 
                                     XML_STYLE_NAME, sStyleName);

            // export <text:ruby> and <text:ruby-base> start elements
            GetExport().StartElement( XML_NAMESPACE_TEXT, XML_RUBY, sal_False);
            GetExport().ClearAttrList();
            GetExport().StartElement( XML_NAMESPACE_TEXT, XML_RUBY_BASE, 
                                      sal_False );
            bOpenRuby = sal_True;
        }
        else
        {
            // ruby end

            // check for an open ruby
            DBG_ASSERT(bOpenRuby, "Can't close a ruby if none is open!");
            if( !bOpenRuby )
                return;

            // close <text:ruby-base>
            GetExport().EndElement(XML_NAMESPACE_TEXT, XML_RUBY_BASE, 
                                   sal_False);

            // write the ruby text (with char style)
            {
                if (sOpenRubyCharStyle.getLength() > 0)
                    GetExport().AddAttribute(
                        XML_NAMESPACE_TEXT, XML_STYLE_NAME, 
                        sOpenRubyCharStyle);

                SvXMLElementExport aRuby(
                    GetExport(), XML_NAMESPACE_TEXT, XML_RUBY_TEXT,
                    sal_False, sal_False);

                GetExport().Characters(sOpenRubyText);
            }

            // and finally, close the ruby
            GetExport().EndElement(XML_NAMESPACE_TEXT, XML_RUBY, sal_False);
            bOpenRuby = sal_False;
        }
    }
}




void XMLTextParagraphExport::PreventExportOfControlsInMuteSections( 
    const Reference<XIndexAccess> & rShapes,
    UniReference<xmloff::OFormLayerXMLExport> xFormExport   )
{
    // check parameters ad pre-conditions
    if( ( ! rShapes.is() ) || ( ! xFormExport.is() ) )
    {
        // if we don't have shapes or a form export, there's nothing to do
        return;
    }
    DBG_ASSERT( pSectionExport != NULL, "We need the section export." );

    sal_Int32 nShapes = xShapes->getCount();
    for( sal_Int32 i = 0; i < nShapes; i++ )
    {
        // now we need to check
        // 1) if this is a control shape, and
        // 2) if it's in a mute section
        // if both answers are 'yes', notify the form layer export

        // we join accessing the shape and testing for control
        Reference<XControlShape> xControlShape;
        xShapes->getByIndex( i ) >>= xControlShape;
        if( xControlShape.is() )
        {
            //            Reference<XPropertySet> xPropSet( xControlShape, UNO_QUERY );
            //            Reference<XTextContent> xTextContent;
            //            xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "TextRange" ) ) ) >>= xTextContent;

            Reference<XTextContent> xTextContent( xControlShape, UNO_QUERY );
            if( xTextContent.is() )
            {
                if( pSectionExport->IsMuteSection( xTextContent, sal_False ) )
                {
                    // Ah, we've found a shape that
                    // 1) is a control shape
                    // 2) is anchored in a mute section
                    // so: don't export it!
                    xFormExport->excludeFromExport(
                        xControlShape->getControl() );
                }
                // else: not in mute section -> should be exported -> nothing
                // to do
            }
            // else: no anchor -> ignore
        }
        // else: no control shape -> nothing to do
    }
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
