/*************************************************************************
 *
 *  $RCSfile: arguments.h,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 16:13:54 $
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

#ifndef __FRAMEWORK_ARGUMENTS_H_
#define __FRAMEWORK_ARGUMENTS_H_

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

/*-************************************************************************************************************//**
    @short          These arguments are included in given parameter lists - e.g. at dispatch calls at Sequence< PropertyValue >.
                    You can use class ArgumentAnalyzer to set/get it from a given list!
*//*-*************************************************************************************************************/

#define ARGUMENTNAME_ASTEMPLATE                             DECLARE_ASCII("AsTemplate"          )   // bool
#define ARGUMENTNAME_CHARACTERSET                           DECLARE_ASCII("CharacterSet"        )   // string
#define ARGUMENTNAME_DEEPDETECTION                          DECLARE_ASCII("DeepDetection"       )   // bool
#define ARGUMENTNAME_DETECTSERVICE                          DECLARE_ASCII("DetectService"       )   // string
#define ARGUMENTNAME_EXTENSION                              DECLARE_ASCII("Extension"           )   // string
#define ARGUMENTNAME_FILTERNAME                             DECLARE_ASCII("FilterName"          )   // string
#define ARGUMENTNAME_FILTEROPTIONS                          DECLARE_ASCII("FilterOptions"       )   // string
#define ARGUMENTNAME_FORMAT                                 DECLARE_ASCII("Format"              )   // string
#define ARGUMENTNAME_FRAMENAME                              DECLARE_ASCII("FrameName"           )   // string
#define ARGUMENTNAME_HIDDEN                                 DECLARE_ASCII("Hidden"              )   // bool
#define ARGUMENTNAME_INPUTSTREAM                            DECLARE_ASCII("InputStream"         )   // Reference< XInputStream >
#define ARGUMENTNAME_INTERACTIONHANDLER                     DECLARE_ASCII("InteractionHandler"  )   // Reference< XInteractionHandler >
#define ARGUMENTNAME_JUMPMARK                               DECLARE_ASCII("JumpMark"            )   // string
#define ARGUMENTNAME_MACROEXECUTIONMODE                     DECLARE_ASCII("MacroExecutionMode"  )   // int16
#define ARGUMENTNAME_MEDIATYPE                              DECLARE_ASCII("MediaType"           )   // string
#define ARGUMENTNAME_MINIMIZED                              DECLARE_ASCII("Minimized"           )   // bool
#define ARGUMENTNAME_OPENNEWVIEW                            DECLARE_ASCII("OpenNewView"         )   // bool
#define ARGUMENTNAME_OUTPUTSTREAM                           DECLARE_ASCII("OutputStream"        )   // Reference< XOutputStream >
#define ARGUMENTNAME_PATTERN                                DECLARE_ASCII("Pattern"             )   // string
#define ARGUMENTNAME_POSSIZE                                DECLARE_ASCII("PosSize"             )   // rectangle
#define ARGUMENTNAME_POSTDATA                               DECLARE_ASCII("PostData"            )   // Reference< XInputStream >
#define ARGUMENTNAME_POSTSTRING                             DECLARE_ASCII("PostString"          )   // string
#define ARGUMENTNAME_PREVIEW                                DECLARE_ASCII("Preview"             )   // bool
#define ARGUMENTNAME_READONLY                               DECLARE_ASCII("ReadOnly"            )   // bool
#define ARGUMENTNAME_REFERRER                               DECLARE_ASCII("Referer"             )   // string
#define ARGUMENTNAME_SILENT                                 DECLARE_ASCII("Silent"              )   // bool
#define ARGUMENTNAME_STATUSINDICATOR                        DECLARE_ASCII("StatusIndicator"     )   // Reference< XStatusIndicator >
#define ARGUMENTNAME_TEMPLATENAME                           DECLARE_ASCII("TemplateName"        )   // string
#define ARGUMENTNAME_TEMPLATEREGIONNAME                     DECLARE_ASCII("TemplateRegionName"  )   // string
#define ARGUMENTNAME_TYPENAME                               DECLARE_ASCII("TypeName"            )   // string
#define ARGUMENTNAME_UPDATEDOCMODE                          DECLARE_ASCII("UpdateDocMode"       )   // int16
#define ARGUMENTNAME_URL                                    DECLARE_ASCII("URL"                 )   // string
#define ARGUMENTNAME_VERSION                                DECLARE_ASCII("Version"             )   // int16
#define ARGUMENTNAME_VIEWID                                 DECLARE_ASCII("ViewId"              )   // int16
#define ARGUMENTNAME_REPAIRPACKAGE                          DECLARE_ASCII("RepairPackage"       )   // bool
#define ARGUMENTNAME_DOCUMENTTITLE                          DECLARE_ASCII("DocumentTitle"       )   // string

/*-************************************************************************************************************//**
    @short          define our argument mask
    @descr          These mask could be used to define a subset for analyzing arguments or specify
                    one argument for set/get operations.
                    We use first byte [bit 0..7] of an int32 to define 8 layer, and follow 3 bytes [bit 8..31]
                    to address 24 properties per layer! Please use defines to build these mask!!!
                    With these values we build an special enum field for better using at our Argumentanalyzer-interface ...
                    Special define ANALYZE_ALL is our default for analyzing. Then we analyze complete argument list.
*//*-*************************************************************************************************************/
#define ARGUMENTLAYER_1                                     0x00000001
#define ARGUMENTLAYER_2                                     0x00000002
#define ARGUMENTLAYER_3                                     0x00000004
#define ARGUMENTLAYER_4                                     0x00000008
#define ARGUMENTLAYER_5                                     0x00000010
#define ARGUMENTLAYER_6                                     0x00000020
#define ARGUMENTLAYER_7                                     0x00000040
#define ARGUMENTLAYER_8                                     0x00000080

#define ARGUMENTFLAG_1                                      0x00000100
#define ARGUMENTFLAG_2                                      0x00000200
#define ARGUMENTFLAG_3                                      0x00000400
#define ARGUMENTFLAG_4                                      0x00000800
#define ARGUMENTFLAG_5                                      0x00001000
#define ARGUMENTFLAG_6                                      0x00002000
#define ARGUMENTFLAG_7                                      0x00004000
#define ARGUMENTFLAG_8                                      0x00008000
#define ARGUMENTFLAG_9                                      0x00010000
#define ARGUMENTFLAG_10                                     0x00020000
#define ARGUMENTFLAG_11                                     0x00040000
#define ARGUMENTFLAG_12                                     0x00080000
#define ARGUMENTFLAG_13                                     0x00100000
#define ARGUMENTFLAG_14                                     0x00200000
#define ARGUMENTFLAG_15                                     0x00400000
#define ARGUMENTFLAG_16                                     0x00800000
#define ARGUMENTFLAG_17                                     0x01000000
#define ARGUMENTFLAG_18                                     0x02000000
#define ARGUMENTFLAG_19                                     0x04000000
#define ARGUMENTFLAG_20                                     0x08000000
#define ARGUMENTFLAG_21                                     0x10000000
#define ARGUMENTFLAG_22                                     0x20000000
#define ARGUMENTFLAG_23                                     0x40000000
#define ARGUMENTFLAG_24                                     0x80000000

#define ANALYZE_ALL_ARGUMENTS                               0xFFFFFFFF

enum EArgument
{
    E_ASTEMPLATE            = ARGUMENTLAYER_1 | ARGUMENTFLAG_1 ,
    E_CHARACTERSET          = ARGUMENTLAYER_1 | ARGUMENTFLAG_2 ,
    E_DEEPDETECTION         = ARGUMENTLAYER_1 | ARGUMENTFLAG_3 ,
    E_DETECTSERVICE         = ARGUMENTLAYER_1 | ARGUMENTFLAG_4 ,
    E_EXTENSION             = ARGUMENTLAYER_1 | ARGUMENTFLAG_5 ,
    E_FILTERNAME            = ARGUMENTLAYER_1 | ARGUMENTFLAG_6 ,
    E_FILTEROPTIONS         = ARGUMENTLAYER_1 | ARGUMENTFLAG_7 ,
    E_FORMAT                = ARGUMENTLAYER_1 | ARGUMENTFLAG_8 ,
    E_FRAMENAME             = ARGUMENTLAYER_1 | ARGUMENTFLAG_9 ,
    E_HIDDEN                = ARGUMENTLAYER_1 | ARGUMENTFLAG_10,
    E_INPUTSTREAM           = ARGUMENTLAYER_1 | ARGUMENTFLAG_11,
    E_INTERACTIONHANDLER    = ARGUMENTLAYER_1 | ARGUMENTFLAG_12,
    E_JUMPMARK              = ARGUMENTLAYER_1 | ARGUMENTFLAG_13,
    E_MACROEXECUTIONMODE    = ARGUMENTLAYER_1 | ARGUMENTFLAG_14,
    E_MEDIATYPE             = ARGUMENTLAYER_1 | ARGUMENTFLAG_15,
    E_MINIMIZED             = ARGUMENTLAYER_1 | ARGUMENTFLAG_16,
    E_OPENNEWVIEW           = ARGUMENTLAYER_1 | ARGUMENTFLAG_17,
    E_OUTPUTSTREAM          = ARGUMENTLAYER_1 | ARGUMENTFLAG_18,
    E_PATTERN               = ARGUMENTLAYER_1 | ARGUMENTFLAG_19,
    E_POSSIZE               = ARGUMENTLAYER_1 | ARGUMENTFLAG_20,
    E_POSTDATA              = ARGUMENTLAYER_1 | ARGUMENTFLAG_21,
    E_POSTSTRING            = ARGUMENTLAYER_1 | ARGUMENTFLAG_22,
    E_PREVIEW               = ARGUMENTLAYER_1 | ARGUMENTFLAG_23,
    E_READONLY              = ARGUMENTLAYER_1 | ARGUMENTFLAG_24,

    E_REFERRER              = ARGUMENTLAYER_2 | ARGUMENTFLAG_1 ,
    E_SILENT                = ARGUMENTLAYER_2 | ARGUMENTFLAG_2 ,
    E_STATUSINDICATOR       = ARGUMENTLAYER_2 | ARGUMENTFLAG_3 ,
    E_TEMPLATENAME          = ARGUMENTLAYER_2 | ARGUMENTFLAG_4 ,
    E_TEMPLATEREGIONNAME    = ARGUMENTLAYER_2 | ARGUMENTFLAG_5 ,
    E_TYPENAME              = ARGUMENTLAYER_2 | ARGUMENTFLAG_6 ,
    E_UPDATEDOCMODE         = ARGUMENTLAYER_2 | ARGUMENTFLAG_7 ,
    E_URL                   = ARGUMENTLAYER_2 | ARGUMENTFLAG_8 ,
    E_VERSION               = ARGUMENTLAYER_2 | ARGUMENTFLAG_9 ,
    E_VIEWID                = ARGUMENTLAYER_2 | ARGUMENTFLAG_10,
    E_REPAIRPACKAGE         = ARGUMENTLAYER_2 | ARGUMENTFLAG_11,
    E_DOCUMENTTITLE         = ARGUMENTLAYER_2 | ARGUMENTFLAG_12
};

} // namespace framework

#endif // #ifndef __FRAMEWORK_ARGUMENTS_H_
