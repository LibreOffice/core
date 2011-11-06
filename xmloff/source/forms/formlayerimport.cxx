/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include <xmloff/formlayerimport.hxx>
#include "layerimport.hxx"

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
    void OFormLayerXMLImport::setAutoStyleContext(SvXMLStylesContext* _pNewContext)
    {
        m_pImpl->setAutoStyleContext(_pNewContext);
    }

    //---------------------------------------------------------------------
    void OFormLayerXMLImport::seekPage(const Reference< XDrawPage >& _rxDrawPage)
    {
        m_pImpl->seekPage(_rxDrawPage);
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
        const rtl::OUString& _rLocalName)
    {
        return m_pImpl->createOfficeFormsContext(_rImport, _nPrefix, _rLocalName);
    }

    //---------------------------------------------------------------------
    SvXMLImportContext* OFormLayerXMLImport::createContext(const sal_uInt16 _nPrefix, const rtl::OUString& _rLocalName,
        const Reference< sax::XAttributeList >& _rxAttribs)
    {
        return m_pImpl->createContext(_nPrefix, _rLocalName, _rxAttribs);
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
}   // namespace xmloff
//.........................................................................

