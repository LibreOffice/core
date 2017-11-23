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

#include <QtGui/QStyle>
#include <QtGui/QStyleOption>
#include <QtGui/QPainter>
#include <QtGui/QFrame>
#include <QtGui/QLabel>

#include <kapplication.h>
#include <kdebug.h>

#undef Region

#include "KDESalGraphics.hxx"
#include "KDESalInstance.hxx"

#include <vcl/settings.hxx>
#include <vcl/decoview.hxx>
#include <rtl/ustrbuf.hxx>

/**
  Conversion function between VCL ControlState together with
  ImplControlValue and Qt state flags.
  @param nControlState State of the widget (default, focused, ...) in Native Widget Framework.
  @param aValue Value held by the widget (on, off, ...)
*/
QStyle::State vclStateValue2StateFlag( ControlState nControlState,
    const ImplControlValue& aValue )
{
    QStyle::State nState =
        ( (nControlState & ControlState::ENABLED)?  QStyle::State_Enabled:   QStyle::State_None ) |
        ( (nControlState & ControlState::FOCUSED)?  QStyle::State_HasFocus:  QStyle::State_None ) |
        ( (nControlState & ControlState::PRESSED)?  QStyle::State_Sunken:    QStyle::State_None ) |
        ( (nControlState & ControlState::SELECTED)? QStyle::State_Selected : QStyle::State_None ) |
        ( (nControlState & ControlState::ROLLOVER)? QStyle::State_MouseOver: QStyle::State_None );

    switch ( aValue.getTristateVal() )
    {
        case ButtonValue::On:    nState |= QStyle::State_On;       break;
        case ButtonValue::Off:   nState |= QStyle::State_Off;      break;
        case ButtonValue::Mixed: nState |= QStyle::State_NoChange; break;
        default: break;
    }

    return nState;
}

/**
 Convert tools::Rectangle to QRect.
 @param rControlRegion The tools::Rectangle to convert.
 @return The matching QRect
*/
QRect region2QRect( const tools::Rectangle& rControlRegion )
{
    return QRect(rControlRegion.Left(), rControlRegion.Top(), rControlRegion.GetWidth(), rControlRegion.GetHeight());
}

bool KDESalGraphics::IsNativeControlSupported( ControlType type, ControlPart part )
{
    switch (type)
    {
        case ControlType::Pushbutton:
        case ControlType::Radiobutton:
        case ControlType::Checkbox:
        case ControlType::Tooltip:
        case ControlType::Progress:
        case ControlType::ListNode:
            return (part == ControlPart::Entire);

        case ControlType::Menubar:
        case ControlType::MenuPopup:
        case ControlType::Editbox:
        case ControlType::MultilineEditbox:
        case ControlType::Combobox:
        case ControlType::Toolbar:
        case ControlType::Frame:
        case ControlType::Scrollbar:
        case ControlType::WindowBackground:
        case ControlType::Fixedline:
            return true;

        case ControlType::Listbox:
            return (part == ControlPart::Entire || part == ControlPart::HasBackgroundTexture);

        case ControlType::Spinbox:
            return (part == ControlPart::Entire || part == ControlPart::HasBackgroundTexture);

        case ControlType::Slider:
            return (part == ControlPart::TrackHorzArea || part == ControlPart::TrackVertArea);

        default:
            break;
    }

    return false;
}

/// helper drawing methods
namespace
{
    void draw( QStyle::ControlElement element, QStyleOption* option, QImage* image, QStyle::State const & state, QRect rect = QRect())
    {
        option->state |= state;
        option->rect = !rect.isNull() ? rect : image->rect();

        QPainter painter(image);
        QApplication::style()->drawControl(element, option, &painter);
    }

    void draw( QStyle::PrimitiveElement element, QStyleOption* option, QImage* image, QStyle::State const & state, QRect rect = QRect())
    {
        option->state |= state;
        option->rect = !rect.isNull() ? rect : image->rect();

        QPainter painter(image);
        QApplication::style()->drawPrimitive(element, option, &painter);
    }

    void draw( QStyle::ComplexControl element, QStyleOptionComplex* option, QImage* image, QStyle::State const & state )
    {
        option->state |= state;
        option->rect = image->rect();

        QPainter painter(image);
        QApplication::style()->drawComplexControl(element, option, &painter);
    }

    void lcl_drawFrame( QStyle::PrimitiveElement element, QImage* image, QStyle::State const & state,
                        QStyle::PixelMetric eLineMetric = QStyle::PM_DefaultFrameWidth )
    {
    #if ( QT_VERSION >= QT_VERSION_CHECK( 4, 5, 0 ) )
        QStyleOptionFrameV3 option;
        option.frameShape = QFrame::StyledPanel;
        option.state = QStyle::State_Sunken;
        option.lineWidth = QApplication::style()->pixelMetric( eLineMetric );
    #else
        QStyleOptionFrame option;

        QFrame aFrame( nullptr );
        aFrame.setFrameRect( QRect(0, 0, image->width(), image->height()) );
        aFrame.setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
        aFrame.ensurePolished();

        option.initFrom( &aFrame );
        option.lineWidth = aFrame.lineWidth();
        option.midLineWidth = aFrame.midLineWidth();
    #endif
        draw(element, &option, image, state);
    }
}

#if QT_VERSION >= QT_VERSION_CHECK( 4, 5, 0 )
#define IMAGE_BASED_PAINTING
#else
#undef IMAGE_BASED_PAINTING
#endif

#ifdef IMAGE_BASED_PAINTING
// There is a small catch with this function, although hopefully only philosophical.
// Officially Xlib's vcl::Region is an opaque data type, with only functions for manipulating it.
// However, whoever designed it apparently didn't give it that much thought, as it's impossible
// to find out what exactly a region actually is (except for really weird ways like XClipBox()
// and repeated XPointInRegion(), which would be awfully slow). Fortunately, the header file
// describing the structure actually happens to be installed too, and there's at least one
// widely used software using it (Compiz). So access the data directly too and assume that
// everybody who compiles with Qt4 support has Xlib new enough and good enough to support this.
// In case this doesn't work for somebody, try #include <X11/region.h> instead, or build
// without IMAGE_BASED_PAINTING (in which case QApplication::setGraphicsSystem( "native" ) may
// be needed too).
#include <X11/Xregion.h>
static QRegion XRegionToQRegion( Region xr )
{
    QRegion qr;
    for( long i = 0;
         i < xr->numRects;
         ++i )
    {
        BOX& b = xr->rects[ i ];
        qr |= QRect( b.x1, b.y1, b.x2 - b.x1, b.y2 - b.y1 ); // x2,y2 is outside, not the bottom-right corner
    }
    return qr;
}
#endif

bool KDESalGraphics::drawNativeControl( ControlType type, ControlPart part,
                                        const tools::Rectangle& rControlRegion, ControlState nControlState,
                                        const ImplControlValue& value,
                                        const OUString& )
{
    bool nativeSupport = IsNativeControlSupported( type, part );
    if( ! nativeSupport ) {
        assert( ! nativeSupport && "drawNativeControl called without native support!" );
        return false;
    }

    if( lastPopupRect.isValid() && ( type != ControlType::MenuPopup || part != ControlPart::MenuItem ))
        lastPopupRect = QRect();

    bool returnVal = true;

    QRect widgetRect = region2QRect(rControlRegion);

    //if no image, or resized, make a new image
    if (!m_image || m_image->size() != widgetRect.size())
    {
        m_image.reset(new QImage( widgetRect.width(), widgetRect.height(), QImage::Format_ARGB32 ) );
    }

    // Default image color - just once
    switch (type)
    {
        case ControlType::MenuPopup:
            if( part == ControlPart::MenuItemCheckMark || part == ControlPart::MenuItemRadioMark )
            {
                // it is necessary to fill the background transparently first, as this
                // is painted after menuitem highlight, otherwise there would be a grey area
                m_image->fill( Qt::transparent );
                break;
            }
            SAL_FALLTHROUGH; // QPalette::Window
        case ControlType::Menubar:
        case ControlType::WindowBackground:
            m_image->fill( KApplication::palette().color(QPalette::Window).rgb() );
            break;
        case ControlType::Tooltip:
            m_image->fill(KApplication::palette().color(QPalette::ToolTipBase).rgb());
            break;
        case ControlType::Pushbutton:
            m_image->fill(KApplication::palette().color(QPalette::Button).rgb());
            break;
        case ControlType::Scrollbar:
            if ((part == ControlPart::DrawBackgroundVert)
                || (part == ControlPart::DrawBackgroundHorz))
            {
                m_image->fill( KApplication::palette().color(QPalette::Window).rgb() );
                break;
            }
            SAL_FALLTHROUGH; // Qt::transparent
        default:
            m_image->fill( Qt::transparent );
            break;
    }

    QRegion* localClipRegion = nullptr;

    if (type == ControlType::Pushbutton)
    {
        QStyleOptionButton option;
        draw( QStyle::CE_PushButton, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == ControlType::Menubar)
    {
        if (part == ControlPart::MenuItem)
        {
            QStyleOptionMenuItem option;
            if ( ( nControlState & ControlState::ROLLOVER )
                && QApplication::style()->styleHint( QStyle::SH_MenuBar_MouseTracking ) )
                option.state |= QStyle::State_Selected;

            if ( nControlState & ControlState::SELECTED ) // Passing State_Sunken is currently not documented.
                option.state |= QStyle::State_Sunken;  // But some kinds of QStyle interpret it.

            draw( QStyle::CE_MenuBarItem, &option, m_image.get(),
                  vclStateValue2StateFlag(nControlState, value) );
        }
        else if (part == ControlPart::Entire)
        {
            QStyleOptionMenuItem option;
            draw( QStyle::CE_MenuBarEmptyArea, &option, m_image.get(),
                  vclStateValue2StateFlag(nControlState, value) );
        }
        else
        {
            returnVal = false;
        }
    }
    else if (type == ControlType::MenuPopup)
    {
        OSL_ASSERT( part == ControlPart::MenuItem ? lastPopupRect.isValid() : !lastPopupRect.isValid());
        if( part == ControlPart::MenuItem )
        {
            QStyleOptionMenuItem option;
            draw( QStyle::CE_MenuItem, &option, m_image.get(),
                  vclStateValue2StateFlag(nControlState, value) );
            // HACK: LO core first paints the entire popup and only then it paints menu items,
            // but QMenu::paintEvent() paints popup frame after all items. That means highlighted
            // items here would paint the highlight over the frame border. Since calls to ControlPart::MenuItem
            // are always preceded by calls to ControlPart::Entire, just remember the size for the whole
            // popup (otherwise not possible to get here) and draw the border afterwards.
            QRect framerect( lastPopupRect.topLeft() - widgetRect.topLeft(),
                widgetRect.size().expandedTo( lastPopupRect.size()));
            QStyleOptionFrame frame;
            draw( QStyle::PE_FrameMenu, &frame, m_image.get(), vclStateValue2StateFlag( nControlState, value ), framerect );
        }
        else if( part == ControlPart::Separator )
        {
            QStyleOptionMenuItem option;
            option.menuItemType = QStyleOptionMenuItem::Separator;
            // Painting the whole menu item area results in different background
            // with at least Plastique style, so clip only to the separator itself
            // (QSize( 2, 2 ) is hardcoded in Qt)
            option.rect = m_image->rect();
            QSize size = QApplication::style()->sizeFromContents( QStyle::CT_MenuItem, &option, QSize( 2, 2 ));
            QRect rect = m_image->rect();
            QPoint center = rect.center();
            rect.setHeight( size.height());
            rect.moveCenter( center );
            // don't paint over popup frame border (like the hack above, but here it can be simpler)
            int fw = QApplication::style()->pixelMetric( QStyle::PM_MenuPanelWidth );
            localClipRegion = new QRegion(rect.translated(widgetRect.topLeft()).adjusted(fw, 0, -fw, 0));
            draw( QStyle::CE_MenuItem, &option, m_image.get(),
                  vclStateValue2StateFlag(nControlState, value), rect );
        }
        else if( part == ControlPart::MenuItemCheckMark || part == ControlPart::MenuItemRadioMark )
        {
            QStyleOptionMenuItem option;
            option.checkType = ( part == ControlPart::MenuItemCheckMark )
                ? QStyleOptionMenuItem::NonExclusive : QStyleOptionMenuItem::Exclusive;
            option.checked = bool( nControlState & ControlState::PRESSED );
            // widgetRect is now the rectangle for the checkbox/radiobutton itself, but Qt
            // paints the whole menu item, so translate position (and it'll be clipped);
            // it is also necessary to fill the background transparently first, as this
            // is painted after menuitem highlight, otherwise there would be a grey area
            assert( value.getType() == ControlType::MenuPopup );
            const MenupopupValue* menuVal = static_cast<const MenupopupValue*>(&value);
            QRect menuItemRect( region2QRect( menuVal->maItemRect ));
            QRect rect( menuItemRect.topLeft() - widgetRect.topLeft(),
                widgetRect.size().expandedTo( menuItemRect.size()));
            // checkboxes are always displayed next to images in menus, so are never centered
            const int focus_size = QApplication::style()->pixelMetric( QStyle::PM_FocusFrameHMargin );
            rect.moveTo( -focus_size, rect.y() );
            draw( QStyle::CE_MenuItem, &option, m_image.get(),
                  vclStateValue2StateFlag(nControlState & ~ControlState::PRESSED, value), rect );
        }
        else if( part == ControlPart::Entire )
        {
            QStyleOptionMenuItem option;
            draw( QStyle::PE_PanelMenu, &option, m_image.get(), vclStateValue2StateFlag( nControlState, value ));
            // Try hard to get any frame!
            QStyleOptionFrame frame;
            draw( QStyle::PE_FrameMenu, &frame, m_image.get(), vclStateValue2StateFlag( nControlState, value ));
            draw( QStyle::PE_FrameWindow, &frame, m_image.get(), vclStateValue2StateFlag( nControlState, value ));
            lastPopupRect = widgetRect;
        }
        else
            returnVal = false;
    }
    else if ( (type == ControlType::Toolbar) && (part == ControlPart::Button) )
    {
        QStyleOptionToolButton option;

        option.arrowType = Qt::NoArrow;
        option.subControls = QStyle::SC_ToolButton;

        option.state = vclStateValue2StateFlag( nControlState, value );
        option.state |= QStyle::State_Raised | QStyle::State_Enabled | QStyle::State_AutoRaise;

        draw( QStyle::CC_ToolButton, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if ( (type == ControlType::Toolbar) && (part == ControlPart::Entire) )
    {
        QStyleOptionToolBar option;

        option.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
        option.state = vclStateValue2StateFlag( nControlState, value );

        draw( QStyle::CE_ToolBar, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if ( (type == ControlType::Toolbar)
            && (part == ControlPart::ThumbVert || part == ControlPart::ThumbHorz) )
    {   // reduce paint area only to the handle area
        const int handleExtend = QApplication::style()->pixelMetric(QStyle::PM_ToolBarHandleExtent);
        QRect rect;
        QStyleOption option;

        if (part == ControlPart::ThumbVert)
        {
            rect = QRect( 0, 0, handleExtend, widgetRect.height());
            localClipRegion = new QRegion(widgetRect.x(), widgetRect.y(), handleExtend, widgetRect.height());
            option.state = QStyle::State_Horizontal;
        }
        else
        {
            rect = QRect( 0, 0, widgetRect.width(), handleExtend);
            localClipRegion = new QRegion(widgetRect.x(), widgetRect.y(), widgetRect.width(), handleExtend);
        }

        draw( QStyle::PE_IndicatorToolBarHandle, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value), rect );
    }
    else if (type == ControlType::Editbox || type == ControlType::MultilineEditbox)
    {
        lcl_drawFrame( QStyle::PE_FrameLineEdit, m_image.get(),
                       vclStateValue2StateFlag(nControlState, value));
    }
    else if (type == ControlType::Combobox)
    {
        QStyleOptionComboBox option;
        option.editable = true;
        draw( QStyle::CC_ComboBox, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == ControlType::Listbox)
    {
        QStyleOptionComboBox option;
        option.editable = false;
        switch (part) {
            case ControlPart::ListboxWindow:
                lcl_drawFrame( QStyle::PE_Frame, m_image.get(),
                               vclStateValue2StateFlag(nControlState, value),
                               QStyle::PM_ComboBoxFrameWidth );
                break;
            case ControlPart::SubEdit:
                draw( QStyle::CE_ComboBoxLabel, &option, m_image.get(),
                      vclStateValue2StateFlag(nControlState, value) );
                break;
            case ControlPart::Entire:
                draw( QStyle::CC_ComboBox, &option, m_image.get(),
                      vclStateValue2StateFlag(nControlState, value) );
                break;
            case ControlPart::ButtonDown:
                option.subControls = QStyle::SC_ComboBoxArrow;
                draw( QStyle::CC_ComboBox, &option, m_image.get(),
                      vclStateValue2StateFlag(nControlState, value) );
                break;
            default:
                returnVal = false;
                break;
        }
    }
    else if (type == ControlType::ListNode)
    {
        QStyleOption option;
        option.state = QStyle::State_Item | QStyle::State_Children;

        if (value.getTristateVal() == ButtonValue::On)
            option.state |= QStyle::State_Open;

        draw( QStyle::PE_IndicatorBranch, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == ControlType::Checkbox)
    {
        QStyleOptionButton option;
        draw( QStyle::CE_CheckBox, &option, m_image.get(),
               vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == ControlType::Scrollbar)
    {
        if ((part == ControlPart::DrawBackgroundVert) || (part == ControlPart::DrawBackgroundHorz))
        {
            QStyleOptionSlider option;
            OSL_ASSERT( value.getType() == ControlType::Scrollbar );
            const ScrollbarValue* sbVal = static_cast<const ScrollbarValue *>(&value);

            //if the scroll bar is active (aka not degenrate...allow for hover events
            if (sbVal->mnVisibleSize < sbVal->mnMax)
                option.state = QStyle::State_MouseOver;

            bool horizontal = ( part == ControlPart::DrawBackgroundHorz ); //horizontal or vertical
            option.orientation = horizontal ? Qt::Horizontal : Qt::Vertical;
            if( horizontal )
                option.state |= QStyle::State_Horizontal;

            //setup parameters from the OO values
            option.minimum = sbVal->mnMin;
            option.maximum = sbVal->mnMax - sbVal->mnVisibleSize;
            option.maximum = qMax( option.maximum, option.minimum ); // bnc#619772
            option.sliderValue = sbVal->mnCur;
            option.sliderPosition = sbVal->mnCur;
            option.pageStep = sbVal->mnVisibleSize;
            if (part == ControlPart::DrawBackgroundHorz)
                option.upsideDown = sbVal->maButton1Rect.Left() > sbVal->maButton2Rect.Left();

            //setup the active control...always the slider
            if (sbVal->mnThumbState & ControlState::ROLLOVER)
                option.activeSubControls = QStyle::SC_ScrollBarSlider;

            draw( QStyle::CC_ScrollBar, &option, m_image.get(),
                  vclStateValue2StateFlag(nControlState, value) );
        }
        else
        {
            returnVal = false;
        }
    }
    else if (type == ControlType::Spinbox)
    {
        QStyleOptionSpinBox option;
        option.frame = true;

        // determine active control
        if( value.getType() == ControlType::SpinButtons )
        {
            const SpinbuttonValue* pSpinVal = static_cast<const SpinbuttonValue *>(&value);
            if( pSpinVal->mnUpperState & ControlState::PRESSED )
                option.activeSubControls |= QStyle::SC_SpinBoxUp;
            if( pSpinVal->mnLowerState & ControlState::PRESSED )
                option.activeSubControls |= QStyle::SC_SpinBoxDown;
            if( pSpinVal->mnUpperState & ControlState::ENABLED )
                option.stepEnabled |= QAbstractSpinBox::StepUpEnabled;
            if( pSpinVal->mnLowerState & ControlState::ENABLED )
                option.stepEnabled |= QAbstractSpinBox::StepDownEnabled;
            if( pSpinVal->mnUpperState & ControlState::ROLLOVER )
                option.state = QStyle::State_MouseOver;
            if( pSpinVal->mnLowerState & ControlState::ROLLOVER )
                option.state = QStyle::State_MouseOver;
        }

        draw( QStyle::CC_SpinBox, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == ControlType::Radiobutton)
    {
        QStyleOptionButton option;
        draw( QStyle::CE_RadioButton, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == ControlType::Tooltip)
    {
        QStyleOption option;
        draw( QStyle::PE_PanelTipLabel, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == ControlType::Frame)
    {
        lcl_drawFrame( QStyle::PE_Frame, m_image.get(),
                       vclStateValue2StateFlag(nControlState, value) );
        // draw just the border, see http://qa.openoffice.org/issues/show_bug.cgi?id=107945
        int fw = QApplication::style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        localClipRegion = new QRegion(QRegion(widgetRect).subtracted(widgetRect.adjusted(fw, fw, -fw, -fw)));
    }
    else if (type == ControlType::WindowBackground)
    {
        // Nothing to do - see "Default image color" switch ^^
    }
    else if (type == ControlType::Fixedline)
    {
        QStyleOptionMenuItem option;
        option.menuItemType = QStyleOptionMenuItem::Separator;
        option.state |= QStyle::State_Item;

        draw( QStyle::CE_MenuItem, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == ControlType::Slider && (part == ControlPart::TrackHorzArea || part == ControlPart::TrackVertArea))
    {
        OSL_ASSERT( value.getType() == ControlType::Slider );
        const SliderValue* slVal = static_cast<const SliderValue *>(&value);
        QStyleOptionSlider option;

        option.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
        option.state = vclStateValue2StateFlag( nControlState, value );
        option.maximum     = slVal->mnMax;
        option.minimum     = slVal->mnMin;
        option.sliderPosition = option.sliderValue = slVal->mnCur;
        bool horizontal = ( part == ControlPart::TrackHorzArea ); //horizontal or vertical
        option.orientation = horizontal ? Qt::Horizontal : Qt::Vertical;
        if( horizontal )
            option.state |= QStyle::State_Horizontal;

        draw( QStyle::CC_Slider, &option, m_image.get(), vclStateValue2StateFlag(nControlState, value) );
    }
    else if( type == ControlType::Progress && part == ControlPart::Entire )
    {
        QStyleOptionProgressBarV2 option;
        option.minimum = 0;
        option.maximum = widgetRect.width();
        option.progress = value.getNumericVal();
        option.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
        option.state = vclStateValue2StateFlag( nControlState, value );

        draw( QStyle::CE_ProgressBar, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value) );
    }
    else
    {
        returnVal = false;
    }

    if (returnVal)
    {
#ifdef IMAGE_BASED_PAINTING
        // Create a wrapper QPixmap around the destination pixmap, allowing the use of QPainter.
        // Using X11SalGraphics::CopyScreenArea() would require using QPixmap and if Qt uses
        // other graphics system than native, QPixmap::handle() would be 0 (i.e. it wouldn't work),
        // I have no idea how to create QPixmap with non-null handle() in such case, so go this way.
        // See XRegionToQRegion() comment for a small catch (although not real hopefully).
        QPixmap destPixmap = QPixmap::fromX11Pixmap( GetDrawable(), QPixmap::ExplicitlyShared );
        QPainter paint( &destPixmap );
        if (localClipRegion && mpClipRegion)
            paint.setClipRegion(localClipRegion->intersected(XRegionToQRegion(mpClipRegion)));
        else if (localClipRegion)
            paint.setClipRegion(*localClipRegion);
        else if( mpClipRegion )
            paint.setClipRegion( XRegionToQRegion( mpClipRegion ));
        paint.drawImage( widgetRect.left(), widgetRect.top(), *m_image,
            0, 0, widgetRect.width(), widgetRect.height(),
            Qt::ColorOnly | Qt::OrderedDither | Qt::OrderedAlphaDither );
#else
        GC gc = GetFontGC();
        if( gc )
        {
            Region pTempClipRegion = NULL;
            if (localClipRegion)
            {
                pTempClipRegion = XCreateRegion();
                foreach(const QRect& r, localClipRegion->rects())
                {
                    XRectangle xr;
                    xr.x = r.x();
                    xr.y = r.y();
                    xr.width = r.width();
                    xr.height = r.height();
                    XUnionRectWithRegion( &xr, pTempClipRegion, pTempClipRegion );
                }
                if( mpClipRegion )
                    XIntersectRegion( pTempClipRegion, mpClipRegion, pTempClipRegion );
                XSetRegion( GetXDisplay(), gc, pTempClipRegion );
            }
            QPixmap pixmap = QPixmap::fromImage(*m_image, Qt::ColorOnly | Qt::OrderedDither | Qt::OrderedAlphaDither);
            X11SalGraphics::CopyScreenArea( GetXDisplay(),
                pixmap.handle(), pixmap.x11Info().screen(), pixmap.x11Info().depth(),
                GetDrawable(), GetScreenNumber(), GetVisual().GetDepth(),
                gc, 0, 0, widgetRect.width(), widgetRect.height(), widgetRect.left(), widgetRect.top());

            if( pTempClipRegion )
            {
                if( mpClipRegion )
                    XSetRegion( GetXDisplay(), gc, mpClipRegion );
                else
                    XSetClipMask( GetXDisplay(), gc, None );
                XDestroyRegion( pTempClipRegion );
            }
        }
        else
            returnVal = false;
#endif
    }
    delete localClipRegion;
    return returnVal;
}

bool KDESalGraphics::getNativeControlRegion( ControlType type, ControlPart part,
                                             const tools::Rectangle& controlRegion, ControlState controlState,
                                             const ImplControlValue& val,
                                             const OUString&,
                                             tools::Rectangle &nativeBoundingRegion, tools::Rectangle &nativeContentRegion )
{
    bool retVal = false;

    QRect boundingRect = region2QRect( controlRegion );
    QRect contentRect = boundingRect;
    QStyleOptionComplex styleOption;

    switch ( type )
    {
        // Metrics of the push button
        case ControlType::Pushbutton:
            if (part == ControlPart::Entire)
            {
                styleOption.state = vclStateValue2StateFlag(controlState, val);

                if ( controlState & ControlState::DEFAULT )
                {
                    int size = QApplication::style()->pixelMetric(
                        QStyle::PM_ButtonDefaultIndicator, &styleOption );
                    boundingRect.adjust( -size, -size, size, size );
                    retVal = true;
                }
            }
            break;
        case ControlType::Editbox:
        case ControlType::MultilineEditbox:
        {
            QStyleOptionFrameV3 fo;
            fo.frameShape = QFrame::StyledPanel;
            fo.state = QStyle::State_Sunken;
            fo.lineWidth = QApplication::style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
            QSize aMinSize = QApplication::style()->
                sizeFromContents( QStyle::CT_LineEdit, &fo, contentRect.size() );
            if( aMinSize.height() > boundingRect.height() )
            {
                int nHeight = (aMinSize.height() - boundingRect.height()) / 2;
                assert( 0 == (aMinSize.height() - boundingRect.height()) % 2 );
                boundingRect.adjust( 0, -nHeight, 0, nHeight );
            }
            if( aMinSize.width() > boundingRect.width() )
            {
                int nWidth = (aMinSize.width() - boundingRect.width()) / 2;
                assert( 0 == (aMinSize.width() - boundingRect.width()) % 2 );
                boundingRect.adjust( -nWidth, 0, nWidth, 0 );
            }
            retVal = true;
            break;
        }
        case ControlType::Checkbox:
            if (part == ControlPart::Entire)
            {
                styleOption.state = vclStateValue2StateFlag(controlState, val);

                contentRect.setWidth(QApplication::style()->pixelMetric(
                    QStyle::PM_IndicatorWidth, &styleOption));
                contentRect.setHeight(QApplication::style()->pixelMetric(
                    QStyle::PM_IndicatorHeight, &styleOption));

                contentRect.adjust(0, 0,
                    2 * QApplication::style()->pixelMetric(
                        QStyle::PM_FocusFrameHMargin, &styleOption),
                    2 * QApplication::style()->pixelMetric(
                        QStyle::PM_FocusFrameVMargin, &styleOption)
                    );

                boundingRect = contentRect;

                retVal = true;
            }
            break;
        case ControlType::Combobox:
        case ControlType::Listbox:
        {
            QStyleOptionComboBox cbo;

            cbo.rect = QRect(0, 0, contentRect.width(), contentRect.height());
            cbo.state = vclStateValue2StateFlag(controlState, val);

            switch ( part )
            {
                case ControlPart::Entire:
                {
                    // find out the minimum size that should be used
                    // assume contents is a text ling
                    int nHeight = QApplication::fontMetrics().height();
                    QSize aContentSize( contentRect.width(), nHeight );
                    QSize aMinSize = QApplication::style()->
                        sizeFromContents( QStyle::CT_ComboBox, &cbo, aContentSize );
                    if( aMinSize.height() > contentRect.height() )
                        contentRect.adjust( 0, 0, 0, aMinSize.height() - contentRect.height() );
                    boundingRect = contentRect;
                    retVal = true;
                    break;
                }
                case ControlPart::ButtonDown:
                    contentRect = QApplication::style()->subControlRect(
                        QStyle::CC_ComboBox, &cbo, QStyle::SC_ComboBoxArrow );
                    contentRect.translate( boundingRect.left(), boundingRect.top() );
                    retVal = true;
                    break;
                case ControlPart::SubEdit:
                {
                    contentRect = QApplication::style()->subControlRect(
                        QStyle::CC_ComboBox, &cbo, QStyle::SC_ComboBoxEditField );
                    contentRect.translate( boundingRect.left(), boundingRect.top() );
                    retVal = true;
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case ControlType::Spinbox:
        {
            QStyleOptionSpinBox sbo;
            sbo.frame = true;

            sbo.rect = QRect(0, 0, contentRect.width(), contentRect.height());
            sbo.state = vclStateValue2StateFlag(controlState, val);

            switch ( part )
            {
                case ControlPart::Entire:
                {
                    int nHeight = QApplication::fontMetrics().height();
                    QSize aContentSize( contentRect.width(), nHeight );
                    QSize aMinSize = QApplication::style()->
                        sizeFromContents( QStyle::CT_SpinBox, &sbo, aContentSize );
                    if( aMinSize.height() > contentRect.height() )
                        contentRect.adjust( 0, 0, 0, aMinSize.height() - contentRect.height() );
                    boundingRect = contentRect;
                    retVal = true;
                    break;
                }
                case ControlPart::ButtonUp:
                    contentRect = QApplication::style()->subControlRect(
                        QStyle::CC_SpinBox, &sbo, QStyle::SC_SpinBoxUp );
                    contentRect.translate( boundingRect.left(), boundingRect.top() );
                    retVal = true;
                    boundingRect = QRect();
                    break;

                case ControlPart::ButtonDown:
                    contentRect = QApplication::style()->subControlRect(
                        QStyle::CC_SpinBox, &sbo, QStyle::SC_SpinBoxDown );
                    retVal = true;
                    contentRect.translate( boundingRect.left(), boundingRect.top() );
                    boundingRect = QRect();
                    break;

                case ControlPart::SubEdit:
                    contentRect = QApplication::style()->subControlRect(
                        QStyle::CC_SpinBox, &sbo, QStyle::SC_SpinBoxEditField );
                    retVal = true;
                    contentRect.translate( boundingRect.left(), boundingRect.top() );
                    break;
                default:
                    break;
            }
            break;
        }
        case ControlType::MenuPopup:
        {
            int h, w;
            switch ( part ) {
            case ControlPart::MenuItemCheckMark:
                h = QApplication::style()->pixelMetric(QStyle::PM_IndicatorHeight);
                w = QApplication::style()->pixelMetric(QStyle::PM_IndicatorWidth);
                retVal = true;
                break;
            case ControlPart::MenuItemRadioMark:
                h = QApplication::style()->pixelMetric(QStyle::PM_ExclusiveIndicatorHeight);
                w = QApplication::style()->pixelMetric(QStyle::PM_ExclusiveIndicatorWidth);
                retVal = true;
                break;
            default:
                break;
            }
            if (retVal) {
                contentRect = QRect(0, 0, w, h);
                boundingRect = contentRect;
            }
            break;
        }
        case ControlType::Frame:
        {
            if( part == ControlPart::Border )
            {
                auto nStyle = static_cast<DrawFrameFlags>(
                    val.getNumericVal() & 0xFFF0);
                if( nStyle & DrawFrameFlags::NoDraw )
                {
                    int nFrameWidth = QApplication::style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
                    contentRect.adjust(nFrameWidth, nFrameWidth, -nFrameWidth, -nFrameWidth);
                }
                retVal = true;
            }
            break;
        }
        case ControlType::Radiobutton:
        {
            const int h = QApplication::style()->pixelMetric(QStyle::PM_ExclusiveIndicatorHeight);
            const int w = QApplication::style()->pixelMetric(QStyle::PM_ExclusiveIndicatorWidth);

            contentRect = QRect(boundingRect.left(), boundingRect.top(), w, h);
            contentRect.adjust(0, 0,
                2 * QApplication::style()->pixelMetric(
                    QStyle::PM_FocusFrameHMargin, &styleOption),
                2 * QApplication::style()->pixelMetric(
                    QStyle::PM_FocusFrameVMargin, &styleOption)
                );
            boundingRect = contentRect;

            retVal = true;
            break;
        }
        case ControlType::Slider:
        {
            const int w = QApplication::style()->pixelMetric(QStyle::PM_SliderLength);
            if( part == ControlPart::ThumbHorz )
            {
                contentRect = QRect(boundingRect.left(), boundingRect.top(), w, boundingRect.height());
                boundingRect = contentRect;
                retVal = true;
            }
            else if( part == ControlPart::ThumbVert )
            {
                contentRect = QRect(boundingRect.left(), boundingRect.top(), boundingRect.width(), w);
                boundingRect = contentRect;
                retVal = true;
            }
            break;
        }
        case ControlType::Toolbar:
        {
            const int nWorH = QApplication::style()->pixelMetric(QStyle::PM_ToolBarHandleExtent);
            if( part == ControlPart::ThumbHorz )
            {
                contentRect = QRect(boundingRect.left(), boundingRect.top(), boundingRect.width(), nWorH );
                boundingRect = contentRect;
                retVal = true;
            }
            else if( part == ControlPart::ThumbVert )
            {
                contentRect = QRect(boundingRect.left(), boundingRect.top(), nWorH, boundingRect.height() );
                boundingRect = contentRect;
                retVal = true;
            }
            break;
        }
        case ControlType::Scrollbar:
        {
            // core can't handle 3-button scrollbars well, so we fix that in hitTestNativeControl(),
            // for the rest also provide the track area (i.e. area not taken by buttons)
            if( part == ControlPart::TrackVertArea || part == ControlPart::TrackHorzArea )
            {
                QStyleOptionSlider option;
                bool horizontal = ( part == ControlPart::TrackHorzArea ); //horizontal or vertical
                option.orientation = horizontal ? Qt::Horizontal : Qt::Vertical;
                if( horizontal )
                    option.state |= QStyle::State_Horizontal;
                // getNativeControlRegion usually gets ImplControlValue as 'val' (i.e. not the proper
                // subclass), so use random sensible values (doesn't matter anyway, as the wanted
                // geometry here depends only on button sizes)
                option.maximum = 10;
                option.minimum = 0;
                option.sliderPosition = option.sliderValue = 4;
                option.pageStep = 2;
                // Adjust coordinates to make the widget appear to be at (0,0), i.e. make
                // widget and screen coordinates the same. QStyle functions should use screen
                // coordinates but at least QPlastiqueStyle::subControlRect() is buggy
                // and sometimes uses widget coordinates.
                QRect rect = contentRect;
                rect.moveTo( 0, 0 );
                option.rect = rect;
                rect = QApplication::style()->subControlRect( QStyle::CC_ScrollBar, &option,
                    QStyle::SC_ScrollBarGroove );
                rect.translate( contentRect.topLeft()); // reverse the workaround above
                contentRect = boundingRect = rect;
                retVal = true;
            }
            break;
        }
        default:
            break;
    }
    if (retVal)
    {
        // Bounding region
        Point aBPoint( boundingRect.x(), boundingRect.y() );
        Size aBSize( boundingRect.width(), boundingRect.height() );
        nativeBoundingRegion = tools::Rectangle( aBPoint, aBSize );

        // vcl::Region of the content
        Point aPoint( contentRect.x(), contentRect.y() );
        Size  aSize( contentRect.width(), contentRect.height() );
        nativeContentRegion = tools::Rectangle( aPoint, aSize );
    }

    return retVal;
}

/** Test whether the position is in the native widget.
    If the return value is TRUE, bIsInside contains information whether
    aPos was or was not inside the native widget specified by the
    nType/nPart combination.
*/
bool KDESalGraphics::hitTestNativeControl( ControlType nType, ControlPart nPart,
                                           const tools::Rectangle& rControlRegion, const Point& rPos,
                                           bool& rIsInside )
{
    if ( nType == ControlType::Scrollbar )
    {
        if( nPart != ControlPart::ButtonUp && nPart != ControlPart::ButtonDown
            && nPart != ControlPart::ButtonLeft && nPart != ControlPart::ButtonRight )
        { // we adjust only for buttons (because some scrollbars have 3 buttons,
          // and LO core doesn't handle such scrollbars well)
            return FALSE;
        }
        rIsInside = FALSE;
        bool bHorizontal = ( nPart == ControlPart::ButtonLeft || nPart == ControlPart::ButtonRight );
        QRect rect = region2QRect( rControlRegion );
        QPoint pos( rPos.X(), rPos.Y());
        // Adjust coordinates to make the widget appear to be at (0,0), i.e. make
        // widget and screen coordinates the same. QStyle functions should use screen
        // coordinates but at least QPlastiqueStyle::subControlRect() is buggy
        // and sometimes uses widget coordinates.
        pos -= rect.topLeft();
        rect.moveTo( 0, 0 );
        QStyleOptionSlider options;
        options.orientation = bHorizontal ? Qt::Horizontal : Qt::Vertical;
        if( bHorizontal )
            options.state |= QStyle::State_Horizontal;
        options.rect = rect;
        // some random sensible values, since we call this code only for scrollbar buttons,
        // the slider position does not exactly matter
        options.maximum = 10;
        options.minimum = 0;
        options.sliderPosition = options.sliderValue = 4;
        options.pageStep = 2;
        QStyle::SubControl control = QApplication::style()->hitTestComplexControl( QStyle::CC_ScrollBar, &options, pos );
        if( nPart == ControlPart::ButtonUp || nPart == ControlPart::ButtonLeft )
            rIsInside = ( control == QStyle::SC_ScrollBarSubLine );
        else // DOWN, RIGHT
            rIsInside = ( control == QStyle::SC_ScrollBarAddLine );
        return TRUE;
    }
    return FALSE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
