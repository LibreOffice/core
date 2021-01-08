/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <xmloff/xmlimp.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include "XMLTextListItemContext.hxx"
#include "XMLTextListBlockContext.hxx"
#include <txtlists.hxx>
#include <sal/log.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;


// OD 2008-05-07 #refactorlists#
// add optional parameter <bRestartNumberingAtSubList> and its handling
XMLTextListBlockContext::XMLTextListBlockContext(
        SvXMLImport& rImport,
        XMLTextImportHelper& rTxtImp,
        const Reference< xml::sax::XFastAttributeList > & xAttrList,
        const bool bRestartNumberingAtSubList )
:   SvXMLImportContext( rImport )
,   mrTxtImport( rTxtImp )
,   msListStyleName()
,   mxParentListBlock( )
,   mnLevel( 0 )
,   mbRestartNumbering( false )
,   mbSetDefaults( false )
,   msListId()
,   msContinueListId()
{
    static constexpr OUStringLiteral s_PropNameDefaultListId = u"DefaultListId";
    {
        // get the parent list block context (if any); this is a bit ugly...
        XMLTextListBlockContext * pLB(nullptr);
        XMLTextListItemContext  * pLI(nullptr);
        XMLNumberedParaContext  * pNP(nullptr);
        rTxtImp.GetTextListHelper().ListContextTop(pLB, pLI, pNP);
        mxParentListBlock = pLB;
    }
    // Inherit style name from parent list, as well as the flags whether
    // numbering must be restarted and formats have to be created.
    OUString sParentListStyleName;
    if( mxParentListBlock.is() )
    {
        XMLTextListBlockContext *pParent = mxParentListBlock.get();
        msListStyleName = pParent->msListStyleName;
        sParentListStyleName = msListStyleName;
        mxNumRules = pParent->GetNumRules();
        mnLevel = pParent->GetLevel() + 1;
        mbRestartNumbering = pParent->IsRestartNumbering() ||
                             bRestartNumberingAtSubList;
        mbSetDefaults = pParent->mbSetDefaults;
        msListId = pParent->GetListId();
        msContinueListId = pParent->GetContinueListId();
    }

    bool bIsContinueNumberingAttributePresent( false );
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch( aIter.getToken() )
        {
        case XML_ELEMENT(XML, XML_ID):
//FIXME: there is no UNO API for lists
            // xml:id is also the list ID (#i92221#)
            if ( mnLevel == 0 ) // root <list> element
            {
                msListId = aIter.toString();
            }
            break;
        case XML_ELEMENT(TEXT, XML_CONTINUE_NUMBERING):
            mbRestartNumbering = !IsXMLToken(aIter, XML_TRUE);
            bIsContinueNumberingAttributePresent = true;
            break;
        case XML_ELEMENT(TEXT, XML_STYLE_NAME):
            msListStyleName = aIter.toString();
            break;
        case XML_ELEMENT(TEXT, XML_CONTINUE_LIST):
            if ( mnLevel == 0 ) // root <list> element
            {
                msContinueListId = aIter.toString();
            }
            break;
        default:
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    // Remember this list block.
    mrTxtImport.GetTextListHelper().PushListContext( this );

    mxNumRules = XMLTextListsHelper::MakeNumRule(GetImport(), mxNumRules,
        sParentListStyleName, msListStyleName,
        mnLevel, &mbRestartNumbering, &mbSetDefaults );
    if( !mxNumRules.is() )
        return;

    if ( mnLevel != 0 ) // root <list> element
        return;

    XMLTextListsHelper& rTextListsHelper( mrTxtImport.GetTextListHelper() );
    // Inconsistent behavior regarding lists (#i92811#)
    OUString sListStyleDefaultListId;
    {
        uno::Reference< beans::XPropertySet > xNumRuleProps( mxNumRules, UNO_QUERY );
        if ( xNumRuleProps.is() )
        {
            uno::Reference< beans::XPropertySetInfo > xNumRulePropSetInfo(
                                        xNumRuleProps->getPropertySetInfo());
            if (xNumRulePropSetInfo.is() &&
                xNumRulePropSetInfo->hasPropertyByName(
                     s_PropNameDefaultListId))
            {
                xNumRuleProps->getPropertyValue(s_PropNameDefaultListId)
                    >>= sListStyleDefaultListId;
                SAL_WARN_IF( sListStyleDefaultListId.isEmpty(), "xmloff",
                            "no default list id found at numbering rules instance. Serious defect." );
            }
        }
    }
    if ( msListId.isEmpty() )  // no text:id property found
    {
        sal_Int32 nUPD( 0 );
        sal_Int32 nBuild( 0 );
        const bool bBuildIdFound = GetImport().getBuildIds( nUPD, nBuild );
        if ( rImport.IsTextDocInOOoFileFormat() ||
             ( bBuildIdFound && nUPD == 680 ) )
        {
            /* handling former documents written by OpenOffice.org:
               use default list id of numbering rules instance, if existing
               (#i92811#)
            */
            if ( !sListStyleDefaultListId.isEmpty() )
            {
                msListId = sListStyleDefaultListId;
                if ( !bIsContinueNumberingAttributePresent &&
                     !mbRestartNumbering &&
                     rTextListsHelper.IsListProcessed( msListId ) )
                {
                    mbRestartNumbering = true;
                }
            }
        }
        if ( msListId.isEmpty() )
        {
            // generate a new list id for the list
            msListId = rTextListsHelper.GenerateNewListId();
        }
    }

    if ( bIsContinueNumberingAttributePresent && !mbRestartNumbering &&
         msContinueListId.isEmpty() )
    {
        const OUString& Last( rTextListsHelper.GetLastProcessedListId() );
        if ( rTextListsHelper.GetListStyleOfLastProcessedList() == msListStyleName
             && Last != msListId )
        {
            msContinueListId = Last;
        }
    }

    if ( !msContinueListId.isEmpty() )
    {
        if ( !rTextListsHelper.IsListProcessed( msContinueListId ) )
        {
            msContinueListId.clear();
        }
        else
        {
            // search continue list chain for master list and
            // continue the master list.
            OUString sTmpStr =
                rTextListsHelper.GetContinueListIdOfProcessedList( msContinueListId );
            while ( !sTmpStr.isEmpty() )
            {
                msContinueListId = sTmpStr;

                sTmpStr =
                    rTextListsHelper.GetContinueListIdOfProcessedList( msContinueListId );
            }
        }
    }

    if ( !rTextListsHelper.IsListProcessed( msListId ) )
    {
        // Inconsistent behavior regarding lists (#i92811#)
        rTextListsHelper.KeepListAsProcessed(
            msListId, msListStyleName, msContinueListId,
            sListStyleDefaultListId );
    }
}

XMLTextListBlockContext::~XMLTextListBlockContext()
{
}

void XMLTextListBlockContext::endFastElement(sal_Int32 )
{
    // Numbering has not to be restarted if it has been restarted within
    // a child list.
    XMLTextListBlockContext *pParent = mxParentListBlock.get();
    if( pParent )
    {
        pParent->mbRestartNumbering = mbRestartNumbering;
    }

    // Restore current list block.
    mrTxtImport.GetTextListHelper().PopListContext();

    // Any paragraph following the list within the same list item must not
    // be numbered.
    mrTxtImport.GetTextListHelper().SetListItem( nullptr );
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLTextListBlockContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    bool bHeader = false;
    switch( nElement )
    {
    case XML_ELEMENT(TEXT, XML_LIST_HEADER):
        bHeader = true;
        [[fallthrough]];
    case XML_ELEMENT(TEXT, XML_LIST_ITEM):
        pContext = new XMLTextListItemContext( GetImport(), mrTxtImport,
                                              xAttrList, bHeader );
        break;
    default:
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    }

    return pContext;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
