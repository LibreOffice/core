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

#ifndef _TOOLKIT_CONTROLS_STDTABCONTROLLERMODEL_HXX_
#define _TOOLKIT_CONTROLS_STDTABCONTROLLERMODEL_HXX_


#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/weakagg.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <osl/mutex.hxx>

#include <tools/list.hxx>
#include <tools/gen.hxx>
#include <vector>

struct UnoControlModelEntry;

typedef ::std::vector< UnoControlModelEntry* > UnoControlModelEntryListBase;

class UnoControlModelEntryList
{
private:
    UnoControlModelEntryListBase maList;
    ::rtl::OUString maGroupName;

public:
                    UnoControlModelEntryList();
                    ~UnoControlModelEntryList();

    const ::rtl::OUString&      GetName() const                         { return maGroupName; }
    void                        SetName( const ::rtl::OUString& rName ) { maGroupName = rName; }

    void    Reset();
    void    DestroyEntry( size_t nEntry );
    size_t  size() const;
    UnoControlModelEntry* operator[]( size_t i ) const;
    void push_back( UnoControlModelEntry* item );
    void insert( size_t i, UnoControlModelEntry* item );
};

struct UnoControlModelEntry
{
    sal_Bool        bGroup;
    union
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >* pxControl;
        UnoControlModelEntryList*   pGroup;
    };
};

// Keine Referenz halten, nur temporaer fuer AutoTabOrder
struct ComponentEntry
{
    ::com::sun::star::awt::XWindow*     pComponent;
    Point                               aPos;
};

DECLARE_LIST( ComponentEntryList, ComponentEntry* )

#define CONTROLPOS_NOTFOUND 0xFFFFFFFF

class StdTabControllerModel :   public ::com::sun::star::awt::XTabControllerModel,
                                public ::com::sun::star::lang::XServiceInfo,
                                public ::com::sun::star::io::XPersistObject,
                                public ::com::sun::star::lang::XTypeProvider,
                                public ::cppu::OWeakAggObject
{
private:
    ::osl::Mutex                maMutex;
    UnoControlModelEntryList    maControls;
    sal_Bool                    mbGroupControl;

protected:
    ::osl::Mutex&           GetMutex() { return maMutex; }
    sal_uInt32              ImplGetControlCount( const UnoControlModelEntryList& rList ) const;
    void                    ImplGetControlModels( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > ** pRefs, const UnoControlModelEntryList& rList ) const;
    void                    ImplSetControlModels( UnoControlModelEntryList& rList, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Controls ) const;
    sal_uInt32              ImplGetControlPos( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >  xCtrl, const UnoControlModelEntryList& rList ) const;

public:
                            StdTabControllerModel();
                            ~StdTabControllerModel();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return OWeakAggObject::queryInterface(rType); }
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }

    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTabControllerModel
    sal_Bool SAL_CALL getGroupControl(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setGroupControl( sal_Bool GroupControl ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setControlModels( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Controls ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > SAL_CALL getControlModels(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setGroup( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Group, const ::rtl::OUString& GroupName ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getGroupCount(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL getGroup( sal_Int32 nGroup, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Group, ::rtl::OUString& Name ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL getGroupByName( const ::rtl::OUString& Name, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Group ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL write( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream >& OutStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL read( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream >& InStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    DECLIMPL_SERVICEINFO( StdTabControllerModel, szServiceName2_TabControllerModel )
};



#endif // _TOOLKIT_HELPER_STDTABCONTROLLERMODEL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
