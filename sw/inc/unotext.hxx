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

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCopy.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/XRelativeTextContentInsert.hpp>
#include <com/sun/star/text/XRelativeTextContentRemove.hpp>
#include <com/sun/star/text/XTextAppendAndConvert.hpp>

#include <unobaseclass.hxx>

namespace com { namespace sun { namespace star {
    namespace text {
        class XTextContent;
    }
} } }

class SfxItemPropertySet;
class SwDoc;
class SwStartNode;
class SwPaM;
class OTextCursorHelper;
class SwXTextRange;

class SwXText
    : public css::lang::XTypeProvider
    , public css::lang::XUnoTunnel
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
    virtual bool CheckForOwnMemberMeta(
            const SwPaM & rPam, const bool bAbsorb)
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException);

protected:

    bool            IsValid() const;
    void            Invalidate();
    void            SetDoc(SwDoc *const pDoc);

    virtual ~SwXText();

public: /*not protected because C++ is retarded*/
    virtual const SwStartNode *GetStartNode() const;

public:

    SwXText(SwDoc *const pDoc, const enum CursorType eType);

    const SwDoc*    GetDoc() const;
          SwDoc*    GetDoc();

    virtual css::uno::Reference< css::text::XTextCursor >
        CreateCursor()
        throw (css::uno::RuntimeException);

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface(
            const css::uno::Type& rType)
        throw (css::uno::RuntimeException, std::exception) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type >
        SAL_CALL getTypes()
        throw (css::uno::RuntimeException, std::exception) override;

    static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(
            const css::uno::Sequence< sal_Int8 >& rIdentifier)
        throw (css::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue(
            const OUString& rPropertyName,
            const css::uno::Any& rValue)
        throw (css::beans::UnknownPropertyException,
                css::beans::PropertyVetoException,
                css::lang::IllegalArgumentException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
            const OUString& rPropertyName)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference<
                css::beans::XPropertyChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    // XTextRange
    virtual css::uno::Reference< css::text::XText >
        SAL_CALL getText()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getStart()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getEnd()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getString()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setString(const OUString& rString)
        throw (css::uno::RuntimeException, std::exception) override;

    // XSimpleText
    virtual void SAL_CALL insertString(
            const css::uno::Reference< css::text::XTextRange > & xRange,
            const OUString& aString, sal_Bool bAbsorb)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL insertControlCharacter(
            const css::uno::Reference< css::text::XTextRange > & xRange,
            sal_Int16 nControlCharacter, sal_Bool bAbsorb)
        throw (css::lang::IllegalArgumentException,
                css::uno::RuntimeException, std::exception) override;

    // XText
    virtual void SAL_CALL insertTextContent(
            const css::uno::Reference< css::text::XTextRange > & xRange,
            const css::uno::Reference< css::text::XTextContent > & xContent,
            sal_Bool bAbsorb)
        throw (css::lang::IllegalArgumentException,
            css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeTextContent(
            const css::uno::Reference< css::text::XTextContent > & xContent)
        throw (css::container::NoSuchElementException,
                css::uno::RuntimeException, std::exception) override;

    // XParagraphAppend
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
        finishParagraph(
            const css::uno::Sequence< css::beans::PropertyValue >&  rCharacterAndParagraphProperties)
        throw (css::lang::IllegalArgumentException,
                css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
        finishParagraphInsert(
            const css::uno::Sequence< css::beans::PropertyValue >&  rCharacterAndParagraphProperties,
            const css::uno::Reference< css::text::XTextRange >&     xInsertPosition)
        throw (css::lang::IllegalArgumentException,
                css::uno::RuntimeException, std::exception) override;

    // XTextPortionAppend
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
        appendTextPortion(
            const OUString& rText,
            const css::uno::Sequence< css::beans::PropertyValue >& rCharacterAndParagraphProperties)
        throw (css::lang::IllegalArgumentException,
               css::beans::UnknownPropertyException,
               css::beans::PropertyVetoException,
               css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
        insertTextPortion(
            const OUString& rText,
            const css::uno::Sequence< css::beans::PropertyValue >&  rCharacterAndParagraphProperties,
            const css::uno::Reference< css::text::XTextRange >&     rTextRange)
        throw (css::lang::IllegalArgumentException,
               css::beans::UnknownPropertyException,
               css::beans::PropertyVetoException,
               css::uno::RuntimeException,
               std::exception) override;

    // XTextContentAppend
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
        appendTextContent(
            const css::uno::Reference< css::text::XTextContent >&   xTextContent,
            const css::uno::Sequence< css::beans::PropertyValue >&  rCharacterAndParagraphProperties)
        throw (css::lang::IllegalArgumentException,
                css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
        insertTextContentWithProperties(
            const css::uno::Reference< css::text::XTextContent >&  xTextContent,
            const css::uno::Sequence< css::beans::PropertyValue >& rCharacterAndParagraphProperties,
            const css::uno::Reference< css::text::XTextRange >& xInsertPosition)
        throw (css::lang::IllegalArgumentException,
                css::uno::RuntimeException, std::exception) override;

    // XTextConvert
    virtual css::uno::Reference< css::text::XTextContent > SAL_CALL
        convertToTextFrame(
            const css::uno::Reference< css::text::XTextRange >& xStart,
            const css::uno::Reference< css::text::XTextRange >& xEnd,
            const css::uno::Sequence< css::beans::PropertyValue >& xFrameProperties)
        throw (css::lang::IllegalArgumentException,
               css::beans::UnknownPropertyException,
               css::beans::PropertyVetoException,
               css::uno::RuntimeException, std::exception) override;
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
                rTableProperties)
        throw (css::lang::IllegalArgumentException,
                css::uno::RuntimeException, std::exception) override;

    // XTextCopy
    virtual void SAL_CALL copyText(
            const css::uno::Reference< css::text::XTextCopy >& xSource )
        throw (css::uno::RuntimeException, std::exception) override;

    // XTextRangeCompare
    sal_Int16 SAL_CALL compareRegionStarts(
            const css::uno::Reference< css::text::XTextRange >& xR1,
            const css::uno::Reference< css::text::XTextRange >& xR2)
        throw (css::lang::IllegalArgumentException,
                css::uno::RuntimeException, std::exception) override;
    sal_Int16 SAL_CALL compareRegionEnds(
            const css::uno::Reference< css::text::XTextRange >& xR1,
            const css::uno::Reference< css::text::XTextRange >& xR2)
        throw (css::lang::IllegalArgumentException,
                css::uno::RuntimeException, std::exception) override;

    // XRelativeTextContentInsert
    virtual void SAL_CALL insertTextContentBefore(
            const css::uno::Reference< css::text::XTextContent>& xNewContent,
            const css::uno::Reference< css::text::XTextContent>& xSuccessor)
        throw (css::lang::IllegalArgumentException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL insertTextContentAfter(
            const css::uno::Reference< css::text::XTextContent>& xNewContent,
            const css::uno::Reference< css::text::XTextContent>& xPredecessor)
        throw (css::lang::IllegalArgumentException,
                css::uno::RuntimeException, std::exception) override;

    // XRelativeTextContentRemove
    virtual void SAL_CALL removeTextContentBefore(
            const css::uno::Reference< css::text::XTextContent>& xSuccessor)
        throw (css::lang::IllegalArgumentException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeTextContentAfter(
            const css::uno::Reference< css::text::XTextContent>& xPredecessor)
        throw (css::lang::IllegalArgumentException,
                css::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_SW_INC_UNOTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
