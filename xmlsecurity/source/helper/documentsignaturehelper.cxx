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
#include "rtl/uri.hxx"

using namespace ::com::sun::star::uno;
namespace css = ::com::sun::star;
using rtl::OUString;


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
//If the OOo 3.0 mode is used then we exclude
//'mimetype' and all content of 'META-INF'.
//If the argument 'bSigning' is true then the element list is created for a signing
//operation in which case we use the latest signing algorithm. That is all elements
//we find in the zip storage are added to the list. We do not support the old signatures
//which did not contain all files.
//If 'bSigning' is false, then we validate. If the user enabled validating according to OOo 3.0
//then mimetype and all content of META-INF must be excluded.
void ImplFillElementList(
    std::vector< rtl::OUString >& rList, const Reference < css::embed::XStorage >& rxStore,
    const ::rtl::OUString rRootStorageName, const bool bRecursive,
    const DocumentSignatureAlgorithm mode)
{
    ::rtl::OUString aMetaInfName( RTL_CONSTASCII_USTRINGPARAM( "META-INF" ) );
    ::rtl::OUString sMimeTypeName (RTL_CONSTASCII_USTRINGPARAM("mimetype"));
    ::rtl::OUString aSep( RTL_CONSTASCII_USTRINGPARAM( "/" ) );

    Reference < css::container::XNameAccess > xElements( rxStore, UNO_QUERY );
    Sequence< ::rtl::OUString > aElements = xElements->getElementNames();
    sal_Int32 nElements = aElements.getLength();
    const ::rtl::OUString* pNames = aElements.getConstArray();

    for ( sal_Int32 n = 0; n < nElements; n++ )
    {
        if (mode != OOo3_2Document
            && (pNames[n] == aMetaInfName
            || pNames[n] == sMimeTypeName))
        {
            continue;
        }
        else
        {
            ::rtl::OUString sEncName = ::rtl::Uri::encode(
                pNames[n], rtl_UriCharClassRelSegment,
                rtl_UriEncodeStrict, RTL_TEXTENCODING_UTF8);
            if (sEncName.getLength() == 0 && pNames[n].getLength() != 0)
                throw css::uno::Exception(::rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("Failed to encode element name of XStorage")), 0);

            if ( rxStore->isStreamElement( pNames[n] ) )
            {
                //Exclude documentsignatures.xml!
                if (pNames[n].equals(
                    DocumentSignatureHelper::GetDocumentContentSignatureDefaultStreamName()))
                    continue;
                ::rtl::OUString aFullName( rRootStorageName + sEncName );
                rList.push_back(aFullName);
            }
            else if ( bRecursive && rxStore->isStorageElement( pNames[n] ) )
            {
                Reference < css::embed::XStorage > xSubStore = rxStore->openStorageElement( pNames[n], css::embed::ElementModes::READ );
                rtl::OUString aFullRootName( rRootStorageName + sEncName + aSep );
                ImplFillElementList(rList, xSubStore, aFullRootName, bRecursive, mode);
            }
        }
    }
}


bool DocumentSignatureHelper::isODFPre_1_2(const ::rtl::OUString & sVersion)
{
    //The property version exists only if the document is at least version 1.2
    //That is, if the document has version 1.1 and sVersion is empty.
    //The constant is defined in comphelper/documentconstants.hxx
    if (compareVersions(sVersion, ODFVER_012_TEXT) == -1)
        return true;
    return false;
}

bool DocumentSignatureHelper::isOOo3_2_Signature(const SignatureInformation & sigInfo)
{
    ::rtl::OUString sManifestURI(RTL_CONSTASCII_USTRINGPARAM("META-INF/manifest.xml"));
    bool bOOo3_2 = false;
    typedef ::std::vector< SignatureReferenceInformation >::const_iterator CIT;
    for (CIT i = sigInfo.vSignatureReferenceInfors.begin();
        i < sigInfo.vSignatureReferenceInfors.end(); i++)
    {
        if (i->ouURI.equals(sManifestURI))
        {
            bOOo3_2 = true;
            break;
        }
    }
    return  bOOo3_2;
}

DocumentSignatureAlgorithm
DocumentSignatureHelper::getDocumentAlgorithm(
    const ::rtl::OUString & sODFVersion, const SignatureInformation & sigInfo)
{
    OSL_ASSERT(sODFVersion.getLength());
    DocumentSignatureAlgorithm mode = OOo3_2Document;
    if (!isOOo3_2_Signature(sigInfo))
    {
        if (isODFPre_1_2(sODFVersion))
            mode = OOo2Document;
        else
            mode = OOo3_0Document;
    }
    return mode;
}

//The function creates a list of files which are to be signed or for which
//the signature is to be validated. The strings are UTF8 encoded URIs which
//contain '/' as path separators.
//
//The algorithm how document signatures are created and validated has
//changed over time. The change affects only which files within the document
//are changed. Document signatures created by OOo 2.x only used particular files. Since
//OOo 3.0 everything except "mimetype" and "META-INF" are signed. As of OOo 3.2 everything
//except META-INF/documentsignatures.xml is signed.
//Signatures are validated according to the algorithm which was then used for validation.
//That is, when validating a signature which was created by OOo 3.0, then mimetype and
//META-INF are not used.
//
//When a signature is created then we always use the latest algorithm. That is, we use
//that of OOo 3.2
std::vector< rtl::OUString >
DocumentSignatureHelper::CreateElementList(
    const Reference < css::embed::XStorage >& rxStore,
    const ::rtl::OUString /*rRootStorageName*/, DocumentSignatureMode eMode,
    const DocumentSignatureAlgorithm mode)
{
    std::vector< rtl::OUString > aElements;
    ::rtl::OUString aSep( RTL_CONSTASCII_USTRINGPARAM( "/" ) );

    switch ( eMode )
    {
        case SignatureModeDocumentContent:
        {
            if (mode == OOo2Document) //that is, ODF 1.0, 1.1
            {
                // 1) Main content
                ImplFillElementList(aElements, rxStore, ::rtl::OUString(), false, mode);

                // 2) Pictures...
                rtl::OUString aSubStorageName( rtl::OUString::createFromAscii( "Pictures" ) );
                try
                {
                    Reference < css::embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, css::embed::ElementModes::READ );
                    ImplFillElementList(aElements, xSubStore, aSubStorageName+aSep, true, mode);
                }
                catch(css::io::IOException& )
                {
                    ; // Doesn't have to exist...
                }
                // 3) OLE....
                aSubStorageName = rtl::OUString::createFromAscii( "ObjectReplacements" );
                try
                {
                    Reference < css::embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, css::embed::ElementModes::READ );
                    ImplFillElementList(aElements, xSubStore, aSubStorageName+aSep, true, mode);
                    xSubStore.clear();

                    // Object folders...
                    rtl::OUString aMatchStr( rtl::OUString::createFromAscii( "Object " ) );
                    Reference < css::container::XNameAccess > xElements( rxStore, UNO_QUERY );
                    Sequence< ::rtl::OUString > aElementNames = xElements->getElementNames();
                    sal_Int32 nElements = aElementNames.getLength();
                    const ::rtl::OUString* pNames = aElementNames.getConstArray();
                    for ( sal_Int32 n = 0; n < nElements; n++ )
                    {
                        if ( ( pNames[n].match( aMatchStr ) ) && rxStore->isStorageElement( pNames[n] ) )
                        {
                            Reference < css::embed::XStorage > xTmpSubStore = rxStore->openStorageElement( pNames[n], css::embed::ElementModes::READ );
                            ImplFillElementList(aElements, xTmpSubStore, pNames[n]+aSep, true, mode);
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
                ImplFillElementList(aElements, rxStore, ::rtl::OUString(), true, mode);
            }
        }
        break;
        case SignatureModeMacros:
        {
            // 1) Macros
            rtl::OUString aSubStorageName( rtl::OUString::createFromAscii( "Basic" ) );
            try
            {
                Reference < css::embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, css::embed::ElementModes::READ );
                ImplFillElementList(aElements, xSubStore, aSubStorageName+aSep, true, mode);
            }
            catch( com::sun::star::io::IOException& )
            {
                ; // Doesn't have to exist...
            }

            // 2) Dialogs
            aSubStorageName = rtl::OUString::createFromAscii( "Dialogs") ;
            try
            {
                Reference < css::embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, css::embed::ElementModes::READ );
                ImplFillElementList(aElements, xSubStore, aSubStorageName+aSep, true, mode);
            }
            catch( com::sun::star::io::IOException& )
            {
                ; // Doesn't have to exist...
            }
            // 3) Scripts
            aSubStorageName = rtl::OUString::createFromAscii( "Scripts") ;
            try
            {
                Reference < css::embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, css::embed::ElementModes::READ );
                ImplFillElementList(aElements, xSubStore, aSubStorageName+aSep, true, mode);
            }
            catch( css::io::IOException& )
            {
                ; // Doesn't have to exist...
            }
        }
        break;
        case SignatureModePackage:
        {
            // Everything except META-INF
            ImplFillElementList(aElements, rxStore, ::rtl::OUString(), true, mode);
        }
        break;
    }

    return aElements;
}

SignatureStreamHelper DocumentSignatureHelper::OpenSignatureStream(
    const Reference < css::embed::XStorage >& rxStore, sal_Int32 nOpenMode, DocumentSignatureMode eDocSigMode )
{
    sal_Int32 nSubStorageOpenMode = css::embed::ElementModes::READ;
    if ( nOpenMode & css::embed::ElementModes::WRITE )
        nSubStorageOpenMode = css::embed::ElementModes::WRITE;

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
    catch(css::io::IOException& )
    {
        // Doesn't have to exist...
        DBG_ASSERT( nOpenMode == css::embed::ElementModes::READ, "Error creating signature stream..." );
    }

    return aHelper;
}

//sElementList contains all files which are expected to be signed. Only those files must me signed,
//no more, no less.
//The DocumentSignatureAlgorithm indicates if the document was created with OOo 2.x. Then
//the uri s in the Reference elements in the signature, were not properly encoded.
// For example: <Reference URI="ObjectReplacements/Object 1">
bool DocumentSignatureHelper::checkIfAllFilesAreSigned(
    const ::std::vector< ::rtl::OUString > & sElementList,
    const SignatureInformation & sigInfo,
    const DocumentSignatureAlgorithm alg)
{
    // Can only be valid if ALL streams are signed, which means real stream count == signed stream count
    unsigned int nRealCount = 0;
    for ( int i = sigInfo.vSignatureReferenceInfors.size(); i; )
    {
        const SignatureReferenceInformation& rInf = sigInfo.vSignatureReferenceInfors[--i];
        // There is also an extra entry of type TYPE_SAMEDOCUMENT_REFERENCE because of signature date.
        if ( ( rInf.nType == TYPE_BINARYSTREAM_REFERENCE ) || ( rInf.nType == TYPE_XMLSTREAM_REFERENCE ) )
        {
            ::rtl::OUString sReferenceURI = rInf.ouURI;
            if (alg == OOo2Document)
            {
                //Comparing URIs is a difficult. Therefore we kind of normalize
                //it before comparing. We assume that our URI do not have a leading "./"
                //and fragments at the end (...#...)
                sReferenceURI = ::rtl::Uri::encode(
                    sReferenceURI, rtl_UriCharClassPchar,
                    rtl_UriEncodeCheckEscapes, RTL_TEXTENCODING_UTF8);
            }

            //find the file in the element list
            typedef ::std::vector< ::rtl::OUString >::const_iterator CIT;
            for (CIT aIter = sElementList.begin(); aIter < sElementList.end(); aIter++)
            {
                ::rtl::OUString sElementListURI = *aIter;
                if (alg == OOo2Document)
                {
                    sElementListURI =
                        ::rtl::Uri::encode(
                        sElementListURI, rtl_UriCharClassPchar,
                        rtl_UriEncodeCheckEscapes, RTL_TEXTENCODING_UTF8);
                }
                if (sElementListURI.equals(sReferenceURI))
                {
                    nRealCount++;
                    break;
                }
            }
        }
    }
    return  sElementList.size() == nRealCount;
}

/*Compares the Uri which are obtained from CreateElementList with
  the  path obtained from the manifest.xml.
  Returns true if both strings are equal.
*/
bool DocumentSignatureHelper::equalsReferenceUriManifestPath(
    const OUString & rUri, const OUString & rPath)
{
    bool retVal = false;
    //split up the uri and path into segments. Both are separated by '/'
    std::vector<OUString> vUriSegments;
    sal_Int32 nIndex = 0;
    do
    {
        OUString aToken = rUri.getToken( 0, '/', nIndex );
        vUriSegments.push_back(aToken);
    }
    while (nIndex >= 0);

    std::vector<OUString> vPathSegments;
    nIndex = 0;
    do
    {
        OUString aToken = rPath.getToken( 0, '/', nIndex );
        vPathSegments.push_back(aToken);
    }
    while (nIndex >= 0);

    //Now compare each segment of the uri with its counterpart from the path
    if (vUriSegments.size() == vPathSegments.size())
    {
        retVal = true;
        typedef std::vector<OUString>::const_iterator CIT;
        for (CIT i = vUriSegments.begin(), j = vPathSegments.begin();
            i != vUriSegments.end(); i++, j++)
        {
            //Decode the uri segment, so that %20 becomes ' ', etc.
            OUString sDecUri = ::rtl::Uri::decode(
                *i, rtl_UriDecodeWithCharset,  RTL_TEXTENCODING_UTF8);
            if (!sDecUri.equals(*j))
            {
                retVal = false;
                break;
            }
        }
    }

    return retVal;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
