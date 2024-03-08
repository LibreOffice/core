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


namespace {
size_t family_to_index(SfxStyleFamily family)
{
    switch (family) {
    case SfxStyleFamily::Char:
        return 0;
    case SfxStyleFamily::Para:
        return 1;
    case SfxStyleFamily::Frame:
        return 2;
    case SfxStyleFamily::Page:
        return 3;
    case SfxStyleFamily::Pseudo:
        return 4;
    case SfxStyleFamily::Table:
        return 5;
    case SfxStyleFamily::All:
        return 6;
    default: break;
    }
    assert(false); // only for compiler warning. all cases are handled in the switch
    return 0;
}
}

namespace svl {

IndexedStyleSheets::IndexedStyleSheets()
{
}

void IndexedStyleSheets::Register(SfxStyleSheetBase& style, sal_Int32 pos)
{
    mPositionsByName.insert(std::make_pair(style.GetName(), pos));
    size_t position = family_to_index(style.GetFamily());
    mStyleSheetsByFamily.at(position).push_back(&style);
    size_t positionForFamilyAll = family_to_index(SfxStyleFamily::All);
    mStyleSheetsByFamily.at(positionForFamilyAll).push_back(&style);
}

void
IndexedStyleSheets::Reindex()
{
    mPositionsByName.clear();
    for (size_t i = 0; i < NUMBER_OF_FAMILIES; i++) {
        mStyleSheetsByFamily[i].clear();
    }

    sal_Int32 i = 0;
    for (const auto& rxStyleSheet : mStyleSheets) {
        SfxStyleSheetBase* p = rxStyleSheet.get();
        Register(*p, i);
        ++i;
    }
}

void
IndexedStyleSheets::AddStyleSheet(const rtl::Reference< SfxStyleSheetBase >& style)
{
    if (!HasStyleSheet(style)) {
        mStyleSheets.push_back(style);
        // since we just added an element to the vector, we can safely do -1 as it will always be >= 1
        Register(*style, mStyleSheets.size()-1);
    }
}

bool
IndexedStyleSheets::RemoveStyleSheet(const rtl::Reference< SfxStyleSheetBase >& style)
{
    std::pair<MapType::const_iterator, MapType::const_iterator> range = mPositionsByName.equal_range(style->GetName());
    for (MapType::const_iterator it = range.first; it != range.second; ++it)
    {
        sal_Int32 pos = it->second;
        if (mStyleSheets.at(pos) == style)
        {
            mStyleSheets.erase(mStyleSheets.begin() + pos);
            Reindex();
            return true;
        }
    }
    return false;
}

std::vector<sal_Int32> IndexedStyleSheets::FindPositionsByName(const OUString& name) const
{
    std::vector<sal_Int32> r;
    std::pair<MapType::const_iterator, MapType::const_iterator> range = mPositionsByName.equal_range(name);
    for (MapType::const_iterator it = range.first; it != range.second; ++it) {
        r.push_back(it->second);
    }
    return r;
}

std::vector<sal_Int32> IndexedStyleSheets::FindPositionsByNameAndPredicate(const OUString& name,
        StyleSheetPredicate& predicate, SearchBehavior behavior) const
{
    std::vector<sal_Int32> r;
    auto range = mPositionsByName.equal_range(name);
    for (auto it = range.first; it != range.second; ++it) {
        sal_Int32 pos = it->second;
        SfxStyleSheetBase *ssheet = mStyleSheets.at(pos).get();
        if (predicate.Check(*ssheet)) {
            r.push_back(pos);
            if (behavior == SearchBehavior::ReturnFirst) {
                break;
            }
        }
    }
    return r;
}


sal_Int32
IndexedStyleSheets::GetNumberOfStyleSheetsWithPredicate(StyleSheetPredicate& predicate) const
{
    return std::count_if(mStyleSheets.begin(), mStyleSheets.end(),
        [&predicate](const rtl::Reference<SfxStyleSheetBase>& rxStyleSheet) {
            const SfxStyleSheetBase *ssheet = rxStyleSheet.get();
            return predicate.Check(*ssheet);
        });
}

SfxStyleSheetBase*
IndexedStyleSheets::GetNthStyleSheetThatMatchesPredicate(
        sal_Int32 n,
        StyleSheetPredicate& predicate,
        sal_Int32 startAt)
{
    SfxStyleSheetBase* retval = nullptr;
    sal_Int32 matching = 0;
    for (VectorType::const_iterator it = mStyleSheets.begin()+startAt; it != mStyleSheets.end(); ++it) {
        SfxStyleSheetBase *ssheet = it->get();
        if (predicate.Check(*ssheet)) {
            if (matching == n) {
                retval = it->get();
                break;
            }
            ++matching;
        }
    }
    return retval;
}

sal_Int32 IndexedStyleSheets::FindStyleSheetPosition(const SfxStyleSheetBase& style) const
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
    for (const auto& rxStyleSheet : mStyleSheets) {
        disposer.Dispose(rxStyleSheet);
    }
    mStyleSheets.clear();
    mPositionsByName.clear();
}

IndexedStyleSheets::~IndexedStyleSheets()
{
}

bool
IndexedStyleSheets::HasStyleSheet(const rtl::Reference< SfxStyleSheetBase >& style) const
{
    std::pair<MapType::const_iterator, MapType::const_iterator> range = mPositionsByName.equal_range(style->GetName());
    for (MapType::const_iterator it = range.first; it != range.second; ++it)
    {
        if (mStyleSheets.at(it->second) == style)
            return true;
    }
    return false;
}

SfxStyleSheetBase*
IndexedStyleSheets::GetStyleSheetByPosition(sal_Int32 pos)
{
    if( pos < static_cast<sal_Int32>(mStyleSheets.size()) )
        return mStyleSheets.at(pos).get();
    return nullptr;
}

void
IndexedStyleSheets::ApplyToAllStyleSheets(StyleSheetCallback& callback) const
{
    for (const auto& rxStyleSheet : mStyleSheets) {
        callback.DoIt(*rxStyleSheet);
    }
}

std::vector<sal_Int32>
IndexedStyleSheets::FindPositionsByPredicate(StyleSheetPredicate& predicate) const
{
    std::vector<sal_Int32> r;
    for (VectorType::const_iterator it = mStyleSheets.begin(); it != mStyleSheets.end(); ++it) {
        if (predicate.Check(**it)) {
            r.push_back(std::distance(mStyleSheets.begin(), it));
        }
    }
    return r;
}

const std::vector<SfxStyleSheetBase*>&
IndexedStyleSheets::GetStyleSheetsByFamily(SfxStyleFamily e) const
{
    size_t position = family_to_index(e);
    return mStyleSheetsByFamily.at(position);
}

} /* namespace svl */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
