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


#include "officeforms.hxx"

#include <sax/tools/converter.hxx>

#include <xmloff/xmltoken.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <comphelper/extract.hxx>
#include "strings.hxx"

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::xml;
    using ::xmloff::token::XML_FORMS;
    using ::com::sun::star::xml::sax::XAttributeList;

    //=========================================================================
    //= OFormsRootImport
    //=========================================================================
    TYPEINIT1(OFormsRootImport, SvXMLImportContext);
    //-------------------------------------------------------------------------
    OFormsRootImport::OFormsRootImport( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName )
        :SvXMLImportContext(rImport, nPrfx, rLocalName)
    {
    }

    //-------------------------------------------------------------------------
    OFormsRootImport::~OFormsRootImport()
    {
    }

    //-------------------------------------------------------------------------
    SvXMLImportContext* OFormsRootImport::CreateChildContext( sal_uInt16 _nPrefix, const OUString& _rLocalName,
            const Reference< XAttributeList>& xAttrList )
    {
        return GetImport().GetFormImport()->createContext( _nPrefix, _rLocalName, xAttrList );
    }

    //-------------------------------------------------------------------------
    void OFormsRootImport::implImportBool(const Reference< XAttributeList >& _rxAttributes, OfficeFormsAttributes _eAttribute,
            const Reference< XPropertySet >& _rxProps, const Reference< XPropertySetInfo >& _rxPropInfo,
            const OUString& _rPropName, sal_Bool _bDefault)
    {
        // the complete attribute name to look for
        OUString sCompleteAttributeName = GetImport().GetNamespaceMap().GetQNameByIndex(
            OAttributeMetaData::getOfficeFormsAttributeNamespace(_eAttribute),
            OUString::createFromAscii(OAttributeMetaData::getOfficeFormsAttributeName(_eAttribute)));

        // get and convert the value
        OUString sAttributeValue = _rxAttributes->getValueByName(sCompleteAttributeName);
        bool bValue = _bDefault;
        ::sax::Converter::convertBool(bValue, sAttributeValue);

        // set the property
        if (_rxPropInfo->hasPropertyByName(_rPropName))
        {
            _rxProps->setPropertyValue(_rPropName, makeAny(bValue));
        }
    }

    //-------------------------------------------------------------------------
    void OFormsRootImport::StartElement( const Reference< XAttributeList >& _rxAttrList )
    {
        ENTER_LOG_CONTEXT( "xmloff::OFormsRootImport - importing the complete tree" );
        SvXMLImportContext::StartElement( _rxAttrList );

        try
        {
            Reference< XPropertySet > xDocProperties(GetImport().GetModel(), UNO_QUERY);
            if ( xDocProperties.is() )
            {   // an empty model is allowed: when doing a copy'n'paste from e.g. Writer to Calc,
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
            OSL_FAIL("OFormsRootImport::StartElement: caught an exception while setting the document properties!");
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
        const OUString& _rPropName, sal_Bool _bDefault)
    {
        // retrieve the property value
        sal_Bool bValue = _bDefault;
        if (_rxPropInfo->hasPropertyByName(_rPropName))
            bValue = ::cppu::any2bool(_rxProps->getPropertyValue(_rPropName));

        // convert into a string
        OUStringBuffer aValue;
        ::sax::Converter::convertBool(aValue, bValue);

        // add the attribute
        _rExp.AddAttribute(
            OAttributeMetaData::getOfficeFormsAttributeNamespace(_eAttribute),
            OAttributeMetaData::getOfficeFormsAttributeName(_eAttribute),
            aValue.makeStringAndClear());
    }

    //-------------------------------------------------------------------------
    void OFormsRootExport::addModelAttributes(SvXMLExport& _rExp) SAL_THROW(())
    {
        try
        {
            Reference< XPropertySet > xDocProperties(_rExp.GetModel(), UNO_QUERY);
            if ( xDocProperties.is() )
            {   // an empty model is allowed: when doing a copy'n'paste from e.g. Writer to Calc,
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
            OSL_FAIL("OFormsRootExport::addModelAttributes: caught an exception while retrieving the document properties!");
        }
    }

//.........................................................................
}   // namespace xmloff
//.........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
