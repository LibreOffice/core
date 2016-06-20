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

#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QFrame>
#include <QLabel>

#include <kapplication.h>
#include <kdebug.h>

#undef Region

#include "KDESalGraphics.hxx"
#include "KDESalInstance.hxx"

#include <vcl/settings.hxx>
#include <vcl/decoview.hxx>
#include <rtl/ustrbuf.hxx>
#include <unx/saldata.hxx>

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
        case BUTTONVALUE_ON:    nState |= QStyle::State_On;       break;
        case BUTTONVALUE_OFF:   nState |= QStyle::State_Off;      break;
        case BUTTONVALUE_MIXED: nState |= QStyle::State_NoChange; break;
        default: break;
    }

    return nState;
}

/**
 Convert VCL Rectangle to QRect.
 @param rControlRegion The Rectangle to convert.
 @return The matching QRect
*/
QRect region2QRect( const Rectangle& rControlRegion )
{
    return QRect(rControlRegion.Left(), rControlRegion.Top(), rControlRegion.GetWidth(), rControlRegion.GetHeight());
}

bool KDESalGraphics::IsNativeControlSupported( ControlType type, ControlPart part )
{
    switch (type)
    {
        case CTRL_PUSHBUTTON:
        case CTRL_RADIOBUTTON:
        case CTRL_CHECKBOX:
        case CTRL_TOOLTIP:
        case CTRL_PROGRESS:
        case CTRL_LISTNODE:
            return (part == PART_ENTIRE_CONTROL);

        case CTRL_MENUBAR:
        case CTRL_MENU_POPUP:
        case CTRL_EDITBOX:
        case CTRL_COMBOBOX:
        case CTRL_TOOLBAR:
        case CTRL_FRAME:
        case CTRL_SCROLLBAR:
        case CTRL_WINDOW_BACKGROUND:
        case CTRL_GROUPBOX:
        case CTRL_FIXEDLINE:
            return true;

        case CTRL_LISTBOX:
            return (part == PART_ENTIRE_CONTROL || part == HAS_BACKGROUND_TEXTURE);

        case CTRL_SPINBOX:
            return (part == PART_ENTIRE_CONTROL || part == HAS_BACKGROUND_TEXTURE);

        case CTRL_SLIDER:
            return (part == PART_TRACK_HORZ_AREA || part == PART_TRACK_VERT_AREA);

        default:
            break;
    }

    return false;
}

/// helper drawing methods
namespace
{
    void draw( QStyle::ControlElement element, QStyleOption* option, QImage* image, QStyle::State state, QRect rect = QRect())
    {
        option->state |= state;
        option->rect = !rect.isNull() ? rect : image->rect();

        QPainter painter(image);
        QApplication::style()->drawControl(element, option, &painter);
    }

    void draw( QStyle::PrimitiveElement element, QStyleOption* option, QImage* image, QStyle::State state, QRect rect = QRect())
    {
        option->state |= state;
        option->rect = !rect.isNull() ? rect : image->rect();

        QPainter painter(image);
        QApplication::style()->drawPrimitive(element, option, &painter);
    }

    void draw( QStyle::ComplexControl element, QStyleOptionComplex* option, QImage* image, QStyle::State state )
    {
        option->state |= state;
        option->rect = image->rect();

        QPainter painter(image);
        QApplication::style()->drawComplexControl(element, option, &painter);
    }

    void lcl_drawFrame(QStyle::PrimitiveElement element, QImage* image, QStyle::State state)
    {
    #if ( QT_VERSION >= QT_VERSION_CHECK( 4, 5, 0 ) )
        QStyleOptionFrameV3 option;
        option.frameShape = QFrame::StyledPanel;
        option.state = QStyle::State_Sunken;
    #else
        QStyleOptionFrame option;

        QFrame aFrame( NULL );
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
                                        const Rectangle& rControlRegion, ControlState nControlState,
                                        const ImplControlValue& value,
                                        const OUString& )
{
    bool nativeSupport = IsNativeControlSupported( type, part );
    if( ! nativeSupport ) {
        assert( ! nativeSupport && "drawNativeControl called without native support!" );
        return false;
    }

    if( lastPopupRect.isValid() && ( type != CTRL_MENU_POPUP || part != PART_MENU_ITEM ))
        lastPopupRect = QRect();

    bool returnVal = true;

    QRect widgetRect = region2QRect(rControlRegion);
    if( type == CTRL_SPINBOX && part == PART_ALL_BUTTONS )
        type = CTRL_SPINBUTTONS;
    if( type == CTRL_SPINBUTTONS )
    {
        OSL_ASSERT( value.getType() != CTRL_SPINBUTTONS );
        const SpinbuttonValue* pSpinVal = static_cast<const SpinbuttonValue *>(&value);
        Rectangle aButtonRect( pSpinVal->maUpperRect);
        aButtonRect.Union( pSpinVal->maLowerRect );
        widgetRect = QRect( aButtonRect.Left(), aButtonRect.Top(),
                            aButtonRect.Right(), aButtonRect.Bottom() );
    }

    //if no image, or resized, make a new image
    if (!m_image || m_image->size() != widgetRect.size())
    {
        m_image.reset(new QImage( widgetRect.width(), widgetRect.height(), QImage::Format_ARGB32 ) );
    }
    m_image->fill(KApplication::palette().color(QPalette::Window).rgb());

    QRegion* clipRegion = nullptr;

    if (type == CTRL_PUSHBUTTON)
    {
        m_image->fill( Qt::transparent );
        QStyleOptionButton option;
        draw( QStyle::CE_PushButton, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == CTRL_MENUBAR)
    {
        if (part == PART_MENU_ITEM)
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
        else if (part == PART_ENTIRE_CONTROL)
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
    else if (type == CTRL_MENU_POPUP)
    {
        OSL_ASSERT( part == PART_MENU_ITEM ? lastPopupRect.isValid() : !lastPopupRect.isValid());
        if( part == PART_MENU_ITEM )
        {
            QStyleOptionMenuItem option;
            draw( QStyle::CE_MenuItem, &option, m_image.get(),
                  vclStateValue2StateFlag(nControlState, value) );
            // HACK: LO core first paints the entire popup and only then it paints menu items,
            // but QMenu::paintEvent() paints popup frame after all items. That means highlighted
            // items here would paint the highlight over the frame border. Since calls to PART_MENU_ITEM
            // are always preceded by calls to PART_ENTIRE_CONTROL, just remember the size for the whole
            // popup (otherwise not possible to get here) and draw the border afterwards.
            QRect framerect( lastPopupRect.topLeft() - widgetRect.topLeft(),
                widgetRect.size().expandedTo( lastPopupRect.size()));
            QStyleOptionFrame frame;
            draw( QStyle::PE_FrameMenu, &frame, m_image.get(), vclStateValue2StateFlag( nControlState, value ), framerect );
        }
        else if( part == PART_MENU_SEPARATOR )
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
            clipRegion = new QRegion( rect.translated( widgetRect.topLeft()).adjusted( fw, 0, -fw, 0 ));
            draw( QStyle::CE_MenuItem, &option, m_image.get(),
                  vclStateValue2StateFlag(nControlState, value), rect );
        }
        else if( part == PART_MENU_ITEM_CHECK_MARK || part == PART_MENU_ITEM_RADIO_MARK )
        {
            QStyleOptionMenuItem option;
            option.checkType = ( part == PART_MENU_ITEM_CHECK_MARK )
                ? QStyleOptionMenuItem::NonExclusive : QStyleOptionMenuItem::Exclusive;
            option.checked = bool( nControlState & ControlState::PRESSED );
            // widgetRect is now the rectangle for the checkbox/radiobutton itself, but Qt
            // paints the whole menu item, so translate position (and it'll be clipped);
            // it is also necessary to fill the background transparently first, as this
            // is painted after menuitem highlight, otherwise there would be a grey area
            assert( value.getType() == CTRL_MENU_POPUP );
            const MenupopupValue* menuVal = static_cast<const MenupopupValue*>(&value);
            QRect menuItemRect( region2QRect( menuVal->maItemRect ));
            QRect rect( menuItemRect.topLeft() - widgetRect.topLeft(),
                widgetRect.size().expandedTo( menuItemRect.size()));
            m_image->fill( Qt::transparent );
            draw( QStyle::CE_MenuItem, &option, m_image.get(),
                  vclStateValue2StateFlag(nControlState, value), rect );
        }
        else if( part == PART_ENTIRE_CONTROL )
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
    else if ( (type == CTRL_TOOLBAR) && (part == PART_BUTTON) )
    {
        m_image->fill( Qt::transparent );
        QStyleOptionToolButton option;

        option.arrowType = Qt::NoArrow;
        option.subControls = QStyle::SC_ToolButton;

        option.state = vclStateValue2StateFlag( nControlState, value );
        option.state |= QStyle::State_Raised | QStyle::State_Enabled | QStyle::State_AutoRaise;

        draw( QStyle::CC_ToolButton, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if ( (type == CTRL_TOOLBAR) && (part == PART_ENTIRE_CONTROL) )
    {
        QStyleOptionToolBar option;

        option.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
        option.state = vclStateValue2StateFlag( nControlState, value );

        draw( QStyle::CE_ToolBar, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if ( (type == CTRL_TOOLBAR) && (part == PART_THUMB_VERT) )
    {   // reduce paint area only to the handle area
        const int width = QApplication::style()->pixelMetric(QStyle::PM_ToolBarHandleExtent);
        QRect rect( 0, 0, width, widgetRect.height());
        clipRegion = new QRegion( widgetRect.x(), widgetRect.y(), width, widgetRect.height());

        QStyleOption option;
        option.state = QStyle::State_Horizontal;

        draw( QStyle::PE_IndicatorToolBarHandle, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value), rect );
    }
    else if (type == CTRL_EDITBOX)
    {
        QStyleOptionFrameV2 option;
        draw( QStyle::PE_PanelLineEdit, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value), m_image->rect().adjusted( 2, 2, -2, -2 ));

        draw( QStyle::PE_FrameLineEdit, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value));
    }
    else if (type == CTRL_COMBOBOX)
    {
        QStyleOptionComboBox option;
        option.editable = true;

        draw( QStyle::CC_ComboBox, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == CTRL_LISTBOX)
    {
        QStyleOptionComboBox option;
        switch (part) {
            case PART_WINDOW:
                lcl_drawFrame( QStyle::PE_Frame, m_image.get(),
                               vclStateValue2StateFlag(nControlState, value) );
                break;
            case PART_SUB_EDIT:
                draw( QStyle::CE_ComboBoxLabel, &option, m_image.get(),
                      vclStateValue2StateFlag(nControlState, value) );
                break;
            case PART_ENTIRE_CONTROL:
                draw( QStyle::CC_ComboBox, &option, m_image.get(),
                      vclStateValue2StateFlag(nControlState, value) );
                break;
            case PART_BUTTON_DOWN:
                m_image->fill( Qt::transparent );
                option.subControls = QStyle::SC_ComboBoxArrow;
                draw( QStyle::CC_ComboBox, &option, m_image.get(),
                      vclStateValue2StateFlag(nControlState, value) );
                break;
            default:
                break;
        }
    }
    else if (type == CTRL_LISTNODE)
    {
        m_image->fill( Qt::transparent );
        QStyleOption option;
        option.state = QStyle::State_Item | QStyle::State_Children;

        if (value.getTristateVal() == BUTTONVALUE_ON)
            option.state |= QStyle::State_Open;

        draw( QStyle::PE_IndicatorBranch, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == CTRL_CHECKBOX)
    {
        m_image->fill( Qt::transparent );
        QStyleOptionButton option;
        draw( QStyle::CE_CheckBox, &option, m_image.get(),
               vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == CTRL_SCROLLBAR)
    {
        if ((part == PART_DRAW_BACKGROUND_VERT) || (part == PART_DRAW_BACKGROUND_HORZ))
        {
            QStyleOptionSlider option;
            OSL_ASSERT( value.getType() == CTRL_SCROLLBAR );
            const ScrollbarValue* sbVal = static_cast<const ScrollbarValue *>(&value);

            //if the scroll bar is active (aka not degenrate...allow for hover events
            if (sbVal->mnVisibleSize < sbVal->mnMax)
                option.state = QStyle::State_MouseOver;

            bool horizontal = ( part == PART_DRAW_BACKGROUND_HORZ ); //horizontal or vertical
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
            if (part == PART_DRAW_BACKGROUND_HORZ)
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
    else if (type == CTRL_SPINBOX)
    {
        QStyleOptionSpinBox option;

        // determine active control
        if( value.getType() == CTRL_SPINBUTTONS )
        {
            const SpinbuttonValue* pSpinVal = static_cast<const SpinbuttonValue *>(&value);
            if( (pSpinVal->mnUpperState & ControlState::PRESSED) )
                option.activeSubControls |= QStyle::SC_SpinBoxUp;
            if( (pSpinVal->mnLowerState & ControlState::PRESSED) )
                option.activeSubControls |= QStyle::SC_SpinBoxDown;
        }

        draw( QStyle::CC_SpinBox, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == CTRL_GROUPBOX)
    {
        QStyleOptionGroupBox option;
        draw( QStyle::CC_GroupBox, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == CTRL_RADIOBUTTON)
    {
        m_image->fill( Qt::transparent );
        QStyleOptionButton option;
        draw( QStyle::CE_RadioButton, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == CTRL_TOOLTIP)
    {
        QStyleOption option;
        draw( QStyle::PE_PanelTipLabel, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == CTRL_FRAME)
    {
        lcl_drawFrame( QStyle::PE_Frame, m_image.get(),
                       vclStateValue2StateFlag(nControlState, value) );

        // draw just the border, see http://qa.openoffice.org/issues/show_bug.cgi?id=107945
        int fw = static_cast< KDESalInstance* >(GetSalData()->m_pInstance)->getFrameWidth();
        clipRegion = new QRegion( QRegion( widgetRect ).subtracted( widgetRect.adjusted( fw, fw, -fw, -fw )));
    }
    else if (type == CTRL_WINDOW_BACKGROUND)
    {
        m_image->fill(KApplication::palette().color(QPalette::Window).rgb());
    }
    else if (type == CTRL_FIXEDLINE)
    {
        QStyleOptionMenuItem option;
        option.menuItemType = QStyleOptionMenuItem::Separator;
        option.state |= QStyle::State_Item;

        draw( QStyle::CE_MenuItem, &option, m_image.get(),
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == CTRL_SLIDER && (part == PART_TRACK_HORZ_AREA || part == PART_TRACK_VERT_AREA))
    {
        OSL_ASSERT( value.getType() == CTRL_SLIDER );
        const SliderValue* slVal = static_cast<const SliderValue *>(&value);
        QStyleOptionSlider option;

        option.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
        option.state = vclStateValue2StateFlag( nControlState, value );
        option.maximum     = slVal->mnMax;
        option.minimum     = slVal->mnMin;
        option.sliderPosition = option.sliderValue = slVal->mnCur;
        bool horizontal = ( part == PART_TRACK_HORZ_AREA ); //horizontal or vertical
        option.orientation = horizontal ? Qt::Horizontal : Qt::Vertical;
        if( horizontal )
            option.state |= QStyle::State_Horizontal;

        draw( QStyle::CC_Slider, &option, m_image.get(), vclStateValue2StateFlag(nControlState, value) );
    }
    else if( type == CTRL_PROGRESS && part == PART_ENTIRE_CONTROL )
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
        if( clipRegion && mpClipRegion )
            paint.setClipRegion( clipRegion->intersected( XRegionToQRegion( mpClipRegion )));
        else if( clipRegion )
            paint.setClipRegion( *clipRegion );
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
            if( clipRegion )
            {
                pTempClipRegion = XCreateRegion();
                foreach( const QRect& r, clipRegion->rects())
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
    delete clipRegion;
    return returnVal;
}

bool KDESalGraphics::getNativeControlRegion( ControlType type, ControlPart part,
                                             const Rectangle& controlRegion, ControlState controlState,
                                             const ImplControlValue& val,
                                             const OUString&,
                                             Rectangle &nativeBoundingRegion, Rectangle &nativeContentRegion )
{
    bool retVal = false;

    QRect boundingRect = region2QRect( controlRegion );
    QRect contentRect = boundingRect;
    QStyleOptionComplex styleOption;

    switch ( type )
    {
        // Metrics of the push button
        case CTRL_PUSHBUTTON:
            if (part == PART_ENTIRE_CONTROL)
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
        case CTRL_EDITBOX:
        {
            int nFontHeight    = QApplication::fontMetrics().height();
            //int nFrameSize     = QApplication::style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
            int nLayoutTop     = QApplication::style()->pixelMetric(QStyle::PM_LayoutTopMargin);
            int nLayoutBottom  = QApplication::style()->pixelMetric(QStyle::PM_LayoutBottomMargin);
            int nLayoutLeft    = QApplication::style()->pixelMetric(QStyle::PM_LayoutLeftMargin);
            int nLayoutRight   = QApplication::style()->pixelMetric(QStyle::PM_LayoutRightMargin);

            int nMinHeight = (nFontHeight + nLayoutTop + nLayoutBottom);
            if( boundingRect.height() < nMinHeight )
            {
                int delta = nMinHeight - boundingRect.height();
                boundingRect.adjust( 0, 0, 0, delta );
            }
            contentRect = boundingRect;
            contentRect.adjust( -nLayoutLeft+1, -nLayoutTop+1, nLayoutRight-1, nLayoutBottom-1 );
            retVal = true;

            break;
        }
        case CTRL_CHECKBOX:
            if (part == PART_ENTIRE_CONTROL)
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

                break;
            }
        case CTRL_COMBOBOX:
        case CTRL_LISTBOX:
        {
            QStyleOptionComboBox cbo;

            cbo.rect = QRect(0, 0, contentRect.width(), contentRect.height());
            cbo.state = vclStateValue2StateFlag(controlState, val);

            switch ( part )
            {
                case PART_ENTIRE_CONTROL:
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
                    // FIXME: why this difference between comboboxes and listboxes ?
                    // because a combobox has a sub edit and that is positioned
                    // inside the outer bordered control ?
                    if( type == CTRL_COMBOBOX ) {
                        int size = QApplication::style()->pixelMetric(QStyle::PM_ComboBoxFrameWidth) - 2;
                        contentRect.adjust(-size,-size,size,size);
                    }
                    else {
                        int hmargin = QApplication::style()->pixelMetric(
                                QStyle::PM_FocusFrameHMargin, &styleOption);
                        int vmargin = QApplication::style()->pixelMetric(
                                QStyle::PM_FocusFrameVMargin, &styleOption);
                        boundingRect.translate( -hmargin, -vmargin );
                        boundingRect.adjust( -hmargin, -vmargin, 2 * hmargin, 2 * vmargin );
                    }
                    retVal = true;
                    break;
                }
                case PART_BUTTON_DOWN:
                    contentRect = QApplication::style()->subControlRect(
                        QStyle::CC_ComboBox, &cbo, QStyle::SC_ComboBoxArrow );

                    contentRect.translate( boundingRect.left(), boundingRect.top() );

                    retVal = true;
                    break;
                case PART_SUB_EDIT:
                {
                    contentRect = QApplication::style()->subControlRect(
                        QStyle::CC_ComboBox, &cbo, QStyle::SC_ComboBoxEditField );

                    int hmargin = QApplication::style()->pixelMetric(
                            QStyle::PM_FocusFrameHMargin, &styleOption);
                    int vmargin = QApplication::style()->pixelMetric(
                            QStyle::PM_FocusFrameVMargin, &styleOption);

                    contentRect.translate( boundingRect.left() + hmargin, boundingRect.top() + vmargin );
                    contentRect.adjust( 0, 0, -2 * hmargin, -2 * vmargin );

                    retVal = true;
                    break;
                }
                case PART_WINDOW:
                    retVal = true;
                    break;
                default:
                    break;
            }
            break;
        }
        case CTRL_SPINBOX:
        {
            QStyleOptionSpinBox sbo;

            sbo.rect = QRect(0, 0, contentRect.width(), contentRect.height());
            sbo.state = vclStateValue2StateFlag(controlState, val);

            switch ( part )
            {
                case PART_BUTTON_UP:
                    contentRect = QApplication::style()->subControlRect(
                        QStyle::CC_SpinBox, &sbo, QStyle::SC_SpinBoxUp );
                    contentRect.translate( boundingRect.left(), boundingRect.top() );
                    retVal = true;
                    boundingRect = QRect();
                    break;

                case PART_BUTTON_DOWN:
                    contentRect = QApplication::style()->subControlRect(
                        QStyle::CC_SpinBox, &sbo, QStyle::SC_SpinBoxDown );
                    retVal = true;
                    contentRect.translate( boundingRect.left(), boundingRect.top() );
                    boundingRect = QRect();
                    break;

                case PART_SUB_EDIT:
                    contentRect = QApplication::style()->subControlRect(
                        QStyle::CC_SpinBox, &sbo, QStyle::SC_SpinBoxEditField );
                    retVal = true;
                    contentRect.translate( boundingRect.left(), boundingRect.top() );
                    break;
                default:
                    retVal = true;
            }
            break;
        }
        case CTRL_MENU_POPUP:
        {
            int h, w;
            switch ( part ) {
            case PART_MENU_ITEM_CHECK_MARK:
                h = QApplication::style()->pixelMetric(QStyle::PM_IndicatorHeight);
                w = QApplication::style()->pixelMetric(QStyle::PM_IndicatorWidth);
                retVal = true;
                break;
            case PART_MENU_ITEM_RADIO_MARK:
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
        case CTRL_FRAME:
        {
            if( part == PART_BORDER )
            {
                int nFrameWidth = static_cast< KDESalInstance* >(GetSalData()->m_pInstance)->getFrameWidth();
                auto nStyle = static_cast<DrawFrameFlags>(
                    val.getNumericVal() & 0xFFF0);
                if( nStyle & DrawFrameFlags::NoDraw )
                {
                    // in this case the question is: how thick would a frame be
                    // see brdwin.cxx, decoview.cxx
                    // most probably the behavior in decoview.cxx is wrong.
                    contentRect.adjust(nFrameWidth, nFrameWidth, -nFrameWidth, -nFrameWidth);
                }
                retVal = true;
            }
            break;
        }
        case CTRL_RADIOBUTTON:
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
        case CTRL_SLIDER:
        {
            const int w = QApplication::style()->pixelMetric(QStyle::PM_SliderLength);
            if( part == PART_THUMB_HORZ )
            {
                contentRect = QRect(boundingRect.left(), boundingRect.top(), w, boundingRect.height());
                boundingRect = contentRect;
                retVal = true;
            }
            else if( part == PART_THUMB_VERT )
            {
                contentRect = QRect(boundingRect.left(), boundingRect.top(), boundingRect.width(), w);
                boundingRect = contentRect;
                retVal = true;
            }
            break;
        }
        case CTRL_SCROLLBAR:
        {
            // core can't handle 3-button scrollbars well, so we fix that in hitTestNativeControl(),
            // for the rest also provide the track area (i.e. area not taken by buttons)
            if( part == PART_TRACK_VERT_AREA || part == PART_TRACK_HORZ_AREA )
            {
                QStyleOptionSlider option;
                bool horizontal = ( part == PART_TRACK_HORZ_AREA ); //horizontal or vertical
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
        }
        default:
            break;
    }
    if (retVal)
    {
        // Bounding region
        Point aBPoint( boundingRect.x(), boundingRect.y() );
        Size aBSize( boundingRect.width(), boundingRect.height() );
        nativeBoundingRegion = Rectangle( aBPoint, aBSize );

        // vcl::Region of the content
        Point aPoint( contentRect.x(), contentRect.y() );
        Size  aSize( contentRect.width(), contentRect.height() );
        nativeContentRegion = Rectangle( aPoint, aSize );
    }

    return retVal;
}

/** Test whether the position is in the native widget.
    If the return value is TRUE, bIsInside contains information whether
    aPos was or was not inside the native widget specified by the
    nType/nPart combination.
*/
bool KDESalGraphics::hitTestNativeControl( ControlType nType, ControlPart nPart,
                                           const Rectangle& rControlRegion, const Point& rPos,
                                           bool& rIsInside )
{
    if ( nType == CTRL_SCROLLBAR )
    {
        if( nPart != PART_BUTTON_UP && nPart != PART_BUTTON_DOWN
            && nPart != PART_BUTTON_LEFT && nPart != PART_BUTTON_RIGHT )
        { // we adjust only for buttons (because some scrollbars have 3 buttons,
          // and LO core doesn't handle such scrollbars well)
            return FALSE;
        }
        rIsInside = FALSE;
        bool bHorizontal = ( nPart == PART_BUTTON_LEFT || nPart == PART_BUTTON_RIGHT );
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
        if( nPart == PART_BUTTON_UP || nPart == PART_BUTTON_LEFT )
            rIsInside = ( control == QStyle::SC_ScrollBarSubLine );
        else // DOWN, RIGHT
            rIsInside = ( control == QStyle::SC_ScrollBarAddLine );
        return TRUE;
    }
    return FALSE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
