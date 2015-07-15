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

#include <cstddef>

#include <kdecommandthread.hxx>

#include <config_vclplug.h>

#if ENABLE_TDE
#include <tqstringlist.h>
#include <tdeapplication.h>
#else // ENABLE_TDE
#include <qstringlist.h>
#include <kapplication.h>
#endif // ENABLE_TDE

#if OSL_DEBUG_LEVEL > 1
#include <iostream>
#endif

// CommandEvent

KDECommandEvent::KDECommandEvent( const QString &qCommand, QStringList *pStringList )
    : QCustomEvent( TypeId, pStringList ),
      m_eCommand( Unknown )
{
    struct {
        const char *pName;
        CommandEventType eType;
    } *pIdx, pMapping[] =
    {
        { "appendControl",     AppendControl },
        { "enableControl",     EnableControl },
        { "getValue",          GetValue },
        { "setValue",          SetValue },
        { "appendFilter",      AppendFilter },
        { "appendFilterGroup", AppendFilterGroup },
        { "getCurrentFilter",  GetCurrentFilter },
        { "setCurrentFilter",  SetCurrentFilter },
        { "getDirectory",      GetDirectory },
        { "setDirectory",      SetDirectory },
        { "getFiles",          GetFiles },
        { "getSelectedFiles",  GetSelectedFiles },
        { "setTitle",          SetTitle },
        { "setType",           SetType },
        { "setDefaultName",    SetDefaultName },
        { "setMultiSelection", SetMultiSelection },
        { "exec",              Exec },
        { 0, Unknown }
    };

    for ( pIdx = pMapping; pIdx->pName && qCommand != pIdx->pName; ++pIdx )
        ;

    m_eCommand = pIdx->eType;
}

// CommandThread

namespace {

QStringList* tokenize( const QString &rString )
{
    // Commands look like:
    // command arg1 arg2 arg3 ...
    // Args may be enclosed in '"', if they contain spaces.

    QStringList *pList = new QStringList();

    QString qBuffer;
    qBuffer.reserve( 1024 );

    const QChar *pUnicode = rString.unicode();
    const QChar *pEnd     = pUnicode + rString.length();
    bool bQuoted          = false;

    for ( ; pUnicode != pEnd; ++pUnicode )
    {
        if ( *pUnicode == '\\' )
        {
            ++pUnicode;
            if ( pUnicode != pEnd )
            {
                if ( *pUnicode == 'n' )
                    qBuffer.append( '\n' );
                else
                    qBuffer.append( *pUnicode );
            }
        }
        else if ( *pUnicode == '"' )
            bQuoted = !bQuoted;
        else if ( *pUnicode == ' ' && !bQuoted )
        {
            pList->push_back( qBuffer );
            qBuffer.setLength( 0 );
        }
        else
            qBuffer.append( *pUnicode );
    }
    pList->push_back( qBuffer );

    return pList;
}

}

KDECommandThread::KDECommandThread( QWidget *pObject )
    : m_pObject( pObject )
{
}

KDECommandThread::~KDECommandThread()
{
}

void KDECommandThread::run()
{
    QTextIStream qStream( stdin );
    qStream.setEncoding( QTextStream::UnicodeUTF8 );

    QString qLine;
    bool bQuit = false;
    while ( !bQuit && !qStream.atEnd() )
    {
        qLine = qStream.readLine();
        handleCommand( qLine, bQuit );
    }
}

void KDECommandThread::handleCommand( const QString &rString, bool &bQuit )
{
    QMutexLocker qMutexLocker( &m_aMutex );

#if OSL_DEBUG_LEVEL > 1
    ::std::cerr << "kdefilepicker received: " << rString.latin1() << ::std::endl;
#endif

    bQuit = false;
    QStringList *pTokens = tokenize( rString );

    if ( !pTokens )
        return;
    if ( pTokens->empty() )
    {
        delete pTokens, pTokens = NULL;
        return;
    }

    QString qCommand = pTokens->front();
    pTokens->pop_front();
#if OSL_DEBUG_LEVEL > 1
    ::std::cerr << "kdefilepicker first command: " << qCommand.latin1() << ::std::endl;
#endif

    if ( qCommand == "exit" )
    {
        bQuit = true;
        QApplication::exit();
        kapp->wakeUpGuiThread();
#if OSL_DEBUG_LEVEL > 1
        ::std::cerr << "kdefilepicker: exiting" << ::std::endl;
#endif
    }
    else
        QApplication::postEvent( m_pObject, new KDECommandEvent( qCommand, pTokens ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
