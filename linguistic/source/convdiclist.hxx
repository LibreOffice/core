/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: convdiclist.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:49:07 $
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

#ifndef _LINGUISTIC_CONVDICLIST_HXX_
#define _LINGUISTIC_CONVDICLIST_HXX_

#ifndef _COM_SUN_STAR_LINGUISTIC2_XCONVERSIONDICTIONARYLIST_HPP_
#include <com/sun/star/linguistic2/XConversionDictionaryList.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif

#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif

#include "misc.hxx"
#include "lngopt.hxx"


class ConvDicNameContainer;

///////////////////////////////////////////////////////////////////////////

class ConvDicList :
    public cppu::WeakImplHelper3
    <
        ::com::sun::star::linguistic2::XConversionDictionaryList,
        ::com::sun::star::lang::XComponent,
        ::com::sun::star::lang::XServiceInfo
    >
{

    class MyAppExitListener : public linguistic::AppExitListener
    {
        ConvDicList & rMyDicList;

    public:
        MyAppExitListener( ConvDicList &rDicList ) : rMyDicList( rDicList ) {}
        virtual void    AtExit();
    };


    ::cppu::OInterfaceContainerHelper       aEvtListeners;

    ConvDicNameContainer                   *pNameContainer;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XNameContainer >   xNameContainer;

    MyAppExitListener                      *pExitListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::
                XTerminateListener >        xExitListener;

    BOOL                                    bDisposing;

    // disallow copy-constructor and assignment-operator for now
    ConvDicList( const ConvDicList & );
    ConvDicList & operator = (const ConvDicList &);

    ConvDicNameContainer &  GetNameContainer();

public:
    ConvDicList();
    virtual ~ConvDicList();

    // XConversionDictionaryList
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > SAL_CALL getDictionaryContainer(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XConversionDictionary > SAL_CALL addNewDictionary( const ::rtl::OUString& aName, const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nConversionDictionaryType ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL queryConversions( const ::rtl::OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength, const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nConversionDictionaryType, ::com::sun::star::linguistic2::ConversionDirection eDirection, sal_Int32 nTextConversionOptions ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL queryMaxCharCount( const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nConversionDictionaryType, ::com::sun::star::linguistic2::ConversionDirection eDirection ) throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);


    static inline ::rtl::OUString
        getImplementationName_Static() throw();
    static com::sun::star::uno::Sequence< ::rtl::OUString >
        getSupportedServiceNames_Static() throw();

    // non UNO-specific
    void                    FlushDics();
};

inline ::rtl::OUString ConvDicList::getImplementationName_Static() throw()
{
    return A2OU( "com.sun.star.lingu2.ConvDicList" );
}

///////////////////////////////////////////////////////////////////////////

#endif

