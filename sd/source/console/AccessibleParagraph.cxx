/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "AccessibleParagraph.hxx"

#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;


namespace {

//===== AccessibleRelationSet =================================================

typedef ::cppu::WeakComponentImplHelper <
    css::accessibility::XAccessibleRelationSet
    > AccessibleRelationSetInterfaceBase;

class AccessibleRelationSet
    : public ::cppu::BaseMutex,
      public AccessibleRelationSetInterfaceBase
{
public:
    AccessibleRelationSet();

    void AddRelation (
        const AccessibleRelationType eRelationType,
        const Reference<XAccessible>& rxObject);

    //----- XAccessibleRelationSet --------------------------------------------

    virtual sal_Int32 SAL_CALL getRelationCount() override;

    virtual AccessibleRelation SAL_CALL getRelation (sal_Int32 nIndex) override;

    virtual sal_Bool SAL_CALL containsRelation(css::accessibility::AccessibleRelationType eRelationType) override;

    virtual AccessibleRelation SAL_CALL getRelationByType(AccessibleRelationType eRelationType) override;

private:
    ::std::vector<AccessibleRelation> maRelations;
};


//===== AccessibleRelationSet =================================================

AccessibleRelationSet::AccessibleRelationSet()
    : AccessibleRelationSetInterfaceBase(m_aMutex)
{
}

void AccessibleRelationSet::AddRelation (
    const AccessibleRelationType eRelationType,
    const Reference<XAccessible>& rxObject)
{
    maRelations.emplace_back();
    maRelations.back().RelationType = eRelationType;
    maRelations.back().TargetSet = { rxObject };
}

//----- XAccessibleRelationSet ------------------------------------------------

sal_Int32 SAL_CALL AccessibleRelationSet::getRelationCount()
{
    return maRelations.size();
}

AccessibleRelation SAL_CALL AccessibleRelationSet::getRelation (sal_Int32 nIndex)
{
    if (nIndex<0 && o3tl::make_unsigned(nIndex)>=maRelations.size())
        return AccessibleRelation();
    else
        return maRelations[nIndex];
}

sal_Bool SAL_CALL AccessibleRelationSet::containsRelation(AccessibleRelationType eRelationType)
{
    return std::any_of(maRelations.begin(), maRelations.end(),
                       [eRelationType](const AccessibleRelation& rRelation) { return rRelation.RelationType == eRelationType; });
}

AccessibleRelation SAL_CALL AccessibleRelationSet::getRelationByType(AccessibleRelationType eRelationType)
{
    auto iRelation = std::find_if(maRelations.begin(), maRelations.end(),
                                  [eRelationType](const AccessibleRelation& rRelation) { return rRelation.RelationType == eRelationType; });
    if (iRelation != maRelations.end())
        return *iRelation;
    return AccessibleRelation();
}

}


AccessibleParagraph::AccessibleParagraph(
    const OUString& rsName,
    SharedPresenterTextParagraph xParagraph,
    const sal_Int32 nParagraphIndex)
    : PresenterAccessibleParagraphInterfaceBase(AccessibleRole::PARAGRAPH, rsName),
      mpParagraph(std::move(xParagraph)),
      mnParagraphIndex(nParagraphIndex)
{
}

//----- XAccessibleContext ----------------------------------------------------

Reference<XAccessibleRelationSet> SAL_CALL
    AccessibleParagraph::getAccessibleRelationSet()
{
    ThrowIfDisposed();

    rtl::Reference<AccessibleRelationSet> pSet (new AccessibleRelationSet);

    if (mxParentAccessible.is())
    {
        Reference<XAccessibleContext> xParentContext (mxParentAccessible->getAccessibleContext());
        if (xParentContext.is())
        {
            if (mnParagraphIndex>0)
                pSet->AddRelation(
                    AccessibleRelationType_CONTENT_FLOWS_FROM,
                    xParentContext->getAccessibleChild(mnParagraphIndex-1));

            if (mnParagraphIndex<xParentContext->getAccessibleChildCount()-1)
                pSet->AddRelation(
                    AccessibleRelationType_CONTENT_FLOWS_TO,
                    xParentContext->getAccessibleChild(mnParagraphIndex+1));
        }
    }

    return pSet;
}

//----- XAccessibleText -------------------------------------------------------

sal_Int32 SAL_CALL AccessibleParagraph::getCaretPosition()
{
    ThrowIfDisposed();

    sal_Int32 nPosition (-1);
    if (mpParagraph)
        nPosition = mpParagraph->GetCaretPosition();

    return nPosition;
}

sal_Bool SAL_CALL AccessibleParagraph::setCaretPosition (sal_Int32 nIndex)
{
    ThrowIfDisposed();

    if (mpParagraph)
    {
        mpParagraph->SetCaretPosition(nIndex);
        return true;
    }
    else
        return false;
}

sal_Unicode SAL_CALL AccessibleParagraph::getCharacter (sal_Int32 nIndex)
{
    ThrowIfDisposed();

    if (!mpParagraph)
        throw lang::IndexOutOfBoundsException(u"no text support in current mode"_ustr, static_cast<uno::XWeak*>(this));
    return mpParagraph->GetCharacter(nIndex);
}

Sequence<css::beans::PropertyValue> SAL_CALL
    AccessibleParagraph::getCharacterAttributes (
        ::sal_Int32 nIndex,
        const css::uno::Sequence<OUString>& rRequestedAttributes)
{
    ThrowIfDisposed();

#if OSL_DEBUG_LEVEL > 0
    SAL_INFO( "sdext.presenter", __func__ << " at " << this << ", " << nIndex << " returns empty set" );
    for (sal_Int32 nAttributeIndex(0), nAttributeCount(rRequestedAttributes.getLength());
         nAttributeIndex < nAttributeCount;
         ++nAttributeIndex)
    {
        SAL_INFO( "sdext.presenter",
                  "    requested attribute " << nAttributeIndex << " is " << rRequestedAttributes[nAttributeIndex] );
    }
#else
    (void)nIndex;
    (void)rRequestedAttributes;
#endif

    // Character properties are not supported.
    return Sequence<css::beans::PropertyValue>();
}

awt::Rectangle SAL_CALL AccessibleParagraph::getCharacterBounds (
    sal_Int32 nIndex)
{
    ThrowIfDisposed();

    awt::Rectangle aCharacterBox;
    if (nIndex < 0)
    {
        throw lang::IndexOutOfBoundsException(u"invalid text index"_ustr, static_cast<uno::XWeak*>(this));
    }
    else if (mpParagraph)
    {
        aCharacterBox = mpParagraph->GetCharacterBounds(nIndex, false);
        // Convert coordinates relative to the window origin into absolute
        // screen coordinates.
        const awt::Point aWindowLocationOnScreen (getLocationOnScreen());
        aCharacterBox.X += aWindowLocationOnScreen.X;
        aCharacterBox.Y += aWindowLocationOnScreen.Y;
    }
    else
    {
        throw lang::IndexOutOfBoundsException(u"no text support in current mode"_ustr, static_cast<uno::XWeak*>(this));
    }

    return aCharacterBox;
}

sal_Int32 SAL_CALL AccessibleParagraph::getCharacterCount()
{
    ThrowIfDisposed();

    sal_Int32 nCount (0);
    if (mpParagraph)
        nCount = mpParagraph->GetCharacterCount();

    return nCount;
}

sal_Int32 SAL_CALL AccessibleParagraph::getIndexAtPoint (
    const css::awt::Point& )
{
    ThrowIfDisposed();
    return -1;
}

OUString SAL_CALL AccessibleParagraph::getSelectedText()
{
    ThrowIfDisposed();

    return getTextRange(getSelectionStart(), getSelectionEnd());
}

sal_Int32 SAL_CALL AccessibleParagraph::getSelectionStart()
{
    ThrowIfDisposed();

    return getCaretPosition();
}

sal_Int32 SAL_CALL AccessibleParagraph::getSelectionEnd()
{
    ThrowIfDisposed();

    return getCaretPosition();
}

sal_Bool SAL_CALL AccessibleParagraph::setSelection (
    sal_Int32 nStartIndex,
    sal_Int32)
{
    ThrowIfDisposed();

    return setCaretPosition(nStartIndex);
}

OUString SAL_CALL AccessibleParagraph::getText()
{
    ThrowIfDisposed();

    OUString sText;
    if (mpParagraph)
        sText = mpParagraph->GetText();

    return sText;
}

OUString SAL_CALL AccessibleParagraph::getTextRange (
    sal_Int32 nLocalStartIndex,
    sal_Int32 nLocalEndIndex)
{
    ThrowIfDisposed();

    OUString sText;
    if (mpParagraph)
    {
        const TextSegment aSegment (
            mpParagraph->CreateTextSegment(nLocalStartIndex, nLocalEndIndex));
        sText = aSegment.SegmentText;
    }

    return sText;
}

TextSegment SAL_CALL AccessibleParagraph::getTextAtIndex (
    sal_Int32 nLocalCharacterIndex,
    sal_Int16 nTextType)
{
    ThrowIfDisposed();

    TextSegment aSegment;
    if (mpParagraph)
        aSegment = mpParagraph->GetTextSegment(0, nLocalCharacterIndex, nTextType);

    return aSegment;
}

TextSegment SAL_CALL AccessibleParagraph::getTextBeforeIndex (
    sal_Int32 nLocalCharacterIndex,
    sal_Int16 nTextType)
{
    ThrowIfDisposed();

    TextSegment aSegment;
    if (mpParagraph)
        aSegment = mpParagraph->GetTextSegment(-1, nLocalCharacterIndex, nTextType);

    return aSegment;
}

TextSegment SAL_CALL AccessibleParagraph::getTextBehindIndex (
    sal_Int32 nLocalCharacterIndex,
    sal_Int16 nTextType)
{
    ThrowIfDisposed();

    TextSegment aSegment;
    if (mpParagraph)
        aSegment = mpParagraph->GetTextSegment(+1, nLocalCharacterIndex, nTextType);

    return aSegment;
}

sal_Bool SAL_CALL AccessibleParagraph::copyText (
    sal_Int32,
    sal_Int32)
{
    ThrowIfDisposed();

    // Return false because copying to clipboard is not supported.
    // It IS supported in the notes view.  There is no need to duplicate
    // this here.
    return false;
}

sal_Bool SAL_CALL AccessibleParagraph::scrollSubstringTo(
    sal_Int32,
    sal_Int32,
    AccessibleScrollType)
{
    return false;
}

//----- protected -------------------------------------------------------------

awt::Point AccessibleParagraph::GetRelativeLocation()
{
    awt::Point aLocation (AccessibleObject::GetRelativeLocation());
    if (mpParagraph)
    {
        const awt::Point aParagraphLocation (mpParagraph->GetRelativeLocation());
        aLocation.X += aParagraphLocation.X;
        aLocation.Y += aParagraphLocation.Y;
    }

    return aLocation;
}

awt::Size AccessibleParagraph::GetSize()
{
    if (mpParagraph)
        return mpParagraph->GetSize();
    else
        return AccessibleObject::GetSize();
}

awt::Point AccessibleParagraph::GetAbsoluteParentLocation()
{
    if (mxParentAccessible.is())
    {
        Reference<XAccessibleContext> xParentContext =
            mxParentAccessible->getAccessibleContext();
        if (xParentContext.is())
        {
            Reference<XAccessibleComponent> xGrandParentComponent(
                xParentContext->getAccessibleParent(), UNO_QUERY);
            if (xGrandParentComponent.is())
                return xGrandParentComponent->getLocationOnScreen();
        }
    }

    return awt::Point();
}

bool AccessibleParagraph::GetWindowState (const sal_Int64 nType) const
{
    switch (nType)
    {
        case AccessibleStateType::EDITABLE:
            return bool(mpParagraph);

        case AccessibleStateType::ACTIVE:
            return true;

        default:
            return AccessibleObject::GetWindowState(nType);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
