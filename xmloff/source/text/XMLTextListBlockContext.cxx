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

#include <tools/debug.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnumi.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include "XMLTextListItemContext.hxx"
#include "XMLTextListBlockContext.hxx"
#include "txtlists.hxx"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

TYPEINIT1( XMLTextListBlockContext, SvXMLImportContext );



XMLTextListBlockContext::XMLTextListBlockContext(
        SvXMLImport& rImport,
        XMLTextImportHelper& rTxtImp,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        const sal_Bool bRestartNumberingAtSubList )
:   SvXMLImportContext( rImport, nPrfx, rLName )
,   mrTxtImport( rTxtImp )
,   msListStyleName()
,   mxParentListBlock( )
,   mnLevel( 0 )
,   mbRestartNumbering( sal_False )
,   mbSetDefaults( sal_False )
,   msListId()
,   msContinueListId()
{
    static OUString s_PropNameDefaultListId("DefaultListId");
    {
        
        XMLTextListBlockContext * pLB(0);
        XMLTextListItemContext  * pLI(0);
        XMLNumberedParaContext  * pNP(0);
        rTxtImp.GetTextListHelper().ListContextTop(pLB, pLI, pNP);
        mxParentListBlock = pLB;
    }
    
    
    OUString sParentListStyleName;
    if( mxParentListBlock.Is() )
    {
        XMLTextListBlockContext *pParent =
                                (XMLTextListBlockContext *)&mxParentListBlock;
        msListStyleName = pParent->GetListStyleName();
        sParentListStyleName = msListStyleName;
        mxNumRules = pParent->GetNumRules();
        mnLevel = pParent->GetLevel() + 1;
        mbRestartNumbering = pParent->IsRestartNumbering() ||
                             bRestartNumberingAtSubList;
        mbSetDefaults = pParent->mbSetDefaults;
        msListId = pParent->GetListId();
        msContinueListId = pParent->GetContinueListId();
    }

    const SvXMLTokenMap& rTokenMap = mrTxtImport.GetTextListBlockAttrTokenMap();

    bool bIsContinueNumberingAttributePresent( false );
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        switch( rTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_TEXT_LIST_BLOCK_XMLID:
            sXmlId = rValue;

            
            if ( mnLevel == 0 ) 
            {
                msListId = rValue;
            }
            break;
        case XML_TOK_TEXT_LIST_BLOCK_CONTINUE_NUMBERING:
            mbRestartNumbering = !IsXMLToken(rValue, XML_TRUE);
            bIsContinueNumberingAttributePresent = true;
            break;
        case XML_TOK_TEXT_LIST_BLOCK_STYLE_NAME:
            msListStyleName = rValue;
            break;
        case XML_TOK_TEXT_LIST_BLOCK_CONTINUE_LIST:
            if ( mnLevel == 0 ) 
            {
                msContinueListId = rValue;
            }
            break;
        }
    }

    mxNumRules = XMLTextListsHelper::MakeNumRule(GetImport(), mxNumRules,
        sParentListStyleName, msListStyleName,
        mnLevel, &mbRestartNumbering, &mbSetDefaults );
    if( !mxNumRules.is() )
        return;

    if ( mnLevel == 0 ) 
    {
        XMLTextListsHelper& rTextListsHelper( mrTxtImport.GetTextListHelper() );
        
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
                    DBG_ASSERT( !sListStyleDefaultListId.isEmpty(),
                                "no default list id found at numbering rules instance. Serious defect -> please inform OD." );
                }
            }
        }
        if ( msListId.isEmpty() )  
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
                        mbRestartNumbering = sal_True;
                    }
                }
            }
            if ( msListId.isEmpty() )
            {
                
                msListId = rTextListsHelper.GenerateNewListId();
            }
        }

        if ( bIsContinueNumberingAttributePresent && !mbRestartNumbering &&
             msContinueListId.isEmpty() )
        {
            OUString Last( rTextListsHelper.GetLastProcessedListId() );
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
                msContinueListId = "";
            }
            else
            {
                
                
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
            
            rTextListsHelper.KeepListAsProcessed(
                msListId, msListStyleName, msContinueListId,
                sListStyleDefaultListId );
        }
    }

    
    mrTxtImport.GetTextListHelper().PushListContext( this );
}

XMLTextListBlockContext::~XMLTextListBlockContext()
{
}

void XMLTextListBlockContext::EndElement()
{
    
    
    XMLTextListBlockContext *pParent =
                                (XMLTextListBlockContext *)&mxParentListBlock;
    if( pParent )
    {
        pParent->mbRestartNumbering = mbRestartNumbering;
    }

    
    mrTxtImport.GetTextListHelper().PopListContext();

    
    
    mrTxtImport.GetTextListHelper().SetListItem( 0 );
}

SvXMLImportContext *XMLTextListBlockContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap =
                        mrTxtImport.GetTextListBlockElemTokenMap();
    sal_Bool bHeader = sal_False;
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
    case XML_TOK_TEXT_LIST_HEADER:
        bHeader = sal_True;
    case XML_TOK_TEXT_LIST_ITEM:
        pContext = new XMLTextListItemContext( GetImport(), mrTxtImport,
                                                nPrefix, rLocalName,
                                              xAttrList, bHeader );
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

const OUString& XMLTextListBlockContext::GetListId() const
{
    return msListId;
}

const OUString& XMLTextListBlockContext::GetContinueListId() const
{
    return msContinueListId;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
