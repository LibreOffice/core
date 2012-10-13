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


#include "toolkit/controls/spinningprogress.hxx"
#include "toolkit/helper/servicenames.hxx"
#include "toolkit/helper/unopropertyarrayhelper.hxx"


#include <rtl/ustrbuf.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/throbber.hxx>

//......................................................................................................................
namespace toolkit
{
//......................................................................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::lang::XMultiServiceFactory;
    /** === end UNO using === **/

    //==================================================================================================================
    //= SpinningProgressControlModel
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    SpinningProgressControlModel::SpinningProgressControlModel( Reference< XMultiServiceFactory > const & i_factory )
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
                    const ::std::vector< ::rtl::OUString > aDefaultURLs( Throbber::getDefaultImageURLs( aImageSets[i] ) );
                    const Sequence< ::rtl::OUString > aImageURLs( &aDefaultURLs[0], aDefaultURLs.size() );
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

    //------------------------------------------------------------------------------------------------------------------
    SpinningProgressControlModel::SpinningProgressControlModel( const SpinningProgressControlModel& i_copySource )
        :SpinningProgressControlModel_Base( i_copySource )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    SpinningProgressControlModel::~SpinningProgressControlModel()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    UnoControlModel* SpinningProgressControlModel::Clone() const
    {
        return new SpinningProgressControlModel( *this );
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XPropertySetInfo > SAL_CALL SpinningProgressControlModel::getPropertySetInfo(  ) throw(RuntimeException)
    {
        static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL SpinningProgressControlModel::getServiceName() throw(RuntimeException)
    {
        return ::rtl::OUString::createFromAscii( szServiceName_SpinningProgressControlModel );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL SpinningProgressControlModel::getImplementationName(  ) throw(RuntimeException)
    {
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.toolkit.SpinningProgressControlModel"));
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL SpinningProgressControlModel::getSupportedServiceNames() throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aServiceNames(3);
        aServiceNames[0] = ::rtl::OUString::createFromAscii( szServiceName_SpinningProgressControlModel );
        aServiceNames[1] = ::rtl::OUString::createFromAscii( szServiceName_AnimatedImagesControlModel );
        aServiceNames[2] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlModel"));
        return aServiceNames;
    }

//......................................................................................................................
} // namespace toolkit
//......................................................................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
