/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef HELPSEARCH_HXX
#define HELPSEARCH_HXX

#include <helpcompiler/dllapi.h>

#include <rtl/ustring.hxx>
#include <vector>

class L10N_DLLPUBLIC HelpSearch{
	private:
                OUString d_lang;
                OString d_indexDir;

	public:

	/**
	 * @param lang Help files language.
	 * @param indexDir The directory where the index files are stored.
	 */
        HelpSearch(OUString const &lang, OUString const &indexDir);

	/**
	 * Query the index for a certain query string.
	 * @param queryStr The query.
	 * @param captionOnly Set to true to search in the caption, not the content.
	 * @param rDocuments Vector to write the paths of the found documents.
	 * @param rScores Vector to write the scores to.
	 */
        bool query(OUString const &queryStr, bool captionOnly,
            std::vector<OUString> &rDocuments, std::vector<float> &rScores);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
