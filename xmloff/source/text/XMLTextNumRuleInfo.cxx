/*************************************************************************
 *
 *  $RCSfile: XMLTextNumRuleInfo.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mib $ $Date: 2000-10-31 09:00:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_NUMBERINGTYPE_HPP_
#include <com/sun/star/style/NumberingType.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _XMLOFF_XMLTEXTNUMRULEINFO_HXX
#include "XMLTextNumRuleInfo.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;

XMLTextNumRuleInfo::XMLTextNumRuleInfo() :
    sNumberingRules(RTL_CONSTASCII_USTRINGPARAM("NumberingRules")),
    sNumberingLevel(RTL_CONSTASCII_USTRINGPARAM("NumberingLevel")),
    sNumberingStartValue(RTL_CONSTASCII_USTRINGPARAM("NumberingStartValue")),
    sParaIsNumberingRestart(RTL_CONSTASCII_USTRINGPARAM("ParaIsNumberingRestart")),
    sNumberingType(RTL_CONSTASCII_USTRINGPARAM("NumberingType")),
    sIsNumbering(RTL_CONSTASCII_USTRINGPARAM("IsNumbering")),
    sNumberingIsNumber(RTL_CONSTASCII_USTRINGPARAM("NumberingIsNumber"))
{
    Reset();
}

void XMLTextNumRuleInfo::Set(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & xTextContent )
{
    Reset();

    Reference< XPropertySet > xPropSet( xTextContent, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();

    Any aAny;

    // check if numbering is enabled in edit engine
    if( xPropSetInfo->hasPropertyByName( sIsNumbering ) )
    {
        aAny = xPropSet->getPropertyValue( sIsNumbering );
        if( !(*(sal_Bool *)aAny.getValue()) )
            return;
    }

    if( xPropSetInfo->hasPropertyByName( sNumberingRules ) )
    {
        aAny = xPropSet->getPropertyValue( sNumberingRules );
        aAny >>= xNumRules;
    }

    if( xNumRules.is() )
    {
        Reference < XNamed > xNamed( xNumRules, UNO_QUERY );
        if( xNamed.is() )
        {
            bIsNamed = sal_True;
            sName = xNamed->getName();
        }

        aAny = xPropSet->getPropertyValue( sNumberingLevel );
        aAny >>= nLevel;

        bIsNumbered = sal_True;
        if( xPropSetInfo->hasPropertyByName( sNumberingIsNumber ) )
        {
            aAny = xPropSet->getPropertyValue( sNumberingIsNumber );
            bIsNumbered = *(sal_Bool *)aAny.getValue();
        }

        if( bIsNumbered )
        {
            if( xPropSetInfo->hasPropertyByName( sParaIsNumberingRestart ) )
            {
                aAny = xPropSet->getPropertyValue( sParaIsNumberingRestart );
                bIsRestart = *(sal_Bool *)aAny.getValue();
            }
            if( xPropSetInfo->hasPropertyByName( sNumberingStartValue ) )
            {
                aAny = xPropSet->getPropertyValue( sNumberingStartValue );
                aAny >>= nStartValue;
            }
        }

        DBG_ASSERT( nLevel < xNumRules->getCount(), "wrong num rule level" );

        aAny = xNumRules->getByIndex( nLevel );
        Sequence<PropertyValue> aProps;
        aAny >>= aProps;
        const PropertyValue* pPropArray = aProps.getConstArray();
        sal_Int32 nCount = aProps.getLength();
        for( sal_Int32 i=0; i<nCount; i++ )
        {
            const beans::PropertyValue& rProp = pPropArray[i];

            if( rProp.Name == sNumberingType )
            {
                sal_Int16 nType;
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



