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

#include <TableWindow.hxx>
#include <TableWindowListBox.hxx>
#include <TableWindowData.hxx>
#include <imageprovider.hxx>
#include <JoinController.hxx>
#include <JoinTableView.hxx>
#include <JoinDesignView.hxx>
#include <osl/diagnose.h>
#include <utility>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/wall.hxx>
#include <vcl/weldutils.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/application/DatabaseObject.hpp>
#include <bitmaps.hlst>
#include <TableWindowAccess.hxx>
#include <connectivity/dbtools.hxx>

using namespace dbaui;
using namespace ::com::sun::star;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::accessibility;

namespace DatabaseObject = css::sdb::application::DatabaseObject;

#define TABWIN_SIZING_AREA      4
#define TABWIN_WIDTH_MIN    90
#define TABWIN_HEIGHT_MIN   80

namespace {

void Draw3DBorder(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
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

OTableWindow::OTableWindow( vcl::Window* pParent, TTableWindowData::value_type pTabWinData )
    : ::comphelper::OContainerListener(m_aMutex)
    , Window( pParent, WB_3DLOOK|WB_MOVEABLE )
    , m_xTitle( VclPtr<OTableWindowTitle>::Create(this) )
    , m_pData(std::move( pTabWinData ))
    , m_nMoveCount(0)
    , m_nMoveIncrement(1)
    , m_nSizingFlags( SizingFlags::NONE )
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
        OSL_ENSURE(m_xListBox->get_widget().n_children()==0,"Forgot to call EmptyListbox()!");
    }
    m_xListBox.disposeAndClear();
    if ( m_pContainerListener.is() )
        m_pContainerListener->dispose();

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
        aOutSize.setWidth( TABWIN_WIDTH_MIN );
    if( aOutSize.Height() < TABWIN_HEIGHT_MIN )
        aOutSize.setHeight( TABWIN_HEIGHT_MIN );

    GetData()->SetSize( aOutSize );
    Window::SetSizePixel( aOutSize );
}

void OTableWindow::SetPosSizePixel( const Point& rNewPos, const Size& rNewSize )
{
    SetPosPixel( rNewPos );
    SetSizePixel( rNewSize );
}

void OTableWindow::FillListBox()
{
    clearListBox();
    weld::TreeView& rTreeView = m_xListBox->get_widget();
    assert(!rTreeView.n_children());

    if ( !m_pContainerListener.is() )
    {
        Reference< XContainer> xContainer(m_pData->getColumns(),UNO_QUERY);
        if ( xContainer.is() )
            m_pContainerListener = new ::comphelper::OContainerListenerAdapter(this,xContainer);
    }

    // mark all primary keys with special image
    OUString aPrimKeyImage(BMP_PRIMARY_KEY);

    if (GetData()->IsShowAll())
    {
        rTreeView.append(weld::toId(createUserData(nullptr,false)), OUString("*"));
    }

    Reference<XNameAccess> xPKeyColumns;
    try
    {
        xPKeyColumns = dbtools::getPrimaryKeyColumns_throw(m_pData->getTable());
    }
    catch(Exception&)
    {
        TOOLS_WARN_EXCEPTION( "dbaccess", "");
    }
    try
    {
        Reference< XNameAccess > xColumns = m_pData->getColumns();
        if( xColumns.is() )
        {
            for (auto& column : xColumns->getElementNames())
            {
                bool bPrimaryKeyColumn = xPKeyColumns.is() && xPKeyColumns->hasByName(column);

                OUString sId;
                Reference<XPropertySet> xColumn(xColumns->getByName(column), UNO_QUERY);
                if (xColumn.is())
                    sId = weld::toId(createUserData(xColumn, bPrimaryKeyColumn));

                rTreeView.append(sId, column);

                // is this column in the primary key
                if ( bPrimaryKeyColumn )
                    rTreeView.set_image(rTreeView.n_children() - 1, aPrimKeyImage);
            }

        }
    }
    catch(Exception&)
    {
        TOOLS_WARN_EXCEPTION( "dbaccess", "");
    }
}

void* OTableWindow::createUserData(const Reference< XPropertySet>& /*_xColumn*/,bool /*_bPrimaryKey*/)
{
    return nullptr;
}

void OTableWindow::deleteUserData(void*& _pUserData)
{
    OSL_ENSURE(!_pUserData,"INVALID call. Need to delete the userclass!");
    _pUserData = nullptr;
}

void OTableWindow::clearListBox()
{
    if ( !m_xListBox )
        return;

    weld::TreeView& rTreeView = m_xListBox->get_widget();
    rTreeView.all_foreach([this, &rTreeView](weld::TreeIter& rEntry){
        void* pUserData = weld::fromId<void*>(rTreeView.get_id(rEntry));
        deleteUserData(pUserData);
        return false;
    });

    rTreeView.clear();
}

void OTableWindow::impl_updateImage()
{
    weld::Image& rImage = m_xTitle->GetImage();
    ImageProvider aImageProvider( getDesignView()->getController().getConnection() );
    rImage.set_from_icon_name(aImageProvider.getImageId(GetComposedName(), m_pData->isQuery() ? DatabaseObject::QUERY : DatabaseObject::TABLE));
    rImage.show();
}

bool OTableWindow::Init()
{
    // create list box if necessary
    if ( !m_xListBox )
    {
        m_xListBox = VclPtr<OTableWindowListBox>::Create(this);
        assert(m_xListBox && "OTableWindow::Init() : CreateListBox returned NULL !");
        m_xListBox->get_widget().set_selection_mode(SelectionMode::Multiple);
    }

    // Set the title
    weld::Label& rLabel = m_xTitle->GetLabel();
    rLabel.set_label(m_pData->GetWinName());
    rLabel.set_tooltip_text(GetComposedName());
    m_xTitle->Show();

    m_xListBox->Show();

    // add the fields to the ListBox
    FillListBox();
    m_xListBox->get_widget().unselect_all();

    impl_updateImage();

    return true;
}

void OTableWindow::DataChanged(const DataChangedEvent& rDCEvt)
{
    if (rDCEvt.GetType() == DataChangedEventType::SETTINGS)
    {
        // In the worst-case the colours have changed so
        // adapt myself to the new colours
        const StyleSettings&  aSystemStyle = Application::GetSettings().GetStyleSettings();
        SetBackground(Wallpaper(aSystemStyle.GetFaceColor()));
        SetTextColor(aSystemStyle.GetButtonTextColor());
    }
}

void OTableWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    tools::Rectangle aRect(Point(0,0), GetOutputSizePixel());
    Window::Paint(rRenderContext, rRect);
    Draw3DBorder(rRenderContext, aRect);
}

tools::Rectangle OTableWindow::getSizingRect(const Point& _rPos,const Size& _rOutputSize) const
{
    tools::Rectangle aSizingRect( GetPosPixel(), GetSizePixel() );

    if( m_nSizingFlags & SizingFlags::Top )
    {
        if( _rPos.Y() < 0 )
            aSizingRect.SetTop( 0 );
        else
            aSizingRect.SetTop( _rPos.Y() );
    }

    if( m_nSizingFlags & SizingFlags::Bottom )
    {
        if( _rPos.Y() > _rOutputSize.Height() )
            aSizingRect.SetBottom( _rOutputSize.Height() );
        else
            aSizingRect.SetBottom( _rPos.Y() );
    }

    if( m_nSizingFlags & SizingFlags::Right )
    {
        if( _rPos.X() > _rOutputSize.Width() )
            aSizingRect.SetRight( _rOutputSize.Width() );
        else
            aSizingRect.SetRight( _rPos.X() );
    }

    if( m_nSizingFlags & SizingFlags::Left )
    {
        if( _rPos.X() < 0 )
            aSizingRect.SetLeft( 0 );
        else
            aSizingRect.SetLeft( _rPos.X() );
    }
    return aSizingRect;
}

void OTableWindow::setSizingFlag(const Point& _rPos)
{
    Size    aOutSize = GetOutputSizePixel();
    // Set the flags when the mouse cursor is in the sizing area
    m_nSizingFlags = SizingFlags::NONE;

    if( _rPos.X() < TABWIN_SIZING_AREA )
        m_nSizingFlags |= SizingFlags::Left;

    if( _rPos.Y() < TABWIN_SIZING_AREA )
        m_nSizingFlags |= SizingFlags::Top;

    if( _rPos.X() > aOutSize.Width()-TABWIN_SIZING_AREA )
        m_nSizingFlags |= SizingFlags::Right;

    if( _rPos.Y() > aOutSize.Height()-TABWIN_SIZING_AREA )
        m_nSizingFlags |= SizingFlags::Bottom;
}

void OTableWindow::MouseMove( const MouseEvent& rEvt )
{
    Window::MouseMove(rEvt);

    OJoinTableView* pCont = getTableView();
    if (pCont->getDesignView()->getController().isReadOnly())
        return;

    Point   aPos = rEvt.GetPosPixel();
    setSizingFlag(aPos);
    PointerStyle aPointer = PointerStyle::Arrow;

    // Set the mouse cursor when it is in the sizing area
    if ( m_nSizingFlags == SizingFlags::Top ||
         m_nSizingFlags == SizingFlags::Bottom )
        aPointer = PointerStyle::SSize;
    else if ( m_nSizingFlags == SizingFlags::Left ||
              m_nSizingFlags ==SizingFlags::Right )
        aPointer = PointerStyle::ESize;
    else if ( m_nSizingFlags == (SizingFlags::Left | SizingFlags::Top) ||
              m_nSizingFlags == (SizingFlags::Right | SizingFlags::Bottom) )
        aPointer = PointerStyle::SESize;
    else if ( m_nSizingFlags == (SizingFlags::Right | SizingFlags::Top) ||
              m_nSizingFlags == (SizingFlags::Left | SizingFlags::Bottom) )
        aPointer = PointerStyle::NESize;

    SetPointer( aPointer );
}

void OTableWindow::MouseButtonDown( const MouseEvent& rEvt )
{
    // When resizing, the parent must be informed that
    // the window size of its child has changed
    if( m_nSizingFlags != SizingFlags::NONE )
        getTableView()->BeginChildSizing( this, GetPointer() );

    Window::MouseButtonDown( rEvt );
}

void OTableWindow::Resize()
{
    // The window must not disappear so we enforce a minimum size
    Size    aOutSize = GetOutputSizePixel();
    aOutSize = Size(CalcZoom(aOutSize.Width()),CalcZoom(aOutSize.Height()));

    tools::Long nTitleHeight = CalcZoom( GetTextHeight() )+ CalcZoom( 4 );

    // Set the title and ListBox
    tools::Long n5Pos = CalcZoom(5);
    tools::Long nPositionX = n5Pos;
    tools::Long nPositionY = n5Pos;

    Size aPreferredSize = m_xTitle->get_preferred_size();
    if (nTitleHeight < aPreferredSize.Height())
        nTitleHeight = aPreferredSize.Height();

    m_xTitle->SetPosSizePixel( Point( nPositionX, nPositionY ), Size( aOutSize.Width() - nPositionX - n5Pos, nTitleHeight ) );

    tools::Long nTitleToList = CalcZoom( 3 );

    m_xListBox->SetPosSizePixel(
        Point( n5Pos, nPositionY + nTitleHeight + nTitleToList ),
        Size( aOutSize.Width() - 2 * n5Pos, aOutSize.Height() - ( nPositionY + nTitleHeight + nTitleToList ) - n5Pos )
    );

    Window::Invalidate();
}

void OTableWindow::SetBoldTitle( bool bBold )
{
    weld::Label& rLabel = m_xTitle->GetLabel();
    vcl::Font aFont = rLabel.get_font();
    aFont.SetWeight(bBold ? WEIGHT_BOLD : WEIGHT_NORMAL);
    rLabel.set_font(aFont);
}

void OTableWindow::GetFocus()
{
    Window::GetFocus();
    // we have to forward the focus to our listbox to enable keystrokes
    if(m_xListBox)
        m_xListBox->GrabFocus();
}

void OTableWindow::setActive(bool _bActive)
{
    SetBoldTitle( _bActive );
    if (_bActive || !m_xListBox)
        return;

    weld::TreeView& rTreeView = m_xListBox->get_widget();
    if (rTreeView.get_selected_index() != -1)
        rTreeView.unselect_all();
}

void OTableWindow::Remove()
{
    // Delete the window
    OJoinTableView* pTabWinCont = getTableView();
    VclPtr<OTableWindow> aHoldSelf(this); // keep ourselves alive during the RemoveTabWin process
    pTabWinCont->RemoveTabWin( this );
    pTabWinCont->Invalidate();
}

bool OTableWindow::ExistsAConn() const
{
    return getTableView()->ExistsAConn(this);
}

void OTableWindow::EnumValidFields(std::vector< OUString>& arrstrFields)
{
    arrstrFields.clear();
    weld::TreeView& rTreeView = m_xListBox->get_widget();

    // This default implementation counts every item in the ListBox ... for any other behaviour it must be over-written
    rTreeView.all_foreach([&rTreeView, &arrstrFields](weld::TreeIter& rEntry){
        arrstrFields.push_back(rTreeView.get_text(rEntry));
        return false;
    });
}

void OTableWindow::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    // FIXME RenderContext

    if ( nType != StateChangedType::Zoom )
        return;

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    vcl::Font aFont = rStyleSettings.GetGroupFont();
    if ( IsControlFont() )
        aFont.Merge( GetControlFont() );
    SetZoomedPointFont(*GetOutDev(), aFont);

    m_xTitle->SetZoom(GetZoom());
    m_xListBox->SetZoom(GetZoom());
    Resize();
    Invalidate();
}

Reference< XAccessible > OTableWindow::CreateAccessible()
{
    return new OTableWindowAccess(this);
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
                    weld::TreeView& rTreeView = m_xListBox->get_widget();
                    std::unique_ptr<weld::TreeIter> xCurrent = rTreeView.make_iterator();
                    if (rTreeView.get_cursor(xCurrent.get()))
                        ptWhere = rTreeView.get_row_area(*xCurrent).Center();
                    else
                        ptWhere = m_xTitle->GetPosPixel();
                }

                ::tools::Rectangle aRect(ptWhere, Size(1, 1));
                weld::Window* pPopupParent = weld::GetPopupParent(*this, aRect);
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(pPopupParent, "dbaccess/ui/jointablemenu.ui"));
                std::unique_ptr<weld::Menu> xContextMenu(xBuilder->weld_menu("menu"));
                if (!xContextMenu->popup_at_rect(pPopupParent, aRect).isEmpty())
                    Remove();
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
        case NotifyEventType::KEYINPUT:
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
                    aStartPoint.setX( GetSizePixel().Width() );
                    aStartPoint.setY( GetSizePixel().Height() );
                }

                switch( rCode.GetCode() )
                {
                    case KEY_DOWN:
                        bHandled = true;
                        aStartPoint.AdjustY(m_nMoveIncrement );
                        break;
                    case KEY_UP:
                        bHandled = true;
                        aStartPoint.AdjustY(-m_nMoveIncrement );
                        break;
                    case KEY_LEFT:
                        bHandled = true;
                        aStartPoint.AdjustX(-m_nMoveIncrement );
                        break;
                    case KEY_RIGHT:
                        bHandled = true;
                        aStartPoint.AdjustX(m_nMoveIncrement );
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
                                aNewSize.setWidth( TABWIN_WIDTH_MIN );
                            if ( aNewSize.Height() < TABWIN_HEIGHT_MIN )
                                aNewSize.setHeight( TABWIN_HEIGHT_MIN );

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
                                getDesignView()->getController().setModified( true );
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
                    m_nSizingFlags = SizingFlags::NONE;
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
        case NotifyEventType::KEYUP:
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
    return m_xTitle->GetLabel().get_label();
}

void OTableWindow::_elementInserted( const container::ContainerEvent& /*_rEvent*/ )
{
    FillListBox();
}

void OTableWindow::_elementRemoved( const container::ContainerEvent& /*_rEvent*/ )
{
    FillListBox();
}

void OTableWindow::_elementReplaced( const container::ContainerEvent& /*_rEvent*/ )
{
    FillListBox();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
