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

#ifndef TOOLKIT_SPINNINGPROGRESS_HXX
#define TOOLKIT_SPINNINGPROGRESS_HXX

#include "toolkit/controls/animatedimages.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

//......................................................................................................................
namespace toolkit
{
//......................................................................................................................

    //==================================================================================================================
    //= SpinningProgressControlModel
    //==================================================================================================================
    typedef AnimatedImagesControlModel SpinningProgressControlModel_Base;
    class SpinningProgressControlModel : public SpinningProgressControlModel_Base
    {
    public:
                                    SpinningProgressControlModel();
                                    SpinningProgressControlModel( const SpinningProgressControlModel& i_copySource );

        virtual UnoControlModel*    Clone() const;

        // XPropertySet
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

        // XPersistObject
        ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    protected:
                                    ~SpinningProgressControlModel();
    };

//......................................................................................................................
} // namespace toolkit
//......................................................................................................................

#endif // TOOLKIT_SPINNINGPROGRESS_HXX
