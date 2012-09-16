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

#ifndef HELPSEARCH_HXX
#define HELPSEARCH_HXX

#include <helpcompiler/dllapi.h>

#include <rtl/ustring.hxx>
#include <vector>

class L10N_DLLPUBLIC HelpSearch{
	private:
		rtl::OUString d_lang;
		rtl::OString d_indexDir;

	public:

	/**
	 * @param lang Help files language.
	 * @param indexDir The directory where the index files are stored.
	 */
	HelpSearch(rtl::OUString const &lang, rtl::OUString const &indexDir);

	/**
	 * Query the index for a certain query string.
	 * @param queryStr The query.
	 * @param captionOnly Set to true to search in the caption, not the content.
	 * @param rDocuments Vector to write the paths of the found documents.
	 * @param rScores Vector to write the scores to.
	 */
	bool query(rtl::OUString const &queryStr, bool captionOnly,
		std::vector<rtl::OUString> &rDocuments, std::vector<float> &rScores);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
