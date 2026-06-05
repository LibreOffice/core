/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef MACROS_INC
#define MACROS_INC
/* =====================================================
 * global helper macros
 * =====================================================*/

#define DOSTRING( x )                           #x
#define STRING( x )                             DOSTRING( x )
#define DOCONCAT2( x, y )                       x##y
#define CONCAT2( x, y)                          DOCONCAT2(x,y)
#define DOCONCAT3( x, y, z )                    x##y##z
#define CONCAT3( x, y, z )                      DOCONCAT3(x,y,z)
#define DOCONCAT4( x, y, z, a )                 x##y##z##a
#define CONCAT4( x, y, z, a )                   DOCONCAT4(x,y,z,a)
#define DOCONCAT5( x, y, z, a, b )              x##y##z##a##b
#define CONCAT5( x, y, z, a, b )                DOCONCAT5(x,y,z,a,b)
#define DOCONCAT6( x, y, z, a, b, c )           x##y##z##a##b##c
#define CONCAT6( x, y, z, a, b, c )             DOCONCAT6(x,y,z,a,b,c)
#define DOCONCAT7( x, y, z, a, b, c, u)         x##y##z##a##b##c##u
#define CONCAT7( x, y, z, a, b, c, u)           DOCONCAT7(x,y,z,a,b,c,u)
#define DOCONCAT8( x, y, z, a, b, c, u, v)      x##y##z##a##b##c##u##v
#define CONCAT8( x, y, z, a, b, c, u, v)        DOCONCAT8(x,y,z,a,b,c,u,v)
#define DOCONCAT9( x, y, z, a, b, c, u, v, w)   x##y##z##a##b##c##u##v##w
#define CONCAT9( x, y, z, a, b, c, u, v, w)     DOCONCAT9(x,y,z,a,b,c,u,v,w)

#ifdef UNX
    #ifdef MACOSX
        #define UNXSUFFIX .dylib
    #else
        #define UNXSUFFIX .so
    #endif
#endif

#ifdef UNX
    #define PROFILENAME(name)                 STRING(CONCAT2(name,rc))
#else
    #define PROFILENAME(name)                 STRING(CONCAT2(name,.ini))
#endif

#ifdef MACOSX
    #define GID_BRAND_DIR_ETC gid_Brand_Dir_Share
    #define GID_DIR_PY gid_Brand_Dir_Share
    #define GID_DIR_URE_LIB gid_Dir_Frameworks
#else
    #define GID_BRAND_DIR_ETC gid_Brand_Dir_Program
    #define GID_DIR_PY gid_Brand_Dir_Program
    #define GID_DIR_URE_LIB gid_Brand_Dir_Program
#endif

#define RESFILENAME(name,lang)                STRING(CONCAT3(name,lang,.res))
#define EXTRAFILENAME(name,lang)              STRING(CONCAT3(name,lang,.zip))
#define EXTRAFILELISTNAME(prefix,name,lang)   STRING(CONCAT4(prefix,name,lang,.filelist))
#define CONFIGLANGFILENAME(name,lang,ext)     STRING(CONCAT4(name,lang,.,ext))

#define SCP2_OOO_BIN_DIR gid_Brand_Dir_Program

#ifdef MACOSX
    #define SCP2_OOO_LIB_DIR gid_Dir_Frameworks
#else
    #define SCP2_OOO_LIB_DIR SCP2_OOO_BIN_DIR
#endif

#define BIN_FILE_BODY \
        UnixRights   = 755

#define LIB_FILE_BODY \
        UnixRights   = 755

#define TXT_FILE_BODY \
        UnixRights   = 644

#define USER_FILE_BODY \
        UnixRights   = 644

#define PACKED_TXT_FILE_BODY \
        TXT_FILE_BODY; \
        Styles = (PACKED)

#ifdef WITH_HELPPACK_INTEGRATION
#define FILELIST_TXT_FILE_BODY_HELPPACK \
        TXT_FILE_BODY; \
        Styles = (FILELIST,FORCEHELPPACK,LANGUAGEMODULE)
#else
#define FILELIST_TXT_FILE_BODY_HELPPACK \
        TXT_FILE_BODY; \
        Styles = (FILELIST,HELPPACK,LANGUAGEMODULE)
#endif

#define PACKAGE_FILELIST(id,name) \
File id \
    Dir = FILELIST_DIR; \
    Name = name; \
    Styles = (FILELIST, USE_INTERNAL_RIGHTS, PACKED); \
End

#define PACKAGE_FILELIST_COMPONENTCONDITION(id,name,cond) \
File id \
    Dir = FILELIST_DIR; \
    Name = name; \
    Styles = (FILELIST, USE_INTERNAL_RIGHTS, PACKED); \
    ComponentCondition = cond; \
End

#ifdef WNT
#define PACKAGE_FILELIST_FONT(id,name) \
File id \
    Dir = FILELIST_DIR; \
    Name = name; \
    Styles = (FILELIST, USE_INTERNAL_RIGHTS, PACKED, FONT); \
End
#else
#define PACKAGE_FILELIST_FONT PACKAGE_FILELIST
#endif // WNT

#define PACKED_LIB_FILE_BODY \
        LIB_FILE_BODY; \
        Styles = (PACKED); \
        Dir    = SCP2_OOO_LIB_DIR

#define LIBO_LIB_FILE(id,name) \
    File id \
        Name = name; \
        PACKED_LIB_FILE_BODY; \
    End

#define LIBO_LIB_FILE_BINARYTABLE(id,name) \
    File id \
        Name = name; \
        PACKED_LIB_FILE_BODY; \
        Styles = (PACKED, BINARYTABLE, BINARYTABLE_ONLY); \
    End

#define LIBO_LIB_FILE_COMPONENTCONDITION(id,name,cond) \
    File id \
        Name = name; \
        PACKED_LIB_FILE_BODY; \
        ComponentCondition = cond; \
    End

#define SHLXTHDL_LIB_FILE(id,name) \
    File id \
        Name = name; \
        LIB_FILE_BODY; \
        Styles = (PACKED); \
        Dir    = gid_Dir_Shlxthdl; \
    End

#define LIBO_JAR_FILE(id,name) \
    File id \
        TXT_FILE_BODY; \
        Name = name; \
        Dir = gid_Dir_Classes; \
        Styles = (PACKED); \
    End

#define URE_JAR_FILE(id,name) \
    File id \
        TXT_FILE_BODY; \
        Name = name; \
        Dir = gid_Dir_Classes; \
        Styles = (PACKED); \
    End

#define SCP2_URE_DL_DIR GID_DIR_URE_LIB

#if defined MACOSX
#define FILELIST_DIR PREDEFINED_PROGDIR
#else
#define FILELIST_DIR gid_Dir_Brand_Root
#endif

#define UI_FILELIST(name, file) \
    File CONCAT2(gid_File_Share_Config_Sofficecfg_uiconfig_, name) \
        TXT_FILE_BODY; \
        Styles = (FILELIST); \
        Dir = FILELIST_DIR; \
        Name = file; \
    End

#include <langmacros.inc>

#define MO_FILELIST_ALL_LANG(name) \
    File CONCAT3(gid_File_Res_, name, _Lang) \
        TXT_FILE_BODY; \
        Styles = (PACKED); \
        Dir = gid_Dir_Resource; \
        MO_ALL_LANG_EXCEPT_EN_US_AND_QTZ(name); \
    End

#define URE_PRIVATE_LIB(id,name) \
    File id \
        LIB_FILE_BODY; \
        Dir = SCP2_URE_DL_DIR; \
        Name = name; \
        Styles = (PACKED); \
    End

#define URE_EXECUTABLE(id,name) \
    File id \
        BIN_FILE_BODY; \
        Dir = gid_Brand_Dir_Program; \
        Name = name; \
        Styles = (PACKED); \
    End

#define LIBO_EXECUTABLE(id,name) \
    File id \
        BIN_FILE_BODY; \
        Dir = gid_Brand_Dir_Program; \
        Name = name; \
        Styles = (PACKED); \
    End

#define LIBO_EXECUTABLE_COMPONENTCONDITION(id,name,cond) \
    File id \
        BIN_FILE_BODY; \
        Dir = gid_Brand_Dir_Program; \
        Name = name; \
        Styles = (PACKED); \
        ComponentCondition = cond; \
    End

#define GALLERY_NAME(id) \

#define GALLERY_FILELIST(id) \
File gid_File_Extra_Gall##id \
    Dir = FILELIST_DIR; \
    TXT_FILE_BODY; \
    Styles = (FILELIST, WORKSTATION); \
    Name = STRING(CONCAT3(Gallery/,id,.filelist)); \
End \
File gid_File_Extra_GallFiles##id \
    Dir = FILELIST_DIR; \
    TXT_FILE_BODY; \
    Styles = (FILELIST, WORKSTATION); \
    Name = STRING(CONCAT3(Gallery/Files/,id,.filelist)); \
End

#endif // MACROS_INC
