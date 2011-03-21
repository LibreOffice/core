/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#define _SV_SALNATIVEWIDGETS_KDE_CXX

#define Region QtXRegion

#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QFrame>
#include <QLabel>

#include <kapplication.h>
#include <kdebug.h>

#undef Region

#include "KDESalGraphics.hxx"

#include "vcl/settings.hxx"
#include "vcl/decoview.hxx"
#include "rtl/ustrbuf.hxx"

using namespace ::rtl;

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
        ( (nControlState & CTRL_STATE_DEFAULT)?  QStyle::State_None:      QStyle::State_None ) |
        ( (nControlState & CTRL_STATE_ENABLED)?  QStyle::State_Enabled:   QStyle::State_None ) |
        ( (nControlState & CTRL_STATE_FOCUSED)?  QStyle::State_HasFocus:  QStyle::State_None ) |
        ( (nControlState & CTRL_STATE_PRESSED)?  QStyle::State_Sunken:    QStyle::State_None ) |
        ( (nControlState & CTRL_STATE_SELECTED)? QStyle::State_Selected : QStyle::State_None ) |
        ( (nControlState & CTRL_STATE_ROLLOVER)? QStyle::State_MouseOver: QStyle::State_None );
        //TODO ( (nControlState & CTRL_STATE_HIDDEN)?   QStyle::State_:   QStyle::State_None ) |

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

KDESalGraphics::KDESalGraphics() :
    m_image(0)
{
}

KDESalGraphics::~KDESalGraphics()
{
    if (m_image)
        delete m_image;
}

sal_Bool KDESalGraphics::IsNativeControlSupported( ControlType type, ControlPart part )
{
    if (type == CTRL_PUSHBUTTON) return true;

    if (type == CTRL_MENUBAR) return true;

    if (type == CTRL_MENU_POPUP) return true;

    if (type == CTRL_EDITBOX) return true;

    if (type == CTRL_COMBOBOX) return true;

    if (type == CTRL_TOOLBAR) return true;

    if (type == CTRL_CHECKBOX) return true;

    if (type == CTRL_LISTBOX) return true;

    if (type == CTRL_LISTNODE) return true;

    if (type == CTRL_FRAME) return true;

    if (type == CTRL_SCROLLBAR) return true;

    if (type == CTRL_WINDOW_BACKGROUND) return true;

    if (type == CTRL_SPINBOX && (part == PART_ENTIRE_CONTROL || part == HAS_BACKGROUND_TEXTURE) ) return true;

    // no spinbuttons for KDE, paint spinbox complete
    //if (type == CTRL_SPINBUTTONS) return true;

    if (type == CTRL_GROUPBOX) return true;

    if (type == CTRL_FIXEDLINE) return true;

    if (type == CTRL_FIXEDBORDER) return true;

    if (type == CTRL_TOOLTIP) return true;

    if (type == CTRL_RADIOBUTTON) return true;

    if (type == CTRL_SLIDER && (part == PART_TRACK_HORZ_AREA || part == PART_TRACK_VERT_AREA) )
        return true;

    return false;

    if ( (type == CTRL_TAB_ITEM) && (part == PART_ENTIRE_CONTROL) ) return true;
    if ( (type == CTRL_TAB_PANE) && (part == PART_ENTIRE_CONTROL) ) return true;
    // no CTRL_TAB_BODY for KDE
    if ( (type == CTRL_PROGRESS)    && (part == PART_ENTIRE_CONTROL) ) return true;

    return false;
}

/** Test whether the position is in the native widget.
    If the return value is TRUE, bIsInside contains information whether
    aPos was or was not inside the native widget specified by the
    nType/nPart combination.
*/
sal_Bool KDESalGraphics::hitTestNativeControl( ControlType nType, ControlPart nPart,
                                           const Rectangle& rControlRegion, const Point& rPos,
                                           sal_Bool& rIsInside )
{
    if ( nType == CTRL_SCROLLBAR )
    {
        if( nPart != PART_BUTTON_UP && nPart != PART_BUTTON_DOWN
            && nPart != PART_BUTTON_LEFT && nPart != PART_BUTTON_RIGHT )
        { // we adjust only for buttons (because some scrollbars have 3 buttons)
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
        options.rect = rect;
        // some random sensible values, since we call this code only for scrollbar buttons,
        // the slider position does not exactly matter
        options.maximum = 10;
        options.minimum = 0;
        options.sliderPosition = options.sliderValue = 4;
        options.pageStep = 2;
        QStyle::SubControl control = kapp->style()->hitTestComplexControl( QStyle::CC_ScrollBar, &options, pos );
        if( nPart == PART_BUTTON_UP || nPart == PART_BUTTON_LEFT )
            rIsInside = ( control == QStyle::SC_ScrollBarSubLine );
        else // DOWN, RIGHT
            rIsInside = ( control == QStyle::SC_ScrollBarAddLine );
        return TRUE;
    }
    return FALSE;
}

/// helper drawing methods
namespace
{
    void draw( QStyle::ControlElement element, QStyleOption* option, QImage* image, QStyle::State state )
    {
        option->state |= state;
        option->rect = image->rect();

        QPainter painter(image);
        kapp->style()->drawControl(element, option, &painter);
    }

    void draw( QStyle::PrimitiveElement element, QStyleOption* option, QImage* image, QStyle::State state, QRect rect = QRect())
    {
        option->state |= state;
        option->rect = !rect.isNull() ? rect : image->rect();

        QPainter painter(image);
        kapp->style()->drawPrimitive(element, option, &painter);
    }

    void draw( QStyle::ComplexControl element, QStyleOptionComplex* option, QImage* image, QStyle::State state )
    {
        option->state |= state;
        option->rect = image->rect();

        QPainter painter(image);
        kapp->style()->drawComplexControl(element, option, &painter);
    }

    int getFrameWidth()
    {
        static int s_nFrameWidth = -1;
        if( s_nFrameWidth < 0 )
        {
            // fill in a default
            s_nFrameWidth = 2;
            QFrame aFrame( NULL );
            aFrame.setFrameRect( QRect(0, 0, 100, 30) );
            aFrame.setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
            aFrame.ensurePolished();
            s_nFrameWidth = aFrame.frameWidth();
        }
        return s_nFrameWidth;
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
// Officially Xlib's Region is an opaque data type, with only functions for manipulating it.
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
static QRegion XRegionToQRegion( XLIB_Region xr )
{
    QRegion qr;
    for( int i = 0;
         i < xr->numRects;
         ++i )
    {
        BOX& b = xr->rects[ i ];
        qr |= QRect( b.x1, b.y1, b.x2 - b.x1, b.y2 - b.y1 ); // x2,y2 is outside, not the bottom-right corner
    }
    return qr;
}
#endif

sal_Bool KDESalGraphics::drawNativeControl( ControlType type, ControlPart part,
                                        const Rectangle& rControlRegion, ControlState nControlState,
                                        const ImplControlValue& value,
                                        const OUString& )
{
    // put not implemented types here
    if (type == CTRL_SPINBUTTONS)
    {
        return false;
    }

    sal_Bool returnVal = true;

    QRect widgetRect = region2QRect(rControlRegion);
    if( type == CTRL_SPINBOX && part == PART_ALL_BUTTONS )
        type = CTRL_SPINBUTTONS;
    if( type == CTRL_SPINBUTTONS )
    {
        OSL_ASSERT( value.getType() != CTRL_SPINBUTTONS );
        const SpinbuttonValue* pSpinVal = static_cast<const SpinbuttonValue *>(&value);
        Rectangle aButtonRect( pSpinVal->maUpperRect);
        aButtonRect.Union( pSpinVal->maLowerRect );;
        widgetRect = QRect( aButtonRect.Left(), aButtonRect.Top(),
                            aButtonRect.Right(), aButtonRect.Bottom() );
    }

    //if no image, or resized, make a new image
    if (!m_image || m_image->size() != widgetRect.size())
    {
        if (m_image)
            delete m_image;

        m_image = new QImage( widgetRect.width(),
                              widgetRect.height(),
                              QImage::Format_ARGB32 );
    }
    m_image->fill(KApplication::palette().color(QPalette::Window).rgb());

    QRegion* clipRegion = NULL;

    if (type == CTRL_PUSHBUTTON)
    {
        QStyleOptionButton option;
        draw( QStyle::CE_PushButton, &option, m_image,
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if ( (type == CTRL_MENUBAR))
    {
        if (part == PART_MENU_ITEM)
        {
            QStyleOptionMenuItem option;
            draw( QStyle::CE_MenuBarItem, &option, m_image,
                  vclStateValue2StateFlag(nControlState, value) );
        }
        else if (part == PART_ENTIRE_CONTROL)
        {
        }
        else
        {
            returnVal = false;
        }
    }
    else if (type == CTRL_MENU_POPUP)
    {
        if (part == PART_MENU_ITEM)
        {
            QStyleOptionMenuItem option;
            draw( QStyle::CE_MenuItem, &option, m_image,
                  vclStateValue2StateFlag(nControlState, value) );
        }
        else if (part == PART_MENU_ITEM_CHECK_MARK)
        {
            m_image->fill(Qt::transparent);
            if(nControlState & CTRL_STATE_PRESSED) // at least Oxygen paints always as checked
            {
                QStyleOptionButton option;
                draw( QStyle::PE_IndicatorMenuCheckMark, &option, m_image,
                      vclStateValue2StateFlag(nControlState, value));
            }
        }
        else if (part == PART_MENU_ITEM_RADIO_MARK)
        {
            m_image->fill(Qt::transparent);
            QStyleOptionButton option;
            // we get always passed BUTTONVALUE_DONTKNOW in 'value', and the checked
            // state is actually CTRL_STATE_PRESSED
            QStyle::State set = ( nControlState & CTRL_STATE_PRESSED ) ? QStyle::State_On : QStyle::State_Off;
            draw( QStyle::PE_IndicatorRadioButton, &option, m_image,
                  vclStateValue2StateFlag(nControlState, value) | set );
        }
        else
        {
            #if ( QT_VERSION >= QT_VERSION_CHECK( 4, 5, 0 ) )
            QStyleOptionFrameV3 option;
            option.frameShape = QFrame::StyledPanel;
            #else
            QStyleOptionFrameV2 option;
            #endif
            draw( QStyle::PE_FrameMenu, &option, m_image,
                  vclStateValue2StateFlag(nControlState, value) );
        }
    }
    else if ( (type == CTRL_TOOLBAR) && (part == PART_BUTTON) )
    {
        QStyleOptionToolButton option;

        option.arrowType = Qt::NoArrow;
        option.subControls = QStyle::SC_ToolButton;

        option.state = vclStateValue2StateFlag( nControlState, value );
        option.state |= QStyle::State_Raised | QStyle::State_Enabled | QStyle::State_AutoRaise;

        draw( QStyle::CC_ToolButton, &option, m_image,
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if ( (type == CTRL_TOOLBAR) && (part == PART_ENTIRE_CONTROL) )
    {
        QStyleOptionToolBar option;

        option.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
        option.state = vclStateValue2StateFlag( nControlState, value );

        draw( QStyle::CE_ToolBar, &option, m_image,
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if ( (type == CTRL_TOOLBAR) && (part == PART_THUMB_VERT) )
    {   // reduce paint area only to the handle area
        const int width = kapp->style()->pixelMetric(QStyle::PM_ToolBarHandleExtent);
        QRect rect( 0, 0, width, widgetRect.height());
        clipRegion = new QRegion( widgetRect.x(), widgetRect.y(), width, widgetRect.height());

        QStyleOption option;
        option.state = QStyle::State_Horizontal;

        draw( QStyle::PE_IndicatorToolBarHandle, &option, m_image,
              vclStateValue2StateFlag(nControlState, value), rect );
    }
    else if (type == CTRL_EDITBOX)
    {
        QStyleOptionFrameV2 option;
        draw( QStyle::PE_PanelLineEdit, &option, m_image,
              vclStateValue2StateFlag(nControlState, value), m_image->rect().adjusted( 2, 2, -2, -2 ));

        draw( QStyle::PE_FrameLineEdit, &option, m_image,
              vclStateValue2StateFlag(nControlState, value));
    }
    else if (type == CTRL_COMBOBOX)
    {
        QStyleOptionComboBox option;
        option.editable = true;

        draw( QStyle::CC_ComboBox, &option, m_image,
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == CTRL_LISTBOX)
    {
        if( part == PART_WINDOW )
        {
            lcl_drawFrame( QStyle::PE_Frame, m_image,
                           vclStateValue2StateFlag(nControlState, value) );
        }
        else
        {
            QStyleOptionComboBox option;
            if (part == PART_SUB_EDIT)
            {
                draw( QStyle::CE_ComboBoxLabel, &option, m_image,
                      vclStateValue2StateFlag(nControlState, value) );
            }
            else
            {
                draw( QStyle::CC_ComboBox, &option, m_image,
                      vclStateValue2StateFlag(nControlState, value) );
            }
        }
    }
    else if (type == CTRL_LISTNODE)
    {
        QStyleOption option;
        option.state = QStyle::State_Item | QStyle::State_Children;

        if (nControlState & CTRL_STATE_PRESSED)
            option.state |= QStyle::State_Open;

        draw( QStyle::PE_IndicatorBranch, &option, m_image,
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == CTRL_CHECKBOX)
    {
        QStyleOptionButton option;
        draw( QStyle::CE_CheckBox, &option, m_image,
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

            //horizontal or vertical
            if (part == PART_DRAW_BACKGROUND_VERT)
                option.orientation = Qt::Vertical;
            else
                option.state |= QStyle::State_Horizontal;

            //setup parameters from the OO values
            option.minimum = sbVal->mnMin;
            option.maximum = sbVal->mnMax - sbVal->mnVisibleSize;
            option.maximum = qMax( option.maximum, option.minimum ); // bnc#619772
            option.sliderValue = sbVal->mnCur;
            option.sliderPosition = sbVal->mnCur;
            option.pageStep = sbVal->mnVisibleSize;

            //setup the active control...always the slider
            if (sbVal->mnThumbState & CTRL_STATE_ROLLOVER)
                option.activeSubControls = QStyle::SC_ScrollBarSlider;

            draw( QStyle::CC_ScrollBar, &option, m_image,
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
            if( (pSpinVal->mnUpperState & CTRL_STATE_PRESSED) )
                option.activeSubControls |= QStyle::SC_SpinBoxUp;
            if( (pSpinVal->mnLowerState & CTRL_STATE_PRESSED) )
                option.activeSubControls |= QStyle::SC_SpinBoxDown;
        }

        draw( QStyle::CC_SpinBox, &option, m_image,
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == CTRL_GROUPBOX)
    {
        QStyleOptionGroupBox option;
        draw( QStyle::CC_GroupBox, &option, m_image,
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == CTRL_RADIOBUTTON)
    {
        QStyleOptionButton option;
        draw( QStyle::CE_RadioButton, &option, m_image,
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == CTRL_TOOLTIP)
    {
        QStyleOption option;
        draw( QStyle::PE_PanelTipLabel, &option, m_image,
              vclStateValue2StateFlag(nControlState, value) );
    }
    else if (type == CTRL_FRAME)
    {
        lcl_drawFrame( QStyle::PE_Frame, m_image,
                       vclStateValue2StateFlag(nControlState, value) );

        // draw just the border, see http://qa.openoffice.org/issues/show_bug.cgi?id=107945
        int fw = getFrameWidth();
        clipRegion = new QRegion( QRegion( widgetRect ).subtracted( widgetRect.adjusted( fw, fw, -fw, -fw )));
    }
    else if (type == CTRL_FIXEDBORDER)
    {
        lcl_drawFrame( QStyle::PE_FrameWindow, m_image,
                       vclStateValue2StateFlag(nControlState, value) );
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

        draw( QStyle::CE_MenuItem, &option, m_image,
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
        option.orientation = (part == PART_TRACK_HORZ_AREA) ? Qt::Horizontal : Qt::Vertical;

        draw( QStyle::CC_Slider, &option, m_image, vclStateValue2StateFlag(nControlState, value) );
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
        GC gc = SelectFont();
        if( gc )
        {
            XLIB_Region pTempClipRegion = NULL;
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

sal_Bool KDESalGraphics::getNativeControlRegion( ControlType type, ControlPart part,
                                             const Rectangle& controlRegion, ControlState controlState,
                                             const ImplControlValue& val,
                                             const OUString&,
                                             Rectangle &nativeBoundingRegion, Rectangle &nativeContentRegion )
{
    sal_Bool retVal = false;

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

                if ( controlState & CTRL_STATE_DEFAULT )
                {
                    int size = kapp->style()->pixelMetric(
                        QStyle::PM_ButtonDefaultIndicator, &styleOption );

                    boundingRect.adjust( -size, -size, size, size );

                    retVal = true;
                }
            }
            break;
        case CTRL_EDITBOX:
        {
            int nFontHeight    = kapp->fontMetrics().height();
            //int nFrameSize     = kapp->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
            int nLayoutTop     = kapp->style()->pixelMetric(QStyle::PM_LayoutTopMargin);
            int nLayoutBottom  = kapp->style()->pixelMetric(QStyle::PM_LayoutBottomMargin);
            int nLayoutLeft    = kapp->style()->pixelMetric(QStyle::PM_LayoutLeftMargin);
            int nLayoutRight   = kapp->style()->pixelMetric(QStyle::PM_LayoutRightMargin);

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

                contentRect.setWidth(kapp->style()->pixelMetric(
                    QStyle::PM_IndicatorWidth, &styleOption));
                contentRect.setHeight(kapp->style()->pixelMetric(
                    QStyle::PM_IndicatorHeight, &styleOption));

                contentRect.adjust(0, 0,
                    2 * kapp->style()->pixelMetric(
                        QStyle::PM_FocusFrameHMargin, &styleOption),
                    2 * kapp->style()->pixelMetric(
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
                    int size = kapp->style()->pixelMetric(QStyle::PM_ComboBoxFrameWidth) - 2;

                    // find out the minimum size that should be used
                    // assume contents is a text ling
                    int nHeight = kapp->fontMetrics().height();
                    QSize aContentSize( contentRect.width(), nHeight );
                    QSize aMinSize = kapp->style()->
                        sizeFromContents( QStyle::CT_ComboBox, &cbo, aContentSize );
                    if( aMinSize.height() > contentRect.height() )
                        contentRect.adjust( 0, 0, 0, aMinSize.height() - contentRect.height() );
                    boundingRect = contentRect;
                    // FIXME: why this difference between comboboxes and listboxes ?
                    // because a combobox has a sub edit and that is positioned
                    // inside the outer bordered control ?
                    if( type == CTRL_COMBOBOX )
                        contentRect.adjust(-size,-size,size,size);
                    retVal = true;
                    break;
                }
                case PART_BUTTON_DOWN:
                    //the entire control can be used as the "down" button
                    retVal = true;
                    break;
                case PART_SUB_EDIT:
                    contentRect = kapp->style()->subControlRect(
                        QStyle::CC_ComboBox, &cbo, QStyle::SC_ComboBoxEditField );

                    contentRect.translate( boundingRect.left(), boundingRect.top() );

                    retVal = true;
                    break;
                case PART_WINDOW:
                    retVal = true;
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
                    contentRect = kapp->style()->subControlRect(
                        QStyle::CC_SpinBox, &sbo, QStyle::SC_SpinBoxUp );
                    contentRect.translate( boundingRect.left(), boundingRect.top() );
                    retVal = true;
                    boundingRect = QRect();
                    break;

                case PART_BUTTON_DOWN:
                    contentRect = kapp->style()->subControlRect(
                        QStyle::CC_SpinBox, &sbo, QStyle::SC_SpinBoxDown );
                    retVal = true;
                    contentRect.translate( boundingRect.left(), boundingRect.top() );
                    boundingRect = QRect();
                    break;

                case PART_SUB_EDIT:
                    contentRect = kapp->style()->subControlRect(
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
            //just limit the widget of the menu items
            //OO isn't very flexible in all reguards with the menu
            //so we do the best we can
            if (part == PART_MENU_ITEM_CHECK_MARK)
            {
                contentRect.setWidth(contentRect.height());
                retVal = true;
            }
            else if (part == PART_MENU_ITEM_RADIO_MARK)
            {
                contentRect.setWidth(contentRect.height());
                retVal = true;
            }
            break;
        case CTRL_FRAME:
        {
            if( part == PART_BORDER )
            {
                int nFrameWidth = getFrameWidth();
                sal_uInt16 nStyle = val.getNumericVal();
                if( nStyle & FRAME_DRAW_NODRAW )
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
            const int h = kapp->style()->pixelMetric(QStyle::PM_ExclusiveIndicatorHeight);
            const int w = kapp->style()->pixelMetric(QStyle::PM_ExclusiveIndicatorWidth);

            contentRect = QRect(boundingRect.left(), boundingRect.top(), w, h);
            contentRect.adjust(0, 0,
                2 * kapp->style()->pixelMetric(
                    QStyle::PM_FocusFrameHMargin, &styleOption),
                2 * kapp->style()->pixelMetric(
                    QStyle::PM_FocusFrameVMargin, &styleOption)
                );
            boundingRect = contentRect;

            retVal = true;
            break;
        }
        case CTRL_SLIDER:
        {
            const int w = kapp->style()->pixelMetric(QStyle::PM_SliderLength);
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
        default:
            break;
    }
    if (retVal)
    {
        // Bounding region
        Point aBPoint( boundingRect.x(), boundingRect.y() );
        Size aBSize( boundingRect.width(), boundingRect.height() );
        nativeBoundingRegion = Rectangle( aBPoint, aBSize );

        // Region of the content
        Point aPoint( contentRect.x(), contentRect.y() );
        Size  aSize( contentRect.width(), contentRect.height() );
        nativeContentRegion = Rectangle( aPoint, aSize );
    }

    return retVal;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
