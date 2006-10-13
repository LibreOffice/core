/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ohierarchyholder.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-13 11:49:25 $
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

#ifndef _OHIERARCHYHOLDER_HXX_
#define _OHIERARCHYHOLDER_HXX_

#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_XTRANSACTIONLISTENER_HPP_
#include <com/sun/star/embed/XTransactionListener.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_XEXTENDEDSTORAGESTREAM_HPP_
#include <com/sun/star/embed/XExtendedStorageStream.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#include <rtl/ref.hxx>

#include <hash_map>
#include <list>
#include <vector>

struct OHierarchyElement_Impl;

struct eqFunc
{
    sal_Bool operator()( const rtl::OUString &r1,
                         const rtl::OUString &r2) const
    {
        return r1 == r2;
    }
};
typedef ::std::hash_map< ::rtl::OUString,
                         ::rtl::Reference< OHierarchyElement_Impl >,
                         ::rtl::OUStringHash,
                         eqFunc > OHierarchyElementList_Impl;

typedef ::std::vector< ::rtl::OUString > OStringList_Impl;

struct OHierarchyElement_Impl : public cppu::WeakImplHelper1< ::com::sun::star::embed::XTransactionListener >
{
    ::osl::Mutex m_aMutex;

    ::rtl::Reference< OHierarchyElement_Impl > m_rParent;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > m_xOwnStorage;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::embed::XStorage > m_xWeakOwnStorage;

    OHierarchyElementList_Impl m_aChildren;

    ::std::list< ::com::sun::star::uno::Reference< ::com::sun::star::embed::XExtendedStorageStream > > m_aOpenStreams;

public:
    OHierarchyElement_Impl( OHierarchyElement_Impl* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage )
    : m_rParent( pParent )
    , m_xOwnStorage( xStorage )
    {}

    OHierarchyElement_Impl( const ::com::sun::star::uno::WeakReference< ::com::sun::star::embed::XStorage >& xWeakStorage )
    : m_rParent( NULL )
    , m_xWeakOwnStorage( xWeakStorage )
    {}

    void Commit();

    void SetParent( const ::rtl::Reference< OHierarchyElement_Impl >& rParent ) { m_rParent = rParent; }

    void TestForClosing();

    void RemoveElement( const ::rtl::Reference< OHierarchyElement_Impl >& aRef );

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XExtendedStorageStream >
                                        GetStreamHierarchically( sal_Int32 nStorageMode,
                                                                OStringList_Impl& aPath,
                                                                sal_Int32 nStreamMode,
                                                                const ::rtl::OUString& aPassword = ::rtl::OUString() );

    void RemoveStreamHierarchically( OStringList_Impl& aListPath );

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);


    // XTransactionListener
    virtual void SAL_CALL preCommit( const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL commited( const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL preRevert( const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL reverted( const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

};

class OHierarchyHolder_Impl : public ::cppu::OWeakObject
{
    ::com::sun::star::uno::WeakReference< ::com::sun::star::embed::XStorage > m_xWeakOwnStorage;
    OHierarchyElement_Impl m_aChild;
public:
    OHierarchyHolder_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xOwnStorage )
    : m_xWeakOwnStorage( xOwnStorage )
    , m_aChild( ::com::sun::star::uno::WeakReference< ::com::sun::star::embed::XStorage >( xOwnStorage ) )
    {}

    static OStringList_Impl GetListPathFromString( const ::rtl::OUString& aPath );

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XExtendedStorageStream >
                                        GetStreamHierarchically( sal_Int32 nStorageMode,
                                                                OStringList_Impl& aListPath,
                                                                sal_Int32 nStreamMode,
                                                                const ::rtl::OUString& aPassword = ::rtl::OUString() );

    void RemoveStreamHierarchically( OStringList_Impl& aListPath );
};

#endif // _OHIERARCHYHOLDER

