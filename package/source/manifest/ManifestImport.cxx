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



// MARKER( update_precomp.py ): autogen include statement, do not remove
#include "precompiled_package.hxx"
#include <ManifestImport.hxx>
#include <ManifestDefines.hxx>
#include <sax/tools/converter.hxx>

#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>
#include <com/sun/star/xml/crypto/CipherID.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star;
using namespace rtl;
using namespace std;

// helper for ignoring multiple settings of the same property
#define setProperty(e,v) do{ if(!maValues[e].hasValue()) maValues[e] <<= v;} while(0)

static const char* getMnfstPropName( int nManifestPropId )
{
    const char* pName;
    switch( nManifestPropId )
    {
        case PKG_MNFST_MEDIATYPE:   pName = "MediaType"; break;
        case PKG_MNFST_VERSION:     pName = "Version"; break;
        case PKG_MNFST_FULLPATH:    pName = "FullPath"; break;
        case PKG_MNFST_INIVECTOR:   pName = "InitialisationVector"; break;
        case PKG_MNFST_SALT:        pName = "Salt"; break;
        case PKG_MNFST_ITERATION:   pName = "IterationCount"; break;
        case PKG_MNFST_UCOMPSIZE:   pName = "Size"; break;
        case PKG_MNFST_DIGEST:      pName = "Digest"; break;
        case PKG_MNFST_ENCALG:      pName = "EncryptionAlgorithm"; break;
        case PKG_MNFST_STARTALG:    pName = "StartKeyAlgorithm"; break;
        case PKG_MNFST_DIGESTALG:   pName = "DigestAlgorithm"; break;
        case PKG_MNFST_DERKEYSIZE:  pName = "DerivedKeySize"; break;
        default: pName = NULL;
    }
    return pName;
}

// ---------------------------------------------------
ManifestImport::ManifestImport( vector < Sequence < PropertyValue > > & rNewManVector )
: rManVector ( rNewManVector )
, nDerivedKeySize( 0 )
, bIgnoreEncryptData( false )

, sCdataAttribute               ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_CDATA ) )
, sMediaTypeAttribute           ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_MEDIA_TYPE ) )
, sVersionAttribute             ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_VERSION ) )
, sFullPathAttribute            ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_FULL_PATH ) )
, sSizeAttribute                ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_SIZE ) )
, sSaltAttribute                ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_SALT ) )
, sInitialisationVectorAttribute(RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_INITIALISATION_VECTOR ) )
, sIterationCountAttribute      ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_ITERATION_COUNT ) )
, sKeySizeAttribute            ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_KEY_SIZE ) )
, sAlgorithmNameAttribute       ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_ALGORITHM_NAME ) )
, sStartKeyAlgNameAttribute    ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_START_KEY_GENERATION_NAME ) )
, sKeyDerivationNameAttribute   ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_KEY_DERIVATION_NAME ) )
, sChecksumAttribute            ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_CHECKSUM ) )
, sChecksumTypeAttribute        ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_CHECKSUM_TYPE ) )
{
    aStack.reserve( 10 );
}

// ---------------------------------------------------
ManifestImport::~ManifestImport ( void )
{
}

// ---------------------------------------------------
void SAL_CALL ManifestImport::startDocument(  )
        throw( xml::sax::SAXException, uno::RuntimeException )
{
}

// ---------------------------------------------------
void SAL_CALL ManifestImport::endDocument(  )
        throw( xml::sax::SAXException, uno::RuntimeException )
{
}

// ---------------------------------------------------
void SAL_CALL ManifestImport::startElement( const OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs )
        throw( xml::sax::SAXException, uno::RuntimeException )
{
    StringHashMap aConvertedAttribs;
    ::rtl::OUString aConvertedName = PushNameAndNamespaces( aName, xAttribs, aConvertedAttribs );

    if ( aConvertedName.equalsAscii( ELEMENT_FILE_ENTRY ) )
    {
        setProperty( PKG_MNFST_FULLPATH, aConvertedAttribs[sFullPathAttribute]);
        setProperty( PKG_MNFST_MEDIATYPE, aConvertedAttribs[sMediaTypeAttribute]);

        const OUString& sVersion = aConvertedAttribs[sVersionAttribute];
        if ( sVersion.getLength() )
            setProperty( PKG_MNFST_VERSION, sVersion );

        const OUString& sSize = aConvertedAttribs[sSizeAttribute];
        if ( sSize.getLength() )
            setProperty( PKG_MNFST_UCOMPSIZE, sSize.toInt32() );
    }
    else if ( aStack.size() > 1 )
    {
        ManifestStack::reverse_iterator aIter = aStack.rbegin();
        aIter++;

        if ( aIter->m_aConvertedName.equalsAscii( ELEMENT_FILE_ENTRY ) )
        {
            if ( aConvertedName.equalsAscii( ELEMENT_ENCRYPTION_DATA ) )
            {
                // If this element exists, then this stream is encrypted and we need
                // to import the initialisation vector, salt and iteration count used
                nDerivedKeySize = 0;
                if ( !bIgnoreEncryptData )
                {
                    sal_Int32 nDigestId = 0;
                    const OUString& rChecksumType = aConvertedAttribs[sChecksumTypeAttribute];
                    if( rChecksumType.equalsAscii( SHA1_1K_NAME )
                    ||  rChecksumType.equalsAscii( SHA1_1K_URL ) )
                        nDigestId = xml::crypto::DigestID::SHA1_1K;
                    else if ( rChecksumType.equalsAscii( SHA256_1K_URL ) )
                        nDigestId = xml::crypto::DigestID::SHA256_1K;
                    else
                        bIgnoreEncryptData = true;

                    if ( !bIgnoreEncryptData )
                    {
                        setProperty( PKG_MNFST_DIGESTALG, nDigestId );
                        const OUString& sChecksumData = aConvertedAttribs[sChecksumAttribute];
                        uno::Sequence < sal_Int8 > aDecodeBuffer;
                        ::sax::Converter::decodeBase64( aDecodeBuffer, sChecksumData );
                        setProperty( PKG_MNFST_DIGEST, aDecodeBuffer );
                    }
                }
            }
        }
        else if ( aIter->m_aConvertedName.equalsAscii( ELEMENT_ENCRYPTION_DATA ) )
        {
            if ( aConvertedName.equalsAscii( ELEMENT_ALGORITHM ) )
            {
                if ( !bIgnoreEncryptData )
                {
                    sal_Int32 nCypherId = 0;
                    const OUString& rAlgoName = aConvertedAttribs[sAlgorithmNameAttribute];
                    if ( rAlgoName.equalsAscii( BLOWFISH_NAME )
                    ||   rAlgoName.equalsAscii( BLOWFISH_URL ) )
                         nCypherId = xml::crypto::CipherID::BLOWFISH_CFB_8;
                    else if( rAlgoName.equalsAscii( AES256_URL ) )
                    {
                         nCypherId = xml::crypto::CipherID::AES_CBC_W3C_PADDING;
                        OSL_ENSURE( !nDerivedKeySize || nDerivedKeySize == 32, "Unexpected derived key length!" );
                        nDerivedKeySize = 32;
                    }
                    else if( rAlgoName.equalsAscii( AES192_URL ) )
                    {
                         nCypherId = xml::crypto::CipherID::AES_CBC_W3C_PADDING;
                        OSL_ENSURE( !nDerivedKeySize || nDerivedKeySize == 24, "Unexpected derived key length!" );
                        nDerivedKeySize = 24;
                    }
                    else if( rAlgoName.equalsAscii( AES128_URL ) )
                    {
                         nCypherId = xml::crypto::CipherID::AES_CBC_W3C_PADDING;
                        OSL_ENSURE( !nDerivedKeySize || nDerivedKeySize == 16, "Unexpected derived key length!" );
                        nDerivedKeySize = 16;
                    }
                    else
                        bIgnoreEncryptData = true;

                    if ( !bIgnoreEncryptData )
                    {
                         setProperty( PKG_MNFST_ENCALG, nCypherId );
                        const OUString& sInitVector = aConvertedAttribs[sInitialisationVectorAttribute];
                        uno::Sequence < sal_Int8 > aDecodeBuffer;
                        ::sax::Converter::decodeBase64 ( aDecodeBuffer, sInitVector );
                         setProperty( PKG_MNFST_INIVECTOR, aDecodeBuffer );
                    }
                }
            }
            else if ( aConvertedName.equalsAscii( ELEMENT_KEY_DERIVATION ) )
            {
                if ( !bIgnoreEncryptData )
                {
                    const OUString& rKeyDerivString = aConvertedAttribs[sKeyDerivationNameAttribute];
                    if ( rKeyDerivString.equalsAscii( PBKDF2_NAME ) || rKeyDerivString.equalsAscii( PBKDF2_URL ) )
                    {
                        const OUString& rSaltString = aConvertedAttribs[sSaltAttribute];
                        uno::Sequence < sal_Int8 > aDecodeBuffer;
                        ::sax::Converter::decodeBase64 ( aDecodeBuffer, rSaltString );
                         setProperty( PKG_MNFST_SALT, aDecodeBuffer );

                        const OUString& rIterationCount = aConvertedAttribs[sIterationCountAttribute];
                        setProperty( PKG_MNFST_ITERATION, rIterationCount.toInt32() );

                        const OUString& rKeySize = aConvertedAttribs[sKeySizeAttribute];
                        if ( rKeySize.getLength() )
                        {
                            const sal_Int32 nKey = rKeySize.toInt32();
                            OSL_ENSURE( !nDerivedKeySize || nKey == nDerivedKeySize , "Provided derived key length differs from the expected one!" );
                            nDerivedKeySize = nKey;
                        }
                        else if ( !nDerivedKeySize )
                            nDerivedKeySize = 16;
                        else if ( nDerivedKeySize != 16 )
                            OSL_ENSURE( sal_False, "Default derived key length differs from the expected one!" );

                        setProperty( PKG_MNFST_DERKEYSIZE, nDerivedKeySize );
                    }
                    else
                        bIgnoreEncryptData = true;
                }
            }
            else if ( aConvertedName.equalsAscii( ELEMENT_START_KEY_GENERATION ) )
            {
                const OUString& rSKeyAlg = aConvertedAttribs[sStartKeyAlgNameAttribute];
                if ( rSKeyAlg.equalsAscii( SHA256_URL ) )
                    setProperty( PKG_MNFST_STARTALG, xml::crypto::DigestID::SHA256 );
                else if ( rSKeyAlg.equalsAscii( SHA1_NAME ) || rSKeyAlg.equalsAscii( SHA1_URL ) )
                    setProperty( PKG_MNFST_STARTALG, xml::crypto::DigestID::SHA1 );
                else
                    bIgnoreEncryptData = true;
            }
        }
    }
}

// ---------------------------------------------------
void SAL_CALL ManifestImport::endElement( const OUString& aName )
    throw( xml::sax::SAXException, uno::RuntimeException )
{
    if( aStack.empty() )
        return;

    const OUString aConvertedName = ConvertName( aName );
    if( !aStack.rbegin()->m_aConvertedName.equals( aConvertedName ) )
        return;

    aStack.pop_back();

    if( !aConvertedName.equalsAscii( ELEMENT_FILE_ENTRY ) )
        return;

    // create the property sequence
    // Put full-path property first for MBA
    // TODO: get rid of fullpath-first requirement
    const bool bHasFullPath = maValues[PKG_MNFST_FULLPATH].hasValue();
    OSL_ENSURE( bHasFullPath, "Full path missing in manifest" );

    int nNumProperty = bHasFullPath ? 1 : 0;
    PropertyValue aProperties[ PKG_SIZE_ENCR_MNFST ];
    for( int i = 0; i < PKG_SIZE_ENCR_MNFST; ++i)
    {
        if(! maValues[i].hasValue() )
            continue;

        const int nDest = (i == PKG_MNFST_FULLPATH) ? 0 : nNumProperty++;
        PropertyValue& rProp = aProperties[ nDest ];
        rProp.Name = OUString::createFromAscii( getMnfstPropName(i));
        rProp.Value = maValues[i];
        maValues[i].clear();
    }

    // add the property sequence to the vector of manifests
    rManVector.push_back ( PropertyValues( aProperties, nNumProperty ) );
    bIgnoreEncryptData = false;
}

// ---------------------------------------------------
void SAL_CALL ManifestImport::characters( const OUString& /*aChars*/ )
        throw( xml::sax::SAXException, uno::RuntimeException )
{
}

// ---------------------------------------------------
void SAL_CALL ManifestImport::ignorableWhitespace( const OUString& /*aWhitespaces*/ )
        throw( xml::sax::SAXException, uno::RuntimeException )
{
}

// ---------------------------------------------------
void SAL_CALL ManifestImport::processingInstruction( const OUString& /*aTarget*/, const OUString& /*aData*/ )
        throw( xml::sax::SAXException, uno::RuntimeException )
{
}

// ---------------------------------------------------
void SAL_CALL ManifestImport::setDocumentLocator( const uno::Reference< xml::sax::XLocator >& /*xLocator*/ )
        throw( xml::sax::SAXException, uno::RuntimeException )
{
}

// ---------------------------------------------------
::rtl::OUString ManifestImport::PushNameAndNamespaces( const ::rtl::OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs, StringHashMap& o_aConvertedAttribs )
{
    StringHashMap aNamespaces;
    ::std::vector< ::std::pair< ::rtl::OUString, ::rtl::OUString > > aAttribsStrs;

    if ( xAttribs.is() )
    {
        sal_Int16 nAttrCount = xAttribs.is() ? xAttribs->getLength() : 0;
        aAttribsStrs.reserve( nAttrCount );

        for( sal_Int16 nInd = 0; nInd < nAttrCount; nInd++ )
        {
            ::rtl::OUString aAttrName = xAttribs->getNameByIndex( nInd );
            ::rtl::OUString aAttrValue = xAttribs->getValueByIndex( nInd );
            if ( aAttrName.getLength() >= 5
              && aAttrName.compareToAscii( "xmlns", 5 ) == 0
              && ( aAttrName.getLength() == 5 || aAttrName.getStr()[5] == ( sal_Unicode )':' ) )
            {
                // this is a namespace declaration
                ::rtl::OUString aNsName( ( aAttrName.getLength() == 5 ) ? ::rtl::OUString() : aAttrName.copy( 6 ) );
                aNamespaces[aNsName] = aAttrValue;
            }
            else
            {
                // this is no namespace declaration
                aAttribsStrs.push_back( pair< ::rtl::OUString, ::rtl::OUString >( aAttrName, aAttrValue ) );
            }
        }
    }

    ::rtl::OUString aConvertedName = ConvertNameWithNamespace( aName, aNamespaces );
    if ( !aConvertedName.getLength() )
        aConvertedName = ConvertName( aName );

    aStack.push_back( ManifestScopeEntry( aConvertedName, aNamespaces ) );

    for ( sal_uInt16 nInd = 0; nInd < aAttribsStrs.size(); nInd++ )
    {
        // convert the attribute names on filling
        o_aConvertedAttribs[ConvertName( aAttribsStrs[nInd].first )] = aAttribsStrs[nInd].second;
    }

    return aConvertedName;
}


// ---------------------------------------------------
::rtl::OUString ManifestImport::ConvertNameWithNamespace( const ::rtl::OUString& aName, const StringHashMap& aNamespaces )
{
    ::rtl::OUString aNsAlias;
    ::rtl::OUString aPureName = aName;

    sal_Int32 nInd = aName.indexOf( ( sal_Unicode )':' );
    if ( nInd != -1 && nInd < aName.getLength() )
    {
        aNsAlias = aName.copy( 0, nInd );
        aPureName = aName.copy( nInd + 1 );
    }

    ::rtl::OUString aResult;

    StringHashMap::const_iterator aIter = aNamespaces.find( aNsAlias );
    if ( aIter != aNamespaces.end()
      && ( aIter->second.equalsAscii( MANIFEST_NAMESPACE )
        || aIter->second.equalsAscii( MANIFEST_OASIS_NAMESPACE ) ) )
    {
        // no check for manifest.xml consistency currently since the old versions have supported inconsistent documents as well
        aResult = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MANIFEST_NSPREFIX ) );
        aResult += aPureName;
    }

    return aResult;
}

// ---------------------------------------------------
::rtl::OUString ManifestImport::ConvertName( const ::rtl::OUString& aName )
{
    ::rtl::OUString aConvertedName;
    for ( ManifestStack::reverse_iterator aIter = aStack.rbegin(); !aConvertedName.getLength() && aIter != aStack.rend(); aIter++ )
    {
        if ( !aIter->m_aNamespaces.empty() )
            aConvertedName = ConvertNameWithNamespace( aName, aIter->m_aNamespaces );
    }

    if ( !aConvertedName.getLength() )
        aConvertedName = aName;

    return aConvertedName;
}

