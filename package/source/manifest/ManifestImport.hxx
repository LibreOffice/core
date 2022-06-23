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
#include <unordered_map>
#include <utility>
#include <vector>
#include <rtl/ustrbuf.hxx>

namespace com::sun::star {
    namespace xml::sax { class XAttributeList; }
    namespace beans { struct PropertyValue; }
}

typedef std::unordered_map< OUString, OUString > StringHashMap;

struct ManifestScopeEntry
{
    OUString        m_aConvertedName;
    StringHashMap   m_aNamespaces;
    bool            m_bValid;

    ManifestScopeEntry( OUString aConvertedName, StringHashMap&& aNamespaces )
    : m_aConvertedName(std::move( aConvertedName ))
    , m_aNamespaces( std::move(aNamespaces) )
    , m_bValid( true )
    {}
};

typedef ::std::vector< ManifestScopeEntry > ManifestStack;

class ManifestImport final : public cppu::WeakImplHelper < css::xml::sax::XDocumentHandler >
{
    std::vector< css::beans::NamedValue > aKeyInfoSequence;
    std::vector< css::uno::Sequence< css::beans::NamedValue > > aKeys;
    std::vector< css::beans::PropertyValue > aSequence;
    OUStringBuffer aCurrentCharacters{64};
    ManifestStack aStack;
    bool bIgnoreEncryptData;
    bool bPgpEncryption;
    sal_Int32 nDerivedKeySize;
    ::std::vector < css::uno::Sequence < css::beans::PropertyValue > > & rManVector;


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
    void doEncryptedKey(StringHashMap &);
    void doEncryptionMethod(StringHashMap &, const OUString &);
    void doEncryptedCipherValue();
    void doEncryptedKeyId();
    void doEncryptedKeyPacket();
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
