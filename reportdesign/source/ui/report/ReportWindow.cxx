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
#include "ReportWindow.hxx"
#include "ReportSection.hxx"
#include "SectionView.hxx"
#include "ViewsWindow.hxx"
#include "DesignView.hxx"
#include "UITools.hxx"

#include <svtools/colorcfg.hxx>
#include <svl/itempool.hxx>
#include <unotools/syslocale.hxx>

#include <vcl/settings.hxx>

#include "RptDef.hxx"
#include "dlgedfunc.hxx"
#include "RptModel.hxx"
#include "strings.hxx"
#include "RptPage.hxx"
#include "ReportController.hxx"
#include "EndMarker.hxx"
#include "ColorChanger.hxx"
#include "ScrollHelper.hxx"

#include "helpids.hrc"
#include "dlgedfac.hxx"
#include <toolkit/helper/vclunohelper.hxx>


#define SECTION_OFFSET  3
namespace rptui
{
using namespace ::com::sun::star;
using namespace ::comphelper;

// class OReportWindow


OReportWindow::OReportWindow(OScrollWindowHelper* _pParent,ODesignView* _pView)
: Window(_pParent,WB_DIALOGCONTROL)
, ::comphelper::OPropertyChangeListener(m_aMutex)
,m_aHRuler(VclPtr<Ruler>::Create(this))
,m_pView(_pView)
,m_pParent(_pParent)
,m_aViewsWindow(VclPtr<rptui::OViewsWindow>::Create(this))
,m_pObjFac( new DlgEdFactory() )
{
    SetHelpId(UID_RPT_REPORTWINDOW);
    SetMapMode( MapMode( MapUnit::Map100thMM ) );

    m_aViewsWindow->Show();

    m_aHRuler->Show();
    m_aHRuler->Activate();
    m_aHRuler->SetPagePos();
    m_aHRuler->SetBorders();
    m_aHRuler->SetIndents();
    m_aHRuler->SetMargin1();
    m_aHRuler->SetMargin2();
    const MeasurementSystem eSystem = SvtSysLocale().GetLocaleData().getMeasurementSystemEnum();
    m_aHRuler->SetUnit(MeasurementSystem::Metric == eSystem ? FUNIT_CM : FUNIT_INCH);

    ImplInitSettings();
    m_pReportListener = addStyleListener(_pView->getController().getReportDefinition(),this);
}

OReportWindow::~OReportWindow()
{
    disposeOnce();
}

void OReportWindow::dispose()
{
    if ( m_pReportListener.is() )
        m_pReportListener->dispose();
    m_aHRuler.disposeAndClear();
    m_aViewsWindow.disposeAndClear();
    m_pView.clear();
    m_pParent.clear();
    vcl::Window::dispose();
}

void OReportWindow::SetInsertObj( sal_uInt16 eObj,const OUString& _sShapeType )
{
    m_aViewsWindow->SetInsertObj( eObj,_sShapeType);
}

OUString OReportWindow::GetInsertObjString() const
{
    return m_aViewsWindow->GetInsertObjString();
}

void OReportWindow::SetMode( DlgEdMode eNewMode )
{
    m_aViewsWindow->SetMode(eNewMode);
}

void OReportWindow::removeSection(sal_uInt16 _nPosition)
{
    m_aViewsWindow->removeSection(_nPosition);
    m_pParent->setTotalSize(GetTotalWidth(),GetTotalHeight());
    m_aViewsWindow->Invalidate(InvalidateFlags::Transparent);
}

void OReportWindow::addSection(const uno::Reference< report::XSection >& _xSection,const OUString& _sColorEntry,sal_uInt16 _nPosition)
{
    if ( !_xSection.is() )
        return;

    m_aViewsWindow->addSection(_xSection,_sColorEntry,_nPosition);

    m_pParent->setTotalSize(GetTotalWidth(),GetTotalHeight());
}

void OReportWindow::toggleGrid(bool _bVisible)
{
    m_aViewsWindow->toggleGrid(_bVisible);
}

void OReportWindow::showRuler(bool _bShow)
{
    m_aHRuler->Show(_bShow);

    m_aViewsWindow->showRuler(_bShow);
}

sal_Int32 OReportWindow::getMaxMarkerWidth() const
{
    Fraction aStartWidth(long(REPORT_STARTMARKER_WIDTH));
    aStartWidth *= m_aViewsWindow->GetMapMode().GetScaleX();
    return sal_Int32((long)aStartWidth);
}

sal_Int32 OReportWindow::GetTotalWidth() const
{
    sal_Int32 nWidth = 0;
    if ( !m_aViewsWindow->empty() )
    {
        Fraction aStartWidth(long(REPORT_ENDMARKER_WIDTH + REPORT_STARTMARKER_WIDTH ));
        const Fraction aZoom(m_pView->getController().getZoomValue(),100);
        aStartWidth *= aZoom;
        const sal_Int32 nPaperWidth = getStyleProperty<awt::Size>(m_pView->getController().getReportDefinition(),PROPERTY_PAPERSIZE).Width;
        Fraction aPaperWidth(nPaperWidth,1);
        aPaperWidth *= aZoom;
        const Size aPageSize = LogicToPixel(Size(aPaperWidth,0));
        nWidth = aPageSize.Width() + long(aStartWidth);
    }
    return nWidth;
}

void OReportWindow::Resize()
{
    Window::Resize();
    if ( !m_aViewsWindow->empty() )
    {
        const Size aTotalOutputSize = GetOutputSizePixel();
        Fraction aStartWidth(long(REPORT_STARTMARKER_WIDTH)*m_pView->getController().getZoomValue(),100);

        const Point aOffset = LogicToPixel( Point( SECTION_OFFSET, 0 ), MapUnit::MapAppFont );
        Point aStartPoint((long)aStartWidth + aOffset.X(),0);
        uno::Reference<report::XReportDefinition> xReportDefinition = getReportView()->getController().getReportDefinition();
        const sal_Int32 nPaperWidth = getStyleProperty<awt::Size>(xReportDefinition,PROPERTY_PAPERSIZE).Width;
        sal_Int32 nLeftMargin = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_LEFTMARGIN);
        sal_Int32 nRightMargin = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_RIGHTMARGIN);
        Size aPageSize  = m_aViewsWindow->LogicToPixel(Size(nPaperWidth ,0));
        nLeftMargin     = m_aViewsWindow->LogicToPixel(Size(nLeftMargin,0)).Width();
        nRightMargin    = m_aViewsWindow->LogicToPixel(Size(nRightMargin,0)).Width();

        aPageSize.Height() = m_aHRuler->GetSizePixel().Height();

        const long nTermp(m_aViewsWindow->getTotalHeight() + aPageSize.Height());
        long nSectionsHeight = ::std::max<long>(nTermp,aTotalOutputSize.Height());

        m_aHRuler->SetPosSizePixel(aStartPoint,aPageSize);
        m_aHRuler->SetNullOffset(nLeftMargin);
        m_aHRuler->SetMargin1(0);
        m_aHRuler->SetMargin2(aPageSize.Width() - nLeftMargin - nRightMargin);

        aStartPoint.Y() += aPageSize.Height();
        nSectionsHeight -= aStartPoint.Y();

        aStartPoint.X() = aOffset.X();

        m_aViewsWindow->SetPosSizePixel(aStartPoint,Size(aTotalOutputSize.Width(),nSectionsHeight));
    }
}

Point OReportWindow::getThumbPos() const
{
    return m_pParent->getThumbPos();
}

void OReportWindow::ImplInitSettings()
{
    SetBackground( );
}

void OReportWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

sal_Int32 OReportWindow::GetTotalHeight() const
{
    return m_aViewsWindow->getTotalHeight();
}

void OReportWindow::ScrollChildren(const Point& _aThumbPos)
{
    MapMode aMap = m_aHRuler->GetMapMode();
    Point aOrg( aMap.GetOrigin() );
    if ( aOrg.X() != (-_aThumbPos.X()) )
    {
        aMap.SetOrigin( Point(- _aThumbPos.X(), aOrg.Y()));
        m_aHRuler->SetMapMode( aMap );
        m_aHRuler->Scroll(-(aOrg.X() + _aThumbPos.X()),0);
    }

    m_aViewsWindow->scrollChildren(_aThumbPos);
}

sal_uInt16 OReportWindow::getSectionCount() const
{
    return m_aViewsWindow->getSectionCount();
}

void OReportWindow::notifySizeChanged()
{
    m_pParent->setTotalSize(GetTotalWidth(),GetTotalHeight());
}

bool OReportWindow::HasSelection() const
{
    return m_aViewsWindow->HasSelection();
}

void OReportWindow::Delete()
{

    m_aViewsWindow->Delete();
}

void OReportWindow::Copy()
{

    m_aViewsWindow->Copy();
}

void OReportWindow::Paste()
{

    m_aViewsWindow->Paste();
}

bool OReportWindow::IsPasteAllowed() const
{
    return m_aViewsWindow->IsPasteAllowed();
}

void OReportWindow::SelectAll(const sal_uInt16 _nObjectType)
{

    m_aViewsWindow->SelectAll(_nObjectType);
}

void OReportWindow::unmarkAllObjects()
{

    m_aViewsWindow->unmarkAllObjects(nullptr);
}

void OReportWindow::showProperties(const uno::Reference< report::XSection>& _xReportComponent)
{
    OSectionWindow* pSectionWindow = m_aViewsWindow->getSectionWindow( _xReportComponent );
    m_pView->UpdatePropertyBrowserDelayed(pSectionWindow->getReportSection().getSectionView());
}

bool OReportWindow::handleKeyEvent(const KeyEvent& _rEvent)
{
    return m_aViewsWindow->handleKeyEvent(_rEvent);
}

void OReportWindow::setMarked(OSectionView* _pSectionView, bool _bMark)
{
    if ( _pSectionView )
        m_aViewsWindow->setMarked(_pSectionView,_bMark);
}

void OReportWindow::setMarked(const uno::Reference< report::XSection>& _xSection, bool _bMark)
{

    m_aViewsWindow->setMarked(_xSection,_bMark);
}

void OReportWindow::setMarked(const uno::Sequence< uno::Reference< report::XReportComponent> >& _xShape, bool _bMark)
{

    m_aViewsWindow->setMarked(_xShape,_bMark);
}

OSectionWindow* OReportWindow::getMarkedSection(NearSectionAccess nsa) const
{
    return  m_aViewsWindow->getMarkedSection(nsa);
}

OSectionWindow* OReportWindow::getSectionWindow(const css::uno::Reference< css::report::XSection>& _xSection) const
{
    return  m_aViewsWindow->getSectionWindow(_xSection);
}

void OReportWindow::markSection(const sal_uInt16 _nPos)
{

    m_aViewsWindow->markSection(_nPos);
}

void OReportWindow::fillCollapsedSections(::std::vector<sal_uInt16>& _rCollapsedPositions) const
{

    m_aViewsWindow->fillCollapsedSections(_rCollapsedPositions);
}

void OReportWindow::collapseSections(const uno::Sequence< css::beans::PropertyValue>& _aCollpasedSections)
{

    m_aViewsWindow->collapseSections(_aCollpasedSections);
}

void OReportWindow::alignMarkedObjects(ControlModification _nControlModification, bool _bAlignAtSection)
{
    m_aViewsWindow->alignMarkedObjects(_nControlModification, _bAlignAtSection);
}

void OReportWindow::setGridSnap(bool bOn)
{

    m_aViewsWindow->setGridSnap(bOn);
}

void OReportWindow::setDragStripes(bool bOn)
{
    m_aViewsWindow->setDragStripes(bOn);
}

sal_uInt32 OReportWindow::getMarkedObjectCount() const
{
    return m_aViewsWindow->getMarkedObjectCount();
}

void OReportWindow::zoom(const Fraction& _aZoom)
{
    m_aHRuler->SetZoom(_aZoom);
    m_aHRuler->Invalidate();

    m_aViewsWindow->zoom(_aZoom);

    notifySizeChanged();
    const Point aNewThumbPos( m_pParent->getThumbPos() );

    ScrollChildren( aNewThumbPos );
    Resize();

    Invalidate(InvalidateFlags::NoErase | InvalidateFlags::NoChildren | InvalidateFlags::Transparent);
}

void OReportWindow::fillControlModelSelection(::std::vector< uno::Reference< uno::XInterface > >& _rSelection) const
{
    m_aViewsWindow->fillControlModelSelection(_rSelection);
}

sal_Int32 OReportWindow::impl_getRealPixelWidth() const
{
    const sal_Int32 nPaperWidth = getStyleProperty<awt::Size>(m_pView->getController().getReportDefinition(),PROPERTY_PAPERSIZE).Width;
    MapMode aMap( MapUnit::Map100thMM );
    const Size aPageSize = LogicToPixel(Size(nPaperWidth,0),aMap);
    return aPageSize.Width() + REPORT_ENDMARKER_WIDTH + REPORT_STARTMARKER_WIDTH + SECTION_OFFSET;
}

sal_uInt16 OReportWindow::getZoomFactor(SvxZoomType _eType) const
{
    sal_uInt16 nZoom(100);
    const Size aSize( GetSizePixel() );
    switch( _eType)
    {
        case SvxZoomType::PERCENT:
            nZoom = m_pView->getController().getZoomValue();
            break;
        case SvxZoomType::OPTIMAL:
            break;
        case SvxZoomType::WHOLEPAGE:
            {
                nZoom = (sal_uInt16)(long)Fraction(aSize.Width()*100,impl_getRealPixelWidth());
                MapMode aMap( MapUnit::Map100thMM );
                const Size aHeight = m_aViewsWindow->LogicToPixel(m_aViewsWindow->PixelToLogic(Size(0,GetTotalHeight() + m_aHRuler->GetSizePixel().Height())),aMap);
                nZoom = ::std::min(nZoom,(sal_uInt16)(long)Fraction(aSize.Height()*100,aHeight.Height()));
            }
            break;
        case SvxZoomType::PAGEWIDTH:
            nZoom = (sal_uInt16)(long)Fraction(aSize.Width()*100,impl_getRealPixelWidth());
            break;
        default:
            break;
    }

    return nZoom;
}

void OReportWindow::_propertyChanged(const beans::PropertyChangeEvent&)
{
    Resize();
    m_aViewsWindow->Resize();
    Invalidate(InvalidateFlags::Transparent);
}

}   //rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
