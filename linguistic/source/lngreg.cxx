/*************************************************************************
 *
 *  $RCSfile: lngreg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-17 12:37:38 $
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


#include <cppuhelper/factory.hxx>   // helper for factories
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

#include <com/sun/star/registry/XRegistryKey.hpp>

using namespace com::sun::star::lang;

using namespace com::sun::star::registry;

////////////////////////////////////////
// declaration of external RegEntry-functions defined by the service objects
//

extern sal_Bool SAL_CALL LngSvcMgr_writeInfo
(
    void * /*pServiceManager*/,
    XRegistryKey * pRegistryKey
);

extern sal_Bool SAL_CALL DicList_writeInfo
(
    void * /*pServiceManager*/, XRegistryKey * pRegistryKey
);

extern sal_Bool SAL_CALL LinguProps_writeInfo
(
    void * /*pServiceManager*/,
    XRegistryKey * pRegistryKey
);

extern void * SAL_CALL LngSvcMgr_getFactory
(
    const sal_Char * pImplName,
    XMultiServiceFactory * pServiceManager,
    void * /*pRegistryKey*/
);

extern void * SAL_CALL DicList_getFactory
(
    const sal_Char * pImplName,
    XMultiServiceFactory * pServiceManager,
    void *
);

void * SAL_CALL LinguProps_getFactory
(
    const sal_Char * pImplName,
    XMultiServiceFactory * pServiceManager,
    void *
);

////////////////////////////////////////
// definition of the two functions that are used to provide the services
//

extern "C"
{

sal_Bool SAL_CALL component_writeInfo
(
    void * pServiceManager,
    XRegistryKey * pRegistryKey
)
{
    sal_Bool bRet = LngSvcMgr_writeInfo( pServiceManager, pRegistryKey );
    if(bRet)
        bRet = LinguProps_writeInfo( pServiceManager, pRegistryKey );
    if(bRet)
        bRet = DicList_writeInfo( pServiceManager, pRegistryKey );
    return bRet;
}

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet =
        LngSvcMgr_getFactory(
            pImplName,
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            pRegistryKey );

    if(!pRet)
        pRet = LinguProps_getFactory(
            pImplName,
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            pRegistryKey );

    if(!pRet)
        pRet =  DicList_getFactory(
            pImplName,
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            pRegistryKey );

    return pRet;
}
}

///////////////////////////////////////////////////////////////////////////

