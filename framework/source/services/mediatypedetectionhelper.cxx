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
#include <services.h>
#include <svl/inettype.hxx>

namespace framework
{

using namespace ::com::sun::star;

//  constructor

MediaTypeDetectionHelper::MediaTypeDetectionHelper( const uno::Reference< lang::XMultiServiceFactory >&  )
{
}

//  destructor

MediaTypeDetectionHelper::~MediaTypeDetectionHelper()
{
}

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   MediaTypeDetectionHelper
                                        , ::cppu::OWeakObject
                                        , "com.sun.star.frame.MediaTypeDetectionHelper"
                                        , IMPLEMENTATIONNAME_MEDIATYPEDETECTIONHELPER
                                    )

DEFINE_INIT_SERVICE                 (   MediaTypeDetectionHelper,
                                        {
                                        }
                                    )

//   XStringMapping

sal_Bool SAL_CALL MediaTypeDetectionHelper::mapStrings(uno::Sequence< OUString >& rSeq)
    throw (uno::RuntimeException, std::exception)
{
    bool bModified = false;
    for( sal_Int32 i = rSeq.getLength(); i--; )
    {

        OUString& rUrl = rSeq[i];
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
