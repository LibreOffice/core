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

#ifndef INCLUDED_SW_INC_UNOTEXT_HXX
#define INCLUDED_SW_INC_UNOTEXT_HXX

#include "swdllapi.h"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextCopy.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/XRelativeTextContentInsert.hpp>
#include <com/sun/star/text/XRelativeTextContentRemove.hpp>
#include <com/sun/star/text/XTextAppendAndConvert.hpp>
#include <sal/types.h>

#include "unobaseclass.hxx"

namespace com::sun::star {
    namespace text {
        class XTextContent;
        class XText;
    }
}

class SwDoc;
class SwStartNode;
class SwPaM;
class SwXTextCursor;

class SAL_DLLPUBLIC_RTTI SwXText
    : public css::lang::XTypeProvider
    , public css::beans::XPropertySet
    , public css::text::XTextAppendAndConvert
    , public css::text::XTextCopy
    , public css::text::XTextRangeCompare
    , public css::text::XRelativeTextContentInsert
    , public css::text::XRelativeTextContentRemove
{

private:

    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    virtual void PrepareForAttach(
            css::uno::Reference< css::text::XTextRange > & xRange,
            SwPaM const & rPam);
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    virtual bool CheckForOwnMemberMeta(
            const SwPaM & rPam, const bool bAbsorb);

protected:

    bool            IsValid() const;
    void            Invalidate();
    void            SetDoc(SwDoc *const pDoc);

    virtual ~SwXText();

public: /*not protected because C++ is retarded*/
    virtual const SwStartNode *GetStartNode() const;

public:

    SwXText(SwDoc *const pDoc, const CursorType eType);

    const SwDoc*    GetDoc() const;
          SwDoc*    GetDoc();

    // declare these here to resolve ambiguity when we declared rtl::Reference<subtype-of-SwXText>
    virtual void SAL_CALL acquire() override = 0;
    virtual void SAL_CALL release() override = 0;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface(
            const css::uno::Type& rType) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type >
        SAL_CALL getTypes() override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue(
            const OUString& rPropertyName,
            const css::uno::Any& rValue) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
            const OUString& rPropertyName) override;
    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference<
                css::beans::XPropertyChangeListener >& xListener) override;
    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener) override;
    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener) override;
    virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener) override;

    // XTextRange
    virtual css::uno::Reference< css::text::XText >
        SAL_CALL getText() override;
    SW_DLLPUBLIC virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getStart() override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getEnd() override;
    virtual OUString SAL_CALL getString() override;
    virtual void SAL_CALL setString(const OUString& rString) override;

    // XSimpleText
    virtual void SAL_CALL insertString(
            const css::uno::Reference< css::text::XTextRange > & xRange,
            const OUString& aString, sal_Bool bAbsorb) override;
    virtual void SAL_CALL insertControlCharacter(
            const css::uno::Reference< css::text::XTextRange > & xRange,
            sal_Int16 nControlCharacter, sal_Bool bAbsorb) override;
    SW_DLLPUBLIC virtual css::uno::Reference< css::text::XTextCursor > SAL_CALL createTextCursorByRange(
            const ::css::uno::Reference< ::css::text::XTextRange >& aTextPosition ) override final;
    virtual rtl::Reference< SwXTextCursor > createXTextCursorByRange(
            const ::css::uno::Reference< ::css::text::XTextRange >& aTextPosition ) = 0;
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL createTextCursor() override final;
    virtual rtl::Reference< SwXTextCursor > createXTextCursor() = 0;

    // XText
    virtual void SAL_CALL insertTextContent(
            const css::uno::Reference< css::text::XTextRange > & xRange,
            const css::uno::Reference< css::text::XTextContent > & xContent,
            sal_Bool bAbsorb) override;
    virtual void SAL_CALL removeTextContent(
            const css::uno::Reference< css::text::XTextContent > & xContent) override;

    // XParagraphAppend
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
        finishParagraph(
            const css::uno::Sequence< css::beans::PropertyValue >&  rCharacterAndParagraphProperties) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
        finishParagraphInsert(
            const css::uno::Sequence< css::beans::PropertyValue >&  rCharacterAndParagraphProperties,
            const css::uno::Reference< css::text::XTextRange >&     xInsertPosition) override;

    // XTextPortionAppend
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
        appendTextPortion(
            const OUString& rText,
            const css::uno::Sequence< css::beans::PropertyValue >& rCharacterAndParagraphProperties) override;

    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
        insertTextPortion(
            const OUString& rText,
            const css::uno::Sequence< css::beans::PropertyValue >&  rCharacterAndParagraphProperties,
            const css::uno::Reference< css::text::XTextRange >&     rTextRange) override;

    // XTextContentAppend
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
        appendTextContent(
            const css::uno::Reference< css::text::XTextContent >&   xTextContent,
            const css::uno::Sequence< css::beans::PropertyValue >&  rCharacterAndParagraphProperties) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
        insertTextContentWithProperties(
            const css::uno::Reference< css::text::XTextContent >&  xTextContent,
            const css::uno::Sequence< css::beans::PropertyValue >& rCharacterAndParagraphProperties,
            const css::uno::Reference< css::text::XTextRange >& xInsertPosition) override;

    // XTextConvert
    virtual css::uno::Reference< css::text::XTextContent > SAL_CALL
        convertToTextFrame(
            const css::uno::Reference< css::text::XTextRange >& xStart,
            const css::uno::Reference< css::text::XTextRange >& xEnd,
            const css::uno::Sequence< css::beans::PropertyValue >& xFrameProperties) override;
    virtual css::uno::Reference<
                css::text::XTextTable > SAL_CALL
        convertToTable(
            css::uno::Sequence<
                css::uno::Sequence<
                    css::uno::Sequence<
                        css::uno::Reference<
                            css::text::XTextRange > > > > const&
                rTableRanges,
           css::uno::Sequence<
                css::uno::Sequence<
                    css::uno::Sequence<
                        css::beans::PropertyValue > > > const&
                rCellProperties,
           css::uno::Sequence<
                css::uno::Sequence<
                    css::beans::PropertyValue > > const&
                rRowProperties,
           css::uno::Sequence<
                css::beans::PropertyValue > const&
                rTableProperties) override;

    // XTextCopy
    virtual void SAL_CALL copyText(
            const css::uno::Reference< css::text::XTextCopy >& xSource ) override;

    // XTextRangeCompare
    sal_Int16 SAL_CALL compareRegionStarts(
            const css::uno::Reference< css::text::XTextRange >& xR1,
            const css::uno::Reference< css::text::XTextRange >& xR2) override;
    sal_Int16 SAL_CALL compareRegionEnds(
            const css::uno::Reference< css::text::XTextRange >& xR1,
            const css::uno::Reference< css::text::XTextRange >& xR2) override;

    // XRelativeTextContentInsert
    virtual void SAL_CALL insertTextContentBefore(
            const css::uno::Reference< css::text::XTextContent>& xNewContent,
            const css::uno::Reference< css::text::XTextContent>& xSuccessor) override;
    virtual void SAL_CALL insertTextContentAfter(
            const css::uno::Reference< css::text::XTextContent>& xNewContent,
            const css::uno::Reference< css::text::XTextContent>& xPredecessor) override;

    // XRelativeTextContentRemove
    virtual void SAL_CALL removeTextContentBefore(
            const css::uno::Reference< css::text::XTextContent>& xSuccessor) override;
    virtual void SAL_CALL removeTextContentAfter(
            const css::uno::Reference< css::text::XTextContent>& xPredecessor) override;
};

#endif // INCLUDED_SW_INC_UNOTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
