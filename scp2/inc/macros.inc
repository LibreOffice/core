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

#define ICU_MAJOR 2
#define ICU_MINOR 6
#define ICU_MICRO 0

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
    #define LIBNAME(name)                     STRING(CONCAT5(lib,name,OFFICEUPD,DLLSUFFIX,UNXSUFFIX))
    #define LIBSHORTNAME(name)                STRING(CONCAT4(lib,name,DLLSUFFIX,UNXSUFFIX))
    #define FILTER_LIBNAME(name)              LIBNAME(name)
    #define EXENAME(name)                     STRING(name)
    #define PROFILENAME(name)                 STRING(CONCAT2(name,rc))
    #define SPECIAL_NAME(name)                STRING(CONCAT3(lib,name,UNXSUFFIX))
    #define SPECIAL_COMPONENT_LIB_NAME(name)  STRING(CONCAT2(name,UNXSUFFIX))
#else
    #define LIBNAME(name)                     STRING(CONCAT4(name,OFFICEUPD,DLLSUFFIX,.dll))
    #define LIBSHORTNAME(name)                STRING(CONCAT3(name,DLLSUFFIX,.dll))
    #define FILTER_LIBNAME(name)              LIBNAME(name)
    #define EXENAME(name)                     STRING(CONCAT2(name,.exe))
    #define PROFILENAME(name)                 STRING(CONCAT2(name,.ini))
    #define SPECIAL_NAME(name)                STRING(CONCAT2(name,.dll))
    #define SPECIAL_COMPONENT_LIB_NAME(name)  STRING(CONCAT2(name,.dll))
#endif

#define RESFILENAME(name,lang)                STRING(CONCAT4(name,OFFICEUPD,lang,.res))
#define SHORTRESFILENAME(name,lang)           STRING(CONCAT3(name,lang,.res))
#define READMETXTFILENAME(name,lang,ext)      STRING(CONCAT4(name,lang,.,ext))
#define READMEFILENAME(name,lang)             STRING(CONCAT2(name,lang))
#define HELPFILENAME(name,endung)             STRING(CONCAT3(name,.,endung))
#define EXTRAFILENAME(name,lang)              STRING(CONCAT3(name,lang,.zip))


#define FILE_BODY \
        Carrier      = gid_Datacarrier; \
        Time         = "0200"; \
        Date         = "29022004";

#define BIN_FILE_BODY \
        FILE_BODY \
        UnixRights   = 555

#define TXT_FILE_BODY \
        FILE_BODY \
        UnixRights   = 444

#define USER_FILE_BODY \
        FILE_BODY \
        UnixRights   = 644

#define PACKED_TXT_FILE_BODY \
        TXT_FILE_BODY; \
        Styles = (PACKED)

#define ARCHIVE_TXT_FILE_BODY \
        TXT_FILE_BODY; \
        Styles = (ARCHIVE)

#define ARCHIVE_SUBST_FILE_BODY \
        TXT_FILE_BODY; \
        Styles = (ARCHIVE,SUBSTITUTE)

#define STD_SETUPZIP_BMP_FILE(id,name) \
    File id \
        TXT_FILE_BODY; \
        Name      = DOSTRING(name); \
        Styles    = (PACKED, SETUPZIP); \
        Dir       = gid_Dir_Program; \
    End

#define PACKED_LIB_FILE_BODY \
        TXT_FILE_BODY; \
        Styles = (PACKED); \
        Dir    = gid_Dir_Program

#define PACKED_FILTER_FILE_BODY \
        TXT_FILE_BODY; \
        Styles = (PACKED); \
        Dir    = gid_Dir_Filter

#define STD_LIB_FILE(id,name) \
    File id \
        Name   = LIBNAME(name); \
        PACKED_LIB_FILE_BODY; \
    End

#define STD_RES_FILE(id,name) \
    File id \
        TXT_FILE_BODY; \
        RESFILE_ALL_LANG(name); \
        Dir     = gid_Dir_Resource; \
        Styles  = (PACKED); \
    End

#define STD_SETUPZIP_RES_FILE(id,name) \
    File id \
        TXT_FILE_BODY; \
        RESFILE_ALL_LANG(name); \
        Dir      = gid_Dir_Resource; \
        Styles   = (PACKED, SETUPZIP); \
    End

#define STD_FILTER_FILE(id,name) \
    File id \
        PACKED_FILTER_FILE_BODY; \
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

#define SETUP_FONTUNX_FILE(id,name,fontname) \
    File id \
        TXT_FILE_BODY; \
        Dir       = gid_Dir_Fonts_Truetype; \
        Styles    = (PACKED, FONT_WARN_IF_EXISTS, SETUPZIP); \
        Name      = STRING(name) ; \
    End

#define SETUP_FONTWIN_FILE(id,name,fontname) \
    File id \
        TXT_FILE_BODY; \
        Dir      = PREDEFINED_OSSYSTEMFONTDIR; \
        Styles   = (PACKED,FONT,DONT_DELETE,FONT_WARN_IF_EXISTS, SETUPZIP); \
        Name     = STRING(name) ; \
        FontName = STRING(fontname) ; \
        NetDir   = gid_Dir_Winfonts; \
    End

#define STD_JAR_FILE(id,name) \
    File id \
        TXT_FILE_BODY; \
        Name     = STRING(CONCAT2(name,.jar)); \
        Dir      = gid_Dir_Classes; \
        Styles   = (PACKED,DONT_OVERWRITE); \
    End

#define UNO_JAR_FILE(id,name) \
    File id \
        TXT_FILE_BODY; \
        Name     = STRING(CONCAT2(name,.jar)); \
        Dir      = gid_Dir_Classes; \
        Styles   = (PACKED,DONT_OVERWRITE, UNO_COMPONENT); \
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

#define STD_UNO_LIB_FILE(id,name) \
    File id \
        Name    = LIBNAME(name); \
        PACKED_UNO_LIB_FILE_BODY; \
    End

#define SPECIAL_UNO_LIB_FILE(id,name) \
    File id \
        Name    = SPECIAL_NAME(name); \
        PACKED_UNO_LIB_FILE_BODY; \
    End

#define SETUPZIP_LIB_FILE(id,name) \
    File id \
        TXT_FILE_BODY; \
        Name     = LIBNAME(name); \
        Dir      = gid_Dir_Program;

#define STD_SETUPZIP_LIB_FILE(id,name) \
        SETUPZIP_LIB_FILE(id,name) \
        Styles   = (PACKED, SETUPZIP); \
    End

#define UNO_SETUPZIP_LIB_FILE(id,name) \
        SETUPZIP_LIB_FILE(id,name) \
        RegistryID   = gid_Starregistry_Services_Rdb; \
        Styles       = (PACKED, SETUPZIP, UNO_COMPONENT); \
    End

#define SPECIAL_UNO_COMPONENT_LIB_FILE(id,name) \
    File id \
        Name     = SPECIAL_COMPONENT_LIB_NAME(name); \
        PACKED_UNO_LIB_FILE_BODY; \
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
        PACKED_FILTER_FILE_BODY; \
        Name            = SOLSW4WFILTERFILENAME(name); \
    End

#define SPECIAL_NO_WARNING_IF_NOT_EXISTS_FILE(id,name) \
    File id \
        TXT_FILE_BODY; \
        Name = LIBSHORTNAME(name); \
        Dir = gid_Dir_Program; \
        Styles = (PACKED, NO_WARNING_IF_NOT_EXISTS); \
    End

#define SPECIAL_UNO_NO_WARNING_IF_NOT_EXISTS_FILE(id,name) \
    File id \
        TXT_FILE_BODY; \
        Name = LIBSHORTNAME(name); \
        Dir = gid_Dir_Program; \
        Styles = (PACKED, NO_WARNING_IF_NOT_EXISTS, UNO_COMPONENT); \
        RegistryID = gid_Starregistry_Services_Rdb; \
    End


#define ALL_LANG(ident, resid) \
    ident (01)        = resid; \
    ident (03)        = resid; \
    ident (07)        = resid; \
    ident (30)        = resid; \
    ident (31)        = resid; \
    ident (33)        = resid; \
    ident (34)        = resid; \
    ident (35)        = resid; \
    ident (36)        = resid; \
    ident (37)        = resid; \
    ident (50)        = resid; \
    ident (39)        = resid; \
    ident (42)        = resid; \
    ident (43)        = resid; \
    ident (44)        = resid; \
    ident (45)        = resid; \
    ident (46)        = resid; \
    ident (47)        = resid; \
    ident (48)        = resid; \
    ident (49)        = resid; \
    ident (55)        = resid; \
    ident (66)        = resid; \
    ident (77)        = resid; \
    ident (81)        = resid; \
    ident (82)        = resid; \
    ident (86)        = resid; \
    ident (88)        = resid; \
    ident (90)        = resid; \
    ident (91)        = resid; \
    ident (96)        = resid; \
    ident (97)        = resid; \
    ident (99)        = resid

#define DIR_ENGLISH_ALL_LANG            \
    DosName (01)       = "english";     \
    DosName (03)       = "portuguese";  \
    DosName (07)       = "russian";     \
    DosName (30)       = "greek";       \
    DosName (31)       = "dutch";       \
    DosName (33)       = "french";      \
    DosName (34)       = "spanish";     \
    DosName (35)       = "finnish";     \
    DosName (36)       = "hungarian";   \
    DosName (37)       = "catalan";     \
    DosName (50)       = "slovenian";   \
    DosName (39)       = "italian";     \
    DosName (42)       = "czech";       \
    DosName (43)       = "slovak";      \
    DosName (44)       = "english_uk";  \
    DosName (45)       = "danish";      \
    DosName (46)       = "swedish";     \
    DosName (47)       = "norwegian";   \
    DosName (48)       = "polish";      \
    DosName (49)       = "german";      \
    DosName (55)       = "portuguese_brazilian";   \
    DosName (66)       = "thai";        \
    DosName (77)       = "estonian";        \
    DosName (81)       = "japanese";    \
    DosName (82)       = "korean";      \
    DosName (86)       = "chinese_simplified";     \
    DosName (88)       = "chinese_traditional";    \
    DosName (90)       = "turkish";     \
    DosName (91)       = "hindi";       \
    DosName (96)       = "arabic";      \
    DosName (97)       = "hebrew";      \
    DosName (99)       = STRING(ISO_CODE)

#define DIR_ISOLANGUAGE_ALL_LANG_2 \
    DosName (01)       = "en-US";  \
    DosName (03)       = "pt";     \
    DosName (07)       = "ru";     \
    DosName (30)       = "el";     \
    DosName (31)       = "nl";     \
    DosName (33)       = "fr";     \
    DosName (34)       = "es";     \
    DosName (35)       = "fi";     \
    DosName (36)       = "hu";     \
    DosName (37)       = "ca";     \
    DosName (50)       = "sl";     \
    DosName (39)       = "it";     \
    DosName (42)       = "cs";     \
    DosName (43)       = "sk";     \
    DosName (44)       = "en-GB";  \
    DosName (45)       = "da";     \
    DosName (46)       = "sv";     \
    DosName (47)       = "no";     \
    DosName (48)       = "pl";     \
    DosName (49)       = "de";     \
    DosName (55)       = "pt-BR";  \
    DosName (66)       = "th";     \
    DosName (77)       = "et";     \
    DosName (81)       = "ja";     \
    DosName (82)       = "ko";     \
    DosName (86)       = "zh-CN";  \
    DosName (88)       = "zh-TW";  \
    DosName (90)       = "tr";     \
    DosName (91)       = "hi-IN";  \
    DosName (96)       = "ar";     \
    DosName (97)       = "he";     \
    DosName (99)       = STRING(ISO_CODE)

#define DIR_IDENT_ALL_LANG(name) \
    DosName (01)       = STRING(name);     \
    DosName (03)       = STRING(name);     \
    DosName (07)       = STRING(name);     \
    DosName (30)       = STRING(name);     \
    DosName (31)       = STRING(name);     \
    DosName (33)       = STRING(name);     \
    DosName (34)       = STRING(name);     \
    DosName (35)       = STRING(name);     \
    DosName (36)       = STRING(name);     \
    DosName (37)       = STRING(name);     \
    DosName (50)       = STRING(name);     \
    DosName (39)       = STRING(name);     \
    DosName (42)       = STRING(name);     \
    DosName (43)       = STRING(name);     \
    DosName (44)       = STRING(name);     \
    DosName (45)       = STRING(name);     \
    DosName (46)       = STRING(name);     \
    DosName (47)       = STRING(name);     \
    DosName (48)       = STRING(name);     \
    DosName (49)       = STRING(name);     \
    DosName (55)       = STRING(name);     \
    DosName (66)       = STRING(name);     \
    DosName (77)       = STRING(name);     \
    DosName (81)       = STRING(name);     \
    DosName (82)       = STRING(name);     \
    DosName (86)       = STRING(name);     \
    DosName (88)       = STRING(name);     \
    DosName (90)       = STRING(name);     \
    DosName (91)       = STRING(name);     \
    DosName (96)       = STRING(name);     \
    DosName (97)       = STRING(name);     \
    DosName (99)       = STRING(name)

#define DIR_ISOLANGUAGE_ALL_LANG   \
    DosName (01)       = "en";     \
    DosName (03)       = "pt";     \
    DosName (07)       = "ru";     \
    DosName (30)       = "el";     \
    DosName (31)       = "nl";     \
    DosName (33)       = "fr";     \
    DosName (34)       = "es";     \
    DosName (35)       = "fi";     \
    DosName (36)       = "hu";     \
    DosName (37)       = "ca";     \
    DosName (50)       = "sl";     \
    DosName (39)       = "it";     \
    DosName (42)       = "cs";     \
    DosName (43)       = "sk";     \
    DosName (44)       = "en-GB";  \
    DosName (45)       = "da";     \
    DosName (46)       = "sv";     \
    DosName (47)       = "no";     \
    DosName (48)       = "pl";     \
    DosName (49)       = "de";     \
    DosName (55)       = "pt-BR";  \
    DosName (66)       = "th";     \
    DosName (77)       = "et";     \
    DosName (81)       = "ja";     \
    DosName (82)       = "ko";     \
    DosName (86)       = "zh-CN";  \
    DosName (88)       = "zh-TW";  \
    DosName (90)       = "tr";     \
    DosName (91)       = "hi-IN";  \
    DosName (96)       = "ar";     \
    DosName (97)       = "he";     \
    DosName (99)       = STRING(ISO_CODE)

#define EXTRA_ALL_LANG(name) \
    Name (01)       = EXTRAFILENAME(name,01);     \
    Name (03)       = EXTRAFILENAME(name,03);     \
    Name (07)       = EXTRAFILENAME(name,07);     \
    Name (30)       = EXTRAFILENAME(name,30);     \
    Name (31)       = EXTRAFILENAME(name,31);     \
    Name (33)       = EXTRAFILENAME(name,33);     \
    Name (34)       = EXTRAFILENAME(name,34);     \
    Name (35)       = EXTRAFILENAME(name,35);     \
    Name (36)       = EXTRAFILENAME(name,36);     \
    Name (37)       = EXTRAFILENAME(name,37);     \
    Name (50)       = EXTRAFILENAME(name,50);     \
    Name (39)       = EXTRAFILENAME(name,39);     \
    Name (42)       = EXTRAFILENAME(name,42);     \
    Name (43)       = EXTRAFILENAME(name,43);     \
    Name (44)       = EXTRAFILENAME(name,44);     \
    Name (45)       = EXTRAFILENAME(name,45);     \
    Name (46)       = EXTRAFILENAME(name,46);     \
    Name (47)       = EXTRAFILENAME(name,47);     \
    Name (48)       = EXTRAFILENAME(name,48);     \
    Name (49)       = EXTRAFILENAME(name,49);     \
    Name (55)       = EXTRAFILENAME(name,55);     \
    Name (66)       = EXTRAFILENAME(name,66);     \
    Name (77)       = EXTRAFILENAME(name,77);     \
    Name (81)       = EXTRAFILENAME(name,81);     \
    Name (82)       = EXTRAFILENAME(name,82);     \
    Name (86)       = EXTRAFILENAME(name,86);     \
    Name (88)       = EXTRAFILENAME(name,88);     \
    Name (90)       = EXTRAFILENAME(name,90);     \
    Name (91)       = EXTRAFILENAME(name,91);     \
    Name (96)       = EXTRAFILENAME(name,96);     \
    Name (97)       = EXTRAFILENAME(name,97);     \
    Name (99)       = EXTRAFILENAME(name,99)

#define EXTRA_IDENT_ALL_LANG(name) \
    Name (01)       = STRING(name);     \
    Name (03)       = STRING(name);     \
    Name (07)       = STRING(name);     \
    Name (30)       = STRING(name);     \
    Name (31)       = STRING(name);     \
    Name (33)       = STRING(name);     \
    Name (34)       = STRING(name);     \
    Name (35)       = STRING(name);     \
    Name (36)       = STRING(name);     \
    Name (37)       = STRING(name);     \
    Name (50)       = STRING(name);     \
    Name (39)       = STRING(name);     \
    Name (42)       = STRING(name);     \
    Name (43)       = STRING(name);     \
    Name (44)       = STRING(name);     \
    Name (45)       = STRING(name);     \
    Name (46)       = STRING(name);     \
    Name (47)       = STRING(name);     \
    Name (48)       = STRING(name);     \
    Name (49)       = STRING(name);     \
    Name (55)       = STRING(name);     \
    Name (66)       = STRING(name);     \
    Name (77)       = STRING(name);     \
    Name (81)       = STRING(name);     \
    Name (82)       = STRING(name);     \
    Name (86)       = STRING(name);     \
    Name (88)       = STRING(name);     \
    Name (90)       = STRING(name);     \
    Name (91)       = STRING(name);     \
    Name (96)       = STRING(name);     \
    Name (97)       = STRING(name);     \
    Name (99)       = STRING(name)

#define RESFILE_ALL_LANG(name) \
    Name (01)       = RESFILENAME(name,01);  \
    Name (03)       = RESFILENAME(name,03);  \
    Name (07)       = RESFILENAME(name,07);  \
    Name (30)       = RESFILENAME(name,30);  \
    Name (31)       = RESFILENAME(name,31);  \
    Name (33)       = RESFILENAME(name,33);  \
    Name (34)       = RESFILENAME(name,34);  \
    Name (35)       = RESFILENAME(name,35);  \
    Name (36)       = RESFILENAME(name,36);  \
    Name (37)       = RESFILENAME(name,37);  \
    Name (50)       = RESFILENAME(name,50);  \
    Name (39)       = RESFILENAME(name,39);  \
    Name (42)       = RESFILENAME(name,42);  \
    Name (43)       = RESFILENAME(name,43);  \
    Name (44)       = RESFILENAME(name,44);  \
    Name (45)       = RESFILENAME(name,45);  \
    Name (46)       = RESFILENAME(name,46);  \
    Name (47)       = RESFILENAME(name,47);  \
    Name (48)       = RESFILENAME(name,48);  \
    Name (49)       = RESFILENAME(name,49);  \
    Name (55)       = RESFILENAME(name,55);  \
    Name (66)       = RESFILENAME(name,66);  \
    Name (77)       = RESFILENAME(name,77);  \
    Name (81)       = RESFILENAME(name,81);  \
    Name (82)       = RESFILENAME(name,82);  \
    Name (86)       = RESFILENAME(name,86);  \
    Name (88)       = RESFILENAME(name,88);  \
    Name (90)       = RESFILENAME(name,90);  \
    Name (91)       = RESFILENAME(name,91);  \
    Name (96)       = RESFILENAME(name,96);  \
    Name (97)       = RESFILENAME(name,97);  \
    Name (99)       = RESFILENAME(name,99)

#define SHORT_RESFILE_ALL_LANG(name) \
    Name (01)       = SHORTRESFILENAME(name,01);     \
    Name (03)       = SHORTRESFILENAME(name,03);     \
    Name (07)       = SHORTRESFILENAME(name,07);     \
    Name (30)       = SHORTRESFILENAME(name,30);     \
    Name (31)       = SHORTRESFILENAME(name,31);     \
    Name (33)       = SHORTRESFILENAME(name,33);     \
    Name (34)       = SHORTRESFILENAME(name,34);     \
    Name (35)       = SHORTRESFILENAME(name,35);     \
    Name (36)       = SHORTRESFILENAME(name,36);     \
    Name (37)       = SHORTRESFILENAME(name,37);     \
    Name (50)       = SHORTRESFILENAME(name,50);     \
    Name (39)       = SHORTRESFILENAME(name,39);     \
    Name (42)       = SHORTRESFILENAME(name,42);     \
    Name (43)       = SHORTRESFILENAME(name,43);     \
    Name (44)       = SHORTRESFILENAME(name,44);     \
    Name (45)       = SHORTRESFILENAME(name,45);     \
    Name (46)       = SHORTRESFILENAME(name,46);     \
    Name (47)       = SHORTRESFILENAME(name,47);     \
    Name (48)       = SHORTRESFILENAME(name,48);     \
    Name (49)       = SHORTRESFILENAME(name,49);     \
    Name (55)       = SHORTRESFILENAME(name,55);     \
    Name (66)       = SHORTRESFILENAME(name,66);     \
    Name (77)       = SHORTRESFILENAME(name,77);     \
    Name (81)       = SHORTRESFILENAME(name,81);     \
    Name (82)       = SHORTRESFILENAME(name,82);     \
    Name (86)       = SHORTRESFILENAME(name,86);     \
    Name (88)       = SHORTRESFILENAME(name,88);     \
    Name (90)       = SHORTRESFILENAME(name,90);     \
    Name (91)       = SHORTRESFILENAME(name,91);     \
    Name (96)       = SHORTRESFILENAME(name,96);     \
    Name (97)       = SHORTRESFILENAME(name,97);     \
    Name (99)       = SHORTRESFILENAME(name,99)

#define README_ALL_LANG(key, name) \
    key (01)       = READMEFILENAME(name,01);     \
    key (03)       = READMEFILENAME(name,03);     \
    key (07)       = READMEFILENAME(name,07);     \
    key (30)       = READMEFILENAME(name,30);     \
    key (31)       = READMEFILENAME(name,31);     \
    key (33)       = READMEFILENAME(name,33);     \
    key (34)       = READMEFILENAME(name,34);     \
    key (35)       = READMEFILENAME(name,35);     \
    key (36)       = READMEFILENAME(name,36);     \
    key (37)       = READMEFILENAME(name,37);     \
    key (50)       = READMEFILENAME(name,50);     \
    key (39)       = READMEFILENAME(name,39);     \
    key (42)       = READMEFILENAME(name,42);     \
    key (43)       = READMEFILENAME(name,43);     \
    key (44)       = READMEFILENAME(name,44);     \
    key (45)       = READMEFILENAME(name,45);     \
    key (46)       = READMEFILENAME(name,46);     \
    key (47)       = READMEFILENAME(name,47);     \
    key (48)       = READMEFILENAME(name,48);     \
    key (49)       = READMEFILENAME(name,49);     \
    key (55)       = READMEFILENAME(name,55);     \
    key (66)       = READMEFILENAME(name,66);     \
    key (77)       = READMEFILENAME(name,77);     \
    key (81)       = READMEFILENAME(name,81);     \
    key (82)       = READMEFILENAME(name,82);     \
    key (86)       = READMEFILENAME(name,86);     \
    key (88)       = READMEFILENAME(name,88);     \
    key (90)       = READMEFILENAME(name,90);     \
    key (91)       = READMEFILENAME(name,91);     \
    key (96)       = READMEFILENAME(name,96);     \
    key (97)       = READMEFILENAME(name,97);     \
    key (99)       = READMEFILENAME(name,99)

#define README_TXT_ALL_LANG(key, name, ext) \
    key (01)       = READMETXTFILENAME(name,01,ext);     \
    key (03)       = READMETXTFILENAME(name,03,ext);     \
    key (07)       = READMETXTFILENAME(name,07,ext);     \
    key (30)       = READMETXTFILENAME(name,30,ext);     \
    key (31)       = READMETXTFILENAME(name,31,ext);     \
    key (33)       = READMETXTFILENAME(name,33,ext);     \
    key (34)       = READMETXTFILENAME(name,34,ext);     \
    key (35)       = READMETXTFILENAME(name,35,ext);     \
    key (36)       = READMETXTFILENAME(name,36,ext);     \
    key (37)       = READMETXTFILENAME(name,37,ext);     \
    key (50)       = READMETXTFILENAME(name,50,ext);     \
    key (39)       = READMETXTFILENAME(name,39,ext);     \
    key (42)       = READMETXTFILENAME(name,42,ext);     \
    key (43)       = READMETXTFILENAME(name,43,ext);     \
    key (44)       = READMETXTFILENAME(name,44,ext);     \
    key (45)       = READMETXTFILENAME(name,45,ext);     \
    key (46)       = READMETXTFILENAME(name,46,ext);     \
    key (47)       = READMETXTFILENAME(name,47,ext);     \
    key (48)       = READMETXTFILENAME(name,48,ext);     \
    key (49)       = READMETXTFILENAME(name,49,ext);     \
    key (55)       = READMETXTFILENAME(name,55,ext);     \
    key (66)       = READMETXTFILENAME(name,66,ext);     \
    key (77)       = READMETXTFILENAME(name,77,ext);     \
    key (81)       = READMETXTFILENAME(name,81,ext);     \
    key (82)       = READMETXTFILENAME(name,82,ext);     \
    key (86)       = READMETXTFILENAME(name,86,ext);     \
    key (88)       = READMETXTFILENAME(name,88,ext);     \
    key (90)       = READMETXTFILENAME(name,90,ext);     \
    key (91)       = READMETXTFILENAME(name,91,ext);     \
    key (96)       = READMETXTFILENAME(name,96,ext);     \
    key (97)       = READMETXTFILENAME(name,97,ext);     \
    key (99)       = READMETXTFILENAME(name,99,ext)

#define VALUE_ALL_LANG_LETTER_DIR   \
    Value (01)     = STRING(en);    \
    Value (03)     = STRING(pt);    \
    Value (07)     = STRING(ru);    \
    Value (30)     = STRING(el);    \
    Value (31)     = STRING(nl);    \
    Value (33)     = STRING(fr);    \
    Value (34)     = STRING(es);    \
    Value (35)     = STRING(fi);    \
    Value (36)     = STRING(hu);    \
    Value (37)     = STRING(ca);    \
    Value (50)     = STRING(sl);    \
    Value (39)     = STRING(it);    \
    Value (42)     = STRING(cs);    \
    Value (43)     = STRING(sk);    \
    Value (44)     = STRING(en-GB); \
    Value (45)     = STRING(da);    \
    Value (46)     = STRING(sv);    \
    Value (47)     = STRING(no);    \
    Value (48)     = STRING(pl);    \
    Value (49)     = STRING(de);    \
    Value (55)     = STRING(pt-BR); \
    Value (66)     = STRING(th);    \
    Value (77)     = STRING(et);    \
    Value (81)     = STRING(ja);    \
    Value (82)     = STRING(ko);    \
    Value (86)     = STRING(zh-CN); \
    Value (88)     = STRING(zh-TW); \
    Value (90)     = STRING(tr);    \
    Value (91)     = STRING(hi-IN); \
    Value (96)     = STRING(ar);    \
    Value (97)     = STRING(he);    \
    Value (99)     = STRING(ISO_CODE)

#define KEY_ALL_LANG_LETTER \
    Key (01)       = STRING(en-US); \
    Key (03)       = STRING(pt);    \
    Key (07)       = STRING(ru);    \
    Key (30)       = STRING(el);    \
    Key (31)       = STRING(nl);    \
    Key (33)       = STRING(fr);    \
    Key (34)       = STRING(es);    \
    Key (35)       = STRING(fi);    \
    Key (36)       = STRING(hu);    \
    Key (37)       = STRING(ca);    \
    Key (50)       = STRING(sl);    \
    Key (39)       = STRING(it);    \
    Key (42)       = STRING(cs);    \
    Key (43)       = STRING(sk);    \
    Key (44)       = STRING(en-GB); \
    Key (45)       = STRING(da);    \
    Key (46)       = STRING(sv);    \
    Key (47)       = STRING(no);    \
    Key (48)       = STRING(pl);    \
    Key (49)       = STRING(de);    \
    Key (55)       = STRING(pt-BR); \
    Key (66)       = STRING(th);    \
    Key (77)       = STRING(et);    \
    Key (81)       = STRING(ja);    \
    Key (82)       = STRING(ko);    \
    Key (86)       = STRING(zh-CN); \
    Key (88)       = STRING(zh-TW); \
    Key (90)       = STRING(tr);    \
    Key (91)       = STRING(hi-IN); \
    Key (96)       = STRING(ar);    \
    Key (97)       = STRING(he);    \
    Key (99)       = STRING(ISO_CODE)

#endif // MACROS_INC

