/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: IDocumentTimerAccess.hxx,v $
 * $Revision: 1.5 $
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

 #ifndef IDOCUMENTTIMERACCESS_HXX_INCLUDED
 #define IDOCUMENTTIMERACCESS_HXX_INCLUDED

 /** Get information about the current document state
 */
 class IDocumentTimerAccess
 {
 public:
    /**
    Set modus to start, i.e. start timer if block count == 0
    */
    virtual void StartIdling() = 0;

    /**
    Set modus to stopped, i.e. stop timer if running
    */
    virtual void StopIdling() = 0;

    /**
    Increment block count, stop timer if running
    */
    virtual void BlockIdling() = 0;

    /**
    Decrement block count, start timer if block count == 0 AND modus == start
    */
    virtual void UnblockIdling() = 0;

 protected:
    virtual ~IDocumentTimerAccess() {};
 };

 #endif // IDOCUMENTTIMERACCESS_HXX_INCLUDED
