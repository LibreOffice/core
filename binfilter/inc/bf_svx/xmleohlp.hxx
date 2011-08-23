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

#ifndef _XMLEOHLP_HXX
#define _XMLEOHLP_HXX

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _SVSTOR_HXX
#include <bf_so3/svstor.hxx>
#endif
#include <map>

#ifndef _COM_SUN_STAR_DOCUMENT_XEMBEDDEDOBJECTRESOLVER_HPP_
#include <com/sun/star/document/XEmbeddedObjectResolver.hpp>
#endif
class SvGlobalName;
namespace binfilter {
class SvPersist;

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

struct OUStringLess;
class OutputStorageWrapper_Impl; 

class SvXMLEmbeddedObjectHelper : public ::cppu::WeakComponentImplHelper2<
    ::com::sun::star::document::XEmbeddedObjectResolver,
    ::com::sun::star::container::XNameAccess >
{
    typedef ::std::map< ::rtl::OUString, OutputStorageWrapper_Impl*,
                         OUStringLess > SvXMLEmbeddedObjectHelper_Impl;
private:

    ::osl::Mutex				maMutex;

    const ::rtl::OUString		maDefaultContainerStorageName;
    ::rtl::OUString				maCurContainerStorageName;

    SvStorage*					mpRootStorage;	// package
    SvPersist*					mpDocPersist;
    SvStorageRef				mxContainerStorage;	// container sub package for
                                                // objects

    SvXMLEmbeddedObjectHelperMode		meCreateMode;
    SvXMLEmbeddedObjectHelper_Impl		*mpStreamMap;
    void*						mpDummy2;

    sal_Bool					ImplGetStorageNames(
                                    const ::rtl::OUString& rURLStr, 
                                    ::rtl::OUString& rContainerStorageName, 
                                    ::rtl::OUString& rObjectStorageName,
                                    sal_Bool bInternalToExternal ) const;

    SvStorageRef				ImplGetContainerStorage(
                                    const ::rtl::OUString& rStorageName );
    SvStorageRef				ImplGetObjectStorage(
                                    const ::rtl::OUString& rContainerStorageName, 
                                    const ::rtl::OUString& rObjectStorageName,
                                    sal_Bool bUCBStorage );
    String 						ImplGetUniqueName( SvStorage* pStg,
                                                     const sal_Char* p ) const;
    sal_Bool					ImplReadObject(
                                    const ::rtl::OUString& rContainerStorageName, 
                                    ::rtl::OUString& rObjName,
                                    const SvGlobalName *pClassId,
                                    SvStorage *pTempStor );

    ::rtl::OUString				ImplInsertEmbeddedObjectURL(
                                    const ::rtl::OUString& rURLStr );

protected:						

                                SvXMLEmbeddedObjectHelper();
                                ~SvXMLEmbeddedObjectHelper();
    void						Init( SvStorage *pRootStorage, 
                                      SvPersist& rDocPersist,
                                      SvXMLEmbeddedObjectHelperMode eCreateMode );

    virtual void SAL_CALL		disposing();

public:							
                                SvXMLEmbeddedObjectHelper(
                                    SvPersist& rDocPersist, 
                                    SvXMLEmbeddedObjectHelperMode eCreateMode );

    static SvXMLEmbeddedObjectHelper*	Create(
                                    SvStorage& rRootStorage, 
                                    SvPersist& rDocPersist,
                                    SvXMLEmbeddedObjectHelperMode eCreateMode,
                                    sal_Bool bDirect = sal_True );
    static SvXMLEmbeddedObjectHelper*	Create(
                                    SvPersist& rDocPersist,
                                    SvXMLEmbeddedObjectHelperMode eCreateMode );
    static void					Destroy( SvXMLEmbeddedObjectHelper* pSvXMLEmbeddedObjectHelper );

    void						Flush();

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

}//end of namespace binfilter
#endif
