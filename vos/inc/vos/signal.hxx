/*************************************************************************
 *
 *  $RCSfile: signal.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:13 $
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


#ifndef _VOS_SIGNAL_HXX_
#define _VOS_SIGNAL_HXX_

#ifndef _VOS_TYPES_HXX_
#   include <vos/types.hxx>
#endif
#ifndef _VOS_OBJECT_HXX_
#   include <vos/object.hxx>
#endif
#ifndef _OSL_SIGNAL_H_
#   include <osl/signal.h>
#endif

extern oslSignalAction SAL_CALL _OSignalHandler_Function(void* pthis, oslSignalInfo* pInfo);

#ifdef _USE_NAMESPACE
namespace vos
{
#endif

#if defined ( _USE_NAMESPACE ) && !defined ( WNT )
oslSignalAction SAL_CALL _cpp_OSignalHandler_Function(void* pthis, oslSignalInfo* pInfo);
#endif

/** OSignalHandler is an objectoriented interface for signal handlers.

    @author  Ralf Hofmann
    @version 1.0
*/

class OSignalHandler : public NAMESPACE_VOS(OObject)
{
    VOS_DECLARE_CLASSINFO(VOS_NAMESPACE(OSignalHandler, vos));

public:

    enum TSignal
    {
        TSignal_System              = osl_Signal_System,
        TSignal_Terminate           = osl_Signal_Terminate,
        TSignal_AccessViolation     = osl_Signal_AccessViolation,
        TSignal_IntegerDivideByZero = osl_Signal_IntegerDivideByZero,
        TSignal_FloatDivideByZero   = osl_Signal_FloatDivideByZero,
        TSignal_DebugBreak          = osl_Signal_DebugBreak,
        TSignal_SignalUser          = osl_Signal_User
    };

    enum TSignalAction
    {
        TAction_CallNextHandler  = osl_Signal_ActCallNextHdl,
        TAction_Ignore           = osl_Signal_ActIgnore,
        TAction_AbortApplication = osl_Signal_ActAbortApp,
        TAction_KillApplication  = osl_Signal_ActKillApp
    };

    typedef oslSignalInfo TSignalInfo;

    /// Constructor
    OSignalHandler();

    /// Destructor kills thread if neccessary
    virtual ~OSignalHandler();

    static TSignalAction SAL_CALL raise(sal_Int32 Signal, void *pData = 0);

protected:

    /// Working method which should be overridden.
    virtual TSignalAction SAL_CALL signal(TSignalInfo *pInfo) = 0;

protected:
    oslSignalHandler m_hHandler;

#if defined ( _USE_NAMESPACE ) && defined ( WNT )
    friend oslSignalAction SAL_CALL ::_OSignalHandler_Function(void* pthis, oslSignalInfo* pInfo);
#elif defined ( _USE_NAMESPACE )
    friend oslSignalAction SAL_CALL _cpp_OSignalHandler_Function(void* pthis, oslSignalInfo* pInfo);
#else
    friend oslSignalAction SAL_CALL _OSignalHandler_Function(void* pthis, oslSignalInfo* pInfo);
#endif


};

#ifdef _USE_NAMESPACE
}
#endif

#endif // _VOS_SIGNAL_HXX_

