/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2010 Novell, Inc.
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

#ifndef _KDEFILEPICKER_HXX_
#define _KDEFILEPICKER_HXX_

#ifdef ENABLE_TDE
#include <shell/tde_defines.h>
#endif // ENABLE_TDE

#include <cstddef>

#include <kfiledialog.h>
#include <kfilefiltercombo.h>

class QGrid;
class QHBox;
class QVBox;

class KDEFileDialog : public KFileDialog
{
#ifdef ENABLE_TDE
    TQ_OBJECT
#else // ENABLE_TDE
    Q_OBJECT
#endif // ENABLE_TDE

protected:
    typedef QPair< QString, QString > FilterEntry;
    typedef QValueList< FilterEntry > FilterList;

    QVBox                      *m_pCustomWidget;
    QHBox                      *m_pCombosAndButtons;

    QVBox                      *m_pLabels;
    QVBox                      *m_pComboBoxes;
    QVBox                      *m_pPushButtons;

    QGrid                      *m_pCheckBoxes;

    FilterList                  m_aFilters;

    /** Are we a "Save As" dialog?
     *
     *  We cannot use KFileDialog::setOperationMode() here, because then
     *  it automatically adds an "Automatically select filename extension"
     *  check box, and completely destroys the dialog's layout
     *  (custom list boxes are under this check box, which looks ugly).
     */
    bool                        m_bIsSave;
    bool                        m_bIsExecuting;

    bool                        m_bCanNotifySelection;

public:
    KDEFileDialog( const QString &startDir, const QString &filter,
                QWidget *parent, const char *name );
    virtual ~KDEFileDialog();

protected:
    virtual void                resizeEvent( QResizeEvent *pEvent );
    virtual void                showEvent( QShowEvent *pEvent );
    void                        updateCustomWidgetLayout();

    virtual void                customEvent( QCustomEvent *pEvent );

protected:
    void                        appendControl( const QString &rId, const QString &rType, const QString &rTitle );
    QWidget*                    findControl( const QString &rId ) const;
    void                        enableControl( const QString &rId, const QString &rValue );
    void                        getValue( const QString &rId, const QString &rAction );
    void                        setValue( const QString &rId, const QString &rAction, const QStringList &rValue );

    void                        appendFilter( const QString &rTitle, const QString &rFilter );
    QString                     filters() const;
    QString                     addExtension( const QString &rFileName ) const;

    void                        setIsSave( bool bIsSave ) { m_bIsSave = bIsSave; }
    bool                        isSave( void ) const { return m_bIsSave; }

    void                        setIsExecuting( bool bIsExecuting ) { m_bIsExecuting = bIsExecuting; }
    bool                        isExecuting( void ) const { return m_bIsExecuting; }

    bool                        isSupportedProtocol( const QString &rProtocol ) const;
    KURL                        mostLocalURL( const KURL &rURL ) const;
    QString                     localCopy( const QString &rFileName ) const;

    void                        setCanNotifySelection( bool bCanNotifySelection ) { m_bCanNotifySelection = bCanNotifySelection; }
    bool                        canNotifySelection( void ) const { return m_bCanNotifySelection; }

protected slots:
#ifdef ENABLE_TDE
    void                        fileHighlightedCommand( const TQString & );
#else // ENABLE_TDE
    void                        fileHighlightedCommand( const QString & );
#endif // ENABLE_TDE
    void                        selectionChangedCommand();

protected:
    void                        sendCommand( const QString &rCommand );
    void                        appendURL( QString &rBuffer, const KURL &rURL );
    void                        appendEscaped( QString &rBuffer, const QString &rString );
    QString                     escapeString( const QString &rString );
};

class KDEFileFilterComboHack : public KFileFilterCombo
{
public:
    void setCurrentFilter( const QString& filter );
};

#endif // _KDEFILEPICKER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
