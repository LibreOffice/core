/*************************************************************************
 *
 *  $RCSfile: officeipcthread.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cd $ $Date: 2001-07-16 12:52:33 $
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

#ifndef _DESKTOP_OFFICEIPCTHREAD_HXX_
#define _DESKTOP_OFFICEIPCTHREAD_HXX_

#ifndef _VOS_PIPE_HXX_
#include <vos/pipe.hxx>
#endif
#ifndef _VOS_SECURITY_HXX_
#include <vos/security.hxx>
#endif
#ifndef _VOS_THREAD_HXX_
#include <vos/thread.hxx>
#endif
#ifndef _VOS_SIGNAL_HXX_
#include <vos/signal.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

class SalMainPipeExchangeSignalHandler : public vos::OSignalHandler
{
    virtual TSignalAction SAL_CALL signal(TSignalInfo *pInfo);
};

class OfficeIPCThread : public vos::OThread
{
  private:
    static OfficeIPCThread*     pGlobalOfficeIPCThread;

    vos::OPipe                  maPipe;
    vos::OStreamPipe            maStreamPipe;
    static vos::OSecurity       maSecurity;
    rtl::OUString               maPipeIdent;

    OfficeIPCThread();

  protected:
    /// Working method which should be overridden
    virtual void SAL_CALL run();

  public:
    virtual ~OfficeIPCThread();

    // return FALSE if second office
    static sal_Bool EnableOfficeIPCThread();
    static void     DisableOfficeIPCThread();
};

#endif // _DESKTOP_OFFICEIPCTHREAD_HXX_
