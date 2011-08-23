/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "valueproperties.hxx"
#include "strings.hxx"
#include <com/sun/star/form/FormComponentType.hpp>
namespace binfilter {

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
                _rpValuePropertyName = PROPERTY_HIDDEN_VALUE;
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

    //---------------------------------------------------------------------
    void OValuePropertiesMetaData::getRuntimeValuePropertyNames(
        OControlElement::ElementType _eType, sal_Int16 _nFormComponentType,
        sal_Char const * & _rpValuePropertyName, sal_Char const * & _rpDefaultValuePropertyName )
    {
        // reset the pointers in case we can't determine the property names
        _rpValuePropertyName = _rpDefaultValuePropertyName = NULL;
        switch (_nFormComponentType)
        {
            case FormComponentType::TEXTFIELD:
                if (OControlElement::FORMATTED_TEXT == _eType)
                {
                    _rpValuePropertyName = PROPERTY_EFFECTIVE_VALUE;
                    _rpDefaultValuePropertyName = PROPERTY_EFFECTIVE_DEFAULT;
                }
                else
                {
                    _rpValuePropertyName = PROPERTY_TEXT;
                    _rpDefaultValuePropertyName = PROPERTY_DEFAULT_TEXT;
                }
                break;

            case FormComponentType::DATEFIELD:
            case FormComponentType::TIMEFIELD:
            case FormComponentType::NUMERICFIELD:
            case FormComponentType::CURRENCYFIELD:
            case FormComponentType::PATTERNFIELD:
            case FormComponentType::FILECONTROL:
            case FormComponentType::COMBOBOX:
                // For these types, the runtime properties are the same as the ones which in the XML
                // stream are named "value properties"
                getValuePropertyNames( _eType, _nFormComponentType, _rpValuePropertyName, _rpDefaultValuePropertyName );
                break;

            case FormComponentType::CHECKBOX:
            case FormComponentType::RADIOBUTTON:
                _rpValuePropertyName = PROPERTY_STATE;
                _rpDefaultValuePropertyName = PROPERTY_DEFAULT_STATE;
                break;
        }
    }

//.........................................................................
}	// namespace xmloff
//.........................................................................


}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
