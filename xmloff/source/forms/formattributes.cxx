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

#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlimp.hxx>
#include <osl/diagnose.h>

using namespace xmloff::token;

namespace xmloff
{

    using namespace ::com::sun::star::uno;

    //= OAttributeMetaData
    OUString OAttributeMetaData::getCommonControlAttributeName(CCAFlags _nId)
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

    sal_Int32 OAttributeMetaData::getCommonControlAttributeToken(CCAFlags _nId)
    {
        switch (_nId)
        {
            case CCAFlags::Name:             return XML_NAME;
            case CCAFlags::ServiceName:      return XML_CONTROL_IMPLEMENTATION;
            case CCAFlags::ButtonType:       return XML_BUTTON_TYPE;
// disabled(AddAttributeIdLegacy)   case CCAFlags::ControlId:        return "id";
            case CCAFlags::CurrentSelected:  return XML_CURRENT_SELECTED;
            case CCAFlags::CurrentValue:     return XML_CURRENT_VALUE;
            case CCAFlags::Disabled:         return XML_DISABLED;
            case CCAFlags::EnableVisible:    return XML_VISIBLE;
            case CCAFlags::Dropdown:         return XML_DROPDOWN;
            case CCAFlags::For:              return XML_FOR;
            case CCAFlags::ImageData:        return XML_IMAGE_DATA;
            case CCAFlags::Label:            return XML_LABEL;
            case CCAFlags::MaxLength:        return XML_MAX_LENGTH;
            case CCAFlags::Printable:        return XML_PRINTABLE;
            case CCAFlags::ReadOnly:         return XML_READONLY;
            case CCAFlags::Selected:         return XML_SELECTED;
            case CCAFlags::Size:             return XML_SIZE;
            case CCAFlags::TabIndex:         return XML_TAB_INDEX;
            case CCAFlags::TargetFrame:      return XML_TARGET_FRAME;
            case CCAFlags::TargetLocation:   return XML_HREF;      // the only special thing here: TargetLocation is represented by an xlink:href attribute
            case CCAFlags::TabStop:          return XML_TAB_STOP;
            case CCAFlags::Title:            return XML_TITLE;
            case CCAFlags::Value:            return XML_VALUE;
            case CCAFlags::Orientation:      return XML_ORIENTATION;
            case CCAFlags::VisualEffect:     return XML_VISUAL_EFFECT;
            default:
                assert(false && "OAttributeMetaData::getCommonControlAttributeName: invalid id (maybe you or-ed two flags?)!");
        }
        return XML_UNKNOWN;
    }

    sal_uInt16 OAttributeMetaData::getCommonControlAttributeNamespace(CCAFlags _nId)
    {
        if (CCAFlags::TargetLocation == _nId)
            return XML_NAMESPACE_XLINK;

        if (CCAFlags::TargetFrame == _nId)
            return XML_NAMESPACE_OFFICE;

        return XML_NAMESPACE_FORM;
    }

    OUString OAttributeMetaData::getFormAttributeName(FormAttributes _eAttrib)
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
            case faDetailFields:        return "detail-fields";
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

    sal_Int32 OAttributeMetaData::getFormAttributeToken(FormAttributes _eAttrib)
    {
        switch (_eAttrib)
        {
            case faName:                return XML_NAME;
            case faAction:              return XML_HREF;      // the only special thing here: Action is represented by an xlink:href attribute
            case faEnctype:             return XML_ENCTYPE;
            case faMethod:              return XML_METHOD;
            case faAllowDeletes:        return XML_ALLOW_DELETES;
            case faAllowInserts:        return XML_ALLOW_INSERTS;
            case faAllowUpdates:        return XML_ALLOW_UPDATES;
            case faApplyFilter:         return XML_APPLY_FILTER;
            case faCommand:             return XML_COMMAND;
            case faCommandType:         return XML_COMMAND_TYPE;
            case faEscapeProcessing:    return XML_ESCAPE_PROCESSING;
            case faDatasource:          return XML_DATASOURCE;
            case faDetailFields:        return XML_DETAIL_FIELDS;
            case faFilter:              return XML_FILTER;
            case faIgnoreResult:        return XML_IGNORE_RESULT;
            case faMasterFields:        return XML_MASTER_FIELDS;
            case faNavigationMode:      return XML_NAVIGATION_MODE;
            case faOrder:               return XML_ORDER;
            case faTabbingCycle:        return XML_TAB_CYCLE;
            default:
                assert(false && "OAttributeMetaData::getFormAttributeName: invalid id!");
        }
        return XML_NONE;
    }

    sal_uInt16 OAttributeMetaData::getFormAttributeNamespace(FormAttributes _eAttrib)
    {
        if (faAction == _eAttrib)
            return XML_NAMESPACE_XLINK;

        return XML_NAMESPACE_FORM;
    }

    OUString OAttributeMetaData::getDatabaseAttributeName(DAFlags _nId)
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

    sal_Int32 OAttributeMetaData::getDatabaseAttributeToken(DAFlags _nId)
    {
        switch (_nId)
        {
            case DAFlags::BoundColumn:       return XML_BOUND_COLUMN;
            case DAFlags::ConvertEmpty:      return XML_CONVERT_EMPTY_TO_NULL;
            case DAFlags::DataField:         return XML_DATA_FIELD;
            case DAFlags::ListSource:        return XML_LIST_SOURCE;
            case DAFlags::ListSource_TYPE:   return XML_LIST_SOURCE_TYPE;
            case DAFlags::InputRequired:     return XML_INPUT_REQUIRED;
            default:
                assert(false && "OAttributeMetaData::getDatabaseAttributeName: invalid id (maybe you or-ed two flags?)!");
        }
        return XML_NONE;
    }

    OUString OAttributeMetaData::getBindingAttributeName(BAFlags _nId)
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

    sal_Int32 OAttributeMetaData::getBindingAttributeToken(BAFlags _nId)
    {
        switch (_nId)
        {
            case BAFlags::LinkedCell:      return XML_LINKED_CELL;
            case BAFlags::ListLinkingType: return XML_LIST_LINKAGE_TYPE;
            case BAFlags::ListCellRange:   return XML_SOURCE_CELL_RANGE;
            default:
                assert(false && "OAttributeMetaData::getBindingAttributeName: invalid id (maybe you or-ed two flags?)!");
        }
        return XML_UNKNOWN;
    }

    OUString OAttributeMetaData::getSpecialAttributeName(SCAFlags _nId)
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

    sal_Int32 OAttributeMetaData::getSpecialAttributeToken(SCAFlags _nId)
    {
        switch (_nId)
        {
            case SCAFlags::EchoChar:        return XML_ECHO_CHAR;
            case SCAFlags::MaxValue:        return XML_MAX_VALUE;
            case SCAFlags::MinValue:        return XML_MIN_VALUE;
            case SCAFlags::Validation:      return XML_VALIDATION;
            case SCAFlags::GroupName:       return XML_GROUP_NAME;
            case SCAFlags::MultiLine:       return XML_MULTI_LINE;
            case SCAFlags::AutoCompletion:  return XML_AUTO_COMPLETE;
            case SCAFlags::Multiple:        return XML_MULTIPLE;
            case SCAFlags::DefaultButton:   return XML_DEFAULT_BUTTON;
            case SCAFlags::CurrentState:    return XML_CURRENT_STATE;
            case SCAFlags::IsTristate:      return XML_IS_TRISTATE;
            case SCAFlags::State:           return XML_STATE;
            case SCAFlags::ColumnStyleName: return XML_TEXT_STYLE_NAME;
            case SCAFlags::StepSize:        return XML_STEP_SIZE;
            case SCAFlags::PageStepSize:    return XML_PAGE_STEP_SIZE;
            case SCAFlags::RepeatDelay:     return XML_DELAY_FOR_REPEAT;
            case SCAFlags::Toggle:          return XML_TOGGLE;
            case SCAFlags::FocusOnClick:    return XML_FOCUS_ON_CLICK;
            default:
                assert(false && "OAttributeMetaData::getSpecialAttributeName: invalid id (maybe you or-ed two flags?)!");
        }
        return XML_UNKNOWN;
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

    OUString OAttributeMetaData::getOfficeFormsAttributeName(OfficeFormsAttributes _eAttrib)
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

    xmloff::token::XMLTokenEnum OAttributeMetaData::getOfficeFormsAttributeToken(OfficeFormsAttributes _eAttrib)
    {
        switch (_eAttrib)
        {
            case ofaAutomaticFocus:     return token::XML_AUTOMATIC_FOCUS;
            case ofaApplyDesignMode:    return token::XML_APPLY_DESIGN_MODE;
            default:
                assert(false && "OAttributeMetaData::getOfficeFormsAttributeName: invalid id!");
        }
        return token::XML_NONE;
    }

    //= OAttribute2Property
    OAttribute2Property::OAttribute2Property()
    {
    }

    OAttribute2Property::~OAttribute2Property()
    {
    }

    const OAttribute2Property::AttributeAssignment* OAttribute2Property::getAttributeTranslation(
            sal_Int32 nAttributeToken)
    {
        auto aPos = m_aKnownProperties.find(nAttributeToken & TOKEN_MASK);
        if (m_aKnownProperties.end() != aPos)
            return &aPos->second;
        return nullptr;
    }

    void OAttribute2Property::addStringProperty(
        sal_Int32 nAttributeToken, const OUString& _rPropertyName)
    {
        implAdd(nAttributeToken, _rPropertyName, ::cppu::UnoType<OUString>::get());
    }

    void OAttribute2Property::addBooleanProperty(
        sal_Int32 nAttributeToken, const OUString& _rPropertyName,
        const bool /*_bAttributeDefault*/, const bool _bInverseSemantics)
    {
        AttributeAssignment& aAssignment = implAdd(nAttributeToken, _rPropertyName, cppu::UnoType<bool>::get());
        aAssignment.bInverseSemantics = _bInverseSemantics;
    }

    void OAttribute2Property::addInt16Property(
        sal_Int32 nAttributeToken, const OUString& _rPropertyName)
    {
        implAdd(nAttributeToken, _rPropertyName, ::cppu::UnoType<sal_Int16>::get());
    }

    void OAttribute2Property::addInt32Property(
        sal_Int32 nAttributeToken, const OUString& _rPropertyName)
    {
        implAdd( nAttributeToken, _rPropertyName, ::cppu::UnoType<sal_Int32>::get() );
    }

    void OAttribute2Property::addEnumPropertyImpl(
            sal_Int32 nAttributeToken, const OUString& _rPropertyName,
            const SvXMLEnumMapEntry<sal_uInt16>* _pValueMap,
            const css::uno::Type* _pType)
    {
        AttributeAssignment& aAssignment = implAdd(nAttributeToken, _rPropertyName,
            _pType ? *_pType : ::cppu::UnoType<sal_Int32>::get());
        aAssignment.pEnumMap = _pValueMap;
    }

    OAttribute2Property::AttributeAssignment& OAttribute2Property::implAdd(
            sal_Int32 nAttributeToken, const OUString& _rPropertyName,
            const css::uno::Type& _rType)
    {
        nAttributeToken &= TOKEN_MASK;
        OSL_ENSURE(m_aKnownProperties.end() == m_aKnownProperties.find(nAttributeToken),
            "OAttribute2Property::implAdd: already have this attribute!");

        AttributeAssignment aAssignment;
        aAssignment.sPropertyName = _rPropertyName;
        aAssignment.aPropertyType = _rType;

        // redundance, the accessor is stored in aAssignment.sAttributeName, too
        m_aKnownProperties[nAttributeToken] = aAssignment;
        return m_aKnownProperties[nAttributeToken];
    }

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
