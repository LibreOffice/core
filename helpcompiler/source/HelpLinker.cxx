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
#include <helpcompiler/HelpLinker.hxx>

#include <map>

#include <string.h>
#include <limits.h>

#include <libxslt/xslt.h>
#include <libxslt/xsltutils.h>
#include <libxslt/functions.h>
#include <libxslt/extensions.h>

#include <sal/main.h>
#include <sal/types.h>
#include <osl/time.h>
#include <rtl/bootstrap.hxx>

#include <expat.h>

IndexerPreProcessor::IndexerPreProcessor
    ( const std::string& aModuleName, const fs::path& fsIndexBaseDir,
      const fs::path& idxCaptionStylesheet, const fs::path& idxContentStylesheet )
        : m_aModuleName( aModuleName )
        , m_fsIndexBaseDir( fsIndexBaseDir )
{
    m_fsCaptionFilesDirName = fsIndexBaseDir / "caption";
    fs::create_directory( m_fsCaptionFilesDirName );

    m_fsContentFilesDirName = fsIndexBaseDir / "content";
    fs::create_directory( m_fsContentFilesDirName );

    m_xsltStylesheetPtrCaption = xsltParseStylesheetFile
        ((const xmlChar *)idxCaptionStylesheet.native_file_string().c_str());
    m_xsltStylesheetPtrContent = xsltParseStylesheetFile
        ((const xmlChar *)idxContentStylesheet.native_file_string().c_str());
}

IndexerPreProcessor::~IndexerPreProcessor()
{
    if( m_xsltStylesheetPtrCaption )
        xsltFreeStylesheet( m_xsltStylesheetPtrCaption );
    if( m_xsltStylesheetPtrContent )
        xsltFreeStylesheet( m_xsltStylesheetPtrContent );
}

std::string getEncodedPath( const std::string& Path )
{
    rtl::OString aOStr_Path( Path.c_str() );
    rtl::OUString aOUStr_Path( rtl::OStringToOUString
        ( aOStr_Path, fs::getThreadTextEncoding() ) );
    rtl::OUString aPathURL;
    osl::File::getFileURLFromSystemPath( aOUStr_Path, aPathURL );
    rtl::OString aOStr_PathURL( rtl::OUStringToOString
        ( aPathURL, fs::getThreadTextEncoding() ) );
    std::string aStdStr_PathURL( aOStr_PathURL.getStr() );
    return aStdStr_PathURL;
}

void IndexerPreProcessor::processDocument
    ( xmlDocPtr doc, const std::string &EncodedDocPath )
{
    std::string aStdStr_EncodedDocPathURL = getEncodedPath( EncodedDocPath );

    if( m_xsltStylesheetPtrCaption )
    {
        xmlDocPtr resCaption = xsltApplyStylesheet( m_xsltStylesheetPtrCaption, doc, NULL );
        xmlNodePtr pResNodeCaption = resCaption->xmlChildrenNode;
        if( pResNodeCaption )
        {
            fs::path fsCaptionPureTextFile_docURL = m_fsCaptionFilesDirName / aStdStr_EncodedDocPathURL;
#ifdef WNT     //We need _wfopen to support long file paths on Windows XP
            FILE* pFile_docURL = _wfopen(
                fsCaptionPureTextFile_docURL.native_file_string_w(), L"w" );
#else
            FILE* pFile_docURL = fopen(
                fsCaptionPureTextFile_docURL.native_file_string().c_str(), "w" );
#endif
            if( pFile_docURL )
            {
                fprintf( pFile_docURL, "%s\n", pResNodeCaption->content );
                fclose( pFile_docURL );
            }
        }
        xmlFreeDoc(resCaption);
    }

    if( m_xsltStylesheetPtrContent )
    {
        xmlDocPtr resContent = xsltApplyStylesheet( m_xsltStylesheetPtrContent, doc, NULL );
        xmlNodePtr pResNodeContent = resContent->xmlChildrenNode;
        if( pResNodeContent )
        {
            fs::path fsContentPureTextFile_docURL = m_fsContentFilesDirName / aStdStr_EncodedDocPathURL;
#ifdef WNT     //We need _wfopen to support long file paths on Windows XP
            FILE* pFile_docURL = _wfopen(
                fsContentPureTextFile_docURL.native_file_string_w(), L"w" );
#else
            FILE* pFile_docURL = fopen(
                fsContentPureTextFile_docURL.native_file_string().c_str(), "w" );
#endif
            if( pFile_docURL )
            {
                fprintf( pFile_docURL, "%s\n", pResNodeContent->content );
                fclose( pFile_docURL );
            }
        }
        xmlFreeDoc(resContent);
    }
}

struct Data
{
    std::vector<std::string> _idList;
    typedef std::vector<std::string>::const_iterator cIter;

    void append(const std::string &id)
    {
        _idList.push_back(id);
    }

    std::string getString() const
    {
        std::string ret;
        cIter aEnd = _idList.end();
        for (cIter aIter = _idList.begin(); aIter != aEnd; ++aIter)
            ret += *aIter + ";";
        return ret;
    }
};

void writeKeyValue_DBHelp( FILE* pFile, const std::string& aKeyStr, const std::string& aValueStr )
{
    if( pFile == NULL )
        return;
    char cLF = 10;
    unsigned int nKeyLen = aKeyStr.length();
    unsigned int nValueLen = aValueStr.length();
    fprintf( pFile, "%x ", nKeyLen );
    if( nKeyLen > 0 )
    {
        if (fwrite( aKeyStr.c_str(), 1, nKeyLen, pFile ) != nKeyLen)
            fprintf(stderr, "fwrite to db failed\n");
    }
    if (fprintf( pFile, " %x ", nValueLen ) < 0)
        fprintf(stderr, "fwrite to db failed\n");
    if( nValueLen > 0 )
    {
        if (fwrite( aValueStr.c_str(), 1, nValueLen, pFile ) != nValueLen)
            fprintf(stderr, "fwrite to db failed\n");
    }
    if (fprintf( pFile, "%c", cLF ) < 0)
        fprintf(stderr, "fwrite to db failed\n");
}

class HelpKeyword
{
private:
    typedef boost::unordered_map<std::string, Data, pref_hash> DataHashtable;
    DataHashtable _hash;

public:
    void insert(const std::string &key, const std::string &id)
    {
        Data &data = _hash[key];
        data.append(id);
    }

    void dump_DBHelp( const fs::path& rFileName )
    {
#ifdef WNT     //We need _wfopen to support long file paths on Windows XP
        FILE* pFile = _wfopen( rFileName.native_file_string_w(), L"wb" );
#else
        FILE* pFile = fopen( rFileName.native_file_string().c_str(), "wb" );
#endif
        if( pFile == NULL )
            return;

        DataHashtable::const_iterator aEnd = _hash.end();
        for (DataHashtable::const_iterator aIter = _hash.begin(); aIter != aEnd; ++aIter)
            writeKeyValue_DBHelp( pFile, aIter->first, aIter->second.getString() );

        fclose( pFile );
    }
};

namespace URLEncoder
{
    static std::string encode(const std::string &rIn)
    {
        const char *good = "!$&'()*+,-.=@_";
        static const char hex[17] = "0123456789ABCDEF";

        std::string result;
        for (size_t i=0; i < rIn.length(); ++i)
        {
            unsigned char c = rIn[i];
            if (isalnum (c) || strchr (good, c))
                result += c;
            else {
                result += '%';
                result += hex[c >> 4];
                result += hex[c & 0xf];
            }
        }
        return result;
    }
}

void HelpLinker::addBookmark( FILE* pFile_DBHelp, std::string thishid,
        const std::string& fileB, const std::string& anchorB,
        const std::string& jarfileB, const std::string& titleB)
{
    HCDBG(std::cerr << "HelpLinker::addBookmark " << thishid << " " <<
        fileB << " " << anchorB << " " << jarfileB << " " << titleB << std::endl);

    thishid = URLEncoder::encode(thishid);

    int fileLen = fileB.length();
    if (!anchorB.empty())
        fileLen += (1 + anchorB.length());
    int dataLen = 1 + fileLen + 1 + jarfileB.length() + 1 + titleB.length();

    std::vector<unsigned char> dataB(dataLen);
    size_t i = 0;
    dataB[i++] = static_cast<unsigned char>(fileLen);
    for (size_t j = 0; j < fileB.length(); ++j)
        dataB[i++] = static_cast<unsigned char>(fileB[j]);
    if (!anchorB.empty())
    {
        dataB[i++] = '#';
        for (size_t j = 0; j < anchorB.length(); ++j)
            dataB[i++] = anchorB[j];
    }
    dataB[i++] = static_cast<unsigned char>(jarfileB.length());
    for (size_t j = 0; j < jarfileB.length(); ++j)
        dataB[i++] = jarfileB[j];

    dataB[i++] = static_cast<unsigned char>(titleB.length());
    for (size_t j = 0; j < titleB.length(); ++j)
        dataB[i++] = titleB[j];

    if( pFile_DBHelp != NULL )
    {
        std::string aValueStr( dataB.begin(), dataB.end() );
        writeKeyValue_DBHelp( pFile_DBHelp, thishid, aValueStr );
    }
}

void HelpLinker::initIndexerPreProcessor()
{
    if( m_pIndexerPreProcessor )
        delete m_pIndexerPreProcessor;
    std::string mod = module;
    std::transform (mod.begin(), mod.end(), mod.begin(), tocharlower);
    m_pIndexerPreProcessor = new IndexerPreProcessor( mod, indexDirParentName,
         idxCaptionStylesheet, idxContentStylesheet );
}

/**
*
*/
void HelpLinker::link() throw( HelpProcessingException )
{
    bool bIndexForExtension = true;

    if( bExtensionMode )
    {
        indexDirParentName = extensionDestination;
    }
    else
    {
        indexDirParentName = zipdir;
        fs::create_directory(indexDirParentName);
    }

    std::string mod = module;
    std::transform (mod.begin(), mod.end(), mod.begin(), tocharlower);

    // do the work here
    // continue with introduction of the overall process thing into the
    // here all hzip files will be worked on
    std::string appl = mod;
    if (appl[0] == 's')
        appl = appl.substr(1);

    bool bUse_ = true;
    if( !bExtensionMode )
        bUse_ = false;

    fs::path helpTextFileName_DBHelp(indexDirParentName / (mod + (bUse_ ? ".ht_" : ".ht")));
#ifdef WNT
    //We need _wfopen to support long file paths on Windows XP
    FILE* pFileHelpText_DBHelp = _wfopen
        ( helpTextFileName_DBHelp.native_file_string_w(), L"wb" );
#else

    FILE* pFileHelpText_DBHelp = fopen
        ( helpTextFileName_DBHelp.native_file_string().c_str(), "wb" );
#endif

    fs::path dbBaseFileName_DBHelp(indexDirParentName / (mod + (bUse_ ? ".db_" : ".db")));
#ifdef WNT
    //We need _wfopen to support long file paths on Windows XP
    FILE* pFileDbBase_DBHelp = _wfopen
        ( dbBaseFileName_DBHelp.native_file_string_w(), L"wb" );
#else
    FILE* pFileDbBase_DBHelp = fopen
        ( dbBaseFileName_DBHelp.native_file_string().c_str(), "wb" );
#endif

    fs::path keyWordFileName_DBHelp(indexDirParentName / (mod + (bUse_ ? ".key_" : ".key")));

    HelpKeyword helpKeyword;

    // catch HelpProcessingException to avoid locking data bases
    try
    {

    // lastly, initialize the indexBuilder
    if ( (!bExtensionMode || bIndexForExtension) && !helpFiles.empty())
        initIndexerPreProcessor();

    // here we start our loop over the hzip files.
    HashSet::iterator end = helpFiles.end();
    for (HashSet::iterator iter = helpFiles.begin(); iter != end; ++iter)
    {
        // process one file
        // streamTable contains the streams in the hzip file
        StreamTable streamTable;
        const std::string &xhpFileName = *iter;

        if (!bExtensionMode && xhpFileName.rfind(".xhp") != xhpFileName.length()-4)
        {
            // only work on .xhp - files
            SAL_WARN("helpcompiler",
                "ERROR: input list entry '"
                    << xhpFileName
                    << "' has the wrong extension (only files with extension .xhp "
                    << "are accepted)");

            continue;
        }

        fs::path langsourceRoot(sourceRoot);
        fs::path xhpFile;

        if( bExtensionMode )
        {
            // langsourceRoot == sourceRoot for extensions
            std::string xhpFileNameComplete( extensionPath );
            xhpFileNameComplete.append( '/' + xhpFileName );
            xhpFile = fs::path( xhpFileNameComplete );
        }
        else
        {
            langsourceRoot.append('/' + lang + '/');
            xhpFile = fs::path(xhpFileName, fs::native);
        }

        HelpCompiler hc( streamTable, xhpFile, langsourceRoot,
            embeddStylesheet, module, lang, bExtensionMode );

        HCDBG(std::cerr << "before compile of " << xhpFileName << std::endl);
        bool success = hc.compile();
        HCDBG(std::cerr << "after compile of " << xhpFileName << std::endl);

        if (!success && !bExtensionMode)
        {
            std::stringstream aStrStream;
            aStrStream <<
                "\nERROR: compiling help particle '"
                    << xhpFileName
                    << "' for language '"
                    << lang
                    << "' failed!";
            throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
        }

        std::string documentPath = streamTable.document_path;
        if (documentPath.find("/") == 0)
            documentPath = documentPath.substr(1);

        std::string documentJarfile = streamTable.document_module + ".jar";

        std::string documentTitle = streamTable.document_title;
        if (documentTitle.empty())
            documentTitle = "<notitle>";

        const std::string& fileB = documentPath;
        const std::string& jarfileB = documentJarfile;
        std::string& titleB = documentTitle;

        // add once this as its own id.
        addBookmark( pFileDbBase_DBHelp, documentPath, fileB, std::string(), jarfileB, titleB);

        const HashSet *hidlist = streamTable.appl_hidlist;
        if (!hidlist)
            hidlist = streamTable.default_hidlist;
        if (hidlist && !hidlist->empty())
        {
            // now iterate over all elements of the hidlist
            HashSet::const_iterator aEnd = hidlist->end();
            for (HashSet::const_iterator hidListIter = hidlist->begin();
                hidListIter != aEnd; ++hidListIter)
            {
                std::string thishid = *hidListIter;

                std::string anchorB;
                size_t index = thishid.rfind('#');
                if (index != std::string::npos)
                {
                    anchorB = thishid.substr(1 + index);
                    thishid = thishid.substr(0, index);
                }
                addBookmark( pFileDbBase_DBHelp, thishid, fileB, anchorB, jarfileB, titleB);
            }
        }

        // now the keywords
        const Hashtable *anchorToLL = streamTable.appl_keywords;
        if (!anchorToLL)
            anchorToLL = streamTable.default_keywords;
        if (anchorToLL && !anchorToLL->empty())
        {
            std::string fakedHid = URLEncoder::encode(documentPath);
            Hashtable::const_iterator aEnd = anchorToLL->end();
            for (Hashtable::const_iterator enumer = anchorToLL->begin();
                enumer != aEnd; ++enumer)
            {
                const std::string &anchor = enumer->first;
                addBookmark(pFileDbBase_DBHelp, documentPath, fileB,
                            anchor, jarfileB, titleB);
                std::string totalId = fakedHid + "#" + anchor;
                // std::cerr << hzipFileName << std::endl;
                const LinkedList& ll = enumer->second;
                LinkedList::const_iterator aOtherEnd = ll.end();
                for (LinkedList::const_iterator llIter = ll.begin();
                    llIter != aOtherEnd; ++llIter)
                {
                        helpKeyword.insert(*llIter, totalId);
                }
            }

        }

        // and last the helptexts
        const Stringtable *helpTextHash = streamTable.appl_helptexts;
        if (!helpTextHash)
            helpTextHash = streamTable.default_helptexts;
        if (helpTextHash && !helpTextHash->empty())
        {
            Stringtable::const_iterator aEnd = helpTextHash->end();
            for (Stringtable::const_iterator helpTextIter = helpTextHash->begin();
                helpTextIter != aEnd; ++helpTextIter)
            {
                std::string helpTextId = helpTextIter->first;
                const std::string& helpTextText = helpTextIter->second;

                helpTextId = URLEncoder::encode(helpTextId);

                if( pFileHelpText_DBHelp != NULL )
                    writeKeyValue_DBHelp( pFileHelpText_DBHelp, helpTextId, helpTextText );
            }
        }

        //IndexerPreProcessor
        if( !bExtensionMode || bIndexForExtension )
        {
            // now the indexing
            xmlDocPtr document = streamTable.appl_doc;
            if (!document)
                document = streamTable.default_doc;
            if (document)
            {
                std::string temp = module;
                std::transform (temp.begin(), temp.end(), temp.begin(), tocharlower);
                m_pIndexerPreProcessor->processDocument(document, URLEncoder::encode(documentPath) );
            }
        }

    } // while loop over hzip files ending

    } // try
    catch( const HelpProcessingException& )
    {
        // catch HelpProcessingException to avoid locking data bases
        if( pFileHelpText_DBHelp != NULL )
            fclose( pFileHelpText_DBHelp );
        if( pFileDbBase_DBHelp != NULL )
            fclose( pFileDbBase_DBHelp );
        throw;
    }

    if( pFileHelpText_DBHelp != NULL )
        fclose( pFileHelpText_DBHelp );
    if( pFileDbBase_DBHelp != NULL )
        fclose( pFileDbBase_DBHelp );

    helpKeyword.dump_DBHelp( keyWordFileName_DBHelp);

    if( !bExtensionMode )
    {
        // New index
        Stringtable::iterator aEnd = additionalFiles.end();
        for (Stringtable::iterator enumer = additionalFiles.begin(); enumer != aEnd;
            ++enumer)
        {
            const std::string &additionalFileName = enumer->second;
            const std::string &additionalFileKey = enumer->first;

            fs::path fsAdditionalFileName( additionalFileName, fs::native );
            HCDBG({
                    std::string aNativeStr = fsAdditionalFileName.native_file_string();
                    const char* pStr = aNativeStr.c_str();
                    std::cerr << pStr << std::endl;
            });

            fs::path fsTargetName( indexDirParentName / additionalFileKey );

            fs::copy( fsAdditionalFileName, fsTargetName );
        }
    }
}


void HelpLinker::main( std::vector<std::string> &args,
                       std::string* pExtensionPath, std::string* pDestination,
                       const rtl::OUString* pOfficeHelpPath )
    throw( HelpProcessingException )
{
    bExtensionMode = false;
    helpFiles.clear();

    if ((!args.empty()) && args[0][0] == '@')
    {
        std::vector<std::string> stringList;
        std::ifstream fileReader(args[0].substr(1).c_str());

        while (fileReader)
        {
            std::string token;
            fileReader >> token;
            if (!token.empty())
                stringList.push_back(token);
        }
        fileReader.close();

        args = stringList;
    }

    size_t i = 0;
    bool bSrcOption = false;
    while (i < args.size())
    {
        if (args[i].compare("-extlangsrc") == 0)
        {
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "extension source missing" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
            }
            extsource = args[i];
        }
        else if (args[i].compare("-extlangdest") == 0)
        {
            //If this argument is not provided then the location provided in -extsource will
            //also be the destination
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "extension destination missing" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
            }
            extdestination = args[i];
        }
        else if (args[i].compare("-src") == 0)
        {
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "sourceroot missing" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
            }
            bSrcOption = true;
            sourceRoot = fs::path(args[i], fs::native);
        }
        else if (args[i].compare("-sty") == 0)
        {
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "embeddingStylesheet missing" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
            }

            embeddStylesheet = fs::path(args[i], fs::native);
        }
        else if (args[i].compare("-zipdir") == 0)
        {
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "idxtemp missing" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
            }

            zipdir = fs::path(args[i], fs::native);
        }
        else if (args[i].compare("-idxcaption") == 0)
        {
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "idxcaption stylesheet missing" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
            }

            idxCaptionStylesheet = fs::path(args[i], fs::native);
        }
        else if (args[i].compare("-idxcontent") == 0)
        {
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "idxcontent stylesheet missing" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
            }

            idxContentStylesheet = fs::path(args[i], fs::native);
        }
        else if (args[i].compare("-o") == 0)
        {
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "outputfilename missing" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
            }

            outputFile = fs::path(args[i], fs::native);
        }
        else if (args[i].compare("-mod") == 0)
        {
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "module name missing" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
            }

            module = args[i];
        }
        else if (args[i].compare("-lang") == 0)
        {
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "language name missing" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
            }

            lang = args[i];
        }
        else if (args[i].compare("-hid") == 0)
        {
            ++i;
            throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, "obsolete -hid argument used" );
        }
        else if (args[i].compare("-add") == 0)
        {
            std::string addFile, addFileUnderPath;
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "pathname missing" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
            }

            addFileUnderPath = args[i];
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "pathname missing" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
            }
            addFile = args[i];
            if (!addFileUnderPath.empty() && !addFile.empty())
                additionalFiles[addFileUnderPath] = addFile;
        }
        else
            helpFiles.push_back(args[i]);
        ++i;
    }

    //We can be called from the helplinker executable or the extension manager
    //In the latter case extsource is not used.
    if( (pExtensionPath && pExtensionPath->length() > 0 && pOfficeHelpPath)
        || !extsource.empty())
    {
        bExtensionMode = true;
        if (!extsource.empty())
        {
            //called from helplinker.exe, pExtensionPath and pOfficeHelpPath
            //should be NULL
            sourceRoot = fs::path(extsource, fs::native);
            extensionPath = sourceRoot.toUTF8();

            if (extdestination.empty())
            {
                std::stringstream aStrStream;
                aStrStream << "-extlangdest is missing" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
            }
            else
            {
                //Convert from system path to file URL!!!
                fs::path p(extdestination, fs::native);
                extensionDestination = p.toUTF8();
            }
        }
        else
        { //called from extension manager
            extensionPath = *pExtensionPath;
            sourceRoot = fs::path(extensionPath);
            extensionDestination = *pDestination;
        }
        //check if -src option was used. This option must not be used
        //when extension help is compiled.
        if (bSrcOption)
        {
            std::stringstream aStrStream;
            aStrStream << "-src must not be used together with -extsource missing" << std::endl;
            throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
        }
    }

    if (!bExtensionMode && zipdir.empty())
    {
        std::stringstream aStrStream;
        aStrStream << "no index dir given" << std::endl;
        throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
    }

    if ( (!bExtensionMode && idxCaptionStylesheet.empty())
        || (!extsource.empty() && idxCaptionStylesheet.empty()) )
    {
        //No extension mode and extension mode using commandline
        //!extsource.empty indicates extension mode using commandline
        // -idxcaption paramter is required
        std::stringstream aStrStream;
        aStrStream << "no index caption stylesheet given" << std::endl;
        throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
    }
    else if ( bExtensionMode &&  extsource.empty())
    {
        //This part is used when compileExtensionHelp is called from the extensions manager.
        //If extension help is compiled using helplinker in the build process
        rtl::OUString aIdxCaptionPathFileURL( *pOfficeHelpPath );
        aIdxCaptionPathFileURL += rtl::OUString("/idxcaption.xsl");

        rtl::OString aOStr_IdxCaptionPathFileURL( rtl::OUStringToOString
            ( aIdxCaptionPathFileURL, fs::getThreadTextEncoding() ) );
        std::string aStdStr_IdxCaptionPathFileURL( aOStr_IdxCaptionPathFileURL.getStr() );

        idxCaptionStylesheet = fs::path( aStdStr_IdxCaptionPathFileURL );
    }

    if ( (!bExtensionMode && idxContentStylesheet.empty())
        || (!extsource.empty() && idxContentStylesheet.empty()) )
    {
        //No extension mode and extension mode using commandline
        //!extsource.empty indicates extension mode using commandline
        // -idxcontent paramter is required
        std::stringstream aStrStream;
        aStrStream << "no index content stylesheet given" << std::endl;
        throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
    }
    else if ( bExtensionMode && extsource.empty())
    {
        //If extension help is compiled using helplinker in the build process
        //then  -idxcontent must be supplied
        //This part is used when compileExtensionHelp is called from the extensions manager.
        rtl::OUString aIdxContentPathFileURL( *pOfficeHelpPath );
        aIdxContentPathFileURL += rtl::OUString("/idxcontent.xsl");

        rtl::OString aOStr_IdxContentPathFileURL( rtl::OUStringToOString
            ( aIdxContentPathFileURL, fs::getThreadTextEncoding() ) );
        std::string aStdStr_IdxContentPathFileURL( aOStr_IdxContentPathFileURL.getStr() );

        idxContentStylesheet = fs::path( aStdStr_IdxContentPathFileURL );
    }
    if (!bExtensionMode && embeddStylesheet.empty())
    {
        std::stringstream aStrStream;
        aStrStream << "no embedding resolving file given" << std::endl;
        throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
    }
    if (sourceRoot.empty())
    {
        std::stringstream aStrStream;
        aStrStream << "no sourceroot given" << std::endl;
        throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
    }
    if (!bExtensionMode && outputFile.empty())
    {
        std::stringstream aStrStream;
        aStrStream << "no output file given" << std::endl;
        throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
    }
    if (module.empty())
    {
        std::stringstream aStrStream;
        aStrStream << "module missing" << std::endl;
        throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
    }
    if (!bExtensionMode && lang.empty())
    {
        std::stringstream aStrStream;
        aStrStream << "language missing" << std::endl;
        throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
    }
    link();
}

// Variable to set an exception in "C" StructuredXMLErrorFunction
static const HelpProcessingException* GpXMLParsingException = NULL;

extern "C" void StructuredXMLErrorFunction(void *userData, xmlErrorPtr error)
{
    (void)userData;
    (void)error;

    std::string aErrorMsg = error->message;
    std::string aXMLParsingFile;
    if( error->file != NULL )
        aXMLParsingFile = error->file;
    int nXMLParsingLine = error->line;
    HelpProcessingException* pException = new HelpProcessingException( aErrorMsg, aXMLParsingFile, nXMLParsingLine );
    GpXMLParsingException = pException;

    // Reset error handler
    xmlSetStructuredErrorFunc( NULL, NULL );
}

HelpProcessingErrorInfo& HelpProcessingErrorInfo::operator=( const struct HelpProcessingException& e )
{
    m_eErrorClass = e.m_eErrorClass;
    rtl::OString tmpErrorMsg( e.m_aErrorMsg.c_str() );
    m_aErrorMsg = rtl::OStringToOUString( tmpErrorMsg, fs::getThreadTextEncoding() );
    rtl::OString tmpXMLParsingFile( e.m_aXMLParsingFile.c_str() );
    m_aXMLParsingFile = rtl::OStringToOUString( tmpXMLParsingFile, fs::getThreadTextEncoding() );
    m_nXMLParsingLine = e.m_nXMLParsingLine;
    return *this;
}


// Returns true in case of success, false in case of error
HELPLINKER_DLLPUBLIC bool compileExtensionHelp
(
    const rtl::OUString& aOfficeHelpPath,
    const rtl::OUString& aExtensionName,
    const rtl::OUString& aExtensionLanguageRoot,
    sal_Int32 nXhpFileCount, const rtl::OUString* pXhpFiles,
    const rtl::OUString& aDestination,
    HelpProcessingErrorInfo& o_rHelpProcessingErrorInfo
)
{
    bool bSuccess = true;

    std::vector<std::string> args;
    args.reserve(nXhpFileCount + 2);
    args.push_back(std::string("-mod"));
    rtl::OString aOExtensionName = rtl::OUStringToOString( aExtensionName, fs::getThreadTextEncoding() );
    args.push_back(std::string(aOExtensionName.getStr()));

    for( sal_Int32 iXhp = 0 ; iXhp < nXhpFileCount ; ++iXhp )
    {
        rtl::OUString aXhpFile = pXhpFiles[iXhp];

        rtl::OString aOXhpFile = rtl::OUStringToOString( aXhpFile, fs::getThreadTextEncoding() );
        args.push_back(std::string(aOXhpFile.getStr()));
    }

    rtl::OString aOExtensionLanguageRoot = rtl::OUStringToOString( aExtensionLanguageRoot, fs::getThreadTextEncoding() );
    const char* pExtensionPath = aOExtensionLanguageRoot.getStr();
    std::string aStdStrExtensionPath = pExtensionPath;
    rtl::OString aODestination = rtl::OUStringToOString(aDestination, fs::getThreadTextEncoding());
    const char* pDestination = aODestination.getStr();
    std::string aStdStrDestination = pDestination;

    // Set error handler
    xmlSetStructuredErrorFunc( NULL, (xmlStructuredErrorFunc)StructuredXMLErrorFunction );
    try
    {
        HelpLinker* pHelpLinker = new HelpLinker();
        pHelpLinker->main( args, &aStdStrExtensionPath, &aStdStrDestination, &aOfficeHelpPath );
        delete pHelpLinker;
    }
    catch( const HelpProcessingException& e )
    {
        if( GpXMLParsingException != NULL )
        {
            o_rHelpProcessingErrorInfo = *GpXMLParsingException;
            delete GpXMLParsingException;
            GpXMLParsingException = NULL;
        }
        else
        {
            o_rHelpProcessingErrorInfo = e;
        }
        bSuccess = false;
    }
    // Reset error handler
    xmlSetStructuredErrorFunc( NULL, NULL );

    // i83624: Tree files
    ::rtl::OUString aTreeFileURL = aExtensionLanguageRoot;
    aTreeFileURL += rtl::OUString("/help.tree");
    osl::DirectoryItem aTreeFileItem;
    osl::FileBase::RC rcGet = osl::DirectoryItem::get( aTreeFileURL, aTreeFileItem );
    osl::FileStatus aFileStatus( osl_FileStatus_Mask_FileSize );
    if( rcGet == osl::FileBase::E_None &&
        aTreeFileItem.getFileStatus( aFileStatus ) == osl::FileBase::E_None &&
        aFileStatus.isValid( osl_FileStatus_Mask_FileSize ) )
    {
        sal_uInt64 ret, len = aFileStatus.getFileSize();
        char* s = new char[ int(len) ];  // the buffer to hold the installed files
        osl::File aFile( aTreeFileURL );
        aFile.open( osl_File_OpenFlag_Read );
        aFile.read( s, len, ret );
        aFile.close();

        XML_Parser parser = XML_ParserCreate( 0 );
        XML_Status parsed = XML_Parse( parser, s, int( len ), true );

        if (XML_STATUS_ERROR == parsed)
        {
            XML_Error nError = XML_GetErrorCode( parser );
            o_rHelpProcessingErrorInfo.m_eErrorClass = HELPPROCESSING_XMLPARSING_ERROR;
            o_rHelpProcessingErrorInfo.m_aErrorMsg = rtl::OUString::createFromAscii( XML_ErrorString( nError ) );;
            o_rHelpProcessingErrorInfo.m_aXMLParsingFile = aTreeFileURL;
            // CRAHSES!!! o_rHelpProcessingErrorInfo.m_nXMLParsingLine = XML_GetCurrentLineNumber( parser );
            bSuccess = false;
        }

        XML_ParserFree( parser );
        delete[] s;
    }

    return bSuccess;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
