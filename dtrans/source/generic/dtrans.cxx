/*************************************************************************
 *
 *  $RCSfile: dtrans.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-22 14:26:15 $
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


#include <cppuhelper/factory.hxx>
#include <clipboardmanager.hxx>
#include <generic_clipboard.hxx>

using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace rtl;

extern "C"
{

//==================================================================================================

void SAL_CALL component_getImplementationEnvironment(const sal_Char ** ppEnvTypeName,
                                                     uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//==================================================================================================

sal_Bool SAL_CALL component_writeInfo(void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString::createFromAscii("/" CLIPBOARDMANAGER_IMPLEMENTATION_NAME "/UNO/SERVICES" ) ) );

            const Sequence< OUString > & rSNL = ClipboardManager_getSupportedServiceNames();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            xNewKey = reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                OUString::createFromAscii("/" GENERIC_CLIPBOARD_IMPLEMENTATION_NAME "/UNO/SERVICES" ) );

            const Sequence< OUString > & rSNL2 = GenericClipboard_getSupportedServiceNames();
            pArray = rSNL2.getConstArray();
            for ( nPos = rSNL2.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }

    return sal_False;
}

//==================================================================================================

void * SAL_CALL component_getFactory(
    const sal_Char * pImplName,
    void * pServiceManager,
    void * pRegistryKey
)
{
    void * pRet = 0;

    if (pServiceManager)
    {
        Reference< XSingleServiceFactory > xFactory;

        if (rtl_str_compare( pImplName, CLIPBOARDMANAGER_IMPLEMENTATION_NAME ) == 0)
        {
            xFactory = createOneInstanceFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                OUString::createFromAscii( pImplName ),
                ClipboardManager_createInstance,
                ClipboardManager_getSupportedServiceNames() );
        }
        else if (rtl_str_compare( pImplName, GENERIC_CLIPBOARD_IMPLEMENTATION_NAME ) == 0)
        {
            xFactory = createSingleFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                OUString::createFromAscii( pImplName ),
                GenericClipboard_createInstance,
                GenericClipboard_getSupportedServiceNames() );
        }

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

}
