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
#include <controls/animatedimages.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <vcl/toolkit/throbber.hxx>

using namespace css;
using namespace css::uno;

namespace {

typedef toolkit::AnimatedImagesControlModel SpinningProgressControlModel_Base;
class SpinningProgressControlModel : public SpinningProgressControlModel_Base
{
public:
    explicit SpinningProgressControlModel( css::uno::Reference< css::uno::XComponentContext > const & i_factory );
    SpinningProgressControlModel(const SpinningProgressControlModel& rOther) : SpinningProgressControlModel_Base(rOther) {}

    virtual rtl::Reference<UnoControlModel> Clone() const override;

    // XPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    // XPersistObject
    OUString SAL_CALL getServiceName() override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName(  ) override;
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

protected:
    virtual ~SpinningProgressControlModel() override;
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
                    Throbber::ImageSet::N16px, Throbber::ImageSet::N32px, Throbber::ImageSet::N64px
                };
                for ( size_t i=0; i < SAL_N_ELEMENTS(aImageSets); ++i )
                {
                    const ::std::vector< OUString > aDefaultURLs( Throbber::getDefaultImageURLs( aImageSets[i] ) );
                    const Sequence< OUString > aImageURLs( aDefaultURLs.data(), aDefaultURLs.size() );
                    insertImageSet( i, aImageURLs );
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("toolkit.controls");
            }
        }
        osl_atomic_decrement( &m_refCount );
    }


    SpinningProgressControlModel::~SpinningProgressControlModel()
    {
    }


    rtl::Reference<UnoControlModel> SpinningProgressControlModel::Clone() const
    {
        return new SpinningProgressControlModel( *this );
    }


    Reference< beans::XPropertySetInfo > SAL_CALL SpinningProgressControlModel::getPropertySetInfo(  )
    {
        static Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }


    OUString SAL_CALL SpinningProgressControlModel::getServiceName()
    {
        return u"com.sun.star.awt.SpinningProgressControlModel"_ustr;
    }


    OUString SAL_CALL SpinningProgressControlModel::getImplementationName(  )
    {
        return u"org.openoffice.comp.toolkit.SpinningProgressControlModel"_ustr;
    }


    Sequence< OUString > SAL_CALL SpinningProgressControlModel::getSupportedServiceNames()
    {
        return { u"com.sun.star.awt.SpinningProgressControlModel"_ustr,
                 u"com.sun.star.awt.AnimatedImagesControlModel"_ustr,
                 u"com.sun.star.awt.UnoControlModel"_ustr };
    }

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
org_openoffice_comp_toolkit_SpinningProgressControlModel_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SpinningProgressControlModel(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
