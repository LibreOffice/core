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
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XAutoTextGroup.hpp>
#include <com/sun/star/text/XAutoTextEntry.hpp>
#include <com/sun/star/text/XAutoTextContainer2.hpp>
#include <com/sun/star/text/XText.hpp>
#include <svl/itemprop.hxx>
#include <svl/lstner.hxx>
#include <cppuhelper/implbase.hxx>
#include <svtools/unoevent.hxx>
class SwTextBlocks;
class SwGlossaries;
class SwDoc;
class SwDocShell;
class SwXBodyText;

#ifndef SW_DECL_SWDOCSHELL_DEFINED
#define SW_DECL_SWDOCSHELL_DEFINED
#include <tools/ref.hxx>
typedef tools::SvRef<SwDocShell> SwDocShellRef;
#endif

css::uno::Reference< css::uno::XInterface >
    SAL_CALL SwXAutoTextContainer_createInstance(
        const css::uno::Reference< css::lang::XMultiServiceFactory > & ) throw ( css::uno::Exception );

class SwXAutoTextContainer : public cppu::WeakImplHelper
<
    css::text::XAutoTextContainer2,
    css::lang::XServiceInfo
>
{
    SwGlossaries *pGlossaries;

protected:
    virtual ~SwXAutoTextContainer();    // ref-counted objects are not to be deleted from outside -> protected dtor

public:
    SwXAutoTextContainer();

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;

    //XNameAccess
    virtual css::uno::Any SAL_CALL getByName(const OUString& Name)  throw( css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) throw( css::uno::RuntimeException, std::exception ) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(css::uno::RuntimeException, std::exception) override;

    //XAutoTextContainer
    virtual css::uno::Reference< css::text::XAutoTextGroup >  SAL_CALL insertNewByName(const OUString& aGroupName) throw( css::lang::IllegalArgumentException, css::container::ElementExistException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeByName(const OUString& aGroupName) throw( css::container::NoSuchElementException, css::uno::RuntimeException, std::exception ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

};

class SwXAutoTextGroup : public cppu::WeakImplHelper
<
    css::text::XAutoTextGroup,
    css::beans::XPropertySet,
    css::lang::XServiceInfo,
    css::container::XIndexAccess,
    css::container::XNamed,
    css::lang::XUnoTunnel
>
{
    const SfxItemPropertySet* pPropSet;
    SwGlossaries*           pGlossaries;
    OUString                sName;
    OUString                m_sGroupName;   // prefix m_ to disambiguate from some local vars in the implementation

protected:
    virtual ~SwXAutoTextGroup();    // ref-counted objects are not to be deleted from outside -> protected dtor

public:
        SwXAutoTextGroup(const OUString& rName, SwGlossaries* pGloss);

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException, std::exception) override;

    //XAutoTextGroup
    virtual css::uno::Sequence< OUString > SAL_CALL getTitles() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL renameByName(const OUString& aElementName, const OUString& aNewElementName, const OUString& aNewElementTitle) throw( css::lang::IllegalArgumentException, css::container::ElementExistException, css::io::IOException, css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::text::XAutoTextEntry >  SAL_CALL insertNewByName(const OUString& aName, const OUString& aTitle, const css::uno::Reference< css::text::XTextRange > & xTextRange) throw( css::container::ElementExistException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeByName(const OUString& aEntryName) throw( css::container::NoSuchElementException, css::uno::RuntimeException, std::exception ) override;

    //XNamed
    virtual OUString SAL_CALL getName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setName(const OUString& Name_) throw( css::uno::RuntimeException, std::exception ) override;

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;

    //XNameAccess
    virtual css::uno::Any SAL_CALL getByName(const OUString& Name)  throw( css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) throw( css::uno::RuntimeException, std::exception ) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    void    Invalidate();
};

class SwXAutoTextEntry
        :public SfxListener
        ,public cppu::WeakImplHelper
        <
            css::text::XAutoTextEntry,
            css::lang::XServiceInfo,
            css::lang::XUnoTunnel,
            css::text::XText,
            css::document::XEventsSupplier
        >
{
    SwGlossaries*   pGlossaries;
    OUString        sGroupName;
    OUString        sEntryName;
    SwDocShellRef   xDocSh;
    SwXBodyText*    pBodyText;
    css::uno::Reference < css::lang::XServiceInfo> xBodyText;

    void EnsureBodyText ()
    {
        if ( !pBodyText )
            GetBodyText();
    }
    void GetBodyText ();

protected:
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

protected:
    virtual ~SwXAutoTextEntry();    // ref-counted objects are not to be deleted from outside -> protected dtor

public:
    SwXAutoTextEntry(SwGlossaries* , const OUString& rGroupName, const OUString& rEntryName);

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException, std::exception) override;

    //XText
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL createTextCursor() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL createTextCursorByRange(const css::uno::Reference< css::text::XTextRange > & aTextPosition) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL insertString(const css::uno::Reference< css::text::XTextRange > & xRange, const OUString& aString, sal_Bool bAbsorb) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL insertControlCharacter(const css::uno::Reference< css::text::XTextRange > & xRange, sal_Int16 nControlCharacter, sal_Bool bAbsorb) throw( css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL insertTextContent(const css::uno::Reference< css::text::XTextRange > & xRange, const css::uno::Reference< css::text::XTextContent > & xContent, sal_Bool bAbsorb) throw( css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeTextContent(const css::uno::Reference< css::text::XTextContent > & xContent) throw( css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;

    //XTextRange
    virtual css::uno::Reference< css::text::XText >  SAL_CALL getText() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL  getStart() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL   getEnd() throw( css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL  getString() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL  setString(const OUString& aString) throw( css::uno::RuntimeException, std::exception ) override;

    //XAutoTextEntry
    virtual void    SAL_CALL applyTo(const css::uno::Reference< css::text::XTextRange > & xRange)throw( css::uno::RuntimeException, std::exception ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    // XEventsSupplier
    virtual css::uno::Reference< css::container::XNameReplace > SAL_CALL getEvents(  ) throw( css::uno::RuntimeException, std::exception ) override;

    void    Invalidate() {pGlossaries = nullptr;}
    const SwGlossaries* GetGlossaries() { return pGlossaries; }
    const OUString&   GetGroupName() {return sGroupName;}
    const OUString&   GetEntryName() {return sEntryName;}
};

/** Implement the XNameAccess for the AutoText events */
class SwAutoTextEventDescriptor : public SvBaseEventDescriptor
{
    OUString sSwAutoTextEventDescriptor;

    SwXAutoTextEntry& rAutoTextEntry;

    using SvBaseEventDescriptor::replaceByName;
    using SvBaseEventDescriptor::getByName;

public:
    SwAutoTextEventDescriptor(  SwXAutoTextEntry& rAutoText );

    virtual ~SwAutoTextEventDescriptor();

    virtual OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException, std::exception ) override;

protected:

    virtual void replaceByName(
        const sal_uInt16 nEvent,        /// item ID of event
        const SvxMacro& rMacro)     /// event (will be copied)
            throw(
                css::lang::IllegalArgumentException,
                css::container::NoSuchElementException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException) override;

    virtual void getByName(
        SvxMacro& rMacro,           /// macro to be filled
        const sal_uInt16 nEvent )       /// item ID of event
            throw(
                css::container::NoSuchElementException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
