/*************************************************************************
 *
 *  $RCSfile: broadcasthelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-24 06:54:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#define _COMPHELPER_BROADCASTHELPER_HXX_

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif

//... namespace comphelper .......................................................
namespace comphelper
{
//.........................................................................

    //==================================================================================
    //= OMutexAndBroadcastHelper - a class which holds a Mutex and a OBroadcastHelper;
    //=                 needed because when deriving from OPropertySetHelper,
    //=                 the OBroadcastHelper has to be initialized before
    //=                 the OPropertySetHelper
    //==================================================================================
    class OMutexAndBroadcastHelper
    {
    protected:
        ::osl::Mutex                m_aMutex;
        ::cppu::OBroadcastHelper    m_aBHelper;

    public:
        OMutexAndBroadcastHelper() : m_aBHelper( m_aMutex ) { }

        ::osl::Mutex&               GetMutex() { return m_aMutex; }
        ::cppu::OBroadcastHelper&   GetBroadcastHelper() { return m_aBHelper; }

    };

    // base class for all classes who are derived from OPropertySet and from OComponent
    class OBaseMutex
    {
    protected:
        ::osl::Mutex m_aMutex;
    };
}
#endif // _COMPHELPER_BROADCASTHELPER_HXX_
