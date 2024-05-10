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
#include <comphelper/documentconstants.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/XStorage.hpp>

namespace rptxml
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::ui::dialogs;


ORptTypeDetection::ORptTypeDetection(Reference< XComponentContext > const & xContext)
: m_xContext(xContext)
{
}

OUString SAL_CALL ORptTypeDetection::detect( Sequence< css::beans::PropertyValue >& Descriptor )
{

    ::comphelper::SequenceAsHashMap aTemp(Descriptor);
    OUString sTemp = aTemp.getUnpackedValueOrDefault(u"URL"_ustr,OUString());

    if ( !sTemp.isEmpty() )
    {
        INetURLObject aURL(sTemp);
        if ( aURL.GetFileExtension().equalsIgnoreAsciiCase("orp") )
            return u"StarBaseReport"_ustr;
        else
        {
            try
            {
                Reference<XPropertySet> xProp(::comphelper::OStorageHelper::GetStorageFromURL(sTemp,ElementModes::READ, m_xContext),UNO_QUERY);
                if ( xProp.is() )
                {
                    OUString sMediaType;
                    xProp->getPropertyValue(u"MediaType"_ustr) >>= sMediaType;
                    if ( sMediaType == MIMETYPE_OASIS_OPENDOCUMENT_REPORT_ASCII )
                        return u"StarBaseReport"_ustr;
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

// XServiceInfo
OUString SAL_CALL ORptTypeDetection::getImplementationName()
{
    return u"com.sun.star.comp.report.ORptTypeDetection"_ustr;
}


// XServiceInfo
sal_Bool SAL_CALL ORptTypeDetection::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
Sequence< OUString > SAL_CALL ORptTypeDetection::getSupportedServiceNames()
{
    return { u"com.sun.star.document.ExtendedTypeDetection"_ustr };
}


}//rptxml

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
reportdesign_ORptTypeDetection_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new rptxml::ORptTypeDetection(context));
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
