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
            case faAction:              return "href";      // the only special thing here: Action is represented by an xlink:href attribute
            case faEnctype:             return "enctype";
            case faMethod:              return "method";
            case faAllowDeletes:        return "allow-deletes";
            case faAllowInserts:        return "allow-inserts";
            case faAllowUpdates:        return "allow-updates";
            case faApplyFilter:         return "apply-filter";
            case faCommand:             return "command";
            case faCommandType:         return "command-type";
            case faEscapeProcessing:    return "escape-processing";
            case faDatasource:          return "datasource";
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

        return XML_NAMESPACE_FORM;
    }

    const sal_Char* OAttributeMetaData::getDatabaseAttributeName(DAFlags _nId)
    {
        switch (_nId)
        {
            case DAFlags::BoundColumn:       return "bound-column";
            case DAFlags::ConvertEmpty:      return "convert-empty-to-null";
            case DAFlags::DataField:         return "data-field";
            case DAFlags::ListSource:        return "list-source";
            case DAFlags::ListSource_TYPE:   return "list-source-type";
            case DAFlags::InputRequired:     return "input-required";
            default:
                OSL_FAIL("OAttributeMetaData::getDatabaseAttributeName: invalid id (maybe you or-ed two flags?)!");
        }
        return "";
    }

    const sal_Char* OAttributeMetaData::getBindingAttributeName(BAFlags _nId)
    {
        switch (_nId)
        {
            case BAFlags::LinkedCell:       return "linked-cell";
            case BAFlags::ListLinkingType: return "list-linkage-type";
            case BAFlags::ListCellRange:   return "source-cell-range";
            default:
                OSL_FAIL("OAttributeMetaData::getBindingAttributeName: invalid id (maybe you or-ed two flags?)!");
        }
        return "";
    }

    const sal_Char* OAttributeMetaData::getSpecialAttributeName(SCAFlags _nId)
    {
        switch (_nId)
        {
            case SCAFlags::EchoChar:             return "echo-char";
            case SCAFlags::MaxValue:             return "max-value";
            case SCAFlags::MinValue:             return "min-value";
            case SCAFlags::Validation:            return "validation";
            case SCAFlags::GroupName:            return "group-name";
            case SCAFlags::MultiLine:            return "multi-line";
            case SCAFlags::AutoCompletion:  return "auto-complete";
            case SCAFlags::Multiple:              return "multiple";
            case SCAFlags::DefaultButton:        return "default-button";
            case SCAFlags::CurrentState:         return "current-state";
            case SCAFlags::IsTristate:           return "is-tristate";
            case SCAFlags::State:                 return "state";
            case SCAFlags::ColumnStyleName:     return "text-style-name";
            case SCAFlags::StepSize:             return "step-size";
            case SCAFlags::PageStepSize:        return "page-step-size";
            case SCAFlags::RepeatDelay:          return "delay-for-repeat";
            case SCAFlags::Toggle:                return "toggle";
            case SCAFlags::FocusOnClick:        return "focus-on-click";
            default:
                OSL_FAIL("OAttributeMetaData::getSpecialAttributeName: invalid id (maybe you or-ed two flags?)!");
        }
        return "";
    }

    sal_uInt16 OAttributeMetaData::getSpecialAttributeNamespace(SCAFlags _nId)
    {
        switch( _nId )
        {
            case SCAFlags::GroupName:            return XML_NAMESPACE_FORMX;
            default: break;
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
        const sal_Char* _pAttributeName, const OUString& _rPropertyName)
    {
        implAdd(_pAttributeName, _rPropertyName, ::cppu::UnoType<OUString>::get());
    }

    void OAttribute2Property::addBooleanProperty(
        const sal_Char* _pAttributeName, const OUString& _rPropertyName,
        const bool /*_bAttributeDefault*/, const bool _bInverseSemantics)
    {
        AttributeAssignment& aAssignment = implAdd(_pAttributeName, _rPropertyName, cppu::UnoType<bool>::get());
        aAssignment.bInverseSemantics = _bInverseSemantics;
    }

    void OAttribute2Property::addInt16Property(
        const sal_Char* _pAttributeName, const OUString& _rPropertyName)
    {
        implAdd(_pAttributeName, _rPropertyName, ::cppu::UnoType<sal_Int16>::get());
    }

    void OAttribute2Property::addInt32Property(
        const sal_Char* _pAttributeName, const OUString& _rPropertyName)
    {
        implAdd( _pAttributeName, _rPropertyName, ::cppu::UnoType<sal_Int32>::get() );
    }

    void OAttribute2Property::addEnumPropertyImpl(
            const sal_Char* _pAttributeName, const OUString& _rPropertyName,
            const SvXMLEnumMapEntry<sal_uInt16>* _pValueMap,
            const css::uno::Type* _pType)
    {
        AttributeAssignment& aAssignment = implAdd(_pAttributeName, _rPropertyName,
            _pType ? *_pType : ::cppu::UnoType<sal_Int32>::get());
        aAssignment.pEnumMap = _pValueMap;
    }

    OAttribute2Property::AttributeAssignment& OAttribute2Property::implAdd(
            const sal_Char* _pAttributeName, const OUString& _rPropertyName,
            const css::uno::Type& _rType)
    {
        OSL_ENSURE(m_aKnownProperties.end() == m_aKnownProperties.find(OUString::createFromAscii(_pAttributeName)),
            "OAttribute2Property::implAdd: already have this attribute!");

        OUString sAttributeName = OUString::createFromAscii(_pAttributeName);

        AttributeAssignment aAssignment;
        aAssignment.sPropertyName = _rPropertyName;
        aAssignment.aPropertyType = _rType;

        // redundance, the accessor is stored in aAssignment.sAttributeName, too
        return m_aKnownProperties[sAttributeName] = aAssignment;
    }

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
