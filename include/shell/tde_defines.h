/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 * Timothy Pearson <kb9vqf@pearsoncomputing.net> (C) 2012, All Rights Reserved.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 *
 ************************************************************************/

#ifndef INCLUDED_VCL_TDE_DEFINE_HEADERS_H
#define INCLUDED_VCL_TDE_DEFINE_HEADERS_H

#define QObject         TQObject
#define QWidget         TQWidget
#define QVBox           TQVBox
#define QHBox           TQHBox
#define QGrid           TQGrid
#define QString         TQString
#define QStringList     TQStringList
#define QCustomEvent    TQCustomEvent
#define QMutex          TQMutex
#define QEvent          TQEvent
#define QFont           TQFont
#define QDateTime       TQDateTime
#define QThread         TQThread
#define QStyle          TQStyle
#define QApplication    TQApplication
#define QRect           TQRect
#define QProgressBar    TQProgressBar
#define QPopupMenu      TQPopupMenu
#define QMenuBar        TQMenuBar
#define QToolButton     TQToolButton
#define QToolBar        TQToolBar
#define QScrollBar      TQScrollBar
#define QListView       TQListView
#define QSpinWidget     TQSpinWidget
#define QScrollBar      TQScrollBar
#define QLineEdit       TQLineEdit
#define QComboBox       TQComboBox
#define QCheckBox       TQCheckBox
#define QRadioButton    TQRadioButton
#define QPushButton     TQPushButton
#define QPainter        TQPainter
#define QResizeEvent    TQResizeEvent
#define QShowEvent      TQShowEvent
#define QPoint          TQPoint
#define QDialog         TQDialog
#define QFile           TQFile
#define QLabel          TQLabel
#define QObjectList     TQObjectList
#define QCString        TQCString
#define QRegExp         TQRegExp
#define QSessionManager TQSessionManager
#define QPaintDevice    TQPaintDevice
#define QTextIStream    TQTextIStream
#define QTextStream     TQTextStream
#define QMutexLocker    TQMutexLocker
#define QValueList      TQValueList
#define QTab            TQTab
#define QTabBar         TQTabBar
#define QTabWidget      TQTabWidget
#define QMainWindow     TQMainWindow
#define QPixmap         TQPixmap
#define QColorGroup     TQColorGroup
#define QStyleOption    TQStyleOption
#define QSize           TQSize
#define QMenuItem       TQMenuItem
#define QFontInfo       TQFontInfo

#define qSwap           tqSwap
#define qt_cast         tqt_cast
#define qt_xdisplay     tqt_xdisplay
#define qVersion        tqVersion

#define KAboutData      TDEAboutData
#define KApplication    TDEApplication
#define KCmdLineArgs    TDECmdLineArgs
#define KConfig         TDEConfig
#define KGlobal         TDEGlobal
#define KGlobalSettings TDEGlobalSettings
#define KMainWindow     TDEMainWindow
#define KStartupInfo    TDEStartupInfo
#define KStyle          TDEStyle
#define KToolBar        TDEToolBar

#define drawKStylePrimitive    drawTDEStylePrimitive

#define KIO             TDEIO
#define KABC            TDEABC

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
