/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: documentsignaturehelper.cxx,v $
 * $Revision: 1.10 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlsecurity.hxx"

#include <xmlsecurity/documentsignaturehelper.hxx>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include "com/sun/star/beans/XPropertySet.hpp"

#include "comphelper/documentconstants.hxx"
#include <tools/debug.hxx>

using namespace ::com::sun::star;
namespace css = ::com::sun::star;

namespace
{
::rtl::OUString getElement(::rtl::OUString const & version, ::sal_Int32 * index)
{
    while (*index < version.getLength() && version[*index] == '0') {
        ++*index;
    }
    return version.getToken(0, '.', *index);
}



// Return 1 if version1 is greater then version 2, 0 if they are equal
//and -1 if version1 is less version 2
int compareVersions(
    ::rtl::OUString const & version1, ::rtl::OUString const & version2)
{
    for (::sal_Int32 i1 = 0, i2 = 0; i1 >= 0 || i2 >= 0;) {
        ::rtl::OUString e1(getElement(version1, &i1));
        ::rtl::OUString e2(getElement(version2, &i2));
        if (e1.getLength() < e2.getLength()) {
            return -1;
        } else if (e1.getLength() > e2.getLength()) {
            return 1;
        } else if (e1 < e2) {
            return -1;
        } else if (e1 > e2) {
            return 1;
        }
    }
    return 0;
}
}

void ImplFillElementList( std::vector< rtl::OUString >& rList, const uno::Reference < embed::XStorage >& rxStore, const ::rtl::OUString rRootStorageName, bool bRecursive )
{
    ::rtl::OUString aMetaInfName( RTL_CONSTASCII_USTRINGPARAM( "META-INF" ) );
    ::rtl::OUString aSep( RTL_CONSTASCII_USTRINGPARAM( "/" ) );

    uno::Reference < container::XNameAccess > xElements( rxStore, uno::UNO_QUERY );
    uno::Sequence< ::rtl::OUString > aElements = xElements->getElementNames();
    sal_Int32 nElements = aElements.getLength();
    const ::rtl::OUString* pNames = aElements.getConstArray();
    for ( sal_Int32 n = 0; n < nElements; n++ )
    {
        if ( pNames[n] != aMetaInfName )
        {
            if ( rxStore->isStreamElement( pNames[n] ) )
            {
                ::rtl::OUString aFullName( rRootStorageName + pNames[n] );
                rList.push_back( aFullName );
            }
            else if ( bRecursive && rxStore->isStorageElement( pNames[n] ) )
            {
                uno::Reference < embed::XStorage > xSubStore = rxStore->openStorageElement( pNames[n], embed::ElementModes::READ );
                rtl::OUString aFullRootName( rRootStorageName + pNames[n] + aSep );
                ImplFillElementList( rList, xSubStore, aFullRootName, bRecursive );
            }
        }
    }
}


bool DocumentSignatureHelper::isODFPre_1_2(const uno::Reference < embed::XStorage >& rxStore)
{
    ::rtl::OUString sVersion;
    uno::Reference< beans::XPropertySet > xProps(rxStore, uno::UNO_QUERY_THROW );
    xProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Version" ) ) ) >>= sVersion;
    //The property version exists only if the document is at least version 1.2
    //The constant is defined in comphelper/documentconstants.hxx
    if (compareVersions(sVersion, ODFVER_012_TEXT) == -1)
        return true;
    return false;
}


std::vector< rtl::OUString > DocumentSignatureHelper::CreateElementList( const uno::Reference < embed::XStorage >& rxStore, const ::rtl::OUString /*rRootStorageName*/, DocumentSignatureMode eMode )
{
    std::vector< rtl::OUString > aElements;
    ::rtl::OUString aSep( RTL_CONSTASCII_USTRINGPARAM( "/" ) );

    bool bPre1_2 = isODFPre_1_2(rxStore);

    switch ( eMode )
    {
        case SignatureModeDocumentContent:
        {
            if (bPre1_2)
            {
                // 1) Main content
                ImplFillElementList( aElements, rxStore, ::rtl::OUString(), false );

                // 2) Pictures...
                rtl::OUString aSubStorageName( rtl::OUString::createFromAscii( "Pictures" ) );
                try
                {
                    uno::Reference < embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, embed::ElementModes::READ );
                    ImplFillElementList( aElements, xSubStore, aSubStorageName+aSep, true );
                }
                catch( com::sun::star::io::IOException& )
                {
                    ; // Doesn't have to exist...
                }
                // 3) OLE....
                aSubStorageName = rtl::OUString::createFromAscii( "ObjectReplacements" );
                try
                {
                    uno::Reference < embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, embed::ElementModes::READ );
                    ImplFillElementList( aElements, xSubStore, aSubStorageName+aSep, true );
                    xSubStore.clear();

                    // Object folders...
                    rtl::OUString aMatchStr( rtl::OUString::createFromAscii( "Object " ) );
                    uno::Reference < container::XNameAccess > xElements( rxStore, uno::UNO_QUERY );
                    uno::Sequence< ::rtl::OUString > aElementNames = xElements->getElementNames();
                    sal_Int32 nElements = aElementNames.getLength();
                    const ::rtl::OUString* pNames = aElementNames.getConstArray();
                    for ( sal_Int32 n = 0; n < nElements; n++ )
                    {
                        if ( ( pNames[n].match( aMatchStr ) ) && rxStore->isStorageElement( pNames[n] ) )
                        {
                            uno::Reference < embed::XStorage > xTmpSubStore = rxStore->openStorageElement( pNames[n], embed::ElementModes::READ );
                            ImplFillElementList( aElements, xTmpSubStore, pNames[n]+aSep, true );
                        }
                    }
                }
                catch( com::sun::star::io::IOException& )
                {
                    ; // Doesn't have to exist...
                }
            }
            else
            {
                // Everything except META-INF
                ImplFillElementList( aElements, rxStore, ::rtl::OUString(), true );
            }
        }
        break;
        case SignatureModeMacros:
        {
            // 1) Macros
            rtl::OUString aSubStorageName( rtl::OUString::createFromAscii( "Basic" ) );
            try
            {
                uno::Reference < embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, embed::ElementModes::READ );
                ImplFillElementList( aElements, xSubStore, aSubStorageName+aSep, true );
            }
            catch( com::sun::star::io::IOException& )
            {
                ; // Doesn't have to exist...
            }

            // 2) Dialogs
            aSubStorageName = rtl::OUString::createFromAscii( "Dialogs") ;
            try
            {
                uno::Reference < embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, embed::ElementModes::READ );
                ImplFillElementList( aElements, xSubStore, aSubStorageName+aSep, true );
            }
            catch( com::sun::star::io::IOException& )
            {
                ; // Doesn't have to exist...
            }
            // 3) Scripts
            aSubStorageName = rtl::OUString::createFromAscii( "Scripts") ;
            try
            {
                uno::Reference < embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, embed::ElementModes::READ );
                ImplFillElementList( aElements, xSubStore, aSubStorageName+aSep, true );
            }
            catch( com::sun::star::io::IOException& )
            {
                ; // Doesn't have to exist...
            }
        }
        break;
        case SignatureModePackage:
        {
            // Everything except META-INF
            ImplFillElementList( aElements, rxStore, ::rtl::OUString(), true );
        }
        break;
    }

    return aElements;
}

SignatureStreamHelper DocumentSignatureHelper::OpenSignatureStream( const uno::Reference < embed::XStorage >& rxStore, sal_Int32 nOpenMode, DocumentSignatureMode eDocSigMode )
{
    sal_Int32 nSubStorageOpenMode = embed::ElementModes::READ;
    if ( nOpenMode & embed::ElementModes::WRITE )
        nSubStorageOpenMode = embed::ElementModes::WRITE;

    SignatureStreamHelper aHelper;

    try
    {
        ::rtl::OUString aSIGStoreName( RTL_CONSTASCII_USTRINGPARAM( "META-INF" ) );
        aHelper.xSignatureStorage = rxStore->openStorageElement( aSIGStoreName, nSubStorageOpenMode );
        if ( aHelper.xSignatureStorage.is() )
        {
            ::rtl::OUString aSIGStreamName;
            if ( eDocSigMode == SignatureModeDocumentContent )
                aSIGStreamName = DocumentSignatureHelper::GetDocumentContentSignatureDefaultStreamName();
            else if ( eDocSigMode == SignatureModeMacros )
                aSIGStreamName = DocumentSignatureHelper::GetScriptingContentSignatureDefaultStreamName();
            else
                aSIGStreamName = DocumentSignatureHelper::GetPackageSignatureDefaultStreamName();

            aHelper.xSignatureStream = aHelper.xSignatureStorage->openStreamElement( aSIGStreamName, nOpenMode );
        }
    }
    catch( com::sun::star::io::IOException& )
    {
        // Doesn't have to exist...
        DBG_ASSERT( nOpenMode == embed::ElementModes::READ, "Error creating signature stream..." );
    }

    return aHelper;
}

::rtl::OUString DocumentSignatureHelper::GetDocumentContentSignatureDefaultStreamName()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "documentsignatures.xml" ) );
}

::rtl::OUString DocumentSignatureHelper::GetScriptingContentSignatureDefaultStreamName()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "macrosignatures.xml" ) );
}

::rtl::OUString DocumentSignatureHelper::GetPackageSignatureDefaultStreamName()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "packagesignatures.xml" ) );
}
