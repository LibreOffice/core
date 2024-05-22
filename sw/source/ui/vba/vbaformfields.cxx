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
#include <IDocumentMarkAccess.hxx>

#include "vbaformfield.hxx"
#include "vbaformfields.hxx"
#include "wordvbahelper.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

// Helper function to access the fieldmarks
// @param rIndex serves multiple purposes
//        [in] -1 to indicate searching using the provided name, SAL_MAX_INT32 for totals
//        [out] rIndex indicates the found index, or the total number of fieldmarks
static sw::mark::IFieldmark* lcl_getFieldmark(std::string_view rName, sal_Int32& rIndex,
                                              const uno::Reference<frame::XModel>& xModel,
                                              uno::Sequence<OUString>* pElementNames = nullptr)

{
    SwDoc* pDoc = word::getDocShell(xModel)->GetDoc();
    if (!pDoc)
        return nullptr;

    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    if (!pMarkAccess)
        return nullptr;

    sal_Int32 nCounter = 0;
    std::vector<OUString> vElementNames;
    IDocumentMarkAccess::iterator aIter = pMarkAccess->getFieldmarksBegin();
    while (aIter != pMarkAccess->getFieldmarksEnd())
    {
        switch (IDocumentMarkAccess::GetType(**aIter))
        {
            case IDocumentMarkAccess::MarkType::CHECKBOX_FIELDMARK:
            case IDocumentMarkAccess::MarkType::DROPDOWN_FIELDMARK:
            case IDocumentMarkAccess::MarkType::TEXT_FIELDMARK:
            {
                if (rIndex < 0
                    && (*aIter)->GetName().equalsIgnoreAsciiCase(OUString::fromUtf8(rName)))
                {
                    rIndex = nCounter;
                    return dynamic_cast<sw::mark::IFieldmark*>(*aIter);
                }
                else if (rIndex == nCounter)
                    return dynamic_cast<sw::mark::IFieldmark*>(*aIter);

                ++nCounter;
                if (pElementNames)
                    vElementNames.push_back((*aIter)->GetName());
                break;
            }
            default:;
        }
        aIter++;
    }
    rIndex = nCounter;
    if (pElementNames)
        *pElementNames = comphelper::containerToSequence(vElementNames);
    return nullptr;
}

namespace
{
class FormFieldsEnumWrapper : public EnumerationHelper_BASE
{
    uno::Reference<container::XIndexAccess> mxIndexAccess;
    sal_Int32 mnIndex;

public:
    explicit FormFieldsEnumWrapper(const uno::Reference<container::XIndexAccess>& xIndexAccess)
        : mxIndexAccess(xIndexAccess)
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

class FormFieldCollectionHelper
    : public ::cppu::WeakImplHelper<container::XNameAccess, container::XIndexAccess,
                                    container::XEnumerationAccess>
{
private:
    uno::Reference<XHelperInterface> mxParent;
    uno::Reference<uno::XComponentContext> mxContext;
    uno::Reference<text::XTextDocument> mxTextDocument;
    sw::mark::IFieldmark* m_pCache;

public:
    /// @throws css::uno::RuntimeException
    FormFieldCollectionHelper(uno::Reference<ov::XHelperInterface> xParent,
                              uno::Reference<uno::XComponentContext> xContext,
                              uno::Reference<text::XTextDocument> xTextDocument)
        : mxParent(std::move(xParent))
        , mxContext(std::move(xContext))
        , mxTextDocument(std::move(xTextDocument))
        , m_pCache(nullptr)
    {
    }

    // XIndexAccess
    sal_Int32 SAL_CALL getCount() override
    {
        sal_Int32 nCount = SAL_MAX_INT32;
        lcl_getFieldmark("", nCount, mxTextDocument);
        return nCount == SAL_MAX_INT32 ? 0 : nCount;
    }

    uno::Any SAL_CALL getByIndex(sal_Int32 Index) override
    {
        m_pCache = lcl_getFieldmark("", Index, mxTextDocument);
        if (!m_pCache)
            throw lang::IndexOutOfBoundsException();

        return uno::Any(uno::Reference<word::XFormField>(
            new SwVbaFormField(mxParent, mxContext, mxTextDocument, *m_pCache)));
    }

    // XNameAccess
    uno::Sequence<OUString> SAL_CALL getElementNames() override
    {
        sal_Int32 nCount = SAL_MAX_INT32;
        uno::Sequence<OUString> aSeq;
        lcl_getFieldmark("", nCount, mxTextDocument, &aSeq);
        return aSeq;
    }

    uno::Any SAL_CALL getByName(const OUString& aName) override
    {
        if (!hasByName(aName))
            throw container::NoSuchElementException();

        return uno::Any(uno::Reference<word::XFormField>(
            new SwVbaFormField(mxParent, mxContext, mxTextDocument, *m_pCache)));
    }

    sal_Bool SAL_CALL hasByName(const OUString& aName) override
    {
        sal_Int32 nCount = -1;
        m_pCache = lcl_getFieldmark(aName.toUtf8(), nCount, mxTextDocument);
        return m_pCache != nullptr;
    }

    // XElementAccess
    uno::Type SAL_CALL getElementType() override { return cppu::UnoType<word::XFormField>::get(); }

    sal_Bool SAL_CALL hasElements() override { return getCount() != 0; }

    // XEnumerationAccess
    uno::Reference<container::XEnumeration> SAL_CALL createEnumeration() override
    {
        return new FormFieldsEnumWrapper(this);
    }
};
}

SwVbaFormFields::SwVbaFormFields(const uno::Reference<XHelperInterface>& xParent,
                                 const uno::Reference<uno::XComponentContext>& xContext,
                                 const uno::Reference<text::XTextDocument>& xTextDocument)
    : SwVbaFormFields_BASE(xParent, xContext,
                           uno::Reference<container::XIndexAccess>(
                               new FormFieldCollectionHelper(xParent, xContext, xTextDocument)))
{
}

sal_Bool SwVbaFormFields::getShaded()
{
    SAL_INFO("sw.vba", "SwVbaFormFields::getShaded stub");
    return false;
}

void SwVbaFormFields::setShaded(sal_Bool /*bSet*/)
{
    SAL_INFO("sw.vba", "SwVbaFormFields::setShaded stub");
}

// uno::Reference<ooo::vba::word::XFormField> SwVbaFormFields::Add(const uno::Any& Range,
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
//     return uno::Reference<ooo::vba::word::XFormField>(
//         new SwVbaFormField(mxParent, mxContext, m_xTextDocument, *pFieldmark));
// }

// XEnumerationAccess
uno::Type SwVbaFormFields::getElementType() { return cppu::UnoType<word::XFormField>::get(); }

uno::Reference<container::XEnumeration> SwVbaFormFields::createEnumeration()
{
    return new FormFieldsEnumWrapper(m_xIndexAccess);
}

uno::Any SwVbaFormFields::createCollectionObject(const uno::Any& aSource) { return aSource; }

OUString SwVbaFormFields::getServiceImplName() { return u"SwVbaFormFields"_ustr; }

uno::Sequence<OUString> SwVbaFormFields::getServiceNames()
{
    static uno::Sequence<OUString> const sNames{ u"ooo.vba.word.FormFields"_ustr };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
