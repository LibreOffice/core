/*************************************************************************
 *
 *  $RCSfile: smplmailsuppl.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-17 15:44:29 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _SMPLMAILSUPPL_HXX_
#include "smplmailsuppl.hxx"
#endif

#ifndef _SMPLMAILCLIENT_HXX_
#include "smplmailclient.hxx"
#endif

using com::sun::star::uno::Reference;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::lang::XServiceInfo;
using com::sun::star::system::XSimpleMailClientSupplier;
using com::sun::star::system::XSimpleMailClient;
using rtl::OUString;
using osl::Mutex;

using namespace cppu;

#define COMP_IMPL_NAME  "com.sun.star.sys.shell.SimpleSystemMail"

namespace // private
{
    Sequence< OUString > SAL_CALL Component_getSupportedServiceNames()
    {
        Sequence< OUString > aRet(1);
        aRet[0] = OUString::createFromAscii("com.sun.star.sys.shell.SimpleSystemMail");
        return aRet;
    }

} // end private namespace

CSmplMailSuppl::CSmplMailSuppl() :
    WeakComponentImplHelper2<XSimpleMailClientSupplier, XServiceInfo>(m_aMutex)
{
}

CSmplMailSuppl::~CSmplMailSuppl()
{
}

Reference<XSimpleMailClient> SAL_CALL CSmplMailSuppl::querySimpleMailClient()
    throw (RuntimeException)
{
    /* We just try to load the MAPI dll as a test
       if a mail client is available */
    Reference<XSimpleMailClient> xSmplMailClient;
    HMODULE handle = LoadLibrary("mapi32.dll");
    if ((handle != INVALID_HANDLE_VALUE) && (handle != NULL))
    {
        FreeLibrary(handle);
        xSmplMailClient = Reference<XSimpleMailClient>(new CSmplMailClient());
    }
    return xSmplMailClient;
}

// XServiceInfo

OUString SAL_CALL CSmplMailSuppl::getImplementationName()
    throw(RuntimeException)
{
    return OUString::createFromAscii(COMP_IMPL_NAME);
}

sal_Bool SAL_CALL CSmplMailSuppl::supportsService(const OUString& ServiceName)
    throw(RuntimeException)
{
    Sequence <OUString> SupportedServicesNames = Component_getSupportedServiceNames();

    for (sal_Int32 n = SupportedServicesNames.getLength(); n--;)
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

Sequence<OUString> SAL_CALL CSmplMailSuppl::getSupportedServiceNames()
    throw(RuntimeException)
{
    return Component_getSupportedServiceNames();
}

