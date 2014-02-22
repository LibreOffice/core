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

#include "webdavresponseparser.hxx"
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/seqstream.hxx>
#include <com/sun/star/ucb/LockEntry.hpp>
#include <com/sun/star/ucb/LockScope.hpp>
#include <com/sun/star/ucb/LockType.hpp>
#include <map>
#include <hash_map>



using namespace com::sun::star;




namespace
{
    enum WebDAVNamespace
    {
        WebDAVNamespace_unknown = 0,
        WebDAVNamespace_DAV,
        WebDAVNamespace_ucb_openoffice_org_dav_props,

        WebDAVNamespace_last
    };

    WebDAVNamespace StrToWebDAVNamespace(const OUString& rStr)
    {
        if(rStr == "DAV:")
        {
            return WebDAVNamespace_DAV;
        }
        else if(rStr == "http:
        {
            return WebDAVNamespace_ucb_openoffice_org_dav_props;
        }

        return WebDAVNamespace_unknown;
    }
} 




namespace
{
    enum WebDAVName
    {
        WebDAVName_unknown = 0,
        WebDAVName_multistatus,
        WebDAVName_response,
        WebDAVName_href,
        WebDAVName_propstat,
        WebDAVName_prop,
        WebDAVName_resourcetype,
        WebDAVName_collection,
        WebDAVName_getcontenttype,
        WebDAVName_supportedlock,
        WebDAVName_lockentry,
        WebDAVName_lockscope,
        WebDAVName_exclusive,
        WebDAVName_locktype,
        WebDAVName_write,
        WebDAVName_shared,
        WebDAVName_status,
        WebDAVName_getlastmodified,
        WebDAVName_creationdate,
        WebDAVName_getcontentlength,

        WebDAVName_last
    };

    WebDAVName StrToWebDAVName(const OUString& rStr)
    {
        typedef std::hash_map< OUString, WebDAVName, OUStringHash > WebDAVNameMapper;
        typedef std::pair< OUString, WebDAVName > WebDAVNameValueType;
        static WebDAVNameMapper aWebDAVNameMapperList;

        if(aWebDAVNameMapperList.empty())
        {
            aWebDAVNameMapperList.insert(WebDAVNameValueType(OUString("multistatus"), WebDAVName_multistatus));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(OUString("response"), WebDAVName_response));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(OUString("href"), WebDAVName_href));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(OUString("propstat"), WebDAVName_propstat));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(OUString("prop"), WebDAVName_prop));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(OUString("resourcetype"), WebDAVName_resourcetype));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(OUString("collection"), WebDAVName_collection));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(OUString("getcontenttype"), WebDAVName_getcontenttype));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(OUString("supportedlock"), WebDAVName_supportedlock));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(OUString("lockentry"), WebDAVName_lockentry));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(OUString("lockscope"), WebDAVName_lockscope));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(OUString("exclusive"), WebDAVName_exclusive));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(OUString("locktype"), WebDAVName_locktype));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(OUString("write"), WebDAVName_write));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(OUString("shared"), WebDAVName_shared));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(OUString("status"), WebDAVName_status));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(OUString("getlastmodified"), WebDAVName_getlastmodified));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(OUString("creationdate"), WebDAVName_creationdate));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(OUString("getcontentlength"), WebDAVName_getcontentlength));
        }

        const WebDAVNameMapper::const_iterator aResult(aWebDAVNameMapperList.find(rStr));

        if(aResult == aWebDAVNameMapperList.end())
        {
            return WebDAVName_unknown;
        }
        else
        {
            return aResult->second;
        }
    }
} 




namespace
{
    typedef std::map< OUString, OUString > NamespaceMap;
    typedef std::pair< const OUString, OUString > NamespaceValueType;

    class WebDAVContext
    {
    private:
        WebDAVContext*              mpParent;
        NamespaceMap                maNamespaceMap;
        OUString             maWhiteSpace;

        OUString             maNamespace;
        OUString             maName;

        WebDAVNamespace             maWebDAVNamespace;
        WebDAVName                  maWebDAVName;

        
        void parseForNamespaceTokens(const uno::Reference< xml::sax::XAttributeList >& xAttribs);
        OUString mapNamespaceToken(const OUString& rToken) const;
        void splitName(const OUString& rSource);

    public:
        WebDAVContext(WebDAVContext* pParent, const OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs);
        ~WebDAVContext();

        WebDAVContext* getParent() const { return mpParent; }
        OUString& getWhiteSpace() { return maWhiteSpace; }
        void setWhiteSpace(const OUString& rNew) { maWhiteSpace = rNew; }

        const OUString& getNamespace() const { return maNamespace; }
        const OUString& getName() const { return maName; }
        const WebDAVNamespace getWebDAVNamespace() const { return maWebDAVNamespace; }
        const WebDAVName getWebDAVName() const { return maWebDAVName; }
    };

    void WebDAVContext::parseForNamespaceTokens(const uno::Reference< xml::sax::XAttributeList >& xAttribs)
    {
        const sal_Int16 nAttributes(xAttribs->getLength());

        for(sal_Int16 a(0); a < nAttributes; a++)
        {
            const OUString aName(xAttribs->getNameByIndex(a));
            const sal_Int32 nLen(aName.getLength());

            if(nLen)
            {
                if(aName.startsWith("xmlns"))
                {
                    const sal_Int32 nIndex(aName.indexOf(':', 0));

                    if(-1 != nIndex && nIndex + 1 < nLen)
                    {
                        const OUString aToken(aName.copy(nIndex + 1));

                        maNamespaceMap.insert(NamespaceValueType(aToken, xAttribs->getValueByIndex(a)));
                    }
                }
            }
        }
    }

    OUString WebDAVContext::mapNamespaceToken(const OUString& rToken) const
    {
        NamespaceMap::const_iterator iter = maNamespaceMap.find(rToken);

        if(maNamespaceMap.end() == iter)
        {
            if(getParent())
            {
                return getParent()->mapNamespaceToken(rToken);
            }
            else
            {
                return rToken;
            }
        }
        else
        {
            return (*iter).second;
        }
    }

    void WebDAVContext::splitName(const OUString& rSource)
    {
        const sal_Int32 nLen(rSource.getLength());
        maNamespace = "";
        maName = rSource;

        if(nLen)
        {
            const sal_Int32 nIndex(rSource.indexOf(':', 0));

            if(nIndex > 0 && ((nIndex + 1) < nLen))
            {
                maNamespace = mapNamespaceToken(rSource.copy(0, nIndex));
                maName = rSource.copy(nIndex + 1);
            }
        }
    }

    WebDAVContext::WebDAVContext(WebDAVContext* pParent, const OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs)
    :   mpParent(pParent),
        maNamespaceMap(),
        maWhiteSpace(),
        maNamespace(),
        maName(),
        maWebDAVNamespace(WebDAVNamespace_unknown),
        maWebDAVName(WebDAVName_unknown)
    {
        const sal_Int16 nAttributes(xAttribs->getLength());

        if(nAttributes)
        {
            
            parseForNamespaceTokens(xAttribs);
        }

        
        splitName(aName);

        
        maWebDAVNamespace = StrToWebDAVNamespace(maNamespace);
        maWebDAVName = StrToWebDAVName(maName);
    }

    WebDAVContext::~WebDAVContext()
    {
    }
} 




namespace
{
    enum WebDAVResponseParserMode
    {
        WebDAVResponseParserMode_PropFind = 0,
        WebDAVResponseParserMode_PropName
    };

    class WebDAVResponseParser : public cppu::WeakImplHelper1< com::sun::star::xml::sax::XDocumentHandler >
    {
    private:
        std::vector< http_dav_ucp::DAVResource >      maResult_PropFind;
        std::vector< http_dav_ucp::DAVResourceInfo >  maResult_PropName;

        WebDAVContext*                              mpContext;
        OUString                             maHref;
        OUString                             maStatus;
        std::vector< http_dav_ucp::DAVPropertyValue > maResponseProperties;
        std::vector< http_dav_ucp::DAVPropertyValue > maPropStatProperties;
        std::vector< OUString >              maResponseNames;
        std::vector< OUString >              maPropStatNames;
        uno::Sequence< ucb::LockEntry >             maLockEntries;
        ucb::LockScope                              maLockScope;
        ucb::LockType                               maLockType;
        WebDAVResponseParserMode                    meWebDAVResponseParserMode;

        
        bool                                        mbResourceTypeCollection : 1;
        bool                                        mbLockScopeSet : 1;
        bool                                        mbLockTypeSet : 1;

        
        bool whitespaceIsAvailable() const
        {
            return mpContext && mpContext->getWhiteSpace().getLength();
        }
        bool hasParent(WebDAVName aWebDAVName) const
        {
            return mpContext && mpContext->getParent() && aWebDAVName == mpContext->getParent()->getWebDAVName();
        }
        bool propertyIsReady() const
        {
            return hasParent(WebDAVName_prop) && whitespaceIsAvailable();
        }
        bool isCollectingProperties() const
        {
            return WebDAVResponseParserMode_PropFind == meWebDAVResponseParserMode;
        }
        bool isCollectingPropNames() const
        {
            return WebDAVResponseParserMode_PropName == meWebDAVResponseParserMode;
        }
        bool collectThisPropertyAsName() const
        {
            return isCollectingPropNames() && hasParent(WebDAVName_prop);
        }
        void pop_context()
        {
            if(mpContext)
            {
                WebDAVContext* pTemp = mpContext;
                mpContext = mpContext->getParent();
                delete pTemp;
            }
            else
            {
                OSL_ENSURE(false, "Parser context pop without context (!)");
            }
        }

    public:
        WebDAVResponseParser(WebDAVResponseParserMode eWebDAVResponseParserMode);
        ~WebDAVResponseParser();

        
        virtual void SAL_CALL startDocument(  ) throw (xml::sax::SAXException, uno::RuntimeException);
        virtual void SAL_CALL endDocument(  ) throw (xml::sax::SAXException, uno::RuntimeException);
        virtual void SAL_CALL startElement( const OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs ) throw (xml::sax::SAXException, uno::RuntimeException);
        virtual void SAL_CALL endElement( const OUString& aName ) throw (xml::sax::SAXException, uno::RuntimeException);
        virtual void SAL_CALL characters( const OUString& aChars ) throw (xml::sax::SAXException, uno::RuntimeException);
        virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces ) throw (xml::sax::SAXException, uno::RuntimeException);
        virtual void SAL_CALL processingInstruction( const OUString& aTarget, const OUString& aData ) throw (xml::sax::SAXException, uno::RuntimeException);
        virtual void SAL_CALL setDocumentLocator( const uno::Reference< xml::sax::XLocator >& xLocator ) throw (xml::sax::SAXException, uno::RuntimeException);

        const std::vector< http_dav_ucp::DAVResource >& getResult_PropFind() const { return maResult_PropFind; }
        const std::vector< http_dav_ucp::DAVResourceInfo >& getResult_PropName() const { return maResult_PropName; }
    };

    WebDAVResponseParser::WebDAVResponseParser(WebDAVResponseParserMode eWebDAVResponseParserMode)
    :   maResult_PropFind(),
        maResult_PropName(),
        mpContext(0),
        maHref(),
        maStatus(),
        maResponseProperties(),
        maPropStatProperties(),
        maResponseNames(),
        maPropStatNames(),
        maLockEntries(),
        maLockScope(ucb::LockScope_EXCLUSIVE),
        maLockType(ucb::LockType_WRITE),
        meWebDAVResponseParserMode(eWebDAVResponseParserMode),
        mbResourceTypeCollection(false),
        mbLockScopeSet(false),
        mbLockTypeSet(false)
    {
    }

    WebDAVResponseParser::~WebDAVResponseParser()
    {
        OSL_ENSURE(!mpContext, "Parser destructed with existing content (!)");
        while(mpContext)
        {
            pop_context();
        }
    }

    void SAL_CALL WebDAVResponseParser::startDocument(  ) throw (xml::sax::SAXException, uno::RuntimeException)
    {
        OSL_ENSURE(!mpContext, "Parser start with existing content (!)");
    }

    void SAL_CALL WebDAVResponseParser::endDocument(  ) throw (xml::sax::SAXException, uno::RuntimeException)
    {
        OSL_ENSURE(!mpContext, "Parser end with existing content (!)");
    }

    void SAL_CALL WebDAVResponseParser::startElement( const OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs ) throw (xml::sax::SAXException, uno::RuntimeException)
    {
        const sal_Int32 nLen(aName.getLength());

        if(nLen)
        {
            
            mpContext = new WebDAVContext(mpContext, aName, xAttribs);

            if(collectThisPropertyAsName())
            {
                
                
            }
            else
            {
                switch(mpContext->getWebDAVNamespace())
                {
                    default: 
                    {
                        break;
                    }
                    case WebDAVNamespace_DAV:
                    {
                        switch(mpContext->getWebDAVName())
                        {
                            default: 
                            {
                                break;
                            }
                            case WebDAVName_propstat:
                            {
                                
                                if(isCollectingProperties())
                                {
                                    
                                    maPropStatProperties.clear();
                                }
                                else
                                {
                                    
                                    maPropStatNames.clear();
                                }
                                break;
                            }
                            case WebDAVName_response:
                            {
                                
                                maHref = maStatus = "";

                                if(isCollectingProperties())
                                {
                                    
                                    maResponseProperties.clear();
                                }
                                else
                                {
                                    
                                    maResponseNames.clear();
                                }
                                break;
                            }
                            case WebDAVName_resourcetype:
                            {
                                
                                mbResourceTypeCollection = false;
                                break;
                            }
                            case WebDAVName_supportedlock:
                            {
                                
                                maLockEntries.realloc(0);
                                break;
                            }
                            case WebDAVName_lockentry:
                            {
                                
                                mbLockScopeSet = false;
                                mbLockTypeSet = false;
                                break;
                            }
                        }
                        break;
                    }
                    case WebDAVNamespace_ucb_openoffice_org_dav_props:
                    {
                        break;
                    }
                }
            }
        }
    }

    void SAL_CALL WebDAVResponseParser::endElement( const OUString& aName ) throw (xml::sax::SAXException, uno::RuntimeException)
    {
        const sal_Int32 nLen(aName.getLength());
        OSL_ENSURE(mpContext, "Parser EndElement without content (!)");

        if(mpContext && nLen)
        {
            if(collectThisPropertyAsName())
            {
                
                
                maPropStatNames.push_back(mpContext->getNamespace() + mpContext->getName());
            }
            else
            {
                switch(mpContext->getWebDAVNamespace())
                {
                    default: 
                    {
                        break;
                    }
                    case WebDAVNamespace_DAV:
                    {
                        switch(mpContext->getWebDAVName())
                        {
                            default: 
                            {
                                break;
                            }
                            case WebDAVName_href:
                            {
                                
                                if(whitespaceIsAvailable())
                                {
                                    maHref = mpContext->getWhiteSpace();
                                }
                                break;
                            }
                            case WebDAVName_status:
                            {
                                
                                if(whitespaceIsAvailable())
                                {
                                    maStatus = mpContext->getWhiteSpace();
                                }
                                break;
                            }
                            case WebDAVName_getlastmodified:
                            {
                                
                                if(propertyIsReady())
                                {
                                    http_dav_ucp::DAVPropertyValue aDAVPropertyValue;

                                    aDAVPropertyValue.Name = "DAV:getlastmodified";
                                    aDAVPropertyValue.Value <<= mpContext->getWhiteSpace();
                                    maPropStatProperties.push_back(aDAVPropertyValue);
                                }
                                break;
                            }
                            case WebDAVName_creationdate:
                            {
                                
                                if(propertyIsReady())
                                {
                                    http_dav_ucp::DAVPropertyValue aDAVPropertyValue;

                                    aDAVPropertyValue.Name = "DAV:creationdate";
                                    aDAVPropertyValue.Value <<= mpContext->getWhiteSpace();
                                    maPropStatProperties.push_back(aDAVPropertyValue);
                                }
                                break;
                            }
                            case WebDAVName_collection:
                            {
                                
                                if(hasParent(WebDAVName_resourcetype))
                                {
                                    mbResourceTypeCollection = true;
                                }
                                break;
                            }
                            case WebDAVName_resourcetype:
                            {
                                
                                if(hasParent(WebDAVName_prop))
                                {
                                    http_dav_ucp::DAVPropertyValue aDAVPropertyValue;

                                    aDAVPropertyValue.Name = "DAV:resourcetype";
                                    aDAVPropertyValue.Value <<= (mbResourceTypeCollection ? OUString("collection") : OUString());
                                    maPropStatProperties.push_back(aDAVPropertyValue);
                                }
                                break;
                            }
                            case WebDAVName_getcontentlength:
                            {
                                
                                if(propertyIsReady())
                                {
                                    http_dav_ucp::DAVPropertyValue aDAVPropertyValue;

                                    aDAVPropertyValue.Name = "DAV:getcontentlength";
                                    aDAVPropertyValue.Value <<= mpContext->getWhiteSpace();
                                    maPropStatProperties.push_back(aDAVPropertyValue);
                                }
                                break;
                            }
                            case WebDAVName_getcontenttype:
                            {
                                
                                if(propertyIsReady())
                                {
                                    http_dav_ucp::DAVPropertyValue aDAVPropertyValue;

                                    aDAVPropertyValue.Name = "DAV:getcontenttype";
                                    aDAVPropertyValue.Value <<= mpContext->getWhiteSpace();
                                    maPropStatProperties.push_back(aDAVPropertyValue);
                                }
                                break;
                            }
                            case WebDAVName_supportedlock:
                            {
                                
                                if(hasParent(WebDAVName_prop) && maLockEntries.hasElements())
                                {
                                    http_dav_ucp::DAVPropertyValue aDAVPropertyValue;

                                    aDAVPropertyValue.Name = "DAV:supportedlock";
                                    aDAVPropertyValue.Value <<= maLockEntries;
                                    maPropStatProperties.push_back(aDAVPropertyValue);
                                }
                                break;
                            }
                            case WebDAVName_lockentry:
                            {
                                
                                if(hasParent(WebDAVName_supportedlock) && (mbLockScopeSet && mbLockTypeSet))
                                {
                                    const sal_Int32 nLength(maLockEntries.getLength());
                                    ucb::LockEntry aEntry;

                                    aEntry.Scope = maLockScope;
                                    aEntry.Type = maLockType;
                                    maLockEntries.realloc(nLength + 1);
                                    maLockEntries[nLength] = aEntry;
                                }
                                break;
                            }
                            case WebDAVName_exclusive:
                            {
                                
                                if(hasParent(WebDAVName_lockscope))
                                {
                                    maLockScope = ucb::LockScope_EXCLUSIVE;
                                    mbLockScopeSet = true;
                                }
                                break;
                            }
                            case WebDAVName_shared:
                            {
                                
                                if(hasParent(WebDAVName_lockscope))
                                {
                                    maLockScope = ucb::LockScope_SHARED;
                                    mbLockScopeSet = true;
                                }
                                break;
                            }
                            case WebDAVName_write:
                            {
                                
                                if(hasParent(WebDAVName_locktype))
                                {
                                    maLockType = ucb::LockType_WRITE;
                                    mbLockTypeSet = true;
                                }
                                break;
                            }
                            case WebDAVName_propstat:
                            {
                                
                                if(maStatus.getLength())
                                {
                                    if(maStatus == "HTTP/1.1 200 OK")
                                    {
                                        if(isCollectingProperties())
                                        {
                                            if(!maPropStatProperties.empty())
                                            {
                                                
                                                maResponseProperties.insert(maResponseProperties.end(), maPropStatProperties.begin(), maPropStatProperties.end());
                                            }
                                        }
                                        else
                                        {
                                            if(!maPropStatNames.empty())
                                            {
                                                
                                                maResponseNames.insert(maResponseNames.end(), maPropStatNames.begin(), maPropStatNames.end());
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                            case WebDAVName_response:
                            {
                                
                                if(maHref.getLength())
                                {
                                    if(isCollectingProperties())
                                    {
                                        
                                        if(!maResponseProperties.empty())
                                        {
                                            http_dav_ucp::DAVResource aDAVResource;

                                            aDAVResource.uri = maHref;
                                            aDAVResource.properties = maResponseProperties;
                                            maResult_PropFind.push_back(aDAVResource);
                                        }
                                    }
                                    else
                                    {
                                        
                                        if(!maResponseNames.empty())
                                        {
                                            http_dav_ucp::DAVResourceInfo aDAVResourceInfo(maHref);

                                            aDAVResourceInfo.properties = maResponseNames;
                                            maResult_PropName.push_back(aDAVResourceInfo);
                                        }
                                    }
                                }
                                break;
                            }
                        }
                        break;
                    }
                    case WebDAVNamespace_ucb_openoffice_org_dav_props:
                    {
                        break;
                    }
                }
            }

            
            pop_context();
        }
    }

    void SAL_CALL WebDAVResponseParser::characters( const OUString& aChars ) throw (xml::sax::SAXException, uno::RuntimeException)
    {
        
        OSL_ENSURE(mpContext, "Parser characters without content (!)");
        const sal_Int32 nLen(aChars.getLength());

        if(mpContext && nLen)
        {
            
            const OUString aTrimmedChars(aChars.trim());

            if(aTrimmedChars.getLength())
            {
                OUString aNew(mpContext->getWhiteSpace());

                if(aNew.getLength())
                {
                    
                    aNew += OUString(' ');
                }

                aNew += aTrimmedChars;
                mpContext->setWhiteSpace(aNew);
            }
        }
    }

    void SAL_CALL WebDAVResponseParser::ignorableWhitespace( const OUString& /*aWhitespaces*/ ) throw (xml::sax::SAXException, uno::RuntimeException)
    {
    }

    void SAL_CALL WebDAVResponseParser::processingInstruction( const OUString& /*aTarget*/, const OUString& /*aData*/ ) throw (xml::sax::SAXException, uno::RuntimeException)
    {
    }

    void SAL_CALL WebDAVResponseParser::setDocumentLocator( const uno::Reference< xml::sax::XLocator >& /*xLocator*/ ) throw (xml::sax::SAXException, uno::RuntimeException)
    {
    }
} 




namespace
{
    void parseWebDAVPropNameResponse(
        const uno::Reference< io::XInputStream >& xInputStream,
        std::vector< http_dav_ucp::DAVResource >& rPropFind,
        std::vector< http_dav_ucp::DAVResourceInfo >& rPropName,
        WebDAVResponseParserMode eWebDAVResponseParserMode)
    {
        if(xInputStream.is())
        {
            try
            {
                
                xml::sax::InputSource myInputSource;
                myInputSource.aInputStream = xInputStream;

                
                uno::Reference< xml::sax::XParser > xParser = xml::sax::Parser::create(
                    comphelper::getProcessComponentContext() );

                
                WebDAVResponseParser* pWebDAVResponseParser = new WebDAVResponseParser(eWebDAVResponseParserMode);
                uno::Reference< xml::sax::XDocumentHandler > xWebDAVHdl(pWebDAVResponseParser);
                xParser->setDocumentHandler(xWebDAVHdl);

                
                xParser->parseStream(myInputSource);

                
                switch(eWebDAVResponseParserMode)
                {
                    case WebDAVResponseParserMode_PropFind:
                    {
                        rPropFind = pWebDAVResponseParser->getResult_PropFind();
                        break;
                    }
                    case WebDAVResponseParserMode_PropName:
                    {
                        rPropName = pWebDAVResponseParser->getResult_PropName();
                        break;
                    }
                }
            }
            catch(uno::Exception&)
            {
                OSL_ENSURE(false, "WebDAV Parse error (!)");
            }
        }
    }
} 




namespace http_dav_ucp
{
    std::vector< DAVResource > parseWebDAVPropFindResponse(const uno::Reference< io::XInputStream >& xInputStream)
    {
        std::vector< DAVResource > aRetval;
        std::vector< DAVResourceInfo > aFoo;

        parseWebDAVPropNameResponse(xInputStream, aRetval, aFoo, WebDAVResponseParserMode_PropFind);
        return aRetval;
    }

    std::vector< DAVResourceInfo > parseWebDAVPropNameResponse(const uno::Reference< io::XInputStream >& xInputStream)
    {
        std::vector< DAVResource > aFoo;
        std::vector< DAVResourceInfo > aRetval;

        parseWebDAVPropNameResponse(xInputStream, aFoo, aRetval, WebDAVResponseParserMode_PropName);
        return aRetval;
    }
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
