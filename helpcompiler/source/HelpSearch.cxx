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

#include <helpcompiler/HelpSearch.hxx>
#include <osl/file.hxx>
#include <osl/thread.hxx>

#include "LuceneHelper.hxx"

HelpSearch::HelpSearch(rtl::OUString const &lang, rtl::OUString const &indexDir)
    : d_lang(lang)
{
    rtl::OUString ustrSystemPath;
    osl::File::getSystemPathFromFileURL(indexDir, ustrSystemPath);
    d_indexDir = rtl::OUStringToOString(ustrSystemPath, osl_getThreadTextEncoding());
}

bool HelpSearch::query(rtl::OUString const &queryStr, bool captionOnly,
        std::vector<rtl::OUString> &rDocuments, std::vector<float> &rScores) {

    lucene::index::IndexReader *reader = lucene::index::IndexReader::open(d_indexDir.getStr());
    lucene::search::IndexSearcher searcher(reader);

    TCHAR captionField[] = L"caption";
    TCHAR contentField[] = L"content";
    TCHAR *field = captionOnly ? captionField : contentField;

    bool isWildcard = queryStr[queryStr.getLength() - 1] == L'*';
    std::vector<TCHAR> aQueryStr(OUStringToTCHARVec(queryStr));
    lucene::search::Query *pQuery;
    if (isWildcard)
        pQuery = _CLNEW lucene::search::WildcardQuery(_CLNEW lucene::index::Term(field, &aQueryStr[0]));
    else
        pQuery = _CLNEW lucene::search::TermQuery(_CLNEW lucene::index::Term(field, &aQueryStr[0]));

    lucene::search::Hits *hits = searcher.search(pQuery);
    for (unsigned i = 0; i < hits->length(); ++i) {
        lucene::document::Document &doc = hits->doc(i); // Document* belongs to Hits.
        wchar_t const *path = doc.get(L"path");
        rDocuments.push_back(TCHARArrayToOUString(path != 0 ? path : L""));
        rScores.push_back(hits->score(i));
    }

    _CLDELETE(hits);
    _CLDELETE(pQuery);

    reader->close();
    _CLDELETE(reader);

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
