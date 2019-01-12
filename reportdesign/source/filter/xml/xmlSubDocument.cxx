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
#include "xmlSubDocument.hxx"
#include "xmlCell.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include "xmlEnums.hxx"
#include "xmlMasterFields.hxx"
#include "xmlTable.hxx"
#include <comphelper/property.hxx>
#include <com/sun/star/report/XReportControlModel.hpp>

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::report;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;

OXMLSubDocument::OXMLSubDocument( ORptFilter& rImport,
                sal_uInt16 nPrfx
                ,const OUString& rLName
                ,const Reference< XReportComponent > & _xComponent
                ,OXMLTable* _pContainer
                ,OXMLCell* _pCellParent) :
    OXMLReportElementBase( rImport, nPrfx, rLName,_xComponent.get(),_pContainer)
,m_xFake(_xComponent)
,m_pCellParent(_pCellParent)
,m_nCurrentCount(0)
,m_bContainsShape(false)
{

}

OXMLSubDocument::~OXMLSubDocument()
{
}

SvXMLImportContextRef OXMLSubDocument::CreateChildContext_(
        sal_uInt16 _nPrefix,
        const OUString& _rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContextRef xContext = OXMLReportElementBase::CreateChildContext_(_nPrefix,_rLocalName,xAttrList);
    if (xContext)
        return xContext;
    const SvXMLTokenMap&    rTokenMap   = static_cast<ORptFilter&>(GetImport()).GetReportElemTokenMap();

    switch( rTokenMap.Get( _nPrefix, _rLocalName ) )
    {
        case XML_TOK_MASTER_DETAIL_FIELDS:
            {
                GetImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                xContext = new OXMLMasterFields(static_cast<ORptFilter&>(GetImport()), _nPrefix, _rLocalName,xAttrList ,this);
            }
            break;
        case XML_TOK_SUB_FRAME:
            {
                if ( !m_bContainsShape )
                    m_nCurrentCount = m_pContainer->getSection()->getCount();
                rtl::Reference< XMLShapeImportHelper > xShapeImportHelper = GetImport().GetShapeImport();
                uno::Reference< drawing::XShapes > xShapes = m_pContainer->getSection().get();
                xContext = xShapeImportHelper->CreateGroupChildContext(GetImport(),_nPrefix,_rLocalName,xAttrList,xShapes);
                m_bContainsShape = true;
                if (m_pCellParent)
                {
                    // #i94115 say to the parent Cell it contains shapes
                    m_pCellParent->setContainsShape(true);
                }
            }
            break;
        // FIXME: is it *intentional* that this is supported?
        // ORptExport::exportContainer() can export this but the import
        // used to be rather accidental previously
        case XML_TOK_SUB_BODY:
            xContext = new RptXMLDocumentBodyContext(GetImport(), _nPrefix, _rLocalName);
            break;
        default:
            break;
    }

    if (!xContext)
        xContext = new SvXMLImportContext( GetImport(), _nPrefix, _rLocalName );

    return xContext;
}

void OXMLSubDocument::EndElement()
{
    if ( m_bContainsShape )
    {
        m_xReportComponent.set(m_pContainer->getSection()->getByIndex(m_nCurrentCount),uno::UNO_QUERY);
        if ( m_xReportComponent.is() )
        {
            if ( !m_aMasterFields.empty() )
                m_xReportComponent->setMasterFields(Sequence< OUString>(&*m_aMasterFields.begin(),m_aMasterFields.size()));
            if ( !m_aDetailFields.empty() )
                m_xReportComponent->setDetailFields(Sequence< OUString>(&*m_aDetailFields.begin(),m_aDetailFields.size()));

            m_xReportComponent->setName(m_xFake->getName());
            m_xReportComponent->setPrintRepeatedValues(m_xFake->getPrintRepeatedValues());
            uno::Reference< report::XReportControlModel >   xFakeModel(m_xFake,uno::UNO_QUERY);
            uno::Reference< report::XReportControlModel >   xComponentModel(m_xReportComponent,uno::UNO_QUERY);
            if ( xComponentModel.is() && xFakeModel.is() )
            {
                xComponentModel->setPrintWhenGroupChange(xFakeModel->getPrintWhenGroupChange());
                const sal_Int32 nCount = xFakeModel->getCount();
                try
                {
                    for (sal_Int32 i = 0; i < nCount ; ++i)
                    {
                        uno::Reference< report::XFormatCondition > xCond(xFakeModel->getByIndex(i),uno::UNO_QUERY);
                        uno::Reference< report::XFormatCondition > xNewCond = xComponentModel->createFormatCondition();
                        ::comphelper::copyProperties(xCond.get(),xNewCond.get());
                        xComponentModel->insertByIndex(xComponentModel->getCount(),uno::makeAny(xNewCond));
                    }
                }
                catch(uno::Exception&)
                {
                    OSL_FAIL("Can not access format condition!");
                }

            }
        }
    }
}

void OXMLSubDocument::addMasterDetailPair(const ::std::pair< OUString,OUString >& _aPair)
{
    m_aMasterFields.push_back(_aPair.first);
    m_aDetailFields.push_back(_aPair.second);
}

} // namespace rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
