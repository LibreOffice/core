/*************************************************************************
 *
 *  $RCSfile: valueproperties.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2001-02-13 09:07:25 $
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
#include "valueproperties.hxx"
#endif
#ifndef _XMLOFF_FORMS_STRINGS_HXX_
#include "strings.hxx"
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::form;

    //=====================================================================
    //= OValuePropertiesMetaData
    //=====================================================================
    //---------------------------------------------------------------------
    void OValuePropertiesMetaData::getValuePropertyNames(
            OControlElement::ElementType _eType, sal_Int16 _nFormComponentType,
            sal_Char const * & _rpCurrentValuePropertyName, sal_Char const * & _rpValuePropertyName)
    {
        // reset the pointers in case we can't determine the property names
        _rpCurrentValuePropertyName = _rpValuePropertyName = NULL;
        switch (_nFormComponentType)
        {
            case FormComponentType::TEXTFIELD:
                if (OControlElement::FORMATTED_TEXT == _eType)
                {
                    _rpCurrentValuePropertyName = PROPERTY_EFFECTIVE_VALUE;
                    _rpValuePropertyName = PROPERTY_EFFECTIVE_DEFAULT;
                }
                else
                {
                    if (OControlElement::PASSWORD != _eType)
                        // no CurrentValue" for passwords
                        _rpCurrentValuePropertyName = PROPERTY_TEXT;
                    _rpValuePropertyName = PROPERTY_DEFAULT_TEXT;
                }
                break;
            case FormComponentType::DATEFIELD:
                _rpCurrentValuePropertyName = PROPERTY_DATE;
                _rpValuePropertyName = PROPERTY_DEFAULT_DATE;
                break;
            case FormComponentType::TIMEFIELD:
                _rpCurrentValuePropertyName = PROPERTY_TIME;
                _rpValuePropertyName = PROPERTY_DEFAULT_TIME;
                break;
            case FormComponentType::NUMERICFIELD:
            case FormComponentType::CURRENCYFIELD:
                _rpCurrentValuePropertyName = PROPERTY_VALUE;
                _rpValuePropertyName = PROPERTY_DEFAULT_VALUE;
                break;
            case FormComponentType::PATTERNFIELD:
            case FormComponentType::FILECONTROL:
            case FormComponentType::COMBOBOX:
                _rpValuePropertyName = PROPERTY_DEFAULT_TEXT;
                // NO BREAK!!
            case FormComponentType::COMMANDBUTTON:
                _rpCurrentValuePropertyName = PROPERTY_TEXT;
                break;
            case FormComponentType::CHECKBOX:
            case FormComponentType::RADIOBUTTON:
                _rpValuePropertyName = PROPERTY_REFVALUE;
                break;
            case FormComponentType::HIDDENCONTROL:
                _rpValuePropertyName = PROPERTY_VALUE;
                break;
        }
    }


    //---------------------------------------------------------------------
    void OValuePropertiesMetaData::getValueLimitPropertyNames(sal_Int16 _nFormComponentType,
        sal_Char const * & _rpMinValuePropertyName, sal_Char const * & _rpMaxValuePropertyName)
    {
        _rpMinValuePropertyName = _rpMinValuePropertyName = NULL;
        switch (_nFormComponentType)
        {
            case FormComponentType::DATEFIELD:
                _rpMinValuePropertyName = PROPERTY_DATE_MIN;
                _rpMaxValuePropertyName = PROPERTY_DATE_MAX;
                break;
            case FormComponentType::TIMEFIELD:
                _rpMinValuePropertyName = PROPERTY_TIME_MIN;
                _rpMaxValuePropertyName = PROPERTY_TIME_MAX;
                break;
            case FormComponentType::NUMERICFIELD:
            case FormComponentType::CURRENCYFIELD:
                _rpMinValuePropertyName = PROPERTY_VALUE_MIN;
                _rpMaxValuePropertyName = PROPERTY_VALUE_MAX;
                break;
            case FormComponentType::PATTERNFIELD:
                // no min/max value for the pattern field
                break;
            case FormComponentType::TEXTFIELD:
                _rpMinValuePropertyName = PROPERTY_EFFECTIVE_MIN;
                _rpMaxValuePropertyName = PROPERTY_EFFECTIVE_MAX;
                break;
        }
    }

//.........................................................................
}   // namespace xmloff
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2000/12/13 10:36:36  fs
 *  initial checkin - helper class for meta data for the different value properties
 *
 *
 *  Revision 1.0 12.12.00 14:14:36  fs
 ************************************************************************/

