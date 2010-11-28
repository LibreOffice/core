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
#include "precompiled_reportdesign.hxx"
#include "dbloader2.hxx"
#include <comphelper/sequence.hxx>
// -------------------------------------------------------------------------
namespace rptxml
{

using namespace ::ucbhelper;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::ui::dialogs;
using ::com::sun::star::awt::XWindow;

// -------------------------------------------------------------------------
ORptTypeDetection::ORptTypeDetection(Reference< XComponentContext > const & xContext)
: m_xContext(xContext)
{
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ORptTypeDetection::detect( Sequence< ::com::sun::star::beans::PropertyValue >& Descriptor ) throw (RuntimeException)
{

    ::comphelper::SequenceAsHashMap aTemp(Descriptor);
    ::rtl::OUString sTemp = aTemp.getUnpackedValueOrDefault(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL")),::rtl::OUString());

    if ( sTemp.getLength() )
    {
        INetURLObject aURL(sTemp);
        if ( aURL.GetExtension().equalsIgnoreAsciiCaseAscii("orp") )
            return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StarBaseReport"));
        else
        {
            try
            {
                Reference<XPropertySet> xProp(::comphelper::OStorageHelper::GetStorageFromURL(sTemp,ElementModes::READ,Reference< XMultiServiceFactory >(m_xContext->getServiceManager(),UNO_QUERY)),UNO_QUERY);
                if ( xProp.is() )
                {
                    ::rtl::OUString sMediaType;
                    xProp->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MediaType")) ) >>= sMediaType;
                    if ( sMediaType.equalsAscii(MIMETYPE_OASIS_OPENDOCUMENT_REPORT_ASCII) )
                        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StarBaseReport"));
                    ::comphelper::disposeComponent(xProp);
                }
            }
            catch(Exception&)
            {
            }
        }
    }
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL
        ORptTypeDetection::create(Reference< XComponentContext > const & xContext)
{
    return *(new ORptTypeDetection(xContext));
}
// -------------------------------------------------------------------------
// XServiceInfo
::rtl::OUString SAL_CALL ORptTypeDetection::getImplementationName() throw(  )
{
    return getImplementationName_Static();
}
// -------------------------------------------------------------------------

// XServiceInfo
sal_Bool SAL_CALL ORptTypeDetection::supportsService(const ::rtl::OUString& ServiceName) throw(  )
{
    return ::comphelper::existsValue(ServiceName,getSupportedServiceNames_Static());
}
// -------------------------------------------------------------------------
// XServiceInfo
Sequence< ::rtl::OUString > SAL_CALL ORptTypeDetection::getSupportedServiceNames(void) throw(  )
{
    return getSupportedServiceNames_Static();
}
// -------------------------------------------------------------------------
// ORegistryServiceManager_Static
Sequence< ::rtl::OUString > ORptTypeDetection::getSupportedServiceNames_Static(void) throw( RuntimeException )
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS.getArray()[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.ExtendedTypeDetection"));
    return aSNS;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
}//rptxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
