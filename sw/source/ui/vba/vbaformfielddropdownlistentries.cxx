/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <xmloff/odffields.hxx>

#include "vbaformfielddropdownlistentries.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

static uno::Sequence<OUString> lcl_getListEntries(sw::mark::IDropdownFieldmark& rDropDown)
{
    uno::Sequence<OUString> aSeq;
    (*rDropDown.GetParameters())[ODF_FORMDROPDOWN_LISTENTRY] >>= aSeq;
    return aSeq;
}

namespace
{
class ListEntriesEnumWrapper : public EnumerationHelper_BASE
{
    uno::Reference<container::XIndexAccess> mxIndexAccess;
    sal_Int32 mnIndex;

public:
    explicit ListEntriesEnumWrapper(uno::Reference<container::XIndexAccess> xIndexAccess)
        : mxIndexAccess(std::move(xIndexAccess))
        , mnIndex(0)
    {
    }

    sal_Bool SAL_CALL hasMoreElements() override { return (mnIndex < mxIndexAccess->getCount()); }

    uno::Any SAL_CALL nextElement() override
    {
        if (mnIndex < mxIndexAccess->getCount())
        {
            return mxIndexAccess->getByIndex(mnIndex++);
        }
        throw container::NoSuchElementException();
    }
};

class ListEntryCollectionHelper
    : public ::cppu::WeakImplHelper<container::XIndexAccess, container::XEnumerationAccess>
{
private:
    uno::Reference<XHelperInterface> mxParent;
    uno::Reference<uno::XComponentContext> mxContext;
    sw::mark::IDropdownFieldmark& m_rDropDown;

public:
    /// @throws css::uno::RuntimeException
    ListEntryCollectionHelper(uno::Reference<ov::XHelperInterface> xParent,
                              uno::Reference<uno::XComponentContext> xContext,
                              sw::mark::IDropdownFieldmark& rFormField)
        : mxParent(std::move(xParent))
        , mxContext(std::move(xContext))
        , m_rDropDown(rFormField)
    {
    }

    sal_Int32 SAL_CALL getCount() override { return lcl_getListEntries(m_rDropDown).getLength(); }

    uno::Any SAL_CALL getByIndex(sal_Int32 Index) override
    {
        if (Index < 0 || Index >= getCount())
            throw lang::IndexOutOfBoundsException();

        return uno::Any(uno::Reference<word::XListEntry>(
            new SwVbaFormFieldDropDownListEntry(mxParent, mxContext, m_rDropDown, Index)));
    }

    uno::Type SAL_CALL getElementType() override { return cppu::UnoType<word::XListEntry>::get(); }

    sal_Bool SAL_CALL hasElements() override { return getCount() != 0; }

    // XEnumerationAccess
    uno::Reference<container::XEnumeration> SAL_CALL createEnumeration() override
    {
        return new ListEntriesEnumWrapper(this);
    }
};
}

SwVbaFormFieldDropDownListEntries::SwVbaFormFieldDropDownListEntries(
    const uno::Reference<XHelperInterface>& xParent,
    const uno::Reference<uno::XComponentContext>& xContext,
    sw::mark::IDropdownFieldmark& rFormField)
    : SwVbaFormFieldDropDownListEntries_BASE(
          xParent, xContext,
          uno::Reference<container::XIndexAccess>(
              new ListEntryCollectionHelper(xParent, xContext, rFormField)))
    , m_rDropDown(rFormField)
{
}

// XListEntries
uno::Reference<word::XListEntry> SwVbaFormFieldDropDownListEntries::Add(const OUString& rName,
                                                                        const uno::Any& rIndex)
{
    sal_Int32 nZIndex = 0;
    rIndex >>= nZIndex;
    // rIndex is 1-based, nZIndex is 0-based. If rIndex is not given, then add as the last choice.

    // In testing with Word 2010, this gives a compile error: 'ListEntries.Add("Name", 2)'
    // This compiles, but gets an unsupported runtime error:  'ListEntries.Add("Name", 2) = "Choice'
    // So the only thing that actually works is to simply append: 'ListEntires.Add("Name")'
    // but I'll still keep the expected implementation for the broken case.
    if (!nZIndex)
        nZIndex = SAL_MAX_INT32;
    else
        --nZIndex;
    m_rDropDown.AddContent(rName + "__allowDuplicates", &nZIndex);
    m_rDropDown.ReplaceContent(&rName, &nZIndex);

    return uno::Reference<word::XListEntry>(
        new SwVbaFormFieldDropDownListEntry(mxParent, mxContext, m_rDropDown, nZIndex));
}

void SwVbaFormFieldDropDownListEntries::Clear() { m_rDropDown.DelContent(); }

sal_Int32 SwVbaFormFieldDropDownListEntries::getCount()
{
    return lcl_getListEntries(m_rDropDown).getLength();
}

// XEnumerationAccess
uno::Type SwVbaFormFieldDropDownListEntries::getElementType()
{
    return cppu::UnoType<word::XListEntry>::get();
}

uno::Reference<container::XEnumeration> SwVbaFormFieldDropDownListEntries::createEnumeration()
{
    return new ListEntriesEnumWrapper(m_xIndexAccess);
}

// SwVbadropDownListEntries_BASE
uno::Any SwVbaFormFieldDropDownListEntries::createCollectionObject(const uno::Any& aSource)
{
    return aSource;
}

OUString SwVbaFormFieldDropDownListEntries::getServiceImplName()
{
    return u"SwVbaFormFieldDropDownListEntries"_ustr;
}

uno::Sequence<OUString> SwVbaFormFieldDropDownListEntries::getServiceNames()
{
    static uno::Sequence<OUString> const sNames{ u"ooo.vba.word.ListEntries"_ustr };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
