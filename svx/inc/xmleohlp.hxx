/*************************************************************************
 *
 *  $RCSfile: xmleohlp.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $
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

class SfxObjectShell;
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
    SfxObjectShell*             mpDocPersist;
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
                                       sal_Bool *pGraphicRepl=0 ) const;

    SVX_DLLPRIVATE com::sun::star::uno::Reference < com::sun::star::embed::XStorage > ImplGetContainerStorage(
                                    const ::rtl::OUString& rStorageName );

    SVX_DLLPRIVATE String                      ImplGetUniqueName( SfxObjectShell*, const sal_Char* p ) const;
    SVX_DLLPRIVATE sal_Bool                 ImplReadObject(
                                    const ::rtl::OUString& rContainerStorageName,
                                    ::rtl::OUString& rObjName,
                                    const SvGlobalName *pClassId,
                                    SvStream* pTemp );

    SVX_DLLPRIVATE ::rtl::OUString              ImplInsertEmbeddedObjectURL(
                                    const ::rtl::OUString& rURLStr );

protected:

                                SvXMLEmbeddedObjectHelper();
                                ~SvXMLEmbeddedObjectHelper();
    void                        Init( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&,
                                      SfxObjectShell& rDocPersist,
                                      SvXMLEmbeddedObjectHelperMode eCreateMode );

    virtual void SAL_CALL       disposing();

public:
                                SvXMLEmbeddedObjectHelper(
                                    SfxObjectShell& rDocPersist,
                                    SvXMLEmbeddedObjectHelperMode eCreateMode );

    static SvXMLEmbeddedObjectHelper*   Create(
                                    const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&,
                                    SfxObjectShell& rDocPersist,
                                    SvXMLEmbeddedObjectHelperMode eCreateMode,
                                    sal_Bool bDirect = sal_True );
    static SvXMLEmbeddedObjectHelper*   Create(
                                    SfxObjectShell& rDocPersist,
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
