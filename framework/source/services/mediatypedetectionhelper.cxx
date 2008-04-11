/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mediatypedetectionhelper.cxx,v $
 * $Revision: 1.9 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <services/mediatypedetectionhelper.hxx>
#include <services.h>
#include <svtools/inettype.hxx>
#include <tools/string.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework
{

using namespace ::com::sun::star        ;
using namespace ::rtl                   ;

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

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

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************

DEFINE_XINTERFACE_3                 (   MediaTypeDetectionHelper
                                        , OWeakObject
                                        , DIRECT_INTERFACE( lang::XTypeProvider )
                                        , DIRECT_INTERFACE( lang::XServiceInfo  )
                                        , DIRECT_INTERFACE( util::XStringMapping )
                                    )

DEFINE_XTYPEPROVIDER_3              (   MediaTypeDetectionHelper
                                        , lang::XTypeProvider
                                        , lang::XServiceInfo
                                        , util::XStringMapping
                                    )

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   MediaTypeDetectionHelper
                                        , ::cppu::OWeakObject
                                        , SERVICENAME_MEDIATYPEDETECTIONHELPER
                                        , IMPLEMENTATIONNAME_MEDIATYPEDETECTIONHELPER
                                    )

DEFINE_INIT_SERVICE                 (   MediaTypeDetectionHelper,
                                        {
                                        }
                                    )

//*****************************************************************************************************************
//   XStringMapping
//*****************************************************************************************************************

//virtual
sal_Bool SAL_CALL MediaTypeDetectionHelper::mapStrings(
        uno::Sequence< OUString >& rSeq )
        throw(uno::RuntimeException)
{
    sal_Bool bModified = sal_False;
    for( sal_Int32 i = rSeq.getLength(); i--; )
    {

        OUString& rUrl = rSeq[i];
        INetContentType eType = INetContentTypes::GetContentTypeFromURL( rUrl );

        UniString aType( INetContentTypes::GetContentType( eType ) );
        if( aType.Len() )
        {
            rUrl = aType;
            bModified = sal_True;
        }
    }
    return bModified;
}

}   // namespace framework

