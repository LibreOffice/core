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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_UNOATXT_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_UNOATXT_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/XAutoTextGroup.hpp>
#include <com/sun/star/text/XAutoTextEntry.hpp>
#include <com/sun/star/text/XAutoTextContainer2.hpp>
#include <com/sun/star/text/XText.hpp>
#include <svl/itemprop.hxx>
#include <svl/lstner.hxx>
#include <comphelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <svtools/unoevent.hxx>

class SwTextBlocks;
class SwGlossaries;
class SwDoc;
class SwDocShell;
class SwXBodyText;

#ifndef SW_DECL_SWDOCSHELL_DEFINED
#define SW_DECL_SWDOCSHELL_DEFINED
typedef rtl::Reference<SwDocShell> SwDocShellRef;
#endif

class SwXAutoTextContainer final : public cppu::WeakImplHelper
<
    css::text::XAutoTextContainer2,
    css::lang::XServiceInfo
>
{
    SwGlossaries *m_pGlossaries;

    virtual ~SwXAutoTextContainer() override;    // ref-counted objects are not to be deleted from outside -> protected dtor

public:
    SwXAutoTextContainer();

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

    //XNameAccess
    virtual css::uno::Any SAL_CALL getByName(const OUString& Name) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    //XAutoTextContainer
    virtual css::uno::Reference< css::text::XAutoTextGroup >  SAL_CALL insertNewByName(const OUString& aGroupName) override;
    virtual void SAL_CALL removeByName(const OUString& aGroupName) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

};

class SwXAutoTextGroup final : public cppu::WeakImplHelper
<
    css::text::XAutoTextGroup,
    css::beans::XPropertySet,
    css::lang::XServiceInfo,
    css::container::XIndexAccess,
    css::container::XNamed
>
{
    const SfxItemPropertySet* m_pPropSet;
    SwGlossaries*           m_pGlossaries;
    OUString                m_sName;
    OUString                m_sGroupName;   // prefix m_ to disambiguate from some local vars in the implementation

    virtual ~SwXAutoTextGroup() override;    // ref-counted objects are not to be deleted from outside -> protected dtor

public:
        SwXAutoTextGroup(const OUString& rName, SwGlossaries* pGloss);

    //XAutoTextGroup
    virtual css::uno::Sequence< OUString > SAL_CALL getTitles() override;
    virtual void SAL_CALL renameByName(const OUString& aElementName, const OUString& aNewElementName, const OUString& aNewElementTitle) override;
    virtual css::uno::Reference< css::text::XAutoTextEntry >  SAL_CALL insertNewByName(const OUString& aName, const OUString& aTitle, const css::uno::Reference< css::text::XTextRange > & xTextRange) override;
    virtual void SAL_CALL removeByName(const OUString& aEntryName) override;

    //XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName(const OUString& Name_) override;

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

    //XNameAccess
    virtual css::uno::Any SAL_CALL getByName(const OUString& Name) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    void    Invalidate();
};

class SwXAutoTextEntry final
        :public SfxListener
        ,public comphelper::WeakComponentImplHelper
        <
            css::text::XAutoTextEntry,
            css::lang::XServiceInfo,
            css::text::XText,
            css::document::XEventsSupplier
        >
{
    SwGlossaries*   m_pGlossaries;
    OUString        m_sGroupName;
    OUString        m_sEntryName;
    SwDocShellRef   m_xDocSh;
    rtl::Reference<SwXBodyText>
                    mxBodyText;

    void EnsureBodyText ()
    {
        if ( !mxBodyText.is() )
            GetBodyText();
    }
    void GetBodyText ();

    void disposing(std::unique_lock<std::mutex>&) override;

    /** ensure that the current content (which may only be in-memory so far) is flushed to the auto text group file

        <p>If somebody modifies an auto text via this class, then this is not directly reflected to the respective
        glossaries file (on disk), instead we hold a copy of this text (in [p|x]BodyText). On the other hand,
        in applyTo, we do not work with this _copy_, but just tell the target for the application to insert
        the content which we're responsible for - and this target doesn't know about our copy, but only
        about the persistent version.</br>
        So we need to ensure that before somebody else does something with our auto text, we flush our
        (in-memory) copy to disk.</p>

    */
    void    implFlushDocument( bool _bCloseDoc = false );

    // SfxListener overridables
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    virtual ~SwXAutoTextEntry() override;    // ref-counted objects are not to be deleted from outside -> protected dtor

public:
    SwXAutoTextEntry(SwGlossaries* , OUString aGroupName, OUString aEntryName);

    //XText
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL createTextCursor() override;
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL createTextCursorByRange(const css::uno::Reference< css::text::XTextRange > & aTextPosition) override;
    virtual void SAL_CALL insertString(const css::uno::Reference< css::text::XTextRange > & xRange, const OUString& aString, sal_Bool bAbsorb) override;
    virtual void SAL_CALL insertControlCharacter(const css::uno::Reference< css::text::XTextRange > & xRange, sal_Int16 nControlCharacter, sal_Bool bAbsorb) override;
    virtual void SAL_CALL insertTextContent(const css::uno::Reference< css::text::XTextRange > & xRange, const css::uno::Reference< css::text::XTextContent > & xContent, sal_Bool bAbsorb) override;
    virtual void SAL_CALL removeTextContent(const css::uno::Reference< css::text::XTextContent > & xContent) override;

    //XTextRange
    virtual css::uno::Reference< css::text::XText >  SAL_CALL getText() override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL  getStart() override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL   getEnd() override;
    virtual OUString SAL_CALL  getString() override;
    virtual void SAL_CALL  setString(const OUString& aString) override;

    //XAutoTextEntry
    virtual void    SAL_CALL applyTo(const css::uno::Reference< css::text::XTextRange > & xRange) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XEventsSupplier
    virtual css::uno::Reference< css::container::XNameReplace > SAL_CALL getEvents(  ) override;

    void    Invalidate() {m_pGlossaries = nullptr;}
    const SwGlossaries* GetGlossaries() const { return m_pGlossaries; }
    const OUString&   GetGroupName() const {return m_sGroupName;}
    const OUString&   GetEntryName() const {return m_sEntryName;}
};

/** Implement the XNameAccess for the AutoText events */
class SwAutoTextEventDescriptor final : public SvBaseEventDescriptor
{
    SwXAutoTextEntry& m_rAutoTextEntry;

    using SvBaseEventDescriptor::replaceByName;
    using SvBaseEventDescriptor::getByName;

public:
    SwAutoTextEventDescriptor(  SwXAutoTextEntry& rAutoText );

    virtual ~SwAutoTextEventDescriptor() override;

    virtual OUString SAL_CALL getImplementationName() override;

private:

    virtual void replaceByName(
        const SvMacroItemId nEvent,  /// item ID of event
        const SvxMacro& rMacro)      /// event (will be copied)
             override;

    virtual void getByName(
        SvxMacro& rMacro,             /// macro to be filled
        const SvMacroItemId nEvent )  /// item ID of event
             override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
