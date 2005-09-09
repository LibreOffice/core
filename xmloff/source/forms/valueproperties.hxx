/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: valueproperties.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:17:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _XMLOFF_FORMS_VALUEPROPERTIES_HXX_
#define _XMLOFF_FORMS_VALUEPROPERTIES_HXX_

#ifndef _XMLOFF_FORMS_CONTROLELEMENT_HXX_
#include "controlelement.hxx"
#endif

//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= OValuePropertiesMetaData
    //=====================================================================
    class OValuePropertiesMetaData
    {
    protected:
        OValuePropertiesMetaData() { }

    public:
        /** calculate the property names for the <em>current-value</em> and the <em>value</em> attribute.

            <p>If controls of the given FormComponentType do not have any of the properties requested,
            the respective out parameter will be set to NULL.</p>
        */
        static void getValuePropertyNames(
            OControlElement::ElementType _eType,
            sal_Int16 _nFormComponentType,
            sal_Char const * & _rpCurrentValuePropertyName,
            sal_Char const * & _rpValuePropertyName);

        /** calculate the property names for the <em>min-value</em> and the <em>max-value</em> attribute.

            <p>If controls of the given FormComponentType do not have any of the properties requested,
            the respective out parameter will be set to NULL.</p>
        */
        static void getValueLimitPropertyNames(
            sal_Int16 _nFormComponentType,
            sal_Char const * & _rpMinValuePropertyName,
            sal_Char const * & _rpMaxValuePropertyName);

        /** calculate the names of the properties which, at runtime, are used for <em>value</em> and
            <em>default value</em>.
        */
        static void getRuntimeValuePropertyNames(
            OControlElement::ElementType _eType,
            sal_Int16 _nFormComponentType,
            sal_Char const * & _rpValuePropertyName,
            sal_Char const * & _rpDefaultValuePropertyName);
    };

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // _XMLOFF_FORMS_VALUEPROPERTIES_HXX_


