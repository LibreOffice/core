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

#ifndef SW_UNOTEXT_HXX
#define SW_UNOTEXT_HXX

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
    : public ::com::sun::star::lang::XTypeProvider
    , public ::com::sun::star::lang::XUnoTunnel
    , public ::com::sun::star::beans::XPropertySet
    , public ::com::sun::star::text::XTextAppendAndConvert
    , public ::com::sun::star::text::XTextCopy
    , public ::com::sun::star::text::XTextRangeCompare
    , public ::com::sun::star::text::XRelativeTextContentInsert
    , public ::com::sun::star::text::XRelativeTextContentRemove
{

private:

    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    virtual void PrepareForAttach(
            ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xRange,
            SwPaM const & rPam);
    virtual bool CheckForOwnMemberMeta(
            const SwPaM & rPam, const bool bAbsorb)
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);

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

    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextCursor >
        CreateCursor()
        throw (::com::sun::star::uno::RuntimeException);


    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
            const ::com::sun::star::uno::Type& rType)
        throw (::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >
        SAL_CALL getTypes()
        throw (::com::sun::star::uno::RuntimeException);

    static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(
            const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier)
        throw (::com::sun::star::uno::RuntimeException);

    // XPropertySet
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Any& rValue)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
            const ::rtl::OUString& rPropertyName)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertyChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertyChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XVetoableChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XVetoableChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    // XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >
        SAL_CALL getText()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > SAL_CALL getStart()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > SAL_CALL getEnd()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getString()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setString(const ::rtl::OUString& rString)
        throw (::com::sun::star::uno::RuntimeException);

    // XSimpleText
    virtual void SAL_CALL insertString(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xRange,
            const ::rtl::OUString& aString, sal_Bool bAbsorb)
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertControlCharacter(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xRange,
            sal_Int16 nControlCharacter, sal_Bool bAbsorb)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);

    // XText
    virtual void SAL_CALL insertTextContent(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xRange,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextContent > & xContent,
            sal_Bool bAbsorb)
        throw (::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTextContent(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextContent > & xContent)
        throw (::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::uno::RuntimeException);

    // XParagraphAppend
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > SAL_CALL
        finishParagraph(
            const ::com::sun::star::uno::Sequence<
                    ::com::sun::star::beans::PropertyValue >&
                rCharacterAndParagraphProperties)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > SAL_CALL
        finishParagraphInsert(
            const ::com::sun::star::uno::Sequence<
                    ::com::sun::star::beans::PropertyValue >&
                rCharacterAndParagraphProperties,
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::text::XTextRange >&
                xInsertPosition)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);

    // XTextPortionAppend
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > SAL_CALL
        appendTextPortion(
            const ::rtl::OUString& rText,
            const ::com::sun::star::uno::Sequence<
                    ::com::sun::star::beans::PropertyValue >&
                rCharacterAndParagraphProperties)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > SAL_CALL
        insertTextPortion(
            const ::rtl::OUString& rText,
            const ::com::sun::star::uno::Sequence<
                    ::com::sun::star::beans::PropertyValue >&
                rCharacterAndParagraphProperties,
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::text::XTextRange >&
                rTextRange)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);

    // XTextContentAppend
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > SAL_CALL
        appendTextContent(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextContent >& xTextContent,
            const ::com::sun::star::uno::Sequence<
                    ::com::sun::star::beans::PropertyValue >&
                rCharacterAndParagraphProperties)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > SAL_CALL
        insertTextContentWithProperties(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextContent >& xTextContent,
            const ::com::sun::star::uno::Sequence<
                    ::com::sun::star::beans::PropertyValue >&
                rCharacterAndParagraphProperties,
            const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xInsertPosition)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);

    // XTextConvert
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextContent > SAL_CALL
        convertToTextFrame(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange >& xStart,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange >& xEnd,
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::PropertyValue >& xFrameProperties)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextTable > SAL_CALL
        convertToTable(
            ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Sequence<
                    ::com::sun::star::uno::Sequence<
                        ::com::sun::star::uno::Reference<
                            ::com::sun::star::text::XTextRange > > > > const&
                rTableRanges,
           ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Sequence<
                    ::com::sun::star::uno::Sequence<
                        ::com::sun::star::beans::PropertyValue > > > const&
                rCellProperties,
           ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Sequence<
                    ::com::sun::star::beans::PropertyValue > > const&
                rRowProperties,
           ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::PropertyValue > const&
                rTableProperties)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);

    // XTextCopy
    virtual void SAL_CALL copyText(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextCopy >& xSource )
        throw (::com::sun::star::uno::RuntimeException);

    // XTextRangeCompare
    sal_Int16 SAL_CALL compareRegionStarts(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange >& xR1,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange >& xR2)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL compareRegionEnds(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange >& xR1,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange >& xR2)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);

    // XRelativeTextContentInsert
    virtual void SAL_CALL insertTextContentBefore(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextContent>& xNewContent,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextContent>& xSuccessor)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertTextContentAfter(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextContent>& xNewContent,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextContent>& xPredecessor)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);

    // XRelativeTextContentRemove
    virtual void SAL_CALL removeTextContentBefore(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextContent>& xSuccessor)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTextContentAfter(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextContent>& xPredecessor)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);
};

#endif // SW_UNOTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
