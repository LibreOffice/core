/*************************************************************************
 *
 *  $RCSfile: propertyexport.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-17 19:03:06 $
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

#ifndef _XMLOFF_FORMS_PROPERTYEXPORT_HXX_
#include "propertyexport.hxx"
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif
#ifndef _XMLOFF_FORMS_STRINGS_HXX_
#include "strings.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XPERSISTOBJECT_HPP_
#include <com/sun/star/io/XPersistObject.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

#ifndef _UNOTOOLS_DATETIME_HXX_
#include <unotools/datetime.hxx>
#endif
#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif
#ifndef _TOOLS_TIME_HXX
#include <tools/time.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::io;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

    using namespace ::comphelper;

    //=====================================================================
    //= OPropertyExport
    //=====================================================================
    //---------------------------------------------------------------------
    OPropertyExport::OPropertyExport(SvXMLExport& _rContext, const Reference< XPropertySet >& _rxProps)
        :m_rContext(_rContext)
        ,m_xProps(_rxProps)
    {
        // collect the properties which need to be exported
        examinePersistence();
    }

    //---------------------------------------------------------------------
    void OPropertyExport::exportRemainingProperties()
    {
        // TODO
    }

    //---------------------------------------------------------------------
    void OPropertyExport::examinePersistence()
    {
        m_aRemainingProps.clear();
        Reference< XPropertySetInfo > xInfo = m_xProps->getPropertySetInfo();
        if (xInfo.is())
        {
            Sequence< Property > aProperties = xInfo->getProperties();
            const Property* pProperties = aProperties.getConstArray();
            for (sal_Int32 i=0; i<aProperties.getLength(); ++i, ++pProperties)
            {
                static sal_Int32 nExcludeIndicator = PropertyAttribute::READONLY | PropertyAttribute::TRANSIENT;
                if (pProperties->Attributes & nExcludeIndicator)
                    // no readonly props, no transient props
                    continue;
                m_aRemainingProps.insert(pProperties->Name);
            }
        }
    }

    //---------------------------------------------------------------------
    void OPropertyExport::implAddStringPropAttribute(const sal_uInt16 _nNamespaceKey, const sal_Char* _pAttributeName,
            const sal_Char* _pPropertyName)
    {
        DBG_CHECK_PROPERTY(_pPropertyName, ::rtl::OUString);

        // no try-catch here, this would be to expensive. The outer scope has to handle exceptions (which should not
        // happen if we're used correctly :)

        // this is way simple, as we don't need to convert anything (the property already is a string)

        // get the string
        ::rtl::OUString sPropValue;
        m_xProps->getPropertyValue(::rtl::OUString::createFromAscii(_pPropertyName)) >>= sPropValue;

        // add the attribute
        if (sPropValue.getLength())
            AddAttribute(_nNamespaceKey, _pAttributeName, sPropValue);
    }

    //---------------------------------------------------------------------
    void OPropertyExport::implAddBooleanPropAttribute(const sal_uInt16 _nNamespaceKey, const sal_Char* _pAttributeName,
            const sal_Char* _pPropertyName, const sal_Bool _bDefault, const sal_Bool _bInverseSemantics)
    {
        DBG_CHECK_PROPERTY_NO_TYPE(_pPropertyName);
        // no check of the property value type: this method is allowed to be called with any interger properties
        // (e.g. sal_Int32, sal_uInt16 etc)

        // get the value
        sal_Bool bCurrentValue = _bDefault;
        Any aCurrentValue = m_xProps->getPropertyValue(::rtl::OUString::createFromAscii(_pPropertyName));
        if (aCurrentValue.hasValue())
            bCurrentValue = ::cppu::any2bool(aCurrentValue);
            // this will extract a boolean value even if the Any contains a int or short or something like that ...

        if (_bInverseSemantics)
            bCurrentValue = !bCurrentValue;

        // add the attribute
        if (_bDefault != bCurrentValue)
        {
            // let the formatter of the export context build a string
            ::rtl::OUStringBuffer sBuffer;
            m_rContext.GetMM100UnitConverter().convertBool(sBuffer, bCurrentValue);

            AddAttribute(_nNamespaceKey, _pAttributeName, sBuffer.makeStringAndClear());
        }
    }

    //---------------------------------------------------------------------
    void OPropertyExport::implAddInt16PropAttribute(const sal_uInt16 _nNamespaceKey, const sal_Char* _pAttributeName,
            const sal_Char* _pPropertyName, const sal_Int16 _nDefault)
    {
        DBG_CHECK_PROPERTY(_pPropertyName, sal_Int16);

        // get the value
        sal_Int16 nCurrentValue(_nDefault);
        m_xProps->getPropertyValue(::rtl::OUString::createFromAscii(_pPropertyName)) >>= nCurrentValue;

        // add the attribute
        if (_nDefault != nCurrentValue)
        {
            // let the formatter of the export context build a string
            ::rtl::OUStringBuffer sBuffer;
            m_rContext.GetMM100UnitConverter().convertNumber(sBuffer, (sal_Int32)nCurrentValue);

            AddAttribute(_nNamespaceKey, _pAttributeName, sBuffer.makeStringAndClear());
        }
    }

    //---------------------------------------------------------------------
    void OPropertyExport::implAddEnumPropAttribute(
            const sal_uInt16 _nNamespaceKey, const sal_Char* _pAttributeName,
            const sal_Char* _pPropertyName, const SvXMLEnumMapEntry* _pValueMap, const sal_Int32 _nDefault)
    {
        // get the value
        sal_Int32 nCurrentValue(_nDefault);
        ::cppu::enum2int(nCurrentValue, m_xProps->getPropertyValue(::rtl::OUString::createFromAscii(_pPropertyName)));

        // add the attribute
        if (_nDefault != nCurrentValue)
        {
            // let the formatter of the export context build a string
            ::rtl::OUStringBuffer sBuffer;
            m_rContext.GetMM100UnitConverter().convertEnum(sBuffer, (sal_uInt16)nCurrentValue, _pValueMap);

            AddAttribute(_nNamespaceKey, _pAttributeName, sBuffer.makeStringAndClear());
        }
    }

    //---------------------------------------------------------------------
    void OPropertyExport::implExportTargetFrame()
    {
        DBG_CHECK_PROPERTY((const sal_Char*)PROPERTY_TARGETFRAME, ::rtl::OUString);

        ::rtl::OUString sTargetFrame = comphelper::getString(m_xProps->getPropertyValue(PROPERTY_TARGETFRAME));
        if (sTargetFrame.getLength() && (0 != sTargetFrame.compareToAscii("_blank")))
        {   // an empty string and "_blank" have the same meaning and don't have to be written
            AddAttribute(XML_NAMESPACE_XLINK, "href", sTargetFrame);
        }
        m_aRemainingProps.erase(PROPERTY_TARGETFRAME);
    }

    //---------------------------------------------------------------------
    void OPropertyExport::implExportServiceName()
    {
        Reference< XPersistObject > xPersistence(m_xProps, UNO_QUERY);
        if (!xPersistence.is())
        {
            OSL_ENSURE(sal_False, "OPropertyExport::implExportServiceName: no XPersistObject!");
            return;
        }

        ::rtl::OUString sServiceName = xPersistence->getServiceName();
        // we don't want to write the old service name directly: it's a name used for compatibility reasons, but
        // as we start some kind of new file format here (with this xml export), we don't care about
        // compatibility ...
        // So we translate the old persistence service name into new ones, if possible

        ::rtl::OUString sToWriteServiceName = sServiceName;
#define CHECK_N_TRANSLATE( name )   \
        else if (0 == sServiceName.compareToAscii(SERVICE_PERSISTENT_COMPONENT_##name)) \
            sToWriteServiceName = SERVICE_##name

        if (sal_False)
            ;
        CHECK_N_TRANSLATE( FORM );
        CHECK_N_TRANSLATE( FORM );
        CHECK_N_TRANSLATE( LISTBOX );
        CHECK_N_TRANSLATE( COMBOBOX );
        CHECK_N_TRANSLATE( RADIOBUTTON );
        CHECK_N_TRANSLATE( GROUPBOX );
        CHECK_N_TRANSLATE( FIXEDTEXT );
        CHECK_N_TRANSLATE( COMMANDBUTTON );
        CHECK_N_TRANSLATE( CHECKBOX );
        CHECK_N_TRANSLATE( GRID );
        CHECK_N_TRANSLATE( IMAGEBUTTON );
        CHECK_N_TRANSLATE( FILECONTROL );
        CHECK_N_TRANSLATE( TIMEFIELD );
        CHECK_N_TRANSLATE( DATEFIELD );
        CHECK_N_TRANSLATE( NUMERICFIELD );
        CHECK_N_TRANSLATE( CURRENCYFIELD );
        CHECK_N_TRANSLATE( PATTERNFIELD );
        CHECK_N_TRANSLATE( HIDDENCONTROL );
        CHECK_N_TRANSLATE( IMAGECONTROL );
        CHECK_N_TRANSLATE( FORMATTEDFIELD );
        else if (0 == sServiceName.compareToAscii(SERVICE_PERSISTENT_COMPONENT_EDIT))
        {   // special handling for the edit field: we have to controls using this as persistence service name
            sToWriteServiceName = SERVICE_EDIT;
            Reference< XServiceInfo > xSI(m_xProps, UNO_QUERY);
            if (xSI.is() && xSI->supportsService(SERVICE_FORMATTEDFIELD))
                sToWriteServiceName = SERVICE_FORMATTEDFIELD;
        }
#ifdef DBG_UTIL
        Reference< XServiceInfo > xSI(m_xProps, UNO_QUERY);
        OSL_ENSURE(xSI.is() && xSI->supportsService(sToWriteServiceName),
            "OPropertyExport::implExportServiceName: wrong service name translation!");

#endif

        // now write this
        AddAttribute(XML_NAMESPACE_FORM, "service-name", sToWriteServiceName);
    }

    //---------------------------------------------------------------------
    void OPropertyExport::implExportGenericPropertyAttribute(
            const sal_uInt16 _nAttributeNamespaceKey, const sal_Char* _pAttributeName, const sal_Char* _pPropertyName)
    {
        DBG_CHECK_PROPERTY_NO_TYPE(_pPropertyName);

        ::rtl::OUString sPropertyName = ::rtl::OUString::createFromAscii(_pPropertyName);
        Any aCurrentValue = m_xProps->getPropertyValue(sPropertyName);
        if (!aCurrentValue.hasValue())
            // nothing to do without a concrete value
            return;

        ::rtl::OUString sValue = implConvertAny(aCurrentValue);
        if (!sValue.getLength() && (TypeClass_STRING == aCurrentValue.getValueTypeClass()))
        {
            // check whether or not the property is allowed to be VOID
            Reference< XPropertySetInfo > xPropInfo = m_xProps->getPropertySetInfo();
            if (xPropInfo.is())
            {
                Property aProperty = xPropInfo->getPropertyByName(sPropertyName);
                if ((aProperty.Attributes & PropertyAttribute::MAYBEVOID) == 0)
                    // the string is empty, and the property is not allowed to be void
                    // -> don't need to write the attibute, 'cause missing it is unambiguous
                    return;
            }
        }

        // finally add the attribuite to the context
        AddAttribute(_nAttributeNamespaceKey, _pAttributeName, sValue);

        m_aRemainingProps.erase(sPropertyName);
    }

    //---------------------------------------------------------------------
    ::rtl::OUString OPropertyExport::implConvertAny(const Any& _rValue)
    {
        ::rtl::OUStringBuffer aBuffer;
        switch (_rValue.getValueTypeClass())
        {
            case TypeClass_STRING:
            {   // extract the string
                ::rtl::OUString sCurrentValue;
                _rValue >>= sCurrentValue;
                aBuffer.append(sCurrentValue);
            }
            break;
            case TypeClass_DOUBLE:
                // let the unit converter format is as string
                m_rContext.GetMM100UnitConverter().convertNumber(aBuffer, getDouble(_rValue));
                break;
            case TypeClass_BOOLEAN:
                // let the unit converter format is as string
                m_rContext.GetMM100UnitConverter().convertBool(aBuffer, getBOOL(_rValue));
                break;
            case TypeClass_BYTE:
            case TypeClass_SHORT:
            case TypeClass_LONG:
                // let the unit converter format is as string
                m_rContext.GetMM100UnitConverter().convertNumber(aBuffer, getINT32(_rValue));
                break;
            case TypeClass_HYPER:
                // TODO
                OSL_ENSURE(sal_False, "OPropertyExport::implExportGenericPropertyAttribute: missing implementation for sal_Int64!");
                break;
            default:
            {   // hmmm .... what else do we know?
                double fValue = 0;
                ::com::sun::star::util::Date aDate;
                ::com::sun::star::util::Time aTime;
                ::com::sun::star::util::DateTime aDateTime;
                if (_rValue >>= aDate)
                {
                    Date aToolsDate;
                    ::utl::typeConvert(aDate, aToolsDate);
                    fValue = aToolsDate.GetDate();
                }
                else if (_rValue >>= aTime)
                {
                    fValue = ((aTime.Hours * 60 + aTime.Minutes) * 60 + aTime.Seconds) * 100 + aTime.HundredthSeconds;
                    fValue = fValue / 8640000.0;
                }
                else if (_rValue >>= aDateTime)
                {
                    DateTime aToolsDateTime;
                    ::utl::typeConvert(aDateTime, aToolsDateTime);
                    // the time part (the digits behind the comma)
                    fValue = ((aDateTime.Hours * 60 + aDateTime.Minutes) * 60 + aDateTime.Seconds) * 100 + aDateTime.HundredthSeconds;
                    fValue = fValue / 8640000.0;
                    // plus the data part (the digits in front of the comma)
                    fValue += aToolsDateTime.GetDate();
                }
                else
                {
                    // no more options ...
                    OSL_ENSURE(sal_False, "OPropertyExport::implExportGenericPropertyAttribute: unsupported value type!");
                    break;
                }
                // let the unit converter format is as string
                m_rContext.GetMM100UnitConverter().convertNumber(aBuffer, fValue);
            }
            break;
        }

        return aBuffer.makeStringAndClear();
    }

#ifdef DBG_UTIL
    //---------------------------------------------------------------------
    void OPropertyExport::AddAttribute(sal_uInt16 _nPrefix, const sal_Char* _pName, const ::rtl::OUString& _rValue)
    {
        OSL_ENSURE(0 == m_rContext.GetXAttrList()->getValueByName(::rtl::OUString::createFromAscii(_pName)).getLength(),
            "OPropertyExport::AddAttribute: already have such an attribute");

        m_rContext.AddAttribute(_nPrefix, _pName, _rValue);
    }

    //---------------------------------------------------------------------
    void OPropertyExport::dbg_implCheckProperty(const ::rtl::OUString& _rPropertyName, const Type* _pType)
    {
        try
        {
            // we need a PropertySetInfo
            Reference< XPropertySetInfo > xPropInfo = m_xProps->getPropertySetInfo();
            if (!xPropInfo.is())
            {
                OSL_ENSURE(sal_False, "OPropertyExport::dbg_implCheckProperty: no PropertySetInfo!");
                return;
            }

            // the property must exist
            if (!xPropInfo->hasPropertyByName(_rPropertyName))
            {
                OSL_ENSURE(sal_False,
                    ::rtl::OString("OPropertyExport::dbg_implCheckProperty: no property with the name ") +=
                    ::rtl::OString(_rPropertyName.getStr(), _rPropertyName.getLength(), RTL_TEXTENCODING_ASCII_US) +=
                    ::rtl::OString("!"));
                return;
            }

            if (_pType)
            {
                // and it must have the correct type
                Property aPropertyDescription = xPropInfo->getPropertyByName(_rPropertyName);
                OSL_ENSURE(aPropertyDescription.Type.equals(*_pType), "OPropertyExport::dbg_implCheckProperty: invalid property type!");
            }
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "OPropertyExport::dbg_implCheckProperty: caught an exception, could not check the property!");
        }
    }
#endif // DBG_UTIL - dbg_implCheckProperty

//.........................................................................
}   // namespace xmloff
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 15.11.00 17:50:20  fs
 ************************************************************************/

