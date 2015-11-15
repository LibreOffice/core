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
#include "dbloader2.hxx"
#include <tools/urlobj.hxx>
#include <cppuhelper/supportsservice.hxx>

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


ORptTypeDetection::ORptTypeDetection(Reference< XComponentContext > const & xContext)
: m_xContext(xContext)
{
}

OUString SAL_CALL ORptTypeDetection::detect( Sequence< css::beans::PropertyValue >& Descriptor ) throw (RuntimeException, std::exception)
{

    ::comphelper::SequenceAsHashMap aTemp(Descriptor);
    OUString sTemp = aTemp.getUnpackedValueOrDefault("URL",OUString());

    if ( !sTemp.isEmpty() )
    {
        INetURLObject aURL(sTemp);
        if ( aURL.GetExtension().equalsIgnoreAsciiCase("orp") )
            return OUString("StarBaseReport");
        else
        {
            try
            {
                Reference<XPropertySet> xProp(::comphelper::OStorageHelper::GetStorageFromURL(sTemp,ElementModes::READ, m_xContext),UNO_QUERY);
                if ( xProp.is() )
                {
                    OUString sMediaType;
                    xProp->getPropertyValue("MediaType") >>= sMediaType;
                    if ( sMediaType == MIMETYPE_OASIS_OPENDOCUMENT_REPORT_ASCII )
                        return OUString("StarBaseReport");
                    ::comphelper::disposeComponent(xProp);
                }
            }
            catch(Exception&)
            {
            }
        }
    }
    return OUString();
}

Reference< XInterface > SAL_CALL
        ORptTypeDetection::create(Reference< XComponentContext > const & xContext)
{
    return *(new ORptTypeDetection(xContext));
}

// XServiceInfo
OUString SAL_CALL ORptTypeDetection::getImplementationName() throw(std::exception  )
{
    return getImplementationName_Static();
}


// XServiceInfo
sal_Bool SAL_CALL ORptTypeDetection::supportsService(const OUString& ServiceName) throw(std::exception  )
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
Sequence< OUString > SAL_CALL ORptTypeDetection::getSupportedServiceNames() throw(std::exception  )
{
    return getSupportedServiceNames_Static();
}

// ORegistryServiceManager_Static
Sequence< OUString > ORptTypeDetection::getSupportedServiceNames_Static() throw( RuntimeException )
{
    Sequence<OUString> aSNS { "com.sun.star.document.ExtendedTypeDetection" };
    return aSNS;
}


}//rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
