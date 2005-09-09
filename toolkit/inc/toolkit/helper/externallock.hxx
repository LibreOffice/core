/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: externallock.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:54:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _TOOLKIT_HELPER_EXTERNALLOCK_HXX_
#define _TOOLKIT_HELPER_EXTERNALLOCK_HXX_

#ifndef TOOLKIT_DLLAPI_H
#include <toolkit/dllapi.h>
#endif

#ifndef COMPHELPER_ACCESSIBLE_CONTEXT_HELPER_HXX
#include <comphelper/accessiblecontexthelper.hxx>
#endif

// -----------------------------------------------------------------------------
//  class VCLExternalSolarLock
// -----------------------------------------------------------------------------

class TOOLKIT_DLLPUBLIC VCLExternalSolarLock : public ::comphelper::IMutex
{
public:
    virtual void acquire();
    virtual void release();
};

#endif  // _TOOLKIT_HELPER_EXTERNALLOCK_HXX_

