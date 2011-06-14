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
#ifndef _IFINISHEDTHREADLISTENER_HXX
#define _IFINISHEDTHREADLISTENER_HXX

#include <osl/interlck.h>

/** interface class to listen on the finish of a thread

    OD 2007-03-30 #i73788#
    Note: The thread provides its ThreadID on the finish notification

    @author OD
*/
class IFinishedThreadListener
{
    public:

        inline virtual ~IFinishedThreadListener()
        {
        };

        virtual void NotifyAboutFinishedThread( const oslInterlockedCount nThreadID ) = 0;

    protected:

        inline IFinishedThreadListener()
        {
        };
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
