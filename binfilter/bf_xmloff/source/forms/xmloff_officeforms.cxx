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

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_ 
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _XMLOFF_FORMS_OFFICEFORMS_HXX_
#include "officeforms.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX 
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLEXP_HXX 
#include "xmlexp.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX 
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _XMLOFF_FORMS_STRINGS_HXX_
#include "strings.hxx"
#endif
namespace binfilter {

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::xml;
    using ::binfilter::xmloff::token::XML_FORMS;

    //=========================================================================
    //= OFormsRootImport
    //=========================================================================
    TYPEINIT1(OFormsRootImport, SvXMLImportContext);
    //-------------------------------------------------------------------------
    OFormsRootImport::OFormsRootImport( SvXMLImport& rImport, sal_uInt16 nPrfx, const ::rtl::OUString& rLocalName )
        :SvXMLImportContext(rImport, nPrfx, rLocalName)
    {
    }

    //-------------------------------------------------------------------------
    OFormsRootImport::~OFormsRootImport()
    {
    }

    //-------------------------------------------------------------------------
    SvXMLImportContext* OFormsRootImport::CreateChildContext( USHORT _nPrefix, const ::rtl::OUString& _rLocalName,
            const Reference< sax::XAttributeList>& xAttrList )
    {
        return GetImport().GetFormImport()->createContext( _nPrefix, _rLocalName, xAttrList );
    }

    //-------------------------------------------------------------------------
    void OFormsRootImport::implImportBool(const Reference< sax::XAttributeList >& _rxAttributes, OfficeFormsAttributes _eAttribute,
            const Reference< XPropertySet >& _rxProps, const Reference< XPropertySetInfo >& _rxPropInfo,
            const ::rtl::OUString& _rPropName, sal_Bool _bDefault)
    {
        // the complete attribute name to look for
        ::rtl::OUString sCompleteAttributeName = GetImport().GetNamespaceMap().GetQNameByIndex(
            getOfficeFormsAttributeNamespace(_eAttribute),
            ::rtl::OUString::createFromAscii(getOfficeFormsAttributeName(_eAttribute)));

        // get and convert the value
        ::rtl::OUString sAttributeValue = _rxAttributes->getValueByName(sCompleteAttributeName);
        sal_Bool bValue = _bDefault;
        GetImport().GetMM100UnitConverter().convertBool(bValue, sAttributeValue);

        // set the property
        if (_rxPropInfo->hasPropertyByName(_rPropName))
            _rxProps->setPropertyValue(_rPropName, ::cppu::bool2any(bValue));
    }

    //-------------------------------------------------------------------------
    void OFormsRootImport::StartElement( const Reference< sax::XAttributeList >& _rxAttrList )
    {
        ENTER_LOG_CONTEXT( "xmloff::OFormsRootImport - importing the complete tree" );
        SvXMLImportContext::StartElement( _rxAttrList );

        try
        {
            Reference< XPropertySet > xDocProperties(GetImport().GetModel(), UNO_QUERY);
            if ( xDocProperties.is() )
            {	// an empty model is allowed: when doing a copy'n'paste from e.g. Writer to Calc,
                // this is done via streaming the controls as XML.
                Reference< XPropertySetInfo > xDocPropInfo;
                if (xDocProperties.is())
                    xDocPropInfo = xDocProperties->getPropertySetInfo();

                implImportBool(_rxAttrList, ofaAutomaticFocus, xDocProperties, xDocPropInfo, PROPERTY_AUTOCONTROLFOCUS, sal_False);
                implImportBool(_rxAttrList, ofaApplyDesignMode, xDocProperties, xDocPropInfo, PROPERTY_APPLYDESIGNMODE, sal_True);
            }
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "OFormsRootImport::StartElement: caught an exception while setting the document properties!");
        }
    }

    //-------------------------------------------------------------------------
    void OFormsRootImport::EndElement()
    {
        SvXMLImportContext::EndElement();
        LEAVE_LOG_CONTEXT( );
    }

    //=====================================================================
    //= OFormsRootExport
    //=====================================================================
    //---------------------------------------------------------------------
    OFormsRootExport::OFormsRootExport( SvXMLExport& _rExp )
        :m_pImplElement(NULL)
    {
        addModelAttributes(_rExp);

        m_pImplElement = new SvXMLElementExport(_rExp, XML_NAMESPACE_OFFICE, XML_FORMS, sal_True, sal_True);
    }

    //---------------------------------------------------------------------
    OFormsRootExport::~OFormsRootExport( )
    {
        delete m_pImplElement;
    }

    //-------------------------------------------------------------------------
    void OFormsRootExport::implExportBool(SvXMLExport& _rExp, OfficeFormsAttributes _eAttribute,
        const Reference< XPropertySet >& _rxProps, const Reference< XPropertySetInfo >& _rxPropInfo,
        const ::rtl::OUString& _rPropName, sal_Bool _bDefault)
    {
        // retrieve the property value
        sal_Bool bValue = _bDefault;
        if (_rxPropInfo->hasPropertyByName(_rPropName))
            bValue = ::cppu::any2bool(_rxProps->getPropertyValue(_rPropName));

        // convert into a string
        ::rtl::OUStringBuffer aValue;
        _rExp.GetMM100UnitConverter().convertBool(aValue, bValue);

        // add the attribute
        _rExp.AddAttribute(
            getOfficeFormsAttributeNamespace(_eAttribute),
            getOfficeFormsAttributeName(_eAttribute),
            aValue.makeStringAndClear());
    }

    //-------------------------------------------------------------------------
    void OFormsRootExport::addModelAttributes(SvXMLExport& _rExp) SAL_THROW(())
    {
        try
        {
            Reference< XPropertySet > xDocProperties(_rExp.GetModel(), UNO_QUERY);
            if ( xDocProperties.is() )
            {	// an empty model is allowed: when doing a copy'n'paste from e.g. Writer to Calc,
                // this is done via streaming the controls as XML.
                Reference< XPropertySetInfo > xDocPropInfo;
                if (xDocProperties.is())
                    xDocPropInfo = xDocProperties->getPropertySetInfo();

                implExportBool(_rExp, ofaAutomaticFocus, xDocProperties, xDocPropInfo, PROPERTY_AUTOCONTROLFOCUS, sal_False);
                implExportBool(_rExp, ofaApplyDesignMode, xDocProperties, xDocPropInfo, PROPERTY_APPLYDESIGNMODE, sal_True);
            }
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "OFormsRootExport::addModelAttributes: caught an exception while retrieving the document properties!");
        }
    }

//.........................................................................
}	// namespace xmloff
//.........................................................................


}//end of namespace binfilter
