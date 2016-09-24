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

#include "valueproperties.hxx"
#include "strings.hxx"
#include <com/sun/star/form/FormComponentType.hpp>
#include <sal/log.hxx>

namespace xmloff
{

    using namespace ::com::sun::star::form;

    //= OValuePropertiesMetaData
    void OValuePropertiesMetaData::getValuePropertyNames(
            OControlElement::ElementType _eType, sal_Int16 _nFormComponentType,
            sal_Char const * & _rpCurrentValuePropertyName, sal_Char const * & _rpValuePropertyName)
    {
        // reset the pointers in case we can't determine the property names
        _rpCurrentValuePropertyName = _rpValuePropertyName = nullptr;
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

            case FormComponentType::NUMERICFIELD:
            case FormComponentType::CURRENCYFIELD:
                _rpCurrentValuePropertyName = PROPERTY_VALUE;
                _rpValuePropertyName = PROPERTY_DEFAULT_VALUE;
                break;

            case FormComponentType::PATTERNFIELD:
            case FormComponentType::FILECONTROL:
            case FormComponentType::COMBOBOX:
                _rpValuePropertyName = PROPERTY_DEFAULT_TEXT;
                SAL_FALLTHROUGH;
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

            case FormComponentType::SCROLLBAR:
                _rpCurrentValuePropertyName = PROPERTY_SCROLLVALUE;
                _rpValuePropertyName = PROPERTY_SCROLLVALUE_DEFAULT;
                break;

            case FormComponentType::SPINBUTTON:
                _rpCurrentValuePropertyName = PROPERTY_SPINVALUE;
                _rpValuePropertyName = PROPERTY_DEFAULT_SPINVALUE;
                break;

            default:
                SAL_WARN( "xmloff", "OValuePropertiesMetaData::getValuePropertyNames: unsupported component type!" );
                break;
        }
    }

    void OValuePropertiesMetaData::getValueLimitPropertyNames(sal_Int16 _nFormComponentType,
        sal_Char const * & _rpMinValuePropertyName, sal_Char const * & _rpMaxValuePropertyName)
    {
        _rpMinValuePropertyName = _rpMaxValuePropertyName = nullptr;
        switch (_nFormComponentType)
        {
            case FormComponentType::NUMERICFIELD:
            case FormComponentType::CURRENCYFIELD:
                _rpMinValuePropertyName = PROPERTY_VALUE_MIN;
                _rpMaxValuePropertyName = PROPERTY_VALUE_MAX;
                break;

            case FormComponentType::TEXTFIELD:
                _rpMinValuePropertyName = PROPERTY_EFFECTIVE_MIN;
                _rpMaxValuePropertyName = PROPERTY_EFFECTIVE_MAX;
                break;

            case FormComponentType::SCROLLBAR:
                _rpMinValuePropertyName = PROPERTY_SCROLLVALUE_MIN;
                _rpMaxValuePropertyName = PROPERTY_SCROLLVALUE_MAX;
                break;

            case FormComponentType::SPINBUTTON:
                _rpMinValuePropertyName = PROPERTY_SPINVALUE_MIN;
                _rpMaxValuePropertyName = PROPERTY_SPINVALUE_MAX;
                break;

            default:
                SAL_WARN("xmloff", "OValuePropertiesMetaData::getValueLimitPropertyNames: unsupported component type!" );
                break;
        }
    }

    void OValuePropertiesMetaData::getRuntimeValuePropertyNames(
        OControlElement::ElementType _eType, sal_Int16 _nFormComponentType,
        sal_Char const * & _rpValuePropertyName, sal_Char const * & _rpDefaultValuePropertyName )
    {
        // reset the pointers in case we can't determine the property names
        _rpValuePropertyName = _rpDefaultValuePropertyName = nullptr;
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
                _rpValuePropertyName = PROPERTY_DATE;
                _rpDefaultValuePropertyName = PROPERTY_DEFAULT_DATE;
                break;

            case FormComponentType::TIMEFIELD:
                _rpValuePropertyName = PROPERTY_TIME;
                _rpDefaultValuePropertyName = PROPERTY_DEFAULT_TIME;
                break;

            case FormComponentType::NUMERICFIELD:
            case FormComponentType::CURRENCYFIELD:
            case FormComponentType::PATTERNFIELD:
            case FormComponentType::FILECONTROL:
            case FormComponentType::COMBOBOX:
            case FormComponentType::SCROLLBAR:
            case FormComponentType::SPINBUTTON:
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

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
