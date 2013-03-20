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

#pragma once
#if 1

#include <config_vclplug.h>

#ifdef ENABLE_TDE
#include <shell/tde_defines.h>
#endif // ENABLE_TDE

#ifdef ENABLE_TDE
#include <tqevent.h>
#include <tqmutex.h>
#include <tqthread.h>
#else // ENABLE_TDE
#include <qevent.h>
#include <qmutex.h>
#include <qthread.h>
#endif // ENABLE_TDE

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
