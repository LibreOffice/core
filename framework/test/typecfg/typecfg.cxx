/*************************************************************************
 *
 *  $RCSfile: typecfg.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:22:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_SERVICEMANAGER_HXX_
#include <classes/servicemanager.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_FILTERCACHE_HXX_
#include <classes/filtercache.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef __SGI_STL_HASH_MAP
#include <hash_map>
#endif

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <stdio.h>

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

#define RDBFILE                                 DECLARE_ASCII("typecfg.rdb"     )
#define ARGUMENT_GENERATE_CFGVIEW               DECLARE_ASCII("-cfgview"        )
#define ARGUMENT_CHECK_FILTERREGISTRATION       DECLARE_ASCII("-registerfilter" )
#define ARGUMENT_GENERATE_TYPEDETECTION_XCD     DECLARE_ASCII("-generatexcd"    )

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::std                       ;
using namespace ::vos                       ;
using namespace ::rtl                       ;
using namespace ::framework                 ;
using namespace ::comphelper                ;
using namespace ::com::sun::star::uno       ;
using namespace ::com::sun::star::lang      ;
using namespace ::com::sun::star::container ;
using namespace ::com::sun::star::beans     ;

//_________________________________________________________________________________________________________________
//  defines
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

enum EMode
{
    E_GENERATE_CFGVIEW              ,
    E_CHECK_FILTERREGISTRATION      ,
    E_GENERATE_TYPEDETECTION_XCD
};

/*-***************************************************************************************************************/
class TypeApplication : public Application
{
    //*************************************************************************************************************
    public:

        void Main();

    //*************************************************************************************************************
    private:

        void impl_parseCommandLine          ();
        void impl_generateCFGView           ();
        void impl_checkFilterRegistration   ();
        void impl_generateTypeDetectionXCD  ();

    //*************************************************************************************************************
    private:

        EMode                               m_eMode             ;
        Reference< XMultiServiceFactory >   m_xServiceManager   ;

        FilterCache*                        m_pCache            ;

};  //  class FilterApplication

//_________________________________________________________________________________________________________________
//  global variables
//_________________________________________________________________________________________________________________

TypeApplication gApplication;

//*****************************************************************************************************************
void TypeApplication::Main()
{
    // Init global servicemanager and set it.
    ServiceManager aManager;
    m_xServiceManager = aManager.getSharedUNOServiceManager( RDBFILE );
    setProcessServiceFactory( m_xServiceManager );

    m_pCache = new FilterCache;

    impl_parseCommandLine();

    switch( m_eMode )
    {
        case E_GENERATE_CFGVIEW                 :   impl_generateCFGView();
                                                    break;
        case E_CHECK_FILTERREGISTRATION         :   impl_checkFilterRegistration();
                                                    break;
        case E_GENERATE_TYPEDETECTION_XCD       :   impl_generateTypeDetectionXCD();
                                                    break;
    }

    delete m_pCache;
    m_pCache = NULL;
}

//*****************************************************************************************************************
void TypeApplication::impl_parseCommandLine()
{
    OStartupInfo    aInfo       ;
    OUString        sArgument   ;
    sal_Int32       nArgument   = 0                         ;
    sal_Int32       nCount      = aInfo.getCommandArgCount();

    while( nArgument<nCount )
    {
        aInfo.getCommandArg( nArgument, sArgument );

        if( sArgument == ARGUMENT_GENERATE_CFGVIEW )
        {
            m_eMode = E_GENERATE_CFGVIEW;
            break;
        }
        else
        if( sArgument == ARGUMENT_CHECK_FILTERREGISTRATION )
        {
            m_eMode = E_CHECK_FILTERREGISTRATION;
            break;
        }
        else
        if( sArgument == ARGUMENT_GENERATE_TYPEDETECTION_XCD )
        {
            m_eMode = E_GENERATE_TYPEDETECTION_XCD;
            break;
        }

        ++nArgument;
    }
}

//*****************************************************************************************************************
void TypeApplication::impl_generateCFGView()
{
    #ifdef ENABLE_FILTERCACHEDEBUG
        // Cache use ref count!
        FilterCache aCache;
        aCache.impldbg_generateHTMLView();
    #endif  //ENABLE_FILTERCACHEDEBUG
}

//*****************************************************************************************************************
void TypeApplication::impl_checkFilterRegistration()
{
    Reference< XNameContainer > xFilterContainer( m_xServiceManager->createInstance( SERVICENAME_FILTERFACTORY ), UNO_QUERY );
    LOG_ASSERT2( xFilterContainer.is()==sal_False, "TypeApplication::impl_checkFilterRegistration()", "Couldn't create filter factory!" )
    if( xFilterContainer.is() == sal_True )
    {
        Sequence< PropertyValue > lProperties( 8 );

        lProperties[0].Name     =   DECLARE_ASCII("Type")               ;
        lProperties[0].Value    <<= DECLARE_ASCII("MeinType")           ;

        lProperties[1].Name     =   DECLARE_ASCII("UIName")             ;
        lProperties[1].Value    <<= DECLARE_ASCII("MeinUIName")         ;

        lProperties[2].Name     =   DECLARE_ASCII("UINames")            ;
        lProperties[2].Value    <<= Sequence< PropertyValue >()         ;

        lProperties[3].Name     =   DECLARE_ASCII("DocumentService")    ;
        lProperties[3].Value    <<= DECLARE_ASCII("MeinDocService")     ;

        lProperties[4].Name     =   DECLARE_ASCII("FilterService")      ;
        lProperties[4].Value    <<= DECLARE_ASCII("MeinFilterService")  ;

        lProperties[5].Name     =   DECLARE_ASCII("Flags")              ;
        lProperties[5].Value    <<= (sal_Int32)256                      ;

        lProperties[6].Name     =   DECLARE_ASCII("UserData")           ;
        lProperties[6].Value    <<= Sequence< OUString >()              ;

        lProperties[7].Name     =   DECLARE_ASCII("FileFormatVersion")  ;
        lProperties[7].Value    <<= (sal_Int32)0                        ;

        lProperties[8].Name     =   DECLARE_ASCII("TemplateName")       ;
        lProperties[8].Value    <<= DECLARE_ASCII("MeinTemplate")       ;
    }
}

//*****************************************************************************************************************
void TypeApplication::impl_generateTypeDetectionXCD()
{
    #ifdef ENABLE_GENERATEFILTERCACHE
    // Cache use ref count!
    FilterCache aCache                  ;
    sal_Bool    bWriteable  =   sal_True;
    sal_Unicode cSeparator  =   ','     ;
    aCache.impldbg_generateXCD( "org.openoffice.Office.TypeDetection.xcd", bWriteable, cSeparator );
    #endif  //ENABLE_GENERATEFILTERCACHE
}
