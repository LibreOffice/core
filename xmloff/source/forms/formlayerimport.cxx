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

#include <xmloff/formlayerimport.hxx>
#include "layerimport.hxx"

namespace xmloff
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::drawing;
    using namespace ::com::sun::star;

    //= OFormLayerXMLExport

    OFormLayerXMLImport::OFormLayerXMLImport(SvXMLImport& _rImporter)
        :m_pImpl(NULL)
    {
        m_pImpl = new OFormLayerXMLImport_Impl(_rImporter);
    }

    OFormLayerXMLImport::~OFormLayerXMLImport()
    {
        delete m_pImpl;
    }

    void OFormLayerXMLImport::setAutoStyleContext(SvXMLStylesContext* _pNewContext)
    {
        m_pImpl->setAutoStyleContext(_pNewContext);
    }

    void OFormLayerXMLImport::startPage(const Reference< XDrawPage >& _rxDrawPage)
    {
        m_pImpl->startPage(_rxDrawPage);
    }

    void OFormLayerXMLImport::endPage()
    {
        m_pImpl->endPage();
    }

    Reference< XPropertySet > OFormLayerXMLImport::lookupControl(const OUString& _rId)
    {
        return m_pImpl->lookupControlId(_rId);
    }

    SvXMLImportContext* OFormLayerXMLImport::createOfficeFormsContext(
        SvXMLImport& _rImport,
        sal_uInt16 _nPrefix,
        const OUString& _rLocalName)
    {
        return m_pImpl->createOfficeFormsContext(_rImport, _nPrefix, _rLocalName);
    }

    SvXMLImportContext* OFormLayerXMLImport::createContext(const sal_uInt16 _nPrefix, const OUString& _rLocalName,
        const Reference< xml::sax::XAttributeList >& _rxAttribs)
    {
        return m_pImpl->createContext(_nPrefix, _rLocalName, _rxAttribs);
    }

    void OFormLayerXMLImport::applyControlNumberStyle(const Reference< XPropertySet >& _rxControlModel, const OUString& _rControlNumerStyleName)
    {
        m_pImpl->applyControlNumberStyle(_rxControlModel, _rControlNumerStyleName);
    }

    void OFormLayerXMLImport::documentDone( )
    {
        m_pImpl->documentDone( );
    }

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
