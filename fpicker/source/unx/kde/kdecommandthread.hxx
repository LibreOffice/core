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

#ifndef _KDECOMMANDTHREAD_HXX_
#define _KDECOMMANDTHREAD_HXX_

#include <qevent.h>
#include <qmutex.h>
#include <qthread.h>

class KDECommandEvent : public QCustomEvent
{
public:
    enum CommandEventType {
        Unknown = 0,

        AppendControl,
        EnableControl,
        GetValue,
        SetValue,

        AppendFilter,
        AppendFilterGroup,
        UpdateFilters,
        GetCurrentFilter,
        SetCurrentFilter,

        GetDirectory,
        SetDirectory,

        GetFiles,

        SetTitle,
        SetType,
        SetDefaultName,
        SetMultiSelection,

        Exec
    };
    static const QEvent::Type   TypeId = (QEvent::Type) ( (int) QEvent::User + 42 /*random magic value*/ );

protected:
    CommandEventType            m_eCommand;

public:
    KDECommandEvent( const QString &qCommand, QStringList *pStringList );

    CommandEventType            command() const { return m_eCommand; }
    QStringList*                stringList() { return static_cast< QStringList* >( data() ); }
};

class KDECommandThread : public QThread
{
protected:
    QObject                    *m_pObject;

    QMutex                      m_aMutex;

public:
    KDECommandThread( QWidget *pObject );
    virtual ~KDECommandThread();

    virtual void                run();

protected:
    void                        handleCommand( const QString &rString, bool &bQuit );
    QStringList*                tokenize( const QString &rString );
};

#endif // _KDECOMMANDTHREAD_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
