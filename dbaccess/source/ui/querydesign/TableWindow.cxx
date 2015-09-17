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

#include "TableWindow.hxx"
#include "TableWindowListBox.hxx"
#include "QueryTableView.hxx"
#include "QueryDesignView.hxx"
#include "TableWindowData.hxx"
#include "imageprovider.hxx"
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <vcl/svapp.hxx>
#include <vcl/wall.hxx>
#include <vcl/settings.hxx>

#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include "querycontroller.hxx"
#include "dbu_qry.hrc"
#include "dbustrings.hrc"
#include "Query.hrc"
#include <comphelper/extract.hxx>
#include "UITools.hxx"
#include "TableWindowAccess.hxx"
#include "browserids.hxx"
#include <connectivity/dbtools.hxx>
#include "svtools/treelistentry.hxx"

using namespace dbaui;
using namespace ::utl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;

namespace DatabaseObject = css::sdb::application::DatabaseObject;

#define TABWIN_SIZING_AREA      4
#define TABWIN_WIDTH_MIN    90
#define TABWIN_HEIGHT_MIN   80

namespace {

void Draw3DBorder(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    // Use the System Style-Settings for my colours
    const StyleSettings& aSystemStyle = Application::GetSettings().GetStyleSettings();

    // Black lines for bottom and right
    rRenderContext.SetLineColor(aSystemStyle.GetDarkShadowColor());
    rRenderContext.DrawLine(rRect.BottomLeft(), rRect.BottomRight());
    rRenderContext.DrawLine(rRect.BottomRight(), rRect.TopRight());

    // Dark grey lines over the black lines
    rRenderContext.SetLineColor(aSystemStyle.GetShadowColor());
    Point aEHvector(1, 1);
    rRenderContext.DrawLine(rRect.BottomLeft() + Point(1, -1), rRect.BottomRight() - aEHvector);
    rRenderContext.DrawLine(rRect.BottomRight() - aEHvector, rRect.TopRight() + Point(-1, 1));

    // Light grey lines for top and left
    rRenderContext.SetLineColor(aSystemStyle.GetLightColor());
    rRenderContext.DrawLine(rRect.BottomLeft() + Point(1, -2), rRect.TopLeft() + aEHvector);
    rRenderContext.DrawLine(rRect.TopLeft() + aEHvector, rRect.TopRight() + Point(-2, 1));
}

}

// class OTableWindow
OTableWindow::OTableWindow( vcl::Window* pParent, const TTableWindowData::value_type& pTabWinData )
          : ::comphelper::OContainerListener(m_aMutex)
          ,Window( pParent, WB_3DLOOK|WB_MOVEABLE )
          ,m_aTypeImage( VclPtr<FixedImage>::Create(this) )
          ,m_xTitle( VclPtr<OTableWindowTitle>::Create(this) )
          ,m_pAccessible(NULL)
          ,m_pData( pTabWinData )
          ,m_nMoveCount(0)
          ,m_nMoveIncrement(1)
          ,m_nSizingFlags( SIZING_NONE )
          ,m_bActive( false )
{

    // Set position and size
    if( GetData()->HasPosition() )
        SetPosPixel( GetData()->GetPosition() );

    if( GetData()->HasSize() )
        SetSizePixel( GetData()->GetSize() );

    // Set background
    const StyleSettings&  aSystemStyle = Application::GetSettings().GetStyleSettings();
    SetBackground(Wallpaper(aSystemStyle.GetFaceColor()));
    // Set the text colour even though there is no text,
    // because derived classes might need it
    SetTextColor(aSystemStyle.GetButtonTextColor());

    EnableClipSiblings();
}

OTableWindow::~OTableWindow()
{
    disposeOnce();
}

void OTableWindow::dispose()
{
    if (m_xListBox)
    {
        OSL_ENSURE(m_xListBox->GetEntryCount()==0,"Forgot to call EmptyListbox()!");
    }
    m_xListBox.disposeAndClear();
    if ( m_pContainerListener.is() )
        m_pContainerListener->dispose();

    m_pAccessible = NULL;
    m_aTypeImage.disposeAndClear();
    m_xTitle.disposeAndClear();
    vcl::Window::dispose();
}

const OJoinTableView* OTableWindow::getTableView() const
{
    OSL_ENSURE(static_cast<OJoinTableView*>(GetParent()),"No OJoinTableView!");
    return static_cast<OJoinTableView*>(GetParent());
}

OJoinTableView* OTableWindow::getTableView()
{
    OSL_ENSURE(static_cast<OJoinTableView*>(GetParent()),"No OJoinTableView!");
    return static_cast<OJoinTableView*>(GetParent());
}

OJoinDesignView* OTableWindow::getDesignView()
{
    OSL_ENSURE(static_cast<OJoinDesignView*>(GetParent()->GetParent()->GetParent()),"No OJoinDesignView!");
    return static_cast<OJoinDesignView*>(GetParent()->GetParent()->GetParent());
}

void OTableWindow::SetPosPixel( const Point& rNewPos )
{
    Point aNewPosData = rNewPos + getTableView()->GetScrollOffset();
    GetData()->SetPosition( aNewPosData );
    Window::SetPosPixel( rNewPos );
}

void OTableWindow::SetSizePixel( const Size& rNewSize )
{
    Size aOutSize(rNewSize);
    if( aOutSize.Width() < TABWIN_WIDTH_MIN )
        aOutSize.Width() = TABWIN_WIDTH_MIN;
    if( aOutSize.Height() < TABWIN_HEIGHT_MIN )
        aOutSize.Height() = TABWIN_HEIGHT_MIN;

    GetData()->SetSize( aOutSize );
    Window::SetSizePixel( aOutSize );
}

void OTableWindow::SetPosSizePixel( const Point& rNewPos, const Size& rNewSize )
{
    SetPosPixel( rNewPos );
    SetSizePixel( rNewSize );
}

VclPtr<OTableWindowListBox> OTableWindow::CreateListBox()
{
    return VclPtr<OTableWindowListBox>::Create(this);
}

bool OTableWindow::FillListBox()
{
    m_xListBox->Clear();
    if ( !m_pContainerListener.is() )
    {
        Reference< XContainer> xContainer(m_pData->getColumns(),UNO_QUERY);
        if ( xContainer.is() )
            m_pContainerListener = new ::comphelper::OContainerListenerAdapter(this,xContainer);
    }
    // mark all primary keys with special image
    ModuleRes TmpRes(IMG_JOINS);
    ImageList aImageList(TmpRes);
    Image aPrimKeyImage = aImageList.GetImage(IMG_PRIMARY_KEY);

    if (GetData()->IsShowAll())
    {
        SvTreeListEntry* pEntry = m_xListBox->InsertEntry( OUString("*") );
        pEntry->SetUserData( createUserData(NULL,false) );
    }

    Reference<XNameAccess> xPKeyColumns;
    try
    {
        xPKeyColumns = dbtools::getPrimaryKeyColumns_throw(m_pData->getTable());
    }
    catch(Exception&)
    {
        OSL_FAIL("Exception occurred!");
    }
    try
    {
        Reference< XNameAccess > xColumns = m_pData->getColumns();
        if( xColumns.is() )
        {
            Sequence< OUString> aColumns = xColumns->getElementNames();
            const OUString* pIter = aColumns.getConstArray();
            const OUString* pEnd = pIter + aColumns.getLength();

            SvTreeListEntry* pEntry = NULL;
            for (; pIter != pEnd; ++pIter)
            {
                bool bPrimaryKeyColumn = xPKeyColumns.is() && xPKeyColumns->hasByName( *pIter );
                // is this column in the primary key
                if ( bPrimaryKeyColumn )
                    pEntry = m_xListBox->InsertEntry(*pIter, aPrimKeyImage, aPrimKeyImage);
                else
                    pEntry = m_xListBox->InsertEntry(*pIter);

                Reference<XPropertySet> xColumn(xColumns->getByName(*pIter),UNO_QUERY);
                if ( xColumn.is() )
                    pEntry->SetUserData( createUserData(xColumn,bPrimaryKeyColumn) );
            }
        }
    }
    catch(Exception&)
    {
        OSL_FAIL("Exception occurred!");
    }

    return true;
}

void* OTableWindow::createUserData(const Reference< XPropertySet>& /*_xColumn*/,bool /*_bPrimaryKey*/)
{
    return NULL;
}

void OTableWindow::deleteUserData(void*& _pUserData)
{
    OSL_ENSURE(!_pUserData,"INVALID call. Need to delete the userclass!");
    _pUserData = NULL;
}

void OTableWindow::clearListBox()
{
    if ( m_xListBox )
    {
        SvTreeListEntry* pEntry = m_xListBox->First();

        while(pEntry)
        {
            void* pUserData = pEntry->GetUserData();
            deleteUserData(pUserData);
            SvTreeListEntry* pNextEntry = m_xListBox->Next(pEntry);
            m_xListBox->GetModel()->Remove(pEntry);
            pEntry = pNextEntry;
        }
    }
}

void OTableWindow::impl_updateImage()
{
    ImageProvider aImageProvider( getDesignView()->getController().getConnection() );

    Image aImage;
    aImageProvider.getImages( GetComposedName(), m_pData->isQuery() ? DatabaseObject::QUERY : DatabaseObject::TABLE, aImage );

    if ( !aImage )
    {
        OSL_FAIL( "OTableWindow::impl_updateImage: no images!" );
        return;
    }

    m_aTypeImage->SetModeImage( aImage );
    m_aTypeImage->Show();
}

bool OTableWindow::Init()
{
    // create list box if necessary
    if ( !m_xListBox )
    {
        m_xListBox = CreateListBox();
        OSL_ENSURE( m_xListBox != nullptr, "OTableWindow::Init() : CreateListBox returned NULL !" );
        m_xListBox->SetSelectionMode( MULTIPLE_SELECTION );
    }

    // Set the title
    m_xTitle->SetText( m_pData->GetWinName() );
    m_xTitle->Show();

    m_xListBox->Show();

    // add the fields to the ListBox
    clearListBox();
    bool bSuccess = FillListBox();
    if ( bSuccess )
        m_xListBox->SelectAll( false );

    impl_updateImage();

    return bSuccess;
}

void OTableWindow::DataChanged(const DataChangedEvent& rDCEvt)
{
    if (rDCEvt.GetType() == DataChangedEventType::SETTINGS)
    {
        // In the worst-case the colours have changed so
        // adapt myself to the new colours
        const StyleSettings&  aSystemStyle = Application::GetSettings().GetStyleSettings();
        SetBackground(Wallpaper(Color(aSystemStyle.GetFaceColor())));
        SetTextColor(aSystemStyle.GetButtonTextColor());
    }
}

void OTableWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    Rectangle aRect(Point(0,0), GetOutputSizePixel());
    Window::Paint(rRenderContext, rRect);
    Draw3DBorder(rRenderContext, aRect);
}

Rectangle OTableWindow::getSizingRect(const Point& _rPos,const Size& _rOutputSize) const
{
    Rectangle aSizingRect = Rectangle( GetPosPixel(), GetSizePixel() );
    sal_uInt16 nSizingFlags = GetSizingFlags();

    if( nSizingFlags & SIZING_TOP )
    {
        if( _rPos.Y() < 0 )
            aSizingRect.Top() = 0;
        else
            aSizingRect.Top() = _rPos.Y();
    }

    if( nSizingFlags & SIZING_BOTTOM )
    {
        if( _rPos.Y() > _rOutputSize.Height() )
            aSizingRect.Bottom() = _rOutputSize.Height();
        else
            aSizingRect.Bottom() = _rPos.Y();
    }

    if( nSizingFlags & SIZING_RIGHT )
    {
        if( _rPos.X() > _rOutputSize.Width() )
            aSizingRect.Right() = _rOutputSize.Width();
        else
            aSizingRect.Right() = _rPos.X();
    }

    if( nSizingFlags & SIZING_LEFT )
    {
        if( _rPos.X() < 0 )
            aSizingRect.Left() = 0;
        else
            aSizingRect.Left() = _rPos.X();
    }
    return aSizingRect;
}

void OTableWindow::setSizingFlag(const Point& _rPos)
{
    Size    aOutSize = GetOutputSizePixel();
    // Set the flags when the mouse cursor is in the sizing area
    m_nSizingFlags = SIZING_NONE;

    if( _rPos.X() < TABWIN_SIZING_AREA )
        m_nSizingFlags |= SIZING_LEFT;

    if( _rPos.Y() < TABWIN_SIZING_AREA )
        m_nSizingFlags |= SIZING_TOP;

    if( _rPos.X() > aOutSize.Width()-TABWIN_SIZING_AREA )
        m_nSizingFlags |= SIZING_RIGHT;

    if( _rPos.Y() > aOutSize.Height()-TABWIN_SIZING_AREA )
        m_nSizingFlags |= SIZING_BOTTOM;
}

void OTableWindow::MouseMove( const MouseEvent& rEvt )
{
    Window::MouseMove(rEvt);

    OJoinTableView* pCont = getTableView();
    if (pCont->getDesignView()->getController().isReadOnly())
        return;

    Point   aPos = rEvt.GetPosPixel();
    setSizingFlag(aPos);
    Pointer aPointer;

    // Set the mouse cursor when it is in the sizing area
    switch( m_nSizingFlags )
    {
    case SIZING_TOP:
    case SIZING_BOTTOM:
        aPointer = Pointer( PointerStyle::SSize );
        break;

    case SIZING_LEFT:
    case SIZING_RIGHT:
        aPointer = Pointer( PointerStyle::ESize );
        break;

    case SIZING_LEFT+SIZING_TOP:
    case SIZING_RIGHT+SIZING_BOTTOM:
        aPointer = Pointer( PointerStyle::SESize );
        break;

    case SIZING_RIGHT+SIZING_TOP:
    case SIZING_LEFT+SIZING_BOTTOM:
        aPointer = Pointer( PointerStyle::NESize );
        break;
    }

    SetPointer( aPointer );
}

void OTableWindow::MouseButtonDown( const MouseEvent& rEvt )
{
    // When resizing, the parent must be informed that
    // the window size of its child has changed
    if( m_nSizingFlags )
        getTableView()->BeginChildSizing( this, GetPointer() );

    Window::MouseButtonDown( rEvt );
}

void OTableWindow::Resize()
{
    // The window must not disappear so we enforce a minimum size
    Size    aOutSize = GetOutputSizePixel();
    aOutSize = Size(CalcZoom(aOutSize.Width()),CalcZoom(aOutSize.Height()));

    long nTitleHeight = CalcZoom( GetTextHeight() )+ CalcZoom( 4 );

    // Set the title and ListBox
    long n5Pos = CalcZoom(5);
    long nPositionX = n5Pos;
    long nPositionY = n5Pos;

    // position the image which indicates the type
    m_aTypeImage->SetPosPixel( Point( nPositionX, nPositionY ) );
    Size aImageSize( m_aTypeImage->GetImage().GetSizePixel() );
    m_aTypeImage->SetSizePixel( aImageSize );

    if ( nTitleHeight < aImageSize.Height() )
        nTitleHeight = aImageSize.Height();

    nPositionX += aImageSize.Width() + CalcZoom( 2 );
    m_xTitle->SetPosSizePixel( Point( nPositionX, nPositionY ), Size( aOutSize.Width() - nPositionX - n5Pos, nTitleHeight ) );

    long nTitleToList = CalcZoom( 3 );

    m_xListBox->SetPosSizePixel(
        Point( n5Pos, nPositionY + nTitleHeight + nTitleToList ),
        Size( aOutSize.Width() - 2 * n5Pos, aOutSize.Height() - ( nPositionY + nTitleHeight + nTitleToList ) - n5Pos )
    );

    Window::Invalidate();
}

void OTableWindow::SetBoldTitle( bool bBold )
{
    vcl::Font aFont = m_xTitle->GetFont();
    aFont.SetWeight( bBold?WEIGHT_BOLD:WEIGHT_NORMAL );
    m_xTitle->SetFont( aFont );
    m_xTitle->Invalidate();
}

void OTableWindow::GetFocus()
{
    Window::GetFocus();
    // we have to forward the focus to our listbox to enable keystokes
    if(m_xListBox)
        m_xListBox->GrabFocus();
}

void OTableWindow::setActive(bool _bActive)
{
    SetBoldTitle( _bActive );
    m_bActive = _bActive;
    if (!_bActive && m_xListBox && m_xListBox->GetSelectionCount() != 0)
        m_xListBox->SelectAll(false);
}

void OTableWindow::Remove()
{
    // Delete the window
    OJoinTableView* pTabWinCont = getTableView();
    pTabWinCont->RemoveTabWin( this );
    pTabWinCont->Invalidate();
}

bool OTableWindow::HandleKeyInput( const KeyEvent& rEvt )
{
    const vcl::KeyCode& rCode = rEvt.GetKeyCode();
    sal_uInt16 nCode = rCode.GetCode();
    bool   bShift = rCode.IsShift();
    bool   bCtrl = rCode.IsMod1();

    bool bHandle = false;

    if( !bCtrl && !bShift && (nCode==KEY_DELETE) )
    {
        Remove();
        bHandle = true;
    }
    return bHandle;
}

bool OTableWindow::ExistsAConn() const
{
    return getTableView()->ExistsAConn(this);
}

void OTableWindow::EnumValidFields(::std::vector< OUString>& arrstrFields)
{
    arrstrFields.clear();
    // This default implementation counts every item in the ListBox ... for any other behaviour it must be over-written
    if ( m_xListBox )
    {
        arrstrFields.reserve(m_xListBox->GetEntryCount());
        SvTreeListEntry* pEntryLoop = m_xListBox->First();
        while (pEntryLoop)
        {
            arrstrFields.push_back(m_xListBox->GetEntryText(pEntryLoop));
            pEntryLoop = m_xListBox->Next(pEntryLoop);
        }
    }
}

void OTableWindow::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    // FIXME RenderContext

    if ( nType == StateChangedType::Zoom )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

        vcl::Font aFont = rStyleSettings.GetGroupFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetZoomedPointFont(*this, aFont);

        m_xTitle->SetZoom(GetZoom());
        m_xListBox->SetZoom(GetZoom());
        Resize();
        Invalidate();
    }
}

Reference< XAccessible > OTableWindow::CreateAccessible()
{
    OTableWindowAccess* pAccessible = new OTableWindowAccess(this);
    m_pAccessible = pAccessible;
    return pAccessible;
}

void OTableWindow::Command(const CommandEvent& rEvt)
{
    switch (rEvt.GetCommand())
    {
        case CommandEventId::ContextMenu:
        {
            OJoinController& rController = getDesignView()->getController();
            if(!rController.isReadOnly() && rController.isConnected())
            {
                Point ptWhere;
                if ( rEvt.IsMouseEvent() )
                    ptWhere = rEvt.GetMousePosPixel();
                else
                {
                    SvTreeListEntry* pCurrent = m_xListBox->GetCurEntry();
                    if ( pCurrent )
                        ptWhere = m_xListBox->GetEntryPosition(pCurrent);
                    else
                        ptWhere = m_xTitle->GetPosPixel();
                }

                PopupMenu aContextMenu(ModuleRes(RID_MENU_JOINVIEW_TABLE));
                switch (aContextMenu.Execute(this, ptWhere))
                {
                    case SID_DELETE:
                        Remove();
                        break;
                }
            }
            break;
        }
        default:
            Window::Command(rEvt);
    }
}

bool OTableWindow::PreNotify(NotifyEvent& rNEvt)
{
    bool bHandled = false;
    switch (rNEvt.GetType())
    {
        case MouseNotifyEvent::KEYINPUT:
        {
            if ( getDesignView()->getController().isReadOnly() )
                break;

            const KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
            const vcl::KeyCode& rCode = pKeyEvent->GetKeyCode();
            if ( rCode.IsMod1() )
            {
                Point aStartPoint = GetPosPixel();
                if ( rCode.IsShift() )
                {
                    aStartPoint.X() = GetSizePixel().Width();
                    aStartPoint.Y() = GetSizePixel().Height();
                }

                switch( rCode.GetCode() )
                {
                    case KEY_DOWN:
                        bHandled = true;
                        aStartPoint.Y() += m_nMoveIncrement;
                        break;
                    case KEY_UP:
                        bHandled = true;
                        aStartPoint.Y() += -m_nMoveIncrement;
                        break;
                    case KEY_LEFT:
                        bHandled = true;
                        aStartPoint.X() += -m_nMoveIncrement;
                        break;
                    case KEY_RIGHT:
                        bHandled = true;
                        aStartPoint.X()  += m_nMoveIncrement;
                        break;
                }
                if ( bHandled )
                {
                    if ( rCode.IsShift() )
                    {
                        OJoinTableView* pView = getTableView();
                        Point ptOld = GetPosPixel();
                        Size aSize = pView->getRealOutputSize();
                        Size aNewSize(aStartPoint.X(),aStartPoint.Y());
                        if (   ((ptOld.X() + aNewSize.Width())  <= aSize.Width())
                            && ((ptOld.Y() + aNewSize.Height()) <= aSize.Height()) )
                        {
                            if ( aNewSize.Width() < TABWIN_WIDTH_MIN )
                                aNewSize.Width() = TABWIN_WIDTH_MIN;
                            if ( aNewSize.Height() < TABWIN_HEIGHT_MIN )
                                aNewSize.Height() = TABWIN_HEIGHT_MIN;

                            Size szOld = GetSizePixel();

                            aNewSize = Size(pView->CalcZoom(aNewSize.Width()),pView->CalcZoom(aNewSize.Height()));
                            SetPosSizePixel( ptOld, aNewSize );
                            pView->TabWinSized(this, ptOld, szOld);
                            Invalidate( InvalidateFlags::NoChildren );
                        }
                    }
                    else
                    {
                        // remember how often the user moved our window
                        ++m_nMoveCount;
                        if( m_nMoveCount == 5 )
                            m_nMoveIncrement = 10;
                        else if( m_nMoveCount > 15 )
                            m_nMoveCount = m_nMoveIncrement = 20;

                        Point aOldDataPoint = GetData()->GetPosition();
                        Point aNewDataPoint = aStartPoint + getTableView()->GetScrollOffset();
                        if ( aNewDataPoint.X() > -1 && aNewDataPoint.Y() > -1 )
                        {
                            OJoinTableView* pView = getTableView();
                            if ( pView->isMovementAllowed(aNewDataPoint, GetData()->GetSize()) )
                            {
                                SetPosPixel(aStartPoint);

                                // aNewDataPoint can not be used here because SetPosPixel reset it
                                pView->EnsureVisible(GetData()->GetPosition(), GetData()->GetSize());
                                pView->TabWinMoved(this,aOldDataPoint);
                                Invalidate(InvalidateFlags::NoChildren);
                                getDesignView()->getController().setModified( sal_True );
                            }
                            else
                            {
                                m_nMoveCount        = 0; // reset our movement count
                                m_nMoveIncrement    = 1;
                            }
                        }
                        else
                        {
                            m_nMoveCount        = 0; // reset our movement count
                            m_nMoveIncrement    = 1;
                        }
                    }
                    resetSizingFlag();
                }
                else
                {
                    m_nMoveCount        = 0; // reset our movement count
                    m_nMoveIncrement    = 1;
                }
            }
            else
            {
                m_nMoveCount        = 0; // reset our movement count
                m_nMoveIncrement    = 1;
            }
            break;
        }
        case MouseNotifyEvent::KEYUP:
        {
            const KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
            const vcl::KeyCode& rCode = pKeyEvent->GetKeyCode();
            sal_uInt16 nKeyCode = rCode.GetCode();
            if ( rCode.IsMod2() && nKeyCode != KEY_UP && nKeyCode != KEY_DOWN && nKeyCode != KEY_LEFT && nKeyCode != KEY_RIGHT )
            {
                m_nMoveCount        = 0; // reset our movement count
                m_nMoveIncrement    = 1;
            }
            break;
        }
        default:
            break;
    }
    if (!bHandled)
        return Window::PreNotify(rNEvt);
    return true;
}

OUString OTableWindow::getTitle() const
{
    return m_xTitle->GetText();
}

void OTableWindow::_elementInserted( const container::ContainerEvent& /*_rEvent*/ )  throw(css::uno::RuntimeException, std::exception)
{
    FillListBox();
}

void OTableWindow::_elementRemoved( const container::ContainerEvent& /*_rEvent*/ ) throw(css::uno::RuntimeException, std::exception)
{
    FillListBox();
}

void OTableWindow::_elementReplaced( const container::ContainerEvent& /*_rEvent*/ ) throw(css::uno::RuntimeException, std::exception)
{
    FillListBox();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
