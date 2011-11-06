/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <tools/tempfile.hxx>

#include <osl/file.hxx>

#include <cppuhelper/servicefactory.hxx>

#include <vcl/svapp.hxx>
#include <vcl/unohelp.hxx>

#include <svdata.hxx>

#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <com/sun/star/i18n/XCollator.hpp>
#include <com/sun/star/awt/XExtendedToolkit.hpp>
#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>

using namespace ::com::sun::star;
using namespace ::rtl;

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
#if defined(WNT) || defined(OS2)
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
                pSVData->maAppData.mxMSF->createInstance( OUString::createFromAscii( "com.sun.star.registry.ImplementationRegistration" )), uno::UNO_QUERY );

            if( xReg.is() )
            {
                sal_Int32 nCompCount = 0;
                while ( aVCLComponentsArray[ nCompCount ].pLibName )
                {
                    OUString aComponentPathString = CreateLibraryName( aVCLComponentsArray[ nCompCount ].pLibName,  aVCLComponentsArray[ nCompCount ].bHasSUPD );
                    if (aComponentPathString.getLength() )
                    {
                        try
                        {
                            xReg->registerImplementation(
                                OUString::createFromAscii( "com.sun.star.loader.SharedLibrary" ),aComponentPathString, NULL );
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
        uno::Reference < uno::XInterface > xI = xMSF->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.i18n.BreakIterator" ) );
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
        uno::Reference < uno::XInterface > xI = xMSF->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.i18n.CharacterClassification" ) );
        if ( xI.is() )
        {
            uno::Any x = xI->queryInterface( ::getCppuType((const uno::Reference< i18n::XCharacterClassification >*)0) );
            x >>= xB;
        }
    }
    return xB;
}

uno::Reference < i18n::XCollator > vcl::unohelper::CreateCollator()
{
    uno::Reference < i18n::XCollator > xB;
    uno::Reference< lang::XMultiServiceFactory > xMSF = GetMultiServiceFactory();
    if ( xMSF.is() )
    {
        uno::Reference < uno::XInterface > xI = xMSF->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.i18n.Collator" ) );
        if ( xI.is() )
        {
            uno::Any x = xI->queryInterface( ::getCppuType((const uno::Reference< i18n::XCollator >*)0) );
            x >>= xB;
        }
    }
    return xB;
}

::rtl::OUString vcl::unohelper::CreateLibraryName( const sal_Char* pModName, sal_Bool bSUPD )
{
    // create variable library name suffixes
    OUString aDLLSuffix; //= OUString::createFromAscii( STRING(DLLPOSTFIX) );

    OUString aLibName;

#if defined( WNT) || defined(OS2)
    aLibName = OUString::createFromAscii( pModName );
    if ( bSUPD )
    {
        aLibName += aDLLSuffix;
    }
    aLibName += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".dll" ));
#else
    aLibName = OUString( RTL_CONSTASCII_USTRINGPARAM( "lib" ));
    aLibName += OUString::createFromAscii( pModName );
    if ( bSUPD )
    {
        aLibName += aDLLSuffix;
    }
#ifdef MACOSX
    aLibName += OUString( RTL_CONSTASCII_USTRINGPARAM( ".dylib" ));
#else
    aLibName += OUString( RTL_CONSTASCII_USTRINGPARAM( ".so" ));
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
