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
 **************************************************************/

 // MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

//This file is about the conversion of the UOF v2.0 and ODF document format from CS2C 20120610.
#include "uof2merge.hxx"

#include <cppuhelper/implbase1.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

#include "XMLBase64Codec.hxx"

namespace XSLT{

const ::rtl::OUString UOF2ROOTELEM = ::rtl::OUString::createFromAscii("uof:UOF_0000");
const ::rtl::OUString UOF2OBJDATAXML = ::rtl::OUString::createFromAscii("objectdata.xml");
const ::rtl::OUString UOF2DATADIR = ::rtl::OUString::createFromAscii("data");

/************************************************************************/
/*  class UOF2AttributeList                                             */
/************************************************************************/

class UOF2AttributeList : public ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XAttributeList >
{
public:
    struct UOF2Attribute
    {
        ::rtl::OUString m_sName;
        ::rtl::OUString m_sValue;
        ::rtl::OUString m_sType;
        UOF2Attribute( const ::rtl::OUString& rName, const ::rtl::OUString& rValue, const ::rtl::OUString& rType)
        : m_sName(rName)
        , m_sValue(rValue)
        , m_sType(rType)
        {
        }
    };

    explicit UOF2AttributeList();
    virtual ~UOF2AttributeList();

    void addAttribute( const UOF2Attribute& rAttribute );

    virtual sal_Int16 SAL_CALL getLength() throw ( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getNameByIndex( sal_Int16 i) throw ( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getTypeByIndex( sal_Int16 i) throw ( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getTypeByName( const ::rtl::OUString& rName ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getValueByIndex( sal_Int16 i ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getValueByName( const ::rtl::OUString& rName ) throw ( ::com::sun::star::uno::RuntimeException );
private:
    ::std::vector< UOF2Attribute > m_aAttributes;
};

UOF2AttributeList::UOF2AttributeList()
{
}

UOF2AttributeList::~UOF2AttributeList()
{
}

void UOF2AttributeList::addAttribute( const UOF2Attribute& rAttribute )
{
    if(rAttribute.m_sName.getLength() && rAttribute.m_sValue.getLength())
        m_aAttributes.push_back(rAttribute);
}

sal_Int16 SAL_CALL UOF2AttributeList::getLength() throw ( ::com::sun::star::uno::RuntimeException )
{
    return static_cast< sal_Int16 >(m_aAttributes.size());
}

::rtl::OUString SAL_CALL UOF2AttributeList::getNameByIndex( sal_Int16 i ) throw ( ::com::sun::star::uno::RuntimeException )
{
    return m_aAttributes[i].m_sName;
}

::rtl::OUString SAL_CALL UOF2AttributeList::getTypeByIndex( sal_Int16 i ) throw ( ::com::sun::star::uno::RuntimeException )
{
    return m_aAttributes[i].m_sType;
}

::rtl::OUString SAL_CALL UOF2AttributeList::getTypeByName( const ::rtl::OUString& rName ) throw ( ::com::sun::star::uno::RuntimeException )
{
    ::std::vector< UOF2AttributeList::UOF2Attribute >::const_iterator aIter = m_aAttributes.begin();
    ::std::vector< UOF2AttributeList::UOF2Attribute >::const_iterator aEnd = m_aAttributes.end();
    while(aIter != aEnd)
    {
        if((*aIter).m_sName.equals(rName))
            return (*aIter).m_sType;
        ++aIter;
    }

    return ::rtl::OUString();
}

::rtl::OUString SAL_CALL UOF2AttributeList::getValueByIndex( sal_Int16 i ) throw ( ::com::sun::star::uno::RuntimeException )
{
    return m_aAttributes[i].m_sValue;
}

::rtl::OUString SAL_CALL UOF2AttributeList::getValueByName( const ::rtl::OUString& rName ) throw ( ::com::sun::star::uno::RuntimeException )
{
    ::std::vector< UOF2AttributeList::UOF2Attribute >::const_iterator aIter = m_aAttributes.begin();
    ::std::vector< UOF2AttributeList::UOF2Attribute >::const_iterator aEnd = m_aAttributes.end();
    while(aIter != aEnd)
    {
        if((*aIter).m_sName.equals(rName))
            return (*aIter).m_sValue;
        ++aIter;
    }

    return ::rtl::OUString();
}

/************************************************************************/
/* class UOF2FlatDocMergeHandler                                        */
/************************************************************************/

class UOF2FlatDocMergeHandler : public ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XDocumentHandler >
{
public:
    explicit UOF2FlatDocMergeHandler(UOF2Merge& rUOF2Merge);
    virtual ~UOF2FlatDocMergeHandler();

    virtual void SAL_CALL startDocument()
        throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL endDocument()
        throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL startElement( const ::rtl::OUString& rElemName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& rAttribs )
        throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL endElement( const ::rtl::OUString& rElemName )
        throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL characters( const ::rtl::OUString& rElemName )
        throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL ignorableWhitespace( const ::rtl::OUString& rWhiteSpaces )
        throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL processingInstruction( const ::rtl::OUString& rTarget, const ::rtl::OUString& rData )
        throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setDocumentLocator( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& xLocator )
        throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
protected:
    UOF2Merge& getUOF2Merge(){ return m_rUOF2Merge; }
private:
    UOF2Merge& m_rUOF2Merge;
    sal_Int32 m_nLevel;
private:
    UOF2FlatDocMergeHandler(const UOF2FlatDocMergeHandler& rDocHdl);
    UOF2FlatDocMergeHandler& operator=(const UOF2FlatDocMergeHandler& rDocHdl);
};

UOF2FlatDocMergeHandler::UOF2FlatDocMergeHandler( UOF2Merge& rUOF2Merge )
: m_rUOF2Merge(rUOF2Merge)
, m_nLevel(0)
{
}

UOF2FlatDocMergeHandler::~UOF2FlatDocMergeHandler()
{
}

void SAL_CALL UOF2FlatDocMergeHandler::startDocument()
    throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL UOF2FlatDocMergeHandler::endDocument()
    throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL UOF2FlatDocMergeHandler::startElement( const ::rtl::OUString& rElemName,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& rAttribs )
                            throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
    ++m_nLevel;
    if( m_nLevel == 1)
    {
        UOF2AttributeList *pUOF2AttrList = new UOF2AttributeList;
        sal_Int16 nLen = rAttribs->getLength();
        if(nLen > 0)
        {
            for( sal_Int16 i = 0; i < nLen; ++i)
            {
                bool bIsExistNMS = false;
                if((rAttribs->getNameByIndex(i).indexOf( ::rtl::OUString::createFromAscii("xmlns:"))) == 0)
                {
                    bIsExistNMS = m_rUOF2Merge.isInsertedNamespace(rAttribs->getNameByIndex(i));
                    if(!bIsExistNMS)
                        m_rUOF2Merge.addNamespace(rAttribs->getNameByIndex(i), rAttribs->getValueByIndex(i));
                }

                if(!bIsExistNMS)
                {
                    pUOF2AttrList->addAttribute(
                        UOF2AttributeList::UOF2Attribute( rAttribs->getNameByIndex(i), rAttribs->getValueByIndex(i), rAttribs->getTypeByIndex(i)) );
                }
            }
        }
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > xAttrList(pUOF2AttrList);
        m_rUOF2Merge.getSaxWriter()->startElement(rElemName, xAttrList);
    }
    else
        m_rUOF2Merge.getSaxWriter()->startElement(rElemName, rAttribs);
}

void SAL_CALL UOF2FlatDocMergeHandler::endElement( const ::rtl::OUString& rElemName )
    throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
    --m_nLevel;
    m_rUOF2Merge.getSaxWriter()->endElement(rElemName);
}

void SAL_CALL UOF2FlatDocMergeHandler::characters( const ::rtl::OUString& rElemName )
    throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
    m_rUOF2Merge.getSaxWriter()->characters(rElemName);
}

void SAL_CALL UOF2FlatDocMergeHandler::ignorableWhitespace( const ::rtl::OUString& /*rWhiteSpaces*/ )
    throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL UOF2FlatDocMergeHandler::processingInstruction( const ::rtl::OUString& /*rTarget*/, const ::rtl::OUString&/* rData */)
    throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL UOF2FlatDocMergeHandler::setDocumentLocator(
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& /*xLocator*/ )
    throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
}

/************************************************************************/
/* class UOF2UOFXMLDocMergeHandler                                      */
/************************************************************************/

class UOF2UOFXMLDocMergeHandler : public UOF2FlatDocMergeHandler
{
public:
    explicit UOF2UOFXMLDocMergeHandler( UOF2Merge& rUOF2Merge);
    virtual ~UOF2UOFXMLDocMergeHandler();

    virtual void SAL_CALL endElement( const ::rtl::OUString& rElemName )
        throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
};

UOF2UOFXMLDocMergeHandler::UOF2UOFXMLDocMergeHandler( UOF2Merge& rUOF2Merge )
: UOF2FlatDocMergeHandler(rUOF2Merge)
{
}

UOF2UOFXMLDocMergeHandler::~UOF2UOFXMLDocMergeHandler()
{
}

void SAL_CALL UOF2UOFXMLDocMergeHandler::endElement( const ::rtl::OUString& /*rElemName*/ )
    throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
}

/************************************************************************/
/* class UOF2ObjdataXMLDocMergeHandler                                  */
/************************************************************************/

class UOF2ObjdataXMLDocMergeHandler : public UOF2FlatDocMergeHandler
{
public:
    UOF2ObjdataXMLDocMergeHandler( UOF2Merge& rMerge );
    virtual ~UOF2ObjdataXMLDocMergeHandler();

    virtual void SAL_CALL startElement( const ::rtl::OUString& rElemName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& rAttribs )
        throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL endElement( const ::rtl::OUString& rElemName )
        throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL characters( const ::rtl::OUString& rChars )
        throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
private:
    static const ::rtl::OUString OBJPATH;
    static const ::rtl::OUString OBJDATA;

    bool m_bIsObjPathElem;
};

const ::rtl::OUString UOF2ObjdataXMLDocMergeHandler::OBJPATH( ::rtl::OStringToOUString( ::rtl::OString("对象:路径_D703"),  RTL_TEXTENCODING_UTF8 ) );
const ::rtl::OUString UOF2ObjdataXMLDocMergeHandler::OBJDATA( ::rtl::OStringToOUString( ::rtl::OString("对象:数据_D702"), RTL_TEXTENCODING_UTF8 ) );

UOF2ObjdataXMLDocMergeHandler::UOF2ObjdataXMLDocMergeHandler( UOF2Merge& rMerge )
: UOF2FlatDocMergeHandler(rMerge)
, m_bIsObjPathElem(false)
{
}

UOF2ObjdataXMLDocMergeHandler::~UOF2ObjdataXMLDocMergeHandler()
{
}

void SAL_CALL UOF2ObjdataXMLDocMergeHandler::startElement( const ::rtl::OUString& rElemName,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& rAttribs )
                    throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
    if(rElemName.equals(OBJPATH))
    {
        m_bIsObjPathElem = true;
        UOF2FlatDocMergeHandler::startElement(OBJDATA, rAttribs);
    }
    else
    {
        UOF2FlatDocMergeHandler::startElement(rElemName, rAttribs);
        m_bIsObjPathElem = false;
    }
}

void SAL_CALL UOF2ObjdataXMLDocMergeHandler::endElement( const ::rtl::OUString& rElemName )
    throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
    if(m_bIsObjPathElem)
        UOF2FlatDocMergeHandler::endElement(OBJDATA);
    else
        UOF2FlatDocMergeHandler::endElement(rElemName);

    m_bIsObjPathElem = false;
}

void SAL_CALL UOF2ObjdataXMLDocMergeHandler::characters( const ::rtl::OUString& rChars )
    throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
    if(m_bIsObjPathElem)
    {
        ::rtl::OUStringBuffer sBuffer;
        bool bHasBase64 = getUOF2Merge().getBase64Codec(sBuffer, rChars);
        if(bHasBase64)
            UOF2FlatDocMergeHandler::characters(sBuffer.makeStringAndClear());
    }
    else
        UOF2FlatDocMergeHandler::characters(rChars);
}

/************************************************************************/
/* class UOF2Merge                                                      */
/************************************************************************/

UOF2Merge::UOF2Merge( UOF2Storage& rStorage,
                     const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory )
: m_rUOF2Storage(rStorage)
, m_xServiceFactory(rxFactory)
{
    OSL_ENSURE(rxFactory.is(), "UOF2Merge::UOF2Merge need XMultiServiceFactory");
    OSL_ENSURE(rStorage.isValidUOF2Doc(), "UOF2Merge::UOF2Merge - You must import valid UOF2 document");
    init();
}

UOF2Merge::~UOF2Merge()
{
}

void UOF2Merge::init()
{
    try
    {
        m_xPipeInStream.set(m_xServiceFactory->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.io.Pipe") ),
            ::com::sun::star::uno::UNO_QUERY);
        m_xPipeOutStream.set(m_xPipeInStream, ::com::sun::star::uno::UNO_QUERY);

        m_xSaxParser.set(m_xServiceFactory->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.xml.sax.Parser") ),
            ::com::sun::star::uno::UNO_QUERY);

        m_xExtDocHdl.set(m_xServiceFactory->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.xml.sax.Writer") ),
            ::com::sun::star::uno::UNO_QUERY);

        ::com::sun::star::uno::Reference< ::com::sun::star::io::XActiveDataSource > xmlSource(
            m_xExtDocHdl, ::com::sun::star::uno::UNO_QUERY);
        xmlSource->setOutputStream(m_xPipeOutStream);
    }
    catch( ::com::sun::star::uno::Exception& exc)
    {
        OSL_ENSURE(0, ::rtl::OUStringToOString(exc.Message, RTL_TEXTENCODING_ASCII_US).getStr());
    }
}

bool UOF2Merge::merge()
{
    bool bRet = true;

    ::std::vector< ::rtl::OUString > aElemNames;
    StorageRef storageRef = m_rUOF2Storage.getMainStorageRef();
    storageRef->getElementNames(aElemNames);

    m_xExtDocHdl->startDocument();

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xUOFXMLInputStream =
        storageRef->openInputStream(UOFELEMNAME);
    startUOFRootXML(xUOFXMLInputStream);

    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XDocumentHandler > xUOF2SubXMLDocHdl( new UOF2FlatDocMergeHandler(*this) );
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > xObjdataXMLDocHdl;

    ::std::vector< ::rtl::OUString >::const_iterator aIter = aElemNames.begin();
    ::std::vector< ::rtl::OUString >::const_iterator aEndIt = aElemNames.end();
    while(aIter != aEndIt)
    {
        m_xSaxParser->setDocumentHandler(xUOF2SubXMLDocHdl);

        if((*aIter) != UOFELEMNAME)
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xInputStream = storageRef->openInputStream(*aIter);
            if(xInputStream.is())
            {
                if((*aIter) == UOF2OBJDATAXML)
                {
                    xObjdataXMLDocHdl.set( new UOF2ObjdataXMLDocMergeHandler(*this) );
                    m_xSaxParser->setDocumentHandler(xObjdataXMLDocHdl);
                }

                ::com::sun::star::xml::sax::InputSource inputSource;
                inputSource.sSystemId = *aIter;
                inputSource.aInputStream = xInputStream;
                m_xSaxParser->parseStream(inputSource);
            }
            else
            {
                StorageRef subStorage =
                    storageRef->openSubStorage(*aIter, false);
                if(subStorage.get())
                {
                    if((*aIter) != UOF2DATADIR)
                    {
                        ::std::vector< ::rtl::OUString > aSubElemNames;
                        subStorage->getElementNames(aSubElemNames);
                        if(!aSubElemNames.empty())
                        {
                            ::std::vector< ::rtl::OUString >::const_iterator aSubIter = aSubElemNames.begin();
                            ::std::vector< ::rtl::OUString >::const_iterator aSubEnd = aSubElemNames.end();
                            while(aSubIter != aSubEnd)
                            {
                                ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xSubInputStream =
                                    subStorage->openInputStream(*aSubIter);
                                if(xSubInputStream.is())
                                {
                                    ::com::sun::star::xml::sax::InputSource inputSource;
                                    inputSource.sSystemId = *aSubIter;
                                    inputSource.aInputStream = xSubInputStream;
                                    m_xSaxParser->parseStream(inputSource);
                                }
                                ++aSubIter;
                            }
                        }
                    }
                }
            }
        }
        ++aIter;
    }
    endUOFRootXML();
    m_xExtDocHdl->endDocument();

    return bRet;
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > UOF2Merge::getMergedInStream() const
{
    return m_xPipeInStream;
}

::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XExtendedDocumentHandler > UOF2Merge::getSaxWriter()
{
    return m_xExtDocHdl;
}

void UOF2Merge::startUOFRootXML( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xUOFXMLInStream )
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XDocumentHandler > xUOFXMLDocHdl(new UOF2UOFXMLDocMergeHandler(*this));
    m_xSaxParser->setDocumentHandler(xUOFXMLDocHdl);

    ::com::sun::star::xml::sax::InputSource inputSource;
    inputSource.sSystemId = UOFELEMNAME;
    inputSource.aInputStream = xUOFXMLInStream;
    m_xSaxParser->parseStream(inputSource);
}

void UOF2Merge::endUOFRootXML()
{
    m_xExtDocHdl->endElement( ::rtl::OUString::createFromAscii("uof:UOF_0000") );
}

void UOF2Merge::addNamespace( const ::rtl::OUString& rName, const ::rtl::OUString& rURL )
{
    if(rName.getLength()> 0 && rURL.getLength() > 0)
    {
        m_aNamespaceMap.insert( ::std::map< ::rtl::OUString, ::rtl::OUString >::value_type( rName, rURL ));
    }
}

bool UOF2Merge::isInsertedNamespace( const ::rtl::OUString& rName ) const
{
    bool bRet = false;
    typedef ::std::map< ::rtl::OUString, ::rtl::OUString >::const_iterator NMSIter;
    NMSIter aFoundIter = m_aNamespaceMap.find( rName );
    if(aFoundIter != m_aNamespaceMap.end())
        bRet = true;

    return bRet;
}

bool UOF2Merge::getBase64Codec( ::rtl::OUStringBuffer& rBuffer, const ::rtl::OUString& rObjPath )
{
    bool bRet = false;
    if(rObjPath.getLength())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xInputStream =
            m_rUOF2Storage.getMainStorageRef()->openInputStream(rObjPath);
        if(xInputStream.is())
        {
            sal_Int32 nMax = 512;
            ::com::sun::star::uno::Sequence< sal_Int8 > aOutSeq;
            sal_Int32 nRead = 0;
            while(true)
            {
                ::com::sun::star::uno::Sequence< sal_Int8 > aInSeq;
                nRead = xInputStream->readBytes(aInSeq, nMax);
                if(nRead)
                {
                    sal_Int32 nLen = aInSeq.getLength();
                    if(nLen)
                    {
                        sal_Int32 nOrigLen = aOutSeq.getLength();
                        aOutSeq.realloc(nOrigLen + nLen);
                        sal_Int8 * pArray = aOutSeq.getArray() + nOrigLen;
                        for(sal_Int32 i = 0; i < nLen; ++i)
                        {
                            *pArray++ = aInSeq[i];
                        }
                    }
                }
                else
                    break;
            }

            if(aOutSeq.getLength() > 0)
            {
                XMLBase64Codec::encodeBase64(rBuffer, aOutSeq);
                bRet = true;
            }
        }
    }

    return bRet;
}

}