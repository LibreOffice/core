/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XMLTextNumRuleInfo.cxx,v $
 * $Revision: 1.15 $
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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include "XMLTextNumRuleInfo.hxx"
// --> OD 2008-04-25 #refactorlists#
#include "XMLTextListAutoStylePool.hxx"
// <--

using ::rtl::OUString;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;

// --> OD 2008-05-08 #refactorlists#
// Complete refactoring of the class and enhancement of the class for lists.
XMLTextNumRuleInfo::XMLTextNumRuleInfo()
    : mxNumRules(),
      msNumRulesName(),
      msListId(),
      mnListStartValue( -1 ),
      mnListLevel( 0 ),
      mbIsNumbered( sal_False ),
      mbIsRestart( sal_False ),
//      mbIsOrdered( sal_False ),
//      mbIsNumRulesNamed( sal_False ),
      mnListLevelStartValue( -1 ),
      mbOutlineStyleAsNormalListStyle( sal_False )
{
    Reset();
}

// --> OD 2006-09-27 #i69627#
void XMLTextNumRuleInfo::Set(
        const ::com::sun::star::uno::Reference <
                        ::com::sun::star::text::XTextContent > & xTextContent,
        const sal_Bool bOutlineStyleAsNormalListStyle,
        const XMLTextListAutoStylePool& rListAutoPool )
{
    Reset();
    // --> OD 2006-09-27 #i69627#
    mbOutlineStyleAsNormalListStyle = bOutlineStyleAsNormalListStyle;
    // <--

    Reference< XPropertySet > xPropSet( xTextContent, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();

    // check if this paragraph supports a numbering
    const ::rtl::OUString sNumberingLevel(RTL_CONSTASCII_USTRINGPARAM("NumberingLevel"));
    if( !xPropSetInfo->hasPropertyByName( sNumberingLevel ) )
        return;

    if( xPropSet->getPropertyValue( sNumberingLevel ) >>= mnListLevel )
    {
        const ::rtl::OUString sNumberingRules(RTL_CONSTASCII_USTRINGPARAM("NumberingRules"));
        if( xPropSetInfo->hasPropertyByName( sNumberingRules ) )
        {
            xPropSet->getPropertyValue( sNumberingRules ) >>= mxNumRules;
        }
    }
    else
    {
        // in applications using the outliner we always have a numbering rule,
        // so a void property no numbering
        mnListLevel = 0;
    }

    // --> OD 2006-09-27 #i69627#
    bool bSuppressListStyle( false );
    {
        if ( !mbOutlineStyleAsNormalListStyle )
        {
            sal_Bool bIsOutline = sal_False;
            Reference<XPropertySet> xNumRulesProps(mxNumRules, UNO_QUERY);
            const ::rtl::OUString sNumberingIsOutline(RTL_CONSTASCII_USTRINGPARAM("NumberingIsOutline"));
            if ( xNumRulesProps.is() &&
                 xNumRulesProps->getPropertySetInfo()->
                                    hasPropertyByName( sNumberingIsOutline ) )
            {
                xNumRulesProps->getPropertyValue( sNumberingIsOutline ) >>= bIsOutline;
                bSuppressListStyle = bIsOutline ? true : false;
            }
        }
    }

    if( mxNumRules.is() && !bSuppressListStyle )
    // <--
    {
        // First try to find the numbering rules in the list auto style pool.
        // If not found, the numbering rules instance has to be named.
        msNumRulesName = rListAutoPool.Find( mxNumRules );
        if ( msNumRulesName.getLength() == 0 )
        {
            Reference < XNamed > xNamed( mxNumRules, UNO_QUERY );
            DBG_ASSERT( xNamed.is(),
                        "<XMLTextNumRuleInfo::Set(..)> - numbering rules instance have to be named. Serious defect -> please inform OD." );
            if( xNamed.is() )
            {
//                mbIsNumRulesNamed = sal_True;
                msNumRulesName = xNamed->getName();
            }
        }

        const ::rtl::OUString sPropNameListId(RTL_CONSTASCII_USTRINGPARAM("ListId"));
        if( xPropSetInfo->hasPropertyByName( sPropNameListId ) )
        {
            xPropSet->getPropertyValue( sPropNameListId ) >>= msListId;
        }

        mbIsNumbered = sal_True;
        const ::rtl::OUString sNumberingIsNumber(RTL_CONSTASCII_USTRINGPARAM("NumberingIsNumber"));
        if( xPropSetInfo->hasPropertyByName( sNumberingIsNumber ) )
        {
            if( !(xPropSet->getPropertyValue( sNumberingIsNumber ) >>= mbIsNumbered ) )
            {
                OSL_ENSURE( false, "numbered paragraph without number info" );
                mbIsNumbered = sal_False;
            }
        }

        if( mbIsNumbered )
        {
            const ::rtl::OUString sParaIsNumberingRestart(RTL_CONSTASCII_USTRINGPARAM("ParaIsNumberingRestart"));
            if( xPropSetInfo->hasPropertyByName( sParaIsNumberingRestart ) )
            {
                xPropSet->getPropertyValue( sParaIsNumberingRestart ) >>= mbIsRestart;
            }
            const ::rtl::OUString sNumberingStartValue(RTL_CONSTASCII_USTRINGPARAM("NumberingStartValue"));
            if( xPropSetInfo->hasPropertyByName( sNumberingStartValue ) )
            {
                xPropSet->getPropertyValue( sNumberingStartValue ) >>= mnListStartValue;
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
//        const ::rtl::OUString sNumberingType(RTL_CONSTASCII_USTRINGPARAM("NumberingType"));
        const ::rtl::OUString sPropNameStartWith( RTL_CONSTASCII_USTRINGPARAM("StartWith") );
        sal_Int32 nCount = aProps.getLength();
        for( sal_Int32 i=0; i<nCount; i++ )
        {
          const PropertyValue& rProp = pPropArray[i];

//          if( rProp.Name == sNumberingType )
//          {
//              sal_Int16 nType = 0;
//              rProp.Value >>= nType;
//              if( NumberingType::CHAR_SPECIAL != nType &&
//                  NumberingType::BITMAP != nType )
//              {
//                    mbIsOrdered = sal_True;
//              }
//              break;
//          }
            if ( rProp.Name == sPropNameStartWith )
            {
                rProp.Value >>= mnListLevelStartValue;
            }
        }

        // paragraph's list level range is [0..9] representing list levels [1..10]
        ++mnListLevel;
    }
}

sal_Bool XMLTextNumRuleInfo::BelongsToSameList( const XMLTextNumRuleInfo& rCmp ) const
{
    sal_Bool bRet( sal_True );
    // Currently only the text documents support <ListId>.
    if ( rCmp.msListId.getLength() > 0 ||
         msListId.getLength() > 0 )
    {
        bRet = rCmp.msListId == msListId;
    }
    else
    {
        bRet = HasSameNumRules( rCmp );
    }

    return bRet;
}
// <--
