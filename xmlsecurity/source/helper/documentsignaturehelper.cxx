/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


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

namespace
{
OUString getElement(OUString const & version, ::sal_Int32 * index)
{
    while (*index < version.getLength() && version[*index] == '0') {
        ++*index;
    }
    return version.getToken(0, '.', *index);
}





int compareVersions(
    OUString const & version1, OUString const & version2)
{
    for (::sal_Int32 i1 = 0, i2 = 0; i1 >= 0 || i2 >= 0;) {
        OUString e1(getElement(version1, &i1));
        OUString e2(getElement(version2, &i2));
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








void ImplFillElementList(
    std::vector< OUString >& rList, const Reference < css::embed::XStorage >& rxStore,
    const OUString rRootStorageName, const bool bRecursive,
    const DocumentSignatureAlgorithm mode)
{
    OUString aMetaInfName(  "META-INF"  );
    OUString sMimeTypeName ("mimetype");
    OUString aSep(  "/"  );

    Reference < css::container::XNameAccess > xElements( rxStore, UNO_QUERY );
    Sequence< OUString > aElements = xElements->getElementNames();
    sal_Int32 nElements = aElements.getLength();
    const OUString* pNames = aElements.getConstArray();

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
            OUString sEncName = ::rtl::Uri::encode(
                pNames[n], rtl_UriCharClassRelSegment,
                rtl_UriEncodeStrict, RTL_TEXTENCODING_UTF8);
            if (sEncName.isEmpty() && !pNames[n].isEmpty())
                throw css::uno::Exception("Failed to encode element name of XStorage", 0);

            if ( rxStore->isStreamElement( pNames[n] ) )
            {
                
                if (pNames[n].equals(
                    DocumentSignatureHelper::GetDocumentContentSignatureDefaultStreamName()))
                    continue;
                OUString aFullName( rRootStorageName + sEncName );
                rList.push_back(aFullName);
            }
            else if ( bRecursive && rxStore->isStorageElement( pNames[n] ) )
            {
                Reference < css::embed::XStorage > xSubStore = rxStore->openStorageElement( pNames[n], css::embed::ElementModes::READ );
                OUString aFullRootName( rRootStorageName + sEncName + aSep );
                ImplFillElementList(rList, xSubStore, aFullRootName, bRecursive, mode);
            }
        }
    }
}


bool DocumentSignatureHelper::isODFPre_1_2(const OUString & sVersion)
{
    
    
    
    if (compareVersions(sVersion, ODFVER_012_TEXT) == -1)
        return true;
    return false;
}

bool DocumentSignatureHelper::isOOo3_2_Signature(const SignatureInformation & sigInfo)
{
    OUString sManifestURI("META-INF/manifest.xml");
    bool bOOo3_2 = false;
    typedef ::std::vector< SignatureReferenceInformation >::const_iterator CIT;
    for (CIT i = sigInfo.vSignatureReferenceInfors.begin();
        i < sigInfo.vSignatureReferenceInfors.end(); ++i)
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
    const OUString & sODFVersion, const SignatureInformation & sigInfo)
{
    OSL_ASSERT(!sODFVersion.isEmpty());
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




//








//


std::vector< OUString >
DocumentSignatureHelper::CreateElementList(
    const Reference < css::embed::XStorage >& rxStore,
    const OUString /*rRootStorageName*/, DocumentSignatureMode eMode,
    const DocumentSignatureAlgorithm mode)
{
    std::vector< OUString > aElements;
    OUString aSep(  "/"  );

    switch ( eMode )
    {
        case SignatureModeDocumentContent:
        {
            if (mode == OOo2Document) 
            {
                
                ImplFillElementList(aElements, rxStore, OUString(), false, mode);

                
                OUString aSubStorageName( "Pictures" );
                try
                {
                    Reference < css::embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, css::embed::ElementModes::READ );
                    ImplFillElementList(aElements, xSubStore, aSubStorageName+aSep, true, mode);
                }
                catch(css::io::IOException& )
                {
                    ; 
                }
                
                aSubStorageName = "ObjectReplacements";
                try
                {
                    Reference < css::embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, css::embed::ElementModes::READ );
                    ImplFillElementList(aElements, xSubStore, aSubStorageName+aSep, true, mode);
                    xSubStore.clear();

                    
                    OUString aMatchStr( "Object " );
                    Reference < css::container::XNameAccess > xElements( rxStore, UNO_QUERY );
                    Sequence< OUString > aElementNames = xElements->getElementNames();
                    sal_Int32 nElements = aElementNames.getLength();
                    const OUString* pNames = aElementNames.getConstArray();
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
                    ; 
                }
            }
            else
            {
                
                ImplFillElementList(aElements, rxStore, OUString(), true, mode);
            }
        }
        break;
        case SignatureModeMacros:
        {
            
            OUString aSubStorageName( "Basic" );
            try
            {
                Reference < css::embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, css::embed::ElementModes::READ );
                ImplFillElementList(aElements, xSubStore, aSubStorageName+aSep, true, mode);
            }
            catch( com::sun::star::io::IOException& )
            {
                ; 
            }

            
            aSubStorageName = "Dialogs";
            try
            {
                Reference < css::embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, css::embed::ElementModes::READ );
                ImplFillElementList(aElements, xSubStore, aSubStorageName+aSep, true, mode);
            }
            catch( com::sun::star::io::IOException& )
            {
                ; 
            }
            
            aSubStorageName = "Scripts";
            try
            {
                Reference < css::embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, css::embed::ElementModes::READ );
                ImplFillElementList(aElements, xSubStore, aSubStorageName+aSep, true, mode);
            }
            catch( css::io::IOException& )
            {
                ; 
            }
        }
        break;
        case SignatureModePackage:
        {
            
            ImplFillElementList(aElements, rxStore, OUString(), true, mode);
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
        OUString aSIGStoreName(  "META-INF"  );
        aHelper.xSignatureStorage = rxStore->openStorageElement( aSIGStoreName, nSubStorageOpenMode );
        if ( aHelper.xSignatureStorage.is() )
        {
            OUString aSIGStreamName;
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
        
        DBG_ASSERT( nOpenMode == css::embed::ElementModes::READ, "Error creating signature stream..." );
    }

    return aHelper;
}






bool DocumentSignatureHelper::checkIfAllFilesAreSigned(
    const ::std::vector< OUString > & sElementList,
    const SignatureInformation & sigInfo,
    const DocumentSignatureAlgorithm alg)
{
    
    unsigned int nRealCount = 0;
    for ( int i = sigInfo.vSignatureReferenceInfors.size(); i; )
    {
        const SignatureReferenceInformation& rInf = sigInfo.vSignatureReferenceInfors[--i];
        
        if ( ( rInf.nType == TYPE_BINARYSTREAM_REFERENCE ) || ( rInf.nType == TYPE_XMLSTREAM_REFERENCE ) )
        {
            OUString sReferenceURI = rInf.ouURI;
            if (alg == OOo2Document)
            {
                
                
                
                sReferenceURI = ::rtl::Uri::encode(
                    sReferenceURI, rtl_UriCharClassPchar,
                    rtl_UriEncodeCheckEscapes, RTL_TEXTENCODING_UTF8);
            }

            
            typedef ::std::vector< OUString >::const_iterator CIT;
            for (CIT aIter = sElementList.begin(); aIter != sElementList.end(); ++aIter)
            {
                OUString sElementListURI = *aIter;
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

    
    if (vUriSegments.size() == vPathSegments.size())
    {
        retVal = true;
        typedef std::vector<OUString>::const_iterator CIT;
        for (CIT i = vUriSegments.begin(), j = vPathSegments.begin();
            i != vUriSegments.end(); ++i, ++j)
        {
            
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

OUString DocumentSignatureHelper::GetDocumentContentSignatureDefaultStreamName()
{
    return OUString(  "documentsignatures.xml"  );
}

OUString DocumentSignatureHelper::GetScriptingContentSignatureDefaultStreamName()
{
    return OUString(  "macrosignatures.xml"  );
}

OUString DocumentSignatureHelper::GetPackageSignatureDefaultStreamName()
{
    return OUString(  "packagesignatures.xml"  );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
