/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: filterflags.h,v $
 * $Revision: 1.5 $
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

#ifndef __FRAMEWORK_FILTERFLAGS_H_
#define __FRAMEWORK_FILTERFLAGS_H_

namespace framework{

/*-************************************************************************************************************//**
    @short          These values describe our supported filter flags.
    @attention      Don't change flag values without reason - we must support old functionality and position
                    in flag combined values!
*//*-*************************************************************************************************************/

#define FILTERFLAGNAME_IMPORT                               DECLARE_ASCII("Import"           )  // x
#define FILTERFLAGNAME_EXPORT                               DECLARE_ASCII("Export"           )  // x
#define FILTERFLAGNAME_TEMPLATE                             DECLARE_ASCII("Template"         )  // x
#define FILTERFLAGNAME_INTERNAL                             DECLARE_ASCII("Internal"         )  // x
#define FILTERFLAGNAME_TEMPLATEPATH                         DECLARE_ASCII("TemplatePath"     )  // x
#define FILTERFLAGNAME_OWN                                  DECLARE_ASCII("Own"              )  // x
#define FILTERFLAGNAME_ALIEN                                DECLARE_ASCII("Alien"            )  // x
#define FILTERFLAGNAME_USESOPTIONS                          DECLARE_ASCII("UsesOptions"      )  // x
#define FILTERFLAGNAME_DEFAULT                              DECLARE_ASCII("Default"          )  // x
#define FILTERFLAGNAME_EXECUTABLE                           DECLARE_ASCII("Executable"       )  // deprecated
#define FILTERFLAGNAME_SUPPORTSSELECTION                    DECLARE_ASCII("SupportsSelection")  // x
#define FILTERFLAGNAME_MAPTOAPPPLUG                         DECLARE_ASCII("MapToAppPlug"     )  // deprecated
#define FILTERFLAGNAME_NOTINFILEDIALOG                      DECLARE_ASCII("NotInFileDialog"  )  // x
#define FILTERFLAGNAME_NOTINCHOOSER                         DECLARE_ASCII("NotInChooser"     )  // x
#define FILTERFLAGNAME_ASYNCHRON                            DECLARE_ASCII("Asynchron"        )  // x
#define FILTERFLAGNAME_CREATOR                              DECLARE_ASCII("Creator"          )  // deprecated
#define FILTERFLAGNAME_READONLY                             DECLARE_ASCII("Readonly"         )  // x
#define FILTERFLAGNAME_NOTINSTALLED                         DECLARE_ASCII("NotInstalled"     )  // deprecated
#define FILTERFLAGNAME_CONSULTSERVICE                       DECLARE_ASCII("ConsultService"   )  // deprecated
#define FILTERFLAGNAME_3RDPARTYFILTER                       DECLARE_ASCII("3rdPartyFilter"   )  // x
#define FILTERFLAGNAME_PACKED                               DECLARE_ASCII("Packed"           )  // x
#define FILTERFLAGNAME_SILENTEXPORT                         DECLARE_ASCII("SilentExport"     )  // x
#define FILTERFLAGNAME_BROWSERPREFERED                      DECLARE_ASCII("BrowserPrefered"  )  // deprecated
#define FILTERFLAGNAME_PREFERED                             DECLARE_ASCII("Prefered"         )  // x

#define FILTERFLAG_IMPORT                                   0x00000001L     // 1
#define FILTERFLAG_EXPORT                                   0x00000002L     // 2
#define FILTERFLAG_TEMPLATE                                 0x00000004L     // 4
#define FILTERFLAG_INTERNAL                                 0x00000008L     // 8
#define FILTERFLAG_TEMPLATEPATH                             0x00000010L     // 16
#define FILTERFLAG_OWN                                      0x00000020L     // 32
#define FILTERFLAG_ALIEN                                    0x00000040L     // 64
#define FILTERFLAG_USESOPTIONS                              0x00000080L     // 128
#define FILTERFLAG_DEFAULT                                  0x00000100L     // 256
#define FILTERFLAG_EXECUTABLE                               0x00000200L     // 512
#define FILTERFLAG_SUPPORTSSELECTION                        0x00000400L     // 1024
#define FILTERFLAG_MAPTOAPPPLUG                             0x00000800L     // 2048
#define FILTERFLAG_NOTINFILEDIALOG                          0x00001000L     // 4096
#define FILTERFLAG_NOTINCHOOSER                             0x00002000L     // 8192
#define FILTERFLAG_ASYNCHRON                                0x00004000L     // 16384
#define FILTERFLAG_CREATOR                                  0x00008000L     // 32768
#define FILTERFLAG_READONLY                                 0x00010000L     // 65536
#define FILTERFLAG_NOTINSTALLED                             0x00020000L     // 131072
#define FILTERFLAG_CONSULTSERVICE                           0x00040000L     // 262144
#define FILTERFLAG_3RDPARTYFILTER                           0x00080000L     // 524288
#define FILTERFLAG_PACKED                                   0x00100000L     // 1048576
#define FILTERFLAG_SILENTEXPORT                             0x00200000L     // 2097152
#define FILTERFLAG_BROWSERPREFERED                          0x00400000L     // 4194304
//FREE! ... 0x00800000L
#define FILTERFLAG_PREFERED                                 0x10000000L     // 268435456

class FlagCheck
{
    public:

    //___________________________________________

    /** @short  checks if the given flag mask is set.

        @param  nFlags  the flag field, which should be checked.
        @param  nMask   this mask field is searched inside parameter nFlags.

        @return TRUE if mask match to the given flag field.
     */
    static sal_Bool isMaskSet( sal_Int32 nFlags, sal_Int32 nMask )
    {
        return((nFlags & nMask) == nMask);
    }

    //___________________________________________

    /** @short  checks if the given flag field contains unknown flags.

        @descr  Of course it can work only, if not the whole range of an int32
                is used!

        @param  nFlags  the flag field, which should be checked.

        @return TRUE if only well known flags are set.
     */
    static sal_Bool isValid( sal_Int32 nFlags )
    {
        sal_Int32 nCheck = nFlags;
        nCheck &= ~FILTERFLAG_IMPORT;
        nCheck &= ~FILTERFLAG_EXPORT;
        nCheck &= ~FILTERFLAG_TEMPLATE;
        nCheck &= ~FILTERFLAG_INTERNAL;
        nCheck &= ~FILTERFLAG_TEMPLATEPATH;
        nCheck &= ~FILTERFLAG_OWN;
        nCheck &= ~FILTERFLAG_ALIEN;
        nCheck &= ~FILTERFLAG_USESOPTIONS;
        nCheck &= ~FILTERFLAG_DEFAULT;
        nCheck &= ~FILTERFLAG_EXECUTABLE;
        nCheck &= ~FILTERFLAG_SUPPORTSSELECTION;
        nCheck &= ~FILTERFLAG_MAPTOAPPPLUG;
        nCheck &= ~FILTERFLAG_NOTINFILEDIALOG;
        nCheck &= ~FILTERFLAG_NOTINCHOOSER;
        nCheck &= ~FILTERFLAG_ASYNCHRON;
        nCheck &= ~FILTERFLAG_CREATOR;
        nCheck &= ~FILTERFLAG_READONLY;
        nCheck &= ~FILTERFLAG_NOTINSTALLED;
        nCheck &= ~FILTERFLAG_CONSULTSERVICE;
        nCheck &= ~FILTERFLAG_3RDPARTYFILTER;
        nCheck &= ~FILTERFLAG_PACKED;
        nCheck &= ~FILTERFLAG_SILENTEXPORT;
        nCheck &= ~FILTERFLAG_BROWSERPREFERED;
        nCheck &= ~FILTERFLAG_PREFERED;
        return(nCheck == 0);
    }

    //___________________________________________

    /** @short  checks if the given flag field uses deprecated flag values.

        @param  nFlags  the flag field, which should be checked.

        @return TRUE if one deprecated flag item could be found.
     */
    static sal_Bool useDeprecated( sal_Int32 nFlags )
    {
        return(
                isMaskSet(nFlags, FILTERFLAG_EXECUTABLE       ) ||
                isMaskSet(nFlags, FILTERFLAG_MAPTOAPPPLUG     ) ||
                isMaskSet(nFlags, FILTERFLAG_CREATOR          ) ||
                isMaskSet(nFlags, FILTERFLAG_NOTINSTALLED     ) ||
                isMaskSet(nFlags, FILTERFLAG_CONSULTSERVICE   ) ||
                isMaskSet(nFlags, FILTERFLAG_BROWSERPREFERED  )
              );
    }
};

} // namespace framework

#endif // #ifndef __FRAMEWORK_FILTERFLAGS_H_
