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

#define ALL_LANG_STR_VALUE_LNG(ident) \
        ALL_LANG(Value,STR_VALUE_##ident)

#ifdef UNX
    #ifdef MACOSX
        #define UNXSUFFIX .dylib
    #else
        #define UNXSUFFIX .so
    #endif
#endif

#ifdef UNX
    #define LIBNAME(name)                     STRING(CONCAT4(lib,name,DLLPOSTFIX,UNXSUFFIX))
    #define FILTER_LIBNAME(name)              LIBNAME(name)
    #define EXENAME(name)                     STRING(name)
    #define PROFILENAME(name)                 STRING(CONCAT2(name,rc))
    #define SPECIAL_NAME(name)                STRING(CONCAT3(lib,name,UNXSUFFIX))
    #define SPECIAL_COMPONENT_LIB_NAME(name)  STRING(CONCAT2(name,UNXSUFFIX))
#else
    #define LIBNAME(name)                     STRING(CONCAT3(name,DLLPOSTFIX,.dll))
    #define FILTER_LIBNAME(name)              LIBNAME(name)
    #define EXENAME(name)                     STRING(CONCAT2(name,.exe))
    #define PROFILENAME(name)                 STRING(CONCAT2(name,.ini))
    #define SPECIAL_NAME(name)                STRING(CONCAT2(name,.dll))
    #define SPECIAL_COMPONENT_LIB_NAME(name)  STRING(CONCAT2(name,.dll))
#endif

#define RESFILENAME(name,lang)                STRING(CONCAT3(name,lang,.res))
#define READMETXTFILENAME(name,lang,ext)      STRING(CONCAT4(name,lang,.,ext))
#define READMEFILENAME(name,lang)             STRING(CONCAT2(name,lang))
#define EXTRAFILENAME(name,lang)              STRING(CONCAT3(name,lang,.zip))
#define CONFIGLANGFILENAME(name,lang,ext)     STRING(CONCAT4(name,lang,.,ext))

#define SCP2_OOO_BIN_DIR gid_Brand_Dir_Program

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

#define ARCHIVE_TXT_FILE_BODY \
        TXT_FILE_BODY; \
        Styles = (ARCHIVE)

#ifdef WITH_HELPPACK_INTEGRATION
#define ARCHIVE_TXT_FILE_BODY_HELPPACK \
        TXT_FILE_BODY; \
        Styles = (ARCHIVE,FORCEHELPPACK,LANGUAGEMODULE)
#else
#define ARCHIVE_TXT_FILE_BODY_HELPPACK \
        TXT_FILE_BODY; \
        Styles = (ARCHIVE,HELPPACK,LANGUAGEMODULE)
#endif

#define PACKED_LIB_FILE_BODY \
        LIB_FILE_BODY; \
        Styles = (PACKED); \
        Dir    = SCP2_OOO_BIN_DIR

#define PACKED_LIB_FILE_BODY_PATCH \
        LIB_FILE_BODY; \
        Styles = (PACKED,PATCH); \
        Dir    = SCP2_OOO_BIN_DIR

#define STD_LIB_FILE(id,name) \
    File id \
        Name   = LIBNAME(name); \
        PACKED_LIB_FILE_BODY; \
    End

#define SPECIAL_LIB_FILE(id,name) \
    File id \
        Name    = SPECIAL_NAME(name); \
        PACKED_LIB_FILE_BODY; \
    End

#define SPECIAL_LIB_FILE_PATCH(id,name) \
    File id \
        Name    = SPECIAL_NAME(name); \
        PACKED_LIB_FILE_BODY_PATCH; \
    End
 
#define STD_RES_FILE(id,name) \
    File id \
        TXT_FILE_BODY; \
        RESFILE_ALL_LANG(name); \
        Dir     = gid_Dir_Resource; \
        Styles  = (PACKED); \
    End

#define BINFILTER_RES_FILE(id,name) \
    File id \
        TXT_FILE_BODY; \
        Name    = STRING(CONCAT3(name,en-US,.res)); \
        Dir     = gid_Dir_Resource; \
        Styles  = (PACKED); \
    End

#define STD_FILTER_FILE(id,name) \
    File id \
        PACKED_LIB_FILE_BODY; \
        Name = FILTER_LIBNAME(name); \
    End

#define STD_FONTWIN_FILE(id,name,fontname) \
    File id \
        TXT_FILE_BODY; \
        Dir      = PREDEFINED_OSSYSTEMFONTDIR; \
        Styles   = (PACKED,FONT,DONT_DELETE,FONT_WARN_IF_EXISTS); \
        Name     = STRING(name) ; \
        FontName = STRING(fontname) ; \
        NetDir   = gid_Dir_Winfonts; \
    End

#ifdef UNX
#define STD_FONT_FILE(id,name,fontname) \
    File id \
        TXT_FILE_BODY; \
        Dir       = gid_Dir_Fonts_Truetype; \
        Styles    = (PACKED, FONT_WARN_IF_EXISTS); \
        Name      = STRING(name) ; \
    End
#endif

#ifdef UNX
#define STD_FONT_FILE_DUO(id,name,unxfontname,winfontname) \
    File id \
        TXT_FILE_BODY; \
        Dir       = gid_Dir_Fonts_Truetype; \
        Styles    = (PACKED, FONT_WARN_IF_EXISTS); \
        Name      = STRING(name) ; \
    End
#endif

#ifdef WNT
#define STD_FONT_FILE(id,name,fontname) \
    File id \
        TXT_FILE_BODY; \
        Dir      = PREDEFINED_OSSYSTEMFONTDIR; \
        Styles   = (PACKED,FONT,DONT_DELETE,FONT_WARN_IF_EXISTS); \
        Name     = STRING(name) ; \
        FontName = STRING(fontname) ; \
        NetDir   = gid_Dir_Winfonts; \
    End
#endif

#ifdef WNT
#define STD_FONT_FILE_DUO(id,name,unxfontname,winfontname) \
    File id \
        TXT_FILE_BODY; \
        Dir      = PREDEFINED_OSSYSTEMFONTDIR; \
        Styles   = (PACKED,FONT,DONT_DELETE,FONT_WARN_IF_EXISTS); \
        Name     = STRING(name) ; \
        FontName = STRING(winfontname) ; \
        NetDir   = gid_Dir_Winfonts; \
    End
#endif

#define STD_JAR_FILE(id,name) \
    File id \
        TXT_FILE_BODY; \
        Name     = STRING(CONCAT2(name,.jar)); \
        Dir      = gid_Dir_Classes; \
        Styles   = (PACKED,DONT_OVERWRITE); \
    End

#define JARFILENAME(name) STRING(CONCAT2(name,.jar))

#define PACKED_SHELLNEW_FILE_BODY \
        TXT_FILE_BODY; \
        Styles = (PACKED,WORKSTATION, OVERWRITE); \
        Dir    = PREDEFINED_OSSHELLNEWDIR; \
        NetDir = gid_Dir_Shellnew

#define STD_SHELLNEW_FILE(id,name) \
    File id \
        PACKED_SHELLNEW_FILE_BODY; \
        Name      = STRING(name) ; \
    End

#define SPECIAL_COMPONENT_LIB_FILE(id,name) \
    File id \
        Name     = SPECIAL_COMPONENT_LIB_NAME(name); \
        PACKED_LIB_FILE_BODY; \
    End

// ---------------------------------------
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

#define REGISTRY_ENTRY_DOC(name,modid,disp_name) \
    RegistryItem CONCAT2(gid_Regitem_Openoffice_,name) \
        ParentID = PREDEFINED_HKEY_CLASSES_ROOT; \
        ModuleID = modid; \
        Subkey = STRING(CONCAT2(LibreOffice.,name)); \
        REG_VALUE_LNG(disp_name) \
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

#define REGISTRY_ENTRY_NEW_CMD(name,modid,app) \
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
        Value = "\"<progpath>\program\\" STRING(app) "\" -n \"%1\""; \
    End

#define REGISTRY_ENTRY_OPEN_CMD(name,modid,app) \
    RegistryItem CONCAT3(gid_Regitem_Openoffice_,name,_Shell_Open_Command) \
        ParentID = PREDEFINED_HKEY_CLASSES_ROOT; \
        ModuleID = modid; \
        Subkey = STRING(CONCAT3(LibreOffice.,name,\shell\open\command)); \
        Value = "\"<progpath>\program\\" STRING(app) "\" -o \"%1\""; \
    End

#define REGISTRY_ENTRY_PRINT_CMD(name,modid,app) \
    RegistryItem CONCAT3(gid_Regitem_Openoffice_,name,_Shell_Print_Command) \
        ParentID = PREDEFINED_HKEY_CLASSES_ROOT; \
        ModuleID = modid; \
        Subkey = STRING(CONCAT3(LibreOffice.,name,\shell\print\command)); \
        Value = "\"<progpath>\program\\" STRING(app) "\" -p \"%1\""; \
    End

#define REGISTRY_ENTRY_PRINTTO_CMD(name,modid,app) \
    RegistryItem CONCAT3(gid_Regitem_Openoffice_,name,_Shell_Printto_Command) \
        ParentID = PREDEFINED_HKEY_CLASSES_ROOT; \
        ModuleID = modid; \
        Subkey = STRING(CONCAT3(LibreOffice.,name,\shell\printto\command)); \
        Value = "\"<progpath>\program\\" STRING(app) "\" -pt \"%2\" \"%1\""; \
    End

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
    
#define REGISTRY_ENTRY_KINDMAP(name, modid, key) \
    RegistryItem CONCAT2(gid_Regitem_Software_Manufacturer_Productname_Productversion_Explorer_KindMap_,name) \
        ParentID = PREDEFINED_HKEY_LOCAL_MACHINE; \
        ModuleID = modid; \
        ComponentCondition = "VersionNT >= 600"; \
        Subkey = STRING(SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\KindMap); \
        Name = STRING(CONCAT2(.,key)); \
        Value = "document"; \
    End

#define CONDITIONAL_REGISTER_DOC_EXTENSION(name,modid,key,cond,disp_name,icon_id,app,default,doc_type) \
    CONDITIONAL_MODULE_ENTRY(name,modid) \
    REGISTRY_ENTRY_OPEN_WITH(name,cond,doc_type,modid,key) \
    CONDITIONAL_REGISTRY_ENTRY_EXT(name,cond,key) \
    REGISTRY_ENTRY_DOC(name,modid,disp_name) \
    REGISTRY_ENTRY_ICON(name,modid,icon_id) \
    REGISTRY_ENTRY_SHELL(name,default,modid) \
    REGISTRY_ENTRY_NEW_CMD(name,modid,app) \
    REGISTRY_ENTRY_OPEN_CMD(name,modid,app) \
    REGISTRY_ENTRY_PRINT_CMD(name,modid,app) \
    REGISTRY_ENTRY_PRINTTO_CMD(name,modid,app) \
    REGISTRY_ENTRY_CAPABILITIES(name,modid,key,doc_type) \
    REGISTRY_ENTRY_KINDMAP(name, modid, key) \

#define CONDITIONAL_REGISTER_DOCTEMPLATE_EXTENSION(name,modid,key,cond,disp_name,icon_id,app,default,doc_type) \
    CONDITIONAL_MODULE_ENTRY(name,modid) \
    REGISTRY_ENTRY_OPEN_WITH_TMPL(name,cond,doc_type,modid,key) \
    CONDITIONAL_REGISTRY_ENTRY_EXT(name,cond,key) \
    REGISTRY_ENTRY_DOC(name,modid,disp_name) \
    REGISTRY_ENTRY_ICON(name,modid,icon_id) \
    REGISTRY_ENTRY_SHELL(name,default,modid) \
    REGISTRY_ENTRY_NEW_CMD(name,modid,app) \
    REGISTRY_ENTRY_OPEN_CMD(name,modid,app) \
    REGISTRY_ENTRY_PRINT_CMD(name,modid,app) \
    REGISTRY_ENTRY_PRINTTO_CMD(name,modid,app) \
    REGISTRY_ENTRY_TEMPLATE_CAPABILITIES(name,modid,key,doc_type) \
    REGISTRY_ENTRY_KINDMAP(name, modid, key) \

// ---------------------------------------

#ifdef _MSC_VER

#define REGISTER_PROPERTY_HANDLER_FOR_EXTENSION(extension) \
    RegistryItem CONCAT2(gid_Regitem_Software_Microsoft_Windows_CurrentVersion_PropertySystem_PropertyHandlers_,extension) \
        ParentID = PREDEFINED_HKEY_LOCAL_MACHINE; \
        ModuleID = gid_Module_Optional_Winexplorerext; \
        ComponentCondition = "VersionNT >= 600"; \
        Subkey = STRING(CONCAT2(SOFTWARE\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers\.,extension)); \
        Value = "{AE424E85-F6DF-4910-A6A9-438797986431}"; \
    End \
\
    RegistryItem CONCAT3(gid_Regitem_Software_Microsoft_Windows_CurrentVersion_PropertySystem_PropertyHandlers_,extension,_x64) \
        ParentID = PREDEFINED_HKEY_LOCAL_MACHINE; \
        ModuleID = gid_Module_Optional_Winexplorerext_x64; \
        ComponentCondition = "VersionNT64 >= 600"; \
        Subkey = STRING(CONCAT2(SOFTWARE\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers\.,extension)); \
        Value = "{AE424E85-F6DF-4910-A6A9-438797986431}"; \
    End

#else
#define REGISTER_PROPERTY_HANDLER_FOR_EXTENSION(extension) // empty
#endif

// ---------------------------------------

#if defined WNT
#define SCP2_URE_DL_DIR gid_Dir_Ure_Bin
#define SCP2_URE_DL_NORMAL(n) n ".dll"
#define SCP2_URE_DL_BARE(n) n ".dll"
#define SCP2_URE_DL_VER(n, v) n v ".dll"
#define SCP2_URE_DL_COMID_VER(n, v) n v STRING(COMID) ".dll"
#define SCP2_URE_DL_COMID_NORMAL(n) n STRING(COMID) ".dll"
#define SCP2_URE_DL_UNO_VER(n, v) n v ".dll"
#define SCP2_URE_DL_UNO_COMID_VER(n, v) n v STRING(COMID) ".dll"
#define SCP2_URE_SHARE_DIR gid_Dir_Common_Ure
#if defined _MSC_VER
#if defined _DEBUG_RUNTIME
#define SCP2_STDLP_DEBUG "d"
#else
#define SCP2_STDLP_DEBUG ""
#endif
#else // GCC
#define SCP2_STDLP_DEBUG ""
#endif
#else
#define SCP2_URE_DL_DIR gid_Dir_Ure_Lib
#define SCP2_URE_DL_NORMAL(n) "lib" n STRING(UNXSUFFIX)
#define SCP2_URE_DL_BARE(n) n STRING(UNXSUFFIX)
#define SCP2_URE_DL_VER(n, v) "lib" n STRING(UNXSUFFIX) "." v
#define SCP2_URE_DL_COMID_VER(n, v) \
    "lib" n STRING(COMID) STRING(UNXSUFFIX) "." v
#define SCP2_URE_DL_COMID_NORMAL(n) \
    "lib" n STRING(COMID) STRING(UNXSUFFIX)
#define SCP2_URE_DL_UNO_VER(n, v) "libuno_" n STRING(UNXSUFFIX) "." v
#define SCP2_URE_DL_UNO_NORMAL(n) "libuno_" n STRING(UNXSUFFIX)
#define SCP2_URE_DL_UNO_COMID_VER(n, v)	\
    "libuno_" n STRING(COMID) STRING(UNXSUFFIX) "." v
#define SCP2_URE_DL_UNO_COMID_NORMAL(n)	\
    "libuno_" n STRING(COMID) STRING(UNXSUFFIX)
#define SCP2_URE_SHARE_DIR gid_Dir_Ure_Share
#endif
#if defined MACOSX
#define SCP2_URE_JDL_NORMAL(n) "lib" n ".jnilib"
#else
#define SCP2_URE_JDL_NORMAL(n) SCP2_URE_DL_NORMAL(n)
#endif

#include <langmacros.inc>


#endif // MACROS_INC

