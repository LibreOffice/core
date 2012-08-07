/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _UNOATXT_HXX
#define _UNOATXT_HXX

#include <com/sun/star/text/XAutoTextGroup.hpp>
#include <com/sun/star/text/XAutoTextEntry.hpp>
#include <com/sun/star/text/XAutoTextContainer.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <svl/itemprop.hxx>
#include <svl/lstner.hxx>
#include <cppuhelper/implbase3.hxx> // helper for implementations
#include <cppuhelper/implbase4.hxx> // helper for implementations
#include <cppuhelper/implbase5.hxx> // helper for implementations
#include <cppuhelper/implbase6.hxx> // helper for implementations
#include <svtools/unoevent.hxx>
class SwTextBlocks;
class SwGlossaries;
class SwDoc;
class SwDocShell;
class SwXBodyText;

#ifndef SW_DECL_SWDOCSHELL_DEFINED
#define SW_DECL_SWDOCSHELL_DEFINED
#include <tools/ref.hxx>
SV_DECL_REF( SwDocShell )
#endif

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
    SAL_CALL SwXAutoTextContainer_createInstance(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & ) throw ( ::com::sun::star::uno::Exception );

class SwXAutoTextContainer : public cppu::WeakImplHelper3
<
    ::com::sun::star::text::XAutoTextContainer,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::container::XIndexAccess
>
{
    SwGlossaries *pGlossaries;

protected:
    virtual ~SwXAutoTextContainer();    // ref-counted objects are not to be deleted from outside -> protected dtor

public:
    SwXAutoTextContainer();

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName(const rtl::OUString& Name)  throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getElementNames(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasByName(const rtl::OUString& Name) throw( ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XAutoTextContainer
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XAutoTextGroup >  SAL_CALL insertNewByName(const rtl::OUString& aGroupName) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeByName(const rtl::OUString& aGroupName) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

};

class SwXAutoTextGroup : public cppu::WeakImplHelper6
<
    ::com::sun::star::text::XAutoTextGroup,
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::container::XIndexAccess,
    ::com::sun::star::container::XNamed,
    ::com::sun::star::lang::XUnoTunnel
>
{
    const SfxItemPropertySet* pPropSet;
    SwGlossaries*           pGlossaries;
    rtl::OUString           sName;
    String                  m_sGroupName;   // prefix m_ to disambiguate from some local vars in the implementation

protected:
    virtual ~SwXAutoTextGroup();    // ref-counted objects are not to be deleted from outside -> protected dtor

public:
        SwXAutoTextGroup(const rtl::OUString& rName, SwGlossaries* pGloss);


    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    //XAutoTextGroup
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getTitles(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL renameByName(const rtl::OUString& aElementName, const rtl::OUString& aNewElementName, const rtl::OUString& aNewElementTitle) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XAutoTextEntry >  SAL_CALL insertNewByName(const rtl::OUString& aName, const rtl::OUString& aTitle, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange) throw( ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeByName(const rtl::OUString& aEntryName) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException );

    //XNamed
    virtual rtl::OUString SAL_CALL getName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setName(const rtl::OUString& Name_) throw( ::com::sun::star::uno::RuntimeException );

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName(const rtl::OUString& Name)  throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getElementNames(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasByName(const rtl::OUString& Name) throw( ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //
    void    Invalidate();
};

class SwXAutoTextEntry
        :public SfxListener
        ,public cppu::WeakImplHelper5
        <
            ::com::sun::star::text::XAutoTextEntry,
            ::com::sun::star::lang::XServiceInfo,
            ::com::sun::star::lang::XUnoTunnel,
            ::com::sun::star::text::XText,
            ::com::sun::star::document::XEventsSupplier
        >
{
    SwGlossaries*   pGlossaries;
    String          sGroupName;
    String          sEntryName;
    SwDocShellRef   xDocSh;
    SwXBodyText*    pBodyText;
    com::sun::star::uno::Reference < com::sun::star::lang::XServiceInfo> xBodyText;

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
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

protected:
    virtual ~SwXAutoTextEntry();    // ref-counted objects are not to be deleted from outside -> protected dtor

public:
    SwXAutoTextEntry(SwGlossaries* , const String& rGroupName, const String& rEntryName);

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    //XText
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  SAL_CALL createTextCursor(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  SAL_CALL createTextCursorByRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & aTextPosition) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL insertString(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xRange, const rtl::OUString& aString, sal_Bool bAbsorb) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL insertControlCharacter(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xRange, sal_Int16 nControlCharacter, sal_Bool bAbsorb) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL insertTextContent(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xRange, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > & xContent, sal_Bool bAbsorb) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeTextContent(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > & xContent) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

    //XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >  SAL_CALL getText(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL  getStart(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL   getEnd(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual rtl::OUString SAL_CALL  getString(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL  setString(const rtl::OUString& aString) throw( ::com::sun::star::uno::RuntimeException );

    //XAutoTextEntry
    virtual void    SAL_CALL applyTo(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xRange)throw( ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    // XEventsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents(  ) throw( ::com::sun::star::uno::RuntimeException );

    void    Invalidate() {pGlossaries = 0;}
    const SwGlossaries* GetGlossaries() { return pGlossaries; }
    const String&   GetGroupName() {return sGroupName;}
    const String&   GetEntryName() {return sEntryName;}
};


/** Implement the XNameAccess for the AutoText events */
class SwAutoTextEventDescriptor : public SvBaseEventDescriptor
{
    ::rtl::OUString sSwAutoTextEventDescriptor;

    SwXAutoTextEntry& rAutoTextEntry;

    using SvBaseEventDescriptor::replaceByName;
    using SvBaseEventDescriptor::getByName;

public:
    SwAutoTextEventDescriptor(  SwXAutoTextEntry& rAutoText );

    ~SwAutoTextEventDescriptor();

    virtual rtl::OUString SAL_CALL getImplementationName(void)
        throw( ::com::sun::star::uno::RuntimeException );

protected:

    virtual void replaceByName(
        const sal_uInt16 nEvent,        /// item ID of event
        const SvxMacro& rMacro)     /// event (will be copied)
            throw(
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual void getByName(
        SvxMacro& rMacro,           /// macro to be filled
        const sal_uInt16 nEvent )       /// item ID of event
            throw(
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
