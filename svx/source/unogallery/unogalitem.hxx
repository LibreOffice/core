/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_SOURCE_UNOGALLERY_UNOGALITEM_HXX
#define INCLUDED_SVX_SOURCE_UNOGALLERY_UNOGALITEM_HXX

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


// - GalleryItem -


class GalleryItem : public ::cppu::OWeakAggObject,
                    public css::lang::XServiceInfo,
                    public css::lang::XTypeProvider,
                    public css::gallery::XGalleryItem,
                    public ::comphelper::PropertySetHelper
{
    friend class ::unogallery::GalleryTheme;

public:

            GalleryItem( ::unogallery::GalleryTheme& rTheme, const GalleryObject& rObject );
            virtual ~GalleryItem() throw();

    bool    isValid() const;

    static OUString getImplementationName_Static() throw();
    static css::uno::Sequence< OUString >  getSupportedServiceNames_Static() throw();

protected:

    // XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XGalleryItem
    virtual ::sal_Int8 SAL_CALL getType(  ) throw (css::uno::RuntimeException, std::exception) override;

    // PropertySetHelper
    virtual void _setPropertyValues( const comphelper::PropertyMapEntry** ppEntries, const css::uno::Any* pValues ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;
    virtual void _getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, css::uno::Any* pValue )
        throw (css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException,
               std::exception) override;

protected:

    static ::comphelper::PropertySetInfo* createPropertySetInfo();

private:

    ::unogallery::GalleryTheme* mpTheme;
    const ::GalleryObject*      mpGalleryObject;

    const ::GalleryObject*      implGetObject() const { return mpGalleryObject;}
    void                        implSetInvalid();

                                GalleryItem( const GalleryItem& ) = delete;
                                GalleryItem& operator=( const GalleryItem& ) = delete;
};


// - GalleryDrawingModel -


class GalleryDrawingModel : public SvxUnoDrawingModel
{
public:

    explicit    GalleryDrawingModel( SdrModel* pDoc ) throw();
    virtual     ~GalleryDrawingModel() throw();

                UNO3_GETIMPLEMENTATION_DECL( GalleryDrawingModel )
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
