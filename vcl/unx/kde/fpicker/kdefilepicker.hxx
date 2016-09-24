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

#ifndef INCLUDED_VCL_UNX_KDE_FPICKER_KDEFILEPICKER_HXX
#define INCLUDED_VCL_UNX_KDE_FPICKER_KDEFILEPICKER_HXX

#include <sal/config.h>

#include <config_vclplug.h>
#include <sal/types.h>

#if ENABLE_TDE
#include <shell/tde_defines.h>
#endif // ENABLE_TDE

#include <cstddef>

#if ENABLE_TDE
#include <tdefiledialog.h>
#include <tdefilefiltercombo.h>
#else // ENABLE_TDE
#include <kfiledialog.h>
#include <kfilefiltercombo.h>
#endif // ENABLE_TDE

class QGrid;
class QHBox;
class QVBox;

class KDEFileDialog : public KFileDialog
{
#if ENABLE_TDE
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
    virtual void                resizeEvent( QResizeEvent *pEvent ) override;
    virtual void                showEvent( QShowEvent *pEvent ) override;
    void                        updateCustomWidgetLayout();

    virtual void                customEvent( QCustomEvent *pEvent ) override;

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
    bool                        isSave() const { return m_bIsSave; }

    void                        setIsExecuting( bool bIsExecuting ) { m_bIsExecuting = bIsExecuting; }
    bool                        isExecuting() const { return m_bIsExecuting; }

    KURL                        mostLocalURL( const KURL &rURL ) const;
    QString                     localCopy( const QString &rFileName ) const;

    void                        setCanNotifySelection( bool bCanNotifySelection ) { m_bCanNotifySelection = bCanNotifySelection; }
    bool                        canNotifySelection() const { return m_bCanNotifySelection; }

protected slots:
// Qt3 moc does not really understand #ifdef and would process both slots,
// so the FILTER_OUT_FOO tags are used to remove some slots before moc sees them.
#if ENABLE_TDE
    void                        fileHighlightedCommand( const TQString & ); // FILTER_OUT_TDE
#else // ENABLE_TDE
    void                        fileHighlightedCommand( const QString & );  // FILTER_OUT_KDE
#endif // ENABLE_TDE
    void                        selectionChangedCommand();

protected:
    void                        appendURL( QString &rBuffer, const KURL &rURL );
};

class KDEFileFilterComboHack : public KFileFilterCombo
{
public:
    void setCurrentFilter( const QString& filter );
};

#endif // INCLUDED_VCL_UNX_KDE_FPICKER_KDEFILEPICKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
