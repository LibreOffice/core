/*************************************************************************
 *
 *  $RCSfile: property.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-09-29 11:28:15 $
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


#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

//.........................................................................
namespace comphelper
{

    namespace starlang = ::com::sun::star::lang;

//------------------------------------------------------------------
int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC ) && defined( OS2 )
_Optlink
#endif
    PropertyCompare( const void* pFirst, const void* pSecond)
{
    return ((starbeans::Property*)pFirst)->Name.compareTo(((starbeans::Property*)pSecond)->Name);
}

//------------------------------------------------------------------
void copyProperties(const staruno::Reference<starbeans::XPropertySet>& _rxSource,
                    const staruno::Reference<starbeans::XPropertySet>& _rxDest)
{
    if (!_rxSource.is() || !_rxDest.is())
    {
        OSL_ENSHURE(sal_False, "copyProperties : invalid arguments !");
    }

    staruno::Reference< starbeans::XPropertySetInfo > xSourceProps = _rxSource->getPropertySetInfo();
    staruno::Reference< starbeans::XPropertySetInfo > xDestProps = _rxDest->getPropertySetInfo();

    staruno::Sequence< starbeans::Property > aSourceProps = xSourceProps->getProperties();
    const starbeans::Property* pSourceProps = aSourceProps.getConstArray();
    starbeans::Property aDestProp;
    for (sal_Int32 i=0; i<aSourceProps.getLength(); ++i, ++pSourceProps)
    {
        if  (   xDestProps->hasPropertyByName(pSourceProps->Name)
            &&  xDestProps->hasPropertyByName(pSourceProps->Name)
            )
        {
            try
            {
                aDestProp = xDestProps->getPropertyByName(pSourceProps->Name);
                if (0 == (aDestProp.Attributes & starbeans::PropertyAttribute::READONLY))
                    _rxDest->setPropertyValue(pSourceProps->Name, _rxSource->getPropertyValue(pSourceProps->Name));
            }
            catch (staruno::Exception&)
            {
                OSL_ENSHURE(sal_False, "copyProperties : could not transfer at least one of the properties !");
            }
        }
    }
}

//------------------------------------------------------------------
sal_Bool hasProperty(const rtl::OUString& _rName, const staruno::Reference<starbeans::XPropertySet>& _rxSet)
{
    if (_rxSet.is())
    {
        //  XPropertySetInfoRef xInfo(rxSet->getPropertySetInfo());
        return _rxSet->getPropertySetInfo()->hasPropertyByName(_rName);
    }
    return sal_False;
}

//------------------------------------------------------------------
void RemoveProperty(staruno::Sequence<starbeans::Property>& _rProps, const rtl::OUString& _rPropName)
{
    sal_Int32 nLen = _rProps.getLength();

    // binaere Suche
    starbeans::Property aSearchDummy(_rPropName, 0, ::getCppuType(reinterpret_cast<sal_Int32*>(NULL))/*doesn't matter*/, 0);
    const starbeans::Property* pProperties = _rProps.getConstArray();
    starbeans::Property* pResult = (starbeans::Property*) bsearch(&aSearchDummy, (void*)pProperties, nLen, sizeof(starbeans::Property),
        &PropertyCompare);

    // gefunden ?
    if (pResult)
    {
        OSL_ENSHURE(pResult->Name.equals(_rPropName), "::RemoveProperty Properties nicht sortiert");
        removeElementAt(_rProps, pResult - pProperties);
    }
}

//------------------------------------------------------------------
void ModifyPropertyAttributes(staruno::Sequence<starbeans::Property>& seqProps, const ::rtl::OUString& sPropName, sal_Int16 nAddAttrib, sal_Int16 nRemoveAttrib)
{
    sal_Int32 nLen = seqProps.getLength();

    // binaere Suche
    starbeans::Property aSearchDummy(sPropName, 0, ::getCppuType(reinterpret_cast<sal_Int32*>(NULL))/*doesn't matter*/, 0);
    starbeans::Property* pResult = (starbeans::Property*) bsearch(&aSearchDummy, (void*)seqProps.getArray(), nLen, sizeof(starbeans::Property),
        &PropertyCompare);

    // gefunden ?
    if (pResult)
    {
        pResult->Attributes |= nAddAttrib;
        pResult->Attributes &= ~nRemoveAttrib;
    }
}

//------------------------------------------------------------------
sal_Bool tryPropertyValue(staruno::Any& _rConvertedValue, staruno::Any& _rOldValue, const staruno::Any& _rValueToSet, staruno::Any& _rCurrentValue, const staruno::Type& _rExpectedType)
{
    sal_Bool bModified(sal_False);
    if (_rCurrentValue.getValue() != _rValueToSet.getValue())
    {
        if (_rValueToSet.hasValue() && !_rExpectedType.equals(_rValueToSet.getValueType()))
            throw starlang::IllegalArgumentException();

        if (!compare(_rCurrentValue, _rValueToSet))
        {
            _rConvertedValue = _rValueToSet;
            _rOldValue = _rCurrentValue;
            bModified = sal_True;
        }
    }
    return bModified;
}

//.........................................................................
}
//.........................................................................

