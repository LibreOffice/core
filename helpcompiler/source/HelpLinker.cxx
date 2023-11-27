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

#include <HelpCompiler.hxx>
#include <HelpLinker.hxx>

#include <algorithm>
#include <fstream>

#include <string.h>

#include <libxslt/transform.h>

#include <sal/types.h>
#include <o3tl/char16_t2wchar_t.hxx>
#include <sal/log.hxx>

#include <expat.h>
#include <memory>

namespace {
FILE* fopen_impl(const fs::path& rPath, const char* szMode)
{
#ifdef _WIN32     //We need _wfopen to support long file paths on Windows XP
    return _wfopen(rPath.native_file_string_w().c_str(), o3tl::toW(OUString::createFromAscii(szMode).getStr()));
#else
    return fopen(rPath.native_file_string().c_str(), szMode);
#endif
}
}

IndexerPreProcessor::IndexerPreProcessor
    ( const fs::path& fsIndexBaseDir,
      const fs::path& idxCaptionStylesheet, const fs::path& idxContentStylesheet )
{
    m_fsCaptionFilesDirName = fsIndexBaseDir / "caption";
    fs::create_directory( m_fsCaptionFilesDirName );

    m_fsContentFilesDirName = fsIndexBaseDir / "content";
    fs::create_directory( m_fsContentFilesDirName );

    m_xsltStylesheetPtrCaption = xsltParseStylesheetFile
        (reinterpret_cast<const xmlChar *>(idxCaptionStylesheet.native_file_string().c_str()));
    m_xsltStylesheetPtrContent = xsltParseStylesheetFile
        (reinterpret_cast<const xmlChar *>(idxContentStylesheet.native_file_string().c_str()));
}

IndexerPreProcessor::~IndexerPreProcessor()
{
    if( m_xsltStylesheetPtrCaption )
        xsltFreeStylesheet( m_xsltStylesheetPtrCaption );
    if( m_xsltStylesheetPtrContent )
        xsltFreeStylesheet( m_xsltStylesheetPtrContent );
}

static std::string getEncodedPath( const std::string& Path )
{
    OString aOStr_Path( Path.c_str() );
    OUString aOUStr_Path( OStringToOUString
        ( aOStr_Path, fs::getThreadTextEncoding() ) );
    OUString aPathURL;
    osl::File::getFileURLFromSystemPath( aOUStr_Path, aPathURL );
    OString aOStr_PathURL( OUStringToOString
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
        xmlDocPtr resCaption = xsltApplyStylesheet( m_xsltStylesheetPtrCaption, doc, nullptr );
        xmlNodePtr pResNodeCaption = resCaption->xmlChildrenNode;
        if( pResNodeCaption )
        {
            fs::path fsCaptionPureTextFile_docURL = m_fsCaptionFilesDirName / aStdStr_EncodedDocPathURL;
            FILE* pFile_docURL = fopen_impl( fsCaptionPureTextFile_docURL, "w" );
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
        xmlDocPtr resContent = xsltApplyStylesheet( m_xsltStylesheetPtrContent, doc, nullptr );
        xmlNodePtr pResNodeContent = resContent->xmlChildrenNode;
        if( pResNodeContent )
        {
            fs::path fsContentPureTextFile_docURL = m_fsContentFilesDirName / aStdStr_EncodedDocPathURL;
            FILE* pFile_docURL = fopen_impl( fsContentPureTextFile_docURL, "w" );
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

    void append(const std::string &id)
    {
        _idList.push_back(id);
    }

    std::string getString() const
    {
        std::string ret;
        for (auto const& elem : _idList)
            ret += elem + ";";
        return ret;
    }
};

static void writeKeyValue_DBHelp( FILE* pFile, const std::string& aKeyStr, const std::string& aValueStr )
{
    if( pFile == nullptr )
        return;
    char const cLF = 10;
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
    typedef std::unordered_map<std::string, Data> DataHashtable;
    DataHashtable _hash;

public:
    void insert(const std::string &key, const std::string &id)
    {
        Data &data = _hash[key];
        data.append(id);
    }

    void dump_DBHelp( const fs::path& rFileName )
    {
        FILE* pFile = fopen_impl( rFileName, "wb" );
        if( pFile == nullptr )
            return;

        for (auto const& elem : _hash)
            writeKeyValue_DBHelp( pFile, elem.first, elem.second.getString() );

        fclose( pFile );
    }
};

namespace URLEncoder
{
    static std::string encode(const std::string &rIn)
    {
        const char * const good = "!$&'()*+,-.=@_";
        static const char hex[17] = "0123456789ABCDEF";

        std::string result;
        for (char c : rIn)
        {
            if (rtl::isAsciiAlphanumeric (static_cast<unsigned char>(c))
                || strchr (good, c))
            {
                result += c;
            } else {
                result += '%';
                result += hex[static_cast<unsigned char>(c) >> 4];
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
    for (char j : fileB)
        dataB[i++] = static_cast<unsigned char>(j);
    if (!anchorB.empty())
    {
        dataB[i++] = '#';
        for (char j : anchorB)
            dataB[i++] = j;
    }
    dataB[i++] = static_cast<unsigned char>(jarfileB.length());
    for (char j : jarfileB)
        dataB[i++] = j;

    dataB[i++] = static_cast<unsigned char>(titleB.length());
    for (char j : titleB)
        dataB[i++] = j;

    if( pFile_DBHelp != nullptr )
    {
        std::string aValueStr( dataB.begin(), dataB.end() );
        writeKeyValue_DBHelp( pFile_DBHelp, thishid, aValueStr );
    }
}

void HelpLinker::initIndexerPreProcessor()
{
    m_pIndexerPreProcessor.reset( new IndexerPreProcessor( indexDirParentName,
         idxCaptionStylesheet, idxContentStylesheet ) );
}

void HelpLinker::link()
{

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
    bool bUse_ = true;
    if( !bExtensionMode )
        bUse_ = false;

    fs::path helpTextFileName_DBHelp(indexDirParentName / (mod + (bUse_ ? ".ht_" : ".ht")));
    FILE* pFileHelpText_DBHelp = fopen_impl( helpTextFileName_DBHelp, "wb" );

    fs::path dbBaseFileName_DBHelp(indexDirParentName / (mod + (bUse_ ? ".db_" : ".db")));
    FILE* pFileDbBase_DBHelp = fopen_impl( dbBaseFileName_DBHelp, "wb" );

    fs::path keyWordFileName_DBHelp(indexDirParentName / (mod + (bUse_ ? ".key_" : ".key")));

    HelpKeyword helpKeyword;

    // catch HelpProcessingException to avoid locking data bases
    try
    {
        bool bIndexForExtension = true;
        // lastly, initialize the indexBuilder
        if ( (!bExtensionMode || bIndexForExtension) && !helpFiles.empty())
            initIndexerPreProcessor();

        // here we start our loop over the hzip files.
        for (auto const& helpFile : helpFiles)
        {
            // process one file
            // streamTable contains the streams in the hzip file
            StreamTable streamTable;
            const std::string &xhpFileName = helpFile;

            if (!bExtensionMode && xhpFileName.rfind(".xhp") != xhpFileName.length()-4)
            {
                // only work on .xhp - files
                SAL_WARN("helpcompiler",
                    "ERROR: input list entry '"
                        << xhpFileName
                        << "' has the wrong extension (only files with extension .xhp are accepted)");

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
                langsourceRoot.append( "/" );
                if ( m_bUseLangRoot )
                    langsourceRoot.append( lang + '/' );
                xhpFile = fs::path(xhpFileName, fs::native);
            }

            HelpCompiler hc( streamTable, xhpFile, langsourceRoot, zipdir,
                compactStylesheet, embeddStylesheet, module, lang, bExtensionMode );

            HCDBG(std::cerr << "before compile of " << xhpFileName << std::endl);
            hc.compile();
            HCDBG(std::cerr << "after compile of " << xhpFileName << std::endl);

            if (!m_bCreateIndex)
                continue;

            std::string documentPath = streamTable.document_path;
            if (documentPath.compare(0, 1, "/") == 0)
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

            const std::vector<std::string> *hidlist = streamTable.appl_hidlist.get();
            if (hidlist)
            {
                // now iterate over all elements of the hidlist
                for (auto & elem : *hidlist)
                {
                    std::string thishid = elem;

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
            const Hashtable *anchorToLL = streamTable.appl_keywords.get();
            if (anchorToLL && !anchorToLL->empty())
            {
                std::string fakedHid = URLEncoder::encode(documentPath);
                for (auto const& elemAnchor : *anchorToLL)
                {
                    const std::string &anchor = elemAnchor.first;
                    addBookmark(pFileDbBase_DBHelp, documentPath, fileB,
                                anchor, jarfileB, titleB);
                    std::string totalId = fakedHid + "#" + anchor;
                    // std::cerr << hzipFileName << std::endl;
                    const LinkedList& ll = elemAnchor.second;
                    for (auto const& elem : ll)
                    {
                            helpKeyword.insert(elem, totalId);
                    }
                }

            }

            // and last the helptexts
            const Stringtable *helpTextHash = streamTable.appl_helptexts.get();
            if (helpTextHash)
            {
                for (auto const& elem : *helpTextHash)
                {
                    std::string helpTextId = elem.first;
                    const std::string& helpTextText = elem.second;

                    helpTextId = URLEncoder::encode(helpTextId);

                    if( pFileHelpText_DBHelp != nullptr )
                        writeKeyValue_DBHelp( pFileHelpText_DBHelp, helpTextId, helpTextText );
                }
            }

            //IndexerPreProcessor
            if( !bExtensionMode || bIndexForExtension )
            {
                // now the indexing
                xmlDocPtr document = streamTable.appl_doc;
                if (document)
                {
                    std::string temp = module;
                    std::transform (temp.begin(), temp.end(), temp.begin(), tocharlower);
                    m_pIndexerPreProcessor->processDocument(document, URLEncoder::encode(documentPath) );
                }
            }

        }

    }
    catch( const HelpProcessingException& )
    {
        // catch HelpProcessingException to avoid locking data bases
        if( pFileHelpText_DBHelp != nullptr )
            fclose( pFileHelpText_DBHelp );
        if( pFileDbBase_DBHelp != nullptr )
            fclose( pFileDbBase_DBHelp );
        throw;
    }

    if( pFileHelpText_DBHelp != nullptr )
        fclose( pFileHelpText_DBHelp );
    if( pFileDbBase_DBHelp != nullptr )
        fclose( pFileDbBase_DBHelp );

    helpKeyword.dump_DBHelp( keyWordFileName_DBHelp);

    if( !bExtensionMode )
    {
        // New index
        for (auto const& additionalFile : additionalFiles)
        {
            const std::string &additionalFileName = additionalFile.second;
            const std::string &additionalFileKey = additionalFile.first;

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
                       std::string const * pExtensionPath, std::string const * pDestination,
                       const OUString* pOfficeHelpPath )
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
                throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
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
                throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
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
                throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
            }
            bSrcOption = true;
            sourceRoot = fs::path(args[i], fs::native);
        }
        else if (args[i].compare("-compact") == 0)
        {
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "compactStylesheet missing" << std::endl;
                throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
            }

            compactStylesheet = fs::path(args[i], fs::native);
        }
        else if (args[i].compare("-sty") == 0)
        {
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "embeddingStylesheet missing" << std::endl;
                throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
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
                throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
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
                throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
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
                throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
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
                throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
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
                throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
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
                throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
            }

            lang = args[i];
        }
        else if (args[i].compare("-hid") == 0)
        {
            ++i;
            throw HelpProcessingException( HelpProcessingErrorClass::General, "obsolete -hid argument used" );
        }
        else if (args[i].compare("-add") == 0)
        {
            std::string addFile, addFileUnderPath;
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "pathname missing" << std::endl;
                throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
            }

            addFileUnderPath = args[i];
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "pathname missing" << std::endl;
                throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
            }
            addFile = args[i];
            if (!addFileUnderPath.empty() && !addFile.empty())
                additionalFiles[addFileUnderPath] = addFile;
        }
        else if (args[i].compare("-nolangroot") == 0)
            m_bUseLangRoot = false;
        else if (args[i].compare("-noindex") == 0)
            m_bCreateIndex = false;
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
                throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
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
            throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
        }
    }

    if (!bExtensionMode && zipdir.empty())
    {
        std::stringstream aStrStream;
        aStrStream << "no index dir given" << std::endl;
        throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
    }

    if ( (!bExtensionMode && idxCaptionStylesheet.empty())
        || (!extsource.empty() && idxCaptionStylesheet.empty()) )
    {
        //No extension mode and extension mode using commandline
        //!extsource.empty indicates extension mode using commandline
        // -idxcaption parameter is required
        std::stringstream aStrStream;
        aStrStream << "no index caption stylesheet given" << std::endl;
        throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
    }
    else if ( bExtensionMode &&  extsource.empty())
    {
        //This part is used when compileExtensionHelp is called from the extensions manager.
        //If extension help is compiled using helplinker in the build process
        OUString aIdxCaptionPathFileURL = *pOfficeHelpPath + "/idxcaption.xsl";

        OString aOStr_IdxCaptionPathFileURL( OUStringToOString
            ( aIdxCaptionPathFileURL, fs::getThreadTextEncoding() ) );
        std::string aStdStr_IdxCaptionPathFileURL( aOStr_IdxCaptionPathFileURL.getStr() );

        idxCaptionStylesheet = fs::path( aStdStr_IdxCaptionPathFileURL );
    }

    if ( (!bExtensionMode && idxContentStylesheet.empty())
        || (!extsource.empty() && idxContentStylesheet.empty()) )
    {
        //No extension mode and extension mode using commandline
        //!extsource.empty indicates extension mode using commandline
        // -idxcontent parameter is required
        std::stringstream aStrStream;
        aStrStream << "no index content stylesheet given" << std::endl;
        throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
    }
    else if ( bExtensionMode && extsource.empty())
    {
        //If extension help is compiled using helplinker in the build process
        //then  -idxcontent must be supplied
        //This part is used when compileExtensionHelp is called from the extensions manager.
        OUString aIdxContentPathFileURL = *pOfficeHelpPath + "/idxcontent.xsl";

        OString aOStr_IdxContentPathFileURL( OUStringToOString
            ( aIdxContentPathFileURL, fs::getThreadTextEncoding() ) );
        std::string aStdStr_IdxContentPathFileURL( aOStr_IdxContentPathFileURL.getStr() );

        idxContentStylesheet = fs::path( aStdStr_IdxContentPathFileURL );
    }
    if (!bExtensionMode && embeddStylesheet.empty())
    {
        std::stringstream aStrStream;
        aStrStream << "no embedding resolving file given" << std::endl;
        throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
    }
    if (sourceRoot.empty())
    {
        std::stringstream aStrStream;
        aStrStream << "no sourceroot given" << std::endl;
        throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
    }
    if (!bExtensionMode && outputFile.empty())
    {
        std::stringstream aStrStream;
        aStrStream << "no output file given" << std::endl;
        throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
    }
    if (module.empty())
    {
        std::stringstream aStrStream;
        aStrStream << "module missing" << std::endl;
        throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
    }
    if (!bExtensionMode && lang.empty())
    {
        std::stringstream aStrStream;
        aStrStream << "language missing" << std::endl;
        throw HelpProcessingException( HelpProcessingErrorClass::General, aStrStream.str() );
    }
    link();
}

// Variable to set an exception in "C" StructuredXMLErrorFunction
static const HelpProcessingException* GpXMLParsingException = nullptr;

extern "C" {

#if LIBXML_VERSION >= 21200
static void StructuredXMLErrorFunction(SAL_UNUSED_PARAMETER void *, const xmlError* error)
#else
static void StructuredXMLErrorFunction(SAL_UNUSED_PARAMETER void *, xmlErrorPtr error)
#endif
{
    std::string aErrorMsg = error->message;
    std::string aXMLParsingFile;
    if( error->file != nullptr )
        aXMLParsingFile = error->file;
    int nXMLParsingLine = error->line;
    HelpProcessingException* pException = new HelpProcessingException( aErrorMsg, aXMLParsingFile, nXMLParsingLine );
    GpXMLParsingException = pException;

    // Reset error handler
    xmlSetStructuredErrorFunc( nullptr, nullptr );
}

}

HelpProcessingErrorInfo& HelpProcessingErrorInfo::operator=( const struct HelpProcessingException& e )
{
    m_eErrorClass = e.m_eErrorClass;
    OString tmpErrorMsg( e.m_aErrorMsg.c_str() );
    m_aErrorMsg = OStringToOUString( tmpErrorMsg, fs::getThreadTextEncoding() );
    OString tmpXMLParsingFile( e.m_aXMLParsingFile.c_str() );
    m_aXMLParsingFile = OStringToOUString( tmpXMLParsingFile, fs::getThreadTextEncoding() );
    m_nXMLParsingLine = e.m_nXMLParsingLine;
    return *this;
}


// Returns true in case of success, false in case of error
bool compileExtensionHelp
(
    const OUString& aOfficeHelpPath,
    const OUString& aExtensionName,
    const OUString& aExtensionLanguageRoot,
    sal_Int32 nXhpFileCount, const OUString* pXhpFiles,
    const OUString& aDestination,
    HelpProcessingErrorInfo& o_rHelpProcessingErrorInfo
)
{
    bool bSuccess = true;

    std::vector<std::string> args;
    args.reserve(nXhpFileCount + 2);
    args.push_back(std::string("-mod"));
    OString aOExtensionName = OUStringToOString( aExtensionName, fs::getThreadTextEncoding() );
    args.push_back(std::string(aOExtensionName.getStr()));

    for( sal_Int32 iXhp = 0 ; iXhp < nXhpFileCount ; ++iXhp )
    {
        OUString aXhpFile = pXhpFiles[iXhp];

        OString aOXhpFile = OUStringToOString( aXhpFile, fs::getThreadTextEncoding() );
        args.push_back(std::string(aOXhpFile.getStr()));
    }

    OString aOExtensionLanguageRoot = OUStringToOString( aExtensionLanguageRoot, fs::getThreadTextEncoding() );
    const char* pExtensionPath = aOExtensionLanguageRoot.getStr();
    std::string aStdStrExtensionPath = pExtensionPath;
    OString aODestination = OUStringToOString(aDestination, fs::getThreadTextEncoding());
    const char* pDestination = aODestination.getStr();
    std::string aStdStrDestination = pDestination;

    // Set error handler
    xmlSetStructuredErrorFunc( nullptr, StructuredXMLErrorFunction );
    try
    {
        std::unique_ptr<HelpLinker> pHelpLinker(new HelpLinker());
        pHelpLinker->main( args, &aStdStrExtensionPath, &aStdStrDestination, &aOfficeHelpPath );
    }
    catch( const HelpProcessingException& e )
    {
        if( GpXMLParsingException != nullptr )
        {
            o_rHelpProcessingErrorInfo = *GpXMLParsingException;
            delete GpXMLParsingException;
            GpXMLParsingException = nullptr;
        }
        else
        {
            o_rHelpProcessingErrorInfo = e;
        }
        bSuccess = false;
    }
    // Reset error handler
    xmlSetStructuredErrorFunc( nullptr, nullptr );

    // i83624: Tree files
    // The following basically checks if the help.tree is well formed XML.
    // Apparently there have been cases when translations contained
    // non-well-formed XML in the past.
    OUString aTreeFileURL = aExtensionLanguageRoot + "/help.tree";
    osl::DirectoryItem aTreeFileItem;
    osl::FileBase::RC rcGet = osl::DirectoryItem::get( aTreeFileURL, aTreeFileItem );
    osl::FileStatus aFileStatus( osl_FileStatus_Mask_FileSize );
    if( rcGet == osl::FileBase::E_None &&
        aTreeFileItem.getFileStatus( aFileStatus ) == osl::FileBase::E_None &&
        aFileStatus.isValid( osl_FileStatus_Mask_FileSize ) )
    {
        sal_uInt64 ret, len = aFileStatus.getFileSize();
        std::unique_ptr<char[]> s(new char[ int(len) ]);  // the buffer to hold the installed files
        osl::File aFile( aTreeFileURL );
        (void)aFile.open( osl_File_OpenFlag_Read );
        aFile.read( s.get(), len, ret );
        aFile.close();

        XML_Parser parser = XML_ParserCreate( nullptr );
        XML_Status parsed = XML_Parse( parser, s.get(), int( len ), true );

        if (XML_STATUS_ERROR == parsed)
        {
            XML_Error nError = XML_GetErrorCode( parser );
            o_rHelpProcessingErrorInfo.m_eErrorClass = HelpProcessingErrorClass::XmlParsing;
            o_rHelpProcessingErrorInfo.m_aErrorMsg = OUString::createFromAscii( XML_ErrorString( nError ) );
            o_rHelpProcessingErrorInfo.m_aXMLParsingFile = aTreeFileURL;
            // CRASHES!!! o_rHelpProcessingErrorInfo.m_nXMLParsingLine = XML_GetCurrentLineNumber( parser );
            bSuccess = false;
        }

        XML_ParserFree( parser );
    }

    return bSuccess;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
