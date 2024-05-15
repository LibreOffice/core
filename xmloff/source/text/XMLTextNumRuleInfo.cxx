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


#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include "XMLTextNumRuleInfo.hxx"
#include <xmloff/XMLTextListAutoStylePool.hxx>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

// Complete refactoring of the class and enhancement of the class for lists.
XMLTextNumRuleInfo::XMLTextNumRuleInfo()
    : mbListIdIsDefault(false)
    , mnListStartValue( -1 )
    , mnListLevel( 0 )
    , mbIsNumbered( false )
    , mbIsRestart( false )
    , mnListLevelStartValue( -1 )
    , mbOutlineStyleAsNormalListStyle( false )
{
    Reset();
}

// Written OpenDocument file format doesn't fit to the created text document (#i69627#)
void XMLTextNumRuleInfo::Set(
        const css::uno::Reference < css::text::XTextContent > & xTextContent,
        const bool bOutlineStyleAsNormalListStyle,
        const XMLTextListAutoStylePool& rListAutoPool,
        const bool bExportTextNumberElement,
        const bool bListIdIsDefault )
{
    Reset();
    // Written OpenDocument file format doesn't fit to the created text document (#i69627#)
    mbOutlineStyleAsNormalListStyle = bOutlineStyleAsNormalListStyle;

    Reference< XPropertySet > xPropSet( xTextContent, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();

    // check if this paragraph supports a numbering
    if( !xPropSetInfo->hasPropertyByName( u"NumberingLevel"_ustr ) )
        return;

    if( xPropSet->getPropertyValue( u"NumberingLevel"_ustr ) >>= mnListLevel )
    {
        if( xPropSetInfo->hasPropertyByName( u"NumberingRules"_ustr ) )
        {
            xPropSet->getPropertyValue( u"NumberingRules"_ustr ) >>= mxNumRules;
        }
    }
    else
    {
        // in applications using the outliner we always have a numbering rule,
        // so a void property no numbering
        mnListLevel = 0;
    }

    // Assertion saving writer document (#i97312#)
    if ( mxNumRules.is() && mxNumRules->getCount() < 1 )
    {
        SAL_WARN("xmloff",
                    "<XMLTextNumRuleInfo::Set(..)> - numbering rules instance does not contain any numbering rule" );
        Reset();
        return;
    }

    if ( mnListLevel < 0 )
    {
        SAL_WARN("xmloff",
                    "<XMLTextNumRuleInfo::Set(..)> - unexpected numbering level" );
        Reset();
        return;
    }

    // Written OpenDocument file format doesn't fit to the created text document (#i69627#)
    bool bSuppressListStyle( false );
    if ( mxNumRules.is() )
    {
        if ( !mbOutlineStyleAsNormalListStyle )
        {
            Reference<XPropertySet> xNumRulesProps(mxNumRules, UNO_QUERY);
            if ( xNumRulesProps.is() &&
                 xNumRulesProps->getPropertySetInfo()->
                                    hasPropertyByName( u"NumberingIsOutline"_ustr ) )
            {
                bool bIsOutline = false;
                xNumRulesProps->getPropertyValue( u"NumberingIsOutline"_ustr ) >>= bIsOutline;
                bSuppressListStyle = bIsOutline;
            }
        }
    }

    if( mxNumRules.is() && !bSuppressListStyle )
    {
        // First try to find the numbering rules in the list auto style pool.
        // If not found, the numbering rules instance has to be named.
        msNumRulesName = rListAutoPool.Find( mxNumRules );
        if ( msNumRulesName.isEmpty() )
        {
            Reference < XNamed > xNamed( mxNumRules, UNO_QUERY );
            SAL_WARN_IF( !xNamed.is(), "xmloff",
                        "<XMLTextNumRuleInfo::Set(..)> - numbering rules instance have to be named. Serious defect." );
            if( xNamed.is() )
            {
                msNumRulesName = xNamed->getName();
            }
        }
        SAL_WARN_IF( msNumRulesName.isEmpty(), "xmloff",
                    "<XMLTextNumRuleInfo::Set(..)> - no name found for numbering rules instance. Serious defect." );

        if( xPropSetInfo->hasPropertyByName( u"ListId"_ustr ) )
        {
            xPropSet->getPropertyValue( u"ListId"_ustr ) >>= msListId;
        }

        mbListIdIsDefault = bListIdIsDefault;

        mbContinueingPreviousSubTree = false;
        if( xPropSetInfo->hasPropertyByName( u"ContinueingPreviousSubTree"_ustr ) )
        {
            xPropSet->getPropertyValue( u"ContinueingPreviousSubTree"_ustr ) >>= mbContinueingPreviousSubTree;
        }

        mbIsNumbered = true;
        if( xPropSetInfo->hasPropertyByName( u"NumberingIsNumber"_ustr ) )
        {
            if( !(xPropSet->getPropertyValue( u"NumberingIsNumber"_ustr ) >>= mbIsNumbered ) )
            {
                OSL_FAIL( "numbered paragraph without number info" );
                mbIsNumbered = false;
            }
        }

        if( mbIsNumbered )
        {
            if( xPropSetInfo->hasPropertyByName( u"ParaIsNumberingRestart"_ustr ) )
            {
                xPropSet->getPropertyValue( u"ParaIsNumberingRestart"_ustr ) >>= mbIsRestart;
            }
            if( xPropSetInfo->hasPropertyByName( u"NumberingStartValue"_ustr ) )
            {
                xPropSet->getPropertyValue( u"NumberingStartValue"_ustr ) >>= mnListStartValue;
            }
        }

        OSL_ENSURE( mnListLevel < mxNumRules->getCount(), "wrong num rule level" );
        if( mnListLevel >= mxNumRules->getCount() )
        {
            Reset();
            return;
        }

        Sequence<PropertyValue> aProps;
        mxNumRules->getByIndex( mnListLevel ) >>= aProps;

        auto pProp = std::find_if(std::cbegin(aProps), std::cend(aProps),
            [](const PropertyValue& rProp) { return rProp.Name == "StartWith"; });
        if (pProp != std::cend(aProps))
        {
            pProp->Value >>= mnListLevelStartValue;
        }

        msListLabelString.clear();
        if ( bExportTextNumberElement &&
             xPropSetInfo->hasPropertyByName( u"ListLabelString"_ustr ) )
        {
            xPropSet->getPropertyValue( u"ListLabelString"_ustr ) >>= msListLabelString;
        }

        // paragraph's list level range is [0..9] representing list levels [1..10]
        ++mnListLevel;
    }
    else
    {
        mnListLevel = 0;
    }
}

bool XMLTextNumRuleInfo::BelongsToSameList( const XMLTextNumRuleInfo& rCmp ) const
{
    bool bRet( true );
    // Currently only the text documents support <ListId>.
    if ( !rCmp.msListId.isEmpty() || !msListId.isEmpty() )
    {
        bRet = rCmp.msListId == msListId;
    }
    else
    {
        bRet = rCmp.msNumRulesName == msNumRulesName;
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
