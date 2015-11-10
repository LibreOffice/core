/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include <svl/IndexedStyleSheets.hxx>
#include <svl/style.hxx>

#include <stdexcept>
#include <algorithm>
#include <utility>

using rtl::OUString;


namespace {
const size_t NUMBER_OF_FAMILIES = 6;
size_t family_to_index(SfxStyleFamily family)
{
    switch (family) {
    case SFX_STYLE_FAMILY_CHAR:
        return 0;
    case SFX_STYLE_FAMILY_PARA:
        return 1;
    case SFX_STYLE_FAMILY_FRAME:
        return 2;
    case SFX_STYLE_FAMILY_PAGE:
        return 3;
    case SFX_STYLE_FAMILY_PSEUDO:
        return 4;
    case SFX_STYLE_FAMILY_ALL:
        return 5;
    }
    assert(false); // only for compiler warning. all cases are handled in the switch
    return 0;
}
}

namespace svl {

IndexedStyleSheets::IndexedStyleSheets()
{
    for (size_t i = 0; i < NUMBER_OF_FAMILIES; i++) {
        mStyleSheetPositionsByFamily.push_back(std::vector<unsigned>());
    }
;}


void
IndexedStyleSheets::Register(const SfxStyleSheetBase& style, unsigned pos)
{
    mPositionsByName.insert(std::make_pair(style.GetName(), pos));
    size_t position = family_to_index(style.GetFamily());
    mStyleSheetPositionsByFamily.at(position).push_back(pos);
    size_t positionForFamilyAll = family_to_index(SFX_STYLE_FAMILY_ALL);
    mStyleSheetPositionsByFamily.at(positionForFamilyAll).push_back(pos);
}

void
IndexedStyleSheets::Reindex()
{
    mPositionsByName.clear();
    mStyleSheetPositionsByFamily.clear();
    for (size_t i = 0; i < NUMBER_OF_FAMILIES; i++) {
        mStyleSheetPositionsByFamily.push_back(std::vector<unsigned>());
    }

    unsigned i = 0;
    for (VectorType::const_iterator it = mStyleSheets.begin();
                                    it != mStyleSheets.end(); ++it) {
        SfxStyleSheetBase* p = it->get();
        Register(*p, i);
        ++i;
    }
}

unsigned
IndexedStyleSheets::GetNumberOfStyleSheets() const
{
    return mStyleSheets.size();
}

void
IndexedStyleSheets::AddStyleSheet(rtl::Reference< SfxStyleSheetBase > style)
{
    if (!HasStyleSheet(style)) {
        mStyleSheets.push_back(style);
        // since we just added an element to the vector, we can safely do -1 as it will always be >= 1
        Register(*style, mStyleSheets.size()-1);
    }
}

bool
IndexedStyleSheets::RemoveStyleSheet(rtl::Reference< SfxStyleSheetBase > style)
{
    rtl::OUString styleName = style->GetName();
    std::vector<unsigned> positions = FindPositionsByName(styleName);
    bool found = false;
    unsigned stylePosition = 0;
    for (std::vector<unsigned>::const_iterator it = positions.begin();
                                               it != positions.end(); ++it) {
        if (mStyleSheets.at(*it) == style) {
            found = true;
            stylePosition = *it;
            break;
        }
    }

    if (found) {
        mStyleSheets.erase(mStyleSheets.begin() + stylePosition);
        Reindex();
    }
    return found;
}

std::vector<unsigned>
IndexedStyleSheets::FindPositionsByName(const rtl::OUString& name) const
{
    std::vector<unsigned> r;
    std::pair<MapType::const_iterator, MapType::const_iterator> range = mPositionsByName.equal_range(name);
    for (MapType::const_iterator it = range.first; it != range.second; ++it) {
        r.push_back(it->second);
    }
    return r;
}

std::vector<unsigned>
IndexedStyleSheets::FindPositionsByNameAndPredicate(const rtl::OUString& name,
        StyleSheetPredicate& predicate, SearchBehavior behavior) const
{
    std::vector<unsigned> r;
    MapType::const_iterator it = mPositionsByName.find(name);
    for (/**/; it != mPositionsByName.end(); ++it) {
        unsigned pos = it->second;
        SfxStyleSheetBase *ssheet = mStyleSheets.at(pos).get();
        if (predicate.Check(*ssheet)) {
            r.push_back(pos);
            if (behavior == RETURN_FIRST) {
                break;
            }
        }
    }
    return r;
}


unsigned
IndexedStyleSheets::GetNumberOfStyleSheetsWithPredicate(StyleSheetPredicate& predicate) const
{
    unsigned r = 0;
    for (VectorType::const_iterator it = mStyleSheets.begin(); it != mStyleSheets.end(); ++it) {
        const SfxStyleSheetBase *ssheet = it->get();
        if (predicate.Check(*ssheet)) {
            ++r;
        }
    }
    return r;
}

rtl::Reference<SfxStyleSheetBase>
IndexedStyleSheets::GetNthStyleSheetThatMatchesPredicate(
        unsigned n,
        StyleSheetPredicate& predicate,
        unsigned startAt)
{
    rtl::Reference<SfxStyleSheetBase> retval;
    unsigned matching = 0;
    for (VectorType::iterator it = mStyleSheets.begin()+startAt; it != mStyleSheets.end(); ++it) {
        SfxStyleSheetBase *ssheet = it->get();
        if (predicate.Check(*ssheet)) {
            if (matching == n) {
                retval = *it;
                break;
            }
            ++matching;
        }
    }
    return retval;
}

unsigned
IndexedStyleSheets::FindStyleSheetPosition(const SfxStyleSheetBase& style) const
{
    VectorType::const_iterator it = std::find(mStyleSheets.begin(), mStyleSheets.end(), &style);
    if (it == mStyleSheets.end()) {
        throw std::runtime_error("IndexedStyleSheets::FindStylePosition Looked for style not in index");
    }
    return std::distance(mStyleSheets.begin(), it);
}

void
IndexedStyleSheets::Clear(StyleSheetDisposer& disposer)
{
    for (VectorType::iterator it = mStyleSheets.begin(); it != mStyleSheets.end(); ++it) {
        disposer.Dispose(*it);
    }
    mStyleSheets.clear();
    mPositionsByName.clear();
}

IndexedStyleSheets::~IndexedStyleSheets()
{;}

bool
IndexedStyleSheets::HasStyleSheet(rtl::Reference< SfxStyleSheetBase > style) const
{
    rtl::OUString styleName = style->GetName();
    std::vector<unsigned> positions = FindPositionsByName(styleName);
    for (std::vector<unsigned>::const_iterator it = positions.begin();
                                               it != positions.end(); ++it) {
        if (mStyleSheets.at(*it) == style) {
            return true;
        }
    }
    return false;
}

rtl::Reference< SfxStyleSheetBase >
IndexedStyleSheets::GetStyleSheetByPosition(unsigned pos)
{
    if( pos < mStyleSheets.size() )
        return mStyleSheets.at(pos);
    return nullptr;
}

void
IndexedStyleSheets::ApplyToAllStyleSheets(StyleSheetCallback& callback) const
{
    for (VectorType::const_iterator it = mStyleSheets.begin(); it != mStyleSheets.end(); ++it) {
        callback.DoIt(**it);
    }
}

std::vector<unsigned>
IndexedStyleSheets::FindPositionsByPredicate(StyleSheetPredicate& predicate) const
{
    std::vector<unsigned> r;
    for (VectorType::const_iterator it = mStyleSheets.begin(); it != mStyleSheets.end(); ++it) {
        if (predicate.Check(**it)) {
            r.push_back(std::distance(mStyleSheets.begin(), it));
        }
    }
    return r;
}

const std::vector<unsigned>&
IndexedStyleSheets::GetStyleSheetPositionsByFamily(SfxStyleFamily e) const
{
    size_t position = family_to_index(e);
    return mStyleSheetPositionsByFamily.at(position);
}

} /* namespace svl */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
