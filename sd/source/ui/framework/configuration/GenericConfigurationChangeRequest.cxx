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

#include "precompiled_sd.hxx"

#include "GenericConfigurationChangeRequest.hxx"

#include "framework/FrameworkHelper.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::rtl::OUString;

namespace sd { namespace framework {

GenericConfigurationChangeRequest::GenericConfigurationChangeRequest (
    const Reference<XResourceId>& rxResourceId,
    const Mode eMode) throw(::com::sun::star::lang::IllegalArgumentException)
    : GenericConfigurationChangeRequestInterfaceBase(MutexOwner::maMutex),
      mxResourceId(rxResourceId),
      meMode(eMode)
{
    if ( ! rxResourceId.is() || rxResourceId->getResourceURL().getLength()==0)
        throw ::com::sun::star::lang::IllegalArgumentException();
}




GenericConfigurationChangeRequest::~GenericConfigurationChangeRequest (void) throw()
{
}




void SAL_CALL GenericConfigurationChangeRequest::execute (
    const Reference<XConfiguration>& rxConfiguration)
    throw (RuntimeException)
{
    if (rxConfiguration.is())
    {
        switch (meMode)
        {
            case Activation:
                rxConfiguration->addResource(mxResourceId);
                break;

            case Deactivation:
                rxConfiguration->removeResource(mxResourceId);
                break;
        }
    }
}




OUString SAL_CALL GenericConfigurationChangeRequest::getName (void)
    throw (RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM("GenericConfigurationChangeRequest "))
        + (meMode==Activation ? OUString(RTL_CONSTASCII_USTRINGPARAM("activate ")) : OUString(RTL_CONSTASCII_USTRINGPARAM("deactivate ")))
        + FrameworkHelper::ResourceIdToString(mxResourceId);
}




void SAL_CALL GenericConfigurationChangeRequest::setName (const OUString& rsName)
    throw (RuntimeException)
{
    (void)rsName;
    // Ignored.
}

} } // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
