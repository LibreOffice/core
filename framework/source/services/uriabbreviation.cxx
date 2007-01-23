/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: uriabbreviation.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-01-23 07:10:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "services/uriabbreviation.hxx"
#include "services.h"

#include "sal/config.h"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"

#include "tools/urlobj.hxx"

// component helper namespace
namespace css = ::com::sun::star;

// framework namespace
namespace framework
{

namespace css = ::com::sun::star;

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_3                (    UriAbbreviation                                                         ,
                                        cppu::OWeakObject                                                       ,
                                        DIRECT_INTERFACE(css::lang::XTypeProvider                               ),
                                        DIRECT_INTERFACE(css::lang::XServiceInfo                                ),
                                        DIRECT_INTERFACE(css::util::XStringAbbreviation                         )
                                    )

DEFINE_XTYPEPROVIDER_3              (   UriAbbreviation                                                         ,
                                        css::lang::XTypeProvider                                                ,
                                        css::lang::XServiceInfo                                                 ,
                                        css::util::XStringAbbreviation
                                    )

DEFINE_XSERVICEINFO_MULTISERVICE_2 (    UriAbbreviation                                                         ,
                                        ::cppu::OWeakObject                                                     ,
                                        SERVICENAME_STRINGABBREVIATION                                          ,
                                        IMPLEMENTATIONNAME_URIABBREVIATION
                                    )

DEFINE_INIT_SERVICE                 (   UriAbbreviation,
                                        {
                                        }
                                    )

UriAbbreviation::UriAbbreviation(css::uno::Reference< css::uno::XComponentContext > const & context) :
    m_xContext(context)
{
}

// ::com::sun::star::util::XStringAbbreviation:
::rtl::OUString SAL_CALL UriAbbreviation::abbreviateString(const css::uno::Reference< css::util::XStringWidth > & xStringWidth, ::sal_Int32 nWidth, const ::rtl::OUString & aString) throw (css::uno::RuntimeException)
{
    ::rtl::OUString aResult( aString );
    if ( xStringWidth.is() )
    {
        // Use INetURLObject to abbreviate URLs
        INetURLObject aURL( aString );
        aResult = aURL.getAbbreviated( xStringWidth, nWidth, INetURLObject::DECODE_UNAMBIGUOUS );
    }

    return aResult;
}

} // namespace framework
