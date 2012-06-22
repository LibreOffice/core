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

#ifndef _MANIFEST_IMPORT_HXX
#define _MANIFEST_IMPORT_HXX

#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <comphelper/sequenceasvector.hxx>
#include <vector>

#include <HashMaps.hxx>

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { struct PropertyValue; }
} } }

typedef ::boost::unordered_map< ::rtl::OUString, ::rtl::OUString, ::rtl::OUStringHash, eqFunc > StringHashMap;

struct ManifestScopeEntry
{
    ::rtl::OUString m_aConvertedName;
    StringHashMap   m_aNamespaces;
    bool            m_bValid;

    ManifestScopeEntry( const ::rtl::OUString& aConvertedName, const StringHashMap& aNamespaces )
    : m_aConvertedName( aConvertedName )
    , m_aNamespaces( aNamespaces )
    , m_bValid( true )
    {}

    ~ManifestScopeEntry()
    {}
};

typedef ::std::vector< ManifestScopeEntry > ManifestStack;

class ManifestImport : public cppu::WeakImplHelper1 < com::sun::star::xml::sax::XDocumentHandler >
{
protected:
    comphelper::SequenceAsVector< com::sun::star::beans::PropertyValue > aSequence;
    ManifestStack aStack;
    sal_Bool bIgnoreEncryptData;
    sal_Int32 nDerivedKeySize;
    ::std::vector < ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue > > & rManVector;

    const ::rtl::OUString sFileEntryElement;
    const ::rtl::OUString sManifestElement;
    const ::rtl::OUString sEncryptionDataElement;
    const ::rtl::OUString sAlgorithmElement;
    const ::rtl::OUString sStartKeyAlgElement;
    const ::rtl::OUString sKeyDerivationElement;

    const ::rtl::OUString sCdataAttribute;
    const ::rtl::OUString sMediaTypeAttribute;
    const ::rtl::OUString sVersionAttribute;
    const ::rtl::OUString sFullPathAttribute;
    const ::rtl::OUString sSizeAttribute;
    const ::rtl::OUString sSaltAttribute;
    const ::rtl::OUString sInitialisationVectorAttribute;
    const ::rtl::OUString sIterationCountAttribute;
    const ::rtl::OUString sKeySizeAttribute;
    const ::rtl::OUString sAlgorithmNameAttribute;
    const ::rtl::OUString sStartKeyAlgNameAttribute;
    const ::rtl::OUString sKeyDerivationNameAttribute;
    const ::rtl::OUString sChecksumAttribute;
    const ::rtl::OUString sChecksumTypeAttribute;

    const ::rtl::OUString sFullPathProperty;
    const ::rtl::OUString sMediaTypeProperty;
    const ::rtl::OUString sVersionProperty;
    const ::rtl::OUString sIterationCountProperty;
    const ::rtl::OUString sDerivedKeySizeProperty;
    const ::rtl::OUString sSaltProperty;
    const ::rtl::OUString sInitialisationVectorProperty;
    const ::rtl::OUString sSizeProperty;
    const ::rtl::OUString sDigestProperty;
    const ::rtl::OUString sEncryptionAlgProperty;
    const ::rtl::OUString sStartKeyAlgProperty;
    const ::rtl::OUString sDigestAlgProperty;

    const ::rtl::OUString sWhiteSpace;

    const ::rtl::OUString sSHA256_URL;
    const ::rtl::OUString sSHA1_Name;
    const ::rtl::OUString sSHA1_URL;

    const ::rtl::OUString sSHA256_1k_URL;
    const ::rtl::OUString sSHA1_1k_Name;
    const ::rtl::OUString sSHA1_1k_URL;

    const ::rtl::OUString sBlowfish_Name;
    const ::rtl::OUString sBlowfish_URL;
    const ::rtl::OUString sAES128_URL;
    const ::rtl::OUString sAES192_URL;
    const ::rtl::OUString sAES256_URL;

    const ::rtl::OUString sPBKDF2_Name;
    const ::rtl::OUString sPBKDF2_URL;


    ::rtl::OUString PushNameAndNamespaces( const ::rtl::OUString& aName,
                                           const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttribs,
                                           StringHashMap& o_aConvertedAttribs );
    ::rtl::OUString ConvertNameWithNamespace( const ::rtl::OUString& aName, const StringHashMap& aNamespaces );
    ::rtl::OUString ConvertName( const ::rtl::OUString& aName );

public:
    ManifestImport( std::vector < ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue > > & rNewVector );
    ~ManifestImport( void );
    virtual void SAL_CALL startDocument(  )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endDocument(  )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startElement( const ::rtl::OUString& aName, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttribs )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endElement( const ::rtl::OUString& aName )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL characters( const ::rtl::OUString& aChars )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL ignorableWhitespace( const ::rtl::OUString& aWhitespaces )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL processingInstruction( const ::rtl::OUString& aTarget, const ::rtl::OUString& aData )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDocumentLocator( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& xLocator )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
private:
    void doFileEntry(StringHashMap &rConvertedAttribs) throw(::com::sun::star::uno::RuntimeException);
    void doEncryptionData(StringHashMap &rConvertedAttribs) throw(::com::sun::star::uno::RuntimeException);
    void doAlgorithm(StringHashMap &rConvertedAttribs) throw(::com::sun::star::uno::RuntimeException);
    void doKeyDerivation(StringHashMap &rConvertedAttribs) throw(::com::sun::star::uno::RuntimeException);
    void doStartKeyAlg(StringHashMap &rConvertedAttribs) throw(::com::sun::star::uno::RuntimeException);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
