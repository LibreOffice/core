/*************************************************************************
 *
 *  $RCSfile: filterflags.h,v $
 *
 *  $Revision: 1.2 $
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

#ifndef __FRAMEWORK_FILTERFLAGS_H_
#define __FRAMEWORK_FILTERFLAGS_H_

namespace framework{

/*-************************************************************************************************************//**
    @short          These values describe our supported filter flags.
    @attention      Don't change flag values without reason - we must support old functionality and position
                    in flag combined values!
*//*-*************************************************************************************************************/

#define FILTERFLAGNAME_IMPORT                               DECLARE_ASCII("Import"          )
#define FILTERFLAGNAME_EXPORT                               DECLARE_ASCII("Export"          )
#define FILTERFLAGNAME_INTERNAL                             DECLARE_ASCII("Internal"        )
#define FILTERFLAGNAME_TEMPLATE                             DECLARE_ASCII("Template"        )
#define FILTERFLAGNAME_TEMPLATEPATH                         DECLARE_ASCII("TemplatePath"    )
#define FILTERFLAGNAME_OWN                                  DECLARE_ASCII("Own"             )
#define FILTERFLAGNAME_ALIEN                                DECLARE_ASCII("Alien"           )
#define FILTERFLAGNAME_ASYNCHRON                            DECLARE_ASCII("Asynchron"       )
#define FILTERFLAGNAME_READONLY                             DECLARE_ASCII("Readonly"        )
#define FILTERFLAGNAME_USESOPTIONS                          DECLARE_ASCII("UsesOptions"     )
#define FILTERFLAGNAME_DEFAULT                              DECLARE_ASCII("Default"         )
#define FILTERFLAGNAME_NOTINSTALLED                         DECLARE_ASCII("NotInstalled"    )
#define FILTERFLAGNAME_CONSULTSERVICE                       DECLARE_ASCII("ConsultService"  )
#define FILTERFLAGNAME_3RDPARTYFILTER                       DECLARE_ASCII("3rdPartyFilter"  )
#define FILTERFLAGNAME_NOTINCHOOSER                         DECLARE_ASCII("NotInChooser"    )
#define FILTERFLAGNAME_NOTINFILEDIALOG                      DECLARE_ASCII("NotInFileDialog" )
#define FILTERFLAGNAME_PACKED                               DECLARE_ASCII("Packed"          )
#define FILTERFLAGNAME_SILENTEXPORT                         DECLARE_ASCII("SilentExport"    )
#define FILTERFLAGNAME_BROWSERPREFERED                      DECLARE_ASCII("BrowserPrefered" )
#define FILTERFLAGNAME_PREFERED                             DECLARE_ASCII("Prefered"        )

#define FILTERFLAG_IMPORT                                   0x00000001L     // 1
#define FILTERFLAG_EXPORT                                   0x00000002L     // 2
#define FILTERFLAG_TEMPLATE                                 0x00000004L     // 4
#define FILTERFLAG_INTERNAL                                 0x00000008L     // 8
#define FILTERFLAG_TEMPLATEPATH                             0x00000010L     // 16
#define FILTERFLAG_OWN                                      0x00000020L     // 32
#define FILTERFLAG_ALIEN                                    0x00000040L     // 64
#define FILTERFLAG_USESOPTIONS                              0x00000080L     // 128
#define FILTERFLAG_DEFAULT                                  0x00000100L     // 256
#define FILTERFLAG_NOTINFILEDIALOG                          0x00001000L     // 4096
#define FILTERFLAG_NOTINCHOOSER                             0x00002000L     // 8192
#define FILTERFLAG_ASYNCHRON                                0x00004000L     // 16384
#define FILTERFLAG_READONLY                                 0x00010000L     // 65536
#define FILTERFLAG_NOTINSTALLED                             0x00020000L     // 131072
#define FILTERFLAG_CONSULTSERVICE                           0x00040000L     // 262144
#define FILTERFLAG_3RDPARTYFILTER                           0x00080000L     // 524288
#define FILTERFLAG_PACKED                                   0x00100000L     // 1048576
#define FILTERFLAG_SILENTEXPORT                             0x00200000L     // 2097152
#define FILTERFLAG_BROWSERPREFERED                          0x00400000L     // 4194304
#define FILTERFLAG_PREFERED                                 0x10000000L     // 268435456

#define DISABLE_FILTERFLAG_IMPORT                           0xFFFFFFFEL
#define DISABLE_FILTERFLAG_EXPORT                           0xFFFFFFFDL
#define DISABLE_FILTERFLAG_TEMPLATE                         0xFFFFFFFBL
#define DISABLE_FILTERFLAG_INTERNAL                         0xFFFFFFF7L
#define DISABLE_FILTERFLAG_TEMPLATEPATH                     0xFFFFFFEFL
#define DISABLE_FILTERFLAG_OWN                              0xFFFFFFDFL
#define DISABLE_FILTERFLAG_ALIEN                            0xFFFFFFBFL
#define DISABLE_FILTERFLAG_USESOPTIONS                      0xFFFFFF7FL
#define DISABLE_FILTERFLAG_DEFAULT                          0xFFFFFEFFL
#define DISABLE_FILTERFLAG_NOTINFILEDIALOG                  0xFFFFEFFFL
#define DISABLE_FILTERFLAG_NOTINCHOOSER                     0xFFFFDFFFL
#define DISABLE_FILTERFLAG_ASYNCHRON                        0xFFFFBFFFL
#define DISABLE_FILTERFLAG_READONLY                         0xFFFEFFFFL
#define DISABLE_FILTERFLAG_NOTINSTALLED                     0xFFFDFFFFL
#define DISABLE_FILTERFLAG_CONSULTSERVICE                   0xFFFBFFFFL
#define DISABLE_FILTERFLAG_3RDPARTYFILTER                   0xFFF7FFFFL
#define DISABLE_FILTERFLAG_PACKED                           0xFFEFFFFFL
#define DISABLE_FILTERFLAG_SILENTEXPORT                     0xFFDFFFFFL
#define DISABLE_FILTERFLAG_BROWSERPREFERED                  0xFFBFFFFFL
#define DISABLE_FILTERFLAG_PREFERED                         0xEFFFFFFFL

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_FILTERFLAGS_H_
