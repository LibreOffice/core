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
#include "unotools/unotoolsdllapi.h"

#ifndef UNOTOOLS_INC_UNOTOOLS_DESKTOPTERMINATIONOBSERVER_HXX
#define UNOTOOLS_INC_UNOTOOLS_DESKTOPTERMINATIONOBSERVER_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

//........................................................................
namespace utl
{
//........................................................................

    //====================================================================
    //= ITerminationListener
    //====================================================================
    /** non-UNO version of the <type scope="com.sun.star.frame">XTerminateListener</type>
    */
    class ITerminationListener
    {
    public:
        virtual bool    queryTermination() const = 0;
        virtual void    notifyTermination() = 0;
    };

    //====================================================================
    //= DesktopTerminationObserver
    //====================================================================
    /** a class which allows non-UNO components to observe the desktop (aka application)
        for it's shutdown
    */
    class UNOTOOLS_DLLPUBLIC DesktopTerminationObserver
    {
    public:
        /** registers a listener which should be notified when the desktop terminates
            (which means the application is shutting down)
        */
        static void    registerTerminationListener( ITerminationListener* _pListener );

        /** revokes a termination listener
        */
        static void    revokeTerminationListener( ITerminationListener* _pListener );

    private:
        DesktopTerminationObserver();   // never implemented, only static methods
    };

//........................................................................
} // namespace utl
//........................................................................

#endif // UNOTOOLS_INC_UNOTOOLS_DESKTOPTERMINATIONOBSERVER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
