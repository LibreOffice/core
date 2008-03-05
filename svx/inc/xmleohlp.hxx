/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmleohlp.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:59:28 $
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

#ifndef _XMLEOHLP_HXX
#define _XMLEOHLP_HXX

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#include <sot/storage.hxx>
#include <map>

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEMBEDDEDOBJECTRESOLVER_HPP_
#include <com/sun/star/document/XEmbeddedObjectResolver.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// -----------------------------
// - SvXMLEmbeddedObjectHelper -
// -----------------------------

enum SvXMLEmbeddedObjectHelperMode
{
    EMBEDDEDOBJECTHELPER_MODE_READ = 0,
    EMBEDDEDOBJECTHELPER_MODE_WRITE = 1
};

// -----------------------------
// - SvXMLEmbeddedObjectHelper -
// -----------------------------

namespace comphelper {
class IEmbeddedHelper;
}
class SvGlobalName;
struct OUStringLess;
class OutputStorageWrapper_Impl;

class SVX_DLLPUBLIC SvXMLEmbeddedObjectHelper : public ::cppu::WeakComponentImplHelper2<
    ::com::sun::star::document::XEmbeddedObjectResolver,
    ::com::sun::star::container::XNameAccess >
{
    typedef ::std::map< ::rtl::OUString, OutputStorageWrapper_Impl*,
                         OUStringLess > SvXMLEmbeddedObjectHelper_Impl;
private:

    ::osl::Mutex                maMutex;

    const ::rtl::OUString       maReplacementGraphicsContainerStorageName;
    const ::rtl::OUString       maReplacementGraphicsContainerStorageName60;
    ::rtl::OUString             maCurContainerStorageName;


    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > mxRootStorage;  // package
    ::comphelper::IEmbeddedHelper*             mpDocPersist;
    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > mxContainerStorage; // container sub package for
    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > mxTempStorage;  // package
                                                // objects
    SvXMLEmbeddedObjectHelperMode       meCreateMode;
    SvXMLEmbeddedObjectHelper_Impl      *mpStreamMap;

    SVX_DLLPRIVATE sal_Bool                 ImplGetStorageNames(
                                    const ::rtl::OUString& rURLStr,
                                    ::rtl::OUString& rContainerStorageName,
                                    ::rtl::OUString& rObjectStorageName,
                                    sal_Bool bInternalToExternal,
                                       sal_Bool *pGraphicRepl=0,
                                    sal_Bool *pOasisFormat=0 ) const;

    SVX_DLLPRIVATE com::sun::star::uno::Reference < com::sun::star::embed::XStorage > ImplGetContainerStorage(
                                    const ::rtl::OUString& rStorageName );

    SVX_DLLPRIVATE String                      ImplGetUniqueName( ::comphelper::IEmbeddedHelper*, const sal_Char* p ) const;
    SVX_DLLPRIVATE sal_Bool                 ImplReadObject(
                                    const ::rtl::OUString& rContainerStorageName,
                                    ::rtl::OUString& rObjName,
                                    const SvGlobalName *pClassId,
                                    SvStream* pTemp );

    SVX_DLLPRIVATE ::rtl::OUString              ImplInsertEmbeddedObjectURL(
                                    const ::rtl::OUString& rURLStr );

    SVX_DLLPRIVATE ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > ImplGetReplacementImage(
                                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject >& xObj );

protected:

                                SvXMLEmbeddedObjectHelper();
                                ~SvXMLEmbeddedObjectHelper();
    void                        Init( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&,
                                      ::comphelper::IEmbeddedHelper& rDocPersist,
                                      SvXMLEmbeddedObjectHelperMode eCreateMode );

    virtual void SAL_CALL       disposing();

public:
                                SvXMLEmbeddedObjectHelper(
                                    ::comphelper::IEmbeddedHelper& rDocPersist,
                                    SvXMLEmbeddedObjectHelperMode eCreateMode );

    static SvXMLEmbeddedObjectHelper*   Create(
                                    const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&,
                                    ::comphelper::IEmbeddedHelper& rDocPersist,
                                    SvXMLEmbeddedObjectHelperMode eCreateMode,
                                    sal_Bool bDirect = sal_True );
    static SvXMLEmbeddedObjectHelper*   Create(
                                    ::comphelper::IEmbeddedHelper& rDocPersist,
                                    SvXMLEmbeddedObjectHelperMode eCreateMode );
    static void                 Destroy( SvXMLEmbeddedObjectHelper* pSvXMLEmbeddedObjectHelper );

    void                        Flush();

    // XEmbeddedObjectResolver
    virtual ::rtl::OUString SAL_CALL resolveEmbeddedObjectURL( const ::rtl::OUString& aURL ) throw(::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException);
};

#endif
