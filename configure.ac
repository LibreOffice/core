# -*- Mode: Autoconf; tab-width: 4; indent-tabs-mode: nil -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])

AC_INIT([coolwsd], [26.04.0.1], [https://github.com/CollaboraOnline/online/issues], [coolwsd], [https://collaboraonline.github.io/])
LT_INIT([shared, disable-static, dlopen])

AM_INIT_AUTOMAKE([1.10 foreign subdir-objects tar-pax -Wno-portability])

# Drop the 'u' from automake's default 'cru' to silence the warning:
# "ar: `u' modifier ignored since `D' is the default (see `U')"
# Don't use 'crD' as the 'D' flag is GNU-specific and unsupported on macOS/FreeBSD.
AC_SUBST([ARFLAGS], [cr])

AC_CONFIG_MACRO_DIR([m4])

# We don't want to require pkg-config and PKG_CHECK_MODULES on macOS
# It may be missing, and then we'd generate broken shell code; but
# let's guard against accidental use.
m4_if(m4_esyscmd_s([uname -s]),Darwin,[m4_define([PKG_CHECK_MODULES],[AC_MSG_ERROR([It's not expected to use [PKG_CHECK_MODULES] on macOS, please fix configure.ac and make pkg-config a requirement if you really need it.])])],[])

COOLWSD_VERSION_MAJOR=`echo $VERSION | awk -F. '{print $1}'`
COOLWSD_VERSION_MINOR=`echo $VERSION | awk -F. '{print $2}'`
COOLWSD_VERSION_MICRO=`echo $VERSION | awk -F. '{print $3}'`
COOLWSD_VERSION_NANO=`echo $VERSION | awk -F. '{print $4}'`

ANDROID_PACKAGE_VERSIONCODE=
if test "$enable_androidapp" = "yes"; then
    AC_MSG_CHECKING([for Android package versionCode])
    if test -z "$with_android_package_versioncode" -o "$with_android_package_versioncode" = "no"; then
        ANDROID_PACKAGE_VERSIONCODE="1"
        AC_MSG_RESULT([not set, using $ANDROID_PACKAGE_VERSIONCODE])
    else
        COOLWSD_VERSION_MAJOR=`echo $with_android_package_versioncode | awk -F. '{print $1}'`
        COOLWSD_VERSION_MINOR=`echo $with_android_package_versioncode | awk -F. '{print $2}'`
        COOLWSD_VERSION_MICRO=`echo $with_android_package_versioncode | awk -F. '{print $3}'`
        COOLWSD_VERSION_NANO=`echo $with_android_package_versioncode | awk -F. '{print $4}' | awk -F- '{print $1}'`

        ANDROID_PACKAGE_VERSIONCODE=`echo $with_android_package_versioncode | awk -F- '{print $2}'`
        AC_MSG_RESULT([$ANDROID_PACKAGE_VERSIONCODE])
    fi
fi
AC_SUBST(ANDROID_PACKAGE_VERSIONCODE)

COOLWSD_VERSION="$COOLWSD_VERSION_MAJOR.$COOLWSD_VERSION_MINOR.$COOLWSD_VERSION_MICRO.$COOLWSD_VERSION_NANO"

AC_SUBST([COOLWSD_VERSION_MAJOR])
AC_SUBST([COOLWSD_VERSION_MINOR])
AC_SUBST([COOLWSD_VERSION_MICRO])
AC_SUBST([COOLWSD_VERSION_NANO])
AC_SUBST([COOLWSD_VERSION])

AC_DEFINE_UNQUOTED([COOLWSD_VERSION],[["$COOLWSD_VERSION"]],[Collabora Online WebSocket server version])

# try to add a git hash for a version if we're developing
COOLWSD_VERSION_HASH="$COOLWSD_VERSION"
git_hash=`cd ${srcdir} && ( cat dist_git_hash 2> /dev/null || git log -1 --format=%h --abbrev=10 2> /dev/null )`
if test "z$git_hash" != "z"; then
   COOLWSD_VERSION_HASH=$git_hash
fi
AC_SUBST([COOLWSD_VERSION_HASH])

AC_DEFINE_UNQUOTED([COOLWSD_VERSION_HASH],[["$COOLWSD_VERSION_HASH"]],[Collabora Online git hash if present])

AC_CONFIG_SRCDIR([wsd/COOLWSD.cpp])

AC_DEFINE_UNQUOTED([COOLWSD_BUILDCONFIG],[["$ac_configure_args"]],[Options passed to configure script])

# this is a dummy header, autoheader adds all entries to it
# don't include config_unused.h anywhere!
AC_CONFIG_HEADERS([config_unused.h])

AM_CONDITIONAL([X86], [test "$host_cpu" = x86 || test "$host_cpu" = x86_64])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LN_S

# override AC_PROG_CXX:
# avoid libtool from adding -rpath when linking
if test "$host_os" = "emscripten"; then
    hardcode_libdir_flag_spec=
    hardcode_libdir_flag_spec_CXX=
fi

AC_LANG_PUSH([C++])

AC_MSG_CHECKING([whether $CXX is actually Clang])
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
    #if !defined __clang__
    #error
    #endif
    ]])],
    [AC_MSG_RESULT([yes])
     cxx_clang=true],
    [AC_MSG_RESULT([no])
     cxx_clang=false])
AM_CONDITIONAL([CXX_CLANG], [test "$cxx_clang" = true])

# check for python modules
for MODULE in lxml polib; do
    AC_MSG_CHECKING([for ${MODULE} for python3])
    if /usr/bin/env python3 -c "import ${MODULE}" 2> /dev/null ; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
        AS_IF([test `uname -s` = Linux -o `uname -s` = FreeBSD -o `uname -s` = Darwin],
              [AC_MSG_ERROR([${MODULE} for python3 is needed. It might be in a package called python3-${MODULE}.])],
              [AC_MSG_WARN([${MODULE} for python3 is needed. It might be in a package called python3-${MODULE}. But
               if you are building the JS bits on another (Linux) machine, that doesn't matter])])
    fi
done

# On some platforms, std::atomic needs a helper library
AC_MSG_CHECKING([for the need of -latomic])
AC_LINK_IFELSE([AC_LANG_SOURCE([
    #include <atomic>
    #include <cstdint>
    std::atomic<std::int64_t> v;
    int main() {
      return v;
    }
    ])],
               [AC_MSG_RESULT([no])],
               [AC_MSG_RESULT([yes])
                LIBS="$LIBS -latomic"])

# Declare options
AC_ARG_ENABLE([debug],
              AS_HELP_STRING([--enable-debug],
                             [Enable debugging, link with debugging version of Poco libraries]))

AC_ARG_ENABLE([hardening-flags],
              AS_HELP_STRING([--disable-hardening-flags],
                             [Disable detection and use of compiler hardening flags]))

AC_ARG_ENABLE([bundle],
              AS_HELP_STRING([--enable-bundle],
                             [Enable creating bundled JS and CSS even with --enable-debug]))

AC_ARG_ENABLE([debug-protocol],
    AS_HELP_STRING([--disable-debug-protocol],
        [Disables JS protocol logging by default, otherwise it is slaved to enable-debug.])
)

AC_ARG_ENABLE([browsersync],
              AS_HELP_STRING([--enable-browsersync],
                             [Don't copy files to the target directory but create the symlink to allow browsersync usage]))

AC_ARG_ENABLE([anonymization],
              AS_HELP_STRING([--enable-anonymization],
                             [Enable anonymization/obfuscation of user-data in logs]))

AC_ARG_ENABLE([iosapp],
              AS_HELP_STRING([--enable-iosapp],
                             [Use on a Mac where you will build the iOS app.]))

AC_ARG_ENABLE([macosapp],
              AS_HELP_STRING([--enable-macosapp],
                             [Use on a Mac where you will build the macOS app.]))

AC_ARG_ENABLE([windowsapp],
              AS_HELP_STRING([--enable-windowsapp],
                             [Use in WSL2 on a Windows PC where you will build the Windows app that works
                              similarly to the iOS app, from the JavaScript and the pseudo WebSocket
                              message plumbing point of view.]))

AC_ARG_WITH([wasm-fallback],
              AS_HELP_STRING([--with-wasm-fallback=<path>],
                             [Build a COOL where the client can fall back to a WASM implementation if the connection to the server fails.
                              The path is where the WASM build of Online was done.]))

AC_ARG_WITH([wasm-additional-files],
              AS_HELP_STRING([--with-wasm-additional-files=<path>],
                             [Add more files to the in-memory WASM file system in online.wasm in addition to the ones from the WASM
                              build of Collabora Office. The files will be available in the WASM in-memory file system as pathnames
                              stripping the initial prefix <path> where they are on the build system. For instance, if you give
                              --with-wasm-additional-files=$HOME/our-sample-docs, and there is a file $HOME/our-sample-docs/temp/test.docx,
                              that will be available in the online.wasm as /temp/test.docx.]))

AC_ARG_WITH([iosapp-appicon],
              AS_HELP_STRING([--with-iosapp-appicon=<path>],
                             [Point to a directory containing an icon set to use instead of the default empty one.]))

AC_ARG_WITH([iosapp-fonts],
              AS_HELP_STRING([--with-iosapp-fonts=<path>],
                             [Point to a directory containing .ttf or .otf files to be bundled in the iOS app (and
                              thus installed on the device for use of the LO core code).]))

AC_ARG_WITH([app-branding],
              AS_HELP_STRING([--with-app-branding=<path>],
                             [Point to a directory containing a branding.css file and possibly other files it references,
                              to be bundled and used by the iOS app. The directory structure is copied to
                              "browser/dist/branding" and that directory ends upp in the app bundle as "branding".]))

AC_ARG_ENABLE([qtapp],
              AS_HELP_STRING([--enable-qtapp],
                             [Use in a tree where the only purpose is to build the qt "app" that is supposed
                              to work similarly to the iOS app, from the JavaScript and the pseudo WebSocket
                              message plumbing point of view. See qt/README.]))

AC_ARG_ENABLE([qtapp-tests],
              AS_HELP_STRING([--enable-qtapp-tests],
                             [Enable the WebdriverIO test suite for the Qt app.
                              Requires --enable-qtapp and --with-atspi-driver-path.]))

AC_ARG_WITH([atspi-driver-path],
            AS_HELP_STRING([--with-atspi-driver-path=/path/to/selenium-webdriver-at-spi.py],
                           [Path to the selenium-webdriver-at-spi.py script
                            required to run the Qt app tests.]))

AC_ARG_ENABLE(fuzzers,
    AS_HELP_STRING([--enable-fuzzers@<:@=ENGINE@:>@],
        [Build fuzzer targets. ENGINE is 'libfuzzer' (default) or 'afl'.
         Useful in a separate build tree since it disables coolwsd.
         For best results: --with-sanitizer=address,undefined])
)

AC_ARG_ENABLE([androidapp],
              AS_HELP_STRING([--enable-androidapp],
                             [Use in a tree where the only purpose is to build the Android app that is supposed
                              to work similarly to the iOS app, from the JavaScript and the pseudo WebSocket
                              message plumbing point of view.]))

AC_ARG_ENABLE([android-google-play],
              AS_HELP_STRING([--enable-android-google-play],
                             [When enabled, the app encourages the user periodically to rate the app on Google Play Store.]))

AC_ARG_WITH(android-package-versioncode,
            AS_HELP_STRING([--with-android-package-versioncode=6.7.8-23],
                           [Set the versionCode for the Android package.]),
,)

AC_ARG_WITH(android-abi,
            AS_HELP_STRING([--with-android-abi=x86_64],
                           [Allows specification of a concrete ABI that is to be built for, defaults to armeabi-v7a
                            (when only one build dir is provided in --with-lo-builddir) or to all 4 supported ABIs at
                            the same time (when there are more builddirs provided in --with-lo-builddir, separated
                            by colons).  The supported ABIs are: armeabi-v7a, arm64-v8a, x86 and x86_64.
                            Please note that you need to specify the parameters for --with-lo-builddir,
                            --with-poco-includes and --with-poco-libs in the order of armeabi-v7a:arm64-v8a:x86:x86_64.
                            For example, when you use --with-android-abi="x86_64 x86",
                            you have to specify --with-lo-builddir=::/path/to/x86/builddir:/path/to/x86-64/builddir.
                            If you are building for only a single ABI you may omit the colons.]),
,)

AC_ARG_WITH(app-package-name,
            AS_HELP_STRING([--with-app-package-name="org.collabora.app"],
                           [Set the app package name of the build.]),
,)

AC_ARG_WITH([app-name],
              AS_HELP_STRING([--with-app-name=<name>],
                             [Set the user-visible name of the app you build.]))

AC_ARG_WITH([user-id],
              AS_HELP_STRING([--with-user-id=cool],
                             [Set the account name of the user required to run the app (outside debug mode).]))

AC_ARG_WITH([icon-theme],
              AS_HELP_STRING([--with-icon-theme=<path>],
                             [Path to custom icon theme (similar to browser/images).]))

AC_ARG_WITH(vendor,
    AS_HELP_STRING([--with-vendor="John the Builder"],
        [Set vendor of the build.]),
,)

AC_ARG_WITH(info-url,
    AS_HELP_STRING([--with-info-url="https://john-the-builder.org"],
        [Set the info url of the build.]),
,)

AC_ARG_WITH(welcome-url,
    AS_HELP_STRING([--with-welcome-url=<url>],
        [Set the welcome url of the build.]),
,)

AC_ARG_ENABLE([seccomp],
              AS_HELP_STRING([--disable-seccomp],
                             [Disable use of linux/seccomp.h header when kernel on target system does not support it.
                              Beware of the security consequences!]))
AC_ARG_WITH([lokit-path],
            AS_HELP_STRING([--with-lokit-path=<path>],
                           [Path to the "include" directory with the COKit headers]))

AC_ARG_WITH([lo-path],
            AS_HELP_STRING([--with-lo-path=<path>],
                           [For Collabora Online, the path to a working installation directory
                           or instdir of Collabora Office, to be used at run-time.

                           For CODA-W, a *Windows* format path to the core build instdir, used at build-time.
                           All other configure script arguments that are paths are Unix (WSL) paths.]))

AC_ARG_WITH([lo-sourcedir],
            AS_HELP_STRING([--with-lo-sourcedir=<path>],
                           [Mandatory, and relevant, only for a Wasm build.
                            Path to a LibreOffice core source tree.]))

AC_ARG_WITH([lo-builddir],
            AS_HELP_STRING([--with-lo-builddir=<path>],
                           [Mandatory, and relevant, in a tree where building the iOS or Android app, or CODA.
                            Path to a the build directory of the LibreOffice core source tree you want to use.
                            In case of macOS, you MUST manually edit the ios/Mobile.xcodeproj/project.pbxproj
                            file and change the references to "../ios-device" to refer to the corresponding
                            LibreOffice core source directory (which typically would be the same as the build
                            directory you specify with this option).

                            For Android, if building for multiple ABIs, you should specify multiple trees in the
                            format --with-lo-builddir=/path/to/armeabi-v7a/builddir:/path/to/arm64-v8a/builddir:/path/to/x86/builddir:/path/to/x86_64/builddir.
                            You may omit any ABIs you are not building for, except for armeabi-v7a which must
                            always be provided.

                            If you are building Android for a single ABI only, you may optionally specify only
                            the core for your ABI without any colons. In this case, providing an armeabi-v7a core
                            is not a requirement.]))

AC_ARG_WITH([logfile],
            AS_HELP_STRING([--with-logfile=<path>],
                           [Path to the location of the logfile.]))

AC_ARG_WITH([trace-event-file],
            AS_HELP_STRING([--with-trace-event-file=<path>],
                           [Path to the location of the Trace Event file.]))

AC_ARG_WITH([poco-includes],
            AS_HELP_STRING([--with-poco-includes=<path>],
                           [Path to the "include" directory with the Poco
                            headers. For Android, this has the same format as --with-lo-builddir]))

AC_ARG_WITH([zstd-libs],
            AS_HELP_STRING([--with-zstd-libs=<path>],
                           [Path to the directory containing libzstd.a
                            libraries. For Android, this has the same format as --with-lo-builddir]))

AC_ARG_WITH([zstd-includes],
            AS_HELP_STRING([--with-zstd-includes=<path>],
                           [Path to the "include" directory with the zstd
                            headers. For Android, this has the same format as --with-lo-builddir]))

AC_ARG_WITH([poco-libs],
            AS_HELP_STRING([--with-poco-libs=<path>],
                           [Path to the "lib" directory with the Poco
                            libraries.  For Android, this has the same format as --with-lo-builddir]))

AC_ARG_WITH([libpng-includes],
            AS_HELP_STRING([--with-libpng-includes=<path>],
                           [Path to the "include" directory with the libpng
                            headers.  Not used on Android.]))

AC_ARG_WITH([libpng-libs],
            AS_HELP_STRING([--with-libpng-libs=<path>],
                           [Path to the "lib" directory with the libpng
                            libraries.  Not used on Android.]))

AC_ARG_WITH([zlib-includes],
            AS_HELP_STRING([--with-zlib-includes=<path>],
                           [Path to the "include" directory with the libpng
                            headers. Only used on Windows.]))

AC_ARG_WITH([zlib-libs],
            AS_HELP_STRING([--with-zlib-libs=<path>],
                           [Path to the "lib" directory with the zlib
                            libraries. Only used on Windows.]))

AC_ARG_WITH([cppunit-includes],
            AS_HELP_STRING([--with-cppunit-includes=<path>],
                           [Path to the "include" directory with the Cppunit headers]))

AC_ARG_WITH([cppunit-libs],
            AS_HELP_STRING([--with-cppunit-libs=<path>],
                           [Path to the "lib" directory with the Cppunit libraries]))

AC_ARG_ENABLE([ssl],
            AS_HELP_STRING([--disable-ssl],
                           [Compile without SSL support]))

AC_ARG_WITH([support-public-key],
            AS_HELP_STRING([--with-support-public-key=<public-key-name.pub>],
                [Implements signed key with expiration required for support. Targeted at Collabora Online Service Providers.]))

AC_ARG_WITH([max-connections],
            AS_HELP_STRING([--with-max-connections],
                           [Set the limit on the total number of client connections. Def: 20, Min: 3.]))

AC_ARG_WITH([max-documents],
             AS_HELP_STRING([--with-max-documents],
                           [Set the limit on the total number of documents. Def: 10, Min: 2.]))

AC_ARG_WITH([feedback-url],
            AS_HELP_STRING([--with-feedback-url=<url>],
                           [User feedback URL.]))

AC_ARG_WITH([infobar-url],
            AS_HELP_STRING([--with-infobar-url=<url>],
                           [Infobar URL.]))

AC_ARG_WITH([coverage],
             AS_HELP_STRING([--with-coverage=<gcov>],
                            [Enable a code-coverage method. Currently only gcov is supported (works with both gcc and clang).
                            Output HTML in gcov/ directory.
                            Generate report with 'make coverage-report'. Clean coverage files with 'make coverage-clean']))

AC_ARG_WITH([sanitizer],
             AS_HELP_STRING([--with-sanitizer],
                           [Enable one or more compatible sanitizers. E.g. --with-sanitizer=address,undefined,leak]))

AC_ARG_ENABLE([logging-test-asserts],
                AS_HELP_STRING([--enable-logging-test-asserts],
                            [Enable logging of passing test assertions.]))

AC_ARG_WITH([compiler-plugins],
            AS_HELP_STRING([--with-compiler-plugins=<path>],
                [Experimental! Unlikely to work for anyone except Noel! Enable compiler plugins that will perform additional checks during
                 building.]))

AC_ARG_ENABLE(coplugin,
    AS_HELP_STRING([--enable-coplugin],
        [Enables building of a clang plugin that performs COOL-specific checks.])
)

AC_ARG_ENABLE([werror],
            AS_HELP_STRING([--disable-werror],
                          [Do not turn warnings into errors.]))

AC_ARG_ENABLE([cypress],
            AS_HELP_STRING([--enable-cypress],
                           [Enable cypress tests.]))

AC_ARG_ENABLE([tests],
            AS_HELP_STRING([--disable-tests],
                           [Do not build and run unit tests.]))

AC_ARG_WITH([help-url],
            AS_HELP_STRING([--with-help-url=<url>],
                [Set the base URL of the online help. Default is https://help.collaboraoffice.com/help.html?])
                [--without-help-url Help buttons of dialogs will be disabled, external help website will not be used.],,)

AC_ARG_WITH([dictionaries],
            AS_HELP_STRING([--with-dictionaries=<language list>],
                [List of supported languages of Writing Aids (spell checker, grammar checker, thesaurus, hyphenation)
                 on this instance. Allowing too many has negative effect on startup performance. Default: de_DE en_GB
                 en_US es_ES fr_FR it nl pt_BR pt_PT ru.]))

AC_ARG_ENABLE([feature-lock],
              AS_HELP_STRING([--enable-feature-lock],
                             [Enable control over which uno-commands/feature to lock.]))

AC_ARG_ENABLE([feature-restriction],
              AS_HELP_STRING([--enable-feature-restriction],
                             [Enable control over which uno-commands/feature to disable completely.]))

AC_ARG_ENABLE([wasm-embind-uno],
              AS_HELP_STRING([--enable-wasm-embind-uno],
                             [Enable the Embind-based UNO support. Relevant only for a Wasm
                              build.]))

AC_ARG_ENABLE([wasm-embindtest],
              AS_HELP_STRING([--enable-wasm-embindtest],
                             [Run the LibreOffice embindtest as part of running the app. Requires
                              --enable-wasm-embind-uno. Relevant only for a Wasm build.]))

AC_ARG_WITH([wasm-zetajs],
            AS_HELP_STRING([--with-wasm-zetajs=<path>],
                           [Include zetajs scripting support in the app. (Together with
                            --enable-wasm-embindtest, also run the zetajs smoketest as part of
                            running the app.) Requires --enable-wasm-embind-uno. Relevant only for a
                            Wasm build. Path to a zetajs source tree.]))

AC_ARG_ENABLE([wasm-zetajs-example],
              AS_HELP_STRING([--enable-wasm-zetajs-example],
                             [Include a zetajs scripting example in the app. Requires
                              --enable-wasm-embind-uno and --with-wasm-zetajs. Relevant only for a
                              Wasm build.]))

AC_ARG_ENABLE([experimental],
              AS_HELP_STRING([--enable-experimental],
                             [Enable experimental features and behavior]))

# Debug POCO libraries are named like 'PocoNetSSLd' instead of just 'PocoNetSSL',
# but Homebrew does not have those, so avoid this suffix for macOS
AS_IF([test "$enable_debug" = yes -a -n "$with_poco_libs" -a `uname -s` != Darwin],
      [POCO_DEBUG_SUFFIX=d],
      [POCO_DEBUG_SUFFIX=])

ENABLE_DEBUG=false
ENABLE_DEBUG_PROTOCOL=false
ENABLE_BUNDLE=true
COOLWSD_LOGLEVEL="warning"
COOLWSD_LOG_TO_FILE="false"
COOLWSD_LOGFILE="/var/log/coolwsd.log"
COOLWSD_TRACEEVENTFILE="/var/log/coolwsd.trace.json"
COOLWSD_ANONYMIZE_USER_DATA=false
COOLWSD_LOG_UICMD_TO_FILE="false"
COOLWSD_LOGFILE_UICMD="/var/log/coolwsd-ui-cmd.log"
BROWSER_LOGGING="false"
debug_msg="secure mode: product build"
anonym_msg=""
ENABLE_EXPERIMENTAL=false
experimental_msg="disabled by default"
bundle_msg="using uglified bundled JS and CSS"
LOK_LOG_ASSERTIONS=0
log_asserts_msg="disabled"
SSL_VERIFY="true"
SERVER_SIGNATURE="false"

# a reasonable default
NUM_PRESPAWN_CHILDREN=4

AC_MSG_CHECKING([whether to enable debug build])
if test "$enable_debug" = "yes"; then
   AC_MSG_RESULT([yes])
   AC_DEFINE([ENABLE_DEBUG],1,[Whether to compile in some extra debugging support code and disable some security pieces])
   AC_DEFINE([CONFIG_STATIC_TYPE],,[Static config values are disabled in debug builds to allow for overriding in tests])
   ENABLE_DEBUG=true
   ENABLE_DEBUG_0_OR_1=1
   ENABLE_DEBUG_PROTOCOL=true
   COOLWSD_LOGLEVEL="trace"
   COOLWSD_LOG_TO_FILE="true"
   COOLWSD_LOGFILE="/tmp/coolwsd.log"
   COOLWSD_TRACEEVENTFILE="/tmp/coolwsd.trace.json"
   COOLWSD_ANONYMIZE_USER_DATA=false
   COOLWSD_LOG_UICMD_TO_FILE="true"
   COOLWSD_LOGFILE_UICMD="/tmp/coolwsd-ui-cmd.log"
   BROWSER_LOGGING="true"
   debug_msg="low security debugging mode"
   SSL_VERIFY="false"
   SERVER_SIGNATURE="true"

   # helps attaching to the right process
   NUM_PRESPAWN_CHILDREN=1

   # Must use bundle.js in CODA-W because browser/dist/src gets excluded by the PostBuildEvent
   if test "$enable_windowsapp" = "yes"; then
      enable_bundle=yes
   fi

   AC_MSG_CHECKING([whether to bundle JS and CSS files])
   if test "$enable_bundle" != "yes"; then
      ENABLE_BUNDLE=false
      bundle_msg="using individual JS and CSS files"
      AC_MSG_RESULT([no ($bundle_msg)])
   else
      AC_MSG_RESULT([yes])
   fi

   AC_MSG_CHECKING([whether to enable log passing test assertions in debug build (default)])
   if test "$enable_logging_test_asserts" != "no"; then
      LOK_LOG_ASSERTIONS=1
      log_asserts_msg="enabled"
      AC_MSG_RESULT([yes (override with --disable-logging-test-asserts)])
    else
      AC_MSG_RESULT([no (to enable --enable-logging-test-asserts)])
    fi
else
    ENABLE_DEBUG_0_OR_1=0
    AC_MSG_RESULT([no (Release build)])
    AC_DEFINE([ENABLE_DEBUG],0,[Whether to compile in some extra debugging support code and disable some security pieces])
    AC_DEFINE([CONFIG_STATIC_TYPE],static,[Static config values are enabled in release builds for performance])
fi
AC_SUBST(ENABLE_DEBUG)
AC_SUBST(ENABLE_DEBUG_0_OR_1)
AC_SUBST(ENABLE_BUNDLE)
AC_SUBST(COOLWSD_LOGLEVEL)
AC_DEFINE_UNQUOTED([COOLWSD_LOGLEVEL],["$COOLWSD_LOGLEVEL"],[Default coolwsd loglevel.])
AC_SUBST(COOLWSD_LOG_TO_FILE)
AC_SUBST(COOLWSD_LOG_UICMD_TO_FILE)
AC_SUBST(BROWSER_LOGGING)
AC_SUBST(NUM_PRESPAWN_CHILDREN)
AC_DEFINE_UNQUOTED([NUM_PRESPAWN_CHILDREN],["$NUM_PRESPAWN_CHILDREN"],[Default number of pre-spawn children.])

DEBUG_ABSSRCDIR=`(cd "$srcdir" && pwd -P)`
AC_DEFINE_UNQUOTED([DEBUG_ABSSRCDIR], ["$DEBUG_ABSSRCDIR"], [Absolute path to the top-level source directory])

if test "$enable_debug_protocol" = no; then
   ENABLE_DEBUG_PROTOCOL=false
fi
AC_SUBST(ENABLE_DEBUG_PROTOCOL)

ENABLE_BROWSERSYNC=
browsersync_msg="disabled: copy files to the target directory"
if test "$enable_browsersync" = "yes"; then
   ENABLE_BROWSERSYNC=true
   browsersync_msg="enabled: create symlinks in the target directory"
fi
AC_SUBST(ENABLE_BROWSERSYNC)

if test -n "$with_logfile" ; then
   COOLWSD_LOGFILE="$with_logfile"
fi
AC_SUBST(COOLWSD_LOGFILE)
AC_DEFINE_UNQUOTED([COOLWSD_LOGFILE],["$COOLWSD_LOGFILE"],[Default location of coolwsd log file])
AC_SUBST(COOLWSD_LOGFILE_UICMD)
AC_DEFINE_UNQUOTED([COOLWSD_LOGFILE_UICMD],["$COOLWSD_LOGFILE_UICMD"],[Default location of coolwsd UI command log file])

if test -n "$with_trace_event_file" ; then
   COOLWSD_TRACEEVENTFILE="$with_trace_event_file"
fi
AC_SUBST(COOLWSD_TRACEEVENTFILE)
AC_DEFINE_UNQUOTED([COOLWSD_TRACEEVENTFILE], [["]$COOLWSD_TRACEEVENTFILE["]], [Destination for Trace Event output])

if test "$enable_anonymization" = "yes" ; then
   COOLWSD_ANONYMIZE_USER_DATA=true
   anonym_msg="anonymization of user-data is enabled"
fi
AC_DEFINE_UNQUOTED([COOLWSD_ANONYMIZE_USER_DATA],[$COOLWSD_ANONYMIZE_USER_DATA],[Enable permanent anonymization in logs])
AC_SUBST(COOLWSD_ANONYMIZE_USER_DATA)

if test "$enable_logging_test_asserts" = "yes" ; then
   LOK_LOG_ASSERTIONS=1
   log_asserts_msg="enabled"
fi
AC_DEFINE_UNQUOTED([LOK_LOG_ASSERTIONS],[$LOK_LOG_ASSERTIONS],[Enable logging of test assertions])
AC_SUBST(LOK_LOG_ASSERTIONS)

if test -z "$anonym_msg";  then
  anonym_msg="anonymization of user-data is disabled"
fi

# Disable (0) or Enable (2) exotic file formats
ENABLE_EXOTIC_FILE_FORMATS="0"

if test "$enable_experimental" = "yes" ; then
  ENABLE_EXPERIMENTAL=true
  experimental_msg="enabled by default"
  SSL_VERIFY="false"
  ENABLE_EXOTIC_FILE_FORMATS="2"
fi
AC_SUBST(ENABLE_EXPERIMENTAL)

AC_MSG_CHECKING([if ssl verification is enabled by default])
AC_MSG_RESULT([$SSL_VERIFY])
AC_DEFINE_UNQUOTED([SSL_VERIFY],["$SSL_VERIFY"],[Default SSL Verification mode])
AC_SUBST(SSL_VERIFY)

AC_MSG_CHECKING([if exotic file formats are enabled by default])
if test "$ENABLE_EXOTIC_FILE_FORMATS" = "0"; then
  AC_MSG_RESULT([no])
else
  AC_MSG_RESULT([yes])
fi
AC_SUBST(ENABLE_EXOTIC_FILE_FORMATS)

AC_SUBST(SERVER_SIGNATURE)

dnl check for a file at a path with an env-var with a given suffix
AC_DEFUN([CHK_FILE_VAR], dnl $1:env-var, $2:filename-to-match, $3:cleartext-name, $4:suffix, $5:env-var to define if found
[
   if test -n "$[$1][$4]" -o "z[$4]" = "z"; then
       found_file=""
       for f in $2; do
           if test -f "$[$1][$4]/$f"; then
               found_file="$f"
               ifelse([$5], [], [], [AS_VAR_SET([$5], [$f])])
               break
           fi
       done
       if test -n "$found_file"; then
           AC_MSG_RESULT([$[$1][$4]])
       else
           AC_MSG_ERROR([This is not a [$3] [$4] directory, none of ($2) found in $[$1][$4]])
       fi
   fi
]) dnl CHK_FILE_VAR

# macOS: When configuring for building the app itself, on macOS, we need these.
# But not when just configuring for building the JS on Linux, for copying over
# to the Mac.
# Android: We need these to setup the CMakeLists.txt properly.
LOSOURCEDIR=
LOBUILDDIR=
ANDROID_ABI="armeabi-v7a"
ANDROID_ABI_SPLIT="'armeabi-v7a'"
LOBUILDDIR_ARM64_V8A=
LOBUILDDIR_X86=
LOBUILDDIR_X86_64=
POCOINCLUDE=
POCOINCLUDE_ARM64_V8A=
POCOINCLUDE_X86=
POCOINCLUDE_X86_64=
POCOLIB=
POCOLIB_ARM64_V8A=
POCOLIB_X86=
POCOLIB_X86_64=
POCODEBUG=

ZSTDINCLUDE=
ZSTDINCLUDE_ARM64_V8A=
ZSTDINCLUDE_X86=
ZSTDINCLUDE_X86_64=
ZSTDLIB=
ZSTDLIB_ARM64_V8A=
ZSTDLIB_X86=
ZSTDLIB_X86_64=

CORE_VERSION_HASH=""
if test \( "$enable_iosapp" = "yes" -a `uname -s` = "Darwin" \) -o \( "$enable_windowsapp" = "yes" \) -o \( "$enable_androidapp" = "yes" \) -o "$host_os" = "emscripten" ; then
   if test "$enable_androidapp" = "yes" ; then
      AC_MSG_CHECKING([for Android ABI to build for])
      if test -n "$with_android_abi" ; then
         ANDROID_ABI=`echo $with_android_abi | sed 's/:/ /g'`
      else
         if echo "$with_lo_builddir" | grep -qs ':' ; then
            ANDROID_ABI="armeabi-v7a arm64-v8a x86 x86_64"
         fi
      fi
      ANDROID_ABI_SPLIT=`echo $ANDROID_ABI | sed "s/^/'/g;s/ /','/g;s/$/'/g"`
      AC_MSG_RESULT([$ANDROID_ABI])
   fi

   if test "$host_os" = emscripten; then
      AC_MSG_CHECKING([for LibreOffice source tree to build against])
      if test -z "$with_lo_sourcedir"; then
         AC_MSG_ERROR([You MUST use the --with-lo-sourcedir option when configuring a Wasm build.])
      fi
      LOSOURCEDIR=$(readlink -f "$with_lo_sourcedir")
   fi

   AC_MSG_CHECKING([for LibreOffice build tree to build against])
   if test -z "$with_lo_builddir"; then
      AC_MSG_ERROR([You MUST use the --with-lo-builddir option when configuring the mobile or Windows app build tree.])
   fi

   LOBUILDDIR="$with_lo_builddir"
   if test "$enable_androidapp" = "yes" ; then
      if echo "$LOBUILDDIR" | grep -qs ':' ; then
         LOBUILDDIR=`echo $with_lo_builddir | cut -d: -f1`
         LOBUILDDIR_ARM64_V8A=`echo $with_lo_builddir | cut -d: -f2`
         LOBUILDDIR_X86=`echo $with_lo_builddir | cut -d: -f3`
         LOBUILDDIR_X86_64=`echo $with_lo_builddir | cut -d: -f4`
      fi

      if test -z "$LOBUILDDIR_ARM64_V8A" ; then
         LOBUILDDIR_ARM64_V8A="$LOBUILDDIR"
      fi
      if test -z "$LOBUILDDIR_X86" ; then
         LOBUILDDIR_X86="$LOBUILDDIR"
      fi
      if test -z "$LOBUILDDIR_X86_64" ; then
         LOBUILDDIR_X86_64="$LOBUILDDIR"
      fi

      LOBUILDDIR_ARM64_V8A=`readlink -f "$LOBUILDDIR_ARM64_V8A"`
      LOBUILDDIR_X86=`readlink -f "$LOBUILDDIR_X86"`
      LOBUILDDIR_X86_64=`readlink -f "$LOBUILDDIR_X86_64"`
   fi

   LOBUILDDIR=`readlink -f "$LOBUILDDIR"`

   # Get the git hash of the core build.
   # Also use the setuprc or setup.ini file existence as a sanity check.
   if test "$enable_windowsapp" = "yes"; then
      setuprc=setup.ini
   else
      setuprc=setuprc
   fi

   if test -f "$LOBUILDDIR/instdir/program/$setuprc"; then
      AC_MSG_RESULT([$LOBUILDDIR])
   else
      AC_MSG_ERROR([This is not a LibreOffice core build directory: $LOBUILDDIR])
   fi

   CORE_VERSION_HASH=`grep buildid $LOBUILDDIR/instdir/program/$setuprc | sed -e 's/buildid=//' -e 's/............................$//'`

   if test -n "$LOBUILDDIR_ARM64_V8A" ; then
      if test -f "$LOBUILDDIR_ARM64_V8A/workdir/LinkTarget/StaticLibrary/liblibpng.a" ; then
         AC_MSG_RESULT([$LOBUILDDIR_ARM64_V8A])
      else
         AC_MSG_ERROR([This is not a LibreOffice ARM64 core build directory: $LOBUILDDIR_ARM64_V8A])
      fi
   fi

   if test -n "$LOBUILDDIR_X86" ; then
      if test -f "$LOBUILDDIR_X86/workdir/LinkTarget/StaticLibrary/liblibpng.a" ; then
         AC_MSG_RESULT([$LOBUILDDIR_X86])
      else
         AC_MSG_ERROR([This is not a LibreOffice x86 core build directory: $LOBUILDDIR_X86])
      fi
   fi

   if test -n "$LOBUILDDIR_X86_64" ; then
      if test -f "$LOBUILDDIR_X86_64/workdir/LinkTarget/StaticLibrary/liblibpng.a" ; then
         AC_MSG_RESULT([$LOBUILDDIR_X86_64])
      else
         AC_MSG_ERROR([This is not a LibreOffice x86-64 core build directory: $LOBUILDDIR_X86_64])
      fi
   fi

   if test "$enable_windowsapp" = "yes" ; then
      LOBUILDDIR_WINDOWS=`cd "$LOBUILDDIR" && cmd.exe /c cd | tr '\\\\' '/' | tr -d '\r'`
   fi
fi

if test \( "$enable_iosapp" = "yes" -a `uname -s` = "Darwin" \) -o \( "$enable_macosapp" = "yes" \) -o \( "$enable_windowsapp" = "yes" \) -o \( "$enable_androidapp" = "yes" \) -o "$host_os" = "emscripten" ; then
   AC_MSG_CHECKING([for Poco include directory to build against])
   if test -z "$with_poco_includes"; then
      AC_MSG_ERROR([You MUST use the --with-poco-includes option when configuring the mobile or Windows app build tree.])
   fi

   POCOINCLUDE="$with_poco_includes"
   if test "$enable_androidapp" = "yes" ; then
      if echo "$POCOINCLUDE" | grep -qs ':' ; then
         POCOINCLUDE=`echo $with_poco_includes | cut -d: -f1`
         POCOINCLUDE_ARM64_V8A=`echo $with_poco_includes | cut -d: -f2`
         POCOINCLUDE_X86=`echo $with_poco_includes | cut -d: -f3`
         POCOINCLUDE_X86_64=`echo $with_poco_includes | cut -d: -f4`
      fi

      if test -z "$POCOINCLUDE_ARM64_V8A" ; then
         POCOINCLUDE_ARM64_V8A="$POCOINCLUDE"
      fi
      if test -z "$POCOINCLUDE_X86" ; then
         POCOINCLUDE_X86="$POCOINCLUDE"
      fi
      if test -z "$POCOINCLUDE_X86_64" ; then
         POCOINCLUDE_X86_64="$POCOINCLUDE"
      fi
   fi

   # Sanity checks
   CHK_FILE_VAR(POCOINCLUDE,Poco/Poco.h,Poco include)
   CHK_FILE_VAR(POCOINCLUDE,Poco/Poco.h,Poco include,_ARM64_V8A)
   CHK_FILE_VAR(POCOINCLUDE,Poco/Poco.h,Poco include,_X86)
   CHK_FILE_VAR(POCOINCLUDE,Poco/Poco.h,Poco include,_X86_64)

   if test "$enable_windowsapp" = "yes" ; then
      POCOINCLUDE_WINDOWS=`cd "$POCOINCLUDE" && cmd.exe /c cd | tr '\\\\' '/' | tr -d '\r'`
   fi

   AC_MSG_CHECKING([for Poco lib directory to build against])
   if test -z "$with_poco_libs"; then
      AC_MSG_ERROR([You MUST use the --with-poco-libs option when configuring the mobile app build tree.])
   fi

   POCOLIB="$with_poco_libs"
   if test "$enable_androidapp" = "yes" ; then
      if echo "$POCOLIB" | grep -qs ':' ; then
         POCOLIB=`echo $with_poco_libs | cut -d: -f1`
         POCOLIB_ARM64_V8A=`echo $with_poco_libs | cut -d: -f2`
         POCOLIB_X86=`echo $with_poco_libs | cut -d: -f3`
         POCOLIB_X86_64=`echo $with_poco_libs | cut -d: -f4`
      fi

      if test -z "$POCOLIB_ARM64_V8A" ; then
         POCOLIB_ARM64_V8A="$POCOLIB"
      fi
      if test -z "$POCOLIB_X86" ; then
         POCOLIB_X86="$POCOLIB"
      fi
      if test -z "$POCOLIB_X86_64" ; then
         POCOLIB_X86_64="$POCOLIB"
      fi
   fi

   # Sanity checks
   if test "$enable_windowsapp" = "yes"; then
      CHK_FILE_VAR(POCOLIB,PocoFoundationmd.lib PocoFoundationmdd.lib,Poco lib)
   else
      CHK_FILE_VAR(POCOLIB,libPocoFoundation.a libPocoFoundation.so libPocoFoundation.dylib,Poco lib)
      CHK_FILE_VAR(POCOLIB,libPocoFoundation.a,Poco lib,_ARM64_V8A)
      CHK_FILE_VAR(POCOLIB,libPocoFoundation.a,Poco lib,_X86)
      CHK_FILE_VAR(POCOLIB,libPocoFoundation.a,Poco lib,_X86_64)
   fi

   if test "$enable_windowsapp" = "yes" ; then
      POCOLIB_WINDOWS=`cd "$POCOLIB" && cmd.exe /c cd | tr '\\\\' '/' | tr -d '\r'`
   fi

   if test "$ENABLE_DEBUG" = "true" ; then
       POCODEBUG=d
   fi

   AC_MSG_CHECKING([for zstd include directory to build against])
   if test -z "$with_zstd_includes"; then
      AC_MSG_ERROR([You MUST use the --with-zstd-includes option when configuring the mobile or Windows app build tree.])
   fi

   if test -z "$with_zstd_libs"; then
      AC_MSG_ERROR([You MUST use the --with-zstd-libs option when configuring the mobile or Windows app build tree.])
   fi

   ZSTDINCLUDE="$with_zstd_includes"
   if test "$enable_androidapp" = "yes" ; then
      if echo "$ZSTDINCLUDE" | grep -qs ':' ; then
         ZSTDINCLUDE=`echo $with_zstd_includes | cut -d: -f1`
         ZSTDINCLUDE_ARM64_V8A=`echo $with_zstd_includes | cut -d: -f2`
         ZSTDINCLUDE_X86=`echo $with_zstd_includes | cut -d: -f3`
         ZSTDINCLUDE_X86_64=`echo $with_zstd_includes | cut -d: -f4`
      fi

      if test -z "$ZSTDINCLUDE_ARM64_V8A" ; then
         ZSTDINCLUDE_ARM64_V8A="$ZSTDINCLUDE"
      fi
      if test -z "$ZSTDINCLUDE_X86" ; then
         ZSTDINCLUDE_X86="$ZSTDINCLUDE"
      fi
      if test -z "$ZSTDINCLUDE_X86_64" ; then
         ZSTDINCLUDE_X86_64="$ZSTDINCLUDE"
      fi
   fi

   # Sanity checks
   CHK_FILE_VAR(ZSTDINCLUDE,zstd.h,Zstd include)
   CHK_FILE_VAR(ZSTDINCLUDE,zstd.h,Zstd include,_ARM64_V8A)
   CHK_FILE_VAR(ZSTDINCLUDE,zstd.h,Zstd include,_X86)
   CHK_FILE_VAR(ZSTDINCLUDE,zstd.h,Zstd include,_X86_64)

   if test "$enable_windowsapp" = "yes" ; then
      ZSTDINCLUDE_WINDOWS=`cd "$ZSTDINCLUDE" && cmd.exe /c cd | tr '\\\\' '/' | tr -d '\r'`
   fi

   AC_MSG_CHECKING([for zstd lib directory to build against])

   ZSTDLIB="$with_zstd_libs"
   if test "$enable_androidapp" = "yes" ; then
      if echo "$ZSTDLIB" | grep -qs ':' ; then
         ZSTDLIB=`echo $with_zstd_libs | cut -d: -f1`
         ZSTDLIB_ARM64_V8A=`echo $with_zstd_libs | cut -d: -f2`
         ZSTDLIB_X86=`echo $with_zstd_libs | cut -d: -f3`
         ZSTDLIB_X86_64=`echo $with_zstd_libs | cut -d: -f4`
      fi

      if test -z "$ZSTDLIB_ARM64_V8A" ; then
         ZSTDLIB_ARM64_V8A="$ZSTDLIB"
      fi
      if test -z "$ZSTDLIB_X86" ; then
         ZSTDLIB_X86="$ZSTDLIB"
      fi
      if test -z "$ZSTDLIB_X86_64" ; then
         ZSTDLIB_X86_64="$ZSTDLIB"
      fi
   fi

   if test "$enable_windowsapp"; then
      CHK_FILE_VAR(ZSTDLIB,libzstd_static.lib,Zstd lib)
   else
      CHK_FILE_VAR(ZSTDLIB,libzstd.a,Zstd lib)
      CHK_FILE_VAR(ZSTDLIB,libzstd.a,Zstd lib,_ARM64_V8A)
      CHK_FILE_VAR(ZSTDLIB,libzstd.a,Zstd lib,_X86)
      CHK_FILE_VAR(ZSTDLIB,libzstd.a,Zstd lib,_X86_64)
   fi

   if test "$enable_windowsapp" = "yes" ; then
     ZSTDLIB_WINDOWS=`cd "$ZSTDLIB" && cmd.exe /c cd | tr '\\\\' '/' | tr -d '\r'`
   fi
fi

AC_SUBST(LOSOURCEDIR)
AC_SUBST(LOBUILDDIR)
AC_SUBST(LOBUILDDIR_WINDOWS)
AC_SUBST(ANDROID_ABI)
AC_SUBST(ANDROID_ABI_SPLIT)
AC_SUBST(LOBUILDDIR_ARM64_V8A)
AC_SUBST(LOBUILDDIR_X86)
AC_SUBST(LOBUILDDIR_X86_64)
AC_SUBST(POCOINCLUDE)
AC_SUBST(POCOINCLUDE_ARM64_V8A)
AC_SUBST(POCOINCLUDE_X86)
AC_SUBST(POCOINCLUDE_X86_64)
AC_SUBST(POCOINCLUDE_WINDOWS)
AC_SUBST(POCOLIB)
AC_SUBST(POCOLIB_ARM64_V8A)
AC_SUBST(POCOLIB_X86)
AC_SUBST(POCOLIB_X86_64)
AC_SUBST(POCOLIB_WINDOWS)
AC_SUBST(POCODEBUG)
AC_SUBST(ZSTDINCLUDE)
AC_SUBST(ZSTDINCLUDE_ARM64_V8A)
AC_SUBST(ZSTDINCLUDE_X86)
AC_SUBST(ZSTDINCLUDE_X86_64)
AC_SUBST(ZSTDINCLUDE_WINDOWS)
AC_SUBST(ZSTDLIB)
AC_SUBST(ZSTDLIB_ARM64_V8A)
AC_SUBST(ZSTDLIB_X86)
AC_SUBST(ZSTDLIB_X86_64)
AC_SUBST(ZSTDLIB_WINDOWS)
AC_SUBST([CORE_VERSION_HASH])

AC_DEFINE_UNQUOTED([CORE_VERSION_HASH],[["$CORE_VERSION_HASH"]],[LibreOffice core git hash if present])

LIBPNG_INCLUDES="$with_libpng_includes"
LIBPNG_LIBS="$with_libpng_libs"
LIBPNG_STATIC_LIB=""

if test "$enable_macosapp" = "yes"; then
   AC_MSG_CHECKING([for libpng include directory to build against])
   CHK_FILE_VAR(LIBPNG_INCLUDES,png.h,png.h include)
   AC_MSG_CHECKING([for static libpng lib to build against])
   CHK_FILE_VAR(LIBPNG_LIBS,libpng.a liblibpng.a,libpng,,LIBPNG_STATIC_LIB) dnl LibreOffice wrongly builds libpng as liblibpng.a
fi

if test "$enable_windowsapp" = "yes" ; then
   LIBPNG_INCLUDES_WINDOWS=`cd "$LIBPNG_INCLUDES" && cmd.exe /c cd | tr '\\\\' '/' | tr -d '\r'`
   LIBPNG_LIBS_WINDOWS=`cd "$LIBPNG_LIBS" && cmd.exe /c cd | tr '\\\\' '/' | tr -d '\r'`
fi

AC_SUBST(LIBPNG_INCLUDES)
AC_SUBST(LIBPNG_INCLUDES_WINDOWS)
AC_SUBST(LIBPNG_LIBS)
AC_SUBST(LIBPNG_LIBS_WINDOWS)
AC_SUBST(LIBPNG_STATIC_LIB)

ZLIB_INCLUDES="$with_zlib_includes"
ZLIB_LIBS="$with_zlib_libs"

if test "$enable_windowsapp" = "yes" ; then
   ZLIB_INCLUDES_WINDOWS=`cd "$ZLIB_INCLUDES" && cmd.exe /c cd | tr '\\\\' '/' | tr -d '\r'`
   ZLIB_LIBS_WINDOWS=`cd "$ZLIB_LIBS" && cmd.exe /c cd | tr '\\\\' '/' | tr -d '\r'`
fi

AC_SUBST(ZLIB_INCLUDES_WINDOWS)
AC_SUBST(ZLIB_LIBS_WINDOWS)

if test "$enable_iosapp" != yes -a "$enable_windowsapp" != yes -a "$enable_androidapp" != yes; then
   LOKIT_PATH=`readlink -f $with_lokit_path`
fi

if test "$enable_iosapp" = "yes" -o "$enable_androidapp" = "yes"; then
   if test -n "$LIBPNG_INCLUDES" ; then
      AC_MSG_ERROR([--with-libpng-includes is ignored on iOS / Android, please remove the parameter.])
   fi
   if test -n "$LIBPNG_LIBS" ; then
      AC_MSG_ERROR([--with-libpng-libs is ignored on iOS / Android, please remove the parameter.])
   fi
   if test -n "$LOKIT_PATH" ; then
      AC_MSG_ERROR([--with-lokit-path is ignored on iOS / Android, please remove the parameter.])
      LOKIT_PATH=`readlink -f $LOBUILDDIR/include`
   fi
fi

AC_SUBST(LOKIT_PATH)

ENABLE_ANDROIDAPP=
if test "$enable_androidapp" = "yes"; then
    ENABLE_ANDROIDAPP=true
fi
AC_SUBST(ENABLE_ANDROIDAPP)
AM_CONDITIONAL([ENABLE_ANDROIDAPP], [test "$ENABLE_ANDROIDAPP" = "true"])

APP_PACKAGE_NAME=
AC_MSG_CHECKING([for app package name])
if test -z "$with_app_package_name" -o "$with_app_package_name" = "no"; then
    APP_PACKAGE_NAME="org.collabora.app"
    AC_MSG_RESULT([not set, using $APP_PACKAGE_NAME])
else
    APP_PACKAGE_NAME="$with_app_package_name"
    AC_MSG_RESULT([$APP_PACKAGE_NAME])
fi
AC_SUBST(APP_PACKAGE_NAME)

if test "$enable_windowsapp" = "yes" -a -z "$with_app_name"; then
   AC_MSG_ERROR([You must use --with-app-name])
fi

APP_NAME="Collabora Online Development Edition"
if test -n "$with_app_name"; then
   APP_NAME="$with_app_name"
fi
AC_DEFINE_UNQUOTED([APP_NAME],["$APP_NAME"],[The user-visible name of the app you build.])
AC_SUBST(APP_NAME)

VENDOR=
AC_MSG_CHECKING([for vendor])
if test \( "$enable_iosapp" = yes -o "$enable_macosapp" = yes \) -a -z "$with_vendor"; then
    AC_MSG_ERROR([You must use --with-vendor when configuring for the iOS / macOS app])
fi
if test -z "$with_vendor" -o "$with_vendor" = "no"; then
    VENDOR="$USERNAME"

    if test -z "$VENDOR"; then
        VENDOR="$USER"
    fi

    if test -z "$VENDOR"; then
        VENDOR="`id -u -n`"
    fi

    AC_MSG_RESULT([not set, using $VENDOR])
else
    VENDOR="$with_vendor"
    AC_MSG_RESULT([$VENDOR])
fi
AC_SUBST(VENDOR)

INFO_URL='https://collaboraonline.github.io/'
if test -n "$with_info_url" -a "$with_info_url" != "no"; then
    INFO_URL="$with_info_url"
fi
AC_SUBST(INFO_URL)

ENABLE_IOSAPP=
IOSAPP_BUNDLE_SHORT_VERSION="$COOLWSD_VERSION_MAJOR$COOLWSD_VERSION_MINOR.$COOLWSD_VERSION_MICRO.$COOLWSD_VERSION_NANO"
IOSAPP_BUNDLE_VERSION=

if test "$enable_iosapp" = "yes"; then
   ENABLE_IOSAPP=true

   if test -f BUNDLE-VERSION; then
       IOSAPP_BUNDLE_VERSION=$(cat BUNDLE-VERSION)
   else
       IOSAPP_BUNDLE_VERSION=1
   fi
   echo $IOSAPP_BUNDLE_VERSION >BUNDLE-VERSION
fi

AC_SUBST(ENABLE_IOSAPP)
AM_CONDITIONAL([ENABLE_IOSAPP], [test "$ENABLE_IOSAPP" = "true"])
AC_SUBST(IOSAPP_BUNDLE_SHORT_VERSION)
AC_SUBST(IOSAPP_BUNDLE_VERSION)

# Check if we are on macOS (darwin) to define "MACOS" in config.h
# We want this to be general, regardless of using --enable-macosapp, and it
# could be potentially extended to handle the other defines like IOS, WASMAPP
# or _WINDOWS (but at least iOS and Windows are using their own config.h)
ENABLE_MACOS=
case "$host_os" in
    darwin*)
        ENABLE_MACOS=true
        AC_DEFINE([MACOS], [1], [Define to 1 if building for macOS])
        ;;
esac

# Are we building on macOS at all, but eg. only the coolwsd?
AC_SUBST(ENABLE_MACOS)
AM_CONDITIONAL([ENABLE_MACOS], [test "$ENABLE_MACOS" = "true"])

ENABLE_MACOSAPP=
#MACOSAPP_BUNDLE_VERSION=

if test "$enable_macosapp" = "yes"; then
   ENABLE_MACOSAPP=true

   #if test -f BUNDLE-VERSION; then
   #    MACOSAPP_BUNDLE_VERSION=$(cat BUNDLE-VERSION)
   #else
   #    MACOSAPP_BUNDLE_VERSION=1
   #fi
   #echo $MACOSAPP_BUNDLE_VERSION >BUNDLE-VERSION
fi

# Are we building the macOS CODA?
AC_SUBST(ENABLE_MACOSAPP)
AM_CONDITIONAL([ENABLE_MACOSAPP], [test "$ENABLE_MACOSAPP" = "true"])
#AC_SUBST(MACOSAPP_BUNDLE_VERSION)

ENABLE_WINDOWSAPP=

if test "$enable_windowsapp" = "yes"; then
   ENABLE_WINDOWSAPP=true
fi
AC_SUBST(ENABLE_WINDOWSAPP)
AM_CONDITIONAL([ENABLE_WINDOWSAPP], [test "$ENABLE_WINDOWSAPP" = "true"])

AC_MSG_CHECKING([for custom icon theme])
CUSTOM_ICONS_DIRECTORY=
if test -d "$with_icon_theme"; then
    CUSTOM_ICONS_DIRECTORY=`readlink -f $with_icon_theme`
    AC_MSG_RESULT([$CUSTOM_ICONS_DIRECTORY])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(CUSTOM_ICONS_DIRECTORY)

ENABLE_QTAPP=
if test "$enable_qtapp" = "yes"; then
   ENABLE_QTAPP=true
fi
AC_SUBST(ENABLE_QTAPP)
AM_CONDITIONAL([ENABLE_QTAPP], [test "$ENABLE_QTAPP" = "true"])

ENABLE_QTAPP_TESTS=false
AT_SPI_DRIVER_PATH=
if test "$enable_qtapp_tests" = "yes"; then
    if test "$ENABLE_QTAPP" != "true"; then
        AC_MSG_ERROR([--enable-qtapp-tests requires --enable-qtapp])
    fi
    if test -z "$with_atspi_driver_path"; then
        AC_MSG_ERROR([--enable-qtapp-tests requires --with-atspi-driver-path=<path>])
    fi
    if test ! -f "$with_atspi_driver_path"; then
        AC_MSG_ERROR([AT-SPI driver not found: $with_atspi_driver_path])
    fi
    AT_SPI_DRIVER_PATH="$with_atspi_driver_path"
    ENABLE_QTAPP_TESTS=true
fi
AC_SUBST(AT_SPI_DRIVER_PATH)
AM_CONDITIONAL([ENABLE_QTAPP_TESTS], [test "$ENABLE_QTAPP_TESTS" = "true"])

dnl ENABLE_WASM_FALLBACK means we are building an otherwise normal COOL but with the "WASM fallback"
dnl capability. Compare to ENABLE_WASM.

AM_CONDITIONAL([ENABLE_WASM_FALLBACK], [test -n "$with_wasm_fallback"])
WASM_BUILDDIR="$with_wasm_fallback"
AC_SUBST(WASM_BUILDDIR)
AS_IF([test -n "$with_wasm_fallback"],
      [WASM_SUPPORT_ENABLE=true
       AC_DEFINE([ENABLE_WASM_SUPPORT],1,[Define to 1 if the "fallback to Wasm" capability is enabled])],
      [WASM_SUPPORT_ENABLE=false
       AC_DEFINE([ENABLE_WASM_SUPPORT],0,[Define to 1 if the "fallback to Wasm" capability is enabled])])
AC_SUBST(WASM_SUPPORT_ENABLE)

AM_CONDITIONAL([ENABLE_WASM_ADDITIONAL_FILES], [test -n "$with_wasm_additional_files"])
WASM_ADDITIONAL_FILES="$with_wasm_additional_files"
AC_SUBST(WASM_ADDITIONAL_FILES)

AS_IF([test -n "$WASM_ADDITIONAL_FILES" && test ! -d "$WASM_ADDITIONAL_FILES"],
      [AC_MSG_ERROR([No such directory "'"$WASM_ADDITIONAL_FILES"'"])])

dnl ENABLE_WASM means that we are building the WASM "app" that will run locally in the
dnl client when necessary. ENABLE_WASM_FALLBACK and ENABLE_WASM can't both be on simultaneously.

AM_CONDITIONAL([ENABLE_WASM], [test "$host_os" = "emscripten"])
AS_IF([test "$host_os" = "emscripten"],
      [AC_DEFINE([WASMAPP],1,[Define to 1 if this is the WASM app build.])],
      [AC_DEFINE([WASMAPP],0,[Define to 1 if this is the WASM app build.])])

MAX_CONNECTIONS=9999
AS_IF([test -n "$with_max_connections" && test "$with_max_connections" -gt "0"],
      [MAX_CONNECTIONS="$with_max_connections"])
AS_IF([test "$MAX_CONNECTIONS" -lt "3"],
      [MAX_CONNECTIONS="3"])
AC_DEFINE_UNQUOTED([MAX_CONNECTIONS],[$MAX_CONNECTIONS],[Limit the maximum number of open connections])
AC_SUBST(MAX_CONNECTIONS)

MAX_DOCUMENTS=9999
AS_IF([test -n "$with_max_documents" && test "$with_max_documents" -gt "0"],
      [MAX_DOCUMENTS="$with_max_documents"])
AS_IF([test "$MAX_DOCUMENTS" -gt "$MAX_CONNECTIONS"],
      [MAX_DOCUMENTS="$MAX_CONNECTIONS"])
AS_IF([test "$MAX_DOCUMENTS" -lt "2"],
      [MAX_DOCUMENTS="2"])
AC_DEFINE_UNQUOTED([MAX_DOCUMENTS],[$MAX_DOCUMENTS],[Limit the maximum number of open documents])
AC_SUBST(MAX_DOCUMENTS)

FEEDBACK_URL=

if test -n "$with_feedback_url"; then
   FEEDBACK_URL=$with_feedback_url
fi

AC_DEFINE_UNQUOTED([FEEDBACK_URL],["$FEEDBACK_URL"],[User feedback URL.])

INFOBAR_URL=
if test -n "$with_infobar_url"; then
   INFOBAR_URL=$with_infobar_url
fi

AC_DEFINE_UNQUOTED([INFOBAR_URL],["$INFOBAR_URL"],[Infobar URL.])

WELCOME_CONFIG_FRAGMENT=
HOME_MODE_CONFIGURATION=
ENABLE_WELCOME_MESSAGE=0
WELCOME_URL=
CONFIG_INTERFACE_FRAGMENT=
if test -n "$with_welcome_url"; then
    WELCOME_URL=$with_welcome_url
    ENABLE_WELCOME_MESSAGE=1
    HOME_MODE_CONFIGURATION="<home_mode>
        <enable desc=\"Home users can enable this setting, which in turn disables welcome screen and user feedback popups, but also limits concurrent open connections to 20 and concurrent open documents to 10. The default means that number of concurrent open connections and concurrent open documents are unlimited, but welcome screen and user feedback cannot be switched off.\" type=\"bool\" default=\"false\">false</enable>
    </home_mode>"
else
    WELCOME_CONFIG_FRAGMENT="<welcome>
      <enable type=\"bool\" desc=\"Controls whether the welcome screen should be shown to the users on new install and updates. When enabled, your custom welcome screen must be created at /usr/share/coolwsd/browser/dist/welcome/welcome.html.\" default=\"false\">false</enable>
    </welcome>"
    CONFIG_INTERFACE_FRAGMENT="<brandProductName desc=\"Change the displayed name the application will have for the user.\" type=\"string\"></brandProductName>
    <brandProductURL desc=\"Change the product URL, empty makes disable the link\" type=\"string\">https://www.collaboraonline.com</brandProductURL>
    <logoURL desc=\"Change the URL of the logo used by Collabora Online, use none to hide it.\" type=\"string\"></logoURL>"
fi

AC_SUBST([WELCOME_CONFIG_FRAGMENT])
AM_SUBST_NOTMAKE(WELCOME_CONFIG_FRAGMENT)
AC_SUBST([HOME_MODE_CONFIGURATION])
AM_SUBST_NOTMAKE(HOME_MODE_CONFIGURATION)
AC_SUBST([CONFIG_INTERFACE_FRAGMENT])
AM_SUBST_NOTMAKE(CONFIG_INTERFACE_FRAGMENT)
AC_DEFINE_UNQUOTED([ENABLE_WELCOME_MESSAGE],$ENABLE_WELCOME_MESSAGE,[Should the Release notes message be shown on upgrade])
AM_CONDITIONAL([ENABLE_WELCOME_MESSAGE], [test "$ENABLE_WELCOME_MESSAGE" = "1"])
AC_DEFINE_UNQUOTED([WELCOME_URL],["$WELCOME_URL"],[The welcome url of the build.])

# Test for build environment

AS_IF([test "$ENABLE_QTAPP" != true],
      [CXXFLAGS="$CXXFLAGS -Wall -Wextra -Woverloaded-virtual -Wshadow -Wundef"])

CFLAGS="$CFLAGS -Wall -Wextra"

AC_MSG_CHECKING([whether to use extra compiler hardening flags])
enable_hardening=yes
if test "$enable_hardening_flags" = no -o "$host_os" = "emscripten" -o "x$with_sanitizer" != "x"; then
    enable_hardening=no
fi
case "$enable_fuzzers" in
    no|"") ;;
    *) enable_hardening=no ;;
esac
if test "$enable_hardening" = no; then
    AC_MSG_RESULT([no])
 else
    AC_MSG_RESULT([yes])

    AC_LANG_PUSH([C])
    AC_MSG_CHECKING([whether $CC supports -grecord-gcc-switches])
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -Werror -grecord-gcc-switches"
    AC_LINK_IFELSE(
        [AC_LANG_PROGRAM(, [[return 0;]])],
        [AC_MSG_RESULT([yes]); HARDENING_CFLAGS="$HARDENING_CFLAGS -grecord-gcc-switches"],
        [AC_MSG_RESULT([no])])
    CFLAGS=$save_CFLAGS

    AC_MSG_CHECKING([whether $CC supports -D_FORTIFY_SOURCE=2])
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -Werror -Wp,-U_FORTIFY_SOURCE,-D_FORTIFY_SOURCE=2"
    AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([[#include <string.h>]], [[return 0;]])],
        [AC_MSG_RESULT([yes]); HARDENING_CFLAGS="$HARDENING_CFLAGS -Wp,-U_FORTIFY_SOURCE,-D_FORTIFY_SOURCE=2"],
        [AC_MSG_RESULT([no])])
    CFLAGS=$save_CFLAGS

    AC_MSG_CHECKING([whether $CC supports -D_GLIBCXX_ASSERTIONS])
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -Werror -Wp,-D_GLIBCXX_ASSERTIONS"
    AC_LINK_IFELSE(
        [AC_LANG_PROGRAM(, [[return 0;]])],
        [AC_MSG_RESULT([yes]); HARDENING_CFLAGS="$HARDENING_CFLAGS -Wp,-D_GLIBCXX_ASSERTIONS"],
        [AC_MSG_RESULT([no])])
    CFLAGS=$save_CFLAGS

    AC_MSG_CHECKING([whether $CC supports -fstack-protector-strong])
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -Werror -fstack-protector-strong"
    AC_LINK_IFELSE(
        [AC_LANG_PROGRAM(, [[return 0;]])],
        [AC_MSG_RESULT([yes]); HARDENING_CFLAGS="$HARDENING_CFLAGS -fstack-protector-strong"],
        [AC_MSG_RESULT([no])])
    CFLAGS=$save_CFLAGS

    AC_MSG_CHECKING([whether $CC supports -fstack-clash-protection])
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -Werror -fstack-clash-protection"
    AC_LINK_IFELSE(
        [AC_LANG_PROGRAM(, [[return 0;]])],
        [AC_MSG_RESULT([yes]); HARDENING_CFLAGS="$HARDENING_CFLAGS -fstack-clash-protection"],
        [AC_MSG_RESULT([no])])
    CFLAGS=$save_CFLAGS

    AC_MSG_CHECKING([whether $CC supports -fcf-protection])
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -Werror -fcf-protection"
    AC_LINK_IFELSE(
        [AC_LANG_PROGRAM(, [[return 0;]])],
        [AC_MSG_RESULT([yes]); HARDENING_CFLAGS="$HARDENING_CFLAGS -fcf-protection"],
        [AC_MSG_RESULT([no])])
    CFLAGS=$save_CFLAGS

    AC_LANG_POP([C])

    CFLAGS="$CFLAGS $HARDENING_CFLAGS"
    CXXFLAGS="$CXXFLAGS $HARDENING_CFLAGS"
fi

# Code Coverage.
AC_MSG_CHECKING([whether code-coverage is enabled])
if test "x$with_coverage" != "x"; then

    AC_DEFUN([AC_PROG_LCOV], [AC_CHECK_PROG(LCOV,lcov,yes)])
    AC_PROG_LCOV
    if test x"${LCOV}" = x"yes" ; then
        AC_MSG_RESULT([yes ($with_coverage)])
        CODE_COVERAGE=1
        CXXFLAGS="$CXXFLAGS --coverage"
        CFLAGS="$CFLAGS --coverage"
        LDFLAGS="$LDFLAGS -Wl,--dynamic-list-data --coverage"
        coverage_msg="Code-Coverage is enabled"
    else
        AC_MSG_ERROR([Please install lcov (v1.16 preferred) to enable code-coverage])
    fi

else
    AC_MSG_RESULT([no])
    CODE_COVERAGE=0
    coverage_msg="Code-Coverage is disabled"
fi
AC_DEFINE_UNQUOTED([CODE_COVERAGE],[$CODE_COVERAGE],[Define to 1 if this is a code-coverage build.])
AM_CONDITIONAL([ENABLE_CODE_COVERAGE], [test "$CODE_COVERAGE" = "1"])

# Fuzzing.
AC_MSG_CHECKING([fuzzer engine])
case "$enable_fuzzers" in
    yes|libfuzzer)
        FUZZER_ENGINE=libfuzzer
        AC_MSG_RESULT([libfuzzer])
        if test "x$with_sanitizer" != "x"; then
            with_sanitizer="$with_sanitizer,fuzzer-no-link"
        else
            with_sanitizer="fuzzer-no-link"
        fi
        LIBFUZZER=1
        ;;
    afl)
        FUZZER_ENGINE=afl
        AC_MSG_RESULT([afl])
        AFL_CXX=""
        for afl_compiler in afl-clang-lto++ afl-clang-fast++ afl-c++ ; do
            if command -v $afl_compiler >/dev/null 2>&1; then
                AFL_CXX=$afl_compiler
                break
            fi
        done
        if test -z "$AFL_CXX"; then
            AC_MSG_ERROR([AFL++ compiler not found in PATH (tried afl-clang-lto++, afl-clang-fast++, afl-c++)])
        fi
        AFL_CC=$(echo "$AFL_CXX" | sed 's/++$//')
        AC_MSG_NOTICE([using AFL++ compiler: CXX=$AFL_CXX CC=$AFL_CC])
        CXX="$AFL_CXX"
        CC="$AFL_CC"
        if test "x$with_sanitizer" != "x"; then
            AC_MSG_WARN([--with-sanitizer ignored for AFL++ builds (sanitizer variant is built automatically)])
            with_sanitizer=""
        fi
        # The _san variant must use afl-clang-fast++, find it explicitly
        AFL_CLANG_FAST_CXX=""
        AFL_CLANG_FAST_CC=""
        for afl_san_compiler in afl-clang-fast++ ; do
            if command -v $afl_san_compiler >/dev/null 2>&1; then
                AFL_CLANG_FAST_CXX=$afl_san_compiler
                AFL_CLANG_FAST_CC=$(echo "$afl_san_compiler" | sed 's/++$//')
                break
            fi
        done
        if test -z "$AFL_CLANG_FAST_CXX"; then
            AC_MSG_ERROR([afl-clang-fast++ not found (required for sanitizer variant)])
        fi
        AC_MSG_NOTICE([AFL++ san compiler: $AFL_CLANG_FAST_CXX])
        # AFL++ needs libAFLDriver.a to provide main() for LLVMFuzzerTestOneInput harnesses
        AFL_DRIVER_LIB=""
        for dir in $(dirname $(command -v $AFL_CXX))/../lib/afl /usr/local/lib/afl /usr/lib/afl ; do
            if test -f "$dir/libAFLDriver.a"; then
                AFL_DRIVER_LIB="$dir/libAFLDriver.a"
                break
            fi
        done
        if test -z "$AFL_DRIVER_LIB"; then
            AC_MSG_ERROR([libAFLDriver.a not found (needed to link LLVMFuzzerTestOneInput harnesses)])
        fi
        AC_MSG_NOTICE([AFL++ driver: $AFL_DRIVER_LIB])
        LIBFUZZER=1
        ;;
    no|"")
        FUZZER_ENGINE=""
        AFL_DRIVER_LIB=""
        AC_MSG_RESULT([none])
        LIBFUZZER=0
        ;;
    *)
        AC_MSG_ERROR([unknown fuzzer engine '$enable_fuzzers' (use 'libfuzzer' or 'afl')])
        ;;
esac
AC_DEFINE_UNQUOTED([LIBFUZZER],[$LIBFUZZER],[Define to 1 if this is a fuzzer build.])
AM_CONDITIONAL([ENABLE_LIBFUZZER], [test "$LIBFUZZER" = "1"])
AM_CONDITIONAL([ENABLE_AFL], [test "$FUZZER_ENGINE" = "afl"])
AC_SUBST([FUZZER_ENGINE])
AC_SUBST([AFL_DRIVER_LIB])
AC_SUBST([AFL_CLANG_FAST_CXX])
AC_SUBST([AFL_CLANG_FAST_CC])

# Sanitizers.
CXXFLAGS_CXXSTD=
AC_MSG_CHECKING([whether to turn sanitizers on])
if test "x$with_sanitizer" != "x"; then
    AC_MSG_RESULT([yes ($with_sanitizer)])
    LIBSAN=-static-libasan
    for flag in -static-libasan -static-libsan ; do
        AC_MSG_CHECKING([whether to use $flag])
        save_CXXFLAGS=$CXXFLAGS
        CXXFLAGS="$CXXFLAGS -O1 -fno-omit-frame-pointer -fsanitize=$with_sanitizer -frtti $flag"
        AC_LANG_PUSH([C++])
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
            ]])],[CXXFLAGS_CXXSTD=$flag])
        AC_LANG_POP([C++])
        CXXFLAGS=$save_CXXFLAGS
        if test -n "$CXXFLAGS_CXXSTD"; then
            LIBSAN=$flag
            AC_MSG_RESULT([yes ($LIBSAN)])
            break
        else
            AC_MSG_RESULT([no])
        fi
    done
    SANITIZER_FLAGS="-O1 -fno-omit-frame-pointer -fsanitize=$with_sanitizer $LIBSAN"
    CXXFLAGS="$CXXFLAGS -frtti $SANITIZER_FLAGS"
    CFLAGS="$CFLAGS $SANITIZER_FLAGS"
else
    AC_MSG_RESULT([no])
fi
ENABLE_RUNTIME_OPTIMIZATIONS=
for i in $CC; do
    case $i in
    -fsanitize=*)
        ENABLE_RUNTIME_OPTIMIZATIONS=no
        break
        ;;
    esac
done
if test "$ENABLE_RUNTIME_OPTIMIZATIONS" != no; then
    AC_DEFINE([ENABLE_RUNTIME_OPTIMIZATIONS],1,[Define to 1 if this is not a sanitizers build.])
fi

# Check for C++20 support.
HAVE_CXXSTD=
CXXFLAGS_CXXSTD=
AC_MSG_CHECKING([whether $CXX supports C++20])
for flag in -std=c++20 -std=c++2a ; do
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS $flag -Werror"
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
        #include <algorithm>
        #include <functional>
        #include <vector>
        void f(std::vector<int> & v, std::function<bool(int, int)> fn) {
            std::sort(v.begin(), v.end(), fn);
        }
        ]])],[CXXFLAGS_CXXSTD=$flag])
    AC_LANG_POP([C++])
    CXXFLAGS=$save_CXXFLAGS
    if test -n "$CXXFLAGS_CXXSTD"; then
        HAVE_CXXSTD=TRUE
        break
    fi
done
if test "$HAVE_CXXSTD" = TRUE; then
    AC_MSG_RESULT([yes ($CXXFLAGS_CXXSTD)])
else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR(Compiler doesn't support C++20)
fi

# Check if we have clang, rather than gcc, which
# doesn't like -pthread on the linker command-line.
HAVE_CLANG=false
AC_MSG_CHECKING([whether the compiler is clang])
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
    #if !defined(__clang__)
    #error not clang
    #endif
])],
                    [AC_MSG_RESULT([yes])
                    AC_DEFINE([HAVE_CLANG],1,[whether the C++ compiler is clang]) HAVE_CLANG=true],
                    [AC_MSG_RESULT([no])
                    AC_DEFINE([HAVE_CLANG],0,[whether the C++ compiler is clang])])
AM_CONDITIONAL([HAVE_CLANG], [$HAVE_CLANG])
AC_SUBST(HAVE_CLANG)

CXXFLAGS="$CXXFLAGS $CXXFLAGS_CXXSTD"

AS_IF([test -n "$LOKIT_PATH"],
      [CPPFLAGS="$CPPFLAGS -I${LOKIT_PATH}"])
lokit_msg="$LOKIT_PATH"

AS_IF([test "$ENABLE_IOSAPP" != "true" -a "$ENABLE_ANDROIDAPP" != "true"],
      [AC_MSG_CHECKING([for LibreOffice path])
      if test -n "$with_lo_path"; then
          # strip trailing '/' from LO_PATH, 'ln -s' with such path will otherwise fail
          if test "$ENABLE_WINDOWSAPP" != "true"; then
              LO_PATH=`readlink -f ${with_lo_path%/}`
          else
              # LO_PATH is the path used at run-time to find the
              # LibreOffice shared libraries. When running configure
              # for CODA-W, LO_PATH will be used only at compilation
              # time. Thus it should be a Windows path and not a WSL
              # one. Additionally, because it is used by the Visual
              # Studio project in an xcopy command line, it should use
              # backslashes as path separators.
              LO_PATH="$with_lo_path"
          fi
          AC_MSG_RESULT([found])
      else
          AC_MSG_RESULT([not found])
          AS_IF([test "$LIBFUZZER" != "1"],
                [AC_MSG_ERROR([LibreOffice path must be configured: --with-lo-path])])
      fi
      ])

JAIL_PATH=not-set
SYSTEMPLATE_PATH=not-set
have_lo_path=false
AC_MSG_CHECKING([whether to run tests against a LibreOffice])
version_file="$with_lo_path/program/versionrc"
version_file_macos="$with_lo_path/Contents/Resources/versionrc"
if test -f $version_file; then
    JAILS_PATH="\${abs_top_builddir}/jails"
    CACHE_PATH="\${abs_top_builddir}/cache"
    SYSTEMPLATE_PATH="\${abs_top_builddir}/systemplate"
    have_lo_path=true
    lo_msg="test against $LO_PATH"
    AC_MSG_RESULT([yes])
elif test `uname -s` = Darwin -a -f "$version_file_macos"; then
    JAILS_PATH="\${abs_top_builddir}/jails"
    SYSTEMPLATE_PATH="\${abs_top_builddir}/systemplate"
    have_lo_path=true
    lo_msg="test against $LO_PATH"
    AC_MSG_RESULT([yes])
else
    lo_msg="no integration tests"
    AC_MSG_RESULT([no])
fi

# check if the --with-lo-path is set correctly
if test "$enable_macosapp" = "yes" ; then
    AC_MSG_CHECKING([whether --with-lo-path contains a working installation])
    lo_path_frameworks="$LO_PATH/Contents/Frameworks"
    CHK_FILE_VAR(lo_path_frameworks,libsofficeapp.dylib libmergedlo.dylib,lokit) dnl check presence of any of these dylibs
fi

AC_SUBST(LO_PATH)
AC_DEFINE_UNQUOTED([LO_PATH],["$LO_PATH"],[Path to LibreOffice installation])
AC_SUBST(JAILS_PATH)
AC_SUBST(CACHE_PATH)
AC_SUBST(SYSTEMPLATE_PATH)
AM_CONDITIONAL(HAVE_LO_PATH,[test "$have_lo_path" = "true"])

AS_IF([test -n "$with_poco_includes"],
      [CPPFLAGS="$CPPFLAGS -isystem ${with_poco_includes}"])

AS_IF([test -n "$with_poco_libs"],
      [LDFLAGS="$LDFLAGS -L${with_poco_libs}"])

AS_IF([test -n "$with_zstd_includes"],
      [CPPFLAGS="$CPPFLAGS -isystem ${with_zstd_includes}"])

AS_IF([test -n "$with_zstd_libs"],
      [LDFLAGS="$LDFLAGS -L${with_zstd_libs}"])

AS_IF([test -n "$LIBPNG_INCLUDES"],
      [CPPFLAGS="$CPPFLAGS -isystem ${LIBPNG_INCLUDES}"])

AS_IF([test -n "$LIBPNG_LIBS"],
      [LDFLAGS="$LDFLAGS -L${LIBPNG_LIBS}"])

AS_IF([test -n "$with_cppunit_includes"],
      [CPPFLAGS="$CPPFLAGS -isystem ${with_cppunit_includes}"])

AS_IF([test -n "$with_cppunit_libs"],
      [LDFLAGS="$LDFLAGS -L${with_cppunit_libs}"])

AS_IF([test \( `uname -s` = Linux -o `uname -s` = FreeBSD \) -a "$host_os" != "emscripten"],
      [AS_IF([test -n "$with_poco_libs" -a -f "$with_poco_libs/libPocoFoundation.so"],
             [LDFLAGS="$LDFLAGS -Wl,-rpath,${with_poco_libs}"])])

AS_IF([test \( `uname -s` = Linux -o `uname -s` = FreeBSD \) -a "$host_os" != "emscripten"],
      [AS_IF([test -n "$with_cppunit_libs"],
             [LDFLAGS="$LDFLAGS -Wl,-rpath,${with_cppunit_libs}"])])

AS_IF([test `uname -s` != Darwin],
      [AC_SEARCH_LIBS([dlopen],
                      [dl dld],
                      [],
                      [AC_MSG_ERROR([dlopen not found])])])

AS_IF([test "$ENABLE_IOSAPP" != "true" -a "$ENABLE_ANDROIDAPP" != "true" -a "$ENABLE_WINDOWSAPP" != "true" -a "$ENABLE_MACOSAPP" != "true" -a "$host_os" != "emscripten"],
      [PKG_CHECK_MODULES([PNG], [libpng])
dnl    deflated HTTP / web-page serving
       PKG_CHECK_MODULES([ZLIB], [zlib])
dnl    tile compression
       PKG_CHECK_MODULES([ZSTD], [libzstd >= 1.4.0])

       PKG_CHECK_MODULES([CPPUNIT], [cppunit])
       ])

AS_IF([test "$ENABLE_WINDOWSAPP" != "true"],
      [# Check if the platform supports Capabilities
       AC_CHECK_HEADERS([sys/capability.h],
           [ have_capability_h=yes ],
           [ have_capability_h=no ])

       AS_IF([test "x$have_capability_h" = xyes], [
           dnl If the header is found, proceed to look for the libcap library:
           AC_SEARCH_LIBS([cap_get_proc], [cap],
           [
               dnl If found, define a macro we can test in our code
               AC_DEFINE([HAVE_LIBCAP], [1], [Define to 1 if libcap (cap_get_proc) is available])
           ],
           [
               AC_MSG_ERROR([libcap not available?])
           ])
       ], [
           AC_MSG_WARN([sys/capability.h not found; skipping capabilities.])
       ])
      ]
)

dnl config for qtapp frontend for cool
AS_IF([test "$ENABLE_QTAPP" = true],
      [
       PKG_CHECK_MODULES([QT6],
             [Qt6Core Qt6Widgets Qt6Gui Qt6WebEngineCore Qt6WebChannel Qt6WebEngineWidgets Qt6DBus Qt6PrintSupport],,
             [AC_MSG_ERROR([Qt6 not found. Set QT6_CFLAGS and QT6_LIBS if your distro lacks .pc files.])])

       dnl Locate qtpaths6 for Qt6 tool discovery
       AC_PATH_PROG([QTPATHS6], [qtpaths6], [no],
             [$PATH$PATH_SEPARATOR/usr/lib/qt6/bin])
       if test "x$QTPATHS6" = "xno"; then
             AC_MSG_ERROR([qtpaths6 not found. Install Qt6 development tools.])
       fi
       QT_LIBEXEC_DIR=$($QTPATHS6 --qt-query QT_INSTALL_LIBEXECS 2>/dev/null)
       QT_BIN_DIR=$($QTPATHS6 --qt-query QT_INSTALL_BINS 2>/dev/null)

       AC_MSG_CHECKING([for moc])
       if test -n "$QT_LIBEXEC_DIR" -a -x "$QT_LIBEXEC_DIR/moc"; then
             MOC="$QT_LIBEXEC_DIR/moc"
             AC_MSG_RESULT([$MOC])
       else
             AC_MSG_RESULT([no])
             AC_MSG_ERROR([moc not found in $QT_LIBEXEC_DIR])
       fi
       AC_SUBST(MOC)

       AC_MSG_CHECKING([for lrelease])
       if test -n "$QT_BIN_DIR" -a -x "$QT_BIN_DIR/lrelease"; then
             LRELEASE="$QT_BIN_DIR/lrelease"
             AC_MSG_RESULT([$LRELEASE])
       else
             AC_MSG_RESULT([no])
             AC_MSG_ERROR([lrelease not found in $QT_BIN_DIR])
       fi
       AC_SUBST(LRELEASE)

       AC_MSG_CHECKING([for lupdate])
       if test -n "$QT_BIN_DIR" -a -x "$QT_BIN_DIR/lupdate"; then
             LUPDATE="$QT_BIN_DIR/lupdate"
             AC_MSG_RESULT([$LUPDATE])
       else
             AC_MSG_RESULT([no])
             AC_MSG_WARN([lupdate not found in $QT_BIN_DIR])
       fi
       AC_SUBST(LUPDATE)
])

AM_CONDITIONAL([ENABLE_DEBUG], [test "$ENABLE_DEBUG" = "true"])

mobile_app=
ENABLE_MOBILEAPP=
MOBILEAPP=0
AC_MSG_CHECKING([whether this is a mobile app])
if test "$enable_iosapp" = "yes" -o "$enable_macosapp" = "yes" -o "$enable_windowsapp" = "yes" -o "$enable_androidapp" = "yes" -o "$host_os" = "emscripten" -o "$enable_qtapp" = "yes" ; then
     AC_MSG_RESULT([yes])
     mobile_app=true;
     MOBILEAPP=1
     ENABLE_MOBILEAPP=true
     AC_DEFINE([ENABLE_CHILDROOTS],0,[Disable systemplate and jails when building CODA.])
else
     AC_DEFINE([ENABLE_CHILDROOTS],1,[Normal server, enable systemplate and jails.])
     AC_MSG_RESULT([no])
fi
AC_DEFINE_UNQUOTED([MOBILEAPP],[$MOBILEAPP],[Define to 1 if this is a mobileapp (eg. Android) build.])
AC_SUBST(ENABLE_MOBILEAPP)
AM_CONDITIONAL([ENABLE_MOBILEAPP], [test "$ENABLE_MOBILEAPP" = "true"])

if test "$host_os" = "emscripten" ; then
    ENABLE_EMSCRIPTENAPP=true
fi
AC_SUBST(ENABLE_EMSCRIPTENAPP)

has_simd=no
if test "$mobile_app" != "true"; then
    # Check for SIMD / AVX2 acceleration compile-time goodness
    SIMD_CFLAGS="-mavx2 -O3"
    AC_MSG_CHECKING([whether $CC has AVX2 SIMD support])

    save_CFLAGS=$CFLAGS
    AC_LANG_PUSH([C])
    CFLAGS="$CFLAGS $SIMD_CFLAGS"

    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
        #include <immintrin.h>
        int main () {
            __builtin_cpu_init();
            __m256i vec = _mm256_set1_epi32(0);
            (void)vec;
            return __builtin_cpu_supports ("avx2");
        }])],
        [has_simd=yes],
        [has_simd=no])

    AC_LANG_POP([C])
    CFLAGS=$save_CFLAGS

    AC_MSG_RESULT([${has_simd}])
fi

if test "${has_simd}" = "no" ; then
   SIMD_CFLAGS=
   AC_DEFINE([ENABLE_SIMD],0,[Whether to enable SIMD])
else
   AC_DEFINE([ENABLE_SIMD],1,[Whether to enable SIMD])
fi
AC_SUBST(SIMD_CFLAGS)


has_pfm=no
LIBPFM_LIBS=
if test "$mobile_app" != "true"; then
    AC_MSG_CHECKING([whether we have perf counter support])
    AC_CHECK_LIB(pfm, pfm_initialize, [has_pfm=yes], [has_pfm=no])
    AC_MSG_RESULT([${has_pfm}])
fi

if test "${has_pfm}" = "no" ; then
   AC_DEFINE([ENABLE_PFM],0,[Whether to enable PFM])
else
   AC_DEFINE([ENABLE_PFM],1,[Whether to enable PFM])
   LIBPFM_LIBS="$LIBS -lpfm"
fi
AC_SUBST(LIBPFM_LIBS)

ENABLE_SSL=true
if test "$enable_ssl" != "no" -a "$mobile_app" != "true"; then
   ssl_msg="ssl enabled. cert verification: $SSL_VERIFY"
   ENABLE_SSL=true
   AC_DEFINE([ENABLE_SSL],1,[Whether to enable SSL])
else
   if test "$mobile_app" != "true"; then
     ssl_msg="insecure: ssl disabled"
   else
     ssl_msg="invalid for mobile apps (disabled)"
   fi
   ENABLE_SSL=false
   AC_DEFINE([ENABLE_SSL],0,[Whether to enable SSL])
fi
AM_CONDITIONAL([ENABLE_SSL], [$ENABLE_SSL])
AC_SUBST(ENABLE_SSL)

AS_IF([test "$mobile_app" != "true" -a "$host_os" != "emscripten"],
      [AC_CHECK_HEADERS([security/pam_appl.h],
                        [],
                        [AC_MSG_ERROR([header security/pam_appl.h not found, install PAM development package])])
       AC_CHECK_LIB(pam, pam_start, [], [AC_MSG_ERROR([No. Install PAM development package])])])

AC_MSG_CHECKING([whether to build with support public key])
if test "x$with_support_public_key" != "x"; then
    AC_DEFINE([ENABLE_SUPPORT_KEY],1,[Whether to enable support key])

    # generate the public key include
    echo -e "#ifndef INCLUDED_SUPPORT_PUBLIC_KEY_HPP\n#define INCLUDED_SUPPORT_PUBLIC_KEY_HPP\n#include <string>\nconst static std::string SUPPORT_PUBLIC_KEY(" > "${srcdir}/common/support-public-key.hpp"
    sed 's/\(.*\)/"\1\\n"/' "$with_support_public_key" >> "${srcdir}/common/support-public-key.hpp"
    echo -e ");\n#endif" >> "${srcdir}/common/support-public-key.hpp"
    AC_MSG_RESULT([yes])
else
    AC_DEFINE([ENABLE_SUPPORT_KEY],0,[Whether to enable support key])
    rm -f "${srcdir}/common/support-public-key.hpp"
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_SUPPORT_KEY)

if test "$host_os" != "emscripten"; then
    LIBS="$LIBS -lPocoNetSSL${POCO_DEBUG_SUFFIX} -lPocoNet${POCO_DEBUG_SUFFIX} -lPocoUtil${POCO_DEBUG_SUFFIX} -lPocoJSON${POCO_DEBUG_SUFFIX} -lPocoXML${POCO_DEBUG_SUFFIX} -lPocoFoundation${POCO_DEBUG_SUFFIX} -lPocoCrypto${POCO_DEBUG_SUFFIX}"
fi

AS_IF([test "$ENABLE_SSL" = "true"],
      [LIBS="$LIBS -lssl -lcrypto"])

AS_IF([test "$ENABLE_IOSAPP" != "true" -a "$ENABLE_WINDOWSAPP" != "true" -a "$ENABLE_ANDROIDAPP" != "true" -a "$ENABLE_MACOSAPP" != "true" -a "$host_os" != "emscripten"],
      [AC_CHECK_HEADERS([COKit/COKit.h],
                        [],
                        [AC_MSG_ERROR([header COKit/COKit.h not found, perhaps you want to use --with-lokit-path])])
       AC_CHECK_HEADERS([Poco/Net/WebSocket.h],
                        [],
                        [AC_MSG_ERROR([header Poco/Net/WebSocket.h not found, perhaps you want to use --with-poco-includes])])
       DISABLE_SECCOMP=
       if test "$enable_seccomp" != "no" -a `uname -s` = "Linux"; then
           AC_CHECK_HEADERS([linux/seccomp.h],
                            [],
                            [AC_MSG_ERROR([critical security header linux/seccomp.h not found. If kernel on target system does not support SECCOMP, you can use --disable-seccomp, but mind the security consequences.])])
           AC_DEFINE([DISABLE_SECCOMP],0,[Whether to disable SECCOMP])
       else
           AC_DEFINE([DISABLE_SECCOMP],1,[Whether to disable SECCOMP])
       fi

       PKG_CHECK_MODULES([OPENSSL], [openssl])
       AC_MSG_CHECKING([whether OpenSSL is recent enough])
       AC_COMPILE_IFELSE([AC_LANG_SOURCE([
       #include <openssl/opensslv.h>

       /* OpenSSL 3.0.0 is the current mainline. */
       #if OPENSSL_VERSION_NUMBER < 0x30000000
       #error OpenSSL 3.0.0 or newer is required, override at your own risk
       #endif
       ])],
                         AC_MSG_RESULT([yes]),
                         [AC_MSG_ERROR([OpenSSL is too old. Please use a more recent version.])])

       AC_MSG_CHECKING([POCO version])
       AC_COMPILE_IFELSE([AC_LANG_SOURCE([
       #include <Poco/Version.h>
       #if POCO_VERSION < 0x010C0000
       #error Require Poco 1.12.0 or newer
       #endif
       ])],
                         [AC_MSG_RESULT([OK])],
                         [AC_MSG_ERROR([The POCO version is too old, 1.12.0 or newer required])])

       AC_MSG_CHECKING([POCO bugs])
       AC_COMPILE_IFELSE([AC_LANG_SOURCE([
       #include <Poco/Version.h>
       #if POCO_VERSION >= 0x010D0000 && POCO_VERSION < 0x010D0100
       #error known buggy poco 1.13.0
       #endif
       ])],
                         [AC_MSG_RESULT([OK])],
                         [AC_MSG_RESULT([buggy])
                          poco_known_buggy="yes"])

       # Poco headers from distro packages may need -DPOCO_UNBUNDLED
       # (the actual problem was in 1.13 Poco/XML/ParserEngine.h)
       AC_CHECK_HEADERS([Poco/SAX/SAXParser.h])
       if test "x$ac_cv_header_Poco_SAX_SAXParser_h" != xyes; then
            unset ac_cv_header_Poco_SAX_SAXParser_h
            CXXFLAGS="$CXXFLAGS -DPOCO_UNBUNDLED"
            CPPFLAGS="$CPPFLAGS -DPOCO_UNBUNDLED"
            AC_CHECK_HEADERS([Poco/SAX/SAXParser.h],
                [AC_MSG_RESULT([... with POCO_UNBUNDLED])],
                [AC_MSG_ERROR([header Poco/SAX/SAXParser.h not found, perhaps you want to use --with-poco-includes])])
       fi

       # If Poco is built with --unbundled, its static archives reference
       # system pcre2 (PocoFoundation), expat (PocoXML) and zlib (PocoZip)
       # instead of bundled copies.  Detect by trying to link with
       # Poco::RegularExpression which needs PCRE2 from PocoFoundation.
       AC_MSG_CHECKING([if we need to link with PCRE2, expat and zlib (unbundled Poco)])
       AC_LINK_IFELSE([AC_LANG_SOURCE([
       #include <Poco/RegularExpression.h>
       int main(int argc, char **argv)
       {
           (void)argc;
           Poco::RegularExpression e("abc.[[def]]");
           Poco::RegularExpression::Match m;
           return e.match(argv[[1]], m);
       }
       ])],
                      [AC_MSG_RESULT([no])],
                      [AC_MSG_RESULT([yes])
                       PKG_CHECK_MODULES([PCRE2], [libpcre2-8])
                       LIBS="$LIBS $PCRE2_LIBS -lexpat -lz"])

       ])

AS_IF([test `uname -s` = "Linux" -o `uname -s` = "FreeBSD" -o `uname -s` = "Darwin"],
      # We need npm and node unconditionally only on Linux, both in the normal Online case, and when
      # only building cool for the iOS app. When building the iOS app itself on macOS, this is
      # optional. If node and npm are not available, cool will have to be built on a Linux box.
      [AC_PATH_PROG(NODE, node, no)
       if test "$NODE" = "no"; then
            AC_MSG_ERROR([node required to build cool, but not installed])
       else
           NODE_VER=`node --version | sed 's/^v//' | awk -F. '{ print (($1 * 100) + $2) * 100 + $3;}'`
           if test "$NODE_VER" -lt 200000; then
               AC_MSG_ERROR([This node version is old, upgrade to >= 20.0.0])
           fi
       fi

       if test "$NODE" != "no"; then
           AC_PATH_PROG(NPM, npm, no)
           if test "$NPM" = "no"; then
               AC_MSG_ERROR([npm required to build cool, but not installed])
           else
               NPM_VER=`npm -v | awk -F. '{ print (($1 * 100) + $2) * 100 + $3;}'`
               if test "$NPM_VER" -lt 90000; then
                   AC_MSG_ERROR([This npm version is too old, upgrade to >= 9.0.0])
               fi
           fi
       fi

       if test "$enable_cypress" = "yes"; then
           AC_PATH_PROGS(CHROME, chromium chromium-browser chrome google-chrome brave, no)
           if test "$CHROME" = "no"; then
               AC_MSG_ERROR([chrome or chromium is required to run the JS tests, but not installed])
           fi
       fi
       ])

AS_IF([test `uname -s` = "FreeBSD"],
      [AC_CHECK_LIB(iconv, libiconv_open, [], [AC_MSG_ERROR([No. Install 3rd-party libiconv.])])],
      [AS_IF([test `uname -s` = "Darwin" -a "$ENABLE_IOSAPP" != "true"],
             [AC_CHECK_LIB(iconv, iconv_open, [], [AC_MSG_ERROR([No, please install libiconv.])])])])

AS_IF([test `uname -s` = "FreeBSD"],
      [LIBS="$LIBS -lexecinfo -lz -liconv"])

if test "x${prefix}" = "xNONE"; then
    prefix=/usr/local
fi
eval COOLWSD_CONFIGDIR="${sysconfdir}/${PACKAGE}"
AC_SUBST(COOLWSD_CONFIGDIR)
AC_DEFINE_UNQUOTED([COOLWSD_CONFIGDIR],["$COOLWSD_CONFIGDIR"],[Directory that contains configuration of coolwsd.])

COOLWSD_DATADIR=${datadir}/${PACKAGE}
AC_SUBST(COOLWSD_DATADIR)

COOL_USER_ID="cool"
if test -n "$with_user_id"; then
   COOL_USER_ID="$with_user_id"
fi
AC_DEFINE_UNQUOTED([COOL_USER_ID],["$COOL_USER_ID"],[The user-name which is allowed to run coolwsd and its tools])

AC_CONFIG_LINKS([discovery.xml:discovery.xml])
AC_CONFIG_LINKS([browser/package.json:browser/package.json])
AC_CONFIG_LINKS([browser/.eslintrc:browser/.eslintrc])
AC_CONFIG_LINKS([cypress_test/.eslintrc:cypress_test/.eslintrc])
AC_CONFIG_LINKS([browser/.prettierrc:browser/.prettierrc])
AC_CONFIG_LINKS([cypress_test/package.json:cypress_test/package.json])
AC_CONFIG_COMMANDS_PRE([
if test "$srcdir" != '.'; then
   dst_dirs="cypress_test/plugins cypress_test/eslint_plugin browser/node_shrinkpack"
   src_dir=
   dir_name=
   for dst_dir in $dst_dirs
   do
       dirname=`dirname -- "$dst_dir"`
       mkdir -p $dirname ||
             as_fn_error $? "cannot create directory $dirname"

       rm -f "$dst_dir"
       case $srcdir in
         /*)
           src_dir=$srcdir/$dst_dir
           ;;
         *)
           dir=$(dirname "$dst_dir")
           if test "$dir" = .; then
             src_dir=$srcdir/$dst_dir
           else
             src_dir=$(printf %s "$dir" | sed -e 's|[[^/]]*|..|g')/$srcdir/$dst_dir
           fi
           ;;
       esac
       echo "linking $src_dir to $dst_dir"
       ln -s "$src_dir" "$dst_dir" 2>/dev/null ||
          ln "$src_dir" "$dst_dir" 2>/dev/null ||
             cp -p "$srcdir/$dst_dir" "$dst_dir" ||
                as_fn_error $? "cannot link or copy $src_dir to $dst_dir"
   done
fi])

APP_BRANDING_DIR=
APP_IC_LAUNCHER="ic_launcher"
APP_HAS_BRANDING="false"
if test "$with_app_branding" != no -a -d "$with_app_branding"; then
    APP_BRANDING_DIR="$with_app_branding"
    APP_IC_LAUNCHER="ic_launcher_brand"
    APP_HAS_BRANDING="true"
fi
AC_SUBST(APP_BRANDING_DIR)
AC_SUBST(APP_IC_LAUNCHER)
AC_SUBST(APP_HAS_BRANDING)
AM_CONDITIONAL([APP_HAS_BRANDING], [test "$APP_HAS_BRANDING" = "true"])

APP_GOOGLE_PLAY="false"
if test "$enable_android_google_play" = "yes"; then
    APP_GOOGLE_PLAY="true"
fi
AC_SUBST(APP_GOOGLE_PLAY)

AS_IF([test "$ENABLE_IOSAPP" = "true"],
      [
       if test `uname -s` = "Darwin"; then
          IOSAPP_FONTS=''
          mkdir -p ios/Mobile/Fonts
          if test "$with_iosapp_fonts" != no -a -d "$with_iosapp_fonts"; then
              AC_MSG_NOTICE([copying fonts to be bundled])
              IOSAPP_FONTS=`find "$with_iosapp_fonts" -type f |
                            while read fname; do
                               cp "$fname" ios/Mobile/Fonts
                               echo '<string>Fonts/'$(basename $fname)'</string>'
                            done`
          fi
          rm -rf ios/Mobile/Branding
          mkdir ios/Mobile/Branding
          if test -n "$APP_BRANDING_DIR" ; then
              AC_MSG_NOTICE([copying branding files])
              mkdir -p ios/Mobile/Branding
              (cd "$APP_BRANDING_DIR" && tar cf - .) | (cd ios/Mobile/Branding && tar xf -)
          else
              # A Branding/branding.css file must exist, it is
              # referenced unconditionally in cool.html in the
              # iOS app.
              touch ios/Mobile/Branding/branding.css
          fi
          cp coolkitconfig-mobile.xcu ios/Mobile/coolkitconfig.xcu
       fi
      ])
AC_SUBST(IOSAPP_FONTS)

# TODO MACOSAPP_FONTS?

AC_CHECK_FUNCS(ppoll)
AC_CHECK_FUNCS([memrchr])
AC_CHECK_FUNCS([pipe2])

ENABLE_CYPRESS=false
if test "$enable_cypress" = "yes"; then
   cypress_msg="cypress is enabled"
   ENABLE_CYPRESS=true
else
   cypress_msg="cypress is disabled"
   ENABLE_CYPRESS=false
fi
AM_CONDITIONAL([ENABLE_CYPRESS], [$ENABLE_CYPRESS])

ENABLE_TESTS=true
if test "$enable_tests" = "no"; then
    ENABLE_TESTS=false
else
    ENABLE_TESTS=true
fi
AM_CONDITIONAL([ENABLE_TESTS], [$ENABLE_TESTS])


AC_MSG_CHECKING([for URL of online help])
if test -n "$with_help_url" -a "$with_help_url" != "no"; then
    HELP_URL="$with_help_url"
    AC_MSG_RESULT([custom: $HELP_URL])
elif test "$with_help_url" = "no"; then
    HELP_URL=
    AC_MSG_RESULT([none, help buttons are disabled])
else
    HELP_URL="https://help.collaboraoffice.com/help.html?"
    AC_MSG_RESULT([default: $HELP_URL])
fi
AC_SUBST(HELP_URL)
AC_DEFINE_UNQUOTED([HELP_URL],["$HELP_URL"],[Default value of help root URL])

AC_MSG_CHECKING([for allowed languages for Writing Aids])
if test -n "$with_dictionaries" -a "$with_dictionaries" != "no"; then
    ALLOWED_LANGUAGES="$with_dictionaries"
else
    ALLOWED_LANGUAGES="de_DE en_GB en_US es_ES fr_FR it nl pt_BR pt_PT ru"
fi
AC_MSG_RESULT([$ALLOWED_LANGUAGES])
AC_SUBST(ALLOWED_LANGUAGES)

AC_MSG_CHECKING([whether to turn warnings to errors])
if test -z "$enable_werror" -o "$enable_werror" = "yes"; then
    AC_MSG_RESULT([yes])
    CXXFLAGS="$CXXFLAGS -Werror"
    CFLAGS="$CFLAGS -Werror"
else
    AC_MSG_RESULT([no])
fi

AS_IF([test "$enable_feature_lock" = "yes"],
      [AC_DEFINE([ENABLE_FEATURE_LOCK],1,[Whether to compile and enable feature locking])],
      [AC_DEFINE([ENABLE_FEATURE_LOCK],0,[Whether to compile and enable feature locking])])

LOCKED_COMMANDS=
if test "$enable_feature_lock" = "yes"; then
    LOCKED_COMMANDS=".uno:DataDataPilotRun .uno:RecalcPivotTable \
.uno:DeletePivotTable .uno:SamplingDialog .uno:DescriptiveStatisticsDialog \
.uno:AnalysisOfVarianceDialog .uno:CorrelationDialog .uno:CovarianceDialog \
.uno:ExponentialSmoothingDialog .uno:MovingAverageDialog .uno:RegressionDialog \
.uno:TTestDialog .uno:FTestDialog .uno:ZTestDialog .uno:ChiSquareTestDialog \
.uno:FourierAnalysisDialog .uno:Validation .uno:DataFilterSpecialFilter \
.uno:TrackChanges .uno:AcceptTrackedChanges .uno:InsertReferenceField \
.uno:Watermark .uno:InsertIndexesEntry \
.uno:InsertMultiIndex .uno:SlideMasterPage exportepub downloadas-rtf masterslidebutton"
fi
AC_DEFINE_UNQUOTED([LOCKED_COMMANDS],["$LOCKED_COMMANDS"],[Default value of feature_lock.locked_commands])
AC_SUBST(LOCKED_COMMANDS)

UNLOCK_TITLE=
if test "$enable_feature_lock" = "yes"; then
    UNLOCK_TITLE="Unlock your potential"
fi
AC_DEFINE_UNQUOTED([UNLOCK_TITLE],["$UNLOCK_TITLE"],[Default value of feature_lock.unlock_title])
AC_SUBST(UNLOCK_TITLE)

UNLOCK_LINK=
if test "$enable_feature_lock" = "yes"; then
    UNLOCK_LINK="https://www.collaboraonline.com/subscriptions/"
fi
AC_DEFINE_UNQUOTED([UNLOCK_LINK],["$UNLOCK_LINK"],[Default value of feature_lock.unlock_link])
AC_SUBST(UNLOCK_LINK)

UNLOCK_DESCRIPTION=
if test "$enable_feature_lock" = "yes"; then
    UNLOCK_DESCRIPTION="Head over to the details page and discover all the features:"
fi
AC_DEFINE_UNQUOTED([UNLOCK_DESCRIPTION],["$UNLOCK_DESCRIPTION"],[Default value of feature_lock.unlock_description])
AC_SUBST(UNLOCK_DESCRIPTION)

WRITER_UNLOCK_HIGHLIGHTS=
if test "$enable_feature_lock" = "yes"; then
    WRITER_UNLOCK_HIGHLIGHTS="Review and write with ease"
fi
AC_DEFINE_UNQUOTED([WRITER_UNLOCK_HIGHLIGHTS],["$WRITER_UNLOCK_HIGHLIGHTS"],[Default value of feature_lock.writer_unlock_highlights])
AC_SUBST(WRITER_UNLOCK_HIGHLIGHTS)

CALC_UNLOCK_HIGHLIGHTS=
if test "$enable_feature_lock" = "yes"; then
    CALC_UNLOCK_HIGHLIGHTS="Get a better picture of your data"
fi
AC_DEFINE_UNQUOTED([CALC_UNLOCK_HIGHLIGHTS],["$CALC_UNLOCK_HIGHLIGHTS"],[Default value of feature_lock.calc_unlock_highlights])
AC_SUBST(CALC_UNLOCK_HIGHLIGHTS)

IMPRESS_UNLOCK_HIGHLIGHTS=
if test "$enable_feature_lock" = "yes"; then
    IMPRESS_UNLOCK_HIGHLIGHTS="Nail your next presentation"
fi
AC_DEFINE_UNQUOTED([IMPRESS_UNLOCK_HIGHLIGHTS],["$IMPRESS_UNLOCK_HIGHLIGHTS"],[Default value of feature_lock.impress_unlock_highlights])
AC_SUBST(IMPRESS_UNLOCK_HIGHLIGHTS)

DRAW_UNLOCK_HIGHLIGHTS=
if test "$enable_feature_lock" = "yes"; then
    DRAW_UNLOCK_HIGHLIGHTS="Draw and get organized"
fi
AC_DEFINE_UNQUOTED([DRAW_UNLOCK_HIGHLIGHTS],["$DRAW_UNLOCK_HIGHLIGHTS"],[Default value of feature_lock.draw_unlock_highlights])
AC_SUBST(DRAW_UNLOCK_HIGHLIGHTS)

LOCK_CONFIGURATION=
if test "$enable_feature_lock" = "yes"; then
    LOCK_CONFIGURATION="<feature_lock desc=\"Feature lock settings\">
        <locked_hosts desc=\"Allow/deny feature-locked hosts. When allowed, the below host specification overrides the CheckFileInfo response.\" allow=\"false\">
            <fallback desc=\"What to do if the given host is not covered by any rule in locked_hosts\" read_only=\"true\" disabled_commands=\"true\"/>
            <host desc=\"Regex pattern of hostname to set as full-featured or locked.\" read_only=\"false\" disabled_commands=\"false\">localhost</host>
            <host desc=\"Regex pattern of hostname to set as full-featured or locked.\" read_only=\"false\" disabled_commands=\"false\">10\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}</host>
            <host desc=\"Regex pattern of hostname to set as full-featured or locked.\" read_only=\"false\" disabled_commands=\"false\">172\\.3[01]\\.[0-9]{1,3}\\.[0-9]{1,3}</host>
        </locked_hosts>
        <is_lock_readonly desc=\"Give locked users read only access to the documents. If this is set to true it will make all the locked_host readonly who has disabled_commands set to true\" type=\"bool\" default=\"false\">false</is_lock_readonly>
        <locked_commands desc=\"List of commands to lock the functionality\" default=\"$LOCKED_COMMANDS\">$LOCKED_COMMANDS</locked_commands>
        <unlock_title desc=\"Title to show user when prompting the unlock popup\" default=\"$UNLOCK_TITLE\">$UNLOCK_TITLE</unlock_title>
        <unlock_link desc=\"Link from where user can unlock the features\" default=\"$UNLOCK_LINK\">$UNLOCK_LINK</unlock_link>
        <unlock_description desc=\"Description about unlocking features\" default=\"$UNLOCK_DESCRIPTION\">$UNLOCK_DESCRIPTION</unlock_description>
        <unlock_image desc=\"URL of image shown when unlock dialog get displayed\" default=\"\"></unlock_image>
        <writer_unlock_highlights desc=\"Short note about Writer benefits of unlocking\" default=\"$WRITER_UNLOCK_HIGHLIGHTS\">$WRITER_UNLOCK_HIGHLIGHTS</writer_unlock_highlights>
        <calc_unlock_highlights desc=\"Short note about Calc benefits of unlocking\" default=\"$CALC_UNLOCK_HIGHLIGHTS\">$CALC_UNLOCK_HIGHLIGHTS</calc_unlock_highlights>
        <impress_unlock_highlights desc=\"Short note about Impress benefits of unlocking\" default=\"$IMPRESS_UNLOCK_HIGHLIGHTS\">$IMPRESS_UNLOCK_HIGHLIGHTS</impress_unlock_highlights>
        <draw_unlock_highlights desc=\"Short note about Draw benefits of unlocking\" default=\"$DRAW_UNLOCK_HIGHLIGHTS\">$DRAW_UNLOCK_HIGHLIGHTS</draw_unlock_highlights>

        <translations>
            <!-- Example to add german language for feature lock dialog -->
            <!--
            <language name=\"de\">
                <unlock_title desc=\"Title to show user when prompting the unlock popup\" default=\"Entfesseln Sie Ihr Potenzial\">Entfesseln Sie Ihr Potenzial</unlock_title>
                <unlock_description desc=\"Description about unlocking features\" default=\"Gehen Sie zur Detailseite und entdecken Sie alle Funktionen:\">Gehen Sie zur Detailseite und entdecken Sie alle Funktionen:</unlock_description>
                <writer_unlock_highlights desc=\"Short note about Writer benefits of unlocking\" default=\"Überprüfen und schreiben Sie mit Leichtigkeit\">Überprüfen und schreiben Sie mit Leichtigkeit</writer_unlock_highlights>
                <calc_unlock_highlights desc=\"Short note about Calc benefits of unlocking\" default=\"Machen Sie sich ein besseres Bild von Ihren Daten\">Machen Sie sich ein besseres Bild von Ihren Daten</calc_unlock_highlights>
                <impress_unlock_highlights desc=\"Short note about Impress benefits of unlocking\" default=\"Bringen Sie Ihre nächste Präsentation auf den Punkt\">Bringen Sie Ihre nächste Präsentation auf den Punkt</impress_unlock_highlights>
                <draw_unlock_highlights desc=\"Short note about Draw benefits of unlocking\" default=\"Zeichne und organisiere dich\">Zeichne und organisiere dich</draw_unlock_highlights>
            </language>
            -->
        </translations>
    </feature_lock>"
fi

AC_SUBST(LOCK_CONFIGURATION)
AM_SUBST_NOTMAKE(LOCK_CONFIGURATION)

UNLOCK_LINK_PER_HOST=
if test "$enable_feature_lock" = "yes"; then
    UNLOCK_LINK_PER_HOST="<unlock_link desc=\"Link from where user can unlock the features, which will get mapped with alias_group. According to hostname coolwsd will set the unlock link for feature unlock dialog. If not specified it will fallback to feature_lock.unlock_link entry\" default=\"\"></unlock_link>"
fi

AC_SUBST(UNLOCK_LINK_PER_HOST)
AM_SUBST_NOTMAKE(UNLOCK_LINK_PER_HOST)

AS_IF([test "$enable_feature_restriction" = "yes"],
    [AC_DEFINE([ENABLE_FEATURE_RESTRICTION],1,[Whether to compile and enable feature restrictions])],
    [AC_DEFINE([ENABLE_FEATURE_RESTRICTION],0,[Whether to compile and enable feature restrictions])])

FEATURE_RESTRICTION_CONFIGURATION=
if test "$enable_feature_restriction" = "yes"; then
    FEATURE_RESTRICTION_CONFIGURATION="<restricted_commands dec=\"List of uno commands to be disabled\"></restricted_commands>"
fi

AC_SUBST(FEATURE_RESTRICTION_CONFIGURATION)
AM_SUBST_NOTMAKE(FEATURE_RESTRICTION_CONFIGURATION)

ENABLE_WASM_EMBIND_UNO=
if test "$host_os" = emscripten; then
    AC_MSG_CHECKING([whether to enable the Embind-based UNO support])
    if test "$enable_wasm_embind_uno" = yes; then
        AC_MSG_RESULT([yes])
        AC_MSG_CHECKING([whether the LibreOffice installation includes the Embind-base UNO support])
        if test ! -f "$LOBUILDDIR"/workdir/CustomTarget/static/unoembind/bindings_uno.js; then
            AC_MSG_ERROR(
                [cannot find $LOBUILDDIR/workdir/CustomTarget/static/unoembind/bindings_uno.js])
        else
            AC_MSG_RESULT([yes])
        fi
        ENABLE_WASM_EMBIND_UNO=true
    else
        AC_MSG_RESULT([no])
    fi
fi
AM_CONDITIONAL([ENABLE_WASM_EMBIND_UNO], [test "$ENABLE_WASM_EMBIND_UNO" = true])

ENABLE_WASM_EMBINDTEST=
if test "$host_os" = emscripten; then
    AC_MSG_CHECKING([whether to run the LibreOffice embindtest])
    if test "$enable_wasm_embindtest" = yes; then
        if test "$ENABLE_WASM_EMBIND_UNO" != true; then
            AC_MSG_ERROR([requires --enable-wasm-embind-uno])
        else
            AC_MSG_RESULT([yes])
            AC_MSG_CHECKING([whether the LibreOffice installation includes embindtest])
            if test ! -f "$LOBUILDDIR"/instdir/program/soffice.js.linkdeps; then
                AC_MSG_ERROR([cannot find $LOBUILDDIR/instdir/program/soffice.js.linkdeps])
            elif ! LC_ALL=POSIX grep -E -q -e '(^|[[[:space:]]])-ltestunolo($|[[[:space:]]])' \
                    "$LOBUILDDIR"/instdir/program/soffice.js.linkdeps
            then
                AC_MSG_ERROR(
                    [cannot find -ltestunolo in $LOBUILDDIR/instdir/program/soffice.js.linkdeps])
            else
                AC_MSG_RESULT([yes])
            fi
            ENABLE_WASM_EMBINDTEST=true
        fi
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_SUBST([ENABLE_WASM_EMBINDTEST])
AM_CONDITIONAL([ENABLE_WASM_EMBINDTEST], [test "$ENABLE_WASM_EMBINDTEST" = true])

ENABLE_WASM_ZETAJS=
WITH_WASM_ZETAJS=
if test "$host_os" = emscripten; then
    AC_MSG_CHECKING([whether to include zetajs scripting support])
    if test -n "$with_wasm_zetajs"; then
        if test "$ENABLE_WASM_EMBIND_UNO" != true; then
            AC_MSG_ERROR([requires --enable-wasm-embind-uno])
        else
            AC_MSG_RESULT([yes])
            ENABLE_WASM_ZETAJS=true
            WITH_WASM_ZETAJS=$with_wasm_zetajs
        fi
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_SUBST([ENABLE_WASM_ZETAJS])
AC_SUBST([WITH_WASM_ZETAJS])
AM_CONDITIONAL([ENABLE_WASM_ZETAJS], [test "$ENABLE_WASM_ZETAJS" = true])

ENABLE_WASM_ZETAJS_EXAMPLE=
if test "$host_os" = emscripten; then
    AC_MSG_CHECKING([whether to include a zetajs scripting example])
    if test "$enable_wasm_zetajs_example" = yes; then
        if test "$ENABLE_WASM_EMBIND_UNO" != true || test "$ENABLE_WASM_ZETAJS" != true; then
            AC_MSG_ERROR([requires --enable-wasm-embind-uno and --with-wasm-zetajs])
        else
            AC_MSG_RESULT([yes])
        fi
        ENABLE_WASM_ZETAJS_EXAMPLE=true
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_SUBST([ENABLE_WASM_ZETAJS_EXAMPLE])
AM_CONDITIONAL([ENABLE_WASM_ZETAJS_EXAMPLE], [test "$ENABLE_WASM_ZETAJS_EXAMPLE" = true])

AC_CONFIG_FILES([Makefile
                 cypress_test/Makefile
                 macos/Makefile
                 qt/Makefile
                 qt/run-from-build
                 wasm/Makefile
                 test/Makefile
                 browser/Makefile
                 browser/images/Makefile
                 browser/l10n/Makefile
                 browser/npm-shrinkwrap.json
                 coolkitconfig.xcu
                 coolwsd.spec
                 coolwsd.xml
                 debian/coolwsd.postinst])

# maintain config.h.in and config_version.h.in manually
# config.h.in is for configurable variables that are stable if configure parameters are unchanged
if test "$enable_iosapp" != "yes" -a "$enable_windowsapp" != "yes"; then
    AC_CONFIG_HEADERS([config.h])
fi

# config_version.h.in is for version/hash related variables
AC_CONFIG_HEADERS([config_version.h])

if test "$enable_iosapp" = "yes"; then
    AC_CONFIG_FILES([ios/config.h
                 ios/FakeQuickLook/Info.plist
                 ios/Mobile/Info.plist
                 ios/Mobile/Resources/Settings.bundle/Root.plist])
fi

if test `uname -s` = "Darwin"; then
    # Homebrew uses different paths on Intel and Apple Silicon macOS.
    # Having a separate /opt/homebrew makes more sense, but I guess
    # backward compatibility keeps it as /usr/local on Intel Macs.
    if test `uname -m` = "x86_64"; then
        HOMEBREW_PATH=/usr/local
    else
        HOMEBREW_PATH=/opt/homebrew
    fi
    AC_SUBST(HOMEBREW_PATH)
    AC_CONFIG_FILES([macos/coda/Config.xcconfig
                     macos/coolwsd/gmake-wrapper.sh],
                    [chmod +x macos/coolwsd/gmake-wrapper.sh])
fi

if test "$enable_windowsapp" = "yes"; then
    # Don't use AC_CONFIG_HEADERS for windows/coda/config.h because we
    # don't want to set random stuff that the configure script thinks
    # it has detected, as that is very misleading because it is run on
    # WSL. Ideally we shouldn't need to run the configure script when
    # building CODA-W at all. Later.
    AC_CONFIG_FILES([windows/coda/config.h
                     windows/coda/config.props])
fi

if test "$enable_androidapp" = "yes"; then
    AC_CONFIG_FILES([
                 $srcdir/android/variables.gradle:android/variables.gradle.in
                 $srcdir/android/app/appSettings.gradle:android/app/appSettings.gradle.in
                 $srcdir/android/lib/libSettings.gradle:android/lib/libSettings.gradle.in
                 $srcdir/android/lib/src/main/cpp/CMakeLists.txt:android/lib/src/main/cpp/CMakeLists.txt.in])
fi

AC_CONFIG_FILES([test/run_unit.sh],[chmod +x test/run_unit.sh])
AC_CONFIG_FILES([test/run_unit_standalone.sh],[chmod +x test/run_unit_standalone.sh])
AC_CONFIG_FILES([jsstress],[chmod +x jsstress])

AC_OUTPUT

AC_LANG_POP

AS_IF([test "$ENABLE_IOSAPP" = "true"],
      [
       if test `uname -s` = "Darwin"; then
          # Sadly I can't figure out any better way to avoid having to
          # edit the Xcode project file manually. Create symlinks to the
          # LibreOffice core source and build trees, and to the Poco
          # installation.

          AC_MSG_NOTICE([creating symbolic links for the Xcode project to use])

          if test -L lobuilddir-symlink; then
             rm -f lobuilddir-symlink
          elif test -f lobuilddir-symlink; then
             AC_MSG_ERROR([lobuilddir-symlink exists and is not a symbolic link])
          fi
          ln -s "$LOBUILDDIR" lobuilddir-symlink

          if test -L pocoinclude-symlink; then
             rm -f pocoinclude-symlink
          elif test -f pocoinclude-symlink; then
             AC_MSG_ERROR([pocoinclude-symlink exists and is not a symbolic link])
          fi
          ln -s "$POCOINCLUDE" pocoinclude-symlink

          if test -L pocolib-symlink; then
             rm -f pocolib-symlink
          elif test -f pocolib-symlink; then
             AC_MSG_ERROR([pocolib-symlink exists and is not a symbolic link])
          fi
          ln -s "$POCOLIB" pocolib-symlink

          if test -L zstdinclude-symlink; then
             rm -f zstdinclude-symlink
          elif test -f zstdinclude-symlink; then
             AC_MSG_ERROR([zstdinclude-symlink exists and is not a symbolic link])
          fi
          ln -s "$ZSTDINCLUDE" zstdinclude-symlink

          if test -L zstdlib-symlink; then
             rm -f zstdlib-symlink
          elif test -f zstdlib-symlink; then
             AC_MSG_ERROR([zstdlib-symlink exists and is not a symbolic link])
          fi
          ln -s "$ZSTDLIB" zstdlib-symlink

          eval `grep 'export ICU_MAJOR=' "$LOBUILDDIR/config_host.mk"`
          if test -L ICU.dat; then
             rm -f ICU.dat
          elif test -f ICU.dat; then
             AC_MSG_ERROR([ICU.dat exists and is not a symbolic link])
          fi
          ln -s "$LOBUILDDIR/workdir/CustomTarget/ios/resources/icudt${ICU_MAJOR}l.dat" ICU.dat

          # Sadly Xcode doesn't like it if AppIcon.appiconset is a
          # symlink, so make it a directory of symlinks to the actual png
          # and json files.
          rm -rf ios/Mobile/Assets.xcassets/AppIcon.appiconset
          rm -rf ios/FakeQuickLook/Assets.xcassets/Icon.imageset
          mkdir ios/Mobile/Assets.xcassets/AppIcon.appiconset
          mkdir ios/FakeQuickLook/Assets.xcassets/Icon.imageset
          if test -n "$with_iosapp_appicon"; then
             ln -s "$with_iosapp_appicon"/* ios/Mobile/Assets.xcassets/AppIcon.appiconset
             ln -s "$with_iosapp_appicon"/* ios/FakeQuickLook/Assets.xcassets/Icon.imageset
          else
             ln -s ios/Mobile/Assets.xcassets/Empty.appiconset/* ios/Mobile/Assets.xcassets/AppIcon.appiconset
             ln -s ios/Mobile/Assets.xcassets/Empty.appiconset/* ios/FakeQuickLook/Assets.xcassets/Icon.imageset
          fi
       fi
      ],
      [
      ])

dnl !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
dnl Set plugins.so in CPPFLAGS as the last thing - not to interfere with other checks
dnl !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

# need this after the other stuff that uses the compiler because we don't want to run configure-tests with the plugins enabled
AS_IF([test -n "$with_compiler_plugins"],
      [CPPFLAGS="$CPPFLAGS -Xclang -load -Xclang ${with_compiler_plugins}/compilerplugins/obj/plugin.so -Xclang -add-plugin -Xclang loplugin -Xclang -plugin-arg-loplugin -Xclang --cool-base-path=\${abs_top_srcdir}"])

# Clang plugin.
AC_MSG_CHECKING([whether to build a clang plugin])
if test "$enable_coplugin" = "yes"; then
    AC_MSG_RESULT([yes])
    CPPFLAGS="$CPPFLAGS -Xclang -load -Xclang \${abs_top_srcdir}/clang/plugin.so -Xclang -add-plugin -Xclang coplugin"
else
    AC_MSG_RESULT([no])
fi

echo "
Configuration:
    LOKit path                ${lokit_msg}
    LO path                   $LO_PATH
    LO integration tests      ${lo_msg}
    SSL support               $ssl_msg
    Debug & low security      $debug_msg
    Experimental features     $experimental_msg
    Test assertion logging    $log_asserts_msg
    Uglification and bundling $bundle_msg
    Anonymization             $anonym_msg
    Browsersync               $browsersync_msg
    cypress                   $cypress_msg
    C++ compiler flags        $CXXFLAGS
    Code-Coverage             $coverage_msg

    \$ make # to compile"
if test -n "$with_lo_path"; then
echo "    \$ make run # to start coolwsd
    \$ make sync-writer|calc|impress # to start browsersync with specified app
"
fi
if test -n "$poco_known_buggy"; then
    AC_MSG_WARN([Your version of POCO has known bugs: see dev-notes/dependency-issues.md])
fi

# Remove emconfigure artifacts that Autoconf fails to remove by itself:
rm -f a.wasm

dnl vim:set shiftwidth=4 softtabstop=4 expandtab:
