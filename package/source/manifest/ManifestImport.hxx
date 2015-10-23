/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_PACKAGE_SOURCE_MANIFEST_MANIFESTIMPORT_HXX
#define INCLUDED_PACKAGE_SOURCE_MANIFEST_MANIFESTIMPORT_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <vector>

#include <HashMaps.hxx>

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { struct PropertyValue; }
} } }

typedef std::unordered_map< OUString, OUString, OUStringHash, eqFunc > StringHashMap;

struct ManifestScopeEntry
{
    OUString m_aConvertedName;
    StringHashMap   m_aNamespaces;
    bool            m_bValid;

    ManifestScopeEntry( const OUString& aConvertedName, const StringHashMap& aNamespaces )
    : m_aConvertedName( aConvertedName )
    , m_aNamespaces( aNamespaces )
    , m_bValid( true )
    {}

    ~ManifestScopeEntry()
    {}
};

typedef ::std::vector< ManifestScopeEntry > ManifestStack;

class ManifestImport : public cppu::WeakImplHelper < css::xml::sax::XDocumentHandler >
{
protected:
    std::vector< css::beans::PropertyValue > aSequence;
    ManifestStack aStack;
    bool bIgnoreEncryptData;
    sal_Int32 nDerivedKeySize;
    ::std::vector < css::uno::Sequence < css::beans::PropertyValue > > & rManVector;

    const OUString sFileEntryElement;
    const OUString sManifestElement;
    const OUString sEncryptionDataElement;
    const OUString sAlgorithmElement;
    const OUString sStartKeyAlgElement;
    const OUString sKeyDerivationElement;

    const OUString sCdataAttribute;
    const OUString sMediaTypeAttribute;
    const OUString sVersionAttribute;
    const OUString sFullPathAttribute;
    const OUString sSizeAttribute;
    const OUString sSaltAttribute;
    const OUString sInitialisationVectorAttribute;
    const OUString sIterationCountAttribute;
    const OUString sKeySizeAttribute;
    const OUString sAlgorithmNameAttribute;
    const OUString sStartKeyAlgNameAttribute;
    const OUString sKeyDerivationNameAttribute;
    const OUString sChecksumAttribute;
    const OUString sChecksumTypeAttribute;

    const OUString sFullPathProperty;
    const OUString sMediaTypeProperty;
    const OUString sVersionProperty;
    const OUString sIterationCountProperty;
    const OUString sDerivedKeySizeProperty;
    const OUString sSaltProperty;
    const OUString sInitialisationVectorProperty;
    const OUString sSizeProperty;
    const OUString sDigestProperty;
    const OUString sEncryptionAlgProperty;
    const OUString sStartKeyAlgProperty;
    const OUString sDigestAlgProperty;

    const OUString sWhiteSpace;

    const OUString sSHA256_URL;
    const OUString sSHA1_Name;
    const OUString sSHA1_URL;

    const OUString sSHA256_1k_URL;
    const OUString sSHA1_1k_Name;
    const OUString sSHA1_1k_URL;

    const OUString sBlowfish_Name;
    const OUString sBlowfish_URL;
    const OUString sAES128_URL;
    const OUString sAES192_URL;
    const OUString sAES256_URL;

    const OUString sPBKDF2_Name;
    const OUString sPBKDF2_URL;

    OUString PushNameAndNamespaces( const OUString& aName,
                                           const css::uno::Reference< css::xml::sax::XAttributeList >& xAttribs,
                                           StringHashMap& o_aConvertedAttribs );
    static OUString ConvertNameWithNamespace( const OUString& aName, const StringHashMap& aNamespaces );
    OUString ConvertName( const OUString& aName );

public:
    ManifestImport( std::vector < css::uno::Sequence < css::beans::PropertyValue > > & rNewVector );
    virtual ~ManifestImport();
    virtual void SAL_CALL startDocument(  )
        throw(css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endDocument(  )
        throw(css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL startElement( const OUString& aName, const css::uno::Reference< css::xml::sax::XAttributeList >& xAttribs )
        throw(css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement( const OUString& aName )
        throw(css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL characters( const OUString& aChars )
        throw(css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces )
        throw(css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL processingInstruction( const OUString& aTarget, const OUString& aData )
        throw(css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDocumentLocator( const css::uno::Reference< css::xml::sax::XLocator >& xLocator )
        throw(css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
private:
    void doFileEntry(StringHashMap &rConvertedAttribs) throw(css::uno::RuntimeException);
    void doEncryptionData(StringHashMap &rConvertedAttribs) throw(css::uno::RuntimeException);
    void doAlgorithm(StringHashMap &rConvertedAttribs) throw(css::uno::RuntimeException);
    void doKeyDerivation(StringHashMap &rConvertedAttribs) throw(css::uno::RuntimeException);
    void doStartKeyAlg(StringHashMap &rConvertedAttribs) throw(css::uno::RuntimeException);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
