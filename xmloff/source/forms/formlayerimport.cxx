/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formlayerimport.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-25 09:24:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _XMLOFF_FORMLAYERIMPORT_HXX_
#include "formlayerimport.hxx"
#endif
#ifndef _XMLOFF_FORMS_LAYERIMPORT_HXX_
#include "layerimport.hxx"
#endif

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
    ::vos::ORef< SvXMLImportPropertyMapper > OFormLayerXMLImport::getStylePropertyMapper() const
    {
        return m_pImpl->getStylePropertyMapper();
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

