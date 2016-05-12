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

#include "formattributes.hxx"

#include <sax/tools/converter.hxx>

#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>


namespace xmloff
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

    //= OAttributeMetaData
    const sal_Char* OAttributeMetaData::getCommonControlAttributeName(CCAFlags _nId)
    {
        switch (_nId)
        {
            case CCAFlags::Name:              return "name";
            case CCAFlags::ServiceName:      return "control-implementation";
            case CCAFlags::ButtonType:       return "button-type";
// disabled(AddAttributeIdLegacy)   case CCAFlags::ControlId:        return "id";
            case CCAFlags::CurrentSelected:  return "current-selected";
            case CCAFlags::CurrentValue:     return "current-value";
            case CCAFlags::Disabled:          return "disabled";
            case CCAFlags::EnableVisible:     return "visible";
            case CCAFlags::Dropdown:          return "dropdown";
            case CCAFlags::For:               return "for";
            case CCAFlags::ImageData:        return "image-data";
            case CCAFlags::Label:             return "label";
            case CCAFlags::MaxLength:        return "max-length";
            case CCAFlags::Printable:         return "printable";
            case CCAFlags::ReadOnly:          return "readonly";
            case CCAFlags::Selected:          return "selected";
            case CCAFlags::Size:              return "size";
            case CCAFlags::TabIndex:         return "tab-index";
            case CCAFlags::TargetFrame:      return "target-frame";
            case CCAFlags::TargetLocation:   return "href";      // the only special thing here: TargetLocation is represented by an xlink:href attribute
            case CCAFlags::TabStop:          return "tab-stop";
            case CCAFlags::Title:             return "title";
            case CCAFlags::Value:             return "value";
            case CCAFlags::Orientation:       return "orientation";
            case CCAFlags::VisualEffect:     return "visual-effect";
            default:
                OSL_FAIL("OAttributeMetaData::getCommonControlAttributeName: invalid id (maybe you or-ed two flags?)!");
        }
        return "";
    }

    sal_uInt16 OAttributeMetaData::getCommonControlAttributeNamespace(CCAFlags _nId)
    {
        if (CCAFlags::TargetLocation == _nId)
            return XML_NAMESPACE_XLINK;

        if (CCAFlags::TargetFrame == _nId)
            return XML_NAMESPACE_OFFICE;

        return XML_NAMESPACE_FORM;
    }

    const sal_Char* OAttributeMetaData::getFormAttributeName(FormAttributes _eAttrib)
    {
        switch (_eAttrib)
        {
            case faName:                return "name";
            case faServiceName:         return "service-name";
            case faAction:              return "href";      // the only special thing here: Action is represented by an xlink:href attribute
            case faEnctype:             return "enctype";
            case faMethod:              return "method";
            case faTargetFrame:         return "target-frame";
            case faAllowDeletes:        return "allow-deletes";
            case faAllowInserts:        return "allow-inserts";
            case faAllowUpdates:        return "allow-updates";
            case faApplyFilter:         return "apply-filter";
            case faCommand:             return "command";
            case faCommandType:         return "command-type";
            case faEscapeProcessing:    return "escape-processing";
            case faDatasource:          return "datasource";
            case faConnectionResource:  return "connection-resource";
            case faDetailFiels:         return "detail-fields";
            case faFilter:              return "filter";
            case faIgnoreResult:        return "ignore-result";
            case faMasterFields:        return "master-fields";
            case faNavigationMode:      return "navigation-mode";
            case faOrder:               return "order";
            case faTabbingCycle:        return "tab-cycle";
            default:
                OSL_FAIL("OAttributeMetaData::getFormAttributeName: invalid id!");
        }
        return "";
    }

    sal_uInt16 OAttributeMetaData::getFormAttributeNamespace(FormAttributes _eAttrib)
    {
        if (faAction == _eAttrib)
            return XML_NAMESPACE_XLINK;

        if (faTargetFrame == _eAttrib)
            return XML_NAMESPACE_OFFICE;

        return XML_NAMESPACE_FORM;
    }

    const sal_Char* OAttributeMetaData::getDatabaseAttributeName(sal_Int32 _nId)
    {
        switch (_nId)
        {
            case DA_BOUND_COLUMN:       return "bound-column";
            case DA_CONVERT_EMPTY:      return "convert-empty-to-null";
            case DA_DATA_FIELD:         return "data-field";
            case DA_LIST_SOURCE:        return "list-source";
            case DA_LIST_SOURCE_TYPE:   return "list-source-type";
            case DA_INPUT_REQUIRED:     return "input-required";
            default:
                OSL_FAIL("OAttributeMetaData::getDatabaseAttributeName: invalid id (maybe you or-ed two flags?)!");
        }
        return "";
    }

    const sal_Char* OAttributeMetaData::getBindingAttributeName(sal_Int32 _nId)
    {
        switch (_nId)
        {
            case BA_LINKED_CELL:       return "linked-cell";
            case BA_LIST_LINKING_TYPE: return "list-linkage-type";
            case BA_LIST_CELL_RANGE:   return "source-cell-range";
            default:
                OSL_FAIL("OAttributeMetaData::getBindingAttributeName: invalid id (maybe you or-ed two flags?)!");
        }
        return "";
    }

    const sal_Char* OAttributeMetaData::getSpecialAttributeName(sal_Int32 _nId)
    {
        switch (_nId)
        {
            case SCA_ECHO_CHAR:             return "echo-char";
            case SCA_MAX_VALUE:             return "max-value";
            case SCA_MIN_VALUE:             return "min-value";
            case SCA_VALIDATION:            return "validation";
            case SCA_GROUP_NAME:            return "group-name";
            case SCA_MULTI_LINE:            return "multi-line";
            case SCA_AUTOMATIC_COMPLETION:  return "auto-complete";
            case SCA_MULTIPLE:              return "multiple";
            case SCA_DEFAULT_BUTTON:        return "default-button";
            case SCA_CURRENT_STATE:         return "current-state";
            case SCA_IS_TRISTATE:           return "is-tristate";
            case SCA_STATE:                 return "state";
            case SCA_COLUMN_STYLE_NAME:     return "text-style-name";
            case SCA_STEP_SIZE:             return "step-size";
            case SCA_PAGE_STEP_SIZE:        return "page-step-size";
            case SCA_REPEAT_DELAY:          return "delay-for-repeat";
            case SCA_TOGGLE:                return "toggle";
            case SCA_FOCUS_ON_CLICK:        return "focus-on-click";
            default:
                OSL_FAIL("OAttributeMetaData::getSpecialAttributeName: invalid id (maybe you or-ed two flags?)!");
        }
        return "";
    }

    sal_uInt16 OAttributeMetaData::getSpecialAttributeNamespace(sal_Int32 _nId)
    {
        switch( _nId )
        {
            case SCA_GROUP_NAME:            return XML_NAMESPACE_FORMX;
        }
        return XML_NAMESPACE_FORM;
    }

    const sal_Char* OAttributeMetaData::getOfficeFormsAttributeName(OfficeFormsAttributes _eAttrib)
    {
        switch (_eAttrib)
        {
            case ofaAutomaticFocus:     return "automatic-focus";
            case ofaApplyDesignMode:    return "apply-design-mode";
            default:
                OSL_FAIL("OAttributeMetaData::getOfficeFormsAttributeName: invalid id!");
        }
        return "";
    }

    //= OAttribute2Property
    OAttribute2Property::OAttribute2Property()
    {
    }

    OAttribute2Property::~OAttribute2Property()
    {
    }

    const OAttribute2Property::AttributeAssignment* OAttribute2Property::getAttributeTranslation(
            const OUString& _rAttribName)
    {
        AttributeAssignments::const_iterator aPos = m_aKnownProperties.find(_rAttribName);
        if (m_aKnownProperties.end() != aPos)
            return &aPos->second;
        return nullptr;
    }

    void OAttribute2Property::addStringProperty(
        const sal_Char* _pAttributeName, const OUString& _rPropertyName,
        const sal_Char* _pAttributeDefault)
    {
        implAdd(_pAttributeName, _rPropertyName, ::cppu::UnoType<OUString>::get(),
            _pAttributeDefault ? OUString::createFromAscii(_pAttributeDefault) : OUString());
    }

    void OAttribute2Property::addBooleanProperty(
        const sal_Char* _pAttributeName, const OUString& _rPropertyName,
        const bool _bAttributeDefault, const bool _bInverseSemantics)
    {
        OUStringBuffer aDefault;
        ::sax::Converter::convertBool(aDefault, _bAttributeDefault);
        AttributeAssignment& aAssignment = implAdd(_pAttributeName, _rPropertyName, cppu::UnoType<bool>::get(), aDefault.makeStringAndClear());
        aAssignment.bInverseSemantics = _bInverseSemantics;
    }

    void OAttribute2Property::addInt16Property(
        const sal_Char* _pAttributeName, const OUString& _rPropertyName,
        const sal_Int16 _nAttributeDefault)
    {
        OUStringBuffer aDefault;
        ::sax::Converter::convertNumber(aDefault, (sal_Int32)_nAttributeDefault);
        implAdd(_pAttributeName, _rPropertyName, ::cppu::UnoType<sal_Int16>::get(), aDefault.makeStringAndClear());
    }

    void OAttribute2Property::addInt32Property(
        const sal_Char* _pAttributeName, const OUString& _rPropertyName,
        const sal_Int32 _nAttributeDefault)
    {
        OUStringBuffer aDefault;
        ::sax::Converter::convertNumber( aDefault, _nAttributeDefault );
        implAdd( _pAttributeName, _rPropertyName, ::cppu::UnoType<sal_Int32>::get(), aDefault.makeStringAndClear() );
    }

    void OAttribute2Property::addEnumProperty(
            const sal_Char* _pAttributeName, const OUString& _rPropertyName,
            const sal_uInt16 _nAttributeDefault, const SvXMLEnumMapEntry* _pValueMap,
            const css::uno::Type* _pType)
    {
        OUStringBuffer aDefault;
        SvXMLUnitConverter::convertEnum(aDefault, _nAttributeDefault, _pValueMap);
        AttributeAssignment& aAssignment = implAdd(_pAttributeName, _rPropertyName,
            _pType ? *_pType : ::cppu::UnoType<sal_Int32>::get(),
                // this assumes that the setPropertyValue for enums can handle int32's ....
            aDefault.makeStringAndClear());
        aAssignment.pEnumMap = _pValueMap;
    }

    OAttribute2Property::AttributeAssignment& OAttribute2Property::implAdd(
            const sal_Char* _pAttributeName, const OUString& _rPropertyName,
            const css::uno::Type& _rType, const OUString& /*_rDefaultString*/)
    {
        OSL_ENSURE(m_aKnownProperties.end() == m_aKnownProperties.find(OUString::createFromAscii(_pAttributeName)),
            "OAttribute2Property::implAdd: already have this attribute!");

        OUString sAttributeName = OUString::createFromAscii(_pAttributeName);

        AttributeAssignment aAssignment;
        aAssignment.sAttributeName = sAttributeName;
        aAssignment.sPropertyName = _rPropertyName;
        aAssignment.aPropertyType = _rType;

        // redundance, the accessor is stored in aAssignment.sAttributeName, too
        return m_aKnownProperties[sAttributeName] = aAssignment;
    }

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
