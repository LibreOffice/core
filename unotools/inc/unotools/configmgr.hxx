/*************************************************************************
 *
 *  $RCSfile: configmgr.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2000-09-26 09:25:38 $
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

#ifndef _UTL_CONFIGMGR_HXX_
#define _UTL_CONFIGMGR_HXX_

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

//-----------------------------------------------------------------------------
namespace com{ namespace sun{ namespace star{
    namespace lang{
        class XMultiServiceFactory;
    }
    namespace container{
        class XHierarchicalNameAccess;
    }
}}}

//-----------------------------------------------------------------------------
namespace utl
{
    struct ConfigMgr_Impl;
    class ConfigItem;
    class ConfigManager
    {
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                xConfigurationProvider;
            ConfigMgr_Impl*     pMgrImpl;

            static  ConfigManager*  pConfigManager;
        public:
            ConfigManager();
            ~ConfigManager();

            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                GetConfigurationProvider();

            com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess>
                AddConfigItem(utl::ConfigItem& rCfgItem);

            void RemoveConfigItem(utl::ConfigItem& rCfgItem);

            void StoreConfigItems();

            static ConfigManager*           GetConfigManager();
            static void                     RemoveConfigManager();
            static rtl::OUString            GetConfigBaseURL();

            enum ConfigProperty
            {
                INSTALLPATH,
                LOCALE,
                OFFICEINSTALL
            };
            //direct readonly access to some special configuration elements
            static com::sun::star::uno::Any GetDirectConfigProperty(ConfigProperty eProp);

    };
}//namespace utl
#endif //_UTL_CONFIGMGR_HXX_
