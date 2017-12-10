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
#include <com/sun/star/beans/NamedValue.hpp>
#include <vector>

#include <HashMaps.hxx>

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { struct PropertyValue; }
} } }

typedef std::unordered_map< OUString, OUString > StringHashMap;

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
};

typedef ::std::vector< ManifestScopeEntry > ManifestStack;

class ManifestImport final : public cppu::WeakImplHelper < css::xml::sax::XDocumentHandler >
{
    std::vector< css::beans::NamedValue > aKeyInfoSequence;
    std::vector< css::uno::Sequence< css::beans::NamedValue > > aKeys;
    std::vector< css::beans::PropertyValue > aSequence;
    OUString aCurrentCharacters;
    ManifestStack aStack;
    bool bIgnoreEncryptData;
    bool bPgpEncryption;
    sal_Int32 nDerivedKeySize;
    ::std::vector < css::uno::Sequence < css::beans::PropertyValue > > & rManVector;

    const OUString sFileEntryElement;
    const OUString sEncryptionDataElement;
    const OUString sAlgorithmElement;
    const OUString sStartKeyAlgElement;
    const OUString sKeyDerivationElement;

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

    const OUString sKeyInfoElement;
    const OUString sManifestKeyInfoElement;
    const OUString sEncryptedKeyElement;
    const OUString sEncryptionMethodElement;
    const OUString sPgpDataElement;
    const OUString sPgpKeyIDElement;
    const OUString sPGPKeyPacketElement;
    const OUString sAlgorithmAttribute;
    const OUString sCipherDataElement;
    const OUString sCipherValueElement;

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

    const OUString sSHA256_URL_ODF12;
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
    virtual ~ManifestImport() override;
    virtual void SAL_CALL startDocument(  ) override;
    virtual void SAL_CALL endDocument(  ) override;
    virtual void SAL_CALL startElement( const OUString& aName, const css::uno::Reference< css::xml::sax::XAttributeList >& xAttribs ) override;
    virtual void SAL_CALL endElement( const OUString& aName ) override;
    virtual void SAL_CALL characters( const OUString& aChars ) override;
    virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces ) override;
    virtual void SAL_CALL processingInstruction( const OUString& aTarget, const OUString& aData ) override;
    virtual void SAL_CALL setDocumentLocator( const css::uno::Reference< css::xml::sax::XLocator >& xLocator ) override;

private:
    /// @throws css::uno::RuntimeException
    void doFileEntry(StringHashMap &rConvertedAttribs);
    /// @throws css::uno::RuntimeException
    void doEncryptionData(StringHashMap &rConvertedAttribs);
    /// @throws css::uno::RuntimeException
    void doAlgorithm(StringHashMap &rConvertedAttribs);
    /// @throws css::uno::RuntimeException
    void doKeyDerivation(StringHashMap &rConvertedAttribs);
    /// @throws css::uno::RuntimeException
    void doStartKeyAlg(StringHashMap &rConvertedAttribs);
    void doKeyInfoEntry(StringHashMap &);
    void doEncryptedKey(StringHashMap &);
    void doEncryptionMethod(StringHashMap &);
    void doEncryptedKeyInfo(StringHashMap &);
    void doEncryptedCipherData(StringHashMap &);
    void doEncryptedPgpData(StringHashMap &);
    void doEncryptedCipherValue();
    void doEncryptedKeyId();
    void doEncryptedKeyPacket();
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
