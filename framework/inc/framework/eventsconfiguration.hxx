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

#ifndef __FRAMEWORK_XML_EVENTSCONFIGURATION_HXX_
#define __FRAMEWORK_XML_EVENTSCONFIGURATION_HXX_

#include <framework/fwedllapi.h>
#include <svl/svarray.hxx>
#include <tools/string.hxx>
#include <tools/stream.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

// #110897#
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace framework
{

struct FWE_DLLPUBLIC EventsConfig
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString >              aEventNames;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >   aEventsProperties;
};

class FWE_DLLPUBLIC EventsConfiguration
{
    public:
        // #110897#
        static sal_Bool LoadEventsConfig(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            SvStream& rInStream, EventsConfig& aItems );

        // #110897#
        static sal_Bool StoreEventsConfig(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            SvStream& rOutStream, const EventsConfig& aItems );
};

} // namespace framework

#endif // __FRAMEWORK_XML_EVENTSCONFIGURATION_HXX_
