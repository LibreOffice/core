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


#include <helpcompiler/HelpCompiler.hxx>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <osl/thread.hxx>

static void impl_sleep( sal_uInt32 nSec )
{
    TimeValue aTime;
    aTime.Seconds = nSec;
    aTime.Nanosec = 0;

    osl::Thread::wait( aTime );
}

HelpCompiler::HelpCompiler(StreamTable &in_streamTable, const fs::path &in_inputFile,
    const fs::path &in_src, const fs::path &in_resEmbStylesheet,
    const std::string &in_module, const std::string &in_lang, bool in_bExtensionMode)
    : streamTable(in_streamTable), inputFile(in_inputFile),
    src(in_src), module(in_module), lang(in_lang), resEmbStylesheet(in_resEmbStylesheet),
    bExtensionMode( in_bExtensionMode )
{
    xmlKeepBlanksDefaultValue = 0;
    char* guitmp = getenv("GUI");
    if (guitmp)
    {
        gui = (strcmp(guitmp, "UNX") ? gui : "UNIX");
        gui = (strcmp(guitmp, "MAC") ? gui : "MAC");
        gui = (strcmp(guitmp, "WNT") ? gui : "WIN");
    }
}

xmlDocPtr HelpCompiler::getSourceDocument(const fs::path &filePath)
{
    static const char *params[4 + 1];
    static xsltStylesheetPtr cur = NULL;

    xmlDocPtr res;
    if( bExtensionMode )
    {
        res = xmlParseFile(filePath.native_file_string().c_str());
        if( !res ){
            impl_sleep( 3 );
            res = xmlParseFile(filePath.native_file_string().c_str());
        }
    }
    else
    {
        if (!cur)
        {
            static std::string fsroot('\'' + src.toUTF8() + '\'');
            static std::string esclang('\'' + lang + '\'');

            xmlSubstituteEntitiesDefault(1);
            xmlLoadExtDtdDefaultValue = 1;
            cur = xsltParseStylesheetFile((const xmlChar *)resEmbStylesheet.native_file_string().c_str());

            int nbparams = 0;
            params[nbparams++] = "Language";
            params[nbparams++] = esclang.c_str();
            params[nbparams++] = "fsroot";
            params[nbparams++] = fsroot.c_str();
            params[nbparams] = NULL;
        }
        xmlDocPtr doc = xmlParseFile(filePath.native_file_string().c_str());
        if( !doc )
        {
            impl_sleep( 3 );
            doc = xmlParseFile(filePath.native_file_string().c_str());
        }

        //???res = xmlParseFile(filePath.native_file_string().c_str());

        res = xsltApplyStylesheet(cur, doc, params);
        xmlFreeDoc(doc);
    }
    return res;
}

// returns a node representing the whole stuff compiled for the current
// application.
xmlNodePtr HelpCompiler::clone(xmlNodePtr node, const std::string& appl)
{
    xmlNodePtr root = xmlCopyNode(node, 2);
    if (node->xmlChildrenNode)
    {
        xmlNodePtr list = node->xmlChildrenNode;
        while (list)
        {
            if (strcmp((const char*)list->name, "switchinline") == 0 || strcmp((const char*)list->name, "switch") == 0)
            {
                std::string tmp="";
                if (strcmp((const char*)xmlGetProp(list, (xmlChar*)"select"), "sys"))
                {
                    tmp = gui;
                }
                if (strcmp((const char*)xmlGetProp(list, (xmlChar*)"select"), "appl"))
                {
                    tmp = appl;
                }
                if (tmp.compare("") != 0)
                {
                    bool isCase=false;
                    xmlNodePtr caseList=list->xmlChildrenNode;
                    while (caseList)
                    {
                        xmlChar *select = xmlGetProp(caseList, (xmlChar*)"select");
                        if (select)
                        {
                            if (!strcmp((const char*)select, tmp.c_str()) && !isCase)
                            {
                                isCase=true;
                                xmlNodePtr clp = caseList->xmlChildrenNode;
                                while (clp)
                                {
                                    xmlAddChild(root, clone(clp, appl));
                                    clp = clp->next;
                                }
                            }
                            xmlFree(select);
                        }
                        else
                        {
                            if ((strcmp((const char*)caseList->name, "defaultinline") != 0) && (strcmp((const char*)caseList->name, "default") != 0))
                            {
                                xmlAddChild(root, clone(caseList, appl));
                            }
                            else
                            {
                                if (!isCase)
                                {
                                    xmlNodePtr clp = caseList->xmlChildrenNode;
                                    while (clp)
                                    {
                                        xmlAddChild(root, clone(clp, appl));
                                        clp = clp->next;
                                    }
                                }
                            }
                        }
                        caseList = caseList->next;
                    }
                }
            }
            else
            {
                xmlAddChild(root, clone(list, appl));
            }
            list = list->next;
        }
    }
    return root;
}

class myparser
{
public:
    std::string documentId;
    std::string fileName;
    std::string title;
    HashSet *hidlist;
    Hashtable *keywords;
    Stringtable *helptexts;
private:
    HashSet extendedHelpText;
public:
    myparser(const std::string &indocumentId, const std::string &infileName,
        const std::string &intitle) : documentId(indocumentId), fileName(infileName),
        title(intitle)
    {
        hidlist = new HashSet;
        keywords = new Hashtable;
        helptexts = new Stringtable;
    }
    void traverse( xmlNodePtr parentNode );
private:
    std::string module;
    std::string dump(xmlNodePtr node);
};

std::string myparser::dump(xmlNodePtr node)
{
    std::string app;
    if (node->xmlChildrenNode)
    {
        xmlNodePtr list = node->xmlChildrenNode;
        while (list)
        {
            app += dump(list);
            list = list->next;
        }
    }
    if (xmlNodeIsText(node))
    {
        xmlChar *pContent = xmlNodeGetContent(node);
        app += std::string((const char*)pContent);
        xmlFree(pContent);
    }
    return app;
}

void trim(std::string& str)
{
    std::string::size_type pos = str.find_last_not_of(' ');
    if(pos != std::string::npos)
    {
        str.erase(pos + 1);
        pos = str.find_first_not_of(' ');
        if(pos != std::string::npos)
            str.erase(0, pos);
    }
    else
        str.erase(str.begin(), str.end());
}

void myparser::traverse( xmlNodePtr parentNode )
{
    // traverse all nodes that belong to the parent
    xmlNodePtr test ;
    for (test = parentNode->xmlChildrenNode; test; test = test->next)
    {
        if (fileName.empty() && !strcmp((const char*)test->name, "filename"))
        {
            xmlNodePtr node = test->xmlChildrenNode;
            if (xmlNodeIsText(node))
            {
                xmlChar *pContent = xmlNodeGetContent(node);
                fileName = std::string((const char*)pContent);
                xmlFree(pContent);
            }
        }
        else if (title.empty() && !strcmp((const char*)test->name, "title"))
        {
            title = dump(test);
            if (title.empty())
                title = "<notitle>";
        }
        else if (!strcmp((const char*)test->name, "bookmark"))
        {
            xmlChar *branchxml = xmlGetProp(test, (const xmlChar*)"branch");
            xmlChar *idxml = xmlGetProp(test, (const xmlChar*)"id");
            std::string branch((const char*)branchxml);
            std::string anchor((const char*)idxml);
            xmlFree (branchxml);
            xmlFree (idxml);

            std::string hid;

            if (branch.find("hid") == 0)
            {
                size_t index = branch.find('/');
                if (index != std::string::npos)
                {
                    hid = branch.substr(1 + index);
                    // one shall serve as a documentId
                    if (documentId.empty())
                        documentId = hid;
                    extendedHelpText.push_back(hid);
                    std::string foo = anchor.empty() ? hid : hid + "#" + anchor;
                    HCDBG(std::cerr << "hid pushback" << foo << std::endl);
                    hidlist->push_back( anchor.empty() ? hid : hid + "#" + anchor);
                }
                else
                    continue;
            }
            else if (branch.compare("index") == 0)
            {
                LinkedList ll;

                for (xmlNodePtr nd = test->xmlChildrenNode; nd; nd = nd->next)
                {
                    if (strcmp((const char*)nd->name, "bookmark_value"))
                        continue;

                    std::string embedded;
                    xmlChar *embeddedxml = xmlGetProp(nd, (const xmlChar*)"embedded");
                    if (embeddedxml)
                    {
                        embedded = std::string((const char*)embeddedxml);
                        xmlFree (embeddedxml);
                        std::transform (embedded.begin(), embedded.end(),
                            embedded.begin(), tocharlower);
                    }

                    bool isEmbedded = !embedded.empty() && embedded.compare("true") == 0;
                    if (isEmbedded)
                        continue;

                    std::string keyword = dump(nd);
                    size_t keywordSem = keyword.find(';');
                    if (keywordSem != std::string::npos)
                    {
                        std::string tmppre =
                                    keyword.substr(0,keywordSem);
                        trim(tmppre);
                        std::string tmppos =
                                    keyword.substr(1+keywordSem);
                        trim(tmppos);
                        keyword = tmppre + ";" + tmppos;
                    }
                    ll.push_back(keyword);
                }
                if (!ll.empty())
                    (*keywords)[anchor] = ll;
            }
            else if (branch.compare("contents") == 0)
            {
                // currently not used
            }
        }
        else if (!strcmp((const char*)test->name, "ahelp"))
        {
            std::string text = dump(test);
            trim(text);
            std::string name;

            HashSet::const_iterator aEnd = extendedHelpText.end();
            for (HashSet::const_iterator iter = extendedHelpText.begin(); iter != aEnd; ++iter)
            {
                name = *iter;
                (*helptexts)[name] = text;
            }
            extendedHelpText.clear();
        }
        // traverse children
        traverse(test);
    }
}

bool HelpCompiler::compile( void ) throw( HelpProcessingException )
{
    // we now have the jaroutputstream, which will contain the document.
    // now determine the document as a dom tree in variable docResolved

    xmlDocPtr docResolvedOrg = getSourceDocument(inputFile);

    // now add path to the document
    // resolve the dom

    if (!docResolvedOrg)
    {
        impl_sleep( 3 );
        docResolvedOrg = getSourceDocument(inputFile);
        if( !docResolvedOrg )
        {
            std::stringstream aStrStream;
            aStrStream << "ERROR: file not existing: " << inputFile.native_file_string().c_str() << std::endl;
            throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
        }
    }

    std::string documentId;
    std::string fileName;
    std::string title;
    // returns a clone of the document with switch-cases resolved
    std::string appl = module.substr(1);
    for (unsigned int i = 0; i < appl.length(); ++i)
    {
        appl[i]=toupper(appl[i]);
    }
    xmlNodePtr docResolved = clone(xmlDocGetRootElement(docResolvedOrg), appl);
    myparser aparser(documentId, fileName, title);
    aparser.traverse(docResolved);
    documentId = aparser.documentId;
    fileName = aparser.fileName;
    title = aparser.title;

    HCDBG(std::cerr << documentId << " : " << fileName << " : " << title << std::endl);

    xmlDocPtr docResolvedDoc = xmlCopyDoc(docResolvedOrg, false);
    xmlDocSetRootElement(docResolvedDoc, docResolved);

    streamTable.dropappl();
    streamTable.appl_doc = docResolvedDoc;
    streamTable.appl_hidlist = aparser.hidlist;
    streamTable.appl_helptexts = aparser.helptexts;
    streamTable.appl_keywords = aparser.keywords;

    streamTable.document_id = documentId;
    streamTable.document_path = fileName;
    streamTable.document_title = title;
    std::string actMod = module;

    if ( !bExtensionMode && !fileName.empty())
    {
        if (fileName.find("/text/") == 0)
        {
            int len = strlen("/text/");
            actMod = fileName.substr(len);
            actMod = actMod.substr(0, actMod.find('/'));
        }
    }
    streamTable.document_module = actMod;
    xmlFreeDoc(docResolvedOrg);
    return true;
}

namespace fs
{
    rtl_TextEncoding getThreadTextEncoding( void )
    {
        static bool bNeedsInit = true;
        static rtl_TextEncoding nThreadTextEncoding;
        if( bNeedsInit )
        {
            bNeedsInit = false;
            nThreadTextEncoding = osl_getThreadTextEncoding();
        }
        return nThreadTextEncoding;
    }

    void create_directory(const fs::path indexDirName)
    {
        HCDBG(
            std::cerr << "creating " <<
            rtl::OUStringToOString(indexDirName.data, RTL_TEXTENCODING_UTF8).getStr()
            << std::endl
           );
        osl::Directory::createPath(indexDirName.data);
    }

    void copy(const fs::path &src, const fs::path &dest)
    {
        osl::File::copy(src.data, dest.data);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
