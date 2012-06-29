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



#ifndef _MANIFEST_IMPORT_HXX
#define _MANIFEST_IMPORT_HXX

#include <cppuhelper/implbase1.hxx> // helper for implementations
#ifndef _COM_SUN_STAR_XML_SAX_XDUCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#include "PackageConstants.hxx"
#include <vector>

#include <com/sun/star/beans/PropertyValues.hpp>
#include <HashMaps.hxx>

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { struct PropertyValue; }
} } }

typedef ::std::hash_map< ::rtl::OUString, ::rtl::OUString, ::rtl::OUStringHash, eqFunc > StringHashMap;

struct ManifestScopeEntry
{
    ::rtl::OUString m_aConvertedName;
    StringHashMap   m_aNamespaces;

    ManifestScopeEntry( const ::rtl::OUString& aConvertedName, const StringHashMap& aNamespaces )
    : m_aConvertedName( aConvertedName )
    , m_aNamespaces( aNamespaces )
    {}

    ~ManifestScopeEntry()
    {}
};

typedef ::std::vector< ManifestScopeEntry > ManifestStack;

class ManifestImport : public cppu::WeakImplHelper1 < com::sun::star::xml::sax::XDocumentHandler >
{
protected:
    ::com::sun::star::uno::Any maValues[ PKG_SIZE_ENCR_MNFST ];

    ::std::vector < ::com::sun::star::beans::PropertyValues > & rManVector;
    ManifestStack aStack;
    sal_Int32 nDerivedKeySize;
    bool bIgnoreEncryptData;

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
};
#endif
