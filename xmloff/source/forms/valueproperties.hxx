/*************************************************************************
 *
 *  $RCSfile: valueproperties.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 18:20:27 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


