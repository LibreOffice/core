/*************************************************************************
 *
 *  $RCSfile: registrationhelper.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:56 $
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

#ifndef _DBA_REGISTRATION_HELPER_HXX_
#include "registrationhelper.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _UTL_SEQUENCE_HXX_
#include <unotools/sequence.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::rtl;
using namespace ::utl;
using namespace ::cppu;

Sequence< OUString >*               OModuleRegistration::s_pImplementationNames = NULL;
Sequence< Sequence< OUString > >*   OModuleRegistration::s_pSupportedServices = NULL;
Sequence< sal_Int64 >*              OModuleRegistration::s_pCreationFunctionPointers = NULL;
Sequence< sal_Int64 >*              OModuleRegistration::s_pFactoryFunctionPointers = NULL;

//--------------------------------------------------------------------------
void OModuleRegistration::registerComponent(
    const OUString& _rImplementationName,
    const Sequence< OUString >& _rServiceNames,
    ComponentInstantiation _pCreateFunction,
    FactoryInstantiation _pFactoryFunction)
{
    if (!s_pImplementationNames)
    {
        OSL_ENSHURE(!s_pSupportedServices && !s_pCreationFunctionPointers && !s_pFactoryFunctionPointers,
            "OModuleRegistration::registerComponent : inconsistent state (the pointers (1)) !");
        s_pImplementationNames = new Sequence< OUString >;
        s_pSupportedServices = new Sequence< Sequence< OUString > >;
        s_pCreationFunctionPointers = new Sequence< sal_Int64 >;
        s_pFactoryFunctionPointers = new Sequence< sal_Int64 >;
    }
    OSL_ENSHURE(s_pImplementationNames && s_pSupportedServices && s_pCreationFunctionPointers && s_pFactoryFunctionPointers,
        "OModuleRegistration::registerComponent : inconsistent state (the pointers (2)) !");

    OSL_ENSHURE(    (s_pImplementationNames->getLength() == s_pSupportedServices->getLength())
                &&  (s_pImplementationNames->getLength() == s_pCreationFunctionPointers->getLength())
                &&  (s_pImplementationNames->getLength() == s_pFactoryFunctionPointers->getLength()),
        "OModuleRegistration::registerComponent : inconsistent state !");

    sal_Int32 nOldLen = s_pImplementationNames->getLength();
    s_pImplementationNames->realloc(nOldLen + 1);
    s_pSupportedServices->realloc(nOldLen + 1);
    s_pCreationFunctionPointers->realloc(nOldLen + 1);
    s_pFactoryFunctionPointers->realloc(nOldLen + 1);

    s_pImplementationNames->getArray()[nOldLen] = _rImplementationName;
    s_pSupportedServices->getArray()[nOldLen] = _rServiceNames;
    s_pCreationFunctionPointers->getArray()[nOldLen] = reinterpret_cast<sal_Int64>(_pCreateFunction);
    s_pFactoryFunctionPointers->getArray()[nOldLen] = reinterpret_cast<sal_Int64>(_pFactoryFunction);
}

//--------------------------------------------------------------------------
void OModuleRegistration::revokeComponent(const ::rtl::OUString& _rImplementationName)
{
    if (!s_pImplementationNames)
    {
        OSL_ASSERT("OModuleRegistration::revokeComponent : have no class infos ! Are you sure called this method at the right time ?");
        return;
    }
    OSL_ENSHURE(s_pImplementationNames && s_pSupportedServices && s_pCreationFunctionPointers && s_pFactoryFunctionPointers,
        "OModuleRegistration::revokeComponent : inconsistent state (the pointers) !");
    OSL_ENSHURE(    (s_pImplementationNames->getLength() == s_pSupportedServices->getLength())
                &&  (s_pImplementationNames->getLength() == s_pCreationFunctionPointers->getLength())
                &&  (s_pImplementationNames->getLength() == s_pFactoryFunctionPointers->getLength()),
        "OModuleRegistration::revokeComponent : inconsistent state !");

    sal_Int32 nLen = s_pImplementationNames->getLength();
    const OUString* pImplNames = s_pImplementationNames->getConstArray();
    for (sal_Int32 i=0; i<nLen; ++i, ++pImplNames)
    {
        if (pImplNames->equals(_rImplementationName))
        {
            removeElementAt(*s_pImplementationNames, i);
            removeElementAt(*s_pSupportedServices, i);
            removeElementAt(*s_pCreationFunctionPointers, i);
            removeElementAt(*s_pFactoryFunctionPointers, i);
            break;
        }
    }

    if (s_pImplementationNames->getLength() == 0)
    {
        delete s_pImplementationNames; s_pImplementationNames = NULL;
        delete s_pSupportedServices; s_pSupportedServices = NULL;
        delete s_pCreationFunctionPointers; s_pCreationFunctionPointers = NULL;
        delete s_pFactoryFunctionPointers; s_pFactoryFunctionPointers = NULL;
    }
}

//--------------------------------------------------------------------------
sal_Bool OModuleRegistration::writeComponentInfos(
        const Reference< XMultiServiceFactory >& /*_rxServiceManager*/,
        const Reference< XRegistryKey >& _rxRootKey)
{
    OSL_ENSHURE(_rxRootKey.is(), "OModuleRegistration::writeComponentInfos : invalid argument !");

    if (!s_pImplementationNames)
    {
        OSL_ASSERT("OModuleRegistration::writeComponentInfos : have no class infos ! Are you sure called this method at the right time ?");
        return sal_True;
    }
    OSL_ENSHURE(s_pImplementationNames && s_pSupportedServices && s_pCreationFunctionPointers && s_pFactoryFunctionPointers,
        "OModuleRegistration::writeComponentInfos : inconsistent state (the pointers) !");
    OSL_ENSHURE(    (s_pImplementationNames->getLength() == s_pSupportedServices->getLength())
                &&  (s_pImplementationNames->getLength() == s_pCreationFunctionPointers->getLength())
                &&  (s_pImplementationNames->getLength() == s_pFactoryFunctionPointers->getLength()),
        "OModuleRegistration::writeComponentInfos : inconsistent state !");

    sal_Int32 nLen = s_pImplementationNames->getLength();
    const OUString* pImplName = s_pImplementationNames->getConstArray();
    const Sequence< OUString >* pServices = s_pSupportedServices->getConstArray();

    OUString sRootKey("/", 1, RTL_TEXTENCODING_ASCII_US);
    for (sal_Int32 i=0; i<nLen; ++i, ++pImplName, ++pServices)
    {
        OUString aMainKeyName(sRootKey);
        aMainKeyName += *pImplName;
        aMainKeyName += OUString::createFromAscii("/UNO/SERVICES");

        try
        {
            Reference< XRegistryKey >  xNewKey( _rxRootKey->createKey(aMainKeyName) );

            const OUString* pService = pServices->getConstArray();
            for (sal_uInt32 j=0; j<pServices->getLength(); ++j, ++pService)
                xNewKey->createKey(*pService);
        }
        catch(...)
        {
            OSL_ASSERT("OModuleRegistration::writeComponentInfos : something went wrong while creating the keys !");
            return sal_False;
        }
    }

    return sal_True;
}

//--------------------------------------------------------------------------
Reference< XInterface > OModuleRegistration::getComponentFactory(
    const ::rtl::OUString& _rImplementationName,
    const Reference< XMultiServiceFactory >& _rxServiceManager)
{
    OSL_ENSHURE(_rxServiceManager.is(), "OModuleRegistration::getComponentFactory : invalid argument (service manager) !");
    OSL_ENSHURE(_rImplementationName.getLength(), "OModuleRegistration::getComponentFactory : invalid argument (implementation name) !");

    if (!s_pImplementationNames)
    {
        OSL_ASSERT("OModuleRegistration::getComponentFactory : have no class infos ! Are you sure called this method at the right time ?");
        return NULL;
    }
    OSL_ENSHURE(s_pImplementationNames && s_pSupportedServices && s_pCreationFunctionPointers && s_pFactoryFunctionPointers,
        "OModuleRegistration::getComponentFactory : inconsistent state (the pointers) !");
    OSL_ENSHURE(    (s_pImplementationNames->getLength() == s_pSupportedServices->getLength())
                &&  (s_pImplementationNames->getLength() == s_pCreationFunctionPointers->getLength())
                &&  (s_pImplementationNames->getLength() == s_pFactoryFunctionPointers->getLength()),
        "OModuleRegistration::getComponentFactory : inconsistent state !");


    Reference< XInterface > xReturn;


    sal_Int32 nLen = s_pImplementationNames->getLength();
    const OUString* pImplName = s_pImplementationNames->getConstArray();
    const Sequence< OUString >* pServices = s_pSupportedServices->getConstArray();
    const sal_Int64* pComponentFunction = s_pCreationFunctionPointers->getConstArray();
    const sal_Int64* pFactoryFunction = s_pFactoryFunctionPointers->getConstArray();

    for (sal_Int32 i=0; i<nLen; ++i, ++pImplName, ++pServices, ++pComponentFunction, ++pFactoryFunction)
    {
        if (pImplName->equals(_rImplementationName))
        {
            const FactoryInstantiation FactoryInstantiationFunction = reinterpret_cast<const FactoryInstantiation>(*pFactoryFunction);
            const ComponentInstantiation ComponentInstantiationFunction = reinterpret_cast<const ComponentInstantiation>(*pComponentFunction);

            xReturn = FactoryInstantiationFunction( _rxServiceManager, *pImplName, ComponentInstantiationFunction, *pServices);
            if (xReturn.is())
            {
                xReturn->acquire();
                return xReturn.get();
            }
        }
    }

    return NULL;
}


