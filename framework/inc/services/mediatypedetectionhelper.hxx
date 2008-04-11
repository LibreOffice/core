/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mediatypedetectionhelper.hxx,v $
 * $Revision: 1.6 $
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

#ifndef __FRAMEWORK_SERVICES_MEDIATYPEDETECTIONHELPER_HXX_
#define __FRAMEWORK_SERVICES_MEDIATYPEDETECTIONHELPER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <macros/generic.hxx>
#include <macros/debug.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <general.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/util/XStringMapping.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/weak.hxx>

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          -
    @descr          -

    @implements     XInterface
                    XTypeProvider
                    XServiceInfo
                    XStringMapping
    @base           OWeakObject

    @devstatus      deprecated
*//*-*************************************************************************************************************/

class MediaTypeDetectionHelper  :   public css::lang::XTypeProvider
                                ,   public css::lang::XServiceInfo
                                ,   public css::util::XStringMapping
                                ,   public ::cppu::OWeakObject
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      standard ctor
            @descr      These initialize a new instance of this class with all needed informations for work.

            @seealso    -

            @param      "xFactory", reference to factory which has created ouer owner(!). We can use these to create new uno-services.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

         MediaTypeDetectionHelper( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );

        /*-****************************************************************************************************//**
            @short      standard destructor
            @descr      This method destruct an instance of this class and clear some member.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ~MediaTypeDetectionHelper();

        //---------------------------------------------------------------------------------------------------------
        //  XInterface, XTypeProvider, XServiceInfo
        //---------------------------------------------------------------------------------------------------------

        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        //---------------------------------------------------------------------------------------------------------
        //  XStringMapping
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL mapStrings( css::uno::Sequence< ::rtl::OUString >& seqParameter ) throw( css::uno::RuntimeException );

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  should be private every time
    //-------------------------------------------------------------------------------------------------------------

    private:

        css::uno::Reference< css::lang::XMultiServiceFactory >      m_xFactory; /// reference to global servicemanager

};

}       // namespace framework

#endif  // #ifndef __FRAMEWORK_SERVICES_MEDIATYPEDETECTIONHELPER_HXX_
