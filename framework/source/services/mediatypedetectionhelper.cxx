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

#include <services/mediatypedetectionhelper.hxx>
#include <services.h>
#include <svl/inettype.hxx>
#include <tools/string.hxx>
#include <rtl/logfile.hxx>

namespace framework
{

using namespace ::com::sun::star        ;
using namespace ::rtl                   ;

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
MediaTypeDetectionHelper::MediaTypeDetectionHelper( const uno::Reference< lang::XMultiServiceFactory >& xFactory )
    : m_xFactory( xFactory )
{
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
MediaTypeDetectionHelper::~MediaTypeDetectionHelper()
{
}

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   MediaTypeDetectionHelper
                                        , ::cppu::OWeakObject
                                        , DECLARE_ASCII("com.sun.star.frame.MediaTypeDetectionHelper")
                                        , IMPLEMENTATIONNAME_MEDIATYPEDETECTIONHELPER
                                    )

DEFINE_INIT_SERVICE                 (   MediaTypeDetectionHelper,
                                        {
                                        }
                                    )

//*****************************************************************************************************************
//   XStringMapping
//*****************************************************************************************************************

sal_Bool SAL_CALL MediaTypeDetectionHelper::mapStrings(
        uno::Sequence< OUString >& rSeq )
        throw(uno::RuntimeException)
{
    sal_Bool bModified = sal_False;
    for( sal_Int32 i = rSeq.getLength(); i--; )
    {

        OUString& rUrl = rSeq[i];
        INetContentType eType = INetContentTypes::GetContentTypeFromURL( rUrl );

        rtl::OUString aType( INetContentTypes::GetContentType( eType ) );
        if (!aType.isEmpty())
        {
            rUrl = aType;
            bModified = sal_True;
        }
    }
    return bModified;
}

}   // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
