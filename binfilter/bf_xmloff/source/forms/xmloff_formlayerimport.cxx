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

#include "formlayerimport.hxx"
#include "layerimport.hxx"
namespace binfilter {

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::drawing;
    using namespace ::com::sun::star::xml;

    //=====================================================================
    //= OFormLayerXMLExport
    //=====================================================================

    //---------------------------------------------------------------------
    OFormLayerXMLImport::OFormLayerXMLImport(SvXMLImport& _rImporter)
        :m_pImpl(NULL)
    {
        m_pImpl = new OFormLayerXMLImport_Impl(_rImporter);
    }

    //---------------------------------------------------------------------
    OFormLayerXMLImport::~OFormLayerXMLImport()
    {
        delete m_pImpl;
    }

    //---------------------------------------------------------------------
    ::rtl::Reference< SvXMLImportPropertyMapper > OFormLayerXMLImport::getStylePropertyMapper() const
    {
        return m_pImpl->getStylePropertyMapper();
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport::setAutoStyleContext(SvXMLStylesContext* _pNewContext)
    {
        m_pImpl->setAutoStyleContext(_pNewContext);
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport::startPage(const Reference< XDrawPage >& _rxDrawPage)
    {
        m_pImpl->startPage(_rxDrawPage);
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport::endPage()
    {
        m_pImpl->endPage();
    }

    //---------------------------------------------------------------------
    Reference< XPropertySet > OFormLayerXMLImport::lookupControl(const ::rtl::OUString& _rId)
    {
        return m_pImpl->lookupControlId(_rId);
    }

    //---------------------------------------------------------------------
    SvXMLImportContext* OFormLayerXMLImport::createOfficeFormsContext(
        SvXMLImport& _rImport,
        sal_uInt16 _nPrefix,
        const ::rtl::OUString& _rLocalName)
    {
        return m_pImpl->createOfficeFormsContext(_rImport, _nPrefix, _rLocalName);
    }

    //---------------------------------------------------------------------
    SvXMLImportContext* OFormLayerXMLImport::createContext(const sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
        const Reference< sax::XAttributeList >& _rxAttribs)
    {
        return m_pImpl->createContext(_nPrefix, _rLocalName, _rxAttribs);
    }

    //---------------------------------------------------------------------
    XMLPropStyleContext* OFormLayerXMLImport::createControlStyleContext( sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
        const Reference< sax::XAttributeList >& _rxAttrList, SvXMLStylesContext& _rParentStyles,
        sal_uInt16 _nFamily, sal_Bool _bDefaultStyle )
    {
        return m_pImpl->createControlStyleContext( _nPrefix, _rLocalName, _rxAttrList, _rParentStyles, _nFamily, _bDefaultStyle );
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport::applyControlNumberStyle(const Reference< XPropertySet >& _rxControlModel, const ::rtl::OUString& _rControlNumerStyleName)
    {
        m_pImpl->applyControlNumberStyle(_rxControlModel, _rControlNumerStyleName);
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport::documentDone( )
    {
        m_pImpl->documentDone( );
    }

//.........................................................................
}	// namespace xmloff
//.........................................................................

}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
