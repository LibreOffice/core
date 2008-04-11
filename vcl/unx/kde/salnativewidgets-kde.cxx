/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salnativewidgets-kde.cxx,v $
 * $Revision: 1.26 $
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
#include "kde_headers.h"

#include <salunx.h>
#include <saldata.hxx>
#include <saldisp.hxx>

#ifndef _SV_SALGDI_HXX
#include <salgdi.h>
#endif

#ifndef _SV_SALGDI_HXX
#include <salframe.h>
#endif
#include <vcl/settings.hxx>
#include <rtl/ustrbuf.hxx>
#include <plugins/kde/kdedata.hxx>
#include <iostream>

#include <pspgraphics.h>

using namespace ::rtl;

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
        to NULL. The widgets are created on demand and they are still hidden
        (no QWidget::show()), because they are needed just as a parameter for
        QStyle::drawControl().

        @see m_pPushButton
    */
    WidgetPainter( void );

    /** Destructor.

        Destruct all the cached widgets.
    */
    virtual ~WidgetPainter( void );

    /** Paints the specified widget to the X window.

        Use X calls to bitblt (bit block transfer) the widget qWidget to
        the window specified by drawable with the style defined by nStyle.

        @param qWidget
        A pointer to the cached widget.

        @param nState
        The state of the control (focused, on/off, ...)

        @param aValue
        The value (true/false, ...)

        @param dpy
        The display to be used by the X calls.

        @param drawable
        The destination X window.

        @param gc
        The graphics context.
    */
        BOOL drawStyledWidget( QWidget *pWidget,
                ControlState nState, const ImplControlValue& aValue,
                Display *dpy, XLIB_Window drawable, int nScreen, int nDepth, GC gc,
                ControlPart nPart = PART_ENTIRE_CONTROL );

    /** 'Get' method for push button.

        The method returns the cached push button. It is constructed if it
        does not exist. It has NULL as a parent and it stays hidden, but it
        is necessary for the drawStyledWidget() method.

        @return valid push button.
    */
    QPushButton  *pushButton( const Region& rControlRegion, BOOL bDefault );

    /** 'Get' method for radio button.

        @see pushButton()
    */
    QRadioButton *radioButton( const Region& rControlRegion );

    /** 'Get' method for check box.

        @see pushButton()
    */
    QCheckBox    *checkBox( const Region& rControlRegion );

    /** 'Get' method for combo box.

        It returns m_pComboBox or m_pEditableComboBox according to
        bEditable.

        @see pushButton(), m_pEditableComboBox
    */
    QComboBox    *comboBox( const Region& rControlRegion, BOOL bEditable );

    /** 'Get' method for line edit box.

        @see pushButton()
    */
    QLineEdit    *lineEdit( const Region& rControlRegion );

    /** 'Get' method for spin box.

        @see pushButton()
    */
    QSpinWidget  *spinWidget( const Region& rControlRegion );

    /** 'Get' method for tab bar.

        @see pushButton()
    */
    QTabBar      *tabBar( const Region& rControlRegion );

    /** 'Get' method for tab widget.

        @see pushButton()
    */
    QTabWidget   *tabWidget( const Region& rControlRegion );

    /** 'Get' method for list view.

        @see pushButton()
    */
    QListView    *listView( const Region& rControlRegion );

    /** 'Get' method for scroll bar.

        @see pushButton()
    */
    QScrollBar   *scrollBar( const Region& rControlRegion,
        BOOL bHorizontal, const ImplControlValue& aValue );

    /** 'Get' method for tool bar.

      @see pushButton()
    */
    QToolBar     *toolBar( const Region& rControlRegion, BOOL bHorizontal );

    /** 'Get' method for tool button.

      @see pushButton()
    */
    QToolButton  *toolButton( const Region& rControlRegion );

    /** 'Get' method for menu bar.

      @see pushButton()
    */
    QMenuBar     *menuBar( const Region& rControlRegion );

    /** 'Get' method for popup menu.

      @see pushButton()
    */
    QPopupMenu   *popupMenu( const Region& rControlRegion );

    /** 'Get' method for progress bar

      @see pushButton()
    */
    QProgressBar *progressBar( const Region& rControlRegion );

    // TODO other widgets

    protected:
    /** Style conversion function.

        Conversion function between VCL ControlState together with
        ImplControlValue and Qt state flags.

        @param nState
        State of the widget (default, focused, ...) as defined in Native
        Widget Framework.

        @param aValue
        Value held by the widget (on, off, ...)
    */
    QStyle::SFlags vclStateValue2SFlags( ControlState nState, const ImplControlValue& aValue );

    public:
    /** Convert VCL Region to QRect.

        @param rControlRegion
        The region to convert.

        @return
        The bounding box of the region.
    */
    static QRect region2QRect( const Region& rControlRegion );
};

WidgetPainter::WidgetPainter( void )
    : m_pPushButton( NULL ),
      m_pRadioButton( NULL ),
      m_pCheckBox( NULL ),
      m_pComboBox( NULL ),
      m_pEditableComboBox( NULL ),
      m_pLineEdit( NULL ),
      m_pSpinWidget( NULL ),
      m_pSpinEdit( NULL ),
      m_pTabLeft( NULL ),
      m_pTabMiddle( NULL ),
      m_pTabRight( NULL ),
      m_pTabAlone( NULL ),
      m_pTabBarParent( NULL ),
      m_pTabBar( NULL ),
      m_pTabWidget( NULL ),
      m_pListView( NULL ),
      m_pScrollBar( NULL ),
      m_pMainWindow( NULL ),
      m_pToolBarHoriz( NULL ),
      m_pToolBarVert( NULL ),
      m_pToolButton( NULL ),
      m_pMenuBar( NULL ),
      m_pPopupMenu( NULL ),
      m_pProgressBar( NULL )
{
}

WidgetPainter::~WidgetPainter( void )
{
    delete m_pPushButton, m_pPushButton = NULL;
    delete m_pRadioButton, m_pRadioButton = NULL;
    delete m_pCheckBox, m_pCheckBox = NULL;
    delete m_pComboBox, m_pComboBox = NULL;
    delete m_pEditableComboBox, m_pEditableComboBox = NULL;
    delete m_pLineEdit, m_pLineEdit = NULL;
    delete m_pSpinWidget, m_pSpinWidget = NULL;
    m_pSpinEdit = NULL; // Deleted in m_pSpinWidget's destructor
    delete m_pTabAlone, m_pTabAlone = NULL;
    delete m_pTabBarParent, m_pTabBarParent = NULL;
    m_pTabBar = NULL;    // Deleted in m_pTabBarParent's destructor
    m_pTabLeft = NULL;
    m_pTabMiddle = NULL;
    m_pTabRight = NULL;
    delete m_pTabWidget, m_pTabWidget = NULL;
    delete m_pListView, m_pListView = NULL;
    delete m_pScrollBar, m_pScrollBar = NULL;
    delete m_pToolBarHoriz, m_pToolBarHoriz = NULL;
    delete m_pToolBarVert, m_pToolBarVert = NULL;
    delete m_pMainWindow, m_pMainWindow = NULL;
    delete m_pToolButton, m_pToolButton = NULL;
    delete m_pMenuBar, m_pMenuBar = NULL;
    delete m_pPopupMenu, m_pPopupMenu = NULL;
    delete m_pProgressBar, m_pProgressBar = NULL;
}

BOOL WidgetPainter::drawStyledWidget( QWidget *pWidget,
    ControlState nState, const ImplControlValue& aValue,
        Display *dpy, XLIB_Window drawable, int nScreen, int nDepth, GC gc,
        ControlPart nPart )
{
    if ( !pWidget )
    return FALSE;

    // Normalize the widget
    QPoint   qWidgetPos( pWidget->pos() );
    pWidget->move( 0, 0 );

    // Enable/disable the widget
    pWidget->setEnabled( nState & CTRL_STATE_ENABLED );

    // Create pixmap to paint to
    QPixmap  qPixmap( pWidget->width(), pWidget->height() );
    QPainter qPainter( &qPixmap );
    QRect    qRect( 0, 0, pWidget->width(), pWidget->height() );

    // Use the background of the widget
    qPixmap.fill( pWidget, QPoint(0, 0) );

    // Convert the flags
    QStyle::SFlags nStyle = vclStateValue2SFlags( nState, aValue );

    // Store the widget class
    const char *pClassName = pWidget->className();

    // Draw the widget to the pixmap
    if ( strcmp( "QPushButton", pClassName ) == 0 )
    {
    // Workaround for the Platinum style.
    // Platinum takes the state directly from the widget, not from SFlags.
    QPushButton *pPushButton = static_cast<QPushButton *>( pWidget->qt_cast( "QPushButton" ) );
    if ( pPushButton )
    {
        pPushButton->setDown   ( nStyle & QStyle::Style_Down );
        pPushButton->setOn     ( nStyle & QStyle::Style_On );
        pPushButton->setEnabled( nStyle & QStyle::Style_Enabled );
    }

    kapp->style().drawControl( QStyle::CE_PushButton,
        &qPainter, pWidget, qRect,
        pWidget->colorGroup(), nStyle );
    }
    else if ( strcmp( "QRadioButton", pClassName ) == 0 )
    {
    // Bitblt from the screen, because the radio buttons are usually not
    // rectangular, and there could be a bitmap under them
    GC aTmpGC = XCreateGC( dpy, qPixmap.handle(), 0, NULL );
    X11SalGraphics::CopyScreenArea( dpy,
                              drawable, nScreen, nDepth,
                              qPixmap.handle(), qPixmap.x11Screen(), qPixmap.x11Depth(),
                              aTmpGC,
                              qWidgetPos.x(), qWidgetPos.y(), qRect.width(), qRect.height(),
                              0, 0 );
    XFreeGC( dpy, aTmpGC );

    kapp->style().drawControl( QStyle::CE_RadioButton,
        &qPainter, pWidget, qRect,
        pWidget->colorGroup(), nStyle );
    }
    else if ( strcmp( "QCheckBox", pClassName ) == 0 )
    {
    kapp->style().drawControl( QStyle::CE_CheckBox,
        &qPainter, pWidget, qRect,
        pWidget->colorGroup(), nStyle );
    }
    else if ( strcmp( "QComboBox", pClassName ) == 0 )
    {
    kapp->style().drawComplexControl( QStyle::CC_ComboBox,
        &qPainter, pWidget, qRect,
        pWidget->colorGroup(), nStyle );

    // Editable combo box uses the background of the associated edit box
    QComboBox *pComboBox = static_cast<QComboBox *>( pWidget->qt_cast( "QComboBox" ) );
    if ( pComboBox && pComboBox->editable() && pComboBox->lineEdit() )
    {
        QColorGroup::ColorRole eColorRole = ( pComboBox->isEnabled() )?
        QColorGroup::Base: QColorGroup::Background;
        qPainter.fillRect(
            kapp->style().querySubControlMetrics( QStyle::CC_ComboBox,
            pComboBox, QStyle::SC_ComboBoxEditField ),
            pComboBox->lineEdit()->colorGroup().brush( eColorRole ) );
    }
    }
    else if ( strcmp( "QLineEdit", pClassName ) == 0 )
    {
    kapp->style().drawPrimitive( QStyle::PE_PanelLineEdit,
        &qPainter, qRect,
        pWidget->colorGroup(), nStyle | QStyle::Style_Sunken );
    }
    else if ( strcmp( "QSpinWidget", pClassName ) == 0 )
    {
    SpinbuttonValue *pValue = static_cast<SpinbuttonValue *> ( aValue.getOptionalVal() );

    // Is any of the buttons pressed?
    QStyle::SCFlags eActive = QStyle::SC_None;
    if ( pValue )
    {
        if ( pValue->mnUpperState & CTRL_STATE_PRESSED )
        eActive = QStyle::SC_SpinWidgetUp;
        else if ( pValue->mnLowerState & CTRL_STATE_PRESSED )
        eActive = QStyle::SC_SpinWidgetDown;

        // Update the enable/disable state of the widget
        if ( ( nState & CTRL_STATE_ENABLED ) ||
            ( pValue->mnUpperState & CTRL_STATE_ENABLED ) ||
            ( pValue->mnLowerState & CTRL_STATE_ENABLED ) )
        {
        pWidget->setEnabled( true );
        nStyle |= QStyle::Style_Enabled;
        }
        else
        pWidget->setEnabled( false );

        // Mouse-over effect
        if ( (pValue->mnUpperState & CTRL_STATE_ROLLOVER) ||
            (pValue->mnLowerState & CTRL_STATE_ROLLOVER) )
        nStyle |= QStyle::Style_MouseOver;
    }

    // Spin widget uses the background of the associated edit box
    QSpinWidget *pSpinWidget = static_cast<QSpinWidget *>( pWidget->qt_cast( "QSpinWidget" ) );
    if ( pSpinWidget && pSpinWidget->editWidget() )
    {
        QColorGroup::ColorRole eColorRole = ( pSpinWidget->isEnabled() )?
        QColorGroup::Base: QColorGroup::Background;
        qPainter.fillRect(
            kapp->style().querySubControlMetrics( QStyle::CC_SpinWidget,
            pSpinWidget, QStyle::SC_SpinWidgetEditField ),
            pSpinWidget->editWidget()->colorGroup().brush( eColorRole ) );
    }

    // Adjust the frame (needed for Motif Plus style)
    QRect qFrameRect = kapp->style().querySubControlMetrics( QStyle::CC_SpinWidget,
        pWidget, QStyle::SC_SpinWidgetFrame );

    kapp->style().drawComplexControl( QStyle::CC_SpinWidget,
        &qPainter, pWidget, qFrameRect,
        pWidget->colorGroup(), nStyle,
        QStyle::SC_All, eActive );
    }
    else if ( strcmp( "QTabBar", pClassName ) == 0 )
    {
    TabitemValue *pValue = static_cast<TabitemValue *> ( aValue.getOptionalVal() );

    QTab *pTab = NULL;
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
        return FALSE;

    pTab->setRect( qRect );

    kapp->style().drawControl( QStyle::CE_TabBarTab,
        &qPainter, pWidget, qRect,
        pWidget->colorGroup(), nStyle,
        QStyleOption( pTab ) );
    }
    else if ( strcmp( "QTabWidget", pClassName ) == 0 )
    {
    kapp->style().drawPrimitive( QStyle::PE_PanelTabWidget,
        &qPainter, qRect,
        pWidget->colorGroup(), nStyle );
    }
    else if ( strcmp( "QListView", pClassName ) == 0 )
    {
    kapp->style().drawPrimitive( QStyle::PE_Panel,
        &qPainter, qRect,
        pWidget->colorGroup(), nStyle | QStyle::Style_Sunken );
    }
    else if ( strcmp( "QScrollBar", pClassName ) == 0 )
    {
    ScrollbarValue *pValue = static_cast<ScrollbarValue *> ( aValue.getOptionalVal() );

    QStyle::SCFlags eActive = QStyle::SC_None;
    if ( pValue )
    {
        // Workaround for Style_MouseOver-aware themes.
        // Quite ugly, but I do not know about a better solution.
        const char *pStyleName = kapp->style().className();
        if ( strcmp( "QMotifPlusStyle", pStyleName ) == 0 )
        {
        nStyle |= QStyle::Style_MouseOver;
        if ( pValue->mnThumbState & CTRL_STATE_ROLLOVER )
            eActive = QStyle::SC_ScrollBarSlider;
        }
        else if ( strcmp( "QSGIStyle", pStyleName ) == 0 )
        {
        nStyle |= QStyle::Style_MouseOver;
        if ( pValue->mnButton1State & CTRL_STATE_ROLLOVER )
            eActive = QStyle::SC_ScrollBarSubLine;
        else if ( pValue->mnButton2State & CTRL_STATE_ROLLOVER )
            eActive = QStyle::SC_ScrollBarAddLine;
        else if ( pValue->mnThumbState & CTRL_STATE_ROLLOVER )
            eActive = QStyle::SC_ScrollBarSlider;
        }

        if ( pValue->mnButton1State & CTRL_STATE_PRESSED )
        eActive = QStyle::SC_ScrollBarSubLine;
        else if ( pValue->mnButton2State & CTRL_STATE_PRESSED )
        eActive = QStyle::SC_ScrollBarAddLine;
        else if ( pValue->mnThumbState & CTRL_STATE_PRESSED )
        eActive = QStyle::SC_ScrollBarSlider;
        else if ( pValue->mnPage1State & CTRL_STATE_PRESSED )
        eActive = QStyle::SC_ScrollBarSubPage;
        else if ( pValue->mnPage2State & CTRL_STATE_PRESSED )
        eActive = QStyle::SC_ScrollBarAddPage;

        // Update the enable/disable state of the widget
        if ( ( nState & CTRL_STATE_ENABLED ) ||
            ( pValue->mnButton1State & CTRL_STATE_ENABLED ) ||
            ( pValue->mnButton2State & CTRL_STATE_ENABLED ) ||
            ( pValue->mnThumbState & CTRL_STATE_ENABLED ) ||
            ( pValue->mnPage1State & CTRL_STATE_ENABLED ) ||
            ( pValue->mnPage2State & CTRL_STATE_ENABLED ) )
        {
        pWidget->setEnabled( true );
        nStyle |= QStyle::Style_Enabled;
        }
        else
        pWidget->setEnabled( false );
    }

    // Is it a horizontal scroll bar?
    QScrollBar *pScrollBar = static_cast<QScrollBar *> ( pWidget->qt_cast( "QScrollBar" ) );
    QStyle::StyleFlags eHoriz = QStyle::Style_Default;
    if ( pScrollBar && pScrollBar->orientation() == Qt::Horizontal )
        eHoriz = QStyle::Style_Horizontal;

    kapp->style().drawComplexControl( QStyle::CC_ScrollBar,
        &qPainter, pWidget, qRect,
        pWidget->colorGroup(), nStyle | eHoriz,
        QStyle::SC_All, eActive );
    }
    else if ( strcmp( "QToolBar", pClassName ) == 0 )
    {
        QToolBar *pToolBar = static_cast< QToolBar * >( pWidget->qt_cast( "QToolBar" ) );
        bool bIsHorizontal = false;
        if ( pToolBar && pToolBar->orientation() == Qt::Horizontal )
        {
            nStyle |= QStyle::Style_Horizontal;
            bIsHorizontal = true;
        }

        kapp->style().drawControl( QStyle::CE_DockWindowEmptyArea,
                &qPainter, pWidget, qRect,
                pWidget->colorGroup(), nStyle );

        kapp->style().drawPrimitive( QStyle::PE_PanelDockWindow,
                &qPainter, qRect, pWidget->colorGroup(), nStyle );

        if ( nPart == PART_THUMB_HORZ || nPart == PART_THUMB_VERT )
        {
            ToolbarValue *pValue = static_cast< ToolbarValue * >( aValue.getOptionalVal() );

            QRect qThumbRect = region2QRect( pValue->maGripRect );
            qThumbRect.moveBy( -qWidgetPos.x(), -qWidgetPos.y() );
            if ( bIsHorizontal )
                qThumbRect.addCoords( 0, 2, 0, -3 );    // make the thumb a bit nicer
            else
                qThumbRect.addCoords( 2, 0, -3, 0 );    // make the thumb a bit nicer

            if ( kapp->style().inherits( "HighColorStyle" ) ||
                 kapp->style().inherits( "HighContrastStyle" ) ||
                 kapp->style().inherits( "KeramikStyle" ) ||
                 kapp->style().inherits( "KThemeStyle" ) ||
                 kapp->style().inherits( "ThinKeramikStyle" ) )
            {
                // Workaround for the workaround in KStyle::drawPrimitive()
                KStyle *pStyle = static_cast< KStyle * >( &kapp->style() );
                pStyle->drawKStylePrimitive( KStyle::KPE_ToolBarHandle,
                        &qPainter, pToolBar, qThumbRect,
                        pWidget->colorGroup(), nStyle );
            }
            else
                kapp->style().drawPrimitive( QStyle::PE_DockWindowHandle,
                        &qPainter, qThumbRect, pWidget->colorGroup(), nStyle );
        }
    }
    else if ( strcmp( "QToolButton", pClassName ) == 0 )
    {
        if( (nStyle & QStyle::Style_MouseOver) )
            nStyle &= ~QStyle::Style_Off;
        kapp->style().drawComplexControl( QStyle::CC_ToolButton,
                &qPainter, pWidget, qRect,
                pWidget->colorGroup(), nStyle,
                QStyle::SC_ToolButton );
    }
    else if ( strcmp( "QMenuBar", pClassName ) == 0 )
    {
        if ( nPart == PART_ENTIRE_CONTROL )
        {
            kapp->style().drawControl( QStyle::CE_MenuBarEmptyArea,
                    &qPainter, pWidget, qRect,
                    pWidget->colorGroup(), nStyle );
        }
        else if ( nPart == PART_MENU_ITEM )
        {
            int nMenuItem = ( nStyle & QStyle::Style_Enabled )? m_nMenuBarEnabledItem: m_nMenuBarDisabledItem;
            QMenuItem *pMenuItem = static_cast<QMenuBar*>( pWidget )->findItem( nMenuItem );

            if ( nStyle & QStyle::Style_Selected )
                nStyle |= QStyle::Style_Active | QStyle::Style_Down | QStyle::Style_HasFocus;

            kapp->style().drawControl( QStyle::CE_MenuBarItem,
                    &qPainter, pWidget, qRect,
                    pWidget->colorGroup(), nStyle,
                    QStyleOption( pMenuItem ) );
        }
    }
    else if ( strcmp( "QPopupMenu", pClassName ) == 0 )
    {
        int nMenuItem = ( nStyle & QStyle::Style_Enabled )? m_nPopupMenuEnabledItem: m_nPopupMenuDisabledItem;
        QMenuItem *pMenuItem = static_cast<QPopupMenu*>( pWidget )->findItem( nMenuItem );

        if ( nStyle & QStyle::Style_Selected )
            nStyle |= QStyle::Style_Active;

        kapp->style().drawControl( QStyle::CE_PopupMenuItem,
                &qPainter, pWidget, qRect,
                pWidget->colorGroup(), nStyle,
                QStyleOption( pMenuItem, 0, 0 ) );
    }
    else if ( strcmp( "QProgressBar", pClassName ) == 0 )
    {
        long nProgressWidth = aValue.getNumericVal();
        QProgressBar* pProgress = static_cast<QProgressBar*>(pWidget);
        pProgress->setProgress( nProgressWidth, qRect.width() );

        kapp->style().drawControl( QStyle::CE_ProgressBarGroove,
            &qPainter, pWidget, qRect,
            pWidget->colorGroup(), nStyle );
        kapp->style().drawControl( QStyle::CE_ProgressBarContents,
            &qPainter, pWidget, qRect,
            pWidget->colorGroup(), nStyle );
    }
    else
    return FALSE;

    // Bitblt it to the screen
    X11SalGraphics::CopyScreenArea( dpy,
                              qPixmap.handle(), qPixmap.x11Screen(), qPixmap.x11Depth(),
                              drawable, nScreen, nDepth,
                              gc,
                              0, 0, qRect.width(), qRect.height(),
                              qWidgetPos.x(), qWidgetPos.y() );

    // Restore widget's position
    pWidget->move( qWidgetPos );

    return TRUE;
}

QPushButton *WidgetPainter::pushButton( const Region& rControlRegion,
    BOOL bDefault )
{
    if ( !m_pPushButton )
    m_pPushButton = new QPushButton( NULL, "push_button" );

    QRect qRect = region2QRect( rControlRegion );

    // Workaround for broken styles which do not add
    // QStyle::PM_ButtonDefaultIndicator to the size of the default button
    // (for example Keramik)
    // FIXME Fix Keramik style to be consistant with Qt built-in styles. Aargh!
    if ( bDefault )
    {
    QSize qContentsSize( 50, 50 );
    m_pPushButton->setDefault( false );
    QSize qNormalSize = kapp->style().sizeFromContents( QStyle::CT_PushButton,
        m_pPushButton, qContentsSize );
    m_pPushButton->setDefault( true );
    QSize qDefSize = kapp->style().sizeFromContents( QStyle::CT_PushButton,
        m_pPushButton, qContentsSize );

    int nIndicatorSize = kapp->style().pixelMetric(
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

QRadioButton *WidgetPainter::radioButton( const Region& rControlRegion )
{
    if ( !m_pRadioButton )
    m_pRadioButton = new QRadioButton( NULL, "radio_button" );

    QRect qRect = region2QRect( rControlRegion );

    // Workaround for broken themes which do not honor the given size.
    // Quite ugly, but I do not know about a better solution.
    const char *pStyleName = kapp->style().className();
    if ( strcmp( "KThemeStyle", pStyleName ) == 0 )
    {
    QRect qOldRect( qRect );

    qRect.setWidth( kapp->style().pixelMetric(
        QStyle::PM_ExclusiveIndicatorWidth, m_pRadioButton ) );
    qRect.setHeight( kapp->style().pixelMetric(
        QStyle::PM_ExclusiveIndicatorHeight, m_pRadioButton ) );

    qRect.moveBy( ( qOldRect.width() - qRect.width() ) / 2,
        ( qOldRect.height() - qRect.height() ) / 2 );
    }

    m_pRadioButton->move( qRect.topLeft() );
    m_pRadioButton->resize( qRect.size() );

    return m_pRadioButton;
}

QCheckBox *WidgetPainter::checkBox( const Region& rControlRegion )
{
    if ( !m_pCheckBox )
    m_pCheckBox = new QCheckBox( NULL, "check_box" );

    QRect qRect = region2QRect( rControlRegion );

    // Workaround for broken themes which do not honor the given size.
    // Quite ugly, but I do not know about a better solution.
    const char *pStyleName = kapp->style().className();
    if ( strcmp( "KThemeStyle", pStyleName ) == 0 )
    {
    QRect qOldRect( qRect );

    qRect.setWidth( kapp->style().pixelMetric(
        QStyle::PM_IndicatorWidth, m_pCheckBox ) );
    qRect.setHeight( kapp->style().pixelMetric(
        QStyle::PM_IndicatorHeight, m_pCheckBox ) );

    qRect.moveBy( ( qOldRect.width() - qRect.width() ) / 2,
        ( qOldRect.height() - qRect.height() ) / 2 );
    }

    m_pCheckBox->move( qRect.topLeft() );
    m_pCheckBox->resize( qRect.size() );

    return m_pCheckBox;
}

QComboBox *WidgetPainter::comboBox( const Region& rControlRegion,
    BOOL bEditable )
{
    QComboBox *pComboBox = NULL;
    if ( bEditable )
    {
    if ( !m_pEditableComboBox )
        m_pEditableComboBox = new QComboBox( true, NULL, "combo_box_edit" );
    pComboBox = m_pEditableComboBox;
    }
    else
    {
    if ( !m_pComboBox )
        m_pComboBox = new QComboBox( false, NULL, "combo_box" );
    pComboBox = m_pComboBox;
    }

    QRect qRect = region2QRect( rControlRegion );

    pComboBox->move( qRect.topLeft() );
    pComboBox->resize( qRect.size() );

    return pComboBox;
}

QLineEdit *WidgetPainter::lineEdit( const Region& rControlRegion )
{
    if ( !m_pLineEdit )
    m_pLineEdit = new QLineEdit( NULL, "line_edit" );

    QRect qRect = region2QRect( rControlRegion );

    m_pLineEdit->move( qRect.topLeft() );
    m_pLineEdit->resize( qRect.size() );

    return m_pLineEdit;
}

QSpinWidget *WidgetPainter::spinWidget( const Region& rControlRegion )
{
    if ( !m_pSpinWidget )
    {
    m_pSpinWidget = new QSpinWidget( NULL, "spin_widget" );

    m_pSpinEdit = new QLineEdit( NULL, "line_edit_spin" );
    m_pSpinWidget->setEditWidget( m_pSpinEdit );
    }

    QRect qRect = region2QRect( rControlRegion );

    m_pSpinWidget->move( qRect.topLeft() );
    m_pSpinWidget->resize( qRect.size() );
    m_pSpinWidget->arrange();

    return m_pSpinWidget;
}

QTabBar *WidgetPainter::tabBar( const Region& rControlRegion )
{
    if ( !m_pTabBar )
    {
    if ( !m_pTabBarParent )
        m_pTabBarParent = new QWidget( NULL, "tab_bar_parent" );

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

QTabWidget *WidgetPainter::tabWidget( const Region& rControlRegion )
{
    if ( !m_pTabWidget )
    m_pTabWidget = new QTabWidget( NULL, "tab_widget" );

    QRect qRect = region2QRect( rControlRegion );
    --qRect.rTop();

    m_pTabWidget->move( qRect.topLeft() );
    m_pTabWidget->resize( qRect.size() );

    return m_pTabWidget;
}

QListView *WidgetPainter::listView( const Region& rControlRegion )
{
    if ( !m_pListView )
    m_pListView = new QListView( NULL, "list_view" );

    QRect qRect = region2QRect( rControlRegion );

    m_pListView->move( qRect.topLeft() );
    m_pListView->resize( qRect.size() );

    return m_pListView;
}

QScrollBar *WidgetPainter::scrollBar( const Region& rControlRegion,
    BOOL bHorizontal, const ImplControlValue& aValue )
{
    if ( !m_pScrollBar )
    {
    m_pScrollBar = new QScrollBar( NULL, "scroll_bar" );
    m_pScrollBar->setTracking( false );
    m_pScrollBar->setLineStep( 1 );
    }

    QRect qRect = region2QRect( rControlRegion );

    m_pScrollBar->move( qRect.topLeft() );
    m_pScrollBar->resize( qRect.size() );
    m_pScrollBar->setOrientation( bHorizontal? Qt::Horizontal: Qt::Vertical );

    ScrollbarValue *pValue = static_cast<ScrollbarValue *> ( aValue.getOptionalVal() );
    if ( pValue )
    {
    m_pScrollBar->setMinValue( pValue->mnMin );
    m_pScrollBar->setMaxValue( pValue->mnMax - pValue->mnVisibleSize );
    m_pScrollBar->setValue( pValue->mnCur );
    m_pScrollBar->setPageStep( pValue->mnVisibleSize );
    }

    return m_pScrollBar;
}

QToolBar *WidgetPainter::toolBar( const Region& rControlRegion, BOOL bHorizontal )
{
    if ( !m_pMainWindow )
        m_pMainWindow = new QMainWindow( NULL, "main_window" );

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

QToolButton *WidgetPainter::toolButton( const Region& rControlRegion)
{
    if ( !m_pToolButton )
    m_pToolButton = new QToolButton( NULL, "tool_button" );

    QRect qRect = region2QRect( rControlRegion );

    m_pToolButton->move( qRect.topLeft() );
    m_pToolButton->resize( qRect.size() );

    return m_pToolButton;
}

QMenuBar *WidgetPainter::menuBar( const Region& rControlRegion)
{
    if ( !m_pMenuBar )
    {
        m_pMenuBar = new QMenuBar( NULL, "menu_bar" );

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

QPopupMenu *WidgetPainter::popupMenu( const Region& rControlRegion)
{
    if ( !m_pPopupMenu )
    {
        m_pPopupMenu = new QPopupMenu( NULL, "popup_menu" );

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

QProgressBar *WidgetPainter::progressBar( const Region& rControlRegion )
{
    if ( !m_pProgressBar )
    m_pProgressBar = new QProgressBar( NULL, "progress_bar" );

    QRect qRect = region2QRect( rControlRegion );

    m_pProgressBar->move( qRect.topLeft() );
    m_pProgressBar->resize( qRect.size() );

    return m_pProgressBar;
}

QStyle::SFlags WidgetPainter::vclStateValue2SFlags( ControlState nState,
    const ImplControlValue& aValue )
{
    QStyle::SFlags nStyle =
    ( (nState & CTRL_STATE_DEFAULT)?  QStyle::Style_ButtonDefault: QStyle::Style_Default ) |
    ( (nState & CTRL_STATE_ENABLED)?  QStyle::Style_Enabled:       QStyle::Style_Default ) |
    ( (nState & CTRL_STATE_FOCUSED)?  QStyle::Style_HasFocus:      QStyle::Style_Default ) |
    ( (nState & CTRL_STATE_PRESSED)?  QStyle::Style_Down:          QStyle::Style_Raised )  |
    ( (nState & CTRL_STATE_SELECTED)? QStyle::Style_Selected :     QStyle::Style_Default ) |
    ( (nState & CTRL_STATE_ROLLOVER)? QStyle::Style_MouseOver:     QStyle::Style_Default );
    //TODO ( (nState & CTRL_STATE_HIDDEN)?   QStyle::Style_: QStyle::Style_Default ) |

    switch ( aValue.getTristateVal() )
    {
    case BUTTONVALUE_ON:    nStyle |= QStyle::Style_On;       break;
    case BUTTONVALUE_OFF:   nStyle |= QStyle::Style_Off;      break;
    case BUTTONVALUE_MIXED: nStyle |= QStyle::Style_NoChange; break;
    default: break;
    }

    return nStyle;
}

QRect WidgetPainter::region2QRect( const Region& rControlRegion )
{
    Rectangle aRect = rControlRegion.GetBoundRect();

    return QRect( QPoint( aRect.Left(), aRect.Top() ),
          QPoint( aRect.Right(), aRect.Bottom() ) );
}

/** Instance of WidgetPainter.

    It is used to paint the widgets requested by NWF.
*/
static WidgetPainter *pWidgetPainter;

class KDESalGraphics : public X11SalGraphics
{
  public:
    KDESalGraphics() {}
    virtual ~KDESalGraphics() {}
    virtual BOOL IsNativeControlSupported( ControlType nType, ControlPart nPart );
    virtual BOOL hitTestNativeControl( ControlType nType, ControlPart nPart,
                                       const Region& rControlRegion, const Point& aPos,
                                       SalControlHandle& rControlHandle, BOOL& rIsInside );
    virtual BOOL drawNativeControl( ControlType nType, ControlPart nPart,
                                    const Region& rControlRegion, ControlState nState,
                                    const ImplControlValue& aValue, SalControlHandle& rControlHandle,
                                    const OUString& aCaption );
    virtual BOOL drawNativeControlText( ControlType nType, ControlPart nPart,
                                        const Region& rControlRegion, ControlState nState,
                                        const ImplControlValue& aValue, SalControlHandle& rControlHandle,
                                        const OUString& aCaption );
    virtual BOOL getNativeControlRegion( ControlType nType, ControlPart nPart,
                                         const Region& rControlRegion, ControlState nState,
                                         const ImplControlValue& aValue, SalControlHandle& rControlHandle,
                                         const OUString& aCaption,
                                         Region &rNativeBoundingRegion, Region &rNativeContentRegion );
};

/** What widgets can be drawn the native way.

    @param nType
    Type of the widget.

    @param nPart
    Specification of the widget's part if it consists of more than one.

    @return TRUE if the platform supports native drawing of the widget nType
    defined by nPart.
*/
BOOL KDESalGraphics::IsNativeControlSupported( ControlType nType, ControlPart nPart )
{
    return
    ( (nType == CTRL_PUSHBUTTON)  && (nPart == PART_ENTIRE_CONTROL) ) ||
    ( (nType == CTRL_RADIOBUTTON) && (nPart == PART_ENTIRE_CONTROL) ) ||
    ( (nType == CTRL_CHECKBOX)    && (nPart == PART_ENTIRE_CONTROL) ) ||
    ( (nType == CTRL_COMBOBOX)    && (nPart == PART_ENTIRE_CONTROL || nPart == HAS_BACKGROUND_TEXTURE) ) ||
    ( (nType == CTRL_EDITBOX)     && (nPart == PART_ENTIRE_CONTROL || nPart == HAS_BACKGROUND_TEXTURE) ) ||
    ( (nType == CTRL_LISTBOX)     && (nPart == PART_ENTIRE_CONTROL || nPart == PART_WINDOW || nPart == HAS_BACKGROUND_TEXTURE ) ) ||
    ( (nType == CTRL_SPINBOX)     && (nPart == PART_ENTIRE_CONTROL || nPart == HAS_BACKGROUND_TEXTURE) ) ||
    // no CTRL_SPINBUTTONS for KDE
    ( (nType == CTRL_TAB_ITEM)    && (nPart == PART_ENTIRE_CONTROL) ) ||
    ( (nType == CTRL_TAB_PANE)    && (nPart == PART_ENTIRE_CONTROL) ) ||
    // no CTRL_TAB_BODY for KDE
    ( (nType == CTRL_SCROLLBAR)   && (nPart == PART_ENTIRE_CONTROL || nPart == PART_DRAW_BACKGROUND_HORZ || nPart == PART_DRAW_BACKGROUND_VERT) ) ||
    ( (nType == CTRL_SCROLLBAR)   && (nPart == HAS_THREE_BUTTONS) ) || // TODO small optimization is possible here: return this only if the style really has 3 buttons
    // CTRL_GROUPBOX not supported
    // CTRL_FIXEDLINE not supported
    // CTRL_FIXEDBORDER not supported
    ( (nType == CTRL_TOOLBAR)     && (nPart == PART_ENTIRE_CONTROL ||
                                      nPart == PART_DRAW_BACKGROUND_HORZ || nPart == PART_DRAW_BACKGROUND_VERT ||
                                      nPart == PART_THUMB_HORZ || nPart == PART_THUMB_VERT ||
                                      nPart == PART_BUTTON) ) ||
    ( (nType == CTRL_MENUBAR)     && (nPart == PART_ENTIRE_CONTROL || nPart == PART_MENU_ITEM) ) ||
    ( (nType == CTRL_MENU_POPUP)  && (nPart == PART_ENTIRE_CONTROL || nPart == PART_MENU_ITEM) ) ||
    ( (nType == CTRL_PROGRESS)    && (nPart == PART_ENTIRE_CONTROL) )
        ;
}


/** Test whether the position is in the native widget.

    If the return value is TRUE, bIsInside contains information whether
    aPos was or was not inside the native widget specified by the
    nType/nPart combination.
*/
BOOL KDESalGraphics::hitTestNativeControl( ControlType nType, ControlPart nPart,
                                           const Region& rControlRegion, const Point& rPos,
                                           SalControlHandle&, BOOL& rIsInside )
{
    if ( nType == CTRL_SCROLLBAR )
    {
    // make position relative to rControlRegion
    Point aPos = rPos - rControlRegion.GetBoundRect().TopLeft();
    rIsInside = FALSE;

    BOOL bHorizontal = ( nPart == PART_BUTTON_LEFT || nPart == PART_BUTTON_RIGHT );

    QScrollBar *pScrollBar = pWidgetPainter->scrollBar( rControlRegion,
        bHorizontal, ImplControlValue() );
    QRect qRectSubLine = kapp->style().querySubControlMetrics(
        QStyle::CC_ScrollBar, pScrollBar, QStyle::SC_ScrollBarSubLine );
    QRect qRectAddLine = kapp->style().querySubControlMetrics(
        QStyle::CC_ScrollBar, pScrollBar, QStyle::SC_ScrollBarAddLine );

    // There are 2 buttons on the right/bottom side of the scrollbar
    BOOL bTwoSubButtons = FALSE;

    // It is a Platinum style scroll bar
    BOOL bPlatinumStyle = FALSE;

    // Workaround for Platinum and 3 button style scroll bars.
    // It makes the right/down button bigger.
    if ( bHorizontal )
    {
        qRectAddLine.setLeft( kapp->style().querySubControlMetrics(
            QStyle::CC_ScrollBar, pScrollBar,
            QStyle::SC_ScrollBarAddPage ).right() + 1 );
        if ( qRectAddLine.width() > qRectSubLine.width() )
        bTwoSubButtons = TRUE;
        if ( qRectSubLine.left() > kapp->style().querySubControlMetrics( QStyle::CC_ScrollBar, pScrollBar, QStyle::SC_ScrollBarSubPage ).left() )
        bPlatinumStyle = TRUE;
    }
    else
    {
        qRectAddLine.setTop( kapp->style().querySubControlMetrics(
            QStyle::CC_ScrollBar, pScrollBar,
            QStyle::SC_ScrollBarAddPage ).bottom() + 1 );
        if ( qRectAddLine.height() > qRectSubLine.height() )
        bTwoSubButtons = TRUE;
        if ( qRectSubLine.top() > kapp->style().querySubControlMetrics( QStyle::CC_ScrollBar, pScrollBar, QStyle::SC_ScrollBarSubPage ).top() )
        bPlatinumStyle = TRUE;
    }

    switch ( nPart )
    {
        case PART_BUTTON_LEFT:
        if ( !bPlatinumStyle && qRectSubLine.contains( aPos.getX(), aPos.getY() ) )
            rIsInside = TRUE;
        else if ( bTwoSubButtons )
        {
            qRectAddLine.setWidth( qRectAddLine.width() / 2 );
            rIsInside = qRectAddLine.contains( aPos.getX(), aPos.getY() );
        }
        break;

        case PART_BUTTON_UP:
        if ( !bPlatinumStyle && qRectSubLine.contains( aPos.getX(), aPos.getY() ) )
            rIsInside = TRUE;
        else if ( bTwoSubButtons )
        {
            qRectAddLine.setHeight( qRectAddLine.height() / 2 );
            rIsInside = qRectAddLine.contains( aPos.getX(), aPos.getY() );
        }
        break;

        case PART_BUTTON_RIGHT:
        if ( bTwoSubButtons )
            qRectAddLine.setLeft( qRectAddLine.left() + qRectAddLine.width() / 2 );

        rIsInside = qRectAddLine.contains( aPos.getX(), aPos.getY() );
        break;

        case PART_BUTTON_DOWN:
        if ( bTwoSubButtons )
            qRectAddLine.setTop( qRectAddLine.top() + qRectAddLine.height() / 2 );

        rIsInside = qRectAddLine.contains( aPos.getX(), aPos.getY() );
        break;

        // cases PART_TRACK_HORZ_AREA and PART_TRACK_VERT_AREA
        default:
        return FALSE;
    }

    return TRUE;
    }

    return FALSE;
}


/** Draw the requested control described by nPart/nState.

    @param rControlRegion
    The bounding region of the complete control in VCL frame coordinates.

    @param aValue
    An optional value (tristate/numerical/string).

    @param rControlHandle
    Carries platform dependent data and is maintained by the SalFrame implementation.

    @param aCaption
    A caption or title string (like button text etc.)
*/
BOOL KDESalGraphics::drawNativeControl( ControlType nType, ControlPart nPart,
                                        const Region& rControlRegion, ControlState nState,
                                        const ImplControlValue& aValue, SalControlHandle&,
                                        const OUString& )
{
    BOOL bReturn = FALSE;

    Display *dpy = GetXDisplay();
    XLIB_Window drawable = GetDrawable();
    GC gc = SelectPen(); //SelectFont(); // GC with current clipping region set

    if ( (nType == CTRL_PUSHBUTTON) && (nPart == PART_ENTIRE_CONTROL) )
    {
    bReturn = pWidgetPainter->drawStyledWidget(
        pWidgetPainter->pushButton( rControlRegion, (nState & CTRL_STATE_DEFAULT) ),
        nState, aValue,
        dpy, drawable, GetScreenNumber(), GetVisual().GetDepth(), gc );
    }
    else if ( (nType == CTRL_RADIOBUTTON) && (nPart == PART_ENTIRE_CONTROL) )
    {
    bReturn = pWidgetPainter->drawStyledWidget(
        pWidgetPainter->radioButton( rControlRegion ),
        nState, aValue,
        dpy, drawable, GetScreenNumber(), GetVisual().GetDepth(), gc );
    }
    else if ( (nType == CTRL_CHECKBOX) && (nPart == PART_ENTIRE_CONTROL) )
    {
    bReturn = pWidgetPainter->drawStyledWidget(
        pWidgetPainter->checkBox( rControlRegion ),
        nState, aValue,
        dpy, drawable, GetScreenNumber(), GetVisual().GetDepth(), gc );
    }
    else if ( (nType == CTRL_COMBOBOX) && (nPart == PART_ENTIRE_CONTROL) )
    {
    bReturn = pWidgetPainter->drawStyledWidget(
        pWidgetPainter->comboBox( rControlRegion, TRUE ),
        nState, aValue,
        dpy, drawable, GetScreenNumber(), GetVisual().GetDepth(), gc );
    }
    else if ( (nType == CTRL_EDITBOX) && (nPart == PART_ENTIRE_CONTROL) )
    {
    bReturn = pWidgetPainter->drawStyledWidget(
        pWidgetPainter->lineEdit( rControlRegion ),
        nState, aValue,
        dpy, drawable, GetScreenNumber(), GetVisual().GetDepth(), gc );
    }
    else if ( (nType == CTRL_LISTBOX) && (nPart == PART_ENTIRE_CONTROL) )
    {
    bReturn = pWidgetPainter->drawStyledWidget(
        pWidgetPainter->comboBox( rControlRegion, FALSE ),
        nState, aValue,
        dpy, drawable, GetScreenNumber(), GetVisual().GetDepth(), gc );
    }
    else if ( (nType == CTRL_LISTBOX) && (nPart == PART_WINDOW) )
    {
    bReturn = pWidgetPainter->drawStyledWidget(
        pWidgetPainter->listView( rControlRegion ),
        nState, aValue,
        dpy, drawable, GetScreenNumber(), GetVisual().GetDepth(), gc );
    }
    else if ( (nType == CTRL_SPINBOX) && (nPart == PART_ENTIRE_CONTROL) )
    {
    bReturn = pWidgetPainter->drawStyledWidget(
        pWidgetPainter->spinWidget( rControlRegion ),
        nState, aValue,
        dpy, drawable, GetScreenNumber(), GetVisual().GetDepth(), gc );
    }
    else if ( (nType==CTRL_TAB_ITEM) && (nPart == PART_ENTIRE_CONTROL) )
    {
    bReturn = pWidgetPainter->drawStyledWidget(
        pWidgetPainter->tabBar( rControlRegion ),
        nState, aValue,
        dpy, drawable, GetScreenNumber(), GetVisual().GetDepth(), gc );
    }
    else if ( (nType==CTRL_TAB_PANE) && (nPart == PART_ENTIRE_CONTROL) )
    {
    bReturn = pWidgetPainter->drawStyledWidget(
        pWidgetPainter->tabWidget( rControlRegion ),
        nState, aValue,
        dpy, drawable, GetScreenNumber(), GetVisual().GetDepth(), gc );
    }
    else if ( (nType == CTRL_SCROLLBAR) && (nPart == PART_DRAW_BACKGROUND_HORZ || nPart == PART_DRAW_BACKGROUND_VERT) )
    {
    bReturn = pWidgetPainter->drawStyledWidget(
        pWidgetPainter->scrollBar( rControlRegion, nPart == PART_DRAW_BACKGROUND_HORZ, aValue ),
        nState, aValue,
        dpy, drawable, GetScreenNumber(), GetVisual().GetDepth(), gc );
    }
    else if ( (nType == CTRL_TOOLBAR) && (nPart == PART_DRAW_BACKGROUND_HORZ || nPart == PART_DRAW_BACKGROUND_VERT || nPart == PART_THUMB_HORZ || nPart == PART_THUMB_VERT) )
    {
        bReturn = pWidgetPainter->drawStyledWidget(
                pWidgetPainter->toolBar( rControlRegion, nPart == PART_DRAW_BACKGROUND_HORZ || nPart == PART_THUMB_VERT ),
                nState, aValue,
                dpy, drawable, GetScreenNumber(), GetVisual().GetDepth(), gc, nPart );
    }
    else if ( (nType == CTRL_TOOLBAR) && (nPart == PART_BUTTON) )
    {
        bReturn = pWidgetPainter->drawStyledWidget(
                pWidgetPainter->toolButton( rControlRegion ),
                nState, aValue,
                dpy, drawable, GetScreenNumber(), GetVisual().GetDepth(), gc, nPart );
    }
    else if ( (nType == CTRL_MENUBAR) && (nPart == PART_ENTIRE_CONTROL || nPart == PART_MENU_ITEM) )
    {
        bReturn = pWidgetPainter->drawStyledWidget(
                pWidgetPainter->menuBar( rControlRegion ),
                nState, aValue,
                dpy, drawable, GetScreenNumber(), GetVisual().GetDepth(), gc, nPart );
    }
    else if ( (nType == CTRL_MENU_POPUP) && (nPart == PART_ENTIRE_CONTROL || nPart == PART_MENU_ITEM) )
    {
        bReturn = pWidgetPainter->drawStyledWidget(
                pWidgetPainter->popupMenu( rControlRegion ),
                nState, aValue,
                dpy, drawable, GetScreenNumber(), GetVisual().GetDepth(), gc );
    }
    else if ( (nType == CTRL_PROGRESS) && (nPart == PART_ENTIRE_CONTROL) )
    {
        bReturn = pWidgetPainter->drawStyledWidget(
                pWidgetPainter->progressBar( rControlRegion ),
                nState, aValue,
                dpy, drawable, GetScreenNumber(), GetVisual().GetDepth(), gc );
    }

    return bReturn;
}


/** Draw text on the widget.

    OPTIONAL. Draws the requested text for the control described by nPart/nState.
    Used if text is not drawn by DrawNativeControl().

    @param rControlRegion
    The bounding region of the complete control in VCL frame coordinates.

    @param aValue
    An optional value (tristate/numerical/string)

    @param rControlHandle
    Carries platform dependent data and is maintained by the SalFrame implementation.

    @param aCaption
    A caption or title string (like button text etc.)
*/
BOOL KDESalGraphics::drawNativeControlText( ControlType, ControlPart,
                                            const Region&, ControlState,
                                            const ImplControlValue&, SalControlHandle&,
                                            const OUString& )
{
    return FALSE;
}

/** Check if the bounding regions match.

    If the return value is TRUE, rNativeBoundingRegion
    contains the true bounding region covered by the control
    including any adornment, while rNativeContentRegion contains the area
    within the control that can be safely drawn into without drawing over
    the borders of the control.

    @param rControlRegion
    The bounding region of the control in VCL frame coordinates.

    @param aValue
    An optional value (tristate/numerical/string)

    @param rControlHandle
    Carries platform dependent data and is maintained by the SalFrame implementation.

    @param aCaption
    A caption or title string (like button text etc.)
*/
BOOL KDESalGraphics::getNativeControlRegion( ControlType nType, ControlPart nPart,
                                             const Region& rControlRegion, ControlState nState,
                                             const ImplControlValue&, SalControlHandle&,
                                             const OUString&,
                                             Region &rNativeBoundingRegion, Region &rNativeContentRegion )
{
    BOOL bReturn = FALSE;
    QRect qBoundingRect = WidgetPainter::region2QRect( rControlRegion );
    QRect qRect;

    QWidget *pWidget = NULL;
    switch ( nType )
    {
    // Metrics of the push button
    case CTRL_PUSHBUTTON:
        pWidget = pWidgetPainter->pushButton( rControlRegion, ( nState & CTRL_STATE_DEFAULT ) );

        switch ( nPart )
        {
        case PART_ENTIRE_CONTROL:
            qRect = qBoundingRect;

            if ( nState & CTRL_STATE_DEFAULT )
            {
            int nIndicatorSize = kapp->style().pixelMetric(
                QStyle::PM_ButtonDefaultIndicator, pWidget );
            qBoundingRect.addCoords( -nIndicatorSize, -nIndicatorSize,
                nIndicatorSize, nIndicatorSize );
            bReturn = TRUE;
            }
            break;
        }
        break;

        // Metrics of the radio button
        case CTRL_RADIOBUTTON:
            pWidget = pWidgetPainter->radioButton( rControlRegion );

            if ( nPart == PART_ENTIRE_CONTROL )
            {
                qRect.setWidth( kapp->style().pixelMetric( QStyle::PM_ExclusiveIndicatorWidth, pWidget ) );
                qRect.setHeight( kapp->style().pixelMetric( QStyle::PM_ExclusiveIndicatorHeight, pWidget ) );

                bReturn = TRUE;
            }
            break;

        // Metrics of the check box
        case CTRL_CHECKBOX:
            pWidget = pWidgetPainter->checkBox( rControlRegion );

            if ( nPart == PART_ENTIRE_CONTROL )
            {
                qRect.setWidth( kapp->style().pixelMetric( QStyle::PM_IndicatorWidth, pWidget ) );
                qRect.setHeight( kapp->style().pixelMetric( QStyle::PM_IndicatorHeight, pWidget ) );

                bReturn = TRUE;
            }
            break;

    // Metrics of the combo box
    case CTRL_COMBOBOX:
    case CTRL_LISTBOX:
        pWidget = pWidgetPainter->comboBox( rControlRegion, ( nType == CTRL_COMBOBOX ) );
        switch ( nPart )
        {
        case PART_BUTTON_DOWN:
            qRect = kapp->style().querySubControlMetrics(
                QStyle::CC_ComboBox, pWidget, QStyle::SC_ComboBoxArrow );
            qRect.setLeft( kapp->style().querySubControlMetrics(
                QStyle::CC_ComboBox, pWidget,
                QStyle::SC_ComboBoxEditField ).right() + 1 );
            qRect.moveBy( qBoundingRect.left(), qBoundingRect.top() );
            bReturn = TRUE;
            break;

        case PART_SUB_EDIT:
            qRect = kapp->style().querySubControlMetrics(
                QStyle::CC_ComboBox, pWidget, QStyle::SC_ComboBoxEditField );
            qRect.moveBy( qBoundingRect.left(), qBoundingRect.top() );
            bReturn = TRUE;
            break;
        }
        break;

    // Metrics of the spin box
    case CTRL_SPINBOX:
        pWidget = pWidgetPainter->spinWidget( rControlRegion );
        switch ( nPart )
        {
        case PART_BUTTON_UP:
            qRect = kapp->style().querySubControlMetrics(
                QStyle::CC_SpinWidget, pWidget, QStyle::SC_SpinWidgetUp );
            bReturn = TRUE;
            qRect.moveBy( qBoundingRect.left(), qBoundingRect.top() );
            break;

        case PART_BUTTON_DOWN:
            qRect = kapp->style().querySubControlMetrics(
                QStyle::CC_SpinWidget, pWidget, QStyle::SC_SpinWidgetDown );
            bReturn = TRUE;
            qRect.moveBy( qBoundingRect.left(), qBoundingRect.top() );
            break;

        case PART_SUB_EDIT:
            qRect = kapp->style().querySubControlMetrics(
                QStyle::CC_SpinWidget, pWidget, QStyle::SC_SpinWidgetEditField );
            qRect.moveBy( qBoundingRect.left(), qBoundingRect.top() );
            bReturn = TRUE;
            break;
        }
        break;

    // Metrics of the scroll bar
    case CTRL_SCROLLBAR:
        pWidget = pWidgetPainter->scrollBar( rControlRegion,
            ( nPart == PART_BUTTON_LEFT || nPart == PART_BUTTON_RIGHT ),
            ImplControlValue() );
        switch ( nPart )
        {
        case PART_BUTTON_LEFT:
        case PART_BUTTON_UP:
            qRect = kapp->style().querySubControlMetrics(
                QStyle::CC_ScrollBar, pWidget, QStyle::SC_ScrollBarSubLine );

            // Workaround for Platinum style scroll bars. It makes the
            // left/up button invisible.
            if ( nPart == PART_BUTTON_LEFT )
            {
            if ( qRect.left() > kapp->style().querySubControlMetrics(
                    QStyle::CC_ScrollBar, pWidget,
                    QStyle::SC_ScrollBarSubPage ).left() )
            {
                qRect.setLeft( 0 );
                qRect.setRight( 0 );
            }
            }
            else
            {
            if ( qRect.top() > kapp->style().querySubControlMetrics(
                    QStyle::CC_ScrollBar, pWidget,
                    QStyle::SC_ScrollBarSubPage ).top() )
            {
                qRect.setTop( 0 );
                qRect.setBottom( 0 );
            }
            }

            qRect.moveBy( qBoundingRect.left(), qBoundingRect.top() );

            bReturn = TRUE;
            break;

        case PART_BUTTON_RIGHT:
        case PART_BUTTON_DOWN:
            qRect = kapp->style().querySubControlMetrics(
                QStyle::CC_ScrollBar, pWidget, QStyle::SC_ScrollBarAddLine );

            // Workaround for Platinum and 3 button style scroll bars.
            // It makes the right/down button bigger.
            if ( nPart == PART_BUTTON_RIGHT )
            qRect.setLeft( kapp->style().querySubControlMetrics(
                    QStyle::CC_ScrollBar, pWidget,
                    QStyle::SC_ScrollBarAddPage ).right() + 1 );
            else
            qRect.setTop( kapp->style().querySubControlMetrics(
                    QStyle::CC_ScrollBar, pWidget,
                    QStyle::SC_ScrollBarAddPage ).bottom() + 1 );

            qRect.moveBy( qBoundingRect.left(), qBoundingRect.top() );

            bReturn = TRUE;
            break;
        }
            break;
    }

    // Fill rNativeBoundingRegion and rNativeContentRegion
    if ( bReturn )
    {
    // Bounding region
    Point aBPoint( qBoundingRect.x(), qBoundingRect.y() );
    Size aBSize( qBoundingRect.width(), qBoundingRect.height() );
    rNativeBoundingRegion = Region( Rectangle( aBPoint, aBSize ) );

    // Region of the content
    Point aPoint( qRect.x(), qRect.y() );
    Size  aSize( qRect.width(), qRect.height() );
    rNativeContentRegion = Region( Rectangle( aPoint, aSize ) );
    }

    return bReturn;
}

// -----------------------------------------------------------------------
// KDESalFrame implementation
// -----------------------------------------------------------------------

KDESalFrame::KDESalFrame( SalFrame* pParent, ULONG nStyle ) :
    X11SalFrame( pParent, nStyle )
{
}

void KDESalFrame::Show( BOOL bVisible, BOOL bNoActivate )
{
    if ( !GetParent() && ! (GetStyle() & SAL_FRAME_STYLE_INTRO) )
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
static Font toFont( const QFont &rQFont, const ::com::sun::star::lang::Locale& rLocale )
{
    psp::FastPrintFontInfo aInfo;
    QFontInfo qFontInfo( rQFont );

    // set family name
    aInfo.m_aFamilyName = String( rQFont.family().utf8(), RTL_TEXTENCODING_UTF8 );

    // set italic
    aInfo.m_eItalic = ( qFontInfo.italic()? psp::italic::Italic: psp::italic::Upright );

    // set weight
    int nWeight = qFontInfo.weight();
    if ( nWeight <= QFont::Light )
        aInfo.m_eWeight = psp::weight::Light;
    else if ( nWeight <= QFont::Normal )
        aInfo.m_eWeight = psp::weight::Normal;
    else if ( nWeight <= QFont::DemiBold )
        aInfo.m_eWeight = psp::weight::SemiBold;
    else if ( nWeight <= QFont::Bold )
        aInfo.m_eWeight = psp::weight::Bold;
    else
        aInfo.m_eWeight = psp::weight::UltraBold;

    // set width
    int nStretch = rQFont.stretch();
    if ( nStretch <= QFont::UltraCondensed )
        aInfo.m_eWidth = psp::width::UltraCondensed;
    else if ( nStretch <= QFont::ExtraCondensed )
        aInfo.m_eWidth = psp::width::ExtraCondensed;
    else if ( nStretch <= QFont::Condensed )
        aInfo.m_eWidth = psp::width::Condensed;
    else if ( nStretch <= QFont::SemiCondensed )
        aInfo.m_eWidth = psp::width::SemiCondensed;
    else if ( nStretch <= QFont::Unstretched )
        aInfo.m_eWidth = psp::width::Normal;
    else if ( nStretch <= QFont::SemiExpanded )
        aInfo.m_eWidth = psp::width::SemiExpanded;
    else if ( nStretch <= QFont::Expanded )
        aInfo.m_eWidth = psp::width::Expanded;
    else if ( nStretch <= QFont::ExtraExpanded )
        aInfo.m_eWidth = psp::width::ExtraExpanded;
    else
        aInfo.m_eWidth = psp::width::UltraExpanded;

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "font name BEFORE system match: \"%s\"\n", OUStringToOString( aInfo.m_aFamilyName, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif

    // match font to e.g. resolve "Sans"
    psp::PrintFontManager::get().matchFont( aInfo, rLocale );

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "font match %s, name AFTER: \"%s\"\n",
             aInfo.m_nID != 0 ? "succeeded" : "failed",
             OUStringToOString( aInfo.m_aFamilyName, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif

    // font height
    int nPointHeight = qFontInfo.pointSize();
    if ( nPointHeight <= 0 )
        nPointHeight = rQFont.pointSize();

    sal_Int32 nDPIX, nDPIY;
    sal_Int32 nDispDPIY = GetX11SalData()->GetDisplay()->GetResolution().B();
    GetX11SalData()->GetDisplay()->GetScreenFontResolution( nDPIX, nDPIY );

    int nHeight = nPointHeight * nDispDPIY / nDPIY;
    // allow for rounding in back conversion (at SetFont)
    while( (nHeight * nDPIY / nDispDPIY) > nPointHeight )
        nHeight--;
    while( (nHeight * nDPIY / nDispDPIY) < nPointHeight )
        nHeight++;

    // Create the font
    Font aFont( aInfo.m_aFamilyName, Size( 0, nHeight ) );
    if( aInfo.m_eWeight != psp::weight::Unknown )
        aFont.SetWeight( PspGraphics::ToFontWeight( aInfo.m_eWeight ) );
    if( aInfo.m_eWidth != psp::width::Unknown )
        aFont.SetWidthType( PspGraphics::ToFontWidth( aInfo.m_eWidth ) );
    if( aInfo.m_eItalic != psp::italic::Unknown )
        aFont.SetItalic( PspGraphics::ToFontItalic( aInfo.m_eItalic ) );
    if( aInfo.m_ePitch != psp::pitch::Unknown )
        aFont.SetPitch( PspGraphics::ToFontPitch( aInfo.m_ePitch ) );

    return aFont;
}

/** Implementation of KDE integration's main method.
*/
void KDESalFrame::UpdateSettings( AllSettings& rSettings )
{
    StyleSettings aStyleSettings( rSettings.GetStyleSettings() );
    bool bSetTitleFont = false;

    // WM settings
    KConfig *pConfig = KGlobal::config();
    if ( pConfig )
    {
        pConfig->setGroup( "WM" );
        const char *pKey;

        pKey = "activeBackground";
        if ( pConfig->hasKey( pKey ) )
            aStyleSettings.SetActiveColor( readColor( pConfig, pKey ) );

        pKey = "activeBlend";
        if ( pConfig->hasKey( pKey ) )
            aStyleSettings.SetActiveColor2( readColor( pConfig, pKey ) );

        pKey = "inactiveBackground";
        if ( pConfig->hasKey( pKey ) )
            aStyleSettings.SetDeactiveColor( readColor( pConfig, pKey ) );

        pKey = "inactiveBlend";
        if ( pConfig->hasKey( pKey ) )
            aStyleSettings.SetDeactiveColor2( readColor( pConfig, pKey ) );

        pKey = "inactiveForeground";
        if ( pConfig->hasKey( pKey ) )
            aStyleSettings.SetDeactiveTextColor( readColor( pConfig, pKey ) );

        pKey = "activeForeground";
        if ( pConfig->hasKey( pKey ) )
            aStyleSettings.SetActiveTextColor( readColor( pConfig, pKey ) );

        pKey = "titleFont";
        if ( pConfig->hasKey( pKey ) )
        {
            Font aFont = toFont( pConfig->readFontEntry( pKey ), rSettings.GetUILocale() );
            aStyleSettings.SetTitleFont( aFont );
            bSetTitleFont = true;
        }

        pConfig->setGroup( "Icons" );

        pKey = "Theme";
        if ( pConfig->hasKey( pKey ) )
            aStyleSettings.SetPreferredSymbolsStyleName( readEntryUntranslated( pConfig, pKey ) );
    }

    // General settings
    QColorGroup qColorGroup = kapp->palette().active();

    Color aFore = toColor( qColorGroup.foreground() );
    Color aBack = toColor( qColorGroup.background() );
    Color aText = toColor( qColorGroup.text() );
    Color aBase = toColor( qColorGroup.base() );

    // Foreground
    aStyleSettings.SetRadioCheckTextColor( aFore );
    aStyleSettings.SetLabelTextColor( aFore );
    aStyleSettings.SetInfoTextColor( aFore );
    aStyleSettings.SetDialogTextColor( aFore );
    aStyleSettings.SetGroupTextColor( aFore );

    // Text
    aStyleSettings.SetFieldTextColor( aText );
    aStyleSettings.SetFieldRolloverTextColor( aText );
    aStyleSettings.SetWindowTextColor( aText );
    aStyleSettings.SetHelpTextColor( aText );

    // Base
    aStyleSettings.SetFieldColor( aBase );
    aStyleSettings.SetHelpColor( aBase );
    aStyleSettings.SetWindowColor( aBase );

    // Buttons
    aStyleSettings.SetButtonTextColor( toColor( qColorGroup.buttonText() ) );
    aStyleSettings.SetButtonRolloverTextColor( toColor( qColorGroup.buttonText() ) );

    // Disable color
    aStyleSettings.SetDisableColor( toColor( qColorGroup.mid() ) );

    // Workspace
    aStyleSettings.SetWorkspaceColor( toColor( qColorGroup.mid() ) );

    // Background
    aStyleSettings.Set3DColors( aBack );
    aStyleSettings.SetFaceColor( aBack );
    aStyleSettings.SetDialogColor( aBack );
    if( aBack == COL_LIGHTGRAY )
        aStyleSettings.SetCheckedColor( Color( 0xCC, 0xCC, 0xCC ) );
    else
    {
        Color aColor2 = aStyleSettings.GetLightColor();
        aStyleSettings.
            SetCheckedColor( Color( (BYTE)(((USHORT)aBack.GetRed()+(USHORT)aColor2.GetRed())/2),
                        (BYTE)(((USHORT)aBack.GetGreen()+(USHORT)aColor2.GetGreen())/2),
                        (BYTE)(((USHORT)aBack.GetBlue()+(USHORT)aColor2.GetBlue())/2)
                        ) );
    }

    // Selection
    aStyleSettings.SetHighlightColor( toColor( qColorGroup.highlight() ) );
    aStyleSettings.SetHighlightTextColor( toColor( qColorGroup.highlightedText() ) );

    // Font
    Font aFont = toFont( kapp->font(), rSettings.GetUILocale() );

    aStyleSettings.SetAppFont( aFont );
    aStyleSettings.SetHelpFont( aFont );
    if( !bSetTitleFont )
        aStyleSettings.SetTitleFont( aFont );
    aStyleSettings.SetFloatTitleFont( aFont );
    aStyleSettings.SetMenuFont( aFont ); // will be changed according to pMenuBar
    aStyleSettings.SetToolFont( aFont ); // will be changed according to pToolBar
    aStyleSettings.SetLabelFont( aFont );
    aStyleSettings.SetInfoFont( aFont );
    aStyleSettings.SetRadioCheckFont( aFont );
    aStyleSettings.SetPushButtonFont( aFont );
    aStyleSettings.SetFieldFont( aFont );
    aStyleSettings.SetIconFont( aFont );
    aStyleSettings.SetGroupFont( aFont );
    int flash_time = QApplication::cursorFlashTime();
    aStyleSettings.SetCursorBlinkTime( flash_time != 0 ? flash_time/2 : STYLE_CURSOR_NOBLINKTIME );

    KMainWindow qMainWindow;
    qMainWindow.createGUI( "/dev/null" ); // hack

    // Menu
    aStyleSettings.SetSkipDisabledInMenus( TRUE );
    KMenuBar *pMenuBar = qMainWindow.menuBar();
    if ( pMenuBar )
    {
        // Color
        QColorGroup qMenuCG = pMenuBar->colorGroup();

        // Menu text and background color, theme specific
        Color aMenuFore = toColor( qMenuCG.foreground() );
        Color aMenuBack = toColor( qMenuCG.background() );
        if ( kapp->style().inherits( "LightStyleV2" ) ||
             kapp->style().inherits( "LightStyleV3" ) ||
             ( kapp->style().inherits( "QMotifStyle" ) && !kapp->style().inherits( "QSGIStyle" ) ) ||
             kapp->style().inherits( "QWindowsStyle" ) )
        {
            aMenuFore = toColor( qMenuCG.buttonText() );
            aMenuBack = toColor( qMenuCG.button() );
        }

        aStyleSettings.SetMenuTextColor( aMenuFore );
        aStyleSettings.SetMenuColor( aMenuBack );
        aStyleSettings.SetMenuBarColor( aMenuBack );

        aStyleSettings.SetMenuHighlightColor( toColor ( qMenuCG.highlight() ) );

        // Menu items higlight text color, theme specific
        if ( kapp->style().inherits( "HighContrastStyle" ) ||
             kapp->style().inherits( "KeramikStyle" ) ||
             kapp->style().inherits( "QWindowsStyle" ) ||
             kapp->style().inherits( "ThinKeramikStyle" ) ||
             kapp->style().inherits( "PlastikStyle" ) )
        {
            aStyleSettings.SetMenuHighlightTextColor( toColor ( qMenuCG.highlightedText() ) );
        }
        else
            aStyleSettings.SetMenuHighlightTextColor( aMenuFore );

        // set special menubar higlight text color
        if ( kapp->style().inherits( "HighContrastStyle" ) )
            ImplGetSVData()->maNWFData.maMenuBarHighlightTextColor = toColor( qMenuCG.highlightedText() );
        else
            ImplGetSVData()->maNWFData.maMenuBarHighlightTextColor = aMenuFore;

        // Font
        aFont = toFont( pMenuBar->font(), rSettings.GetUILocale() );
        aStyleSettings.SetMenuFont( aFont );
    }

    // Tool bar
    KToolBar *pToolBar = qMainWindow.toolBar();
    if ( pToolBar )
    {
        aFont = toFont( pToolBar->font(), rSettings.GetUILocale() );
        aStyleSettings.SetToolFont( aFont );
    }

    // Scroll bar size
    aStyleSettings.SetScrollBarSize( kapp->style().pixelMetric( QStyle::PM_ScrollBarExtent ) );

    // #i59364# high contrast mode
    bool bHC = ( aStyleSettings.GetFaceColor().IsDark() ||
                 aStyleSettings.GetWindowColor().IsDark() );
    aStyleSettings.SetHighContrastMode( bHC );

    rSettings.SetStyleSettings( aStyleSettings );
}

SalGraphics* KDESalFrame::GetGraphics()
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

    return NULL;
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

void KDESalFrame::updateGraphics()
{
    for( int i = 0; i < nMaxGraphics; i++ )
    {
        if( m_aGraphics[i].bInUse )
            m_aGraphics[i].pGraphics->SetDrawable( GetWindow(), GetScreenNumber() );
    }
}

KDESalFrame::~KDESalFrame()
{
}

KDESalFrame::GraphicsHolder::~GraphicsHolder()
{
    delete pGraphics;
}

// -----------------------------------------------------------------------
// KDESalInstance implementation
// -----------------------------------------------------------------------

SalFrame *
KDESalInstance::CreateFrame( SalFrame *pParent, ULONG nStyle )
{
    return new KDESalFrame( pParent, nStyle );
}

// -----------------------------------------------------------------------
// KDESalData pieces
// -----------------------------------------------------------------------

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
    pWidgetPainter = NULL;

    // We have to destroy the style early
    kapp->setStyle( NULL );
}
