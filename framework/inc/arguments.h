/*************************************************************************
 *
 *  $RCSfile: arguments.h,v $
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

#define ARGUMENTNAME_CHARACTERSET                           DECLARE_ASCII("CharacterSet"        )   // string
#define ARGUMENTNAME_MEDIATYPE                              DECLARE_ASCII("ContentType"         )   // string
#define ARGUMENTNAME_DETECTSERVICE                          DECLARE_ASCII("DetectService"       )   // string
#define ARGUMENTNAME_EXTENSION                              DECLARE_ASCII("Extension"           )   // string
#define ARGUMENTNAME_URL                                    DECLARE_ASCII("FileName"            )   // string
#define ARGUMENTNAME_FILTERNAME                             DECLARE_ASCII("FilterName"          )   // string
#define ARGUMENTNAME_FORMAT                                 DECLARE_ASCII("Format"              )   // string
#define ARGUMENTNAME_FRAMENAME                              DECLARE_ASCII("FrameName"           )   // string
#define ARGUMENTNAME_PATTERN                                DECLARE_ASCII("Pattern"             )   // string
#define ARGUMENTNAME_POSTDATA                               DECLARE_ASCII("PostData"            )   // string
#define ARGUMENTNAME_POSTSTRING                             DECLARE_ASCII("PostString"          )   // string
#define ARGUMENTNAME_REFERRER                               DECLARE_ASCII("Referer"             )   // string
#define ARGUMENTNAME_TEMPLATENAME                           DECLARE_ASCII("TemplateName"        )   // string
#define ARGUMENTNAME_TEMPLATEREGIONNAME                     DECLARE_ASCII("TemplateRegionName"  )   // string
#define ARGUMENTNAME_VERSION                                DECLARE_ASCII("Version"             )   // int16
#define ARGUMENTNAME_VIEWID                                 DECLARE_ASCII("ViewId"              )   // int16
#define ARGUMENTNAME_FLAGS                                  DECLARE_ASCII("Flags"               )   // int32
#define ARGUMENTNAME_ASTEMPLATE                             DECLARE_ASCII("AsTemplate"          )   // bool
#define ARGUMENTNAME_HIDDEN                                 DECLARE_ASCII("Hidden"              )   // bool
#define ARGUMENTNAME_OPENNEWVIEW                            DECLARE_ASCII("OpenNewView"         )   // bool
#define ARGUMENTNAME_READONLY                               DECLARE_ASCII("ReadOnly"            )   // bool
#define ARGUMENTNAME_PREVIEW                                DECLARE_ASCII("Preview"             )   // bool
#define ARGUMENTNAME_SILENT                                 DECLARE_ASCII("Silent"              )   // bool
#define ARGUMENTNAME_POSSIZE                                DECLARE_ASCII("PosSize"             )   // rectangle
#define ARGUMENTNAME_INPUTSTREAM                            DECLARE_ASCII("InputStream"         )   // Reference< XInputStream >
#define ARGUMENTNAME_DEEPDETECTION                          DECLARE_ASCII("DeepDetection"       )   // bool
#define ARGUMENTNAME_JUMPMARK                               DECLARE_ASCII("JumpMark"            )   // string
#define ARGUMENTNAME_TYPENAME                               DECLARE_ASCII("TypeName"            )   // string

/*-************************************************************************************************************//**
    @short          These values are used by class ArgumentAnalyzer internal to mark existing arguments.
                    My be they can be usefull for other things ... Thats why I publish it here!
*//*-*************************************************************************************************************/

#define ARGUMENTFLAG_CHARACTERSET                           0x0000000000000001L
#define ARGUMENTFLAG_MEDIATYPE                              0x0000000000000002L
#define ARGUMENTFLAG_DETECTSERVICE                          0x0000000000000004L
#define ARGUMENTFLAG_EXTENSION                              0x0000000000000008L
#define ARGUMENTFLAG_URL                                    0x0000000000000010L
#define ARGUMENTFLAG_FILTERNAME                             0x0000000000000020L
#define ARGUMENTFLAG_FORMAT                                 0x0000000000000040L
#define ARGUMENTFLAG_FRAMENAME                              0x0000000000000080L
#define ARGUMENTFLAG_PATTERN                                0x0000000000000100L
#define ARGUMENTFLAG_POSTDATA                               0x0000000000000200L
#define ARGUMENTFLAG_POSTSTRING                             0x0000000000000400L
#define ARGUMENTFLAG_REFERRER                               0x0000000000000800L
#define ARGUMENTFLAG_TEMPLATENAME                           0x0000000000001000L
#define ARGUMENTFLAG_TEMPLATEREGIONNAME                     0x0000000000002000L
#define ARGUMENTFLAG_VERSION                                0x0000000000004000L
#define ARGUMENTFLAG_VIEWID                                 0x0000000000008000L
#define ARGUMENTFLAG_FLAGS                                  0x0000000000010000L
#define ARGUMENTFLAG_ASTEMPLATE                             0x0000000000020000L
#define ARGUMENTFLAG_HIDDEN                                 0x0000000000040000L
#define ARGUMENTFLAG_OPENNEWVIEW                            0x0000000000080000L
#define ARGUMENTFLAG_READONLY                               0x0000000000100000L
#define ARGUMENTFLAG_PREVIEW                                0x0000000000200000L
#define ARGUMENTFLAG_SILENT                                 0x0000000000400000L
#define ARGUMENTFLAG_POSSIZE                                0x0000000000800000L
#define ARGUMENTFLAG_INPUTSTREAM                            0x0000000001000000L
#define ARGUMENTFLAG_DEEPDETECTION                          0x0000000002000000L
#define ARGUMENTFLAG_JUMPMARK                               0x0000000004000000L
#define ARGUMENTFLAG_TYPENAME                               0x0000000008000000L

#define DISABLE_ARGUMENT_CHARACTERSET                       0xFFFFFFFFFFFFFFFEL
#define DISABLE_ARGUMENT_MEDIATYPE                          0xFFFFFFFFFFFFFFFDL
#define DISABLE_ARGUMENT_DETECTSERVICE                      0xFFFFFFFFFFFFFFFBL
#define DISABLE_ARGUMENT_EXTENSION                          0xFFFFFFFFFFFFFFF7L
#define DISABLE_ARGUMENT_URL                                0xFFFFFFFFFFFFFFEFL
#define DISABLE_ARGUMENT_FILTERNAME                         0xFFFFFFFFFFFFFFDFL
#define DISABLE_ARGUMENT_FORMAT                             0xFFFFFFFFFFFFFFBFL
#define DISABLE_ARGUMENT_FRAMENAME                          0xFFFFFFFFFFFFFF7FL
#define DISABLE_ARGUMENT_PATTERN                            0xFFFFFFFFFFFFFEFFL
#define DISABLE_ARGUMENT_POSTDATA                           0xFFFFFFFFFFFFFDFFL
#define DISABLE_ARGUMENT_POSTSTRING                         0xFFFFFFFFFFFFFBFFL
#define DISABLE_ARGUMENT_REFERRER                           0xFFFFFFFFFFFFF7FFL
#define DISABLE_ARGUMENT_TEMPLATENAME                       0xFFFFFFFFFFFFEFFFL
#define DISABLE_ARGUMENT_TEMPLATEREGIONNAME                 0xFFFFFFFFFFFFDFFFL
#define DISABLE_ARGUMENT_VERSION                            0xFFFFFFFFFFFFBFFFL
#define DISABLE_ARGUMENT_VIEWID                             0xFFFFFFFFFFFF7FFFL
#define DISABLE_ARGUMENT_FLAGS                              0xFFFFFFFFFFFEFFFFL
#define DISABLE_ARGUMENT_ASTEMPLATE                         0xFFFFFFFFFFFDFFFFL
#define DISABLE_ARGUMENT_HIDDEN                             0xFFFFFFFFFFFBFFFFL
#define DISABLE_ARGUMENT_OPENNEWVIEW                        0xFFFFFFFFFFF7FFFFL
#define DISABLE_ARGUMENT_READONLY                           0xFFFFFFFFFFEFFFFFL
#define DISABLE_ARGUMENT_PREVIEW                            0xFFFFFFFFFFDFFFFFL
#define DISABLE_ARGUMENT_SILENT                             0xFFFFFFFFFFBFFFFFL
#define DISABLE_ARGUMENT_POSSIZE                            0xFFFFFFFFFF7FFFFFL
#define DISABLE_ARGUMENT_INPUTSTREAM                        0xFFFFFFFFFEFFFFFFL
#define DISABLE_ARGUMENT_DEEPDETECTION                      0xFFFFFFFFFDFFFFFFL
#define DISABLE_ARGUMENT_JUMPMARK                           0xFFFFFFFFFBFFFFFFL
#define DISABLE_ARGUMENT_TYPENAME                           0xFFFFFFFFF7FFFFFFL

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_ARGUMENTS_H_
