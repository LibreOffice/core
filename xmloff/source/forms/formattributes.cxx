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
            case CCAFlags::Name:              return u"name"_ustr;
            case CCAFlags::ServiceName:      return u"control-implementation"_ustr;
            case CCAFlags::ButtonType:       return u"button-type"_ustr;
// disabled(AddAttributeIdLegacy)   case CCAFlags::ControlId:        return "id";
            case CCAFlags::CurrentSelected:  return u"current-selected"_ustr;
            case CCAFlags::CurrentValue:     return u"current-value"_ustr;
            case CCAFlags::Disabled:          return u"disabled"_ustr;
            case CCAFlags::EnableVisible:     return u"visible"_ustr;
            case CCAFlags::Dropdown:          return u"dropdown"_ustr;
            case CCAFlags::For:               return u"for"_ustr;
            case CCAFlags::ImageData:        return u"image-data"_ustr;
            case CCAFlags::Label:             return u"label"_ustr;
            case CCAFlags::MaxLength:        return u"max-length"_ustr;
            case CCAFlags::Printable:         return u"printable"_ustr;
            case CCAFlags::ReadOnly:          return u"readonly"_ustr;
            case CCAFlags::Selected:          return u"selected"_ustr;
            case CCAFlags::Size:              return u"size"_ustr;
            case CCAFlags::TabIndex:         return u"tab-index"_ustr;
            case CCAFlags::TargetFrame:      return u"target-frame"_ustr;
            case CCAFlags::TargetLocation:   return u"href"_ustr;      // the only special thing here: TargetLocation is represented by an xlink:href attribute
            case CCAFlags::TabStop:          return u"tab-stop"_ustr;
            case CCAFlags::Title:             return u"title"_ustr;
            case CCAFlags::Value:             return u"value"_ustr;
            case CCAFlags::Orientation:       return u"orientation"_ustr;
            case CCAFlags::VisualEffect:     return u"visual-effect"_ustr;
            default:
                OSL_FAIL("OAttributeMetaData::getCommonControlAttributeName: invalid id (maybe you or-ed two flags?)!");
        }
        return u""_ustr;
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
            case faName:                return u"name"_ustr;
            case faAction:              return u"href"_ustr;      // the only special thing here: Action is represented by an xlink:href attribute
            case faEnctype:             return u"enctype"_ustr;
            case faMethod:              return u"method"_ustr;
            case faAllowDeletes:        return u"allow-deletes"_ustr;
            case faAllowInserts:        return u"allow-inserts"_ustr;
            case faAllowUpdates:        return u"allow-updates"_ustr;
            case faApplyFilter:         return u"apply-filter"_ustr;
            case faCommand:             return u"command"_ustr;
            case faCommandType:         return u"command-type"_ustr;
            case faEscapeProcessing:    return u"escape-processing"_ustr;
            case faDatasource:          return u"datasource"_ustr;
            case faDetailFields:        return u"detail-fields"_ustr;
            case faFilter:              return u"filter"_ustr;
            case faIgnoreResult:        return u"ignore-result"_ustr;
            case faMasterFields:        return u"master-fields"_ustr;
            case faNavigationMode:      return u"navigation-mode"_ustr;
            case faOrder:               return u"order"_ustr;
            case faTabbingCycle:        return u"tab-cycle"_ustr;
            default:
                OSL_FAIL("OAttributeMetaData::getFormAttributeName: invalid id!");
        }
        return u""_ustr;
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
            case DAFlags::BoundColumn:       return u"bound-column"_ustr;
            case DAFlags::ConvertEmpty:      return u"convert-empty-to-null"_ustr;
            case DAFlags::DataField:         return u"data-field"_ustr;
            case DAFlags::ListSource:        return u"list-source"_ustr;
            case DAFlags::ListSource_TYPE:   return u"list-source-type"_ustr;
            case DAFlags::InputRequired:     return u"input-required"_ustr;
            default:
                OSL_FAIL("OAttributeMetaData::getDatabaseAttributeName: invalid id (maybe you or-ed two flags?)!");
        }
        return u""_ustr;
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
            case BAFlags::LinkedCell:       return u"linked-cell"_ustr;
            case BAFlags::ListLinkingType: return u"list-linkage-type"_ustr;
            case BAFlags::ListCellRange:   return u"source-cell-range"_ustr;
            default:
                OSL_FAIL("OAttributeMetaData::getBindingAttributeName: invalid id (maybe you or-ed two flags?)!");
        }
        return u""_ustr;
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
            case SCAFlags::EchoChar:             return u"echo-char"_ustr;
            case SCAFlags::MaxValue:             return u"max-value"_ustr;
            case SCAFlags::MinValue:             return u"min-value"_ustr;
            case SCAFlags::Validation:            return u"validation"_ustr;
            case SCAFlags::GroupName:            return u"group-name"_ustr;
            case SCAFlags::MultiLine:            return u"multi-line"_ustr;
            case SCAFlags::AutoCompletion:  return u"auto-complete"_ustr;
            case SCAFlags::Multiple:              return u"multiple"_ustr;
            case SCAFlags::DefaultButton:        return u"default-button"_ustr;
            case SCAFlags::CurrentState:         return u"current-state"_ustr;
            case SCAFlags::IsTristate:           return u"is-tristate"_ustr;
            case SCAFlags::State:                 return u"state"_ustr;
            case SCAFlags::ColumnStyleName:     return u"text-style-name"_ustr;
            case SCAFlags::StepSize:             return u"step-size"_ustr;
            case SCAFlags::PageStepSize:        return u"page-step-size"_ustr;
            case SCAFlags::RepeatDelay:          return u"delay-for-repeat"_ustr;
            case SCAFlags::Toggle:                return u"toggle"_ustr;
            case SCAFlags::FocusOnClick:        return u"focus-on-click"_ustr;
            default:
                OSL_FAIL("OAttributeMetaData::getSpecialAttributeName: invalid id (maybe you or-ed two flags?)!");
        }
        return u""_ustr;
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
            case ofaAutomaticFocus:     return u"automatic-focus"_ustr;
            case ofaApplyDesignMode:    return u"apply-design-mode"_ustr;
            default:
                OSL_FAIL("OAttributeMetaData::getOfficeFormsAttributeName: invalid id!");
        }
        return u""_ustr;
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
