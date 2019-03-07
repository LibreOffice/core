/*
 * This file is part of the LibreOffice project.
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

#define MOD_NAME_DESC(id) \
        ALL_LANG(Name,STR_NAME_##id); \
        ALL_LANG(Description,STR_DESC_##id)

#define REG_VALUE_LNG(id) \
        ALL_LANG(Value,STR_REG_VAL_##id);

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
#define READMETXTFILENAME(name,lang,ext)      STRING(CONCAT4(name,lang,.,ext))
#define READMEFILENAME(name,lang)             STRING(CONCAT2(name,lang))
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

#define SDK_PACKAGE_FILELIST(id,name) \
File id \
    Dir = FILELIST_SDK_DIR; \
    Name = name; \
    Styles = (FILELIST, USE_INTERNAL_RIGHTS, PACKED); \
End

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

#define SHLXTHDL_LIB_FILE_COMPONENTCONDITION(id,name,cond) \
    File id \
        Name = name; \
        LIB_FILE_BODY; \
        Styles = (PACKED); \
        Dir    = gid_Dir_Shlxthdl; \
        ComponentCondition = cond; \
    End

#ifdef MACOSX
// links are created at configure time by odk/config/setsdkenv_unix.sh.in
#define SDK_LIBRARY_LINK(id,name,target)
#elif defined(WNT)
#define SDK_LIBRARY_LINK(id,name,target) \
    File id \
        TXT_FILE_BODY; \
        Styles = (PACKED); \
        Dir = gid_Dir_Lib_Sdk; \
        Name = name; \
    End
#else
#define SDK_LIBRARY_LINK(id,name,target) \
    Unixlink id \
     BIN_FILE_BODY; \
     Styles = (); \
     Dir = gid_Dir_Lib_Sdk; \
     Name = name; \
     Target = target; \
    End
#endif

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

#define CONDITIONAL_MODULE_ENTRY(name,modid) \
    Module gid_Module_Root_Reg_##name \
	ParentID = modid; \
	Name = STRING(CONCAT2(gid_Module_Root_,name)); \
	Description = STRING(CONCAT2(gid_Module_Root_,name)); \
	Default = YES; \
	Styles = (HIDDEN_ROOT); \
End

#define REGISTRY_ENTRY_OPEN_WITH(name,cond,doc_type,modid,key) \
    RegistryItem gid_Regitem_OpenOffice_##cond##_OpenWith_##doc_type \
        ParentID = PREDEFINED_HKEY_CLASSES_ROOT; \
        ModuleID = modid; \
        Subkey = STRING(CONCAT3(.,key,\OpenWithProgIDs)); \
        Name = STRING(CONCAT2(LibreOffice.,name)); \
        Value = " "; \
    End

#define REGISTRY_ENTRY_OPEN_WITH_TMPL(name,cond,doc_type,modid,key) \
    RegistryItem gid_Regitem_OpenOffice_##cond##_OpenWith_##doc_type \
        ParentID = PREDEFINED_HKEY_CLASSES_ROOT; \
        ModuleID = modid; \
        Subkey = STRING(CONCAT3(.,key,\OpenWithProgIDs)); \
        Name = STRING(CONCAT2(LibreOffice.,name)); \
        Value = " "; \
    End

#define CONDITIONAL_REGISTRY_ENTRY_EXT(name,cond,key) \
    RegistryItem gid_Regitem__##name \
        ParentID = PREDEFINED_HKEY_CLASSES_ROOT; \
        ModuleID = gid_Module_Root_Reg_##name; \
        Subkey = STRING(CONCAT2(.,key)); \
        Value = STRING(CONCAT2(LibreOffice.,name)); \
        ComponentCondition = STRING(CONCAT3(REGISTER_,cond,=1)); \
    End

#define CONDITIONAL_REGISTRY_ENTRY_EXT_MEDIATYPE(name,cond,key,mediatype) \
    RegistryItem gid_Regitem__MediaType_##name \
        ParentID = PREDEFINED_HKEY_CLASSES_ROOT; \
        ModuleID = gid_Module_Root_Reg_##name; \
        Subkey = STRING(CONCAT2(.,key)); \
        Name = STRING(Content Type); \
        Value = STRING(mediatype); \
        ComponentCondition = STRING(CONCAT3(REGISTER_,cond,=1)); \
    End

#define REGISTRY_ENTRY_DOC(name,modid,disp_name) \
    RegistryItem CONCAT2(gid_Regitem_Openoffice_,name) \
        ParentID = PREDEFINED_HKEY_CLASSES_ROOT; \
        ModuleID = modid; \
        Subkey = STRING(CONCAT2(LibreOffice.,name)); \
        REG_VALUE_LNG(disp_name) \
    End

#define REGISTRY_ENTRY_APPUSERMODELID(name,modid,doc_type) \
    RegistryItem CONCAT3(gid_Regitem_Openoffice_,name,_AppUserModelID) \
        ParentID = PREDEFINED_HKEY_CLASSES_ROOT; \
        ModuleID = modid; \
        Subkey = STRING(CONCAT2(LibreOffice.,name)); \
        Name = STRING(AppUserModelID); \
        Value = STRING(CONCAT2(TheDocumentFoundation.LibreOffice.,doc_type)); \
    End

#define REGISTRY_ENTRY_ICON(name,modid,icon_id) \
    RegistryItem CONCAT3(gid_Regitem_Openoffice_,name,_Defaulticon) \
        ParentID = PREDEFINED_HKEY_CLASSES_ROOT; \
        ModuleID = modid; \
        Subkey = STRING(CONCAT3(LibreOffice.,name,\DefaultIcon)); \
        Value = "<progpath>\program\soffice.bin," STRING(icon_id); \
    End

#define REGISTRY_ENTRY_SHELL(name,default,modid) \
    RegistryItem CONCAT3(gid_Regitem_Openoffice_,name,_Shell) \
        ParentID = PREDEFINED_HKEY_CLASSES_ROOT; \
        ModuleID = modid; \
        Subkey = STRING(CONCAT3(LibreOffice.,name,\shell)); \
        Value = STRING(default); \
    End

#define REGISTRY_ENTRY_NEW_CMD_PPA(name,modid,progpath_program_app) \
    RegistryItem CONCAT3(gid_Regitem_Openoffice_,name,_Shell_New) \
        ParentID = PREDEFINED_HKEY_CLASSES_ROOT; \
        ModuleID = modid; \
        Subkey = STRING(CONCAT3(LibreOffice.,name,\shell\new)); \
        REG_VALUE_LNG(NEW) \
    End \
    RegistryItem CONCAT3(gid_Regitem_Openoffice_,name,_Shell_New_Command) \
        ParentID = PREDEFINED_HKEY_CLASSES_ROOT; \
        ModuleID = modid; \
        Subkey = STRING(CONCAT3(LibreOffice.,name,\shell\new\command)); \
        Value = "\"" STRING(progpath_program_app) "\" -n \"%1\""; \
    End

#define REGISTRY_ENTRY_NEW_CMD(name,modid,app) \
	REGISTRY_ENTRY_NEW_CMD_PPA(name,modid,<progpath>\program\app)

#define REGISTRY_ENTRY_OPEN_CMD_PPA(name,modid,progpath_program_app) \
    RegistryItem CONCAT3(gid_Regitem_Openoffice_,name,_Shell_Open_Command) \
        ParentID = PREDEFINED_HKEY_CLASSES_ROOT; \
        ModuleID = modid; \
        Subkey = STRING(CONCAT3(LibreOffice.,name,\shell\open\command)); \
        Value = "\"" STRING(progpath_program_app) "\" -o \"%1\""; \
    End

#define REGISTRY_ENTRY_OPEN_CMD(name,modid,app) \
	REGISTRY_ENTRY_OPEN_CMD_PPA(name,modid,<progpath>\program\app)

#define REGISTRY_ENTRY_PRINT_CMD_PPA(name,modid,progpath_program_app) \
    RegistryItem CONCAT3(gid_Regitem_Openoffice_,name,_Shell_Print_Command) \
        ParentID = PREDEFINED_HKEY_CLASSES_ROOT; \
        ModuleID = modid; \
        Subkey = STRING(CONCAT3(LibreOffice.,name,\shell\print\command)); \
        Value = "\"" STRING(progpath_program_app) "\" -p \"%1\""; \
    End

#define REGISTRY_ENTRY_PRINT_CMD(name,modid,app) \
	REGISTRY_ENTRY_PRINT_CMD_PPA(name,modid,<progpath>\program\app)

#define REGISTRY_ENTRY_PRINTTO_CMD_PPA(name,modid,progpath_program_app) \
    RegistryItem CONCAT3(gid_Regitem_Openoffice_,name,_Shell_Printto_Command) \
        ParentID = PREDEFINED_HKEY_CLASSES_ROOT; \
        ModuleID = modid; \
        Subkey = STRING(CONCAT3(LibreOffice.,name,\shell\printto\command)); \
        Value = "\"" STRING(progpath_program_app) "\" -pt \"%2\" \"%1\""; \
    End

#define REGISTRY_ENTRY_PRINTTO_CMD(name,modid,app) \
	REGISTRY_ENTRY_PRINTTO_CMD_PPA(name,modid,<progpath>\program\app)

#define REGISTRY_ENTRY_CAPABILITIES(name,modid,key,doc_type) \
    RegistryItem CONCAT2(gid_Regitem_Software_Manufacturer_Productname_Productversion_Capabilities_FileAssociations_,name) \
        ParentID = PREDEFINED_HKEY_LOCAL_MACHINE; \
        ModuleID = modid; \
        Subkey = STRING(Software\%MANUFACTURER\%PRODUCTNAME%PRODUCTADDON\%PRODUCTVERSION\Capabilities\FileAssociations); \
        Name = STRING(CONCAT2(.,key)); \
        Value = STRING(CONCAT2(LibreOffice.,name)); \
        Styles = (); \
    End

#define REGISTRY_ENTRY_TEMPLATE_CAPABILITIES(name,modid,key,doc_type) \
    RegistryItem CONCAT2(gid_Regitem_Software_Manufacturer_Productname_Productversion_Capabilities_FileAssociations_,name) \
        ParentID = PREDEFINED_HKEY_LOCAL_MACHINE; \
        ModuleID = modid; \
        Subkey = STRING(Software\%MANUFACTURER\%PRODUCTNAME%PRODUCTADDON\%PRODUCTVERSION\Capabilities\FileAssociations); \
        Name = STRING(CONCAT2(.,key)); \
        Value = STRING(CONCAT2(LibreOffice.,name)); \
        Styles = (); \
    End

#define REGISTRY_ENTRY_CAPABILITIES_URL(scheme,name,modid,handler) \
    RegistryItem CONCAT2(gid_Regitem_Software_Manufacturer_Productname_Productversion_Capabilities_URLAssociations_,name) \
        ParentID = PREDEFINED_HKEY_LOCAL_MACHINE; \
        ModuleID = modid; \
        Subkey = STRING(Software\%MANUFACTURER\%PRODUCTNAME%PRODUCTADDON\%PRODUCTVERSION\Capabilities\URLAssociations); \
        Name = scheme; \
        Value = handler; \
        Styles = (); \
    End

#define CONDITIONAL_REGISTER_DOC_OPEN_EXTENSION(name,modid,key,cond,disp_name,icon_id,app,default,doc_type) \
    CONDITIONAL_MODULE_ENTRY(name,modid) \
    REGISTRY_ENTRY_OPEN_WITH(name,cond,doc_type,modid,key) \
    CONDITIONAL_REGISTRY_ENTRY_EXT(name,cond,key) \
    REGISTRY_ENTRY_DOC(name,modid,disp_name) \
    REGISTRY_ENTRY_APPUSERMODELID(name,modid,doc_type) \
    REGISTRY_ENTRY_ICON(name,modid,icon_id) \
    REGISTRY_ENTRY_SHELL(name,default,modid) \
    REGISTRY_ENTRY_OPEN_CMD(name,modid,app) \
    REGISTRY_ENTRY_CAPABILITIES(name,modid,key,doc_type) \

#define CONDITIONAL_REGISTER_DOC_EXTENSION(name,modid,key,cond,disp_name,icon_id,app,default,doc_type) \
    CONDITIONAL_REGISTER_DOC_OPEN_EXTENSION(name,modid,key,cond,disp_name,icon_id,app,default,doc_type) \
    REGISTRY_ENTRY_NEW_CMD(name,modid,app) \
    REGISTRY_ENTRY_PRINT_CMD(name,modid,app) \
    REGISTRY_ENTRY_PRINTTO_CMD(name,modid,app) \

#define CONDITIONAL_REGISTER_DOCTEMPLATE_EXTENSION(name,modid,key,cond,disp_name,icon_id,app,default,doc_type) \
    CONDITIONAL_MODULE_ENTRY(name,modid) \
    REGISTRY_ENTRY_OPEN_WITH_TMPL(name,cond,doc_type,modid,key) \
    CONDITIONAL_REGISTRY_ENTRY_EXT(name,cond,key) \
    REGISTRY_ENTRY_DOC(name,modid,disp_name) \
    REGISTRY_ENTRY_APPUSERMODELID(name,modid,doc_type) \
    REGISTRY_ENTRY_ICON(name,modid,icon_id) \
    REGISTRY_ENTRY_SHELL(name,default,modid) \
    REGISTRY_ENTRY_NEW_CMD(name,modid,app) \
    REGISTRY_ENTRY_OPEN_CMD(name,modid,app) \
    REGISTRY_ENTRY_PRINT_CMD(name,modid,app) \
    REGISTRY_ENTRY_PRINTTO_CMD(name,modid,app) \
    REGISTRY_ENTRY_TEMPLATE_CAPABILITIES(name,modid,key,doc_type) \

#ifdef _MSC_VER

#ifdef BUILD_X64

#define REGISTER_PROPERTY_HANDLER_FOR_EXTENSION(extension) \
    RegistryItem CONCAT2(gid_Regitem_Software_Microsoft_Windows_CurrentVersion_PropertySystem_PropertyHandlers_,extension) \
        ParentID = PREDEFINED_HKEY_LOCAL_MACHINE; \
        ModuleID = gid_Module_Optional_Winexplorerext; \
        Subkey = STRING(CONCAT2(SOFTWARE\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers\.,extension)); \
        Value = "{AE424E85-F6DF-4910-A6A9-438797986431}"; \
    End \
\
    RegistryItem CONCAT3(gid_Regitem_Software_Microsoft_Windows_CurrentVersion_PropertySystem_PropertyHandlers_,extension,_x64) \
        ParentID = PREDEFINED_HKEY_LOCAL_MACHINE; \
        ModuleID = gid_Module_Optional_Winexplorerext_x64; \
        ComponentCondition = "VersionNT64"; \
        Subkey = STRING(CONCAT2(SOFTWARE\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers\.,extension)); \
        Value = "{AE424E85-F6DF-4910-A6A9-438797986431}"; \
    End

#else

#define REGISTER_PROPERTY_HANDLER_FOR_EXTENSION(extension) \
    RegistryItem CONCAT2(gid_Regitem_Software_Microsoft_Windows_CurrentVersion_PropertySystem_PropertyHandlers_,extension) \
        ParentID = PREDEFINED_HKEY_LOCAL_MACHINE; \
        ModuleID = gid_Module_Optional_Winexplorerext; \
        Subkey = STRING(CONCAT2(SOFTWARE\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers\.,extension)); \
        Value = "{AE424E85-F6DF-4910-A6A9-438797986431}"; \
    End

#endif

#else
#define REGISTER_PROPERTY_HANDLER_FOR_EXTENSION(extension) // empty
#endif


#define SCP2_URE_DL_DIR GID_DIR_URE_LIB
#if defined WNT
#define SCP2_URE_DL_NORMAL(n) n ".dll"
#define SCP2_URE_DL_VER(n, v) n v ".dll"
#define SCP2_URE_DL_UNO_VER(n, v) n v ".dll"
#define SCP2_URE_DL_UNO_COMID_VER(n, v) n v STRING(COMID) ".dll"
#else
#define SCP2_URE_DL_NORMAL(n) "lib" n STRING(UNXSUFFIX)
#define SCP2_URE_DL_VER(n, v) "lib" n STRING(UNXSUFFIX) "." v
#define SCP2_URE_DL_UNO_VER(n, v) "libuno_" n STRING(UNXSUFFIX) "." v
#define SCP2_URE_DL_UNO_NORMAL(n) "libuno_" n STRING(UNXSUFFIX)
#define SCP2_URE_DL_UNO_COMID_VER(n, v)	\
    "libuno_" n STRING(COMID) STRING(UNXSUFFIX) "." v
#define SCP2_URE_DL_UNO_COMID_NORMAL(n)	\
    "libuno_" n STRING(COMID) STRING(UNXSUFFIX)
#endif
#if defined MACOSX
#define SCP2_URE_JDL_NORMAL(n) "lib" n ".jnilib"
#else
#define SCP2_URE_JDL_NORMAL(n) SCP2_URE_DL_NORMAL(n)
#endif

#if defined MACOSX
#define FILELIST_DIR PREDEFINED_PROGDIR
#define FILELIST_SDK_DIR PREDEFINED_PROGDIR
#else
#define FILELIST_DIR gid_Dir_Brand_Root
#if defined WNT
#define FILELIST_SDK_DIR gid_Dir_Sdkoo_Root
#else
#define FILELIST_SDK_DIR PREDEFINED_PROGDIR
#endif
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

#define SDK_EXECUTABLE(id,name) \
    File id \
        BIN_FILE_BODY; \
        Dir = gid_Dir_Sdkoo_Bin; \
        Name = name; \
        Styles = (PACKED); \
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

#define REGISTER_URI_HANDLER_IMPL(protocol,protid,modid,cond) \
RegistryItem CONCAT2(gid_Regitem_,protid) \
    ParentID = PREDEFINED_HKEY_CLASSES_ROOT; \
    ModuleID = modid; \
    Subkey = STRING(protocol); \
    Value = "URL:" STRING(protocol) " Protocol"; \
    cond \
End \
RegistryItem CONCAT3(gid_Regitem_,protid,_URL_Protocol) \
    ParentID = PREDEFINED_HKEY_CLASSES_ROOT; \
    ModuleID = modid; \
    Subkey = STRING(protocol); \
    Name = "URL Protocol"; \
End \
RegistryItem CONCAT3(gid_Regitem_,protid,_UseOriginalUrlEncoding) \
    ParentID = PREDEFINED_HKEY_CLASSES_ROOT; \
    ModuleID = modid; \
    Subkey = STRING(protocol); \
    Name = "UseOriginalUrlEncoding"; \
    Value = "#1"; \
End \
RegistryItem CONCAT3(gid_Regitem_,protid,_DefaultIcon) \
    ParentID = PREDEFINED_HKEY_CLASSES_ROOT; \
    ModuleID = modid; \
    Subkey = STRING(CONCAT2(protocol,\DefaultIcon)); \
    Value = "<progpath>\program\soffice.bin,0"; \
End \
RegistryItem CONCAT3(gid_Regitem_,protid,_shell_open_command) \
    ParentID = PREDEFINED_HKEY_CLASSES_ROOT; \
    ModuleID = modid; \
    Subkey = STRING(CONCAT2(protocol,\shell\open\command)); \
    Value = "\"<progpath>\program\soffice.exe\" \"%1\""; \
End

#define REG_COND(cond) \
    ComponentCondition = STRING(CONCAT2(cond,=1));

#define REG_EMPTY_COND

#define REGISTER_URI_HANDLER(protocol,protid,modid) \
    REGISTER_URI_HANDLER_IMPL(protocol,protid,modid,REG_EMPTY_COND)

#define CONDITIONAL_REGISTER_URI_HANDLER(protocol,protid,modid,cond) \
    CONDITIONAL_MODULE_ENTRY(protid,modid) \
    REGISTER_URI_HANDLER_IMPL(protocol,protid,gid_Module_Root_Reg_##protid,REG_COND(cond))

#endif // MACROS_INC
