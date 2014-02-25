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

#include <kdemodalityfilter.hxx>

#if ENABLE_TDE
#include <tdeapplication.h>
#include <kdialogbase.h>
#else // ENABLE_TDE
#include <kapplication.h>
#include <kdialogbase.h>
#endif // ENABLE_TDE

#include <netwm.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>


// Modality filter


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
