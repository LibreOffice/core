/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "vbacontentcontrollistentries.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

namespace
{
class ContentControlListEntriesEnumWrapper : public EnumerationHelper_BASE
{
    uno::Reference<container::XIndexAccess> mxIndexAccess;
    sal_Int32 mnIndex;

public:
    explicit ContentControlListEntriesEnumWrapper(
        uno::Reference<container::XIndexAccess> xIndexAccess)
        : mxIndexAccess(std::move(xIndexAccess))
        , mnIndex(0)
    {
    }

    virtual sal_Bool SAL_CALL hasMoreElements() override
    {
        return (mnIndex < mxIndexAccess->getCount());
    }

    virtual uno::Any SAL_CALL nextElement() override
    {
        if (mnIndex < mxIndexAccess->getCount())
        {
            return mxIndexAccess->getByIndex(mnIndex++);
        }
        throw container::NoSuchElementException();
    }
};

class ContentControlListEntryCollectionHelper
    : public ::cppu::WeakImplHelper<container::XIndexAccess, container::XEnumerationAccess>
{
private:
    uno::Reference<XHelperInterface> mxParent;
    uno::Reference<uno::XComponentContext> mxContext;
    std::shared_ptr<SwContentControl> m_pCC;

public:
    /// @throws css::uno::RuntimeException
    ContentControlListEntryCollectionHelper(uno::Reference<ov::XHelperInterface> xParent,
                                            uno::Reference<uno::XComponentContext> xContext,
                                            std::shared_ptr<SwContentControl> pCC)
        : mxParent(std::move(xParent))
        , mxContext(std::move(xContext))
        , m_pCC(std::move(pCC))
    {
    }

    sal_Int32 SAL_CALL getCount() override { return m_pCC->GetListItems().size(); }

    uno::Any SAL_CALL getByIndex(sal_Int32 Index) override
    {
        if (Index < 0 || Index >= getCount())
            throw lang::IndexOutOfBoundsException();

        return uno::Any(uno::Reference<word::XContentControlListEntry>(
            new SwVbaContentControlListEntry(mxParent, mxContext, m_pCC, Index)));
    }

    uno::Type SAL_CALL getElementType() override
    {
        return cppu::UnoType<word::XContentControlListEntry>::get();
    }

    sal_Bool SAL_CALL hasElements() override { return getCount() != 0; }

    // XEnumerationAccess
    uno::Reference<container::XEnumeration> SAL_CALL createEnumeration() override
    {
        return new ContentControlListEntriesEnumWrapper(this);
    }
};
}

/**
 * DropDownLists and ComboBoxes contain a list of name/value pairs to choose from.
 * Use of DropDownListEntries from any other control is invalid.
 */
SwVbaContentControlListEntries::SwVbaContentControlListEntries(
    const uno::Reference<XHelperInterface>& xParent,
    const uno::Reference<uno::XComponentContext>& xContext, std::shared_ptr<SwContentControl> pCC)
    : SwVbaContentControlListEntries_BASE(
          xParent, xContext,
          uno::Reference<container::XIndexAccess>(
              new ContentControlListEntryCollectionHelper(xParent, xContext, pCC)))
    , m_pCC(pCC)
{
}

uno::Reference<word::XContentControlListEntry>
SwVbaContentControlListEntries::Add(const OUString& rName, const uno::Any& rValue,
                                    const uno::Any& rIndex)
{
    // No duplicate Names allowed in VBA
    for (auto& rListItem : m_pCC->GetListItems())
    {
        if (rListItem.ToString() == rName)
            return uno::Reference<word::XContentControlListEntry>();
    }

    sal_Int32 nZIndex = SAL_MAX_INT32;
    rIndex >>= nZIndex;
    // rIndex is 1-based, nZIndex is 0-based. If rIndex is not given, then add as the last choice.
    assert(nZIndex > 0);
    --nZIndex;
    nZIndex = std::min(static_cast<size_t>(nZIndex), m_pCC->GetListItems().size());

    OUString sValue;
    rValue >>= sValue;
    if (m_pCC->AddListItem(nZIndex, rName, sValue))
    {
        return uno::Reference<word::XContentControlListEntry>(
            new SwVbaContentControlListEntry(mxParent, mxContext, m_pCC, nZIndex));
    }

    return uno::Reference<word::XContentControlListEntry>();
}

void SwVbaContentControlListEntries::Clear() { m_pCC->ClearListItems(); }

sal_Int32 SwVbaContentControlListEntries::getCount() { return m_pCC->GetListItems().size(); }

// XEnumerationAccess
uno::Type SwVbaContentControlListEntries::getElementType()
{
    return cppu::UnoType<word::XContentControlListEntry>::get();
}

uno::Reference<container::XEnumeration> SwVbaContentControlListEntries::createEnumeration()
{
    return new ContentControlListEntriesEnumWrapper(m_xIndexAccess);
}

// SwVbaContentControlListEntries_BASE
uno::Any SwVbaContentControlListEntries::createCollectionObject(const uno::Any& aSource)
{
    return aSource;
}

OUString SwVbaContentControlListEntries::getServiceImplName()
{
    return u"SwVbaContentControlListEntries"_ustr;
}

uno::Sequence<OUString> SwVbaContentControlListEntries::getServiceNames()
{
    static uno::Sequence<OUString> const sNames{ u"ooo.vba.word.ContentControlListEntries"_ustr };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
