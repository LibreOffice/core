/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/sequence.hxx>
#include <sal/log.hxx>

#include <doc.hxx>
#include <docsh.hxx>
#include <textcontentcontrol.hxx>

#include "vbacontentcontrol.hxx"
#include "vbacontentcontrols.hxx"
#include "wordvbahelper.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

// Helper function to access the content controls
// @param rIndex
//        [in] negative indexes indicate the need to search by name, otherwise get by index,
//             using SAL_MAX_INT32 to indicate the need to just get the total count.
//        [out] rIndex indicates the found index, or the total number of content controls
static std::shared_ptr<SwContentControl>
lcl_getContentControl(std::u16string_view sName, std::u16string_view sTag,
                      std::u16string_view sTitle, sal_Int32& rIndex,
                      const uno::Reference<text::XTextDocument>& xTextDocument,
                      uno::Sequence<OUString>* pElementNames = nullptr)
{
    SwDoc* pDoc = word::getDocShell(xTextDocument)->GetDoc();
    if (!pDoc)
        return nullptr;

    assert(sTag.empty() || sTitle.empty()); // only one grouping at a time is allowed

    std::shared_ptr<SwContentControl> pControl;
    std::vector<OUString> vElementNames;
    SwContentControlManager& rManager = pDoc->GetContentControlManager();
    const size_t nLen = rManager.GetCount();
    if (!pElementNames && rIndex > 0 && sName.empty() && sTag.empty() && sTitle.empty())
    {
        size_t i = static_cast<size_t>(rIndex);
        // This is the normal get-by-index/getCount mode - no need for fancy filtering.
        if (i < nLen)
            pControl = rManager.Get(i)->GetContentControl().GetContentControl();
        else
            rIndex = nLen;
    }
    else
    {
        // loop through everything collecting names, filtering by Tag/Title
        sal_Int32 nCounter = 0;
        for (size_t i = 0; i < nLen; ++i)
        {
            pControl = rManager.Get(i)->GetContentControl().GetContentControl();
            if (!sTag.empty() && sTag != pControl->GetTag())
            {
                pControl = nullptr;
                continue;
            }
            if (!sTitle.empty() && sTitle != pControl->GetAlias())
            {
                pControl = nullptr;
                continue;
            }

            // When treated as a name, consider the integer ID to be unsigned
            const OUString sID = OUString::number(static_cast<sal_uInt32>(pControl->GetId()));
            if (!sName.empty() && sName != sID)
            {
                pControl = nullptr;
                continue;
            }

            if (pElementNames)
                vElementNames.push_back(sID);

            if (rIndex == nCounter || !sName.empty())
                break;

            pControl = nullptr;
            ++nCounter;
        }
        rIndex = nCounter;
    }
    if (pElementNames)
        *pElementNames = comphelper::containerToSequence(vElementNames);
    return pControl;
}

namespace
{
class ContentControlsEnumWrapper : public EnumerationHelper_BASE
{
    uno::Reference<container::XIndexAccess> mxIndexAccess;
    sal_Int32 mnIndex;

public:
    explicit ContentControlsEnumWrapper(uno::Reference<container::XIndexAccess> xIndexAccess)
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

class ContentControlCollectionHelper
    : public ::cppu::WeakImplHelper<container::XNameAccess, container::XIndexAccess,
                                    container::XEnumerationAccess>
{
private:
    uno::Reference<XHelperInterface> mxParent;
    uno::Reference<uno::XComponentContext> mxContext;
    uno::Reference<text::XTextDocument> mxTextDocument;
    const OUString m_sTag;
    const OUString m_sTitle;
    std::shared_ptr<SwContentControl> m_pCache;

public:
    /// @throws css::uno::RuntimeException
    ContentControlCollectionHelper(uno::Reference<ov::XHelperInterface> xParent,
                                   uno::Reference<uno::XComponentContext> xContext,
                                   uno::Reference<text::XTextDocument> xTextDocument,
                                   const OUString& rTag, const OUString& rTitle)

        : mxParent(std::move(xParent))
        , mxContext(std::move(xContext))
        , mxTextDocument(std::move(xTextDocument))
        , m_sTag(rTag)
        , m_sTitle(rTitle)
    {
    }

    // XIndexAccess
    sal_Int32 SAL_CALL getCount() override
    {
        sal_Int32 nCount = SAL_MAX_INT32;
        lcl_getContentControl(u"", m_sTag, m_sTitle, nCount, mxTextDocument);
        return nCount == SAL_MAX_INT32 || nCount < 0 ? 0 : nCount;
    }

    uno::Any SAL_CALL getByIndex(sal_Int32 Index) override
    {
        m_pCache = lcl_getContentControl(u"", m_sTag, m_sTitle, Index, mxTextDocument);
        if (!m_pCache)
            throw lang::IndexOutOfBoundsException();

        return uno::Any(uno::Reference<word::XContentControl>(
            new SwVbaContentControl(mxParent, mxContext, mxTextDocument, m_pCache)));
    }

    // XNameAccess
    uno::Sequence<OUString> SAL_CALL getElementNames() override
    {
        sal_Int32 nCount = SAL_MAX_INT32;
        uno::Sequence<OUString> aSeq;
        lcl_getContentControl(u"", m_sTag, m_sTitle, nCount, mxTextDocument, &aSeq);
        return aSeq;
    }

    uno::Any SAL_CALL getByName(const OUString& aName) override
    {
        if (!hasByName(aName))
            throw container::NoSuchElementException();

        return uno::Any(uno::Reference<word::XContentControl>(
            new SwVbaContentControl(mxParent, mxContext, mxTextDocument, m_pCache)));
    }

    sal_Bool SAL_CALL hasByName(const OUString& aName) override
    {
        sal_Int32 nCount = -1;
        m_pCache = lcl_getContentControl(aName, m_sTag, m_sTitle, nCount, mxTextDocument);
        return m_pCache != nullptr;
    }

    // XElementAccess
    uno::Type SAL_CALL getElementType() override
    {
        return cppu::UnoType<word::XContentControl>::get();
    }

    sal_Bool SAL_CALL hasElements() override { return getCount() != 0; }

    // XEnumerationAccess
    uno::Reference<container::XEnumeration> SAL_CALL createEnumeration() override
    {
        return new ContentControlsEnumWrapper(this);
    }
};
}

/**
 * Content Controls can be accessed and filtered in many different ways.
 * Surprisingly however, there is no clear, descriptive "by name" access.
 * Instead, each content control (probably) has a unique _signed-integer_ identifier,
 * which can be passed to Item() as a float or _unsigned-integer_ string
 * (to differentiate it from getByIndex).
 *
 * Index access can be filtered by Tag, Title, Range, and XML link.
 * TODO: add filtering for Range, SelectLinkedControls, SelectUnlinkedControls
 */
SwVbaContentControls::SwVbaContentControls(const uno::Reference<XHelperInterface>& xParent,
                                           const uno::Reference<uno::XComponentContext>& xContext,
                                           const uno::Reference<text::XTextDocument>& xTextDocument,
                                           const OUString& rTag, const OUString& rTitle)
    : SwVbaContentControls_BASE(
          xParent, xContext,
          uno::Reference<container::XIndexAccess>(
              new ContentControlCollectionHelper(xParent, xContext, xTextDocument, rTag, rTitle)))
{
}

// uno::Reference<ooo::vba::word::XContentControl> SwVbaContentControls::Add(const uno::Any& Range,
//                                                                 sal_Int32 Type)
// {
//     sw::mark::IFieldmark* pFieldmark = nullptr;
//     switch (Type)
//     {
//         case ooo::vba::word::WdFieldType::wdFieldFormCheckBox:
//             break;
//         case ooo::vba::word::WdFieldType::wdFieldFormDropDown:
//             break;
//         case ooo::vba::word::WdFieldType::wdFieldFormTextInput:
//         default:;
//     }
//
//     return uno::Reference<ooo::vba::word::XContentControl>(
//         new SwVbaContentControl(mxParent, mxContext, m_xTextDocument, pFieldmark));
// }

// XEnumerationAccess
uno::Type SwVbaContentControls::getElementType()
{
    return cppu::UnoType<word::XContentControl>::get();
}

uno::Reference<container::XEnumeration> SwVbaContentControls::createEnumeration()
{
    return new ContentControlsEnumWrapper(m_xIndexAccess);
}

uno::Any SwVbaContentControls::createCollectionObject(const uno::Any& aSource) { return aSource; }

OUString SwVbaContentControls::getServiceImplName() { return u"SwVbaContentControls"_ustr; }

uno::Sequence<OUString> SwVbaContentControls::getServiceNames()
{
    static uno::Sequence<OUString> const sNames{ u"ooo.vba.word.ContentControls"_ustr };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
