/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlistimp.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 16:26:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _LINGUISTIC_DLISTIMP_HXX_
#define _LINGUISTIC_DLISTIMP_HXX_

#ifndef _COM_SUN_STAR_LINGUISTIC2_XSEARCHABLEDICTIONARYLIST_HPP_
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <cppuhelper/implbase3.hxx> // helper for implementations
#include <cppuhelper/interfacecontainer.h>

#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif

#include "misc.hxx"
#include "lngopt.hxx"

class DicEvtListenerHelper;

///////////////////////////////////////////////////////////////////////////

class ActDic
{
public:
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >    xDic;

    ActDic() {}
    ActDic(const ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary > &rDic) : xDic(rDic) {}
};
SV_DECL_OBJARR( ActDicArray, ActDic, 16, 16 )

class DicList :
    public cppu::WeakImplHelper3
    <
        ::com::sun::star::linguistic2::XSearchableDictionaryList,
        ::com::sun::star::lang::XComponent,
        ::com::sun::star::lang::XServiceInfo
    >
{
    class MyAppExitListener : public linguistic::AppExitListener
    {
        DicList & rMyDicList;

    public:
        MyAppExitListener( DicList &rDicList ) : rMyDicList( rDicList ) {}
        virtual void    AtExit();
    };

    LinguOptions    aOpt;

    ::cppu::OInterfaceContainerHelper       aEvtListeners;
    ActDicArray*                            pDicList;

    ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::
                XDictionaryEventListener >  xDicEvtLstnrHelper;
    DicEvtListenerHelper                    *pDicEvtLstnrHelper;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::
                XTerminateListener >        xExitListener;
    MyAppExitListener                       *pExitListener;

    BOOL    bDisposing;

    // disallow copy-constructor and assignment-operator for now
    DicList( const DicList & );
    DicList & operator = (const DicList &);

    void            _CreateDicList();
    ActDicArray&    GetDicList()
                        {
                            if( !pDicList )
                                _CreateDicList();
                            return *pDicList;
                        }

    void            launchEvent(INT16 nEvent, com::sun::star::uno::Sequence<
                            ::com::sun::star::linguistic2::XDictionary > xDic);
    void            searchForDictionaries( ActDicArray &rDicList,
                                            const String &rDicDir, BOOL bIsWritePath );
    INT32           getDicPos(const com::sun::star::uno::Reference<
                            ::com::sun::star::linguistic2::XDictionary > &xDic);

public:
    DicList();
    virtual ~DicList();

    // XDictionaryList
    virtual sal_Int16 SAL_CALL
        getCount()
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XDictionary > > SAL_CALL
        getDictionaries()
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XDictionary > SAL_CALL
        getDictionaryByName( const ::rtl::OUString& aDictionaryName )
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        addDictionary( const ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XDictionary >& xDictionary )
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        removeDictionary( const ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XDictionary >& xDictionary )
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        addDictionaryListEventListener( const ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XDictionaryListEventListener >& xListener,
                sal_Bool bReceiveVerbose )
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        removeDictionaryListEventListener(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XDictionaryListEventListener >& xListener )
                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL
        beginCollectEvents()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL
        endCollectEvents()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL
        flushEvents()
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XDictionary > SAL_CALL
        createDictionary( const ::rtl::OUString& aName,
                const ::com::sun::star::lang::Locale& aLocale,
                ::com::sun::star::linguistic2::DictionaryType eDicType,
                const ::rtl::OUString& aURL )
            throw(::com::sun::star::uno::RuntimeException);

    // XSearchableDictionaryList
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XDictionaryEntry > SAL_CALL
        queryDictionaryEntry( const ::rtl::OUString& aWord,
                const ::com::sun::star::lang::Locale& aLocale,
                sal_Bool bSearchPosDics, sal_Bool bSpellEntry )
            throw(::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL
        dispose()
            throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        addEventListener( const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XEventListener >& xListener )
            throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        removeEventListener( const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XEventListener >& aListener )
            throw(::com::sun::star::uno::RuntimeException);


    ////////////////////////////////////////////////////////////
    // Service specific part
    //

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL
        getImplementationName()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        supportsService( const ::rtl::OUString& ServiceName )
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames()
            throw(::com::sun::star::uno::RuntimeException);


    static inline ::rtl::OUString
        getImplementationName_Static() throw();
    static com::sun::star::uno::Sequence< ::rtl::OUString >
        getSupportedServiceNames_Static() throw();

    // non UNO-specific
    void    SaveDics();
};

inline ::rtl::OUString DicList::getImplementationName_Static() throw()
{
    return A2OU( "com.sun.star.lingu2.DicList" );
}

///////////////////////////////////////////////////////////////////////////

#endif

