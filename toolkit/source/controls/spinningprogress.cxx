/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
