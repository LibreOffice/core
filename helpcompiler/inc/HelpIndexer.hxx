/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Gert van Valkenhoef <g.h.m.van.valkenhoef@rug.nl>
 *  (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef HELPINDEXER_HXX
#define HELPINDEXER_HXX

#include <helpcompiler/dllapi.h>

#include <rtl/ustring.hxx>
#include <set>

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
		rtl::OUString d_lang;
		rtl::OUString d_module;
		rtl::OUString d_captionDir;
		rtl::OUString d_contentDir;
		rtl::OUString d_indexDir;
		rtl::OUString d_error;
		std::set<rtl::OUString> d_files;

	public:

	/**
	 * @param lang Help files language.
	 * @param module The module of the helpfiles.
	 * @param srcDir The help directory to index
	 * @param outDir The directory to write the "module".idxl directory to
	 */
	HelpIndexer(rtl::OUString const &lang, rtl::OUString const &module,
		rtl::OUString const &srcDir, rtl::OUString const &outDir);

	/**
	 * Run the indexer.
	 * @return true if index successfully generated.
	 */
	bool indexDocuments();

	/**
	 * Get the error string (empty if no error occurred).
	 */
	rtl::OUString const & getErrorMessage();

	private:

	/**
	 * Scan the caption & contents directories for help files.
	 */
	bool scanForFiles();

	/**
	 * Scan for files in the given directory.
	 */
	bool scanForFiles(rtl::OUString const &path);

	/**
	 * Fill the Document with information on the given help file.
	 */
	bool helpDocument(rtl::OUString const & fileName, lucene::document::Document *doc);

	/**
	 * Create a reader for the given file, and create an "empty" reader in case the file doesn't exist.
	 */
	lucene::util::Reader *helpFileReader(rtl::OUString const & path);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
