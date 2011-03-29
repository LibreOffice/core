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

#ifndef _SVX_UNOGALITEM_HXX
#define _SVX_UNOGALITEM_HXX

#include <svx/unomodel.hxx>
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/gallery/XGalleryItem.hpp>
#include <comphelper/propertysethelper.hxx>
#include <comphelper/propertysetinfo.hxx>

class GalleryTheme;
struct GalleryObject;
namespace unogallery { class GalleryTheme; }

namespace unogallery {

// ---------------
// - GalleryItem -
// ---------------

class GalleryItem : public ::cppu::OWeakAggObject,
                    public ::com::sun::star::lang::XServiceInfo,
                    public ::com::sun::star::lang::XTypeProvider,
                    public ::com::sun::star::gallery::XGalleryItem,
                    public ::comphelper::PropertySetHelper
{
    friend class ::unogallery::GalleryTheme;

public:

            GalleryItem( ::unogallery::GalleryTheme& rTheme, const GalleryObject& rObject );
            ~GalleryItem() throw();

    bool    isValid() const;

    static ::rtl::OUString getImplementationName_Static() throw();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString >  getSupportedServiceNames_Static() throw();

protected:

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    // XGalleryItem
    virtual ::sal_Int8 SAL_CALL getType(  ) throw (::com::sun::star::uno::RuntimeException);

    // PropertySetHelper
    virtual void _setPropertyValues( const comphelper::PropertyMapEntry** ppEntries, const ::com::sun::star::uno::Any* pValues ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, ::com::sun::star::uno::Any* pValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException );

protected:

    ::comphelper::PropertySetInfo* createPropertySetInfo();

private:

    ::unogallery::GalleryTheme* mpTheme;
    const ::GalleryObject*      mpGalleryObject;

    const ::GalleryObject*      implGetObject() const;
    void                        implSetInvalid();

                                // not available
                                GalleryItem();
                                GalleryItem( const GalleryItem& );
                                GalleryItem& operator=( const GalleryItem& );
};

// -----------------------
// - GalleryDrawingModel -
// -----------------------

class GalleryDrawingModel : public SvxUnoDrawingModel
{
public:

                GalleryDrawingModel( SdrModel* pDoc ) throw();
    virtual     ~GalleryDrawingModel() throw();

                UNO3_GETIMPLEMENTATION_DECL( GalleryDrawingModel )
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
