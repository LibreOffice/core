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
#define CONFIGLANGFILENAME(name,lang,ext)     STRING(CONCAT4(name,lang,.,ext))


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
    ident (en-US)        = resid; \
    ident (pt)        = resid; \
    ident (ru)        = resid; \
    ident (el)        = resid; \
    ident (nl)        = resid; \
    ident (fr)        = resid; \
    ident (es)        = resid; \
    ident (fi)        = resid; \
    ident (hu)        = resid; \
    ident (ca)        = resid; \
    ident (sl)        = resid; \
    ident (it)        = resid; \
    ident (cs)        = resid; \
    ident (sk)        = resid; \
    ident (en-GB)        = resid; \
    ident (da)        = resid; \
    ident (sv)        = resid; \
    ident (no)        = resid; \
    ident (pl)        = resid; \
    ident (de)        = resid; \
    ident (pt-BR)        = resid; \
    ident (th)        = resid; \
    ident (et)        = resid; \
    ident (ja)        = resid; \
    ident (ko)        = resid; \
    ident (zh-CN)        = resid; \
    ident (zh-TW)        = resid; \
    ident (tr)        = resid; \
    ident (hi-IN)        = resid; \
    ident (ar)        = resid; \
    ident (he)        = resid; \
    ident (99)        = resid

#define DIR_ENGLISH_ALL_LANG            \
    DosName (en-US)       = "english";     \
    DosName (pt)       = "portuguese";  \
    DosName (ru)       = "russian";     \
    DosName (el)       = "greek";       \
    DosName (nl)       = "dutch";       \
    DosName (fr)       = "french";      \
    DosName (es)       = "spanish";     \
    DosName (fi)       = "finnish";     \
    DosName (hu)       = "hungarian";   \
    DosName (ca)       = "catalan";     \
    DosName (sl)       = "slovenian";   \
    DosName (it)       = "italian";     \
    DosName (cs)       = "czech";       \
    DosName (sk)       = "slovak";      \
    DosName (en-GB)       = "english_uk";  \
    DosName (da)       = "danish";      \
    DosName (sv)       = "swedish";     \
    DosName (no)       = "norwegian";   \
    DosName (pl)       = "polish";      \
    DosName (de)       = "german";      \
    DosName (pt-BR)       = "portuguese_brazilian";   \
    DosName (th)       = "thai";        \
    DosName (et)       = "estonian";        \
    DosName (ja)       = "japanese";    \
    DosName (ko)       = "korean";      \
    DosName (zh-CN)       = "chinese_simplified";     \
    DosName (zh-TW)       = "chinese_traditional";    \
    DosName (tr)       = "turkish";     \
    DosName (hi-IN)       = "hindi";       \
    DosName (ar)       = "arabic";      \
    DosName (he)       = "hebrew";      \
    DosName (99)       = STRING(ISO_CODE)

#define DIR_ISOLANGUAGE_ALL_LANG_2 \
    DosName (en-US)       = "en-US";  \
    DosName (pt)       = "pt";     \
    DosName (ru)       = "ru";     \
    DosName (el)       = "el";     \
    DosName (nl)       = "nl";     \
    DosName (fr)       = "fr";     \
    DosName (es)       = "es";     \
    DosName (fi)       = "fi";     \
    DosName (hu)       = "hu";     \
    DosName (ca)       = "ca";     \
    DosName (sl)       = "sl";     \
    DosName (it)       = "it";     \
    DosName (cs)       = "cs";     \
    DosName (sk)       = "sk";     \
    DosName (en-GB)       = "en-GB";  \
    DosName (da)       = "da";     \
    DosName (sv)       = "sv";     \
    DosName (no)       = "no";     \
    DosName (pl)       = "pl";     \
    DosName (de)       = "de";     \
    DosName (pt-BR)       = "pt-BR";  \
    DosName (th)       = "th";     \
    DosName (et)       = "et";     \
    DosName (ja)       = "ja";     \
    DosName (ko)       = "ko";     \
    DosName (zh-CN)       = "zh-CN";  \
    DosName (zh-TW)       = "zh-TW";  \
    DosName (tr)       = "tr";     \
    DosName (hi-IN)       = "hi-IN";  \
    DosName (ar)       = "ar";     \
    DosName (he)       = "he";     \
    DosName (99)       = STRING(ISO_CODE)

#define DIR_IDENT_ALL_LANG(name) \
    DosName (en-US)       = STRING(name);     \
    DosName (pt)       = STRING(name);     \
    DosName (ru)       = STRING(name);     \
    DosName (el)       = STRING(name);     \
    DosName (nl)       = STRING(name);     \
    DosName (fr)       = STRING(name);     \
    DosName (es)       = STRING(name);     \
    DosName (fi)       = STRING(name);     \
    DosName (hu)       = STRING(name);     \
    DosName (ca)       = STRING(name);     \
    DosName (sl)       = STRING(name);     \
    DosName (it)       = STRING(name);     \
    DosName (cs)       = STRING(name);     \
    DosName (sk)       = STRING(name);     \
    DosName (en-GB)       = STRING(name);     \
    DosName (da)       = STRING(name);     \
    DosName (sv)       = STRING(name);     \
    DosName (no)       = STRING(name);     \
    DosName (pl)       = STRING(name);     \
    DosName (de)       = STRING(name);     \
    DosName (pt-BR)       = STRING(name);     \
    DosName (th)       = STRING(name);     \
    DosName (et)       = STRING(name);     \
    DosName (ja)       = STRING(name);     \
    DosName (ko)       = STRING(name);     \
    DosName (zh-CN)       = STRING(name);     \
    DosName (zh-TW)       = STRING(name);     \
    DosName (tr)       = STRING(name);     \
    DosName (hi-IN)       = STRING(name);     \
    DosName (ar)       = STRING(name);     \
    DosName (he)       = STRING(name);     \
    DosName (99)       = STRING(name)

#define DIR_ISOLANGUAGE_ALL_LANG   \
    DosName (en-US)       = "en";     \
    DosName (pt)       = "pt";     \
    DosName (ru)       = "ru";     \
    DosName (el)       = "el";     \
    DosName (nl)       = "nl";     \
    DosName (fr)       = "fr";     \
    DosName (es)       = "es";     \
    DosName (fi)       = "fi";     \
    DosName (hu)       = "hu";     \
    DosName (ca)       = "ca";     \
    DosName (sl)       = "sl";     \
    DosName (it)       = "it";     \
    DosName (cs)       = "cs";     \
    DosName (sk)       = "sk";     \
    DosName (en-GB)       = "en-GB";  \
    DosName (da)       = "da";     \
    DosName (sv)       = "sv";     \
    DosName (no)       = "no";     \
    DosName (pl)       = "pl";     \
    DosName (de)       = "de";     \
    DosName (pt-BR)       = "pt-BR";  \
    DosName (th)       = "th";     \
    DosName (et)       = "et";     \
    DosName (ja)       = "ja";     \
    DosName (ko)       = "ko";     \
    DosName (zh-CN)       = "zh-CN";  \
    DosName (zh-TW)       = "zh-TW";  \
    DosName (tr)       = "tr";     \
    DosName (hi-IN)       = "hi-IN";  \
    DosName (ar)       = "ar";     \
    DosName (he)       = "he";     \
    DosName (99)       = STRING(ISO_CODE)

#define EXTRA_ALL_LANG(name) \
    Name (en-US)       = EXTRAFILENAME(name,_en-US);     \
    Name (pt)       = EXTRAFILENAME(name,_pt);     \
    Name (ru)       = EXTRAFILENAME(name,_ru);     \
    Name (el)       = EXTRAFILENAME(name,_el);     \
    Name (nl)       = EXTRAFILENAME(name,_nl);     \
    Name (fr)       = EXTRAFILENAME(name,_fr);     \
    Name (es)       = EXTRAFILENAME(name,_es);     \
    Name (fi)       = EXTRAFILENAME(name,_fi);     \
    Name (hu)       = EXTRAFILENAME(name,_hu);     \
    Name (ca)       = EXTRAFILENAME(name,_ca);     \
    Name (sl)       = EXTRAFILENAME(name,_sl);     \
    Name (it)       = EXTRAFILENAME(name,_it);     \
    Name (cs)       = EXTRAFILENAME(name,_cs);     \
    Name (sk)       = EXTRAFILENAME(name,_sk);     \
    Name (en-GB)       = EXTRAFILENAME(name,_en-GB);     \
    Name (da)       = EXTRAFILENAME(name,_da);     \
    Name (sv)       = EXTRAFILENAME(name,_sv);     \
    Name (no)       = EXTRAFILENAME(name,_no);     \
    Name (pl)       = EXTRAFILENAME(name,_pl);     \
    Name (de)       = EXTRAFILENAME(name,_de);     \
    Name (pt-BR)       = EXTRAFILENAME(name,_pt-BR);     \
    Name (th)       = EXTRAFILENAME(name,_th);     \
    Name (et)       = EXTRAFILENAME(name,_et);     \
    Name (ja)       = EXTRAFILENAME(name,_ja);     \
    Name (ko)       = EXTRAFILENAME(name,_ko);     \
    Name (zh-CN)       = EXTRAFILENAME(name,_zh-CN);     \
    Name (zh-TW)       = EXTRAFILENAME(name,_zh-TW);     \
    Name (tr)       = EXTRAFILENAME(name,_tr);     \
    Name (hi-IN)       = EXTRAFILENAME(name,_hi-IN);     \
    Name (ar)       = EXTRAFILENAME(name,_ar);     \
    Name (he)       = EXTRAFILENAME(name,_he);     \
    Name (99)       = EXTRAFILENAME(name,_99)

#define EXTRA_IDENT_ALL_LANG(name) \
    Name (en-US)       = STRING(name);     \
    Name (pt)       = STRING(name);     \
    Name (ru)       = STRING(name);     \
    Name (el)       = STRING(name);     \
    Name (nl)       = STRING(name);     \
    Name (fr)       = STRING(name);     \
    Name (es)       = STRING(name);     \
    Name (fi)       = STRING(name);     \
    Name (hu)       = STRING(name);     \
    Name (ca)       = STRING(name);     \
    Name (sl)       = STRING(name);     \
    Name (it)       = STRING(name);     \
    Name (cs)       = STRING(name);     \
    Name (sk)       = STRING(name);     \
    Name (en-GB)       = STRING(name);     \
    Name (da)       = STRING(name);     \
    Name (sv)       = STRING(name);     \
    Name (no)       = STRING(name);     \
    Name (pl)       = STRING(name);     \
    Name (de)       = STRING(name);     \
    Name (pt-BR)       = STRING(name);     \
    Name (th)       = STRING(name);     \
    Name (et)       = STRING(name);     \
    Name (ja)       = STRING(name);     \
    Name (ko)       = STRING(name);     \
    Name (zh-CN)       = STRING(name);     \
    Name (zh-TW)       = STRING(name);     \
    Name (tr)       = STRING(name);     \
    Name (hi-IN)       = STRING(name);     \
    Name (ar)       = STRING(name);     \
    Name (he)       = STRING(name);     \
    Name (99)       = STRING(name)

#define RESFILE_ALL_LANG(name) \
    Name (en-US)       = RESFILENAME(name,en-US);  \
    Name (pt)       = RESFILENAME(name,pt);  \
    Name (ru)       = RESFILENAME(name,ru);  \
    Name (el)       = RESFILENAME(name,el);  \
    Name (nl)       = RESFILENAME(name,nl);  \
    Name (fr)       = RESFILENAME(name,fr);  \
    Name (es)       = RESFILENAME(name,es);  \
    Name (fi)       = RESFILENAME(name,fi);  \
    Name (hu)       = RESFILENAME(name,hu);  \
    Name (ca)       = RESFILENAME(name,ca);  \
    Name (sl)       = RESFILENAME(name,sl);  \
    Name (it)       = RESFILENAME(name,it);  \
    Name (cs)       = RESFILENAME(name,cs);  \
    Name (sk)       = RESFILENAME(name,sk);  \
    Name (en-GB)       = RESFILENAME(name,en-GB);  \
    Name (da)       = RESFILENAME(name,da);  \
    Name (sv)       = RESFILENAME(name,sv);  \
    Name (no)       = RESFILENAME(name,no);  \
    Name (pl)       = RESFILENAME(name,pl);  \
    Name (de)       = RESFILENAME(name,de);  \
    Name (pt-BR)       = RESFILENAME(name,pt-BR);  \
    Name (th)       = RESFILENAME(name,th);  \
    Name (et)       = RESFILENAME(name,et);  \
    Name (ja)       = RESFILENAME(name,ja);  \
    Name (ko)       = RESFILENAME(name,ko);  \
    Name (zh-CN)       = RESFILENAME(name,zh-CN);  \
    Name (zh-TW)       = RESFILENAME(name,zh-TW);  \
    Name (tr)       = RESFILENAME(name,tr);  \
    Name (hi-IN)       = RESFILENAME(name,hi-IN);  \
    Name (ar)       = RESFILENAME(name,ar);  \
    Name (he)       = RESFILENAME(name,he);  \
    Name (99)       = RESFILENAME(name,99)

#define SHORT_RESFILE_ALL_LANG(name) \
    Name (en-US)       = SHORTRESFILENAME(name,en-US);     \
    Name (pt)       = SHORTRESFILENAME(name,pt);     \
    Name (ru)       = SHORTRESFILENAME(name,ru);     \
    Name (el)       = SHORTRESFILENAME(name,el);     \
    Name (nl)       = SHORTRESFILENAME(name,nl);     \
    Name (fr)       = SHORTRESFILENAME(name,fr);     \
    Name (es)       = SHORTRESFILENAME(name,es);     \
    Name (fi)       = SHORTRESFILENAME(name,fi);     \
    Name (hu)       = SHORTRESFILENAME(name,hu);     \
    Name (ca)       = SHORTRESFILENAME(name,ca);     \
    Name (sl)       = SHORTRESFILENAME(name,sl);     \
    Name (it)       = SHORTRESFILENAME(name,it);     \
    Name (cs)       = SHORTRESFILENAME(name,cs);     \
    Name (sk)       = SHORTRESFILENAME(name,sk);     \
    Name (en-GB)       = SHORTRESFILENAME(name,en-GB);     \
    Name (da)       = SHORTRESFILENAME(name,da);     \
    Name (sv)       = SHORTRESFILENAME(name,sv);     \
    Name (no)       = SHORTRESFILENAME(name,no);     \
    Name (pl)       = SHORTRESFILENAME(name,pl);     \
    Name (de)       = SHORTRESFILENAME(name,de);     \
    Name (pt-BR)       = SHORTRESFILENAME(name,pt-BR);     \
    Name (th)       = SHORTRESFILENAME(name,th);     \
    Name (et)       = SHORTRESFILENAME(name,et);     \
    Name (ja)       = SHORTRESFILENAME(name,ja);     \
    Name (ko)       = SHORTRESFILENAME(name,ko);     \
    Name (zh-CN)       = SHORTRESFILENAME(name,zh-CN);     \
    Name (zh-TW)       = SHORTRESFILENAME(name,zh-TW);     \
    Name (tr)       = SHORTRESFILENAME(name,tr);     \
    Name (hi-IN)       = SHORTRESFILENAME(name,hi-IN);     \
    Name (ar)       = SHORTRESFILENAME(name,ar);     \
    Name (he)       = SHORTRESFILENAME(name,he);     \
    Name (99)       = SHORTRESFILENAME(name,99)

#define README_ALL_LANG(key, name) \
    key (en-US)       = READMEFILENAME(name,_en-US);     \
    key (pt)       = READMEFILENAME(name,_pt);     \
    key (ru)       = READMEFILENAME(name,_ru);     \
    key (el)       = READMEFILENAME(name,_el);     \
    key (nl)       = READMEFILENAME(name,_nl);     \
    key (fr)       = READMEFILENAME(name,_fr);     \
    key (es)       = READMEFILENAME(name,_es);     \
    key (fi)       = READMEFILENAME(name,_fi);     \
    key (hu)       = READMEFILENAME(name,_hu);     \
    key (ca)       = READMEFILENAME(name,_ca);     \
    key (sl)       = READMEFILENAME(name,_sl);     \
    key (it)       = READMEFILENAME(name,_it);     \
    key (cs)       = READMEFILENAME(name,_cs);     \
    key (sk)       = READMEFILENAME(name,_sk);     \
    key (en-GB)       = READMEFILENAME(name,_en-GB);     \
    key (da)       = READMEFILENAME(name,_da);     \
    key (sv)       = READMEFILENAME(name,_sv);     \
    key (no)       = READMEFILENAME(name,_no);     \
    key (pl)       = READMEFILENAME(name,_pl);     \
    key (de)       = READMEFILENAME(name,_de);     \
    key (pt-BR)       = READMEFILENAME(name,_pt-BR);     \
    key (th)       = READMEFILENAME(name,_th);     \
    key (et)       = READMEFILENAME(name,_et);     \
    key (ja)       = READMEFILENAME(name,_ja);     \
    key (ko)       = READMEFILENAME(name,_ko);     \
    key (zh-CN)       = READMEFILENAME(name,_zh-CN);     \
    key (zh-TW)       = READMEFILENAME(name,_zh-TW);     \
    key (tr)       = READMEFILENAME(name,_tr);     \
    key (hi-IN)       = READMEFILENAME(name,_hi-IN);     \
    key (ar)       = READMEFILENAME(name,_ar);     \
    key (he)       = READMEFILENAME(name,_he);     \
    key (99)       = READMEFILENAME(name,_99)

#define README_TXT_ALL_LANG(key, name, ext) \
    key (en-US)       = READMETXTFILENAME(name,_en-US,ext);     \
    key (pt)       = READMETXTFILENAME(name,_pt,ext);     \
    key (ru)       = READMETXTFILENAME(name,_ru,ext);     \
    key (el)       = READMETXTFILENAME(name,_el,ext);     \
    key (nl)       = READMETXTFILENAME(name,_nl,ext);     \
    key (fr)       = READMETXTFILENAME(name,_fr,ext);     \
    key (es)       = READMETXTFILENAME(name,_es,ext);     \
    key (fi)       = READMETXTFILENAME(name,_fi,ext);     \
    key (hu)       = READMETXTFILENAME(name,_hu,ext);     \
    key (ca)       = READMETXTFILENAME(name,_ca,ext);     \
    key (sl)       = READMETXTFILENAME(name,_sl,ext);     \
    key (it)       = READMETXTFILENAME(name,_it,ext);     \
    key (cs)       = READMETXTFILENAME(name,_cs,ext);     \
    key (sk)       = READMETXTFILENAME(name,_sk,ext);     \
    key (en-GB)       = READMETXTFILENAME(name,_en-GB,ext);     \
    key (da)       = READMETXTFILENAME(name,_da,ext);     \
    key (sv)       = READMETXTFILENAME(name,_sv,ext);     \
    key (no)       = READMETXTFILENAME(name,_no,ext);     \
    key (pl)       = READMETXTFILENAME(name,_pl,ext);     \
    key (de)       = READMETXTFILENAME(name,_de,ext);     \
    key (pt-BR)       = READMETXTFILENAME(name,_pt-BR,ext);     \
    key (th)       = READMETXTFILENAME(name,_th,ext);     \
    key (et)       = READMETXTFILENAME(name,_et,ext);     \
    key (ja)       = READMETXTFILENAME(name,_ja,ext);     \
    key (ko)       = READMETXTFILENAME(name,_ko,ext);     \
    key (zh-CN)       = READMETXTFILENAME(name,_zh-CN,ext);     \
    key (zh-TW)       = READMETXTFILENAME(name,_zh-TW,ext);     \
    key (tr)       = READMETXTFILENAME(name,_tr,ext);     \
    key (hi-IN)       = READMETXTFILENAME(name,_hi-IN,ext);     \
    key (ar)       = READMETXTFILENAME(name,_ar,ext);     \
    key (he)       = READMETXTFILENAME(name,_he,ext);     \
    key (99)       = READMETXTFILENAME(name,_99,ext)

#define VALUE_ALL_LANG_LETTER_DIR   \
    Value (en-US)     = STRING(en);    \
    Value (pt)     = STRING(pt);    \
    Value (ru)     = STRING(ru);    \
    Value (el)     = STRING(el);    \
    Value (nl)     = STRING(nl);    \
    Value (fr)     = STRING(fr);    \
    Value (es)     = STRING(es);    \
    Value (fi)     = STRING(fi);    \
    Value (hu)     = STRING(hu);    \
    Value (ca)     = STRING(ca);    \
    Value (sl)     = STRING(sl);    \
    Value (it)     = STRING(it);    \
    Value (cs)     = STRING(cs);    \
    Value (sk)     = STRING(sk);    \
    Value (en-GB)     = STRING(en-GB); \
    Value (da)     = STRING(da);    \
    Value (sv)     = STRING(sv);    \
    Value (no)     = STRING(no);    \
    Value (pl)     = STRING(pl);    \
    Value (de)     = STRING(de);    \
    Value (pt-BR)     = STRING(pt-BR); \
    Value (th)     = STRING(th);    \
    Value (et)     = STRING(et);    \
    Value (ja)     = STRING(ja);    \
    Value (ko)     = STRING(ko);    \
    Value (zh-CN)     = STRING(zh-CN); \
    Value (zh-TW)     = STRING(zh-TW); \
    Value (tr)     = STRING(tr);    \
    Value (hi-IN)     = STRING(hi-IN); \
    Value (ar)     = STRING(ar);    \
    Value (he)     = STRING(he);    \
    Value (99)     = STRING(ISO_CODE)

#define KEY_ALL_LANG_LETTER \
    Key (en-US)       = STRING(en-US); \
    Key (pt)       = STRING(pt);    \
    Key (ru)       = STRING(ru);    \
    Key (el)       = STRING(el);    \
    Key (nl)       = STRING(nl);    \
    Key (fr)       = STRING(fr);    \
    Key (es)       = STRING(es);    \
    Key (fi)       = STRING(fi);    \
    Key (hu)       = STRING(hu);    \
    Key (ca)       = STRING(ca);    \
    Key (sl)       = STRING(sl);    \
    Key (it)       = STRING(it);    \
    Key (cs)       = STRING(cs);    \
    Key (sk)       = STRING(sk);    \
    Key (en-GB)       = STRING(en-GB); \
    Key (da)       = STRING(da);    \
    Key (sv)       = STRING(sv);    \
    Key (no)       = STRING(no);    \
    Key (pl)       = STRING(pl);    \
    Key (de)       = STRING(de);    \
    Key (pt-BR)       = STRING(pt-BR); \
    Key (th)       = STRING(th);    \
    Key (et)       = STRING(et);    \
    Key (ja)       = STRING(ja);    \
    Key (ko)       = STRING(ko);    \
    Key (zh-CN)       = STRING(zh-CN); \
    Key (zh-TW)       = STRING(zh-TW); \
    Key (tr)       = STRING(tr);    \
    Key (hi-IN)       = STRING(hi-IN); \
    Key (ar)       = STRING(ar);    \
    Key (he)       = STRING(he);    \
    Key (99)       = STRING(ISO_CODE)

#define FILE_ALL_LANG_LETTER(name, ext) \
    Name (01)       = CONFIGLANGFILENAME(name,en-US,ext); \
    Name (03)       = CONFIGLANGFILENAME(name,pt,ext);    \
    Name (07)       = CONFIGLANGFILENAME(name,ru,ext);    \
    Name (30)       = CONFIGLANGFILENAME(name,el,ext);    \
    Name (31)       = CONFIGLANGFILENAME(name,nl,ext);    \
    Name (33)       = CONFIGLANGFILENAME(name,fr,ext);    \
    Name (34)       = CONFIGLANGFILENAME(name,es,ext);    \
    Name (35)       = CONFIGLANGFILENAME(name,fi,ext);    \
    Name (36)       = CONFIGLANGFILENAME(name,hu,ext);    \
    Name (37)       = CONFIGLANGFILENAME(name,ca,ext);    \
    Name (50)       = CONFIGLANGFILENAME(name,sl,ext);    \
    Name (39)       = CONFIGLANGFILENAME(name,it,ext);    \
    Name (42)       = CONFIGLANGFILENAME(name,cs,ext);    \
    Name (43)       = CONFIGLANGFILENAME(name,sk,ext);    \
    Name (44)       = CONFIGLANGFILENAME(name,en-GB,ext); \
    Name (45)       = CONFIGLANGFILENAME(name,da,ext);    \
    Name (46)       = CONFIGLANGFILENAME(name,sv,ext);    \
    Name (47)       = CONFIGLANGFILENAME(name,no,ext);    \
    Name (48)       = CONFIGLANGFILENAME(name,pl,ext);    \
    Name (49)       = CONFIGLANGFILENAME(name,de,ext);    \
    Name (55)       = CONFIGLANGFILENAME(name,pt-BR,ext); \
    Name (66)       = CONFIGLANGFILENAME(name,th,ext);    \
    Name (77)       = CONFIGLANGFILENAME(name,et,ext);    \
    Name (81)       = CONFIGLANGFILENAME(name,ja,ext);    \
    Name (82)       = CONFIGLANGFILENAME(name,ko,ext);    \
    Name (86)       = CONFIGLANGFILENAME(name,zh-CN,ext); \
    Name (88)       = CONFIGLANGFILENAME(name,zh-TW,ext); \
    Name (90)       = CONFIGLANGFILENAME(name,tr,ext);    \
    Name (91)       = CONFIGLANGFILENAME(name,hi-IN,ext); \
    Name (96)       = CONFIGLANGFILENAME(name,ar,ext);    \
    Name (97)       = CONFIGLANGFILENAME(name,he,ext);    \
    Name (99)       = CONFIGLANGFILENAME(name,ISO_CODE,ext)

#endif // MACROS_INC

