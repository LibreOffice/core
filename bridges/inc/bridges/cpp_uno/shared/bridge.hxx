/*************************************************************************
 *
 *  $RCSfile: bridge.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-02-03 12:31:04 $
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

#ifndef INCLUDED_BRIDGES_CPP_UNO_SHARED_BRIDGE_HXX
#define INCLUDED_BRIDGES_CPP_UNO_SHARED_BRIDGE_HXX

#include "osl/interlck.h"
#include "sal/types.h"
#include "typelib/typedescription.h"
#include "uno/environment.h"
#include "uno/mapping.h"

namespace bridges { namespace cpp_uno { namespace shared {

/**
 * Holding environments and mappings.
 */
class Bridge {
public:
    // Interface for generic/component.cxx:

    static uno_Mapping * createMapping(
        uno_ExtEnvironment * pCppEnv, uno_ExtEnvironment * pUnoEnv,
        bool bExportCpp2Uno) SAL_THROW(());

    static void SAL_CALL freeMapping(uno_Mapping * pMapping) SAL_THROW(());

    // Interface for Cpp/UnoInterfaceProxy:

    void acquire() SAL_THROW(());
    void release() SAL_THROW(());

    // Interface for individual CPP--UNO bridges:

    uno_ExtEnvironment * getCppEnv() { return pCppEnv; }
    uno_ExtEnvironment * getUnoEnv() { return pUnoEnv; }

    uno_Mapping * getCpp2Uno() { return &aCpp2Uno; }
    uno_Mapping * getUno2Cpp() { return &aUno2Cpp; }

private:
    Bridge(Bridge &); // not implemented
    void operator =(Bridge); // not implemented

    Bridge(
        uno_ExtEnvironment * pCppEnv_, uno_ExtEnvironment * pUnoEnv_,
        bool bExportCpp2Uno_) SAL_THROW(());

    ~Bridge() SAL_THROW(());

    struct Mapping: public uno_Mapping {
        Bridge * pBridge;
    };

    static void SAL_CALL acquireMapping(uno_Mapping * pMapping) SAL_THROW(());
    static void SAL_CALL releaseMapping(uno_Mapping * pMapping) SAL_THROW(());
    static void SAL_CALL cpp2unoMapping(
        uno_Mapping * pMapping, void ** ppUnoI, void * pCppI,
        typelib_InterfaceTypeDescription * pTypeDescr) SAL_THROW(());
    static void SAL_CALL uno2cppMapping(
        uno_Mapping * pMapping, void ** ppCppI, void * pUnoI,
        typelib_InterfaceTypeDescription * pTypeDescr) SAL_THROW(());

    oslInterlockedCount nRef;

    uno_ExtEnvironment * pCppEnv;
    uno_ExtEnvironment * pUnoEnv;

    Mapping aCpp2Uno;
    Mapping aUno2Cpp;

    bool bExportCpp2Uno;
};

} } }

#endif
