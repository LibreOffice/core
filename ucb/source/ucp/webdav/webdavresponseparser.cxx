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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#include <webdavresponseparser.hxx>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/seqstream.hxx>
#include <com/sun/star/ucb/LockEntry.hpp>
#include <com/sun/star/ucb/LockScope.hpp>
#include <com/sun/star/ucb/LockType.hpp>
#include <map>
#include <hash_map>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////
// WebDAVNamespace enum and StringToEnum converter

namespace
{
    enum WebDAVNamespace
    {
        WebDAVNamespace_unknown = 0,
        WebDAVNamespace_DAV,
        WebDAVNamespace_ucb_openoffice_org_dav_props,

        WebDAVNamespace_last
    };

    WebDAVNamespace StrToWebDAVNamespace(const rtl::OUString& rStr)
    {
        static ::rtl::OUString aStrDAV(::rtl::OUString::createFromAscii("DAV:"));
        static ::rtl::OUString aStrUcbOpenofficeOrgDAVProps(::rtl::OUString::createFromAscii("http://ucb.openoffice.org/dav/props/"));

        if(rStr.equals(aStrDAV))
        {
            return WebDAVNamespace_DAV;
        }
        else if(rStr.equals(aStrUcbOpenofficeOrgDAVProps))
        {
            return WebDAVNamespace_ucb_openoffice_org_dav_props;
        }

        return WebDAVNamespace_unknown;
    }
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////
// WebDAVName enum and StringToEnum converter using hash_map

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

    WebDAVName StrToWebDAVName(const rtl::OUString& rStr)
    {
        typedef std::hash_map< rtl::OUString, WebDAVName, rtl::OUStringHash > WebDAVNameMapper;
        typedef std::pair< rtl::OUString, WebDAVName > WebDAVNameValueType;
        static WebDAVNameMapper aWebDAVNameMapperList;

        if(aWebDAVNameMapperList.empty())
        {
            aWebDAVNameMapperList.insert(WebDAVNameValueType(rtl::OUString::createFromAscii("multistatus"), WebDAVName_multistatus));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(rtl::OUString::createFromAscii("response"), WebDAVName_response));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(rtl::OUString::createFromAscii("href"), WebDAVName_href));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(rtl::OUString::createFromAscii("propstat"), WebDAVName_propstat));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(rtl::OUString::createFromAscii("prop"), WebDAVName_prop));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(rtl::OUString::createFromAscii("resourcetype"), WebDAVName_resourcetype));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(rtl::OUString::createFromAscii("collection"), WebDAVName_collection));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(rtl::OUString::createFromAscii("getcontenttype"), WebDAVName_getcontenttype));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(rtl::OUString::createFromAscii("supportedlock"), WebDAVName_supportedlock));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(rtl::OUString::createFromAscii("lockentry"), WebDAVName_lockentry));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(rtl::OUString::createFromAscii("lockscope"), WebDAVName_lockscope));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(rtl::OUString::createFromAscii("exclusive"), WebDAVName_exclusive));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(rtl::OUString::createFromAscii("locktype"), WebDAVName_locktype));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(rtl::OUString::createFromAscii("write"), WebDAVName_write));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(rtl::OUString::createFromAscii("shared"), WebDAVName_shared));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(rtl::OUString::createFromAscii("status"), WebDAVName_status));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(rtl::OUString::createFromAscii("getlastmodified"), WebDAVName_getlastmodified));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(rtl::OUString::createFromAscii("creationdate"), WebDAVName_creationdate));
            aWebDAVNameMapperList.insert(WebDAVNameValueType(rtl::OUString::createFromAscii("getcontentlength"), WebDAVName_getcontentlength));
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
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////
// WebDAVContext, holding information for each start/endElement pair

namespace
{
    typedef std::map< ::rtl::OUString, ::rtl::OUString > NamespaceMap;
    typedef std::pair< const ::rtl::OUString, ::rtl::OUString > NamespaceValueType;

    class WebDAVContext
    {
    private:
        WebDAVContext*              mpParent;
        NamespaceMap                maNamespaceMap;
        ::rtl::OUString             maWhiteSpace;

        ::rtl::OUString             maNamespace;
        ::rtl::OUString             maName;

        WebDAVNamespace             maWebDAVNamespace;
        WebDAVName                  maWebDAVName;

        // local helpers
        void parseForNamespaceTokens(const uno::Reference< xml::sax::XAttributeList >& xAttribs);
        ::rtl::OUString mapNamespaceToken(const ::rtl::OUString& rToken) const;
        void splitName(const ::rtl::OUString& rSource);

    public:
        WebDAVContext(WebDAVContext* pParent, const ::rtl::OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs);
        ~WebDAVContext();

        WebDAVContext* getParent() const { return mpParent; }
        ::rtl::OUString& getWhiteSpace() { return maWhiteSpace; }
        void setWhiteSpace(const ::rtl::OUString& rNew) { maWhiteSpace = rNew; }

        const ::rtl::OUString& getNamespace() const { return maNamespace; }
        const ::rtl::OUString& getName() const { return maName; }
        const WebDAVNamespace getWebDAVNamespace() const { return maWebDAVNamespace; }
        const WebDAVName getWebDAVName() const { return maWebDAVName; }
    };

    void WebDAVContext::parseForNamespaceTokens(const uno::Reference< xml::sax::XAttributeList >& xAttribs)
    {
        const sal_Int16 nAttributes(xAttribs->getLength());
        static ::rtl::OUString aStrXmlns(::rtl::OUString::createFromAscii("xmlns"));

        for(sal_Int16 a(0); a < nAttributes; a++)
        {
            const ::rtl::OUString aName(xAttribs->getNameByIndex(a));
            const sal_Int32 nLen(aName.getLength());

            if(nLen)
            {
                if(aName.match(aStrXmlns, 0))
                {
                    const sal_Int32 nIndex(aName.indexOf(sal_Unicode(':'), 0));

                    if(-1 != nIndex && nIndex + 1 < nLen)
                    {
                        const ::rtl::OUString aToken(aName.copy(nIndex + 1));

                        maNamespaceMap.insert(NamespaceValueType(aToken, xAttribs->getValueByIndex(a)));
                    }
                }
            }
        }
    }

    ::rtl::OUString WebDAVContext::mapNamespaceToken(const ::rtl::OUString& rToken) const
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

    void WebDAVContext::splitName(const ::rtl::OUString& rSource)
    {
        const sal_Int32 nLen(rSource.getLength());
        maNamespace = ::rtl::OUString();
        maName = rSource;

        if(nLen)
        {
            const sal_Int32 nIndex(rSource.indexOf(sal_Unicode(':'), 0));

            if(-1 != nIndex && nIndex > 0 && nIndex + 1 < nLen)
            {
                maNamespace = mapNamespaceToken(rSource.copy(0, nIndex));
                maName = rSource.copy(nIndex + 1);
            }
        }
    }

    WebDAVContext::WebDAVContext(WebDAVContext* pParent, const ::rtl::OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs)
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
            // parse evtl. namespace entries
            parseForNamespaceTokens(xAttribs);
        }

        // split name to namespace and name
        splitName(aName);

        // evaluate enums for namespace and name
        maWebDAVNamespace = StrToWebDAVNamespace(maNamespace);
        maWebDAVName = StrToWebDAVName(maName);
    }

    WebDAVContext::~WebDAVContext()
    {
    }
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////
// the Xml parser itself

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
        ::rtl::OUString                             maHref;
        ::rtl::OUString                             maStatus;
        std::vector< http_dav_ucp::DAVPropertyValue > maResponseProperties;
        std::vector< http_dav_ucp::DAVPropertyValue > maPropStatProperties;
        std::vector< ::rtl::OUString >              maResponseNames;
        std::vector< ::rtl::OUString >              maPropStatNames;
        uno::Sequence< ucb::LockEntry >             maLockEntries;
        ucb::LockScope                              maLockScope;
        ucb::LockType                               maLockType;
        WebDAVResponseParserMode                    meWebDAVResponseParserMode;

        // bitfield
        bool                                        mbResourceTypeCollection : 1;
        bool                                        mbLockScopeSet : 1;
        bool                                        mbLockTypeSet : 1;

        // local helpers
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

        // Methods XDocumentHandler
        virtual void SAL_CALL startDocument(  ) throw (xml::sax::SAXException, uno::RuntimeException);
        virtual void SAL_CALL endDocument(  ) throw (xml::sax::SAXException, uno::RuntimeException);
        virtual void SAL_CALL startElement( const ::rtl::OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs ) throw (xml::sax::SAXException, uno::RuntimeException);
        virtual void SAL_CALL endElement( const ::rtl::OUString& aName ) throw (xml::sax::SAXException, uno::RuntimeException);
        virtual void SAL_CALL characters( const ::rtl::OUString& aChars ) throw (xml::sax::SAXException, uno::RuntimeException);
        virtual void SAL_CALL ignorableWhitespace( const ::rtl::OUString& aWhitespaces ) throw (xml::sax::SAXException, uno::RuntimeException);
        virtual void SAL_CALL processingInstruction( const ::rtl::OUString& aTarget, const ::rtl::OUString& aData ) throw (xml::sax::SAXException, uno::RuntimeException);
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

    void SAL_CALL WebDAVResponseParser::startElement( const ::rtl::OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs ) throw (xml::sax::SAXException, uno::RuntimeException)
    {
        const sal_Int32 nLen(aName.getLength());

        if(nLen)
        {
            // create new context (push)
            mpContext = new WebDAVContext(mpContext, aName, xAttribs);

            if(collectThisPropertyAsName())
            {
                // When collecting property names and parent is prop there is no need
                // to handle the content of this property deeper (evtl. preparations)
            }
            else
            {
                switch(mpContext->getWebDAVNamespace())
                {
                    default: // WebDAVNamespace_unknown, WebDAVNamespace_last or unhandled
                    {
                        break;
                    }
                    case WebDAVNamespace_DAV:
                    {
                        switch(mpContext->getWebDAVName())
                        {
                            default: // WebDAVName_unknown, WebDAVName_last or unhandled
                            {
                                break;
                            }
                            case WebDAVName_propstat:
                            {
                                // propstat start
                                if(isCollectingProperties())
                                {
                                    // reset maPropStatProperties
                                    maPropStatProperties.clear();
                                }
                                else
                                {
                                    // when collecting properties reset maPropStatNames
                                    maPropStatNames.clear();
                                }
                                break;
                            }
                            case WebDAVName_response:
                            {
                                // response start, reset Href and status and maResponseProperties
                                maHref = maStatus = ::rtl::OUString();

                                if(isCollectingProperties())
                                {
                                    // reset maResponseProperties
                                    maResponseProperties.clear();
                                }
                                else
                                {
                                    // reset maResponseNames when collecting properties
                                    maResponseNames.clear();
                                }
                                break;
                            }
                            case WebDAVName_resourcetype:
                            {
                                // resourcetype start, reset collection
                                mbResourceTypeCollection = false;
                                break;
                            }
                            case WebDAVName_supportedlock:
                            {
                                // supportedlock start, reset maLockEntries
                                maLockEntries.realloc(0);
                                break;
                            }
                            case WebDAVName_lockentry:
                            {
                                // lockentry start, reset maLockEntries
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

    void SAL_CALL WebDAVResponseParser::endElement( const ::rtl::OUString& aName ) throw (xml::sax::SAXException, uno::RuntimeException)
    {
        const sal_Int32 nLen(aName.getLength());
        OSL_ENSURE(mpContext, "Parser EndElement without content (!)");

        if(mpContext && nLen)
        {
            if(collectThisPropertyAsName())
            {
                // When collecting property names and parent is prop, just append the prop name
                // to the collection, no need to parse deeper
                maPropStatNames.push_back(mpContext->getNamespace() + mpContext->getName());
            }
            else
            {
                switch(mpContext->getWebDAVNamespace())
                {
                    default: // WebDAVNamespace_unknown, WebDAVNamespace_last or unhandled
                    {
                        break;
                    }
                    case WebDAVNamespace_DAV:
                    {
                        switch(mpContext->getWebDAVName())
                        {
                            default: // WebDAVName_unknown, WebDAVName_last or unhandled
                            {
                                break;
                            }
                            case WebDAVName_href:
                            {
                                // href end, save it if we have whitespace
                                if(whitespaceIsAvailable())
                                {
                                    maHref = mpContext->getWhiteSpace();
                                }
                                break;
                            }
                            case WebDAVName_status:
                            {
                                // status end, save it if we have whitespace
                                if(whitespaceIsAvailable())
                                {
                                    maStatus = mpContext->getWhiteSpace();
                                }
                                break;
                            }
                            case WebDAVName_getlastmodified:
                            {
                                // getlastmodified end, safe if content is correct
                                if(propertyIsReady())
                                {
                                    static rtl::OUString aStr(rtl::OUString::createFromAscii("DAV:getlastmodified"));
                                    http_dav_ucp::DAVPropertyValue aDAVPropertyValue;

                                    aDAVPropertyValue.Name = aStr;
                                    aDAVPropertyValue.Value <<= mpContext->getWhiteSpace();
                                    maPropStatProperties.push_back(aDAVPropertyValue);
                                }
                                break;
                            }
                            case WebDAVName_creationdate:
                            {
                                // creationdate end, safe if content is correct
                                if(propertyIsReady())
                                {
                                    static rtl::OUString aStr(rtl::OUString::createFromAscii("DAV:creationdate"));
                                    http_dav_ucp::DAVPropertyValue aDAVPropertyValue;

                                    aDAVPropertyValue.Name = aStr;
                                    aDAVPropertyValue.Value <<= mpContext->getWhiteSpace();
                                    maPropStatProperties.push_back(aDAVPropertyValue);
                                }
                                break;
                            }
                            case WebDAVName_collection:
                            {
                                // collection end, check and set
                                if(hasParent(WebDAVName_resourcetype))
                                {
                                    mbResourceTypeCollection = true;
                                }
                                break;
                            }
                            case WebDAVName_resourcetype:
                            {
                                // resourcetype end, check for collection
                                if(hasParent(WebDAVName_prop))
                                {
                                    static rtl::OUString aStrA(rtl::OUString::createFromAscii("DAV:resourcetype"));
                                    static rtl::OUString aStrB(rtl::OUString::createFromAscii("collection"));
                                    http_dav_ucp::DAVPropertyValue aDAVPropertyValue;

                                    aDAVPropertyValue.Name = aStrA;
                                    aDAVPropertyValue.Value <<= (mbResourceTypeCollection ? aStrB : rtl::OUString());
                                    maPropStatProperties.push_back(aDAVPropertyValue);
                                }
                                break;
                            }
                            case WebDAVName_getcontentlength:
                            {
                                // getcontentlength end, safe if content is correct
                                if(propertyIsReady())
                                {
                                    static rtl::OUString aStr(rtl::OUString::createFromAscii("DAV:getcontentlength"));
                                    http_dav_ucp::DAVPropertyValue aDAVPropertyValue;

                                    aDAVPropertyValue.Name = aStr;
                                    aDAVPropertyValue.Value <<= mpContext->getWhiteSpace();
                                    maPropStatProperties.push_back(aDAVPropertyValue);
                                }
                                break;
                            }
                            case WebDAVName_getcontenttype:
                            {
                                // getcontenttype end, safe if content is correct
                                if(propertyIsReady())
                                {
                                    static rtl::OUString aStr(rtl::OUString::createFromAscii("DAV:getcontenttype"));
                                    http_dav_ucp::DAVPropertyValue aDAVPropertyValue;

                                    aDAVPropertyValue.Name = aStr;
                                    aDAVPropertyValue.Value <<= mpContext->getWhiteSpace();
                                    maPropStatProperties.push_back(aDAVPropertyValue);
                                }
                                break;
                            }
                            case WebDAVName_supportedlock:
                            {
                                // supportedlock end
                                if(hasParent(WebDAVName_prop) && maLockEntries.hasElements())
                                {
                                    static rtl::OUString aStr(rtl::OUString::createFromAscii("DAV:supportedlock"));
                                    http_dav_ucp::DAVPropertyValue aDAVPropertyValue;

                                    aDAVPropertyValue.Name = aStr;
                                    aDAVPropertyValue.Value <<= maLockEntries;
                                    maPropStatProperties.push_back(aDAVPropertyValue);
                                }
                                break;
                            }
                            case WebDAVName_lockentry:
                            {
                                // lockentry end
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
                                // exclusive lockscope end
                                if(hasParent(WebDAVName_lockscope))
                                {
                                    maLockScope = ucb::LockScope_EXCLUSIVE;
                                    mbLockScopeSet = true;
                                }
                                break;
                            }
                            case WebDAVName_shared:
                            {
                                // shared lockscope end
                                if(hasParent(WebDAVName_lockscope))
                                {
                                    maLockScope = ucb::LockScope_SHARED;
                                    mbLockScopeSet = true;
                                }
                                break;
                            }
                            case WebDAVName_write:
                            {
                                // write locktype end
                                if(hasParent(WebDAVName_locktype))
                                {
                                    maLockType = ucb::LockType_WRITE;
                                    mbLockTypeSet = true;
                                }
                                break;
                            }
                            case WebDAVName_propstat:
                            {
                                // propstat end, check status
                                if(maStatus.getLength())
                                {
                                    static ::rtl::OUString aStrStatusOkay(::rtl::OUString::createFromAscii("HTTP/1.1 200 OK"));

                                    if(maStatus.equals(aStrStatusOkay))
                                    {
                                        if(isCollectingProperties())
                                        {
                                            if(maPropStatProperties.size())
                                            {
                                                // append to maResponseProperties if okay
                                                maResponseProperties.insert(maResponseProperties.end(), maPropStatProperties.begin(), maPropStatProperties.end());
                                            }
                                        }
                                        else
                                        {
                                            if(maPropStatNames.size())
                                            {
                                                // when collecting properties append to
                                                maResponseNames.insert(maResponseNames.end(), maPropStatNames.begin(), maPropStatNames.end());
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                            case WebDAVName_response:
                            {
                                // respose end
                                if(maHref.getLength())
                                {
                                    if(isCollectingProperties())
                                    {
                                        // create DAVResource when we have content
                                        if(maResponseProperties.size())
                                        {
                                            http_dav_ucp::DAVResource aDAVResource;

                                            aDAVResource.uri = maHref;
                                            aDAVResource.properties = maResponseProperties;
                                            maResult_PropFind.push_back(aDAVResource);
                                        }
                                    }
                                    else
                                    {
                                        // when collecting properties add them to result when there are some
                                        if(maResponseNames.size())
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

            // destroy last context (pop)
            pop_context();
        }
    }

    void SAL_CALL WebDAVResponseParser::characters( const ::rtl::OUString& aChars ) throw (xml::sax::SAXException, uno::RuntimeException)
    {
        // collect whitespace over evtl. several calls in mpContext
        OSL_ENSURE(mpContext, "Parser characters without content (!)");
        const sal_Int32 nLen(aChars.getLength());

        if(mpContext && nLen)
        {
            // remove leading/trailing blanks and CRLF
            const ::rtl::OUString aTrimmedChars(aChars.trim());

            if(aTrimmedChars.getLength())
            {
                ::rtl::OUString aNew(mpContext->getWhiteSpace());

                if(aNew.getLength())
                {
                    // add one char when appending (see html1.1 spec)
                    aNew += ::rtl::OUString(sal_Unicode(' '));
                }

                aNew += aTrimmedChars;
                mpContext->setWhiteSpace(aNew);
            }
        }
    }

    void SAL_CALL WebDAVResponseParser::ignorableWhitespace( const ::rtl::OUString& /*aWhitespaces*/ ) throw (xml::sax::SAXException, uno::RuntimeException)
    {
    }

    void SAL_CALL WebDAVResponseParser::processingInstruction( const ::rtl::OUString& /*aTarget*/, const ::rtl::OUString& /*aData*/ ) throw (xml::sax::SAXException, uno::RuntimeException)
    {
    }

    void SAL_CALL WebDAVResponseParser::setDocumentLocator( const uno::Reference< xml::sax::XLocator >& /*xLocator*/ ) throw (xml::sax::SAXException, uno::RuntimeException)
    {
    }
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////
// wrapper for various calls to the parser

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
                // prepare ParserInputSrouce
                xml::sax::InputSource myInputSource;
                myInputSource.aInputStream = xInputStream;

                // get parser
                uno::Reference< xml::sax::XParser > xParser(
                    comphelper::getProcessServiceFactory()->createInstance(
                        rtl::OUString::createFromAscii("com.sun.star.xml.sax.Parser") ),
                    uno::UNO_QUERY_THROW );

                // create parser; connect parser and filter
                WebDAVResponseParser* pWebDAVResponseParser = new WebDAVResponseParser(eWebDAVResponseParserMode);
                uno::Reference< xml::sax::XDocumentHandler > xWebDAVHdl(pWebDAVResponseParser);
                xParser->setDocumentHandler(xWebDAVHdl);

                // finally, parse the stream
                xParser->parseStream(myInputSource);

                // get result
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
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////
// helper to parse a XML WebDAV response

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
} // namespace http_dav_ucp

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
