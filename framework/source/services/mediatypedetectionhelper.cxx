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

#include <services/mediatypedetectionhelper.hxx>
#include <svl/inettype.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace framework
{

using namespace ::com::sun::star;

//  constructor

MediaTypeDetectionHelper::MediaTypeDetectionHelper()
{
}

//  destructor

MediaTypeDetectionHelper::~MediaTypeDetectionHelper()
{
}

// XInterface, XTypeProvider, XServiceInfo

OUString SAL_CALL MediaTypeDetectionHelper::getImplementationName()
{
    return u"com.sun.star.comp.framework.MediaTypeDetectionHelper"_ustr;
}

sal_Bool SAL_CALL MediaTypeDetectionHelper::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL MediaTypeDetectionHelper::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.MediaTypeDetectionHelper"_ustr };
}


//   XStringMapping

sal_Bool SAL_CALL MediaTypeDetectionHelper::mapStrings(uno::Sequence< OUString >& rSeq)
{
    bool bModified = false;
    auto rSeqRange = asNonConstRange(rSeq);
    for( sal_Int32 i = rSeq.getLength(); i--; )
    {

        OUString& rUrl = rSeqRange[i];
        INetContentType eType = INetContentTypes::GetContentTypeFromURL( rUrl );

        OUString aType( INetContentTypes::GetContentType( eType ) );
        if (!aType.isEmpty())
        {
            rUrl = aType;
            bModified = true;
        }
    }
    return bModified;
}

}   // namespace framework


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
framework_MediaTypeDetectionHelper_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new framework::MediaTypeDetectionHelper());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
