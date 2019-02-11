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

#include <ViewsWindow.hxx>
#include <ScrollHelper.hxx>
#include <UndoActions.hxx>
#include <ReportWindow.hxx>
#include <DesignView.hxx>
#include <svtools/colorcfg.hxx>
#include <ReportController.hxx>
#include <UITools.hxx>
#include <RptDef.hxx>
#include <strings.hrc>
#include <SectionView.hxx>
#include <ReportSection.hxx>
#include <strings.hxx>
#include <rptui_slotid.hrc>
#include <dlgedclip.hxx>
#include <ColorChanger.hxx>
#include <RptObject.hxx>
#include <EndMarker.hxx>
#include <sal/log.hxx>
#include <svx/svdpagv.hxx>
#include <svx/unoshape.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <core_resource.hxx>
#include <svx/svdundo.hxx>
#include <toolkit/helper/convert.hxx>
#include <algorithm>
#include <cstdlib>
#include <numeric>

namespace rptui
{
#define DEFAUL_MOVE_SIZE    100

using namespace ::com::sun::star;
using namespace ::comphelper;

static bool lcl_getNewRectSize(const tools::Rectangle& _aObjRect,long& _nXMov, long& _nYMov,SdrObject const * _pObj,SdrView const * _pView, ControlModification _nControlModification)
{
    bool bMoveAllowed = _nXMov != 0 || _nYMov != 0;
    if ( bMoveAllowed )
    {
        tools::Rectangle aNewRect = _aObjRect;
        SdrObject* pOverlappedObj = nullptr;
        do
        {
            aNewRect = _aObjRect;
            switch(_nControlModification)
            {
                case ControlModification::HEIGHT_GREATEST:
                case ControlModification::WIDTH_GREATEST:
                    aNewRect.setWidth(_nXMov);
                    aNewRect.setHeight(_nYMov);
                    break;
                default:
                    aNewRect.Move(_nXMov,_nYMov);
                    break;
            }
            if (dynamic_cast<OUnoObject const *>(_pObj) != nullptr || dynamic_cast<OOle2Obj const *>(_pObj) != nullptr)
            {
                pOverlappedObj = isOver(aNewRect,*_pObj->getSdrPageFromSdrObject(),*_pView,true,_pObj);
                if ( pOverlappedObj && _pObj != pOverlappedObj )
                {
                    tools::Rectangle aOverlappingRect = pOverlappedObj->GetSnapRect();
                    sal_Int32 nXTemp = _nXMov;
                    sal_Int32 nYTemp = _nYMov;
                    switch(_nControlModification)
                    {
                        case ControlModification::LEFT:
                            nXTemp += aOverlappingRect.Right() - aNewRect.Left();
                            bMoveAllowed = _nXMov != nXTemp;
                            break;
                        case ControlModification::RIGHT:
                            nXTemp += aOverlappingRect.Left() - aNewRect.Right();
                            bMoveAllowed = _nXMov != nXTemp;
                            break;
                        case ControlModification::TOP:
                            nYTemp += aOverlappingRect.Bottom() - aNewRect.Top();
                            bMoveAllowed = _nYMov != nYTemp;
                            break;
                        case ControlModification::BOTTOM:
                            nYTemp += aOverlappingRect.Top() - aNewRect.Bottom();
                            bMoveAllowed = _nYMov != nYTemp;
                            break;
                        case ControlModification::CENTER_HORIZONTAL:
                            if ( _aObjRect.Left() < aOverlappingRect.Left() )
                                nXTemp += aOverlappingRect.Left() - aNewRect.Left() - aNewRect.getWidth();
                            else
                                nXTemp += aOverlappingRect.Right() - aNewRect.Left();
                            bMoveAllowed = _nXMov != nXTemp;
                            break;
                        case ControlModification::CENTER_VERTICAL:
                            if ( _aObjRect.Top() < aOverlappingRect.Top() )
                                nYTemp += aOverlappingRect.Top() - aNewRect.Top() - aNewRect.getHeight();
                            else
                                nYTemp += aOverlappingRect.Bottom() - aNewRect.Top();
                            bMoveAllowed = _nYMov != nYTemp;
                            break;
                        case ControlModification::HEIGHT_GREATEST:
                        case ControlModification::WIDTH_GREATEST:
                            {
                                tools::Rectangle aIntersectionRect = aNewRect.GetIntersection(aOverlappingRect);
                                if ( !aIntersectionRect.IsEmpty() )
                                {
                                    if ( _nControlModification == ControlModification::WIDTH_GREATEST )
                                    {
                                        if ( aNewRect.Left() < aIntersectionRect.Left() )
                                        {
                                            aNewRect.SetRight( aIntersectionRect.Left() );
                                        }
                                        else if ( aNewRect.Left() < aIntersectionRect.Right() )
                                        {
                                            aNewRect.SetLeft( aIntersectionRect.Right() );
                                        }
                                    }
                                    else if ( _nControlModification == ControlModification::HEIGHT_GREATEST )
                                    {
                                        if ( aNewRect.Top() < aIntersectionRect.Top() )
                                        {
                                            aNewRect.SetBottom( aIntersectionRect.Top() );
                                        }
                                        else if ( aNewRect.Top() < aIntersectionRect.Bottom() )
                                        {
                                            aNewRect.SetTop( aIntersectionRect.Bottom() );
                                        }
                                    }
                                    nYTemp = aNewRect.getHeight();
                                    bMoveAllowed = _nYMov != nYTemp;
                                    nXTemp = aNewRect.getWidth();
                                    bMoveAllowed = bMoveAllowed && _nXMov != nXTemp;
                                }
                            }
                            break;
                        default:
                            break;
                    }

                    _nXMov = nXTemp;
                    _nYMov = nYTemp;
                }
                else
                    pOverlappedObj = nullptr;
            }
        }
        while ( pOverlappedObj && bMoveAllowed );
    }
    return bMoveAllowed;
}

OViewsWindow::OViewsWindow( OReportWindow* _pReportWindow)
    : Window( _pReportWindow,WB_DIALOGCONTROL)
    , m_pParent(_pReportWindow)
    , m_bInUnmark(false)
{
    SetPaintTransparent(true);
    SetMapMode(MapMode(MapUnit::Map100thMM));
    m_aColorConfig.AddListener(this);
    ImplInitSettings();
}

OViewsWindow::~OViewsWindow()
{
    disposeOnce();
}

void OViewsWindow::dispose()
{
    m_aColorConfig.RemoveListener(this);
    for (auto& rxSection : m_aSections)
        rxSection.disposeAndClear();
    m_aSections.clear();
    m_pParent.clear();
    vcl::Window::dispose();
}

void OViewsWindow::impl_resizeSectionWindow(OSectionWindow& _rSectionWindow,Point& _rStartPoint,bool _bSet)
{
    const uno::Reference< report::XSection> xSection = _rSectionWindow.getReportSection().getSection();

    Size aSectionSize = _rSectionWindow.LogicToPixel( Size( 0,xSection->getHeight() ) );
    aSectionSize.setWidth( getView()->GetTotalWidth() );

    const sal_Int32 nMinHeight = _rSectionWindow.getStartMarker().getMinHeight();
    if ( _rSectionWindow.getStartMarker().isCollapsed() || nMinHeight > aSectionSize.Height() )
    {
        aSectionSize.setHeight( nMinHeight );
    }
    aSectionSize.AdjustHeight(static_cast<long>(StyleSettings::GetSplitSize() * static_cast<double>(_rSectionWindow.GetMapMode().GetScaleY())) );

    if ( _bSet )
        _rSectionWindow.SetPosSizePixel(_rStartPoint,aSectionSize);

    _rStartPoint.AdjustY(aSectionSize.Height() );
}


void OViewsWindow::resize(const OSectionWindow& _rSectionWindow)
{
    bool bSet = false;
    Point aStartPoint;
    for (VclPtr<OSectionWindow> const & pSectionWindow : m_aSections)
    {
        if ( pSectionWindow == &_rSectionWindow )
        {
            aStartPoint = pSectionWindow->GetPosPixel();
            bSet = true;
        }

        if ( bSet )
        {
            impl_resizeSectionWindow(*pSectionWindow,aStartPoint,bSet);
            static const InvalidateFlags nIn = InvalidateFlags::Update | InvalidateFlags::Transparent;
            pSectionWindow->getStartMarker().Invalidate( nIn ); // InvalidateFlags::NoErase |InvalidateFlags::NoChildren| InvalidateFlags::Transparent
            pSectionWindow->getEndMarker().Invalidate( nIn );
        }
    }
    m_pParent->notifySizeChanged();
}

void OViewsWindow::Resize()
{
    Window::Resize();
    if ( !m_aSections.empty() )
    {
        const Point aOffset(m_pParent->getThumbPos());
        Point aStartPoint(0,-aOffset.Y());
        for (VclPtr<OSectionWindow> const & pSectionWindow : m_aSections)
        {
            impl_resizeSectionWindow(*pSectionWindow,aStartPoint,true);
        }
    }
}

void OViewsWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    Window::Paint(rRenderContext, rRect);

    rRenderContext.SetBackground();
    rRenderContext.SetFillColor(Application::GetSettings().GetStyleSettings().GetDialogColor());
    rRenderContext.SetTextFillColor(Application::GetSettings().GetStyleSettings().GetDialogColor());

    Size aOut(GetOutputSizePixel());
    long nStartWidth = long(REPORT_STARTMARKER_WIDTH * rRenderContext.GetMapMode().GetScaleX());

    aOut.AdjustWidth( -nStartWidth );
    aOut = rRenderContext.PixelToLogic(aOut);

    tools::Rectangle aRect(rRenderContext.PixelToLogic(Point(nStartWidth,0)), aOut);
    Wallpaper aWall(m_aColorConfig.GetColorValue(::svtools::APPBACKGROUND).nColor);
    rRenderContext.DrawWallpaper(aRect, aWall);
}

void OViewsWindow::ImplInitSettings()
{
    EnableChildTransparentMode();
}

void OViewsWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void OViewsWindow::addSection(const uno::Reference< report::XSection >& _xSection,const OUString& _sColorEntry,sal_uInt16 _nPosition)
{
    VclPtrInstance<OSectionWindow> pSectionWindow(this,_xSection,_sColorEntry);
    m_aSections.insert(getIteratorAtPos(_nPosition) , TSectionsMap::value_type(pSectionWindow));
    m_pParent->setMarked(&pSectionWindow->getReportSection().getSectionView(),m_aSections.size() == 1);
    Resize();
}

void OViewsWindow::removeSection(sal_uInt16 _nPosition)
{
    if ( _nPosition < m_aSections.size() )
    {
        TSectionsMap::iterator aPos = getIteratorAtPos(_nPosition);
        TSectionsMap::const_iterator aNew = getIteratorAtPos(_nPosition == 0 ? _nPosition+1: _nPosition - 1);

        m_pParent->getReportView()->UpdatePropertyBrowserDelayed((*aNew)->getReportSection().getSectionView());

        aPos->disposeAndClear();
        m_aSections.erase(aPos);
        Resize();
    }
}

void OViewsWindow::toggleGrid(bool _bVisible)
{
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        [_bVisible] (const TSectionsMap::value_type& sectionPtr) {
            sectionPtr->getReportSection().SetGridVisible(_bVisible);
        });
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        [] (const TSectionsMap::value_type& sectionPtr) {
            sectionPtr->getReportSection().Window::Invalidate(InvalidateFlags::NoErase);
        });
}

sal_Int32 OViewsWindow::getTotalHeight() const
{
    return std::accumulate(m_aSections.begin(), m_aSections.end(), sal_Int32(0),
        [](const sal_Int32 nHeight, const VclPtr<OSectionWindow>& rxSection) { return nHeight + rxSection->GetSizePixel().Height(); });
}

sal_uInt16 OViewsWindow::getSectionCount() const
{
    return static_cast<sal_uInt16>(m_aSections.size());
}

void OViewsWindow::SetInsertObj( sal_uInt16 eObj,const OUString& _sShapeType )
{
    for (const auto& rxSection : m_aSections)
        rxSection->getReportSection().getSectionView().SetCurrentObj( eObj, SdrInventor::ReportDesign );

    m_sShapeType = _sShapeType;
}


void OViewsWindow::SetMode( DlgEdMode eNewMode )
{
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        [&eNewMode] (const TSectionsMap::value_type& sectionPtr) {
            sectionPtr->getReportSection().SetMode(eNewMode);
        });
}

bool OViewsWindow::HasSelection() const
{
    return std::any_of(m_aSections.begin(), m_aSections.end(),
        [](const VclPtr<OSectionWindow>& rxSection) { return rxSection->getReportSection().getSectionView().AreObjectsMarked(); });
}

void OViewsWindow::Delete()
{
    m_bInUnmark = true;
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        [] (const TSectionsMap::value_type& sectionPtr) {
            sectionPtr->getReportSection().Delete();
        });
    m_bInUnmark = false;
}

void OViewsWindow::Copy()
{
    uno::Sequence< beans::NamedValue > aAllreadyCopiedObjects;
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        [&aAllreadyCopiedObjects] (const TSectionsMap::value_type& sectionPtr) {
            sectionPtr->getReportSection().Copy(aAllreadyCopiedObjects);
        });

    rtl::Reference<OReportExchange> pCopy = new OReportExchange(aAllreadyCopiedObjects);
    pCopy->CopyToClipboard(this);
}

void OViewsWindow::Paste()
{
    TransferableDataHelper aTransferData(TransferableDataHelper::CreateFromSystemClipboard(this));
    OReportExchange::TSectionElements aCopies = OReportExchange::extractCopies(aTransferData);
    if ( aCopies.getLength() > 1 )
        ::std::for_each(m_aSections.begin(),m_aSections.end(),
            [&aCopies] (const TSectionsMap::value_type& sectionPtr) {
                sectionPtr->getReportSection().Paste(aCopies);
            });
    else
    {
        OSectionWindow*  pMarkedSection = getMarkedSection();
        if ( pMarkedSection )
            pMarkedSection->getReportSection().Paste(aCopies,true);
    }
}

OSectionWindow* OViewsWindow::getSectionWindow(const uno::Reference< report::XSection>& _xSection) const
{
    OSL_ENSURE(_xSection.is(),"Section is NULL!");

    OSectionWindow* pSectionWindow = nullptr;
    for (VclPtr<OSectionWindow> const & p : m_aSections)
    {
        if (p->getReportSection().getSection() == _xSection)
        {
            pSectionWindow = p.get();
            break;
        }
    }

    return pSectionWindow;
}


OSectionWindow* OViewsWindow::getMarkedSection(NearSectionAccess nsa) const
{
    OSectionWindow* pRet = nullptr;
    TSectionsMap::const_iterator aIter = m_aSections.begin();
    TSectionsMap::const_iterator aEnd = m_aSections.end();
    sal_uInt32 nCurrentPosition = 0;
    for (; aIter != aEnd ; ++aIter)
    {
        if ( (*aIter)->getStartMarker().isMarked() )
        {
            if (nsa == CURRENT)
            {
                pRet = aIter->get();
                break;
            }
            else if ( nsa == PREVIOUS )
            {
                if (nCurrentPosition > 0)
                {
                    pRet = (--aIter)->get();
                    if (pRet == nullptr)
                    {
                        pRet = m_aSections.begin()->get();
                    }
                }
                else
                {
                    // if we are out of bounds return the first one
                    pRet = m_aSections.begin()->get();
                }
                break;
            }
            else if ( nsa == POST )
            {
                sal_uInt32 nSize = m_aSections.size();
                if ((nCurrentPosition + 1) < nSize)
                {
                    pRet = (++aIter)->get();
                    if (pRet == nullptr)
                    {
                        pRet = (--aEnd)->get();
                    }
                }
                else
                {
                    // if we are out of bounds return the last one
                    pRet = (--aEnd)->get();
                }
                break;
            }
        }
        ++nCurrentPosition;
    }

    return pRet;
}

void OViewsWindow::markSection(const sal_uInt16 _nPos)
{
    if ( _nPos < m_aSections.size() )
        m_pParent->setMarked(m_aSections[_nPos]->getReportSection().getSection(),true);
}

bool OViewsWindow::IsPasteAllowed() const
{
    TransferableDataHelper aTransferData( TransferableDataHelper::CreateFromSystemClipboard( const_cast< OViewsWindow* >( this ) ) );
    return aTransferData.HasFormat(OReportExchange::getDescriptorFormatId());
}

void OViewsWindow::SelectAll(const sal_uInt16 _nObjectType)
{
    m_bInUnmark = true;
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        [&_nObjectType] (const TSectionsMap::value_type& sectionPtr) {
            sectionPtr->getReportSection().SelectAll(_nObjectType);
        });
    m_bInUnmark = false;
}

void OViewsWindow::unmarkAllObjects(OSectionView const * _pSectionView)
{
    if ( !m_bInUnmark )
    {
        m_bInUnmark = true;
        for (const auto& rxSection : m_aSections)
        {
            if ( &rxSection->getReportSection().getSectionView() != _pSectionView )
            {
                rxSection->getReportSection().deactivateOle();
                rxSection->getReportSection().getSectionView().UnmarkAllObj();
            }
        }
        m_bInUnmark = false;
    }
}

void OViewsWindow::ConfigurationChanged( utl::ConfigurationBroadcaster*, ConfigurationHints)
{
    ImplInitSettings();
    Invalidate();
}

void OViewsWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        GrabFocus();
        const uno::Sequence< beans::PropertyValue> aArgs;
        getView()->getReportView()->getController().executeChecked(SID_SELECT_REPORT,aArgs);
    }
    Window::MouseButtonDown(rMEvt);
}

void OViewsWindow::showRuler(bool _bShow)
{
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        [_bShow] (const TSectionsMap::value_type& sectionPtr) {
            sectionPtr->getStartMarker().showRuler(_bShow);
        });
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        [] (const TSectionsMap::value_type& sectionPtr) {
            sectionPtr->getStartMarker().Window::Invalidate(InvalidateFlags::NoErase);
        });
}

void OViewsWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        auto aIter = std::find_if(m_aSections.begin(), m_aSections.end(),
            [](const VclPtr<OSectionWindow>& rxSection) { return rxSection->getReportSection().getSectionView().AreObjectsMarked(); });
        if (aIter != m_aSections.end())
        {
            (*aIter)->getReportSection().MouseButtonUp(rMEvt);
        }

        // remove special insert mode
        for (const auto& rxSection : m_aSections)
        {
            rxSection->getReportSection().getPage()->resetSpecialMode();
        }
    }
}

bool OViewsWindow::handleKeyEvent(const KeyEvent& _rEvent)
{
    bool bRet = false;
    for (const auto& rxSection : m_aSections)
    {
        if ( rxSection->getStartMarker().isMarked() )
        {
            bRet = rxSection->getReportSection().handleKeyEvent(_rEvent);
        }
    }
    return bRet;
}

OViewsWindow::TSectionsMap::iterator OViewsWindow::getIteratorAtPos(sal_uInt16 _nPos)
{
    TSectionsMap::iterator aRet = m_aSections.end();
    if ( _nPos < m_aSections.size() )
        aRet = m_aSections.begin() + _nPos;
    return aRet;
}

void OViewsWindow::setMarked(OSectionView const * _pSectionView, bool _bMark)
{
    OSL_ENSURE(_pSectionView != nullptr,"SectionView is NULL!");
    if ( _pSectionView )
        setMarked(_pSectionView->getReportSection()->getSection(),_bMark);
}

void OViewsWindow::setMarked(const uno::Reference< report::XSection>& _xSection, bool _bMark)
{
    for (const auto& rxSection : m_aSections)
    {
        if ( rxSection->getReportSection().getSection() != _xSection )
        {
            rxSection->setMarked(false);
        }
        else if ( rxSection->getStartMarker().isMarked() != _bMark )
        {
            rxSection->setMarked(_bMark);
        }
    }
}

void OViewsWindow::setMarked(const uno::Sequence< uno::Reference< report::XReportComponent> >& _aShapes, bool _bMark)
{
    bool bFirst = true;
    const uno::Reference< report::XReportComponent>* pIter = _aShapes.getConstArray();
    const uno::Reference< report::XReportComponent>* pEnd  = pIter + _aShapes.getLength();
    for(;pIter != pEnd;++pIter)
    {
        const uno::Reference< report::XSection> xSection = (*pIter)->getSection();
        if ( xSection.is() )
        {
            if ( bFirst )
            {
                bFirst = false;
                m_pParent->setMarked(xSection,_bMark);
            }
            OSectionWindow* pSectionWindow = getSectionWindow(xSection);
            if ( pSectionWindow )
            {
                SvxShape* pShape = SvxShape::getImplementation( *pIter );
                SdrObject* pObject = pShape ? pShape->GetSdrObject() : nullptr;
                OSL_ENSURE( pObject, "OViewsWindow::setMarked: no SdrObject for the shape!" );
                if ( pObject )
                    pSectionWindow->getReportSection().getSectionView().MarkObj( pObject, pSectionWindow->getReportSection().getSectionView().GetSdrPageView(), !_bMark );
            }
        }
    }
}

void OViewsWindow::collectRectangles(TRectangleMap& _rSortRectangles)
{
    for (const auto& rxSection : m_aSections)
    {
        OSectionView& rView = rxSection->getReportSection().getSectionView();
        if ( rView.AreObjectsMarked() )
        {
            rView.SortMarkedObjects();
            const size_t nCount = rView.GetMarkedObjectCount();
            for (size_t i=0; i < nCount; ++i)
            {
                const SdrMark* pM = rView.GetSdrMarkByIndex(i);
                SdrObject* pObj = pM->GetMarkedSdrObj();
                tools::Rectangle aObjRect(pObj->GetSnapRect());
                _rSortRectangles.emplace(aObjRect,TRectangleMap::mapped_type(pObj,&rView));
            }
        }
    }
}

void OViewsWindow::collectBoundResizeRect(const TRectangleMap& _rSortRectangles, ControlModification _nControlModification,bool _bAlignAtSection, tools::Rectangle& _rBound, tools::Rectangle& _rResize)
{
    bool bOnlyOnce = false;
    for (const auto& [aObjRect, rObjViewPair] : _rSortRectangles)
    {
        if ( _rResize.IsEmpty() )
            _rResize = aObjRect;
        switch(_nControlModification)
        {
            case ControlModification::WIDTH_SMALLEST:
                if ( _rResize.getWidth() > aObjRect.getWidth() )
                    _rResize = aObjRect;
                break;
            case ControlModification::HEIGHT_SMALLEST:
                if ( _rResize.getHeight() > aObjRect.getHeight() )
                    _rResize = aObjRect;
                break;
            case ControlModification::WIDTH_GREATEST:
                if ( _rResize.getWidth() < aObjRect.getWidth() )
                    _rResize = aObjRect;
                break;
            case ControlModification::HEIGHT_GREATEST:
                if ( _rResize.getHeight() < aObjRect.getHeight() )
                    _rResize = aObjRect;
                break;
            default: break;
        }

        SdrObjTransformInfoRec aInfo;
        const SdrObject* pObj = rObjViewPair.first;
        pObj->TakeObjInfo(aInfo);
        bool bHasFixed = !aInfo.bMoveAllowed || pObj->IsMoveProtect();
        if ( bHasFixed )
            _rBound.Union(aObjRect);
        else
        {
            if ( _bAlignAtSection || _rSortRectangles.size() == 1 )
            { // align single object at the page
                if ( ! bOnlyOnce )
                {
                    bOnlyOnce = true;
                    OReportSection* pReportSection = rObjViewPair.second->getReportSection();
                    const uno::Reference< report::XSection>& xSection = pReportSection->getSection();
                    try
                    {
                        uno::Reference<report::XReportDefinition> xReportDefinition = xSection->getReportDefinition();
                        _rBound.Union(tools::Rectangle(getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_LEFTMARGIN),0,
                                            getStyleProperty<awt::Size>(xReportDefinition,PROPERTY_PAPERSIZE).Width  - getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_RIGHTMARGIN),
                                            xSection->getHeight()));
                    }
                    catch(const uno::Exception &){}
                }
            }
            else
            {
                _rBound.Union(rObjViewPair.second->GetMarkedObjRect());
            }
        }
    }
}

void OViewsWindow::alignMarkedObjects(ControlModification _nControlModification, bool _bAlignAtSection)
{
    if ( _nControlModification == ControlModification::NONE )
        return;

    Point aRefPoint;
    RectangleLess::CompareMode eCompareMode = RectangleLess::POS_LEFT;
    switch (_nControlModification)
    {
        case ControlModification::TOP   : eCompareMode = RectangleLess::POS_UPPER; break;
        case ControlModification::BOTTOM: eCompareMode = RectangleLess::POS_DOWN; break;
        case ControlModification::LEFT  : eCompareMode = RectangleLess::POS_LEFT; break;
        case ControlModification::RIGHT : eCompareMode = RectangleLess::POS_RIGHT; break;
        case ControlModification::CENTER_HORIZONTAL :
        case ControlModification::CENTER_VERTICAL :
            {
                eCompareMode = (ControlModification::CENTER_VERTICAL == _nControlModification) ?  RectangleLess::POS_CENTER_VERTICAL :  RectangleLess::POS_CENTER_HORIZONTAL;
                uno::Reference<report::XSection> xSection = (*m_aSections.begin())->getReportSection().getSection();
                uno::Reference<report::XReportDefinition> xReportDefinition = xSection->getReportDefinition();
                aRefPoint = tools::Rectangle(getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_LEFTMARGIN),0,
                                        getStyleProperty<awt::Size>(xReportDefinition,PROPERTY_PAPERSIZE).Width  - getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_RIGHTMARGIN),
                                        xSection->getHeight()).Center();
            }
            break;
        default: break;
    }
    RectangleLess aCompare(eCompareMode,aRefPoint);
    TRectangleMap aSortRectangles(aCompare);
    collectRectangles(aSortRectangles);

    tools::Rectangle aBound;
    tools::Rectangle aResize;
    collectBoundResizeRect(aSortRectangles,_nControlModification,_bAlignAtSection,aBound,aResize);

    bool bMove = true;

    auto aGetFun = ::std::mem_fn(&tools::Rectangle::Bottom);
    auto aSetFun = ::std::mem_fn(&tools::Rectangle::SetBottom);
    auto aRefFun = ::std::mem_fn(&tools::Rectangle::Top);
    TRectangleMap::const_iterator aRectIter = aSortRectangles.begin();
    TRectangleMap::const_iterator aRectEnd = aSortRectangles.end();
    for (;aRectIter != aRectEnd ; ++aRectIter)
    {
        tools::Rectangle aObjRect = aRectIter->first;
        SdrObject* pObj = aRectIter->second.first;
        SdrView* pView = aRectIter->second.second;
        Point aCenter(aBound.Center());
        SdrObjTransformInfoRec aInfo;
        pObj->TakeObjInfo(aInfo);
        if (aInfo.bMoveAllowed && !pObj->IsMoveProtect())
        {
            long nXMov = 0;
            long nYMov = 0;
            long* pValue = &nXMov;
            switch(_nControlModification)
            {
                case ControlModification::TOP   :
                    aGetFun  = ::std::mem_fn(&tools::Rectangle::Top);
                    aSetFun  = ::std::mem_fn(&tools::Rectangle::SetTop);
                    aRefFun  = ::std::mem_fn(&tools::Rectangle::Bottom);
                    pValue = &nYMov;
                    break;
                case ControlModification::BOTTOM:
                    // defaults are already set
                    pValue = &nYMov;
                    break;
                case ControlModification::CENTER_VERTICAL:
                    nYMov = aCenter.Y() - aObjRect.Center().Y();
                    pValue = &nYMov;
                    bMove = false;
                    break;
                case ControlModification::RIGHT :
                    aGetFun  = ::std::mem_fn(&tools::Rectangle::Right);
                    aSetFun  = ::std::mem_fn(&tools::Rectangle::SetRight);
                    aRefFun  = ::std::mem_fn(&tools::Rectangle::Left);
                    break;
                case ControlModification::CENTER_HORIZONTAL:
                    nXMov = aCenter.X() - aObjRect.Center().X();
                    bMove = false;
                    break;
                case ControlModification::LEFT  :
                    aGetFun  = ::std::mem_fn(&tools::Rectangle::Left);
                    aSetFun  = ::std::mem_fn(&tools::Rectangle::SetLeft);
                    aRefFun  = ::std::mem_fn(&tools::Rectangle::Right);
                    break;
                default:
                    bMove = false;
                    break;
            }
            if ( bMove )
            {
                tools::Rectangle aTest = aObjRect;
                aSetFun(&aTest, aGetFun(&aBound));
                TRectangleMap::const_iterator aInterSectRectIter = aSortRectangles.begin();
                for (; aInterSectRectIter != aRectIter; ++aInterSectRectIter)
                {
                    if ( pView == aInterSectRectIter->second.second && (dynamic_cast<OUnoObject*>(aInterSectRectIter->second.first) || dynamic_cast<OOle2Obj*>(aInterSectRectIter->second.first)))
                    {
                        SdrObject* pPreviousObj = aInterSectRectIter->second.first;
                        tools::Rectangle aIntersectRect = aTest.GetIntersection( pPreviousObj->GetSnapRect());
                        if ( !aIntersectRect.IsEmpty() && (aIntersectRect.Left() != aIntersectRect.Right() && aIntersectRect.Top() != aIntersectRect.Bottom() ) )
                        {
                            *pValue = aRefFun(&aIntersectRect) - aGetFun(&aObjRect);
                            break;
                        }
                    }
                }
                if ( aInterSectRectIter == aRectIter )
                    *pValue = aGetFun(&aBound) - aGetFun(&aObjRect);
            }

            if ( lcl_getNewRectSize(aObjRect,nXMov,nYMov,pObj,pView,_nControlModification) )
            {
                const Size aSize(nXMov,nYMov);
                pView->AddUndo(pView->GetModel()->GetSdrUndoFactory().CreateUndoMoveObject(*pObj,aSize));
                pObj->Move(aSize);
                aObjRect =   pObj->GetSnapRect();
            }

            // resizing control
            if ( !aResize.IsEmpty() && aObjRect != aResize )
            {
                nXMov = aResize.getWidth();
                nYMov = aResize.getHeight();
                switch(_nControlModification)
                {
                    case ControlModification::WIDTH_GREATEST:
                    case ControlModification::HEIGHT_GREATEST:
                        if ( _nControlModification == ControlModification::HEIGHT_GREATEST )
                            nXMov = aObjRect.getWidth();
                        else if ( _nControlModification == ControlModification::WIDTH_GREATEST )
                            nYMov = aObjRect.getHeight();
                        lcl_getNewRectSize(aObjRect,nXMov,nYMov,pObj,pView,_nControlModification);
                        [[fallthrough]];
                    case ControlModification::WIDTH_SMALLEST:
                    case ControlModification::HEIGHT_SMALLEST:
                        pView->AddUndo( pView->GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));
                        {
                            OObjectBase* pObjBase = dynamic_cast<OObjectBase*>(pObj);
                            OSL_ENSURE(pObjBase,"Where comes this object from?");
                            if ( pObjBase )
                            {
                                if ( _nControlModification == ControlModification::WIDTH_SMALLEST || _nControlModification == ControlModification::WIDTH_GREATEST )
                                    pObjBase->getReportComponent()->setSize(awt::Size(nXMov,aObjRect.getHeight()));
                                else if ( _nControlModification == ControlModification::HEIGHT_GREATEST || _nControlModification == ControlModification::HEIGHT_SMALLEST )
                                    pObjBase->getReportComponent()->setSize(awt::Size(aObjRect.getWidth(),nYMov));
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        pView->AdjustMarkHdl();
    }
}

void OViewsWindow::createDefault()
{
    OSectionWindow* pMarkedSection = getMarkedSection();
    if ( pMarkedSection )
        pMarkedSection->getReportSection().createDefault(m_sShapeType);
}

void OViewsWindow::setGridSnap(bool bOn)
{
    for (const auto& rxSection : m_aSections)
    {
        rxSection->getReportSection().getSectionView().SetGridSnap(bOn);
        rxSection->getReportSection().Invalidate();
    }
}

void OViewsWindow::setDragStripes(bool bOn)
{
    for (const auto& rxSection : m_aSections)
        rxSection->getReportSection().getSectionView().SetDragStripes(bOn);
}

sal_uInt16 OViewsWindow::getPosition(const OSectionWindow* _pSectionWindow) const
{
    auto aIter = std::find_if(m_aSections.begin(), m_aSections.end(),
        [&_pSectionWindow](const VclPtr<OSectionWindow>& rxSection) { return _pSectionWindow == rxSection.get(); });
    return static_cast<sal_uInt16>(std::distance(m_aSections.begin(), aIter));
}

OSectionWindow* OViewsWindow::getSectionWindow(const sal_uInt16 _nPos) const
{
    OSectionWindow* aReturn = nullptr;

    if ( _nPos < m_aSections.size() )
        aReturn = m_aSections[_nPos].get();

    return aReturn;
}

namespace
{
    enum SectionViewAction
    {
        eEndDragObj,
        eEndAction,
        eForceToAnotherPage,
        eBreakAction
    };
    class ApplySectionViewAction
    {
    private:
        SectionViewAction const m_eAction;

    public:
        explicit ApplySectionViewAction()
            : m_eAction(eEndDragObj)
        {
        }

        explicit ApplySectionViewAction(SectionViewAction _eAction)
            : m_eAction(_eAction)
        {
        }

        void operator() ( const OViewsWindow::TSectionsMap::value_type& _rhs )
        {
            OSectionView& rView( _rhs->getReportSection().getSectionView() );
            switch ( m_eAction )
            {
            case eEndDragObj:
                rView.EndDragObj();
                break;
            case eEndAction:
                if ( rView.IsAction() )
                    rView.EndAction (      );
                break;
            case eForceToAnotherPage:
                rView.ForceMarkedToAnotherPage();
                break;
            case eBreakAction:
                if ( rView.IsAction() )
                    rView.BrkAction (      );
                break;
                // default:

            }
        }
    };
}

void OViewsWindow::BrkAction()
{
    EndDragObj_removeInvisibleObjects();
    ::std::for_each( m_aSections.begin(), m_aSections.end(), ApplySectionViewAction(eBreakAction) );
}

void OViewsWindow::BegDragObj_createInvisibleObjectAtPosition(const tools::Rectangle& _aRect, const OSectionView& _rSection)
{
    Point aNewPos(0,0);

    for (const auto& rxSection : m_aSections)
    {
        OReportSection& rReportSection = rxSection->getReportSection();
        rReportSection.getPage()->setSpecialMode();
        OSectionView& rView = rReportSection.getSectionView();

        if ( &rView != &_rSection )
        {
            SdrObject *pNewObj = new SdrUnoObj(
                rView.getSdrModelFromSdrView(),
                "com.sun.star.form.component.FixedText");

            pNewObj->SetLogicRect(_aRect);
            pNewObj->Move(Size(0, aNewPos.Y()));
            bool bChanged = rView.GetModel()->IsChanged();
            rReportSection.getPage()->InsertObject(pNewObj);
            rView.GetModel()->SetChanged(bChanged);
            m_aBegDragTempList.push_back(pNewObj);

            rView.MarkObj( pNewObj, rView.GetSdrPageView() );
        }
        const long nSectionHeight = rReportSection.PixelToLogic(rReportSection.GetOutputSizePixel()).Height();
        aNewPos.AdjustY( -nSectionHeight );
    }
}

void OViewsWindow::BegDragObj(const Point& _aPnt, SdrHdl* _pHdl,const OSectionView* _pSection)
{
    SAL_INFO(
        "reportdesign", "Clickpoint X:" << _aPnt.X() << " Y:" << _aPnt.Y());

    m_aBegDragTempList.clear();

    // Calculate the absolute clickpoint in the views
    Point aAbsolutePnt = _aPnt;
    for (const auto& rxSection : m_aSections)
    {
        OReportSection& rReportSection = rxSection->getReportSection();
        OSectionView* pView = &rReportSection.getSectionView();
        if (pView == _pSection)
            break;
        const long nSectionHeight = rReportSection.PixelToLogic(rReportSection.GetOutputSizePixel()).Height();
        aAbsolutePnt.AdjustY(nSectionHeight );
    }
    m_aDragDelta = Point(SAL_MAX_INT32, SAL_MAX_INT32);
    SAL_INFO(
        "reportdesign",
        "Absolute X:" << aAbsolutePnt.X() << " Y:" << aAbsolutePnt.Y());

    // Create drag lines over all viewable Views
    // Therefore we need to identify the marked objects
    // and create temporary objects on all other views at the same position
    // relative to its occurrence.

    int nViewCount = 0;
    Point aNewObjPos(0,0);
    Point aLeftTop = Point(SAL_MAX_INT32, SAL_MAX_INT32);
    for (const auto& rxSection : m_aSections)
    {
        OReportSection& rReportSection = rxSection->getReportSection();

        OSectionView& rView = rReportSection.getSectionView();

        if ( rView.AreObjectsMarked() )
        {
            const size_t nCount = rView.GetMarkedObjectCount();
            for (size_t i=0; i < nCount; ++i)
            {
                const SdrMark* pM = rView.GetSdrMarkByIndex(i);
                SdrObject* pObj = pM->GetMarkedSdrObj();
                if (::std::find(m_aBegDragTempList.begin(),m_aBegDragTempList.end(),pObj) == m_aBegDragTempList.end())
                {
                    tools::Rectangle aRect( pObj->GetCurrentBoundRect() );
                    aRect.Move(0, aNewObjPos.Y());

                    aLeftTop.setX( ::std::min( aRect.Left(), aLeftTop.X() ) );
                    aLeftTop.setY( ::std::min( aRect.Top(), aLeftTop.Y() ) );

                    SAL_INFO(
                        "reportdesign",
                        "createInvisible X:" << aRect.Left() << " Y:"
                            << aRect.Top() << " on View #" << nViewCount);

                    BegDragObj_createInvisibleObjectAtPosition(aRect, rView);
                }
            }
        }
        ++nViewCount;
        tools::Rectangle aClipRect = rView.GetWorkArea();
        aClipRect.SetTop( -aNewObjPos.Y() );
        rView.SetWorkArea( aClipRect );

        const long nSectionHeight = rReportSection.PixelToLogic(rReportSection.GetOutputSizePixel()).Height();
        aNewObjPos.AdjustY(nSectionHeight );
    }

    const sal_Int32 nDeltaX = std::abs(aLeftTop.X() - aAbsolutePnt.X());
    const sal_Int32 nDeltaY = std::abs(aLeftTop.Y() - aAbsolutePnt.Y());
    m_aDragDelta.setX( nDeltaX );
    m_aDragDelta.setY( nDeltaY );

    Point aNewPos = aAbsolutePnt;

    const short nDrgLog = static_cast<short>(PixelToLogic(Size(3,0)).Width());
    nViewCount = 0;
    for (const auto& rxSection : m_aSections)
    {
        OReportSection& rReportSection = rxSection->getReportSection();

        SdrHdl* pHdl = _pHdl;
        if ( pHdl )
        {
            if ( &rReportSection.getSectionView() != _pSection )
            {
                const SdrHdlList& rHdlList = rReportSection.getSectionView().GetHdlList();
                pHdl = rHdlList.GetHdl(_pHdl->GetKind());
            }
        }
        SAL_INFO(
            "reportdesign",
            "X:" << aNewPos.X() << " Y:" << aNewPos.Y() << " on View#"
                << nViewCount++);
        rReportSection.getSectionView().BegDragObj(aNewPos, nullptr, pHdl, nDrgLog);

        const long nSectionHeight = rReportSection.PixelToLogic(rReportSection.GetOutputSizePixel()).Height();
        aNewPos.AdjustY( -nSectionHeight );
    }
}


void OViewsWindow::ForceMarkedToAnotherPage()
{
    ::std::for_each( m_aSections.begin(), m_aSections.end(), ApplySectionViewAction(eForceToAnotherPage ) );
}

void OViewsWindow::BegMarkObj(const Point& _aPnt,const OSectionView* _pSection)
{
    bool bAdd = true;
    Point aNewPos = _aPnt;

    long nLastSectionHeight = 0;
    for (const auto& rxSection : m_aSections)
    {
        OReportSection& rReportSection = rxSection->getReportSection();
        if ( &rReportSection.getSectionView() == _pSection )
        {
            bAdd = false;
            aNewPos = _aPnt; // 2,2
        }
        else if ( bAdd )
        {
            const long nSectionHeight = rReportSection.PixelToLogic(rReportSection.GetOutputSizePixel()).Height();
            aNewPos.AdjustY(nSectionHeight );
        }
        else
        {
            aNewPos.AdjustY( -nLastSectionHeight );
        }
        rReportSection.getSectionView().BegMarkObj ( aNewPos );
        nLastSectionHeight = rReportSection.PixelToLogic(rReportSection.GetOutputSizePixel()).Height();
    }
}

OSectionView* OViewsWindow::getSectionRelativeToPosition(const OSectionView* _pSection,Point& _rPnt)
{
    OSectionView* pSection = nullptr;
    TSectionsMap::const_iterator aIter = m_aSections.begin();
    const TSectionsMap::const_iterator aEnd = m_aSections.end();
    aIter = std::find_if(aIter, aEnd, [&_pSection](const VclPtr<OSectionWindow>& rxSection) {
        return &rxSection->getReportSection().getSectionView() == _pSection; });
    sal_Int32 nCount = static_cast<sal_Int32>(std::distance(m_aSections.cbegin(), aIter));
    OSL_ENSURE(aIter != aEnd,"This can never happen!");
    if ( _rPnt.Y() < 0 )
    {
        if ( nCount )
            --aIter;
        for (; nCount && (_rPnt.Y() < 0); --nCount)
        {
            OReportSection& rReportSection = (*aIter)->getReportSection();
            const sal_Int32 nHeight = rReportSection.PixelToLogic(rReportSection.GetOutputSizePixel()).Height();
            _rPnt.AdjustY(nHeight );
            if ( (nCount -1) > 0 && (_rPnt.Y() < 0) )
                --aIter;
        }
        if ( nCount == 0 )
            pSection = &(*m_aSections.begin())->getReportSection().getSectionView();
        else
            pSection = &(*aIter)->getReportSection().getSectionView();
    }
    else
    {
        for (; aIter != aEnd; ++aIter)
        {
            OReportSection& rReportSection = (*aIter)->getReportSection();
            const long nHeight = rReportSection.PixelToLogic(rReportSection.GetOutputSizePixel()).Height();
            if ( (_rPnt.Y() - nHeight) < 0  )
                break;
            _rPnt.AdjustY( -nHeight );
        }
        if ( aIter != aEnd )
            pSection = &(*aIter)->getReportSection().getSectionView();
        else
            pSection = &(*(aEnd-1))->getReportSection().getSectionView();
    }

    return pSection;
}

void OViewsWindow::EndDragObj_removeInvisibleObjects()
{
    for (const auto& rxSection : m_aSections)
    {
        OReportSection& rReportSection = rxSection->getReportSection();
        rReportSection.getPage()->resetSpecialMode();
    }
}

void OViewsWindow::EndDragObj(bool _bControlKeyPressed, const OSectionView* _pSection, const Point& _aPnt)
{
    const OUString sUndoAction = RptResId(RID_STR_UNDO_CHANGEPOSITION);
    const UndoContext aUndoContext( getView()->getReportView()->getController().getUndoManager(), sUndoAction );

    Point aNewPos = _aPnt;
    OSectionView* pInSection = getSectionRelativeToPosition(_pSection, aNewPos);
    if (!_bControlKeyPressed &&
        _pSection && !_pSection->IsDragResize() && /* Not in resize mode */
        _pSection != pInSection)
    {
        EndDragObj_removeInvisibleObjects();

        // we need to manipulate the current clickpoint, we subtract the old delta from BeginDrag
        aNewPos -= m_aDragDelta;

        uno::Sequence< beans::NamedValue > aAllreadyCopiedObjects;
        for (const auto& rxSection : m_aSections)
        {
            OReportSection& rReportSection = rxSection->getReportSection();
            if ( pInSection != &rReportSection.getSectionView() )
            {
                rReportSection.getSectionView().BrkAction();
                rReportSection.Copy(aAllreadyCopiedObjects,true);
            }
            else
                pInSection->EndDragObj();
        }

        if ( aAllreadyCopiedObjects.getLength() )
        {
            try
            {
                uno::Reference<report::XReportDefinition> xReportDefinition = getView()->getReportView()->getController().getReportDefinition();
                const sal_Int32 nLeftMargin  = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_LEFTMARGIN);
                const sal_Int32 nRightMargin = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_RIGHTMARGIN);
                const sal_Int32 nPaperWidth  = getStyleProperty<awt::Size>(xReportDefinition,PROPERTY_PAPERSIZE).Width;

                if ( aNewPos.X() < nLeftMargin )
                    aNewPos.setX( nLeftMargin );
                if ( aNewPos.Y() < 0 )
                    aNewPos.setY( 0 );

                Point aPrevious;
                for (beans::NamedValue const & namedVal : aAllreadyCopiedObjects)
                {
                    uno::Sequence< uno::Reference<report::XReportComponent> > aClones;
                    namedVal.Value >>= aClones;
                    uno::Reference<report::XReportComponent>* pColIter = aClones.getArray();
                    const uno::Reference<report::XReportComponent>* pColEnd = pColIter + aClones.getLength();

                    // move the cloned Components to new positions
                    for (; pColIter != pColEnd; ++pColIter)
                    {
                        uno::Reference< report::XReportComponent> xRC(*pColIter);
                        aPrevious = VCLPoint(xRC->getPosition());
                        awt::Size aSize = xRC->getSize();

                        if ( aNewPos.X() < nLeftMargin )
                        {
                            aNewPos.setX( nLeftMargin );
                        }
                        else if ( (aNewPos.X() + aSize.Width) > (nPaperWidth - nRightMargin) )
                        {
                            aNewPos.setX( nPaperWidth - nRightMargin - aSize.Width );
                        }
                        if ( aNewPos.Y() < 0 )
                        {
                            aNewPos.setY( 0 );
                        }
                        if ( aNewPos.X() < 0 )
                        {
                            aSize.Width += aNewPos.X();
                            aNewPos.setX( 0 );
                            xRC->setSize(aSize);
                        }
                        xRC->setPosition(AWTPoint(aNewPos));
                        if ( (pColIter+1) != pColEnd )
                        {
                            // bring aNewPos to the position of the next object
                            uno::Reference< report::XReportComponent> xRCNext(*(pColIter + 1),uno::UNO_QUERY);
                            Point aNextPosition = VCLPoint(xRCNext->getPosition());
                            aNewPos += aNextPosition - aPrevious;
                        }
                    }
                }
            }
            catch(uno::Exception&)
            {
            }
            pInSection->getReportSection()->Paste(aAllreadyCopiedObjects,true);
        }
    }
    else
    {
        ::std::for_each( m_aSections.begin(), m_aSections.end(), ApplySectionViewAction() );
        EndDragObj_removeInvisibleObjects();
    }
    m_aDragDelta = Point(SAL_MAX_INT32, SAL_MAX_INT32);
}

void OViewsWindow::EndAction()
{
    ::std::for_each( m_aSections.begin(), m_aSections.end(), ApplySectionViewAction(eEndAction) );
}

void OViewsWindow::MovAction(const Point& _aPnt,const OSectionView* _pSection, bool _bControlKeySet)
{
    Point aRealMousePos = _aPnt;
    Point aCurrentSectionPos;
    SAL_INFO(
        "reportdesign",
        "X:" << aRealMousePos.X() << " Y:" << aRealMousePos.Y());

    Point aHdlPos;
    SdrHdl* pHdl = _pSection->GetDragHdl();
    if ( pHdl )
    {
        aHdlPos = pHdl->GetPos();
    }

    for (const auto& rxSection : m_aSections)
    {
        OReportSection& rReportSection = rxSection->getReportSection();
        if ( &rReportSection.getSectionView() == _pSection )
            break;
        const long nSectionHeight = rxSection->PixelToLogic(rReportSection.GetOutputSizePixel()).Height();
        aCurrentSectionPos.AdjustY(nSectionHeight );
    }
    aRealMousePos += aCurrentSectionPos;

    // If control key is pressed the work area is limited to the section with the current selection.
    Point aPosForWorkArea(0,0);
    for (const auto& rxSection : m_aSections)
    {
        OReportSection& rReportSection = rxSection->getReportSection();
        OSectionView& rView = rReportSection.getSectionView();
        const long nSectionHeight = rxSection->PixelToLogic(rxSection->GetOutputSizePixel()).Height();

        if (_bControlKeySet)
        {
            tools::Rectangle aClipRect = rView.GetWorkArea();
            aClipRect.SetTop( aCurrentSectionPos.Y() - aPosForWorkArea.Y() );
            aClipRect.SetBottom( aClipRect.Top() + nSectionHeight );
            rView.SetWorkArea( aClipRect );
        }
        else
        {
            tools::Rectangle aClipRect = rView.GetWorkArea();
            aClipRect.SetTop( -aPosForWorkArea.Y() );
            rView.SetWorkArea( aClipRect );
        }
        aPosForWorkArea.AdjustY(nSectionHeight );
    }


    for (const auto& rxSection : m_aSections)
    {
        OReportSection& rReportSection = rxSection->getReportSection();
        SdrHdl* pCurrentHdl = rReportSection.getSectionView().GetDragHdl();
        if ( pCurrentHdl && aRealMousePos.Y() > 0 )
            aRealMousePos = _aPnt + pCurrentHdl->GetPos() - aHdlPos;
        rReportSection.getSectionView().MovAction ( aRealMousePos );
        const long nSectionHeight = rxSection->PixelToLogic(rxSection->GetOutputSizePixel()).Height();
        aRealMousePos.AdjustY( -nSectionHeight );
    }
}

bool OViewsWindow::IsAction() const
{
    return std::any_of(m_aSections.begin(), m_aSections.end(),
        [](const VclPtr<OSectionWindow>& rxSection) { return rxSection->getReportSection().getSectionView().IsAction(); });
}

bool OViewsWindow::IsDragObj() const
{
    return std::any_of(m_aSections.begin(), m_aSections.end(),
        [](const VclPtr<OSectionWindow>& rxSection) { return rxSection->getReportSection().getSectionView().IsAction(); });
}

sal_uInt32 OViewsWindow::getMarkedObjectCount() const
{
    return std::accumulate(m_aSections.begin(), m_aSections.end(), sal_uInt32(0),
        [](const sal_uInt32 nCount, const VclPtr<OSectionWindow>& rxSection) {
            return nCount + static_cast<sal_uInt32>(rxSection->getReportSection().getSectionView().GetMarkedObjectCount()); });
}

void OViewsWindow::handleKey(const vcl::KeyCode& _rCode)
{
    const sal_uInt16 nCode = _rCode.GetCode();
    if ( _rCode.IsMod1() )
    {
        // scroll page
        OScrollWindowHelper* pScrollWindow = getView()->getScrollWindow();
        ScrollBar& rScrollBar = ( nCode == KEY_LEFT || nCode == KEY_RIGHT ) ? pScrollWindow->GetHScroll() : pScrollWindow->GetVScroll();
        if ( rScrollBar.IsVisible() )
            rScrollBar.DoScrollAction(( nCode == KEY_RIGHT || nCode == KEY_UP ) ? ScrollType::LineUp : ScrollType::LineDown );
        return;
    }

    for (const auto& rxSection : m_aSections)
    {
        OReportSection& rReportSection = rxSection->getReportSection();
        long nX = 0;
        long nY = 0;

        if ( nCode == KEY_UP )
            nY = -1;
        else if ( nCode == KEY_DOWN )
            nY =  1;
        else if ( nCode == KEY_LEFT )
            nX = -1;
        else if ( nCode == KEY_RIGHT )
            nX =  1;

        if ( rReportSection.getSectionView().AreObjectsMarked() )
        {
            if ( _rCode.IsMod2() )
            {
                // move in 1 pixel distance
                const Size aPixelSize = rReportSection.PixelToLogic( Size( 1, 1 ) );
                nX *= aPixelSize.Width();
                nY *= aPixelSize.Height();
            }
            else
            {
                // move in 1 mm distance
                nX *= DEFAUL_MOVE_SIZE;
                nY *= DEFAUL_MOVE_SIZE;
            }

            OSectionView& rView = rReportSection.getSectionView();
            const SdrHdlList& rHdlList = rView.GetHdlList();
            SdrHdl* pHdl = rHdlList.GetFocusHdl();

            if ( pHdl == nullptr )
            {
                // no handle selected
                if ( rView.IsMoveAllowed() )
                {
                    // restrict movement to work area
                    tools::Rectangle rWorkArea = rView.GetWorkArea();
                    rWorkArea.AdjustRight( 1 );

                    if ( !rWorkArea.IsEmpty() )
                    {
                        if ( rWorkArea.Top() < 0 )
                            rWorkArea.SetTop( 0 );
                        tools::Rectangle aMarkRect( rView.GetMarkedObjRect() );
                        aMarkRect.Move( nX, nY );

                        if ( !rWorkArea.IsInside( aMarkRect ) )
                        {
                            if ( aMarkRect.Left() < rWorkArea.Left() )
                                nX += rWorkArea.Left() - aMarkRect.Left();

                            if ( aMarkRect.Right() > rWorkArea.Right() )
                                nX -= aMarkRect.Right() - rWorkArea.Right();

                            if ( aMarkRect.Top() < rWorkArea.Top() )
                                nY += rWorkArea.Top() - aMarkRect.Top();

                            if ( aMarkRect.Bottom() > rWorkArea.Bottom() )
                                nY -= aMarkRect.Bottom() - rWorkArea.Bottom();
                        }
                        bool bCheck = false;
                        const SdrMarkList& rMarkList = rView.GetMarkedObjectList();
                        for (size_t i = 0; !bCheck && i < rMarkList.GetMarkCount(); ++i )
                        {
                            SdrMark* pMark = rMarkList.GetMark(i);
                            bCheck = dynamic_cast<OUnoObject*>(pMark->GetMarkedSdrObj()) != nullptr|| dynamic_cast<OOle2Obj*>(pMark->GetMarkedSdrObj());
                        }


                        if ( bCheck )
                        {
                            SdrObject* pOverlapped = isOver(aMarkRect,*rReportSection.getPage(),rView);
                            if ( pOverlapped )
                            {
                                do
                                {
                                    tools::Rectangle aOver = pOverlapped->GetLastBoundRect();
                                    Point aPos;
                                    if ( nCode == KEY_UP )
                                    {
                                        aPos.setX( aMarkRect.Left() );
                                        aPos.setY( aOver.Top() - aMarkRect.getHeight() );
                                        nY += (aPos.Y() - aMarkRect.Top());
                                    }
                                    else if ( nCode == KEY_DOWN )
                                    {
                                        aPos.setX( aMarkRect.Left() );
                                        aPos.setY( aOver.Bottom() );
                                        nY += (aPos.Y() - aMarkRect.Top());
                                    }
                                    else if ( nCode == KEY_LEFT )
                                    {
                                        aPos.setX( aOver.Left() - aMarkRect.getWidth() );
                                        aPos.setY( aMarkRect.Top() );
                                        nX += (aPos.X() - aMarkRect.Left());
                                    }
                                    else if ( nCode == KEY_RIGHT )
                                    {
                                        aPos.setX( aOver.Right() );
                                        aPos.setY( aMarkRect.Top() );
                                        nX += (aPos.X() - aMarkRect.Left());
                                    }

                                    aMarkRect.SetPos(aPos);
                                    if ( !rWorkArea.IsInside( aMarkRect ) )
                                    {
                                        break;
                                    }
                                    pOverlapped = isOver(aMarkRect,*rReportSection.getPage(),rView);
                                }
                                while(pOverlapped != nullptr);
                                if (pOverlapped != nullptr)
                                    break;
                            }
                        }
                    }

                    if ( nX != 0 || nY != 0 )
                    {
                        rView.MoveAllMarked( Size( nX, nY ) );
                        rView.MakeVisible( rView.GetAllMarkedRect(), rReportSection);
                    }
                }
            }
            else // pHdl != nullptr
            {
                // move the handle
                if (nX || nY)
                {
                    const Point aStartPoint( pHdl->GetPos() );
                    const Point aEndPoint( pHdl->GetPos() + Point( nX, nY ) );
                    const SdrDragStat& rDragStat = rView.GetDragStat();

                    // start dragging
                    rView.BegDragObj( aStartPoint, nullptr, pHdl, 0 );

                    if ( rView.IsDragObj() )
                    {
                        const bool bWasNoSnap = rDragStat.IsNoSnap();
                        const bool bWasSnapEnabled = rView.IsSnapEnabled();

                        // switch snapping off
                        if ( !bWasNoSnap )
                            const_cast<SdrDragStat&>(rDragStat).SetNoSnap();
                        if ( bWasSnapEnabled )
                            rView.SetSnapEnabled( false );

                        tools::Rectangle aNewRect;
                        bool bCheck = false;
                        const SdrMarkList& rMarkList = rView.GetMarkedObjectList();
                        for (size_t i = 0; !bCheck && i < rMarkList.GetMarkCount(); ++i )
                        {
                            SdrMark* pMark = rMarkList.GetMark(i);
                            bCheck = dynamic_cast<OUnoObject*>(pMark->GetMarkedSdrObj()) != nullptr || dynamic_cast<OOle2Obj*>(pMark->GetMarkedSdrObj()) != nullptr;
                            if ( bCheck )
                                aNewRect.Union(pMark->GetMarkedSdrObj()->GetLastBoundRect());
                        }

                        switch(pHdl->GetKind())
                        {
                            case SdrHdlKind::Left:
                            case SdrHdlKind::UpperLeft:
                            case SdrHdlKind::LowerLeft:
                            case SdrHdlKind::Upper:
                                aNewRect.AdjustLeft(nX );
                                aNewRect.AdjustTop(nY );
                                break;
                            case SdrHdlKind::UpperRight:
                            case SdrHdlKind::Right:
                            case SdrHdlKind::LowerRight:
                            case SdrHdlKind::Lower:
                                aNewRect.setWidth(aNewRect.getWidth() + nX);
                                aNewRect.setHeight(aNewRect.getHeight() + nY);
                                break;
                            default:
                                break;
                        }
                        if ( !(bCheck && isOver(aNewRect,*rReportSection.getPage(),rView)) )
                            rView.MovAction(aEndPoint);
                        rView.EndDragObj();

                        // restore snap
                        if ( !bWasNoSnap )
                            const_cast<SdrDragStat&>(rDragStat).SetNoSnap( bWasNoSnap );
                        if ( bWasSnapEnabled )
                            rView.SetSnapEnabled( bWasSnapEnabled );
                    }

                    // make moved handle visible
                    const tools::Rectangle aVisRect( aEndPoint - Point( DEFAUL_MOVE_SIZE, DEFAUL_MOVE_SIZE ), Size( 200, 200 ) );
                    rView.MakeVisible( aVisRect, rReportSection);
                }
            }
            rView.AdjustMarkHdl();
        }
    }
}

void OViewsWindow::stopScrollTimer()
{
    ::std::for_each(m_aSections.begin(),m_aSections.end(),
        [] (const TSectionsMap::value_type& sectionPtr) {
            sectionPtr->getReportSection().stopScrollTimer();
        });
}

void OViewsWindow::fillCollapsedSections(::std::vector<sal_uInt16>& _rCollapsedPositions) const
{
    sal_uInt16 i = 0;
    for (const auto& rxSection : m_aSections)
    {
        if ( rxSection->getStartMarker().isCollapsed() )
            _rCollapsedPositions.push_back(i);
        ++i;
    }
}

void OViewsWindow::collapseSections(const uno::Sequence< beans::PropertyValue>& _aCollpasedSections)
{
    const beans::PropertyValue* pIter = _aCollpasedSections.getConstArray();
    const beans::PropertyValue* pEnd = pIter + _aCollpasedSections.getLength();
    for (; pIter != pEnd; ++pIter)
    {
        sal_uInt16 nPos = sal_uInt16(-1);
        if ( (pIter->Value >>= nPos) && nPos < m_aSections.size() )
        {
            m_aSections[nPos]->setCollapsed(true);
        }
    }
}

void OViewsWindow::zoom(const Fraction& _aZoom)
{
    const MapMode& aMapMode = GetMapMode();

    Fraction aStartWidth(long(REPORT_STARTMARKER_WIDTH));
    if ( _aZoom < aMapMode.GetScaleX() )
        aStartWidth *= aMapMode.GetScaleX();
    else
        aStartWidth *= _aZoom;

    setZoomFactor(_aZoom,*this);

    for (const auto& rxSection : m_aSections)
    {
        rxSection->zoom(_aZoom);
    }

    Resize();

    Size aOut = GetOutputSizePixel();
    aOut.setWidth( long(aStartWidth) );
    aOut = PixelToLogic(aOut);

    tools::Rectangle aRect(PixelToLogic(Point(0,0)),aOut);
    Invalidate(aRect, InvalidateFlags::NoChildren);
}

void OViewsWindow::scrollChildren(const Point& _aThumbPos)
{
    const Point aPos(PixelToLogic(_aThumbPos));
    {
        MapMode aMapMode = GetMapMode();
        const Point aOld = aMapMode.GetOrigin();
        aMapMode.SetOrigin(m_pParent->GetMapMode().GetOrigin());

        const Point aPosY(m_pParent->PixelToLogic(_aThumbPos,aMapMode));

        aMapMode.SetOrigin( Point(aOld.X() , - aPosY.Y()));
        SetMapMode( aMapMode );
        Scroll(0, -( aOld.Y() + aPosY.Y()),ScrollFlags::Children);
    }

    for (const auto& rxSection : m_aSections)
    {
        rxSection->scrollChildren(aPos.X());
    }
}

void OViewsWindow::fillControlModelSelection(::std::vector< uno::Reference< uno::XInterface > >& _rSelection) const
{
    for (const auto& rxSection : m_aSections)
    {
        rxSection->getReportSection().fillControlModelSelection(_rSelection);
    }
}

} // rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
