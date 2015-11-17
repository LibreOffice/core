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

#ifndef INCLUDED_UNOTOOLS_DESKTOPTERMINATIONOBSERVER_HXX
#define INCLUDED_UNOTOOLS_DESKTOPTERMINATIONOBSERVER_HXX

#include <unotools/unotoolsdllapi.h>

namespace utl
{

    //= ITerminationListener

    /** non-UNO version of the com.sun.star.frame::XTerminateListener
    */
    class ITerminationListener
    {
    public:
        virtual bool    queryTermination() const = 0;
        virtual void    notifyTermination() = 0;

    protected:
        ~ITerminationListener() {}
    };

    //= DesktopTerminationObserver

    /** a class which allows non-UNO components to observe the desktop (aka application)
        for it's shutdown
    */
    namespace DesktopTerminationObserver
    {
        /** registers a listener which should be notified when the desktop terminates
            (which means the application is shutting down)
        */
        UNOTOOLS_DLLPUBLIC void    registerTerminationListener( ITerminationListener* _pListener );

        /** revokes a termination listener
        */
        UNOTOOLS_DLLPUBLIC void    revokeTerminationListener( ITerminationListener* _pListener );
    }

} // namespace utl

#endif // INCLUDED_UNOTOOLS_DESKTOPTERMINATIONOBSERVER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
