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
 */

#include "interpre.hxx"
#include "libxml/xpath.h"

#include <boost/shared_ptr.hpp>

// TODO: Add new methods for ScInterpreter here.

void ScInterpreter::ScFilterXML()
{
    sal_uInt8 nParamCount = GetByte();
    if (MustHaveParamCount( nParamCount, 2 ) )
    {
        OUString aXPathExpression = GetString();
        OUString aString = GetString();

        const char* pXPathExpr = OUStringToOString( aXPathExpression, RTL_TEXTENCODING_UTF8 ).getStr();
        const char* pXML = OUStringToOString( aString, RTL_TEXTENCODING_UTF8 ).getStr();

        boost::shared_ptr<xmlParserCtxt> pContext(
                xmlNewParserCtxt(), xmlFreeParserCtxt );

        boost::shared_ptr<xmlDoc> pDoc( xmlParseMemory( pXML, aString.getLength() ),
                xmlFreeDoc );


        boost::shared_ptr<xmlXPathContext> pXPathCtx( xmlXPathNewContext(pDoc.get()),
                xmlXPathFreeContext );

        boost::shared_ptr<xmlXPathObject> pXPathObj( xmlXPathEvalExpression(BAD_CAST(pXPathExpr), pXPathCtx.get()),
                xmlXPathFreeObject );

        switch(pXPathObj->type)
        {
            case XPATH_UNDEFINED:
                break;
            case XPATH_NODESET:
                {
                    xmlNodeSetPtr pNodeSet = pXPathObj->nodesetval;
                    size_t nSize = pNodeSet->nodeNr;
                    if( nSize >= 1 )
                    {
                        if(pNodeSet->nodeTab[0]->type == XML_NAMESPACE_DECL)
                        {
                            xmlNsPtr ns = (xmlNsPtr)pNodeSet->nodeTab[0];
                            xmlNodePtr cur = (xmlNodePtr)ns->next;
                            boost::shared_ptr<xmlChar> pChar2(xmlNodeGetContent(cur), xmlFree);
                            OUString aResult = OUString::createFromAscii((char*)pChar2.get());
                            PushString(aResult);
                        }
                        else if(pNodeSet->nodeTab[0]->type == XML_ELEMENT_NODE)
                        {
                            xmlNodePtr cur = pNodeSet->nodeTab[0];
                            boost::shared_ptr<xmlChar> pChar2(xmlNodeGetContent(cur), xmlFree);
                            OUString aResult = OUString::createFromAscii((char*)pChar2.get());
                            PushString(aResult);
                        }
                        else
                        {
                            xmlNodePtr cur = pNodeSet->nodeTab[0];
                            boost::shared_ptr<xmlChar> pChar2(xmlNodeGetContent(cur), xmlFree);
                            OUString aResult = OUString::createFromAscii((char*)pChar2.get());
                            PushString(aResult);
                        }
                    }
                }
                break;
            case XPATH_BOOLEAN:
                assert(false);
                break;
            case XPATH_NUMBER:
                assert(false);
                break;
            case XPATH_STRING:
                assert(false);
                break;
            case XPATH_POINT:
                assert(false);
                break;
            case XPATH_RANGE:
                assert(false);
                break;
            case XPATH_LOCATIONSET:
                assert(false);
                break;
            case XPATH_USERS:
                assert(false);
                break;
            case XPATH_XSLT_TREE:
                assert(false);
                break;

        }

    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
