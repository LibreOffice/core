/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formlayerimport.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:10:42 $
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

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.12.298.1  2005/09/05 14:38:59  rt
 *  #i54170# Change license header: remove SISSL
 *
 *  Revision 1.12  2004/07/13 08:13:03  rt
 *  INTEGRATION: CWS oasis (1.11.66); FILE MERGED
 *  2004/06/08 14:49:18 mib 1.11.66.1: - #i20153#: form controls
 *
 *  Revision 1.11.66.1  2004/06/08 14:49:18  mib
 *  - #i20153#: form controls
 *
 *  Revision 1.11  2003/12/01 12:04:36  rt
 *  INTEGRATION: CWS geordi2q09 (1.10.24); FILE MERGED
 *  2003/11/24 15:17:09 obo 1.10.24.2: undo last change
 *  2003/11/21 17:02:46 obo 1.10.24.1: #111934# join CWS comboboxlink
 *
 *  Revision 1.10.24.2  2003/11/24 15:17:09  obo
 *  undo last change
 *
 *  Revision 1.10  2003/10/21 08:39:58  obo
 *  INTEGRATION: CWS formcelllinkage (1.9.160); FILE MERGED
 *  2003/10/01 09:55:22 fs 1.9.160.1: #i18994# merging the changes from the CWS fs002
 *
 *  Revision 1.9.160.1  2003/10/01 09:55:22  fs
 *  #i18994# merging the changes from the CWS fs002
 *
 *  Revision 1.9.156.1  2003/09/25 14:28:40  fs
 *  #18994# merging the changes from cws_srx645_fs002 branch
 *
 *  Revision 1.9.152.1  2003/09/17 12:26:55  fs
 *  #18999# #19367# persistence for cell value and cell range bindings
 *
 *  Revision 1.9  2002/10/25 13:15:25  fs
 *  #104402# +createControlStyleContext
 *
 *  Revision 1.8  2001/05/28 14:59:18  fs
 *  #86712# added control number style related functionality
 *
 *  Revision 1.7  2001/03/20 13:38:42  fs
 *  #83970# +createOfficeFormsContext
 *
 *  Revision 1.6  2001/03/20 08:02:56  fs
 *  removed setAutoStyle (was obsolete)
 *
 *  Revision 1.5  2001/02/01 09:46:47  fs
 *  no own style handling anymore - the shape exporter is responsible for our styles now
 *
 *  Revision 1.4  2000/12/18 15:14:35  fs
 *  some changes ... now exporting/importing styles
 *
 *  Revision 1.3  2000/12/13 10:40:15  fs
 *  new import related implementations - at this version, we should be able to import everything we export (which is all except events and styles)
 *
 *  Revision 1.2  2000/12/12 12:01:05  fs
 *  new implementations for the import - still under construction
 *
 *  Revision 1.1  2000/12/06 17:31:22  fs
 *  initial checkin - implementations for formlayer import/export - still under construction
 *
 *
 *  Revision 1.0 04.12.00 12:52:47  fs
 ************************************************************************/

