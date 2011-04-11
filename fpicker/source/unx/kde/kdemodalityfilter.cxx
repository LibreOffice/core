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

#include <kdemodalityfilter.hxx>

#include <kapplication.h>
#include <kdialogbase.h>

#include <netwm.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

//////////////////////////////////////////////////////////////////////////
// Modality filter
//////////////////////////////////////////////////////////////////////////

KDEModalityFilter::KDEModalityFilter( WId nWinId )
    : m_nWinId( nWinId )
{
    kapp->installEventFilter( this );
}

KDEModalityFilter::~KDEModalityFilter()
{
    kapp->removeEventFilter( this );
}

bool KDEModalityFilter::eventFilter( QObject *pObject, QEvent *pEvent )
{
    if ( pObject->isWidgetType() && pEvent->type() == QEvent::Show )
    {
        KDialogBase* pDlg = ::qt_cast< KDialogBase* >( pObject );
        if ( pDlg != NULL && m_nWinId != 0 )
        {
            XSetTransientForHint( qt_xdisplay(), pDlg->winId(), m_nWinId );
            m_nWinId = 0;
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
