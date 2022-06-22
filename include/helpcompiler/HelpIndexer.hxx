/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_HELPCOMPILER_HELPINDEXER_HXX
#define INCLUDED_HELPCOMPILER_HELPINDEXER_HXX

#include <helpcompiler/dllapi.h>

#include <rtl/ustring.hxx>
#include <set>
#include <string_view>

// I assume that TCHAR is defined as wchar_t throughout

namespace lucene
{
namespace document
{
class Document;
}
namespace util
{
class Reader;
}
}

class L10N_DLLPUBLIC HelpIndexer {
    private:
                OUString d_lang;
                OUString d_module;
                OUString d_captionDir;
                OUString d_contentDir;
                OUString d_indexDir;
                OUString d_error;
                std::set<OUString> d_files;

    public:

    /**
     * @param lang Help files language.
     * @param module The module of the helpfiles.
     * @param srcDir The help directory to index
     * @param outDir The directory to write the "module".idxl directory to
     */
        HelpIndexer(OUString const &lang, OUString const &module,
            std::u16string_view srcDir, std::u16string_view outDir);

    /**
     * Run the indexer.
     * @return true if index successfully generated.
     */
    bool indexDocuments();

    /**
     * Get the error string (empty if no error occurred).
     */
    OUString const & getErrorMessage() const { return d_error;}

    private:

    /**
     * Scan the caption & contents directories for help files.
     */
    bool scanForFiles();

    /**
     * Scan for files in the given directory.
     */
    bool scanForFiles(OUString const &path);

    /**
     * Fill the Document with information on the given help file.
     */
    void helpDocument(OUString const & fileName, lucene::document::Document *doc) const;

    /**
     * Create a reader for the given file, and create an "empty" reader in case the file doesn't exist.
     */
    static lucene::util::Reader *helpFileReader(OUString const & path);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
