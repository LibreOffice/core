/*************************************************************************
 *
 *  $RCSfile: unointerfaceproxy.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-02-03 12:32:23 $
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

#ifndef INCLUDED_BRIDGES_CPP_UNO_SHARED_UNOINTERFACEPROXY_HXX
#define INCLUDED_BRIDGES_CPP_UNO_SHARED_UNOINTERFACEPROXY_HXX

#include "osl/interlck.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "typelib/typedescription.h"
#include "uno/dispatcher.h"
#include "uno/environment.h"

namespace com { namespace sun { namespace star { namespace uno {
    class XInterface;
} } } }

namespace bridges { namespace cpp_uno { namespace shared {

class Bridge;

/**
 * A uno proxy wrapping a cpp interface.
 */
class UnoInterfaceProxy: public uno_Interface {
public:
    // Interface for Bridge:

    static UnoInterfaceProxy * create(
        Bridge * pBridge, com::sun::star::uno::XInterface * pCppI,
        typelib_InterfaceTypeDescription * pTypeDescr,
        rtl::OUString const & rOId) SAL_THROW(());

    static void SAL_CALL free(uno_ExtEnvironment * pEnv, void * pProxy)
        SAL_THROW(());

    // Interface for individual CPP--UNO bridges:

    Bridge * getBridge() { return pBridge; }
    com::sun::star::uno::XInterface * getCppI() { return pCppI; }

private:
    UnoInterfaceProxy(UnoInterfaceProxy &); // not implemented
    void operator =(UnoInterfaceProxy); // not implemented

    UnoInterfaceProxy(
        Bridge * pBridge_, com::sun::star::uno::XInterface * pCppI_,
        typelib_InterfaceTypeDescription * pTypeDescr_,
        rtl::OUString const & rOId_) SAL_THROW(());

    ~UnoInterfaceProxy();

    static void SAL_CALL acquireProxy(uno_Interface * pUnoI) SAL_THROW(());
    static void SAL_CALL releaseProxy(uno_Interface * pUnoI) SAL_THROW(());

    // This function is not defined in the generic part, but instead has to be
    // defined individually for each CPP--UNO bridge:
    static void SAL_CALL dispatch(
        uno_Interface * pUnoI, typelib_TypeDescription const * pMemberDescr,
        void * pReturn, void * pArgs[], uno_Any ** ppException) SAL_THROW(());

    oslInterlockedCount nRef;
    Bridge * pBridge;

    // mapping information
    com::sun::star::uno::XInterface * pCppI; // wrapped interface
    typelib_InterfaceTypeDescription * pTypeDescr;
    rtl::OUString oid;
};

} } }

#endif
