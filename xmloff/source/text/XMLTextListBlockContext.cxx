/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XMLTextListBlockContext.cxx,v $
 * $Revision: 1.14 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include <tools/debug.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnumi.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include "XMLTextListItemContext.hxx"
#include "XMLTextListBlockContext.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

TYPEINIT1( XMLTextListBlockContext, SvXMLImportContext );

// OD 2008-05-07 #refactorlists#
// add optional parameter <bRestartNumberingAtSubList> and its handling
XMLTextListBlockContext::XMLTextListBlockContext(
        SvXMLImport& rImport,
        XMLTextImportHelper& rTxtImp,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        const sal_Bool bRestartNumberingAtSubList )
:   SvXMLImportContext( rImport, nPrfx, rLName )
,   mrTxtImport( rTxtImp )
// --> OD 2008-04-22 #refactorlists#
,   msListStyleName()
// <--
,   mxParentListBlock( rTxtImp.GetListBlock() )
,   mnLevel( 0 )
// --> OD 2008-05-07 #refactorlists#
//,   mbRestartNumbering( sal_True )
,   mbRestartNumbering( sal_False )
// <--
,   mbSetDefaults( sal_False )
// --> OD 2008-04-22 #refactorlists#
,   msListId()
,   msContinueListId()
// <--
{
    // Inherit style name from parent list, as well as the flags whether
    // numbering must be restarted and formats have to be created.
    OUString sParentListStyleName;
    if( mxParentListBlock.Is() )
    {
        XMLTextListBlockContext *pParent =
                                (XMLTextListBlockContext *)&mxParentListBlock;
        msListStyleName = pParent->GetListStyleName();
        sParentListStyleName = msListStyleName;
        mxNumRules = pParent->GetNumRules();
        mnLevel = pParent->GetLevel() + 1;
        // --> OD 2008-05-07 #refactorlists#
//        mbRestartNumbering = pParent->IsRestartNumbering();
        mbRestartNumbering = pParent->IsRestartNumbering() ||
                             bRestartNumberingAtSubList;
        // <--
        mbSetDefaults = pParent->mbSetDefaults;
        // --> OD 2008-04-22 #refactorlists#
        msListId = pParent->GetListId();
        msContinueListId = pParent->GetContinueListId();
        // <--
    }

    const SvXMLTokenMap& rTokenMap = mrTxtImport.GetTextListBlockAttrTokenMap();

    // --> OD 2008-05-07 #refactorlists#
    bool bIsContinueNumberingAttributePresent( false );
    // <--
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
//FIXME: there is no UNO API for lists
            // --> OD 2008-07-31 #i92221# - xml:id is also the list ID
            if ( mnLevel == 0 ) // root <list> element
            {
                msListId = rValue;
            }
            // <--
            break;
        case XML_TOK_TEXT_LIST_BLOCK_CONTINUE_NUMBERING:
            mbRestartNumbering = !IsXMLToken(rValue, XML_TRUE);
            // --> OD 2008-05-07 #refactorlists#
            bIsContinueNumberingAttributePresent = true;
            // <--
            break;
        case XML_TOK_TEXT_LIST_BLOCK_STYLE_NAME:
            msListStyleName = rValue;
            break;
        // --> OD 2008-04-22 #refactorlists#
        case XML_TOK_TEXT_LIST_BLOCK_CONTINUE_LIST:
            if ( mnLevel == 0 ) // root <list> element
            {
                msContinueListId = rValue;
            }
            break;
        }
    }

    if ( msListStyleName.getLength() &&
         msListStyleName != sParentListStyleName )
    {
        OUString sDisplayStyleName(
                GetImport().GetStyleDisplayName( XML_STYLE_FAMILY_TEXT_LIST,
                                                 msListStyleName ) );
        const Reference < XNameContainer >& rNumStyles =
                                            mrTxtImport.GetNumberingStyles();
        if( rNumStyles.is() && rNumStyles->hasByName( sDisplayStyleName ) )
        {
            Reference < XStyle > xStyle;
            Any aAny = rNumStyles->getByName( sDisplayStyleName );
            aAny >>= xStyle;

            // --> OD 2008-05-07 #refactorlists# - no longer needed
//            // If the style has not been used, the restart numbering has
//            // to be set never.
//            if ( mbRestartNumbering && !xStyle->isInUse() )
//            {
//                mbRestartNumbering = sal_False;
//            }
            // <--

            Reference< XPropertySet > xPropSet( xStyle, UNO_QUERY );
            aAny = xPropSet->getPropertyValue( mrTxtImport.sNumberingRules );
            aAny >>= mxNumRules;
        }
        else
        {
            const SvxXMLListStyleContext *pListStyle =
                                mrTxtImport.FindAutoListStyle( msListStyleName );
            if( pListStyle )
            {
                mxNumRules = pListStyle->GetNumRules();
                // --> OD 2008-05-07 #refactorlists# - no longer needed
//                sal_Bool bUsed = mxNumRules.is();
                // <--
                if( !mxNumRules.is() )
                {
                    pListStyle->CreateAndInsertAuto();
                    mxNumRules = pListStyle->GetNumRules();
                }
                // --> OD 2008-05-07 #refactorlists# - no longer needed
//                if( mbRestartNumbering && !bUsed )
//                    mbRestartNumbering = sal_False;
                // <--
            }
        }
    }

    if( !mxNumRules.is() )
    {
        // If no style name has been specified for this style and for any
        // parent or if no num rule this the specified name is existing,
        // create a new one.

        mxNumRules =
            SvxXMLListStyleContext::CreateNumRule( GetImport().GetModel() );
        DBG_ASSERT( mxNumRules.is(), "go no numbering rule" );
        if( !mxNumRules.is() )
            return;

        // Because it is a new num rule, numbering must be restarted never.
        mbRestartNumbering = sal_False;
        mbSetDefaults = sal_True;
    }

    const sal_Int32 nLevelCount = mxNumRules->getCount();
    if( mnLevel >= nLevelCount )
        mnLevel = sal::static_int_cast< sal_Int16 >(nLevelCount-1);

    if( mbSetDefaults )
    {
        // Because there is no list style sheet for this style, a default
        // format must be set for any level of this num rule.
        SvxXMLListStyleContext::SetDefaultStyle( mxNumRules, mnLevel,
                                                   sal_False );
    }

    // --> OD 2008-04-23 #refactorlists#
    if ( mnLevel == 0 ) // root <list> element
    {
        if ( msListId.getLength() == 0 )  // no text:id property found
        {
            sal_Int32 nUPD( 0 );
            sal_Int32 nBuild( 0 );
            const bool bBuildIdFound = GetImport().getBuildIds( nUPD, nBuild );
            if ( rImport.IsTextDocInOOoFileFormat() ||
                 ( bBuildIdFound && nUPD == 680 ) )
            {
                // handling former documents written by OpenOffice.org:
                // use default list id of numbering rules instance, if existing
                uno::Reference< beans::XPropertySet > xNumRuleProps( mxNumRules, UNO_QUERY );
                if ( xNumRuleProps.is() )
                {
                    uno::Reference< beans::XPropertySetInfo > xNumRulePropSetInfo(
                                                xNumRuleProps->getPropertySetInfo());
                    if ( xNumRulePropSetInfo.is() &&
                         xNumRulePropSetInfo->hasPropertyByName( mrTxtImport.sPropNameDefaultListId) )
                    {
                        xNumRuleProps->getPropertyValue( mrTxtImport.sPropNameDefaultListId ) >>= msListId;
                        DBG_ASSERT( msListId.getLength() != 0,
                                    "no default list id found at numbering rules instance. Serious defect -> please inform OD." );
                        if ( !bIsContinueNumberingAttributePresent &&
                             !mbRestartNumbering &&
                             mrTxtImport.IsListProcessed( msListId ) )
                        {
                            mbRestartNumbering = sal_True;
                        }
                    }
                }
            }
            if ( msListId.getLength() == 0 )
            {
                // generate a new list id for the list
                msListId = mrTxtImport.GenerateNewListId();
            }
        }

        if ( bIsContinueNumberingAttributePresent && !mbRestartNumbering &&
             msContinueListId.getLength() == 0 )
        {
            if ( mrTxtImport.GetListStyleOfLastProcessedList() == msListStyleName &&
                 mrTxtImport.GetLastProcessedListId() != msListId )
            {
                msContinueListId = mrTxtImport.GetLastProcessedListId();
            }
        }

        if ( msContinueListId.getLength() > 0 )
        {
            if ( !mrTxtImport.IsListProcessed( msContinueListId ) )
            {
                msContinueListId = ::rtl::OUString();
            }
            else
            {
                // search continue list chain for master list and
                // continue the master list.
                ::rtl::OUString sTmpStr =
                    mrTxtImport.GetContinueListIdOfProcessedList( msContinueListId );
                while ( sTmpStr.getLength() > 0 )
                {
                    msContinueListId = sTmpStr;

                    sTmpStr =
                        mrTxtImport.GetContinueListIdOfProcessedList( msContinueListId );
                }
            }
        }

        if ( !mrTxtImport.IsListProcessed( msListId ) )
        {
            mrTxtImport.KeepListAsProcessed( msListId, msListStyleName, msContinueListId );
        }
    }
    // <--

    // Remember this list block.
    mrTxtImport.SetListBlock( this );

    // There is no list item by now.
    mrTxtImport.SetListItem( 0 );
}

XMLTextListBlockContext::~XMLTextListBlockContext()
{
}

void XMLTextListBlockContext::EndElement()
{
    // Numbering has not to be restarted if it has been restarted within
    // a child list.
    XMLTextListBlockContext *pParent =
                                (XMLTextListBlockContext *)&mxParentListBlock;
    if( pParent )
    {
        pParent->mbRestartNumbering = mbRestartNumbering;
    }

    // Restore current list block.
    mrTxtImport.SetListBlock( pParent );

    // Any paragraph following the list within the same list item must not
    // be numbered.
    mrTxtImport.SetListItem( 0 );
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

// --> OD 2008-04-22 #refactorlists#
const ::rtl::OUString& XMLTextListBlockContext::GetListId() const
{
    return msListId;
}

const ::rtl::OUString& XMLTextListBlockContext::GetContinueListId() const
{
    return msContinueListId;
}
// <--
