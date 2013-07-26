/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "interpre.hxx"
#include <rtl/strbuf.hxx>

#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>

#include "libxml/xpath.h"

#include <boost/shared_ptr.hpp>
#include <cstring>

using namespace com::sun::star;

// TODO: Add new methods for ScInterpreter here.

void ScInterpreter::ScFilterXML()
{
    sal_uInt8 nParamCount = GetByte();
    if (MustHaveParamCount( nParamCount, 2 ) )
    {
        OUString aXPathExpression = GetString();
        OUString aString = GetString();
        if(aString.isEmpty() || aXPathExpression.isEmpty())
        {
            PushError( errNoValue );
            return;
        }

        OString aOXPathExpression = OUStringToOString( aXPathExpression, RTL_TEXTENCODING_UTF8 );
        const char* pXPathExpr = aOXPathExpression.getStr();
        OString aOString = OUStringToOString( aString, RTL_TEXTENCODING_UTF8 );
        const char* pXML = aOString.getStr();

        boost::shared_ptr<xmlParserCtxt> pContext(
                xmlNewParserCtxt(), xmlFreeParserCtxt );

        boost::shared_ptr<xmlDoc> pDoc( xmlParseMemory( pXML, aOString.getLength() ),
                xmlFreeDoc );

        if(!pDoc)
        {
            PushError( errNoValue );
            return;
        }


        boost::shared_ptr<xmlXPathContext> pXPathCtx( xmlXPathNewContext(pDoc.get()),
                xmlXPathFreeContext );

        boost::shared_ptr<xmlXPathObject> pXPathObj( xmlXPathEvalExpression(BAD_CAST(pXPathExpr), pXPathCtx.get()),
                xmlXPathFreeObject );

        if(!pXPathObj)
        {
            PushError( errNoValue );
            return;
        }

        rtl::OUString aResult;

        switch(pXPathObj->type)
        {
            case XPATH_UNDEFINED:
                break;
            case XPATH_NODESET:
                {
                    xmlNodeSetPtr pNodeSet = pXPathObj->nodesetval;
                    if(!pNodeSet)
                    {
                        PushError( errNoValue );
                        return;
                    }

                    size_t nSize = pNodeSet->nodeNr;
                    if( nSize >= 1 )
                    {
                        if(pNodeSet->nodeTab[0]->type == XML_NAMESPACE_DECL)
                        {
                            xmlNsPtr ns = (xmlNsPtr)pNodeSet->nodeTab[0];
                            xmlNodePtr cur = (xmlNodePtr)ns->next;
                            boost::shared_ptr<xmlChar> pChar2(xmlNodeGetContent(cur), xmlFree);
                            aResult = OUString::createFromAscii((char*)pChar2.get());
                        }
                        else if(pNodeSet->nodeTab[0]->type == XML_ELEMENT_NODE)
                        {
                            xmlNodePtr cur = pNodeSet->nodeTab[0];
                            boost::shared_ptr<xmlChar> pChar2(xmlNodeGetContent(cur), xmlFree);
                            aResult = OUString::createFromAscii((char*)pChar2.get());
                        }
                        else
                        {
                            xmlNodePtr cur = pNodeSet->nodeTab[0];
                            boost::shared_ptr<xmlChar> pChar2(xmlNodeGetContent(cur), xmlFree);
                            aResult = OUString::createFromAscii((char*)pChar2.get());
                        }
                    }
                    else
                    {
                        PushError( errNoValue );
                        return;
                    }
                }
                PushString(aResult);
                break;
            case XPATH_BOOLEAN:
                {
                    bool bVal = pXPathObj->boolval != 0;
                    PushDouble(bVal);
                }
                break;
            case XPATH_NUMBER:
                {
                    double fVal = pXPathObj->floatval;
                    PushDouble(fVal);
                }
                break;
            case XPATH_STRING:
                PushString(OUString::createFromAscii((char*)pXPathObj->stringval));
                break;
            case XPATH_POINT:
                PushNoValue();
                break;
            case XPATH_RANGE:
                PushNoValue();
                break;
            case XPATH_LOCATIONSET:
                PushNoValue();
                break;
            case XPATH_USERS:
                PushNoValue();
                break;
            case XPATH_XSLT_TREE:
                PushNoValue();
                break;

        }
    }
}

void ScInterpreter::ScWebservice()
{
    sal_uInt8 nParamCount = GetByte();
    if (MustHaveParamCount( nParamCount, 1 ) )
    {
        OUString aURI = GetString();

        if(aURI.isEmpty())
        {
            PushError( errNoValue );
            return;
        }

        uno::Reference< ucb::XSimpleFileAccess3 > xFileAccess( ucb::SimpleFileAccess::create( comphelper::getProcessComponentContext() ), uno::UNO_QUERY );
        if(!xFileAccess.is())
        {
            PushError( errNoValue );
            return;
        }

        uno::Reference< io::XInputStream > xStream;
        try {
            xStream = xFileAccess->openFileRead( aURI );
        }
        catch (...)
        {
            // don't let any exceptions pass
            PushError( errNoValue );
            return;
        }
        if ( !xStream.is() )
        {
            PushError( errNoValue );
            return;
        }

        const sal_Int32 BUF_LEN = 8000;
        uno::Sequence< sal_Int8 > buffer( BUF_LEN );
        OStringBuffer aBuffer( 64000 );

        sal_Int32 nRead = 0;
        while ( ( nRead = xStream->readBytes( buffer, BUF_LEN ) ) == BUF_LEN )
        {
            aBuffer.append( reinterpret_cast< const char* >( buffer.getConstArray() ), nRead );
        }

        if ( nRead > 0 )
        {
            aBuffer.append( reinterpret_cast< const char* >( buffer.getConstArray() ), nRead );
        }

        xStream->closeInput();

        OUString aContent = OStringToOUString( aBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8 );
        PushString( aContent );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
