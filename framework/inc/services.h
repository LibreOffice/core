/*************************************************************************
 *
 *  $RCSfile: services.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2001-03-29 13:17:08 $
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

#ifndef __FRAMEWORK_SERVICES_H_
#define __FRAMEWORK_SERVICES_H_

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  used servicenames by framework
//_________________________________________________________________________________________________________________

#define SERVICENAME_FRAME                                   DECLARE_ASCII("com.sun.star.frame.Frame"                            )
#define SERVICENAME_DESKTOP                                 DECLARE_ASCII("com.sun.star.frame.Desktop"                          )
#define SERVICENAME_TASK                                    DECLARE_ASCII("com.sun.star.frame.Task"                             )
#define SERVICENAME_FRAMELOADERFACTORY                      DECLARE_ASCII("com.sun.star.frame.FrameLoaderFactory"               )
#define SERVICENAME_FILTERFACTORY                           DECLARE_ASCII("com.sun.star.document.FilterFactory"                 )
#define SERVICENAME_URLTRANSFORMER                          DECLARE_ASCII("com.sun.star.util.URLTransformer"                    )
#define SERVICENAME_PLUGINFRAME                             DECLARE_ASCII("com.sun.star.mozilla.Plugin"                         )
#define SERVICENAME_BRIDGEFACTORY                           DECLARE_ASCII("com.sun.star.bridge.BridgeFactory"                   )
#define SERVICENAME_INIMANAGER                              DECLARE_ASCII("com.sun.star.config.INIManager"                      )
#define SERVICENAME_MEDIATYPEDETECTIONHELPER                DECLARE_ASCII("com.sun.star.frame.MediaTypeDetectionHelper"         )
#define SERVICENAME_LOGINDIALOG                             DECLARE_ASCII("com.sun.star.framework.LoginDialog"                  )
#define SERVICENAME_CONFIGURATION                           DECLARE_ASCII("com.sun.star.configuration.ConfigurationRegistry"    )
#define SERVICENAME_VCLTOOLKIT                              DECLARE_ASCII("com.sun.star.awt.Toolkit"                            )
#define SERVICENAME_SIMPLEREGISTRY                          DECLARE_ASCII("com.sun.star.registry.SimpleRegistry"                )
#define SERVICENAME_NESTEDREGISTRY                          DECLARE_ASCII("com.sun.star.registry.NestedRegistry"                )
#define SERVICENAME_GENERICFRAMELOADER                      DECLARE_ASCII("com.sun.star.comp.office.FilterDetect"               )
#define SERVICENAME_TYPEDETECTION                           DECLARE_ASCII("com.sun.star.document.TypeDetection"                 )
#define SERVICENAME_DOCUMENTPROPERTIES                      DECLARE_ASCII("com.sun.star.document.DocumentProperties"            )
#define SERVICENAME_CFGMANAGEMENT                           DECLARE_ASCII("com.sun.star.configuration.ConfigurationRegistry"    )   // describe simple registry service for read access to configuration
#define SERVICENAME_CFGPROVIDER                             DECLARE_ASCII("com.sun.star.configuration.ConfigurationProvider"    )   // describe cfg-provider service for fatoffice, network installation
#define SERVICENAME_ADMINPROVIDER                           DECLARE_ASCII("com.sun.star.configuration.AdministrationProvider"   )   // describe cfg-provider service for sun webtop
#define SERVICENAME_CFGUPDATEACCESS                         DECLARE_ASCII("com.sun.star.configuration.ConfigurationUpdateAccess")

//_________________________________________________________________________________________________________________
//  used implementationnames by framework
//_________________________________________________________________________________________________________________

#define IMPLEMENTATIONNAME_FRAME                            DECLARE_ASCII("com.sun.star.comp.framework.Frame"                   )
#define IMPLEMENTATIONNAME_DESKTOP                          DECLARE_ASCII("com.sun.star.comp.framework.Desktop"                 )
#define IMPLEMENTATIONNAME_TASK                             DECLARE_ASCII("com.sun.star.comp.framework.Task"                    )
#define IMPLEMENTATIONNAME_FRAMELOADERFACTORY               DECLARE_ASCII("com.sun.star.comp.framework.FrameLoaderFactory"      )
#define IMPLEMENTATIONNAME_FILTERFACTORY                    DECLARE_ASCII("com.sun.star.comp.framework.FilterFactory"           )
#define IMPLEMENTATIONNAME_URLTRANSFORMER                   DECLARE_ASCII("com.sun.star.comp.framework.URLTransformer"          )
#define IMPLEMENTATIONNAME_PLUGINFRAME                      DECLARE_ASCII("com.sun.star.comp.framework.PluginFrame"             )
#define IMPLEMENTATIONNAME_GENERICFRAMELOADER               DECLARE_ASCII("com.sun.star.comp.office.FrameLoader"                )
#define IMPLEMENTATIONNAME_MEDIATYPEDETECTIONHELPER         DECLARE_ASCII("com.sun.star.comp.framework.MediaTypeDetectionHelper")
#define IMPLEMENTATIONNAME_LOGINDIALOG                      DECLARE_ASCII("com.sun.star.comp.framework.LoginDialog"             )
#define IMPLEMENTATIONNAME_TYPEDETECTION                    DECLARE_ASCII("com.sun.star.comp.framework.TypeDetection"           )
#define IMPLEMENTATIONNAME_DOCUMENTPROPERTIES               DECLARE_ASCII("com.sun.star.comp.framework.DocumentProperties"      )

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_SERVICES_H_
