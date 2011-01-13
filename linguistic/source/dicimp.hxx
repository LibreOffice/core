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

#ifndef _LINGUISTIC_DICIMP_HXX_
#define _LINGUISTIC_DICIMP_HXX_

#include <com/sun/star/linguistic2/XDictionary.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <cppuhelper/interfacecontainer.h>
#include <tools/string.hxx>
#include <tools/stream.hxx>

#include "defs.hxx"
#include "linguistic/misc.hxx"


///////////////////////////////////////////////////////////////////////////

#define DIC_MAX_ENTRIES     30000

sal_Int16           ReadDicVersion( SvStreamPtr &rpStream, sal_uInt16 &nLng, sal_Bool &bNeg );
const String    GetDicExtension();

///////////////////////////////////////////////////////////////////////////

class DictionaryNeo :
    public ::cppu::WeakImplHelper2
    <
        ::com::sun::star::linguistic2::XDictionary,
        ::com::sun::star::frame::XStorable
    >
{

    ::cppu::OInterfaceContainerHelper                           aDicEvtListeners;
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XDictionaryEntry > > aEntries;
    ::rtl::OUString                                             aDicName;
    ::rtl::OUString                                             aMainURL;
    ::com::sun::star::linguistic2::DictionaryType               eDicType;
    sal_Int16                                                       nCount;
    sal_Int16                                                       nLanguage;
    sal_Int16                                                       nDicVersion;
    sal_Bool                                                        bNeedEntries;
    sal_Bool                                                        bIsModified;
    sal_Bool                                                        bIsActive;
    sal_Bool                                                        bIsReadonly;

    // disallow copy-constructor and assignment-operator for now
    DictionaryNeo(const DictionaryNeo &);
    DictionaryNeo & operator = (const DictionaryNeo &);

    void                    launchEvent(sal_Int16 nEvent,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XDictionaryEntry > xEntry);

    sal_uLong                   loadEntries(const ::rtl::OUString &rMainURL);
    sal_uLong                   saveEntries(const ::rtl::OUString &rMainURL);
    int                     cmpDicEntry(const ::rtl::OUString &rWord1,
                                        const ::rtl::OUString &rWord2,
                                        sal_Bool bSimilarOnly = sal_False);
    sal_Bool                    seekEntry(const ::rtl::OUString &rWord, sal_Int32 *pPos,
                                        sal_Bool bSimilarOnly = sal_False);
    sal_Bool                    isSorted();

    sal_Bool                    addEntry_Impl(const ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionaryEntry > xDicEntry,
                                          sal_Bool bIsLoadEntries = sal_False);

public:
    DictionaryNeo();
    DictionaryNeo(const ::rtl::OUString &rName, sal_Int16 nLang,
                    ::com::sun::star::linguistic2::DictionaryType eType,
                    const ::rtl::OUString &rMainURL,
                    sal_Bool bWriteable );
    virtual ~DictionaryNeo();

    // XNamed
    virtual ::rtl::OUString SAL_CALL
        getName()
            throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        setName( const ::rtl::OUString& aName )
            throw(::com::sun::star::uno::RuntimeException);

    // XDictionary
    virtual ::com::sun::star::linguistic2::DictionaryType SAL_CALL
        getDictionaryType()
            throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        setActive( sal_Bool bActivate )
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        isActive()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL
        getCount()
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL
        getLocale()
            throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        setLocale( const ::com::sun::star::lang::Locale& aLocale )
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XDictionaryEntry > SAL_CALL
        getEntry( const ::rtl::OUString& aWord )
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        addEntry( const ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XDictionaryEntry >& xDicEntry )
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        add( const ::rtl::OUString& aWord, sal_Bool bIsNegative,
                const ::rtl::OUString& aRplcText )
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        remove( const ::rtl::OUString& aWord )
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        isFull()
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XDictionaryEntry > > SAL_CALL
        getEntries()
            throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        clear()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        addDictionaryEventListener( const ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XDictionaryEventListener >& xListener )
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        removeDictionaryEventListener( const ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XDictionaryEventListener >& xListener )
            throw(::com::sun::star::uno::RuntimeException);

    // XStorable
    virtual sal_Bool SAL_CALL
        hasLocation()
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL
        getLocation()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        isReadonly()
            throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        store()
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        storeAsURL( const ::rtl::OUString& aURL,
                const ::com::sun::star::uno::Sequence<
                    ::com::sun::star::beans::PropertyValue >& aArgs )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        storeToURL( const ::rtl::OUString& aURL,
                const ::com::sun::star::uno::Sequence<
                    ::com::sun::star::beans::PropertyValue >& aArgs )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException);
};


///////////////////////////////////////////////////////////////////////////

class DicEntry :
    public cppu::WeakImplHelper1
    <
        ::com::sun::star::linguistic2::XDictionaryEntry
    >
{
    ::rtl::OUString aDicWord,       // including hyphen positions represented by "="
                    aReplacement;   // including hyphen positions represented by "="
    sal_Bool            bIsNegativ;

    // disallow copy-constructor and assignment-operator for now
    DicEntry(const DicEntry &);
    DicEntry & operator = (const DicEntry &);

    void            splitDicFileWord(const ::rtl::OUString &rDicFileWord,
                                     ::rtl::OUString &rDicWord,
                                     ::rtl::OUString &rReplacement);

public:
    DicEntry();
    DicEntry(const ::rtl::OUString &rDicFileWord, sal_Bool bIsNegativ);
    DicEntry(const ::rtl::OUString &rDicWord, sal_Bool bIsNegativ,
             const ::rtl::OUString &rRplcText);
    virtual ~DicEntry();

    // XDictionaryEntry
    virtual ::rtl::OUString SAL_CALL
        getDictionaryWord() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        isNegative() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL
        getReplacementText() throw(::com::sun::star::uno::RuntimeException);
};


///////////////////////////////////////////////////////////////////////////

#endif

