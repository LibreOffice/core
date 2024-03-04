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
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <comphelper/extract.hxx>
#include <comphelper/diagnose_ex.hxx>
#include "strings.hxx"

namespace xmloff
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::xml;
    using ::xmloff::token::XML_FORMS;
    using ::com::sun::star::xml::sax::XFastAttributeList;

    //= OFormsRootImport
    OFormsRootImport::OFormsRootImport( SvXMLImport& rImport )
        :SvXMLImportContext(rImport)
    {
    }

    OFormsRootImport::~OFormsRootImport()
    {
    }

    css::uno::Reference< css::xml::sax::XFastContextHandler > OFormsRootImport::createFastChildContext(
        sal_Int32 _nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList )
    {
        SvXMLImportContext* pRet = nullptr;
        try
        {
            pRet = GetImport().GetFormImport()->createContext( _nElement, xAttrList );
        } catch (const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("xmloff.forms");
        }
        return pRet;
    }

    void OFormsRootImport::implImportBool(const Reference< XFastAttributeList >& _rxAttributes, OfficeFormsAttributes _eAttribute,
            const Reference< XPropertySet >& _rxProps, const Reference< XPropertySetInfo >& _rxPropInfo,
            const OUString& _rPropName, bool _bDefault)
    {
        // the complete attribute name to look for
        sal_Int32 nCompleteAttributeName = XML_ELEMENT(
            FORM,
            OAttributeMetaData::getOfficeFormsAttributeToken(_eAttribute));

        // get and convert the value
        OUString sAttributeValue = _rxAttributes->getOptionalValue(nCompleteAttributeName);
        bool bValue = _bDefault;
        (void)::sax::Converter::convertBool(bValue, sAttributeValue);

        // set the property
        if (_rxPropInfo->hasPropertyByName(_rPropName))
        {
            _rxProps->setPropertyValue(_rPropName, Any(bValue));
        }
    }

    void OFormsRootImport::startFastElement( sal_Int32 /*nElement*/, const Reference< XFastAttributeList >& _rxAttrList )
    {
        ENTER_LOG_CONTEXT( "xmloff::OFormsRootImport - importing the complete tree" );

        try
        {
            Reference< XPropertySet > xDocProperties(GetImport().GetModel(), UNO_QUERY);
            if ( xDocProperties.is() )
            {   // an empty model is allowed: when doing a copy'n'paste from e.g. Writer to Calc,
                // this is done via streaming the controls as XML.
                Reference< XPropertySetInfo > xDocPropInfo;
                if (xDocProperties.is())
                    xDocPropInfo = xDocProperties->getPropertySetInfo();

                implImportBool(_rxAttrList, ofaAutomaticFocus, xDocProperties, xDocPropInfo, PROPERTY_AUTOCONTROLFOCUS, false);
                implImportBool(_rxAttrList, ofaApplyDesignMode, xDocProperties, xDocPropInfo, PROPERTY_APPLYDESIGNMODE, true);
            }
        }
        catch(Exception&)
        {
            TOOLS_WARN_EXCEPTION("xmloff.forms",
                                 "caught an exception while setting the document properties!");
        }
    }

    void OFormsRootImport::endFastElement(sal_Int32 )
    {
        LEAVE_LOG_CONTEXT( );
    }

    //= OFormsRootExport
    OFormsRootExport::OFormsRootExport( SvXMLExport& _rExp )
    {
        addModelAttributes(_rExp);

        m_pImplElement.reset( new SvXMLElementExport(_rExp, XML_NAMESPACE_OFFICE, XML_FORMS, true, true) );
    }

    OFormsRootExport::~OFormsRootExport( )
    {
    }

    void OFormsRootExport::implExportBool(SvXMLExport& _rExp, OfficeFormsAttributes _eAttribute,
        const Reference< XPropertySet >& _rxProps, const Reference< XPropertySetInfo >& _rxPropInfo,
        const OUString& _rPropName, bool _bDefault)
    {
        // retrieve the property value
        bool bValue = _bDefault;
        if (_rxPropInfo->hasPropertyByName(_rPropName))
            bValue = ::cppu::any2bool(_rxProps->getPropertyValue(_rPropName));

        // convert into a string
        OUStringBuffer aValue;
        ::sax::Converter::convertBool(aValue, bValue);

        // add the attribute
        _rExp.AddAttribute(
            OAttributeMetaData::getOfficeFormsAttributeNamespace(),
            OAttributeMetaData::getOfficeFormsAttributeName(_eAttribute),
            aValue.makeStringAndClear());
    }

    void OFormsRootExport::addModelAttributes(SvXMLExport& _rExp)
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

                implExportBool(_rExp, ofaAutomaticFocus, xDocProperties, xDocPropInfo, PROPERTY_AUTOCONTROLFOCUS, false);
                implExportBool(_rExp, ofaApplyDesignMode, xDocProperties, xDocPropInfo, PROPERTY_APPLYDESIGNMODE, true);
            }
        }
        catch(Exception&)
        {
            TOOLS_WARN_EXCEPTION("xmloff.forms",
                                 "caught an exception while retrieving the document properties!");
        }
    }

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
