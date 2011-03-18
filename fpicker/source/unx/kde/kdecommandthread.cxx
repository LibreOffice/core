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

#include <cstddef>

#include <kdecommandthread.hxx>

#include <qstringlist.h>

#include <kapplication.h>

#if OSL_DEBUG_LEVEL > 1
#include <iostream>
#endif

//////////////////////////////////////////////////////////////////////////
// CommandEvent
//////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////
// CommandThread
//////////////////////////////////////////////////////////////////////////

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

    if ( qCommand == "exit" )
    {
        bQuit = true;
        kapp->exit();
        kapp->wakeUpGuiThread();
    }
    else
        kapp->postEvent( m_pObject, new KDECommandEvent( qCommand, pTokens ) );
}

QStringList* KDECommandThread::tokenize( const QString &rString )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
