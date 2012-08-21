/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <tools/tempfile.hxx>

#include <osl/file.hxx>

#include <cppuhelper/servicefactory.hxx>

#include <vcl/svapp.hxx>
#include <vcl/unohelp.hxx>

#include <svdata.hxx>

#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/componentcontext.hxx>

#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <com/sun/star/awt/XExtendedToolkit.hpp>
#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/registry/ImplementationRegistration.hpp>


using namespace ::com::sun::star;
using namespace ::rtl;

#define DOSTRING( x )                       #x
#define STRING( x )                         DOSTRING( x )

struct VCLRegServiceInfo
{
    const sal_Char*     pLibName;
    sal_Bool            bHasSUPD;
};

static VCLRegServiceInfo aVCLComponentsArray[] =
{
    {"i18n", sal_True},
    {"i18npool", sal_True},
#ifdef UNX
#ifdef MACOSX
    {"dtransaqua", sal_True},
#else
    {"dtransX11", sal_True},
#endif
#endif
#if defined(WNT)
    {"sysdtrans", sal_False},
#endif
    {"dtrans", sal_False},
    {"mcnttype", sal_False},
    {"ftransl", sal_False},
    {"dnd", sal_False},
    {NULL, sal_False}
};

uno::Reference< lang::XMultiServiceFactory > vcl::unohelper::GetMultiServiceFactory()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->maAppData.mxMSF.is() )
    {
        pSVData->maAppData.mxMSF = ::comphelper::getProcessServiceFactory();
    }
    if ( !pSVData->maAppData.mxMSF.is() )
    {
        TempFile aTempFile;
        OUString aTempFileName;
        osl::FileBase::getSystemPathFromFileURL( aTempFile.GetName(), aTempFileName );
        pSVData->maAppData.mpMSFTempFileName = new String(aTempFileName);

        try
        {
            pSVData->maAppData.mxMSF = ::cppu::createRegistryServiceFactory( aTempFileName, rtl::OUString(), sal_False );
            uno::Reference < registry::XImplementationRegistration > xReg(
                registry::ImplementationRegistration::create( comphelper::ComponentContext(pSVData->maAppData.mxMSF).getUNOContext() ) );

            if( xReg.is() )
            {
                sal_Int32 nCompCount = 0;
                while ( aVCLComponentsArray[ nCompCount ].pLibName )
                {
                    OUString aComponentPathString = CreateLibraryName( aVCLComponentsArray[ nCompCount ].pLibName,  aVCLComponentsArray[ nCompCount ].bHasSUPD );
                    if (!aComponentPathString.isEmpty() )
                    {
                        try
                        {
                            xReg->registerImplementation(
                                OUString("com.sun.star.loader.SharedLibrary"),aComponentPathString, NULL );
                        }
                        catch( ::com::sun::star::uno::Exception & )
                        {
                        }
                    }
                    nCompCount++;
                }
            }
        }
        catch( ::com::sun::star::uno::Exception & )
        {
            delete pSVData->maAppData.mpMSFTempFileName;
            pSVData->maAppData.mpMSFTempFileName = NULL;
        }
    }
    return pSVData->maAppData.mxMSF;
}


uno::Reference < i18n::XBreakIterator > vcl::unohelper::CreateBreakIterator()
{
    uno::Reference < i18n::XBreakIterator > xB;
    uno::Reference< lang::XMultiServiceFactory > xMSF = GetMultiServiceFactory();
    if ( xMSF.is() )
    {
        uno::Reference < uno::XInterface > xI = xMSF->createInstance( ::rtl::OUString("com.sun.star.i18n.BreakIterator") );
        if ( xI.is() )
        {
            uno::Any x = xI->queryInterface( ::getCppuType((const uno::Reference< i18n::XBreakIterator >*)0) );
            x >>= xB;
        }
    }
    return xB;
}

uno::Reference < i18n::XCharacterClassification > vcl::unohelper::CreateCharacterClassification()
{
    uno::Reference < i18n::XCharacterClassification > xB;
    uno::Reference< lang::XMultiServiceFactory > xMSF = GetMultiServiceFactory();
    if ( xMSF.is() )
    {
        uno::Reference < uno::XInterface > xI = xMSF->createInstance( ::rtl::OUString("com.sun.star.i18n.CharacterClassification") );
        if ( xI.is() )
        {
            uno::Any x = xI->queryInterface( ::getCppuType((const uno::Reference< i18n::XCharacterClassification >*)0) );
            x >>= xB;
        }
    }
    return xB;
}

::rtl::OUString vcl::unohelper::CreateLibraryName( const sal_Char* pModName, sal_Bool bSUPD )
{
    // create variable library name suffixes
    OUString aDLLSuffix = OUString::createFromAscii( STRING(DLLPOSTFIX) );

    OUString aLibName;

#if defined( WNT)
    aLibName = OUString::createFromAscii( pModName );
    if ( bSUPD )
    {
        aLibName += aDLLSuffix;
    }
    aLibName += rtl::OUString( ".dll" );
#else
    aLibName = OUString( "lib" );
    aLibName += OUString::createFromAscii( pModName );
    if ( bSUPD )
    {
        aLibName += aDLLSuffix;
    }
#ifdef MACOSX
    aLibName += OUString( ".dylib" );
#else
    aLibName += OUString( ".so" );
#endif
#endif

    return aLibName;
}

void vcl::unohelper::NotifyAccessibleStateEventGlobally( const ::com::sun::star::accessibility::AccessibleEventObject& rEventObject )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XExtendedToolkit > xExtToolkit( Application::GetVCLToolkit(), uno::UNO_QUERY );
    if ( xExtToolkit.is() )
    {
        // Only for focus events
        sal_Int16 nType = ::com::sun::star::accessibility::AccessibleStateType::INVALID;
        rEventObject.NewValue >>= nType;
        if ( nType == ::com::sun::star::accessibility::AccessibleStateType::FOCUSED )
            xExtToolkit->fireFocusGained( rEventObject.Source );
        else
        {
            rEventObject.OldValue >>= nType;
            if ( nType == ::com::sun::star::accessibility::AccessibleStateType::FOCUSED )
                xExtToolkit->fireFocusLost( rEventObject.Source );
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
