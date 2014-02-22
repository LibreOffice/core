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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include "XMLTextNumRuleInfo.hxx"
#include <xmloff/XMLTextListAutoStylePool.hxx>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;


XMLTextNumRuleInfo::XMLTextNumRuleInfo()
    : msNumberingRules("NumberingRules")
    , msNumberingLevel("NumberingLevel")
    , msNumberingStartValue("NumberingStartValue")
    , msParaIsNumberingRestart("ParaIsNumberingRestart")
    , msNumberingIsNumber("NumberingIsNumber")
    , msNumberingIsOutline("NumberingIsOutline")
    , msPropNameListId("ListId")
    , msPropNameStartWith("StartWith")
    , msContinueingPreviousSubTree("ContinueingPreviousSubTree")
    , msListLabelStringProp("ListLabelString")
    , mxNumRules()
    , msNumRulesName()
    , msListId()
    , mnListStartValue( -1 )
    , mnListLevel( 0 )
    , mbIsNumbered( sal_False )
    , mbIsRestart( sal_False )
    , mnListLevelStartValue( -1 )
    , mbOutlineStyleAsNormalListStyle( sal_False )
{
    Reset();
}


void XMLTextNumRuleInfo::Set(
        const ::com::sun::star::uno::Reference <
                        ::com::sun::star::text::XTextContent > & xTextContent,
        const sal_Bool bOutlineStyleAsNormalListStyle,
        const XMLTextListAutoStylePool& rListAutoPool,
        const sal_Bool bExportTextNumberElement )
{
    Reset();
    
    mbOutlineStyleAsNormalListStyle = bOutlineStyleAsNormalListStyle;

    Reference< XPropertySet > xPropSet( xTextContent, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();

    
    if( !xPropSetInfo->hasPropertyByName( msNumberingLevel ) )
        return;

    if( xPropSet->getPropertyValue( msNumberingLevel ) >>= mnListLevel )
    {
        if( xPropSetInfo->hasPropertyByName( msNumberingRules ) )
        {
            xPropSet->getPropertyValue( msNumberingRules ) >>= mxNumRules;
        }
    }
    else
    {
        
        
        mnListLevel = 0;
    }

    
    if ( mxNumRules.is() && mxNumRules->getCount() < 1 )
    {
        DBG_ASSERT( false,
                    "<XMLTextNumRuleInfo::Set(..)> - numbering rules instance does not contain any numbering rule" );
        Reset();
        return;
    }

    if ( mnListLevel < 0 )
    {
        DBG_ASSERT( false,
                    "<XMLTextNumRuleInfo::Set(..)> - unexpected numbering level" );
        Reset();
        return;
    }

    
    bool bSuppressListStyle( false );
    if ( mxNumRules.is() )
    {
        if ( !mbOutlineStyleAsNormalListStyle )
        {
            sal_Bool bIsOutline = sal_False;
            Reference<XPropertySet> xNumRulesProps(mxNumRules, UNO_QUERY);
            if ( xNumRulesProps.is() &&
                 xNumRulesProps->getPropertySetInfo()->
                                    hasPropertyByName( msNumberingIsOutline ) )
            {
                xNumRulesProps->getPropertyValue( msNumberingIsOutline ) >>= bIsOutline;
                bSuppressListStyle = bIsOutline ? true : false;
            }
        }
    }

    if( mxNumRules.is() && !bSuppressListStyle )
    {
        
        
        msNumRulesName = rListAutoPool.Find( mxNumRules );
        if ( msNumRulesName.isEmpty() )
        {
            Reference < XNamed > xNamed( mxNumRules, UNO_QUERY );
            DBG_ASSERT( xNamed.is(),
                        "<XMLTextNumRuleInfo::Set(..)> - numbering rules instance have to be named. Serious defect -> please inform OD." );
            if( xNamed.is() )
            {
                msNumRulesName = xNamed->getName();
            }
        }
        DBG_ASSERT( !msNumRulesName.isEmpty(),
                    "<XMLTextNumRuleInfo::Set(..)> - no name found for numbering rules instance. Serious defect -> please inform OD." );

        if( xPropSetInfo->hasPropertyByName( msPropNameListId ) )
        {
            xPropSet->getPropertyValue( msPropNameListId ) >>= msListId;
        }

        mbContinueingPreviousSubTree = sal_False;
        if( xPropSetInfo->hasPropertyByName( msContinueingPreviousSubTree ) )
        {
            xPropSet->getPropertyValue( msContinueingPreviousSubTree ) >>= mbContinueingPreviousSubTree;
        }

        mbIsNumbered = sal_True;
        if( xPropSetInfo->hasPropertyByName( msNumberingIsNumber ) )
        {
            if( !(xPropSet->getPropertyValue( msNumberingIsNumber ) >>= mbIsNumbered ) )
            {
                OSL_FAIL( "numbered paragraph without number info" );
                mbIsNumbered = sal_False;
            }
        }

        if( mbIsNumbered )
        {
            if( xPropSetInfo->hasPropertyByName( msParaIsNumberingRestart ) )
            {
                xPropSet->getPropertyValue( msParaIsNumberingRestart ) >>= mbIsRestart;
            }
            if( xPropSetInfo->hasPropertyByName( msNumberingStartValue ) )
            {
                xPropSet->getPropertyValue( msNumberingStartValue ) >>= mnListStartValue;
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

        const PropertyValue* pPropArray = aProps.getConstArray();
        sal_Int32 nCount = aProps.getLength();
        for( sal_Int32 i=0; i<nCount; i++ )
        {
          const PropertyValue& rProp = pPropArray[i];

            if ( rProp.Name == msPropNameStartWith )
            {
                rProp.Value >>= mnListLevelStartValue;
                break;
            }
        }

        msListLabelString = OUString();
        if ( bExportTextNumberElement &&
             xPropSetInfo->hasPropertyByName( msListLabelStringProp ) )
        {
            xPropSet->getPropertyValue( msListLabelStringProp ) >>= msListLabelString;
        }

        
        ++mnListLevel;
    }
    else
    {
        mnListLevel = 0;
    }
}

sal_Bool XMLTextNumRuleInfo::BelongsToSameList( const XMLTextNumRuleInfo& rCmp ) const
{
    sal_Bool bRet( sal_True );
    
    if ( !rCmp.msListId.isEmpty() || !msListId.isEmpty() )
    {
        bRet = rCmp.msListId == msListId;
    }
    else
    {
        bRet = HasSameNumRules( rCmp );
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
