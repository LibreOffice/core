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
    sal_Int32 nIndex;

public:
    explicit ContentControlListEntriesEnumWrapper(
        uno::Reference<container::XIndexAccess> xIndexAccess)
        : mxIndexAccess(xIndexAccess)
        , nIndex(0)
    {
    }

    virtual sal_Bool SAL_CALL hasMoreElements() override
    {
        return (nIndex < mxIndexAccess->getCount());
    }

    virtual uno::Any SAL_CALL nextElement() override
    {
        if (nIndex < mxIndexAccess->getCount())
        {
            return mxIndexAccess->getByIndex(nIndex++);
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
    SwTextContentControl& m_rCC;

public:
    /// @throws css::uno::RuntimeException
    ContentControlListEntryCollectionHelper(uno::Reference<ov::XHelperInterface> xParent,
                                            uno::Reference<uno::XComponentContext> xContext,
                                            SwTextContentControl& rCC)
        : mxParent(xParent)
        , mxContext(xContext)
        , m_rCC(rCC)
    {
    }

    sal_Int32 SAL_CALL getCount() override
    {
        return m_rCC.GetContentControl().GetContentControl()->GetListItems().size();
    }

    uno::Any SAL_CALL getByIndex(sal_Int32 Index) override
    {
        if (Index < 0 || Index >= getCount())
            throw lang::IndexOutOfBoundsException();

        return uno::Any(uno::Reference<word::XContentControlListEntry>(
            new SwVbaContentControlListEntry(mxParent, mxContext, m_rCC, Index)));
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
    const uno::Reference<uno::XComponentContext>& xContext, SwTextContentControl& rCC)
    : SwVbaContentControlListEntries_BASE(
          xParent, xContext,
          uno::Reference<container::XIndexAccess>(
              new ContentControlListEntryCollectionHelper(xParent, xContext, rCC)))
    , m_rCC(rCC)
{
}

uno::Reference<word::XContentControlListEntry>
SwVbaContentControlListEntries::Add(const OUString& rName, const uno::Any& rValue,
                                    const uno::Any& rIndex)
{
    // No duplicate Names allowed in VBA
    std::shared_ptr<SwContentControl> pCC = m_rCC.GetContentControl().GetContentControl();
    for (auto& rListItem : pCC->GetListItems())
    {
        if (rListItem.ToString() == rName)
            return uno::Reference<word::XContentControlListEntry>();
    }

    sal_Int32 nZIndex = SAL_MAX_INT32;
    rIndex >>= nZIndex;
    // rIndex is 1-based, nZIndex is 0-based. If rIndex is not given, then add as the last choice.
    assert(nZIndex > 0);
    --nZIndex;
    nZIndex = std::min(static_cast<size_t>(nZIndex), pCC->GetListItems().size());

    OUString sValue;
    rValue >>= sValue;
    if (pCC->AddListItem(nZIndex, rName, sValue))
    {
        return uno::Reference<word::XContentControlListEntry>(
            new SwVbaContentControlListEntry(mxParent, mxContext, m_rCC, nZIndex));
    }

    return uno::Reference<word::XContentControlListEntry>();
}

void SwVbaContentControlListEntries::Clear()
{
    m_rCC.GetContentControl().GetContentControl()->ClearListItems();
}

sal_Int32 SwVbaContentControlListEntries::getCount()
{
    return m_rCC.GetContentControl().GetContentControl()->GetListItems().size();
}

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
    return "SwVbaContentControlListEntries";
}

uno::Sequence<OUString> SwVbaContentControlListEntries::getServiceNames()
{
    static uno::Sequence<OUString> const sNames{ "ooo.vba.word.ContentControlListEntries" };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
