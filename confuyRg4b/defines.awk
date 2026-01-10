BEGIN {
D["PACKAGE_NAME"]=" \"LibreOffice\""
D["PACKAGE_TARNAME"]=" \"libreoffice\""
D["PACKAGE_VERSION"]=" \"26.8.0.0.alpha0+\""
D["PACKAGE_STRING"]=" \"LibreOffice 26.8.0.0.alpha0+\""
D["PACKAGE_BUGREPORT"]=" \"\""
D["PACKAGE_URL"]=" \"http://documentfoundation.org/\""
D["LIBO_VERSION_MAJOR"]=" 26"
D["LIBO_VERSION_MINOR"]=" 8"
D["LIBO_VERSION_MICRO"]=" 0"
D["LIBO_VERSION_PATCH"]=" 0"
D["LIBO_THIS_YEAR"]=" 2026"
D["SRCDIR"]=" \"C:/Users/philh/dev/officelabs-suite/officelabs-master/libreoffice-fork\""
D["SRC_ROOT"]=" \"C:/Users/philh/dev/officelabs-suite/officelabs-master/libreoffice-fork\""
D["BUILDDIR"]=" \"C:/Users/philh/dev/officelabs-suite/officelabs-master/libreoffice-fork\""
D["HAVE_DLAPI"]=" 1"
D["HAVE_FEATURE_DESKTOP"]=" 1"
D["HAVE_FEATURE_MULTIUSER_ENVIRONMENT"]=" 1"
D["HAVE_FEATURE_AVMEDIA"]=" 1"
D["HAVE_FEATURE_DBCONNECTIVITY"]=" 1"
D["HAVE_FEATURE_SCRIPTING"]=" 1"
D["HAVE_FEATURE_EXTENSIONS"]=" 1"
D["LIBO_BIN_FOLDER"]=" \"program\""
D["LIBO_ETC_FOLDER"]=" \"program\""
D["LIBO_LIBEXEC_FOLDER"]=" \"program\""
D["LIBO_LIB_FOLDER"]=" \"program\""
D["LIBO_LIB_PYUNO_FOLDER"]=" \"program\""
D["LIBO_SHARE_FOLDER"]=" \"share\""
D["LIBO_SHARE_HELP_FOLDER"]=" \"help\""
D["LIBO_SHARE_DOTNET_FOLDER"]=" \"program/dotnet\""
D["LIBO_SHARE_JAVA_FOLDER"]=" \"program/classes\""
D["LIBO_SHARE_PRESETS_FOLDER"]=" \"presets\""
D["LIBO_SHARE_RESOURCE_FOLDER"]=" \"program/resource\""
D["LIBO_SHARE_SHELL_FOLDER"]=" \"program/shell\""
D["LIBO_URE_BIN_FOLDER"]=" \"program\""
D["LIBO_URE_ETC_FOLDER"]=" \"program\""
D["LIBO_URE_LIB_FOLDER"]=" \"program\""
D["LIBO_URE_MISC_FOLDER"]=" \"program\""
D["LIBO_URE_SHARE_JAVA_FOLDER"]=" \"program/classes\""
D["HAVE_FEATURE_UI"]=" 1"
D["SDKDIR"]=" \"C:/Users/philh/dev/officelabs-suite/officelabs-master/libreoffice-fork/instdir/sdk\""
D["WORKDIR"]=" \"C:/Users/philh/dev/officelabs-suite/officelabs-master/libreoffice-fork/workdir\""
D["HAVE_FEATURE_XMLHELP"]=" 1"
D["ENABLE_PCH"]=" 1"
D["SAL_TYPES_SIZEOFSHORT"]=" 2"
D["SAL_TYPES_SIZEOFINT"]=" 4"
D["SAL_TYPES_SIZEOFLONG"]=" 4"
D["SAL_TYPES_SIZEOFLONGLONG"]=" 8"
D["SAL_TYPES_SIZEOFPOINTER"]=" 8"
D["SAL_TYPES_ALIGNMENT2"]=" 2"
D["SAL_TYPES_ALIGNMENT4"]=" 4"
D["SAL_TYPES_ALIGNMENT8"]=" 8"
D["HAVE_FEATURE_JUMBO_SHEETS"]=" 1"
D["ENABLE_RUNTIME_OPTIMIZATIONS"]=" 1"
D["HAVE_P1155R3"]=" 1"
D["ENABLE_EOT"]=" 1"
D["WPS_VERSION_MAJOR"]=" 0"
D["WPS_VERSION_MINOR"]=" 4"
D["WPS_VERSION_MICRO"]=" 14"
D["MWAW_VERSION_MAJOR"]=" 0"
D["MWAW_VERSION_MINOR"]=" 3"
D["MWAW_VERSION_MICRO"]=" 21"
D["ETONYEK_VERSION_MAJOR"]=" 0"
D["ETONYEK_VERSION_MINOR"]=" 1"
D["ETONYEK_VERSION_MICRO"]=" 10"
D["EBOOK_VERSION_MAJOR"]=" 0"
D["EBOOK_VERSION_MINOR"]=" 1"
D["EBOOK_VERSION_MICRO"]=" 2"
D["STAROFFICE_VERSION_MAJOR"]=" 0"
D["STAROFFICE_VERSION_MINOR"]=" 0"
D["STAROFFICE_VERSION_MICRO"]=" 7"
D["PYTHON_VERSION_STRING"]=" L\"3.13.11\""
D["ENABLE_LIBRELOGO"]=" 1"
D["HAVE_FEATURE_NSS"]=" 1"
D["USE_TLS_NSS"]=" 1"
D["HAVE_FEATURE_CURL"]=" 1"
D["USE_CRYPTO_MSCAPI"]=" 1"
D["ENABLE_ZXING"]=" 1"
D["HAVE_FEATURE_OPENGL"]=" 1"
D["HAVE_FEATURE_OPENCL"]=" 1"
D["HAVE_FEATURE_PDFIMPORT"]=" 1"
D["HAVE_FEATURE_POPPLER"]=" 1"
D["ENABLE_PDFIMPORT"]=" 1"
D["HAVE_FEATURE_GPGME"]=" 1"
D["HAVE_MORE_FONTS"]=" 1"
D["ENABLE_EXTENSION_UPDATE"]=" 1"
D["WITH_LANG"]=" \"\""
D["WITH_LOCALE_ALL"]=" 1"
D["EXTRA_BUILDID"]=" \"\""
D["OOO_VENDOR"]=" \"philh\""
D["GDRIVE_CLIENT_ID"]=" \"\""
D["GDRIVE_CLIENT_SECRET"]=" \"\""
D["ALFRESCO_CLOUD_CLIENT_ID"]=" \"\""
D["ALFRESCO_CLOUD_CLIENT_SECRET"]=" \"\""
D["ONEDRIVE_CLIENT_ID"]=" \"\""
D["ONEDRIVE_CLIENT_SECRET"]=" \"\""
D["HAVE_CPP_STRONG_ORDER"]=" 1"
  for (key in D) D_is_set[key] = 1
  FS = ""
}
/^[\t ]*#[\t ]*(define|undef)[\t ]+[_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ][_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789]*([\t (]|$)/ {
  line = $ 0
  split(line, arg, " ")
  if (arg[1] == "#") {
    defundef = arg[2]
    mac1 = arg[3]
  } else {
    defundef = substr(arg[1], 2)
    mac1 = arg[2]
  }
  split(mac1, mac2, "(") #)
  macro = mac2[1]
  prefix = substr(line, 1, index(line, defundef) - 1)
  if (D_is_set[macro]) {
    # Preserve the white space surrounding the "#".
    print prefix "define", macro P[macro] D[macro]
    next
  } else {
    # Replace #undef with comments.  This is necessary, for example,
    # in the case of _POSIX_SOURCE, which is predefined and required
    # on some systems where configure will not decide to define it.
    if (defundef == "undef") {
      print "/*", prefix defundef, macro, "*/"
      next
    }
  }
}
{ print }
