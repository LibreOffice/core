/*************************************************************************
 *
 *  $RCSfile: formattributes.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: fs $ $Date: 2001-02-13 13:44:41 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_FORMATTRIBUTES_HXX_
#include "formattributes.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

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
            case CCA_NAME:              return "name";
            case CCA_SERVICE_NAME:      return "service-name";
            case CCA_BUTTON_TYPE:       return "button-type";
            case CCA_CONTROL_ID:        return "id";
            case CCA_CURRENT_SELECTED:  return "current-selected";
            case CCA_CURRENT_VALUE:     return "current-value";
            case CCA_DISABLED:          return "disabled";
            case CCA_DROPDOWN:          return "dropdown";
            case CCA_FOR:               return "for";
            case CCA_IMAGE_DATA:        return "image-data";
            case CCA_LABEL:             return "label";
            case CCA_MAX_LENGTH:        return "max-length";
            case CCA_PRINTABLE:         return "printable";
            case CCA_READONLY:          return "readonly";
            case CCA_SELECTED:          return "selected";
            case CCA_SIZE:              return "size";
            case CCA_TAB_INDEX:         return "tab-index";
            case CCA_TARGET_FRAME:      return "target-frame";
            case CCA_TARGET_LOCATION:   return "href";      // the only special thing here: TargetLocation is represented by an xlink:href attribute
            case CCA_TAB_STOP:          return "tab-stop";
            case CCA_TITLE:             return "title";
            case CCA_VALUE:             return "value";
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
            case faName:            return "name";
            case faServiceName:     return "service-name";
            case faAction:          return "href";      // the only special thing here: Action is represented by an xlink:href attribute
            case faEnctype:         return "enc-type";
            case faMethod:          return "method";
            case faTargetFrame:     return "target-frame";
            case faAllowDeletes:    return "allow-deletes";
            case faAllowInserts:    return "allow-inserts";
            case faAllowUpdates:    return "allow-updates";
            case faApplyFilter:     return "apply-filter";
            case faCommand:         return "command";
            case faCommandType:     return "command-type";
            case faEscapeProcessing:return "escape-processing";
            case faDatasource:      return "datasource";
            case faDetailFiels:     return "detail-fiels";
            case faFilter:          return "filter";
            case faIgnoreResult:    return "ignore-result";
            case faMasterFields:    return "master-fields";
            case faNavigationMode:  return "navigation-mode";
            case faOrder:           return "order";
            case faTabbingCycle:    return "tabbing-cycle";
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
            case DA_BOUND_COLUMN:       return "bound-column";
            case DA_CONVERT_EMPTY:      return "convert-empty-to-null";
            case DA_DATA_FIELD:         return "data-field";
            case DA_LIST_SOURCE:        return "list-source";
            case DA_LIST_SOURCE_TYPE:   return "list-source-type";
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
    const sal_Char* OAttributeMetaData::getSpecialAttributeName(sal_Int32 _nId)
    {
        switch (_nId)
        {
            case SCA_ECHO_CHAR:             return "echo-char";
            case SCA_MAX_VALUE:             return "max-value";
            case SCA_MIN_VALUE:             return "min-value";
            case SCA_VALIDATION:            return "validation";
            case SCA_MULTI_LINE:            return "multi-line";
            case SCA_AUTOMATIC_COMPLETION:  return "auto-complete";
            case SCA_MULTIPLE:              return "multiple";
            case SCA_DEFAULT_BUTTON:        return "default-button";
            case SCA_CURRENT_STATE:         return "current-state";
            case SCA_IS_TRISTATE:           return "is-tristate";
            case SCA_STATE:                 return "state";
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
}   // namespace xmloff
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.5  2001/01/03 16:25:34  fs
 *  file format change (extra wrapper element for controls, similar to columns)
 *
 *  Revision 1.4  2000/12/12 12:01:05  fs
 *  new implementations for the import - still under construction
 *
 *  Revision 1.3  2000/12/06 17:28:05  fs
 *  changes for the formlayer import - still under construction
 *
 *  Revision 1.2  2000/11/19 15:41:32  fs
 *  extended the export capabilities - generic controls / grid columns / generic properties / some missing form properties
 *
 *  Revision 1.1  2000/11/17 19:01:36  fs
 *  initial checkin - export and/or import the applications form layer
 *
 *
 *  Revision 1.0 14.11.00 09:53:05  fs
 ************************************************************************/

