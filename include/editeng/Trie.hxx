/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TRIE_HXX
#define TRIE_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <vector>
#include <editeng/editengdllapi.h>
#include <boost/scoped_ptr.hpp>

namespace editeng
{

struct TrieNode;

class EDITENG_DLLPUBLIC Trie
{
private:
    boost::scoped_ptr<TrieNode> mRoot;

public:
    Trie();
    virtual ~Trie();

    void insert(OUString sInputString) const;
    void findSuggestions(OUString sWordPart, std::vector<OUString>& rSuggesstionList) const;

};

}

#endif // TRIE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
