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

#include <QtGraphics_Controls.hxx>

#include <QtGui/QPainter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>

#include <QtInstance.hxx>
#include <QtTools.hxx>
#include <QtGraphicsBase.hxx>
#include <vcl/decoview.hxx>

/**
  Conversion function between VCL ControlState together with
  ImplControlValue and Qt state flags.
  @param nControlState State of the widget (default, focused, ...) in Native Widget Framework.
  @param aValue Value held by the widget (on, off, ...)
*/
static QStyle::State vclStateValue2StateFlag(ControlState nControlState,
                                             const ImplControlValue& aValue)
{
    QStyle::State nState
        = ((nControlState & ControlState::ENABLED) ? QStyle::State_Enabled : QStyle::State_None)
          | ((nControlState & ControlState::FOCUSED)
                 ? QStyle::State_HasFocus | QStyle::State_KeyboardFocusChange
                 : QStyle::State_None)
          | ((nControlState & ControlState::PRESSED) ? QStyle::State_Sunken : QStyle::State_None)
          | ((nControlState & ControlState::SELECTED) ? QStyle::State_Selected : QStyle::State_None)
          | ((nControlState & ControlState::ROLLOVER) ? QStyle::State_MouseOver
                                                      : QStyle::State_None);

    switch (aValue.getTristateVal())
    {
        case ButtonValue::On:
            nState |= QStyle::State_On;
            break;
        case ButtonValue::Off:
            nState |= QStyle::State_Off;
            break;
        case ButtonValue::Mixed:
            nState |= QStyle::State_NoChange;
            break;
        default:
            break;
    }

    return nState;
}

static void lcl_ApplyBackgroundColorToStyleOption(QStyleOption& rOption,
                                                  const Color& rBackgroundColor)
{
    if (rBackgroundColor != COL_AUTO)
    {
        QColor aColor = toQColor(rBackgroundColor);
        for (QPalette::ColorRole role : { QPalette::Window, QPalette::Button, QPalette::Base })
            rOption.palette.setColor(role, aColor);
    }
}

QtGraphics_Controls::QtGraphics_Controls(const QtGraphicsBase& rGraphics)
    : m_rGraphics(rGraphics)
{
}

bool QtGraphics_Controls::isNativeControlSupported(ControlType type, ControlPart part)
{
    switch (type)
    {
        case ControlType::Tooltip:
        case ControlType::Progress:
        case ControlType::ListNode:
            return (part == ControlPart::Entire);

        case ControlType::Pushbutton:
        case ControlType::Radiobutton:
        case ControlType::Checkbox:
            return (part == ControlPart::Entire) || (part == ControlPart::Focus);

        case ControlType::ListHeader:
            return (part == ControlPart::Button);

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

        case ControlType::TabItem:
        case ControlType::TabPane:
            return ((part == ControlPart::Entire) || part == ControlPart::TabPaneWithHeader);

        default:
            break;
    }

    return false;
}

inline int QtGraphics_Controls::pixelMetric(QStyle::PixelMetric metric, const QStyleOption* option,
                                            const QWidget* pWidget)
{
    return QApplication::style()->pixelMetric(metric, option, pWidget);
}

inline QSize QtGraphics_Controls::sizeFromContents(QStyle::ContentsType type,
                                                   const QStyleOption* option,
                                                   const QSize& contentsSize)
{
    return QApplication::style()->sizeFromContents(type, option, contentsSize);
}

inline QRect QtGraphics_Controls::subControlRect(QStyle::ComplexControl control,
                                                 const QStyleOptionComplex* option,
                                                 QStyle::SubControl subControl)
{
    return QApplication::style()->subControlRect(control, option, subControl);
}

inline QRect QtGraphics_Controls::subElementRect(QStyle::SubElement element,
                                                 const QStyleOption* option)
{
    return QApplication::style()->subElementRect(element, option);
}

void QtGraphics_Controls::draw(QStyle::ControlElement element, QStyleOption& rOption, QImage* image,
                               const Color& rBackgroundColor, QStyle::State const state, QRect rect)
{
    const QRect& targetRect = !rect.isNull() ? rect : image->rect();

    rOption.state |= state;
    rOption.rect = downscale(targetRect);

    lcl_ApplyBackgroundColorToStyleOption(rOption, rBackgroundColor);

    QPainter painter(image);
    QApplication::style()->drawControl(element, &rOption, &painter);
}

void QtGraphics_Controls::draw(QStyle::PrimitiveElement element, QStyleOption& rOption,
                               QImage* image, const Color& rBackgroundColor,
                               QStyle::State const state, QRect rect)
{
    const QRect& targetRect = !rect.isNull() ? rect : image->rect();

    rOption.state |= state;
    rOption.rect = downscale(targetRect);

    lcl_ApplyBackgroundColorToStyleOption(rOption, rBackgroundColor);

    QPainter painter(image);
    QApplication::style()->drawPrimitive(element, &rOption, &painter);
}

void QtGraphics_Controls::draw(QStyle::ComplexControl element, QStyleOptionComplex& rOption,
                               QImage* image, const Color& rBackgroundColor,
                               QStyle::State const state)
{
    const QRect& targetRect = image->rect();

    rOption.state |= state;
    rOption.rect = downscale(targetRect);

    lcl_ApplyBackgroundColorToStyleOption(rOption, rBackgroundColor);

    QPainter painter(image);
    QApplication::style()->drawComplexControl(element, &rOption, &painter);
}

void QtGraphics_Controls::drawFrame(QStyle::PrimitiveElement element, QImage* image,
                                    const Color& rBackgroundColor, QStyle::State const& state,
                                    bool bClip, QStyle::PixelMetric eLineMetric)
{
    const int fw = pixelMetric(eLineMetric);
    QStyleOptionFrame option;
    option.frameShape = QFrame::StyledPanel;
    option.state = QStyle::State_Sunken | state;
    option.lineWidth = fw;

    QRect aRect = downscale(image->rect());
    option.rect = aRect;

    lcl_ApplyBackgroundColorToStyleOption(option, rBackgroundColor);

    QPainter painter(image);
    if (bClip)
        painter.setClipRegion(QRegion(aRect).subtracted(aRect.adjusted(fw, fw, -fw, -fw)));
    QApplication::style()->drawPrimitive(element, &option, &painter);
}

void QtGraphics_Controls::fillQStyleOptionTab(const ImplControlValue& value, QStyleOptionTab& sot)
{
    const TabitemValue& rValue = static_cast<const TabitemValue&>(value);
    if (rValue.isFirst())
        sot.position = rValue.isLast() ? QStyleOptionTab::OnlyOneTab : QStyleOptionTab::Beginning;
    else if (rValue.isLast())
        sot.position = rValue.isFirst() ? QStyleOptionTab::OnlyOneTab : QStyleOptionTab::End;
    else
        sot.position = QStyleOptionTab::Middle;
}

void QtGraphics_Controls::fullQStyleOptionTabWidgetFrame(QStyleOptionTabWidgetFrame& option,
                                                         bool bDownscale)
{
    option.state = QStyle::State_Enabled;
    option.rightCornerWidgetSize = QSize(0, 0);
    option.leftCornerWidgetSize = QSize(0, 0);
    int nLineWidth = pixelMetric(QStyle::PM_DefaultFrameWidth);
    option.lineWidth = bDownscale ? std::max(1, downscale(nLineWidth, Round::Ceil)) : nLineWidth;
    option.midLineWidth = 0;
    option.shape = QTabBar::RoundedNorth;
}

bool QtGraphics_Controls::drawNativeControl(ControlType type, ControlPart part,
                                            const tools::Rectangle& rControlRegion,
                                            ControlState nControlState,
                                            const ImplControlValue& value, const OUString&,
                                            const Color& rBackgroundColor)
{
    bool nativeSupport = isNativeControlSupported(type, part);
    if (!nativeSupport)
    {
        assert(!nativeSupport && "drawNativeControl called without native support!");
        return false;
    }

    if (m_lastPopupRect.isValid()
        && (type != ControlType::MenuPopup || part != ControlPart::MenuItem))
        m_lastPopupRect = QRect();

    bool returnVal = true;

    QRect widgetRect = toQRect(rControlRegion);

    //if no image, or resized, make a new image
    if (!m_image || m_image->size() != widgetRect.size())
    {
        m_image.reset(new QImage(widgetRect.width(), widgetRect.height(),
                                 QImage::Format_ARGB32_Premultiplied));
        m_image->setDevicePixelRatio(m_rGraphics.devicePixelRatioF());
    }

    // Default image color - just once
    switch (type)
    {
        case ControlType::MenuPopup:
            if (part == ControlPart::MenuItemCheckMark || part == ControlPart::MenuItemRadioMark)
            {
                // it is necessary to fill the background transparently first, as this
                // is painted after menuitem highlight, otherwise there would be a grey area
                m_image->fill(Qt::transparent);
                break;
            }
            [[fallthrough]]; // QPalette::Window
        case ControlType::Menubar:
        case ControlType::WindowBackground:
            m_image->fill(QApplication::palette().color(QPalette::Window).rgb());
            break;
        case ControlType::Tooltip:
            m_image->fill(QApplication::palette().color(QPalette::ToolTipBase).rgb());
            break;
        case ControlType::Scrollbar:
            if ((part == ControlPart::DrawBackgroundVert)
                || (part == ControlPart::DrawBackgroundHorz))
            {
                m_image->fill(QApplication::palette().color(QPalette::Window).rgb());
                break;
            }
            [[fallthrough]]; // Qt::transparent
        default:
            m_image->fill(Qt::transparent);
            break;
    }

    if (type == ControlType::Pushbutton)
    {
        const PushButtonValue& rPBValue = static_cast<const PushButtonValue&>(value);
        if (part == ControlPart::Focus)
            // Nothing to do. Drawing focus separately is not needed because that's
            // already handled by the ControlState::FOCUSED state being set when
            // drawing the entire control
            return true;
        assert(part == ControlPart::Entire);
        QStyleOptionButton option;
        if (nControlState & ControlState::DEFAULT)
            option.features |= QStyleOptionButton::DefaultButton;
        if (rPBValue.m_bFlatButton)
            option.features |= QStyleOptionButton::Flat;
        draw(QStyle::CE_PushButton, option, m_image.get(), rBackgroundColor,
             vclStateValue2StateFlag(nControlState, value));
    }
    else if (type == ControlType::Menubar)
    {
        if (part == ControlPart::MenuItem)
        {
            QStyleOptionMenuItem option;
            option.state = vclStateValue2StateFlag(nControlState, value);
            if ((nControlState & ControlState::ROLLOVER)
                && QApplication::style()->styleHint(QStyle::SH_MenuBar_MouseTracking))
                option.state |= QStyle::State_Selected;

            if (nControlState
                & ControlState::SELECTED) // Passing State_Sunken is currently not documented.
                option.state |= QStyle::State_Sunken; // But some kinds of QStyle interpret it.

            draw(QStyle::CE_MenuBarItem, option, m_image.get(), rBackgroundColor);
        }
        else if (part == ControlPart::Entire)
        {
            QStyleOptionMenuItem option;
            draw(QStyle::CE_MenuBarEmptyArea, option, m_image.get(), rBackgroundColor,
                 vclStateValue2StateFlag(nControlState, value));
        }
        else
        {
            returnVal = false;
        }
    }
    else if (type == ControlType::MenuPopup)
    {
        assert(part == ControlPart::MenuItem ? m_lastPopupRect.isValid()
                                             : !m_lastPopupRect.isValid());
        if (part == ControlPart::MenuItem)
        {
            QStyleOptionMenuItem option;
            draw(QStyle::CE_MenuItem, option, m_image.get(), rBackgroundColor,
                 vclStateValue2StateFlag(nControlState, value));
            // HACK: LO core first paints the entire popup and only then it paints menu items,
            // but QMenu::paintEvent() paints popup frame after all items. That means highlighted
            // items here would paint the highlight over the frame border. Since calls to ControlPart::MenuItem
            // are always preceded by calls to ControlPart::Entire, just remember the size for the whole
            // popup (otherwise not possible to get here) and draw the border afterwards.
            QRect framerect(m_lastPopupRect.topLeft() - widgetRect.topLeft(),
                            widgetRect.size().expandedTo(m_lastPopupRect.size()));
            QStyleOptionFrame frame;
            draw(QStyle::PE_FrameMenu, frame, m_image.get(), rBackgroundColor,
                 vclStateValue2StateFlag(nControlState, value), framerect);
        }
        else if (part == ControlPart::Separator)
        {
            QStyleOptionMenuItem option;
            option.menuItemType = QStyleOptionMenuItem::Separator;
            // Painting the whole menu item area results in different background
            // with at least Plastique style, so clip only to the separator itself
            // (QSize( 2, 2 ) is hardcoded in Qt)
            option.rect = m_image->rect();
            QSize size = sizeFromContents(QStyle::CT_MenuItem, &option, QSize(2, 2));
            QRect rect = m_image->rect();
            QPoint center = rect.center();
            rect.setHeight(size.height());
            rect.moveCenter(center);
            option.state |= vclStateValue2StateFlag(nControlState, value);
            option.rect = rect;

            QPainter painter(m_image.get());
            // don't paint over popup frame border (like the hack above, but here it can be simpler)
            const int fw = pixelMetric(QStyle::PM_MenuPanelWidth);
            painter.setClipRect(rect.adjusted(fw, 0, -fw, 0));
            QApplication::style()->drawControl(QStyle::CE_MenuItem, &option, &painter);
        }
        else if (part == ControlPart::MenuItemCheckMark || part == ControlPart::MenuItemRadioMark)
        {
            QStyleOptionMenuItem option;
            option.checkType = (part == ControlPart::MenuItemCheckMark)
                                   ? QStyleOptionMenuItem::NonExclusive
                                   : QStyleOptionMenuItem::Exclusive;
            option.checked = bool(nControlState & ControlState::PRESSED);
            // widgetRect is now the rectangle for the checkbox/radiobutton itself, but Qt
            // paints the whole menu item, so translate position (and it'll be clipped);
            // it is also necessary to fill the background transparently first, as this
            // is painted after menuitem highlight, otherwise there would be a grey area
            assert(value.getType() == ControlType::MenuPopup);
            const MenupopupValue* menuVal = static_cast<const MenupopupValue*>(&value);
            QRect menuItemRect(toQRect(menuVal->maItemRect));
            QRect rect(menuItemRect.topLeft() - widgetRect.topLeft(),
                       widgetRect.size().expandedTo(menuItemRect.size()));
            // checkboxes are always displayed next to images in menus, so are never centered
            const int focus_size = pixelMetric(QStyle::PM_FocusFrameHMargin);
            rect.moveTo(-focus_size, rect.y());
            draw(QStyle::CE_MenuItem, option, m_image.get(), rBackgroundColor,
                 vclStateValue2StateFlag(nControlState & ~ControlState::PRESSED, value), rect);
        }
        else if (part == ControlPart::Entire)
        {
            QStyleOptionMenuItem option;
            option.state = vclStateValue2StateFlag(nControlState, value);
            draw(QStyle::PE_PanelMenu, option, m_image.get(), rBackgroundColor);
            // Try hard to get any frame!
            QStyleOptionFrame frame;
            draw(QStyle::PE_FrameMenu, frame, m_image.get(), rBackgroundColor);
            draw(QStyle::PE_FrameWindow, frame, m_image.get(), rBackgroundColor);
            m_lastPopupRect = widgetRect;
        }
        else
            returnVal = false;
    }
    else if ((type == ControlType::Toolbar) && (part == ControlPart::Button))
    {
        QStyleOptionToolButton option;

        option.arrowType = Qt::NoArrow;
        option.subControls = QStyle::SC_ToolButton;
        option.state = vclStateValue2StateFlag(nControlState, value);
        option.state |= QStyle::State_Raised | QStyle::State_Enabled | QStyle::State_AutoRaise;

        draw(QStyle::CC_ToolButton, option, m_image.get(), rBackgroundColor);
    }
    else if ((type == ControlType::Toolbar) && (part == ControlPart::Entire))
    {
        QStyleOptionToolBar option;
        draw(QStyle::CE_ToolBar, option, m_image.get(), rBackgroundColor,
             vclStateValue2StateFlag(nControlState, value));
    }
    else if ((type == ControlType::Toolbar)
             && (part == ControlPart::ThumbVert || part == ControlPart::ThumbHorz))
    {
        // reduce paint area only to the handle area
        const int handleExtend = pixelMetric(QStyle::PM_ToolBarHandleExtent);
        QStyleOption option;
        QRect aRect = m_image->rect();
        if (part == ControlPart::ThumbVert)
        {
            aRect.setWidth(handleExtend);
            option.state = QStyle::State_Horizontal;
        }
        else
            aRect.setHeight(handleExtend);
        draw(QStyle::PE_IndicatorToolBarHandle, option, m_image.get(), rBackgroundColor,
             vclStateValue2StateFlag(nControlState, value), aRect);
    }
    else if (type == ControlType::Editbox || type == ControlType::MultilineEditbox)
    {
        drawFrame(QStyle::PE_FrameLineEdit, m_image.get(), rBackgroundColor,
                  vclStateValue2StateFlag(nControlState, value), false);
    }
    else if (type == ControlType::Combobox)
    {
        QStyleOptionComboBox option;
        option.editable = true;
        draw(QStyle::CC_ComboBox, option, m_image.get(), rBackgroundColor,
             vclStateValue2StateFlag(nControlState, value));
    }
    else if (type == ControlType::Listbox)
    {
        QStyleOptionComboBox option;
        option.editable = false;
        switch (part)
        {
            case ControlPart::ListboxWindow:
                drawFrame(QStyle::PE_Frame, m_image.get(), rBackgroundColor,
                          vclStateValue2StateFlag(nControlState, value), true,
                          QStyle::PM_ComboBoxFrameWidth);
                break;
            case ControlPart::SubEdit:
                draw(QStyle::CE_ComboBoxLabel, option, m_image.get(), rBackgroundColor,
                     vclStateValue2StateFlag(nControlState, value));
                break;
            case ControlPart::Entire:
                draw(QStyle::CC_ComboBox, option, m_image.get(), rBackgroundColor,
                     vclStateValue2StateFlag(nControlState, value));
                break;
            case ControlPart::ButtonDown:
                option.subControls = QStyle::SC_ComboBoxArrow;
                draw(QStyle::CC_ComboBox, option, m_image.get(), rBackgroundColor,
                     vclStateValue2StateFlag(nControlState, value));
                break;
            default:
                returnVal = false;
                break;
        }
    }
    else if (type == ControlType::ListNode)
    {
        QStyleOption option;
        option.state = vclStateValue2StateFlag(nControlState, value);
        option.state |= QStyle::State_Item | QStyle::State_Children;

        if (value.getTristateVal() == ButtonValue::On)
            option.state |= QStyle::State_Open;

        draw(QStyle::PE_IndicatorBranch, option, m_image.get(), rBackgroundColor);
    }
    else if (type == ControlType::ListHeader)
    {
        QStyleOptionHeader option;
        draw(QStyle::CE_HeaderSection, option, m_image.get(), rBackgroundColor,
             vclStateValue2StateFlag(nControlState, value));
    }
    else if (type == ControlType::Checkbox)
    {
        if (part == ControlPart::Entire)
        {
            QStyleOptionButton option;
            // clear FOCUSED bit, focus is drawn separately
            nControlState &= ~ControlState::FOCUSED;
            draw(QStyle::CE_CheckBox, option, m_image.get(), rBackgroundColor,
                 vclStateValue2StateFlag(nControlState, value));
        }
        else if (part == ControlPart::Focus)
        {
            QStyleOptionFocusRect option;
            draw(QStyle::PE_FrameFocusRect, option, m_image.get(), rBackgroundColor,
                 vclStateValue2StateFlag(nControlState, value));
        }
    }
    else if (type == ControlType::Scrollbar)
    {
        if ((part == ControlPart::DrawBackgroundVert) || (part == ControlPart::DrawBackgroundHorz))
        {
            QStyleOptionSlider option;
            assert(value.getType() == ControlType::Scrollbar);
            const ScrollbarValue* sbVal = static_cast<const ScrollbarValue*>(&value);

            //if the scroll bar is active (aka not degenerate... allow for hover events)
            if (sbVal->mnVisibleSize < sbVal->mnMax)
                option.state = QStyle::State_MouseOver;

            bool horizontal = (part == ControlPart::DrawBackgroundHorz); //horizontal or vertical
            option.orientation = horizontal ? Qt::Horizontal : Qt::Vertical;
            if (horizontal)
                option.state |= QStyle::State_Horizontal;

            // If the scrollbar has a mnMin == 0 and mnMax == 0 then mnVisibleSize is set to -1?!
            // I don't know if a negative mnVisibleSize makes any sense, so just handle this case
            // without crashing LO with a SIGFPE in the Qt library.
            const tools::Long nVisibleSize
                = (sbVal->mnMin == sbVal->mnMax) ? 0 : sbVal->mnVisibleSize;

            option.minimum = sbVal->mnMin;
            option.maximum = sbVal->mnMax - nVisibleSize;
            option.maximum = qMax(option.maximum, option.minimum); // bnc#619772
            option.sliderValue = sbVal->mnCur;
            option.sliderPosition = sbVal->mnCur;
            option.pageStep = nVisibleSize;
            if (part == ControlPart::DrawBackgroundHorz)
                option.upsideDown
                    = (QGuiApplication::isRightToLeft()
                       && sbVal->maButton1Rect.Left() < sbVal->maButton2Rect.Left())
                      || (QGuiApplication::isLeftToRight()
                          && sbVal->maButton1Rect.Left() > sbVal->maButton2Rect.Left());

            //setup the active control... always the slider
            if (sbVal->mnThumbState & ControlState::ROLLOVER)
                option.activeSubControls = QStyle::SC_ScrollBarSlider;

            draw(QStyle::CC_ScrollBar, option, m_image.get(), rBackgroundColor,
                 vclStateValue2StateFlag(nControlState, value));
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
        if (value.getType() == ControlType::SpinButtons)
        {
            const SpinbuttonValue* pSpinVal = static_cast<const SpinbuttonValue*>(&value);
            if (pSpinVal->mnUpperState & ControlState::PRESSED)
                option.activeSubControls |= QStyle::SC_SpinBoxUp;
            if (pSpinVal->mnLowerState & ControlState::PRESSED)
                option.activeSubControls |= QStyle::SC_SpinBoxDown;
            if (pSpinVal->mnUpperState & ControlState::ENABLED)
                option.stepEnabled |= QAbstractSpinBox::StepUpEnabled;
            if (pSpinVal->mnLowerState & ControlState::ENABLED)
                option.stepEnabled |= QAbstractSpinBox::StepDownEnabled;
            if (pSpinVal->mnUpperState & ControlState::ROLLOVER)
                option.state = QStyle::State_MouseOver;
            if (pSpinVal->mnLowerState & ControlState::ROLLOVER)
                option.state = QStyle::State_MouseOver;
        }

        draw(QStyle::CC_SpinBox, option, m_image.get(), rBackgroundColor,
             vclStateValue2StateFlag(nControlState, value));
    }
    else if (type == ControlType::Radiobutton)
    {
        if (part == ControlPart::Entire)
        {
            QStyleOptionButton option;
            // clear FOCUSED bit, focus is drawn separately
            nControlState &= ~ControlState::FOCUSED;
            draw(QStyle::CE_RadioButton, option, m_image.get(), rBackgroundColor,
                 vclStateValue2StateFlag(nControlState, value));
        }
        else if (part == ControlPart::Focus)
        {
            QStyleOptionFocusRect option;
            draw(QStyle::PE_FrameFocusRect, option, m_image.get(), rBackgroundColor,
                 vclStateValue2StateFlag(nControlState, value));
        }
    }
    else if (type == ControlType::Tooltip)
    {
        QStyleOption option;
        draw(QStyle::PE_PanelTipLabel, option, m_image.get(), rBackgroundColor,
             vclStateValue2StateFlag(nControlState, value));
    }
    else if (type == ControlType::Frame)
    {
        drawFrame(QStyle::PE_Frame, m_image.get(), rBackgroundColor,
                  vclStateValue2StateFlag(nControlState, value));
    }
    else if (type == ControlType::WindowBackground)
    {
        // Nothing to do - see "Default image color" switch ^^
    }
    else if (type == ControlType::Fixedline)
    {
        QStyleOptionMenuItem option;
        option.menuItemType = QStyleOptionMenuItem::Separator;
        option.state = vclStateValue2StateFlag(nControlState, value);
        option.state |= QStyle::State_Item;

        draw(QStyle::CE_MenuItem, option, m_image.get(), rBackgroundColor);
    }
    else if (type == ControlType::Slider
             && (part == ControlPart::TrackHorzArea || part == ControlPart::TrackVertArea))
    {
        assert(value.getType() == ControlType::Slider);
        const SliderValue* slVal = static_cast<const SliderValue*>(&value);
        QStyleOptionSlider option;

        option.state = vclStateValue2StateFlag(nControlState, value);
        option.maximum = slVal->mnMax;
        option.minimum = slVal->mnMin;
        option.sliderPosition = option.sliderValue = slVal->mnCur;
        bool horizontal = (part == ControlPart::TrackHorzArea); //horizontal or vertical
        option.orientation = horizontal ? Qt::Horizontal : Qt::Vertical;
        if (horizontal)
            option.state |= QStyle::State_Horizontal;

        draw(QStyle::CC_Slider, option, m_image.get(), rBackgroundColor);
    }
    else if (type == ControlType::Progress && part == ControlPart::Entire)
    {
        QStyleOptionProgressBar option;
        option.minimum = 0;
        option.maximum = widgetRect.width();
        option.progress = value.getNumericVal();

        draw(QStyle::CE_ProgressBar, option, m_image.get(), rBackgroundColor,
             vclStateValue2StateFlag(nControlState, value));
    }
    else if (type == ControlType::TabItem && part == ControlPart::Entire)
    {
        QStyleOptionTab sot;
        fillQStyleOptionTab(value, sot);
        draw(QStyle::CE_TabBarTabShape, sot, m_image.get(), rBackgroundColor,
             vclStateValue2StateFlag(nControlState, value));
    }
    else if (type == ControlType::TabPane && part == ControlPart::Entire)
    {
        const TabPaneValue& rValue = static_cast<const TabPaneValue&>(value);

        // get the overlap size for the tabs, so they will overlap the frame
        QStyleOptionTab tabOverlap;
        tabOverlap.shape = QTabBar::RoundedNorth;
        TabPaneValue::m_nOverlap = pixelMetric(QStyle::PM_TabBarBaseOverlap, &tabOverlap);

        QStyleOptionTabWidgetFrame option;
        fullQStyleOptionTabWidgetFrame(option, false);
        option.tabBarRect = toQRect(rValue.m_aTabHeaderRect);
        option.selectedTabRect
            = rValue.m_aSelectedTabRect.IsEmpty() ? QRect() : toQRect(rValue.m_aSelectedTabRect);
        option.tabBarSize = toQSize(rValue.m_aTabHeaderRect.GetSize());
        option.rect = m_image->rect();
        QRect aRect = subElementRect(QStyle::SE_TabWidgetTabPane, &option);
        draw(QStyle::PE_FrameTabWidget, option, m_image.get(), rBackgroundColor,
             vclStateValue2StateFlag(nControlState, value), aRect);
    }
    else
    {
        returnVal = false;
    }

    return returnVal;
}

bool QtGraphics_Controls::getNativeControlRegion(
    ControlType type, ControlPart part, const tools::Rectangle& controlRegion,
    ControlState controlState, const ImplControlValue& val, const OUString& rCaption,
    tools::Rectangle& nativeBoundingRegion, tools::Rectangle& nativeContentRegion)
{
    QtInstance* pQtInstance(GetQtInstance());
    assert(pQtInstance);
    if (!pQtInstance->IsMainThread())
    {
        bool bRet;
        pQtInstance->RunInMainThread([&]() {
            bRet = getNativeControlRegion(type, part, controlRegion, controlState, val, rCaption,
                                          nativeBoundingRegion, nativeContentRegion);
        });
        return bRet;
    }

    bool retVal = false;

    QRect boundingRect = toQRect(controlRegion);
    QRect contentRect = boundingRect;
    QStyleOptionComplex styleOption;

    switch (type)
    {
        // Metrics of the push button
        case ControlType::Pushbutton:
            if (part == ControlPart::Entire)
            {
                styleOption.state = vclStateValue2StateFlag(controlState, val);

                if (controlState & ControlState::DEFAULT)
                {
                    int size = upscale(pixelMetric(QStyle::PM_ButtonDefaultIndicator, &styleOption),
                                       Round::Ceil);
                    boundingRect.adjust(-size, -size, size, size);
                    retVal = true;
                }
            }
            else if (part == ControlPart::Focus)
                retVal = true;
            break;
        case ControlType::Editbox:
        case ControlType::MultilineEditbox:
        {
            // we have to get stable borders, otherwise layout loops.
            // so we simply only scale the detected borders.
            QStyleOptionFrame fo;
            fo.frameShape = QFrame::StyledPanel;
            fo.state = QStyle::State_Sunken;
            fo.lineWidth = pixelMetric(QStyle::PM_DefaultFrameWidth);
            fo.rect = downscale(contentRect);
            fo.rect.setSize(sizeFromContents(QStyle::CT_LineEdit, &fo, fo.rect.size()));
            QRect aSubRect = subElementRect(QStyle::SE_LineEditContents, &fo);

            // VCL tests borders with small defaults before layout, where Qt returns no sub-rect,
            // so this gets us at least some frame.
            int nLine = upscale(fo.lineWidth, Round::Ceil);
            int nLeft = qMin(-nLine, upscale(fo.rect.left() - aSubRect.left(), Round::Floor));
            int nTop = qMin(-nLine, upscale(fo.rect.top() - aSubRect.top(), Round::Floor));
            int nRight = qMax(nLine, upscale(fo.rect.right() - aSubRect.right(), Round::Ceil));
            int nBottom = qMax(nLine, upscale(fo.rect.bottom() - aSubRect.bottom(), Round::Ceil));
            boundingRect.adjust(nLeft, nTop, nRight, nBottom);

            // tdf#150451: ensure a minimum size that fits text content + frame at top and bottom.
            // Themes may use the widget type for determining the actual frame width to use,
            // so pass a dummy QLineEdit
            //
            // NOTE: This is currently only done here for the minimum size calculation and
            // not above because the handling for edit boxes here and in the calling code
            // currently does all kinds of "interesting" things like doing extra size adjustments
            // or passing the content rect where the bounding rect would be expected,...
            // Ideally this should be cleaned up in the callers and all platform integrations
            // to adhere to what the doc in vcl/inc/WidgetDrawInterface.hxx says, but this
            // here keeps it working with existing code for now.
            // (s.a. discussion in https://gerrit.libreoffice.org/c/core/+/146516 for more details)
            QLineEdit aDummyEdit;
            const int nFrameWidth = pixelMetric(QStyle::PM_DefaultFrameWidth, nullptr, &aDummyEdit);
            QFontMetrics aFontMetrics(QApplication::font());
            const int minHeight = upscale(aFontMetrics.height() + 2 * nFrameWidth, Round::Floor);
            if (boundingRect.height() < minHeight)
            {
                const int nDiff = minHeight - boundingRect.height();
                boundingRect.setHeight(boundingRect.height() + nDiff);
                contentRect.setHeight(contentRect.height() + nDiff);
            }

            retVal = true;
            break;
        }
        case ControlType::Checkbox:
            if (part == ControlPart::Entire)
            {
                styleOption.state = vclStateValue2StateFlag(controlState, val);

                int nWidth = pixelMetric(QStyle::PM_IndicatorWidth, &styleOption);
                int nHeight = pixelMetric(QStyle::PM_IndicatorHeight, &styleOption);
                contentRect.setSize(upscale(QSize(nWidth, nHeight), Round::Ceil));

                int nHMargin = pixelMetric(QStyle::PM_FocusFrameHMargin, &styleOption);
                int nVMargin = pixelMetric(QStyle::PM_FocusFrameVMargin, &styleOption);
                contentRect.adjust(0, 0, 2 * upscale(nHMargin, Round::Ceil),
                                   2 * upscale(nVMargin, Round::Ceil));

                boundingRect = contentRect;
                retVal = true;
            }
            break;
        case ControlType::Combobox:
        case ControlType::Listbox:
        {
            QStyleOptionComboBox cbo;

            cbo.rect = downscale(QRect(0, 0, contentRect.width(), contentRect.height()));
            cbo.state = vclStateValue2StateFlag(controlState, val);

            switch (part)
            {
                case ControlPart::Entire:
                {
                    // find out the minimum size that should be used
                    // assume contents is a text line
                    QSize aContentSize = downscale(contentRect.size(), Round::Ceil);
                    QFontMetrics aFontMetrics(QApplication::font());
                    aContentSize.setHeight(aFontMetrics.height());
                    QSize aMinSize = upscale(
                        sizeFromContents(QStyle::CT_ComboBox, &cbo, aContentSize), Round::Ceil);
                    if (aMinSize.height() > contentRect.height())
                        contentRect.setHeight(aMinSize.height());
                    boundingRect = contentRect;
                    retVal = true;
                    break;
                }
                case ControlPart::ButtonDown:
                {
                    contentRect = upscale(
                        subControlRect(QStyle::CC_ComboBox, &cbo, QStyle::SC_ComboBoxArrow));
                    contentRect.translate(boundingRect.left(), boundingRect.top());
                    retVal = true;
                    break;
                }
                case ControlPart::SubEdit:
                {
                    contentRect = upscale(
                        subControlRect(QStyle::CC_ComboBox, &cbo, QStyle::SC_ComboBoxEditField));
                    contentRect.translate(boundingRect.left(), boundingRect.top());
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

            sbo.rect = downscale(QRect(0, 0, contentRect.width(), contentRect.height()));
            sbo.state = vclStateValue2StateFlag(controlState, val);

            switch (part)
            {
                case ControlPart::Entire:
                {
                    QSize aContentSize = downscale(contentRect.size(), Round::Ceil);
                    QFontMetrics aFontMetrics(QApplication::font());
                    aContentSize.setHeight(aFontMetrics.height());
                    QSize aMinSize = upscale(
                        sizeFromContents(QStyle::CT_SpinBox, &sbo, aContentSize), Round::Ceil);
                    if (aMinSize.height() > contentRect.height())
                        contentRect.setHeight(aMinSize.height());
                    boundingRect = contentRect;
                    retVal = true;
                    break;
                }
                case ControlPart::ButtonUp:
                    contentRect
                        = upscale(subControlRect(QStyle::CC_SpinBox, &sbo, QStyle::SC_SpinBoxUp));
                    contentRect.translate(boundingRect.left(), boundingRect.top());
                    retVal = true;
                    break;
                case ControlPart::ButtonDown:
                    contentRect
                        = upscale(subControlRect(QStyle::CC_SpinBox, &sbo, QStyle::SC_SpinBoxDown));
                    contentRect.translate(boundingRect.left(), boundingRect.top());
                    retVal = true;
                    break;
                case ControlPart::SubEdit:
                    contentRect = upscale(
                        subControlRect(QStyle::CC_SpinBox, &sbo, QStyle::SC_SpinBoxEditField));
                    contentRect.translate(boundingRect.left(), boundingRect.top());
                    retVal = true;
                    break;
                default:
                    break;
            }
            break;
        }
        case ControlType::MenuPopup:
        {
            int h, w;
            switch (part)
            {
                case ControlPart::MenuItemCheckMark:
                    h = upscale(pixelMetric(QStyle::PM_IndicatorHeight), Round::Floor);
                    w = upscale(pixelMetric(QStyle::PM_IndicatorWidth), Round::Floor);
                    retVal = true;
                    break;
                case ControlPart::MenuItemRadioMark:
                    h = upscale(pixelMetric(QStyle::PM_ExclusiveIndicatorHeight), Round::Floor);
                    w = upscale(pixelMetric(QStyle::PM_ExclusiveIndicatorWidth), Round::Floor);
                    retVal = true;
                    break;
                default:
                    break;
            }
            if (retVal)
            {
                contentRect = QRect(0, 0, w, h);
                boundingRect = contentRect;
            }
            break;
        }
        case ControlType::Frame:
        {
            if (part == ControlPart::Border)
            {
                int nFrameWidth = upscale(pixelMetric(QStyle::PM_DefaultFrameWidth), Round::Ceil);
                contentRect.adjust(nFrameWidth, nFrameWidth, -nFrameWidth, -nFrameWidth);
                retVal = true;
            }
            break;
        }
        case ControlType::Radiobutton:
        {
            const int h = upscale(pixelMetric(QStyle::PM_ExclusiveIndicatorHeight), Round::Ceil);
            const int w = upscale(pixelMetric(QStyle::PM_ExclusiveIndicatorWidth), Round::Ceil);

            contentRect = QRect(boundingRect.left(), boundingRect.top(), w, h);
            int nHMargin = pixelMetric(QStyle::PM_FocusFrameHMargin, &styleOption);
            int nVMargin = pixelMetric(QStyle::PM_FocusFrameVMargin, &styleOption);
            contentRect.adjust(0, 0, upscale(2 * nHMargin, Round::Ceil),
                               upscale(2 * nVMargin, Round::Ceil));
            boundingRect = contentRect;

            retVal = true;
            break;
        }
        case ControlType::Slider:
        {
            const int w = upscale(pixelMetric(QStyle::PM_SliderLength), Round::Ceil);
            if (part == ControlPart::ThumbHorz)
            {
                contentRect
                    = QRect(boundingRect.left(), boundingRect.top(), w, boundingRect.height());
                boundingRect = contentRect;
                retVal = true;
            }
            else if (part == ControlPart::ThumbVert)
            {
                contentRect
                    = QRect(boundingRect.left(), boundingRect.top(), boundingRect.width(), w);
                boundingRect = contentRect;
                retVal = true;
            }
            break;
        }
        case ControlType::Toolbar:
        {
            const int nWorH = upscale(pixelMetric(QStyle::PM_ToolBarHandleExtent), Round::Ceil);
            if (part == ControlPart::ThumbHorz)
            {
                contentRect
                    = QRect(boundingRect.left(), boundingRect.top(), boundingRect.width(), nWorH);
                boundingRect = contentRect;
                retVal = true;
            }
            else if (part == ControlPart::ThumbVert)
            {
                contentRect
                    = QRect(boundingRect.left(), boundingRect.top(), nWorH, boundingRect.height());
                boundingRect = contentRect;
                retVal = true;
            }
            else if (part == ControlPart::Button)
            {
                QStyleOptionToolButton option;
                option.arrowType = Qt::NoArrow;
                option.features = QStyleOptionToolButton::None;
                option.rect = downscale(QRect({ 0, 0 }, contentRect.size()));
                contentRect = upscale(
                    subControlRect(QStyle::CC_ToolButton, &option, QStyle::SC_ToolButton));
                boundingRect = contentRect;
                retVal = true;
            }
            break;
        }
        case ControlType::Scrollbar:
        {
            // core can't handle 3-button scrollbars well, so we fix that in hitTestNativeControl(),
            // for the rest also provide the track area (i.e. area not taken by buttons)
            if (part == ControlPart::TrackVertArea || part == ControlPart::TrackHorzArea)
            {
                QStyleOptionSlider option;
                bool horizontal = (part == ControlPart::TrackHorzArea); //horizontal or vertical
                option.orientation = horizontal ? Qt::Horizontal : Qt::Vertical;
                if (horizontal)
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
                option.rect = downscale(QRect({ 0, 0 }, contentRect.size()));
                contentRect = upscale(
                    subControlRect(QStyle::CC_ScrollBar, &option, QStyle::SC_ScrollBarGroove));
                contentRect.translate(boundingRect.left()
                                          - (contentRect.width() - boundingRect.width()),
                                      boundingRect.top());
                boundingRect = contentRect;
                retVal = true;
            }
            break;
        }
        case ControlType::TabItem:
        {
            QStyleOptionTab sot;
            fillQStyleOptionTab(val, sot);
            QSize aMinSize = upscale(sizeFromContents(QStyle::CT_TabBarTab, &sot,
                                                      downscale(contentRect.size(), Round::Ceil)),
                                     Round::Ceil);
            contentRect.setSize(aMinSize);
            boundingRect = contentRect;
            retVal = true;
            break;
        }
        case ControlType::TabPane:
        {
            const TabPaneValue& rValue = static_cast<const TabPaneValue&>(val);
            QStyleOptionTabWidgetFrame sotwf;
            fullQStyleOptionTabWidgetFrame(sotwf, true);
            QSize contentSize(
                std::max(rValue.m_aTabHeaderRect.GetWidth(), controlRegion.GetWidth()),
                rValue.m_aTabHeaderRect.GetHeight() + controlRegion.GetHeight());
            QSize aMinSize = upscale(
                sizeFromContents(QStyle::CT_TabWidget, &sotwf, downscale(contentSize, Round::Ceil)),
                Round::Ceil);
            contentRect.setSize(aMinSize);
            boundingRect = contentRect;
            retVal = true;
            break;
        }
        default:
            break;
    }
    if (retVal)
    {
        nativeBoundingRegion = toRectangle(boundingRect);
        nativeContentRegion = toRectangle(contentRect);
    }

    return retVal;
}

/** Test whether the position is in the native widget.
    If the return value is true, bIsInside contains information whether
    aPos was or was not inside the native widget specified by the
    nType/nPart combination.
*/
bool QtGraphics_Controls::hitTestNativeControl(ControlType nType, ControlPart nPart,
                                               const tools::Rectangle& rControlRegion,
                                               const Point& rPos, bool& rIsInside)
{
    if (nType == ControlType::Scrollbar)
    {
        if (nPart != ControlPart::ButtonUp && nPart != ControlPart::ButtonDown
            && nPart != ControlPart::ButtonLeft && nPart != ControlPart::ButtonRight)
        { // we adjust only for buttons (because some scrollbars have 3 buttons,
            // and LO core doesn't handle such scrollbars well)
            return false;
        }
        rIsInside = false;
        bool bHorizontal = (nPart == ControlPart::ButtonLeft || nPart == ControlPart::ButtonRight);
        QRect rect = toQRect(rControlRegion);
        QPoint pos(rPos.X(), rPos.Y());
        // Adjust coordinates to make the widget appear to be at (0,0), i.e. make
        // widget and screen coordinates the same. QStyle functions should use screen
        // coordinates but at least QPlastiqueStyle::subControlRect() is buggy
        // and sometimes uses widget coordinates.
        pos -= rect.topLeft();
        rect.moveTo(0, 0);
        QStyleOptionSlider options;
        options.orientation = bHorizontal ? Qt::Horizontal : Qt::Vertical;
        if (bHorizontal)
            options.state |= QStyle::State_Horizontal;
        options.rect = rect;
        // some random sensible values, since we call this code only for scrollbar buttons,
        // the slider position does not exactly matter
        options.maximum = 10;
        options.minimum = 0;
        options.sliderPosition = options.sliderValue = 4;
        options.pageStep = 2;
        QStyle::SubControl control
            = QApplication::style()->hitTestComplexControl(QStyle::CC_ScrollBar, &options, pos);
        if (nPart == ControlPart::ButtonUp || nPart == ControlPart::ButtonLeft)
            rIsInside = (control == QStyle::SC_ScrollBarSubLine);
        else // DOWN, RIGHT
            rIsInside = (control == QStyle::SC_ScrollBarAddLine);
        return true;
    }
    return false;
}

inline int QtGraphics_Controls::downscale(int size, Round eRound)
{
    return static_cast<int>(eRound == Round::Ceil ? ceil(size / m_rGraphics.devicePixelRatioF())
                                                  : floor(size / m_rGraphics.devicePixelRatioF()));
}

inline int QtGraphics_Controls::upscale(int size, Round eRound)
{
    return static_cast<int>(eRound == Round::Ceil ? ceil(size * m_rGraphics.devicePixelRatioF())
                                                  : floor(size * m_rGraphics.devicePixelRatioF()));
}

inline QRect QtGraphics_Controls::downscale(const QRect& rect)
{
    return QRect(downscale(rect.x(), Round::Floor), downscale(rect.y(), Round::Floor),
                 downscale(rect.width(), Round::Ceil), downscale(rect.height(), Round::Ceil));
}

inline QRect QtGraphics_Controls::upscale(const QRect& rect)
{
    return QRect(upscale(rect.x(), Round::Floor), upscale(rect.y(), Round::Floor),
                 upscale(rect.width(), Round::Ceil), upscale(rect.height(), Round::Ceil));
}

inline QSize QtGraphics_Controls::downscale(const QSize& size, Round eRound)
{
    return QSize(downscale(size.width(), eRound), downscale(size.height(), eRound));
}

inline QSize QtGraphics_Controls::upscale(const QSize& size, Round eRound)
{
    return QSize(upscale(size.width(), eRound), upscale(size.height(), eRound));
}

inline QPoint QtGraphics_Controls::upscale(const QPoint& point, Round eRound)
{
    return QPoint(upscale(point.x(), eRound), upscale(point.y(), eRound));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
