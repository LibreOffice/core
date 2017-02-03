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

#include <shell/kde_headers.h>

#include "UnxFilePicker.hxx"

#include <unx/salunx.h>
#include <unx/saldata.hxx>
#include <unx/saldisp.hxx>
#include <unx/salgdi.h>
#include <unx/kde/kdedata.hxx>
#include "unx/pixmap.hxx"
#include <unx/i18n_im.hxx>

#include <vcl/settings.hxx>
#include "unx/fontmanager.hxx"
#include <vcl/vclenum.hxx>
#include <rtl/ustrbuf.hxx>

#include <config_vclplug.h>

#include <boost/optional.hpp>

#if ENABLE_TDE
#define QPushButton_String      "TQPushButton"
#define QRadioButton_String     "TQRadioButton"
#define QCheckBox_String        "TQCheckBox"
#define QComboBox_String        "TQComboBox"
#define QLineEdit_String        "TQLineEdit"
#define QSpinWidget_String      "TQSpinWidget"
#define QTabBar_String          "TQTabBar"
#define QTabWidget_String       "TQTabWidget"
#define QListView_String        "TQListView"
#define QScrollBar_String       "TQScrollBar"
#define QMotifPlusStyle_String  "TQMotifPlusStyle"
#define QSGIStyle_String        "TQSGIStyle"
#define QToolBar_String         "TQToolBar"
#define QToolButton_String      "TQToolButton"
#define QMenuBar_String         "TQMenuBar"
#define QPopupMenu_String       "TQPopupMenu"
#define QProgressBar_String     "TQProgressBar"
#define QMotifStyle_String      "TQMotifStyle"
#define QWindowsStyle_String    "TQWindowsStyle"
#else // ENABLE_TDE
#define QPushButton_String      "QPushButton"
#define QRadioButton_String     "QRadioButton"
#define QCheckBox_String        "QCheckBox"
#define QComboBox_String        "QComboBox"
#define QLineEdit_String        "QLineEdit"
#define QSpinWidget_String      "QSpinWidget"
#define QTabBar_String          "QTabBar"
#define QTabWidget_String       "QTabWidget"
#define QListView_String        "QListView"
#define QScrollBar_String       "QScrollBar"
#define QMotifPlusStyle_String  "QMotifPlusStyle"
#define QSGIStyle_String        "QSGIStyle"
#define QToolBar_String         "QToolBar"
#define QToolButton_String      "QToolButton"
#define QMenuBar_String         "QMenuBar"
#define QPopupMenu_String       "QPopupMenu"
#define QProgressBar_String     "QProgressBar"
#define QMotifStyle_String      "QMotifStyle"
#define QWindowsStyle_String    "QWindowsStyle"
#endif // ENABLE_TDE

using namespace ::com::sun::star;

namespace {

/** Style conversion function.

    Conversion function between VCL ControlState together with ImplControlValue
    and Qt state flags.

    @param nState
    State of the widget (default, focused, ...) as defined in Native Widget
    Framework.

    @param aValue
    Value held by the widget (on, off, ...)
*/
QStyle::SFlags vclStateValue2SFlags( ControlState nState,
    const ImplControlValue& aValue )
{
    QStyle::SFlags nStyle =
    ( (nState & ControlState::DEFAULT)?  QStyle::Style_ButtonDefault: QStyle::Style_Default ) |
    ( (nState & ControlState::ENABLED)?  QStyle::Style_Enabled:       QStyle::Style_Default ) |
    ( (nState & ControlState::FOCUSED)?  QStyle::Style_HasFocus:      QStyle::Style_Default ) |
    ( (nState & ControlState::PRESSED)?  QStyle::Style_Down:          QStyle::Style_Raised )  |
    ( (nState & ControlState::SELECTED)? QStyle::Style_Selected :     QStyle::Style_Default ) |
    ( (nState & ControlState::ROLLOVER)? QStyle::Style_MouseOver:     QStyle::Style_Default );

    switch ( aValue.getTristateVal() )
    {
    case ButtonValue::On:    nStyle |= QStyle::Style_On;       break;
    case ButtonValue::Off:   nStyle |= QStyle::Style_Off;      break;
    case ButtonValue::Mixed: nStyle |= QStyle::Style_NoChange; break;
    default: break;
    }

    return nStyle;
}

}

/** Qt implementation of X11Pixmap

    Wrapper around a QPixmap.
 */

class KDEX11Pixmap : public X11Pixmap
{
public:
    KDEX11Pixmap( int nWidth, int nHeight );
    virtual ~KDEX11Pixmap() override {};

    virtual int          GetDepth() const override;
    virtual SalX11Screen GetScreen() const override;
    virtual Pixmap       GetPixmap() const override;
    QPixmap              GetQPixmap() const;

protected:
    QPixmap mqPixmap;
};

KDEX11Pixmap::KDEX11Pixmap( int nWidth, int nHeight )
: X11Pixmap( nWidth, nHeight )
, mqPixmap( nWidth, nHeight )
{
}

int KDEX11Pixmap::GetDepth() const
{
    return mqPixmap.x11Depth();
}

SalX11Screen KDEX11Pixmap::GetScreen() const
{
    return SalX11Screen( mqPixmap.x11Screen() );
}

Pixmap KDEX11Pixmap::GetPixmap() const
{
    return mqPixmap.handle();
}

QPixmap KDEX11Pixmap::GetQPixmap() const
{
    return mqPixmap;
}


/** Cached native widgets.

    A class which caches and paints the native widgets.
*/
class WidgetPainter
{
    protected:
    /** Cached push button.

        It is necessary for the QStyle::drawControl(). The buttons are created
        on demand and they are still hidden (no QWidget::show() is called).
    */
    QPushButton  *m_pPushButton;

    /** Cached radio button.

        @see m_pPushButton
    */
    QRadioButton *m_pRadioButton;

    /** Cached check box.

        @see m_pPushButton
    */
    QCheckBox    *m_pCheckBox;

    /** Cached combo box.

        @see m_pPushButton
    */
    QComboBox    *m_pComboBox;

    /** Cached editable combo box.

        Needed, because some styles do not like dynamic changes
        (QComboBox::setEditable()).

        @see m_pPushButton
    */
    QComboBox    *m_pEditableComboBox;

    /** Cached line edit box.

        @see m_pPushButton
    */
    QLineEdit    *m_pLineEdit;

    /** Cached spin box.

        @see m_pPushButton
    */
    QSpinWidget  *m_pSpinWidget;

    /** Cached spin box'es line edit.

        @see m_pPushButton
    */
    QLineEdit    *m_pSpinEdit;

    /** Cached tab.

        Left, middle, right tab and a tab which is alone.

        @see m_pPushButton
    */
    QTab         *m_pTabLeft, *m_pTabMiddle, *m_pTabRight, *m_pTabAlone;

    /** Cached tab bar's parent widget.

        Needed, because the Qt windows style checks for the availability
        of tab bar's parent. We cannot use m_pTabWidget, because
        TabWidget::setTabBar() and TabWidget::tabBar() methods are
        protected.

        @see m_pPushButton, m_pTabWidget
    */
    QWidget      *m_pTabBarParent;

    /** Cached tab bar widget.

        @see m_pPushButton
    */
    QTabBar      *m_pTabBar;

    /** Cached tab widget.

        We need it to draw the tab page. It cannot be used to draw the
        tabs themselves, because the drawing has to be tweaked a little
        due to not enough information from VCL.

        @see m_pPushButton, m_pTabBarParent
    */
    QTabWidget   *m_pTabWidget;

    /** Cached list view.

        @see m_pPushButton
    */
    QListView    *m_pListView;

    /** Cached scroll bar.

        @see m_pPushButton
    */
    QScrollBar   *m_pScrollBar;

    /** Cached dock area. Needed for proper functionality of tool bars.

      @see m_pPushButton
      */
    QMainWindow  *m_pMainWindow;

    /** Cached tool bar.

      @see m_pPushButton
    */
    QToolBar     *m_pToolBarHoriz, *m_pToolBarVert;

    /** Cached tool button.

      @see m_pPushButton
    */
    QToolButton  *m_pToolButton;

    /** Cached menu bar.

      @see m_pPushButton
    */
    QMenuBar     *m_pMenuBar;

    /** Identifiers of menu bar items.
     */
    int           m_nMenuBarEnabledItem, m_nMenuBarDisabledItem;

    /** Cached popup menu.

      @see m_pPushButton
    */
    QPopupMenu   *m_pPopupMenu;

    /** Identifiers of popup menu items.
     */
    int           m_nPopupMenuEnabledItem, m_nPopupMenuDisabledItem;

    /** cached progress bar
      */
    QProgressBar *m_pProgressBar;

    // TODO other widgets

    public:
    /** Implicit constructor.

        It creates an empty WidgetPainter with all the cached widgets initialized
        to nullptr. The widgets are created on demand and they are still hidden
        (no QWidget::show()), because they are needed just as a parameter for
        QStyle::drawControl().

        @see m_pPushButton
    */
    WidgetPainter();

    /** Destructor.

        Destruct all the cached widgets.
    */
    virtual ~WidgetPainter();

    /** Paints the specified widget to the X window.

        Use X calls to bitblt (bit block transfer) the widget qWidget to
        the window specified by drawable with the style defined by nStyle.

        @param qWidget
        A pointer to the cached widget.

        @param nState
        The state of the control (focused, on/off, ...)

        @param aValue
        The value (true/false, ...)

        @param pGraphics
        The SalGraphics instance to read/write screen.
    */
        bool drawStyledWidget( QWidget *pWidget,
                ControlState nState, const ImplControlValue& aValue,
                X11SalGraphics* pGraphics,
                ControlPart nPart = ControlPart::Entire );

    /** 'Get' method for push button.

        The method returns the cached push button. It is constructed if it
        does not exist. It has nullptr as a parent and it stays hidden, but it
        is necessary for the drawStyledWidget() method.

        @return valid push button.
    */
    QPushButton  *pushButton( const Rectangle& rControlRegion, bool bDefault );

    /** 'Get' method for radio button.

        @see pushButton()
    */
    QRadioButton *radioButton( const Rectangle& rControlRegion );

    /** 'Get' method for check box.

        @see pushButton()
    */
    QCheckBox    *checkBox( const Rectangle& rControlRegion );

    /** 'Get' method for combo box.

        It returns m_pComboBox or m_pEditableComboBox according to
        bEditable.

        @see pushButton(), m_pEditableComboBox
    */
    QComboBox    *comboBox( const Rectangle& rControlRegion, bool bEditable );

    /** 'Get' method for line edit box.

        @see pushButton()
    */
    QLineEdit    *lineEdit( const Rectangle& rControlRegion );

    /** 'Get' method for spin box.

        @see pushButton()
    */
    QSpinWidget  *spinWidget( const Rectangle& rControlRegion );

    /** 'Get' method for tab bar.

        @see pushButton()
    */
    QTabBar      *tabBar( const Rectangle& rControlRegion );

    /** 'Get' method for tab widget.

        @see pushButton()
    */
    QTabWidget   *tabWidget( const Rectangle& rControlRegion );

    /** 'Get' method for list view.

        @see pushButton()
    */
    QListView    *listView( const Rectangle& rControlRegion );

    /** 'Get' method for scroll bar.

        @see pushButton()
    */
    QScrollBar   *scrollBar( const Rectangle& rControlRegion,
        bool bHorizontal, const ImplControlValue& aValue );

    /** 'Get' method for tool bar.

      @see pushButton()
    */
    QToolBar     *toolBar( const Rectangle& rControlRegion, bool bHorizontal );

    /** 'Get' method for tool button.

      @see pushButton()
    */
    QToolButton  *toolButton( const Rectangle& rControlRegion );

    /** 'Get' method for menu bar.

      @see pushButton()
    */
    QMenuBar     *menuBar( const Rectangle& rControlRegion );

    /** 'Get' method for popup menu.

      @see pushButton()
    */
    QPopupMenu   *popupMenu( const Rectangle& rControlRegion );

    /** 'Get' method for progress bar

      @see pushButton()
    */
    QProgressBar *progressBar( const Rectangle& rControlRegion );

    // TODO other widgets

    public:
    /** Convert VCL Rectangle to QRect.

        @param rControlRegion
        The region to convert.

        @return
        The bounding box of the region.
    */
    static QRect region2QRect( const Rectangle& rControlRegion );
};

WidgetPainter::WidgetPainter()
    : m_pPushButton( nullptr ),
      m_pRadioButton( nullptr ),
      m_pCheckBox( nullptr ),
      m_pComboBox( nullptr ),
      m_pEditableComboBox( nullptr ),
      m_pLineEdit( nullptr ),
      m_pSpinWidget( nullptr ),
      m_pSpinEdit( nullptr ),
      m_pTabLeft( nullptr ),
      m_pTabMiddle( nullptr ),
      m_pTabRight( nullptr ),
      m_pTabAlone( nullptr ),
      m_pTabBarParent( nullptr ),
      m_pTabBar( nullptr ),
      m_pTabWidget( nullptr ),
      m_pListView( nullptr ),
      m_pScrollBar( nullptr ),
      m_pMainWindow( nullptr ),
      m_pToolBarHoriz( nullptr ),
      m_pToolBarVert( nullptr ),
      m_pToolButton( nullptr ),
      m_pMenuBar( nullptr ),
      m_nMenuBarEnabledItem( 0 ),
      m_nMenuBarDisabledItem( 0 ),
      m_pPopupMenu( nullptr ),
      m_nPopupMenuEnabledItem( 0 ),
      m_nPopupMenuDisabledItem( 0 ),
      m_pProgressBar( nullptr )
{
}

WidgetPainter::~WidgetPainter()
{
    delete m_pPushButton; m_pPushButton = nullptr;
    delete m_pRadioButton; m_pRadioButton = nullptr;
    delete m_pCheckBox; m_pCheckBox = nullptr;
    delete m_pComboBox; m_pComboBox = nullptr;
    delete m_pEditableComboBox; m_pEditableComboBox = nullptr;
    delete m_pLineEdit; m_pLineEdit = nullptr;
    delete m_pSpinWidget; m_pSpinWidget = nullptr;
    m_pSpinEdit = nullptr; // Deleted in m_pSpinWidget's destructor
    delete m_pTabAlone; m_pTabAlone = nullptr;
    delete m_pTabBarParent; m_pTabBarParent = nullptr;
    m_pTabBar = nullptr;    // Deleted in m_pTabBarParent's destructor
    m_pTabLeft = nullptr;
    m_pTabMiddle = nullptr;
    m_pTabRight = nullptr;
    delete m_pTabWidget; m_pTabWidget = nullptr;
    delete m_pListView; m_pListView = nullptr;
    delete m_pScrollBar; m_pScrollBar = nullptr;
    delete m_pToolBarHoriz; m_pToolBarHoriz = nullptr;
    delete m_pToolBarVert; m_pToolBarVert = nullptr;
    delete m_pMainWindow; m_pMainWindow = nullptr;
    delete m_pToolButton; m_pToolButton = nullptr;
    delete m_pMenuBar; m_pMenuBar = nullptr;
    delete m_pPopupMenu; m_pPopupMenu = nullptr;
    delete m_pProgressBar; m_pProgressBar = nullptr;
}

bool WidgetPainter::drawStyledWidget( QWidget *pWidget,
    ControlState nState, const ImplControlValue& aValue,
    X11SalGraphics* pGraphics, ControlPart nPart )
{
    if ( !pWidget )
        return false;

    // Normalize the widget
    QPoint   qWidgetPos( pWidget->pos() );
    pWidget->move( 0, 0 );

    // Enable/disable the widget
    pWidget->setEnabled( bool(nState & ControlState::ENABLED) );

    // Create pixmap to paint to
    KDEX11Pixmap xPixmap( pWidget->width(), pWidget->height() );
    QPixmap      qPixmap( xPixmap.GetQPixmap() );
    QPainter     qPainter( &qPixmap );
    QRect        qRect( 0, 0, pWidget->width(), pWidget->height() );

    // Use the background of the widget
    qPixmap.fill( pWidget, QPoint(0, 0) );

    // Convert the flags
    QStyle::SFlags nStyle = vclStateValue2SFlags( nState, aValue );

    // Store the widget class
    const char *pClassName = pWidget->className();

    // Draw the widget to the pixmap
    if ( strcmp( QPushButton_String, pClassName ) == 0 )
    {
        // Workaround for the Platinum style.
        // Platinum takes the state directly from the widget, not from SFlags.
        QPushButton *pPushButton = static_cast<QPushButton *>( pWidget->qt_cast( QPushButton_String ) );
        if ( pPushButton )
        {
            pPushButton->setDown   ( nStyle & QStyle::Style_Down );
            pPushButton->setOn     ( nStyle & QStyle::Style_On );
            pPushButton->setEnabled( nStyle & QStyle::Style_Enabled );
        }

        QApplication::style().drawControl( QStyle::CE_PushButton,
            &qPainter, pWidget, qRect,
            pWidget->colorGroup(), nStyle );
    }
    else if ( strcmp( QRadioButton_String, pClassName ) == 0 )
    {
        // Bitblt from the screen, because the radio buttons are usually not
        // rectangular, and there could be a bitmap under them
        pGraphics->FillPixmapFromScreen( &xPixmap, qWidgetPos.x(), qWidgetPos.y() );

        QApplication::style().drawControl( QStyle::CE_RadioButton,
            &qPainter, pWidget, qRect,
            pWidget->colorGroup(), nStyle );
    }
    else if ( strcmp( QCheckBox_String, pClassName ) == 0 )
    {
        QApplication::style().drawControl( QStyle::CE_CheckBox,
            &qPainter, pWidget, qRect,
            pWidget->colorGroup(), nStyle );
    }
    else if ( strcmp( QComboBox_String, pClassName ) == 0 )
    {
        QApplication::style().drawComplexControl( QStyle::CC_ComboBox,
            &qPainter, pWidget, qRect,
            pWidget->colorGroup(), nStyle );

        // Editable combo box uses the background of the associated edit box
        QComboBox *pComboBox = static_cast<QComboBox *>( pWidget->qt_cast( QComboBox_String ) );
        if ( pComboBox && pComboBox->editable() && pComboBox->lineEdit() )
        {
            QColorGroup::ColorRole eColorRole = ( pComboBox->isEnabled() )?
            QColorGroup::Base: QColorGroup::Background;
            qPainter.fillRect(
                QApplication::style().querySubControlMetrics( QStyle::CC_ComboBox,
                pComboBox, QStyle::SC_ComboBoxEditField ),
                pComboBox->lineEdit()->colorGroup().brush( eColorRole ) );
        }
    }
    else if ( strcmp( QLineEdit_String, pClassName ) == 0 )
    {
        QApplication::style().drawPrimitive( QStyle::PE_PanelLineEdit,
            &qPainter, qRect,
            pWidget->colorGroup(), nStyle | QStyle::Style_Sunken );
    }
    else if ( strcmp( QSpinWidget_String, pClassName ) == 0 )
    {
        const SpinbuttonValue* pValue = (aValue.getType() == ControlType::SpinButtons) ? static_cast<const SpinbuttonValue*>(&aValue) : nullptr;

        // Is any of the buttons pressed?
        QStyle::SCFlags eActive = QStyle::SC_None;
        if ( pValue )
        {
            if ( pValue->mnUpperState & ControlState::PRESSED )
                eActive = QStyle::SC_SpinWidgetUp;
            else if ( pValue->mnLowerState & ControlState::PRESSED )
                eActive = QStyle::SC_SpinWidgetDown;

            // Update the enable/disable state of the widget
            if ( ( nState & ControlState::ENABLED ) ||
                ( pValue->mnUpperState & ControlState::ENABLED ) ||
                ( pValue->mnLowerState & ControlState::ENABLED ) )
            {
                pWidget->setEnabled( true );
                nStyle |= QStyle::Style_Enabled;
            }
            else
                pWidget->setEnabled( false );

            // Mouse-over effect
            if ( (pValue->mnUpperState & ControlState::ROLLOVER) ||
                (pValue->mnLowerState & ControlState::ROLLOVER) )
            nStyle |= QStyle::Style_MouseOver;
        }

        // Spin widget uses the background of the associated edit box
        QSpinWidget *pSpinWidget = static_cast<QSpinWidget *>( pWidget->qt_cast( QSpinWidget_String ) );
        if ( pSpinWidget && pSpinWidget->editWidget() )
        {
            QColorGroup::ColorRole eColorRole = ( pSpinWidget->isEnabled() )?
            QColorGroup::Base: QColorGroup::Background;
            qPainter.fillRect(
                QApplication::style().querySubControlMetrics( QStyle::CC_SpinWidget,
                pSpinWidget, QStyle::SC_SpinWidgetEditField ),
                pSpinWidget->editWidget()->colorGroup().brush( eColorRole ) );
        }

        // Adjust the frame (needed for Motif Plus style)
        QRect qFrameRect = QApplication::style().querySubControlMetrics( QStyle::CC_SpinWidget,
            pWidget, QStyle::SC_SpinWidgetFrame );

        QApplication::style().drawComplexControl( QStyle::CC_SpinWidget,
            &qPainter, pWidget, qFrameRect,
            pWidget->colorGroup(), nStyle,
            QStyle::SC_All, eActive );
    }
    else if ( strcmp( QTabBar_String, pClassName ) == 0 )
    {
        const TabitemValue *pValue = static_cast<const TabitemValue *> ( &aValue );

        QTab *pTab = nullptr;
        if ( pValue )
        {
            if ( ( pValue->isFirst() || pValue->isLeftAligned() ) && ( pValue->isLast() || pValue->isRightAligned() ) )
            pTab = m_pTabAlone;
            else if ( pValue->isFirst() || pValue->isLeftAligned() )
            pTab = m_pTabLeft;
            else if ( pValue->isLast() || pValue->isRightAligned() )
            pTab = m_pTabRight;
            else
            pTab = m_pTabMiddle;
        }
        if ( !pTab )
            return false;

        pTab->setRect( qRect );

        QApplication::style().drawControl( QStyle::CE_TabBarTab,
            &qPainter, pWidget, qRect,
            pWidget->colorGroup(), nStyle,
            QStyleOption( pTab ) );
    }
    else if ( strcmp( QTabWidget_String, pClassName ) == 0 )
    {
        QApplication::style().drawPrimitive( QStyle::PE_PanelTabWidget,
            &qPainter, qRect,
            pWidget->colorGroup(), nStyle );
    }
    else if ( strcmp( QListView_String, pClassName ) == 0 )
    {
        QApplication::style().drawPrimitive( QStyle::PE_Panel,
            &qPainter, qRect,
            pWidget->colorGroup(), nStyle | QStyle::Style_Sunken );
    }
    else if ( strcmp( QScrollBar_String, pClassName ) == 0 )
    {
    const ScrollbarValue* pValue = (aValue.getType() == ControlType::Scrollbar) ? static_cast<const ScrollbarValue*>(&aValue) : nullptr;

    QStyle::SCFlags eActive = QStyle::SC_None;
    if ( pValue )
    {
        // Workaround for Style_MouseOver-aware themes.
        // Quite ugly, but I do not know about a better solution.
        const char *pStyleName = QApplication::style().className();
        if ( strcmp( QMotifPlusStyle_String, pStyleName ) == 0 )
        {
            nStyle |= QStyle::Style_MouseOver;
            if ( pValue->mnThumbState & ControlState::ROLLOVER )
                eActive = QStyle::SC_ScrollBarSlider;
        }
        else if ( strcmp( QSGIStyle_String, pStyleName ) == 0 )
        {
            nStyle |= QStyle::Style_MouseOver;
            if ( pValue->mnButton1State & ControlState::ROLLOVER )
                eActive = QStyle::SC_ScrollBarSubLine;
            else if ( pValue->mnButton2State & ControlState::ROLLOVER )
                eActive = QStyle::SC_ScrollBarAddLine;
            else if ( pValue->mnThumbState & ControlState::ROLLOVER )
                eActive = QStyle::SC_ScrollBarSlider;
        }

        if ( pValue->mnButton1State & ControlState::PRESSED )
           eActive = QStyle::SC_ScrollBarSubLine;
        else if ( pValue->mnButton2State & ControlState::PRESSED )
            eActive = QStyle::SC_ScrollBarAddLine;
        else if ( pValue->mnThumbState & ControlState::PRESSED )
            eActive = QStyle::SC_ScrollBarSlider;
        else if ( pValue->mnPage1State & ControlState::PRESSED )
            eActive = QStyle::SC_ScrollBarSubPage;
        else if ( pValue->mnPage2State & ControlState::PRESSED )
            eActive = QStyle::SC_ScrollBarAddPage;

        // Update the enable/disable state of the widget
        if ( ( nState & ControlState::ENABLED ) ||
            ( pValue->mnButton1State & ControlState::ENABLED ) ||
            ( pValue->mnButton2State & ControlState::ENABLED ) ||
            ( pValue->mnThumbState & ControlState::ENABLED ) ||
            ( pValue->mnPage1State & ControlState::ENABLED ) ||
            ( pValue->mnPage2State & ControlState::ENABLED ) )
        {
            pWidget->setEnabled( true );
            nStyle |= QStyle::Style_Enabled;
        }
        else
            pWidget->setEnabled( false );
    }

    // Is it a horizontal scroll bar?
    QScrollBar *pScrollBar = static_cast<QScrollBar *> ( pWidget->qt_cast( QScrollBar_String ) );
    QStyle::StyleFlags eHoriz = QStyle::Style_Default;
    if ( pScrollBar && pScrollBar->orientation() == Qt::Horizontal )
        eHoriz = QStyle::Style_Horizontal;

    QApplication::style().drawComplexControl( QStyle::CC_ScrollBar,
        &qPainter, pWidget, qRect,
        pWidget->colorGroup(), nStyle | eHoriz,
        QStyle::SC_All, eActive );
    }
    else if ( strcmp( QToolBar_String, pClassName ) == 0 )
    {
        QToolBar *pToolBar = static_cast< QToolBar * >( pWidget->qt_cast( QToolBar_String ) );
        bool bIsHorizontal = false;
        if ( pToolBar && pToolBar->orientation() == Qt::Horizontal )
        {
            nStyle |= QStyle::Style_Horizontal;
            bIsHorizontal = true;
        }

        QApplication::style().drawControl( QStyle::CE_DockWindowEmptyArea,
                &qPainter, pWidget, qRect,
                pWidget->colorGroup(), nStyle );

        QApplication::style().drawPrimitive( QStyle::PE_PanelDockWindow,
                &qPainter, qRect, pWidget->colorGroup(), nStyle );

        if ( nPart == ControlPart::ThumbHorz || nPart == ControlPart::ThumbVert )
        {
            const ToolbarValue *pValue = static_cast< const ToolbarValue * >( &aValue );

            QRect qThumbRect = region2QRect( pValue->maGripRect );
            qThumbRect.moveBy( -qWidgetPos.x(), -qWidgetPos.y() );
            if ( bIsHorizontal )
                qThumbRect.addCoords( 0, 2, 0, -3 );    // make the thumb a bit nicer
            else
                qThumbRect.addCoords( 2, 0, -3, 0 );    // make the thumb a bit nicer

            if ( QApplication::style().inherits( "HighColorStyle" ) ||
                 QApplication::style().inherits( "HighContrastStyle" ) ||
                 QApplication::style().inherits( "KeramikStyle" ) ||
                 QApplication::style().inherits( "KThemeStyle" ) ||
                 QApplication::style().inherits( "ThinKeramikStyle" ) )
            {
                // Workaround for the workaround in KStyle::drawPrimitive()
                KStyle *pStyle = static_cast< KStyle * >( &QApplication::style() );
                pStyle->drawKStylePrimitive( KStyle::KPE_ToolBarHandle,
                        &qPainter, pToolBar, qThumbRect,
                        pWidget->colorGroup(), nStyle );
            }
            else
                QApplication::style().drawPrimitive( QStyle::PE_DockWindowHandle,
                        &qPainter, qThumbRect, pWidget->colorGroup(), nStyle );
        }
    }
    else if ( strcmp( QToolButton_String, pClassName ) == 0 )
    {
        if( (nStyle & QStyle::Style_MouseOver) )
            nStyle &= ~QStyle::Style_Off;
        QApplication::style().drawComplexControl( QStyle::CC_ToolButton,
                &qPainter, pWidget, qRect,
                pWidget->colorGroup(), nStyle,
                QStyle::SC_ToolButton );
    }
    else if ( strcmp( QMenuBar_String, pClassName ) == 0 )
    {
        if ( nPart == ControlPart::Entire )
        {
            QApplication::style().drawControl( QStyle::CE_MenuBarEmptyArea,
                    &qPainter, pWidget, qRect,
                    pWidget->colorGroup(), nStyle );
        }
        else if ( nPart == ControlPart::MenuItem )
        {
            int nMenuItem = ( nStyle & QStyle::Style_Enabled )? m_nMenuBarEnabledItem: m_nMenuBarDisabledItem;
            QMenuItem *pMenuItem = static_cast<QMenuBar*>( pWidget )->findItem( nMenuItem );

            if ( ( nStyle & QStyle::Style_MouseOver )
                && QApplication::style().styleHint( QStyle::SH_MenuBar_MouseTracking ) )
                nStyle |= QStyle::Style_Active;

            if ( nStyle & QStyle::Style_Selected )
                nStyle |= QStyle::Style_Active | QStyle::Style_Down | QStyle::Style_HasFocus;

            QApplication::style().drawControl( QStyle::CE_MenuBarItem,
                    &qPainter, pWidget, qRect,
                    pWidget->colorGroup(), nStyle,
                    QStyleOption( pMenuItem ) );
        }
    }
    else if ( strcmp( QPopupMenu_String, pClassName ) == 0 )
    {
        int nMenuItem = ( nStyle & QStyle::Style_Enabled )? m_nPopupMenuEnabledItem: m_nPopupMenuDisabledItem;
        QMenuItem *pMenuItem = static_cast<QPopupMenu*>( pWidget )->findItem( nMenuItem );

        if ( nStyle & QStyle::Style_Selected )
            nStyle |= QStyle::Style_Active;

        QApplication::style().drawControl( QStyle::CE_PopupMenuItem,
                &qPainter, pWidget, qRect,
                pWidget->colorGroup(), nStyle,
                QStyleOption( pMenuItem, 0, 0 ) );
    }
    else if ( strcmp( QProgressBar_String, pClassName ) == 0 )
    {
        long nProgressWidth = aValue.getNumericVal();
        QProgressBar* pProgress = static_cast<QProgressBar*>(pWidget);
        pProgress->setProgress( nProgressWidth, qRect.width() );

        QApplication::style().drawControl( QStyle::CE_ProgressBarGroove,
            &qPainter, pWidget, qRect,
            pWidget->colorGroup(), nStyle );
        QApplication::style().drawControl( QStyle::CE_ProgressBarContents,
            &qPainter, pWidget, qRect,
            pWidget->colorGroup(), nStyle );
    }
    else
        return false;

    // Bitblt it to the screen
    pGraphics->RenderPixmapToScreen( &xPixmap, nullptr, qWidgetPos.x(), qWidgetPos.y() );

    // Restore widget's position
    pWidget->move( qWidgetPos );

    return true;
}

QPushButton *WidgetPainter::pushButton( const Rectangle& rControlRegion,
    bool bDefault )
{
    if ( !m_pPushButton )
       m_pPushButton = new QPushButton( nullptr, "push_button" );

    QRect qRect = region2QRect( rControlRegion );

    // Workaround for broken styles which do not add
    // QStyle::PM_ButtonDefaultIndicator to the size of the default button
    // (for example Keramik)
    // FIXME Fix Keramik style to be consistent with Qt built-in styles. Aargh!
    if ( bDefault )
    {
        QSize qContentsSize( 50, 50 );
        m_pPushButton->setDefault( false );
        QSize qNormalSize = QApplication::style().sizeFromContents( QStyle::CT_PushButton,
            m_pPushButton, qContentsSize );
        m_pPushButton->setDefault( true );
        QSize qDefSize = QApplication::style().sizeFromContents( QStyle::CT_PushButton,
            m_pPushButton, qContentsSize );

        int nIndicatorSize = QApplication::style().pixelMetric(
            QStyle::PM_ButtonDefaultIndicator, m_pPushButton );
        if ( qNormalSize.width() == qDefSize.width() )
            qRect.addCoords( nIndicatorSize, 0, -nIndicatorSize, 0 );
        if ( qNormalSize.height() == qDefSize.height() )
            qRect.addCoords( 0, nIndicatorSize, 0, -nIndicatorSize );
    }

    m_pPushButton->move( qRect.topLeft() );
    m_pPushButton->resize( qRect.size() );
    m_pPushButton->setDefault( bDefault );

    return m_pPushButton;
}

QRadioButton *WidgetPainter::radioButton( const Rectangle& rControlRegion )
{
    if ( !m_pRadioButton )
        m_pRadioButton = new QRadioButton( nullptr, "radio_button" );

    QRect qRect = region2QRect( rControlRegion );

    // Workaround for broken themes which do not honor the given size.
    // Quite ugly, but I do not know about a better solution.
    const char *pStyleName = QApplication::style().className();
    if ( strcmp( "KThemeStyle", pStyleName ) == 0 )
    {
        QRect qOldRect( qRect );

        qRect.setWidth( QApplication::style().pixelMetric(
            QStyle::PM_ExclusiveIndicatorWidth, m_pRadioButton ) );
        qRect.setHeight( QApplication::style().pixelMetric(
            QStyle::PM_ExclusiveIndicatorHeight, m_pRadioButton ) );

        qRect.moveBy( ( qOldRect.width() - qRect.width() ) / 2,
            ( qOldRect.height() - qRect.height() ) / 2 );
    }

    m_pRadioButton->move( qRect.topLeft() );
    m_pRadioButton->resize( qRect.size() );

    return m_pRadioButton;
}

QCheckBox *WidgetPainter::checkBox( const Rectangle& rControlRegion )
{
    if ( !m_pCheckBox )
        m_pCheckBox = new QCheckBox( nullptr, "check_box" );

    QRect qRect = region2QRect( rControlRegion );

    // Workaround for broken themes which do not honor the given size.
    // Quite ugly, but I do not know about a better solution.
    const char *pStyleName = QApplication::style().className();
    if ( strcmp( "KThemeStyle", pStyleName ) == 0 )
    {
        QRect qOldRect( qRect );

        qRect.setWidth( QApplication::style().pixelMetric(
            QStyle::PM_IndicatorWidth, m_pCheckBox ) );
        qRect.setHeight( QApplication::style().pixelMetric(
            QStyle::PM_IndicatorHeight, m_pCheckBox ) );

        qRect.moveBy( ( qOldRect.width() - qRect.width() ) / 2,
            ( qOldRect.height() - qRect.height() ) / 2 );
    }

    m_pCheckBox->move( qRect.topLeft() );
    m_pCheckBox->resize( qRect.size() );

    return m_pCheckBox;
}

QComboBox *WidgetPainter::comboBox( const Rectangle& rControlRegion,
    bool bEditable )
{
    QComboBox *pComboBox = nullptr;
    if ( bEditable )
    {
        if ( !m_pEditableComboBox )
            m_pEditableComboBox = new QComboBox( true, nullptr, "combo_box_edit" );
        pComboBox = m_pEditableComboBox;
    }
    else
    {
        if ( !m_pComboBox )
            m_pComboBox = new QComboBox( false, nullptr, "combo_box" );
        pComboBox = m_pComboBox;
    }

    QRect qRect = region2QRect( rControlRegion );

    pComboBox->move( qRect.topLeft() );
    pComboBox->resize( qRect.size() );

    return pComboBox;
}

QLineEdit *WidgetPainter::lineEdit( const Rectangle& rControlRegion )
{
    if ( !m_pLineEdit )
        m_pLineEdit = new QLineEdit( nullptr, "line_edit" );

    QRect qRect = region2QRect( rControlRegion );

    m_pLineEdit->move( qRect.topLeft() );
    m_pLineEdit->resize( qRect.size() );

    return m_pLineEdit;
}

QSpinWidget *WidgetPainter::spinWidget( const Rectangle& rControlRegion )
{
    if ( !m_pSpinWidget )
    {
        m_pSpinWidget = new QSpinWidget( nullptr, "spin_widget" );
        m_pSpinEdit = new QLineEdit( nullptr, "line_edit_spin" );
        m_pSpinWidget->setEditWidget( m_pSpinEdit );
    }

    QRect qRect = region2QRect( rControlRegion );

    m_pSpinWidget->move( qRect.topLeft() );
    m_pSpinWidget->resize( qRect.size() );
    m_pSpinWidget->arrange();

    return m_pSpinWidget;
}

QTabBar *WidgetPainter::tabBar( const Rectangle& rControlRegion )
{
    if ( !m_pTabBar )
    {
        if ( !m_pTabBarParent )
            m_pTabBarParent = new QWidget( nullptr, "tab_bar_parent" );

        m_pTabBar = new QTabBar( m_pTabBarParent, "tab_bar" );

        m_pTabLeft = new QTab();
        m_pTabMiddle = new QTab();
        m_pTabRight = new QTab();
        m_pTabAlone = new QTab();

        m_pTabBar->addTab( m_pTabLeft );
        m_pTabBar->addTab( m_pTabMiddle );
        m_pTabBar->addTab( m_pTabRight );
    }

    QRect qRect = region2QRect( rControlRegion );

    m_pTabBar->move( qRect.topLeft() );
    m_pTabBar->resize( qRect.size() );

    m_pTabBar->setShape( QTabBar::RoundedAbove );

    return m_pTabBar;
}

QTabWidget *WidgetPainter::tabWidget( const Rectangle& rControlRegion )
{
    if ( !m_pTabWidget )
        m_pTabWidget = new QTabWidget( nullptr, "tab_widget" );

    QRect qRect = region2QRect( rControlRegion );
    --qRect.rTop();

    m_pTabWidget->move( qRect.topLeft() );
    m_pTabWidget->resize( qRect.size() );

    return m_pTabWidget;
}

QListView *WidgetPainter::listView( const Rectangle& rControlRegion )
{
    if ( !m_pListView )
        m_pListView = new QListView( nullptr, "list_view" );

    QRect qRect = region2QRect( rControlRegion );

    m_pListView->move( qRect.topLeft() );
    m_pListView->resize( qRect.size() );

    return m_pListView;
}

QScrollBar *WidgetPainter::scrollBar( const Rectangle& rControlRegion,
    bool bHorizontal, const ImplControlValue& aValue )
{
    if ( !m_pScrollBar )
    {
        m_pScrollBar = new QScrollBar( nullptr, "scroll_bar" );
        m_pScrollBar->setTracking( false );
        m_pScrollBar->setLineStep( 1 );
    }

    QRect qRect = region2QRect( rControlRegion );

    m_pScrollBar->move( qRect.topLeft() );
    m_pScrollBar->resize( qRect.size() );
    m_pScrollBar->setOrientation( bHorizontal? Qt::Horizontal: Qt::Vertical );

    const ScrollbarValue* pValue = (aValue.getType() == ControlType::Scrollbar) ? static_cast<const ScrollbarValue*>(&aValue) : nullptr;
    if ( pValue )
    {
        m_pScrollBar->setMinValue( pValue->mnMin );
        m_pScrollBar->setMaxValue( pValue->mnMax - pValue->mnVisibleSize );
        m_pScrollBar->setValue( pValue->mnCur );
        m_pScrollBar->setPageStep( pValue->mnVisibleSize );
    }

    return m_pScrollBar;
}

QToolBar *WidgetPainter::toolBar( const Rectangle& rControlRegion, bool bHorizontal )
{
    if ( !m_pMainWindow )
        m_pMainWindow = new QMainWindow( nullptr, "main_window" );

    QToolBar *pToolBar;
    if ( bHorizontal )
    {
        if ( !m_pToolBarHoriz )
        {
            m_pToolBarHoriz = new QToolBar( m_pMainWindow, "tool_bar_horiz" );
            m_pMainWindow->moveDockWindow( m_pToolBarHoriz, Qt::DockTop );
        }
        pToolBar = m_pToolBarHoriz;
    }
    else
    {
        if ( !m_pToolBarVert )
        {
            m_pToolBarVert = new QToolBar( m_pMainWindow, "tool_bar_horiz" );
            m_pMainWindow->moveDockWindow( m_pToolBarVert, Qt::DockLeft );
        }
        pToolBar = m_pToolBarVert;
    }

    QRect qRect = region2QRect( rControlRegion );

    pToolBar->move( qRect.topLeft() );
    pToolBar->resize( qRect.size() );

    return pToolBar;
}

QToolButton *WidgetPainter::toolButton( const Rectangle& rControlRegion)
{
    if ( !m_pToolButton )
        m_pToolButton = new QToolButton( nullptr, "tool_button" );

    QRect qRect = region2QRect( rControlRegion );

    m_pToolButton->move( qRect.topLeft() );
    m_pToolButton->resize( qRect.size() );

    return m_pToolButton;
}

QMenuBar *WidgetPainter::menuBar( const Rectangle& rControlRegion)
{
    if ( !m_pMenuBar )
    {
        m_pMenuBar = new QMenuBar( nullptr, "menu_bar" );

        m_nMenuBarEnabledItem = m_pMenuBar->insertItem( "" );
        m_nMenuBarDisabledItem = m_pMenuBar->insertItem( "" );

        m_pMenuBar->setItemEnabled( m_nMenuBarEnabledItem, true );
        m_pMenuBar->setItemEnabled( m_nMenuBarDisabledItem, false );
    }

    QRect qRect = region2QRect( rControlRegion );

    m_pMenuBar->move( qRect.topLeft() );
    m_pMenuBar->resize( qRect.size() );

    return m_pMenuBar;
}

QPopupMenu *WidgetPainter::popupMenu( const Rectangle& rControlRegion)
{
    if ( !m_pPopupMenu )
    {
        m_pPopupMenu = new QPopupMenu( nullptr, "popup_menu" );

        m_nPopupMenuEnabledItem = m_pPopupMenu->insertItem( "" );
        m_nPopupMenuDisabledItem = m_pPopupMenu->insertItem( "" );

        m_pPopupMenu->setItemEnabled( m_nPopupMenuEnabledItem, true );
        m_pPopupMenu->setItemEnabled( m_nPopupMenuDisabledItem, false );
    }

    QRect qRect = region2QRect( rControlRegion );

    m_pPopupMenu->move( qRect.topLeft() );
    m_pPopupMenu->resize( qRect.size() );

    return m_pPopupMenu;
}

QProgressBar *WidgetPainter::progressBar( const Rectangle& rControlRegion )
{
    if ( !m_pProgressBar )
        m_pProgressBar = new QProgressBar( nullptr, "progress_bar" );

    QRect qRect = region2QRect( rControlRegion );

    m_pProgressBar->move( qRect.topLeft() );
    m_pProgressBar->resize( qRect.size() );

    return m_pProgressBar;
}

QRect WidgetPainter::region2QRect( const Rectangle& rControlRegion )
{
    return QRect( QPoint( rControlRegion.Left(), rControlRegion.Top() ),
                  QPoint( rControlRegion.Right(), rControlRegion.Bottom() ) );
}

/** Instance of WidgetPainter.

    It is used to paint the widgets requested by NWF.
*/
static WidgetPainter *pWidgetPainter;

class KDESalGraphics : public X11SalGraphics
{
  public:
    KDESalGraphics() {}
    virtual ~KDESalGraphics() override {}
    virtual bool IsNativeControlSupported( ControlType nType, ControlPart nPart ) override;
    virtual bool hitTestNativeControl( ControlType nType, ControlPart nPart,
                                       const Rectangle& rControlRegion, const Point& aPos,
                                       bool& rIsInside ) override;
    virtual bool drawNativeControl( ControlType nType, ControlPart nPart,
                                    const Rectangle& rControlRegion, ControlState nState,
                                    const ImplControlValue& aValue,
                                    const OUString& aCaption ) override;
    virtual bool getNativeControlRegion( ControlType nType, ControlPart nPart,
                                         const Rectangle& rControlRegion, ControlState nState,
                                         const ImplControlValue& aValue,
                                         const OUString& aCaption,
                                         Rectangle &rNativeBoundingRegion, Rectangle &rNativeContentRegion ) override;
};

bool KDESalGraphics::IsNativeControlSupported( ControlType nType, ControlPart nPart )
{
    return
    ( (nType == ControlType::Pushbutton)  && (nPart == ControlPart::Entire) ) ||
    ( (nType == ControlType::Radiobutton) && (nPart == ControlPart::Entire) ) ||
    ( (nType == ControlType::Checkbox)    && (nPart == ControlPart::Entire) ) ||
    ( (nType == ControlType::Combobox)    && (nPart == ControlPart::Entire || nPart == ControlPart::HasBackgroundTexture) ) ||
    ( (nType == ControlType::Editbox)     && (nPart == ControlPart::Entire || nPart == ControlPart::HasBackgroundTexture) ) ||
    ( (nType == ControlType::Listbox)     && (nPart == ControlPart::Entire || nPart == ControlPart::ListboxWindow || nPart == ControlPart::HasBackgroundTexture ) ) ||
    ( (nType == ControlType::Spinbox)     && (nPart == ControlPart::Entire || nPart == ControlPart::HasBackgroundTexture) ) ||
    // no ControlType::SpinButtons for KDE
    ( (nType == ControlType::TabItem)    && (nPart == ControlPart::Entire) ) ||
    ( (nType == ControlType::TabPane)    && (nPart == ControlPart::Entire) ) ||
    // no ControlType::TabBody for KDE
    ( (nType == ControlType::Scrollbar)   && (nPart == ControlPart::Entire || nPart == ControlPart::DrawBackgroundHorz || nPart == ControlPart::DrawBackgroundVert) ) ||
    ( (nType == ControlType::Scrollbar)   && (nPart == ControlPart::HasThreeButtons) ) || // TODO small optimization is possible here: return this only if the style really has 3 buttons
    // ControlType::Fixedline not supported
    ( (nType == ControlType::Toolbar)     && (nPart == ControlPart::Entire ||
                                      nPart == ControlPart::DrawBackgroundHorz || nPart == ControlPart::DrawBackgroundVert ||
                                      nPart == ControlPart::ThumbHorz || nPart == ControlPart::ThumbVert ||
                                      nPart == ControlPart::Button) ) ||
    ( (nType == ControlType::Menubar)     && (nPart == ControlPart::Entire || nPart == ControlPart::MenuItem) ) ||
    ( (nType == ControlType::MenuPopup)  && (nPart == ControlPart::Entire || nPart == ControlPart::MenuItem) ) ||
    ( (nType == ControlType::Progress)    && (nPart == ControlPart::Entire) )
        ;
}

bool KDESalGraphics::hitTestNativeControl( ControlType nType, ControlPart nPart,
                                           const Rectangle& rControlRegion, const Point& rPos,
                                           bool& rIsInside )
{
    if ( nType == ControlType::Scrollbar )
    {
    // make position relative to rControlRegion
    Point aPos = rPos - rControlRegion.TopLeft();
    rIsInside = false;

    bool bHorizontal = ( nPart == ControlPart::ButtonLeft || nPart == ControlPart::ButtonRight );

    QScrollBar *pScrollBar = pWidgetPainter->scrollBar( rControlRegion,
        bHorizontal, ImplControlValue() );
    QRect qRectSubLine = QApplication::style().querySubControlMetrics(
        QStyle::CC_ScrollBar, pScrollBar, QStyle::SC_ScrollBarSubLine );
    QRect qRectAddLine = QApplication::style().querySubControlMetrics(
        QStyle::CC_ScrollBar, pScrollBar, QStyle::SC_ScrollBarAddLine );

    // There are 2 buttons on the right/bottom side of the scrollbar
    bool bTwoSubButtons = false;

    // It is a Platinum style scroll bar
    bool bPlatinumStyle = false;

    // Workaround for Platinum and 3 button style scroll bars.
    // It makes the right/down button bigger.
    if ( bHorizontal )
    {
        qRectAddLine.setLeft( QApplication::style().querySubControlMetrics(
            QStyle::CC_ScrollBar, pScrollBar,
            QStyle::SC_ScrollBarAddPage ).right() + 1 );
        if ( qRectAddLine.width() > qRectSubLine.width() )
            bTwoSubButtons = true;
        if ( qRectSubLine.left() > QApplication::style().querySubControlMetrics( QStyle::CC_ScrollBar, pScrollBar, QStyle::SC_ScrollBarSubPage ).left() )
            bPlatinumStyle = true;
    }
    else
    {
        qRectAddLine.setTop( QApplication::style().querySubControlMetrics(
            QStyle::CC_ScrollBar, pScrollBar,
            QStyle::SC_ScrollBarAddPage ).bottom() + 1 );
        if ( qRectAddLine.height() > qRectSubLine.height() )
            bTwoSubButtons = true;
        if ( qRectSubLine.top() > QApplication::style().querySubControlMetrics( QStyle::CC_ScrollBar, pScrollBar, QStyle::SC_ScrollBarSubPage ).top() )
            bPlatinumStyle = true;
    }

    switch ( nPart )
    {
        case ControlPart::ButtonLeft:
        if ( !bPlatinumStyle && qRectSubLine.contains( aPos.getX(), aPos.getY() ) )
            rIsInside = true;
        else if ( bTwoSubButtons )
        {
            qRectAddLine.setWidth( qRectAddLine.width() / 2 );
            rIsInside = qRectAddLine.contains( aPos.getX(), aPos.getY() );
        }
        break;

        case ControlPart::ButtonUp:
        if ( !bPlatinumStyle && qRectSubLine.contains( aPos.getX(), aPos.getY() ) )
            rIsInside = true;
        else if ( bTwoSubButtons )
        {
            qRectAddLine.setHeight( qRectAddLine.height() / 2 );
            rIsInside = qRectAddLine.contains( aPos.getX(), aPos.getY() );
        }
        break;

        case ControlPart::ButtonRight:
        if ( bTwoSubButtons )
            qRectAddLine.setLeft( qRectAddLine.left() + qRectAddLine.width() / 2 );

        rIsInside = qRectAddLine.contains( aPos.getX(), aPos.getY() );
        break;

        case ControlPart::ButtonDown:
        if ( bTwoSubButtons )
            qRectAddLine.setTop( qRectAddLine.top() + qRectAddLine.height() / 2 );

        rIsInside = qRectAddLine.contains( aPos.getX(), aPos.getY() );
        break;

        // cases ControlPart::TrackHorzArea and ControlPart::TrackVertArea
        default:
        return false;
    }

    return true;
    }

    return false;
}

bool KDESalGraphics::drawNativeControl( ControlType nType, ControlPart nPart,
                                        const Rectangle& rControlRegion, ControlState nState,
                                        const ImplControlValue& aValue,
                                        const OUString& )
{
    bool bReturn = false;

    if ( (nType == ControlType::Pushbutton) && (nPart == ControlPart::Entire) )
    {
    bReturn = pWidgetPainter->drawStyledWidget(
        pWidgetPainter->pushButton( rControlRegion, bool(nState & ControlState::DEFAULT) ),
        nState, aValue, this );
    }
    else if ( (nType == ControlType::Radiobutton) && (nPart == ControlPart::Entire) )
    {
    bReturn = pWidgetPainter->drawStyledWidget(
        pWidgetPainter->radioButton( rControlRegion ),
        nState, aValue, this );
    }
    else if ( (nType == ControlType::Checkbox) && (nPart == ControlPart::Entire) )
    {
    bReturn = pWidgetPainter->drawStyledWidget(
        pWidgetPainter->checkBox( rControlRegion ),
        nState, aValue, this );
    }
    else if ( (nType == ControlType::Combobox) && (nPart == ControlPart::Entire) )
    {
    bReturn = pWidgetPainter->drawStyledWidget(
        pWidgetPainter->comboBox( rControlRegion, true ),
        nState, aValue, this );
    }
    else if ( (nType == ControlType::Editbox) && (nPart == ControlPart::Entire) )
    {
    bReturn = pWidgetPainter->drawStyledWidget(
        pWidgetPainter->lineEdit( rControlRegion ),
        nState, aValue, this );
    }
    else if ( (nType == ControlType::Listbox) && (nPart == ControlPart::Entire) )
    {
    bReturn = pWidgetPainter->drawStyledWidget(
        pWidgetPainter->comboBox( rControlRegion, false ),
        nState, aValue, this );
    }
    else if ( (nType == ControlType::Listbox) && (nPart == ControlPart::ListboxWindow) )
    {
    bReturn = pWidgetPainter->drawStyledWidget(
        pWidgetPainter->listView( rControlRegion ),
        nState, aValue, this );
    }
    else if ( (nType == ControlType::Spinbox) && (nPart == ControlPart::Entire) )
    {
    bReturn = pWidgetPainter->drawStyledWidget(
        pWidgetPainter->spinWidget( rControlRegion ),
        nState, aValue, this );
    }
    else if ( (nType==ControlType::TabItem) && (nPart == ControlPart::Entire) )
    {
    bReturn = pWidgetPainter->drawStyledWidget(
        pWidgetPainter->tabBar( rControlRegion ),
        nState, aValue, this );
    }
    else if ( (nType==ControlType::TabPane) && (nPart == ControlPart::Entire) )
    {
    bReturn = pWidgetPainter->drawStyledWidget(
        pWidgetPainter->tabWidget( rControlRegion ),
        nState, aValue, this );
    }
    else if ( (nType == ControlType::Scrollbar) && (nPart == ControlPart::DrawBackgroundHorz || nPart == ControlPart::DrawBackgroundVert) )
    {
    bReturn = pWidgetPainter->drawStyledWidget(
        pWidgetPainter->scrollBar( rControlRegion, nPart == ControlPart::DrawBackgroundHorz, aValue ),
        nState, aValue, this );
    }
    else if ( (nType == ControlType::Toolbar) && (nPart == ControlPart::DrawBackgroundHorz || nPart == ControlPart::DrawBackgroundVert || nPart == ControlPart::ThumbHorz || nPart == ControlPart::ThumbVert) )
    {
        bReturn = pWidgetPainter->drawStyledWidget(
                pWidgetPainter->toolBar( rControlRegion, nPart == ControlPart::DrawBackgroundHorz || nPart == ControlPart::ThumbVert ),
                nState, aValue, this, nPart );
    }
    else if ( (nType == ControlType::Toolbar) && (nPart == ControlPart::Button) )
    {
        bReturn = pWidgetPainter->drawStyledWidget(
                pWidgetPainter->toolButton( rControlRegion ),
                nState, aValue, this, nPart );
    }
    else if ( (nType == ControlType::Menubar) && (nPart == ControlPart::Entire || nPart == ControlPart::MenuItem) )
    {
        bReturn = pWidgetPainter->drawStyledWidget(
                pWidgetPainter->menuBar( rControlRegion ),
                nState, aValue, this, nPart );
    }
    else if ( (nType == ControlType::MenuPopup) && (nPart == ControlPart::Entire || nPart == ControlPart::MenuItem) )
    {
        bReturn = pWidgetPainter->drawStyledWidget(
                pWidgetPainter->popupMenu( rControlRegion ),
                nState, aValue, this );
    }
    else if ( (nType == ControlType::Progress) && (nPart == ControlPart::Entire) )
    {
        bReturn = pWidgetPainter->drawStyledWidget(
                pWidgetPainter->progressBar( rControlRegion ),
                nState, aValue, this );
    }

    return bReturn;
}

bool KDESalGraphics::getNativeControlRegion( ControlType nType, ControlPart nPart,
                                             const Rectangle& rControlRegion, ControlState nState,
                                             const ImplControlValue&,
                                             const OUString&,
                                             Rectangle &rNativeBoundingRegion, Rectangle &rNativeContentRegion )
{
    bool bReturn = false;
    QRect qBoundingRect = WidgetPainter::region2QRect( rControlRegion );
    QRect qRect;

    QWidget *pWidget = nullptr;
    switch ( nType )
    {
    // Metrics of the push button
    case ControlType::Pushbutton:
        pWidget = pWidgetPainter->pushButton( rControlRegion, bool( nState & ControlState::DEFAULT ) );

        switch ( nPart )
        {
        case ControlPart::Entire:
            qRect = qBoundingRect;

            if ( nState & ControlState::DEFAULT )
            {
            int nIndicatorSize = QApplication::style().pixelMetric(
                QStyle::PM_ButtonDefaultIndicator, pWidget );
            qBoundingRect.addCoords( -nIndicatorSize, -nIndicatorSize,
                nIndicatorSize, nIndicatorSize );
            bReturn = true;
            }
            break;
        default:
            break;
        }
        break;

        // Metrics of the radio button
        case ControlType::Radiobutton:
            pWidget = pWidgetPainter->radioButton( rControlRegion );

            if ( nPart == ControlPart::Entire )
            {
                qRect.setWidth( QApplication::style().pixelMetric( QStyle::PM_ExclusiveIndicatorWidth, pWidget ) );
                qRect.setHeight( QApplication::style().pixelMetric( QStyle::PM_ExclusiveIndicatorHeight, pWidget ) );

                bReturn = true;
            }
            break;

        // Metrics of the check box
        case ControlType::Checkbox:
            pWidget = pWidgetPainter->checkBox( rControlRegion );

            if ( nPart == ControlPart::Entire )
            {
                qRect.setWidth( QApplication::style().pixelMetric( QStyle::PM_IndicatorWidth, pWidget ) );
                qRect.setHeight( QApplication::style().pixelMetric( QStyle::PM_IndicatorHeight, pWidget ) );

                bReturn = true;
            }
            break;

    // Metrics of the combo box
    case ControlType::Combobox:
    case ControlType::Listbox:
        pWidget = pWidgetPainter->comboBox( rControlRegion, ( nType == ControlType::Combobox ) );
        switch ( nPart )
        {
        case ControlPart::ButtonDown:
            qRect = QApplication::style().querySubControlMetrics(
                QStyle::CC_ComboBox, pWidget, QStyle::SC_ComboBoxArrow );
            qRect.setLeft( QApplication::style().querySubControlMetrics(
                QStyle::CC_ComboBox, pWidget,
                QStyle::SC_ComboBoxEditField ).right() + 1 );
            qRect.moveBy( qBoundingRect.left(), qBoundingRect.top() );
            bReturn = true;
            break;

        case ControlPart::SubEdit:
            qRect = QApplication::style().querySubControlMetrics(
                QStyle::CC_ComboBox, pWidget, QStyle::SC_ComboBoxEditField );
            qRect.moveBy( qBoundingRect.left(), qBoundingRect.top() );
            bReturn = true;
            break;

        default:
            break;
        }
        break;

    // Metrics of the spin box
    case ControlType::Spinbox:
        pWidget = pWidgetPainter->spinWidget( rControlRegion );
        switch ( nPart )
        {
        case ControlPart::ButtonUp:
            qRect = QApplication::style().querySubControlMetrics(
                QStyle::CC_SpinWidget, pWidget, QStyle::SC_SpinWidgetUp );
            bReturn = true;
            qRect.moveBy( qBoundingRect.left(), qBoundingRect.top() );
            break;

        case ControlPart::ButtonDown:
            qRect = QApplication::style().querySubControlMetrics(
                QStyle::CC_SpinWidget, pWidget, QStyle::SC_SpinWidgetDown );
            bReturn = true;
            qRect.moveBy( qBoundingRect.left(), qBoundingRect.top() );
            break;

        case ControlPart::SubEdit:
            qRect = QApplication::style().querySubControlMetrics(
                QStyle::CC_SpinWidget, pWidget, QStyle::SC_SpinWidgetEditField );
            qRect.moveBy( qBoundingRect.left(), qBoundingRect.top() );
            bReturn = true;
            break;

        default:
            break;
        }
        break;

    // Metrics of the scroll bar
    case ControlType::Scrollbar:
        pWidget = pWidgetPainter->scrollBar( rControlRegion,
            ( nPart == ControlPart::ButtonLeft || nPart == ControlPart::ButtonRight ),
            ImplControlValue() );
        switch ( nPart )
        {
        case ControlPart::ButtonLeft:
        case ControlPart::ButtonUp:
            qRect = QApplication::style().querySubControlMetrics(
                QStyle::CC_ScrollBar, pWidget, QStyle::SC_ScrollBarSubLine );

            // Workaround for Platinum style scroll bars. It makes the
            // left/up button invisible.
            if ( nPart == ControlPart::ButtonLeft )
            {
            if ( qRect.left() > QApplication::style().querySubControlMetrics(
                    QStyle::CC_ScrollBar, pWidget,
                    QStyle::SC_ScrollBarSubPage ).left() )
            {
                qRect.setLeft( 0 );
                qRect.setRight( 0 );
            }
            }
            else
            {
            if ( qRect.top() > QApplication::style().querySubControlMetrics(
                    QStyle::CC_ScrollBar, pWidget,
                    QStyle::SC_ScrollBarSubPage ).top() )
            {
                qRect.setTop( 0 );
                qRect.setBottom( 0 );
            }
            }

            qRect.moveBy( qBoundingRect.left(), qBoundingRect.top() );

            bReturn = true;
            break;

        case ControlPart::ButtonRight:
        case ControlPart::ButtonDown:
            qRect = QApplication::style().querySubControlMetrics(
                QStyle::CC_ScrollBar, pWidget, QStyle::SC_ScrollBarAddLine );

            // Workaround for Platinum and 3 button style scroll bars.
            // It makes the right/down button bigger.
            if ( nPart == ControlPart::ButtonRight )
                qRect.setLeft( QApplication::style().querySubControlMetrics(
                    QStyle::CC_ScrollBar, pWidget,
                    QStyle::SC_ScrollBarAddPage ).right() + 1 );
            else
                qRect.setTop( QApplication::style().querySubControlMetrics(
                    QStyle::CC_ScrollBar, pWidget,
                    QStyle::SC_ScrollBarAddPage ).bottom() + 1 );

            qRect.moveBy( qBoundingRect.left(), qBoundingRect.top() );

            bReturn = true;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    // Fill rNativeBoundingRegion and rNativeContentRegion
    if ( bReturn )
    {
    // Bounding region
    Point aBPoint( qBoundingRect.x(), qBoundingRect.y() );
    Size aBSize( qBoundingRect.width(), qBoundingRect.height() );
    rNativeBoundingRegion = Rectangle( aBPoint, aBSize );

    // vcl::Region of the content
    Point aPoint( qRect.x(), qRect.y() );
    Size  aSize( qRect.width(), qRect.height() );
    rNativeContentRegion = Rectangle( aPoint, aSize );
    }

    return bReturn;
}

// KDESalFrame implementation

KDESalFrame::KDESalFrame( SalFrame* pParent, SalFrameStyleFlags nStyle ) :
    X11SalFrame( pParent, nStyle )
{
}

void KDESalFrame::Show( bool bVisible, bool bNoActivate )
{
    if ( !GetParent() && ! (GetStyle() & SalFrameStyleFlags::INTRO) )
    {
        KDEXLib* pXLib = static_cast<KDEXLib*>(GetDisplay()->GetXLib());
        pXLib->doStartup();
    }
    X11SalFrame::Show( bVisible, bNoActivate );
}

/** Helper function to convert colors.
*/
static Color toColor( const QColor &rColor )
{
    return Color( rColor.red(), rColor.green(), rColor.blue() );
}

/** Helper function to read untranslated text entry from KConfig configuration repository.
*/
static OUString readEntryUntranslated( KConfig *pConfig, const char *pKey )
{
    return OUString::createFromAscii( pConfig->readEntryUntranslated( pKey ).ascii() );
}

/** Helper function to read color from KConfig configuration repository.
*/
static Color readColor( KConfig *pConfig, const char *pKey )
{
    return toColor( pConfig->readColorEntry( pKey ) );
}

/** Helper function to add information to Font from QFont.

    Mostly grabbed from the Gtk+ vclplug (salnativewidgets-gtk.cxx).
*/
static vcl::Font toFont( const QFont &rQFont, const css::lang::Locale& rLocale )
{
    psp::FastPrintFontInfo aInfo;
    QFontInfo qFontInfo( rQFont );

    // set family name
    aInfo.m_aFamilyName = OUString( rQFont.family().utf8(), strlen( rQFont.family().utf8() ), RTL_TEXTENCODING_UTF8 );

    // set italic
    aInfo.m_eItalic = ( qFontInfo.italic()? ITALIC_NORMAL: ITALIC_NONE );

    // set weight
    int nWeight = qFontInfo.weight();
    if ( nWeight <= QFont::Light )
        aInfo.m_eWeight = WEIGHT_LIGHT;
    else if ( nWeight <= QFont::Normal )
        aInfo.m_eWeight = WEIGHT_NORMAL;
    else if ( nWeight <= QFont::DemiBold )
        aInfo.m_eWeight = WEIGHT_SEMIBOLD;
    else if ( nWeight <= QFont::Bold )
        aInfo.m_eWeight = WEIGHT_BOLD;
    else
        aInfo.m_eWeight = WEIGHT_ULTRABOLD;

    // set width
    int nStretch = rQFont.stretch();
    if ( nStretch <= QFont::UltraCondensed )
        aInfo.m_eWidth = WIDTH_ULTRA_CONDENSED;
    else if ( nStretch <= QFont::ExtraCondensed )
        aInfo.m_eWidth = WIDTH_EXTRA_CONDENSED;
    else if ( nStretch <= QFont::Condensed )
        aInfo.m_eWidth = WIDTH_CONDENSED;
    else if ( nStretch <= QFont::SemiCondensed )
        aInfo.m_eWidth = WIDTH_SEMI_CONDENSED;
    else if ( nStretch <= QFont::Unstretched )
        aInfo.m_eWidth = WIDTH_NORMAL;
    else if ( nStretch <= QFont::SemiExpanded )
        aInfo.m_eWidth = WIDTH_SEMI_EXPANDED;
    else if ( nStretch <= QFont::Expanded )
        aInfo.m_eWidth = WIDTH_EXPANDED;
    else if ( nStretch <= QFont::ExtraExpanded )
        aInfo.m_eWidth = WIDTH_EXTRA_EXPANDED;
    else
        aInfo.m_eWidth = WIDTH_ULTRA_EXPANDED;

    SAL_INFO( "vcl.kde", "font name BEFORE system match: \"" << aInfo.m_aFamilyName << "\"" );

    // match font to e.g. resolve "Sans"
    psp::PrintFontManager::get().matchFont( aInfo, rLocale );

    SAL_INFO( "vcl.kde", "font match " <<
              (aInfo.m_nID != 0 ? "succeeded" : "failed") <<
              ", name AFTER: \"" << aInfo.m_aFamilyName << "\"" );

    // font height
    int nPointHeight = qFontInfo.pointSize();
    if ( nPointHeight <= 0 )
        nPointHeight = rQFont.pointSize();

    // Create the font
    vcl::Font aFont( aInfo.m_aFamilyName, Size( 0, nPointHeight ) );
    if( aInfo.m_eWeight != WEIGHT_DONTKNOW )
        aFont.SetWeight( aInfo.m_eWeight );
    if( aInfo.m_eWidth != WIDTH_DONTKNOW )
        aFont.SetWidthType( aInfo.m_eWidth );
    if( aInfo.m_eItalic != ITALIC_DONTKNOW )
        aFont.SetItalic( aInfo.m_eItalic );
    if( aInfo.m_ePitch != PITCH_DONTKNOW )
        aFont.SetPitch( aInfo.m_ePitch );

    return aFont;
}

/** Implementation of KDE integration's main method.
*/
void KDESalFrame::UpdateSettings( AllSettings& rSettings )
{
    StyleSettings aStyleSettings( rSettings.GetStyleSettings() );
    bool bSetTitleFont = false;

    aStyleSettings.SetToolbarIconSize( ToolbarIconSize::Large );

    // WM settings
    KConfig *pConfig = KGlobal::config();
    if ( pConfig )
    {
        pConfig->setGroup( "WM" );
        const char *pKey;

        pKey = "activeBackground";
        if ( pConfig->hasKey( pKey ) )
            aStyleSettings.SetActiveColor( readColor( pConfig, pKey ) );

        pKey = "inactiveBackground";
        if ( pConfig->hasKey( pKey ) )
            aStyleSettings.SetDeactiveColor( readColor( pConfig, pKey ) );

        pKey = "inactiveForeground";
        if ( pConfig->hasKey( pKey ) )
            aStyleSettings.SetDeactiveTextColor( readColor( pConfig, pKey ) );

        pKey = "activeForeground";
        if ( pConfig->hasKey( pKey ) )
            aStyleSettings.SetActiveTextColor( readColor( pConfig, pKey ) );

        pKey = "titleFont";
        if ( pConfig->hasKey( pKey ) )
        {
            vcl::Font aFont = toFont( pConfig->readFontEntry( pKey ), rSettings.GetUILanguageTag().getLocale() );
            aStyleSettings.SetTitleFont( aFont );
            bSetTitleFont = true;
        }

        pConfig->setGroup( "Icons" );

        pKey = "Theme";
        if ( pConfig->hasKey( pKey ) )
            aStyleSettings.SetPreferredIconTheme( readEntryUntranslated( pConfig, pKey ) );
    }

    // General settings
    QColorGroup qColorGroup = QApplication::palette().active();

    Color aFore = toColor( qColorGroup.foreground() );
    Color aBack = toColor( qColorGroup.background() );
    Color aText = toColor( qColorGroup.text() );
    Color aBase = toColor( qColorGroup.base() );

    // Foreground
    aStyleSettings.SetRadioCheckTextColor( aFore );
    aStyleSettings.SetLabelTextColor( aFore );
    aStyleSettings.SetDialogTextColor( aFore );
    aStyleSettings.SetGroupTextColor( aFore );

    // Text
    aStyleSettings.SetFieldTextColor( aText );
    aStyleSettings.SetFieldRolloverTextColor( aText );
    aStyleSettings.SetWindowTextColor( aText );
    aStyleSettings.SetToolTextColor( aText );
    aStyleSettings.SetHelpTextColor( aText );

    // Base
    aStyleSettings.SetFieldColor( aBase );
    aStyleSettings.SetHelpColor( aBase );
    aStyleSettings.SetWindowColor( aBase );
    aStyleSettings.SetActiveTabColor( aBase );

    // Buttons
    aStyleSettings.SetButtonTextColor( toColor( qColorGroup.buttonText() ) );
    aStyleSettings.SetButtonRolloverTextColor( toColor( qColorGroup.buttonText() ) );

    // Tabs
    aStyleSettings.SetTabTextColor( toColor( qColorGroup.buttonText() ) );
    aStyleSettings.SetTabRolloverTextColor( toColor( qColorGroup.buttonText() ) );
    aStyleSettings.SetTabHighlightTextColor( toColor( qColorGroup.buttonText() ) );

    // Disable color
    aStyleSettings.SetDisableColor( toColor( qColorGroup.mid() ) );

    // Workspace
    aStyleSettings.SetWorkspaceColor( toColor( qColorGroup.mid() ) );

    // Background
    aStyleSettings.Set3DColors( aBack );
    aStyleSettings.SetFaceColor( aBack );
    aStyleSettings.SetInactiveTabColor( aBack );
    aStyleSettings.SetDialogColor( aBack );
    aStyleSettings.SetCheckedColorSpecialCase( );

    // Selection
    aStyleSettings.SetHighlightColor( toColor( qColorGroup.highlight() ) );
    aStyleSettings.SetHighlightTextColor( toColor( qColorGroup.highlightedText() ) );

    // Font
    vcl::Font aFont = toFont( QApplication::font(), rSettings.GetUILanguageTag().getLocale() );

    aStyleSettings.SetAppFont( aFont );
    aStyleSettings.SetHelpFont( aFont );
    aStyleSettings.SetMenuFont( aFont ); // will be changed according to pMenuBar
    aStyleSettings.SetToolFont( aFont ); // will be changed according to pToolBar
    aStyleSettings.SetLabelFont( aFont );
    aStyleSettings.SetRadioCheckFont( aFont );
    aStyleSettings.SetPushButtonFont( aFont );
    aStyleSettings.SetFieldFont( aFont );
    aStyleSettings.SetIconFont( aFont );
    aStyleSettings.SetTabFont( aFont );
    aStyleSettings.SetGroupFont( aFont );

    aFont.SetWeight( WEIGHT_BOLD );
    if( !bSetTitleFont )
        aStyleSettings.SetTitleFont( aFont );
    aStyleSettings.SetFloatTitleFont( aFont );

    int flash_time = QApplication::cursorFlashTime();
    aStyleSettings.SetCursorBlinkTime( flash_time != 0 ? flash_time/2 : STYLE_CURSOR_NOBLINKTIME );

    KMainWindow qMainWindow;
    qMainWindow.createGUI( "/dev/null" ); // hack

    // Menu
    aStyleSettings.SetSkipDisabledInMenus( true );
    KMenuBar *pMenuBar = qMainWindow.menuBar();
    if ( pMenuBar )
    {
        // Color
        QColorGroup qMenuCG = pMenuBar->colorGroup();

        // Menu text and background color, theme specific
        Color aMenuFore = toColor( qMenuCG.foreground() );
        Color aMenuBack = toColor( qMenuCG.background() );
        if ( QApplication::style().inherits( "LightStyleV2" ) ||
             QApplication::style().inherits( "LightStyleV3" ) ||
             ( QApplication::style().inherits( QMotifStyle_String ) && !QApplication::style().inherits( QSGIStyle_String ) ) ||
             QApplication::style().inherits( QWindowsStyle_String ) )
        {
            aMenuFore = toColor( qMenuCG.buttonText() );
            aMenuBack = toColor( qMenuCG.button() );
        }

        aStyleSettings.SetMenuTextColor( aMenuFore );
        aStyleSettings.SetMenuBarTextColor( aStyleSettings.GetPersonaMenuBarTextColor().get_value_or( aMenuFore ) );
        aStyleSettings.SetMenuColor( aMenuBack );
        aStyleSettings.SetMenuBarColor( aMenuBack );
        aStyleSettings.SetMenuHighlightColor( toColor ( qMenuCG.highlight() ) );

        // Menu items highlight text color, theme specific
        if ( QApplication::style().inherits( "HighContrastStyle" ) ||
             QApplication::style().inherits( "KeramikStyle" ) ||
             QApplication::style().inherits( QWindowsStyle_String ) ||
             QApplication::style().inherits( "ThinKeramikStyle" ) ||
             QApplication::style().inherits( "PlastikStyle" ) )
        {
            aStyleSettings.SetMenuHighlightTextColor( toColor ( qMenuCG.highlightedText() ) );
        }
        else
            aStyleSettings.SetMenuHighlightTextColor( aMenuFore );

        // set special menubar highlight text color
        if ( QApplication::style().inherits( "HighContrastStyle" ) )
            ImplGetSVData()->maNWFData.maMenuBarHighlightTextColor = toColor( qMenuCG.highlightedText() );
        else
            ImplGetSVData()->maNWFData.maMenuBarHighlightTextColor = aMenuFore;

        // set menubar rollover color
        if ( QApplication::style().styleHint( QStyle::SH_MenuBar_MouseTracking ) )
        {
            aStyleSettings.SetMenuBarRolloverColor( toColor ( qMenuCG.highlight() ) );
            aStyleSettings.SetMenuBarRolloverTextColor( ImplGetSVData()->maNWFData.maMenuBarHighlightTextColor );
        }
        else
        {
            aStyleSettings.SetMenuBarRolloverColor( aMenuBack );
            aStyleSettings.SetMenuBarRolloverTextColor( aMenuFore );
        }
        aStyleSettings.SetMenuBarHighlightTextColor(aStyleSettings.GetMenuHighlightTextColor());

        // Font
        aFont = toFont( pMenuBar->font(), rSettings.GetUILanguageTag().getLocale() );
        aStyleSettings.SetMenuFont( aFont );
    }

    // Tool bar
    KToolBar *pToolBar = qMainWindow.toolBar();
    if ( pToolBar )
    {
        aFont = toFont( pToolBar->font(), rSettings.GetUILanguageTag().getLocale() );
        aStyleSettings.SetToolFont( aFont );
    }

    // Scroll bar size
    aStyleSettings.SetScrollBarSize( QApplication::style().pixelMetric( QStyle::PM_ScrollBarExtent ) );

    rSettings.SetStyleSettings( aStyleSettings );
}

SalGraphics* KDESalFrame::AcquireGraphics()
{
    if( GetWindow() )
    {
        for( int i = 0; i < nMaxGraphics; i++ )
        {
            if( ! m_aGraphics[i].bInUse )
            {
                m_aGraphics[i].bInUse = true;
                if( ! m_aGraphics[i].pGraphics )
                {
                    m_aGraphics[i].pGraphics = new KDESalGraphics();
                    m_aGraphics[i].pGraphics->Init( this, GetWindow(), GetScreenNumber() );
                }
                return m_aGraphics[i].pGraphics;
            }
        }
    }

    return nullptr;
}

void KDESalFrame::ReleaseGraphics( SalGraphics *pGraphics )
{
    for( int i = 0; i < nMaxGraphics; i++ )
    {
        if( m_aGraphics[i].pGraphics == pGraphics )
        {
            m_aGraphics[i].bInUse = false;
            break;
        }
    }
}

void KDESalFrame::updateGraphics( bool bClear )
{
    Drawable aDrawable = bClear ? None : GetWindow();
    for( int i = 0; i < nMaxGraphics; i++ )
    {
        if( m_aGraphics[i].bInUse )
            m_aGraphics[i].pGraphics->SetDrawable( aDrawable, GetScreenNumber() );
    }
}

KDESalFrame::~KDESalFrame()
{
}

KDESalFrame::GraphicsHolder::~GraphicsHolder()
{
    delete pGraphics;
}

// KDESalInstance implementation

SalFrame * KDESalInstance::CreateFrame( SalFrame *pParent, SalFrameStyleFlags nStyle )
{
    return new KDESalFrame( pParent, nStyle );
}

uno::Reference< ui::dialogs::XFilePicker2 > KDESalInstance::createFilePicker(
        const uno::Reference< uno::XComponentContext >& xMSF )
{
    return uno::Reference< ui::dialogs::XFilePicker2 >(
                new UnxFilePicker( xMSF ) );
}

SalX11Display* KDESalInstance::CreateDisplay() const
{
    return new SalKDEDisplay( QPaintDevice::x11AppDisplay() );
}

// KDESalData pieces

// Create the widget painter so we have some control over
// the destruction sequence, so Qt doesn't die in action.

void KDEData::initNWF()
{
    ImplSVData *pSVData = ImplGetSVData();
    // draw toolbars on separate lines
    pSVData->maNWFData.mbDockingAreaSeparateTB = true;

    pWidgetPainter = new WidgetPainter();
}

void KDEData::deInitNWF()
{
    delete pWidgetPainter;
    pWidgetPainter = nullptr;

    // We have to destroy the style early
    QApplication::setStyle( nullptr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
