/*************************************************************************
 *
 *  $RCSfile: dicimp.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: tl $ $Date: 2001-03-19 14:51:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _LINGUISTIC_DICIMP_HXX_
#define _LINGUISTIC_DICIMP_HXX_

#include <com/sun/star/linguistic2/XDictionary1.hpp>
#include <com/sun/star/linguistic2/XDictionary.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase3.hxx> // helper for implementations
#include <cppuhelper/implbase1.hxx> // helper for implementations

#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#include "misc.hxx"


///////////////////////////////////////////////////////////////////////////

#define DIC_MAX_ENTRIES     2000

int             GetDicVersion( const sal_Char *pVerStr );
const String    GetDicExtension();

///////////////////////////////////////////////////////////////////////////

class DictionaryNeo :
    public ::cppu::WeakImplHelper3
    <
        ::com::sun::star::linguistic2::XDictionary1,
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
    INT16                                                       nCount;
    INT16                                                       nLanguage;
    INT16                                                       nDicVersion;
    BOOL                                                        bNeedEntries;
    BOOL                                                        bIsModified;
    BOOL                                                        bIsActive;
    BOOL                                                        bIsReadonly;

    // disallow copy-constructor and assignment-operator for now
    DictionaryNeo(const DictionaryNeo &);
    DictionaryNeo & operator = (const DictionaryNeo &);

    void                    launchEvent(INT16 nEvent,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XDictionaryEntry > xEntry);

    ULONG                   loadEntries(const ::rtl::OUString &rMainURL);
    ULONG                   saveEntries(const ::rtl::OUString &rMainURL);
    int                     cmpDicEntry(const ::rtl::OUString &rWord1,
                                        const ::rtl::OUString &rWord2,
                                        BOOL bSimilarOnly = FALSE);
    BOOL                    seekEntry(const ::rtl::OUString &rWord, INT32 *pPos,
                                        BOOL bSimilarOnly = FALSE);
    BOOL                    isSorted();

    BOOL                    addEntry_Impl(const ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionaryEntry > xDicEntry,
                                          BOOL bIsLoadEntries = FALSE);
    BOOL                    isReadonly_Impl();

public:
    DictionaryNeo();
    DictionaryNeo(const ::rtl::OUString &rName, INT16 nLang,
                    ::com::sun::star::linguistic2::DictionaryType eType,
                      const ::rtl::OUString &rMainURL);
    virtual ~DictionaryNeo();

    // XNamed
    virtual ::rtl::OUString SAL_CALL
        getName()
            throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        setName( const ::rtl::OUString& aName )
            throw(::com::sun::star::uno::RuntimeException);

    // XDictionary1 (same as XDictionary but for sal_Int16 as language)
    // only the different ones are listed
    virtual sal_Int16 SAL_CALL
        getLanguage()
            throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        setLanguage( sal_Int16 nLang )
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
    virtual sal_Int16 SAL_CALL
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
    BOOL            bIsNegativ;

    // disallow copy-constructor and assignment-operator for now
    DicEntry(const DicEntry &);
    DicEntry & operator = (const DicEntry &);

    void            splitDicFileWord(const ::rtl::OUString &rDicFileWord,
                                     ::rtl::OUString &rDicWord,
                                     ::rtl::OUString &rReplacement);

public:
    DicEntry();
    DicEntry(const ::rtl::OUString &rDicFileWord, BOOL bIsNegativ);
    DicEntry(const ::rtl::OUString &rDicWord, BOOL bIsNegativ,
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

