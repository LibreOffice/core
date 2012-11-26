/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include "XMLTextListItemContext.hxx"
#include "XMLTextListBlockContext.hxx"
#include "txtlists.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

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
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        const sal_Bool bRestartNumberingAtSubList )
:   SvXMLImportContext( rImport, nPrfx, rLName )
,   mrTxtImport( rTxtImp )
// --> OD 2008-04-22 #refactorlists#
,   msListStyleName()
// <--
,   mxParentListBlock( )
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
    static ::rtl::OUString s_PropNameDefaultListId(
        RTL_CONSTASCII_USTRINGPARAM("DefaultListId"));
    {
        // get the parent list block context (if any); this is a bit ugly...
        XMLTextListBlockContext * pLB(0);
        XMLTextListItemContext  * pLI(0);
        XMLNumberedParaContext  * pNP(0);
        rTxtImp.GetTextListHelper().ListContextTop(pLB, pLI, pNP);
        mxParentListBlock = pLB;
    }
    // Inherit style name from parent list, as well as the flags whether
    // numbering must be restarted and formats have to be created.
    OUString sParentListStyleName;
    // --> OD 2008-11-27 #158694#
    sal_Bool bParentRestartNumbering( sal_False );
    // <--
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
        // --> OD 2008-11-27 #158694#
        bParentRestartNumbering = pParent->IsRestartNumbering();
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

    mxNumRules = XMLTextListsHelper::MakeNumRule(GetImport(), mxNumRules,
        sParentListStyleName, msListStyleName,
        mnLevel, &mbRestartNumbering, &mbSetDefaults );
    if( !mxNumRules.is() )
        return;

    // --> OD 2008-04-23 #refactorlists#
    if ( mnLevel == 0 ) // root <list> element
    {
        XMLTextListsHelper& rTextListsHelper( mrTxtImport.GetTextListHelper() );
        // --> OD 2008-08-15 #i92811#
        ::rtl::OUString sListStyleDefaultListId;
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
                    DBG_ASSERT( sListStyleDefaultListId.getLength() != 0,
                                "no default list id found at numbering rules instance. Serious defect -> please inform OD." );
                }
            }
        }
        // <--
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
                // --> OD 2008-08-15 #i92811#
                if ( sListStyleDefaultListId.getLength() != 0 )
                {
                    msListId = sListStyleDefaultListId;
                    if ( !bIsContinueNumberingAttributePresent &&
                         !mbRestartNumbering &&
                         rTextListsHelper.IsListProcessed( msListId ) )
                    {
                        mbRestartNumbering = sal_True;
                    }
                }
                // <--
            }
            if ( msListId.getLength() == 0 )
            {
                // generate a new list id for the list
                msListId = rTextListsHelper.GenerateNewListId();
            }
        }

        if ( bIsContinueNumberingAttributePresent && !mbRestartNumbering &&
             msContinueListId.getLength() == 0 )
        {
            ::rtl::OUString Last( rTextListsHelper.GetLastProcessedListId() );
            if ( rTextListsHelper.GetListStyleOfLastProcessedList() == msListStyleName
                 && Last != msListId )
            {
                msContinueListId = Last;
            }
        }

        if ( msContinueListId.getLength() > 0 )
        {
            if ( !rTextListsHelper.IsListProcessed( msContinueListId ) )
            {
                msContinueListId = ::rtl::OUString();
            }
            else
            {
                // search continue list chain for master list and
                // continue the master list.
                ::rtl::OUString sTmpStr =
                    rTextListsHelper.GetContinueListIdOfProcessedList( msContinueListId );
                while ( sTmpStr.getLength() > 0 )
                {
                    msContinueListId = sTmpStr;

                    sTmpStr =
                        rTextListsHelper.GetContinueListIdOfProcessedList( msContinueListId );
                }
            }
        }

        if ( !rTextListsHelper.IsListProcessed( msListId ) )
        {
            // --> OD 2008-08-15 #i92811#
            rTextListsHelper.KeepListAsProcessed(
                msListId, msListStyleName, msContinueListId,
                sListStyleDefaultListId );
            // <--
        }
    }
    // <--

    // Remember this list block.
    mrTxtImport.GetTextListHelper().PushListContext( this );
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
    mrTxtImport.GetTextListHelper().PopListContext();

    // Any paragraph following the list within the same list item must not
    // be numbered.
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

