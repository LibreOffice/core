/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/*
 * Copyright (C) 2012, Timothy Pearson <kb9vqf@pearsoncomputing.net>
 */

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
