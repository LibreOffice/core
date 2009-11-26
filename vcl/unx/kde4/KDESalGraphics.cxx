/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2009 by Sun Microsystems, Inc.
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
 Convert VCL Region to QRect.
 @param rControlRegion The region to convert.
 @return The bounding box of the region.
*/
QRect region2QRect( const Region& rControlRegion )
{
    Rectangle aRect = rControlRegion.GetBoundRect();

    return QRect( QPoint( aRect.Left(), aRect.Top() ),
          QPoint( aRect.Right(), aRect.Bottom() ) );
}

BOOL KDESalGraphics::IsNativeControlSupported( ControlType type, ControlPart part )
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

    return false;

    if ( (type == CTRL_TAB_ITEM) && (part == PART_ENTIRE_CONTROL) ) return true;
    if ( (type == CTRL_TAB_PANE) && (part == PART_ENTIRE_CONTROL) ) return true;
    // no CTRL_TAB_BODY for KDE
    if ( (type == CTRL_PROGRESS)    && (part == PART_ENTIRE_CONTROL) ) return true;

    return false;
}


BOOL KDESalGraphics::hitTestNativeControl( ControlType, ControlPart,
                                           const Region&, const Point&,
                                           SalControlHandle&, BOOL& )
{
    return FALSE;
}

void lcl_drawFrame( QRect& i_rRect, QPainter& i_rPainter, QStyle::PrimitiveElement i_nElement,
                    ControlState i_nState, const ImplControlValue& i_rValue )
{
    #if ( QT_VERSION >= QT_VERSION_CHECK( 4, 5, 0 ) )
    QStyleOptionFrameV3 styleOption;
    styleOption.frameShape = QFrame::StyledPanel;
    #else
    QStyleOptionFrame styleOption;
    QFrame aFrame( NULL );
    aFrame.setFrameRect( QRect(0, 0, i_rRect.width(), i_rRect.height()) );
    aFrame.setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    aFrame.ensurePolished();
    styleOption.initFrom( &aFrame );
    styleOption.lineWidth = aFrame.lineWidth();
    styleOption.midLineWidth = aFrame.midLineWidth();
    #endif
    styleOption.rect = QRect(0, 0, i_rRect.width(), i_rRect.height());
    styleOption.state = vclStateValue2StateFlag( i_nState, i_rValue );
    #if ( QT_VERSION < QT_VERSION_CHECK( 4, 5, 0 ) )
    styleOption.state |= QStyle::State_Sunken;
    #endif
    kapp->style()->drawPrimitive(i_nElement, &styleOption, &i_rPainter);
}

BOOL KDESalGraphics::drawNativeControl( ControlType type, ControlPart part,
                                        const Region& rControlRegion, ControlState nControlState,
                                        const ImplControlValue& value, SalControlHandle&,
                                        const OUString& )
{
    // put not implemented types here
    if (type == CTRL_SPINBUTTONS)
    {
        return false;
    }

    BOOL returnVal = true;

    Display* dpy = GetXDisplay();
    XLIB_Window drawable = GetDrawable();
    GC gc = SelectPen();

    QRect widgetRect = region2QRect(rControlRegion);
    if( type == CTRL_SPINBOX && part == PART_ALL_BUTTONS )
        type = CTRL_SPINBUTTONS;
    if( type == CTRL_SPINBUTTONS )
    {
        SpinbuttonValue* pSpinVal = (SpinbuttonValue *)(value.getOptionalVal());
        Rectangle aButtonRect( pSpinVal->maUpperRect);
        aButtonRect.Union( pSpinVal->maLowerRect );;
        widgetRect = QRect( aButtonRect.Left(), aButtonRect.Top(),
                            aButtonRect.Right(), aButtonRect.Bottom() );
    }

    //draw right onto the window
    QPixmap pixmap(widgetRect.width(), widgetRect.height());

    if (pixmap.isNull())
    {
        return false;
    }

    QPainter painter(&pixmap);
    // painter.setBackgroundMode(Qt::OpaqueMode);

    //copy previous screen contents for proper blending
    #if ( QT_VERSION >= QT_VERSION_CHECK( 4, 5, 0 ) )
    QPixmap screen = QPixmap::fromX11Pixmap(drawable);
    painter.drawPixmap(0,0, screen, widgetRect.left(), widgetRect.top(), widgetRect.width(), widgetRect.height());
    #else
    const QX11Info& rX11Info( pixmap.x11Info() );
    X11SalGraphics::CopyScreenArea( dpy,
                              drawable, GetScreenNumber(), GetBitCount(),
                              pixmap.handle(), rX11Info.screen(), rX11Info.depth(),
                              GetDisplay()->GetCopyGC( GetScreenNumber() ),
                              widgetRect.left(), widgetRect.top(), widgetRect.width(), widgetRect.height(),
                              0, 0 );
    #endif

    if (type == CTRL_PUSHBUTTON)
    {
        QStyleOptionButton styleOption;

        styleOption.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
        styleOption.state =vclStateValue2StateFlag( nControlState, value );

        kapp->style()->drawControl( QStyle::CE_PushButton, &styleOption, &painter);
    }
    else if ( (type == CTRL_MENUBAR))
    {
        if (part == PART_MENU_ITEM)
        {
            QStyleOptionMenuItem styleOption;

            styleOption.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
            styleOption.state = vclStateValue2StateFlag( nControlState, value );

            kapp->style()->drawControl( QStyle::CE_MenuBarItem, &styleOption, &painter);
        }
        else
        {
            pixmap.fill(KApplication::palette().color(QPalette::Window));
        }
    }
    else if (type == CTRL_MENU_POPUP)
    {
        if (part == PART_MENU_ITEM)
        {
            QStyleOptionMenuItem styleOption;

            styleOption.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
            styleOption.state = vclStateValue2StateFlag( nControlState, value );

            kapp->style()->drawControl( QStyle::CE_MenuItem, &styleOption, &painter);
        }
        else if (part == PART_MENU_ITEM_CHECK_MARK)
        {
            QStyleOptionButton styleOption;

            styleOption.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
            styleOption.state = vclStateValue2StateFlag( nControlState, value );

            if (nControlState & CTRL_STATE_PRESSED)
            {
                kapp->style()->drawPrimitive( QStyle::PE_IndicatorMenuCheckMark, &styleOption, &painter);
            }
        }
        else if (part == PART_MENU_ITEM_RADIO_MARK)
        {
            QStyleOptionButton styleOption;

            styleOption.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
            styleOption.state = vclStateValue2StateFlag( nControlState, value );

            if (nControlState & CTRL_STATE_PRESSED)
            {
                kapp->style()->drawPrimitive( QStyle::PE_IndicatorRadioButton, &styleOption, &painter);
            }
        }
        else
        {
            pixmap.fill(KApplication::palette().color(QPalette::Window));

            #if ( QT_VERSION >= QT_VERSION_CHECK( 4, 5, 0 ) )
            QStyleOptionFrameV3 styleOption;
            #else
            QStyleOptionFrameV2 styleOption;
            #endif

            styleOption.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
            styleOption.state = vclStateValue2StateFlag( nControlState, value );
            #if ( QT_VERSION >= QT_VERSION_CHECK( 4, 5, 0 ) )
            styleOption.frameShape = QFrame::StyledPanel;
            #endif

            kapp->style()->drawPrimitive( QStyle::PE_FrameMenu, &styleOption, &painter);
        }
    }
    else if ( (type == CTRL_TOOLBAR) && (part == PART_BUTTON) )
    {
        QStyleOptionToolButton styleOption;

        styleOption.arrowType = Qt::NoArrow;
        styleOption.subControls = QStyle::SC_ToolButton;

        styleOption.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
        styleOption.state = vclStateValue2StateFlag( nControlState, value );
        styleOption.state |= QStyle::State_Raised | QStyle::State_Enabled | QStyle::State_AutoRaise;

        kapp->style()->drawComplexControl( QStyle::CC_ToolButton, &styleOption, &painter);
    }
    else if ( (type == CTRL_TOOLBAR) && (part == PART_ENTIRE_CONTROL) )
    {
        QStyleOptionToolBar styleOption;

        styleOption.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
        styleOption.state = vclStateValue2StateFlag( nControlState, value );

        kapp->style()->drawControl( QStyle::CE_ToolBar, &styleOption, &painter);
    }
    else if ( (type == CTRL_TOOLBAR) && (part == PART_THUMB_VERT) )
    {
        QStyleOption styleOption;

        int width = kapp->style()->pixelMetric(QStyle::PM_ToolBarHandleExtent);

        styleOption.rect = QRect(0, 0, width, widgetRect.height());
        styleOption.state = QStyle::State_Horizontal;

        kapp->style()->drawPrimitive( QStyle::PE_IndicatorToolBarHandle, &styleOption, &painter);
    }
    else if (type == CTRL_EDITBOX)
    {
        pixmap.fill(KApplication::palette().color(QPalette::Window));

        //TODO hover?? OO does not seem to do this for line edits

        #if ( QT_VERSION >= QT_VERSION_CHECK( 4, 5, 0 ) )
        QStyleOptionFrameV3 styleOption;
        #else
        QStyleOptionFrameV2 styleOption;
        #endif

        styleOption.state = vclStateValue2StateFlag( nControlState, value );

        //TODO...how does the line edit draw itself internally??
        styleOption.rect = QRect(2, 2, widgetRect.width()-4, widgetRect.height()-4);
        kapp->style()->drawPrimitive( QStyle::PE_PanelLineEdit, &styleOption, &painter);

        styleOption.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
        kapp->style()->drawPrimitive( QStyle::PE_FrameLineEdit, &styleOption, &painter);
    }
    else if (type == CTRL_COMBOBOX)
    {
        pixmap.fill(KApplication::palette().color(QPalette::Window));

        QStyleOptionComboBox styleOption;

        styleOption.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
        styleOption.state = vclStateValue2StateFlag( nControlState, value );

        styleOption.editable = true;

        kapp->style()->drawComplexControl(QStyle::CC_ComboBox, &styleOption, &painter);
    }
    else if (type == CTRL_LISTBOX)
    {
        if( part == PART_WINDOW )
        {
            lcl_drawFrame( widgetRect, painter, QStyle::PE_Frame, nControlState, value );
        }
        else
        {
            QStyleOptionComboBox styleOption;

            styleOption.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
            styleOption.state = vclStateValue2StateFlag( nControlState, value );

            if (part == PART_SUB_EDIT)
            {
                kapp->style()->drawControl(QStyle::CE_ComboBoxLabel, &styleOption, &painter);
            }
            else
            {
                kapp->style()->drawComplexControl(QStyle::CC_ComboBox, &styleOption, &painter);
            }
        }
    }
    else if (type == CTRL_LISTNODE)
    {
        QStyleOption styleOption;

        styleOption.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
        styleOption.state = vclStateValue2StateFlag( nControlState, value );

        styleOption.state |= QStyle::State_Item;
        styleOption.state |= QStyle::State_Children;

        if (nControlState & CTRL_STATE_PRESSED)
        {
            styleOption.state |= QStyle::State_Open;
        }

        kapp->style()->drawPrimitive(QStyle::PE_IndicatorBranch, &styleOption, &painter);
    }
    else if (type == CTRL_CHECKBOX)
    {
        QStyleOptionButton styleOption;

        styleOption.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
        styleOption.state = vclStateValue2StateFlag( nControlState, value );

        kapp->style()->drawControl(QStyle::CE_CheckBox, &styleOption, &painter);
    }
    else if (type == CTRL_SCROLLBAR)
    {
        pixmap.fill(KApplication::palette().color(QPalette::Window));

        if ((part == PART_DRAW_BACKGROUND_VERT) || (part == PART_DRAW_BACKGROUND_HORZ))
        {
            ScrollbarValue* sbVal = static_cast<ScrollbarValue *> ( value.getOptionalVal() );

            QStyleOptionSlider styleOption;
            styleOption.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());

            //if the scroll bar is active (aka not degenrate...allow for hover events
            if (sbVal->mnVisibleSize < sbVal->mnMax)
            {
                styleOption.state = vclStateValue2StateFlag( nControlState, value );
                styleOption.state |= QStyle::State_MouseOver;
            }

            //horizontal or vertical
            if (part == PART_DRAW_BACKGROUND_VERT)
            {
                styleOption.orientation = Qt::Vertical;
            }
            else
            {
                styleOption.state |= QStyle::State_Horizontal;
            }

            //setup parameters from the OO values
            styleOption.minimum = sbVal->mnMin;
            styleOption.maximum = sbVal->mnMax - sbVal->mnVisibleSize;
            styleOption.sliderValue = sbVal->mnCur;
            styleOption.sliderPosition = sbVal->mnCur;
            styleOption.pageStep = sbVal->mnVisibleSize;

            //setup the active control...always the slider
            if (sbVal->mnThumbState & CTRL_STATE_ROLLOVER)
            {
                styleOption.activeSubControls = QStyle::SC_ScrollBarSlider;
            }

            kapp->style()->drawComplexControl(QStyle::CC_ScrollBar, &styleOption, &painter);
        }
    }
    else if (type == CTRL_SPINBOX)
    {
        pixmap.fill(KApplication::palette().color(QPalette::Window));

        QStyleOptionSpinBox styleOption;
        styleOption.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
        styleOption.state = vclStateValue2StateFlag( nControlState, value );
        // determine active control
        SpinbuttonValue* pSpinVal = (SpinbuttonValue *)(value.getOptionalVal());
        if( pSpinVal )
        {
            if( (pSpinVal->mnUpperState & CTRL_STATE_PRESSED) )
                styleOption.activeSubControls |= QStyle::SC_SpinBoxUp;
            if( (pSpinVal->mnLowerState & CTRL_STATE_PRESSED) )
                styleOption.activeSubControls |= QStyle::SC_SpinBoxDown;
        }

        kapp->style()->drawComplexControl(QStyle::CC_SpinBox, &styleOption, &painter);
    }
    else if (type == CTRL_GROUPBOX)
    {
        QStyleOptionGroupBox styleOption;
        styleOption.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
        styleOption.state = vclStateValue2StateFlag( nControlState, value );

        kapp->style()->drawComplexControl(QStyle::CC_GroupBox, &styleOption, &painter);
    }
    else if (type == CTRL_RADIOBUTTON)
    {
        QStyleOptionButton styleOption;
        styleOption.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
        styleOption.state = vclStateValue2StateFlag( nControlState, value );

        kapp->style()->drawControl(QStyle::CE_RadioButton, &styleOption, &painter);
    }
    else if (type == CTRL_TOOLTIP)
    {
        QStyleOption styleOption;
        styleOption.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
        styleOption.state = vclStateValue2StateFlag( nControlState, value );

        kapp->style()->drawPrimitive(QStyle::PE_PanelTipLabel, &styleOption, &painter);
    }
    else if (type == CTRL_FRAME)
    {
        pixmap.fill(KApplication::palette().color(QPalette::Window));
        lcl_drawFrame( widgetRect, painter, QStyle::PE_Frame, nControlState, value );
    }
    else if (type == CTRL_FIXEDBORDER)
    {
        pixmap.fill(KApplication::palette().color(QPalette::Window));
        lcl_drawFrame( widgetRect, painter, QStyle::PE_FrameWindow, nControlState, value );
    }
    else if (type == CTRL_WINDOW_BACKGROUND)
    {
        pixmap.fill(KApplication::palette().color(QPalette::Window));
    }
    else if (type == CTRL_FIXEDLINE)
    {
        QStyleOptionMenuItem styleOption;

        styleOption.rect = QRect(0, 0, widgetRect.width(), widgetRect.height());
        styleOption.state = vclStateValue2StateFlag( nControlState, value );
        styleOption.menuItemType = QStyleOptionMenuItem::Separator;
        styleOption.state |= QStyle::State_Item;

        kapp->style()->drawControl( QStyle::CE_MenuItem, &styleOption, &painter);
    }
    else
    {
        returnVal = false;
    }

    if (returnVal)
    {
        X11SalGraphics::CopyScreenArea( dpy,
            pixmap.handle(), pixmap.x11Info().screen(), pixmap.x11Info().depth(),
            drawable, GetScreenNumber(), GetVisual().GetDepth(), gc,
            0, 0, widgetRect.width(), widgetRect.height(), widgetRect.left(), widgetRect.top() );
    }

    return returnVal;
}

BOOL KDESalGraphics::getNativeControlRegion( ControlType type, ControlPart part,
                                             const Region& controlRegion, ControlState controlState,
                                             const ImplControlValue& val, SalControlHandle&,
                                             const OUString&,
                                             Region &nativeBoundingRegion, Region &nativeContentRegion )
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
                int size = kapp->style()->pixelMetric(QStyle::PM_LayoutLeftMargin);
                USHORT nStyle = val.getNumericVal();
                if( nStyle & FRAME_DRAW_NODRAW )
                {
                    // in this case the question is: how thick would a frame be
                    // see brdwin.cxx, decoview.cxx
                    // most probably the behavior in decoview.cxx is wrong.
                    contentRect.adjust(size, size, -size, -size);
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
        }
        default:
            break;
    }
#if 0


        // Metrics of the scroll bar
        case CTRL_SCROLLBAR:
            //pWidget = pWidgetPainter->scrollBar( rControlRegion,
                //( part == PART_BUTTON_LEFT || part == PART_BUTTON_RIGHT ),
                //ImplControlValue() );
            //aStyleOption.initFrom( pWidget );

            switch ( part )
            {
            case PART_BUTTON_LEFT:
            case PART_BUTTON_UP:
                qRect = kapp->style()->subControlRect(
                    QStyle::CC_ScrollBar, &aStyleOption, QStyle::SC_ScrollBarSubLine );

                // Workaround for Platinum style scroll bars. It makes the
                // left/up button invisible.
                if ( part == PART_BUTTON_LEFT )
                {
                    if ( qRect.left() > kapp->style()->subControlRect(
                        QStyle::CC_ScrollBar, &aStyleOption,
                        QStyle::SC_ScrollBarSubPage ).left() )
                    {
                        qRect.setLeft( 0 );
                        qRect.setRight( 0 );
                    }
                }
                else
                {
                    if ( qRect.top() > kapp->style()->subControlRect(
                        QStyle::CC_ScrollBar, &aStyleOption,
                        QStyle::SC_ScrollBarSubPage ).top() )
                    {
                        qRect.setTop( 0 );
                        qRect.setBottom( 0 );
                    }
                }

                qRect.translate( qBoundingRect.left(), qBoundingRect.top() );

                bReturn = TRUE;
                break;

            case PART_BUTTON_RIGHT:
            case PART_BUTTON_DOWN:
                qRect = kapp->style()->subControlRect(
                    QStyle::CC_ScrollBar, &aStyleOption, QStyle::SC_ScrollBarAddLine );

                // Workaround for Platinum and 3 button style scroll bars.
                // It makes the right/down button bigger.
                if ( part == PART_BUTTON_RIGHT )
                    qRect.setLeft( kapp->style()->subControlRect(
                        QStyle::CC_ScrollBar, &aStyleOption,
                        QStyle::SC_ScrollBarAddPage ).right() + 1 );
                else
                    qRect.setTop( kapp->style()->subControlRect(
                        QStyle::CC_ScrollBar, &aStyleOption,
                        QStyle::SC_ScrollBarAddPage ).bottom() + 1 );

                qRect.translate( qBoundingRect.left(), qBoundingRect.top() );

                bReturn = TRUE;
                break;
            }
            break;
    }
#endif

    if (retVal)
    {
        // Bounding region
        Point aBPoint( boundingRect.x(), boundingRect.y() );
        Size aBSize( boundingRect.width(), boundingRect.height() );
        nativeBoundingRegion = Region( Rectangle( aBPoint, aBSize ) );

        // Region of the content
        Point aPoint( contentRect.x(), contentRect.y() );
        Size  aSize( contentRect.width(), contentRect.height() );
        nativeContentRegion = Region( Rectangle( aPoint, aSize ) );
    }

    return retVal;
}
