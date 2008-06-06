/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XMLTextNumRuleInfo.cxx,v $
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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/ucb/XAnyCompare.hpp>
#include "XMLTextNumRuleInfo.hxx"

#include <xmloff/xmlexp.hxx>

using ::rtl::OUString;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;

XMLTextNumRuleInfo::XMLTextNumRuleInfo()
: sNumberingRules(RTL_CONSTASCII_USTRINGPARAM("NumberingRules"))
, sNumberingLevel(RTL_CONSTASCII_USTRINGPARAM("NumberingLevel"))
, sNumberingStartValue(RTL_CONSTASCII_USTRINGPARAM("NumberingStartValue"))
, sParaIsNumberingRestart(RTL_CONSTASCII_USTRINGPARAM("ParaIsNumberingRestart"))
, sNumberingType(RTL_CONSTASCII_USTRINGPARAM("NumberingType"))
, sNumberingIsNumber(RTL_CONSTASCII_USTRINGPARAM("NumberingIsNumber"))
, sNumberingIsOutline(RTL_CONSTASCII_USTRINGPARAM("NumberingIsOutline"))
{
    Reset();
}

// --> OD 2006-09-27 #i69627#
void XMLTextNumRuleInfo::Set(
        const ::com::sun::star::uno::Reference <
                        ::com::sun::star::text::XTextContent > & xTextContent,
        const sal_Bool bOutlineStyleAsNormalListStyle )
{
    Reset();
    // --> OD 2006-09-27 #i69627#
    mbOutlineStyleAsNormalListStyle = bOutlineStyleAsNormalListStyle;
    // <--

    Reference< XPropertySet > xPropSet( xTextContent, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

    // check if this paragraph supports a numbering
    if( !xPropSetInfo->hasPropertyByName( sNumberingLevel ) )
        return;

    if( xPropSet->getPropertyValue( sNumberingLevel ) >>= nLevel )
    {
        if( xPropSetInfo->hasPropertyByName( sNumberingRules ) )
            xPropSet->getPropertyValue( sNumberingRules ) >>= xNumRules;
    }
    else
    {
        // in applications using the outliner we always have a numbering rule,
        // so a void property no numbering
        nLevel = 0;
    }

    // --> OD 2006-09-27 #i69627#
    bool bSuppressListStyle( false );
    {
        if ( !mbOutlineStyleAsNormalListStyle )
        {
            sal_Bool bIsOutline = sal_False;
            Reference<XPropertySet> xNumRulesProps(xNumRules, UNO_QUERY);
            if (xNumRulesProps.is() &&
                xNumRulesProps->getPropertySetInfo()->
                hasPropertyByName( sNumberingIsOutline ) )
            {
                xNumRulesProps->getPropertyValue( sNumberingIsOutline ) >>= bIsOutline;
                bSuppressListStyle = bIsOutline ? true : false;
            }
        }
    }

    if( xNumRules.is() && !bSuppressListStyle )
    // <--
    {
        Reference < XNamed > xNamed( xNumRules, UNO_QUERY );
        if( xNamed.is() )
        {
            bIsNamed = sal_True;
            sName = xNamed->getName();
        }

        if( xPropSetInfo->hasPropertyByName( sNumberingIsNumber ) )
        {
            if( !(xPropSet->getPropertyValue( sNumberingIsNumber ) >>= bIsNumbered ) )
            {
                OSL_ENSURE( false, "numbered paragraph without number info" );
                bIsNumbered = sal_False;
            }
        }

        if( bIsNumbered )
        {
            if( xPropSetInfo->hasPropertyByName( sParaIsNumberingRestart ) )
            {
                xPropSet->getPropertyValue( sParaIsNumberingRestart ) >>= bIsRestart;
            }
            if( xPropSetInfo->hasPropertyByName( sNumberingStartValue ) )
            {
                xPropSet->getPropertyValue( sNumberingStartValue ) >>= nStartValue;
            }
        }

        OSL_ENSURE( nLevel < xNumRules->getCount(), "wrong num rule level" );
        if( nLevel >= xNumRules->getCount() )
        {
            Reset();
            return;
        }

        Sequence<PropertyValue> aProps;
        xNumRules->getByIndex( nLevel ) >>= aProps;

        const PropertyValue* pPropArray = aProps.getConstArray();
        sal_Int32 nCount = aProps.getLength();
        for( sal_Int32 i=0; i<nCount; i++ )
        {
            const beans::PropertyValue& rProp = pPropArray[i];

            if( rProp.Name == sNumberingType )
            {
                sal_Int16 nType = 0;
                rProp.Value >>= nType;
                if( NumberingType::CHAR_SPECIAL != nType &&
                    NumberingType::BITMAP != nType )
                {
                    bIsOrdered = sal_True;
                }
                break;
            }
        }
        nLevel++;
    }
}

sal_Bool XMLTextNumRuleInfo::HasSameNumRules( const XMLTextNumRuleInfo& rCmp ) const
{
    if( bIsNamed && rCmp.bIsNamed )
        return rCmp.sName == sName;

    if(rCmp.xNumRules == xNumRules)
        return sal_True;

    uno::Reference< ucb::XAnyCompare > xNumRuleCompare( rCmp.xNumRules, UNO_QUERY );

    return xNumRuleCompare.is() && (xNumRuleCompare->compare( Any( rCmp.xNumRules ), Any( xNumRules ) ) == 0);
}


