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

#include "formattributes.hxx"
#include "xmlnmspe.hxx"
#include "xmluconv.hxx"
#include <rtl/ustrbuf.hxx>
namespace binfilter {

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;


    //=====================================================================
    //= OAttributeMetaData
    //=====================================================================
    //---------------------------------------------------------------------
    const sal_Char* OAttributeMetaData::getCommonControlAttributeName(sal_Int32 _nId)
    {
        switch (_nId)
        {
            case CCA_NAME: 				return "name";
            case CCA_SERVICE_NAME: 		return "service-name";
            case CCA_BUTTON_TYPE: 		return "button-type";
            case CCA_CONTROL_ID: 		return "id";
            case CCA_CURRENT_SELECTED: 	return "current-selected";
            case CCA_CURRENT_VALUE: 	return "current-value";
            case CCA_DISABLED: 			return "disabled";
            case CCA_DROPDOWN: 			return "dropdown";
            case CCA_FOR: 				return "for";
            case CCA_IMAGE_DATA: 		return "image-data";
            case CCA_LABEL: 			return "label";
            case CCA_MAX_LENGTH: 		return "max-length";
            case CCA_PRINTABLE: 		return "printable";
            case CCA_READONLY: 			return "readonly";
            case CCA_SELECTED: 			return "selected";
            case CCA_SIZE: 				return "size";
            case CCA_TAB_INDEX: 		return "tab-index";
            case CCA_TARGET_FRAME: 		return "target-frame";
            case CCA_TARGET_LOCATION: 	return "href";		// the only special thing here: TargetLocation is represented by an xlink:href attribute
            case CCA_TAB_STOP: 			return "tab-stop";
            case CCA_TITLE: 			return "title";
            case CCA_VALUE: 			return "value";
            default:
                OSL_ENSURE(sal_False, "OAttributeMetaData::getCommonControlAttributeName: invalid id (maybe you or-ed two flags?)!");
        }
        return "";
    }

    //---------------------------------------------------------------------
    sal_uInt16 OAttributeMetaData::getCommonControlAttributeNamespace(sal_Int32 _nId)
    {
        if (CCA_TARGET_LOCATION == _nId)
            return XML_NAMESPACE_XLINK;

        if (CCA_TARGET_FRAME == _nId)
            return XML_NAMESPACE_OFFICE;

        return XML_NAMESPACE_FORM;
    }

    //---------------------------------------------------------------------
    const sal_Char* OAttributeMetaData::getFormAttributeName(FormAttributes _eAttrib)
    {
        switch (_eAttrib)
        {
            case faName: 			return "name";
            case faServiceName: 	return "service-name";
            case faAction: 			return "href";		// the only special thing here: Action is represented by an xlink:href attribute
            case faEnctype: 		return "enctype";
            case faMethod: 			return "method";
            case faTargetFrame: 	return "target-frame";
            case faAllowDeletes:	return "allow-deletes";
            case faAllowInserts:	return "allow-inserts";
            case faAllowUpdates:	return "allow-updates";
            case faApplyFilter: 	return "apply-filter";
            case faCommand: 		return "command";
            case faCommandType: 	return "command-type";
            case faEscapeProcessing:return "escape-processing";
            case faDatasource: 		return "datasource";
            case faDetailFiels: 	return "detail-fields";
            case faFilter: 			return "filter";
            case faIgnoreResult: 	return "ignore-result";
            case faMasterFields: 	return "master-fields";
            case faNavigationMode:	return "navigation-mode";
            case faOrder: 			return "order";
            case faTabbingCycle: 	return "tab-cycle";
            default:
                OSL_ENSURE(sal_False, "OAttributeMetaData::getFormAttributeName: invalid id!");
        }
        return "";
    }

    //---------------------------------------------------------------------
    sal_uInt16 OAttributeMetaData::getFormAttributeNamespace(FormAttributes _eAttrib)
    {
        if (faAction == _eAttrib)
            return XML_NAMESPACE_XLINK;

        if (faTargetFrame == _eAttrib)
            return XML_NAMESPACE_OFFICE;

        return XML_NAMESPACE_FORM;
    }

    //---------------------------------------------------------------------
    const sal_Char* OAttributeMetaData::getDatabaseAttributeName(sal_Int32 _nId)
    {
        switch (_nId)
        {
            case DA_BOUND_COLUMN:		return "bound-column";
            case DA_CONVERT_EMPTY:		return "convert-empty-to-null";
            case DA_DATA_FIELD:			return "data-field";
            case DA_LIST_SOURCE:		return "list-source";
            case DA_LIST_SOURCE_TYPE:	return "list-source-type";
            default:
                OSL_ENSURE(sal_False, "OAttributeMetaData::getDatabaseAttributeName: invalid id (maybe you or-ed two flags?)!");
        }
        return "";
    }

    //---------------------------------------------------------------------
    sal_uInt16 OAttributeMetaData::getDatabaseAttributeNamespace(sal_Int32 /*_nId*/)
    {
        // nothing special here
        return XML_NAMESPACE_FORM;
    }

    //---------------------------------------------------------------------
    const sal_Char* OAttributeMetaData::getBindingAttributeName(sal_Int32 _nId)
    {
        switch (_nId)
        {
            case BA_LINKED_CELL:       return "linked-cell";
            case BA_LIST_LINKING_TYPE: return "list-linkage-type";
            case BA_LIST_CELL_RANGE:   return "source-cell-range";
            default:
                OSL_ENSURE(sal_False, "OAttributeMetaData::getBindingAttributeName: invalid id (maybe you or-ed two flags?)!");
        }
        return "";
    }

    //---------------------------------------------------------------------
    sal_uInt16 OAttributeMetaData::getBindingAttributeNamespace(sal_Int32 _nId)
    {
        // nothing special here
        return XML_NAMESPACE_FORM;
    }

    //---------------------------------------------------------------------
    const sal_Char* OAttributeMetaData::getSpecialAttributeName(sal_Int32 _nId)
    {
        switch (_nId)
        {
            case SCA_ECHO_CHAR: 			return "echo-char";
            case SCA_MAX_VALUE:				return "max-value";
            case SCA_MIN_VALUE:				return "min-value";
            case SCA_VALIDATION:			return "validation";
            case SCA_MULTI_LINE:			return "multi-line";
            case SCA_AUTOMATIC_COMPLETION:	return "auto-complete";
            case SCA_MULTIPLE: 				return "multiple";
            case SCA_DEFAULT_BUTTON: 		return "default-button";
            case SCA_CURRENT_STATE: 		return "current-state";
            case SCA_IS_TRISTATE: 			return "is-tristate";
            case SCA_STATE: 				return "state";
            case SCA_COLUMN_STYLE_NAME:		return "column-style-name";
            default:
                OSL_ENSURE(sal_False, "OAttributeMetaData::getSpecialAttributeName: invalid id (maybe you or-ed two flags?)!");
        }
        return "";
    }

    //---------------------------------------------------------------------
    sal_uInt16 OAttributeMetaData::getSpecialAttributeNamespace(sal_Int32 /*_nId*/)
    {
        // nothing special here
        return XML_NAMESPACE_FORM;
    }

    //---------------------------------------------------------------------
    const sal_Char* OAttributeMetaData::getOfficeFormsAttributeName(OfficeFormsAttributes _eAttrib)
    {
        switch (_eAttrib)
        {
            case ofaAutomaticFocus:		return "automatic-focus";
            case ofaApplyDesignMode:	return "apply-design-mode";
            default:
                OSL_ENSURE(sal_False, "OAttributeMetaData::getOfficeFormsAttributeName: invalid id!");
        }
        return "";
    }

    //---------------------------------------------------------------------
    sal_uInt16 OAttributeMetaData::getOfficeFormsAttributeNamespace(OfficeFormsAttributes /* _eAttrib */)
    {
        // nothing special here
        return XML_NAMESPACE_FORM;
    }

    //=====================================================================
    //= OAttribute2Property
    //=====================================================================
    //---------------------------------------------------------------------
    OAttribute2Property::OAttribute2Property()
    {
    }

    //---------------------------------------------------------------------
    OAttribute2Property::~OAttribute2Property()
    {
    }

    //---------------------------------------------------------------------
    const OAttribute2Property::AttributeAssignment* OAttribute2Property::getAttributeTranslation(
            const ::rtl::OUString& _rAttribName)
    {
        ConstAttributeAssignmentsIterator aPos = m_aKnownProperties.find(_rAttribName);
        if (m_aKnownProperties.end() != aPos)
            return &aPos->second;
        return NULL;
    }

    //---------------------------------------------------------------------
    void OAttribute2Property::addStringProperty(
        const sal_Char* _pAttributeName, const ::rtl::OUString& _rPropertyName,
        const sal_Char* _pAttributeDefault)
    {
        implAdd(_pAttributeName, _rPropertyName, ::getCppuType( static_cast< ::rtl::OUString* >(NULL) ),
            _pAttributeDefault ? ::rtl::OUString::createFromAscii(_pAttributeDefault) : ::rtl::OUString());
    }

    //---------------------------------------------------------------------
    void OAttribute2Property::addBooleanProperty(
        const sal_Char* _pAttributeName, const ::rtl::OUString& _rPropertyName,
        const sal_Bool _bAttributeDefault, const sal_Bool _bInverseSemantics)
    {
        ::rtl::OUStringBuffer aDefault;
        SvXMLUnitConverter::convertBool(aDefault, _bAttributeDefault);
        AttributeAssignment& aAssignment = implAdd(_pAttributeName, _rPropertyName, ::getBooleanCppuType(), aDefault.makeStringAndClear());
        aAssignment.bInverseSemantics = _bInverseSemantics;
    }

    //---------------------------------------------------------------------
    void OAttribute2Property::addInt16Property(
        const sal_Char* _pAttributeName, const ::rtl::OUString& _rPropertyName,
        const sal_Int16 _nAttributeDefault)
    {
        ::rtl::OUStringBuffer aDefault;
        SvXMLUnitConverter::convertNumber(aDefault, (sal_Int32)_nAttributeDefault);
        implAdd(_pAttributeName, _rPropertyName, ::getCppuType( static_cast< sal_Int16* >(NULL) ), aDefault.makeStringAndClear());
    }

    //---------------------------------------------------------------------
    void OAttribute2Property::addEnumProperty(
            const sal_Char* _pAttributeName, const ::rtl::OUString& _rPropertyName,
            const sal_uInt16 _nAttributeDefault, const SvXMLEnumMapEntry* _pValueMap,
            const ::com::sun::star::uno::Type* _pType)
    {
        ::rtl::OUStringBuffer aDefault;
        SvXMLUnitConverter::convertEnum(aDefault, _nAttributeDefault, _pValueMap);
        AttributeAssignment& aAssignment = implAdd(_pAttributeName, _rPropertyName,
            _pType ? *_pType : ::getCppuType( static_cast< sal_Int32* >(NULL) ),
                // this assumes that the setPropertyValue for enums can handle int32's ....
            aDefault.makeStringAndClear());
        aAssignment.pEnumMap = _pValueMap;
    }

    //---------------------------------------------------------------------
    OAttribute2Property::AttributeAssignment& OAttribute2Property::implAdd(
            const sal_Char* _pAttributeName, const ::rtl::OUString& _rPropertyName,
            const ::com::sun::star::uno::Type& _rType, const ::rtl::OUString& _rDefaultString)
    {
        OSL_ENSURE(m_aKnownProperties.end() == m_aKnownProperties.find(::rtl::OUString::createFromAscii(_pAttributeName)),
            "OAttribute2Property::implAdd: already have this attribute!");

        ::rtl::OUString sAttributeName = ::rtl::OUString::createFromAscii(_pAttributeName);

        AttributeAssignment aAssignment;
        aAssignment.sAttributeName = sAttributeName;
        aAssignment.sPropertyName = _rPropertyName;
        aAssignment.aPropertyType = _rType;

        // redundance, the accessor is stored in aAssignment.sAttributeName, too
        return m_aKnownProperties[sAttributeName] = aAssignment;
    }

//.........................................................................
}	// namespace xmloff
//.........................................................................

}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
