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

#define CURL_MAJOR 3
#define CURL_MINOR 0
#define CURL_MICRO 0

#define MOD_NAME_DESC(id) \
        ALL_LANG(Name,STR_NAME_##id); \
        ALL_LANG(Description,STR_DESC_##id)

#define MOD_NAME_DESC_ON_DESELECT(id) \
        MOD_NAME_DESC(id); \
        ALL_LANG(OnDeselect,STR_DESELECT_##id)

#define REG_VALUE_LNG(id) \
        ALL_LANG(Value,STR_REG_VAL_##id);

#define FOLDERITEM_NAME_LNG(id) \
        ALL_LANG(Name,STR_FI_NAME_##id);

#define ALL_LANG_STR_KEY(ident) \
        ALL_LANG(Key,ident)

#define ALL_LANG_STR_KEY_LNG(ident) \
        ALL_LANG(Key,STR_KEY_##ident)

#define ALL_LANG_STR_VALUE_LNG(ident) \
        ALL_LANG(Value,STR_VALUE_##ident)

#define ALL_LANG_VALUE(vident) \
        ALL_LANG(Value,vident)

#define ALL_LANG_KEY(vident) \
        ALL_LANG(Key,vident)


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
#define SHORTRESFILENAME(name,lang)           STRING(CONCAT3(name,lang,.res))
#define READMETXTFILENAME(name,lang,ext)      STRING(CONCAT4(name,lang,.,ext))
#define READMEFILENAME(name,lang)             STRING(CONCAT2(name,lang))
#define HELPFILENAME(name,endung)             STRING(CONCAT3(name,.,endung))
#define EXTRAFILENAME(name,lang)              STRING(CONCAT3(name,lang,.zip))
#define CONFIGLANGFILENAME(name,lang,ext)     STRING(CONCAT4(name,lang,.,ext))

#define BIN_FILE_BODY \
        UnixRights   = 555

#define TXT_FILE_BODY \
        UnixRights   = 444

#define USER_FILE_BODY \
        UnixRights   = 644

#define PACKED_TXT_FILE_BODY \
        TXT_FILE_BODY; \
        Styles = (PACKED)

#define PACKED_TXT_FILE_BODY_PATCH \
        TXT_FILE_BODY; \
        Styles = (PACKED,PATCH)

#define ARCHIVE_TXT_FILE_BODY \
        TXT_FILE_BODY; \
        Styles = (ARCHIVE)

#define ARCHIVE_TXT_FILE_BODY_PATCH \
        TXT_FILE_BODY; \
        Styles = (ARCHIVE,PATCH)

#define ARCHIVE_SUBST_FILE_BODY \
        TXT_FILE_BODY; \
        Styles = (ARCHIVE,SUBSTITUTE)

#define ARCHIVE_SUBST_FILE_BODY_PATCH \
        TXT_FILE_BODY; \
        Styles = (ARCHIVE,SUBSTITUTE,PATCH)

#define PACKED_LIB_FILE_BODY \
        TXT_FILE_BODY; \
        Styles = (PACKED); \
        Dir    = gid_Dir_Program

#define PACKED_LIB_FILE_BODY_PATCH \
        TXT_FILE_BODY; \
        Styles = (PACKED,PATCH); \
        Dir    = gid_Dir_Program

#define PACKED_FILTER_FILE_BODY \
        TXT_FILE_BODY; \
        Styles = (PACKED); \
        Dir    = gid_Dir_Filter

#define PACKED_FILTER_FILE_BODY_PATCH \
        TXT_FILE_BODY; \
        Styles = (PACKED,PATCH); \
        Dir    = gid_Dir_Filter

#define STD_LIB_FILE(id,name) \
    File id \
        Name   = LIBNAME(name); \
        PACKED_LIB_FILE_BODY; \
    End

#define STD_LIB_FILE_PATCH(id,name) \
    File id \
        Name   = LIBNAME(name); \
        PACKED_LIB_FILE_BODY_PATCH; \
    End

#define STD_RES_FILE(id,name) \
    File id \
        TXT_FILE_BODY; \
        RESFILE_ALL_LANG(name); \
        Dir     = gid_Dir_Resource; \
        Styles  = (PACKED); \
    End

#define BRAND_RES_FILE(id,name) \
    File id \
        TXT_FILE_BODY; \
        RESFILE_ALL_LANG(name); \
        Dir     = gid_Brand_Dir_Resource; \
        Styles  = (PACKED); \
    End

#define STD_RES_FILE_PATCH(id,name) \
    File id \
        TXT_FILE_BODY; \
        RESFILE_ALL_LANG(name); \
        Dir     = gid_Dir_Resource; \
        Styles  = (PACKED,PATCH); \
    End

#define STD_RES_FILE_ONLY_PATCH(id,name) \
    File id \
        TXT_FILE_BODY; \
        RESFILE_ALL_LANG(name); \
        Dir     = gid_Dir_Resource; \
        Styles  = (PACKED,PATCH,PATCH_ONLY); \
    End

#define STD_FILTER_FILE(id,name) \
    File id \
        PACKED_LIB_FILE_BODY; \
        Name = FILTER_LIBNAME(name); \
    End

#define STD_FILTER_FILE_PATCH(id,name) \
    File id \
        PACKED_LIB_FILE_BODY_PATCH; \
        Name = FILTER_LIBNAME(name); \
    End

#define STD_FONTUNX_FILE(id,name,fontname) \
    File id \
        TXT_FILE_BODY; \
        Dir       = gid_Dir_Fonts_Truetype; \
        Styles    = (PACKED, FONT_WARN_IF_EXISTS); \
        Name      = STRING(name) ; \
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

#define STD_JAR_FILE_PATCH(id,name) \
    File id \
        TXT_FILE_BODY; \
        Name     = STRING(CONCAT2(name,.jar)); \
        Dir      = gid_Dir_Classes; \
        Styles   = (PACKED,DONT_OVERWRITE,PATCH); \
    End

#define UNO_JAR_FILE(id,name) \
    File id \
        TXT_FILE_BODY; \
        Name     = STRING(CONCAT2(name,.jar)); \
        RegistryID = gid_Starregistry_Services_Rdb; \
        Dir      = gid_Dir_Classes; \
        Styles   = (PACKED,DONT_OVERWRITE, UNO_COMPONENT); \
    End

#define UNO_JAR_FILE_PATCH(id,name) \
    File id \
        TXT_FILE_BODY; \
        Name     = STRING(CONCAT2(name,.jar)); \
        RegistryID = gid_Starregistry_Services_Rdb; \
        Dir      = gid_Dir_Classes; \
        Styles   = (PACKED,DONT_OVERWRITE,UNO_COMPONENT,PATCH); \
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

#define PACKED_UNO_LIB_FILE_BODY \
        TXT_FILE_BODY; \
        Styles = (PACKED,UNO_COMPONENT); \
        Dir    = gid_Dir_Program; \
        RegistryID = gid_Starregistry_Services_Rdb

#define PACKED_UNO_LIB_FILE_BODY_PATCH \
        TXT_FILE_BODY; \
        Styles = (PACKED,UNO_COMPONENT,PATCH); \
        Dir    = gid_Dir_Program; \
        RegistryID = gid_Starregistry_Services_Rdb

#define STD_UNO_LIB_FILE(id,name) \
    File id \
        Name    = LIBNAME(name); \
        PACKED_UNO_LIB_FILE_BODY; \
    End

#define STD_UNO_LIB_FILE_PATCH(id,name) \
    File id \
        Name    = LIBNAME(name); \
        PACKED_UNO_LIB_FILE_BODY_PATCH; \
    End

#define SPECIAL_UNO_LIB_FILE(id,name) \
    File id \
        Name    = SPECIAL_NAME(name); \
        PACKED_UNO_LIB_FILE_BODY; \
    End

#define SPECIAL_UNO_LIB_FILE_PATCH(id,name) \
    File id \
        Name    = SPECIAL_NAME(name); \
        PACKED_UNO_LIB_FILE_BODY_PATCH; \
    End

#define SPECIAL_UNO_COMPONENT_LIB_FILE(id,name) \
    File id \
        Name     = SPECIAL_COMPONENT_LIB_NAME(name); \
        PACKED_UNO_LIB_FILE_BODY; \
    End

#define SPECIAL_UNO_COMPONENT_LIB_FILE_PATCH(id,name) \
    File id \
        Name     = SPECIAL_COMPONENT_LIB_NAME(name); \
        PACKED_UNO_LIB_FILE_BODY_PATCH; \
    End

#define WINW4WFILTERFILENAME(name) STRING(CONCAT2(name,f32w.dll))

#define STD_WIN_W4WFILTER_FILE(id,name) \
    File id \
        PACKED_FILTER_FILE_BODY; \
        Name            = WINW4WFILTERFILENAME(name);  \
    End

#define SOLSW4WFILTERFILENAME(name) STRING(CONCAT2(w4w, name))

#define STD_SOLS_W4WFILTER_FILE(id,name) \
    File id \
        BIN_FILE_BODY; \
        Styles = (PACKED); \
        Dir = gid_Dir_Filter; \
        Name = SOLSW4WFILTERFILENAME(name); \
    End

#define SPECIAL_NO_WARNING_IF_NOT_EXISTS_FILE(id,name) \
    File id \
        TXT_FILE_BODY; \
        Name = LIBNAME(name); \
        Dir = gid_Dir_Program; \
        Styles = (PACKED, NO_WARNING_IF_NOT_EXISTS); \
    End

#define SPECIAL_UNO_NO_WARNING_IF_NOT_EXISTS_FILE(id,name) \
    File id \
        TXT_FILE_BODY; \
        Name = LIBNAME(name); \
        Dir = gid_Dir_Program; \
        Styles = (PACKED, NO_WARNING_IF_NOT_EXISTS, UNO_COMPONENT); \
        RegistryID = gid_Starregistry_Services_Rdb; \
    End

#if defined WNT
#define SCP2_URE_DL_DIR gid_Dir_Ure_Bin
#define SCP2_URE_DL_NORMAL(n) n ".dll"
#define SCP2_URE_DL_BARE(n) n ".dll"
#define SCP2_URE_DL_VER(n, v) n v ".dll"
#define SCP2_URE_DL_COMID_VER(n, v) n v STRING(COMID) ".dll"
#define SCP2_URE_DL_UNO_VER(n, v) n v ".dll"
#define SCP2_URE_DL_UNO_COMID_VER(n, v) n v STRING(COMID) ".dll"
#define SCP2_URE_SHARE_DIR gid_Dir_Common_Ure
#else
#define SCP2_URE_DL_DIR gid_Dir_Ure_Lib
#define SCP2_URE_DL_NORMAL(n) "lib" n STRING(UNXSUFFIX)
#define SCP2_URE_DL_BARE(n) n STRING(UNXSUFFIX)
#define SCP2_URE_DL_VER(n, v) "lib" n STRING(UNXSUFFIX) "." v
#define SCP2_URE_DL_COMID_VER(n, v) \
    "lib" n STRING(COMID) STRING(UNXSUFFIX) "." v
#define SCP2_URE_DL_UNO_VER(n, v) "libuno_" n STRING(UNXSUFFIX) "." v
#define SCP2_URE_DL_UNO_COMID_VER(n, v)	\
    "libuno_" n STRING(COMID) STRING(UNXSUFFIX) "." v
#define SCP2_URE_SHARE_DIR gid_Dir_Ure_Share
#endif

#include <langmacros.inc>


#endif // MACROS_INC

