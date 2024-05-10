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

#include <RptModel.hxx>
#include <RptPage.hxx>
#include <dbaccess/dbsubcomponentcontroller.hxx>
#include <vcl/svapp.hxx>

#include <UndoEnv.hxx>
#include <ReportUndoFactory.hxx>
#include <ReportDefinition.hxx>

#include <RptDef.hxx>
#include <strings.hxx>
#include <FixedLine.hxx>
#include <FormattedField.hxx>
#include <FixedText.hxx>
#include <ImageControl.hxx>
#include <Shape.hxx>

namespace rptui
{
using namespace reportdesign;
using namespace com::sun::star;


OReportModel::OReportModel(::reportdesign::OReportDefinition* _pReportDefinition)
:   SdrModel(
        nullptr,
        _pReportDefinition)
    ,m_pController(nullptr)
    ,m_pReportDefinition(_pReportDefinition)
{
    m_xUndoEnv = new OXUndoEnvironment(*this);
    SetSdrUndoFactory(new OReportUndoFactory);
}


OReportModel::~OReportModel()
{
    // There are some nasty interactions which mean that we have to delete
    // the pages before we destroy the model - otherwise we will trigger
    // callbacks which will attempt to recreate SvxShape objects and
    // fail because the model is being torn down.
    while (GetPageCount())
        RemovePage(GetPageCount()-1);

    detachController();
}

void OReportModel::detachController()
{
    if (!m_pReportDefinition)
        return;
    m_pController = nullptr;
    m_xUndoEnv->EndListening( *this );
    ClearUndoBuffer();
    m_xUndoEnv->Clear(OXUndoEnvironment::Accessor());
    m_pReportDefinition = nullptr;
}

rtl::Reference<SdrPage> OReportModel::AllocPage(bool /*bMasterPage*/)
{
    OSL_FAIL("Who called me!");
    return nullptr;
}


void OReportModel::SetChanged( bool bChanged )
{
    SdrModel::SetChanged( bChanged );
    SetModified( bChanged );
}


void OReportModel::SetModified(bool _bModified)
{
    if ( m_pController )
        m_pController->setModified(_bModified);
}

rtl::Reference<SdrPage> OReportModel::RemovePage(sal_uInt16 nPgNum)
{
    rtl::Reference<OReportPage> pPage = dynamic_cast<OReportPage*>(SdrModel::RemovePage(nPgNum).get());
    return pPage;
}

OReportPage* OReportModel::createNewPage(const uno::Reference< report::XSection >& _xSection)
{
    SolarMutexGuard aSolarGuard;
    rtl::Reference<OReportPage> pPage = new OReportPage( *this ,_xSection);
    InsertPage(pPage.get());
    m_xUndoEnv->AddSection(_xSection);
    return pPage.get();
}

OReportPage* OReportModel::getPage(const uno::Reference< report::XSection >& _xSection)
{
    OReportPage* pPage = nullptr;
    sal_uInt16 nCount = GetPageCount();
    for (sal_uInt16 i = 0; i < nCount && !pPage ; ++i)
    {
        OReportPage* pRptPage = dynamic_cast<OReportPage*>( GetPage(i)  );
        if ( pRptPage && pRptPage->getSection() == _xSection )
            pPage = pRptPage;
    }
    return pPage;
}

SvxNumType OReportModel::GetPageNumType() const
{
    uno::Reference< report::XReportDefinition > xReportDefinition( getReportDefinition() );
    if ( xReportDefinition.is() )
        return static_cast<SvxNumType>(getStyleProperty<sal_Int16>(xReportDefinition,PROPERTY_NUMBERINGTYPE));
    return SVX_NUM_ARABIC;
}


uno::Reference< report::XReportDefinition > OReportModel::getReportDefinition() const
{
    uno::Reference< report::XReportDefinition > xReportDefinition = m_pReportDefinition;
    OSL_ENSURE( xReportDefinition.is(), "OReportModel::getReportDefinition: invalid model at our controller!" );
    return xReportDefinition;
}

uno::Reference< frame::XModel > OReportModel::createUnoModel()
{
    return getReportDefinition();
}

uno::Reference< uno::XInterface > OReportModel::createShape(const OUString& aServiceSpecifier,uno::Reference< drawing::XShape >& _rShape,sal_Int32 nOrientation)
{
    uno::Reference< uno::XInterface > xRet;
    if ( _rShape.is() )
    {
        if ( aServiceSpecifier == SERVICE_FORMATTEDFIELD )
        {
            uno::Reference<report::XFormattedField> xProp = new OFormattedField(m_pReportDefinition->getContext(),m_pReportDefinition,_rShape);
            xRet = xProp;
            if ( _rShape.is() )
                throw uno::Exception(u"no shape"_ustr, nullptr);
            xProp->setPropertyValue( PROPERTY_FORMATSSUPPLIER, uno::Any(uno::Reference< util::XNumberFormatsSupplier >(m_pReportDefinition)) );
        }
        else if ( aServiceSpecifier == SERVICE_FIXEDTEXT)
        {
            xRet = getXWeak(new OFixedText(m_pReportDefinition->getContext(),m_pReportDefinition,_rShape));
            if ( _rShape.is() )
                throw uno::Exception(u"no shape"_ustr, nullptr);
        }
        else if ( aServiceSpecifier == SERVICE_FIXEDLINE)
        {
            xRet = getXWeak(new OFixedLine(m_pReportDefinition->getContext(),m_pReportDefinition,_rShape,nOrientation));
            if ( _rShape.is() )
                throw uno::Exception(u"no shape"_ustr, nullptr);
        }
        else if ( aServiceSpecifier == SERVICE_IMAGECONTROL )
        {
            xRet = getXWeak(new OImageControl(m_pReportDefinition->getContext(),m_pReportDefinition,_rShape));
            if ( _rShape.is() )
                throw uno::Exception(u"no shape"_ustr, nullptr);
        }
        else if ( aServiceSpecifier == SERVICE_REPORTDEFINITION )
        {
            xRet = getXWeak(new OReportDefinition(m_pReportDefinition->getContext(),m_pReportDefinition,_rShape));
            if ( _rShape.is() )
                throw uno::Exception(u"no shape"_ustr, nullptr);
        }
        else if ( _rShape.is() )
        {
            xRet = getXWeak(new OShape(m_pReportDefinition->getContext(),m_pReportDefinition,_rShape,aServiceSpecifier));
            if ( _rShape.is() )
                throw uno::Exception(u"no shape"_ustr, nullptr);
        }
    }
    return xRet;
}

}   //rptui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
