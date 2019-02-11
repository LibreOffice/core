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
#include <memory>
#include <ReportSection.hxx>
#include <ReportWindow.hxx>
#include <DesignView.hxx>
#include <strings.hxx>
#include <RptObject.hxx>
#include <RptModel.hxx>
#include <SectionView.hxx>
#include <RptPage.hxx>
#include <ReportController.hxx>
#include <UITools.hxx>
#include <ViewsWindow.hxx>

#include <svx/svdpagv.hxx>
#include <editeng/adjustitem.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/unoshape.hxx>
#include <svx/gallery.hxx>
#include <svx/svxids.hrc>
#include <svx/svditer.hxx>
#include <svx/dbaexchange.hxx>

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <comphelper/propertyvalue.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <toolkit/helper/convert.hxx>
#include <RptDef.hxx>
#include <SectionWindow.hxx>
#include <helpids.h>
#include <dlgedclip.hxx>
#include <UndoActions.hxx>
#include <rptui_slotid.hrc>

#include <connectivity/dbtools.hxx>

#include <vcl/commandevent.hxx>
#include <vcl/lineinfo.hxx>
#include <ColorChanger.hxx>

#include <svl/itempool.hxx>
#include <svtools/extcolorcfg.hxx>
#include <unotools/confignode.hxx>


namespace rptui
{

using namespace ::com::sun::star;


static Color lcl_getOverlappedControlColor(/*const uno::Reference <lang::XMultiServiceFactory> _rxFactory*/)
{
    svtools::ExtendedColorConfig aConfig;
    return aConfig.GetColorValue(CFG_REPORTDESIGNER, DBOVERLAPPEDCONTROL).getColor();
}

OReportSection::OReportSection(OSectionWindow* _pParent,const uno::Reference< report::XSection >& _xSection)
    : Window(_pParent,WB_DIALOGCONTROL)
    , ::comphelper::OPropertyChangeListener(m_aMutex)
    , DropTargetHelper(this)
    , m_pPage(nullptr)
    , m_pView(nullptr)
    , m_pParent(_pParent)
    , m_xSection(_xSection)
    , m_nPaintEntranceCount(0)
    , m_eMode(DlgEdMode::Select)
{
    //EnableChildTransparentMode();
    SetHelpId(HID_REPORTSECTION);
    SetMapMode(MapMode(MapUnit::Map100thMM));
    SetParentClipMode(ParentClipMode::Clip);
    EnableChildTransparentMode( false );
    SetPaintTransparent( false );

    try
    {
        fill();
    }
    catch(uno::Exception&)
    {
        OSL_FAIL("Exception caught!");
    }

    m_pFunc.reset(new DlgEdFuncSelect( this ));
    m_pFunc->setOverlappedControlColor(lcl_getOverlappedControlColor() );
}

OReportSection::~OReportSection()
{
    disposeOnce();
}

void OReportSection::dispose()
{
    m_pPage = nullptr;
    if ( m_pMulti.is() )
        m_pMulti->dispose();

    if ( m_pReportListener.is() )
        m_pReportListener->dispose();
    m_pFunc.reset();

    {
        ::std::unique_ptr<OSectionView> aTemp( m_pView);
        if ( m_pView )
            m_pView->EndListening( *m_pModel );
        m_pView = nullptr;
    }
    m_pParent.clear();
    vcl::Window::dispose();
}

void OReportSection::Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect )
{
    Window::Paint(rRenderContext, rRect);

    if ( m_pView && m_nPaintEntranceCount == 0)
    {
        ++m_nPaintEntranceCount;
         // repaint, get PageView and prepare Region
        SdrPageView* pPgView = m_pView->GetSdrPageView();
        const vcl::Region aPaintRectRegion(rRect);

        // #i74769#
        SdrPaintWindow* pTargetPaintWindow = nullptr;

        // mark repaint start
        if (pPgView)
        {
            pTargetPaintWindow = pPgView->GetView().BeginDrawLayers(this, aPaintRectRegion);
            OSL_ENSURE(pTargetPaintWindow, "BeginDrawLayers: Got no SdrPaintWindow (!)");
            // draw background self using wallpaper
            OutputDevice& rTargetOutDev = pTargetPaintWindow->GetTargetOutputDevice();
            rTargetOutDev.DrawWallpaper(rRect, Wallpaper(pPgView->GetApplicationDocumentColor()));
        }

        // do paint (unbuffered) and mark repaint end
        if(pPgView)
        {
            pPgView->DrawLayer(RPT_LAYER_FRONT, &rRenderContext);
            pPgView->GetView().EndDrawLayers(*pTargetPaintWindow, true);
        }

        m_pView->CompleteRedraw(&rRenderContext, aPaintRectRegion);
        --m_nPaintEntranceCount;
    }
}

void OReportSection::fill()
{
    if ( !m_xSection.is() )
        return;

    m_pMulti = new comphelper::OPropertyChangeMultiplexer(this,m_xSection.get());
    m_pMulti->addProperty(PROPERTY_BACKCOLOR);

    m_pReportListener = addStyleListener(m_xSection->getReportDefinition(),this);

    m_pModel = m_pParent->getViewsWindow()->getView()->getReportView()->getController().getSdrModel();
    m_pPage = m_pModel->getPage(m_xSection);

    m_pView = new OSectionView(
        *m_pModel,
        this,
        m_pParent->getViewsWindow()->getView());

    // #i93597# tell SdrPage that only left and right page border is defined
    // instead of the full rectangle definition
    m_pPage->setPageBorderOnlyLeftRight(true);

    // without the following call, no grid is painted
    m_pView->ShowSdrPage( m_pPage );

    m_pView->SetMoveSnapOnlyTopLeft( true );
    ODesignView* pDesignView = m_pParent->getViewsWindow()->getView()->getReportView();

    // #i93595# Adapted grid to a more coarse grid and subdivisions for better visualisation. This
    // is only for visualisation and has nothing to do with the actual snap
    const Size aGridSizeCoarse(pDesignView->getGridSizeCoarse());
    const Size aGridSizeFine(pDesignView->getGridSizeFine());
    m_pView->SetGridCoarse(aGridSizeCoarse);
    m_pView->SetGridFine(aGridSizeFine);

    // #i93595# set snap grid width to snap to all existing subdivisions
    const Fraction aX(aGridSizeFine.Width());
    const Fraction aY(aGridSizeFine.Height());
    m_pView->SetSnapGridWidth(aX, aY);

    m_pView->SetGridSnap( true );
    m_pView->SetGridFront( false );
    m_pView->SetDragStripes( true );
    m_pView->SetPageVisible();
    sal_Int32 nColor = m_xSection->getBackColor();
    if ( nColor == static_cast<sal_Int32>(COL_TRANSPARENT) )
        nColor = getStyleProperty<sal_Int32>(m_xSection->getReportDefinition(),PROPERTY_BACKCOLOR);
    m_pView->SetApplicationDocumentColor(Color(nColor));

    uno::Reference<report::XReportDefinition> xReportDefinition = m_xSection->getReportDefinition();
    const sal_Int32 nLeftMargin = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_LEFTMARGIN);
    const sal_Int32 nRightMargin = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_RIGHTMARGIN);
    m_pPage->SetLeftBorder(nLeftMargin);
    m_pPage->SetRightBorder(nRightMargin);

// LLA: TODO
//  m_pPage->SetUpperBorder(-10000);

    m_pView->SetDesignMode();

    m_pPage->SetSize( Size( getStyleProperty<awt::Size>(xReportDefinition,PROPERTY_PAPERSIZE).Width,5*m_xSection->getHeight()) );
    const Size aPageSize = m_pPage->GetSize();
    m_pView->SetWorkArea( tools::Rectangle( Point( nLeftMargin, 0), Size(aPageSize.Width() - nLeftMargin - nRightMargin,aPageSize.Height()) ) );
}

void OReportSection::Paste(const uno::Sequence< beans::NamedValue >& _aAllreadyCopiedObjects,bool _bForce)
{
    OSL_ENSURE(m_xSection.is(),"Why is the section here NULL!");
    if ( m_xSection.is() && _aAllreadyCopiedObjects.getLength() )
    {
        // stop all drawing actions
        m_pView->BrkAction();

        // unmark all objects
        m_pView->UnmarkAll();
        const OUString sSectionName = m_xSection->getName();
        const sal_Int32 nLength = _aAllreadyCopiedObjects.getLength();
        const beans::NamedValue* pIter = _aAllreadyCopiedObjects.getConstArray();
        const beans::NamedValue* pEnd  = pIter + nLength;
        for(;pIter != pEnd;++pIter)
        {
            if ( _bForce || pIter->Name == sSectionName)
            {
                try
                {
                    uno::Sequence< uno::Reference<report::XReportComponent> > aCopies;
                    pIter->Value >>= aCopies;
                    const uno::Reference<report::XReportComponent>* pCopiesIter = aCopies.getConstArray();
                    const uno::Reference<report::XReportComponent>* pCopiesEnd = pCopiesIter + aCopies.getLength();
                    for (;pCopiesIter != pCopiesEnd ; ++pCopiesIter)
                    {
                        SvxShape* pShape = SvxShape::getImplementation( *pCopiesIter );
                        SdrObject* pObject = pShape ? pShape->GetSdrObject() : nullptr;
                        if ( pObject )
                        {
                            // Clone to target SdrModel
                            SdrObject* pNewObj(pObject->CloneSdrObject(*m_pModel.get()));
                            m_pPage->InsertObject(pNewObj, SAL_MAX_SIZE);
                            tools::Rectangle aRet(VCLPoint((*pCopiesIter)->getPosition()),VCLSize((*pCopiesIter)->getSize()));
                            aRet.setHeight(aRet.getHeight() + 1);
                            aRet.setWidth(aRet.getWidth() + 1);
                            bool bOverlapping = true;
                            while ( bOverlapping )
                            {
                                bOverlapping = isOver(aRet,*m_pPage,*m_pView,true,pNewObj) != nullptr;
                                if ( bOverlapping )
                                {
                                    aRet.Move(0,aRet.getHeight()+1);
                                    pNewObj->SetLogicRect(aRet);
                                }
                            }
                            m_pView->AddUndo( m_pView->GetModel()->GetSdrUndoFactory().CreateUndoNewObject( *pNewObj ) );
                            m_pView->MarkObj( pNewObj, m_pView->GetSdrPageView() );
                            if ( m_xSection.is() && (static_cast<sal_uInt32>(aRet.getHeight() + aRet.Top()) > m_xSection->getHeight()) )
                                m_xSection->setHeight(aRet.getHeight() + aRet.Top());
                        }
                    }
                }
                catch(uno::Exception&)
                {
                    OSL_FAIL("Exception caught while pasting a new object!");
                }
                if ( !_bForce )
                    break;
            }
        }
    }
}

void OReportSection::Delete()
{
    if( !m_pView->AreObjectsMarked() )
        return;

    m_pView->BrkAction();
    m_pView->DeleteMarked();
}

void OReportSection::SetMode( DlgEdMode eNewMode )
{
    if ( eNewMode != m_eMode )
    {
        if ( eNewMode == DlgEdMode::Insert )
        {
            m_pFunc.reset(new DlgEdFuncInsert( this ));
        }
        else
        {
            m_pFunc.reset(new DlgEdFuncSelect( this ));
        }
        m_pFunc->setOverlappedControlColor(lcl_getOverlappedControlColor( ) );
        m_pModel->SetReadOnly(false);
        m_eMode = eNewMode;
    }
}

void OReportSection::Copy(uno::Sequence< beans::NamedValue >& _rAllreadyCopiedObjects)
{
    Copy(_rAllreadyCopiedObjects,false);
}

void OReportSection::Copy(uno::Sequence< beans::NamedValue >& _rAllreadyCopiedObjects,bool _bEraseAnddNoClone)
{
    OSL_ENSURE(m_xSection.is(),"Why is the section here NULL!");
    if( !m_pView->AreObjectsMarked() || !m_xSection.is() )
        return;

    // insert control models of marked objects into clipboard dialog model
    const SdrMarkList& rMarkedList = m_pView->GetMarkedObjectList();
    const size_t nMark = rMarkedList.GetMarkCount();

    ::std::vector< uno::Reference<report::XReportComponent> > aCopies;
    aCopies.reserve(nMark);

    SdrUndoFactory& rUndo = m_pView->GetModel()->GetSdrUndoFactory();

    for( size_t i = nMark; i > 0; )
    {
        --i;
        SdrObject* pSdrObject = rMarkedList.GetMark(i)->GetMarkedSdrObj();
        OObjectBase* pObj = dynamic_cast<OObjectBase*>(pSdrObject);
        if ( pObj  )
        {
            try
            {
                SdrObject* pNewObj(pSdrObject->CloneSdrObject(pSdrObject->getSdrModelFromSdrObject()));
                aCopies.emplace_back(pNewObj->getUnoShape(),uno::UNO_QUERY);
                if ( _bEraseAnddNoClone )
                {
                    m_pView->AddUndo( rUndo.CreateUndoDeleteObject( *pSdrObject ) );
                    m_pPage->RemoveObject(pSdrObject->GetOrdNum());
                }

            }
            catch(uno::Exception&)
            {
                OSL_FAIL("Can't copy report elements!");
            }
        }
    }

    if ( !aCopies.empty() )
    {
        ::std::reverse(aCopies.begin(),aCopies.end());
        const sal_Int32 nLength = _rAllreadyCopiedObjects.getLength();
        _rAllreadyCopiedObjects.realloc( nLength + 1);
        beans::NamedValue* pNewValue = _rAllreadyCopiedObjects.getArray() + nLength;
        pNewValue->Name = m_xSection->getName();
        pNewValue->Value <<= uno::Sequence< uno::Reference<report::XReportComponent> >(&(*aCopies.begin()),aCopies.size());
    }
}

void OReportSection::MouseButtonDown( const MouseEvent& rMEvt )
{
    m_pParent->getViewsWindow()->getView()->setMarked(m_pView, true); // mark the section in which is clicked
    m_pFunc->MouseButtonDown( rMEvt );
    Window::MouseButtonDown(rMEvt);
}

void OReportSection::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( !m_pFunc->MouseButtonUp( rMEvt ) )
        m_pParent->getViewsWindow()->getView()->getReportView()->getController().executeUnChecked(SID_OBJECT_SELECT,uno::Sequence< beans::PropertyValue>());
}


void OReportSection::MouseMove( const MouseEvent& rMEvt )
{
    m_pFunc->MouseMove( rMEvt );

}

void OReportSection::SetGridVisible(bool _bVisible)
{
    m_pView->SetGridVisible( _bVisible );
}

void OReportSection::SelectAll(const sal_uInt16 _nObjectType)
{
    if ( m_pView )
    {
        if ( _nObjectType == OBJ_NONE )
            m_pView->MarkAllObj();
        else
        {
            m_pView->UnmarkAll();
            SdrObjListIter aIter(m_pPage,SdrIterMode::DeepNoGroups);
            SdrObject* pObjIter = nullptr;
            while( (pObjIter = aIter.Next()) != nullptr )
            {
                if ( pObjIter->GetObjIdentifier() == _nObjectType )
                    m_pView->MarkObj( pObjIter, m_pView->GetSdrPageView() );
            }
        }
    }
}

void OReportSection::Command( const CommandEvent& _rCEvt )
{
    Window::Command(_rCEvt);
    if (_rCEvt.GetCommand() == CommandEventId::ContextMenu)
    {
        OReportController& rController = m_pParent->getViewsWindow()->getView()->getReportView()->getController();
        uno::Reference<frame::XFrame> xFrame = rController.getFrame();
        css::uno::Sequence<css::uno::Any> aArgs {
            css::uno::makeAny(comphelper::makePropertyValue("Value", OUString("report"))),
            css::uno::makeAny(comphelper::makePropertyValue("Frame", xFrame)),
            css::uno::makeAny(comphelper::makePropertyValue("IsContextMenu", true))
        };

        css::uno::Reference<css::uno::XComponentContext> xContext(rController.getORB());
        css::uno::Reference<css::frame::XPopupMenuController> xMenuController(
            xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            "com.sun.star.comp.framework.ResourceMenuController", aArgs, xContext), css::uno::UNO_QUERY);

        if (!xMenuController.is())
            return;

        rtl::Reference<VCLXPopupMenu> xPopupMenu(new VCLXPopupMenu);
        xMenuController->setPopupMenu(xPopupMenu.get());

        Point aPos = _rCEvt.GetMousePosPixel();
        m_pView->EndAction();
        static_cast<PopupMenu*>(xPopupMenu->GetMenu())->Execute(this, aPos);

        css::uno::Reference<css::lang::XComponent> xComponent(xMenuController, css::uno::UNO_QUERY);
        xComponent->dispose();
    }
}

void OReportSection::_propertyChanged(const beans::PropertyChangeEvent& _rEvent)
{
    if ( m_xSection.is() )
    {
        if ( _rEvent.Source == m_xSection || PROPERTY_BACKCOLOR == _rEvent.PropertyName )
        {
            sal_Int32 nColor = m_xSection->getBackColor();
            if ( nColor == static_cast<sal_Int32>(COL_TRANSPARENT) )
                nColor = getStyleProperty<sal_Int32>(m_xSection->getReportDefinition(),PROPERTY_BACKCOLOR);
            m_pView->SetApplicationDocumentColor(Color(nColor));
            Invalidate(InvalidateFlags::NoChildren|InvalidateFlags::NoErase);
        }
        else
        {
            uno::Reference<report::XReportDefinition> xReportDefinition = m_xSection->getReportDefinition();
            const sal_Int32 nLeftMargin  = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_LEFTMARGIN);
            const sal_Int32 nRightMargin = getStyleProperty<sal_Int32>(xReportDefinition,PROPERTY_RIGHTMARGIN);
            const sal_Int32 nPaperWidth  = getStyleProperty<awt::Size>(xReportDefinition,PROPERTY_PAPERSIZE).Width;

            if ( _rEvent.PropertyName == PROPERTY_LEFTMARGIN )
            {
                m_pPage->SetLeftBorder(nLeftMargin);
            }
            else if ( _rEvent.PropertyName == PROPERTY_RIGHTMARGIN )
            {
                m_pPage->SetRightBorder(nRightMargin);
            }
            const Size aOldPageSize = m_pPage->GetSize();
            sal_Int32 nNewHeight = 5*m_xSection->getHeight();
            if ( aOldPageSize.Height() != nNewHeight || nPaperWidth != aOldPageSize.Width() )
            {
                m_pPage->SetSize( Size( nPaperWidth,nNewHeight) );
                const Size aPageSize = m_pPage->GetSize();
                m_pView->SetWorkArea( tools::Rectangle( Point( nLeftMargin, 0), Size(aPageSize.Width() - nLeftMargin - nRightMargin,aPageSize.Height()) ) );
            }
            impl_adjustObjectSizePosition(nPaperWidth,nLeftMargin,nRightMargin);
            m_pParent->Invalidate(InvalidateFlags::Update | InvalidateFlags::Transparent);
        }
    }
}
void OReportSection::impl_adjustObjectSizePosition(sal_Int32 i_nPaperWidth,sal_Int32 i_nLeftMargin,sal_Int32 i_nRightMargin)
{
    try
    {
        sal_Int32 nRightBorder = i_nPaperWidth - i_nRightMargin;
        const sal_Int32 nCount = m_xSection->getCount();
        for (sal_Int32 i = 0; i < nCount; ++i)
        {
            uno::Reference< report::XReportComponent> xReportComponent(m_xSection->getByIndex(i),uno::UNO_QUERY_THROW);
            awt::Point aPos = xReportComponent->getPosition();
            awt::Size aSize = xReportComponent->getSize();
            SvxShape* pShape = SvxShape::getImplementation( xReportComponent );
            SdrObject* pObject = pShape ? pShape->GetSdrObject() : nullptr;
            if ( pObject )
            {
                bool bChanged = false;

                OObjectBase& rBase = dynamic_cast<OObjectBase&>(*pObject);
                rBase.EndListening();
                if ( aPos.X < i_nLeftMargin )
                {
                    aPos.X  = i_nLeftMargin;
                    bChanged = true;
                }
                if ( (aPos.X + aSize.Width) > nRightBorder )
                {
                    aPos.X = nRightBorder - aSize.Width;
                    if ( aPos.X < i_nLeftMargin )
                    {
                        aSize.Width += aPos.X - i_nLeftMargin;
                        aPos.X = i_nLeftMargin;
                        // add listener around
                        rBase.StartListening();
                        xReportComponent->setSize(aSize);
                        rBase.EndListening();
                    }
                    bChanged = true;
                }
                if ( aPos.Y < 0 )
                    aPos.Y = 0;
                if ( bChanged )
                {
                    xReportComponent->setPosition(aPos);
                    correctOverlapping(pObject,*this,false);
                    tools::Rectangle aRet(VCLPoint(xReportComponent->getPosition()),VCLSize(xReportComponent->getSize()));
                    aRet.setHeight(aRet.getHeight() + 1);
                    aRet.setWidth(aRet.getWidth() + 1);
                    if ( m_xSection.is() && (static_cast<sal_uInt32>(aRet.getHeight() + aRet.Top()) > m_xSection->getHeight()) )
                        m_xSection->setHeight(aRet.getHeight() + aRet.Top());

                    pObject->RecalcBoundRect();
                }
                rBase.StartListening();
            }
        }
    }
    catch(const uno::Exception &)
    {
        OSL_FAIL("Exception caught: OReportSection::impl_adjustObjectSizePosition()");
    }
}

bool OReportSection::handleKeyEvent(const KeyEvent& _rEvent)
{
    return m_pFunc.get() && m_pFunc->handleKeyEvent(_rEvent);
}

void OReportSection::deactivateOle()
{
    if (m_pFunc)
        m_pFunc->deactivateOle(true);
}

void OReportSection::createDefault(const OUString& _sType)
{
    SdrObject* pObj = m_pView->GetCreateObj();
    if ( !pObj )
        return;
    createDefault(_sType,pObj);
}

void OReportSection::createDefault(const OUString& _sType,SdrObject* _pObj)
{
    bool bAttributesAppliedFromGallery = false;

    if ( GalleryExplorer::GetSdrObjCount( GALLERY_THEME_POWERPOINT ) )
    {
        std::vector< OUString > aObjList;
        if ( GalleryExplorer::FillObjListTitle( GALLERY_THEME_POWERPOINT, aObjList ) )
        {
            auto aIter = std::find_if(aObjList.begin(), aObjList.end(),
                [&_sType](const OUString& rObj) { return rObj.equalsIgnoreAsciiCase(_sType); });
            if (aIter != aObjList.end())
            {
                auto i = static_cast<sal_uInt32>(std::distance(aObjList.begin(), aIter));
                OReportModel aReportModel(nullptr);
                SfxItemPool& rPool = aReportModel.GetItemPool();
                rPool.FreezeIdRanges();
                if ( GalleryExplorer::GetSdrObj( GALLERY_THEME_POWERPOINT, i, &aReportModel ) )
                {
                    const SdrObject* pSourceObj = aReportModel.GetPage( 0 )->GetObj( 0 );
                    if( pSourceObj )
                    {
                        const SfxItemSet& rSource = pSourceObj->GetMergedItemSet();
                        SfxItemSet aDest(
                            _pObj->getSdrModelFromSdrObject().GetItemPool(),
                            svl::Items<
                                // Ranges from SdrAttrObj:
                                SDRATTR_START, SDRATTR_SHADOW_LAST,
                                SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                                SDRATTR_TEXTDIRECTION,
                                    SDRATTR_TEXTDIRECTION,
                                // Graphic attributes, 3D properties,
                                // CustomShape properties:
                                SDRATTR_GRAF_FIRST,
                                    SDRATTR_CUSTOMSHAPE_LAST,
                                // Range from SdrTextObj:
                                EE_ITEMS_START, EE_ITEMS_END>{});
                        aDest.Set( rSource );
                        _pObj->SetMergedItemSet( aDest );
                        sal_Int32 nAngle = pSourceObj->GetRotateAngle();
                        if ( nAngle )
                        {
                            double a = nAngle * F_PI18000;
                            _pObj->NbcRotate( _pObj->GetSnapRect().Center(), nAngle, sin( a ), cos( a ) );
                        }
                        bAttributesAppliedFromGallery = true;
                    }
                }
            }
        }
    }
    if ( !bAttributesAppliedFromGallery )
    {
        _pObj->SetMergedItem( SvxAdjustItem( SvxAdjust::Center, EE_PARA_JUST) );
        _pObj->SetMergedItem( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_CENTER ) );
        _pObj->SetMergedItem( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_BLOCK ) );
        _pObj->SetMergedItem( makeSdrTextAutoGrowHeightItem( false ) );
        static_cast<SdrObjCustomShape*>(_pObj)->MergeDefaultAttributes( &_sType );
    }
}

uno::Reference< report::XReportComponent > OReportSection::getCurrentControlModel() const
{
    uno::Reference< report::XReportComponent > xModel;
    if ( m_pView )
    {
        const SdrMarkList& rMarkList = m_pView->GetMarkedObjectList();

        if ( rMarkList.GetMarkCount() == 1 )
        {
            SdrObject* pDlgEdObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            OObjectBase* pObj = dynamic_cast<OObjectBase*>(pDlgEdObj);
            if ( pObj )
                xModel = pObj->getReportComponent().get();
        }
    }
    return xModel;
}

void OReportSection::fillControlModelSelection(::std::vector< uno::Reference< uno::XInterface > >& _rSelection) const
{
    if ( m_pView )
    {
        const SdrMarkList& rMarkList = m_pView->GetMarkedObjectList();
        const size_t nMarkCount = rMarkList.GetMarkCount();

        for (size_t i=0; i < nMarkCount; ++i)
        {
            const SdrObject* pDlgEdObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
            const OObjectBase* pObj = dynamic_cast<const OObjectBase*>(pDlgEdObj);
            if ( pObj )
            {
                uno::Reference<uno::XInterface> xInterface(pObj->getReportComponent());
                _rSelection.push_back(xInterface);
            }
        }
    }
}

sal_Int8 OReportSection::AcceptDrop( const AcceptDropEvent& _rEvt )
{
    ::Point aDropPos(_rEvt.maPosPixel);
    const MouseEvent aMouseEvt(aDropPos);
    if ( m_pFunc->isOverlapping(aMouseEvt) )
        return DND_ACTION_NONE;

    if ( _rEvt.mnAction == DND_ACTION_COPY ||
         _rEvt.mnAction == DND_ACTION_LINK
         )
    {
        if (!m_pParent) return DND_ACTION_NONE;
        sal_uInt16 nCurrentPosition = 0;
        nCurrentPosition = m_pParent->getViewsWindow()->getPosition(m_pParent);
        if (_rEvt.mnAction == DND_ACTION_COPY )
        {
            // we must assure, we can't drop in the top section
            if (nCurrentPosition < 1)
            {
                return DND_ACTION_NONE;
            }
            return DND_ACTION_LINK;
        }
        if (_rEvt.mnAction == DND_ACTION_LINK)
        {
            // we must assure, we can't drop in the bottom section
            if (m_pParent->getViewsWindow()->getSectionCount() > (nCurrentPosition + 1)  )
            {
                return DND_ACTION_COPY;
            }
            return DND_ACTION_NONE;
        }
    }
    else
    {
        const DataFlavorExVector& rFlavors = GetDataFlavorExVector();
        if (   svx::OMultiColumnTransferable::canExtractDescriptor(rFlavors)
            || svx::OColumnTransferable::canExtractColumnDescriptor(rFlavors, ColumnTransferFormatFlags::FIELD_DESCRIPTOR | ColumnTransferFormatFlags::CONTROL_EXCHANGE | ColumnTransferFormatFlags::COLUMN_DESCRIPTOR) )
            return _rEvt.mnAction;

        const sal_Int8 nDropOption = ( OReportExchange::canExtract(rFlavors) ) ? DND_ACTION_COPYMOVE : DND_ACTION_NONE;

        return nDropOption;
    }
    return DND_ACTION_NONE;
}


sal_Int8 OReportSection::ExecuteDrop( const ExecuteDropEvent& _rEvt )
{
    ::Point aDropPos(PixelToLogic(_rEvt.maPosPixel));
    const MouseEvent aMouseEvt(aDropPos);
    if ( m_pFunc->isOverlapping(aMouseEvt) )
        return DND_ACTION_NONE;

    sal_Int8 nDropOption = DND_ACTION_NONE;
    const TransferableDataHelper aDropped(_rEvt.maDropEvent.Transferable);
    const DataFlavorExVector& rFlavors = aDropped.GetDataFlavorExVector();
    bool bMultipleFormat = svx::OMultiColumnTransferable::canExtractDescriptor(rFlavors);
    if ( OReportExchange::canExtract(rFlavors) )
    {
        OReportExchange::TSectionElements aCopies = OReportExchange::extractCopies(aDropped);
        Paste(aCopies,true);
        nDropOption = DND_ACTION_COPYMOVE;
        m_pParent->getViewsWindow()->BrkAction();
        m_pParent->getViewsWindow()->unmarkAllObjects(m_pView);
    }
    else if ( bMultipleFormat
        || svx::OColumnTransferable::canExtractColumnDescriptor(rFlavors, ColumnTransferFormatFlags::FIELD_DESCRIPTOR | ColumnTransferFormatFlags::CONTROL_EXCHANGE | ColumnTransferFormatFlags::COLUMN_DESCRIPTOR) )
    {
        m_pParent->getViewsWindow()->getView()->setMarked(m_pView, true);
        m_pView->UnmarkAll();
        const tools::Rectangle& rRect = m_pView->GetWorkArea();
        if ( aDropPos.X() < rRect.Left() )
            aDropPos.setX( rRect.Left() );
        else if ( aDropPos.X() > rRect.Right() )
            aDropPos.setX( rRect.Right() );

        if ( aDropPos.Y() > rRect.Bottom() )
            aDropPos.setY( rRect.Bottom() );

        uno::Sequence<beans::PropertyValue> aValues;
        if ( !bMultipleFormat )
        {
            svx::ODataAccessDescriptor aDescriptor = svx::OColumnTransferable::extractColumnDescriptor(aDropped);

            aValues.realloc(1);
            aValues[0].Value <<= aDescriptor.createPropertyValueSequence();
        }
        else
            aValues = svx::OMultiColumnTransferable::extractDescriptor(aDropped);

        for(beans::PropertyValue & propVal : aValues)
        {
            uno::Sequence<beans::PropertyValue> aCurrent;
            propVal.Value >>= aCurrent;
            sal_Int32 nLength = aCurrent.getLength();
            if ( nLength )
            {
                aCurrent.realloc(nLength + 3);
                aCurrent[nLength].Name = PROPERTY_POSITION;
                aCurrent[nLength++].Value <<= AWTPoint(aDropPos);
                // give also the DND Action (Shift|Ctrl) Key to really say what we want
                aCurrent[nLength].Name = "DNDAction";
                aCurrent[nLength++].Value <<= _rEvt.mnAction;

                aCurrent[nLength].Name = "Section";
                aCurrent[nLength++].Value <<= getSection();
                propVal.Value <<= aCurrent;
            }
        }

        // we use this way to create undo actions
        OReportController& rController = m_pParent->getViewsWindow()->getView()->getReportView()->getController();
        rController.executeChecked(SID_ADD_CONTROL_PAIR,aValues);
        nDropOption = DND_ACTION_COPY;
    }
    return nDropOption;
}

void OReportSection::stopScrollTimer()
{
    m_pFunc->stopScrollTimer();
}

bool OReportSection::isUiActive() const
{
    return m_pFunc->isUiActive();
}


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
