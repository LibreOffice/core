/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: factreg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:30:15 $
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
#ifndef _RTL_UNLOAD_H_
#include <rtl/unload.h>
#endif

namespace io_stm {

extern rtl_StandardModuleCount g_moduleCount;

// OPipeImpl
Reference< XInterface > SAL_CALL OPipeImpl_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw (Exception);
OUString    OPipeImpl_getImplementationName();
Sequence<OUString> OPipeImpl_getSupportedServiceNames(void);

Reference< XInterface > SAL_CALL ODataInputStream_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw (Exception);
OUString    ODataInputStream_getImplementationName();
Sequence<OUString> ODataInputStream_getSupportedServiceNames(void);

Reference< XInterface > SAL_CALL ODataOutputStream_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw (Exception);
OUString ODataOutputStream_getImplementationName();
Sequence<OUString> ODataOutputStream_getSupportedServiceNames(void);

Reference< XInterface > SAL_CALL OMarkableOutputStream_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw (Exception);
OUString OMarkableOutputStream_getImplementationName();
Sequence<OUString> OMarkableOutputStream_getSupportedServiceNames(void);

Reference< XInterface > SAL_CALL OMarkableInputStream_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw (Exception);
OUString    OMarkableInputStream_getImplementationName() ;
Sequence<OUString> OMarkableInputStream_getSupportedServiceNames(void);

Reference< XInterface > SAL_CALL OObjectOutputStream_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw(Exception);
OUString OObjectOutputStream_getImplementationName();
Sequence<OUString> OObjectOutputStream_getSupportedServiceNames(void);

Reference< XInterface > SAL_CALL OObjectInputStream_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw(Exception);
OUString    OObjectInputStream_getImplementationName() ;
Sequence<OUString> OObjectInputStream_getSupportedServiceNames(void);

Reference< XInterface > SAL_CALL OPumpImpl_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw (Exception);
OUString OPumpImpl_getImplementationName();
Sequence<OUString> OPumpImpl_getSupportedServiceNames(void);

}
