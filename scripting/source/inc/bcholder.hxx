/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bcholder.hxx,v $
 * $Revision: 1.4 $
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

#ifndef SCRIPTING_BCHOLDER_HXX
#define SCRIPTING_BCHOLDER_HXX

#include <osl/mutex.hxx>
#include <cppuhelper/interfacecontainer.h>


//.........................................................................
namespace scripting_helper
{
//.........................................................................

    //  ----------------------------------------------------
    //  class OMutexHolder
    //  ----------------------------------------------------

    class OMutexHolder
    {
    protected:
        ::osl::Mutex m_aMutex;
    };

    //  ----------------------------------------------------
    //  class OBroadcastHelperHolder
    //  ----------------------------------------------------

    class OBroadcastHelperHolder
    {
    protected:
        ::cppu::OBroadcastHelper    m_aBHelper;

    public:
        OBroadcastHelperHolder( ::osl::Mutex& rMutex ) : m_aBHelper( rMutex ) { }

        ::cppu::OBroadcastHelper&       GetBroadcastHelper()        { return m_aBHelper; }
        const ::cppu::OBroadcastHelper& GetBroadcastHelper() const  { return m_aBHelper; }
    };

//.........................................................................
}   // namespace scripting_helper
//.........................................................................

#endif // SCRIPTING_BCHOLDER_HXX
