/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlSubDocument.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:05:55 $
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
#include "precompiled_reportdesign.hxx"

#ifndef RPT_XMLSUBDOCUMENT_HXX
#include "xmlSubDocument.hxx"
#endif
#ifndef RPT_XMLFILTER_HXX
#include "xmlfilter.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef RPT_XMLENUMS_HXX
#include "xmlEnums.hxx"
#endif
#ifndef RPT_XMLMASTERFIELDS_HXX
#include "xmlMasterFields.hxx"
#endif
#include "xmlTable.hxx"
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#include <comphelper/property.hxx>
#include <com/sun/star/report/XReportControlModel.hpp>

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::report;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME( rpt_OXMLSubDocument )

OXMLSubDocument::OXMLSubDocument( ORptFilter& rImport,
                sal_uInt16 nPrfx
                ,const ::rtl::OUString& rLName
                ,const Reference< XReportComponent > & _xComponent
                ,OXMLTable* _pContainer) :
    OXMLReportElementBase( rImport, nPrfx, rLName,_xComponent.get(),_pContainer)
,m_xFake(_xComponent)
,m_nCurrentCount(0)
,m_bContainsShape(false)
{
    DBG_CTOR( rpt_OXMLSubDocument,NULL);

}
// -----------------------------------------------------------------------------

OXMLSubDocument::~OXMLSubDocument()
{
    DBG_DTOR( rpt_OXMLSubDocument,NULL);
}

// -----------------------------------------------------------------------------
SvXMLImportContext* OXMLSubDocument::_CreateChildContext(
        sal_uInt16 _nPrefix,
        const ::rtl::OUString& _rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = OXMLReportElementBase::_CreateChildContext(_nPrefix,_rLocalName,xAttrList);
    if ( pContext )
        return pContext;
    const SvXMLTokenMap&    rTokenMap   = static_cast<ORptFilter&>(GetImport()).GetReportElemTokenMap();

    switch( rTokenMap.Get( _nPrefix, _rLocalName ) )
    {
        case XML_TOK_MASTER_DETAIL_FIELDS:
            {
                GetImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                pContext = new OXMLMasterFields(static_cast<ORptFilter&>(GetImport()), _nPrefix, _rLocalName,xAttrList ,this);
            }
            break;
        case XML_TOK_SUB_FRAME:
            {
                if ( !m_bContainsShape )
                    m_nCurrentCount = m_pContainer->getSection()->getCount();
                UniReference< XMLShapeImportHelper > xShapeImportHelper = GetImport().GetShapeImport();
                uno::Reference< drawing::XShapes > xShapes = m_pContainer->getSection().get();
                pContext = xShapeImportHelper->CreateGroupChildContext(GetImport(),_nPrefix,_rLocalName,xAttrList,xShapes);
                m_bContainsShape = true;
            }
            break;
        default:
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), _nPrefix, _rLocalName );

    return pContext;
}
// -----------------------------------------------------------------------------
void OXMLSubDocument::EndElement()
{
    if ( m_bContainsShape )
    {
        m_xComponent.set(m_pContainer->getSection()->getByIndex(m_nCurrentCount),uno::UNO_QUERY);
        if ( m_xComponent.is() )
        {
            m_pContainer->addCell(m_xComponent.get());

            if ( !m_aMasterFields.empty() )
                m_xComponent->setMasterFields(Sequence< ::rtl::OUString>(&*m_aMasterFields.begin(),m_aMasterFields.size()));
            if ( !m_aDetailFields.empty() )
                m_xComponent->setDetailFields(Sequence< ::rtl::OUString>(&*m_aDetailFields.begin(),m_aDetailFields.size()));

            m_xComponent->setName(m_xFake->getName());
            m_xComponent->setPrintRepeatedValues(m_xFake->getPrintRepeatedValues());
            uno::Reference< report::XReportControlModel >   xFakeModel(m_xFake,uno::UNO_QUERY);
            uno::Reference< report::XReportControlModel >   xComponentModel(m_xComponent,uno::UNO_QUERY);
            if ( xComponentModel.is() && xFakeModel.is() )
            {
                const sal_Int32 nCount = xFakeModel->getCount();
                try
                {
                    for (sal_Int32 i = 0; i < nCount ; ++i)
                    {
                        uno::Reference< report::XFormatCondition > xCond(xFakeModel->getByIndex(i),uno::UNO_QUERY);
                        uno::Reference< report::XFormatCondition > xNewCond = xComponentModel->createFormatCondition();
                        ::comphelper::copyProperties(xCond.get(),xNewCond.get());
                        xComponentModel->insertByIndex(xComponentModel->getCount(),uno::makeAny(xNewCond));
                    } // for (sal_Int32 i = 0; i < nCount ; ++i)
                }
                catch(uno::Exception&)
                {
                    OSL_ENSURE(0,"Can not access format condition!");
                }

            }
        }
    }
}
// -----------------------------------------------------------------------------
void OXMLSubDocument::addMasterDetailPair(const ::std::pair< ::rtl::OUString,::rtl::OUString >& _aPair)
{
    m_aMasterFields.push_back(_aPair.first);
    m_aDetailFields.push_back(_aPair.second);
}
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
