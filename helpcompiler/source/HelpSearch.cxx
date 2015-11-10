/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <helpcompiler/HelpSearch.hxx>
#include <osl/file.hxx>
#include <osl/thread.hxx>

#include "LuceneHelper.hxx"

HelpSearch::HelpSearch(OUString const &indexDir)
{
    OUString ustrSystemPath;
    osl::File::getSystemPathFromFileURL(indexDir, ustrSystemPath);
    d_indexDir = OUStringToOString(ustrSystemPath, osl_getThreadTextEncoding());
}

bool HelpSearch::query(OUString const &queryStr, bool captionOnly,
        std::vector<OUString> &rDocuments, std::vector<float> &rScores) {

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
    for (size_t i = 0; i < hits->length(); ++i) {
        lucene::document::Document &doc = hits->doc(i); // Document* belongs to Hits.
        wchar_t const *path = doc.get(L"path");
        rDocuments.push_back(TCHARArrayToOUString(path != nullptr ? path : L""));
        rScores.push_back(hits->score(i));
    }

    _CLDELETE(hits);
    _CLDELETE(pQuery);

    reader->close();
    _CLDELETE(reader);

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
