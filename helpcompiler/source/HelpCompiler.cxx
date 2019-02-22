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


#include <memory>
#include <HelpCompiler.hxx>
#include <BasCodeTagger.hxx>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <osl/thread.hxx>
#include <chrono>
#include <rtl/character.hxx>
#include <sal/log.hxx>

static void impl_sleep( sal_uInt32 nSec )
{
    osl::Thread::wait( std::chrono::seconds(nSec) );
}
HelpCompiler::HelpCompiler(StreamTable &in_streamTable, const fs::path &in_inputFile,
    const fs::path &in_src, const fs::path &in_zipdir, const fs::path &in_resCompactStylesheet,
    const fs::path &in_resEmbStylesheet, const std::string &in_module, const std::string &in_lang,
    bool in_bExtensionMode)
    : streamTable(in_streamTable), inputFile(in_inputFile),
    src(in_src), zipdir(in_zipdir), module(in_module), lang(in_lang), resCompactStylesheet(in_resCompactStylesheet),
    resEmbStylesheet(in_resEmbStylesheet), bExtensionMode( in_bExtensionMode )
{
    xmlKeepBlanksDefaultValue = 0;
    char* os = getenv("OS");
    if (os)
    {
        gui = (strcmp(os, "WNT") ? "UNIX" : "WIN");
        gui = (strcmp(os, "MACOSX") ? gui : "MAC");
    }
}

void HelpCompiler::tagBasicCodeExamples( xmlDocPtr doc )
{
    try
    {
        BasicCodeTagger bct( doc );
        bct.tagBasicCodes();
    }
    catch ( BasicCodeTagger::TaggerException &ex )
    {
        if ( ex != BasicCodeTagger::EMPTY_DOCUMENT )
            throw;
    }
}

xmlDocPtr HelpCompiler::compactXhpForJar( xmlDocPtr doc )
{
    static xsltStylesheetPtr compact = nullptr;
    static const char *params[2 + 1];
    params[0] = nullptr;
    xmlDocPtr compacted;

    if (!compact)
    {
        compact = xsltParseStylesheetFile(reinterpret_cast<const xmlChar *>(resCompactStylesheet.native_file_string().c_str()));
    }

    compacted = xsltApplyStylesheet(compact, doc, params);
    return compacted;
}

void HelpCompiler::saveXhpForJar( xmlDocPtr doc, const fs::path &filePath )
{
    //save processed xhp document in ziptmp<module>_<lang>/text directory
#ifdef _WIN32
    std::string pathSep = "\\";
#else
    std::string pathSep = "/";
#endif
    const std::string& sourceXhpPath = filePath.native_file_string();
    std::string zipdirPath = zipdir.native_file_string();
    const std::string srcdirPath( src.native_file_string() );
    // srcdirPath contains trailing /, but we want the file path with / at the beginning
    std::string jarXhpPath = sourceXhpPath.substr( srcdirPath.length() - 1 );
    std::string xhpFileName = jarXhpPath.substr( jarXhpPath.rfind( pathSep ) + 1 );
    jarXhpPath = jarXhpPath.substr( 0, jarXhpPath.rfind( pathSep ) );
    if ( !jarXhpPath.compare( 1, 11, "text" + pathSep + "sbasic" ) )
    {
        tagBasicCodeExamples( doc );
    }
    if ( !jarXhpPath.compare( 1, 11, "text" + pathSep + "shared" ) )
    {
        const size_t pos = zipdirPath.find( "ziptmp" );
        if ( pos != std::string::npos )
            zipdirPath.replace( pos + 6, module.length(), "shared" );
    }
    xmlDocPtr compacted = compactXhpForJar( doc );
    fs::create_directory( fs::path( zipdirPath + jarXhpPath, fs::native ) );
    if ( -1 == xmlSaveFormatFileEnc( (zipdirPath + jarXhpPath + pathSep + xhpFileName).c_str(), compacted, "utf-8", 0 ) )
        std::cerr << "Error saving file to " << (zipdirPath + jarXhpPath + pathSep + xhpFileName).c_str() << std::endl;
    xmlFreeDoc(compacted);
}


xmlDocPtr HelpCompiler::getSourceDocument(const fs::path &filePath)
{
    static xsltStylesheetPtr cur = nullptr;

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
        static const char *params[2 + 1];
        if (!cur)
        {
            static std::string fsroot('\'' + src.toUTF8() + '\'');

            xmlSubstituteEntitiesDefault(1);
            xmlLoadExtDtdDefaultValue = 1;
            cur = xsltParseStylesheetFile(reinterpret_cast<const xmlChar *>(resEmbStylesheet.native_file_string().c_str()));

            int nbparams = 0;
            params[nbparams++] = "fsroot";
            params[nbparams++] = fsroot.c_str();
            params[nbparams] = nullptr;
        }
        xmlDocPtr doc = xmlParseFile(filePath.native_file_string().c_str());
        if( !doc )
        {
            impl_sleep( 3 );
            doc = xmlParseFile(filePath.native_file_string().c_str());
        }

        saveXhpForJar( doc, filePath );

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
            if (strcmp(reinterpret_cast<const char*>(list->name), "switchinline") == 0 || strcmp(reinterpret_cast<const char*>(list->name), "switch") == 0)
            {
                std::string tmp="";
                xmlChar * prop = xmlGetProp(list, reinterpret_cast<xmlChar const *>("select"));
                if (prop != nullptr)
                {
                    if (strcmp(reinterpret_cast<char *>(prop), "sys") == 0)
                    {
                        tmp = gui;
                    }
                    else if (strcmp(reinterpret_cast<char *>(prop), "appl") == 0)
                    {
                        tmp = appl;
                    }
                    xmlFree(prop);
                }
                if (tmp.compare("") != 0)
                {
                    bool isCase=false;
                    xmlNodePtr caseList=list->xmlChildrenNode;
                    while (caseList)
                    {
                        xmlChar *select = xmlGetProp(caseList, reinterpret_cast<xmlChar const *>("select"));
                        if (select)
                        {
                            if (!strcmp(reinterpret_cast<char*>(select), tmp.c_str()) && !isCase)
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
                            if ((strcmp(reinterpret_cast<const char*>(caseList->name), "defaultinline") != 0) && (strcmp(reinterpret_cast<const char*>(caseList->name), "default") != 0))
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
    std::unique_ptr< std::vector<std::string> > hidlist;
    std::unique_ptr<Hashtable> keywords;
    std::unique_ptr<Stringtable> helptexts;
private:
    std::vector<std::string> extendedHelpText;
public:
    myparser(const std::string &indocumentId, const std::string &infileName,
        const std::string &intitle) : documentId(indocumentId), fileName(infileName),
        title(intitle)
    {
        hidlist.reset(new std::vector<std::string>);
        keywords.reset(new Hashtable);
        helptexts.reset(new Stringtable);
    }
    void traverse( xmlNodePtr parentNode );
private:
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
        app += std::string(reinterpret_cast<char*>(pContent));
        xmlFree(pContent);
    }
    return app;
}

static void trim(std::string& str)
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
        str.clear();
}

void myparser::traverse( xmlNodePtr parentNode )
{
    // traverse all nodes that belong to the parent
    xmlNodePtr test ;
    for (test = parentNode->xmlChildrenNode; test; test = test->next)
    {
        if (fileName.empty() && !strcmp(reinterpret_cast<const char*>(test->name), "filename"))
        {
            xmlNodePtr node = test->xmlChildrenNode;
            if (xmlNodeIsText(node))
            {
                xmlChar *pContent = xmlNodeGetContent(node);
                fileName = std::string(reinterpret_cast<char*>(pContent));
                xmlFree(pContent);
            }
        }
        else if (title.empty() && !strcmp(reinterpret_cast<const char*>(test->name), "title"))
        {
            title = dump(test);
            if (title.empty())
                title = "<notitle>";
        }
        else if (!strcmp(reinterpret_cast<const char*>(test->name), "bookmark"))
        {
            xmlChar *branchxml = xmlGetProp(test, reinterpret_cast<const xmlChar*>("branch"));
            if (branchxml == nullptr) {
                throw HelpProcessingException(
                    HelpProcessingErrorClass::XmlParsing, "bookmark lacks branch attribute");
            }
            std::string branch(reinterpret_cast<char*>(branchxml));
            xmlFree (branchxml);
            xmlChar *idxml = xmlGetProp(test, reinterpret_cast<const xmlChar*>("id"));
            if (idxml == nullptr) {
                throw HelpProcessingException(
                    HelpProcessingErrorClass::XmlParsing, "bookmark lacks id attribute");
            }
            std::string anchor(reinterpret_cast<char*>(idxml));
            xmlFree (idxml);

            if (branch.compare(0, 3, "hid") == 0)
            {
                size_t index = branch.find('/');
                if (index != std::string::npos)
                {
                    auto hid = branch.substr(1 + index);
                    // one shall serve as a documentId
                    if (documentId.empty())
                        documentId = hid;
                    extendedHelpText.push_back(hid);
                    HCDBG(std::cerr << "hid pushback" << (anchor.empty() ? hid : hid + "#" + anchor) << std::endl);
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
                    if (strcmp(reinterpret_cast<const char*>(nd->name), "bookmark_value"))
                        continue;

                    std::string embedded;
                    xmlChar *embeddedxml = xmlGetProp(nd, reinterpret_cast<const xmlChar*>("embedded"));
                    if (embeddedxml)
                    {
                        embedded = std::string(reinterpret_cast<char*>(embeddedxml));
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
        else if (!strcmp(reinterpret_cast<const char*>(test->name), "ahelp"))
        {
            //tool-tip
            std::string text = dump(test);
            std::replace(text.begin(), text.end(), '\n', ' ');
            trim(text);

            //tool-tip target
            std::string hidstr(".");  //. == previous seen hid bookmarks
            xmlChar *hid = xmlGetProp(test, reinterpret_cast<const xmlChar*>("hid"));
            if (hid)
            {
                hidstr = std::string(reinterpret_cast<char*>(hid));
                xmlFree (hid);
            }

            if (hidstr != "." && !hidstr.empty())  //simple case of explicitly named target
            {
                assert(!hidstr.empty());
                (*helptexts)[hidstr] = text;
            }
            else //apply to list of "current" hids determined by recent bookmarks that have hid in their branch
            {
                //TODO: make these asserts and flush out all our broken help ids
                SAL_WARN_IF(hidstr.empty(), "helpcompiler", "hid='' for text:" << text);
                SAL_WARN_IF(!hidstr.empty() && extendedHelpText.empty(), "helpcompiler", "hid='.' with no hid bookmark branches in file: " << fileName + " for text: " << text);
                for (const std::string& name : extendedHelpText)
                {
                    (*helptexts)[name] = text;
                }
            }
            extendedHelpText.clear();
        }
        // traverse children
        traverse(test);
    }
}

void HelpCompiler::compile()
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
            throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
        }
    }

    std::string documentId;
    std::string fileName;
    std::string title;
    // returns a clone of the document with switch-cases resolved
    std::string appl = module.substr(1);
    for (char & i : appl)
    {
        i=rtl::toAsciiUpperCase(static_cast<unsigned char>(i));
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
    streamTable.appl_hidlist = std::move(aparser.hidlist);
    streamTable.appl_helptexts = std::move(aparser.helptexts);
    streamTable.appl_keywords = std::move(aparser.keywords);

    streamTable.document_path = fileName;
    streamTable.document_title = title;
    std::string actMod = module;

    if ( !bExtensionMode && !fileName.empty())
    {
        if (fileName.compare(0, 6, "/text/") == 0)
        {
            actMod = fileName.substr(strlen("/text/"));
            actMod = actMod.substr(0, actMod.find('/'));
        }
    }
    streamTable.document_module = actMod;
    xmlFreeDoc(docResolvedOrg);
}

namespace fs
{
    rtl_TextEncoding getThreadTextEncoding()
    {
        static rtl_TextEncoding nThreadTextEncoding = osl_getThreadTextEncoding();
        return nThreadTextEncoding;
    }

    void create_directory(const fs::path& indexDirName)
    {
        HCDBG(
            std::cerr << "creating " <<
            OUStringToOString(indexDirName.data, RTL_TEXTENCODING_UTF8).getStr()
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
