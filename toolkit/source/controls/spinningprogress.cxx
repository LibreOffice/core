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

#include <com/sun/star/uno/XComponentContext.hpp>
#include <rtl/ustrbuf.hxx>
#include <toolkit/controls/animatedimages.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/throbber.hxx>

using namespace css;
using namespace css::uno;

namespace {

typedef toolkit::AnimatedImagesControlModel SpinningProgressControlModel_Base;
class SpinningProgressControlModel : public SpinningProgressControlModel_Base
{
public:
    explicit SpinningProgressControlModel( css::uno::Reference< css::uno::XComponentContext > const & i_factory );
    SpinningProgressControlModel( const SpinningProgressControlModel& i_copySource );

    virtual UnoControlModel* Clone() const override;

    // XPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

protected:
    virtual ~SpinningProgressControlModel();
};

    SpinningProgressControlModel::SpinningProgressControlModel( Reference< XComponentContext > const & i_factory )
        :SpinningProgressControlModel_Base( i_factory )
    {
        // default image sets
        osl_atomic_increment( &m_refCount );
        {
            try
            {
                Throbber::ImageSet aImageSets[] =
                {
                    Throbber::IMAGES_16_PX, Throbber::IMAGES_32_PX, Throbber::IMAGES_64_PX
                };
                for ( size_t i=0; i < sizeof( aImageSets ) / sizeof( aImageSets[0] ); ++i )
                {
                    const ::std::vector< OUString > aDefaultURLs( Throbber::getDefaultImageURLs( aImageSets[i] ) );
                    const Sequence< OUString > aImageURLs( &aDefaultURLs[0], aDefaultURLs.size() );
                    insertImageSet( i, aImageURLs );
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        osl_atomic_decrement( &m_refCount );
    }


    SpinningProgressControlModel::SpinningProgressControlModel( const SpinningProgressControlModel& i_copySource )
        :SpinningProgressControlModel_Base( i_copySource )
    {
    }


    SpinningProgressControlModel::~SpinningProgressControlModel()
    {
    }


    UnoControlModel* SpinningProgressControlModel::Clone() const
    {
        return new SpinningProgressControlModel( *this );
    }


    Reference< beans::XPropertySetInfo > SAL_CALL SpinningProgressControlModel::getPropertySetInfo(  ) throw(RuntimeException, std::exception)
    {
        static Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }


    OUString SAL_CALL SpinningProgressControlModel::getServiceName() throw(RuntimeException, std::exception)
    {
        return OUString("com.sun.star.awt.SpinningProgressControlModel");
    }


    OUString SAL_CALL SpinningProgressControlModel::getImplementationName(  ) throw(RuntimeException, std::exception)
    {
        return OUString("org.openoffice.comp.toolkit.SpinningProgressControlModel");
    }


    Sequence< OUString > SAL_CALL SpinningProgressControlModel::getSupportedServiceNames() throw(RuntimeException, std::exception)
    {
        Sequence< OUString > aServiceNames(3);
        aServiceNames[0] = "com.sun.star.awt.SpinningProgressControlModel";
        aServiceNames[1] = "com.sun.star.awt.AnimatedImagesControlModel";
        aServiceNames[2] = "com.sun.star.awt.UnoControlModel";
        return aServiceNames;
    }

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
org_openoffice_comp_toolkit_SpinningProgressControlModel_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SpinningProgressControlModel(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
