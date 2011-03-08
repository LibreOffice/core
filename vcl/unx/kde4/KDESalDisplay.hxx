/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

#pragma once

#include <saldisp.hxx>

class SalKDEDisplay : public SalX11Display
{
    public:
        SalKDEDisplay( Display* pDisp );
        virtual ~SalKDEDisplay();
        static SalKDEDisplay* self();
        inline int userEventsCount() const { return m_aUserEvents.size(); }
        inline void EventGuardAcquire() { osl_acquireMutex( hEventGuard_ ); }
        inline void EventGuardRelease() { osl_releaseMutex( hEventGuard_ ); }
//        virtual long Dispatch( XEvent *event );
        virtual void Yield();
        bool checkDirectInputEvent( XEvent* ev );
    private:
        Atom xim_protocol;
        static SalKDEDisplay* selfptr;
};

inline SalKDEDisplay* SalKDEDisplay::self()
{
    return selfptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
