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

#ifndef INCLUDED_SW_INC_UNOTEXTCURSOR_HXX
#define INCLUDED_SW_INC_UNOTEXTCURSOR_HXX

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/util/XSortable.hpp>
#include <com/sun/star/document/XDocumentInsertable.hpp>
#include <com/sun/star/text/XSentenceCursor.hpp>
#include <com/sun/star/text/XWordCursor.hpp>
#include <com/sun/star/text/XParagraphCursor.hpp>
#include <com/sun/star/text/XRedline.hpp>
#include <com/sun/star/text/XMarkingAccess.hpp>

#include <cppuhelper/implbase.hxx>

#include <comphelper/uno3.hxx>

#include <unobaseclass.hxx>
#include <TextCursorHelper.hxx>

class SwDoc;
struct SwPosition;
class SwUnoCursor;

typedef ::cppu::WeakImplHelper
<   css::lang::XServiceInfo
,   css::beans::XPropertySet
,   css::beans::XPropertyState
,   css::beans::XMultiPropertySet
,   css::beans::XMultiPropertyStates
,   css::container::XEnumerationAccess
,   css::container::XContentEnumerationAccess
,   css::util::XSortable
,   css::document::XDocumentInsertable
,   css::text::XSentenceCursor
,   css::text::XWordCursor
,   css::text::XParagraphCursor
,   css::text::XRedline
,   css::text::XMarkingAccess
> SwXTextCursor_Base;

class SwXTextCursor
    : public SwXTextCursor_Base
    , public OTextCursorHelper
{

private:

    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    virtual ~SwXTextCursor();

public:

    SwXTextCursor(
            SwDoc & rDoc,
            css::uno::Reference< css::text::XText > const& xParent,
            const enum CursorType eType,
            SwPosition const& rPos,
            SwPosition const*const pMark = nullptr);
    SwXTextCursor(
            css::uno::Reference< css::text::XText > const& xParent,
            SwPaM const& rSourceCursor,
            const enum CursorType eType = CURSOR_ALL);

    SwUnoCursor& GetCursor();
    bool IsAtEndOfMeta() const;

    void DeleteAndInsert(OUString const& rText,
                const bool bForceExpandHints);

    // OTextCursorHelper
    virtual const SwPaM*        GetPaM() const override;
    virtual SwPaM*              GetPaM() override;
    virtual const SwDoc*        GetDoc() const override;
    virtual SwDoc*              GetDoc() override;

    DECLARE_XINTERFACE()

    static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(
            const css::uno::Sequence< sal_Int8 >& rIdentifier)
        throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(
            const OUString& rServiceName)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
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
            const css::uno::Reference<
                css::beans::XPropertyChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference<
                css::beans::XVetoableChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference<
                css::beans::XVetoableChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    // XPropertyState
    virtual css::beans::PropertyState SAL_CALL
        getPropertyState(const OUString& rPropertyName)
        throw (css::beans::UnknownPropertyException,
                css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL
        getPropertyStates(
            const css::uno::Sequence< OUString >& rPropertyNames)
        throw (css::beans::UnknownPropertyException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyToDefault(
            const OUString& rPropertyName)
        throw (css::beans::UnknownPropertyException,
                css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault(
            const OUString& rPropertyName)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues(
            const css::uno::Sequence< OUString >& aPropertyNames,
            const css::uno::Sequence< css::uno::Any >& aValues )
        throw (
            css::beans::PropertyVetoException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< css::uno::Any > SAL_CALL
        getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL addPropertiesChangeListener(
        const css::uno::Sequence< OUString >& aPropertyNames,
        const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL removePropertiesChangeListener(
        const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL firePropertiesChangeEvent(
        const css::uno::Sequence< OUString >& aPropertyNames,
        const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener )
        throw (css::uno::RuntimeException, std::exception) override;

    // XMultiPropertyStates
    virtual void SAL_CALL setAllPropertiesToDefault()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertiesToDefault(
            const css::uno::Sequence< OUString >&  rPropertyNames)
        throw (css::beans::UnknownPropertyException,
                css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Any >
        SAL_CALL getPropertyDefaults(
            const css::uno::Sequence< OUString >&  rPropertyNames)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements()
        throw (css::uno::RuntimeException, std::exception) override;

    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration >  SAL_CALL
        createEnumeration()
        throw (css::uno::RuntimeException, std::exception) override;

    // XContentEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
        createContentEnumeration(const OUString& rServiceName)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getAvailableServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

    // XSortable
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
        createSortDescriptor()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL sort(
            const css::uno::Sequence< css::beans::PropertyValue >& xDescriptor)
        throw (css::uno::RuntimeException,
               std::exception) override;

    // XDocumentInsertable
    virtual void SAL_CALL insertDocumentFromURL(
            const OUString& rURL,
            const css::uno::Sequence< css::beans::PropertyValue >& rOptions)
        throw (css::lang::IllegalArgumentException,
                css::io::IOException,
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

    // XTextCursor
    virtual void SAL_CALL collapseToStart()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL collapseToEnd()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isCollapsed()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL goLeft(sal_Int16 nCount, sal_Bool bExpand)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL goRight(sal_Int16 nCount, sal_Bool bExpand)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL gotoStart(sal_Bool bExpand)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL gotoEnd(sal_Bool bExpand)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL gotoRange(
            const css::uno::Reference< css::text::XTextRange >& xRange,
            sal_Bool bExpand)
        throw (css::uno::RuntimeException, std::exception) override;

    // XWordCursor
    virtual sal_Bool SAL_CALL isStartOfWord()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isEndOfWord()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL gotoNextWord(sal_Bool bExpand)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL gotoPreviousWord(sal_Bool bExpand)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL gotoEndOfWord(sal_Bool bExpand)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL gotoStartOfWord(sal_Bool bExpand)
        throw (css::uno::RuntimeException, std::exception) override;

    // XSentenceCursor
    virtual sal_Bool SAL_CALL isStartOfSentence()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isEndOfSentence()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL gotoNextSentence(sal_Bool Expand)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL gotoPreviousSentence(sal_Bool Expand)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL gotoStartOfSentence(sal_Bool Expand)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL gotoEndOfSentence(sal_Bool Expand)
        throw (css::uno::RuntimeException, std::exception) override;

    // XParagraphCursor
    virtual sal_Bool SAL_CALL isStartOfParagraph()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isEndOfParagraph()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL gotoStartOfParagraph(sal_Bool Expand)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL gotoEndOfParagraph(sal_Bool Expand)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL gotoNextParagraph(sal_Bool Expand)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL gotoPreviousParagraph(sal_Bool Expand)
        throw (css::uno::RuntimeException, std::exception) override;

    // XRedline
    virtual void SAL_CALL makeRedline(
            const OUString& rRedlineType,
            const css::uno::Sequence< css::beans::PropertyValue >& RedlineProperties)
        throw (css::lang::IllegalArgumentException,
                css::uno::RuntimeException, std::exception) override;

    //XMarkingAccess
    virtual void SAL_CALL invalidateMarkings(::sal_Int32 nType)
        throw (css::uno::RuntimeException, std::exception) override;

};

#endif // INCLUDED_SW_INC_UNOTEXTCURSOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
