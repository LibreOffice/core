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
#include "precompiled_linguistic.hxx"
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <tools/fsys.hxx>
#include <tools/string.hxx>
#include <i18npool/mslangid.hxx>
#include <tools/stream.hxx>
#include <osl/mutex.hxx>
#include <unotools/processfactory.hxx>
#include <ucbhelper/content.hxx>

#include <cppuhelper/factory.hxx>   // helper for factories
#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#include <com/sun/star/linguistic2/ConversionDictionaryType.hpp>
#include <com/sun/star/linguistic2/ConversionPropertyType.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HPP_
#include <com/sun/star/uno/Reference.h>
#endif
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/util/XFlushListener.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <unotools/streamwrap.hxx>

#include "convdic.hxx"
#include "convdicxml.hxx"
#include "linguistic/misc.hxx"
#include "defs.hxx"

using namespace std;
using namespace utl;
using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

#define XML_NAMESPACE_TCD_STRING        "http://openoffice.org/2003/text-conversion-dictionary"
#define CONV_TYPE_HANGUL_HANJA          "Hangul / Hanja"
#define CONV_TYPE_SCHINESE_TCHINESE     "Chinese simplified / Chinese traditional"

///////////////////////////////////////////////////////////////////////////

static const OUString ConversionTypeToText( sal_Int16 nConversionType )
{
    OUString aRes;
    if (nConversionType == ConversionDictionaryType::HANGUL_HANJA)
        aRes = A2OU( CONV_TYPE_HANGUL_HANJA );
    else if (nConversionType == ConversionDictionaryType::SCHINESE_TCHINESE)
        aRes = A2OU( CONV_TYPE_SCHINESE_TCHINESE );
    return aRes;
}

static sal_Int16 GetConversionTypeFromText( const String &rText )
{
    sal_Int16 nRes = -1;
    if (rText.EqualsAscii( CONV_TYPE_HANGUL_HANJA ))
        nRes = ConversionDictionaryType::HANGUL_HANJA;
    else if (rText.EqualsAscii( CONV_TYPE_SCHINESE_TCHINESE ))
        nRes = ConversionDictionaryType::SCHINESE_TCHINESE;
    return nRes;
}

///////////////////////////////////////////////////////////////////////////

class ConvDicXMLImportContext :
    public SvXMLImportContext
{
public:
    ConvDicXMLImportContext(
            ConvDicXMLImport &rImport,
            sal_uInt16 nPrfx, const OUString& rLName ) :
        SvXMLImportContext( rImport, nPrfx, rLName )
    {
    }

    const ConvDicXMLImport & GetConvDicImport() const
    {
        return (const ConvDicXMLImport &) GetImport();
    }

    ConvDicXMLImport & GetConvDicImport()
    {
        return (ConvDicXMLImport &) GetImport();
    }

    // SvXMLImportContext
    virtual void Characters( const OUString &rChars );
    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< xml::sax::XAttributeList > &rxAttrList);
};


class ConvDicXMLDictionaryContext_Impl :
    public ConvDicXMLImportContext
{
    sal_Int16       nLanguage;
    sal_Int16   nConversionType;

public:
    ConvDicXMLDictionaryContext_Impl( ConvDicXMLImport &rImport,
            sal_uInt16 nPrefix, const OUString& rLName) :
        ConvDicXMLImportContext( rImport, nPrefix, rLName )
    {
        nLanguage = LANGUAGE_NONE;
        nConversionType = -1;
    }

    // SvXMLImportContext
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< xml::sax::XAttributeList > &rxAttrList );

    sal_Int16       GetLanguage() const         { return nLanguage; }
    sal_Int16   GetConversionType() const   { return nConversionType; }
};


class ConvDicXMLEntryTextContext_Impl :
    public ConvDicXMLImportContext
{
    OUString    aLeftText;
    sal_Int16   nPropertyType;  // used for Chinese simplified/traditional conversion
    ConvDicXMLDictionaryContext_Impl    &rDicContext;

public:
    ConvDicXMLEntryTextContext_Impl(
            ConvDicXMLImport &rImport,
            sal_uInt16 nPrefix, const OUString& rLName,
            ConvDicXMLDictionaryContext_Impl &rParentContext ) :
        ConvDicXMLImportContext( rImport, nPrefix, rLName ),
        nPropertyType( ConversionPropertyType::NOT_DEFINED ),
        rDicContext( rParentContext )
    {
    }

    // SvXMLImportContext
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< xml::sax::XAttributeList > &rxAttrList );

    const OUString &    GetLeftText() const { return aLeftText; }
    sal_Int16           GetPropertyType() const { return nPropertyType; }
    void                SetPropertyType( sal_Int16 nVal )   { nPropertyType = nVal; }
};


class ConvDicXMLRightTextContext_Impl :
    public ConvDicXMLImportContext
{
    OUString aRightText;
    ConvDicXMLEntryTextContext_Impl &rEntryContext;

public:
    ConvDicXMLRightTextContext_Impl(
            ConvDicXMLImport &rImport,
            sal_uInt16 nPrefix, const OUString& rLName,
            ConvDicXMLEntryTextContext_Impl &rParentContext ) :
        ConvDicXMLImportContext( rImport, nPrefix, rLName ),
        rEntryContext( rParentContext )
    {
    }

    // SvXMLImportContext
    virtual void EndElement();
    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< xml::sax::XAttributeList > &rxAttrList );
    virtual void Characters( const OUString &rChars );

    const OUString &    GetRightText() const    { return aRightText; }
    const OUString &    GetLeftText() const     { return rEntryContext.GetLeftText(); }
    ConvDic *           GetDic()                { return GetConvDicImport().GetDic(); }
};

///////////////////////////////////////////////////////////////////////////

void ConvDicXMLImportContext::Characters(const OUString & /*rChars*/)
{
    /*
    Whitespace occurring within the content of token elements is "trimmed"
    from the ends (i.e. all whitespace at the beginning and end of the
    content is removed), and "collapsed" internally (i.e. each sequence of
    1 or more whitespace characters is replaced with one blank character).
    */
    //collapsing not done yet!

    // warning-free code: since the result is not used there is no need for trimming...
    //const OUString &rChars2 = rChars.trim();
}

SvXMLImportContext * ConvDicXMLImportContext::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & /*rxAttrList*/ )
{
    SvXMLImportContext *pContext = 0;
    if (nPrefix == XML_NAMESPACE_TCD && rLocalName.equalsAscii( "text-conversion-dictionary" ))
        pContext = new ConvDicXMLDictionaryContext_Impl( GetConvDicImport(), nPrefix, rLocalName );
    else
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    return pContext;
}

////////////////////////////////////////

void ConvDicXMLDictionaryContext_Impl::StartElement(
    const uno::Reference< xml::sax::XAttributeList > &rxAttrList )
{
    sal_Int16 nAttrCount = rxAttrList.is() ? rxAttrList->getLength() : 0;
    for (sal_Int16 i = 0;  i < nAttrCount;  ++i)
    {
        OUString aAttrName = rxAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
                                    GetKeyByAttrName( aAttrName, &aLocalName );
        OUString aValue = rxAttrList->getValueByIndex(i);

        if (nPrefix == XML_NAMESPACE_TCD && aLocalName.equalsAscii( "lang" ))
            nLanguage = MsLangId::convertIsoStringToLanguage( aValue );
        else if (nPrefix == XML_NAMESPACE_TCD && aLocalName.equalsAscii( "conversion-type" ))
            nConversionType = GetConversionTypeFromText( aValue );
    }
    GetConvDicImport().SetLanguage( nLanguage );
    GetConvDicImport().SetConversionType( nConversionType );

    //!! hack to stop the parser from reading the rest of the file  !!
    //!! when only the header (language, conversion type) is needed !!
//   if (GetConvDicImport().GetDic() == 0)
//        throw uno::RuntimeException();
}

SvXMLImportContext * ConvDicXMLDictionaryContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & /*rxAttrList*/ )
{
    SvXMLImportContext *pContext = 0;
    if (nPrefix == XML_NAMESPACE_TCD  &&  rLocalName.equalsAscii( "entry" ))
        pContext = new ConvDicXMLEntryTextContext_Impl( GetConvDicImport(), nPrefix, rLocalName, *this );
    else
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    return pContext;
}

////////////////////////////////////////

SvXMLImportContext * ConvDicXMLEntryTextContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & /*rxAttrList*/ )
{
    SvXMLImportContext *pContext = 0;
    if (nPrefix == XML_NAMESPACE_TCD  &&  rLocalName.equalsAscii( "right-text" ))
        pContext = new ConvDicXMLRightTextContext_Impl( GetConvDicImport(), nPrefix, rLocalName, *this );
    else
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    return pContext;
}

void ConvDicXMLEntryTextContext_Impl::StartElement(
        const uno::Reference< xml::sax::XAttributeList >& rxAttrList )
{
    sal_Int16 nAttrCount = rxAttrList.is() ? rxAttrList->getLength() : 0;
    for (sal_Int16 i = 0;  i < nAttrCount;  ++i)
    {
        OUString aAttrName = rxAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
                                    GetKeyByAttrName( aAttrName, &aLocalName );
        OUString aValue = rxAttrList->getValueByIndex(i);

        if (nPrefix == XML_NAMESPACE_TCD && aLocalName.equalsAscii( "left-text" ))
            aLeftText = aValue;
        if (nPrefix == XML_NAMESPACE_TCD && aLocalName.equalsAscii( "property-type" ))
            nPropertyType = (sal_Int16) aValue.toInt32();
    }
}

////////////////////////////////////////

SvXMLImportContext * ConvDicXMLRightTextContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & /*rxAttrList*/ )
{
    // leaf: return default (empty) context
    SvXMLImportContext *pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    return pContext;
}

void ConvDicXMLRightTextContext_Impl::Characters( const OUString &rChars )
{
    aRightText += rChars;
}

void ConvDicXMLRightTextContext_Impl::EndElement()
{
    ConvDic *pDic = GetDic();
    if (pDic)
        pDic->AddEntry( GetLeftText(), GetRightText() );
}


///////////////////////////////////////////////////////////////////////////

sal_Bool ConvDicXMLExport::Export()
{
    sal_Bool bRet = sal_False;

    uno::Reference< document::XExporter > xExporter( this );
    uno::Reference< document::XFilter > xFilter( xExporter, UNO_QUERY );
    uno::Sequence< beans::PropertyValue > aProps(0);
    xFilter->filter( aProps );      // calls exportDoc implicitly

    return bRet = bSuccess;
}


sal_uInt32 ConvDicXMLExport::exportDoc( enum ::xmloff::token::XMLTokenEnum /*eClass*/ )
{
    _GetNamespaceMap().Add( A2OU( "tcd" ),
            A2OU( XML_NAMESPACE_TCD_STRING ), XML_NAMESPACE_TCD );

    GetDocHandler()->startDocument();

    // Add xmlns line and some other arguments
    AddAttribute( _GetNamespaceMap().GetAttrNameByKey( XML_NAMESPACE_TCD ),
                  _GetNamespaceMap().GetNameByKey( XML_NAMESPACE_TCD ) );
    AddAttributeASCII( XML_NAMESPACE_TCD, "package", "org.openoffice.Office" );

    OUString aIsoLang( MsLangId::convertLanguageToIsoString( rDic.nLanguage ) );
    AddAttribute( XML_NAMESPACE_TCD, "lang", aIsoLang );
    OUString aConvType( ConversionTypeToText( rDic.nConversionType ) );
    AddAttribute( XML_NAMESPACE_TCD, "conversion-type", aConvType );

    //!! block necessary in order to have SvXMLElementExport d-tor called
    //!! before the call to endDocument
    {
        SvXMLElementExport aRoot( *this, XML_NAMESPACE_TCD, "text-conversion-dictionary", sal_True, sal_True );
        _ExportContent();
    }

    GetDocHandler()->endDocument();

    bSuccess = sal_True;
    return 0;
}


void ConvDicXMLExport::_ExportContent()
{
    // acquire sorted list of all keys
    ConvMapKeySet   aKeySet;
    ConvMap::iterator aIt;
    for (aIt = rDic.aFromLeft.begin();  aIt != rDic.aFromLeft.end();  ++aIt)
        aKeySet.insert( (*aIt).first );

    ConvMapKeySet::iterator aKeyIt;
    for (aKeyIt = aKeySet.begin();  aKeyIt != aKeySet.end();  ++aKeyIt)
    {
        OUString aLeftText( *aKeyIt );
        AddAttribute( XML_NAMESPACE_TCD, "left-text", aLeftText );
        if (rDic.pConvPropType.get())   // property-type list available?
        {
            sal_Int16 nPropertyType = -1;
            PropTypeMap::iterator aIt2 = rDic.pConvPropType->find( aLeftText );
            if (aIt2 != rDic.pConvPropType->end())
                nPropertyType = (*aIt2).second;
            DBG_ASSERT( nPropertyType, "property-type not found" );
            if (nPropertyType == -1)
                nPropertyType = ConversionPropertyType::NOT_DEFINED;
            AddAttribute( XML_NAMESPACE_TCD, "property-type", OUString::valueOf( (sal_Int32) nPropertyType ) );
        }
        SvXMLElementExport aEntryMain( *this, XML_NAMESPACE_TCD,
                "entry" , sal_True, sal_True );

        pair< ConvMap::iterator, ConvMap::iterator > aRange =
                rDic.aFromLeft.equal_range( *aKeyIt );
        for (aIt = aRange.first;  aIt != aRange.second;  ++aIt)
        {
            DBG_ASSERT( *aKeyIt == (*aIt).first, "key <-> entry mismatch" );
            OUString aRightText( (*aIt).second );
            SvXMLElementExport aEntryRightText( *this, XML_NAMESPACE_TCD,
                    "right-text" , sal_True, sal_False );
            Characters( aRightText );
        }
    }
}

::rtl::OUString SAL_CALL ConvDicXMLExport::getImplementationName()
    throw( uno::RuntimeException )
{
    return A2OU( "com.sun.star.lingu2.ConvDicXMLExport" );
}

///////////////////////////////////////////////////////////////////////////

void SAL_CALL ConvDicXMLImport::startDocument(void)
    throw( xml::sax::SAXException, uno::RuntimeException )
{
    // register namespace at first possible opportunity
    GetNamespaceMap().Add( A2OU( "tcd" ),
            A2OU( XML_NAMESPACE_TCD_STRING ), XML_NAMESPACE_TCD );
    SvXMLImport::startDocument();
}

void SAL_CALL ConvDicXMLImport::endDocument(void)
    throw( xml::sax::SAXException, uno::RuntimeException )
{
    SvXMLImport::endDocument();
}

SvXMLImportContext * ConvDicXMLImport::CreateContext(
        sal_uInt16 nPrefix,
        const rtl::OUString &rLocalName,
        const uno::Reference < xml::sax::XAttributeList > & /*rxAttrList*/ )
{
    SvXMLImportContext *pContext = 0;
    if (nPrefix == XML_NAMESPACE_TCD && rLocalName.equalsAscii( "text-conversion-dictionary" ))
        pContext = new ConvDicXMLDictionaryContext_Impl( *this, nPrefix, rLocalName );
    else
        pContext = new SvXMLImportContext( *this, nPrefix, rLocalName );
    return pContext;
}


OUString SAL_CALL ConvDicXMLImport::getImplementationName()
    throw( uno::RuntimeException )
{
    return A2OU( "com.sun.star.lingu2.ConvDicXMLImport" );
}

///////////////////////////////////////////////////////////////////////////

