/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_SW_SOURCE_CORE_ACCESS_ACCNOTEXTFRAME_HXX
#define INCLUDED_SW_SOURCE_CORE_ACCESS_ACCNOTEXTFRAME_HXX

#include "accframebase.hxx"
#include <com/sun/star/accessibility/AccessibleScrollType.hpp>
#include <com/sun/star/accessibility/XAccessibleImage.hpp>
#include <com/sun/star/accessibility/XAccessibleHypertext.hpp>

class SwFlyFrame;
class SwNoTextNode;

class SwAccessibleNoTextFrame : public  SwAccessibleFrameBase,
                                public css::accessibility::XAccessibleImage,
                                public css::accessibility::XAccessibleHypertext//Added by yangzhh for HyperLink
{
    friend class SwAccessibleNoTextHyperlink;
    css::uno::Reference< css::accessibility::XAccessibleHyperlink > m_xHyperlink;
    OUString msTitle;
    OUString msDesc;

protected:
    virtual ~SwAccessibleNoTextFrame() override;

    const SwNoTextNode *GetNoTextNode() const;

    virtual void Notify(const SfxHint&) override;

public:
    SwAccessibleNoTextFrame( std::shared_ptr<SwAccessibleMap> const& pInitMap,
                             sal_Int16 nInitRole,
                             const SwFlyFrame *pFlyFrame );

    // XAccessibleContext

    // #i73249# - Return the object's current name.
    virtual OUString SAL_CALL
        getAccessibleName() override;

    /// Return this object's description.
    virtual OUString SAL_CALL
        getAccessibleDescription() override;

    // XInterface methods need to be implemented to disambiguate
    // between those inherited through SwAccessibleContext and
    // XAccessibleImage.

    virtual css::uno::Any SAL_CALL queryInterface(
        const css::uno::Type& aType ) override;

    virtual void SAL_CALL acquire(  ) noexcept override
        { SwAccessibleContext::acquire(); };

    virtual void SAL_CALL release(  ) noexcept override
        { SwAccessibleContext::release(); };

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;

    // XAccessibleImage
    virtual OUString SAL_CALL
        getAccessibleImageDescription(  ) override;

    virtual sal_Int32 SAL_CALL
        getAccessibleImageHeight(  ) override;

    virtual sal_Int32 SAL_CALL
        getAccessibleImageWidth(  ) override;

    // The object is not visible any longer and should be destroyed
    virtual void Dispose(bool bRecursive, bool bCanSkipInvisible = true) override;

    virtual sal_Int32 SAL_CALL getCaretPosition(  ) override;
    virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) override;
    virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) override;//Shen Zhen Jie changed sal_Unicode to sal_uInt32
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes ) override;
    virtual css::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) override;
    virtual sal_Int32 SAL_CALL getCharacterCount(  ) override;
    virtual sal_Int32 SAL_CALL getIndexAtPoint( const css::awt::Point& aPoint ) override;
    virtual OUString SAL_CALL getSelectedText(  ) override;
    virtual sal_Int32 SAL_CALL getSelectionStart(  ) override;
    virtual sal_Int32 SAL_CALL getSelectionEnd(  ) override;
    virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual OUString SAL_CALL getText(  ) override;
    virtual OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual sal_Bool SAL_CALL scrollSubstringTo( sal_Int32 nStartIndex, sal_Int32 nEndIndex, css::accessibility::AccessibleScrollType aScrollType) override;

    // XAccessibleHypertext
    virtual sal_Int32 SAL_CALL getHyperLinkCount() override;
    virtual css::uno::Reference< css::accessibility::XAccessibleHyperlink >
    SAL_CALL getHyperLink( sal_Int32 nLinkIndex ) override;
    virtual sal_Int32 SAL_CALL getHyperLinkIndex( sal_Int32 nCharIndex ) override;

    SwAccessibleMap *GetAccessibleMap(){ return GetMap();}

public:
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
