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

#ifndef TOOLKIT_SPINNINGPROGRESS_HXX
#define TOOLKIT_SPINNINGPROGRESS_HXX

#include "toolkit/controls/animatedimages.hxx"

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
                                    SpinningProgressControlModel( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & i_factory );
                                    SpinningProgressControlModel( const SpinningProgressControlModel& i_copySource );

        virtual UnoControlModel*    Clone() const;

        // XPropertySet
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

        // XPersistObject
        OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    protected:
                                    ~SpinningProgressControlModel();
    };

//......................................................................................................................
} // namespace toolkit
//......................................................................................................................

#endif // TOOLKIT_SPINNINGPROGRESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
