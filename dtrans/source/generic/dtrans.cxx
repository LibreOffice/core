/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dtrans.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:00:14 $
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
                                                     uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//==================================================================================================

sal_Bool SAL_CALL component_writeInfo(void * /*pServiceManager*/, void * pRegistryKey )
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
            sal_Int32 nPos;
            for ( nPos = rSNL.getLength(); nPos--; )
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
    void * /*pRegistryKey*/
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
