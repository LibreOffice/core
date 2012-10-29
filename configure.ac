dnl -*- Mode: Autoconf; tab-width: 4; indent-tabs-mode: nil; fill-column: 102 -*-
dnl configure.ac serves as input for the GNU autoconf package
dnl in order to create a configure script.

AC_INIT([LibreOffice],[3.7],[],[],[http://documentfoundation.org/])
AC_PREREQ([2.59])

save_CC=$CC
save_CXX=$CXX

BUILD_TYPE="LibO"
SCPDEFS=""
GIT_REPO_NAMES=""
GIT_NEEDED_SUBMODULES=""
LO_PATH= # used by path_munge to construct a PATH variable

PathFormat()
{
    formatted_path="$1"
    if test "$build_os" = "cygwin"; then
        pf_part1=
        pf_conv_to_dos=
        for pf_part in $formatted_path; do
            if test -z "$pf_part1"; then
                pf_part1="$pf_part"
            else
                pf_conv_to_dos="yes"
            fi
        done
        if test "$pf_conv_to_dos" = "yes"; then
            formatted_path=`cygpath -d "$formatted_path"`
            if test $? -ne 0;  then
                AC_MSG_ERROR([path conversion failed for "$1".])
            fi
        fi
        fp_count_colon=`echo "$formatted_path" | $GREP -c "[:]"`
        fp_count_slash=`echo "$formatted_path" | $GREP -c "[/]"`
        if test "$fp_count_slash$fp_count_colon" != "00"; then
            if test "$fp_count_colon" = "0"; then
                formatted_path=`realpath "$formatted_path"`
                if test $? -ne 0;  then
                    AC_MSG_ERROR([realpath failed for "$1".])
                fi
            fi
            formatted_path=`cygpath -m "$formatted_path"`
            if test $? -ne 0;  then
                AC_MSG_ERROR([path conversion failed for "$1".])
            fi
        fi
    fi
}

cat /dev/null > warn
have_WARNINGS="no"
add_warning()
{
    if test "$have_WARNINGS" = "no" ; then
        echo "*************************************" >> warn
        have_WARNINGS="yes"
        if which tput >/dev/null 2>/dev/null && test `tput colors` -ge 8; then
            dnl <esc> as actual byte (U+1b), [ escaped using quadrigraph @<:@
            COLORWARN='*@<:@1;33;40m WARNING @<:@0m:'
        else
            COLORWARN="* WARNING :"
        fi
    fi
    echo "$COLORWARN $@" >> warn
}

if test -n "$SOLARENV" ; then
    AC_MSG_ERROR([You have sourced config_host.mk in this shell.  This may lead to trouble, please run in a fresh (login) shell.])
fi

echo "********************************************************************"
echo "*"
echo "*   Running ${PACKAGE_NAME} build configuration."
echo "*"
echo "********************************************************************"
echo ""

dnl ===================================================================
dnl checks build and host OSes
dnl do this before argument processing to allow for platform dependent defaults
dnl ===================================================================
AC_CANONICAL_HOST

AC_PROG_EGREP
# AC_PROG_EGREP doesn't set GREP on all systems as well
AC_PATH_PROG(GREP, grep)

if test "$build_os" = "cygwin"; then
    EXEEXT_FOR_BUILD=.exe
    SRC_ROOT=`pwd`
    PathFormat "$SRC_ROOT"
    SRC_ROOT="$formatted_path"
    x_Cygwin=
else
    EXEEXT_FOR_BUILD=
    SRC_ROOT=`pwd`
    x_Cygwin=[\#]
fi

AC_SUBST(SRC_ROOT)
AC_SUBST(EXEEXT_FOR_BUILD)
AC_SUBST(x_Cygwin)

if test "z$EUID" = "z0" -a "`uname -o 2>/dev/null`" = "Cygwin"; then
    AC_MSG_ERROR([You must build LibreOffice as a normal user - not using an administrative account])
fi

# need sed in os checks...
AC_PATH_PROGS(SED, sed)
if test -z "$SED"; then
    AC_MSG_ERROR([install sed to run this script])
fi

dnl ===================================================================
dnl When building for Android the --with-android-ndk is mandatory
dnl ===================================================================

AC_ARG_WITH(android-ndk,
    AS_HELP_STRING([--with-android-ndk],
        [Specify location of the Android Native Development Kit. Mandatory when building for Android.]),
,)

AC_ARG_WITH(android-ndk-toolchain-version,
    AS_HELP_STRING([--with-android-ndk-toolchain-version],
        [Specify which toolchain version to use, of those present in the
        Android NDK you are using. Mandatory if the NDK used has several
        toolchain versions for the host architecture you are building for.]), ,)

AC_ARG_WITH(android-sdk,
    AS_HELP_STRING([--with-android-sdk],
        [Specify location of the Android SDK. Mandatory when building for Android.]),
,)

ANDROID_NDK_HOME=
if test -n "$with_android_ndk"; then
    ANDROID_NDK_HOME=$with_android_ndk

    # Set up a lot of pre-canned defaults

    if test $host_cpu = arm; then
        android_cpu=arm
    elif test $host_cpu = mips; then
        android_cpu=mips
    else
        # host_cpu is something like "i386" or "i686" I guess, NDK uses
        # "x86" in some contexts
        android_cpu=x86
    fi

    ANDROID_ABI_PREBUILT_BIN=`echo $ANDROID_NDK_HOME/toolchains/$android_cpu*-*$with_android_ndk_toolchain_version/prebuilt/*/bin`
    # Check if there are several toolchain versions
    case "$ANDROID_ABI_PREBUILT_BIN" in
    */bin\ */bin*)
        AC_MSG_ERROR([Several toolchain versions in NDK, you must specify --with-android-ndk-toolchain-version])
    esac

    # This stays empty if there is just one version of the toolchain in the NDK
    ANDROID_NDK_TOOLCHAIN_VERSION_SUBDIR=
    case "`echo $ANDROID_NDK_HOME/toolchains/$android_cpu*-*/prebuilt/*/bin`" in
    */bin\ */bin*)
        # Trailing slash intentional and necessary, compare to how this is used
        ANDROID_NDK_TOOLCHAIN_VERSION_SUBDIR="${with_android_ndk_toolchain_version}/"
        ;;
    esac

    if test $host_cpu = arm; then
       android_gcc_prefix=arm-linux-androideabi
    elif test $host_cpu = mips; then
       android_gcc_prefix=mipsel-linux-android
    elif test -f $ANDROID_ABI_PREBUILT_BIN/i686-android-linux-gcc; then
        android_gcc_prefix=i686-android-linux
    elif test -f $ANDROID_ABI_PREBUILT_BIN/i686-linux-android-gcc; then
        android_gcc_prefix=i686-linux-android
    else
        AC_MSG_ERROR([Can't figure out the toolchain prefix])
    fi

    test -z "$SYSBASE" && export SYSBASE=$ANDROID_NDK_HOME/platforms/android-9/arch-$android_cpu
    test -z "$AR" && AR=$ANDROID_ABI_PREBUILT_BIN/$android_gcc_prefix-ar
    test -z "$NM" && NM=$ANDROID_ABI_PREBUILT_BIN/$android_gcc_prefix-nm
    test -z "$OBJDUMP" && OBJDUMP=$ANDROID_ABI_PREBUILT_BIN/$android_gcc_prefix-objdump
    test -z "$RANLIB" && RANLIB=$ANDROID_ABI_PREBUILT_BIN/$android_gcc_prefix-ranlib
    test -z "$STRIP" && STRIP=$ANDROID_ABI_PREBUILT_BIN/$android_gcc_prefix-strip

    if test $host_cpu = arm; then
        ANDROID_APP_ABI=armeabi-v7a
        ANDROIDCFLAGS="-march=armv7-a -mfloat-abi=softfp -mthumb -mfpu=neon -Wl,--fix-cortex-a8"
    elif test $host_cpu = mips; then
        ANDROID_APP_ABI=mips
        ANDROIDCFLAGS=""
    else # x86
        ANDROID_APP_ABI=x86
        ANDROIDCFLAGS="-march=atom"
    fi
    ANDROIDCFLAGS="$ANDROIDCFLAGS -Wno-psabi --sysroot $ANDROID_NDK_HOME/platforms/android-9/arch-$android_cpu -L$ANDROID_NDK_HOME/sources/cxx-stl/gnu-libstdc++/${ANDROID_NDK_TOOLCHAIN_VERSION_SUBDIR}libs/$ANDROID_APP_ABI"

    # When using the 4.6 or newer toolchain, use the gold linker
    case "$with_android_ndk_toolchain_version" in
    4.[[6789]]*|[[56789]].*)
        # The NDK doesn't have ld.gold for MIPS for some reason
        if test "$host_cpu" != mips; then
            ANDROIDCFLAGS="$ANDROIDCFLAGS -fuse-ld=gold"
        fi
        ;;
    esac

    # gdbserver can be in different locations
    if test -f $ANDROID_NDK_HOME/toolchains/arm-linux-androideabi-4.4.3/prebuilt/gdbserver; then
        ANDROID_NDK_GDBSERVER=$ANDROID_NDK_HOME/toolchains/arm-linux-androideabi-4.4.3/prebuilt/gdbserver
    elif test -f $ANDROID_NDK_HOME/prebuilt/android-$android_cpu/gdbserver/gdbserver; then
        ANDROID_NDK_GDBSERVER=$ANDROID_NDK_HOME/prebuilt/android-$android_cpu/gdbserver/gdbserver
    else
        AC_MSG_ERROR([Can't find gdbserver for your Android target])
    fi

    if test $host_cpu = arm; then
        ANDROIDCXXFLAGS="$ANDROIDCFLAGS -I $ANDROID_NDK_HOME/sources/cxx-stl/gnu-libstdc++/${ANDROID_NDK_TOOLCHAIN_VERSION_SUBDIR}include -I$ANDROID_NDK_HOME/sources/cxx-stl/gnu-libstdc++/${ANDROID_NDK_TOOLCHAIN_VERSION_SUBDIR}libs/armeabi-v7a/include"
    elif test $host_cpu = mips; then
        ANDROIDCXXFLAGS="$ANDROIDCFLAGS -I $ANDROID_NDK_HOME/sources/cxx-stl/gnu-libstdc++/${ANDROID_NDK_TOOLCHAIN_VERSION_SUBDIR}include -I$ANDROID_NDK_HOME/sources/cxx-stl/gnu-libstdc++/${ANDROID_NDK_TOOLCHAIN_VERSION_SUBDIR}libs/mips/include"
    else # x86
        ANDROIDCXXFLAGS="$ANDROIDCFLAGS -I $ANDROID_NDK_HOME/sources/cxx-stl/gnu-libstdc++/${ANDROID_NDK_TOOLCHAIN_VERSION_SUBDIR}include -I$ANDROID_NDK_HOME/sources/cxx-stl/gnu-libstdc++/${ANDROID_NDK_TOOLCHAIN_VERSION_SUBDIR}libs/x86/include"
    fi

    test -z "$CC" && CC="$ANDROID_ABI_PREBUILT_BIN/$android_gcc_prefix-gcc $ANDROIDCFLAGS"
    test -z "$CXX" && CXX="$ANDROID_ABI_PREBUILT_BIN/$android_gcc_prefix-g++ $ANDROIDCXXFLAGS"
fi
AC_SUBST(ANDROID_NDK_HOME)
AC_SUBST(ANDROID_NDK_TOOLCHAIN_VERSION_SUBDIR)
AC_SUBST(ANDROID_NDK_GDBSERVER)
AC_SUBST(ANDROID_APP_ABI)

dnl ===================================================================
dnl Also --with-android-sdk is mandatory
dnl ===================================================================
ANDROID_SDK_HOME=
if test -n "$with_android_sdk"; then
   ANDROID_SDK_HOME=$with_android_sdk
   PATH="$ANDROID_SDK_HOME/platform-tools:$ANDROID_SDK_HOME/tools:$PATH"
fi
AC_SUBST(ANDROID_SDK_HOME)

dnl ===================================================================
dnl The following is a list of supported systems.
dnl Sequential to keep the logic very simple
dnl These values may be checked and reset later.
dnl ===================================================================
#defaults unless the os test overrides this:
test_randr=yes
test_xrender=yes
test_cups=yes
test_dbus=yes
test_fontconfig=yes
test_cairo=yes

# Default values, as such probably valid just for Linux, set
# differently below just for Mac OSX,but at least better than
# hardcoding these as we used to do. Much of this is duplicated also
# in solenv for old build system and for gbuild, ideally we should
# perhaps define stuff like this only here in configure.ac?

LINKFLAGSSHL="-shared"
PICSWITCH="-fpic"
DLLPOST=".so"

LINKFLAGSNOUNDEFS="-Wl,-z,defs"

case "$host_os" in

solaris*)
    test_gtk=yes
    build_gstreamer=yes
    build_gstreamer_0_10=yes
    test_tde=yes
    test_kde=yes
    test_freetype=yes
    test_gstreamer=yes
    _os=SunOS

    dnl ===========================================================
    dnl Check whether we're using Solaris 10 - SPARC or Intel.
    dnl ===========================================================
    AC_MSG_CHECKING([the Solaris operating system release])
    _os_release=`echo $host_os | $SED -e s/solaris2\.//`
    if test "$_os_release" -lt "10"; then
        AC_MSG_ERROR([use Solaris >= 10 to build LibreOffice])
    else
        AC_MSG_RESULT([ok ($_os_release)])
    fi

    dnl Check whether we're using a SPARC or i386 processor
    AC_MSG_CHECKING([the processor type])
    if test "$host_cpu" = "sparc" -o "$host_cpu" = "i386"; then
        AC_MSG_RESULT([ok ($host_cpu)])
    else
        AC_MSG_ERROR([only SPARC and i386 processors are supported])
    fi
    ;;

linux-gnu*|k*bsd*-gnu*)
    test_gtk=yes
    build_gstreamer=yes
    build_gstreamer_0_10=yes
    test_tde=yes
    test_kde=yes
    test_kde4=yes
    test_freetype=yes
    test_unix_quickstarter=yes
    _os=Linux
    ;;

gnu)
    test_randr=no
    test_xrender=no
    _os=GNU
     ;;

cygwin*|interix*|mingw32*)

    # When building on Windows normally with MSVC under Cygwin,
    # configure thinks that the host platform (the platform the
    # built code will run on) is Cygwin, even if it obviously is
    # Windows, which in Autoconf terminology is called
    # "mingw32". (Which is misleading as MinGW is the name of the
    # tool-chain, not an operating system.)

    # Somewhat confusing, yes. But this configure script doesn't
    # look at $host etc that much, it mostly uses its own $_os
    # variable, set here in this case statement.

    # When cross-compiling to Windows from Unix, the host platform
    # is "mingw32" (because in that case it is the MinGW
    # tool-chain that is used).

    test_cups=no
    test_dbus=no
    test_randr=no
    test_xrender=no
    test_freetype=no
    test_fontconfig=no
    _os=WINNT
    use_shl_version="TRUE"
    DYNAMIC_CRT="TRUE"
    # If the host OS matches "mingw32*", that means we are using the
    # MinGW cross-compiler, because we don't see the point in building
    # LibreOffice using MinGW on Windows. If you want to build on
    # Windows, use MSVC. If you want to use MinGW, surely you want to
    # cross-compile (from Linux or some other Unix).

    case "$host_os" in
    mingw32*)
        WITH_MINGW=yes
        if test -z "$CC"; then
            CC="$host_cpu-$host_vendor-$host_os-gcc"
        fi
        if test -z "$CXX"; then
            CXX="$host_cpu-$host_vendor-$host_os-g++"
        fi
        ;;
    esac
    ;;

darwin*) # Mac OS X or iOS
    test_gtk=yes
    test_randr=no
    test_xrender=no
    test_freetype=no
    test_fontconfig=no
    test_dbus=no
    if test "$host_cpu" = "arm"; then
        _os=iOS
        test_gtk=no
        test_cups=no
    else
        _os=Darwin
    fi
    enable_systray=no
    # See comment above the case "$host_os"
    LINKFLAGSSHL="-dynamiclib -single_module"

    # -fPIC is default
    PICSWITCH=""

    DLLPOST=".dylib"

    # -undefined error is the default
    LINKFLAGSNOUNDEFS=""
;;

freebsd*)
    test_gtk=yes
    build_gstreamer=yes
    build_gstreamer_0_10=yes
    test_tde=yes
    test_kde=yes
    test_kde4=yes
    test_freetype=yes
    AC_MSG_CHECKING([the FreeBSD operating system release])
    if test -n "$with_os_version"; then
        OSVERSION="$with_os_version"
    else
        OSVERSION=`/sbin/sysctl -n kern.osreldate`
    fi
    AC_MSG_RESULT([found OSVERSION=$OSVERSION])
    AC_MSG_CHECKING([which thread library to use])
    if test "$OSVERSION" -lt "500016"; then
        PTHREAD_CFLAGS="-D_THREAD_SAFE"
        PTHREAD_LIBS="-pthread"
    elif test "$OSVERSION" -lt "502102"; then
        PTHREAD_CFLAGS="-D_THREAD_SAFE"
        PTHREAD_LIBS="-lc_r"
    else
        PTHREAD_CFLAGS=""
        PTHREAD_LIBS="-pthread"
    fi
    AC_MSG_RESULT([$PTHREAD_LIBS])
    _os=FreeBSD
    ;;

*netbsd*)
    test_gtk=yes
    build_gstreamer=yes
    build_gstreamer_0_10=yes
    test_tde=no
    test_kde=no
    test_kde4=yes
    test_freetype=yes
    PTHREAD_LIBS="-pthread -lpthread"
    _os=NetBSD
    ;;

aix*)
    test_randr=no
    test_freetype=yes
    PTHREAD_LIBS=-pthread
    _os=AIX
    ;;

openbsd*)
    test_gtk=yes
    test_tde=yes
    test_kde=yes
    test_freetype=yes
    PTHREAD_CFLAGS="-D_THREAD_SAFE"
    PTHREAD_LIBS="-pthread"
    _os=OpenBSD
    ;;

dragonfly*)
    test_gtk=yes
    build_gstreamer=yes
    build_gstreamer_0_10=yes
    test_tde=yes
    test_kde=yes
    test_kde4=yes
    test_freetype=yes
    PTHREAD_LIBS="-pthread"
    _os=DragonFly
    ;;

linux-android*)
    build_gstreamer=no
    build_gstreamer_0_10=no
    test_cups=no
    test_dbus=no
    test_fontconfig=no
    test_freetype=no
    test_gtk=no
    test_tde=no
    test_kde=no
    test_kde4=no
    test_randr=no
    test_xrender=no
    test_unix_quickstarter=no
    _os=Android

    if test -z "$with_android_ndk"; then
        AC_MSG_ERROR([the --with-android-ndk option is mandatory])
    fi

    if test -z "$with_android_sdk"; then
        AC_MSG_ERROR([the --with-android-sdk option is mandatory])
    fi

    # Verify that the NDK and SDK options are proper
    if test ! -f "$ANDROID_NDK_HOME/platforms/android-9/arch-arm/usr/lib/libc.a"; then
        AC_MSG_ERROR([the --with-android-ndk option does not point to an Android NDK])
    fi

    if test ! -d "$ANDROID_SDK_HOME/platforms"; then
        AC_MSG_ERROR([the --with-android-sdk option does not point to an Android SDK])
    fi

    BUILD_TYPE="$BUILD_TYPE FONTCONFIG FREETYPE"
    FREETYPE_TARBALL=dbf2caca1d3afd410a29217a9809d397-freetype-2.4.8.tar.bz2
    FONTCONFIG_TARBALL=77e15a92006ddc2adbb06f840d591c0e-fontconfig-2.8.0.tar.gz
    ;;

*)
    AC_MSG_ERROR([$host_os operating system is not suitable to build LibreOffice for!])
    ;;
esac

if test "$_os" = "AIX"; then
    AC_PATH_PROG(GAWK, gawk)
    if test -z "$GAWK"; then
        AC_MSG_ERROR([gawk not found in \$PATH])
    fi
fi

AC_SUBST(use_shl_version)
AC_SUBST(DYNAMIC_CRT)

AC_SUBST(WITH_MINGW)
AC_SUBST(OSVERSION)
AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_LIBS)

AC_SUBST(FREETYPE_TARBALL)
AC_SUBST(FONTCONFIG_TARBALL)

###############################################################################
# Extensions switches --enable/--disable
###############################################################################
# By default these should be enabled unless having extra dependencies.
# If there is extra dependency over configure options then the enable should
# be automagic based on whether the requiring feature is enabled or not.
# All this options change anything only with --enable-extension-integration.

# The name of this option and its help string makes it sound as if
# extensions are built anyway, just not integrated in the installer,
# if you use --disable-extension-integration. Is that really the
# case?

AC_ARG_ENABLE(extension-integration,
    AS_HELP_STRING([--disable-extension-integration],
        [Disable integration of the built extensions in the installer of the
         product. Use this switch to disable the integration.])
)

AC_ARG_ENABLE(ext-barcode,
    AS_HELP_STRING([--enable-ext-barcode],
        [Enable the Barcode extension.])
)

AC_ARG_ENABLE(database-connectivity,
    AS_HELP_STRING([--disable-database-connectivity],
        [Disable various database connectivity. Work in progress, use only if you are hacking on it.])
)

# This doesn't mean not building (or "integrating") extensions
# (although it probably should; i.e. it should imply
# --disable-extension-integration I guess), it means not supporting
# any extension mechanism at all
AC_ARG_ENABLE(extensions,
    AS_HELP_STRING([--disable-extensions],
        [Disable all add-on extension functionality. Work in progress, use only if you are hacking on it.])
)

AC_ARG_ENABLE(scripting,
    AS_HELP_STRING([--disable-scripting],
        [Disable BASIC, Java and Python. Work in progress, use only if you are hacking on it.])
)

# This is mainly for iOS, but could potentially be used in some
# special case otherwise, too, so factored out as a separate setting
AC_ARG_ENABLE(dynamic-loading,
    AS_HELP_STRING([--disable-dynamic-loading],
        [Disable any use of dynamic loading of code. Work in progress, use only if you are hacking on it.])
)

AC_ARG_ENABLE(ext-diagram,
    AS_HELP_STRING([--enable-ext-diagram],
        [Enable the SmART Gallery (Diagram) extension.])
)

AC_ARG_ENABLE(ext-google-docs,
    AS_HELP_STRING([--enable-ext-google-docs],
        [Enable the Google Documents extension.])
)

AC_ARG_ENABLE(ext-hunart,
    AS_HELP_STRING([--enable-ext-hunart],
        [Enable the Hungarian Cross-reference Toolbar extension.])
)

AC_ARG_ENABLE(ext-languagetool,
    AS_HELP_STRING([--enable-ext-languagetool],
        [Enable the LanguageTool extension.])
)

AC_ARG_ENABLE(ext-mysql-connector,
    AS_HELP_STRING([--enable-ext-mysql-connector],
        [Enable the build of the MySQL Connector extension.])
)

AC_ARG_ENABLE(ext-nlpsolver,
    AS_HELP_STRING([--enable-ext-nlpsolver],
        [Enable the NLPSolver extension.])
)

AC_ARG_ENABLE(ext-ct2n,
    AS_HELP_STRING([--enable-ext-ct2n],
        [Enable the ConvertTextToNumber extension.])
)

AC_ARG_ENABLE(ext-numbertext,
    AS_HELP_STRING([--enable-ext-numbertext],
        [Enable the Numbertext extension.])
)

AC_ARG_ENABLE(ext-oooblogger,
    AS_HELP_STRING([--enable-ext-oooblogger],
        [Enable the OOo Blogger extension.])
)

AC_ARG_ENABLE(ext-pdfimport,
    AS_HELP_STRING([--disable-ext-pdfimport],
        [Disable the PDF Import extension.])
)

AC_ARG_ENABLE(ext-presenter-console,
    AS_HELP_STRING([--disable-ext-presenter-console],
        [Disable the Presenter Console extension.])
)

AC_ARG_ENABLE(ext-presenter-minimizer,
    AS_HELP_STRING([--disable-ext-presenter-minimizer],
        [Disable the Presentation Minimizer extension.])
)

AC_ARG_ENABLE(ext-report-builder,
    AS_HELP_STRING([--disable-ext-report-builder],
        [Disable the Report Builder extension.])
)

AC_ARG_ENABLE(ext-typo,
    AS_HELP_STRING([--enable-ext-typo],
        [Enable the Typography Toolbar extension.])
)

AC_ARG_ENABLE(ext-validator,
    AS_HELP_STRING([--enable-ext-validator],
        [Enable the Validator extension.])
)

AC_ARG_ENABLE(ext-watch-window,
    AS_HELP_STRING([--enable-ext-watch-window],
        [Enable the Watch Window extension to Calc.])
)

AC_ARG_ENABLE(ext-wiki-publisher,
    AS_HELP_STRING([--enable-ext-wiki-publisher],
        [Enable the Wiki Publisher extension.])
)
###############################################################################

dnl ---------- *** ----------

AC_ARG_ENABLE(mergelibs,
    AS_HELP_STRING([--enable-mergelibs],
        [Enables linking of big, merged, library. Experimental feature tested
        only on Linux and Android.])
)

AC_ARG_ENABLE(graphite,
    AS_HELP_STRING([--enable-graphite],
        [Enables the compilation of Graphite smart font rendering.])
)

AC_ARG_ENABLE(fetch-external,
    AS_HELP_STRING([--disable-fetch-external],
        [Disables fetching external tarballs from web sources.])
)

AC_ARG_ENABLE(lockdown,
    AS_HELP_STRING([--disable-lockdown],
        [Disables the gconf integration work in LibreOffice.]),
)

AC_ARG_ENABLE(vba,
    AS_HELP_STRING([--disable-vba],
        [Disables the vba compatibility feature.])
)

AC_ARG_ENABLE(pch,
    AS_HELP_STRING([--enable-pch],
        [DEPRECATED : is ignored])
)

AC_ARG_ENABLE(mozilla,
    AS_HELP_STRING([--enable-mozilla],
        [LibreOffice can include a strangely hacked up Mozilla binary for your
         platform, to build with this version, use this option.])
)

AC_ARG_ENABLE(epm,
    AS_HELP_STRING([--enable-epm],
        [LibreOffice includes self-packaging code, that requires epm, however epm is
         useless for large scale package building.])
)

AC_ARG_ENABLE(odk,
    AS_HELP_STRING([--disable-odk],
        [LibreOffice includes an ODK, office development kit which some packagers may
         wish to build without.])
)

AC_ARG_ENABLE(mathmldtd,
    AS_HELP_STRING([--enable-mathmldtd],
        [Enable bundling of (modified) MathML 1.01 DTD.])
)

AC_ARG_ENABLE(evolution2,
    AS_HELP_STRING([--enable-evolution2],
        [Allows the built-in evolution 2 addressbook connectivity build to be
         enabled.])
)

AC_ARG_ENABLE(directx,
    AS_HELP_STRING([--disable-directx],
        [Remove DirectX implementation for the new XCanvas interface.
         The DirectX support requires more stuff installed on Windows to
         compile. (DirectX SDK, GDI+ libs)])
)

AC_ARG_ENABLE(activex,
    AS_HELP_STRING([--disable-activex],
        [Disable the use of ActiveX for a Windows build.
        This switch is mandatory when using VC++ 2008 Express.])
)

AC_ARG_ENABLE(atl,
    AS_HELP_STRING([--disable-atl],
        [Disable the use of ATL for a Windows build.])
    [
                          This switch is mandatory when using VC++ 2008 Express.
    ],
,)

AC_ARG_ENABLE(werror,
    AS_HELP_STRING([--enable-werror],
        [Turn warnings to errors. (Has no effect in modules where the treating
         of warnings as errors is disabled explicitly.)]),
,)

AC_ARG_ENABLE(assert-always-abort,
    AS_HELP_STRING([--enable-assert-always-abort],
        [make assert() abort even in release code.]),
,)

AC_ARG_ENABLE(debug,
    AS_HELP_STRING([--enable-debug],
        [Include debugging information, disable compiler optimization and inlining plus
         extra debugging code like assertions. Extra large build! (enables -g compiler flag
         and dmake debug=true).]))

AC_ARG_ENABLE(dbgutil,
    AS_HELP_STRING([--enable-dbgutil],
        [Provide debugging support from --enable-debug and include additional debugging
         utilities such as object counting or more expensive checks.
         This is the recommended option for developers.
         Note that this makes the build ABI incompatible, it is not possible to mix object
         files or libraries from a --enable-dbgutil and a --disable-dbgutil build.]))

AC_ARG_ENABLE(selective-debuginfo,
    AS_HELP_STRING([--enable-selective-debuginfo],
        [If --enable-debug or --enable-dbgutil is used, build debugging information
         (-g compiler flag) only for the specified gbuild build targets
         (where all means everything, - prepended means not to enable, / appended means
         everything in the directory; there is no ordering, more specific overrides
         more general, and disabling takes precedence).
         Example: --enable-selective-debuginfo="all -sw/ -Library_sc".]))

AC_ARG_ENABLE(compiler-plugins,
    AS_HELP_STRING([--enable-compiler-plugins],
        [Enable compiler plugins that will perform additional checks during
         building. Enabled automatically by --enable-dbgutil.]))

AC_ARG_ENABLE(linkoo,
    AS_HELP_STRING([--disable-linkoo],
        [Disable linkoo for the smoketest installation.]))

AC_ARG_ENABLE(lto,
    AS_HELP_STRING([--enable-lto],
        [Enable link-time optimization. Suitable for product builds.
         Building takes longer but libraries are optimized for speed.
         (possible only with gcc-4.5 or later,
          better to use gcc-4.6 and 'gold' as linker)]))

AC_ARG_ENABLE(crashdump,
    AS_HELP_STRING([--enable-crashdump],
        [Enable the crashdump feature.]))

AC_ARG_ENABLE(python,
    AS_HELP_STRING([--enable-python=<no/auto/system/internal>],
        [Enables or disables Python support at run-time and build-time.
         Also specifies what Python to use. 'auto' is the
         default. Note that Python can be disabled with
         --disable-python or --enable-python=no only if no
         translations are required.]))

AC_ARG_ENABLE(gtk,
    AS_HELP_STRING([--disable-gtk],
        [Determines whether to use Gtk+ vclplug on platforms where Gtk+ is available.]),
,enable_gtk=yes)

AC_ARG_ENABLE(gtk3,
    AS_HELP_STRING([--enable-gtk3],
        [Determines whether to use Gtk+ 3.0 vclplug on platforms where Gtk+ 3.0 is available.]),
,enable_gtk3=no)

AC_ARG_ENABLE(systray,
    AS_HELP_STRING([--disable-systray],
        [Determines whether to build the systray quickstarter.]),
,enable_systray=yes)

AC_ARG_ENABLE(split-app-modules,
    AS_HELP_STRING([--enable-split-app-modules],
        [Split file lists for app modules, e.g. base, calc.
         Has effect only with make distro-pack-install]),
,)

AC_ARG_ENABLE(split-opt-features,
    AS_HELP_STRING([--enable-split-opt-features],
        [Split file lists for some optional features, .e.g. pyuno, testtool.
         Has effect only with make distro-pack-install]),
,)

AC_ARG_ENABLE(cairo-canvas,
[  --disable-cairo-canvas  Determines whether to build the Cairo canvas on
                          platforms where Cairo is available.
])

AC_ARG_ENABLE(librsvg,
    AS_HELP_STRING([--enable-librsvg=<no/auto/system/internal/fully-internal>],
        [Enables or disables use of librsvg to render SVG at run-time.
         Also specificed what librsvg to use. 'auto' is the default.
         'fully-internal' also forces internal versions of libraries (only)
         librsvg depends on (currently GLib, gdk-pixbuf, libcroco, libgsf,
         pango).]))

AC_ARG_ENABLE(opengl,
    AS_HELP_STRING([--disable-opengl],
        [Determines whether to build the OpenGL 3D slide transitions component.]),
,enable_opengl=yes)

AC_ARG_ENABLE(dbus,
    AS_HELP_STRING([--disable-dbus],
        [Determines whether to enable features that depend on dbus.
         e.g. Presentation mode screensaver control, bluetooth presentation control]),
,enable_dbus=yes)

AC_ARG_ENABLE(packagekit,
    AS_HELP_STRING([--enable-packagekit],
        [Determines whether to enable features using packagekit.
         Right now that is auto font install]),
,)

AC_ARG_ENABLE(sdremote,
    AS_HELP_STRING([--disable-sdremote],
        [Determines whether to enable Impress remote control.]),
,enable_sdremote=yes)

AC_ARG_ENABLE(bluetooth,
    AS_HELP_STRING([--disable-bluetooth],
        [Determines whether to build sdremote with bluetooth support
        (via dbus)]),
,enable_bluetooth=yes)

AC_ARG_ENABLE(gconf,
    AS_HELP_STRING([--disable-gconf],
        [Determines whether to use the GConf support.]),
,enable_gconf=yes)

AC_ARG_ENABLE(gnome-vfs,
    AS_HELP_STRING([--disable-gnome-vfs],
        [Determines whether to use the Gnome Virtual Filing System on platforms
         where that VFS is available.]),
,enable_gnome_vfs=yes)

AC_ARG_ENABLE(gio,
    AS_HELP_STRING([--enable-gio],
        [Determines whether to use the GIO support.]),
,enable_gio=no)

AC_ARG_ENABLE(telepathy,
    AS_HELP_STRING([--enable-telepathy],
        [Determines whether to enable Telepathy for collaboration.]),
,enable_telepathy=no)

AC_ARG_ENABLE(build-mozilla,
    AS_HELP_STRING([--disable-build-mozilla],
        [Use this option if you do not want to build the Mozilla components from
         the Mozilla source code but take precompiled zips.]),
,)

AC_ARG_ENABLE(tde,
    AS_HELP_STRING([--enable-tde],
        [Determines whether to use TQt/TDE vclplug on platforms where TQt and
         TDE are available.]),
,)

AC_ARG_ENABLE(tdeab,
    AS_HELP_STRING([--disable-tdeab],
        [Disable the TDE address book support.]),
,
    if test "$enable_tde" = "yes"; then
        enable_tdeab=yes
    fi
)

AC_ARG_ENABLE(kde,
    AS_HELP_STRING([--enable-kde],
        [Determines whether to use Qt3/KDE3 vclplug on platforms where Qt3 and
         KDE3 are available.]),
,)

AC_ARG_ENABLE(kdeab,
    AS_HELP_STRING([--disable-kdeab],
        [Disable the KDE3 address book support.]),
,
    if test "$enable_kde" = "yes"; then
        enable_kdeab=yes
    fi
)

AC_ARG_ENABLE(kde4,
    AS_HELP_STRING([--enable-kde4],
        [Determines whether to use Qt4/KDE4 vclplug on platforms where Qt4 and
         KDE4 are available. May be used with --enable-kde if you want to support
         both KDE3 and KDE4.]),
,)

AC_ARG_ENABLE(headless,
    AS_HELP_STRING([--enable-headless],
        [Disable building of GUIs to reduce dependencies. Useful for
        server usage. Work in progress, use only if you are hacking on
        it. Not related to the --headless option.]), ,)

AC_ARG_ENABLE(unix-qstart-libpng,
    AS_HELP_STRING([--disable-unix-qstart-libpng],
        [On UNIX systems, we have a faster splash app, that can use libpng to
         render its splash, if we can safely link to the system libpng then
         enabling this is a good idea (ie. for Linux Distro packaging).]),
,enable_unix_qstart_libpng=yes)

AC_ARG_ENABLE(binfilter,
    AS_HELP_STRING([--enable-binfilter],
        [Enable legacy binary file formats filters build.]),
,enable_binfilter=no
)

AC_ARG_ENABLE(rpath,
    AS_HELP_STRING([--disable-rpath],
        [Disable the use of relative paths in shared libraries.]),
,)

AC_ARG_ENABLE(randr,
    AS_HELP_STRING([--disable-randr],
        [Disable RandR support in the vcl project.]),
,enable_randr=yes)

AC_ARG_ENABLE(randr-link,
    AS_HELP_STRING([--disable-randr-link],
        [Disable linking with libXrandr, instead dynamically open it at runtime.]),
,enable_randr_link=yes)

AC_ARG_ENABLE(gstreamer,
    AS_HELP_STRING([--enable-gstreamer],
        [Enable building with the new gstreamer 1.0 avmedia backend.]),
,enable_gstreamer=no)

AC_ARG_ENABLE(gstreamer-0-10,
    AS_HELP_STRING([--disable-gstreamer-0-10],
        [Disable building the gstreamer avmedia backend.]),
,enable_gstreamer_0_10=yes)

AC_ARG_ENABLE(neon,
    AS_HELP_STRING([--disable-neon],
        [Disable neon and the compilation of webdav binding.]),
,)

AC_ARG_ENABLE(cve-tests,
    AS_HELP_STRING([--disable-cve-tests],
        [Prevent CVE tests to be executed]),
,)

AC_ARG_ENABLE(build-unowinreg,
    AS_HELP_STRING([--enable-build-unowinreg],
        [Do not use the prebuilt unowinreg.dll. Build it instead. The MinGW C++
         compiler is needed on Linux.])
    [
                          Usage:     --enable-build-unowinreg
    ],
,)

AC_ARG_ENABLE(verbose,
    AS_HELP_STRING([--enable-verbose],
        [Increase build verbosity.])[
  --disable-verbose       Decrease build verbosity.],
,)

AC_ARG_ENABLE(dependency-tracking,
    AS_HELP_STRING([--enable-dependency-tracking],
        [Do not reject slow dependency extractors.])[
  --disable-dependency-tracking
                          Disables generation of dependency information.
                          Speed up one-time builds.],
,)

AC_ARG_ENABLE(icecream,
    AS_HELP_STRING([--enable-icecream],
        [Use the 'icecream' distributed compiling tool to speedup the compilation.
         It defaults to /opt/icecream for the location of the icecream gcc/g++
         wrappers, you can override that using --with-gcc-home=/the/path switch.]),
,)

AC_ARG_ENABLE(zenity,
    AS_HELP_STRING([--disable-zenity],
        [Do not display a build icon in the notification area (on unix) during build.]),
,enable_zenity=yes)

AC_ARG_ENABLE(nsplugin,
    AS_HELP_STRING([--disable-nsplugin],
        [Do not build nsplugin extension for browser embedding.])
)

AC_ARG_ENABLE(cups,
    AS_HELP_STRING([--disable-cups],
        [Do not build cups support.])
)

AC_ARG_ENABLE(ccache,
    AS_HELP_STRING([--disable-ccache],
        [Do not try to use ccache automatically.
         By default, we will try to detect if ccache is available; in that case if
         CC/CXX are not yet set, and --enable-icecream is not given, we
         attempt to use ccache. --disable-ccache disables ccache completely.
]),
,)

AC_ARG_ENABLE(64-bit,
    AS_HELP_STRING([--enable-64-bit],
        [Build a 64-bit LibreOffice on platforms where the normal and only supported build
         is 32-bit. In other words, this option is experimental and possibly quite broken,
         use only if you are hacking on 64-bit support.]), ,)

AC_ARG_ENABLE(extra-gallery,
    AS_HELP_STRING([--enable-extra-gallery],
        [Add extra gallery content.]),
,)

AC_ARG_ENABLE(extra-template,
    AS_HELP_STRING([--enable-extra-template],
        [Add extra template content.]),
,)

AC_ARG_ENABLE(extra-sample,
    AS_HELP_STRING([--enable-extra-sample],
        [Add extra sample content.]),
,)

AC_ARG_ENABLE(extra-font,
    AS_HELP_STRING([--enable-extra-font],
        [Add extra font content.]),
,)

AC_ARG_ENABLE(oxygenoffice,
    AS_HELP_STRING([--enable-oxygenoffice],
        [Download OxygenOffice branding and set custom settings.]),
,)

AC_ARG_ENABLE(lomenubar,
    AS_HELP_STRING([--enable-lomenubar],
        [Enable global menu support.]),
,)

AC_ARG_ENABLE(online-update,
    AS_HELP_STRING([--enable-online-update],
        [Enable the online update service that will check for new versions of
         LibreOffice. By default, it is on on Windows and Mac, and off on Linux.]),
,)

AC_ARG_ENABLE(release-build,
    AS_HELP_STRING([--enable-release-build],
        [Enable release build.
         See http://wiki.documentfoundation.org/DevBuild]),
,)

AC_ARG_ENABLE(silent-msi,
    AS_HELP_STRING([--enable-silent-msi],
        [Enable MSI with LIMITUI=1 (silent install).]),
,)

AC_ARG_ENABLE(postgresql-sdbc,
    AS_HELP_STRING([--disable-postgresql-sdbc],
        [Disable the build of the PostgreSQL-SDBC driver.])
)

AC_ARG_ENABLE(coretext,
    AS_HELP_STRING([--enable-coretext],
        [Use CoreText framework on Mac (instead of ATSU).
        Known to not work properly, use only if you plan to work on that.]),
)

AC_ARG_ENABLE(winegcc,
    AS_HELP_STRING([--enable-winegcc],
        [Enable use of winegcc during the build, in order to create msi* tools
         needed for MinGW cross-compilation.]),
)

AC_ARG_ENABLE(liblangtag,
    AS_HELP_STRING([--disable-liblangtag],
        [Disable use of liblangtag, and insted use an own simple
         implementation.]),
)

AC_ARG_ENABLE(bogus-pkg-config,
    AS_HELP_STRING([--enable-bogus-pkg-config],
        [MACOSX only: on MacOSX pkg-config can cause trouble. by default if one is found in the PATH, an error is issued. This flag turn that error into a warning.]),
)

dnl ===================================================================
dnl Optional Packages (--with/without-)
dnl ===================================================================

AC_ARG_WITH(gnu-patch,
    AS_HELP_STRING([--with-gnu-patch],
        [Specify location of GNU patch on Solaris or FreeBSD.]),
,)

AC_ARG_WITH(build-platform-configure-options,
        [Specify options for the configure script run for the *build* platform in a cross-compilation])

AC_ARG_WITH(gnu-cp,
    AS_HELP_STRING([--with-gnu-cp],
        [Specify location of GNU cp on Solaris or FreeBSD.]),
,)

AC_ARG_WITH(external-tar,
    AS_HELP_STRING([--with-external-tar=<TARFILE PATH>],
        [Specify path to tarfiles manually.]),
    TARFILE_LOCATION=$withval ,
)

AC_ARG_WITH(solver-and-workdir-root,
    AS_HELP_STRING([--with-solver-and-workdir-root=<PATH>],
        [Specify path that contains SOLARVER and WORKDIR directories manually.])
)

AC_ARG_WITH(linked-git,
    AS_HELP_STRING([--with-linked-git=<OTHER_CLONE_DIR>],
        [Specify another checkout's clonedir to re-use. This makes use of
                 git-new-workdir, and saves a lot of diskspace when having multiple
                 trees side-by-side.]),
    GIT_LINK_SRC=$withval ,
)

AC_ARG_WITH(vba-package-format,
    AS_HELP_STRING([--with-vba-package-format],
        [Specify package format for vba compatibility api. Specifying  "builtin"
         means the api component and associated type library are  part of the
         installation set. Specifying "extn" creates an uno extension that is
         part of the installation set (located in the program directory) that
         MUST be optionally registered using either the unopkg executeable or the
         extension manager gui.])
    [
                          Note: "builtin" is the default, "extn" can cause
                          problems.

                          Usage:     --with-vba-package-format="builtin" or
                                     --with-vba-package-format="extn"
    ],
,)

AC_ARG_WITH(theme,
    AS_HELP_STRING([--with-theme="theme1 theme2..."],
        [Choose which themes to include. By default those themes with an '*' are included.
         Possible choices: *crystal, *default (galaxy), *hicontrast, human, industrial, *oxygen, *tango, *tango_testing.]),
,)

AC_ARG_WITH(helppack-integration,
[
  --without-helppack-integration      It will not integrate the helppacks to the installer
                          of the product.
                          Please use this switch to use the online help or separate help packages.],
,)

AC_ARG_WITH(fonts,
    AS_HELP_STRING([--without-fonts],
        [LibreOffice includes some third-party fonts to provide a reliable basis for
         help content, templates, samples, etc. When these fonts are already
         known to be available on the system then you should use this option.]),
,)

AC_ARG_WITH(ppds,
    AS_HELP_STRING([--without-ppds],
        [Removes Postscript Printer definition files from LibreOffice
         installation set, for people building for specific distributions where
         PPDs are known to be already available (every recent distro with CUPS backend).]),
,)

AC_ARG_WITH(afms,
    AS_HELP_STRING([--without-afms],
        [Removes bitmap font files from LibreOffice installation set, for people
         building for specific distributions where AFM files or TrueType Fonts
         are known to be available.]),
,)

AC_ARG_WITH(agfa-monotype-fonts,
     AS_HELP_STRING([--with-agfa-monotype-fonts],
          [This switch should only be enabled for those who have the right
           to use or distribute the proprietary Agfa Monotype
           fonts.]),
,)

AC_ARG_WITH(epm,
    AS_HELP_STRING([--with-epm],
        [Decides which epm to use. Default is to use the one from the system if
         one is built. When either this is not there or you say =internal epm
         will be built.]),
,)

AC_ARG_WITH(package-format,
    AS_HELP_STRING([--with-package-format],
        [Specify package format(s) for LibreOffice installsets. Default is the
         "normal" one of the OS/Distribution. Possible values: aix, bsd, deb,
         inst, tardist, osx, pkg, rpm, setld, native, portable, archive, dmg,
          installed, msi. Example: --with-package-format="deb dmg"]),
,)

AC_ARG_WITH(system-libs,
    AS_HELP_STRING([--with-system-libs],
        [Use libraries already on system -- enables all --with-system-* flags except
         mozilla.]),
,)

AC_ARG_WITH(system-headers,
    AS_HELP_STRING([--with-system-headers],
        [Use headers already on system -- enables all --with-system-* flags for
         external packages whose headers are the only entities used i.e.
         boost/vigra/odbc/sane-header(s).]),,
    [with_system_headers="$with_system_libs"])

AC_ARG_WITH(system-jars,
    AS_HELP_STRING([--without-system-jars],
        [When building with --with-system-libs, also the needed jars are expected
         on the system. Use this to disable that]),,
    [with_system_jars="$with_system_libs"])

AC_ARG_WITH(system-stdlibs,
    AS_HELP_STRING([--without-system-stdlibs],
        [Bundle the used libstdc++/libgcc_s into the installation set.]),,
    [with_system_stdlibs="$with_system_libs"])

AC_ARG_WITH(system-cairo,
    AS_HELP_STRING([--with-system-cairo],
        [Use Cairo libraries already on system.]),,
    [with_system_cairo="$with_system_libs"])

AC_ARG_WITH(system-graphite,
    AS_HELP_STRING([--with-system-graphite],
        [Use graphite library already installed on system.]),,
    [with_system_graphite="$with_system_libs"])

AC_ARG_WITH(system-nss,
    AS_HELP_STRING([--with-system-nss],
        [Use NSS/nspr libraries already on system.]),,
    [with_system_nss="$with_system_libs"])

AC_ARG_WITH(mozilla-toolkit,
    AS_HELP_STRING([--with-mozilla-toolkit],
        [Choose which GUI toolkit to use while building Mozilla components. (default=gtk2)]),
,)

AC_ARG_WITH(myspell-dicts,
    AS_HELP_STRING([--without-myspell-dicts],
        [Removes myspell dictionaries from LibreOffice installation set, for
         people building for specific distributions where the myspell dictionaries
         are installed from other sources.]),
,)

AC_ARG_WITH(system-dicts,
    AS_HELP_STRING([--without-system-dicts],
        [Do not use dictionaries from system paths.]),
,)

AC_ARG_WITH(external-dict-dir,
    AS_HELP_STRING([--with-external-dict-dir],
        [Specify external dictionary dir.]),
,)

AC_ARG_WITH(external-hyph-dir,
    AS_HELP_STRING([--with-external-hyph-dir],
        [Specify external hyphenation pattern dir.]),
,)

AC_ARG_WITH(external-thes-dir,
    AS_HELP_STRING([--with-external-thes-dir],
        [Specify external thesaurus dir.]),
,)

AC_ARG_WITH(system-zlib,
    AS_HELP_STRING([--with-system-zlib],
        [Use zlib already on system.]),,
    [with_system_zlib=auto])

AC_ARG_WITH(system-openssl,
    AS_HELP_STRING([--with-system-openssl],
        [Use OpenSSL already on system.]),,
    [with_system_openssl="$with_system_libs"])

AC_ARG_WITH(system-jpeg,
    AS_HELP_STRING([--with-system-jpeg],
        [Use jpeg already on system.]),,
    [with_system_jpeg=auto])

AC_ARG_WITH(system-clucene,
    AS_HELP_STRING([--with-system-clucene],
        [Use clucene already on system.]),,
    [with_system_clucene="$with_system_libs"])

AC_ARG_WITH(system-expat,
    AS_HELP_STRING([--with-system-expat],
        [Use expat already on system.]),,
    [with_system_expat="$with_system_libs"])

AC_ARG_WITH(system-libcmis,
    AS_HELP_STRING([--with-system-libcmis],
        [Use libcmis already on system.]),,
    [with_system_libcmis="$with_system_libs"])

AC_ARG_WITH(system-lcms2,
    AS_HELP_STRING([--with-system-lcms2],
        [Use littlecms v2 already on system.]),,
    [with_system_lcms2="$with_system_libs"])

AC_ARG_WITH(system-libcdr,
    AS_HELP_STRING([--with-system-libcdr],
        [Use libcdr already on system.]),,
    [with_system_libcdr="$with_system_libs"])

AC_ARG_WITH(system-libmspub,
    AS_HELP_STRING([--with-system-libmspub],
        [Use libmspub already on system.]),,
    [with_system_libmspub="$with_system_libs"])

AC_ARG_WITH(system-libvisio,
    AS_HELP_STRING([--with-system-libvisio],
        [Use libvisio already on system.]),,
    [with_system_libvisio="$with_system_libs"])

AC_ARG_WITH(system-libwpd,
    AS_HELP_STRING([--with-system-libwpd],
        [Use libwpd already on system.]),,
    [with_system_libwpd="$with_system_libs"])

AC_ARG_WITH(system-libwps,
    AS_HELP_STRING([--with-system-libwps],
        [Use libwps already on system.]),,
    [with_system_libwps="$with_system_libs"])

AC_ARG_WITH(system-libwpg,
    AS_HELP_STRING([--with-system-libwpg],
        [Use libwpg already on system.]),,
    [with_system_libwpg="$with_system_libs"])

AC_ARG_WITH(system-libxml,
    AS_HELP_STRING([--with-system-libxml],
        [Use libxml/libxslt already on system.]),,
    [with_system_libxml=auto])

AC_ARG_WITH(system-icu,
    AS_HELP_STRING([--with-system-icu],
        [Use icu already on system.]),,
    [with_system_icu="$with_system_libs"])

AC_ARG_WITH(system-ucpp,
    AS_HELP_STRING([--with-system-ucpp],
        [Use ucpp already on system.]),,
    [])

AC_ARG_WITH(system-openldap,
    AS_HELP_STRING([--with-system-openldap],
        [Use the OpenLDAP LDAP SDK already on system.]),,
    [with_system_openldap="$with_system_libs"])

AC_ARG_WITH(system-poppler,
    AS_HELP_STRING([--with-system-poppler],
        [Use system poppler. (only needed for pdfimport extension)]),,
    [with_system_poppler="$with_system_libs"])

AC_ARG_WITH(system-db,
    AS_HELP_STRING([--with-system-db],
        [Use Berkeley db already on system.]),,
    [with_system_db="$with_system_libs"])

AC_ARG_WITH(system-apache-commons,
    AS_HELP_STRING([--with-system-apache-commons],
        [Use Apache commons libraries already on system.]),,
    [with_system_apache_commons="$with_system_jars"])

AC_ARG_WITH(system-mysql,
    AS_HELP_STRING([--with-system-mysql],
        [Use MySQL libraries already on system, for building the MySQL Connector/LibreOffice
         extension. If the the mysql_config executable is not in PATH, use MYSQLCONFIG to
         point to it.]),,
    [with_system_mysql="$with_system_libs"])

AC_ARG_WITH(libmysql-path,
    AS_HELP_STRING([--with-libmysql-path],
        [Use Connector/C (libmysql) installation for building the MySQL
         Connector/LibreOffice extension.])
    [
                          Usage:     --with-libmysql-path=<absolute path to
                                                  your Connector/C installation>
    ],
,)

AC_ARG_WITH(system-mysql-cppconn,
    AS_HELP_STRING([--with-system-mysql-cppconn],
        [Use MySQL C++ Connector libraries already on system.]),,
    [with_system_mysql_cppconn="$with_system_libs"])

AC_ARG_WITH(system-postgresql,
    AS_HELP_STRING([--with-system-postgresql],
        [Use PostgreSQL libraries already on system, for building the PostgreSQL-SDBC
         driver. If pg_config is not in PATH, use PGCONFIG to point to it.]),,
    [with_system_postgresql="$with_system_libs"])

AC_ARG_WITH(libpq-path,
    AS_HELP_STRING([--with-libpq-path],
        [Use this PostgreSQL C interface (libpq) installation for building
         the PostgreSQL-SDBC extension.])
    [
                          Usage:     --with-libpq-path=<absolute path to
                                                  your libq installation>
    ],
,)

AC_ARG_WITH(system-hsqldb,
    AS_HELP_STRING([--with-system-hsqldb],
        [Use hsqldb already on system.]))

AC_ARG_WITH(hsqldb-jar,
    AS_HELP_STRING([--with-hsqldb-jar=JARFILE],
        [Specify path to jarfile manually.]),
    HSQLDB_JAR=$withval)

AC_ARG_ENABLE(scripting-beanshell,
    AS_HELP_STRING([--disable-scripting-beanshell],
        [Disable support for scripts in BeanShell.]),
,
)

AC_ARG_WITH(system-beanshell,
    AS_HELP_STRING([--with-system-beanshell],
        [Use beanshell already on system.]),,
    [with_system_beanshell="$with_system_jars"])

AC_ARG_WITH(beanshell-jar,
    AS_HELP_STRING([--with-beanshell-jar=JARFILE],
        [Specify path to jarfile manually.]),
    BSH_JAR=$withval)

AC_ARG_ENABLE(scripting-javascript,
    AS_HELP_STRING([--disable-scripting-javascript],
        [Disable support for scripts in JavaScript.]),
,
)

AC_ARG_WITH(system-rhino,
    AS_HELP_STRING([--with-system-rhino],
        [Use rhino already on system.]),,)
#    [with_system_rhino="$with_system_jars"])
# Above is not used as we have different debug interface
# patched into internal rhino. This code needs to be fixed
# before we can enable it by default.

AC_ARG_WITH(rhino-jar,
    AS_HELP_STRING([--with-rhino-jar=JARFILE],
        [Specify path to jarfile manually.]),
    RHINO_JAR=$withval)

AC_ARG_WITH(commons-codec-jar,
    AS_HELP_STRING([--with-commons-codec-jar=JARFILE],
        [Specify path to jarfile manually.]),
    COMMONS_CODEC_JAR=$withval)

AC_ARG_WITH(commons-lang-jar,
    AS_HELP_STRING([--with-commons-lang-jar=JARFILE],
        [Specify path to jarfile manually.]),
    COMMONS_LANG_JAR=$withval)

AC_ARG_WITH(commons-httpclient-jar,
    AS_HELP_STRING([--with-commons-httpclient-jar=JARFILE],
        [Specify path to jarfile manually.]),
    COMMONS_HTTPCLIENT_JAR=$withval)

AC_ARG_WITH(commons-logging-jar,
    AS_HELP_STRING([--with-commons-logging-jar=JARFILE],
        [Specify path to jarfile manually.]),
    COMMONS_LOGGING_JAR=$withval)

AC_ARG_WITH(system-servlet-api,
    AS_HELP_STRING([--with-system-servlet-api],
        [Use servlet-api already on system.]),,
    [with_system_servlet_api="$with_system_jars"])

AC_ARG_WITH(servlet-api-jar,
    AS_HELP_STRING([--with-servlet-api-jar=JARFILE],
        [Specify path to jarfile manually.]),
    SERVLETAPI_JAR=$withval)

AC_ARG_WITH(system-jfreereport,
    AS_HELP_STRING([--with-system-jfreereport],
        [Use JFreeReport already on system.]),,
    [with_system_jfreereport="$with_system_jars"])

AC_ARG_WITH(sac-jar,
    AS_HELP_STRING([--with-sac-jar=JARFILE],
        [Specify path to jarfile manually.]),
    SAC_JAR=$withval)

AC_ARG_WITH(libxml-jar,
    AS_HELP_STRING([--with-libxml-jar=JARFILE],
        [Specify path to jarfile manually.]),
    LIBXML_JAR=$withval)

AC_ARG_WITH(flute-jar,
    AS_HELP_STRING([--with-flute-jar=JARFILE],
        [Specify path to jarfile manually.]),
    FLUTE_JAR=$withval)

AC_ARG_WITH(jfreereport-jar,
    AS_HELP_STRING([--with-jfreereport-jar=JARFILE],
        [Specify path to jarfile manually.]),
    JFREEREPORT_JAR=$withval)

AC_ARG_WITH(liblayout-jar,
    AS_HELP_STRING([--with-liblayout-jar=JARFILE],
        [Specify path to jarfile manually.]),
    LIBLAYOUT_JAR=$withval)

AC_ARG_WITH(libloader-jar,
    AS_HELP_STRING([--with-libloader-jar=JARFILE],
        [Specify path to jarfile manually.]),
    LIBLOADER_JAR=$withval)

AC_ARG_WITH(libloader-jar,
    AS_HELP_STRING([--with-libloader-jar=JARFILE],
        [Specify path to jarfile manually.]),
    LIBLOADER_JAR=$withval)

AC_ARG_WITH(libformula-jar,
    AS_HELP_STRING([--with-libformula-jar=JARFILE],
        [Specify path to jarfile manually.]),
    LIBFORMULA_JAR=$withval)

AC_ARG_WITH(librepository-jar,
    AS_HELP_STRING([--with-librepository-jar=JARFILE],
        [Specify path to jarfile manually.]),
    LIBREPOSITORY_JAR=$withval)

AC_ARG_WITH(libfonts-jar,
    AS_HELP_STRING([--with-libfonts-jar=JARFILE],
        [Specify path to jarfile manually.]),
    LIBFONTS_JAR=$withval)

AC_ARG_WITH(libserializer-jar,
    AS_HELP_STRING([--with-libserializer-jar=JARFILE],
        [Specify path to jarfile manually.]),
    LIBSERIALIZER_JAR=$withval)

AC_ARG_WITH(libbase-jar,
    AS_HELP_STRING([--with-libbase-jar=JARFILE],
        [Specify path to jarfile manually.]),
    LIBBASE_JAR=$withval)

AC_ARG_WITH(system-odbc,
    AS_HELP_STRING([--with-system-odbc],
        [Use the odbc headers already on system.]),,
    [with_system_odbc="$with_system_headers"])

AC_ARG_WITH(system-sane,
    AS_HELP_STRING([--with-system-sane],
        [Use sane.h already on system.]),,
    [with_system_sane="$with_system_headers"])

AC_ARG_WITH(system-bluez,
    AS_HELP_STRING([--with-system-bluez],
        [Use bluetooth.h already on system.]),,
    [with_system_bluez="$with_system_headers"])

AC_ARG_WITH(system-xextensions-headers,
    AS_HELP_STRING([--with-system-xextensions-headers],
        [To build without system X11 extensions headers, use
         --without-system-xextensions-headers. This is possibly
         useful on legacy unix systems which ship with the libs
         but without the headers.]))

AC_ARG_WITH(system-mesa-headers,
    AS_HELP_STRING([--with-system-mesa-headers],
        [Use Mesa headers already on system.]),,
    [with_system_mesa_headers="$with_system_headers"])

AC_ARG_WITH(system-curl,
    AS_HELP_STRING([--with-system-curl],
        [Use curl already on system.]),,
    [with_system_curl=auto])

AC_ARG_WITH(system-boost,
    AS_HELP_STRING([--with-system-boost],
        [Use boost already on system.]),,
    [with_system_boost="$with_system_headers"])

AC_ARG_WITH(system-mdds,
    AS_HELP_STRING([--with-system-mdds],
        [Use mdds already on system.]),,
    [with_system_mdds="$with_system_headers"])

AC_ARG_WITH(system-vigra,
    AS_HELP_STRING([--with-system-vigra],
        [Use vigra already on system.]),,
    [with_system_vigra="$with_system_headers"])

AC_ARG_WITH(system-neon,
    AS_HELP_STRING([--with-system-neon],
        [Use neon already on system.]),,
    [with_system_neon="$with_system_libs"])

AC_ARG_WITH(system-hunspell,
    AS_HELP_STRING([--with-system-hunspell],
        [Use libhunspell already on system.]),,
    [with_system_hunspell="$with_system_libs"])

AC_ARG_WITH(system-mythes,
    AS_HELP_STRING([--with-system-mythes],
        [Use mythes already on system.]),,
    [with_system_mythes="$with_system_libs"])

AC_ARG_WITH(system-altlinuxhyph,
    AS_HELP_STRING([--with-system-altlinuxhyph],
        [Use ALTLinuxhyph already on system.]),,
    [with_system_altlinuxhyph="$with_system_libs"])

AC_ARG_WITH(system-lpsolve,
    AS_HELP_STRING([--with-system-lpsolve],
        [Use lpsolve already on system.]),,
    [with_system_lpsolve="$with_system_libs"])

AC_ARG_WITH(system-libexttextcat,
    AS_HELP_STRING([--with-system-libexttextcat],
        [Use libexttextcat already on system.]),,
    [with_system_libexttextcat="$with_system_libs"])

AC_ARG_WITH(system-cppunit,
    AS_HELP_STRING([--with-system-cppunit],
        [Use cppunit already on system.]),,
    [with_system_cppunit="$with_system_libs"])

AC_ARG_WITH(system-redland,
    AS_HELP_STRING([--with-system-redland],
        [Use redland library already on system.]),,
    [with_system_redland="$with_system_libs"])

AC_ARG_WITH(system-orcus,
    AS_HELP_STRING([--with-system-orcus],
        [Use orcus library already on system.]),,
    [with_system_orcus="$with_system_libs"])

AC_ARG_WITH(system-liblangtag,
    AS_HELP_STRING([--with-system-liblangtag],
        [Use liblangtag library already on system.]),,
    [with_system_liblangtag="$with_system_libs"])

AC_ARG_WITH(system-mozilla,
    AS_HELP_STRING([--with-system-mozilla],
        [Use Mozilla already on system. Note that some components cannot be built
         against a contemporary Mozilla. The flavour used can be specified by
         --with-system-mozilla=<flavour>. Supported are: libxul (default),
         xulrunner, firefox, seamonkey, mozilla, iceape.]),
    WITH_SYSTEM_MOZILLA=$withval ,
WITH_SYSTEM_MOZILLA=no)

AC_ARG_WITH(system-mozilla-headers,
    AS_HELP_STRING([--with-system-mozilla-headers],
        [Use mozilla headers provided by system instead of bundled ones. Used in
         nsplugin]),,
    [with_system_mozilla_headers="$with_system_headers"]
)

AC_ARG_WITH(system-gettext,
    AS_HELP_STRING([--with-system-gettext],
        [Use gettext runtime library already on system.]),,
    [with_system_gettext="$with_system_libs"])

AC_ARG_WITH(system-libpng,
    AS_HELP_STRING([--with-system-libpng],
        [Use libpng already on system.]),,
    [with_system_libpng=auto])

AC_ARG_WITH(linker-hash-style,
    AS_HELP_STRING([--with-linker-hash-style],
        [Use linker with --hash-style=<style> when linking shared objects.
         Possible values: "sysv", "gnu", "both". The default value is "gnu"
         if supported on the build system, and "sysv" otherwise.]))

AC_ARG_WITH(stlport,
    AS_HELP_STRING([--with-stlport],
        [Build the STLPort library for compatibility with old extensions for
         architectures where STLPort used to be used.]),
    with_stlport=$withval ,
with_stlport=auto)

AC_ARG_WITH(jdk-home,
    AS_HELP_STRING([--with-jdk-home],
        [If you have installed JDK 1.3 or later on your system please supply the
         path here. Note that this is not the location of the java command but the
         location of the entire distribution.])
    [
                          Usage:     --with-jdk-home=<absolute path to JDK home>
    ],
,)

AC_ARG_WITH(gxx_include_path,
    AS_HELP_STRING([--with-gxx-include-path],
        [If you want to override the autodetected g++ include path.])
    [
                          Usage:     --with-gxx-include-path=<absolute path to
                                                                g++ include dir>
    ],
,)

AC_ARG_WITH(help,
    AS_HELP_STRING([--without-help],
        [Disable the build of help.]))

AC_ARG_WITH(java,
    AS_HELP_STRING([--with-java],
        [Specify the name of the Java interpreter command. Typically "java"
         which is the default.

         To build without support for Java components, applets, accessibility
         or the XML filters written in Java, use --without-java or --with-java=no.])
    [
                          Usage:     --with-java==<java command>
                                     --without-java
    ],
    [ test -z "$with_java" -o "$with_java" = "yes" && with_java=java ],
    [ with_java=java ]
)

AC_ARG_WITH(java-target-version,
    AS_HELP_STRING([--with-java-target-version],
        [Generate class files that will work on JVMs with the specified version.
         For example, use --with-java-target-version=1.5 to make sure that the
         application will work with JVM 1.5 even when compiled with JDK 1.6.])
    [
                          This option is ignored when you compile with gcj/gij.

                          Usage:     --with-java-target-version=<jvm version>
    ],
,)

AC_ARG_WITH(jvm-path,
    AS_HELP_STRING([--with-jvm-path],
        [Use a specific JVM search path at runtime.])
    [
                          Usage:     --with-jvm-path=<absolute path to parent of jvm home>

                          e. g.: --with-jvm-path=/usr/lib/
                                 to find JRE/JDK in /usr/lib/jvm/
    ],
,)

AC_ARG_WITH(ant-home,
    AS_HELP_STRING([--with-ant-home],
        [If you have installed Jakarta Ant on your system, please supply the path here.
         Note that this is not the location of the Ant binary but the location
         of the entire distribution.])
    [
                          Usage:     --with-ant-home=<absolute path to Ant home>
    ],
,)

AC_ARG_WITH(junit,
    AS_HELP_STRING([--with-junit],
        [Specifies the JUnit 4 jar file to use for JUnit-based tests.
         --without-junit disables those tests. Not relevant in the --without-java case.])
    [
                          Usage:     --with-junit=<absolute path to JUnit 4 jar>
    ],
,with_junit=yes)

AC_ARG_WITH(perl-home,
    AS_HELP_STRING([--with-perl-home],
        [If you have installed Perl 5 Distribution, on your system, please
         supply the path here. Note that this is not the location of the Perl
         binary but the location of the entire distribution.])
    [
                          Usage:     --with-perl-home=<abs. path to Perl 5 home>
    ],
,)

AC_ARG_WITH(
    [doxygen],
    AS_HELP_STRING(
        [--with-doxygen],
        [Specifies the doxygen executable to use when generating ODK C/C++
         documentation. --without-doxygen disables generation of ODK C/C++
         documentation. Not relevant in the --disable-odk case.])
    [
                          Usage:     --with-doxygen=<absolute path to doxygen executable>
    ],,
    [with_doxygen=yes])

AC_ARG_WITH(cl-home,
    AS_HELP_STRING([--with-cl-home],
        [For Windows NT users, please supply the path for the Microsoft C/C++
         compiler. Note that this is not the location of the compiler binary but
         the location of the entire distribution.])
    [
                          Usage:     --with-cl-home=<absolute path to Microsoft
                                                            C/C++ compiler home>
    ],
,)

AC_ARG_WITH(mspdb-path,
    AS_HELP_STRING([--with-mspdb-path],
        [For Microsoft C/C++ compiler users, please supply the path pointing to
         the mspdb80.dll (if using Visual Studio 2008) or mspdb100.dll (if using
         Visual Studio 2010).])
    [
                          Usage:     --with-mspdb-path=<path to
                                                       mspdb80.dll/mspdb100.dll>
    ],
,)

AC_ARG_WITH(midl-path,
    AS_HELP_STRING([--with-midl-path],
        [For Microsoft compiler users, please supply the path pointing to the midl.exe.])
    [
                          Usage:     --with-midl-path=<abs. path to midl.exe>
    ],
,)

AC_ARG_WITH(csc-path,
    AS_HELP_STRING([--with-csc-path],
        [For Windows builds, please supply the path pointing to the csc.exe.
         Usually found automatically when building on Windows.])
    [
                          Usage:     --with-csc-path=<abs. path to csc.exe>
    ],
,)

AC_ARG_WITH(dotnet-framework-home,
    AS_HELP_STRING([--with-dotnet-framework-home],
        [For Microsoft compiler users, please supply the path pointing to
         lib/mscoree.lib, usually something like:
         "/cygdrive/c/Program Files/Windows SDKs/Windows/v7.0"])
    [
                          Note that in most cases it will be automatically
                          found, though.

                          Usage:     --with-dotnet-framework-home=<absolute path to .NET
                                                          Framework>
    ],
,)

AC_ARG_WITH(windows-sdk-home,
    AS_HELP_STRING([--with-windows-sdk-home],
        [For Windows builds, please supply the path to the Windows SDK.
         Usually found automatically when building on Windows.])
    [
                          Usage:     --with-windows-sdk-home=<absolute path to Windows SDK>
    ],
,)

AC_ARG_WITH(directx-home,
    AS_HELP_STRING([--with-directx-home],
        [For Windows users, please supply the path to the Microsoft DirectX SDK.])
    [
                          Usage:     --with-directx-home=<absolute path to
                                                          Microsoft DirectX SDK>
    ],
,)

AC_ARG_WITH(mozilla-build,
    AS_HELP_STRING([--with-mozilla-build],
        [For Windows users, please supply the path to the Mozilla build tools.])
    [
                          Usage:     --with-mozilla-build=<absolute path to
                                                          Mozilla build tools>

                          At the moment of this writing, an installer for the
                          mozilla build tools can be obtained from http://ftp.
                          mozilla.org/pub/mozilla.org/mozilla/libraries/win32.
    ],
    MOZILLABUILD=$withval ,
)

AC_ARG_WITH(lang,
    AS_HELP_STRING([--with-lang],
        [Use this option to build LibreOffice with additional language support.
         English (US) is always included by default.
         Separate multiple languages with space.
         For all languages, use --with-lang=ALL.])
    [
                          Usage:     --with-lang="es sw tu cs sk"
    ],
,)

# Kerberos and GSSAPI used only by PostgreSQL as of LibO 3.5
AC_ARG_WITH(krb5,
    AS_HELP_STRING([--with-krb5],
        [Enable MIT Kerberos 5 support in modules that support it.
         By default automatically enabled on platforms
         where a good system Kerberos 5 is available.]),
,)

AC_ARG_WITH(gssapi,
    AS_HELP_STRING([--with-gssapi],
        [Enable GSSAPI support in modules that support it.
         By default automatically enabled on platforms
         where a good system GSSAPI is available.]),
,)

dnl ===================================================================
dnl Branding
dnl ===================================================================

AC_ARG_WITH(branding,
[  --with-branding        Use given path to retrieve all branding images.
                          Expects /path/intro.png ...
                          /path/backing_rtl_right.png to be there. Individual
                          images can be overridden via --with-intro-bitmap ...
                          --with-startcenter-rtl-left-bitmap switches.

                          Usage:     --with-branding=/path/to/images
],,)

AC_ARG_WITH(intro-bitmap,
[  --with-intro-bitmap    Prefer the specified intro bitmap over the
                          the default one.

                          Usage:     --with-intro-bitmap=/path/my_ooo_intro.png
],,)

AC_ARG_WITH(intro-progressbar-color,
[  --with-intro-progressbar-color    Set color of progress bar on intro screen.
                                     Comma separated RGB values in decimal format.

                          Usage: --with-intro-progressbar-color=126,170,23
],,)

AC_ARG_WITH(intro-progressbar-size,
[  --with-intro-progressbar-size     Set size of progress bar on intro screen.
                                     Comma separated values in decimal format.

                          Usage: --with-intro-progressbar-size=319,10
],,)

AC_ARG_WITH(intro-progressbar-position,
[  --with-intro-progressbar-position Set position of progress bar on intro screen.
                                     Comma separated values in decimal format.

                          Usage: --with-intro-progressbar-position=164,225
],,)

AC_ARG_WITH(intro-progressbar-frame-color,
[  --with-intro-progressbar-frame-color    Set color of progress bar frame on intro screen.
                                           Comma separated RGB values in decimal format.

                          Usage: --with-intro-progressbar-frame-color=207,208,211
],,)

AC_ARG_WITH(intro-progressbar-text-color,
    AS_HELP_STRING([--with-intro-progressbar-text-color],
        [Set color of progress bar text on intro screen. Comma separated RGB values in decimal format.])
    [
                          Usage: --with-intro-progressbar-text-color=207,208,211
],,)

AC_ARG_WITH(intro-progressbar-text-baseline,
    AS_HELP_STRING([--with-intro-progressbar-text-baseline],
        [Set vertical position of progress bar text on intro screen. Value in decimal format.])
    [
                          Usage: --with-intro-progressbar-text-baseline=250
],,)

AC_ARG_WITH(flat-logo-svg,
[  --with-flat-logo-svg    Allows specification of the flat Logo SVG.

                          Usage:     --with-flat-logo-svg=/path/my_flat_logo.svg
],,)

AC_ARG_WITH(about-background-svg,
[  --with-about-background-svg    Allows specification of the background SVG for the About dialog.

                          Usage:     --with-about-background-svg=/path/my_libo_about.svg
],,)

AC_ARG_WITH(startcenter-left-bitmap,
[  --with-startcenter-left-bitmap    Similarly to --with-intro-bitmap, this allows
                                     specification of bitmap for the Start center.

                          Usage: --with-startcenter-left-bitmap=/path/my_backing_left.png
],,)

AC_ARG_WITH(startcenter-right-bitmap,
[  --with-startcenter-right-bitmap    Similarly to --with-intro-bitmap, this allows
                                      specification of bitmap for the Start center.

                          Usage: --with-startcenter-right-bitmap=/path/my_backing_right.png
],,)

AC_ARG_WITH(startcenter-rtl-left-bitmap,
[  --with-startcenter-rtl-left-bitmap    Similarly to --with-intro-bitmap, this allows
                                         specification of bitmap for the Start center.

                          Usage: --with-startcenter-rtl-left-bitmap=/path/my_backing_rtl_left.png
],,)

AC_ARG_WITH(startcenter-rtl-right-bitmap,
[  --with-startcenter-rtl-right-bitmap    Similarly to --with-intro-bitmap, this allows
                                          specification of bitmap for the Start center.

                          Usage: --with-startcenter-rtl-right-bitmap=/path/my_backing_rtl_right.png
],,)

AC_ARG_WITH(startcenter-space-bitmap,
[  --with-startcenter-space-bitmap    Similarly to --with-intro-bitmap, this allows
                                      specification of bitmap for the Start center.

                          Usage: --with-startcenter-space-bitmap=/path/my_backing_space.png
],,)

AC_ARG_WITH(vendor,
    AS_HELP_STRING([--with-vendor],
        [Set vendor of the build.])
    [
                          Usage:     --with-vendor="John the Builder"
    ],
,)

AC_ARG_WITH(unix-wrapper,
    AS_HELP_STRING([--with-unix-wrapper],
        [Redefines the name of the UNIX wrapper that will be used in the desktop
         files and in the desktop-integration RPMs.])
    [
                          Usage:     --with-unix-wrapper=ooffice
    ],
,)

AC_ARG_WITH(compat-oowrappers,
    AS_HELP_STRING([--with-compat-oowrappers],
        [Install oo* wrappers in parallel with
         lo* ones to keep backward compatibility.
         Has effect only with make distro-pack-install]),
,)

AC_ARG_WITH(asm-home,
    AS_HELP_STRING([--with-asm-home],
        [For Windows, please supply the path for the ml.exe or ml64.exe assembler.])
    [
                          Usage:     --with-asm-home=<path to assembler directory>
    ],
,)

AC_ARG_WITH(os-version,
    AS_HELP_STRING([--with-os-version],
        [For FreeBSD users, use this option option to override the detected OSVERSION.])
    [
                          Usage:     --with-os-version=<OSVERSION>
    ],
,)

AC_ARG_WITH(mingw-cross-compiler,
    AS_HELP_STRING([--with-mingw-cross-compiler],
        [Specify the MinGW cross-compiler to use.])
    [
                          Usage:     --with-mingw-cross-compiler=<mingw32-g++ command>

                          When building on the ODK on Unix and building unowinreg.dll,
                          specify the MinGW C++ cross-compiler.
    ],
,)

AC_ARG_WITH(idlc-cpp,
    AS_HELP_STRING([--with-idlc-cpp],
        [Specify the C Preprocessor to use for idlc.])
    [
                          Usage:     --with-idlc-cpp=cpp

                          Default is ucpp.
    ]
,)

AC_ARG_WITH(build-version,
    AS_HELP_STRING([--with-build-version],
        [Allows the builder to add a custom version tag that will appear in the
         Help/About box for QA purposes.])
    [
                          Usage:     --with-build-version="Built by Jim"
    ],
    with_build_version=$withval ,
)

AC_ARG_WITH(alloc,
    AS_HELP_STRING([--with-alloc],
        [Define which allocator to build with (choices are oo, system, tcmalloc, jemalloc).
         Note that on FreeBSD/NetBSD system==jemalloc]),
,)

AC_ARG_WITH(sun-templates,
    AS_HELP_STRING([--with-sun-templates],
        [Integrate Sun template packages.]),
,)

AC_ARG_WITH(num-cpus,
    AS_HELP_STRING([--with-num-cpus],
        [Number of build processes/cpus to use (number of projects that will build at the same time).
         Multi-process/multi-cpu builds can save a lot of time on multi-cpu machines.
         Defaults to the number of CPUs on the machine.]),
,)

AC_ARG_WITH(max-jobs,
    AS_HELP_STRING([--with-max-jobs],
        [Maximum number of jobs that will be issued at the same time per dmake or gbuild subproject.
         The real number of the jobs is affected by the --with-num-cpus too, it can get up to CPUS*max_jobs.
         Defaults to 1, unless you configure --enable-icecream - then to 10.]),
,)

AC_ARG_WITH(check-jobs,
    AS_HELP_STRING([--with-check-jobs],
        [Maximum number of jobs that will be issued at the same time during 'make subsequenttest'.
         Default value is the same as 'max-jobs'. Useful because the subsequent tests are very lighweight.]),
,)

dnl ===================================================================
dnl Test whether build target is Release Build
dnl ===================================================================
AC_MSG_CHECKING([whether build target is Release Build])
if test "$enable_release_build" = "" -o "$enable_release_build" = "no"; then
    AC_MSG_RESULT([no])
    ENABLE_RELEASE_BUILD="FALSE"
else
    AC_MSG_RESULT([yes])
    ENABLE_RELEASE_BUILD="TRUE"
fi
AC_SUBST(ENABLE_RELEASE_BUILD)

dnl ===================================================================
dnl MacOSX build and runtime environment options
dnl ===================================================================

AC_ARG_WITH(macosx-sdk,
    AS_HELP_STRING([--with-macosx-sdk],
        [Use a specific SDK for building.])
    [
                          Usage:     --with-macosx-sdk=<version>

                          e. g.: --with-macosx-sdk=10.4

                          there are 3 options to control the MacOSX build:
                          --with-macosx-sdk (refered as 'sdk' below)
                          --with-macosx-version-min-required (refered as 'min' below)
                          --with-macosx-version-max-allowed (refered as 'max' below)

                          the connection between these value and the default they take is as follow:
                          ( ? means not specified on the command line, s means the SDK version found,
                          constraint: x <= y <= z)

                          ==========================================
                           command line      || config result
                          ==========================================
                          min  | max  | sdk  || min  | max  | sdk  |
                          ?    | ?    | ?    || 10.4 | 10.s | 10.s |
                          ?    | ?    | 10.x || 10.4 | 10.x | 10.x |
                          ?    | 10.x | ?    || 10.4 | 10.s | 10.s |
                          ?    | 10.x | 10.y || 10.4 | 10.x | 10.y |
                          10.x | ?    | ?    || 10.x | 10.s | 10.s |
                          10.x | ?    | 10.y || 10.x | 10.y | 10.y |
                          10.x | 10.y | ?    || 10.x | 10.y | 10.y |
                          10.x | 10.y | 10.z || 10.x | 10.y | 10.z |


                          see: http://developer.apple.com/library/mac/#technotes/tn2064/_index.html
                          for a detailled technical explanation of these variables

                          Note: MACOSX_DEPLOYMENT_TARGET will be set to the value of 'min'.

                          Note that even if in theory using a --with-macosx-version-max-allowed
                          (i.e. the MAC_OS_X_VERSION_MAX_ALLOWED macro) less than the SDK version
                          should work, in practice Apple doesn't seem to test that, and at least
                          compiling with -DMAC_OS_X_VERSION_MAX_ALLOWED=1060 against the 10.7 SDK
                          fails in a couple of places. Just because of oversights in ifdefs in the SDK
                          headers, but still.
    ],
,)

AC_ARG_WITH(macosx-version-min-required,
    AS_HELP_STRING([--with-macosx-version-min-required],
        [set the minimum OS version needed to run the built LibreOffice])
    [
                          Usage:     --with-macosx-version-min-required=<version>

                          e. g.: --with-macos-version-min-required=10.4
                          see --with-macosx-sdk for more info
    ],
,)

AC_ARG_WITH(macosx-version-max-allowed,
    AS_HELP_STRING([--with-macosx-version-max-allowed],
        [set the maximum allowed OS version the LibreOffice compilation can use APIs from])
    [
                          Usage:     --with-macosx-version-max-allowed=<version>

                          e. g.: --with-macos-version-max-allowed=10.6
                          see --with-macosx-sdk for more info
    ],
,)


dnl ===================================================================
dnl options for stuff used during cross-compilation build
dnl These are superseded by --with-build-platform-configure-options
dnl ===================================================================

AC_ARG_WITH(system-boost-for-build,
    AS_HELP_STRING([--with-system-boost-for-build],
        [Use boost already on system for build tools (cross-compilation only).]))

AC_ARG_WITH(system-cppunit-for-build,
    AS_HELP_STRING([--with-system-cppunit-for-build],
        [Use cppunit already on system for build tools (cross-compilation only).]))

AC_ARG_WITH(system-db-for-build,
    AS_HELP_STRING([--with-system-db-for-build],
        [Use db already on system for build tools (cross-compilation only).]))

AC_ARG_WITH(system-expat-for-build,
    AS_HELP_STRING([--with-system-expat-for-build],
        [Use expat already on system for build tools (cross-compilation only).]))

AC_ARG_WITH(system-icu-for-build,
    AS_HELP_STRING([--with-system-icu-for-build=yes/no/force],
        [Use icu already on system for build tools (cross-compilation only).]))

AC_ARG_WITH(system-libxml-for-build,
    AS_HELP_STRING([--with-system-libxml-for-build],
        [Use libxml/libxslt already on system for build tools (cross-compilation only).]))


dnl ===================================================================
dnl check for required programs (grep, awk, sed, bash)
dnl ===================================================================

pathmunge ()
{
    if test -n "$1"; then
        if test "$build_os" = "cygwin"; then
            PathFormat "$1"
            new_path=`cygpath -u "$formatted_path"`
        else
            new_path="$1"
        fi
        if ! echo "$LO_PATH" | $EGREP -q "(^|:)$1($|:)"; then
            if test "$2" = "after"; then
                LO_PATH="$LO_PATH:$new_path"
            else
                LO_PATH="$new_path:$LO_PATH"
            fi
        fi
        unset new_path
    fi
}

AC_PROG_AWK
AC_PATH_PROG( AWK, $AWK)
if test -z "$AWK"; then
    AC_MSG_ERROR([install awk to run this script])
fi

AC_PATH_PROG(BASH, bash)
if test -z "$BASH"; then
    AC_MSG_ERROR([bash not found in \$PATH])
fi
AC_SUBST(BASH)

AC_MSG_CHECKING([for GNU or BSD tar])
for a in $GNUTAR gtar gnutar tar /usr/sfw/bin/gtar; do
    $a --version 2> /dev/null | egrep "GNU|bsdtar"  2>&1 > /dev/null
    if test $? -eq 0;  then
        GNUTAR=$a
        break
    fi
done
AC_MSG_RESULT($GNUTAR)
if test -z "$GNUTAR"; then
    AC_MSG_ERROR([not found. install GNU or BSD tar.])
fi
AC_SUBST(GNUTAR)

AC_MSG_CHECKING([for tar's option to strip components])
$GNUTAR --help 2> /dev/null | egrep "bsdtar|strip-components" 2>&1 >/dev/null
if test $? -eq 0; then
    STRIP_COMPONENTS="--strip-components"
else
    $GNUTAR --help 2> /dev/null | egrep "strip-path" 2>&1 >/dev/null
    if test $? -eq 0; then
        STRIP_COMPONENTS="--strip-path"
    else
        STRIP_COMPONENTS="unsupported"
    fi
fi
AC_MSG_RESULT($STRIP_COMPONENTS)
if test x$STRIP_COMPONENTS == xunsupported; then
    AC_MSG_ERROR([you need a tar that is able to strip components.])
fi
AC_SUBST(STRIP_COMPONENTS)

dnl It is useful to have a BUILD_TYPE keyword to distinguish "normal"
dnl desktop OSes from "mobile" ones.

dnl We assume that a non-DESKTOP build type is also a non-NATIVE one.
dnl In other words, that when building for an OS that is not a
dnl "desktop" one but a "mobile" one, we are always cross-compiling.

dnl Note the direction of the implication; there is no assumption that
dnl cross-compiling would imply a non-desktop OS.

if test $_os != iOS -a $_os != Android; then
    BUILD_TYPE="$BUILD_TYPE DESKTOP"
fi

dnl Decide whether to build database connectivity stuff (including
dnl Base) or not. We probably don't want to on non-desktop OSes.

if test -z "$enable_database_connectivity"; then
    # Do enable database connectivity for Android for now as otherwise
    # we presumably will get linking errors... We are not as far in
    # the work for iOS, so we might as well disable it for iOS already.

    # And actually, do enable it for iOS, too. Let's get back to
    # figuring out what to do with this later, if ever.

    # (Note that with "enable", I mean "enable building the related
    # code". Very likely little of it will make any sense at run-time
    # on Android or iOS and won't even be shipped with/linked into any
    # app.)

    #if test $_os != iOS; then
        enable_database_connectivity=yes
    #fi
fi

DISABLE_DBCONNECTIVITY=''
if test "$enable_database_connectivity" = yes; then
    BUILD_TYPE="$BUILD_TYPE DBCONNECTIVITY"
else
    DISABLE_DBCONNECTIVITY='TRUE'
    SCPDEFS="$SCPDEFS -DDISABLE_DBCONNECTIVITY"
fi
AC_SUBST(DISABLE_DBCONNECTIVITY)

if test -z "$enable_extensions"; then
    # For iOS disable extensions unless specifically overridden with
    # --enable-extensions.
    if test $_os != iOS; then
        enable_extensions=yes
    fi
fi

DISABLE_EXTENSIONS=''
if test "$enable_extensions" = yes; then
    BUILD_TYPE="$BUILD_TYPE EXTENSIONS"
else
    DISABLE_EXTENSIONS='TRUE'
fi
AC_SUBST(DISABLE_EXTENSIONS)

if test -z "$enable_scripting"; then
    # Disable scripting for iOS unless specifically overridden
    # with --enable-scripting.
    if test $_os != iOS; then
        enable_scripting=yes
    fi
fi

DISABLE_SCRIPTING=''
if test "$enable_scripting" = yes; then
    BUILD_TYPE="$BUILD_TYPE SCRIPTING"
else
    DISABLE_SCRIPTING='TRUE'
    SCPDEFS="$SCPDEFS -DDISABLE_SCRIPTING"
fi
AC_SUBST(DISABLE_SCRIPTING)

if test $_os = iOS -o $_os = Android; then
    # Disable dynamic_loading always for iOS and Android
    enable_dynamic_loading=no
elif test -z "$enable_dynamic_loading"; then
    # Otherwise enable it unless speficically disabled
    enable_dynamic_loading=yes
fi

DISABLE_DYNLOADING=''
if test "$enable_dynamic_loading" = yes; then
    BUILD_TYPE="$BUILD_TYPE DYNLOADING"
else
    DISABLE_DYNLOADING='TRUE'
    SCPDEFS="$SCPDEFS -DDISABLE_DYNLOADING"
fi
AC_SUBST(DISABLE_DYNLOADING)

if test -n "${with_solver_and_workdir_root}"; then
    if ! test -d ${with_solver_and_workdir_root}; then
      AC_MSG_ERROR([directory does not exist: ${with_solver_and_workdir_root}])
    fi
    PathFormat "${with_solver_and_workdir_root}"
    # IsValidFilePath in /sal/osl/w32/file_url.cxx rejects "X:\\" breaking idlc
    if echo ${formatted_path} | $GREP -q '/$'; then
        SOLARVER=${formatted_path}solver
    else
        SOLARVER=${formatted_path}/solver
    fi
else
    SOLARVER=${SRC_ROOT}/solver
fi

dnl ===================================================================
dnl Extra check for Windows. Cygwin builds need gcc to build dmake
dnl although MSVC is used to build other build-time tools and
dnl LibreOffice itself.
dnl ===================================================================
if test "$build_os" = "cygwin"; then
    AC_MSG_CHECKING([for Cygwin gcc/g++])
    if which gcc > /dev/null && which g++ > /dev/null; then
        AC_MSG_RESULT([found])
    else
        AC_MSG_ERROR([Cygwin gcc and g++ are needed, please install them.])
    fi
fi

# remenber SYSBASE value
AC_SUBST(SYSBASE)

dnl ===================================================================
dnl  Checks if ccache is available
dnl ===================================================================
if test "$enable_ccache" = "yes" -o \( "$enable_ccache" = "" -a "$enable_icecream" != "yes" \); then
    case "%$CC%$CXX%" in
    # If $CC and/or $CXX already contain "ccache" (possibly suffixed with some verison number etc),
    # assume that's good then
    *%ccache[[-_' ']]*|*/ccache[[-_' ']]*)
        AC_MSG_NOTICE([ccache seems to be included in a pre-defined CC and/or CXX])
        ;;
    *)
        AC_PATH_PROG([CCACHE],[ccache],[not found])
        if test "$CCACHE" = "not found"; then
            CCACHE=""
        else
            # Need to check for ccache version: otherwise prevents
            # caching of the results (like "-x objective-c++" for Mac)
            if test $_os = Darwin -o $_os = iOS; then
                # Check ccache version
                AC_MSG_CHECKING([whether version of ccache is suitable])
                CCACHE_VERSION=`"$CCACHE" -V | "$AWK" '/^ccache version/{print $3}'`
                CCACHE_NUMVER=`echo $CCACHE_VERSION | $AWK -F. '{ print \$1*10000+\$2*100+\$3 }'`
                if test "$CCACHE_VERSION" = "2.4_OOo" -o "$CCACHE_NUMVER" -ge "030100"; then
                    AC_MSG_RESULT([yes, $CCACHE_VERSION])
                else
                    AC_MSG_RESULT([no, $CCACHE_VERSION])
                    CCACHE=""
                fi
            fi
        fi
        ;;
    esac
else
    CCACHE=""
fi

if test "$CCACHE" != ""; then
    ccache_size_msg=$([ccache -s | tail -n 1 | sed 's/^[^0-9]*//' | sed -e 's/\.[0-9]*//'])
    ccache_size=$(echo "$ccache_size_msg" | grep "G" | sed -e 's/G.*$//')
    if test "$ccache_size" = ""; then
        ccache_size=$(echo "$ccache_size_msg" | grep "M" | sed -e 's/\ M.*$//')
        if test "$ccache_size" = ""; then
            ccache_size=0
        fi
        # we could not determine the size or it was less than 1GB -> disable auto-ccache
        if test $ccache_size -lt 1024; then
            CCACHE=""
            AC_MSG_WARN([ccache's cache size is less than 1GB using it is counter-producive: Disabling auto-ccache detection])
            add_warning "ccache's cache size is less than 1GB using it is counter-producive: auto-ccache detection disabled"
        else
            # warn that ccache may be too small for debug build
            AC_MSG_WARN([ccache's cache size is less than 5GB using it may be counter-producive for debug or symbol-enabled build])
            add_warning "ccache's cache size is less than 5GB using it may be counter-producive for debug or symbol-enabled build"
        fi
    else
        if test $ccache_size -lt 5; then
            #warn that ccache may be too small for debug build
            AC_MSG_WARN([ccache's cache size is less than 5GB using it may be counter-producive for debug or symbol-enabled build])
            add_warning "ccache's cache size is less than 5GB using it may be counter-producive for debug or symbol-enabled build"
        fi
    fi
fi

dnl ===================================================================
dnl  Checks for C compiler,
dnl  The check for the C++ compiler is later on.
dnl ===================================================================
GCC_HOME_SET="true"
AC_MSG_CHECKING([gcc home])
if test -z "$with_gcc_home"; then
    if test "$enable_icecream" = "yes"; then
        if test -d "/usr/lib/icecc/bin"; then
            GCC_HOME="/usr/lib/icecc/"
        else
            GCC_HOME="/opt/icecream/"
        fi
    else
        GCC_HOME=`which gcc | $SED -e s,/bin/gcc,,`
        GCC_HOME_SET="false"
    fi
else
    GCC_HOME="$with_gcc_home"
fi
AC_MSG_RESULT($GCC_HOME)
AC_SUBST(GCC_HOME)

if test "$GCC_HOME_SET" = "true"; then
    if test -z "$CC"; then
        CC="$GCC_HOME/bin/gcc"
    fi
    if test -z "$CXX"; then
        CXX="$GCC_HOME/bin/g++"
    fi
fi

dnl The following checks for gcc, cc and then cl (if it weren't guarded for win32)
if test "$_os" != "WINNT" -o "$WITH_MINGW" = "yes"; then
    # AC_PROG_CC sets CFLAGS to -g -O2 if not set, avoid that
    save_CFLAGS=$CFLAGS
    AC_PROG_CC
    CFLAGS=$save_CFLAGS
fi

COMPATH=`dirname "$CC"`
if test "$COMPATH" = "."; then
    AC_PATH_PROGS(COMPATH, $CC)
    dnl double square bracket to get single because of M4 quote...
    COMPATH=`echo $COMPATH | $SED "s@/[[^/:]]*\\\$@@"`
fi
COMPATH=`echo $COMPATH | $SED "s@/[[Bb]][[Ii]][[Nn]]\\\$@@"`

dnl ===================================================================
dnl  Test the Solaris compiler version
dnl ===================================================================
if test "$_os" = "SunOS"; then
    if test "$CC" = "cc"; then
        AC_PATH_PROGS(_cc, cc)
        COMPATH=`echo $_cc | $SED -n "s/\/bin\/cc//p"`
        AC_MSG_CHECKING([the SunStudio C/C++ compiler version])
        dnl cc -V outputs to standard error!!!!
        _sunstudio_string=`$CC -V 2>&1 | grep '^cc' | $SED -e 's/.* C //'`
        _sunstudio_version=`echo $_sunstudio_string | $AWK '{ print $1 }'`
        _sunstudio_major=`echo $_sunstudio_version | $AWK -F. '{ print $1 }'`
        if test "$_sunstudio_major" != "5"; then
            AC_MSG_ERROR([found version "$_sunstudio_version", use version 5.5, 5.7, 5.8 or 5.9 of the SunStudio C/C++ compiler])
        else
            _sunstudio_minor=`echo $_sunstudio_version | $AWK -F. '{ if ($2 == 5) print "true"; else if ($2 == 7) print "true"; else if ($2 == 8) print "true"; else if ($2 == 9) print "true"; else print "false" }'`
            if test "$_sunstudio_minor" = "false"; then
                AC_MSG_ERROR([found version "$_sunstudio_version", use version 5.5, 5.7, 5.8 or 5.9 of the SunStudio C/C++ compiler])
            else
                dnl compiler will do
                AC_MSG_RESULT([checked])
            fi
        fi
    fi
fi


dnl ===================================================================
dnl Check / find MacOSX SDK and compiler, version checks
dnl ===================================================================
if test "$_os" = "Darwin"; then

    if test "$build_cpu" = i386 -a "$host_cpu" = powerpc; then
        # Cross-compiling for PPC from Intel
        arch='-arch ppc'
    elif test "$enable_64_bit" = "" -o "$enable_64_bit" = "no"; then
        bitness=-m32
    else
        bitness=-m64
        BITNESS_OVERRIDE=64
    fi

    # If no --with-macosx-sdk option is given, first look for the 10.4u
    # SDK (which is distributed with the obsolete Xcode 3), then the
    # 10.6, 10.7 and 10.8 SDKs, in that order. (Don't bother looking
    # for the 10.5 SDK, unlikely somebody would have that but not
    # 10.6, I think.) If not found in some (old) default locations,
    # try the xcode-select tool.

    # The intent is that for "most" Mac-based developers, a suitable
    # SDK will be found automatically without any configure options.

    # For developers still using Xcode 2 or 3, in /Developer, either
    # because it is the only Xcode they have, or they have that in
    # addition to Xcode 4 in /Applications/Xcode.app, the 10.4 SDK
    # should be found.

    # For developers with a current Xcode 4 installed from the Mac App
    # Store, the 10.6, 10.7 or 10.8 SDK should be found.

    AC_MSG_CHECKING([what Mac OS X SDK to use])

    if test -z "$with_macosx_sdk"; then
        if test -d /Developer/SDKs/MacOSX10.4u.sdk; then
            with_macosx_sdk=10.4
        elif test -d /Developer-old/SDKs/MacOSX10.4u.sdk; then
            with_macosx_sdk=10.4
        elif test -d /Xcode3/SDKs/MacOSX10.4u.sdk; then
            with_macosx_sdk=10.4
        elif test -d /Developer/SDKs/MacOSX10.6.sdk; then
            with_macosx_sdk=10.6
        elif test -d /Developer/SDKs/MacOSX10.7.sdk; then
            with_macosx_sdk=10.7
        elif test -x /usr/bin/xcode-select; then
            xcodepath="`xcode-select -print-path`"
            if test -d "$xcodepath/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.6.sdk"; then
                with_macosx_sdk=10.6
            elif test -d "$xcodepath/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk"; then
                with_macosx_sdk=10.7
            elif test -d "$xcodepath/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk"; then
                with_macosx_sdk=10.8
            fi
        fi
        if test -z "$with_macosx_sdk"; then
            AC_MSG_ERROR([Could not figure out the location of a Mac OS X SDK and its version])
        fi
    fi

    case $with_macosx_sdk in
    10.4)
        MACOSX_SDK_VERSION=1040
        ;;
    10.5)
        MACOSX_SDK_VERSION=1050
        ;;
    10.6)
        MACOSX_SDK_VERSION=1060
        ;;
    10.7)
        MACOSX_SDK_VERSION=1070
        ;;
    10.8)
        MACOSX_SDK_VERSION=1080
        ;;
    *)
        AC_MSG_ERROR([with-macosx-sdk $with_macosx_sdk is not a supported value, supported value are 10.4--8])
        ;;
    esac

    # Next find it (again, if we deduced its version above by finding
    # it... but we need to look for it once more in case
    # --with-macosx-sdk was given so that the aboce search did not
    # happen).
    if test -z "$MACOSX_SDK_PATH"; then
        case $with_macosx_sdk in
        10.4)
            if test -d /Developer/SDKs/MacOSX10.4u.sdk; then
                MACOSX_SDK_PATH=/Developer/SDKs/MacOSX10.4u.sdk
            elif test -d /Developer-old/SDKs/MacOSX10.4u.sdk; then
                MACOSX_SDK_PATH=/Developer-old/SDKs/MacOSX10.4u.sdk
            elif test -d /Xcode3/SDKs/MacOSX10.4u.sdk; then
                MACOSX_SDK_PATH=/Xcode3/SDKs/MacOSX10.4u.sdk
            fi
            ;;
        10.6|10.7|10.8)
            if test -d /Developer/SDKs/MacOSX$with_macosx_sdk.sdk; then
                MACOSX_SDK_PATH=/Developer/SDKs/MacOSX$with_macosx_sdk.sdk
            elif test -x /usr/bin/xcode-select; then
                xcodepath="`xcode-select -print-path`"
                if test -d "$xcodepath/Platforms/MacOSX.platform/Developer/SDKs/MacOSX$with_macosx_sdk.sdk"; then
                    MACOSX_SDK_PATH="$xcodepath/Platforms/MacOSX.platform/Developer/SDKs/MacOSX$with_macosx_sdk.sdk"
                fi
            fi
            ;;
        esac
        if test -z "$MACOSX_SDK_PATH"; then
            AC_MSG_ERROR([Could not figure out the location of Mac OS X $with_macosx_sdk SDK])
        fi
    fi
    AC_MSG_RESULT([SDK $with_macosx_sdk at $MACOSX_SDK_PATH])

    if test "$with_macosx_version_min_required" = ""; then
        case $with_macosx_sdk in
        10.4|10.5)
            with_macosx_version_min_required="10.4";;
        *)
            with_macosx_version_min_required="10.6";;
        esac
    fi

    if test "$with_macosx_version_max_allowed" = ""; then
        with_macosx_version_max_allowed="$with_macosx_sdk"
    fi

    FRAMEWORKSHOME="$MACOSX_SDK_PATH/System/Library/Frameworks"
    MACOSX_DEPLOYMENT_TARGET="$with_macosx_version_min_required"

    case "$with_macosx_version_min_required" in
    10.4)
        MAC_OS_X_VERSION_MIN_REQUIRED="1040"
        ;;
    10.5)
        MAC_OS_X_VERSION_MIN_REQUIRED="1050"
        ;;
    10.6)
        MAC_OS_X_VERSION_MIN_REQUIRED="1060"
        ;;
    10.7)
        MAC_OS_X_VERSION_MIN_REQUIRED="1070"
        ;;
    10.8)
        MAC_OS_X_VERSION_MIN_REQUIRED="1080"
        ;;
    *)
        AC_MSG_ERROR([with-macosx-version-min-required $with_macosx_version_min_required is not a supported value, supported value are 10.4--8])
        ;;
    esac

    if test "$BITNESS_OVERRIDE" = 64; then
        case $with_macosx_version_min_required in
        10.4|10.5)
            AC_MSG_ERROR([Can't build 64-bit code for with-macosx-version-min-required=$with_macosx_version_min_required])
            ;;
        esac
    fi

    case "$with_macosx_version_min_required" in
    10.4)
        case "$with_macosx_sdk" in
        10.4)
            ;;
        *)
            AC_MSG_WARN([Building with a SDK > 10.4 possibly breaks 10.4 compatibility. Do not use for deliverable build until verified that it really works])
            add_warning "Building with a SDK > 10.4 possibly breaks 10.4 compatibility. Do not use for deliverable build until verified that it really works"
            ;;
        esac
        ;;
    *)
        AC_MSG_WARN([Building with a minimum version requirement > 10.4 breaks 10.4 compatibility. Do not use for deliverable build])
        add_warning "Building with a minimum version requirement > 10.4 breaks 10.4 compatibility. Do not use for deliverable build"
        ;;
    esac

    # If no CC and CXX environment vars, try to guess where the compiler is
    if test -z "$save_CC"; then
        AC_MSG_CHECKING([what compiler to use])
        case $with_macosx_sdk in
        10.4)
            case "$MACOSX_SDK_PATH" in
            /Developer/*)
                gccprefix=""
                ;;
            /Developer-old/*)
                gccprefix=/Developer-old/usr/bin/
                ;;
            /Xcode3/*)
                gccprefix=/Xcode3/usr/bin/
                ;;
            *)
                AC_MSG_ERROR([Cannot guess gcc location for this SDK])
                ;;
            esac
            CC="${gccprefix}gcc-4.0 $arch -mmacosx-version-min=$with_macosx_version_min_required -isysroot $MACOSX_SDK_PATH"
            CXX="${gccprefix}g++-4.0 $arch -mmacosx-version-min=$with_macosx_version_min_required -isysroot $MACOSX_SDK_PATH"
            ;;
        10.6)
            # Is similar logic as above needed? Is it likely somebody
            # has both an older Xcode with the 10.6 SDK and a current
            # Xcode?
            CC="gcc-4.2 $bitness -mmacosx-version-min=$with_macosx_version_min_required -isysroot $MACOSX_SDK_PATH"
            CXX="g++-4.2 $bitness -mmacosx-version-min=$with_macosx_version_min_required -isysroot $MACOSX_SDK_PATH"
            ;;
        10.7|10.8)
            CC="`xcrun -find clang` $bitness -mmacosx-version-min=$with_macosx_version_min_required -isysroot $MACOSX_SDK_PATH"
            CXX="`xcrun -find clang++` $bitness -mmacosx-version-min=$with_macosx_version_min_required -isysroot $MACOSX_SDK_PATH"
            ;;
        esac
        AC_MSG_RESULT([$CC and $CXX])
    fi

    case "$with_macosx_version_max_allowed" in
    10.4)
        MAC_OS_X_VERSION_MAX_ALLOWED="1040"
        ;;
    10.5)
        MAC_OS_X_VERSION_MAX_ALLOWED="1050"
        ;;
    10.6)
        MAC_OS_X_VERSION_MAX_ALLOWED="1060"
        ;;
    10.7)
        MAC_OS_X_VERSION_MAX_ALLOWED="1070"
        ;;
    10.8)
        MAC_OS_X_VERSION_MAX_ALLOWED="1080"
        ;;
    *)
        AC_MSG_ERROR([with-macosx-version-max-allowed $with_macosx_version_max_allowed is not a supported value, supported value are 10.4--8])
        ;;
    esac

    AC_MSG_CHECKING([that macosx-version-min-required is coherent with macosx-version-max-allowed])
    if test $MAC_OS_X_VERSION_MIN_REQUIRED -gt $MAC_OS_X_VERSION_MAX_ALLOWED; then
        AC_MSG_ERROR([the version minimumn required must be inferior or equal to the version maximum allowed])
    else
        AC_MSG_RESULT([ok])
    fi

    AC_MSG_CHECKING([that macosx-version-max-allowed is coherent with macos-with-sdk])
    if test $MAC_OS_X_VERSION_MAX_ALLOWED -gt $MACOSX_SDK_VERSION; then
        AC_MSG_ERROR([the version maximum allowed cannot be greater thatn the sdk level])
    else
        AC_MSG_RESULT([ok])
    fi
    AC_MSG_NOTICE([MAC_OS_X_VERSION_MIN_REQUIRED=$MAC_OS_X_VERSION_MIN_REQUIRED])
    AC_MSG_NOTICE([MAC_OS_X_VERSION_MAX_ALLOWED=$MAC_OS_X_VERSION_MAX_ALLOWED])
fi
AC_SUBST(FRAMEWORKSHOME)
AC_SUBST(MACOSX_SDK_PATH)
AC_SUBST(MACOSX_SDK_VERSION)
AC_SUBST(MACOSX_DEPLOYMENT_TARGET)
AC_SUBST(MAC_OS_X_VERSION_MIN_REQUIRED)
AC_SUBST(MAC_OS_X_VERSION_MAX_ALLOWED)

dnl ===================================================================
dnl Windows specific tests and stuff
dnl ===================================================================
if test "$_os" = "WINNT" -a "$WITH_MINGW" != yes; then
    AC_MSG_CHECKING([whether to build a 64-bit LibreOffice])
    if test "$enable_64_bit" = "" -o "$enable_64_bit" = "no"; then
        AC_MSG_RESULT([no])
        SDK_ARCH="x86"
    else
        AC_MSG_RESULT([yes])
        SDK_ARCH="x64"
        BITNESS_OVERRIDE=64
    fi

    AC_MSG_CHECKING([whether to use DirectX])
    if test "$enable_directx" = "yes" -o "$enable_directx" = ""; then
        ENABLE_DIRECTX="TRUE"
        AC_MSG_RESULT([yes])
    else
        ENABLE_DIRECTX=""
        AC_MSG_RESULT([no])
    fi

    AC_MSG_CHECKING([whether to use ActiveX])
    if test "$enable_activex" = "yes" -o "$enable_activex" = "TRUE" -o "$enable_activex" = ""; then
        DISABLE_ACTIVEX=""
        SCPDEFS="$SCPDEFS -DWITH_ACTIVEX_COMPONENT"
        AC_MSG_RESULT([yes])
    else
        DISABLE_ACTIVEX="TRUE"
        AC_MSG_RESULT([no])
    fi

    AC_MSG_CHECKING([whether to use ATL])
    if test "$enable_atl" = "yes" -o "$enable_atl" = "TRUE" -o "$enable_atl" = ""; then
        DISABLE_ATL=""
        AC_MSG_RESULT([yes])
    else
        DISABLE_ATL="TRUE"
        AC_MSG_RESULT([no])
    fi
else
    ENABLE_DIRECTX=""
    DISABLE_ACTIVEX="TRUE"
    DISABLE_ATL="TRUE"
fi

AC_SUBST(ENABLE_DIRECTX)
AC_SUBST(DISABLE_ACTIVEX)
AC_SUBST(DISABLE_ATL)

if test "$cross_compiling" = "yes"; then
    CROSS_COMPILING=YES
    SCPDEFS="$SCPDEFS -DCROSS_COMPILING"
else
    CROSS_COMPILING=
    BUILD_TYPE="$BUILD_TYPE NATIVE"
fi
AC_SUBST(CROSS_COMPILING)

dnl ===================================================================
dnl  Test the gcc version
dnl ===================================================================
if test "$GCC" = "yes"; then
    AC_MSG_CHECKING([the GCC version])
    _gcc_version=`$CC -dumpversion`
    _gcc_major=`echo $_gcc_version | $AWK -F. '{ print \$1 }'`
    GCCVER=`echo $_gcc_version | $AWK -F. '{ print \$1*10000+\$2*100+\$3 }'`

    if test "$_os" = "Darwin" -a "$with_macosx_sdk" = "10.4" -a "$GCCVER" -ge "040100"; then
        if test -z "$save_CC" -a -x "$GCC_HOME/bin/gcc-4.0"; then
            export CC="$GCC_HOME/bin/gcc-4.0"
            #  export CC to have it available in set_soenv -> config.guess
            GCCVER2=`$CC -dumpversion | $AWK -F. '{ print \$1*10000+\$2*100+\$3 }'`
            if test "$GCCVER2" -ge "040000" -a "$GCCVER2" -lt "040100"; then
                GCCVER=$GCCVER2
            fi
        fi
        if test "$GCCVER" -ge "040100"; then
            AC_MSG_ERROR([You need to use the gcc-4.0 compiler (gcc $_gcc_version won't work with the MacOSX10.4u.sdk) - set CC accordingly])
        else
            AC_MSG_RESULT([implicitly using CC=$CC])
        fi
    else
        AC_MSG_RESULT([gcc $_gcc_version])
    fi
    if test "$GCCVER" -lt 040000; then
        AC_MSG_ERROR([GCC $_gcc_version is too old, must be at least GCC 4.0.0])
    fi
fi

dnl ===================================================================
dnl  Is GCC actually Clang?
dnl ===================================================================

COM_GCC_IS_CLANG=
if test "$GCC" = "yes"; then
    AC_MSG_CHECKING([whether GCC is actually Clang])
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
        #ifndef __clang__
        you lose
        #endif
        int foo=42;
        ]])],
        [AC_MSG_RESULT([yes])
         COM_GCC_IS_CLANG=TRUE],
        [AC_MSG_RESULT([no])])

    if test "$COM_GCC_IS_CLANG" = TRUE; then
        AC_MSG_CHECKING([the Clang version])
        clang_version=`echo __clang_major__.__clang_minor__.__clang_patchlevel__ | $CC -E -P -`
        CLANG_FULL_VERSION=`echo __clang_version__ | $CC -E -P -`
        CLANGVER=`echo $clang_version | $AWK -F. '{ print \$1*10000+\$2*100+\$3 }'`
        AC_MSG_RESULT([Clang $CLANG_FULL_VERSION, $CLANGVER])
    fi
fi
AC_SUBST(COM_GCC_IS_CLANG)

# ===================================================================
# check various GCC options that Clang does not support now but maybe
# will somewhen in the future, check them even for GCC, so that the
# flags are set
# ===================================================================

HAVE_GCC_GGDB2=
HAVE_GCC_FINLINE_LIMIT=
HAVE_GCC_FNO_INLINE=
if test "$GCC" = "yes"; then
    AC_MSG_CHECKING([whether $CC supports -ggdb2])
    if test -n "$CLANGVER" -a 0"$CLANGVER" -le 30100; then
        # Option just ignored and silly warning that isn't a real
        # warning printed
        :
    else
        save_CFLAGS=$CFLAGS
        CFLAGS="$CFLAGS -Werror -ggdb2"
        AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[ return 0; ]])],[ HAVE_GCC_GGDB2=TRUE ],[])
        CFLAGS=$save_CFLAGS
    fi
    if test "$HAVE_GCC_GGDB2" = "TRUE"; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
    fi

    AC_MSG_CHECKING([whether $CC supports -finline-limit=0])
    if test -n "$CLANGVER" -a 0"$CLANGVER" -le 30100; then
        # As above
        :
    else
        save_CFLAGS=$CFLAGS
        CFLAGS="$CFLAGS -Werror -finline-limit=0"
        AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[ return 0; ]])],[ HAVE_GCC_FINLINE_LIMIT=TRUE ],[])
        CFLAGS=$save_CFLAGS
    fi
    if test "$HAVE_GCC_FINLINE_LIMIT" = "TRUE"; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
    fi

    AC_MSG_CHECKING([whether $CC supports -fno-inline])
    if test -n "$CLANGVER" -a 0"$CLANGVER" -le 30100; then
        # Ditto
        :
    else
        save_CFLAGS=$CFLAGS
        CFLAGS="$CFLAGS -Werror -fno-inline"
        AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[ return 0; ]])],[ HAVE_GCC_FNO_INLINE=TRUE ],[])
        CFLAGS=$save_CFLAGS
    fi
    if test "$HAVE_GCC_FNO_INLINE" = "TRUE"; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_SUBST(HAVE_GCC_GGDB2)
AC_SUBST(HAVE_GCC_FINLINE_LIMIT)
AC_SUBST(HAVE_GCC_FNO_INLINE)

HAVE_LD_BSYMBOLIC_FUNCTIONS=
if test "$GCC" = "yes"; then
    AC_MSG_CHECKING( for -Bsymbolic-functions linker support )
    bsymbolic_functions_ldflags_save=$LDFLAGS
    LDFLAGS="$LDFLAGS -Wl,-Bsymbolic-functions -Wl,--dynamic-list-cpp-new -Wl,--dynamic-list-cpp-typeinfo"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([
#include <stdio.h>
        ],[
printf ("hello world\n");
        ])], HAVE_LD_BSYMBOLIC_FUNCTIONS=TRUE, [])
    if test "$HAVE_LD_BSYMBOLIC_FUNCTIONS" = "TRUE"; then
        AC_MSG_RESULT( found )
    else
        AC_MSG_RESULT( not found )
    fi
    LDFLAGS=$bsymbolic_functions_ldflags_save
fi
AC_SUBST(HAVE_LD_BSYMBOLIC_FUNCTIONS)

dnl ===================================================================
dnl  Check which Microsoft C/C++ or MinGW compiler is used for WINNT
dnl ===================================================================
SHOWINCLUDES_PREFIX=
if test "$_os" = "WINNT"; then
    if test "$WITH_MINGW" != "yes"; then
        AC_MSG_CHECKING([for a friendly Microsoft C/C++ compiler installation path])
        if test -z "$with_cl_home"; then
            vctest=`./oowintool --msvc-productdir`
            if test "$BITNESS_OVERRIDE" = ""; then
                if test -x "$vctest/bin/cl.exe"; then
                    with_cl_home=$vctest
                fi
            else
                if test -x "$vctest/bin/amd64/cl.exe"; then
                    with_cl_home=$vctest
                fi
            fi
        else
            with_cl_home=`cygpath -u "$with_cl_home"`
        fi
        with_cl_home=`cygpath -d "$with_cl_home"`
        with_cl_home=`cygpath -u "$with_cl_home"`
        AC_MSG_RESULT([$with_cl_home])

        dnl ===========================================================
        dnl  Check for mspdb80.dll/mspdb100.dll/mspdb110.dll
        dnl ===========================================================
        dnl MSVS 2008/10/12 Compiler
        if test -n "$with_mspdb_path";then
            with_mspdb_path=`cygpath -u "$with_mspdb_path"`
        fi
        if test -e "$with_mspdb_path/mspdb80.dll" -o -e "$with_mspdb_path/mspdb100.dll" -o -e "$with_mspdb_path/mspdb110.dll"; then
            MSPDB_PATH="$with_mspdb_path"
        fi
        dnl MSVS 2008 case
        if test -z "$MSPDB_PATH" -a -e "$with_cl_home/../Common7/IDE/mspdb80.dll"; then
            MSPDB_PATH="$with_cl_home/../Common7/IDE"
        fi
        dnl Windows SDK 6.0 case
        if test -z "$MSPDB_PATH" -a -e "$with_cl_home/bin/mspdb80.dll"; then
            MSPDB_PATH="$with_cl_home/bin"
        fi
        dnl MSVS 2010 case
        if test -z "$MSPDB_PATH" -a -e "$with_cl_home/../Common7/IDE/mspdb100.dll"; then
            MSPDB_PATH="$with_cl_home/../Common7/IDE"
        fi
        dnl MSVS 2012 case
        if test -z "$MSPDB_PATH" -a -e "$with_cl_home/../Common7/IDE/mspdb110.dll"; then
            MSPDB_PATH="$with_cl_home/../Common7/IDE"
        fi

        if test -z "$MSPDB_PATH";then
            dnl AC_PATH_PROG only checks if MSPDB_PATH is still empty
            AC_PATH_PROG(MSPDB_PATH, mspdb80.dll)
            AC_PATH_PROG(MSPDB_PATH, mspdb100.dll)
            AC_PATH_PROG(MSPDB_PATH, mspdb110.dll)
            MSPDB_PATH=`dirname "$MSPDB_PATH"`
        fi

        if test -z "$MSPDB_PATH"; then
            AC_MSG_ERROR([You need a mspdb80.dll or mspdb100.dll or mspdb110.dll, make sure it is in the path or use --with-mspdb-path])
        fi
        MSPDB_PATH=`cygpath -d "$MSPDB_PATH"`
        MSPDB_PATH=`cygpath -u "$MSPDB_PATH"`
        dnl The path needs to be added before cl is called
        PATH="$MSPDB_PATH:$PATH"

        AC_MSG_CHECKING([the Microsoft C/C++ Compiler])
        if test -z "$CC"; then
            if test "$BITNESS_OVERRIDE" = ""; then
                if test -x "$with_cl_home/bin/cl.exe"; then
                    CC="$with_cl_home/bin/cl.exe"
                fi
            else
                if test -x "$with_cl_home/bin/amd64/cl.exe"; then
                    CC="$with_cl_home/bin/amd64/cl.exe"
                fi
            fi
            if test -z "$CC"; then
                AC_PATH_PROG(CC, cl.exe)
            fi
            # This gives us a posix path with 8.3 filename restrictions
            CC=`cygpath -d "$CC"`
            CC=`cygpath -u "$CC"`
        fi

        if test -n "$CC"; then
            # Remove /cl.exe from CC case insensitive
            AC_MSG_RESULT([found ($CC)])
            if test "$BITNESS_OVERRIDE" = ""; then
                COMPATH=`echo $CC | $SED -e 's@\/[[Bb]][[Ii]][[Nn]]\/[[cC]][[lL]]\.[[eE]][[xX]][[eE]].*@@' -e 's@^.* @@'`
            else
                if test -n "$with_cl_home"; then
                    COMPATH=`echo $with_cl_home`
                fi
            fi
            export INCLUDE=`cygpath -d "$COMPATH/Include"`
            dnl  Check which Microsoft C/C++ compiler is found
            AC_MSG_CHECKING([the Version of Microsoft C/C++ Compiler])
            # The following finds Microsoft, matches nn.nn.nnnn then pulls numbers out.
            CCNUMVER=`$CC 2>&1 | $AWK "/Microsoft/ && /..\\...\\...../ {
                            x = match( \\\$0, /..\\...\\...../ )
                            CCversion = substr( \\\$0, RSTART, RLENGTH)
                            tokencount = split (CCversion,vertoken,\".\")
                            for ( i = 1 ; i <= tokencount ; i++ ) {
                                printf (\"%04d\",vertoken[[i]] )
                            }
                            }"`
            if test "$CCNUMVER" -ge "001700000000"; then
                COMEX=14
                MSVSVER=2012
                VCVER=110
            elif test "$CCNUMVER" -ge "001600000000"; then
                COMEX=13
                MSVSVER=2010
                VCVER=100
            elif test "$CCNUMVER" -ge "001500000000"; then
                COMEX=12
                MSVSVER=2008
                VCVER=90
            else
                AC_MSG_ERROR([Compiler too old. Use Microsoft Visual Studio 2008 or 2010.])
            fi
            PathFormat "$COMPATH"
            COMPATH="$formatted_path"

            AC_MSG_RESULT([found compiler version $CCNUMVER (MSVS $MSVSVER).])
        else
            AC_MSG_ERROR([Microsoft C/C++ Compiler not found. Use --with-cl-home or set path to cl.exe.])
        fi

        dnl We need to guess the prefix of the -showIncludes output, it can be
        dnl localized
        AC_MSG_CHECKING([the dependency generation prefix (cl.exe -showIncludes)])
        echo "#include <stdlib.h>" > conftest.c
        SHOWINCLUDES_PREFIX=`$CC -c -showIncludes conftest.c 2>/dev/null | \
            grep 'stdlib\.h' | head -n1 | sed 's/ [[[:alpha:]]]:.*//'`
        rm -f conftest.c conftest.obj
        if test -z "$SHOWINCLUDES_PREFIX"; then
            AC_MSG_ERROR([cannot determine the -showIncludes prefix])
        else
            AC_MSG_RESULT(["$SHOWINCLUDES_PREFIX"])
        fi

        # Check for 64-bit (cross-)compiler to use to build the 64-bit
        # version of the Explorer extension (and maybe other small
        # bits, too) needed when installing a 32-bit LibreOffice on a
        # 64-bit OS. The 64-bit Explorer extension is a feature that
        # has been present since long in OOo. Don't confuse it with
        # building LibreOffice itself as 64-bit code, which is
        # unfished work and highly experimental.

        BUILD_X64=
        CXX_X64_BINARY=
        LINK_X64_BINARY=
        LIBMGR_X64_BINARY=

        AC_MSG_CHECKING([for a x64 compiler and libraries for 64-bit Explorer extensions])
        if test "$BITNESS_OVERRIDE" = "" -a -f "$with_cl_home/atlmfc/lib/amd64/atls.lib"; then
            # Prefer native x64 compiler to cross-compiler, in case we are running
            # the build on a 64-bit OS.
            if "$with_cl_home/bin/amd64/cl.exe" -? </dev/null >/dev/null 2>&1; then
                BUILD_X64=TRUE
                CXX_X64_BINARY="$with_cl_home/bin/amd64/cl.exe"
                LINK_X64_BINARY="$with_cl_home/bin/amd64/link.exe"
                LIBMGR_X64_BINARY="$with_cl_home/bin/amd64/lib.exe"
            elif "$with_cl_home/bin/x86_amd64/cl.exe" -? </dev/null >/dev/null 2>&1; then
                BUILD_X64=TRUE
                CXX_X64_BINARY="$with_cl_home/bin/x86_amd64/cl.exe"
                LINK_X64_BINARY="$with_cl_home/bin/x86_amd64/link.exe"
                LIBMGR_X64_BINARY="$with_cl_home/bin/x86_amd64/lib.exe"
            fi
        fi
        if test "$BUILD_X64" = TRUE; then
            AC_MSG_RESULT([found])
        else
            AC_MSG_RESULT([not found])
            AC_MSG_WARN([Installation set will not contain 64-bit Explorer extensions])
        fi
        AC_SUBST(BUILD_X64)

        # These are passed to the environment through set_soenv.in as usual, and then
        # used in set_wntx64.mk
        AC_SUBST(CXX_X64_BINARY)
        AC_SUBST(LINK_X64_BINARY)
        AC_SUBST(LIBMGR_X64_BINARY)

    else
        AC_MSG_CHECKING([the compiler is MinGW])
        MACHINE_PREFIX=`$CC -dumpmachine`
        if echo $MACHINE_PREFIX | $GREP -q mingw32; then
            COMPATH=`echo "$COMPATH" | sed -e 's,/bin$,,'`
            AC_MSG_RESULT([yes])
        else
            AC_MSG_ERROR([Compiler is not MinGW.])
        fi
    fi
fi
AC_SUBST(COMEX)
AC_SUBST(VCVER)
PathFormat "$MSPDB_PATH"
MSPDB_PATH="$formatted_path"
AC_SUBST(SHOWINCLUDES_PREFIX)

#
# dbghelp.dll
#
if test "$_os" == "WINNT"; then
    DBGHELP_DLL="13fbc2e8b37ddf28181dd6d8081c2b8e-dbghelp.dll"
fi
AC_SUBST(DBGHELP_DLL)

#
# unowinreg.dll
#
UNOWINREG_DLL="185d60944ea767075d27247c3162b3bc-unowinreg.dll"
AC_SUBST(UNOWINREG_DLL)

#
# prefix C with ccache if needed
#
if test "$CCACHE" != ""; then
    AC_MSG_CHECKING([whether $CC is already ccached])

    AC_LANG_PUSH([C])
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS --ccache-skip -O2"
    dnl an empty program will do, we're checking the compiler flags
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],[])],
                      [use_ccache=yes], [use_ccache=no])
    if test $use_ccache = yes; then
        AC_MSG_RESULT([yes])
    else
        CC="$CCACHE $CC"
        AC_MSG_RESULT([no])
    fi
    CFLAGS=$save_CFLAGS
    AC_LANG_POP([C])
fi

dnl Set the ENABLE_DBGUTIL variable
dnl ===================================================================
AC_MSG_CHECKING([whether to build with additional debug utilities])
if test -n "$enable_dbgutil" -a "$enable_dbgutil" != "no"; then
    ENABLE_DBGUTIL="TRUE"
    PROEXT=""
    PRODUCT=""

    AC_MSG_RESULT([yes])
    # cppunit and graphite expose STL in public headers
    if test "$with_system_cppunit" = "yes"; then
        AC_MSG_ERROR([--with-system-cppunit conflicts with --enable-dbgutil])
    else
        with_system_cppunit=no
    fi
    if test "$with_system_graphite" = "yes"; then
        AC_MSG_ERROR([--with-system-graphite conflicts with --enable-dbgutil])
    else
        with_system_graphite=no
    fi
    if test "$with_system_mysql_cppconn" = "yes"; then
        AC_MSG_ERROR([--with-system-mysql-cppconn conflicts with --enable-dbgutil])
    else
        with_system_mysql_cppconn=no
    fi
else
    ENABLE_DBGUTIL=""
    # PRODUCT is old concept, still used by build.pl .
    PRODUCT="full"
    PROEXT=".pro"
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_DBGUTIL)
AC_SUBST(PRODUCT)
AC_SUBST(PROEXT)

dnl Set the ENABLE_DEBUG variable.
dnl ===================================================================
AC_MSG_CHECKING([whether to do a debug build])
if test -n "$enable_debug" && test "$enable_debug" != "yes" && test "$enable_debug" != "no"; then
    AC_MSG_ERROR([--enable-debug now accepts only yes or no, use --enable-selective-debuginfo])
fi
if test -n "$ENABLE_DBGUTIL"; then
    if test "$enable_debug" = "no"; then
        AC_MSG_ERROR([--disable-debug cannot be used with --enable-dbgutil])
    fi
    ENABLE_DEBUG="TRUE"
    AC_MSG_RESULT([yes (dbgutil)])
elif test -n "$enable_debug" && test "$enable_debug" != "no"; then
    ENABLE_DEBUG="TRUE"
    AC_MSG_RESULT([yes])
else
    ENABLE_DEBUG=""
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_DEBUG)

dnl Selective debuginfo
ENABLE_DEBUGINFO_FOR=
if test -n "$ENABLE_DEBUG"; then
    AC_MSG_CHECKING([whether to use selective debuginfo])
    if test -n "$enable_selective_debuginfo" -a "$enable_selective_debuginfo" != "no"; then
        ENABLE_DEBUGINFO_FOR="$enable_selective_debuginfo"
        AC_MSG_RESULT([for "$enable_selective_debuginfo"])
    else
        ENABLE_DEBUGINFO_FOR=all
        AC_MSG_RESULT([no, for all])
    fi
fi
AC_SUBST(ENABLE_DEBUGINFO_FOR)

dnl Check for explicit C/CXX/OBJC/OBJCXX/LDFLAGS. We by default use the ones specified
dnl by our build system, but explicit override is possible.
AC_MSG_CHECKING(for explicit CFLAGS)
if test -n "$CFLAGS"; then
    AC_MSG_RESULT([$CFLAGS])
else
    AC_MSG_RESULT(no)
fi
AC_MSG_CHECKING(for explicit CXXFLAGS)
if test -n "$CXXFLAGS"; then
    AC_MSG_RESULT([$CXXFLAGS])
else
    AC_MSG_RESULT(no)
fi
AC_MSG_CHECKING(for explicit OBJCFLAGS)
if test -n "$OBJCFLAGS"; then
    AC_MSG_RESULT([$OBJCFLAGS])
else
    AC_MSG_RESULT(no)
fi
AC_MSG_CHECKING(for explicit OBJCXXFLAGS)
if test -n "$OBJCXXFLAGS"; then
    AC_MSG_RESULT([$OBJCXXFLAGS])
else
    AC_MSG_RESULT(no)
fi
AC_MSG_CHECKING(for explicit LDFLAGS)
if test -n "$LDFLAGS"; then
    AC_MSG_RESULT([$LDFLAGS])
else
    AC_MSG_RESULT(no)
fi
AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(OBJCFLAGS)
AC_SUBST(OBJCXXFLAGS)
AC_SUBST(LDFLAGS)

#
# determine CPU, CPUNAME, GUI, GUIBASE, ...
#
LIB64="lib"
SOLARINC=

case "$host_os" in

aix*)
    COM=GCC
    CPU=P
    CPUNAME=POWERPC
    GUI=UNX
    GUIBASE=unx
    OS=AIX
    RTL_OS=AIX
    RTL_ARCH=PowerPC
    PLATFORMID=aix_powerpc
    OUTPATH=unxaigppc
    P_SEP=:
    SOLARLIB="-L../lib -L$SRC_ROOT/solenv/$OUTPATH/lib"
    SOLARLIB="$SOLARLIB -L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    SOLARLIB="$SOLARLIB -L$SRC_ROOT/solenv/$OUTPATH/lib"
    ;;

cygwin*)
    COM=MSC
    GUI=WNT
    GUIBASE=WIN
    OS=WNT
    RTL_OS=Windows
    P_SEP=";"

    case "$host_cpu" in
    i*86|x86_64)
        if test "$BITNESS_OVERRIDE" = 64; then
            CPU=X
            CPUNAME=X86_64
            RTL_ARCH=X86_64
            LIB64="lib/x64"
            PLATFORMID=windows_x86_64
            OUTPATH=wntmscx$COMEX
        else
            CPU=I
            CPUNAME=INTEL
            RTL_ARCH=x86
            PLATFORMID=windows_x86
            OUTPATH=wntmsci$COMEX
        fi
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    SCPDEFS="$SCPDEFS -D_MSC_VER"
    SOLARLIB="-L../lib -L$SRC_ROOT/solenv/$OUTPATH/lib"
    SOLARLIB="$SOLARLIB -L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    ;;

darwin*)
    COM=GCC
    GUI=UNX
    # Not sure if "Aqua" is the right term to use?
    GUIBASE="aqua"
    OS=MACOSX
    RTL_OS=MacOSX
    P_SEP=:

    case "$host_cpu" in
    arm*)
        CPU=R
        CPUNAME=ARM
        RTL_ARCH=ARM_EABI
        GUIBASE=cocoatouch
        PLATFORMID=macosx_arm_eabi
        OUTPATH=unxiosr
        OS=IOS
        ;;
    powerpc*)
        CPU=P
        CPUNAME=POWERPC
        RTL_ARCH=PowerPC
        PLATFORMID=macosx_powerpc
        OUTPATH=unxmacxp
        ;;
    i*86)
        if test "$BITNESS_OVERRIDE" = 64; then
            AC_MSG_ERROR([Can't build 64-bit code in 32-bit OS])
        fi
        CPU=I
        CPUNAME=INTEL
        RTL_ARCH=x86
        PLATFORMID=macosx_x86
        OUTPATH=unxmacxi
        ;;
    x86_64)
        if test "$BITNESS_OVERRIDE" = 64; then
            CPU=X
            CPUNAME=X86_64
            RTL_ARCH=X86_64
            PLATFORMID=macosx_x86_64
            OUTPATH=unxmacxx
        else
            CPU=I
            CPUNAME=INTEL
            RTL_ARCH=x86
            PLATFORMID=macosx_x86
            OUTPATH=unxmacxi
        fi
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    SOLARLIB="-L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    ;;

dragonfly*)
    COM=GCC
    GUI=UNX
    GUIBASE=unx
    OS=DRAGONFLY
    RTL_OS=DragonFly
    OUTPATH=unxdfly
    P_SEP=:

    case "$host_cpu" in
    i*86)
        CPU=I
        CPUNAME=INTEL
        RTL_ARCH=x86
        PLATFORMID=dragonfly_x86
        ;;
    x86_64)
        CPU=X
        CPUNAME=X86_64
        RTL_ARCH=X86_64
        PLATFORMID=dragonfly_x86_64
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    SOLARLIB="-L../lib -L$SRC_ROOT/solenv/$OUTPATH/lib"
    SOLARLIB="$SOLARLIB -L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    SOLARLIB="$SOLARLIB -L$SRC_ROOT/solenv/$OUTPATH/lib"
    ;;

freebsd*)
    COM=GCC
    GUI=UNX
    GUIBASE=unx
    RTL_OS=FreeBSD
    OS=FREEBSD
    OUTPATH=unxfbsd
    P_SEP=:

    case "$host_cpu" in
    i*86)
        CPU=I
        CPUNAME=INTEL
        RTL_ARCH=x86
        PLATFORMID=freebsd_x86
        OUTPATH=unxfbsdi
        ;;
    x86_64)
        CPU=X
        CPUNAME=X86_64
        RTL_ARCH=X86_64
        PLATFORMID=freebsd_x86_64
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    SOLARLIB="-L../lib -L$SRC_ROOT/solenv/$OUTPATH/lib"
    SOLARLIB="$SOLARLIB -L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    SOLARLIB="$SOLARLIB -L$SRC_ROOT/solenv/$OUTPATH/lib"
    ;;

kfreebsd*)
    COM=GCC
    GUI=UNX
    GUIBASE=unx
    OS=LINUX
    RTL_OS=kFreeBSD
    P_SEP=:

    case "$host_cpu" in

    i*86)
        CPU=I
        CPUNAME=INTEL
        RTL_ARCH=x86
        PLATFORMID=kfreebsd_x86
        OUTPATH=unxkfgi6
        ;;
    x86_64)
        CPU=X
        CPUNAME=X86_64
        RTL_ARCH=X86_64
        LIB64="lib64"
        PLATFORMID=kfreebsd_x86_64
        OUTPATH=unxkfgx6
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    SOLARLIB="-L../lib -L$SRC_ROOT/solenv/$OUTPATH/lib"
    SOLARLIB="$SOLARLIB -L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    SOLARLIB="$SOLARLIB -L$SRC_ROOT/solenv/$OUTPATH/lib"
    ;;

linux-gnu*)
    COM=GCC
    GUI=UNX
    GUIBASE=unx
    OS=LINUX
    RTL_OS=Linux
    P_SEP=:

    case "$host_cpu" in

    alpha)
        CPU=L
        CPUNAME=AXP
        RTL_ARCH=ALPHA
        PLATFORMID=linux_alpha
        OUTPATH=unxlngaxp
        ;;
    arm*)
        CPU=R
        CPUNAME=ARM
        EPM_FLAGS="-a arm"
        OUTPATH=unxlngr
        RTL_ARCH=ARM_EABI
        PLATFORMID=linux_arm_eabi
        case "$host_cpu" in
        arm*-linux)
            RTL_ARCH=ARM_OABI
            PLATFORMID=linux_arm_oabi
            ;;
        esac
        ;;
    hppa)
        CPU=H
        CPUNAME=HPPA
        RTL_ARCH=HPPA
        EPM_FLAGS="-a hppa"
        PLATFORMID=linux_hppa
        OUTPATH=unxlnghppa
        ;;
    i*86)
        CPU=I
        CPUNAME=INTEL
        RTL_ARCH=x86
        PLATFORMID=linux_x86
        OUTPATH=unxlngi6
        ;;
    ia64)
        CPU=A
        CPUNAME=IA64
        RTL_ARCH=IA64
        PLATFORMID=linux_ia64
        OUTPATH=unxlnga
        ;;
    mips)
        CPU=M
        CPUNAME=GODSON
        RTL_ARCH=MIPS_EB
        EPM_FLAGS="-a mips"
        PLATFORMID=linux_mips_eb
        OUTPATH=unxlngmips
        ;;
    mips64)
        CPU=M
        CPUNAME=GODSON
        RTL_ARCH=MIPS_EB
        EPM_FLAGS="-a mips64"
        PLATFORMID=linux_mips_eb
        OUTPATH=unxlngmips
        ;;
    mips64el)
        CPU=M
        CPUNAME=GODSON
        RTL_ARCH=MIPS_EL
        EPM_FLAGS="-a mips64el"
        PLATFORMID=linux_mips_el
        OUTPATH=unxlngmips
        ;;
    mipsel)
        CPU=M
        CPUNAME=GODSON
        RTL_ARCH=MIPS_EL
        EPM_FLAGS="-a mipsel"
        PLATFORMID=linux_mips_el
        OUTPATH=unxlngmips
        ;;
    m68k)
        CPU=6
        CPUNAME=M68K
        RTL_ARCH=M68K
        PLATFORMID=linux_m68k
        OUTPATH=unxlngm68k
        ;;
    powerpc)
        CPU=P
        CPUNAME=POWERPC
        RTL_ARCH=PowerPC
        PLATFORMID=linux_powerpc
        OUTPATH=unxlngppc
        ;;
    powerpc64)
        CPU=P
        CPUNAME=POWERPC64
        RTL_ARCH=PowerPC_64
        LIB64="lib64"
        PLATFORMID=linux_powerpc_64
        OUTPATH=unxlngppc64
        ;;
    sparc)
        CPU=S
        CPUNAME=SPARC
        RTL_ARCH=SPARC
        PLATFORMID=linux_sparc
        OUTPATH=unxlngs
        ;;
    s390)
        CPU=3
        CPUNAME=S390
        RTL_ARCH=S390
        PLATFORMID=linux_s390
        OUTPATH=unxlngs390
        ;;
    s390x)
        CPU=3
        CPUNAME=S390X
        RTL_ARCH=S390x
        LIB64="lib64"
        PLATFORMID=linux_s390x
        OUTPATH=unxlngs390x
        ;;
    x86_64)
        CPU=X
        CPUNAME=X86_64
        RTL_ARCH=X86_64
        LIB64="lib64"
        PLATFORMID=linux_x86_64
        OUTPATH=unxlngx6
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    SOLARLIB="-L../lib -L$SRC_ROOT/solenv/$OUTPATH/lib"
    SOLARLIB="$SOLARLIB -L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    SOLARLIB="$SOLARLIB -L$SRC_ROOT/solenv/$OUTPATH/lib"
    ;;

linux-android*)
    COM=GCC
    GUI=UNX
    GUIBASE=android
    OS=ANDROID
    RTL_OS=Android
    P_SEP=:

    case "$host_cpu" in

    arm|armel)
        CPU=R
        CPUNAME=ARM
        RTL_ARCH=ARM_EABI
        PLATFORMID=android_arm_eabi
        OUTPATH=unxandr
        ;;
    mips|mipsel)
        CPU=M
        CPUNAME=GODSON # Weird, but maybe that's the LO convention?
        RTL_ARCH=MIPS_EL
        PLATFORMID=android_mips_el
        OUTPATH=unxandm
        ;;
    i*86)
        CPU=I
        CPUNAME=INTEL
        RTL_ARCH=x86
        PLATFORMID=android_x86
        OUTPATH=unxandi
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    SOLARINC="$SOLARINC -I$SRC_ROOT/solenv/inc/$OUTPATH"
    SOLARLIB="-L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    ;;

mingw*)
    COM=GCC
    GUI=WNT
    GUIBASE=WIN
    OS=WNT
    RTL_OS=Windows
    P_SEP=:

    case "$host_cpu" in
    i*86|x86_64)
        if test "$BITNESS_OVERRIDE" = 64; then
            CPU=X
            CPUNAME=X86_64
            RTL_ARCH=X86_84
            PLATFORMID=windows_x86_64
            OUTPATH=wntgccx$COMEX
        else
            CPU=I
            CPUNAME=INTEL
            RTL_ARCH=x86
            PLATFORMID=windows_x86
            OUTPATH=wntgcci$COMEX
        fi
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    SOLARINC="$SOLARINC -I$SRC_ROOT/solenv/inc/win32 -I$SRC_ROOT/solenv/inc/$OUTPATH"
    SOLARLIB="-L../lib -L$SRC_ROOT/solenv/$OUTPATH/lib"
    SOLARLIB="$SOLARLIB -L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    ;;

*netbsd*)

    COM=GCC
    GUI=UNX
    GUIBASE=unx
    OS=NETBSD
    RTL_OS=NetBSD
    OUTPATH=unxnbsd
    P_SEP=:

    case "$host_cpu" in
    i*86)
        CPU=I
        CPUNAME=INTEL
        RTL_ARCH=x86
        PLATFORMID=netbsd_x86
        ;;
    powerpc)
        CPU=P
        CPUNAME=POWERPC
        RTL_ARCH=PowerPC
        PLATFORMID=netbsd_powerpc
        ;;
    sparc)
        CPU=S
        CPUNAME=SPARC
        RTL_ARCH=SPARC
        PLATFORMID=netbsd_sparc
        ;;
    x86_64)
        CPU=X
        CPUNAME=X86_64
        RTL_ARCH=X86_64
        PLATFORMID=netbsd_x86_64
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    SOLARLIB="-L../lib -L$SRC_ROOT/solenv/$OUTPATH/lib"
    SOLARLIB="$SOLARLIB -L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    SOLARLIB="$SOLARLIB -L$SRC_ROOT/solenv/$OUTPATH/lib"
    ;;

openbsd*)
    COM=GCC
    GUI=UNX
    GUIBASE=unx
    OS=OPENBSD
    RTL_OS=OpenBSD
    OUTPATH=unxobsd
    P_SEP=:

    case "$host_cpu" in
    i*86)
        CPU=I
        CPUNAME=INTEL
        RTL_ARCH=x86
        PLATFORMID=openbsd_x86
        ;;
    x86_64)
        CPU=X
        CPUNAME=X86_64
        RTL_ARCH=X86_64
        PLATFORMID=openbsd_x86_64
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    SOLARINC="$SOLARINC -I/usr/local/include"
    SOLARLIB="-L../lib -L$SRC_ROOT/solenv/$OUTPATH/lib"
    SOLARLIB="$SOLARLIB -L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    SOLARLIB="$SOLARLIB -L$SRC_ROOT/solenv/$OUTPATH/lib"
    SOLARLIB="$SOLARLIB -L/usr/local/lib"

    ;;

solaris*)

    COM=GCC
    GUI=UNX
    GUIBASE=unx
    OS=SOLARIS
    RTL_OS=Solaris
    P_SEP=:

    case "$host_cpu" in
    i*86)
        CPU=I
        CPUNAME=INTEL
        RTL_ARCH=x86
        PLATFORMID=solaris_x86
        OUTPATH=unxsogi
        ;;
    sparc)
        CPU=S
        CPUNAME=SPARC
        RTL_ARCH=SPARC
        PLATFORMID=solaris_sparc
        OUTPATH=unxsogs
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    SOLARINC="$SOLARINC -I/usr/local/include"
    SOLARLIB="-L../lib -L$SRC_ROOT/solenv/$OUTPATH/lib"
    SOLARLIB="$SOLARLIB -L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    SOLARLIB="$SOLARLIB -L$SRC_ROOT/solenv/$OUTPATH/lib"
    SOLARLIB="$SOLARLIB -L$COMPATH/lib"
    SOLARLIB="$SOLARLIB -L/usr/local/bin -L/usr/dt/lib -L/usr/openwin/lib"
    ;;

*)
    AC_MSG_ERROR([$host_os operating system is not suitable to build LibreOffice for!])
    ;;
esac

if test "$enable_headless" = "yes"; then
    GUIBASE=headless
fi

INPATH="${OUTPATH}${PROEXT}"
if test -n "${with_solver_and_workdir_root}"; then
    PathFormat "${with_solver_and_workdir_root}"
    # IsValidFilePath in /sal/osl/w32/file_url.cxx rejects "X:\\" breaking idlc
    if echo ${formatted_path} | $GREP -q '/$'; then
        WORKDIR=${formatted_path}workdir/${INPATH}
    else
        WORKDIR=${formatted_path}/workdir/${INPATH}
    fi
else
    WORKDIR=${SRC_ROOT}/workdir/${INPATH}
fi
OUTDIR="${SOLARVER}/${INPATH}"
SOLARINC="-I. -I${SOLARVER}/$INPATH/inc/external -I${SOLARVER}/$INPATH/inc -I$SRC_ROOT/solenv/inc $SOLARINC"
AC_SUBST(COM)
AC_SUBST(CPU)
AC_SUBST(CPUNAME)
AC_SUBST(RTL_OS)
AC_SUBST(RTL_ARCH)
AC_SUBST(EPM_FLAGS)
AC_SUBST(GUI)
AC_SUBST(GUIBASE)
AC_SUBST(INPATH)
AC_SUBST(OS)
AC_SUBST(OUTDIR)
AC_SUBST(OUTPATH)
AC_SUBST(P_SEP)
AC_SUBST(SOLARVER)
AC_SUBST(WORKDIR)
AC_SUBST(PLATFORMID)

dnl ===================================================================
dnl Test which package format to use
dnl ===================================================================
AC_MSG_CHECKING([which package format to use])
if test -n "$with_package_format"; then
    for i in $with_package_format; do
        case "$i" in
        aix | bsd | deb | inst | tardist | osx | pkg | rpm | setld | native | portable | archive | dmg | installed | msi)
            ;;
        *)
            AC_MSG_ERROR([unsupported format $i. Supported by EPM are:
aix - AIX software distribution
bsd - FreeBSD, NetBSD, or OpenBSD software distribution
deb - Debian software distribution
inst or tardist - IRIX software distribution
osx - MacOS X software distribution
pkg - Solaris software distribution
rpm - RedHat software distribution
setld - Tru64 (setld) software distribution
native - "Native" software distribution for the platform
portable - Portable software distribution

LibreOffice additionally supports:
archive - .tar.gz or .zip
dmg - Mac OS X .dmg
installed - installation tree
msi - Windows .msi
        ])
            ;;
        esac
    done
    PKGFORMAT="$with_package_format"
elif test "$enable_epm" = "yes"; then
    # defaults
    case "$_os" in
    Darwin)
        PKGFORMAT=dmg
        ;;
    SunOS)
        PKGFORMAT=pkg
        ;;
    Linux)
        # if building on Debian, default should be deb...
        if test -e /etc/debian_version; then
            PKGFORMAT=deb
        else
            PKGFORMAT=rpm
        fi
        ;;
    AIX)
        PKGFORMAT=rpm
        ;;
    OpenBSD|DragonFly)
        PKGFORMAT=portable
        ;;
    *BSD)
        PKGFORMAT=bsd
        ;;
    WINNT)
        PKGFORMAT=msi
        ;;
    # we never should get here since we check the arciecture/os at the beginning,
    # but go sure...
    *)
        AC_MSG_ERROR([unknown system])
    esac
else
    if test "$WITH_MINGW" = "yes"; then
        # when tested, we should default this to 'msi', instead of 'archive'
        PKGFORMAT=archive
    else
        PKGFORMAT=native
    fi
fi
AC_MSG_RESULT([$PKGFORMAT])
AC_SUBST(PKGFORMAT)

dnl ===================================================================
dnl Set up a different compiler to produce tools to run on the build
dnl machine when doing cross-compilation
dnl ===================================================================

m4_pattern_allow([PKG_CONFIG_FOR_BUILD])
if test "$cross_compiling" = "yes"; then
    AC_MSG_CHECKING([for BUILD platform configuration])
    echo
    rm -rf CONF-FOR-BUILD config_build.mk
    mkdir CONF-FOR-BUILD
    tar cf - \
        bin/repo-list.in \
        config.guess \
        config_host.mk.in \
        config_host.mk.source \
        configure \
        ooo.lst.in \
        oowintool \
        post_download.in \
        solenv/inc/langlist.mk \
        solenv/inc/postset.mk \
    | (cd CONF-FOR-BUILD && tar xf -)
    (
    unset COM GUI GUIBASE OS CPU CPUNAME
    unset CC CXX SYSBASE CFLAGS
    unset PYTHON_CFLAGS PYTHON_LIBS
    unset AR NM OBJDUMP PKG_CONFIG RANLIB STRIP
    unset CPPUNIT_CFLAGS CPPUNIT_LIBS
    unset LIBXML_CFLAGS LIBXML_LIBS LIBXSLT_CFLAGS LIBXSLT_LIBS XSLTPROC
    test -n "$CC_FOR_BUILD" && export CC="$CC_FOR_BUILD"
    test -n "$CXX_FOR_BUILD" && export CXX="$CXX_FOR_BUILD"
    test -n "$PKG_CONFIG_FOR_BUILD" && export PKG_CONFIG="$PKG_CONFIG_FOR_BUILD"
    cd CONF-FOR-BUILD
    sub_conf_opts=""
    test -n "$enable_ccache" && sub_conf_opts="$sub_conf_opts --enable-ccache=$enable_ccache"
    test -n "$with_ant_home" && sub_conf_opts="$sub_conf_opts --with-ant-home=$with_ant_home"
    test $with_java = no && sub_conf_opts="$sub_conf_opts --without-java"
    test $with_junit = no && sub_conf_opts="$sub_conf_opts --without-junit"
    test -n "$TARFILE_LOCATION" && sub_conf_opts="$sub_conf_opts --with-external-tar=$TARFILE_LOCATION"
    test -n "${with_solver_and_workdir_root}" && sub_conf_opts="$sub_conf_opts --with-solver-and-workdir-root=${with_solver_and_workdir_root}"
    test -n "$with_system_boost_for_build" && sub_conf_opts="$sub_conf_opts --with-system-boost"
    test -n "$with_system_cppunit_for_build" && sub_conf_opts="$sub_conf_opts --with-system-cppunit"
    test -n "$with_system_db_for_build" && sub_conf_opts="$sub_conf_opts --with-system-db"
    test -n "$with_system_expat_for_build" && sub_conf_opts="$sub_conf_opts --with-system-expat"
    test "$with_system_icu_for_build" = "yes" -o "$with_system_icu_for_build" = "force" && sub_conf_opts="$sub_conf_opts --with-system-icu"
    test -n "$with_system_libxml_for_build" && sub_conf_opts="$sub_conf_opts --with-system-libxml"
    # we need the msi build tools on mingw if we are creating the
    # installation set
    if test "$WITH_MINGW" = "yes"; then
        enable_winegcc_for_build=
        for pkgformat in $PKGFORMAT; do
            case "$pkgformat" in
                msi|native) enable_winegcc_for_build=yes ;;
            esac
        done
        test -n "$enable_winegcc_for_build" && sub_conf_opts="$sub_conf_opts --enable-winegcc"
    fi
    sub_conf_opts="$sub_conf_opts $with_build_platform_configure_options"
    # Don't bother having configure look for stuff not needed for the build platform anyway
    ./configure \
        --disable-graphite \
        --disable-build-mozilla \
        --disable-mozilla \
        --disable-postgresql-sdbc \
        --disable-zenity \
        --with-num-cpus="$with_num_cpus" \
        --with-max-jobs="$with_max_jobs" \
        --without-doxygen \
        $sub_conf_opts \
        2>&1 | sed -e 's/^/    /'
    test -f ./config_host.mk 2>/dev/null || exit
    cp config_host.mk ../config_build.mk
    cp config.log ../config.Build.log
    . ./config_host.mk.source
    for V in COM GUI GUIBASE OS CPU CPUNAME CC CXX DISABLE_PYTHON GXX_INCLUDE_PATH MACOSX_DEPLOYMENT_TARGET INPATH OUTPATH \
             PYTHON SYSTEM_LIBXSLT; do
        VV='$'$V
        VV=`eval "echo $VV"`
        if test -n "$VV"; then
            line=${V}_FOR_BUILD='${'${V}_FOR_BUILD:-$VV'}'
            echo "$line" >>build-config
        fi
    done

    for V in OUTDIR SOLARINC SOLARLIB WORKDIR; do
        VV='$'$V
        VV=`eval "echo $VV"`
        VV=`echo $VV | sed -e 's,/CONF-FOR-BUILD,,g'`
        if test -n "$VV"; then
            line="${V}_FOR_BUILD='$VV'"
            echo "$line" >>build-config
        fi
    done

    line=`echo "LO_PATH_FOR_BUILD=$PATH" | sed -e 's,/CONF-FOR-BUILD,,g'`
    echo "$line" >>build-config

    )
    test -f CONF-FOR-BUILD/build-config || AC_MSG_ERROR([Running configure script for BUILD system failed, see CONF-FOR-BUILD/config.log])
    test -f config_build.mk || AC_MSG_ERROR([A file called config_build.mk was supposed to have been copied here, but it isn't found])
    perl -pi -e 's,/CONF-FOR-BUILD,,g' config_build.mk
    eval `cat CONF-FOR-BUILD/build-config`
    AC_MSG_RESULT([checking for BUILD platform configuration... done])
    rm -rf CONF-FOR-BUILD
else
    CC_FOR_BUILD="$CC"
    CXX_FOR_BUILD="$CXX"
    CC_FOR_BUILD="$CC"
    COM_FOR_BUILD="$COM"
    CPUNAME_FOR_BUILD="$CPUNAME"
    CPU_FOR_BUILD="$CPU"
    CXX_FOR_BUILD="$CXX"
    DISABLE_PYTHON_FOR_BUILD="$DISABLE_PYTHON"
    GUIBASE_FOR_BUILD="$GUIBASE"
    GUI_FOR_BUILD="$GUI"
    INPATH_FOR_BUILD="$INPATH"
    MACOSX_DEPLOYMENT_TARGET_FOR_BUILD="$MACOSX_DEPLOYMENT_TARGET"
    OS_FOR_BUILD="$OS"
    OUTDIR_FOR_BUILD="$OUTDIR"
    OUTPATH_FOR_BUILD="$OUTPATH"
    PYTHON_FOR_BUILD="$PYTHON"
    WORKDIR_FOR_BUILD="$WORKDIR"
fi
AC_SUBST(COM_FOR_BUILD)
AC_SUBST(GUI_FOR_BUILD)
AC_SUBST(GUIBASE_FOR_BUILD)
AC_SUBST(OS_FOR_BUILD)
AC_SUBST(CPU_FOR_BUILD)
AC_SUBST(CPUNAME_FOR_BUILD)
AC_SUBST(CC_FOR_BUILD)
AC_SUBST(CXX_FOR_BUILD)
AC_SUBST(INPATH_FOR_BUILD)
AC_SUBST(OUTPATH_FOR_BUILD)
AC_SUBST(MACOSX_DEPLOYMENT_TARGET_FOR_BUILD)
AC_SUBST([PYTHON_FOR_BUILD])

AC_SUBST(OUTDIR_FOR_BUILD)
AC_SUBST(WORKDIR_FOR_BUILD)

UPD="`echo AC_PACKAGE_VERSION | sed "s/\.//"`0"
SOURCEVERSION="OOO$UPD"
AC_SUBST(UPD)
AC_SUBST(SOURCEVERSION)

dnl ===================================================================
dnl Set the WITH_MOZILLA variable.
dnl ===================================================================

AC_MSG_CHECKING([whether to enable build of Mozilla])
if test "$enable_mozilla" = "yes"; then
    AC_MSG_RESULT([yes])
    WITH_MOZILLA=YES
else
    AC_MSG_RESULT([no])
    WITH_MOZILLA=NO
    enable_mozilla=no
fi
AC_SUBST(WITH_MOZILLA)

dnl ===================================================================
dnl Set the ENABLE_CRASHDUMP variable.
dnl ===================================================================
AC_MSG_CHECKING([whether to enable crashdump feature])
if test "$enable_crashdump" = "yes"; then
    ENABLE_CRASHDUMP="TRUE"
    BUILD_TYPE="$BUILD_TYPE CRASHREP"
    AC_MSG_RESULT([yes])
else
    ENABLE_CRASHDUMP=""
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_CRASHDUMP)


dnl Set the ENABLE_WERROR variable. (Activate --enable-werror)
dnl ===================================================================
AC_MSG_CHECKING([whether to turn warnings to errors])
if test -n "$enable_werror" -a "$enable_werror" != "no"; then
    EXTERNAL_WARNINGS_NOT_ERRORS="FALSE"
    AC_MSG_RESULT([yes])
else
    EXTERNAL_WARNINGS_NOT_ERRORS="TRUE"
    AC_MSG_RESULT([no])
fi
AC_SUBST(EXTERNAL_WARNINGS_NOT_ERRORS)

dnl Set the ASSERT_ALWAYS_ABORT variable. (Activate --enable-assert-always-abort)
dnl ===================================================================
AC_MSG_CHECKING([whether to have assert to abort in release code])
if test -n "$enable_assert_always_abort" -a "$enable_assert_always_abort" = "yes"; then
    ASSERT_ALWAYS_ABORT="TRUE"
    AC_MSG_RESULT([yes])
else
    ASSERT_ALWAYS_ABORT="FALSE"
    AC_MSG_RESULT([no])
fi
AC_SUBST(ASSERT_ALWAYS_ABORT)

dnl Determine whether to use linkoo for the smoketest installation
dnl ===================================================================
AC_MSG_CHECKING([whether to use linkoo for the smoketest installation])
if test "$enable_linkoo" = "no"; then
    DISABLE_LINKOO="TRUE"
    AC_MSG_RESULT([no])
else
    DISABLE_LINKOO=
    AC_MSG_RESULT([yes])
fi
AC_SUBST(DISABLE_LINKOO)

# Set the ENABLE_LTO variable
# ===================================================================
AC_MSG_CHECKING([whether to use link-time optimization])
if test -n "$enable_lto" -a "$enable_lto" != "no"; then
    ENABLE_LTO="TRUE"
    AC_MSG_RESULT([yes])
else
    ENABLE_LTO=""
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_LTO)

if test "$enable_headless" = "yes"; then
    # be sure to do not mess with uneeded stuff
    test_randr=no
    test_xrender=no
    test_cups=no
    test_dbus=no
    test_fontconfig=yes
    test_gtk=no
    build_gstreamer=no
    build_gstreamer_0_10=no
    test_tde=no
    test_kde=no
    test_kde4=no
    test_unix_quickstarter=no
    test_cairo=no
    enable_librsvg=no
    enable_gnome_vfs=no
fi

dnl ===================================================================
dnl check for cups support
dnl ===================================================================
ENABLE_CUPS=""

if test "$enable_cups" = "no"; then
    test_cups=no
fi

AC_MSG_CHECKING([whether to enable CUPS support])
if test "$test_cups" = "yes"; then
    ENABLE_CUPS="TRUE"
    AC_MSG_RESULT([yes])

    AC_MSG_CHECKING([whether cups support is present])
    AC_CHECK_LIB([cups], [cupsPrintFiles], [:])
    AC_CHECK_HEADER(cups/cups.h, AC_DEFINE(HAVE_CUPS_H))
    if test "$ac_cv_lib_cups_cupsPrintFiles" != "yes" -a "$ac_cv_header_cups_cups_h" != "yes"; then
        AC_MSG_ERROR([Could not find CUPS. Install libcupsys2-dev or cups-devel.])
    fi

else
    AC_MSG_RESULT([no])
fi

AC_SUBST(ENABLE_CUPS)

# fontconfig checks
if test "$test_fontconfig" = "yes"; then
    PKG_CHECK_MODULES([FONTCONFIG], [fontconfig >= 2.2.0])
else
    case "$BUILD_TYPE" in
    *FONTCONFIG*)
        FONTCONFIG_LIBS="-lfontconfig"
        ;;
    esac
fi
AC_SUBST(FONTCONFIG_CFLAGS)
AC_SUBST(FONTCONFIG_LIBS)

dnl whether to find & fetch external tarballs?
dnl ===================================================================
if test -z "$TARFILE_LOCATION"; then
    TARFILE_LOCATION="$SRC_ROOT/src"
fi
AC_SUBST(TARFILE_LOCATION)

AC_MSG_CHECKING([whether we want to fetch tarballs])
if test "$enable_fetch_external" != "no"; then
    AC_MSG_RESULT([yes])
    DO_FETCH_TARBALLS="YES"
else
    AC_MSG_RESULT([no])
    DO_FETCH_TARBALLS="NO"
fi
AC_SUBST(DO_FETCH_TARBALLS)


dnl Disable legacy binary file formats filters
dnl ===================================================================
AC_MSG_CHECKING([whether to enable filters for legacy binary file formats (StarOffice 5.2)])
if test "$enable_binfilter" = "no"; then
    WITH_BINFILTER="NO"
    AC_MSG_RESULT([no])
else
    WITH_BINFILTER="YES"
    BUILD_TYPE="$BUILD_TYPE BINFILTER"
    GIT_REPO_NAMES="$GIT_REPO_NAMES binfilter"
    GIT_NEEDED_SUBMODULES="binfilter $GIT_NEEDED_SUBMODULES"
    AC_MSG_RESULT([yes])
fi
AC_SUBST(WITH_BINFILTER)

AC_MSG_CHECKING([whether to build help])
if test "$with_help" != "no" -a $_os != iOS -a $_os != Android; then
    AC_MSG_RESULT([yes])
    BUILD_TYPE="$BUILD_TYPE HELP"
    SCPDEFS="$SCPDEFS -DWITH_HELP"
    GIT_REPO_NAMES="$GIT_REPO_NAMES help"
    GIT_NEEDED_SUBMODULES="helpcontent2 $GIT_NEEDED_SUBMODULES"
else
    AC_MSG_RESULT([no])
fi

dnl Test whether to include MySpell dictionaries
dnl ===================================================================
AC_MSG_CHECKING([whether to include MySpell dictionaries])
if test -z "$with_myspell_dicts" -o "$with_myspell_dicts" = "yes"; then
    AC_MSG_RESULT([yes])
    WITH_MYSPELL_DICTS=YES
    BUILD_TYPE="$BUILD_TYPE DICTIONARIES"
    GIT_REPO_NAMES="$GIT_REPO_NAMES dictionaries"
    GIT_NEEDED_SUBMODULES="dictionaries $GIT_NEEDED_SUBMODULES"
else
    AC_MSG_RESULT([no])
    WITH_MYSPELL_DICTS=NO
fi
AC_SUBST(WITH_MYSPELL_DICTS)

AC_MSG_CHECKING([whether to use dicts from external paths])
if test -z "$with_system_dicts" -o "$with_system_dicts" != "no"; then
    AC_MSG_RESULT([yes])
    SYSTEM_DICTS=YES
    AC_MSG_CHECKING([for spelling dictionary directory])
    if test -n "$with_external_dict_dir"; then
        DICT_SYSTEM_DIR=file://$with_external_dict_dir
    else
        DICT_SYSTEM_DIR=file:///usr/share/hunspell
        if test ! -d /usr/share/hunspell -a -d /usr/share/myspell; then
            DICT_SYSTEM_DIR=file:///usr/share/myspell
        fi
    fi
    AC_MSG_RESULT([$DICT_SYSTEM_DIR])
    AC_MSG_CHECKING([for hyphenation patterns directory])
    if test -n "$with_external_hyph_dir"; then
        HYPH_SYSTEM_DIR=file://$with_external_hyph_dir
    else
        HYPH_SYSTEM_DIR=file:///usr/share/hyphen
    fi
    AC_MSG_RESULT([$HYPH_SYSTEM_DIR])
    AC_MSG_CHECKING([for thesaurus directory])
    if test -n "$with_external_thes_dir"; then
        THES_SYSTEM_DIR=file://$with_external_thes_dir
    else
        THES_SYSTEM_DIR=file:///usr/share/mythes
    fi
    AC_MSG_RESULT([$THES_SYSTEM_DIR])
else
    AC_MSG_RESULT([no])
    SYSTEM_DICTS=NO
fi
AC_SUBST(SYSTEM_DICTS)
AC_SUBST(DICT_SYSTEM_DIR)
AC_SUBST(HYPH_SYSTEM_DIR)
AC_SUBST(THES_SYSTEM_DIR)

dnl ===================================================================
dnl ENABLE_PCH i now a no-op
dnl ===================================================================
AC_MSG_CHECKING([whether to enable pch feature])
AC_MSG_RESULT([no, obsolete])

dnl ===================================================================
dnl Search all the common names for GNU make
dnl ===================================================================
AC_MSG_CHECKING([for GNU make])

# try to use our own make if it is available and GNUMAKE was not already defined
if test -z "$GNUMAKE"; then
    if test -x "/opt/lo/bin/make"; then
        GNUMAKE="/opt/lo/bin/make"
    fi
fi

for a in "$MAKE" "$GNUMAKE" make gmake gnumake; do
    if test -n "$a"; then
        $a --version 2> /dev/null | grep GNU  2>&1 > /dev/null
        if test $? -eq 0;  then
            GNUMAKE=$a
            break
        fi
    fi
done
AC_MSG_RESULT($GNUMAKE)
if test -z "$GNUMAKE"; then
    AC_MSG_ERROR([not found. install GNU make.])
fi

AC_MSG_CHECKING([the GNU make version])
_make_version=`$GNUMAKE --version | grep GNU | $GREP -v GPL | $SED -e 's@^[[^0-9]]*@@' -e 's@ .*@@' -e 's@,.*@@'`
_make_longver=`echo $_make_version | $AWK -F. '{ print \$1*10000+\$2*100+\$3 }'`
if test "$_make_longver" -ge "038200"; then
    AC_MSG_RESULT([$GNUMAKE $_make_version])

elif test "$_make_longver" -ge "038100"; then
    if test "$build_os" = "cygwin"; then
        AC_MSG_ERROR([failed ($GNUMAKE version >= 3.82 needed])
    fi
    AC_MSG_RESULT([$GNUMAKE $_make_version])

    dnl ===================================================================
    dnl Search all the common names for sha1sum
    dnl ===================================================================
    AC_PATH_PROGS(SHA1SUM, sha1sum sha1 shasum)
    if test -z "$SHA1SUM"; then
        AC_MSG_ERROR([install the approproate SHA-1 checksumming program for this OS])
    fi
    AC_MSG_CHECKING([for GNU make bug 20033])
    TESTGMAKEBUG20033=`mktemp -d tmp.XXXXXX`
    cat > $TESTGMAKEBUG20033/Makefile << EOF
A := \$(wildcard *.a)

.PHONY: all
all: \$(A:.a=.b)
	@echo survived bug20033. #dont kill these tabs, you will break the Makefile!!!!

.PHONY: setup
setup:
	@touch 1.a 2.a 3.a 4.a 5.a 6.a

define d1
@echo lala \$(1)
@sleep 1
endef

define d2
@echo tyty \$(1)
@sleep 1
endef

%.b : %.a
	\$(eval CHECKSUM := \$(word 1,\$(shell cat \$^ | $SHA1SUM))) \$(if \$(wildcard \$(CACHEDIR)/\$(CHECKSUM)),\
	\$(call d1,\$(CHECKSUM)),\
	\$(call d2,\$(CHECKSUM)))
EOF
    if test -z "`(cd $TESTGMAKEBUG20033 && $GNUMAKE setup && $GNUMAKE -j)|grep survived`"; then
        no_parallelism_make="YES"
        AC_MSG_RESULT([yes, disable parallelism])
    else
        AC_MSG_RESULT([no, keep parallelism enabled])
    fi
    rm -rf $TESTGMAKEBUG20033
else
    AC_MSG_ERROR([failed ($GNUMAKE version >= 3.81 needed])
fi

# find if gnumake support file function
AC_MSG_CHECKING([whether GNU make supports the 'file' function])
TESTGMAKEFILEFUNC="`mktemp -d -t tst.XXXXXX`"
cat > $TESTGMAKEFILEFUNC/Makefile << EOF
\$(file >test.txt,Success )

.PHONY: all
all:
	@cat test.txt

EOF
$GNUMAKE -C $TESTGMAKEFILEFUNC 2>/dev/null 1>&2
if test -f $TESTGMAKEFILEFUNC/test.txt ; then
    HAVE_GNUMAKE_FILE_FUNC="YES"
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
rm -rf $TESTGMAKEFILEFUNC
AC_SUBST(HAVE_GNUMAKE_FILE_FUNC)
AC_SUBST(GNUMAKE)

_make_ver_check=`$GNUMAKE --version | grep LibreOffice`
STALE_MAKE=
make_warning=
if test "$_make_ver_check" = ""; then
   STALE_MAKE=TRUE
fi

HAVE_LD_HASH_STYLE=FALSE
WITH_LINKER_HASH_STYLE=
AC_MSG_CHECKING( for --hash-style gcc linker support )
if test "$GCC" = "yes"; then
    if test -z "$with_linker_hash_style" -o "$with_linker_hash_style" = "yes"; then
        hash_styles="gnu sysv"
    elif test "$with_linker_hash_style" = "no"; then
        hash_styles=
    else
        hash_styles="$with_linker_hash_style"
    fi

    for hash_style in $hash_styles; do
        test "$HAVE_LD_HASH_STYLE" = "TRUE" && continue
        hash_style_ldflags_save=$LDFLAGS
        LDFLAGS="$LDFLAGS -Wl,--hash-style=$hash_style"

        AC_LINK_IFELSE([AC_LANG_PROGRAM(
            [
#include <stdio.h>
            ],[
printf ("");
            ])],
            [ if ./conftest$EXEEXT; then
                  HAVE_LD_HASH_STYLE=TRUE
                  WITH_LINKER_HASH_STYLE=$hash_style
              fi],
            [HAVE_LD_HASH_STYLE=FALSE])
        LDFLAGS=$hash_style_ldflags_save
    done

    if test "$HAVE_LD_HASH_STYLE" = "TRUE"; then
        AC_MSG_RESULT( $WITH_LINKER_HASH_STYLE )
    else
        AC_MSG_RESULT( no )
    fi
    LDFLAGS=$hash_style_ldflags_save
else
    AC_MSG_RESULT( no )
fi
AC_SUBST(HAVE_LD_HASH_STYLE)
AC_SUBST(WITH_LINKER_HASH_STYLE)

dnl ===================================================================
dnl Check whether there's a Perl version available.
dnl ===================================================================
if test -z "$with_perl_home"; then
    AC_PATH_PROG(PERL, perl)
else
    test "$build_os" = "cygwin" && with_perl_home=`cygpath -u "$with_perl_home"`
    _perl_path="$with_perl_home/bin/perl"
    if test -x "$_perl_path"; then
        PERL=$_perl_path
    else
        AC_MSG_ERROR([$_perl_path not found])
    fi
fi

dnl ===================================================================
dnl Testing for Perl version 5 or greater.
dnl $] is the Perl version variable, it is returned as an integer
dnl ===================================================================
if test "$PERL"; then
    AC_MSG_CHECKING([the Perl version])
    ${PERL} -e "exit($]);"
    _perl_version=$?
    if test "$_perl_version" -lt 5; then
        AC_MSG_ERROR([found Perl version "$_perl_version", use version 5 of Perl])
    fi
    AC_MSG_RESULT([checked (perl $_perl_version)])
else
    AC_MSG_ERROR([Perl not found, install version 5 of Perl])
fi

dnl ===================================================================
dnl Testing for required Perl modules
dnl ===================================================================

AC_MSG_CHECKING([for required Perl modules])
if `$PERL -e 'use Archive::Zip; use Cwd; use Digest::MD5'`; then
    AC_MSG_RESULT([all modules found])
else
    AC_MSG_ERROR([Failed to find some modules])
fi


dnl ===================================================================
dnl Check for pkg-config
dnl ===================================================================
if test "$_os" != "WINNT" -o "$WITH_MINGW" = "yes"; then
    PKG_PROG_PKG_CONFIG
fi

if test "$_os" != "WINNT" -o "$WITH_MINGW" = "yes"; then

    # If you use CC=/path/to/compiler/foo-gcc or even CC="ccache
    # /path/to/compiler/foo-gcc" you need to set the AR etc env vars
    # explicitly. Or put /path/to/compiler in PATH yourself.

    AC_CHECK_TOOL(AR,ar)
    AC_CHECK_TOOL(NM,nm)
    AC_CHECK_TOOL(OBJDUMP,objdump)
    AC_CHECK_TOOL(RANLIB,ranlib)
    AC_CHECK_TOOL(STRIP,strip)
    if test "$_os" = "WINNT"; then
        AC_CHECK_TOOL(DLLTOOL,dlltool)
        AC_CHECK_TOOL(WINDRES,windres)
    fi
fi
AC_SUBST(AR)
AC_SUBST(DLLTOOL)
AC_SUBST(NM)
AC_SUBST(OBJDUMP)
AC_SUBST(PKG_CONFIG)
AC_SUBST(RANLIB)
AC_SUBST(STRIP)
AC_SUBST(WINDRES)

dnl ===================================================================
dnl pkg-config checks on Mac OS X
dnl (but --enable-mozilla /does/ require a carefully crafted pkg-config)
dnl ===================================================================

if test $_os = Darwin -a "$enable_mozilla" = no; then
    AC_MSG_CHECKING([for bogus pkg-config])
    if test -n "$PKG_CONFIG"; then
        if test "$PKG_CONFIG" = /usr/bin/pkg-config && ls -l /usr/bin/pkg-config | $GREP -q Mono.framework; then
            AC_MSG_ERROR([yes, from Mono. This *will* break the build. Please remove or hide $PKG_CONFIG])
        else
            if test "$enable_bogus_pkg_config" = "yes"; then
                AC_MSG_RESULT([yes, user-approved from unknown origin.])
            else
                AC_MSG_ERROR([yes, from unknown origin. This *will* break the build. Please remove or hide $PKG_CONFIG])
            fi
        fi
    else
        AC_MSG_RESULT([no, good])
    fi
fi

dnl ===================================================================
dnl  .NET needs special treatment
dnl (does the above comment really mean .NET, or is it misusing
dnl that to mean Visual Studio .NET 2003 ? And does this also
dnl in fact apply equally to what we actually support, i.e.
dnl Visual Studio 2008 and 2010?)
dnl ===================================================================
if test "$build_os" = "cygwin"; then
    dnl Check midl.exe
    AC_PATH_PROG(MIDL_PATH, midl.exe)
    if test -n "$MIDL_PATH";then
        MIDL_PATH=`dirname "$MIDL_PATH"`
    fi
    if test -n "$with_midl_path";then
        with_midl_path=`cygpath -u "$with_midl_path"`
    fi
    if test -x "$with_midl_path/midl.exe"; then
        MIDL_PATH="$with_midl_path"
    fi
    if test -z "$MIDL_PATH" -a -e "$with_cl_home/../Common7/Tools/Bin/midl.exe"; then
        MIDL_PATH="$with_cl_home/../Common7/Tools/Bin"
    fi
    if test -z "$MIDL_PATH"; then
        vstest=`./oowintool --msvs-productdir`
        if test -x "$vstest/Common7/Tools/Bin/midl.exe"; then
            MIDL_PATH="$vstest/Common7/Tools/Bin"
        fi
    fi
    if test -z "$MIDL_PATH"; then
        winsdktest=`./oowintool --windows-sdk-home`
        if test -x "$winsdktest/Bin/midl.exe"; then
            MIDL_PATH="$winsdktest/Bin"
        elif test -x "$winsdktest/Bin/$SDK_ARCH/midl.exe"; then
            MIDL_PATH="$winsdktest/Bin/$SDK_ARCH"
        fi
    fi
    if test ! -x "$MIDL_PATH/midl.exe"; then
        AC_MSG_ERROR([midl.exe not found. Make sure it's in PATH or use --with-midl-path])
    fi
    # Convert to posix path with 8.3 filename restrictions ( No spaces )
    MIDL_PATH=`cygpath -d "$MIDL_PATH"`
    MIDL_PATH=`cygpath -u "$MIDL_PATH"`

    dnl Check csc.exe
    AC_PATH_PROG(CSC_PATH, csc.exe)
    if test -n "$CSC_PATH";then
        CSC_PATH=`dirname "$CSC_PATH"`
    fi
    if test -n "$with_csc_path";then
        with_csc_path=`cygpath -u "$with_csc_path"`
    fi
    if test -x "$with_csc_path/csc.exe"; then
        CSC_PATH="$with_csc_path"
    else
       csctest=`./oowintool --csc-compilerdir`
       if test -x "$csctest/csc.exe"; then
           CSC_PATH="$csctest"
       fi
    fi
    if test ! -x "$CSC_PATH/csc.exe"; then
        AC_MSG_ERROR([csc.exe not found. Make sure it's in the path or use --with-csc-path])
    fi
    # Convert to posix path with 8.3 filename restrictions ( No spaces )
    CSC_PATH=`cygpath -d "$CSC_PATH"`
    CSC_PATH=`cygpath -u "$CSC_PATH"`

    dnl Check al.exe
    AC_PATH_PROG(AL_PATH, al.exe)
    if test -n "$AL_PATH";then
        AL_PATH=`dirname "$AL_PATH"`
    fi
    if test -n "$with_al_path";then
        with_al_path=`cygpath -u "$with_al_path"`
    fi
    if test -x "$with_al_path/al.exe"; then
        AL_PATH="$with_al_path"
    fi
    if test -z "$AL_PATH"; then
        winsdktest=`./oowintool --windows-sdk-home`
        if test -x "$winsdktest/Bin/al.exe"; then
            AL_PATH="$winsdktest/Bin"
        fi
    fi
    if test -z "$AL_PATH"; then
        altest=`./oowintool --al-home`
        if test -x "$altest/bin/al.exe"; then
            AL_PATH="$altest/bin"
        elif test -x "$altest/al.exe"; then
            AL_PATH="$altest"
        fi
    fi
    if test ! -x "$AL_PATH/al.exe"; then
        AC_MSG_ERROR([al.exe not found. Make sure it's in PATH or use --with-al-path])
    fi
    # Convert to posix path with 8.3 filename restrictions ( No spaces )
    AL_PATH=`cygpath -d "$AL_PATH"`
    AL_PATH=`cygpath -u "$AL_PATH"`

    dnl Check mscoree.lib / .NET Framework dir
    AC_MSG_CHECKING(.NET Framework)
    if test -n "$with_dotnet_framework_home"; then
        with_dotnet_framework_home=`cygpath -u "$with_dotnet_framework_home"`
    fi
        if test -f "$with_dotnet_framework_home/lib/mscoree.lib"; then
        DOTNET_FRAMEWORK_HOME="$with_dotnet_framework_home"
    fi
    if test -z "$DOTNET_FRAMEWORK_HOME"; then
        frametest=`./oowintool --dotnetsdk-dir`
        if test -f "$frametest/lib/mscoree.lib"; then
            DOTNET_FRAMEWORK_HOME="$frametest"
        else
            frametest=`./oowintool --windows-sdk-home`
            if test -f "$frametest/lib/mscoree.lib" -o -f "$frametest/lib/win8/um/$SDK_ARCH/mscoree.lib"; then
                DOTNET_FRAMEWORK_HOME="$frametest"
            fi
        fi
    fi
    if test ! -f "$DOTNET_FRAMEWORK_HOME/lib/mscoree.lib" -a ! -f "$DOTNET_FRAMEWORK_HOME/lib/win8/um/$SDK_ARCH/mscoree.lib"; then
        AC_MSG_ERROR([mscoree.lib (.NET Framework) not found. Make sure you use --with-dotnet-framework-home])
    fi
    AC_MSG_RESULT(found)

    PathFormat "$MIDL_PATH"
    MIDL_PATH="$formatted_path"

    PathFormat "$AL_PATH"
    AL_PATH="$formatted_path"

    PathFormat "$DOTNET_FRAMEWORK_HOME"
    DOTNET_FRAMEWORK_HOME="$formatted_path"

    PathFormat "$CSC_PATH"
    CSC_PATH="$formatted_path"

fi

dnl ===================================================================
dnl Check if stdc headers are available excluding MSVC.
dnl ===================================================================
if test "$_os" != "WINNT" -o "$WITH_MINGW" = "yes"; then
    AC_HEADER_STDC
fi

dnl ===================================================================
dnl Testing for C++ compiler and version...
dnl ===================================================================

if test "$_os" != "WINNT" -o "$WITH_MINGW" = "yes"; then
    # AC_PROG_CXX sets CXXFLAGS to -g -O2 if not set, avoid that
    save_CXXFLAGS=$CXXFLAGS
    AC_PROG_CXX
    CXXFLAGS=$save_CXXFLAGS
else
    if test -n "$CC" -a -z "$CXX"; then
        CXX="$CC"
    fi
fi

dnl check if we are using a buggy version of g++ (currently 3.4.0, 3.4.1 and trunk)
if test "$GXX" = "yes"; then
    AC_MSG_CHECKING([the GNU C++ compiler version])

    _gpp_version=`$CXX -dumpversion`
    _gpp_majmin=`echo $_gpp_version | $AWK -F. '{ print \$1*100+\$2 }'`

    if test "$_os" = "Darwin" -a "$with_macosx_sdk" = "10.4" -a "$_gpp_majmin" -ge "401"; then
        if test -z "$save_CXX" -a -x "$GCC_HOME/bin/g++-4.0"; then
            export CXX="$GCC_HOME/bin/g++-4.0"
            _gpp_majmin_2=`$CXX -dumpversion | $AWK -F. '{ print \$1*100+\$2 }'`
            if test "$_gpp_majmin_2" -ge "400" -a "$_gpp_majmin_2" -lt "401"; then
                _gpp_majmin=$_gpp_majmin_2
            fi
        fi
        if test "$_gpp_majmin" -ge "401"; then
            AC_MSG_ERROR([You need to use the g++-4.0 compiler (g++ $_gpp_version won't work with the MacOSX10.4u.sdk) - set CXX accordingly])
        else
            AC_MSG_RESULT([implicitly using CXX=$CXX])
        fi
    else
        AC_MSG_RESULT([checked (g++ $_gpp_version)])
    fi

    if test "$_gpp_majmin" = "304"; then
        AC_MSG_CHECKING([whether $CXX has the enum bug])
        AC_RUN_IFELSE([AC_LANG_SOURCE([[
            extern "C" void abort (void);
            extern "C" void exit (int status);

            enum E { E0, E1, E2, E3, E4, E5 };

            void test (enum E e)
            {
                if (e == E2 || e == E3 || e == E1)
                    exit (1);
            }

            int main (void)
            {
                test (E4);
                test (E5);
                test (E0);
                return 0;
            }
            ]])],[AC_MSG_ERROR([your version of the GNU C++ compile has a bug which prevents LibreOffice from being compiled correctly - please check http://gcc.gnu.org/ml/gcc-patches/2004-07/msg00968.html for details.])],[AC_MSG_RESULT([no])],[])
    fi
fi

#
# prefx CXX with ccache if needed
#
if test "$_os" != "WINNT" -o "$WITH_MINGW" = "yes"; then
    if test "$CCACHE" != ""; then
        AC_MSG_CHECKING([whether $CXX is already ccached])
        AC_LANG_PUSH([C++])
        save_CXXFLAGS=$CXXFLAGS
        CXXFLAGS="$CXXFLAGS --ccache-skip -O2"
        dnl an empty program will do, we're checking the compiler flags
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],[])],
                          [use_ccache=yes], [use_ccache=no])
        if test $use_ccache = yes; then
            AC_MSG_RESULT([yes])
        else
            CXX="$CCACHE $CXX"
            AC_MSG_RESULT([no])
        fi
        CXXFLAGS=$save_CXXFLAGS
        AC_LANG_POP([C++])
    fi
fi

dnl ===================================================================
dnl Find pre-processors.(should do that _after_ messing with CC/CXX)
dnl ===================================================================

if test "$_os" != "WINNT" -o "$WITH_MINGW" = "yes"; then
    AC_PROG_CXXCPP

    dnl Check whether there's a C pre-processor.
    dnl ===================================================================
    dnl When using SunStudio compiler, there is a bug with the cc
    dnl preprocessor, so use CC preprocessor as the cc preprocessor
    dnl See Issuezilla #445.
    dnl ===================================================================
    if test "$_os" = "SunOS"; then
        CPP=$CXXCPP
    else
        AC_PROG_CPP
    fi
fi


dnl ===================================================================
dnl Find integral type sizes and alignments
dnl ===================================================================

if test "$_os" != "WINNT" -o "$WITH_MINGW" = "yes"; then

    AC_CHECK_SIZEOF(long)
    AC_CHECK_SIZEOF(short)
    AC_CHECK_SIZEOF(int)
    AC_CHECK_SIZEOF(long long)
    AC_CHECK_SIZEOF(double)
    AC_CHECK_SIZEOF(void*)

    SIZEOF_SHORT=$ac_cv_sizeof_short
    SIZEOF_INT=$ac_cv_sizeof_int
    SIZEOF_LONG=$ac_cv_sizeof_long
    SIZEOF_LONGLONG=$ac_cv_sizeof_long_long
    SIZEOF_DOUBLE=$ac_cv_sizeof_double
    SIZEOF_POINTER=$ac_cv_sizeof_voidp

    dnl Allow build without AC_CHECK_ALIGNOF, grrr
    m4_pattern_allow([AC_CHECK_ALIGNOF])
    m4_ifdef([AC_CHECK_ALIGNOF],
        [
            AC_CHECK_ALIGNOF(short,[#include <stddef.h>])
            AC_CHECK_ALIGNOF(int,[#include <stddef.h>])
            AC_CHECK_ALIGNOF(long,[#include <stddef.h>])
            AC_CHECK_ALIGNOF(double,[#include <stddef.h>])
        ],
        [
            case "$_os-$host_cpu" in
            Darwin-powerpc)
                test -z "$ac_cv_alignof_short" && ac_cv_alignof_short=1
                test -z "$ac_cv_alignof_int" && ac_cv_alignof_int=1
                test -z "$ac_cv_alignof_long" && ac_cv_alignof_long=1
                test -z "$ac_cv_alignof_double" && ac_cv_alignof_double=1
                ;;
            *)
                if test -z "$ac_cv_alignof_short" -o \
                        -z "$ac_cv_alignof_int" -o \
                        -z "$ac_cv_alignof_long" -o \
                        -z "$ac_cv_alignof_double"; then
                   AC_MSG_ERROR([Your Autoconf doesn't have [AC_][CHECK_ALIGNOF]. You need to set the environment variables ac_cv_alignof_short, ac_cv_alignof_int, ac_cv_alignof_long and ac_cv_alignof_double.])
                fi
                ;;
            esac
        ])

    ALIGNOF_SHORT=$ac_cv_alignof_short
    ALIGNOF_INT=$ac_cv_alignof_int
    ALIGNOF_LONG=$ac_cv_alignof_long
    ALIGNOF_DOUBLE=$ac_cv_alignof_double

    AC_C_BIGENDIAN
    WORDS_BIGENDIAN=$ac_cv_c_bigendian

    dnl Check for large file support
    AC_SYS_LARGEFILE
    if test -n "$ac_cv_sys_file_offset_bits" -a "$ac_cv_sys_file_offset_bits" != "no"; then
        LFS_CFLAGS="-D_FILE_OFFSET_BITS=$ac_cv_sys_file_offset_bits"
    fi
    if test -n "$ac_cv_sys_large_files" -a "$ac_cv_sys_large_files" != "no"; then
        LFS_CFLAGS="$LFS_CFLAGS -D_LARGE_FILES"
    fi
else
    # Hardcode for MSVC
    SIZEOF_SHORT=2
    SIZEOF_INT=4
    SIZEOF_LONG=4
    SIZEOF_LONGLONG=8
    if test "$BITNESS_OVERRIDE" = ""; then
        SIZEOF_POINTER=4
    else
        SIZEOF_POINTER=8
    fi
    ALIGNOF_SHORT=2
    ALIGNOF_INT=4
    ALIGNOF_LONG=4
    ALIGNOF_DOUBLE=8
    WORDS_BIGENDIAN=no
    LFS_CFLAGS=''
fi
AC_SUBST(WORDS_BIGENDIAN)
AC_SUBST(LFS_CFLAGS)

AC_SUBST(SIZEOF_SHORT)
AC_SUBST(SIZEOF_INT)
AC_SUBST(SIZEOF_LONG)
AC_SUBST(SIZEOF_LONGLONG)
AC_SUBST(SIZEOF_DOUBLE)
AC_SUBST(SIZEOF_POINTER)
AC_SUBST(ALIGNOF_SHORT)
AC_SUBST(ALIGNOF_INT)
AC_SUBST(ALIGNOF_LONG)
AC_SUBST(ALIGNOF_DOUBLE)

dnl ===================================================================
dnl Check if valgrind.h is available
dnl ===================================================================
if test "$cross_compiling" != "yes" -a -n "$ENABLE_DBGUTIL" -a \
        -z "$VALGRIND_CFLAGS"; then
    dnl Test $prefix (currently only testing for /usr and /usr/local)
    dnl so that VALGRIND_CFLAGS = -I$prefix/include/valgrind
    prev_cppflags=$CPPFLAGS
    CPPFLAGS="-I/usr/include/valgrind"
    AC_CHECK_HEADER([valgrind.h], [VALGRIND_CFLAGS=$CPPFLAGS], [unset ac_cv_header_valgrind_h])
    if test -z "$VALGRIND_CFLAGS"; then
        CPPFLAGS="-I/usr/local/include/valgrind"
        AC_CHECK_HEADER([valgrind.h], [VALGRIND_CFLAGS=$CPPFLAGS], [])
    fi
    if test -n "$VALGRIND_CFLAGS"; then
        CPPFLAGS=$VALGRIND_CFLAGS
        AC_CHECK_HEADER([memcheck.h], [], [VALGRIND_CFLAGS=""])
    fi
    CPPFLAGS=$prev_cppflags
fi
AC_SUBST([VALGRIND_CFLAGS])

dnl ===================================================================
dnl Compiler plugins
dnl ===================================================================

COMPILER_PLUGINS=
# currently only Clang
if test "$COM_GCC_IS_CLANG" = "TRUE"; then
    if test -n "$enable_compiler_plugins"; then
        compiler_plugins="$enable_compiler_plugins"
    elif test -n "$ENABLE_DBGUTIL"; then
        compiler_plugins=test
    else
        compiler_plugins=no
    fi
    if test "$compiler_plugins" != "no"; then
        AC_LANG_PUSH([C++])
        save_CPPFLAGS=$CPPFLAGS
        CPPFLAGS="$CPPFLAGS $CLANGPLUGIN_CPPFLAGS -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS"
        AC_CHECK_HEADER(clang/AST/RecursiveASTVisitor.h,
            [COMPILER_PLUGINS=TRUE],
            [
            if test "$compiler_plugins" = "yes"; then
                AC_MSG_ERROR([Cannot find Clang headers to build compiler plugins.])
            else
                AC_MSG_WARN([Cannot find Clang headers to build compiler plugins, plugins disabled])
                add_warning "Cannot find Clang headers to build compiler plugins, plugins disabled."
            fi
            ])
        CPPFLAGS=$save_CPPFLAGS
        AC_LANG_POP([C++])
    fi
else
    if test "$enable_compiler_plugins" = "yes"; then
        AC_MSG_ERROR([Compiler plugins are currently supported only with the Clang compiler.])
    fi
fi
AC_SUBST(COMPILER_PLUGINS)
AC_SUBST(CLANGPLUGIN_CPPFLAGS)

dnl ===================================================================
dnl Set the MinGW sys-root
dnl ===================================================================
if test "$WITH_MINGW" = "yes"; then
    for sysroot in /usr/i686-w64-mingw32/sys-root/mingw; do
        if test -d "$sysroot"; then
            MINGW_SYSROOT="$sysroot"
            break
        fi
    done
fi
AC_SUBST([MINGW_SYSROOT])

dnl ===================================================================
dnl Set the MinGW include directories
dnl ===================================================================
if test "$WITH_MINGW" = "yes"; then
    AC_MSG_CHECKING([for MinGW include path])
    cat >conftest.$ac_ext <<_ACEOF
#include <stddef.h>
#include <bits/c++config.h>
_ACEOF
    _mingw_lib_include_path=`$CXX -E -xc++ conftest.$ac_ext | $SED -n -e '/.*1*"\(.*\)\/stddef.h".*/s//\1/p' -e '/.*1*"\(.*\)\/bits\/c++config.h".*/s//\1/p' | sort -u | xargs echo`
    rm conftest.$ac_ext
    if test -z "$_mingw_lib_include_path"; then
        _mingw_lib_include_path="NO_LIB_INCLUDE"
        AC_MSG_RESULT([no MinGW include path])
    else
        AC_MSG_RESULT([$_mingw_lib_include_path])
    fi
    MINGW_LIB_INCLUDE_PATH="$_mingw_lib_include_path"
    AC_SUBST(MINGW_LIB_INCLUDE_PATH)

    mingw_crtbegin=`$CC -print-file-name=crtbegin.o`
    MINGW_CLIB_DIR=`dirname $mingw_crtbegin`

    AC_LANG_PUSH([C++])

    AC_MSG_CHECKING([for dynamic libgcc])
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <iostream>
using namespace std;
]], [[ try { throw 42; } catch (int e) { cout << "Yep, " << e << endl; } ]])],[
            MINGW_GCCDLL=`$OBJDUMP -p conftest.exe | grep 'DLL Name: libgcc' | $SED -e 's@.*DLL Name: @@'`
            if test -n "$MINGW_GCCDLL"; then
                MINGW_SHARED_GCCLIB=YES
                AC_MSG_RESULT([ $MINGW_GCCDLL])
            else
                MINGW_SHARED_GCCLIB=NO
                AC_MSG_RESULT([no])
            fi
       ],[ AC_MSG_RESULT([no])

    ])

    AC_MSG_CHECKING([for dynamic libstdc++])
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <iostream>
using namespace std;
]], [[ cout << "Hello there." << endl; ]])],[
            MINGW_GXXDLL=`$OBJDUMP -p conftest.exe | grep 'DLL Name: libstdc++' | $SED -e 's@.*DLL Name: @@'`
            if test -n "$MINGW_GXXDLL"; then
                mingw_gxxdll_root=${MINGW_GXXDLL%.dll}
                mingw_gxxdll_root=${mingw_gxxdll_root#lib}
                if test "$CROSS_COMPILING" = "YES" ; then
                    dnl m4 escaping!
                    mingw_gxxdll_root=${mingw_gxxdll_root%-[[0-9]]}
                fi
                MINGW_SHARED_LIBSTDCPP=-l$mingw_gxxdll_root
                MINGW_SHARED_GXXLIB=YES
                AC_MSG_RESULT([$MINGW_GXXDLL])
            else
                MINGW_SHARED_GXXLIB=NO
                AC_MSG_RESULT([no])
            fi
       ],[ AC_MSG_RESULT([no])

    ])

    AC_LANG_POP([C++])

    AC_SUBST(MINGW_CLIB_DIR)
    AC_SUBST(MINGW_SHARED_GCCLIB)
    AC_SUBST(MINGW_SHARED_GXXLIB)
    AC_SUBST(MINGW_SHARED_LIBSTDCPP)
    AC_SUBST(MINGW_GCCDLL)
    AC_SUBST(MINGW_GXXDLL)
fi

dnl ===================================================================
dnl Extra checking for the SunOS compiler
dnl ===================================================================
if test "$_os" = "SunOS"; then
    dnl SunStudio C++ compiler packaged with SunStudio C compiler
    if test "$CC" = "cc"; then
    AC_MSG_CHECKING([SunStudio C++ Compiler])
        if test "$CXX" != "CC"; then
            AC_MSG_WARN([SunStudio C++ was not found])
            add_warning "SunStudio C++ was not found"
        else
            AC_MSG_RESULT([checked])
        fi
    fi
fi

dnl *************************************************************
dnl Testing for exception handling - dwarf2 or sjlj exceptions...
dnl *************************************************************
if test "$WITH_MINGW" = "yes"; then
    AC_MSG_CHECKING([exception type])
    AC_LANG_PUSH([C++])
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <iostream>

        extern "C" void _Unwind_SjLj_RaiseException(void) __attribute__ ((__noreturn__));

        ]], [[_Unwind_SjLj_RaiseException() ]])],[exceptions_type="sjlj"],[exceptions_type="dwarf2"
    ])
    AC_MSG_RESULT($exceptions_type)
    AC_LANG_POP([C++])
fi

EXCEPTIONS="$exceptions_type"
AC_SUBST(EXCEPTIONS)

dnl ===================================================================
dnl Checks for what the default STL should be
dnl ===================================================================
AC_MSG_CHECKING([Whether building STLPort library makes sense])
BUILD_STLPORT="no"
if test "$_os" = "Linux"; then
    case "$host_cpu" in
    i?86)
        case "$host_os" in
        k*bsd*-gnu*)
            BUILD_STLPORT="no"
            ;;
        *)
            BUILD_STLPORT="yes"
            ;;
        esac
        ;;
    *)
        BUILD_STLPORT="no"
        ;;
    esac
elif test "$_os" = "SunOS" -a "$COM" = "C52"; then
    BUILD_STLPORT="yes"
elif test "$_os" = "WINNT" -a "$WITH_MINGW" != "yes"; then
    BUILD_STLPORT="yes"
elif test "$_os" = "FreeBSD"; then
    BUILD_STLPORT="yes"
fi
if test "$BUILD_STLPORT" = "yes"; then
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi

AC_MSG_CHECKING([Whether STLPort library will be actually built])
if test "$with_stlport" = "auto" -o "$BUILD_STLPORT" = "no"; then
    with_stlport=$BUILD_STLPORT
fi
if test "$with_stlport" = "yes"; then
    AC_MSG_RESULT([yes])
    WITH_STLPORT=YES
    SCPDEFS="$SCPDEFS -DWITH_STLPORT"
    BUILD_TYPE="$BUILD_TYPE STLPORT"
else
    WITH_STLPORT=NO
    AC_MSG_RESULT([no])
fi

AC_SUBST(WITH_STLPORT)

dnl ===================================================================
dnl gxx include directories needed by STLPort
dnl ===================================================================
# Removed the special FreeBSD treatment. The problem was that with_gxx_include_path
# often contains an i386 which is expanded as a macro.
if test "$GXX" = "yes"; then
    AC_MSG_CHECKING([for g++ include path])
    if test -z "$with_gxx_include_path"; then
        with_gxx_include_path=`echo "#include <cstring>" | $CXX -E -xc++ - 2>/dev/null | $SED -n '/.*1*"\(.*\)\/cstring".*/s//\1/p' | head -n 1`
        if test "$with_gxx_include_path" = "/usr/libexec/(null)/include"; then
            with_gxx_include_path="/usr/include"
        fi
        if echo $with_gxx_include_path | $GREP -q linux; then
            # workaround for Mandriva - issue 100049
            with_gxx_include_path=`cd -P $with_gxx_include_path && pwd`
        fi
    fi
    dnl This is the original code...
    dnl with_gxx_include_path=`$CXX -print-search-dirs | grep instal |$AWK '{ print \$2 }'`/include
    if test -z "$with_gxx_include_path"; then
        with_gxx_include_path="NO_GXX_INCLUDE"
        AC_MSG_RESULT([none])
    else
        PathFormat "$with_gxx_include_path"
        with_gxx_include_path="$formatted_path"
        AC_MSG_RESULT([$with_gxx_include_path])
    fi
else
    with_gxx_include_path="NO_GXX_INCLUDE"
fi
GXX_INCLUDE_PATH="$with_gxx_include_path"
AC_SUBST(GXX_INCLUDE_PATH)

dnl ===================================================================
dnl thread-safe statics
dnl ===================================================================
AC_MSG_CHECKING([whether $CXX supports thread safe statics])
unset HAVE_THREADSAFE_STATICS
if test "$GCC" = "yes"; then
    dnl -fthreadsafe-statics is available since GCC 4, so always available for
    dnl us.  However, some C++ runtimes use a single lock for all static
    dnl variables, which can cause deadlock in multi-threaded applications.
    dnl This is not easily tested here; for POSIX-based systems, if executing
    dnl the following C++ program does not terminate then the tool chain
    dnl apparently has this problem:
    dnl
    dnl   #include <pthread.h>
    dnl   int h() { return 0; }
    dnl   void * g(void * unused) {
    dnl     static int n = h();
    dnl     return &n;
    dnl   }
    dnl   int f() {
    dnl     pthread_t t;
    dnl     pthread_create(&t, 0, g, 0);
    dnl     pthread_join(t, 0);
    dnl     return 0;
    dnl   }
    dnl   int main() {
    dnl     static int n = f();
    dnl     return n;
    dnl   }
    dnl
    dnl Mac OS X up to at least 10.7.1 is known to have this problem, as is at
    dnl least one instance of GCC 4.2.4 (used on a "Linux ooobuild1.osuosl.org
    dnl 2.6.9-101.plus.c4smp #1 SMP Thu Jul 21 19:08:15 EDT 2011 i686 i686 i386
    dnl GNU/Linux" machine); see the definition of __cxa_guard_acquire in GCC's
    dnl libstdc++-v3/libsupc++/guard.cc for what #ifdefs actually make a
    dnl difference there.  Conservative advice from Jakub Jelinek is to assume
    dnl it working in GCC >= 4.3:
    if test "$_os" = "Darwin" -o $_os = Android -o "${GCCVER?}" -lt 040300; then
        AC_MSG_RESULT([broken (i.e., no)])
    else
        HAVE_THREADSAFE_STATICS=TRUE
        AC_MSG_RESULT([yes])
    fi
else
    AC_MSG_RESULT([unknown (assuming no)])
fi
AC_SUBST(HAVE_THREADSAFE_STATICS)

dnl ===================================================================
dnl visibility and c++0x features
dnl ===================================================================
if test "$GCC" = "yes"; then
    AC_MSG_CHECKING([whether $CC supports -fvisibility=hidden])
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -Werror -fvisibility=hidden"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[ return 0; ]])],[ HAVE_GCC_VISIBILITY_FEATURE=TRUE ],[])
    CFLAGS=$save_CFLAGS

    if test "$COM_GCC_IS_CLANG" = TRUE -a $_os = Darwin; then
        # It seems that with Apple's Clang, visibility doesn't work as
        # we would want at least in the connectivity and dbaccess
        # modules. This might be because of something peculiar in
        # those modules? Whatever.
        HAVE_GCC_VISIBILITY_FEATURE=
    fi

    if test "$HAVE_GCC_VISIBILITY_FEATURE" = "TRUE"; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
    fi

    AC_MSG_CHECKING([whether $CC supports -Wno-long-double])
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -Werror -Wno-long-double"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[ return 0; ]])],[ HAVE_GCC_NO_LONG_DOUBLE=TRUE ],[])
    CFLAGS=$save_CFLAGS
    if test "$HAVE_GCC_NO_LONG_DOUBLE" = "TRUE"; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
    fi

    AC_MSG_CHECKING([whether $CC supports -mno-avx])
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -Werror -mno-avx"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[ return 0; ]])],[ HAVE_GCC_AVX=TRUE ],[])
    CFLAGS=$save_CFLAGS
    if test "$HAVE_GCC_AVX" = "TRUE"; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
    fi

    AC_MSG_CHECKING([whether $CC supports atomic functions])
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[
    int v = 0;
    if (__sync_add_and_fetch(&v, 1) != 1 ||
        __sync_sub_and_fetch(&v, 1) != 0)
        return 1;
    __sync_synchronize();
    if (__sync_val_compare_and_swap(&v, 0, 1) != 0 ||
        v != 1)
        return 1;
    return 0;
]])],[HAVE_GCC_BUILTIN_ATOMIC=TRUE],[])
    if test "$HAVE_GCC_BUILTIN_ATOMIC" = "TRUE"; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
    fi

    AC_MSG_CHECKING([whether $CXX supports -std=gnu++0x without Language Defect 757])
    if test "$GCCVER" -ge 040700 -a "$GCCVER" -lt 040702; then
        AC_MSG_NOTICE([Not using -std=gnu++0x on $CXX version 4.7.0/4.7.1 due to libstdc++ ABI breakage.])
    else
        save_CXXFLAGS=$CXXFLAGS
        CXXFLAGS="$CXXFLAGS -std=gnu++0x"
        AC_LANG_PUSH([C++])

        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <stddef.h>

#include <vector>
    // some Clang fail when compiling against GCC 4.7 headers with --std=gnu++0x

template <typename T, size_t S> char (&sal_n_array_size( T(&)[S] ))[S];

namespace
{
        struct b
        {
                int i;
                int j;
        };
}
]], [[
struct a
{
        int i;
        int j;
};
a thinga[]={{0,0}, {1,1}};
b thingb[]={{0,0}, {1,1}};
size_t i = sizeof(sal_n_array_size(thinga));
size_t j = sizeof(sal_n_array_size(thingb));
return !(i != 0 && j != 0);
]])],[HAVE_CXX0X=TRUE],[])

        AC_LANG_POP([C++])
        CXXFLAGS=$save_CXXFLAGS
    fi
    if test "$HAVE_CXX0X" = "TRUE"; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
    fi
fi

AC_SUBST(HAVE_CXX0X)
AC_SUBST(HAVE_GCC_NO_LONG_DOUBLE)
AC_SUBST(HAVE_GCC_AVX)
AC_SUBST(HAVE_GCC_BUILTIN_ATOMIC)

dnl ===================================================================
dnl system stl sanity tests
dnl ===================================================================
HAVE_GCC_VISIBILITY_BROKEN=
if test \( "$_os" != "WINNT" -o "$WITH_MINGW" = "yes" \); then

    AC_LANG_PUSH([C++])

    save_CPPFLAGS="$CPPFLAGS"
    if test -n "$MACOSX_SDK_PATH"; then
        CPPFLAGS="-isysroot $MACOSX_SDK_PATH $CPPFLAGS"
    fi

    if test "$HAVE_GCC_VISIBILITY_FEATURE" = "TRUE"; then
        dnl gcc#19664, gcc#22482, rhbz#162935
        AC_MSG_CHECKING([if STL headers are visibility safe (GCC bug 22482)])
        AC_EGREP_HEADER(visibility push, string, stlvisok=yes, stlvisok=no)
        AC_MSG_RESULT([$stlvisok])
        if test "$stlvisok" = "no"; then
            AC_MSG_WARN([Your gcc STL headers are not visibility safe. Disabling visibility])
            add_warning "Your gcc STL headers are not visibility safe. Disabling visibility"
            unset HAVE_GCC_VISIBILITY_FEATURE
        fi
    fi

    if test "$HAVE_GCC_VISIBILITY_FEATURE" = "TRUE"; then
        sharedlink_ldflags_save=$LDFLAGS
        LDFLAGS="$LDFLAGS -fvisibility-inlines-hidden $PICSWITCH $LINKFLAGSSHL"

        AC_MSG_CHECKING([if gcc is -fvisibility-inlines-hidden safe with STL headers])
        AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <sstream>
using namespace std;
            ]], [[
istringstream strm( "test" ); return 0;
            ]])],
            # Ugh, surely bad to assume an error message will contain
            # the word "unresolvable", a problem with
            # -fvisibility-inlines-hidden and STL headers might cause
            # some more obscure message on some platform, and anway,
            # the error message could be localised.
            [$EGREP -q unresolvable conftest.err;
            if test $? -eq 0; then gccvisok=no; else gccvisok=yes; fi],[gccvisok=no
        ])
        AC_MSG_RESULT([$gccvisok])
        if test "$gccvisok" = "no"; then
            AC_MSG_WARN([Your gcc is not -fvisibility-inlines-hidden safe, disabling that.])
            add_warning "Your gcc is not -fvisibility-inlines-hidden safe, disabling that."
            HAVE_GCC_VISIBILITY_BROKEN="TRUE"
        fi

        LDFLAGS=$sharedlink_ldflags_save
    fi

    if test "$HAVE_GCC_VISIBILITY_FEATURE" = "TRUE" -a "$HAVE_GCC_VISIBILITY_BROKEN" != "TRUE"; then
        AC_MSG_CHECKING([if gcc is -fvisibility-inlines-hidden safe (Clang bug 11250)])
        cat > conftestlib1.cc <<_ACEOF
template<typename T> struct S1 { virtual ~S1() {} virtual void f() {} };
struct S2: S1<int> { virtual ~S2(); };
S2::~S2() {}
_ACEOF
        cat > conftestlib2.cc <<_ACEOF
template<typename T> struct S1 { virtual ~S1() {} virtual void f() {} };
struct S2: S1<int> { virtual ~S2(); };
struct S3: S2 { virtual ~S3(); }; S3::~S3() {}
_ACEOF
        gccvisinlineshiddenok=yes
        if ! $CXX $CXXFLAGS $CPPFLAGS $LINKFLAGSSHL -fPIC -fvisibility-inlines-hidden conftestlib1.cc -o libconftest1$DLLPOST >/dev/null 2>&5; then
            gccvisinlineshiddenok=no
        else
            if ! $CXX $CXXFLAGS $CPPFLAGS $LINKFLAGSSHL -fPIC -fvisibility-inlines-hidden conftestlib2.cc -L. -lconftest1 $LINKFLAGSNOUNDEFS -o libconftest2$DLLPOST >/dev/null 2>&5; then
                gccvisinlineshiddenok=no
            fi
        fi

        rm -fr libconftest*
        AC_MSG_RESULT([$gccvisinlineshiddenok])
        if test "$gccvisinlineshiddenok" = "no"; then
            AC_MSG_WARN([Your gcc/clang is not -fvisibility-inlines-hidden safe, disabling that.])
            add_warning "Your gcc/clang is not -fvisibility-inlines-hidden safe, disabling that."
            HAVE_GCC_VISIBILITY_BROKEN="TRUE"
        fi
    fi

    if test "$HAVE_GCC_VISIBILITY_FEATURE" = "TRUE"; then
        AC_MSG_CHECKING([if gcc has a visibility bug with class-level attributes (GCC bug 26905)])
        cat >visibility.cxx <<_ACEOF
#pragma GCC visibility push(hidden)
struct __attribute__ ((visibility ("default"))) TestStruct {
  static void Init();
};
__attribute__ ((visibility ("default"))) void TestFunc() {
  TestStruct::Init();
}
_ACEOF
        if ! $CXX $CXXFLAGS $CPPFLAGS -fpic -S visibility.cxx 2>/dev/null > /dev/null; then
            gccvisbroken=yes
        else
            case "$host_cpu" in
            i?86|x86_64)
                if test "$_os" = "Darwin"; then
                    gccvisbroken=no
                else
                    if $EGREP -q '@PLT|@GOT' visibility.s; then
                        gccvisbroken=no
                    else
                        gccvisbroken=yes
                    fi
                fi
                ;;
            *)
                gccvisbroken=no
                ;;
            esac
        fi
        rm -f visibility.s visibility.cxx

        AC_MSG_RESULT([$gccvisbroken])
        if test "$gccvisbroken" = "yes"; then
            AC_MSG_WARN([Your gcc is not -fvisibility=hidden safe. Disabling visibility])
            add_warning "Your gcc is not -fvisibility=hidden safe. Disabling visibility"
            unset HAVE_GCC_VISIBILITY_FEATURE
        fi
    fi

    CPPFLAGS="$save_CPPFLAGS"

    AC_LANG_POP([C++])
fi

AC_SUBST(HAVE_GCC_VISIBILITY_FEATURE)
AC_SUBST(HAVE_GCC_VISIBILITY_BROKEN)

dnl ===================================================================
dnl SFINAE test
dnl Pre-C++11 does not allow types without linkage as template arguments.
dnl Substitution Failure Is Not An Error is an idiom that disables
dnl template instances that would cause an error, without actually
dnl causing an error. Old gcc (pre-4.0.2) however causes a real error.
dnl http://gcc.gnu.org/bugzilla/show_bug.cgi?id=21514
dnl ===================================================================
HAVE_SFINAE_ANONYMOUS_BROKEN=
if test \( "$_os" != "WINNT" -o "$WITH_MINGW" = "yes" \); then

    AC_LANG_PUSH([C++])
    AC_MSG_CHECKING([if SFINAE is broken with anonymous types])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
enum { AnonymousEnumValue };
template< typename T > class TestPredicate {};
template<> class TestPredicate< int > { public: typedef bool Type; };
template< typename T >
bool test( const T&, typename TestPredicate< T >::Type = false )
    { return true; };
void test( ... );
            ]], [[
    test( 10 );
    test( AnonymousEnumValue );
            ]])],[sfinae_anonymous_broken=no],[sfinae_anonymous_broken=yes
        ])
        AC_MSG_RESULT([$sfinae_anonymous_broken])
        if test "$sfinae_anonymous_broken" = "yes"; then
            HAVE_SFINAE_ANONYMOUS_BROKEN="TRUE"
        fi
    AC_LANG_POP([C++])
fi

AC_SUBST(HAVE_SFINAE_ANONYMOUS_BROKEN)

dnl ===================================================================
dnl  Clang++ tests
dnl ===================================================================

HAVE_GCC_FNO_DEFAULT_INLINE=
HAVE_GCC_FNO_ENFORCE_EH_SPECS=
if test "$GCC" = "yes"; then
    AC_MSG_CHECKING([whether $CXX supports -fno-default-inline])
    if test -n "$CLANGVER" -a 0"$CLANGVER" -le 30100; then
        # Option just ignored and silly warning that isn't a real
        # warning printed
        :
    else
        AC_LANG_PUSH([C++])
        save_CXXFLAGS=$CXXFLAGS
        CXXFLAGS="$CFLAGS -Werror -fno-default-inline"
        AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[ return 0; ]])],[ HAVE_GCC_FNO_DEFAULT_INLINE=TRUE ],[])
        CXXFLAGS=$save_CXXFLAGS
        AC_LANG_POP([C++])
    fi
    if test "$HAVE_GCC_FNO_DEFAULT_INLINE" = "TRUE"; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
    fi

    AC_MSG_CHECKING([whether $CXX supports -fno-enforce-eh-specs])
    if test -n "$CLANGVER" -a 0"$CLANGVER" -le 30100; then
        # As above
        :
    else
        AC_LANG_PUSH([C++])
        save_CXXFLAGS=$CXXFLAGS
        CXXFLAGS="$CFLAGS -Werror -fno-enforce-eh-specs"
        AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[ return 0; ]])],[ HAVE_GCC_FNO_ENFORCE_EH_SPECS=TRUE ],[])
        CXXFLAGS=$save_CXXFLAGS
        AC_LANG_POP([C++])
    fi
    if test "$HAVE_GCC_FNO_ENFORCE_EH_SPECS" = "TRUE"; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_SUBST(HAVE_GCC_FNO_DEFAULT_INLINE)
AC_SUBST(HAVE_GCC_FNO_ENFORCE_EH_SPECS)


dnl ===================================================================
dnl allocator
dnl ===================================================================
AC_MSG_CHECKING([which memory allocator to use])
if test "$with_alloc" = "system"; then
    AC_MSG_RESULT([system])
    ALLOC="SYS_ALLOC"
    AC_CHECK_FUNCS([malloc realloc calloc free])
fi
if test "$with_alloc" = "tcmalloc"; then
    AC_MSG_RESULT(tcmalloc)
    if ! echo $host_cpu | $GREP -E 'i[[3456]]86' 2>/dev/null >/dev/null; then
        AC_MSG_ERROR([tcmalloc only available/usable on ix86])
    fi
    AC_CHECK_LIB([tcmalloc], [malloc], [:],
        [AC_MSG_ERROR(tcmalloc not found or functional. Install the Google Profiling Tools)], [])
    ALLOC="TCMALLOC"
fi
if test "$with_alloc" = "jemalloc"; then
    if test "$_os" != "FreeBSD" -o "$_os" != "NetBSD"; then
        AC_MSG_RESULT(jemalloc)
        save_CFLAGS=$CFLAGS
        CFLAGS="$CFLAGS -pthread"
        AC_CHECK_LIB([jemalloc], [malloc], [:],
            [AC_MSG_ERROR(jemalloc not found or functional. Install the jemalloc allocator.)], [])
        ALLOC="JEMALLOC"
        CFLAGS=$save_CFLAGS
    else
        AC_MSG_RESULT([system])
        ALLOC="SYS_ALLOC"
        AC_CHECK_FUNCS([malloc realloc calloc free])
    fi
fi
if test "$with_alloc" = "internal" -o -z "$with_alloc"; then
    AC_MSG_RESULT([internal])
fi
AC_CHECK_FUNCS(posix_fallocate, HAVE_POSIX_FALLOCATE=YES, [HAVE_POSIX_FALLOCATE=NO])
AC_SUBST(HAVE_POSIX_FALLOCATE)
AC_SUBST(ALLOC)

dnl ===================================================================
dnl Custom build version
dnl ===================================================================

AC_MSG_CHECKING([whether to add custom build version])
if test "$with_build_version" != ""; then
    BUILD_VER_STRING=$with_build_version
    AC_MSG_RESULT([yes, $BUILD_VER_STRING])
else
    BUILD_VER_STRING=
    AC_MSG_RESULT([no])
fi
AC_SUBST(BUILD_VER_STRING)

dnl ===================================================================
dnl Java support enable
dnl ===================================================================
AC_MSG_CHECKING([whether to build with Java support])
if test "$with_java" != "no"; then
    if test "$DISABLE_SCRIPTING" = TRUE; then
        AC_MSG_RESULT([no, overridden by --disable-scripting])
        SOLAR_JAVA=""
        with_java=no
    else
        AC_MSG_RESULT([yes])
        SOLAR_JAVA="TRUE"
    fi
else
    AC_MSG_RESULT([no])
    SOLAR_JAVA=""
fi

AC_SUBST(SOLAR_JAVA)

dnl SOLAR_JAVA="YES" (yes, silly name, should rename) indicates whether we
dnl want there to be *run-time* (and build-time) support for Java extensions in the
dnl built LibreOffice.

dnl SOLAR_JAVA="BUILD" is claimed to indicate build-time only support
dnl (no runtime support). It doesn't seem to ever be set to this
dnl value, though, and everywhere SOLAR_JAVA is only checked for being
dnl empty or non-empty.

dnl SOLAR_JAVA="" indicate no java support at all

JITC_PROCESSOR_TYPE=""
if test "$_os" = "Linux" -a "$host_cpu" = "powerpc"; then
    # IBMs JDK needs this...
    JITC_PROCESSOR_TYPE=6
    export JITC_PROCESSOR_TYPE
fi
AC_SUBST([JITC_PROCESSOR_TYPE])

if test $_os = "WINNT"; then
    AC_MSG_CHECKING([for Microsoft_VC"$VCVER"_CRT_x86.msm])
    if ./oowintool --msvc-find-msms; then
        AC_MSG_RESULT([yes])
        SCPDEFS="$SCPDEFS -DWITH_VC"$VCVER"_REDIST"
    else
        AC_MSG_RESULT([no])
    fi
fi

dnl ===================================================================
dnl Checks for Java
dnl ===================================================================
if test "$SOLAR_JAVA" != ""; then

    # Windows-specific tests
    if test "$build_os" = "cygwin" -a "$_os" = "WINNT"; then
        if test "$BITNESS_OVERRIDE" = 64; then
            bitness="64-bit"
            otherbitness="32-bit"
        else
            bitness="32-bit"
            otherbitness="64-bit"
        fi

        if test -z "$with_jdk_home"; then

            # Unfortunately apparently no way to find, if needed, the 64-bit
            # JDK in the Registry from the 32-bit Perl oowintool

            _jdk_home=`./oowintool --jdk-home`
            if test -f "$_jdk_home/lib/jvm.lib" -a -f "$_jdk_home/bin/java.exe"; then
                with_jdk_home="$_jdk_home"
                howfound="found by oowintool"
            else
                AC_MSG_ERROR([No JDK found by oowintool, pass the --with-jdk-home option pointing to a $bitness JDK])
            fi
        else
            test "$build_os" = "cygwin" && with_jdk_home=`cygpath -u "$with_jdk_home"`
            howfound="you passed"
        fi
    fi

    JAVA_HOME=; export JAVA_HOME
    if test -z "$with_jdk_home"; then
        AC_PATH_PROG(JAVAINTERPRETER, $with_java)
    else
        _java_path="$with_jdk_home/bin/$with_java"
        dnl Check if there is a Java interpreter at all.
        if test -x "$_java_path"; then
            JAVAINTERPRETER=$_java_path
        else
            AC_MSG_ERROR([$_java_path not found, pass --with-jdk-home])
        fi
    fi

    if test "$build_os" = "cygwin" -a "$_os" = "WINNT"; then
        # Check that the JDK found is correct architecture
        # Why is this necessary, we don't link with any library from the JDK I think,

        shortjdkhome=`cygpath -d "$with_jdk_home"`
        if test "$BITNESS_OVERRIDE" = 64 -a -f "$with_jdk_home/bin/java.exe" -a "`$shortjdkhome/bin/java.exe -version 2>&1 | $GREP -i 64-bit`" = "" >/dev/null; then
            AC_MSG_WARN([You are building 64-bit binaries but the JDK $howfound is 32-bit])
            AC_MSG_ERROR([You should pass the --with-jdk-home option pointing to a 64-bit JDK])
        elif test "$BITNESS_OVERRIDE" = "" -a -f "$_jdk_home/bin/java.exe" -a "`$shortjdkhome/bin/java.exe -version 2>&1 | $GREP -i 64-bit`" != ""  >/dev/null; then
            AC_MSG_WARN([You are building 32-bit binaries but the JDK $howfound is 64-bit])
            AC_MSG_ERROR([You should pass the --with-jdk-home option pointing to a (32-bit) JDK])
        fi

        if test x`echo "$JAVAINTERPRETER" | $GREP -i '\.exe$'` = x; then
            JAVAINTERPRETER="${JAVAINTERPRETER}.exe"
        fi
        JAVAINTERPRETER=`cygpath -d "$JAVAINTERPRETER"`
        JAVAINTERPRETER=`cygpath -u "$JAVAINTERPRETER"`
    elif test "$_os" = "Darwin"; then
        dnl HACK:  There currently is only a 32 bit version of LibreOffice for Mac OS X,
        dnl and Tiger Java complains about -d32 while Snow Leopard Java needs it
        dnl to run in 32 bit mode and be able to load LibreOffice jnilibs:
        AC_MSG_CHECKING([whether to pass -d32 to Java interpreter])
        if "$JAVAINTERPRETER" -d32 >&5 2>&5; then
            AC_MSG_RESULT([yes])
            JAVAIFLAGS=-d32
        else
            AC_MSG_RESULT([no])
        fi
    fi
fi

dnl ===================================================================
dnl Checks for JDK.
dnl ===================================================================

# Note that JAVA_HOME as for now always means the *build* platform's
# JAVA_HOME. Whether all the complexity here actually is needed any
# more or not, no idea.

if test "$SOLAR_JAVA" != ""; then
    _gij_longver=0
    AC_MSG_CHECKING([the installed JDK])
    if test -n "$JAVAINTERPRETER"; then
        dnl java -version sends output to stderr!
        if test `$JAVAINTERPRETER -version 2>&1 | $GREP -c "Kaffe"` -gt 0; then
            AC_MSG_ERROR([No valid check available. Please check the block for your desired java in configure.ac])
        elif test `$JAVAINTERPRETER --version 2>&1 | $GREP -c "GNU libgcj"` -gt 0; then
            JDK=gcj
            AC_MSG_RESULT([checked (gcj)])
            _gij_version=`$JAVAINTERPRETER --version | grep GNU | $SED -e 's@^[[^0-9]]*@@' -e 's@ .*@@' -e 's@,.*@@'`
            _gij_longver=`echo $_gij_version | $AWK -F. '{ print \$1*10000+\$2*100+\$3 }'`

        elif test `$JAVAINTERPRETER -version 2>&1 | $AWK '{ print }' | $GREP -c "BEA"` -gt 0; then
            AC_MSG_ERROR([No valid check available. Please check the block for your desired java in configure.ac])
        elif test `$JAVAINTERPRETER -version 2>&1 | $AWK '{ print }' | $GREP -c "IBM"` -gt 0; then
            JDK=ibm

            dnl IBM JDK specific tests
            _jdk=`$JAVAINTERPRETER -version 2>&1 | $AWK -F'"' '{ print \$2 }' | $SED s/[[-A-Za-z]]*//`
            _jdk_ver=`echo "$_jdk" | $AWK -F. '{ print (($1 * 100) + $2) * 100 + $3;}'`

            if test "$_jdk_ver" -lt 10500; then
                AC_MSG_ERROR([IBM JDK is too old, you need at least 1.5])
            fi

            AC_MSG_RESULT([checked (IBM JDK $_jdk)])

            if test "$with_jdk_home" = ""; then
                AC_MSG_ERROR([In order to successfully build LibreOffice using the IBM JDK,
you must use the "--with-jdk-home" configure option explicitly])
            fi

           JAVA_HOME=$with_jdk_home

        else
            JDK=sun

            dnl Sun JDK specific tests
            _jdk=`$JAVAINTERPRETER -version 2>&1 | $AWK -F'"' '{ print \$2 }' | $SED s/[[-A-Za-z]]*//`
            _jdk_ver=`echo "$_jdk" | $AWK -F. '{ print (($1 * 100) + $2) * 100 + $3;}'`

            if test "$_jdk_ver" -lt 10500; then
                AC_MSG_ERROR([JDK is too old, you need at least 1.5])
            fi
            AC_MSG_RESULT([checked (JDK $_jdk)])
            JAVA_HOME=`echo $JAVAINTERPRETER | $SED -n "s,//*bin//*java,,p"`
            if test "$_os" = "WINNT"; then
                JAVA_HOME=`echo $JAVA_HOME | $SED "s,\.[[eE]][[xX]][[eE]]$,,"`
            fi
        fi
    else
        AC_MSG_ERROR([Java not found. You need at least jdk-1.5, or gcj-4])
    fi
else
    dnl Java disabled
    JAVA_HOME=NO_JAVA_HOME ; export JAVA_HOME
fi

dnl ===================================================================
dnl Check for target Java bytecode version
dnl ===================================================================
if test "$SOLAR_JAVA" != ""; then
    AC_MSG_CHECKING([for target Java bytecode version])
    if test "$JDK" = "gcj" -o "$JDK" = "kaffe"; then
        _java_target_ver="1.5"
        AC_MSG_RESULT([default by $JDK])
        if test -n "$with_java_target_version" -a "$with_java_target_version" != "no"; then
            AC_MSG_WARN([Value defined by --with-java-target-version is ignored!])
        fi
    elif test -n "$with_java_target_version" -a "$with_java_target_version" != "no"; then
        _java_target_ver="$with_java_target_version"
        AC_MSG_RESULT([$_java_target_ver])
    elif test $_jdk_ver -gt 10000; then
        _java_target_ver=`echo "$_jdk_ver" | $AWK '{ maj=substr($0,1,1); min=substr($0,2,2); print int(maj)"."int(min) }'`
        AC_MSG_RESULT([$_java_target_ver])
    else
        AC_MSG_ERROR([Unable to guess Java bytecode version from Java version!])
    fi

    JAVA_SOURCE_VER="$_java_target_ver"
    JAVA_TARGET_VER="$_java_target_ver"
fi

dnl ===================================================================
dnl Checks for javac
dnl ===================================================================
if test "$SOLAR_JAVA" != ""; then
    if test "$JDK" = "gcj"; then
        javacompiler=`echo $with_java | $SED -e "s/gij/gcj/g" | $SED -e "s/java/javac/g"`
    else
        javacompiler="javac"
    fi
    if test -z "$with_jdk_home"; then
        AC_PATH_PROG(JAVACOMPILER, $javacompiler)
    else
        _javac_path="$with_jdk_home/bin/$javacompiler"
        dnl Check if there is a Java compiler at all.
        if test -x "$_javac_path"; then
            JAVACOMPILER=$_javac_path
        fi
    fi
    if test -z "$JAVACOMPILER"; then
        AC_MSG_ERROR([$javacompiler not found set with_jdk_home])
    fi
    if test "$build_os" = "cygwin" -a "$_os" = "WINNT"; then
        if test x`echo "$JAVACOMPILER" | $GREP -i '\.exe$'` = x; then
            JAVACOMPILER="${JAVACOMPILER}.exe"
        fi
        JAVACOMPILER=`cygpath -d "$JAVACOMPILER"`
        JAVACOMPILER=`cygpath -u "$JAVACOMPILER"`
    fi

    if test `$JAVACOMPILER -version 2>&1 | $GREP -c "Eclipse Java Compiler"` -gt 0; then
        AC_MSG_CHECKING([re-checking JDK])
        JDK=gcj
        AC_MSG_RESULT([checked (ecj)])
        _gij_longver="40200"
    fi
fi

JAVACISGCJ=""
dnl ===================================================================
dnl Checks that javac is gcj
dnl ===================================================================
if test "$SOLAR_JAVA" != ""; then
    if test `$JAVACOMPILER --version 2>&1 | $GREP -c "GCC"` -gt 0; then
        JAVACISGCJ="yes"
    fi
fi
AC_SUBST(JAVACISGCJ)

JAVACISKAFFE=""
dnl ===================================================================
dnl Checks that javac is kaffe
dnl ===================================================================
if test "$SOLAR_JAVA" != ""; then
    if test `$JAVACOMPILER -version 2>&1 | $GREP -c "Kaffe"` -gt 0; then
        JAVACISKAFFE="yes"
    fi
fi
AC_SUBST(JAVACISKAFFE)

dnl ===================================================================
dnl Checks for javadoc
dnl ===================================================================
if test "$SOLAR_JAVA" != ""; then
    if test -z "$with_jdk_home"; then
        AC_PATH_PROG(JAVADOC, javadoc)
    else
        _javadoc_path="$with_jdk_home/bin/javadoc"
        dnl Check if there is a javadoc at all.
        if test -x "$_javadoc_path"; then
            JAVADOC=$_javadoc_path
        else
            AC_PATH_PROG(JAVADOC, javadoc)
        fi
    fi
    if test -z "$JAVADOC"; then
        AC_MSG_ERROR([$_javadoc_path not found set with_jdk_home])
    fi
    if test "$build_os" = "cygwin" -a "$_os" = "WINNT"; then
        if test x`echo "$JAVADOC" | $GREP -i '\.exe$'` = x; then
            JAVADOC="${JAVADOC}.exe"
        fi
        JAVADOC=`cygpath -d "$JAVADOC"`
        JAVADOC=`cygpath -u "$JAVADOC"`
    fi
fi

if test "$SOLAR_JAVA" != ""; then
    # check if JAVA_HOME was (maybe incorrectly?) set automatically to /usr
    if test "$JAVA_HOME" = "/usr" -a "x$with_jdk_home" = "x"; then
        if basename $(readlink $(readlink $JAVACOMPILER)) >/dev/null 2>/dev/null; then
           # try to recover first by looking whether we have a alternatives
           # system as in Debian or newer SuSEs where following /usr/bin/javac
           # over /etc/alternatives/javac leads to the right bindir where we
           # just need to strip a bit away to get a valid JAVA_HOME
           JAVA_HOME=$(readlink $(readlink $JAVACOMPILER))
        elif readlink $JAVACOMPILER >/dev/null 2>/dev/null; then
            # maybe only one level of symlink (e.g. on Mac)
            JAVA_HOME=$(readlink $JAVACOMPILER)
            if test "$(dirname $JAVA_HOME)" = "."; then
                # we've got no path to trim back
                JAVA_HOME=""
            fi
        else
            # else warn
            AC_MSG_WARN([JAVA_HOME is set to /usr - this is very likely to be incorrect])
            AC_MSG_WARN([if this is the case, please inform the correct JAVA_HOME with --with-jdk-home])
            add_warning "JAVA_HOME is set to /usr - this is very likely to be incorrect"
            add_warning "if this is the case, please inform the correct JAVA_HOME with --with-jdk-home"
        fi
        dnl now that we have the path to the real javac, make a JAVA_HOME out of it..
        if test "$JAVA_HOME" != "/usr"; then
            if test "$_os" = "Darwin" -o "$OS_FOR_BUILD" = MACOSX; then
                dnl Leopard returns a non-suitable path with readlink - points to "Current" only
                JAVA_HOME=$(echo $JAVA_HOME | $SED -e s,/Current/Commands/javac$,/CurrentJDK/Home,)
                dnl Tiger already returns a JDK path..
                JAVA_HOME=$(echo $JAVA_HOME | $SED -e s,/CurrentJDK/Commands/javac$,/CurrentJDK/Home,)
            else
                JAVA_HOME=$(echo $JAVA_HOME | $SED -e s,/bin/javac$,,)
            fi
        fi
    fi
    # as we drop out of this, JAVA_HOME may have been set to the empty string by readlink

    dnl now if JAVA_HOME has been set to empty, then call findhome to find it
    if test -z "$JAVA_HOME"; then
        if test "x$with_jdk_home" = "x"; then
            cat > findhome.java <<_ACEOF
[import java.io.File;

class findhome
{
    public static void main(String args[])
    {
        String jrelocation = System.getProperty("java.home");
        File jre = new File(jrelocation);
        System.out.println(jre.getParent());
    }
}]
_ACEOF
            AC_MSG_CHECKING([if javac works])
            javac_cmd="$JAVACOMPILER findhome.java 1>&2"
            AC_TRY_EVAL(javac_cmd)
            if test $? = 0 -a -f ./findhome.class; then
                AC_MSG_RESULT([javac works])
            else
                echo "configure: javac test failed" >&5
                cat findhome.java >&5
                AC_MSG_ERROR([javac does not work - java projects will not build!])
            fi
            AC_MSG_CHECKING([if gij knows its java.home])
            JAVA_HOME=`$JAVAINTERPRETER findhome`
            if test $? = 0 -a "$JAVA_HOME" != ""; then
                AC_MSG_RESULT([$JAVA_HOME])
            else
                echo "configure: java test failed" >&5
                cat findhome.java >&5
                AC_MSG_ERROR([gij does not know its java.home - use --with-jdk-home])
            fi
            # clean-up after ourselves
            rm -f ./findhome.java ./findhome.class
        else
            JAVA_HOME=`echo $JAVAINTERPRETER | $SED -n "s,//*bin//*$with_java,,p"`
        fi
    fi

    dnl second sanity check JAVA_HOME if possible
    if test "$JDK" != "gcj" -o "$_gij_longver" -ge "40200"; then
        # now check if $JAVA_HOME is really valid
        if test "$_os" = "Darwin" -o "$OS_FOR_BUILD" = MACOSX; then
            if test ! -f "$JAVA_HOME/lib/jvm.cfg" -a "x$with_jdk_home" = "x"; then
                JAVA_HOME_OK="NO"
            fi
        elif test ! -d "$JAVA_HOME/jre" -a "x$with_jdk_home" = "x"; then
            JAVA_HOME_OK="NO"
        fi
        if test "$JAVA_HOME_OK" = "NO"; then
            AC_MSG_WARN([JAVA_HOME was not explicitly informed with --with-jdk-home. the configure script])
            AC_MSG_WARN([attempted to find JAVA_HOME automatically, but apparently it failed])
            AC_MSG_WARN([in case JAVA_HOME is incorrectly set, some projects with not be built correctly])
            add_warning "JAVA_HOME was not explicitly informed with --with-jdk-home. the configure script"
            add_warning "attempted to find JAVA_HOME automatically, but apparently it failed"
            add_warning "in case JAVA_HOME is incorrectly set, some projects with not be built correctly"
        fi
    fi
    PathFormat "$JAVA_HOME"
    JAVA_HOME="$formatted_path"
fi

AWTLIB=

if test "$SOLAR_JAVA" != ""; then
    AC_MSG_CHECKING([for jawt lib name])
    if test "$JDK" = "gcj"; then
        save_CFLAGS=$CFLAGS
        save_LDFLAGS=$LDFLAGS
        CFLAGS="$CFLAGS -I$JAVA_HOME/include"
        LDFLAGS="$LDFLAGS -L$JAVA_HOME/lib -lgcj"
        exec 6>/dev/null # no output
        AC_CHECK_HEADER(jni.h, [],
                    [AC_MSG_ERROR([jni.h could not be found. Mismatch between gcc and libgcj or libgcj-devel missing?])], [])
        AC_CHECK_LIB(gcjawt, JAWT_GetAWT, [ AWTLIB="-lgcjawt -lgcj"] )
        exec 6>&1 # output on again
        CFLAGS=$save_CFLAGS
        LDFLAGS=$save_LDFLAGS
    fi
    # IBM SDK 1.5.0-sr5 includes libjawt.so with unresolved symbols.
    # A workaround is to link also against libpmawt.so
    if test "$JDK" = "ibm"; then
        save_CFLAGS=$CFLAGS
        save_LDFLAGS=$LDFLAGS
        save_LD_LIBRARY_PATH=$LD_LIBRARY_PATH
        CFLAGS="$CFLAGS -I$JAVA_HOME/include"
        LDFLAGS="$LDFLAGS -L$JAVA_HOME/jre/bin"
        LD_LIBRARY_PATH=$JAVA_HOME/jre/bin:$JAVA_HOME/jre/bin/classic:$JAVA_HOME/jre/bin/xawt:$LD_LIBRARY_PATH
        export LD_LIBRARY_PATH
        exec 6>/dev/null # no output
        AC_CHECK_HEADER(jni.h, [],
                    [AC_MSG_ERROR([jni.h could not be found.])], [])
        AC_CHECK_LIB(jawt, JAWT_GetAWT, [ AWTLIB="-ljawt"] )
        if test -z "$AWTLIB"; then
            LDFLAGS="$LDFLAGS -L$JAVA_HOME/jre/bin/xawt -ljawt"
            AC_CHECK_LIB(mawt, JAWT_GetAWT, [ AWTLIB="-L$JAVA_HOME/jre/bin/xawt -ljawt -lmawt"])
        fi
        exec 6>&1 # output on again
        CFLAGS=$save_CFLAGS
        LDFLAGS=$save_LDFLAGS
        LD_LIBRARY_PATH=$save_LD_LIBRARY_PATH
    fi
    if test -z "$AWTLIB"; then
        if test $_os = WINNT -a "$WITH_MINGW" != yes; then
            AWTLIB=jawt.lib
        else
            AWTLIB=-ljawt
        fi
    fi
    AC_MSG_RESULT([$AWTLIB])
    AC_SUBST(AWTLIB)
fi


if test "$SOLAR_JAVA" != ""; then
#
# Determine JAVALIB
# If your platform follow the common pattern
# just define JAVA_ARCH and JAVA_TOOLKIT for it
# if not, leave JAVA_ARCH empty and define JAVALIB manually
# if you don't want JAVALIB to be exported at all
# define x_JAVALIB=[\#]
#
    JAVA_ARCH=
    JAVA_TOOLKIT=
    x_JAVALIB=
    x_JDKLIB=[\#]

    case "$host_os" in

    aix*)
        JAVA_ARCH="ppc"
        JAVA_TOOLKIT="classic"
        SOLARINC="$SOLARINC -I$JAVA_HOME/include"
        SOLARINC="$SOLARINC -I$JAVA_HOME/include/aix"
        SOLARINC="$SOLARINC -I$JAVA_HOME/include/native_threads/include"
        ;;

    cygwin*)
        x_JDKLIB=
        JDKLIB="$JAVA_HOME/lib"
        SOLARINC="$SOLARINC -I$JAVA_HOME/include/win32"
        SOLARINC="$SOLARINC -I$JAVA_HOME/include"
        ;;

    darwin*)
        x_JAVALIB=[\#]
        SOLARINC="$SOLARINC -I$FRAMEWORKSHOME/JavaVM.framework/Versions/Current/Headers -I$FRAMEWORKSHOME/JavaVM/Headers"
        ;;

    dragonfly*)
        case "$host_cpu" in
        i*86)
            JAVA_ARCH="i386"
            JAVA_TOOLKIT="client"
            ;;
        x86_64)
            JAVA_ARCH="amd64"
            JAVA_TOOLKIT="server"
            ;;
        esac
        SOLARINC="$SOLARINC -I$JAVA_HOME/include"
        SOLARINC="$SOLARINC -I$JAVA_HOME/include/native_threads/include"
        ;;

    freebsd*)
        case "$host_cpu" in
        i*86)
            JAVA_ARCH="i386"
            JAVA_TOOLKIT="client"
            ;;
        x86_64)
            if test "`echo $JAVA_HOME | sed -e 's,.*/,,'`" = "linux"; then
                JAVA_ARCH="i386"
                JAVA_TOOLKIT="client"
            else
                JAVA_ARCH="amd64"
                JAVA_TOOLKIT="server"
            fi
            ;;
        esac
        SOLARINC="$SOLARINC -I$JAVA_HOME/include"
        SOLARINC="$SOLARINC -I$JAVA_HOME/include/freebsd"
        SOLARINC="$SOLARINC -I$JAVA_HOME/include/bsd"
        SOLARINC="$SOLARINC -I$JAVA_HOME/include/linux"
        SOLARINC="$SOLARINC -I$JAVA_HOME/include/native_threads/include"
        ;;

    k*bsd*-gnu*)
        case "$host_cpu" in
        i*86)
            JAVA_ARCH="i386"
            JAVA_TOOLKIT="client"
            ;;
        x86_64)
            JAVA_ARCH="amd64"
            JAVA_TOOLKIT="server"
            ;;
        esac
        SOLARINC="$SOLARINC -I$JAVA_HOME/include"
        SOLARINC="$SOLARINC -I$JAVA_HOME/include/native_threads/include"
        ;;

    linux-gnu*)

        case "$host_cpu" in

        alpha)
            JAVA_ARCH="alpha"
            JAVA_TOOLKIT="server"
            ;;
        arm*)
            JAVA_ARCH="arm"
            JAVA_TOOLKIT="server"
            ;;
        hppa)
            JAVA_ARCH="hppa"
            JAVA_TOOLKIT="server"
            ;;
        i*86)
            JAVA_ARCH="i386"
            case "$JDK" in
            [Ii][Bb][Mm]*)
                JAVA_ARCH=
                JRE_BASE_DIR="$JAVA_HOME/jre/bin"
                JAVALIB="-L$JAVA_HOME/$LIB64 -L$JRE_BASE_DIR -L$JRE_BASE_DIR/classic -L$JRE_BASE_DIR/native_thread"
                ;;
            [Bb][Ee][Aa]*)
                JAVA_TOOLKIT="jrockit"
                ;;
            *)
                JAVA_TOOLKIT="client"
                ;;
            esac
            ;;
        ia64)
            JAVA_ARCH="ia64"
            JAVA_TOOLKIT="server"
            ;;
        m68k)
            JAVA_ARCH="mk68"
            JAVA_TOOLKIT="server"
            ;;
        mips|mips64|mipsel|mips64el)
            JAVA_ARCH="$host_cpu"
            JAVA_TOOLKIT="server"
            ;;
        mips32)
            JAVA_ARCH="$host_cpu"
            JAVA_TOOLKIT="client"
            ;;
        powerpc)
            JAVA_ARCH="ppc"
            case "$JDK" in
            [Ii][Bb][Mm]*)
                JAVA_TOOLKIT="classic"
                ;;
            gcj)
                JAVA_TOOLKIT="client"
                ;;
            *)
                JAVA_TOOLKIT="server"
                ;;
            esac
            ;;
        powerpc64)
            JAVA_ARCH="ppc64"
            if test "$JDK" = "gcj"; then
                JAVA_TOOLKIT="client"
            else
                JAVA_TOOLKIT="server"
            fi
            ;;
        sparc)
            JAVA_ARCH="sparc"
            JAVA_TOOLKIT="server"
            ;;
        s390)
            JAVA_ARCH="s390"
            JAVA_TOOLKIT="server"
            ;;
        s390x)
            JAVA_ARCH="s390x"
            JAVA_TOOLKIT="server"
            ;;
        x86_64)
            JAVA_ARCH="amd64"
            JAVA_TOOLKIT="server"
            ;;
        *)
            AC_MSG_ERROR([No JAVALIB definition host_cpu $host_cpu for host_os $host_os])
            ;;
        esac
        SOLARINC="$SOLARINC -I$JAVA_HOME/include"
        SOLARINC="$SOLARINC -I$JAVA_HOME/include/linux"
        SOLARINC="$SOLARINC -I$JAVA_HOME/include/native_threads/include"
        ;;

    linux-androideabi*)

        case "$host_cpu" in

    arm|armel)
            JAVA_ARCH="arm"
            JAVA_TOOLKIT="server"
            ;;
        esac
        ;;

    mingw*)
        x_JAVALIB=[\#]
        SOLARINC="$SOLARINC -I$JAVA_HOME/include"
        ;;

    *netbsd*)

        case "$host_cpu" in
        i*86)
            JAVA_ARCH="i386"
            JAVA_TOOLKIT="client"
            ;;
        powerpc)
            JAVA_ARCH="pcc"
            JAVA_TOOLKIT="classic"
            ;;
        sparc)
            JAVA_ARCH="sparc"
            JAVA_TOOLKIT="client"
            ;;
        x86_64)
            JAVA_ARCH="amd64"
            JAVA_TOOLKIT="client"
            ;;
        esac
        SOLARINC="$SOLARINC -I$JAVA_HOME/include"
        SOLARINC="$SOLARINC -I$JAVA_HOME/include/netbsd"
        SOLARINC="$SOLARINC -I$JAVA_HOME/include/native_threads/include"
       ;;

    openbsd*)
        case "$host_cpu" in
        i*86)
            JAVA_ARCH="i386"
            JAVA_TOOLKIT="client"
            ;;
        x86_64)
            JAVA_ARCH="amd64"
            JAVA_TOOLKIT="server"
            ;;
        esac
        SOLARINC="$SOLARINC -I$JAVA_HOME/include"
        SOLARINC="$SOLARINC -I$JAVA_HOME/include/openbsd"
        SOLARINC="$SOLARINC -I$JAVA_HOME/include/native_threads/include"
        ;;

    solaris*)

        JAVA_TOOLKIT="xawt"
        case "$host_cpu" in
        i*86)
            JAVA_ARCH="i386"
            ;;
        sparc)
            JAVA_ARCH="sparc"
            ;;
        esac
        SOLARINC="$SOLARINC -I$JAVA_HOME/include"
        SOLARINC="$SOLARINC -I$JAVA_HOME/include/solaris"
        SOLARINC="$SOLARINC -I$JAVA_HOME/include/native_threads/include"
        ;;
    esac

    if test -n "$JAVA_ARCH"; then
        JRE_BASE_DIR="$JAVA_HOME/jre/lib/$JAVA_ARCH"
        JAVALIB="-L$JAVA_HOME/$LIB64"
        test -d "$JRE_BASE_DIR" && JAVALIB="$JAVALIB -L$JRE_BASE_DIR"
        test -d "$JRE_BASE_DIR/$JAVA_TOOLKIT" && JAVALIB="$JAVALIB -L$JRE_BASE_DIR/$JAVA_TOOLKIT"
        test -d "$JRE_BASE_DIR/native_thread" && JAVALIB="$JAVALIB -L$JRE_BASE_DIR/native_thread"
    fi
else
    x_JAVALIB=[\#]
    x_JDKLIB=[\#]
fi

AC_SUBST(JAVALIB)
AC_SUBST(JAVACOMPILER)
AC_SUBST(JAVADOC)
AC_SUBST(JAVAINTERPRETER)
AC_SUBST(JAVAIFLAGS)
AC_SUBST(JAVA_HOME)
AC_SUBST(JAVA_SOURCE_VER)
AC_SUBST(JAVA_TARGET_VER)
AC_SUBST(JDK)
AC_SUBST(JDKLIB)
AC_SUBST(x_JAVALIB)
AC_SUBST(x_JDKLIB)

dnl ===================================================================
dnl Checks for specific files.
dnl ===================================================================

dnl ===================================================================
dnl Checks for programs.
dnl ===================================================================

dnl ===================================================================
dnl Check whether we already have dmake
dnl ===================================================================
AC_PATH_PROG(DMAKE, dmake, no)
if test "$DMAKE" = "no"; then
    BUILD_DMAKE=YES
    echo "dmake will be built on ./bootstrap"
else
    AC_MSG_CHECKING([whether the found dmake is the right dmake])
    # we need to find out whether that dmake we found is "our" dmake
    # or the dmake from Sun's SunStudio Compiler which is something
    # different
    # This test _should_ work because the one accepts -V (ours) and one
    # (the other) not...
    $DMAKE -V 2>/dev/null | grep 'dmake .* Version .*' >/dev/null
    if test $? -eq 0; then
        BUILD_DMAKE=NO
        AC_MSG_RESULT([yes])
        AC_MSG_CHECKING([the dmake version])
        DMAKE_VERSION=`$DMAKE -V | $AWK '$3 == "Version" {print $4}'`
        if test "`echo $DMAKE_VERSION | cut -d'.' -f1`" -gt "4"; then
            AC_MSG_RESULT([OK, >= 4.11])
        elif test "`echo $DMAKE_VERSION | cut -d'.' -f1`" = "4" && \
            test "`echo $DMAKE_VERSION | cut -d'.' -f2`" -ge "11"; then
            AC_MSG_RESULT([OK, >= 4.11])
        else
            AC_MSG_RESULT([too old. >= 4.11 is needed])
            echo "A newer dmake will be built on ./bootstrap"
            BUILD_DMAKE=YES
        fi
    else
        AC_MSG_RESULT([no])
        echo "dmake will be built on ./bootstrap"
        BUILD_DMAKE=YES
    fi
fi
AC_SUBST(BUILD_DMAKE)

AC_MSG_CHECKING([which C preprocessor to use in idlc])
if test -n "$with_idlc_cpp"; then
    AC_MSG_RESULT([$with_idlc_cpp])
    AC_PATH_PROG(SYSTEM_UCPP, $with_idlc_cpp)
else
    AC_MSG_RESULT([ucpp])
    AC_MSG_CHECKING([which ucpp tp use])
    if test -n "$with_system_ucpp" -a "$with_system_ucpp" != "no"; then
        AC_MSG_RESULT([external])
        AC_PATH_PROG(SYSTEM_UCPP, ucpp)
    else
        AC_MSG_RESULT([internal])
        BUILD_TYPE="$BUILD_TYPE UCPP"
    fi
fi
AC_SUBST(SYSTEM_UCPP)

dnl ===================================================================
dnl Check for epm (not needed for Windows)
dnl ===================================================================
AC_MSG_CHECKING([whether to enable EPM for packing])
if test "$enable_epm" = "yes"; then
    AC_MSG_RESULT([yes])
    if test "$_os" != "WINNT"; then
        if test -n "$with_epm"; then
            EPM=$with_epm
        else
            AC_PATH_PROG(EPM, epm, no)
        fi
        if test "$EPM" = "no" -o "$EPM" = "internal"; then
            echo "EPM will be built."
            EPM_TARBALL="3ade8cfe7e59ca8e65052644fed9fca4-epm-3.7.tar.gz"
            BUILD_TYPE="$BUILD_TYPE EPM"
        else
            # Gentoo has some epm which is something different...
            AC_MSG_CHECKING([whether the found epm is the right epm])
            if $EPM | grep "ESP Package Manager" >/dev/null 2>/dev/null; then
                AC_MSG_RESULT([yes])
            else
                AC_MSG_ERROR([no. Install ESP Package Manager (www.easysw.com/epm) and/or specify the path to the right epm])
            fi
            AC_MSG_CHECKING([epm version])
            EPM_VERSION=`$EPM | grep 'ESP Package Manager' | cut -d' ' -f4 | $SED -e s/v//`
            if test "`echo $EPM_VERSION | cut -d'.' -f1`" -gt "3" || \
               test "`echo $EPM_VERSION | cut -d'.' -f1`" -eq "3" -a "`echo $EPM_VERSION | cut -d'.' -f2`" -ge "7"; then
                AC_MSG_RESULT([OK, >= 3.7])
                if test "$_os" = "Darwin"; then
                    AC_MSG_CHECKING([which PackageMaker EPM thinks to use])
                    _pm=`strings $EPM | grep PackageMaker | cut -d" " -f1`
                    if test "$_pm" = "/Developer/Applications/PackageMaker.app/Contents/MacOS/PackageMaker"; then
                        AC_MSG_ERROR([$_pm; PackageMaker expected in wrong path. Either patch your epm with the right path (/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker) or use internal patched epm (--with-epm=internal)])
                    elif test "$_pm" = "/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker"; then
                        AC_MSG_RESULT([$_pm, ok])
                    else # we never should get here, but go safe
                        AC_MSG_ERROR([$_pm; PackageMaker expected in unknown path. Either patch your epm with the right path (/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker) or use internal patched epm (--with-epm=internal)])
                    fi
                fi
            else
                AC_MSG_RESULT([too old. epm >= 3.7 is required.])
                echo "EPM will be built."
                EPM_TARBALL="3ade8cfe7e59ca8e65052644fed9fca4-epm-3.7.tar.gz"
                BUILD_TYPE="$BUILD_TYPE EPM"
            fi
        fi
    fi

    if echo "$PKGFORMAT" | $EGREP rpm 2>&1 >/dev/null; then
        AC_MSG_CHECKING([for rpm])
        for a in "$RPM" rpmbuild rpm; do
            $a --usage >/dev/null 2> /dev/null
            if test $? -eq 0; then
                RPM=$a
                break
            else
                $a --version >/dev/null 2> /dev/null
                if test $? -eq 0; then
                    RPM=$a
                    break
                fi
            fi
        done
        if test -z "$RPM"; then
            AC_MSG_ERROR([not found])
        elif "$RPM" --help 2>&1 | $EGREP buildroot >/dev/null; then
            RPM_PATH=`which $RPM`
            AC_MSG_RESULT([$RPM_PATH])
        else
            AC_MSG_ERROR([cannot build packages. Try installing rpmbuild.])
        fi
    fi
    if echo "$PKGFORMAT" | $EGREP deb 2>&1 >/dev/null; then
        AC_PATH_PROG(DPKG, dpkg, no)
        if test "$DPKG" = "no"; then
            AC_MSG_ERROR([dpkg needed for deb creation. Install dpkg.])
        fi
    fi
    if echo "PKGFORMAT" | $EGREP osx 2>&1 >/dev/null; then
        if test "$_os" = "Darwin"; then
            AC_MSG_CHECKING([for PackageMaker availability])
            if ! test -x /Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker; then
                AC_MSG_ERROR([not installed. Please install Apples Dev Tools])
            else
                AC_MSG_RESULT([ok])
            fi
        else
            AC_MSG_ERROR([PackageMaker needed to build OSX packages and you are not on OSX...])
        fi
    fi
    if echo "$PKGFORMAT" | $EGREP rpm 2>&1 >/dev/null || \
       echo "$PKGFORMAT" | $EGREP pkg 2>&1 >/dev/null; then
        if test "$EPM" != "no" -a "$EPM" != "internal"; then
            if test "`echo $EPM_VERSION | cut -d'.' -f1`" -lt "4"; then
                AC_MSG_CHECKING([whether epm is patched for LibreOffice's needs])
                if grep "Patched for LibreOffice" $EPM >/dev/null 2>/dev/null; then
                    AC_MSG_RESULT([yes])
                else
                    AC_MSG_RESULT([no])
                    if echo "$PKGFORMAT" | $GREP -q rpm; then
                        _pt="rpm"
                        AC_MSG_WARN([the rpms will need to be installed with --nodeps])
                        add_warning "the rpms will need to be installed with --nodeps"
                    else
                        _pt="pkg"
                    fi
                    AC_MSG_WARN([the ${_pt}s will not be relocateable])
                    add_warning "the ${_pt}s will not be relocateable"
                    AC_MSG_WARN([if you want to make sure installation without --nodeps and
                                 relocation will work, you need to patch your epm with the
                                 patch in epm/epm-3.7.patch or build with
                                 --with-epm=internal which will build a suitable epm])
                fi
            fi
        fi
    fi
    if echo "$PKGFORMAT" | $EGREP pkg 2>&1 >/dev/null; then
        AC_PATH_PROG(PKGMK, pkgmk, no)
        if test "$PKGMK" = "no"; then
            AC_MSG_ERROR([pkgmk needed for Solaris pkg creation. Install it.])
        fi
    fi
    AC_SUBST(RPM)
    AC_SUBST(DPKG)
    AC_SUBST(PKGMK)
else
    AC_MSG_RESULT([no])
    EPM=NO
fi
AC_SUBST(EPM)
AC_SUBST(EPM_TARBALL)

dnl ===================================================================
dnl We need winegcc when building MinGW build to be able to cross-build msi tools
dnl ===================================================================
WINEGCC=
if test "$enable_winegcc" = "yes" ; then
    AC_PATH_PROG(WINEGCC, winegcc)
    if test "$WINEGCC" = ""; then
        AC_MSG_ERROR([winegcc was not found, please install wine-devel, or wine-devel-32bit])
    fi
    CC_save="$CC"
    ac_exeext_save="$ac_exeext"
    CC="$WINEGCC -m32"
    ac_exeext=".exe"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([
#include <stdio.h>
        ],[
printf ("hello world\n");
        ])],,
        [AC_MSG_ERROR([winegcc cannot build 32bit binaries, please install glibc-devel-32bit and gcc-32bit])]
    )
    CC="$CC_save"
    ac_exeext="$ac_exeext_save"
fi
AC_SUBST(WINEGCC)

dnl ===================================================================
dnl Check for gperf
dnl ===================================================================
AC_PATH_PROG(GPERF, gperf)
if test -z "$GPERF"; then
    AC_MSG_ERROR([gperf not found but needed. Install it.])
fi
AC_MSG_CHECKING([gperf version])
if test "`$GPERF --version | $EGREP ^GNU\ gperf | $AWK '{ print $3 }' | cut -d. -f1`" -ge "3"; then
    AC_MSG_RESULT([OK])
else
    AC_MSG_ERROR([too old, you need at least 3.0.0])
fi
AC_SUBST(GPERF)

dnl ===================================================================
dnl Check for building ODK
dnl ===================================================================
AC_MSG_CHECKING([whether to build the ODK])
if test "$enable_odk" = "" -o "$enable_odk" != "no"; then
    AC_MSG_RESULT([yes])

    if test "$with_java" != "no"; then
        AC_MSG_CHECKING([whether to build unowinreg.dll])
        if test "$_os" = "WINNT" -a "$enable_build_unowinreg" = ""; then
            # build on Win by default
            enable_build_unowinreg=yes
        fi
        if test "$enable_build_unowinreg" = "" -o "$enable_build_unowinreg" = "no"; then
            AC_MSG_RESULT([no])
            BUILD_UNOWINREG=NO
        else
            AC_MSG_RESULT([yes])
            BUILD_UNOWINREG=YES
        fi
        if test "$_os" != "WINNT" -a "$BUILD_UNOWINREG" = "YES"; then
            if test -z "$with_mingw_cross_compiler"; then
                dnl Guess...
                AC_CHECK_PROGS(MINGWCXX,i386-mingw32msvc-g++ i586-pc-mingw32-g++ i686-pc-mingw32-g++ i686-w64-mingw32-g++,false)
            elif test -x "$with_mingw_cross_compiler"; then
                 MINGWCXX="$with_mingw_cross_compiler"
            else
                AC_CHECK_TOOL(MINGWCXX, "$with_mingw_cross_compiler", false)
            fi

            if test "$MINGWCXX" = "false"; then
                AC_MSG_ERROR([MinGW32 C++ cross-compiler not found.])
            fi

            mingwstrip_test="`echo $MINGWCXX | $SED -e s/g++/strip/`"
            if test -x "$mingwstrip_test"; then
                MINGWSTRIP="$mingwstrip_test"
            else
                AC_CHECK_TOOL(MINGWSTRIP, "$mingwstrip_test", false)
            fi

            if test "$MINGWSTRIP" = "false"; then
                AC_MSG_ERROR(MinGW32 binutils not found.)
            fi
        fi
    fi
    BUILD_TYPE="$BUILD_TYPE ODK"
else
    AC_MSG_RESULT([no])
    BUILD_UNOWINREG=NO
fi
AC_SUBST(BUILD_UNOWINREG)
AC_SUBST(MINGWCXX)
AC_SUBST(MINGWSTRIP)

dnl ===================================================================
dnl Check for system stdlibs
dnl ===================================================================
AC_MSG_CHECKING([whether to provide libstdc++/libgcc_s in the installset])
if test "$with_system_stdlibs" != "no"; then
    AC_MSG_RESULT([no])
    SYSTEM_STDLIBS=YES
else
    AC_MSG_RESULT([yes])
    SYSTEM_STDLIBS=NO
fi
AC_SUBST(SYSTEM_STDLIBS)

dnl ===================================================================
dnl Check for system zlib
dnl ===================================================================
if test "$with_system_zlib" = "auto"; then
    case "$_os" in
    WINNT)
        with_system_zlib="$with_system_libs"
        ;;
    *)
        with_system_zlib=yes
        ;;
    esac
fi

AC_MSG_CHECKING([which zlib to use])
if test "$with_system_zlib" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_ZLIB=YES
    AC_CHECK_HEADER(zlib.h, [],
        [AC_MSG_ERROR(zlib.h not found. install zlib)], [])
    AC_CHECK_LIB(z, deflate, [ ZLIB=-lz ],
        [AC_MSG_ERROR(zlib not found or functional)], [])
else
    AC_MSG_RESULT([internal])
    SYSTEM_ZLIB=NO
    BUILD_TYPE="$BUILD_TYPE ZLIB"
    ZLIB_CFLAGS="-I${OUTDIR}/inc/external/zlib"
    ZLIB_LIBS="-L${OUTDIR}/lib -lz"
    ZLIB_TARBALL="2ab442d169156f34c379c968f3f482dd-zlib-1.2.7.tar.bz2"
fi
AC_SUBST(ZLIB_CFLAGS)
AC_SUBST(ZLIB_LIBS)
AC_SUBST(SYSTEM_ZLIB)
AC_SUBST(ZLIB_TARBALL)

dnl ===================================================================
dnl Check for system jpeg
dnl ===================================================================
AC_MSG_CHECKING([which jpeg to use])
if test "$with_system_jpeg" = "auto"; then
    case "$_os" in
    WINNT|Darwin|iOS|Android)
        with_system_jpeg="$with_system_libs"
        ;;
    *)
        with_system_jpeg=yes
        ;;
    esac
fi

if test "$with_system_jpeg" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_JPEG=YES
    AC_CHECK_HEADER(jpeglib.h, [],
        [AC_MSG_ERROR(jpeg.h not found. install libjpeg)], [])
    AC_CHECK_LIB(jpeg, jpeg_resync_to_restart, [ JPEG3RDLIB=-ljpeg ],
    [AC_MSG_CHECKING(jpeg library not found or fuctional)], [])
    libo_MINGW_CHECK_DLL([JPEG], [libjpeg])
else
    AC_MSG_RESULT([internal])
    SYSTEM_JPEG=NO
    JPEG_TARBALL="a2c10c04f396a9ce72894beb18b4e1f9-jpeg-8c.tar.gz"
    BUILD_TYPE="$BUILD_TYPE JPEG"
fi
AC_SUBST(SYSTEM_JPEG)
AC_SUBST(JPEG_TARBALL)
AC_SUBST([MINGW_JPEG_DLL])

dnl ===================================================================
dnl Check for system clucene
dnl ===================================================================
AC_MSG_CHECKING([which clucene to use])
if test "$with_system_clucene" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_CLUCENE=YES
    PKG_CHECK_MODULES(CLUCENE, libclucene-core)

    AC_LANG_PUSH([C++])
    save_CXXFLAGS=$CXXFLAGS
    save_CPPFLAGS=$CPPFLAGS
    CXXFLAGS="$CXXFLAGS $CLUCENE_CFLAGS"
    CPPFLAGS="$CPPFLAGS $CLUCENE_CFLAGS"
    dnl http://sourceforge.net/tracker/index.php?func=detail&aid=3392466&group_id=80013&atid=558446
    dnl https://bugzilla.redhat.com/show_bug.cgi?id=794795
    AC_CHECK_HEADER([CLucene/analysis/cjk/CJKAnalyzer.h], [],
                 [AC_MSG_ERROR([Your version of libclucene has contribs-lib missing.])], [#include <CLucene.h>])
    CXXFLAGS=$save_CXXFLAGS
    CPPFLAGS=$save_CPPFLAGS
    AC_LANG_POP([C++])

    CLUCENE_LIBS="$CLUCENE_LIBS -lclucene-contribs-lib"
else
    AC_MSG_RESULT([internal])
    SYSTEM_CLUCENE=NO
    CLUCENE_TARBALL=48d647fbd8ef8889e5a7f422c1bfda94-clucene-core-2.3.3.4.tar.gz
    BUILD_TYPE="$BUILD_TYPE CLUCENE"
fi
AC_SUBST(SYSTEM_CLUCENE)
AC_SUBST(CLUCENE_CFLAGS)
AC_SUBST(CLUCENE_LIBS)
AC_SUBST(CLUCENE_TARBALL)

dnl ===================================================================
dnl Check for system expat
dnl ===================================================================
AC_MSG_CHECKING([which expat to use])
if test "$with_system_expat" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_EXPAT=YES
    AC_CHECK_HEADER(expat.h, [],
        [AC_MSG_ERROR(expat.h not found. install expat)], [])
    AC_CHECK_LIB([expat], [XML_ParserCreate], [:],
    [AC_MSG_RESULT(expat library not found or functional.)], [])
    libo_MINGW_CHECK_DLL([EXPAT], [libexpat])
else
    AC_MSG_RESULT([internal])
    SYSTEM_EXPAT=NO
    EXPAT_TARBALL="dd7dab7a5fea97d2a6a43f511449b7cd-expat-2.1.0.tar.gz"
    BUILD_TYPE="$BUILD_TYPE EXPAT"
fi
AC_SUBST(SYSTEM_EXPAT)
AC_SUBST(EXPAT_TARBALL)
AC_SUBST([MINGW_EXPAT_DLL])

dnl ===================================================================
dnl Check for system libcdr
dnl ===================================================================
AC_MSG_CHECKING([which libcdr to use])
if test "$with_system_libcdr" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_LIBCDR=YES
    PKG_CHECK_MODULES( CDR, libcdr-0.0 >= 0.0.5 )
elif test $_os = iOS; then
    # libcdr depends on libwpd and libwpg and those are LGPL
    AC_MSG_RESULT([none])
else
    AC_MSG_RESULT([internal])
    SYSTEM_LIBCDR=NO
    CDR_TARBALL=3c0037fb07dea2f0bbae8386fa7c6a9a-libcdr-0.0.9.tar.bz2
fi
AC_SUBST(SYSTEM_LIBCDR)
AC_SUBST(CDR_CFLAGS)
AC_SUBST(CDR_LIBS)
AC_SUBST(CDR_TARBALL)

dnl ===================================================================
dnl Check for system libmspub
dnl ===================================================================
AC_MSG_CHECKING([which libmspub to use])
if test "$with_system_libmspub" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_LIBMSPUB=YES
    PKG_CHECK_MODULES( MSPUB, libmspub-0.0 )
elif test $_os = iOS; then
    # libmspub depends on libwpd and libwpg and those are LGPL
    AC_MSG_RESULT([none])
else
    AC_MSG_RESULT([internal])
    SYSTEM_LIBMSPUB=NO
    MSPUB_TARBALL=b2db54b6e96287ac995d7ed654ace4fc-libmspub-0.0.3.tar.bz2
fi
AC_SUBST(SYSTEM_LIBMSPUB)
AC_SUBST(MSPUB_CFLAGS)
AC_SUBST(MSPUB_LIBS)
AC_SUBST(MSPUB_TARBALL)

dnl ===================================================================
dnl Check for system libvisio
dnl ===================================================================
AC_MSG_CHECKING([which libvisio to use])
if test "$with_system_libvisio" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_LIBVISIO=YES
    PKG_CHECK_MODULES( VISIO, libvisio-0.0 )
elif test $_os = iOS; then
    # libvisio depends on libwpd and libwpg and those are LGPL
    AC_MSG_RESULT([none])
else
    AC_MSG_RESULT([internal])
    SYSTEM_LIBVISIO=NO
    VISIO_TARBALL=94e7f271e38c976462558b4278590178-libvisio-0.0.19.tar.bz2
fi
AC_SUBST(SYSTEM_LIBVISIO)
AC_SUBST(VISIO_CFLAGS)
AC_SUBST(VISIO_LIBS)
AC_SUBST(VISIO_TARBALL)

dnl ===================================================================
dnl Check for system libcmis
dnl ===================================================================
AC_MSG_CHECKING([which libcmis to use])
if test "$with_system_libcmis" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_LIBCMIS=YES
    PKG_CHECK_MODULES( CMIS, libcmis-0.3 >= 0.3.0 )
else
    AC_MSG_RESULT([internal])
    SYSTEM_LIBCMIS=NO
    CMIS_TARBALL=b2371dc7cf4811c9d32146eec913d296-libcmis-0.3.0.tar.gz
fi
AC_SUBST(SYSTEM_LIBCMIS)
AC_SUBST(CMIS_CFLAGS)
AC_SUBST(CMIS_LIBS)
AC_SUBST(CMIS_TARBALL)


dnl ===================================================================
dnl Check for system libwpd
dnl ===================================================================
AC_MSG_CHECKING([which libwpd to use])
if test "$with_system_libwpd" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_LIBWPD=YES
    PKG_CHECK_MODULES( WPD, libwpd-0.9 libwpd-stream-0.9 )
elif test $_os = iOS; then
    AC_MSG_RESULT([none])
else
    AC_MSG_RESULT([internal])
    SYSTEM_LIBWPD=NO
    WPD_CFLAGS="-I${OUTDIR}/inc/external"
    WPD_LIBS="-L${OUTDIR}/lib -lwpdlib"
    WPD_TARBALL=e7f84e3199dfee9122949448cab3823f-libwpd-0.9.6.tar.bz2
fi
AC_SUBST(SYSTEM_LIBWPD)
AC_SUBST(WPD_CFLAGS)
AC_SUBST(WPD_LIBS)
AC_SUBST(WPD_TARBALL)

dnl ===================================================================
dnl Check for system lcms2
dnl ===================================================================
AC_MSG_CHECKING([which littlecms v2 to use])
if test "$with_system_lcms2" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_LCMS2=YES
    PKG_CHECK_MODULES( LCMS2, lcms2 )
    libo_MINGW_CHECK_DLL([LCMS2], [liblcms2])
else
    AC_MSG_RESULT([internal])
    SYSTEM_LCMS2=NO
    BUILD_TYPE="$BUILD_TYPE LCMS2"
    LCMS2_CFLAGS="-I${OUTDIR}/inc/lcms2"
    if test $_os = Darwin; then
        LCMS2_CFLAGS="$LCMS2_CFLAGS -Wno-long-long"
    fi
    LCMS2_LIBS="-L${OUTDIR}/lib -llcms2"
fi
AC_SUBST(SYSTEM_LCMS2)
AC_SUBST(LCMS2_CFLAGS)
AC_SUBST(LCMS2_LIBS)
AC_SUBST([MINGW_LCMS2_DLL])

dnl ===================================================================
dnl Check for system cppunit
dnl ===================================================================
AC_MSG_CHECKING([which cppunit to use])
if test "$with_system_cppunit" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_CPPUNIT=YES
    # might work for earlier, too but go sure:
    PKG_CHECK_MODULES( CPPUNIT, cppunit >= 1.12.0 )
else
    AC_MSG_RESULT([internal])
    SYSTEM_CPPUNIT=NO
    BUILD_TYPE="$BUILD_TYPE CPPUNIT"
fi
AC_SUBST(SYSTEM_CPPUNIT)
AC_SUBST(CPPUNIT_CFLAGS)
AC_SUBST(CPPUNIT_LIBS)

dnl ===================================================================
dnl Check whether freetype is available
dnl ===================================================================
if test  "$test_freetype" = "yes"; then
    AC_MSG_CHECKING([whether freetype is available])
    PKG_CHECK_MODULES( FREETYPE, freetype2 >= 2.0 )
else
    case "$BUILD_TYPE" in
    *FREETYPE*)
        FREETYPE_LIBS="-lfreetype"
        ;;
    esac
fi
AC_SUBST(FREETYPE_CFLAGS)
AC_SUBST(FREETYPE_LIBS)

dnl ===================================================================
dnl Check for system libwps
dnl ===================================================================
AC_MSG_CHECKING([which libwps to use])
if test "$with_system_libwps" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_LIBWPS=YES
    PKG_CHECK_MODULES( WPS, libwps-0.2 )
elif test $_os = iOS; then
    AC_MSG_RESULT([none])
else
    AC_MSG_RESULT([internal])
    SYSTEM_LIBWPS=NO
    WPS_CFLAGS="-I${OUTDIR}/inc/external"
    WPS_LIBS="-L${OUTDIR}/lib -lwpslib"
    WPS_TARBALL=d197bd6211669a2fa4ca648faf04bcb1-libwps-0.2.7.tar.bz2
fi
AC_SUBST(SYSTEM_LIBWPS)
AC_SUBST(WPS_CFLAGS)
AC_SUBST(WPS_LIBS)
AC_SUBST(WPS_TARBALL)

dnl ===================================================================
dnl Check for system libwpg
dnl ===================================================================
AC_MSG_CHECKING([which libwpg to use])
if test "$with_system_libwpg" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_LIBWPG=YES
    PKG_CHECK_MODULES( WPG, libwpg-0.2 )
elif test $_os = iOS; then
    AC_MSG_RESULT([none])
else
    AC_MSG_RESULT([internal])
    SYSTEM_LIBWPG=NO
    WPG_CFLAGS="-I${OUTDIR}/inc/external"
    WPG_LIBS="-L${OUTDIR}/lib -lwpglib"
    WPG_TARBALL=9d283e02441d8cebdcd1e5d9df227d67-libwpg-0.2.1.tar.bz2
fi
AC_SUBST(SYSTEM_LIBWPG)
AC_SUBST(WPG_CFLAGS)
AC_SUBST(WPG_LIBS)
AC_SUBST(WPG_TARBALL)

dnl ===================================================================
dnl Check whether freetype2 supports emboldening
dnl ===================================================================
if test  "$test_freetype" = "yes"; then
    save_CPPFLAGS="$CPPFLAGS"
    save_LDFLAGS="$LDFLAGS"
    save_LIBS="$LIBS"
    CPPFLAGS="$CPPFLAGS $FREETYPE_CFLAGS"
    LDFLAGS="$LDFLAGS $FREETYPE_LIBS"
    AC_CHECK_LIB(freetype, FT_GlyphSlot_Embolden,
                 [USE_FT_EMBOLDEN="YES"], [USE_FT_EMBOLDEN="NO"], [])
    LDFLAGS="$save_LDFLAGS"
    CPPFLAGS="$save_CPPFLAGS"
    LIBS="$save_LIBS"
fi
AC_SUBST(USE_FT_EMBOLDEN)

# ===================================================================
# Check for system libxslt
# to prevent incompatibilities between internal libxml2 and external libxslt,
# or vice versa, use with_system_libxml here
# ===================================================================
if test "$with_system_libxml" = "auto"; then
    case "$_os" in
    WINNT|iOS|Android)
        with_system_libxml="$with_system_libs"
        ;;
    *)
        with_system_libxml=yes
        ;;
    esac
fi

AC_MSG_CHECKING([which libxslt to use])
if test "$with_system_libxml" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_LIBXSLT=YES
    if test "$_os" = "Darwin"; then
        dnl make sure to use SDK path
        LIBXSLT_CFLAGS="-I$MACOSX_SDK_PATH/usr/include/libxml2"
        LIBEXSLT_CFLAGS="$LIBXSLT_CFLAGS"
        dnl omit -L/usr/lib
        LIBXSLT_LIBS="-lxslt -lxml2 -lz -lpthread -liconv -lm"
        LIBEXSLT_LIBS="-lexslt $LIBXSLT_LIBS"
    else
        PKG_CHECK_MODULES(LIBXSLT, libxslt)
        PKG_CHECK_MODULES(LIBEXSLT, libexslt)
    fi

    dnl Check for xsltproc
    AC_PATH_PROG(XSLTPROC, xsltproc, no)
    if test "$XSLTPROC" = "no"; then
        AC_MSG_ERROR([xsltproc is required])
    fi

    libo_MINGW_CHECK_DLL([LIBXSLT], [libxslt])
    libo_MINGW_CHECK_DLL([LIBEXSLT], [libexslt])
    libo_MINGW_TRY_DLL([ICONV], [iconv])
else
    AC_MSG_RESULT([internal])
    SYSTEM_LIBXSLT=NO
    BUILD_TYPE="$BUILD_TYPE LIBXSLT"

    if test "$cross_compiling" = "yes"; then
        AC_PATH_PROG(XSLTPROC, xsltproc, no)
        if test "$XSLTPROC" = "no"; then
            AC_MSG_ERROR([xsltproc is required])
        fi
    fi
fi
AC_SUBST(SYSTEM_LIBXSLT)
if test -z "$SYSTEM_LIBXSLT_FOR_BUILD"; then
    SYSTEM_LIBXSLT_FOR_BUILD="$SYSTEM_LIBXSLT"
fi
AC_SUBST(SYSTEM_LIBXSLT_FOR_BUILD)

AC_SUBST(LIBEXSLT_CFLAGS)
AC_SUBST(LIBEXSLT_LIBS)
AC_SUBST(LIBXSLT_CFLAGS)
AC_SUBST(LIBXSLT_LIBS)
AC_SUBST(XSLTPROC)
AC_SUBST([MINGW_LIBEXSLT_DLL])
AC_SUBST([MINGW_LIBXSLT_DLL])
AC_SUBST([MINGW_ICONV_DLL])

# ===================================================================
# Check for system libxml
# ===================================================================
AC_MSG_CHECKING([which libxml to use])
if test "$with_system_libxml" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_LIBXML=YES
    if test "$_os" = "Darwin"; then
        dnl make sure to use SDK path
        LIBXML_CFLAGS="-I$MACOSX_SDK_PATH/usr/include/libxml2"
        dnl omit -L/usr/lib
        LIBXML_LIBS="-lxml2 -lz -lpthread -liconv -lm"
    elif test $_os = iOS; then
        dnl make sure to use SDK path
        usr=`echo '#include <stdlib.h>' | $CC -E -MD - | grep usr/include/stdlib.h | head -1 | sed -e 's,# 1 ",,' -e 's,/usr/include/.*,/usr,'`
        LIBXML_CFLAGS="-I$usr/include/libxml2"
        LIBXML_LIBS="-L$usr/lib -lxml2 -liconv"
    else
        PKG_CHECK_MODULES(LIBXML, libxml-2.0 >= 2.0)
    fi

    dnl Check for xmllint
    AC_PATH_PROG(XMLLINT, xmllint, no)
    if test "$XMLLINT" = "no"; then
        AC_MSG_ERROR([xmllint is required])
    fi

    libo_MINGW_CHECK_DLL([LIBXML],[libxml2])
    libo_MINGW_TRY_DLL([ZLIB],[zlib1])
else
    AC_MSG_RESULT([internal])
    SYSTEM_LIBXML=NO
    BUILD_TYPE="$BUILD_TYPE LIBXML2"
fi
AC_SUBST(SYSTEM_LIBXML)
AC_SUBST(LIBXML_CFLAGS)
AC_SUBST(LIBXML_LIBS)
AC_SUBST(XMLLINT)
AC_SUBST([MINGW_LIBXML_DLL])
AC_SUBST([MINGW_ZLIB_DLL])
dnl ===================================================================
dnl Checks for Python
dnl ===================================================================

AC_MSG_CHECKING([which Python to use])

case "$enable_python" in
no|disable)
    enable_python=no
    AC_MSG_RESULT([none])
    DISABLE_PYTHON=TRUE
    ;;
""|yes|auto)
    if test "$DISABLE_SCRIPTING" = TRUE; then
        AC_MSG_RESULT([no, overridden by --disable-scripting])
        enable_python=no
        DISABLE_PYTHON=TRUE
    elif test $build_os = cygwin; then
        dnl When building on Windows we don't attempt to use any installed
        dnl "system"  Python.
        dnl
        dnl (When cross-compiling to Windows from Linux using the mingw32-cross
        dnl compiler from OBS, use mingw32-python from OBS, and ditto for other
        dnl MinGW cross-compilation setups.)
        AC_MSG_RESULT([internal])
        enable_python=internal
    elif test $_os = Darwin -a \( "$with_macosx_version_min_required" = 10.4 -o "$with_macosx_version_min_required" = 10.5 \) \
                            -a \( "$with_macosx_sdk" = 10.4 -o "$with_macosx_sdk" = 10.5 \); then
        # When building against a more recent SDK the ionternal Python can't be compiled, I can't be bothered to fix that for now,
        # building against a recent SDK is only for developer's own builds anyway.
        AC_MSG_RESULT([internal, the Python in Mac OS X $with_macosx_version_min_required is incompatible with Python3-ified pyuno])
        enable_python=internal
    else
        AC_MSG_RESULT([checking below])
        enable_python=auto
    fi
    ;;
internal)
    AC_MSG_RESULT([internal])
    ;;
system)
    if test $_os = Darwin -a -z "$PYTHON" -a \( "$with_macosx_version_min_required" = 10.4 -o "$with_macosx_version_min_required" = 10.5 \); then
        AC_MSG_ERROR([Cannot use "system" Python, the Python in Mac OS X $with_macosx_version_min_required is incompatible with Python3-ified pyuno])
    fi
    AC_MSG_RESULT([system])
    ;;
*)
    AC_MSG_ERROR([Incorrect --enable-python option])
    ;;
esac

AC_SUBST(DISABLE_PYTHON)

if test $_os = WINNT -a "$WITH_MINGW" != yes -a $enable_python = system; then
    AC_MSG_ERROR([Must use internal Python when building with MSVC])
fi

if test $enable_python != no; then
    BUILD_TYPE="$BUILD_TYPE PYUNO"
fi

if test "$cross_compiling" = yes; then
    if test $enable_python = auto; then
        enable_python=system
    fi
else
    if test $enable_python = system -a -z "$PYTHON"; then
        if test $_os = Darwin; then
            # We already have logic above to make sure the system Python
            # is used only when building against SDK 10.6 or newer.

            # Make sure we use the 2.6 Python when building against the
            # 10.6 SDK.
            case $with_macosx_sdk in
            10.6)
                python_version=2.6;;
            10.7|10.8)
                python_version=2.7;;
            *)
                # ???
                python_version=2.7;;
            esac
            PYTHON=python$python_version
            PYTHON_CFLAGS="-I/Developer/SDKs/MacOSX${with_macosx_sdk}.sdk/System/Library/Frameworks/Python.framework/Versions/${python_version}/include/python${python_version}"
            PYTHON_LIBS="-framework Python"
        else
            # This causes an error if no python command is found
            AM_PATH_PYTHON([2.6])
        fi
    elif test $enable_python = auto; then
        dnl This allows lack of system Python
        AM_PATH_PYTHON([2.6],, [:])

        if test "$PYTHON" = :; then
            enable_python=internal
        else
            enable_python=system
        fi
    fi
fi

if test $enable_python = system; then
    if test -n "$PYTHON_CFLAGS" -a -n "$PYTHON_LIBS"; then
        # Fallback: Accept these in the environment, or as set above
        # for MacOSX.
        :
    elif test "$cross_compiling" != yes; then
        python_include=`$PYTHON -c "import distutils.sysconfig; print(distutils.sysconfig.get_config_var('INCLUDEPY'));"`
        python_version=`$PYTHON -c "import distutils.sysconfig; print(distutils.sysconfig.get_config_var('VERSION'));"`
        python_libs=`$PYTHON -c "import distutils.sysconfig; print(distutils.sysconfig.get_config_var('LIBS'));"`
        python_libdir=`$PYTHON -c "import distutils.sysconfig; print(distutils.sysconfig.get_config_var('LIBDIR'));"`
        if test -z "$PKG_CONFIG"; then
            PYTHON_CFLAGS="-I$python_include"
            PYTHON_LIBS="-L$python_libdir -lpython$python_version $python_libs"
        elif $PKG_CONFIG --exists python-$python_version; then
            PYTHON_CFLAGS="`$PKG_CONFIG --cflags python-$python_version`"
            PYTHON_LIBS="`$PKG_CONFIG --libs python-$python_version` $python_libs"
        else
            PYTHON_CFLAGS="-I$python_include"
            PYTHON_LIBS="-L$python_libdir -lpython$python_version $python_libs"
        fi
    else
        dnl How to find out the cross-compilation Python installation path?
        dnl Let's hardocode what we know for different distributions for now...
        for python_version in 2.6; do
            if test -f ${MINGW_SYSROOT}/include/python${python_version}/Python.h; then
                PYTHON_CFLAGS="-I${MINGW_SYSROOT}/include/python$python_version"
                PYTHON_LIBS="-L${MINGW_SYSROOT}lib -lpython$python_version $python_libs"
                MINGW_PYTHON_EXE=python.exe
                libo_MINGW_CHECK_DLL([PYTHON], [libpython$python_version])
                libo_MINGW_CHECK_DLL([READLINE], [libreadline6])
                libo_MINGW_CHECK_DLL([TERMCAP], [libtermcap])
                # could we somehow extract the really mingw python version from
                # actual distro package?
                # 2.6.2 currently on OpenSUSE 12.1?
                # rpm -q mingw32-python => mingw32-python-2.6.2-17.17.noarch
                MINGW_PYTHON_MAJOR_VERSION="$python_version"
                MINGW_PYVERSION=$python_version.2
                break
            fi
        done
        test -n "$PYTHON_CFLAGS" && break
    fi
fi

dnl By now enable_python should be "system", "internal" or "no"
case $enable_python in
system)
    SYSTEM_PYTHON=YES

    dnl Check if the headers really work
    save_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $PYTHON_CFLAGS"
    AC_CHECK_HEADER(Python.h, [],
       [AC_MSG_ERROR([Python headers not found. You probably want to set both the PYTHON_CFLAGS and PYTHON_LIBS environment variables.])],
       [])
    CPPFLAGS="$save_CPPFLAGS"

    dnl FIXME Check if the Python library can be linked with, too?
    ;;

internal)
    SYSTEM_PYTHON=NO
    BUILD_TYPE="$BUILD_TYPE PYTHON"
    # Embedded Python dies without Home set
    if test "$HOME" = ""; then
        export HOME=""
    fi
    # bz2 tarball and bzip2 is not standard
    if test -z "$BZIP2"; then
        AC_PATH_PROG( BZIP2, bzip2)
        if test -z "$BZIP2"; then
            AC_MSG_ERROR([the internal Python module has a .tar.bz2. You need bzip2])
        fi
    fi
    ;;
no)
    SYSTEM_PYTHON=NO
    ;;
*)
    AC_MSG_ERROR([Internal configure script error, invalid enable_python value "$enable_python"])
    ;;
esac

dnl Cross-build of python requires python that is runnable on the build
dnl platform, because it is used in the build process
if test "$cross_compiling" = yes; then
    if test "$DISABLE_PYTHON" = no -a "$SYSTEM_PYTHON" = no -a "$DISABLE_PYTHON_FOR_BUILD" = yes; then
        AC_MSG_ERROR([Cross-build of python requires python that is runnable on the build platform. You must not use --disable-python for the 'build' config.])
    fi
fi

AC_SUBST(SYSTEM_PYTHON)
AC_SUBST(PYTHON_CFLAGS)
AC_SUBST(PYTHON_LIBS)
AC_SUBST([MINGW_PYTHON_EXE])
AC_SUBST([MINGW_PYTHON_DLL])
AC_SUBST([MINGW_READLINE_DLL])
AC_SUBST([MINGW_TERMCAP_DLL])
AC_SUBST(MINGW_PYVERSION)
AC_SUBST(MINGW_PYTHON_MAJOR_VERSION)

dnl ===================================================================
dnl Check for system Berkeley db
dnl ===================================================================
AC_MSG_CHECKING([which db to use])
if test "$with_system_db" = "yes"; then
    SYSTEM_DB=YES
    AC_MSG_RESULT([external])

    db_header=
    for dbver in 5.3 5.1 5.0 5 4.8 4.7 4; do
        for dash in - ''; do
            AC_CHECK_HEADER([db$dash$dbver/db.h],
                [ db_header="db$dash$dbver/db.h"; break 2 ])
        done
    done

    if test -z "$db_header"; then
        AC_CHECK_HEADER([db/db.h], [db_header='db/db.h'; dbver=''])
    fi

    if test -z "$db_header"; then
       AC_CHECK_HEADER(db.h, [ db_header='db.h'; dbver='' ])
    fi

    if test -z "$db_header"; then
        AC_MSG_ERROR([db.h not found. Use the correct -I flag, or install the Berkeley db development package.])
    fi

    AC_MSG_CHECKING([whether db is at least 4.1])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <$db_header>]], [[int array[(DB_VERSION_MAJOR > 4 || (DB_VERSION_MAJOR == 4 && DB_VERSION_MINOR >= 1))-1];]])],[AC_MSG_RESULT([yes])],[AC_MSG_ERROR([no. you need at least db 4.1])])

    SYSTEM_DB_CFLAGS="-DSYSTEM_DB_HEADER='<$db_header>'"

    DB_LIB=
    dnl At least on OpenBSD dbver is not appended to the library
    if test "$_os" = "OpenBSD"; then
        dbver=''
    fi
    for dash in - ''; do
        AC_CHECK_LIB(db$dash$dbver, dbopen,
            [ DB_LIB="db$dash$dbver"; DB_CPPLIB="db_cxx$dash$dbver"; break ])
        AC_CHECK_LIB(db$dash$dbver, __db185_open,
            [ DB_LIB="db$dash$dbver"; DB_CPPLIB="db_cxx$dash$dbver"; break ])
    done

    if test -z "$DB_LIB"; then
        AC_MSG_ERROR([db library not found. Use the correct -L flag,
or install the Berkeley db development package.])
    fi

    SCPDEFS="$SCPDEFS -DSYSTEM_DB"
    libo_MINGW_CHECK_DLL([DB], [libdb])
elif test $_os != iOS -a $_os != Android; then
    AC_MSG_RESULT([internal])
    SYSTEM_DB=NO
    BUILD_TYPE="$BUILD_TYPE BERKELEYDB"
else
   AC_MSG_RESULT([none])
fi
AC_SUBST(SYSTEM_DB)
AC_SUBST(SYSTEM_DB_CFLAGS)
AC_SUBST(DB_LIB)
AC_SUBST(DB_CPPLIB)
AC_SUBST([MINGW_DB_DLL])

AC_MSG_CHECKING([whether to build the MySQL Connector extension])
if test "x$enable_ext_mysql_connector" = "xyes" -a "x$enable_extension_integration" != "xno"; then
    AC_MSG_RESULT([yes])
    ENABLE_MYSQLC=YES
    BUILD_TYPE="$BUILD_TYPE MYSQLC"
else
    AC_MSG_RESULT([no])
    ENABLE_MYSQLC=NO
fi
AC_SUBST(ENABLE_MYSQLC)

if test "$ENABLE_MYSQLC" = "YES"; then

    SCPDEFS="$SCPDEFS -DWITH_EXTENSION_MYSQLC"

    dnl ===================================================================
    dnl Check for system MySQL
    dnl ===================================================================
    AC_MSG_CHECKING([for MySQL prerequisites])
    if test "$with_system_mysql" = "yes"; then
        AC_MSG_RESULT([external MySQL])
        SYSTEM_MYSQL=YES
        AC_PATH_PROG( MYSQLCONFIG, mysql_config)
        AC_MSG_CHECKING([MySQL version])
        MYSQL_VERSION=`$MYSQLCONFIG --version`
        MYSQL_MAJOR=`$MYSQLCONFIG --version | cut -d"." -f1`
        if test "$MYSQL_MAJOR" -ge "5"; then
            AC_MSG_RESULT([OK])
        else
            AC_MSG_ERROR([too old, use 5.0.x or 5.1.x])
        fi
        AC_MSG_CHECKING([for MySQL Client library])
        MYSQL_INC=`$MYSQLCONFIG --include`
        MYSQL_LIB=`$MYSQLCONFIG --libs`
        MYSQL_DEFINES=`$MYSQLCONFIG --cflags | $SED -e s,$MYSQL_INC,,`
        AC_MSG_RESULT([includes $MYSQL_INC, libraries $MYSQL_LIB])
    else
        SYSTEM_MYSQL=NO
        if test -n "$with_libmysql_path"; then
            AC_MSG_RESULT([external Connector/C (libmysql)])
            LIBMYSQL=libmysql.so
            if test "$_os" = "Darwin"; then
                LIBMYSQL=libmysql.dylib
            elif test "$_os" = "WINNT"; then
                LIBMYSQL=libmysql.dll
            fi
            AC_MSG_CHECKING([for $LIBMYSQL])
            if test -e "$with_libmysql_path/lib/$LIBMYSQL"; then
                AC_MSG_RESULT([found.])
                LIBMYSQL_PATH=$with_libmysql_path
            else
                AC_MSG_ERROR([not found. Please specify proper path in --with-libmysql-path.])
            fi
        else
            AC_MSG_ERROR([not given. Please specify either --with-system-mysql or --with-libmysql-path])
        fi
    fi
    AC_SUBST(SYSTEM_MYSQL)
    AC_SUBST(MYSQL_INC)
    AC_SUBST(MYSQL_LIB)
    AC_SUBST(MYSQL_DEFINES)
    AC_SUBST(LIBMYSQL_PATH)

    AC_LANG_PUSH([C++])
    dnl ===================================================================
    dnl Check for system MySQL C++ Connector
    dnl ===================================================================
    # FIXME!
    # who thought this too-generic cppconn dir was a good idea?
    AC_MSG_CHECKING([MySQL Connector/C++])
    if test "$with_system_mysql_cppconn" = "yes"; then
        AC_MSG_RESULT([external])
        SYSTEM_MYSQL_CPPCONN=YES
        AC_LANG_PUSH([C++])
        AC_CHECK_HEADER(mysql_driver.h, [],
                    [AC_MSG_ERROR(mysql_driver.h not found. install MySQL C++ Connectivity)], [])
        AC_CHECK_LIB([mysqlcppconn], [main], [:],
                    [AC_MSG_ERROR(MySQL C++ Connectivity lib not found or functional)], [])
        save_LIBS=$LIBS
        LIBS="$LIBS -lmysqlcppconn"
        AC_MSG_CHECKING([version])
        AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <mysql_driver.h>

int main(int argc, char **argv) {
    sql::Driver *driver;
    driver = get_driver_instance();
    if (driver->getMajorVersion() > 1 || \
       (driver->getMajorVersion() == 1 && driver->getMinorVersion() > 0) || \
       (driver->getMajorVersion() == 1 && driver->getMinorVersion() == 0 && driver->getPatchVersion() >= 6))
        return 0;
      else
        return 1;
}
      ]])],[AC_MSG_RESULT(OK)],[AC_MSG_ERROR([not suitable, we need >= 1.0.6])],[])

        AC_LANG_POP([C++])
        LIBS=$save_LIBS
    else
        AC_MSG_RESULT([internal])
        AC_MSG_CHECKING([for mysqlcppconn module])
        if test -d mysqlcppconn; then
            AC_MSG_RESULT([OK])
        else
            AC_MSG_ERROR([not existing. get it (did you get the -extensions tarball?)])
        fi
        BUILD_TYPE="$BUILD_TYPE MYSQLCPPCONN"
        SYSTEM_MYSQL_CPPCONN=NO
    fi
    AC_LANG_POP([C++])
    AC_SUBST(SYSTEM_MYSQL_CPPCONN)
fi

dnl ===================================================================
dnl Check for system hsqldb
dnl ===================================================================
if test "$with_java" != "no"; then
HSQLDB_USE_JDBC_4_1=
AC_MSG_CHECKING([which hsqldb to use])
if test "$with_system_hsqldb" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_HSQLDB=YES
    if test -z $HSQLDB_JAR; then
        HSQLDB_JAR=/usr/share/java/hsqldb.jar
    fi
    AC_CHECK_FILE($HSQLDB_JAR, [],
               [AC_MSG_ERROR(hsqldb.jar not found.)], [])
    AC_MSG_CHECKING([whether hsqldb is 1.8.0.x])
    export HSQLDB_JAR
    if $PERL -e 'use Archive::Zip;
            my $file = "$ENV{'HSQLDB_JAR'}";
            my $zip = Archive::Zip->new( $file );
            my $mf = $zip->contents ( "META-INF/MANIFEST.MF" );
            if ( $mf =~ m/Specification-Version: 1.8.*/ ) {
               push @l, split(/\n/, $mf);
               foreach my $line (@l) {
                  if ($line =~ m/Specification-Version:/) {
                      ($t, $version) = split (/:/,$line);
                      $version =~ s/^\s//;
                      ($a, $b, $c, $d) = split (/\./,$version);
                      if ($c == "0" && $d > "8")  {
                          exit 0;
                      } else {
                          exit 1;
                      }
                  }
               }
            } else {
                exit 1;
            }'; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_ERROR([no, you need hsqldb >= 1.8.0.9 but < 1.8.1])
    fi
else
    AC_MSG_RESULT([internal])
    SYSTEM_HSQLDB=NO
    BUILD_TYPE="$BUILD_TYPE HSQLDB"
    AC_MSG_CHECKING([whether hsqldb should be built with JDBC 4.1])
    javanumver=`$JAVAINTERPRETER -version 2>&1 | $AWK -v num=true -f $SRC_ROOT/solenv/bin/getcompver.awk`
    if expr "$javanumver" '>=' 000100060000 > /dev/null; then
        AC_MSG_RESULT([yes])
        HSQLDB_USE_JDBC_4_1=YES
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_SUBST(SYSTEM_HSQLDB)
AC_SUBST(HSQLDB_JAR)
AC_SUBST([HSQLDB_USE_JDBC_4_1])
fi

dnl ===================================================================
dnl Check for PostgreSQL stuff
dnl ===================================================================
if test "x$enable_postgresql_sdbc" != "xno"; then
    SCPDEFS="$SCPDEFS -DWITH_POSTGRESQL_SDBC"

    # if/when anything else than PostgreSQL uses Kerberos,
    # move this out of `test "x$enable_postgresql_sdbc" != "xno"'
    WITH_KRB5=NO
    WITH_GSSAPI=no
    case "$_os" in
    Darwin)
        # MacOS X has system MIT Kerberos 5 since 10.4
        if test "$with_krb5" != "no"; then
            WITH_KRB5=YES
            save_LIBS=$LIBS
            AC_SEARCH_LIBS(com_err, [com_err 'com_err -lssl -lcrypto' krb5 'krb5 -lcrypto -ldes -lasn1 -lroken'], [],
                [AC_MSG_ERROR([could not find function 'com_err' required for Kerberos 5])])
            LIBS=$save_LIBS
            AC_SEARCH_LIBS(krb5_sendauth, [krb5 'krb5 -lcrypto -ldes -lasn1 -lroken'], [],
                [AC_MSG_ERROR([could not find function 'krb5_sendauth' required for Kerberos 5])])
            LIBS=$save_LIBS
        fi
        if test "$with_gssapi" != "no"; then
          WITH_GSSAPI=YES
          save_LIBS=$LIBS
          AC_SEARCH_LIBS(gss_init_sec_context, [gssapi_krb5 gss 'gssapi -lkrb5 -lcrypto'], [],
                [AC_MSG_ERROR([could not find function 'gss_init_sec_context' required for GSSAPI])])
          LIBS=$save_LIBS
        fi
        ;;
    WINNT)
        if test "$with_krb5" = "yes" -o "$with_gssapi" = "yes"; then
            AC_MSG_ERROR([Refusing to enable MIT Kerberos 5 or GSSAPI on Windows.])
        fi
        ;;
    Linux|GNU|*BSD|DragonFly)
        if test "$with_krb5" != "no"; then
            WITH_KRB5=YES
            save_LIBS=$LIBS
            AC_SEARCH_LIBS(com_err, [com_err 'com_err -lssl -lcrypto' krb5 'krb5 -lcrypto -ldes -lasn1 -lroken'], [],
                [AC_MSG_ERROR([could not find function 'com_err' required for Kerberos 5])])
            LIBS=$save_LIBS
            AC_SEARCH_LIBS(krb5_sendauth, [krb5 'krb5 -lcrypto -ldes -lasn1 -lroken'], [],
                [AC_MSG_ERROR([could not find function 'krb5_sendauth' required for Kerberos 5])])
            LIBS=$save_LIBS
        fi
        if test "$with_gssapi" != "no"; then
            WITH_GSSAPI=YES
            save_LIBS=$LIBS
            AC_SEARCH_LIBS(gss_init_sec_context, [gssapi_krb5 gss 'gssapi -lkrb5 -lcrypto'], [],
                [AC_MSG_ERROR([could not find function 'gss_init_sec_context' required for GSSAPI])])
            LIBS=$save_LIBS
        fi
        ;;
    *)
        if test "$with_krb5" = "yes"; then
            WITH_KRB5=YES
            save_LIBS=$LIBS
            AC_SEARCH_LIBS(com_err, [com_err 'com_err -lssl -lcrypto' krb5 'krb5 -lcrypto -ldes -lasn1 -lroken'], [],
                [AC_MSG_ERROR([could not find function 'com_err' required for Kerberos 5])])
            LIBS=$save_LIBS
            AC_SEARCH_LIBS(krb5_sendauth, [krb5 'krb5 -lcrypto -ldes -lasn1 -lroken'], [],
                [AC_MSG_ERROR([could not find function 'krb5_sendauth' required for Kerberos 5])])
            LIBS=$save_LIBS
        fi
        if test "$with_gssapi" = "yes"; then
            WITH_GSSAPI=YES
            save_LIBS=$LIBS
            AC_SEARCH_LIBS(gss_init_sec_context, [gssapi_krb5 gss 'gssapi -lkrb5 -lcrypto'], [],
                [AC_MSG_ERROR([could not find function 'gss_init_sec_context' required for GSSAPI])])
            LIBS=$save_LIBS
        fi
    esac

    AC_MSG_CHECKING([PostgreSQL C interface])
    if test "$with_system_postgresql" = "yes"; then
        AC_MSG_RESULT([external PostgreSQL])
        SYSTEM_POSTGRESQL=YES
        if test "$_os" = Darwin; then
            supp_path=''
            for d in /Library/PostgreSQL/9.*/bin /sw/opt/postgresql/9.*/bin /opt/local/lib/postgresql9*/bin; do
                pg_supp_path="$P_SEP$d$pg_supp_path"
            done
        fi
        AC_PATH_PROG(PGCONFIG, pg_config, ,$PATH$pg_supp_path)
        if test -z "$PGCONFIG"; then
            AC_MSG_ERROR([pg_config needed; set PGCONFIG if not in PATH])
        fi
        POSTGRESQL_INC=-I$(${PGCONFIG} --includedir)
        POSTGRESQL_LIB="-L$(${PGCONFIG} --libdir)"
    else
        if test -n "$with_libpq_path"; then
            SYSTEM_POSTGRESQL=YES
            AC_MSG_RESULT([external libpq])
            POSTGRESQL_LIB="-L${with_libpq_path}/lib/"
            POSTGRESQL_INC=-I"${with_libpq_path}/include/"
        else
            SYSTEM_POSTGRESQL=NO
            AC_MSG_RESULT([internal])
            POSTGRESQL_LIB=""
            POSTGRESQL_INC="%OVERRIDE_ME%"
            BUILD_TYPE="$BUILD_TYPE POSTGRESQL"
	    POSTGRESQL_TARBALL="061a9f17323117c9358ed60f33ecff78-postgresql-9.1.1.tar.bz2"
        fi
    fi
    if test "${SYSTEM_POSTGRESQL}" = "YES"; then
        AC_MSG_NOTICE([checking system PostgreSQL prerequisites])
        save_CFLAGS=$CFLAGS
        save_CPPFLAGS=$CPPFLAGS
        save_LIBS=$LIBS
        CPPFLAGS="${CPPFLAGS} ${POSTGRESQL_INC}"
        LIBS="${LIBS} ${POSTGRESQL_LIB}"
        AC_CHECK_HEADER([libpq-fe.h], [], [AC_MSG_ERROR([libpq-fe.h is needed])], [])
        AC_CHECK_LIB([pq], [PQconnectdbParams], [:],
            [AC_MSG_ERROR(libpq not found or too old. Need >= 9.0)], [])
        CFLAGS=$save_CFLAGS
        CPPFLAGS=$save_CPPFLAGS
        LIBS=$save_LIBS
    fi
    BUILD_POSTGRESQL_SDBC=YES
fi
AC_SUBST(WITH_KRB5)
AC_SUBST(WITH_GSSAPI)
AC_SUBST(BUILD_POSTGRESQL_SDBC)
AC_SUBST(SYSTEM_POSTGRESQL)
AC_SUBST(POSTGRESQL_INC)
AC_SUBST(POSTGRESQL_LIB)
AC_SUBST(POSTGRESQL_TARBALL)

dnl ===================================================================
dnl Check for system curl
dnl ===================================================================
AC_MSG_CHECKING([which libcurl to use])
if test "$with_system_curl" = "auto"; then
    case "$_os" in
    Darwin)
        with_system_curl=yes
        ;;
    *)
        with_system_curl="$with_system_libs"
        ;;
    esac
fi

if test "$with_system_curl" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_CURL=YES

    curl_version=""
    if test "$cross_compiling" = "yes"; then
        dnl At least the OBS mingw32-libcurl-devel package
        dnl comes with a proper .pc file
        PKG_CHECK_MODULES(CURL, libcurl,, [:])
        if test -n "$CURL_PKG_ERRORS"; then
            AC_MSG_RESULT([no])
        else
            curl_version=`$PKG_CONFIG --modversion libcurl`
        fi
    fi
    if test -z "$curl_version"; then
        AC_PATH_PROG(CURLCONFIG, curl-config)
        if test -z "$CURLCONFIG"; then
            AC_MSG_ERROR([install the libcurl development package])
        fi
        CURL_LIBS=`$CURLCONFIG --libs`
        CURL_CFLAGS=`$CURLCONFIG --cflags`
        curl_version=`$CURLCONFIG --version | $SED -e 's/^libcurl //'`
    fi

    AC_MSG_CHECKING([whether libcurl is >= 7.13.1])

    case $curl_version in
    dnl brackets doubled below because Autoconf uses them as m4 quote characters,
    dnl so they need to be doubled to end up in the configure script
    7.13.1|7.1[[4-9]].*|7.[[2-9]]?.*|7.???.*|[[8-9]].*|[[1-9]][[0-9]].*)
        AC_MSG_RESULT([yes, you have $curl_version])
        ;;
    *)
        AC_MSG_ERROR([no, you have $curl_version])
        ;;
    esac

    libo_MINGW_CHECK_DLL([CURL], [libcurl])
    libo_MINGW_TRY_DLL([INTL], [libintl])
    libo_MINGW_TRY_DLL([IDN], [libidn])
    libo_MINGW_TRY_DLL([NSPR4], [libnspr4])
    libo_MINGW_TRY_DLL([NSSUTIL3], [nssutil3])
    libo_MINGW_TRY_DLL([PLC4], [libplc4])
    libo_MINGW_TRY_DLL([PLDS4], [libplds4])
    libo_MINGW_TRY_DLL([NSS3], [nss3])
    libo_MINGW_TRY_DLL([SSL3], [ssl3])
    libo_MINGW_TRY_DLL([GPG_ERROR], [libgpg-error])
    libo_MINGW_TRY_DLL([GCRYPT], [libgcrypt])
    libo_MINGW_TRY_DLL([SSH2], [libssh2])
else
    AC_MSG_RESULT([internal])
    SYSTEM_CURL=NO
    CURL_TARBALL="3fa4d5236f2a36ca5c3af6715e837691-curl-7.26.0.tar.gz"
    BUILD_TYPE="$BUILD_TYPE CURL"
fi
AC_SUBST(SYSTEM_CURL)
AC_SUBST(CURL_TARBALL)
AC_SUBST(CURL_CFLAGS)
AC_SUBST(CURL_LIBS)
AC_SUBST([MINGW_CURL_DLL])
AC_SUBST([MINGW_GCRYPT_DLL])
AC_SUBST([MINGW_GPG_ERROR_DLL])
AC_SUBST([MINGW_IDN_DLL])
AC_SUBST([MINGW_INTL_DLL])
AC_SUBST([MINGW_NSPR4_DLL])
AC_SUBST([MINGW_NSS3_DLL])
AC_SUBST([MINGW_NSSUTIL3_DLL])
AC_SUBST([MINGW_PLC4_DLL])
AC_SUBST([MINGW_PLDS4_DLL])
AC_SUBST([MINGW_SSH2_DLL])
AC_SUBST([MINGW_SSL3_DLL])

dnl ===================================================================
dnl Check for system boost
dnl ===================================================================
AC_MSG_CHECKING([which boost to use])
if test "$with_system_boost" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_BOOST=YES
    AC_LANG_PUSH([C++])
    AC_CHECK_HEADER(boost/shared_ptr.hpp, [],
       [AC_MSG_ERROR(boost/shared_ptr.hpp not found. install boost)], [])
    AC_CHECK_HEADER(boost/spirit/include/classic_core.hpp, [],
       [AC_MSG_ERROR(boost/spirit/include/classic_core.hpp not found. install boost >= 1.36)], [])
    AC_CHECK_HEADER(boost/function.hpp, [],
       [AC_MSG_ERROR(boost/function.hpp not found. install boost)], [])
    AC_CHECK_HEADER([boost/unordered_map.hpp], [HAVE_BOOST_UNORDERED_MAP=TRUE],
                   [AC_MSG_WARN([boost/unordered_map.hpp not found])], [])

    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS -fno-exceptions"
    AC_MSG_CHECKING([whether boost/function.hpp compiles with -fno-exceptions])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <boost/function.hpp>
]], [[]])],[ac_cv_cxx_boost_no_exceptions_broken=no],[ac_cv_cxx_boost_no_exceptions_broken=yes])

    if test "$ac_cv_cxx_boost_no_exceptions_broken" = "yes"; then
        AC_MSG_ERROR([no, see https://bugzilla.redhat.com/show_bug.cgi?id=477131])
    else
        AC_MSG_RESULT([yes])
    fi
    CXXFLAGS=$save_CXXFLAGS
    AC_LANG_POP([C++])

    AX_BOOST_BASE
    AX_BOOST_DATE_TIME
    mingw_boost_date_time_dll=`echo $BOOST_DATE_TIME_LIB | sed -e 's/^-l//' -e 's/\.dll$//'`
    libo_MINGW_TRY_DLL([BOOST_DATE_TIME], [$mingw_boost_date_time_dll])
    if test -z "$MINGW_BOOST_DATE_TIME_DLL"; then
        # if not found, try again with 'lib' prefix
        libo_MINGW_CHECK_DLL([BOOST_DATE_TIME],
                             [lib$mingw_boost_date_time_dll])
    fi
else
    AC_MSG_RESULT([internal])
    BUILD_TYPE="$BUILD_TYPE BOOST"
    SYSTEM_BOOST=NO
    BOOST_TARBALL=f02578f5218f217a9f20e9c30e119c6a-boost_1_44_0.tar.bz2
    HAVE_BOOST_UNORDERED_MAP=TRUE
fi
AC_SUBST([HAVE_BOOST_UNORDERED_MAP])
AC_SUBST([MINGW_BOOST_DATE_TIME_DLL])
AC_SUBST(SYSTEM_BOOST)
AC_SUBST(BOOST_TARBALL)

dnl ===================================================================
dnl Check for system mdds
dnl ===================================================================
AC_MSG_CHECKING([which mdds to use])
if test "$with_system_mdds" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_MDDS=YES

    dnl ===================================================================
    dnl Determine which hash container mdds shall use
    dnl ===================================================================
    AC_MSG_CHECKING([which hash container mdds shall use])
    if test "x$HAVE_CXX0X" = "xTRUE"; then
        MDDS_CPPFLAGS="-std=gnu++0x"
        AC_MSG_RESULT([std::unordered_map])
    else
        MDDS_CPPFLAGS="-DMDDS_HASH_CONTAINER_BOOST"
        AC_MSG_RESULT([boost::unordered_map])
    fi

    AC_LANG_PUSH([C++])
    save_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $MDDS_CPPFLAGS"
    AC_CHECK_HEADER(mdds/flat_segment_tree.hpp, [],
        [AC_MSG_ERROR(mdds/flat_segment_tree.hpp not found. install mdds)], [])
    AC_CHECK_HEADER(mdds/multi_type_matrix.hpp, [],
        [AC_MSG_ERROR(mdds/multi_type_matrix.hpp not found. install mdds >= 0.6.0)], [])
    CPPFLAGS="$save_CPPFLAGS"
    AC_LANG_POP([C++])
else
    AC_MSG_RESULT([internal])
    MDDS_TARBALL=9f9e15966b5624834157fe3d748312bc-mdds_0.6.1.tar.bz2
    SYSTEM_MDDS=NO

    dnl ===================================================================
    dnl Determine which hash container mdds shall use
    dnl ===================================================================
    AC_MSG_CHECKING([which hash container mdds shall use])
    MDDS_CPPFLAGS="-DMDDS_HASH_CONTAINER_BOOST"
    AC_MSG_RESULT([boost::unordered_map])
fi
AC_SUBST(SYSTEM_MDDS)
AC_SUBST(MDDS_TARBALL)
AC_SUBST([MDDS_CPPFLAGS])

dnl ===================================================================
dnl Check for system vigra
dnl ===================================================================
AC_MSG_CHECKING([which vigra to use])
if test "$with_system_vigra" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_VIGRA=YES
    AC_LANG_PUSH([C++])
    AC_CHECK_HEADER(vigra/copyimage.hxx, [],
       [AC_MSG_ERROR(vigra/copyimage.hxx not found. install vigra)], [])
    AC_LANG_POP([C++])
else
    AC_MSG_RESULT([internal])
    VIGRA_TARBALL=ea91f2fb4212a21d708aced277e6e85a-vigra1.4.0.tar.gz
    SYSTEM_VIGRA=NO
fi
AC_SUBST(SYSTEM_VIGRA)
AC_SUBST(VIGRA_TARBALL)

dnl ===================================================================
dnl Check for system odbc
dnl ===================================================================
AC_MSG_CHECKING([which odbc headers to use])
if test "$with_system_odbc" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_ODBC_HEADERS=YES

    AC_CHECK_HEADER(sqlext.h, [],
      [AC_MSG_ERROR(odbc not found. install odbc)], [])
elif test "$enable_database_connectivity" != yes; then
    AC_MSG_RESULT([none])
else
    AC_MSG_RESULT([internal])
    SYSTEM_ODBC_HEADERS=NO
fi
AC_SUBST(SYSTEM_ODBC_HEADERS)

AC_MSG_CHECKING([whether to build Mozilla addressbook connectivity])
if test "$_os" = "WINNT"; then
   if test "$enable_mozilla" = "no"; then
      AC_MSG_RESULT([no])
   elif test "$with_system_mozilla" = "yes"; then
      AC_MSG_RESULT([no, not possible with system-mozilla])
   else
      AC_MSG_RESULT([yes (old mozab driver)])
   fi
else
# on all other platforms Mozilla addressbook connectivity
# is unconditionally built
# new and shiny mork driver
   AC_MSG_RESULT([yes (new mork driver)])
fi

dnl ===================================================================
dnl Check for system openldap
dnl ===================================================================

if test "$_os" != "WINNT" -a "$_os" != "iOS" -a "$_os" != "Android"; then
AC_MSG_CHECKING([which openldap library to use])
if test "$with_system_openldap" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_OPENLDAP=YES
    AC_CHECK_HEADERS(ldap.h, [], [AC_MSG_ERROR(ldap.h not found. install openldap libs)], [])
    AC_CHECK_LIB([ldap], [ldap_simple_bind_s], [:], [AC_MSG_ERROR(openldap lib not found or functional)], [])
    AC_CHECK_LIB([ldap], [ldap_set_option], [:], [AC_MSG_ERROR(openldap lib not found or functional)], [])
else
    AC_MSG_RESULT([internal])
    SYSTEM_OPENLDAP=NO
    OPENLDAP_TARBALL="804c6cb5698db30b75ad0ff1c25baefd-openldap-2.4.31.tgz"
    BUILD_TYPE="$BUILD_TYPE OPENLDAP"
fi
fi
AC_SUBST(SYSTEM_OPENLDAP)
AC_SUBST(OPENLDAP_TARBALL)

dnl ===================================================================
dnl Check for system mozilla
dnl ===================================================================

AC_MSG_CHECKING([which Mozilla to use])
if test -n "$with_system_mozilla" -a "$with_system_mozilla" != "no"; then
    AC_MSG_RESULT([external])
    SYSTEM_MOZILLA=YES
    AC_MSG_CHECKING([which Mozilla flavour to use])
    if test -n "$with_system_mozilla" -a "$with_system_mozilla" = "libxul"; then
        MOZ_FLAVOUR=libxul
    elif test -n "$with_system_mozilla" -a "$with_system_mozilla" = "xulrunner"; then
        MOZ_FLAVOUR=xulrunner
    elif test -n "$with_system_mozilla" -a "$with_system_mozilla" = "seamonkey"; then
        MOZ_FLAVOUR=seamonkey
    elif test -n "$with_system_mozilla" -a "$with_system_mozilla" = "iceape"; then
        MOZ_FLAVOUR=iceape
    elif test -n "$with_system_mozilla" -a "$with_system_mozilla" = "firefox"; then
        MOZ_FLAVOUR=firefox
    elif test -n "$with_system_mozilla" -a "$with_system_mozilla" = "mozilla"; then
        MOZ_FLAVOUR=mozilla
    else
        MOZ_FLAVOUR=libxul
    fi
    tmp=`echo $MOZ_FLAVOUR | $PERL -e 'print ucfirst(<STDIN>);'`
    AC_MSG_RESULT($tmp)

    if test "$MOZ_FLAVOUR" != "libxul"; then
        PKG_CHECK_MODULES( MOZILLAXPCOM, $MOZ_FLAVOUR-xpcom, HASXPCOM="TRUE", HASXPCOM="" )
        MOZ_INC=`$PKG_CONFIG --variable=includedir $MOZ_FLAVOUR-xpcom`
        MOZ_LIB=`$PKG_CONFIG --variable=libdir $MOZ_FLAVOUR-xpcom`
    fi

    if test -z "$HASXPCOM"; then
        PKG_CHECK_MODULES( MOZILLAXPCOM, libxul )
        MOZ_INC=`$PKG_CONFIG --variable=includedir libxul`
        MOZ_LIB=`$PKG_CONFIG --variable=libdir libxul`
        if ! test -e "$MOZ_LIB/libxul.so"; then
            MOZ_LIB=`$PKG_CONFIG --variable=sdkdir libxul`
            if test -e "$MOZ_LIB/sdk/lib/libxul.so"; then
                MOZ_LIB="$MOZ_LIB/sdk/lib"
            fi
        fi
    fi

    MOZ_LIB_XPCOM=$MOZILLAXPCOM_LIBS

    #e.g. http://fedoraproject.org/wiki/Releases/FeatureXULRunnerAPIChanges
    #the plugin pkg-config etc. reverts to "mozilla-plugin" with libxul
    if test "$MOZ_FLAVOUR" = "libxul"; then
        MOZ_FLAVOUR="mozilla"
    fi

elif test "$enable_mozilla" = "no"; then
    AC_MSG_RESULT([none])
    WITH_MOZILLA=NO
else
    AC_MSG_RESULT([internal])
    SYSTEM_MOZILLA=NO
    BUILD_TYPE="$BUILD_TYPE MOZ"
    MOZ_TARBALL=a169ab152209200a7bad29a275cb0333-seamonkey-1.1.14.source.tar.gz
    MOZILLA_VERSION=1.1.14
    AC_SUBST(MOZILLA_VERSION)

    AC_MSG_CHECKING([for toolkit Mozilla should use])
    if test -z "$with_mozilla_toolkit"; then
        if test "$_os" != "WINNT"; then
            if test "$_os" = "Darwin"; then
                MOZILLA_TOOLKIT=mac
                AC_MSG_RESULT([mac])
            else
                MOZILLA_TOOLKIT=gtk2
                AC_MSG_RESULT([gtk2])
            fi
        fi
    else
        MOZILLA_TOOLKIT=$with_mozilla_toolkit
        enable_build_mozilla=1
        AC_MSG_RESULT([$MOZILLA_TOOLKIT])
    fi

    AC_SUBST(MOZILLA_TOOLKIT)

    # default to enabling build mozilla
    if test "$enable_build_mozilla" != "no"; then
        enable_build_mozilla=yes
    else
        enable_build_mozilla=
    fi

    AC_MSG_CHECKING([whether to build Mozilla/SeaMonkey])
    if test -n "$enable_build_mozilla"; then
        BUILD_MOZAB="TRUE"
        AC_MSG_RESULT([yes])
    else
        BUILD_MOZAB=""
        AC_MSG_RESULT([no])
    fi

    if test "$BUILD_MOZAB" = "TRUE"; then
        if test "$_os" = "WINNT"; then
            if test "$WITH_MINGW" != "yes"; then
                # compiling with MSVC. Only supported platform here is MSVS2005 at the moment.
                if test "$MSVSVER" != "2005"; then
                    AC_MSG_ERROR([Building SeaMonkey is supported with Microsoft Visual Studio 2005 only.])
                fi
            else
                AC_MSG_WARN([Building SeaMonkey with MinGW is not tested, and likely to break.])
                add_warning "Building SeaMonkey with MinGW is not tested, and likely to break."
            fi
        fi

        if test "$_os" = "WINNT"; then
            AC_MSG_CHECKING([for moztools binaries])
            if test ! -e "$TARFILE_LOCATION/vc8-moztools.zip"; then
                AC_MSG_ERROR([The following file is missing in $TARFILE_LOCATION: vc8-moztools.zip
(from ftp://ftp.mozilla.org/pub/mozilla.org/mozilla/libraries/win32/historic/vc8/)])
            else
                AC_MSG_RESULT([ok])
            fi
        elif test "$_os" = "Darwin"; then
            PKG_CHECK_MODULES(MOZLIBREQ, libIDL-2.0 >= 0.6.3,  MOZIDL="TRUE", MOZIDL="")
            if test -z "$MOZIDL"; then
                AC_MSG_ERROR([libIDL 0.6.3 or newer is needed to build Mozilla with Mac toolkit.])
            fi
        else
            # Generic Unix/Linux section
            if test "$MOZILLA_TOOLKIT" = "gtk2"; then
                PKG_CHECK_MODULES(MOZLIBREQ, gtk+-2.0, MOZGTK="TRUE", MOZGTK="")
                if test -z "$MOZGTK"; then
                    AC_MSG_ERROR([GTK2 is needed to build Mozilla.])
                fi
                PKG_CHECK_MODULES(MOZLIBREQ, libIDL-2.0 >= 0.8.0, MOZIDL="TRUE", MOZIDL="")
                if test -z "$MOZIDL"; then
                    AC_MSG_ERROR([libIDL >= 0.8.0 is needed when using GTK2 to build Mozilla.])
                fi
            else
                PKG_CHECK_MODULES(MOZLIBREQ, gtk+ >= 1.2.3, MOZGTK="TRUE", MOZGTK="")
                if test -z "$MOZGTK"; then
                    AC_MSG_ERROR([GTK+ 1.2 is needed when not using GTK2 to build Mozilla.])
                fi
                PKG_CHECK_MODULES(MOZLIBREQ, libidl >= 0.6.3 libidl <= 0.6.8, MOZIDL="TRUE", MOZIDL="")
                if test -z "$MOZIDL"; then
                    AC_MSG_ERROR([libIDL 0.6.3 - 0.6.8 is needed when not using GTK2 to build Mozilla.])
                fi
            fi
        fi
    else
        AC_MSG_CHECKING([which pre-built Mozilla module in moz/zipped])
        if test -e "$SRC_ROOT/moz/zipped/${OS}${COM}${CPU}inc.zip" -a \
                -e "$SRC_ROOT/moz/zipped/${OS}${COM}${CPU}lib.zip" -a \
                -e "$SRC_ROOT/moz/zipped/${OS}${COM}${CPU}runtime.zip"; then
            AC_MSG_RESULT([Found])
            PREBUILD_MOZAB="YES"
            if test "$_os" = "WINNT"; then
                AC_MSG_CHECKING([for MSVC 2005 runtime needed for prebuilt Mozilla])
                if test -e "$SRC_ROOT/external/msvcp80/Microsoft.VC80.CRT.manifest" -a \
                        -e "$SRC_ROOT/external/msvcp80/msvcp80.dll" -a \
                        -e "$SRC_ROOT/external/msvcp80/msvcr80.dll"; then
                    AC_MSG_RESULT([Found])
                else
                    AC_MSG_RESULT([Not Found])
                    AC_MSG_ERROR([For the pre-built Mozilla libraries, you need MSVC 2005 runtime files
Microsoft.VC80.CRT.manifest, msvcp80.dll, and msvcr80.dll
in external/msvcp80.

You can get them by installing:
<http://download.microsoft.com/download/8/B/4/8B42259F-5D70-43F4-AC2E-4B208FD8D66A/vcredist_x86.EXE>
and copying the abovementioned files from:
c:/windows/winsxs/x86_Microsoft.VC80.CRT_1fc8b3b9a1e18e3b_8.0.50727.4053_x-ww_e6967989/
and
c:/windows/winsxs/manifests/x86_Microsoft.VC80.CRT_1fc8b3b9a1e18e3b_8.0.50727.4053_x-ww_e6967989.manifest
])
                fi
            fi
        else
            AC_MSG_RESULT([Not Found])
            AC_MSG_ERROR([If you don't use system mozilla and also disable the build of internal mozilla, you must provided the appropriate libraries yourself.
You need to copy $OS$COM$CPU{inc,lib,runtime}.zip into moz/zipped.
These files can be found at <http://dev-www.libreoffice.org/mozilla/>
])
        fi
    fi
fi

if test "$SYSTEM_MOZILLA" = "YES"; then
    SOLARLIB="$SOLARLIB -L$MOZ_LIB"
fi

AC_SUBST(BUILD_MOZAB)
AC_SUBST(PREBUILD_MOZAB)
AC_SUBST(MOZILLABUILD)
AC_SUBST(SYSTEM_MOZILLA)
AC_SUBST(MOZ_FLAVOUR)
AC_SUBST(MOZ_INC)
AC_SUBST(MOZ_LIB)
AC_SUBST(MOZ_LIB_XPCOM)
AC_SUBST(MOZ_TARBALL)

dnl ===================================================================
dnl Check for system NSS
dnl ===================================================================
AC_MSG_CHECKING([which NSS to use])
if test "$with_system_nss" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_NSS="YES"
    PKG_CHECK_MODULES(NSS, nss >= 3.9.3 nspr >= 4.8)
else
    SYSTEM_NSS="NO"
    BUILD_TYPE="$BUILD_TYPE NSS"
    AC_MSG_RESULT([internal])
    if test "$build_os" = "cygwin"; then
        AC_MSG_CHECKING([for Mozilla build tooling. Required for nss and/or mozilla])
        if test -z "$MOZILLABUILD"; then
            AC_MSG_ERROR([Mozilla build tooling not found.
Use the --with-mozilla-build option after installing the tools obtained
from http://ftp.mozilla.org/pub/mozilla.org/mozilla/libraries/win32])
        else
            if test \( "$WITH_MINGW" = "yes" \); then
                if test ! -d "$MOZILLABUILD"; then
                    AC_MSG_ERROR([Mozilla build tooling incomplete!])
                else
                    AC_MSG_RESULT([ok])
                fi
            else
                if test ! -d "$MOZILLABUILD/moztools" \
                        -o ! -d "$MOZILLABUILD/msys"; then
                    AC_MSG_ERROR([Mozilla build tooling incomplete!])
                else
                    AC_MSG_RESULT([ok])
                fi
            fi
        fi
    fi
fi # system nss
AC_SUBST(SYSTEM_NSS)
AC_SUBST(NSS_CFLAGS)
AC_SUBST(NSS_LIBS)

dnl ===================================================================
dnl Check for system mozilla headers
dnl ===================================================================
HAVE_NON_CONST_NPP_GETMIMEDESCRIPTION=
AC_MSG_CHECKING([which Mozilla headers to use])

if test "$with_system_mozilla_headers" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_MOZILLA_HEADERS=YES
    # First try npapi-sdk:
    PKG_CHECK_MODULES(MOZILLA_HEADERS, npapi-sdk, [LOCATED=yes], [LOCATED=no])
    # Then go with libxul:
    if test "x$LOCATED" != "xyes"; then
        PKG_CHECK_MODULES(MOZILLA_HEADERS, libxul, [LOCATED=yes], [LOCATED=no])
    fi
    if test "x$LOCATED" != "xyes"; then
        PKG_CHECK_MODULES(MOZILLA_HEADERS, mozilla-plugin, [LOCATED=yes], [LOCATED=no])
    fi
    # if still not found bail out
    if test "x$LOCATED" != "xyes"; then
        AC_MSG_ERROR([npapi.h header file not found])
    fi

    AC_LANG_PUSH([C])
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS $MOZILLA_HEADERS_CFLAGS"
    AC_MSG_CHECKING([for NPP_GetMIMEDescription return type])
    AC_COMPILE_IFELSE(
        [AC_LANG_SOURCE([[
            #define XP_UNIX
            #include <npapi.h>
            const char* NPP_GetMIMEDescription(void) { return "foo"; }
            ]])],
        [AC_MSG_RESULT([const char*])],
        [
        AC_MSG_RESULT([char*])
        HAVE_NON_CONST_NPP_GETMIMEDESCRIPTION=TRUE
        ])
    CFLAGS=$save_CFLAGS
    AC_LANG_POP([C])
else
    if test "$enable_mozilla" = "no"; then
        AC_MSG_RESULT([none])
    else
        AC_MSG_RESULT([internal])
    fi
    SYSTEM_MOZILLA_HEADERS=NO
fi
AC_SUBST(MOZILLA_HEADERS_CFLAGS)
AC_SUBST(SYSTEM_MOZILLA_HEADERS)
AC_SUBST(HAVE_NON_CONST_NPP_GETMIMEDESCRIPTION)

dnl ===================================================================
dnl Check for system sane
dnl ===================================================================
AC_MSG_CHECKING([which sane header to use])
if test "$with_system_sane" = "yes"; then
    AC_MSG_RESULT([external])
    AC_CHECK_HEADER(sane/sane.h, [],
      [AC_MSG_ERROR(sane not found. install sane)], [])
else
    AC_MSG_RESULT([internal])
    BUILD_TYPE="$BUILD_TYPE SANE"
fi

dnl ===================================================================
dnl Check for system icu
dnl ===================================================================
SYSTEM_GENBRK=
SYSTEM_GENCCODE=
SYSTEM_GENCMN=

ICU_MAJOR=49
ICU_MINOR=1
ICU_MICRO=1
ICU_RECLASSIFIED_CLOSE_PARENTHESIS="YES"
ICU_RECLASSIFIED_PREPEND_SET_EMPTY="YES"
ICU_RECLASSIFIED_CONDITIONAL_JAPANESE_STARTER="YES"
ICU_RECLASSIFIED_HEBREW_LETTER="YES"
AC_MSG_CHECKING([which icu to use])
if test "$with_system_icu" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_ICU=YES
    AC_LANG_PUSH([C++])
    AC_MSG_CHECKING([for unicode/rbbi.h])
    AC_PREPROC_IFELSE([AC_LANG_SOURCE([[unicode/rbbi.h]])],[AC_MSG_RESULT(checked.)],[AC_MSG_ERROR(icu headers not found.)])
    AC_LANG_POP([C++])

    ICUPATH="$PATH"
    if test "$WITH_MINGW" = "yes"; then
        ICUPATH=`$CC -print-sysroot`/mingw/bin:"$ICUPATH"
    fi
    AC_PATH_PROG(ICUCONFIG,icu-config,,[$ICUPATH])

    AC_MSG_CHECKING([ICU version])
    ICU_VERSION=`$ICUCONFIG --noverify --version 2>/dev/null || $ICUCONFIG --version`
    ICU_MAJOR=`echo $ICU_VERSION | cut -d"." -f1`
    ICU_MINOR=`echo $ICU_VERSION | cut -d"." -f2`
    ICU_MICRO=`echo $ICU_VERSION | cut -d"." -f3`

    if test "$ICU_MAJOR" -ge "4"; then
        AC_MSG_RESULT([OK, $ICU_VERSION])
    else
        AC_MSG_ERROR([not suitable, only >= 4.0 supported currently])
    fi

    if test "$cross_compiling" = "yes" -a \( "$with_system_icu_for_build" = "yes" -o "$with_system_icu_for_build" = "force" \); then
        AC_PATH_PROG([ICUCONFIG_FOR_BUILD], [icu-config], [], [$PATH:/usr/sbin:/sbin])
        ICU_VERSION_FOR_BUILD=`$ICUCONFIG_FOR_BUILD --noverify --version 2>/dev/null || $ICUCONFIG_FOR_BUILD --version`
        ICU_MAJOR_FOR_BUILD=`echo $ICU_VERSION_FOR_BUILD | cut -d"." -f1`
        ICU_MINOR_FOR_BUILD=`echo $ICU_VERSION_FOR_BUILD | cut -d"." -f2`
        AC_MSG_CHECKING([if MinGW and system versions of ICU are compatible])
        if test "$ICU_MAJOR" -eq "$ICU_MAJOR_FOR_BUILD" -a "$ICU_MINOR" -eq "$ICU_MINOR_FOR_BUILD"; then
            AC_MSG_RESULT([yes])
        else
            AC_MSG_RESULT([no])
            if test "$with_system_icu_for_build" != "force"; then
                AC_MSG_ERROR([System ICU is not version-compatible with MinGW ICU.
You can use --with-system-icu-for-build=force to use it anyway.])
            fi
        fi
    fi

    if test "$cross_compiling" != "yes" -o "$with_system_icu_for_build" = "yes" -o "$with_system_icu_for_build" = "force"; then
        # using the system icu tools can lead to version confusion, use the
        # ones from the build environment when cross-compiling
        AC_PATH_PROG(SYSTEM_GENBRK, genbrk, [], [$PATH:/usr/sbin:/sbin])
        if test -z "$SYSTEM_GENBRK"; then
            AC_MSG_ERROR([\'genbrk\' not found in \$PATH, install the icu development tool \'genbrk\'])
        fi
        AC_PATH_PROG(SYSTEM_GENCCODE, genccode, [], [$PATH:/usr/sbin:/sbin:/usr/local/sbin])
        if test -z "$SYSTEM_GENCCODE"; then
            AC_MSG_ERROR([\'genccode\' not found in \$PATH, install the icu development tool \'genccode\'])
        fi
        AC_PATH_PROG(SYSTEM_GENCMN, gencmn, [], [$PATH:/usr/sbin:/sbin:/usr/local/sbin])
        if test -z "$SYSTEM_GENCMN"; then
            AC_MSG_ERROR([\'gencmn\' not found in \$PATH, install the icu development tool \'gencmn\'])
        fi
        if test "$ICU_MAJOR" -ge "49" -o \( "$ICU_MAJOR" = "4" -a "$ICU_MINOR" -ge "4" \); then
            ICU_RECLASSIFIED_CLOSE_PARENTHESIS="YES"
        else
            ICU_RECLASSIFIED_CLOSE_PARENTHESIS="NO"
        fi
        if test "$ICU_MAJOR" -ge "49"; then
            ICU_RECLASSIFIED_PREPEND_SET_EMPTY="YES"
        else
            ICU_RECLASSIFIED_PREPEND_SET_EMPTY="NO"
        fi
        if test "$ICU_MAJOR" -le 4; then
            ICU_RECLASSIFIED_CONDITIONAL_JAPANESE_STARTER="NO"
            ICU_RECLASSIFIED_HEBREW_LETTER="NO"
        fi
    fi

    libo_MINGW_CHECK_DLL([ICUDATA], [icudata][$ICU_MAJOR][$ICU_MINOR])
    libo_MINGW_CHECK_DLL([ICUI18N], [icui18n][$ICU_MAJOR][$ICU_MINOR])
    libo_MINGW_CHECK_DLL([ICUUC], [icuuc][$ICU_MAJOR][$ICU_MINOR])
else
    AC_MSG_RESULT([internal])
    SYSTEM_ICU="NO"
    BUILD_TYPE="$BUILD_TYPE ICU"
fi

AC_SUBST(SYSTEM_ICU)
AC_SUBST(SYSTEM_GENBRK)
AC_SUBST(SYSTEM_GENCCODE)
AC_SUBST(SYSTEM_GENCMN)
AC_SUBST(ICU_MAJOR)
AC_SUBST(ICU_MINOR)
AC_SUBST(ICU_MICRO)
AC_SUBST(ICU_RECLASSIFIED_CLOSE_PARENTHESIS)
AC_SUBST([ICU_RECLASSIFIED_CONDITIONAL_JAPANESE_STARTER])
AC_SUBST([ICU_RECLASSIFIED_HEBREW_LETTER])
AC_SUBST(ICU_RECLASSIFIED_PREPEND_SET_EMPTY)
AC_SUBST([MINGW_ICUDATA_DLL])
AC_SUBST([MINGW_ICUI18N_DLL])
AC_SUBST([MINGW_ICUUC_DLL])

dnl ===================================================================
dnl Graphite
dnl ===================================================================

AC_MSG_CHECKING([whether to enable graphite support])
if test "$_os" = "WINNT" -o "$_os" = "Linux" && test "$enable_graphite" = "" -o "$enable_graphite" != "no"; then
    AC_MSG_RESULT([yes])
    ENABLE_GRAPHITE="TRUE"
    AC_MSG_CHECKING([which graphite to use])
    if test "$with_system_graphite" = "yes"; then
        AC_MSG_RESULT([external])
        SYSTEM_GRAPHITE=YES
        PKG_CHECK_MODULES( GRAPHITE, graphite2 >= 0.9.3 )
        libo_MINGW_CHECK_DLL([GRAPHITE2], [libgraphite2])
    else
        AC_MSG_RESULT([internal])
        SYSTEM_GRAPHITE=NO
        BUILD_TYPE="$BUILD_TYPE GRAPHITE"
        GRAPHITE_TARBALL=f5ef3f7f10fa8c3542c6a085a233080b-graphite2-1.2.0.tgz
    fi
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_GRAPHITE)
AC_SUBST(SYSTEM_GRAPHITE)
AC_SUBST(GRAPHITE_CFLAGS)
AC_SUBST(GRAPHITE_LIBS)
AC_SUBST(GRAPHITE_TARBALL)
AC_SUBST([MINGW_GRAPHITE2_DLL])

dnl ===================================================================
dnl Check for nsplugin
dnl ===================================================================
AC_MSG_CHECKING([whether to build nsplugin extension])
ENABLE_NSPLUGIN=NO
if test $_os = Darwin -o $_os = iOS -o $_os = Android -o "$enable_headless" = "yes"; then
    enable_gtk=no
fi
if test "$enable_nsplugin" != "no"; then
    if test "$enable_gtk" != "no"; then
        AC_MSG_RESULT([yes])
        ENABLE_NSPLUGIN=YES
    else
        AC_MSG_RESULT([no, disabled gtk2])
    fi
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_NSPLUGIN)

AC_MSG_CHECKING([whether to use X11])
dnl ***************************************
dnl testing for X libraries and includes...
dnl ***************************************
WANT_X11="no"
if test $_os != Darwin -a $_os != WINNT -a $_os != Android -a $_os != iOS -a "$enable_headless" != "yes"; then
    WANT_X11="yes"
fi
AC_MSG_RESULT([$WANT_X11])

if test "$WANT_X11" = "yes"; then
    AC_PATH_X
    AC_PATH_XTRA
    CPPFLAGS="$CPPFLAGS $X_CFLAGS"

    if test "x$x_includes" = "x"; then
        x_includes="default_x_includes"
    fi
    if test "x$x_libraries" = "x"; then
        x_libraries="default_x_libraries"
    fi
    dnl The variables $x_libraries and $x_includes are set.
    if test -z "$x_libraries"; then
        AC_MSG_ERROR([No X libraries found]) # Exit
    fi
    if test -z "$x_includes"; then
        AC_MSG_ERROR([No X includes found]) # Exit
    fi
    CFLAGS="$CFLAGS $X_CFLAGS"
    LDFLAGS="$LDFLAGS $X_LDFLAGS $X_LIBS"
    AC_CHECK_LIB(X11, XOpenDisplay, x_libs="-lX11 $X_EXTRA_LIBS", [AC_MSG_ERROR([X Development libraries not found])])
else
    x_includes="no_x_includes"
    x_libraries="no_x_libraries"
fi
if test -z "$x_includes"; then
    x_includes="no_x_includes"
fi
if test -z "$x_libraries"; then
    x_libraries="no_x_libraries"
fi
if test "$x_includes" = "default_x_includes"; then
    XINC="/usr/include"
else
    XINC="$x_includes"
fi
AC_SUBST(XINC)
if test "$x_libraries" = "default_x_libraries"; then
    XLIB=`$PKG_CONFIG --variable=libdir x11`
    if test "x$XLIB" = x; then
        XLIB="/usr/lib"
    fi
else
    XLIB="$x_libraries"
fi
if test "$XLIB" != "no_x_libraries" -a "$XLIB" != "/usr/lib" -a "$XLIB" != "/usr/lib64"; then
    SOLARLIB="$SOLARLIB -L$XLIB"
fi
if test "$XINC" != "no_x_includes" -a "$XINC" != "/usr/include"; then
    SOLARINC="$SOLARINC -I$XINC"
fi
AC_SUBST(XLIB)

if test "$WANT_X11" = "yes"; then
    dnl ===================================================================
    dnl Check for Composite.h for Mozilla plugin
    dnl ===================================================================
    AC_CHECK_HEADERS(X11/Composite.h,[],[AC_MSG_ERROR([Xt headers not found])],
     [#include <X11/Intrinsic.h>])

    dnl ===================================================================
    dnl Check for extension headers
    dnl ===================================================================
    AC_MSG_CHECKING([whether to use internal X11 extensions headers])
    if test "$with_system_xextensions_headers" != "no"; then
        AC_MSG_RESULT([no])
        AC_CHECK_HEADERS(X11/extensions/shape.h,[],[AC_MSG_ERROR([libXext headers not found])],
        [#include <X11/extensions/shape.h>])
    else
        AC_MSG_RESULT([yes])
        BUILD_TYPE="$BUILD_TYPE X11_EXTENSIONS"
    fi

fi

dnl ===================================================================
dnl Check for system Xrender
dnl ===================================================================
AC_MSG_CHECKING([whether to use Xrender])
if test "$WANT_X11" = "yes" -a  "$test_xrender" = "yes"; then
    AC_MSG_RESULT([yes])
    PKG_CHECK_MODULES(XRENDER, xrender)
    AC_CHECK_LIB([Xrender], [XRenderQueryVersion], [:],
      [AC_MSG_ERROR(libXrender not found or functional)], [])
    AC_MSG_CHECKING([which Xrender headers to use])
    if test "$with_system_xextensions_headers" != "no"; then
        AC_MSG_RESULT([external])
        AC_CHECK_HEADER(X11/extensions/Xrender.h, [],
          [AC_MSG_ERROR(Xrender not found. install X)], [])
    else
        AC_MSG_RESULT([internal])
    fi
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(XRENDER_CFLAGS)
AC_SUBST(XRENDER_LIBS)

dnl ===================================================================
dnl Check for XRandr
dnl ===================================================================
AC_MSG_CHECKING([whether to enable RandR support])
if test "$WANT_X11" = "yes" -a "$test_randr" = "yes" -a \( "$enable_randr" = "yes" -o "$enable_randr" = "TRUE" \); then
    if test -z "$enable_randr_link" -o "$enable_randr_link" = "no"; then
        XRANDR_DLOPEN="TRUE"
        AC_MSG_RESULT([configured to dlopen libXrandr at runtime])
    else
        AC_MSG_RESULT([yes])
        XRANDR_DLOPEN="FALSE"
        PKG_CHECK_MODULES(XRANDR, xrandr >= 1.2, ENABLE_RANDR="TRUE", ENABLE_RANDR="")
        if test "$ENABLE_RANDR" != "TRUE"; then
            AC_CHECK_HEADER(X11/extensions/Xrandr.h, [],
                        [AC_MSG_ERROR([X11/extensions/Xrandr.h could not be found. X11 dev missing?])], [])
            XRANDR_CFLAGS=" "
            AC_CHECK_LIB([Xrandr], [XRRQueryExtension], [:],
              [ AC_MSG_ERROR(libXrandr not found or functional) ], [])
            XRANDR_LIBS="-lXrandr "
            ENABLE_RANDR="TRUE"
        fi
    fi
else
    ENABLE_RANDR=""
    AC_MSG_RESULT([no])
fi
AC_SUBST(XRANDR_DLOPEN)
AC_SUBST(XRANDR_CFLAGS)
AC_SUBST(XRANDR_LIBS)
AC_SUBST(ENABLE_RANDR)

dnl ===================================================================
dnl Check for building neon
dnl ===================================================================
AC_MSG_CHECKING([whether to use neon])
if test "$enable_neon" = "no"; then
    AC_MSG_RESULT([no])
    DISABLE_NEON=TRUE
    AC_SUBST(DISABLE_NEON)
else
    AC_MSG_RESULT([yes])
dnl ===================================================================
dnl Check for system neon
dnl ===================================================================
AC_MSG_CHECKING([which neon to use])
if test "$with_system_neon" = "yes"; then
    AC_MSG_RESULT([external])
    PKG_CHECK_MODULES(NEON, neon >= 0.26.0, , AC_MSG_ERROR([you need neon >= 0.26.x for system-neon]))
    NEON_VERSION="`$PKG_CONFIG --modversion neon | $SED 's/\.//g'`"
    SYSTEM_NEON=YES
    libo_MINGW_CHECK_DLL([NEON], [libneon])
    libo_MINGW_TRY_DLL([TASN1], [libtasn1])
    libo_MINGW_TRY_DLL([GNUTLS], [libgnutls])
else
    AC_MSG_RESULT([internal])
    SYSTEM_NEON=NO
    BUILD_TYPE="$BUILD_TYPE NEON"
    NEON_TARBALL=ff369e69ef0f0143beb5626164e87ae2-neon-0.29.5.tar.gz
    NEON_VERSION=0295
fi
AC_SUBST(SYSTEM_NEON)
AC_SUBST(NEON_LIBS)
AC_SUBST(NEON_CFLAGS)
AC_SUBST(NEON_TARBALL)
AC_SUBST(NEON_VERSION)
AC_SUBST([MINGW_GNUTLS_DLL])
AC_SUBST([MINGW_NEON_DLL])
AC_SUBST([MINGW_TASN1_DLL])
fi

dnl ===================================================================
dnl Check for disabling cve_tests
dnl ===================================================================
AC_MSG_CHECKING([whether to execute CVE tests])
if test "$enable_cve_tests" = "no"; then
    AC_MSG_RESULT([no])
    DISABLE_CVE_TESTS=TRUE
    AC_SUBST(DISABLE_CVE_TESTS)
else
    AC_MSG_RESULT([yes])
fi

dnl ===================================================================
dnl Check for system openssl
dnl ===================================================================
if test "$_os" = "Darwin" -o "$_os" = "NetBSD" -o "$_os" = "OpenBSD" -o \
        "$_os" = "DragonFly" && test "$with_system_openssl" != "no"; then
    with_system_openssl=yes
fi
AC_MSG_CHECKING([which libssl to use])
if test "$with_system_openssl" = "yes"; then
    AC_MSG_RESULT([external])
    # Mac OS builds should get out without extra stuff is the Mac porters'
    # wish. And pkg-config is although Xcode ships a .pc for openssl
    if test "$_os" = "Darwin" -o "$_os" = "NetBSD" -o "$_os" = "OpenBSD" -o \
            "$_os" = "DragonFly"; then
        OPENSSL_CFLAGS=
        OPENSSL_LIBS="-lssl -lcrypto"
    else
        PKG_CHECK_MODULES( OPENSSL, openssl )
    fi
    SYSTEM_OPENSSL=YES
    libo_MINGW_CHECK_DLL([SSL], [libssl])
    libo_MINGW_CHECK_DLL([CRYPTO], [libcrypto])
else
    AC_MSG_RESULT([internal])
    SYSTEM_OPENSSL=NO
    BUILD_TYPE="$BUILD_TYPE OPENSSL"
fi
AC_SUBST(SYSTEM_OPENSSL)
AC_SUBST(OPENSSL_CFLAGS)
AC_SUBST(OPENSSL_LIBS)
AC_SUBST([MINGW_SSL_DLL])
AC_SUBST([MINGW_CRYPTO_DLL])

dnl ===================================================================
dnl Check for system redland
dnl ===================================================================
AC_MSG_CHECKING([which redland library to use])
if test "$with_system_redland" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_REDLAND=YES
    dnl versions before 1.0.8 write RDF/XML that is useless for ODF (@xml:base)
    PKG_CHECK_MODULES(REDLAND, redland >= 1.0.8)
    libo_MINGW_CHECK_DLL([REDLAND], [librdf])
    libo_MINGW_TRY_DLL([RAPTOR], [libraptor])
    libo_MINGW_TRY_DLL([RASQAL], [librasqal])
    libo_MINGW_TRY_DLL([SQLITE3], [libsqlite3])
else
    AC_MSG_RESULT([internal])
    BUILD_TYPE="$BUILD_TYPE REDLAND"
    SYSTEM_REDLAND=NO
fi
AC_SUBST(SYSTEM_REDLAND)
AC_SUBST(REDLAND_CFLAGS)
AC_SUBST(REDLAND_LIBS)
AC_SUBST([MINGW_RAPTOR_DLL])
AC_SUBST([MINGW_RASQAL_DLL])
AC_SUBST([MINGW_REDLAND_DLL])
AC_SUBST([MINGW_SQLITE3_DLL])

dnl ===================================================================
dnl Check for system orcus
dnl ===================================================================
AC_MSG_CHECKING([which orcus library to use])
if test "$with_system_orcus" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_LIBORCUS=YES
    PKG_CHECK_MODULES(ORCUS, liborcus-0.2 >= 0.1.0)
else
    AC_MSG_RESULT([internal])
    ORCUS_TARBALL=46d9f4cf8b145c21ce1056e116d2ce71-liborcus_0.1.0.tar.bz2
    SYSTEM_LIBORCUS=NO
fi
AC_SUBST(SYSTEM_LIBORCUS)
AC_SUBST(ORCUS_CFLAGS)
AC_SUBST(ORCUS_LIBS)
AC_SUBST(ORCUS_TARBALL)

dnl ===================================================================
dnl Check for system hunspell
dnl ===================================================================
AC_MSG_CHECKING([which libhunspell to use])
if test "$with_system_hunspell" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_HUNSPELL=YES
    AC_LANG_PUSH([C++])
    PKG_CHECK_MODULES(HUNSPELL, hunspell, HUNSPELL_PC="TRUE", HUNSPELL_PC="" )
    if test "$HUNSPELL_PC" != "TRUE"; then
        AC_CHECK_HEADER(hunspell.hxx, [],
            [
            AC_CHECK_HEADER(hunspell/hunspell.hxx, [ HUNSPELL_CFLAGS=-I/usr/include/hunspell ],
            [AC_MSG_ERROR(hunspell headers not found.)], [])
            ], [])
        AC_CHECK_LIB([hunspell], [main], [:],
           [ AC_MSG_ERROR(hunspell library not found.) ], [])
        HUNSPELL_LIBS=-lhunspell
    fi
    AC_LANG_POP([C++])
    libo_MINGW_CHECK_DLL([HUNSPELL], [libhunspell-1.3])
else
    AC_MSG_RESULT([internal])
    SYSTEM_HUNSPELL=NO
    HUNSPELL_CFLAGS="-I${WORKDIR}/UnpackedTarball/hunspell/src/hunspell"
    HUNSPELL_LIBS="-L${OUTDIR}/lib -lhunspell"
    HUNSPELL_TARBALL="3121aaf3e13e5d88dfff13fb4a5f1ab8-hunspell-1.3.2.tar.gz"
    BUILD_TYPE="$BUILD_TYPE HUNSPELL"
fi
AC_SUBST(SYSTEM_HUNSPELL)
AC_SUBST(HUNSPELL_TARBALL)
AC_SUBST(HUNSPELL_CFLAGS)
AC_SUBST(HUNSPELL_LIBS)
AC_SUBST([MINGW_HUNSPELL_DLL])

dnl ===================================================================
dnl Checking for altlinuxhyph
dnl ===================================================================
AC_MSG_CHECKING([which altlinuxhyph to use])
if test "$with_system_altlinuxhyph" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_HYPH=YES
    AC_CHECK_HEADER(hyphen.h, [],
       [ AC_MSG_ERROR(altlinuxhyph headers not found.)], [])
    AC_CHECK_MEMBER(struct _HyphenDict.cset, [],
       [ AC_MSG_ERROR(no. You are sure you have altlinuyhyph headers?)],
       [#include <hyphen.h>])
    AC_CHECK_LIB(hyphen, hnj_hyphen_hyphenate2, [HYPHEN_LIB=-lhyphen],
        [ AC_MSG_ERROR(altlinuxhyph library not found or too old.)], [])
    if test -z "$HYPHEN_LIB"; then
        AC_CHECK_LIB(hyph, hnj_hyphen_hyphenate2, [HYPHEN_LIB=-lhyph],
           [ AC_MSG_ERROR(altlinuxhyph library not found or too old.)], [])
    fi
    if test -z "$HYPHEN_LIB"; then
        AC_CHECK_LIB(hnj, hnj_hyphen_hyphenate2, [HYPHEN_LIB=-lhnj],
           [ AC_MSG_ERROR(altlinuxhyph library not found or too old.)], [])
    fi
    libo_MINGW_CHECK_DLL([HYPHEN], [libhyphen])
else
    AC_MSG_RESULT([internal])
    SYSTEM_HYPH=NO
    BUILD_TYPE="$BUILD_TYPE HYPHEN"
fi
AC_SUBST(SYSTEM_HYPH)
AC_SUBST(HYPHEN_LIB)
AC_SUBST([MINGW_HYPHEN_DLL])

dnl ===================================================================
dnl Checking for mythes
dnl ===================================================================
AC_MSG_CHECKING([which mythes to use])
if test "$with_system_mythes" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_MYTHES=YES
    AC_LANG_PUSH([C++])
    PKG_CHECK_MODULES(MYTHES, mythes, MYTHES_PKGCONFIG=yes, MYTHES_PKGCONFIG=no)
    if test "$MYTHES_PKGCONFIG" = "no"; then
        AC_CHECK_HEADER(mythes.hxx, [],
            [ AC_MSG_ERROR(mythes.hxx headers not found.)], [])
        AC_CHECK_LIB([mythes-1.2], [main], [:],
            [ MYTHES_FOUND=no], [])
    if test "$MYTHES_FOUND" = "no"; then
        AC_CHECK_LIB(mythes, main, [MYTHES_FOUND=yes],
                [ MYTHES_FOUND=no], [])
    fi
    if test "$MYTHES_FOUND" = "no"; then
        AC_MSG_ERROR([mythes library not found!.])
    fi
    fi
    AC_LANG_POP([C++])
    libo_MINGW_CHECK_DLL([MYTHES], [libmythes-1.2])
else
    AC_MSG_RESULT([internal])
    SYSTEM_MYTHES=NO
    MYTHES_TARBALL="46e92b68e31e858512b680b3b61dc4c1-mythes-1.2.3.tar.gz"
    BUILD_TYPE="$BUILD_TYPE MYTHES"
fi
AC_SUBST(SYSTEM_MYTHES)
AC_SUBST(MYTHES_CFLAGS)
AC_SUBST(MYTHES_TARBALL)
AC_SUBST(MYTHES_LIBS)
AC_SUBST([MINGW_MYTHES_DLL])

dnl ===================================================================
dnl Checking for lpsolve
dnl ===================================================================
AC_MSG_CHECKING([which lpsolve to use])
if test "$with_system_lpsolve" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_LPSOLVE=YES
    AC_CHECK_HEADER(lpsolve/lp_lib.h, [],
       [ AC_MSG_ERROR(lpsolve headers not found.)], [])
    save_LIBS=$LIBS
    # some systems need this. Like Ubuntu....
    AC_CHECK_LIB(m, floor)
    AC_CHECK_LIB(dl, dlopen)
    AC_CHECK_LIB([lpsolve55], [make_lp], [:],
        [ AC_MSG_ERROR(lpsolve library not found or too old.)], [])
    LIBS=$save_LIBS
    libo_MINGW_CHECK_DLL([LPSOLVE], [lpsolve55])
else
    AC_MSG_RESULT([internal])
    SYSTEM_LPSOLVE=NO
    BUILD_TYPE="$BUILD_TYPE LPSOLVE"
fi
AC_SUBST(SYSTEM_LPSOLVE)
AC_SUBST([MINGW_LPSOLVE_DLL])

dnl ===================================================================
dnl Checking for libexttextcat
dnl ===================================================================
AC_MSG_CHECKING([which libexttextcat to use])
if test "$with_system_libexttextcat" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_LIBEXTTEXTCAT=YES
    PKG_CHECK_MODULES([LIBEXTTEXTCAT], [libexttextcat >= 3.1.1])
    SYSTEM_LIBEXTTEXTCAT_DATA=file://`$PKG_CONFIG --variable=pkgdatadir libexttextcat`
else
    AC_MSG_RESULT([internal])
    SYSTEM_LIBEXTTEXTCAT=NO
    BUILD_TYPE="$BUILD_TYPE LIBEXTTEXTCAT"
fi
AC_SUBST(SYSTEM_LIBEXTTEXTCAT)
AC_SUBST(SYSTEM_LIBEXTTEXTCAT_DATA)
AC_SUBST(LIBEXTTEXTCAT_CFLAGS)
AC_SUBST(LIBEXTTEXTCAT_LIBS)

dnl ***************************************
dnl testing libc version for Linux...
dnl ***************************************
if test "$_os" = "Linux"; then
    AC_MSG_CHECKING([whether libc is >= 2.1.1])
    exec 6>/dev/null # no output
    AC_CHECK_LIB(c, gnu_get_libc_version, HAVE_LIBC=yes; export HAVE_LIBC)
    exec 6>&1 # output on again
    if test "$HAVE_LIBC"; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_ERROR([no, upgrade libc])
    fi
fi

if test "$_os" != "WINNT"; then
    AC_CHECK_FUNCS(getopt, HAVE_GETOPT=YES, [HAVE_GETOPT=NO])
    AC_CHECK_FUNCS(readdir_r, HAVE_READDIR_R=YES, [HAVE_READDIR_R=NO])
    if test "$HAVE_GETOPT" = "YES" -a "$HAVE_READDIR_R" = "YES"; then
        SYSTEM_LIBC=YES
    fi
else
    HAVE_GETOPT=NO
    HAVE_READDIR_R=NO
    SYSTEM_LIBC=YES
fi
AC_SUBST(HAVE_GETOPT)
AC_SUBST(HAVE_READDIR_R)
AC_SUBST(SYSTEM_LIBC)

dnl =========================================
dnl Check for the Windows  SDK.
dnl =========================================
dnl FIXME: I don't know yet if the Windows SDK works with MinGW, keep it until I know better,
dnl and add "-a \( "$WITH_MINGW" != "yes" \)" then
if test "$_os" = "WINNT"; then
    AC_MSG_CHECKING([for Windows SDK])
    if test "$build_os" = "cygwin"; then
        if test -z "$with_windows_sdk_home"; then
            # This first line will detect a February 2003 Microsoft Platform SDK
            WINDOWS_SDK_HOME=`./oowintool --windows-sdk-home`
            # But there might be also an April 2005 PSDK, unfortunately MS changed
            # the registry entry. (we prefer the old version!?)
            if test -z "$WINDOWS_SDK_HOME"; then
                WINDOWS_SDK_HOME=`cat /proc/registry/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/MicrosoftSDK/InstalledSDKs/*/Install\ Dir 2> /dev/null | tr '\000' '\n' | head -n 1`
            fi
            # normalize if found
            if test -n "$WINDOWS_SDK_HOME"; then
                WINDOWS_SDK_HOME=`cygpath -d "$WINDOWS_SDK_HOME"`
                WINDOWS_SDK_HOME=`cygpath -u "$WINDOWS_SDK_HOME"`

                #If this sdk is incomplete, lets see if the one
                #recommended to be installed is available
                if test ! -x "$WINDOWS_SDK_HOME/bin/msiinfo.exe"; then
                    WINDOWS_SDK7_HOME=`cat "/proc/registry/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v7.1/InstallationFolder" 2> /dev/null | tr '\000' '\n' | head -n 1`
                    if test -n "$WINDOWS_SDK7_HOME"; then
                        WINDOWS_SDK_HOME=`cygpath -d "$WINDOWS_SDK7_HOME"`
                        WINDOWS_SDK_HOME=`cygpath -u "$WINDOWS_SDK_HOME"`
                    fi
                fi
            fi
        else
            WINDOWS_SDK_HOME=`cygpath -u "$with_windows_sdk_home"`
        fi
    fi
    if test -n "$WINDOWS_SDK_HOME"; then
        # Remove a possible trailing backslash
        WINDOWS_SDK_HOME=`echo $WINDOWS_SDK_HOME | $SED 's/\/$//'`
        # Problem with current PSDK (iz 49865)
        # (What "current" does that mean?)
        if test -f "$WINDOWS_SDK_HOME/Lib/libcp.lib"; then
            AC_MSG_ERROR([
Some modules do not build correctly with MS Platform SDK - April 2005
Edition if the library ($WINDOWS_SDK_HOME/Lib/libcp.lib) is found.
Remove/rename/backup that file and restart configure. Details about this
problem can be found in issue 49856.])
        fi

        if test -f "$WINDOWS_SDK_HOME/Include/adoint.h" \
             -a -f "$WINDOWS_SDK_HOME/Include/SqlUcode.h" \
             -a -f "$WINDOWS_SDK_HOME/Include/usp10.h"; then
            HAVE_PSDK_H="yes"
        elif test -f "$WINDOWS_SDK_HOME/Include/um/adoint.h" \
             -a -f "$WINDOWS_SDK_HOME/Include/um/SqlUcode.h" \
             -a -f "$WINDOWS_SDK_HOME/Include/um/usp10.h"; then
            HAVE_PSDK_H="yes"
        else
            HAVE_PSDK_H="no"
        fi
        if test -f "$WINDOWS_SDK_HOME/lib/user32.lib"; then
            HAVE_PSDK_LIB="yes"
        elif test -f "$WINDOWS_SDK_HOME/lib/win8/um/$SDK_ARCH/user32.lib"; then
            HAVE_PSDK_LIB="yes"
        else
            HAVE_PSDK_LIB="no"
        fi

        if test "$HAVE_PSDK_H" = "no" -o "$HAVE_PSDK_LIB" = "no"; then
            AC_MSG_ERROR([Some (all?) Windows SDK files not found, please check if all needed parts of
the  Windows SDK are installed.])
        fi

        if test ! -x "$WINDOWS_SDK_HOME/bin/msiinfo.exe" \
             -o ! -x "$WINDOWS_SDK_HOME/bin/msidb.exe" \
             -o ! -x "$WINDOWS_SDK_HOME/bin/uuidgen.exe" \
             -o ! -x "$WINDOWS_SDK_HOME/bin/msitran.exe"; then :
        elif test ! -x "$WINDOWS_SDK_HOME/bin/x86/msiinfo.exe" \
             -o ! -x "$WINDOWS_SDK_HOME/bin/x86/msidb.exe" \
             -o ! -x "$WINDOWS_SDK_HOME/bin/x86/uuidgen.exe" \
             -o ! -x "$WINDOWS_SDK_HOME/bin/x86/msitran.exe"; then :
        else
            AC_MSG_ERROR([Some (all) files of the Windows Installer SDK are missing, please install.])
        fi
    fi

    if test -z "$WINDOWS_SDK_HOME"; then
        AC_MSG_RESULT([no, hoping the necessary headers and libraries will be found anyway])
    elif echo $WINDOWS_SDK_HOME | grep "v6.1" >/dev/null 2>/dev/null; then
        WINDOWS_SDK_VERSION=61
        AC_MSG_RESULT([found Windows SDK 6.1 ($WINDOWS_SDK_HOME)])
    elif echo $WINDOWS_SDK_HOME | grep "v6.0" >/dev/null 2>/dev/null; then
        WINDOWS_SDK_VERSION=60
        AC_MSG_RESULT([found Windows SDK 6.0 ($WINDOWS_SDK_HOME)])
    elif echo $WINDOWS_SDK_HOME | grep "v7" >/dev/null 2>/dev/null; then
        WINDOWS_SDK_VERSION=70
        AC_MSG_RESULT([found Windows SDK 7 ($WINDOWS_SDK_HOME)])
    elif echo $WINDOWS_SDK_HOME | grep "8.0" >/dev/null 2>/dev/null; then
        WINDOWS_SDK_VERSION=80
        AC_MSG_RESULT([found Windows SDK 8.0 ($WINDOWS_SDK_HOME)])
    else
        AC_MSG_ERROR([Found legacy Windows Platform SDK ($WINDOWS_SDK_HOME)])
    fi
    PathFormat "$WINDOWS_SDK_HOME"
    WINDOWS_SDK_HOME="$formatted_path"
    if test "$build_os" = "cygwin"; then
        SOLARINC="$SOLARINC -I$WINDOWS_SDK_HOME/include -I$COMPATH/include"
        if test -d "$WINDOWS_SDK_HOME/include/um"; then
            SOLARINC="$SOLARINC -I$WINDOWS_SDK_HOME/include/um -I$WINDOWS_SDK_HOME/include/shared"
        fi
    fi
fi
AC_SUBST(WINDOWS_SDK_HOME)
AC_SUBST(WINDOWS_SDK_VERSION)

dnl =========================================
dnl Check for uuidgen
dnl =========================================
if test "$_os" = "WINNT" -a "$cross_compiling" != "yes"; then
    if test "$WITH_MINGW" = "yes"; then
        AC_PATH_PROG([UUIDGEN], [uuid])
        if test -z "$UUIDGEN"; then
            AC_MSG_WARN([uuid is needed for dev-install])
        fi
    else
        # presence is already tested above in the WINDOWS_SDK_HOME check
        UUIDGEN=uuidgen.exe
        AC_SUBST(UUIDGEN)
    fi
else
    AC_PATH_PROG([UUIDGEN], [uuidgen])
    if test -z "$UUIDGEN"; then
        AC_MSG_WARN([uuid is needed for dev-install])
    fi
fi

dnl =========================================
dnl Check for the Microsoft DirectX SDK.
dnl =========================================
if test -n "$ENABLE_DIRECTX" -a "$_os" = "WINNT"; then
    AC_MSG_CHECKING([for DirectX SDK])
    if test "$build_os" = "cygwin"; then
        if test -z "$with_directx_home"; then
            dnl A standard installation of the DirectX SDK sets $DXSDK_DIR
            if test -n "$DXSDK_DIR"; then
                DIRECTXSDK_HOME=`cygpath -d "$DXSDK_DIR"`
                DIRECTXSDK_HOME=`cygpath -u "$DIRECTXSDK_HOME"`
            fi
            # At this point $DIRECTXSDK_HOME might still be undefined. This will lead to
            # the "DirectX SDK not found" error later
        else
            DIRECTXSDK_HOME=`cygpath -u "$with_directx_home"`
        fi
        # Remove a possible trailing backslash
        DIRECTXSDK_HOME=`echo $DIRECTXSDK_HOME | $SED 's/\/$//'`
    elif test -n "$with_directx_home"; then
        DIRECTXSDK_HOME="$with_directx_home"
    fi

    if test -f "$DIRECTXSDK_HOME/Include/ddraw.h" -o -f "$DIRECTXSDK_HOME/Include/d3d9.h"; then
        HAVE_DIRECTXSDK_H="yes"
    else
        HAVE_DIRECTXSDK_H="no"
    fi

    # MS changed the location of the libraries with Dec 2004 DirectX SDK
    if test -d "$DIRECTXSDK_HOME/lib/x86"; then
        DIRECTXSDK_LIB="$DIRECTXSDK_HOME/lib/x86"
    else
        DIRECTXSDK_LIB="$DIRECTXSDK_HOME/lib"
    fi
    PathFormat "$DIRECTXSDK_LIB"
    DIRECTXSDK_LIB="$formatted_path"

    if test -f "$DIRECTXSDK_LIB/ddraw.lib" -o -f "$DIRECTXSDK_LIB/d3d9.lib"; then
        HAVE_DIRECTXSDK_LIB="yes"
    else
        HAVE_DIRECTXSDK_LIB="no"
    fi

    if test "$HAVE_DIRECTXSDK_H" = "yes" -a "$HAVE_DIRECTXSDK_LIB" = "yes"; then
        AC_MSG_RESULT([found])
    else
        AC_MSG_RESULT([no, hoping the necessary headers and libraries will be found anyway])
    fi
    PathFormat "$DIRECTXSDK_HOME"
    DIRECTXSDK_HOME="$formatted_path"
    SOLARINC="$SOLARINC -I$DIRECTXSDK_HOME/include"
fi
AC_SUBST(DIRECTXSDK_HOME)
AC_SUBST(DIRECTXSDK_LIB)

dnl ***************************************
dnl Checking for bison and flex
dnl ***************************************
AC_PATH_PROG(BISON, bison)
ANCIENT_BISON="NO"
if test -z "$BISON"; then
    AC_MSG_ERROR([no bison found in \$PATH, install it])
else
    AC_MSG_CHECKING([the bison version])
    _bison_version=`$BISON --version | grep GNU | $SED -e 's@^[[^0-9]]*@@' -e 's@ .*@@' -e 's@,.*@@'`
    _bison_longver=`echo $_bison_version | $AWK -F. '{ print \$1*1000+\$2}'`
    # Accept newer than 1.875 or older(equal) than 1.75
    if test "$_bison_longver" -ge 1875 -o "$_bison_longver" -le 1075; then
        if test "$_bison_version" = "1.875"; then
            AC_MSG_WARN([suspect ($BISON $_bison_version)])
            add_warning "Suspect ($BISON $_bison_version) suggest upgrade"
        else
            AC_MSG_RESULT([checked ($BISON $_bison_version)])
        fi
    else
        AC_MSG_ERROR([failed ($BISON $_bison_version need 1.875+ (or 1.75 and older))])
    fi
    if test "$_bison_longver" -lt 2000; then
        ANCIENT_BISON="YES"
    fi
fi
AC_SUBST(ANCIENT_BISON)

AC_PATH_PROG(FLEX, flex)
if test -z "$FLEX"; then
    AC_MSG_ERROR([no flex found in \$PATH, install it])
fi
AC_SUBST([FLEX])
dnl ***************************************
dnl Checking for patch
dnl ***************************************
AC_PATH_PROG(PATCH, patch)
if test -z "$PATCH"; then
    AC_MSG_ERROR([\"patch\" not found in \$PATH, install it])
fi

dnl On Solaris, FreeBSD or MacOS X, check if --with-gnu-patch was used
if test "$_os" = "SunOS" -o "$_os" = "FreeBSD" -o "$_os" = "Darwin"; then
    if test -z "$with_gnu_patch"; then
        GNUPATCH=$PATCH
    else
        if test -x "$with_gnu_patch"; then
            GNUPATCH=$with_gnu_patch
        else
            AC_MSG_ERROR([--with-gnu-patch did not point to an executable])
        fi
    fi

    AC_MSG_CHECKING([whether $GNUPATCH is GNU patch])
    if $GNUPATCH --version | grep "Free Software Foundation" >/dev/null 2>/dev/null; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_ERROR([no, GNU patch needed. install or specify with --with-gnu-patch=/path/to/it])
    fi
else
    GNUPATCH=$PATCH
fi

dnl We also need to check for --with-gnu-cp

if test -z "$with_gnu_cp"; then
    # check the place where the good stuff is hidden on Solaris...
    if test -x /usr/gnu/bin/cp; then
        GNUCP=/usr/gnu/bin/cp
    else
        AC_PATH_PROGS(GNUCP, gnucp cp)
    fi
    if test -z $GNUCP; then
        AC_MSG_ERROR([Neither gnucp nor cp found. Install GNU cp and/or specify --with-gnu-cp=/path/to/it])
    fi
else
    if test -x "$with_gnu_cp"; then
        GNUCP=$with_gnu_cp
    else
        AC_MSG_ERROR([--with-gnu-cp did not point to an executable])
    fi
fi

AC_MSG_CHECKING([whether $GNUCP is GNU cp from coreutils with preserve= support])
if $GNUCP --version 2>/dev/null | grep "coreutils" >/dev/null 2>/dev/null; then
    AC_MSG_RESULT([yes])
elif $GNUCP --version 2>/dev/null | grep "GNU fileutils" >/dev/null 2>/dev/null; then
    AC_MSG_RESULT([yes])
else
    case "$build_os" in
    darwin*|netbsd*|openbsd*|freebsd*|dragonfly*|aix*)
        x_GNUCP=[\#]
        GNUCP=''
        AC_MSG_RESULT([no gnucp found - using the system's cp command])
        ;;
    *)
        AC_MSG_ERROR([no, GNU cp needed. install or specify with --with-gnu-cp=/path/to/it])
        ;;
    esac
fi

AC_SUBST(GNUPATCH)
AC_SUBST(GNUCP)
AC_SUBST(x_GNUCP)

dnl ***************************************
dnl testing assembler path
dnl ***************************************
ML_EXE=""
if test "$_os" = "WINNT" -a "$WITH_MINGW" != "yes"; then
    if test "$BITNESS_OVERRIDE" = ""; then
        assembler=ml.exe
        assembler_bin=bin
    else
        assembler=ml64.exe
        assembler_bin=bin/amd64
    fi
    if test -n "$with_asm_home"; then
        with_asm_home=`cygpath -u "$with_asm_home"`
    fi
    if test -x "$with_asm_home/$assembler"; then
        AC_MSG_CHECKING([$assembler assembler path])
        AC_MSG_RESULT([$with_asm_home/$assembler])
        ML_EXE="$with_asm_home/$assembler"
    else
        AC_PATH_PROG(ML_EXE, $assembler)
        if test -z "$ML_EXE"; then
            AC_MSG_CHECKING([$with_cl_home/$assembler_bin/$assembler])
            if test -x "$with_cl_home/$assembler_bin/$assembler"; then
                with_asm_home=$with_cl_home/$assembler_bin
                AC_MSG_RESULT([found])
                ML_EXE="$with_cl_home/$assembler_bin/$assembler"
            else
                AC_MSG_ERROR([Configure did not find $assembler assembler.])
            fi
        else
            with_asm_home="ASM_IN_PATH"
            ML_EXE="$assembler"
        fi
    fi
    PathFormat "$with_asm_home"
    ASM_HOME="$formatted_path"
else
    ASM_HOME=""
fi

AC_SUBST(ML_EXE)

dnl ===================================================================
dnl We need zip and unzip
dnl ===================================================================
AC_PATH_PROG(ZIP, zip)
test -z "$ZIP" && AC_MSG_ERROR([zip is required])
if ! "$ZIP" --filesync < /dev/null 2>/dev/null > /dev/null; then
    AC_MSG_ERROR([Zip version 3.0 or newer is required to build, please install or use --with-zip-home],,)
fi

AC_PATH_PROG(UNZIP, unzip)
test -z "$UNZIP" && AC_MSG_ERROR([unzip is required])

dnl ===================================================================
dnl Zip must be a specific type for different build types.
dnl ===================================================================
if test $build_os = cygwin; then
    if test -n "`$ZIP -h | $GREP -i WinNT`"; then
        AC_MSG_ERROR([$ZIP is not the required Cygwin version of Info-ZIP's zip.exe.])
    fi
fi

dnl ===================================================================
dnl Test which vclplugs have to be built.
dnl ===================================================================
R=""
GTK3_CFLAGS=""
GTK3_LIBS=""
ENABLE_GTK3=""
if test "x$enable_gtk3" = "xyes"; then
    if test "$with_system_cairo" != yes; then
        AC_MSG_WARN([System cairo required for gtk3 support, please use --with-system-cairo])
        add_warning "System cairo required for gtk3 support, please use --with-system-cairo"
    fi
    PKG_CHECK_MODULES(GTK3, gtk+-3.0 >= 3.2 gtk+-unix-print-3.0 gmodule-no-export-2.0 cairo, ENABLE_GTK3="TRUE", ENABLE_GTK3="")
    if test "x$ENABLE_GTK3" = "xTRUE"; then
        R="gtk3"
    else
        AC_MSG_ERROR([gtk3 libraries of the correct versions, not found])
    fi
fi
AC_SUBST(GTK3_LIBS)
AC_SUBST(GTK3_CFLAGS)
AC_SUBST(ENABLE_GTK3)

AC_MSG_CHECKING([which VCLplugs shall be built])
ENABLE_GTK=""
if test "x$enable_gtk" = "xyes"; then
    ENABLE_GTK="TRUE"
    R="gtk $R"
fi
AC_SUBST(ENABLE_GTK)

ENABLE_TDE=""
if test "x$enable_tde" = "xyes"; then
    # Libs kab and tdeab in connectivity and kdeab and tdeab in shell
    # are built from the same sources. So we only allow one of them.
    if test "x$enable_kde" = "xyes"; then
        AC_MSG_ERROR([enabling both KDE and TDE is not supported])
    fi
    ENABLE_TDE="TRUE"
    R="$R tde"
fi
AC_SUBST(ENABLE_TDE)

ENABLE_KDE=""
if test "x$enable_kde" = "xyes"; then
    ENABLE_KDE="TRUE"
    R="$R kde"
fi
AC_SUBST(ENABLE_KDE)

ENABLE_KDE4=""
if test "x$enable_kde4" = "xyes"; then
    ENABLE_KDE4="TRUE"
    R="$R kde4"
fi
AC_SUBST(ENABLE_KDE4)

ENABLE_HEADLESS=""
if test "x$enable_headless" = "xyes"; then
    ENABLE_HEADLESS="TRUE"
    SCPDEFS="$SCPDEFS -DLIBO_HEADLESS"
    R="headless"
fi
AC_SUBST(ENABLE_HEADLESS)

if test -z "$R"; then
    AC_MSG_RESULT([none])
else
    AC_MSG_RESULT([$R])
fi

dnl ===================================================================
dnl GCONF check
dnl ===================================================================

ENABLE_GCONF=""
AC_MSG_CHECKING([whether to enable GConf support])
if test "$_os" != "WINNT" -a "$_os" != "Darwin" -a "$enable_gconf" = "yes"; then
    ENABLE_GCONF="TRUE"
    AC_MSG_RESULT([yes])
    PKG_CHECK_MODULES( GCONF, gconf-2.0 gobject-2.0 )
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(GCONF_LIBS)
AC_SUBST(GCONF_CFLAGS)
AC_SUBST(ENABLE_GCONF)

dnl ===================================================================
dnl Gnome VFS check
dnl ===================================================================

ENABLE_GNOMEVFS=""
AC_MSG_CHECKING([whether to enable GNOME VFS support])
if test "$_os" != "WINNT" -a "$_os" != "Darwin" -a "$enable_gnome_vfs" = "yes" -a "$enable_gconf" = "yes"; then
    ENABLE_GNOMEVFS="TRUE"
    AC_MSG_RESULT([yes])
    PKG_CHECK_MODULES( GNOMEVFS, gnome-vfs-2.0 >= 2.6.0 )
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(GNOMEVFS_LIBS)
AC_SUBST(GNOMEVFS_CFLAGS)
AC_SUBST(ENABLE_GNOMEVFS)

dnl ===================================================================
dnl check for dbus support
dnl ===================================================================
ENABLE_DBUS=""
DBUS_CFLAGS=""
DBUS_LIBS=""

if test "$enable_dbus" = "no"; then
    test_dbus=no
fi

AC_MSG_CHECKING([whether to enable DBUS support])
if test "$test_dbus" = "yes"; then
    ENABLE_DBUS="TRUE"
    AC_MSG_RESULT([yes])
    PKG_CHECK_MODULES(DBUS, dbus-glib-1 >= 0.70)
else
    AC_MSG_RESULT([no])
fi

AC_SUBST(ENABLE_DBUS)
AC_SUBST(DBUS_CFLAGS)
AC_SUBST(DBUS_LIBS)

AC_MSG_CHECKING([whether to enable font install via packagekit])
if test "$ENABLE_DBUS" = "TRUE"; then
    if test -n "$enable_packagekit" -a "$enable_packagekit" != "no"; then
        ENABLE_PACKAGEKIT=YES
        AC_MSG_RESULT([yes])
    else
        ENABLE_PACKAGEKIT=NO
        AC_MSG_RESULT([no])
    fi
else
    AC_MSG_RESULT([no, dbus disabled.])
fi
AC_SUBST(ENABLE_PACKAGEKIT)

AC_MSG_CHECKING([whether to enable sd remotecontrol])
if test -n "$enable_sdremote" -a "$enable_sdremote" != "no"; then
    AC_MSG_RESULT([yes])
    ENABLE_SDREMOTE=YES
    AC_MSG_CHECKING([whether to enable bluetooth support])
    if test -n "$enable_bluetooth" -a "$enable_bluetooth" != "no"; then
        if test "$OS" = "LINUX"; then
            if test "$ENABLE_DBUS" = "TRUE"; then
				AC_MSG_RESULT([yes])
				ENABLE_BLUETOOTH=YES
                dnl ===================================================================
                dnl Check for system bluez
                dnl ===================================================================
                AC_MSG_CHECKING([which bluetooth header to use])
                if test "$with_system_bluez" = "yes"; then
                    AC_MSG_RESULT([external])
                    AC_CHECK_HEADER(bluetooth/bluetooth.h, [],
                        [AC_MSG_ERROR(bluetooth.h not found. install bluez)], [])
                else
                    AC_MSG_RESULT([internal])
                    BUILD_TYPE="$BUILD_TYPE BLUEZ"
                fi
			else
				AC_MSG_RESULT([no, dbus disabled])
				ENABLE_BLUETOOTH=NO
			fi
		else
			AC_MSG_RESULT([no])
			ENABLE_BLUETOOTH=NO
		fi
	fi
else
    ENABLE_SDREMOTE=NO
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_SDREMOTE)
AC_SUBST(ENABLE_BLUETOOTH)

dnl ===================================================================
dnl Check whether the gtk 2.0 libraries are available.
dnl ===================================================================

GTK_CFLAGS=""
GTK_LIBS=""
ENABLE_SYSTRAY_GTK=""
if test  "$test_gtk" = "yes"; then

    if test "$ENABLE_GTK" = "TRUE"; then
        PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.4 gdk-pixbuf-xlib-2.0 >= 2.2 ,,AC_MSG_ERROR([requirements to build the gtk-plugin not met. Use --disable-gtk or install the missing packages]))
        PKG_CHECK_MODULES(GTHREAD, gthread-2.0,,AC_MSG_ERROR([requirements to build the gtk-plugin not met. Use --disable-gtk or install the missing packages]))
        BUILD_TYPE="$BUILD_TYPE GTK"

        if test "x$enable_systray" = "xyes"; then
            PKG_CHECK_MODULES( GTK210, gtk+-2.0 >= 2.10.0,
                            [ ENABLE_SYSTRAY_GTK="TRUE" ],
                            [ ENABLE_SYSTRAY_GTK="" ])
        fi

        AC_MSG_CHECKING([whether to enable Gtk print dialog support])
        PKG_CHECK_MODULES([GTK_PRINT], [gtk+-unix-print-2.0 >= 2.10.0],
                          [ENABLE_GTK_PRINT="TRUE"],
                          [ENABLE_GTK_PRINT=""])

        AC_MSG_CHECKING([whether to enable GIO support])
        if test "$_os" != "WINNT" -a "$_os" != "Darwin" -a "$enable_gio" = "yes"; then
            if test "$ENABLE_GNOMEVFS" = "TRUE"; then
                AC_MSG_ERROR([please use --enable-gio only together with --disable-gnome-vfs.])
            fi
            ENABLE_GIO="TRUE"
            AC_MSG_RESULT([yes])
            PKG_CHECK_MODULES( GIO, gio-2.0 )
        else
            AC_MSG_RESULT([no])
        fi
    fi
fi
AC_SUBST(ENABLE_GIO)
AC_SUBST(GIO_CFLAGS)
AC_SUBST(GIO_LIBS)
AC_SUBST(ENABLE_SYSTRAY_GTK)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)
AC_SUBST(GTHREAD_CFLAGS)
AC_SUBST(GTHREAD_LIBS)
AC_SUBST([ENABLE_GTK_PRINT])
AC_SUBST([GTK_PRINT_CFLAGS])
AC_SUBST([GTK_PRINT_LIBS])


dnl ===================================================================
dnl Check whether the Telepathy libraries are available.
dnl ===================================================================

ENABLE_TELEPATHY=""
TELEPATHY_CFLAGS=""
TELEPATHY_LIBS=""

AC_MSG_CHECKING([whether to enable Telepathy support])
if test "$_os" != "WINNT" -a "$_os" != "Darwin" -a "$enable_telepathy" = "yes"; then
    ENABLE_TELEPATHY="TRUE"
    AC_MSG_RESULT([yes])
    PKG_CHECK_MODULES( TELEPATHY, telepathy-glib >= 0.18.0 glib-2.0 gobject-2.0 gthread-2.0 gio-2.0 )
else
    AC_MSG_RESULT([no])
fi

AC_SUBST(ENABLE_TELEPATHY)
AC_SUBST(TELEPATHY_CFLAGS)
AC_SUBST(TELEPATHY_LIBS)


dnl ===================================================================

PKG_CHECK_MODULES( LIBPNG, libpng, ENABLE_QUICKSTART_LIBPNG="TRUE", ENABLE_QUICKSTART_LIBPNG="" )
AC_SUBST(LIBPNG_LIBS)
AC_SUBST(LIBPNG_CFLAGS)

AC_MSG_CHECKING([whether to enable libpng linking in quickstarter])
if test "x$enable_unix_qstart_libpng" != "xno"; then
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
    ENABLE_QUICKSTART_LIBPNG=""
fi
AC_SUBST(ENABLE_QUICKSTART_LIBPNG)

SPLIT_APP_MODULES=""
if test "$enable_split_app_modules" = "yes"; then
    SPLIT_APP_MODULES="YES"
fi
AC_SUBST(SPLIT_APP_MODULES)

SPLIT_OPT_FEATURES=""
if test "$enable_split_opt_features" = "yes"; then
    SPLIT_OPT_FEATURES="YES"
fi
AC_SUBST(SPLIT_OPT_FEATURES)

if test $_os = Darwin -o $_os = WINNT; then
    if test "$enable_cairo_canvas" = yes; then
        AC_MSG_ERROR([The cairo canvas should not be used for this platform])
    fi
    enable_cairo_canvas=no
elif test -z "$enable_cairo_canvas"; then
    enable_cairo_canvas=yes
fi

ENABLE_CAIRO_CANVAS=""
if test "$test_cairo" = "yes" -a "$enable_cairo_canvas" = "yes"; then
    ENABLE_CAIRO_CANVAS="TRUE"
fi
AC_SUBST(ENABLE_CAIRO_CANVAS)

dnl ===================================================================
dnl Check whether the GStreamer libraries are available.
dnl ===================================================================

ENABLE_GSTREAMER=""

if test "$build_gstreamer" = "yes"; then

    AC_MSG_CHECKING([whether to enable the new GStreamer 1.0 avmedia backend])
    if test "x$enable_gstreamer" != "xno"; then
        ENABLE_GSTREAMER="TRUE"
        AC_MSG_RESULT([yes])
        PKG_CHECK_MODULES( GSTREAMER, gstreamer-1.0 gstreamer-plugins-base-1.0 gstreamer-video-1.0 )
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_SUBST(GSTREAMER_CFLAGS)
AC_SUBST(GSTREAMER_LIBS)
AC_SUBST(ENABLE_GSTREAMER)


ENABLE_GSTREAMER_0_10=""
if test "$build_gstreamer_0_10" = "yes"; then

    AC_MSG_CHECKING([whether to enable the GStreamer avmedia backend])
    if test "x$enable_gstreamer_0_10" != "xno"; then
        ENABLE_GSTREAMER_0_10="TRUE"
        AC_MSG_RESULT([yes])
        PKG_CHECK_MODULES( [GSTREAMER_0_10], [gstreamer-0.10 gstreamer-plugins-base-0.10 gstreamer-interfaces-0.10],, [
            PKG_CHECK_MODULES(  [GSTREAMER_0_10], [gstreamer-0.10 gstreamer-plugins-base-0.10] )
        ])
    else
        AC_MSG_RESULT([no])
    fi
fi

AC_SUBST(GSTREAMER_0_10_CFLAGS)
AC_SUBST(GSTREAMER_0_10_LIBS)
AC_SUBST(ENABLE_GSTREAMER_0_10)

dnl ===================================================================
dnl Check whether the OpenGL libraries are available
dnl ===================================================================

AC_MSG_CHECKING([whether to build the OpenGL Transitions component])
ENABLE_OPENGL=

if test "$_os" = "Darwin"; then
    # We use frameworks on Mac OS X, no need for detail checks
    ENABLE_OPENGL=TRUE
    SYSTEM_MESA_HEADERS=YES
    AC_MSG_RESULT([yes])
else
    if test "x$enable_opengl" != "xno"; then
        if test "$_os" != "WINNT"; then
            save_LDFLAGS=$LDFLAGS
            LDFLAGS="$LDFLAGS -lm"
            AC_MSG_RESULT([yes])
            AC_CHECK_LIB([GL], [main], [:],
                      [AC_MSG_ERROR(libGL not installed or functional)], [])
            AC_CHECK_LIB([GLU], [main], [:],
                      [AC_MSG_ERROR(libGLU not installed or functional)], [])
            ENABLE_OPENGL=TRUE
            LDFLAGS=$save_LDFLAGS

            dnl ===================================================================
            dnl Check for system Mesa
            dnl ===================================================================
            AC_MSG_CHECKING([which Mesa headers to use])
            if test "$with_system_mesa_headers" = "yes"; then
                AC_MSG_RESULT([external])
                SYSTEM_MESA_HEADERS=YES
                AC_LANG_PUSH(C)
                AC_CHECK_HEADER(GL/glxext.h, [],
                   [ AC_MSG_ERROR(mesa headers not found.)], [#include <GL/glx.h>])
                AC_MSG_CHECKING([whether GL/glxext.h defines PFNGLXBINDTEXIMAGEEXTPROC])
                AC_EGREP_HEADER(PFNGLXBINDTEXIMAGEEXTPROC, GL/glxext.h, [AC_MSG_RESULT([yes])], AC_MSG_ERROR(no))
                AC_LANG_POP(C)
            else
                AC_MSG_RESULT([internal])
                SYSTEM_MESA_HEADERS=NO
                BUILD_TYPE="$BUILD_TYPE MESA"
            fi
        else
            AC_MSG_RESULT([not on Windows])
        fi
    else
       AC_MSG_RESULT([no])
    fi
fi

AC_SUBST(SYSTEM_MESA_HEADERS)
AC_SUBST(ENABLE_OPENGL)

# presenter minimizer extension?
AC_MSG_CHECKING([whether to build the Presentation Minimizer extension])
if test "x$enable_ext_presenter_minimizer" != "xno" -a "x$enable_extension_integration" != "xno"; then
   AC_MSG_RESULT([yes])
   ENABLE_MINIMIZER=YES
else
   AC_MSG_RESULT([no])
   ENABLE_MINIMIZER=NO
   SCPDEFS="$SCPDEFS -DWITHOUT_EXTENSION_MINIMIZER"
fi
AC_SUBST(ENABLE_MINIMIZER)

# presenter console extension?
AC_MSG_CHECKING([whether to build the Presenter Console extension])
if test "x$enable_ext_presenter_console" != "xno" -a "x$enable_extension_integration" != "xno"; then
   AC_MSG_RESULT([yes])
   ENABLE_PRESENTER_SCREEN=YES
else
   AC_MSG_RESULT([no])
   ENABLE_PRESENTER_SCREEN=NO
   SCPDEFS="$SCPDEFS -DWITHOUT_EXTENSION_PRESENTER_SCREEN"
fi
AC_SUBST(ENABLE_PRESENTER_SCREEN)

# pdf import extension?
AC_MSG_CHECKING([whether to build the PDF Import extension])
if test "x$enable_ext_pdfimport" != "xno" -a "x$enable_extension_integration" != "xno"; then
  AC_MSG_RESULT([yes])
  ENABLE_PDFIMPORT=YES

  dnl ===================================================================
  dnl Check for system poppler
  dnl ===================================================================
  AC_MSG_CHECKING([which pdf backend to use])
  if test "$with_system_poppler" = "yes"; then
      AC_MSG_RESULT([external])
      SYSTEM_POPPLER=YES
      PKG_CHECK_MODULES( POPPLER, poppler >= 0.8.0 )
      # sdext test fails with 0.20.0
      PKG_CHECK_MODULES( POPPLER_NOT_0_20_0, poppler < 0.20.0,
        , [AC_MSG_ERROR([Poppler 0.20.x is broken, do not use --with-system-poppler or use a different version])])
      AC_LANG_PUSH([C++])
      save_CXXFLAGS=$CXXFLAGS
      save_CPPFLAGS=$CPPFLAGS
      CXXFLAGS="$CXXFLAGS $POPPLER_CFLAGS"
      CPPFLAGS="$CPPFLAGS $POPPLER_CFLAGS"
      AC_CHECK_HEADER([cpp/poppler-version.h], [],
                   [AC_MSG_ERROR([cpp/poppler-version.h not found. Install poppler])], [])
      CXXFLAGS=$save_CXXFLAGS
      CPPFLAGS=$save_CPPFLAGS
      AC_LANG_POP([C++])
  else
      AC_MSG_RESULT([internal])
      SYSTEM_POPPLER=NO
      BUILD_TYPE="$BUILD_TYPE XPDF"
  fi
else
  AC_MSG_RESULT([no])
  ENABLE_PDFIMPORT=NO
  SCPDEFS="$SCPDEFS -DWITHOUT_EXTENSION_PDFIMPORT"
fi
AC_SUBST(ENABLE_PDFIMPORT)
AC_SUBST(SYSTEM_POPPLER)
AC_SUBST(POPPLER_CFLAGS)
AC_SUBST(POPPLER_LIBS)


if test "$ENABLE_PRESENTER_SCREEN" = "YES" -o "$ENABLE_MINIMIZER" = "YES" -o "$ENABLE_PDFIMPORT" = "YES"; then
   BUILD_TYPE="$BUILD_TYPE SDEXT"
fi

AC_MSG_CHECKING([whether to build the Wiki Publisher extension])
if test "x$enable_ext_wiki_publisher" = "xyes" -a "x$enable_extension_integration" != "xno" -a "$with_java" != "no"; then
  AC_MSG_RESULT([yes])
  ENABLE_MEDIAWIKI=YES
  XSLTML_TARBALL=a7983f859eafb2677d7ff386a023bc40-xsltml_2.1.2.zip
  if test  "x$with_java" = "xno"; then
    AC_MSG_ERROR([Wiki Publisher requires Java! Enable Java if you want to build it.])
  fi
else
  AC_MSG_RESULT([no])
  ENABLE_MEDIAWIKI=NO
  SCPDEFS="$SCPDEFS -DWITHOUT_EXTENSION_MEDIAWIKI"
fi
AC_SUBST(ENABLE_MEDIAWIKI)
AC_SUBST(XSLTML_TARBALL)

if test "$ENABLE_MEDIAWIKI" = "YES"; then
    AC_MSG_CHECKING([which Servlet API Jar to use])
    if test "$with_system_servlet_api" = "yes"; then
        AC_MSG_RESULT([external])
        SYSTEM_SERVLETAPI=YES
        if test -z "$SERVLETAPI_JAR"; then
            SERVLETAPI_JAR=/usr/share/java/servlet-api.jar
        fi
        AC_CHECK_FILE($SERVLETAPI_JAR, [],
              [AC_MSG_ERROR(servlet-api.jar not found.)], [])
    else
        AC_MSG_RESULT([internal])
        SYSTEM_SERVLETAPI=NO
        BUILD_TYPE="$BUILD_TYPE TOMCAT"
        TOMCAT_TARBALL="2a177023f9ea8ec8bd00837605c5df1b-jakarta-tomcat-5.0.30-src.tar.gz"
    fi
fi
AC_SUBST(SYSTEM_SERVLETAPI)
AC_SUBST(SERVLETAPI_JAR)

AC_MSG_CHECKING([whether to build the Report Builder extension])
if test "$enable_ext_report_builder" != "no" -a "x$enable_extension_integration" != "xno" -a "$with_java" != "no"; then
    AC_MSG_RESULT([yes])
    ENABLE_REPORTBUILDER=YES
    AC_MSG_CHECKING([which jfreereport libs to use])
    if test "$with_system_jfreereport" = "yes"; then
        SYSTEM_JFREEREPORT=YES
        AC_MSG_RESULT([external])
        if test -z $SAC_JAR; then
            SAC_JAR=/usr/share/java/sac.jar
        fi
        AC_CHECK_FILE($SAC_JAR, [],
             [AC_MSG_ERROR(sac.jar not found.)], [])

        if test -z $LIBXML_JAR; then
            AC_CHECK_FILE(/usr/share/java/libxml-1.0.0.jar,
                [ LIBXML_JAR=/usr/share/java/libxml-1.0.0.jar ],
                [
                    AC_CHECK_FILE(/usr/share/java/libxml.jar,
                       [ LIBXML_JAR=/usr/share/java/libxml.jar ],
                       [AC_MSG_ERROR(libxml.jar replacement not found.)]
                    )
                ]
            )
        else
            AC_CHECK_FILE($LIBXML_JAR, [],
                 [AC_MSG_ERROR(libxml.jar not found.)], [])
        fi

        if test -z $FLUTE_JAR; then
            AC_CHECK_FILE(/usr/share/java/flute-1.3.0.jar,
                [ FLUTE_JAR=/usr/share/java/flute-1.3.0.jar ],
                [
                    AC_CHECK_FILE(/usr/share/java/flute.jar,
                        [ FLUTE_JAR=/usr/share/java/flute.jar ],
                        [ AC_MSG_ERROR(flute-1.3.0.jar replacement not found.)]
                    )
                ]
            )
        else
            AC_CHECK_FILE($FLUTE_JAR, [],
                 [AC_MSG_ERROR(flute-1.3.0.jar not found.)], [])
        fi

        if test -z $JFREEREPORT_JAR; then
            AC_CHECK_FILE(/usr/share/java/flow-engine-0.9.2.jar,
                [ JFREEREPORT_JAR=/usr/share/java/flow-engine-0.9.2.jar ],
                [
                    AC_CHECK_FILE(/usr/share/java/flow-engine.jar,
                        [ JFREEREPORT_JAR=/usr/share/java/flow-engine.jar ],
                        [AC_MSG_ERROR(jfreereport.jar replacement not found.)]
                    )
                ]
            )
        else
            AC_CHECK_FILE($JFREEREPORT_JAR, [],
                 [AC_MSG_ERROR(jfreereport.jar not found.)], [])
        fi

        if test -z $LIBLAYOUT_JAR; then
            AC_CHECK_FILE(/usr/share/java/liblayout-0.2.9.jar,
                [ LIBLAYOUT_JAR=/usr/share/java/liblayout-0.2.9.jar ],
                [
                    AC_CHECK_FILE(/usr/share/java/liblayout.jar,
                        [ LIBLAYOUT_JAR=/usr/share/java/liblayout.jar ],
                        [AC_MSG_ERROR(liblayout.jar replacement not found.)]
                    )
                ]
            )
        else
            AC_CHECK_FILE($LIBLAYOUT_JAR, [],
                 [AC_MSG_ERROR(liblayout.jar not found.)], [])
        fi

        if test -z $LIBLOADER_JAR; then
            AC_CHECK_FILE(/usr/share/java/libloader-1.0.0.jar,
                [ LIBLOADER_JAR=/usr/share/java/libloader-1.0.0.jar ],
                [
                    AC_CHECK_FILE(/usr/share/java/libloader.jar,
                       [ LIBLOADER_JAR=/usr/share/java/libloader.jar ],
                       [AC_MSG_ERROR(libloader.jar replacement not found.)]
                    )
                ]
            )
        else
            AC_CHECK_FILE($LIBLOADER_JAR, [],
                [AC_MSG_ERROR(libloader.jar not found.)], [])
        fi

        if test -z $LIBFORMULA_JAR; then
            AC_CHECK_FILE(/usr/share/java/libformula-0.2.0.jar,
                 [ LIBFORMULA_JAR=/usr/share/java/libformula-0.2.0.jar ],
                 [
                     AC_CHECK_FILE(/usr/share/java/libformula.jar,
                         [ LIBFORMULA_JAR=/usr/share/java/libformula.jar ],
                         [AC_MSG_ERROR(libformula.jar replacement not found.)]
                     )
                 ]
            )
        else
            AC_CHECK_FILE($LIBFORMULA_JAR, [],
                [AC_MSG_ERROR(libformula.jar not found.)], [])
        fi

        if test -z $LIBREPOSITORY_JAR; then
            AC_CHECK_FILE(/usr/share/java/librepository-1.0.0.jar,
                [ LIBREPOSITORY_JAR=/usr/share/java/librepository-1.0.0.jar ],
                [
                    AC_CHECK_FILE(/usr/share/java/librepository.jar,
                        [ LIBREPOSITORY_JAR=/usr/share/java/librepository.jar ],
                        [AC_MSG_ERROR(librepository.jar replacement not found.)]
                    )
                ]
            )
        else
            AC_CHECK_FILE($LIBREPOSITORY_JAR, [],
                [AC_MSG_ERROR(librepository.jar not found.)], [])
        fi

        if test -z $LIBFONTS_JAR; then
            AC_CHECK_FILE(/usr/share/java/libfonts-1.0.0.jar,
                [ LIBFONTS_JAR=/usr/share/java/libfonts-1.0.0.jar ],
                [
                    AC_CHECK_FILE(/usr/share/java/libfonts.jar,
                        [ LIBFONTS_JAR=/usr/share/java/libfonts.jar ],
                        [AC_MSG_ERROR(libfonts.jar replacement not found.)]
                    )
                ]
            )
        else
            AC_CHECK_FILE($LIBFONTS_JAR, [],
                [AC_MSG_ERROR(libfonts.jar not found.)], [])
        fi

        if test -z $LIBSERIALIZER_JAR; then
            AC_CHECK_FILE(/usr/share/java/libserializer-1.0.0.jar,
                [ LIBSERIALIZER_JAR=/usr/share/java/libserializer-1.0.0.jar ],
                [
                    AC_CHECK_FILE(/usr/share/java/libserializer.jar,
                        [ LIBSERIALIZER_JAR=/usr/share/java/libserializer.jar ],
                        [AC_MSG_ERROR(libserializer.jar replacement not found.)]
                    )
                ]
            )
        else
            AC_CHECK_FILE($LIBSERIALIZER_JAR, [],
                [AC_MSG_ERROR(libserializer.jar not found.)], [])
        fi

        if test -z $LIBBASE_JAR; then
            AC_CHECK_FILE(/usr/share/java/libbase-1.0.0.jar,
                [ LIBBASE_JAR=/usr/share/java/libbase-1.0.0.jar ],
                [
                    AC_CHECK_FILE(/usr/share/java/libbase.jar,
                        [ LIBBASE_JAR=/usr/share/java/libbase.jar ],
                        [AC_MSG_ERROR(libbase.jar replacement not found.)]
                    )
                ]
            )
        else
            AC_CHECK_FILE($LIBBASE_JAR, [],
                [AC_MSG_ERROR(libbase.jar not found.)], [])
        fi

    else
        AC_MSG_RESULT([internal])
        SYSTEM_JFREEREPORT=NO
        BUILD_TYPE="$BUILD_TYPE JFREEREPORT"
        JFREEREPORT_SAC_TARBALL="39bb3fcea1514f1369fcfc87542390fd-sacjava-1.3.zip"
        JFREEREPORT_LIBBASE_TARBALL="eeb2c7ddf0d302fba4bfc6e97eac9624-libbase-1.1.6.zip"
        JFREEREPORT_FLUTE_TARBALL="d8bd5eed178db6e2b18eeed243f85aa8-flute-1.1.6.zip"
        JFREEREPORT_LIBLOADER_TARBALL="97b2d4dba862397f446b217e2b623e71-libloader-1.1.6.zip"
        JFREEREPORT_LIBXML_TARBALL="ace6ab49184e329db254e454a010f56d-libxml-1.1.7.zip"
        JFREEREPORT_LIBFORMULA_TARBALL="3404ab6b1792ae5f16bbd603bd1e1d03-libformula-1.1.7.zip"
        JFREEREPORT_LIBFONTS_TARBALL="3bdf40c0d199af31923e900d082ca2dd-libfonts-1.1.6.zip"
        JFREEREPORT_LIBREPOSITORY_TARBALL="8ce2fcd72becf06c41f7201d15373ed9-librepository-1.1.6.zip"
        JFREEREPORT_LIBSERIALIZER_TARBALL="f94d9870737518e3b597f9265f4e9803-libserializer-1.1.6.zip"
        JFREEREPORT_FLOW_ENGINE_TARBALL="ba2930200c9f019c2d93a8c88c651a0f-flow-engine-0.9.4.zip"
        JFREEREPORT_LIBLAYOUT_TARBALL="db60e4fde8dd6d6807523deb71ee34dc-liblayout-0.2.10.zip"
    fi
else
    AC_MSG_RESULT([no])
    ENABLE_REPORTBUILDER=NO
    SCPDEFS="$SCPDEFS -DWITHOUT_EXTENSION_REPORTDESIGN"
    SYSTEM_JFREEREPORT=NO
fi
AC_SUBST(ENABLE_REPORTBUILDER)
AC_SUBST(SYSTEM_JFREEREPORT)
AC_SUBST(SAC_JAR)
AC_SUBST(JFREEREPORT_SAC_TARBALL)
AC_SUBST(JFREEREPORT_LIBBASE_TARBALL)
AC_SUBST(JFREEREPORT_FLUTE_TARBALL)
AC_SUBST(JFREEREPORT_LIBLOADER_TARBALL)
AC_SUBST(JFREEREPORT_LIBXML_TARBALL)
AC_SUBST(JFREEREPORT_LIBFORMULA_TARBALL)
AC_SUBST(JFREEREPORT_LIBFONTS_TARBALL)
AC_SUBST(JFREEREPORT_LIBREPOSITORY_TARBALL)
AC_SUBST(JFREEREPORT_LIBSERIALIZER_TARBALL)
AC_SUBST(JFREEREPORT_FLOW_ENGINE_TARBALL)
AC_SUBST(JFREEREPORT_LIBLAYOUT_TARBALL)
AC_SUBST(LIBXML_JAR)
AC_SUBST(FLUTE_JAR)
AC_SUBST(JFREEREPORT_JAR)
AC_SUBST(LIBBASE_JAR)
AC_SUBST(LIBLAYOUT_JAR)
AC_SUBST(LIBLOADER_JAR)
AC_SUBST(LIBFORMULA_JAR)
AC_SUBST(LIBREPOSITORY_JAR)
AC_SUBST(LIBFONTS_JAR)
AC_SUBST(LIBSERIALIZER_JAR)

# this has to be here because both the Wiki Publisher and the SRB use
# commons-logging
if test "$ENABLE_MEDIAWIKI" = "YES" -o "$ENABLE_REPORTBUILDER" = "YES"; then
    AC_MSG_CHECKING([which Apache commons-* libs to use])
    if test "$with_system_apache_commons" = "yes"; then
        SYSTEM_APACHE_COMMONS=YES
        AC_MSG_RESULT([external])
        if test "$ENABLE_MEDIAWIKI" = "YES"; then
            if test -z $COMMONS_CODEC_JAR; then
                AC_CHECK_FILE(/usr/share/java/commons-codec-1.3.jar,
                    [ COMMONS_CODEC_JAR=/usr/share/java/commons-codec-1.3.jar ],
                    [
                        AC_CHECK_FILE(/usr/share/java/commons-codec.jar,
                            [ COMMONS_CODEC_JAR=/usr/share/java/commons-codec.jar ],
                            [AC_MSG_ERROR(commons-codec.jar replacement not found.)]
                        )
                    ]
                )
            else
                AC_CHECK_FILE($COMMONS_CODEC_JAR, [],
                    [AC_MSG_ERROR(commons-codec.jar not found.)], [])
            fi

            if test -z $COMMONS_LANG_JAR; then
                AC_CHECK_FILE(/usr/share/java/commons-lang-2.3.jar,
                    [ COMMONS_LANG_JAR=/usr/share/java/commons-lang-2.3.jar ],
                    [
                        AC_CHECK_FILE(/usr/share/java/commons-lang.jar,
                            [ COMMONS_LANG_JAR=/usr/share/java/commons-lang.jar ],
                            [AC_MSG_ERROR(commons-lang.jar replacement not found.)]
                            )
                    ]
                )
            else
                AC_CHECK_FILE($COMMONS_LANG_JAR, [],
                    [AC_MSG_ERROR(commons-lang.jar not found.)], [])
            fi

            if test -z $COMMONS_HTTPCLIENT_JAR; then
                AC_CHECK_FILE(/usr/share/java/commons-httpclient-3.1.jar,
                    [ COMMONS_HTTPCLIENT_JAR=/usr/share/java/commons-httpclient-3.1.jar ],
                    [
                        AC_CHECK_FILE(/usr/share/java/commons-httpclient.jar,
                            [ COMMONS_HTTPCLIENT_JAR=/usr/share/java/commons-httpclient.jar ],
                            [AC_MSG_ERROR(commons-httpclient.jar replacement not found.)]
                        )
                    ]
                )
            else
                AC_CHECK_FILE($COMMONS_HTTPCLIENT_JAR, [],
                    [AC_MSG_ERROR(commons-httpclient.jar not found.)], [])
            fi
        fi
        if test "$ENABLE_MEDIAWIKI" = "YES" -o "$ENABLE_REPORTBUILDER" = "YES"; then
            if test -z $COMMONS_LOGGING_JAR; then
                AC_CHECK_FILE(/usr/share/java/commons-logging-1.1.1.jar,
                   [ COMMONS_LOGGING_JAR=/usr/share/java/commons-logging-1.1.1.jar ],
                   [
                        AC_CHECK_FILE(/usr/share/java/commons-logging.jar,
                            [ COMMONS_LOGGING_JAR=/usr/share/java/commons-logging.jar ],
                            [AC_MSG_ERROR(commons-logging.jar replacement not found.)]
                        )
                    ]
                )
            else
                AC_CHECK_FILE($COMMONS_LOGGING_JAR, [],
                    [AC_MSG_ERROR(commons-logging.jar not found.)], [])
            fi
        fi
    else
        AC_MSG_RESULT([internal])
        SYSTEM_APACHE_COMMONS=NO
        APACHE_COMMONS_CODEC_TARBALL=af3c3acf618de6108d65fcdc92b492e1-commons-codec-1.3-src.tar.gz
        APACHE_COMMONS_HTTPCLIENT_TARBALL=2c9b0f83ed5890af02c0df1c1776f39b-commons-httpclient-3.1-src.tar.gz
        APACHE_COMMONS_LANG_TARBALL=2ae988b339daec234019a7066f96733e-commons-lang-2.3-src.tar.gz
        APACHE_COMMONS_LOGGING_TARBALL=3c219630e4302863a9a83d0efde889db-commons-logging-1.1.1-src.tar.gz
        TOMCAT_TARBALL="2a177023f9ea8ec8bd00837605c5df1b-jakarta-tomcat-5.0.30-src.tar.gz"
        BUILD_TYPE="$BUILD_TYPE APACHE_COMMONS TOMCAT"
    fi
fi
AC_SUBST(APACHE_COMMONS_CODEC_TARBALL)
AC_SUBST(APACHE_COMMONS_HTTPCLIENT_TARBALL)
AC_SUBST(APACHE_COMMONS_LANG_TARBALL)
AC_SUBST(APACHE_COMMONS_LOGGING_TARBALL)
AC_SUBST(SYSTEM_APACHE_COMMONS)
AC_SUBST(COMMONS_CODEC_JAR)
AC_SUBST(COMMONS_LANG_JAR)
AC_SUBST(COMMONS_HTTPCLIENT_JAR)
AC_SUBST(COMMONS_LOGGING_JAR)
AC_SUBST(TOMCAT_TARBALL)

# scripting provider for BeanShell?
AC_MSG_CHECKING([whether to build support for scripts in BeanShell])
if test "${enable_scripting_beanshell}" != "no" -a "x$with_java" != "xno"; then
   AC_MSG_RESULT([yes])
   ENABLE_SCRIPTING_BEANSHELL=YES

   dnl ===================================================================
   dnl Check for system beanshell
   dnl ===================================================================
   AC_MSG_CHECKING([which beanshell to use])
   if test "$with_system_beanshell" = "yes"; then
       AC_MSG_RESULT([external])
       SYSTEM_BSH=YES
       if test -z $BSH_JAR; then
           BSH_JAR=/usr/share/java/bsh.jar
       fi
       AC_CHECK_FILE($BSH_JAR, [],
                  [AC_MSG_ERROR(bsh.jar not found.)], [])
   else
       AC_MSG_RESULT([internal])
       SYSTEM_BSH=NO
       BSH_TARBALL="ea570af93c284aa9e5621cd563f54f4d-bsh-2.0b1-src.tar.gz"
       BUILD_TYPE="$BUILD_TYPE BSH"
   fi
else
   AC_MSG_RESULT([no])
   ENABLE_SCRIPTING_BEANSHELL=NO
   SCPDEFS="$SCPDEFS -DWITHOUT_SCRIPTING_BEANSHELL"
fi
AC_SUBST(ENABLE_SCRIPTING_BEANSHELL)
AC_SUBST(SYSTEM_BSH)
AC_SUBST(BSH_JAR)
AC_SUBST(BSH_TARBALL)

# scripting provider for JavaScript?
AC_MSG_CHECKING([whether to build support for scripts in JavaScript])
if test "${enable_scripting_javascript}" != "no" -a "x$with_java" != "xno"; then
   AC_MSG_RESULT([yes])
   ENABLE_SCRIPTING_JAVASCRIPT=YES

   dnl ===================================================================
   dnl Check for system rhino
   dnl ===================================================================
   AC_MSG_CHECKING([which rhino to use])
   if test "$with_system_rhino" = "yes"; then
       AC_MSG_RESULT([external])
       SYSTEM_RHINO=YES
       if test -z $RHINO_JAR; then
           RHINO_JAR=/usr/share/java/js.jar
       fi
       AC_CHECK_FILE($RHINO_JAR, [],
                  [AC_MSG_ERROR(js.jar not found.)], [])
   else
       AC_MSG_RESULT([internal])
       SYSTEM_RHINO=NO
       RHINO_TARBALL="798b2ffdc8bcfe7bca2cf92b62caf685-rhino1_5R5.zip"
       BUILD_TYPE="$BUILD_TYPE RHINO"
   fi
else
   AC_MSG_RESULT([no])
   ENABLE_SCRIPTING_JAVASCRIPT=NO
   SCPDEFS="$SCPDEFS -DWITHOUT_SCRIPTING_JAVASCRIPT"
fi
AC_SUBST(ENABLE_SCRIPTING_JAVASCRIPT)
AC_SUBST(SYSTEM_RHINO)
AC_SUBST(RHINO_JAR)
AC_SUBST(RHINO_TARBALL)

dnl Scripting provider for Python extension?
dnl We always provide this unless we have disabled Python completely
if test $enable_python = no; then
    ENABLE_SCRIPTING_PYTHON=NO
    SCPDEFS="$SCPDEFS -DWITHOUT_EXTENSION_SCRIPTING_PYTHON"
else
    ENABLE_SCRIPTING_PYTHON=YES
fi
AC_SUBST(ENABLE_SCRIPTING_PYTHON)

supports_multilib=
case "$host_cpu" in
x86_64 | powerpc64 | s390x)
    if test "$SIZEOF_LONG" = "8"; then
        supports_multilib="yes"
    fi
    ;;
*)
    ;;
esac

dnl ===================================================================
dnl Check whether the TQt and TDE libraries are available.
dnl ===================================================================

TDE_CFLAGS=""
TDE_LIBS=""
if test "$_os" != "OpenBSD"; then
    MOC="moc"
fi
if test "$test_tde" = "yes" -a "$ENABLE_TDE" = "TRUE"; then
    dnl Search paths for TQt and TDE
    if test -z "$supports_multilib"; then
        tqt_incdirs="$QTINC /usr/local/tqt3/include /usr/include/tqt3 /usr/include /usr/X11R6/include/X11/tqt3 /usr/X11R6/include/tqt3 /usr/lib/tqt3/include /usr/lib/tqt3/include /usr/share/tqt3/include /usr/local/include/X11/tqt3 $x_includes"
        tqt_libdirs="$QTLIB /usr/local/tqt3/lib /usr/lib/tqt3 /usr/lib /usr/X11R6/lib/X11/tqt3 /usr/X11R6/lib/tqt3 /usr/lib/tqt3/lib /usr/lib/tqt3/lib /usr/share/tqt3/lib /usr/local/lib/tqt3 $x_libraries"
    else
        tqt_incdirs="$QTINC /usr/local/tqt3/include /usr/include/tqt3 /usr/include /usr/X11R6/include/X11/tqt3 /usr/X11R6/include/tqt3 /usr/lib64/tqt3/include /usr/lib64/tqt3/include /usr/share/tqt3/include /usr/lib/tqt3/include /usr/lib/tqt3/include /usr/local/include/X11/tqt3 $x_includes"
        tqt_libdirs="$QTLIB /usr/local/tqt3/lib64 /usr/lib64/tqt3 /usr/lib64 /usr/X11R6/lib64/X11/tqt3 /usr/X11R6/lib64/tqt3 /usr/lib64/tqt3/lib64 /usr/lib64/tqt3/lib64 /usr/share/tqt3/lib64 /usr/local/tqt3/lib /usr/lib/tqt3 /usr/lib /usr/X11R6/lib/X11/tqt3 /usr/X11R6/lib/tqt3 /usr/lib/tqt3/lib /usr/lib/tqt3/lib /usr/share/tqt3/lib /usr/local/lib/tqt3 $x_libraries"
    fi
    if test -n "$TQTDIR"; then
        tqt_incdirs="$TQTDIR/include $tqt_incdirs"
        if test -z "$supports_multilib"; then
            tqt_libdirs="$TQTDIR/lib $tqt_libdirs"
        else
            tqt_libdirs="$TQTDIR/lib64 $TQTDIR/lib $tqt_libdirs"
        fi
    fi
    if test -z "$supports_multilib"; then
        tde_incdirs="/usr/lib/trinity/include /usr/local/trinity/include /usr/local/include /usr/trinity/include /usr/include/trinity /opt/trinity/include /opt/trinity/include $x_includes"
        tde_libdirs="/usr/lib/trinity/lib /usr/local/trinity/lib /usr/trinity/lib /usr/lib/trinity /usr/lib/trinity /usr/X11R6/lib /usr/local/lib /opt/trinity/lib /opt/trinity/lib /usr/X11R6/trinity/lib /usr/lib $x_libraries"
    else
        tde_incdirs="/usr/lib64/trinity/include /usr/lib/trinity/include /usr/local/trinity/include /usr/local/include /usr/trinity/include /usr/include/trinity /opt/trinity/include /opt/trinity/include $x_includes"
        tde_libdirs="/usr/lib64/trinity/lib64 /usr/local/trinity/lib64 /usr/trinity/lib64 /usr/lib64/trinity /usr/lib64/trinity /usr/X11R6/lib64 /usr/local/lib64 /opt/trinity/lib64 /opt/trinity/lib64 /usr/X11R6/trinity/lib64 /usr/lib/trinity/lib /usr/local/trinity/lib /usr/trinity/lib /usr/lib/trinity /usr/lib/trinity /usr/lib /usr/X11R6/lib /usr/local/lib /opt/trinity/lib /opt/trinity/lib /usr/X11R6/trinity/lib /usr/lib64 $x_libraries"
    fi
    if test -n "$TDEDIR"; then
        tde_incdirs="$TDEDIR/include $tde_incdirs"
        if test -z "$supports_multilib"; then
            tde_libdirs="$TDEDIR/lib $tde_libdirs"
        else
            tde_libdirs="$TDEDIR/lib64 $TDEDIR/lib $tde_libdirs"
        fi
    fi

    dnl What to test
    tqt_test_include="ntqstyle.h"
    tde_test_include="kapp.h"

    if test "$_os" != "OpenBSD"; then
        tqt_test_library="libtqt-mt.so"
        tde_test_library="libDCOP.so"
    else
        tqt_test_library="libtqt-mt.so*"
        tde_test_library="libDCOP.so*"
    fi

    dnl Check for TQt headers
    AC_MSG_CHECKING([for TQt headers])
    tqt_incdir="no"
    for tde_check in $tqt_incdirs; do
        if test -r "$tde_check/$tqt_test_include"; then
            tqt_incdir="$tde_check"
            break
        fi
    done
    AC_MSG_RESULT([$tqt_incdir])
    if test "x$tqt_incdir" = "xno"; then
        AC_MSG_ERROR([TQt headers not found.  Please specify the root of
your TQt installation by exporting TQTDIR before running "configure".])
    fi

    dnl Check for TQt libraries
    AC_MSG_CHECKING([for TQt libraries])
    tqt_libdir="no"
    for tqt_check in $tqt_libdirs; do
        if test -r "`ls $tqt_check/$tqt_test_library 2>/dev/null | head -1`"; then
            tqt_libdir="$tqt_check"
            break
        fi
    done
    AC_MSG_RESULT([$tqt_libdir])
    if test "x$tqt_libdir" = "xno"; then
        AC_MSG_ERROR([TQt libraries not found.  Please specify the root of
your TQt installation by exporting TQTDIR before running "configure".])
    fi

    dnl Check for Meta Object Compiler
    AC_PATH_PROG( MOC, moc, no, [`dirname $tqt_libdir`/bin:$TQTDIR/bin:$PATH] )
    if test "$MOC" = "no"; then
        AC_MSG_ERROR([TQt Meta Object Compiler not found.  Please specify
the root of your TQt installation by exporting TQTDIR before running "configure".])
    fi

    dnl Check for TDE headers
    AC_MSG_CHECKING([for TDE headers])
    tde_incdir="no"
    for tde_check in $tde_incdirs; do
        if test -r "$tde_check/$tde_test_include"; then
            tde_incdir="$tde_check"
            break
        fi
    done
    AC_MSG_RESULT([$tde_incdir])
    if test "x$tde_incdir" = "xno"; then
        AC_MSG_ERROR([TDE headers not found.  Please specify the root of
your TDE installation by exporting TDEDIR before running "configure".])
    fi

    dnl Check for TDE libraries
    AC_MSG_CHECKING([for TDE libraries])
    tde_libdir="no"
    for tde_check in $tde_libdirs; do
        if test -r "`ls $tde_check/$tde_test_library 2>/dev/null | head -1`"; then
            tde_libdir="$tde_check"
            break
        fi
    done
    AC_MSG_RESULT([$tde_libdir])
    if test "x$tde_libdir" = "xno"; then
        AC_MSG_ERROR([TDE libraries not found.  Please specify the root of
your TDE installation by exporting TDEDIR before running "configure".])
    fi

    dnl Set the variables
    TDE_CFLAGS="-I$tqt_incdir -I$tde_incdir -I/usr/include/tqt -DQT_CLEAN_NAMESPACE -DQT_THREAD_SUPPORT"
    TDE_LIBS="-L$tde_libdir -L$tqt_libdir -lkio -ltdeui -ltdecore -ltqt -ltqt-mt"
fi
AC_SUBST(TDE_CFLAGS)
AC_SUBST(TDE_LIBS)
AC_SUBST(MOC)

dnl ===================================================================
dnl Check whether the Qt3 and KDE3 libraries are available.
dnl ===================================================================

KDE_CFLAGS=""
KDE_LIBS=""
if test "$_os" != "OpenBSD"; then
    MOC="moc"
fi
if test "$test_kde" = "yes" -a "$ENABLE_KDE" = "TRUE"; then
    dnl Search paths for Qt3 and KDE3
    if test -z "$supports_multilib"; then
        qt_incdirs="$QTINC /usr/local/qt/include /usr/include/qt /usr/include /usr/X11R6/include/X11/qt /usr/X11R6/include/qt /usr/lib/qt3/include /usr/lib/qt/include /usr/share/qt3/include /usr/local/include/X11/qt3 $x_includes"
        qt_libdirs="$QTLIB /usr/local/qt/lib /usr/lib/qt /usr/lib /usr/X11R6/lib/X11/qt /usr/X11R6/lib/qt /usr/lib/qt3/lib /usr/lib/qt/lib /usr/share/qt3/lib /usr/local/lib/qt3 $x_libraries"
    else
        qt_incdirs="$QTINC /usr/local/qt/include /usr/include/qt /usr/include /usr/X11R6/include/X11/qt /usr/X11R6/include/qt /usr/lib64/qt3/include /usr/lib64/qt/include /usr/share/qt3/include /usr/lib/qt3/include /usr/lib/qt/include /usr/local/include/X11/qt3 $x_includes"
        qt_libdirs="$QTLIB /usr/local/qt/lib64 /usr/lib64/qt /usr/lib64 /usr/X11R6/lib64/X11/qt /usr/X11R6/lib64/qt /usr/lib64/qt3/lib64 /usr/lib64/qt/lib64 /usr/share/qt3/lib64 /usr/local/qt/lib /usr/lib/qt /usr/lib /usr/X11R6/lib/X11/qt /usr/X11R6/lib/qt /usr/lib/qt3/lib /usr/lib/qt/lib /usr/share/qt3/lib /usr/local/lib/qt3 $x_libraries"
    fi
    if test -n "$QTDIR"; then
        qt_incdirs="$QTDIR/include $qt_incdirs"
        if test -z "$supports_multilib"; then
            qt_libdirs="$QTDIR/lib $qt_libdirs"
        else
            qt_libdirs="$QTDIR/lib64 $QTDIR/lib $qt_libdirs"
        fi
    fi
    if test -z "$supports_multilib"; then
        kde_incdirs="/usr/lib/kde/include /usr/local/kde/include /usr/local/include /usr/kde/include /usr/include/kde /opt/kde3/include /opt/kde/include $x_includes"
        kde_libdirs="/usr/lib/kde/lib /usr/local/kde/lib /usr/kde/lib /usr/lib/kde /usr/lib/kde3 /usr/X11R6/lib /usr/local/lib /opt/kde3/lib /opt/kde/lib /usr/X11R6/kde/lib /usr/lib $x_libraries"
    else
        kde_incdirs="/usr/lib64/kde/include /usr/lib/kde/include /usr/local/kde/include /usr/local/include /usr/kde/include /usr/include/kde /opt/kde3/include /opt/kde/include $x_includes"
        kde_libdirs="/usr/lib64/kde/lib64 /usr/local/kde/lib64 /usr/kde/lib64 /usr/lib64/kde /usr/lib64/kde3 /usr/X11R6/lib64 /usr/local/lib64 /opt/kde3/lib64 /opt/kde/lib64 /usr/X11R6/kde/lib64 /usr/lib/kde/lib /usr/local/kde/lib /usr/kde/lib /usr/lib/kde /usr/lib/kde3 /usr/lib /usr/X11R6/lib /usr/local/lib /opt/kde3/lib /opt/kde/lib /usr/X11R6/kde/lib /usr/lib64 $x_libraries"
    fi
    if test -n "$KDEDIR"; then
        kde_incdirs="$KDEDIR/include $kde_incdirs"
        if test -z "$supports_multilib"; then
            kde_libdirs="$KDEDIR/lib $kde_libdirs"
        else
            kde_libdirs="$KDEDIR/lib64 $KDEDIR/lib $kde_libdirs"
        fi
    fi

    dnl What to test
    qt_test_include="qstyle.h"
    kde_test_include="kapp.h"

    if test "$_os" != "OpenBSD"; then
        qt_test_library="libqt-mt.so"
        kde_test_library="libDCOP.so"
    else
        qt_test_library="libqt-mt.so*"
        kde_test_library="libDCOP.so*"
    fi

    dnl Check for Qt3 headers
    AC_MSG_CHECKING([for Qt3 headers])
    qt_incdir="no"
    for kde_check in $qt_incdirs; do
        if test -r "$kde_check/$qt_test_include"; then
            qt_incdir="$kde_check"
            break
        fi
    done
    AC_MSG_RESULT([$qt_incdir])
    if test "x$qt_incdir" = "xno"; then
        AC_MSG_ERROR([Qt3 headers not found.  Please specify the root of
your Qt3 installation by exporting QTDIR before running "configure".])
    fi

    dnl Check for Qt3 libraries
    AC_MSG_CHECKING([for Qt3 libraries])
    qt_libdir="no"
    for qt_check in $qt_libdirs; do
        if test -r "`ls $qt_check/$qt_test_library 2>/dev/null | head -1`"; then
            qt_libdir="$qt_check"
            break
        fi
    done
    AC_MSG_RESULT([$qt_libdir])
    if test "x$qt_libdir" = "xno"; then
        AC_MSG_ERROR([Qt3 libraries not found.  Please specify the root of
your Qt3 installation by exporting QTDIR before running "configure".])
    fi

    dnl Check for Meta Object Compiler
    AC_PATH_PROG( MOC, moc, no, [`dirname $qt_libdir`/bin:$QTDIR/bin:$PATH] )
    if test "$MOC" = "no"; then
        AC_MSG_ERROR([Qt3 Meta Object Compiler not found.  Please specify
the root of your Qt3 installation by exporting QTDIR before running "configure".])
    fi

    dnl Check for KDE3 headers
    AC_MSG_CHECKING([for KDE3 headers])
    kde_incdir="no"
    for kde_check in $kde_incdirs; do
        if test -r "$kde_check/$kde_test_include"; then
            kde_incdir="$kde_check"
            break
        fi
    done
    AC_MSG_RESULT([$kde_incdir])
    if test "x$kde_incdir" = "xno"; then
        AC_MSG_ERROR([KDE3 headers not found.  Please specify the root of
your KDE3 installation by exporting KDEDIR before running "configure".])
    fi

    dnl Check for KDE3 libraries
    AC_MSG_CHECKING([for KDE3 libraries])
    kde_libdir="no"
    for kde_check in $kde_libdirs; do
        if test -r "`ls $kde_check/$kde_test_library 2>/dev/null | head -1`"; then
            kde_libdir="$kde_check"
            break
        fi
    done
    AC_MSG_RESULT([$kde_libdir])
    if test "x$kde_libdir" = "xno"; then
        AC_MSG_ERROR([KDE3 libraries not found.  Please specify the root of
your KDE3 installation by exporting KDEDIR before running "configure".])
    fi

    dnl Set the variables
    KDE_CFLAGS="-I$qt_incdir -I$kde_incdir -DQT_CLEAN_NAMESPACE -DQT_THREAD_SUPPORT"
    KDE_LIBS="-L$kde_libdir -L$qt_libdir -lkdeui -lkdecore -lqt-mt"
fi
AC_SUBST(KDE_CFLAGS)
AC_SUBST(KDE_LIBS)
AC_SUBST(MOC)

dnl ===================================================================
dnl KDE4 Integration
dnl ===================================================================

KDE4_CFLAGS=""
KDE4_LIBS=""
QMAKE4="qmake"
MOC4="moc"
KDE_GLIB_CFLAGS=""
KDE_GLIB_LIBS=""
KDE_HAVE_GLIB=""
if test "$test_kde4" = "yes" -a "$ENABLE_KDE4" = "TRUE"; then
    qt_incdirs="$QT4INC /usr/include/qt4 /usr/include $x_includes"
    qt_libdirs="$QT4LIB /usr/lib/qt4 /usr/lib $x_libraries"

    kde_incdirs="/usr/include /usr/include/kde4 $x_includes"
    kde_libdirs="/usr/lib /usr/lib/kde4 /usr/lib/kde4/devel $x_libraries"

    if test -n "$supports_multilib"; then
        qt_libdirs="$qt_libdirs /usr/lib64/qt4 /usr/lib64/qt /usr/lib64"
        kde_libdirs="$kde_libdirs /usr/lib64 /usr/lib64/kde4 /usr/lib64/kde4/devel"
    fi

    if test -n "$QTDIR"; then
        qt_incdirs="$QTDIR/include $qt_incdirs"
        if test -z "$supports_multilib"; then
            qt_libdirs="$QTDIR/lib $qt_libdirs"
        else
            qt_libdirs="$QTDIR/lib64 $QTDIR/lib $qt_libdirs"
        fi
    fi
    if test -n "$QT4DIR"; then
        qt_incdirs="$QT4DIR/include $qt_incdirs"
        if test -z "$supports_multilib"; then
            qt_libdirs="$QT4DIR/lib $qt_libdirs"
        else
            qt_libdirs="$QT4DIR/lib64 $QT4DIR/lib $qt_libdirs"
        fi
    fi

    if test -n "$KDEDIR"; then
        kde_incdirs="$KDEDIR/include $kde_incdirs"
        if test -z "$supports_multilib"; then
            kde_libdirs="$KDEDIR/lib $kde_libdirs"
        else
            kde_libdirs="$KDEDIR/lib64 $KDEDIR/lib $kde_libdirs"
        fi
    fi
    if test -n "$KDE4DIR"; then
        kde_incdirs="$KDE4DIR/include $KDE4DIR/include/kde4 $kde_incdirs"
        if test -z "$supports_multilib"; then
            kde_libdirs="$KDE4DIR/lib $kde_libdirs"
        else
            kde_libdirs="$KDE4DIR/lib64 $KDE4DIR/lib $kde_libdirs"
        fi
    fi

    qt_test_include="Qt/qobject.h"
    qt_test_library="libQtCore.so"
    kde_test_include="kwindowsystem.h"
    kde_test_library="libsolid.so"

    AC_MSG_CHECKING([for Qt4 headers])
    qt_header_dir="no"
    for inc_dir in $qt_incdirs; do
        if test -r "$inc_dir/$qt_test_include"; then
            qt_header_dir="$inc_dir"
            break
        fi
    done

    AC_MSG_RESULT([$qt_header_dir])
    if test "x$qt_header_dir" = "xno"; then
        AC_MSG_ERROR([Qt4 headers not found.  Please specify the root of your Qt4 installation by exporting QT4DIR before running "configure".])
    fi

    dnl Check for qmake
    AC_PATH_PROG( QMAKEQT4, qmake-qt4, no, [`dirname $qt_header_dir`/bin:$QT4DIR/bin:$PATH] )
    QMAKE4="$QMAKEQT4"
    if test "$QMAKE4" = "no" ; then
        AC_PATH_PROG( QMAKE4, qmake, no, [`dirname $qt_header_dir`/bin:$QT4DIR/bin:$PATH] )
        if test "$QMAKE4" = "no" ; then
            AC_MSG_ERROR([Qmake not found.  Please specify
the root of your Qt installation by exporting QT4DIR before running "configure".])
        fi
    fi

    qt_libdirs="`$QMAKE4 -query QT_INSTALL_LIBS` $qt_libdirs"
    AC_MSG_CHECKING([for Qt4 libraries])
    qt_lib_dir="no"
    for lib_dir in $qt_libdirs; do
        if test -r "$lib_dir/$qt_test_library"; then
            qt_lib_dir="$lib_dir"
            PKG_CONFIG_PATH="$qt_lib_dir"/pkgconfig:$PKG_CONFIG_PATH
            break
        fi
    done

    AC_MSG_RESULT([$qt_lib_dir])

    if test "x$qt_lib_dir" = "xno"; then
        AC_MSG_ERROR([Qt4 libraries not found.  Please specify the root of your Qt4 installation by exporting QT4DIR before running "configure".])
    fi

    dnl Check for Meta Object Compiler

    AC_PATH_PROG( MOCQT4, moc-qt4, no, [`dirname $qt_lib_dir`/bin:$QT4DIR/bin:$PATH] )
    MOC4="$MOCQT4"
    if test "$MOC4" = "no"; then
        AC_PATH_PROG( MOC4, moc, no, [`dirname $qt_lib_dir`/bin:$QT4DIR/bin:$PATH] )
        if test "$MOC4" = "no"; then
            AC_MSG_ERROR([Qt Meta Object Compiler not found.  Please specify
the root of your Qt installation by exporting QT4DIR before running "configure".])
        fi
    fi

    dnl Check for KDE4 headers
    AC_MSG_CHECKING([for KDE4 headers])
    kde_incdir="no"
    for kde_check in $kde_incdirs; do
        if test -r "$kde_check/$kde_test_include"; then
            kde_incdir="$kde_check"
            break
        fi
    done
    AC_MSG_RESULT([$kde_incdir])
    if test "x$kde_incdir" = "xno"; then
        AC_MSG_ERROR([KDE4 headers not found.  Please specify the root of your KDE4 installation by exporting KDE4DIR before running "configure".])
    fi

    dnl Check for KDE4 libraries
    AC_MSG_CHECKING([for KDE4 libraries])
    kde_libdir="no"
    for kde_check in $kde_libdirs; do
        if test -r "$kde_check/$kde_test_library"; then
            kde_libdir="$kde_check"
            break
        fi
    done

    AC_MSG_RESULT([$kde_libdir])
    if test "x$kde_libdir" = "xno"; then
        AC_MSG_ERROR([KDE4 libraries not found.  Please specify the root of your KDE4 installation by exporting KDE4DIR before running "configure".])
    fi

    KDE4_CFLAGS="`pkg-config --cflags QtCore` `pkg-config --cflags QtGui` -I$kde_incdir -DQT_CLEAN_NAMESPACE -DQT_THREAD_SUPPORT"
    KDE4_LIBS="-L$kde_libdir -L$qt_lib_dir -lkio -lkfile -lkdeui -lkdecore -lQtCore -lQtGui"

    AC_LANG_PUSH([C++])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS $KDE4_CFLAGS"
    AC_MSG_CHECKING([whether KDE is >= 4.2])
       AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <kdeversion.h>

int main(int argc, char **argv) {
       if (KDE_VERSION_MAJOR == 4 && KDE_VERSION_MINOR >= 2) return 0;
       else return 1;
}
       ]])],[AC_MSG_RESULT([yes])],[AC_MSG_ERROR([KDE version too old])],[])
    CXXFLAGS=$save_CXXFLAGS
    AC_LANG_POP([C++])

   # Glib is needed for properly handling Qt event loop with Qt's Glib integration enabled
    PKG_CHECK_MODULES(KDE_GLIB,[glib-2.0 >= 2.4],
        [KDE_HAVE_GLIB=1],
        AC_MSG_WARN([[No Glib found, KDE4 support will not integrate with Qt's Glib event loop support]]))
fi
AC_SUBST(KDE4_CFLAGS)
AC_SUBST(KDE4_LIBS)
AC_SUBST(MOC4)
AC_SUBST(KDE_GLIB_CFLAGS)
AC_SUBST(KDE_GLIB_LIBS)
AC_SUBST(KDE_HAVE_GLIB)

dnl ===================================================================
dnl Test for the enabling the lockdown pieces
dnl ===================================================================
AC_MSG_CHECKING([whether to enable the lockdown pieces])
ENABLE_LOCKDOWN=""
if test -n "$enable_lockdown" -a "$enable_lockdown" != "no"; then
    ENABLE_LOCKDOWN=YES
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_LOCKDOWN)

dnl ===================================================================
dnl Test whether to include Evolution 2 support
dnl ===================================================================
AC_MSG_CHECKING([whether to enable evolution 2 support])
if test "$enable_evolution2" = "yes" -o "$enable_evolution2" = "TRUE"; then
    AC_MSG_RESULT([yes])
    PKG_CHECK_MODULES(GOBJECT, gobject-2.0)
    ENABLE_EVOAB2="TRUE"
else
    ENABLE_EVOAB2=""
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_EVOAB2)
AC_SUBST(GOBJECT_CFLAGS)
AC_SUBST(GOBJECT_LIBS)

dnl ===================================================================
dnl Test whether to include TDE AB support
dnl ===================================================================
AC_MSG_CHECKING([whether to enable TDE address book support])
if test "$enable_tdeab" = "yes" -a "$enable_tde" = "yes"; then
    AC_MSG_RESULT([yes])
    AC_LANG_PUSH([C++])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS $TDE_CFLAGS"
    AC_MSG_CHECKING([whether TDE is at least R14.0])
       AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <tdeversion.h>

int main(int argc, char **argv) {
       if (TDE_VERSION_MAJOR >= 14) return 0;
       else return 1;
}
       ]])],[AC_MSG_RESULT([yes])],[AC_MSG_ERROR([TDE version too old or too recent, please use another version of TDE or disable TDE address book support])],[])
    CXXFLAGS=$save_CXXFLAGS
    AC_LANG_POP([C++])
    ENABLE_TDEAB=TRUE
else
    AC_MSG_RESULT([no])
    ENABLE_TDEAB=
fi
AC_SUBST(ENABLE_TDEAB)

dnl ===================================================================
dnl Test whether to include KDE AB support
dnl ===================================================================
AC_MSG_CHECKING([whether to enable KDE address book support])
if test "$enable_kdeab" = "yes" -a "$enable_kde" = "yes"; then
    AC_MSG_RESULT([yes])
    AC_LANG_PUSH([C++])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS $KDE_CFLAGS"
    AC_MSG_CHECKING([whether KDE is between 3.2 and 3.6])
       AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <kdeversion.h>

int main(int argc, char **argv) {
       if (KDE_VERSION_MAJOR == 3 && 2 <= KDE_VERSION_MINOR && KDE_VERSION_MINOR <= 6) return 0;
       else return 1;
}
       ]])],[AC_MSG_RESULT([yes])],[AC_MSG_ERROR([KDE version too old or too recent, please use another version of KDE or disable KDE address book support])],[])
    CXXFLAGS=$save_CXXFLAGS
    AC_LANG_POP([C++])
    ENABLE_KAB=TRUE
else
    AC_MSG_RESULT([no])
    ENABLE_KAB=
fi
AC_SUBST(ENABLE_KAB)

dnl ===================================================================
dnl Test whether to include MathMLDTD
dnl ===================================================================
AC_MSG_CHECKING([whether to include MathMLDTD])
if test -n "$enable_mathmldtd"; then
    if test "$enable_mathmldtd" = "no"; then
        AC_MSG_RESULT([no])
        SCPDEFS="$SCPDEFS -DWITHOUT_MATHMLDTD"
    else
        AC_MSG_RESULT([yes])
        BUILD_TYPE="$BUILD_TYPE MATHMLDTD"
    fi
else
    AC_MSG_RESULT([no])
    SCPDEFS="$SCPDEFS -DWITHOUT_MATHMLDTD"
fi

dnl ===================================================================
dnl Test which themes to include
dnl ===================================================================
AC_MSG_CHECKING([which themes to include])
# if none given use default subset of available themes
if test "x$with_theme" = "x" -o "x$with_theme" = "xyes"; then
    with_theme="crystal default hicontrast oxygen tango"
    test "$ENABLE_RELEASE_BUILD" = "FALSE" && with_theme="$with_theme tango_testing"
fi

WITH_THEMES=""
for theme in $with_theme; do
    case $theme in
        crystal|default|galaxy|hicontrast|human|industrial|oxygen|tango|tango_testing) : ;;
        *) AC_MSG_ERROR([Unknown value for --with-theme: $theme]) ;;
    esac
    WITH_THEMES="$WITH_THEMES $theme"
    SCPDEFS="$SCPDEFS -DTHEME_`echo $theme|tr '[[:lower:]]' '[[:upper:]]'`"
done
AC_MSG_RESULT([$WITH_THEMES])
AC_SUBST([WITH_THEMES])

dnl ===================================================================
dnl Test whether to integrate helppacks into the product's installer
dnl ===================================================================
AC_MSG_CHECKING([for helppack integration])
if test "$with_helppack_integration" = "no"; then
    WITH_HELPPACK_INTEGRATION=NO
    AC_MSG_RESULT([no integration])
else
    WITH_HELPPACK_INTEGRATION=YES
    SCPDEFS="$SCPDEFS -DWITH_HELPPACK_INTEGRATION"
    AC_MSG_RESULT([integration])
fi
AC_SUBST(WITH_HELPPACK_INTEGRATION)

dnl ===================================================================
dnl Test whether to use CoreText framework
dnl ===================================================================
ENABLE_CORETEXT=NO
if test "$_os" = "Darwin"; then
    AC_MSG_CHECKING([whether to use CoreText framework])
    if test "$enable_coretext" = yes -o "$BITNESS_OVERRIDE" = 64; then
        AC_MSG_RESULT([yes])
        ENABLE_CORETEXT=YES
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_SUBST(ENABLE_CORETEXT)

###############################################################################
# Extensions checking
###############################################################################
dnl ===================================================================
dnl Test whether to integrate extensions into the product's installer
dnl ===================================================================
AC_MSG_CHECKING([for extensions integration])
if test "x$enable_extension_integration" != "xno"; then
    WITH_EXTENSION_INTEGRATION=YES
    SCPDEFS="$SCPDEFS -DWITH_EXTENSION_INTEGRATION"
    AC_MSG_RESULT([yes, use integration])
else
    WITH_EXTENSION_INTEGRATION=NO
    AC_MSG_RESULT([no, do not integrate])
fi
AC_SUBST(WITH_EXTENSION_INTEGRATION)

dnl Should any extra extensions be included?
dnl There are standalone tests for each of these below.
WITH_EXTRA_EXTENSIONS=
AC_SUBST([WITH_EXTRA_EXTENSIONS])

dnl ===================================================================
dnl Test whether to include Watch Window extension
dnl ===================================================================
AC_MSG_CHECKING([for Watch Window extension integration])
WATCH_WINDOW_EXTENSION_PACK=
if test "x$enable_ext_watch_window" = "xyes" -a "x$enable_extension_integration" != "xno"; then
    SCPDEFS="$SCPDEFS -DWITH_EXTENSION_WATCH_WINDOW"
    WATCH_WINDOW_EXTENSION_PACK="23bd75552206dfcd8fd4e29137dcac84-WatchWindow_1.2.0.0.oxt"
    WITH_EXTRA_EXTENSIONS=YES
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(WATCH_WINDOW_EXTENSION_PACK)

dnl ===================================================================
dnl Test whether to include SmART Gallery (Diagram) extension
dnl ===================================================================
AC_MSG_CHECKING([for SmART Gallery (Diagram) extension integration])
DIAGRAM_EXTENSION_PACK=
if test "x$enable_ext_diagram" = "xyes" -a "x$enable_extension_integration" != "xno"; then
    SCPDEFS="$SCPDEFS -DWITH_EXTENSION_DIAGRAM"
    DIAGRAM_EXTENSION_PACK="8d74685d41f8bffe8c3e71fe8deac09d-SmART_0.9.5.oxt"
    WITH_EXTRA_EXTENSIONS=YES
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(DIAGRAM_EXTENSION_PACK)

dnl ===================================================================
dnl Test whether to include Validator extension
dnl ===================================================================
AC_MSG_CHECKING([for Validator extension integration])
VALIDATOR_EXTENSION_PACK=
if test "x$enable_ext_validator" = "xyes" -a "x$enable_extension_integration" != "xno"; then
    SCPDEFS="$SCPDEFS -DWITH_EXTENSION_VALIDATOR"
    VALIDATOR_EXTENSION_PACK="bbdd5639ada63e3130761daaecae1a10-Validator_1.1.0.0.oxt"
    WITH_EXTRA_EXTENSIONS=YES
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(VALIDATOR_EXTENSION_PACK)

dnl ===================================================================
dnl Test whether to include Barcode extension
dnl ===================================================================
AC_MSG_CHECKING([for Barcode extension integration])
BARCODE_EXTENSION_PACK=
if test "x$enable_ext_barcode" = "xyes" -a "x$enable_extension_integration" != "xno"; then
    SCPDEFS="$SCPDEFS -DWITH_EXTENSION_BARCODE"
    BARCODE_EXTENSION_PACK="3ed18025a766f1e955707b969c8113a5-Barcode_1.3.5.0.oxt"
    WITH_EXTRA_EXTENSIONS=YES
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST([BARCODE_EXTENSION_PACK])

dnl ===================================================================
dnl Test whether to include ConvertTextToNumber extension
dnl ===================================================================
AC_MSG_CHECKING([for ConvertTextToNumber extension integration])
if test "x$enable_ext_ct2n" = "xyes" -a "x$enable_extension_integration" != "xno"; then
    BUILD_TYPE="$BUILD_TYPE CT2N"
    SCPDEFS="$SCPDEFS -DWITH_EXTENSION_CT2N"
    CT2N_TARBALL="451ccf439a36a568653b024534669971-ConvertTextToNumber-1.3.2.oxt"
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST([CT2N_TARBALL])

dnl ===================================================================
dnl Test whether to include Numbertext extension
dnl ===================================================================
AC_MSG_CHECKING([for Numbertext extension integration])
NUMBERTEXT_EXTENSION_PACK=
if test "x$enable_ext_numbertext" = "xyes" -a "x$enable_extension_integration" != "xno"; then
    SCPDEFS="$SCPDEFS -DWITH_EXTENSION_NUMBERTEXT"
    NUMBERTEXT_EXTENSION_PACK="b8cbca7b3363e6ca2d02bc0ba2b63904-numbertext_0.9.4.oxt"
    WITH_EXTRA_EXTENSIONS=YES
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(NUMBERTEXT_EXTENSION_PACK)

dnl ===================================================================
dnl Test whether to include Hungarian Cross-reference Toolbar extension
dnl ===================================================================
AC_MSG_CHECKING([for Hungarian Cross-reference Toolbar extension integration])
HUNART_EXTENSION_PACK=
if test "x$enable_ext_hunart" = "xyes" -a "x$enable_extension_integration" != "xno"; then
    SCPDEFS="$SCPDEFS -DWITH_EXTENSION_HUNART"
    HUNART_EXTENSION_PACK="b632bdd25649cc4067bcb410bae23d2b-hunart_0.3.oxt"
    WITH_EXTRA_EXTENSIONS=YES
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(HUNART_EXTENSION_PACK)

dnl ===================================================================
dnl Test whether to include Typography Toolbar extension
dnl ===================================================================
AC_MSG_CHECKING([for Typography Toolbar extension integration])
TYPO_EXTENSION_PACK=
if test "x$enable_ext_typo" = "xyes" -a "x$enable_extension_integration" != "xno"; then
    SCPDEFS="$SCPDEFS -DWITH_EXTENSION_TYPO"
    TYPO_EXTENSION_PACK="9d60b6cfa3ef1926848710bbcd11115b-typo_0.4.2.oxt"
    WITH_EXTRA_EXTENSIONS=YES
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(TYPO_EXTENSION_PACK)

dnl ===================================================================
dnl Test whether to include Google Docs extension
dnl ===================================================================
AC_MSG_CHECKING([for Google Docs extension integration])
GOOGLE_DOCS_EXTENSION_PACK=
if test "x$enable_ext_google_docs" = "xyes" -a "x$enable_extension_integration" != "xno"; then
    SCPDEFS="$SCPDEFS -DWITH_EXTENSION_GOOGLE_DOCS"
    GOOGLE_DOCS_EXTENSION_PACK="2a64b8e6f185a7f6c19d00fb8bb4cf64-gdocs_3.0.1_modified.oxt"
    WITH_EXTRA_EXTENSIONS=YES
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(GOOGLE_DOCS_EXTENSION_PACK)

dnl ===================================================================
dnl Test whether to include NLPSolver extension
dnl ===================================================================
AC_MSG_CHECKING([for NLPSolver extension integration])
if test "x$enable_ext_nlpsolver" = "xyes" -a "x$enable_extension_integration" != "xno" -a "x$with_java" != "xno"; then
    BUILD_TYPE="$BUILD_TYPE NLPSOLVER"
    SCPDEFS="$SCPDEFS -DWITH_EXTENSION_NLPSOLVER"
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi

dnl ===================================================================
dnl Test whether to include LanguageTool extension
dnl ===================================================================
AC_MSG_CHECKING([for LanguageTool extension integration])
if test "x$enable_ext_languagetool" = "xyes" -a "x$enable_extension_integration" != "xno" -a "x$with_java" != "xno"; then
    BUILD_TYPE="$BUILD_TYPE LANGUAGETOOL"
    SCPDEFS="$SCPDEFS -DWITH_EXTENSION_LANGUAGETOOL"
    LANGUAGETOOL_TARBALL="b63e6340a02ff1cacfeadb2c42286161-JLanguageTool-1.7.0.tar.bz2"
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(LANGUAGETOOL_TARBALL)

dnl ===================================================================
dnl Test whether to include oooblogger extension
dnl ===================================================================
AC_MSG_CHECKING([for oooblogger extension integration])
OOOBLOGGER_EXTENSION_PACK=
if test "x$enable_ext_oooblogger" = "xyes" -a "x$enable_extension_integration" != "xno"; then
    SCPDEFS="$SCPDEFS -DWITH_EXTENSION_OOOBLOGGER"
    OOOBLOGGER_EXTENSION_PACK="b7b2d0e04e142f26dd96119c80757d1f-oooblogger_0.1.oxt"
    WITH_EXTRA_EXTENSIONS=YES
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(OOOBLOGGER_EXTENSION_PACK)
###############################################################################

dnl ===================================================================
dnl Test whether to include Sun Professional Template Pack
dnl ===================================================================
AC_MSG_CHECKING([for Sun Professional Template Pack integration (only supported languages displayed)])
if test "$with_sun_templates" = "" -o "$with_sun_templates" = "no"; then
    AC_MSG_RESULT([no integration])
else
    SCPDEFS="$SCPDEFS -DWITH_EXTENSION_SUN_TEMPLATE_PACK"
    sun_supported_langs="en-US de it fr es hu"
    if test "$with_sun_templates" = "yes"; then
        wanted_sun_templates="$sun_supported_langs"
    else
        # check whether the langs are supported by Sun
        wanted_sun_templates=
        for lang in $with_sun_templates; do
            if test -n "`echo $sun_supported_langs | grep "$lang"`"; then
                wanted_sun_templates="$wanted_sun_templates $lang"
            fi
        done
    fi
    SUNTEMPLATES_LANG=
    SUNTEMPLATES_DE_PACK=
    SUNTEMPLATES_EN_US_PACK=
    SUNTEMPLATES_ES_PACK=
    SUNTEMPLATES_FR_PACK=
    SUNTEMPLATES_HU_PACK=
    SUNTEMPLATES_IT_PACK=
    # check whether the langs are requested at all
    for lang in $wanted_sun_templates; do
    if test "$with_lang" = "ALL" -o -n "`echo $with_lang | grep "$lang"`"; then
        SUNTEMPLATES_LANG="$SUNTEMPLATES_LANG $lang"
        case $lang in
        "de") SUNTEMPLATES_DE_PACK="53ca5e56ccd4cab3693ad32c6bd13343-Sun-ODF-Template-Pack-de_1.0.0.oxt";;
        "en-US") SUNTEMPLATES_EN_US_PACK="472ffb92d82cf502be039203c606643d-Sun-ODF-Template-Pack-en-US_1.0.0.oxt";;
        "es") SUNTEMPLATES_ES_PACK="4ad003e7bbda5715f5f38fde1f707af2-Sun-ODF-Template-Pack-es_1.0.0.oxt";;
        "fr") SUNTEMPLATES_FR_PACK="a53080dc876edcddb26eb4c3c7537469-Sun-ODF-Template-Pack-fr_1.0.0.oxt";;
        "hu") SUNTEMPLATES_HU_PACK="09ec2dac030e1dcd5ef7fa1692691dc0-Sun-ODF-Template-Pack-hu_1.0.0.oxt";;
        "it") SUNTEMPLATES_IT_PACK="b33775feda3bcf823cad7ac361fd49a6-Sun-ODF-Template-Pack-it_1.0.0.oxt";;
        esac
    fi
    done
    WITH_EXTRA_EXTENSIONS=YES
    AC_MSG_RESULT([$SUNTEMPLATES_LANG])
fi
AC_SUBST(SUNTEMPLATES_DE_PACK)
AC_SUBST(SUNTEMPLATES_EN_US_PACK)
AC_SUBST(SUNTEMPLATES_ES_PACK)
AC_SUBST(SUNTEMPLATES_FR_PACK)
AC_SUBST(SUNTEMPLATES_HU_PACK)
AC_SUBST(SUNTEMPLATES_IT_PACK)

dnl ===================================================================
dnl Test whether to include fonts
dnl ===================================================================
AC_MSG_CHECKING([whether to include third-party fonts])
if test "$with_fonts" != "no"; then
    AC_MSG_RESULT([yes])
    WITH_FONTS=YES
    BUILD_TYPE="$BUILD_TYPE MORE_FONTS"
    FONT_DEJAVU_TARBALL="f872f4ac066433d8ff92f5e316b36ff9-dejavu-fonts-ttf-2.33.zip"
    FONT_GENTIUM_TARBALL="35efabc239af896dfb79be7ebdd6e6b9-gentiumbasic-fonts-1.10.zip"
    FONT_LIBERATION_NARROW_TARBALL="0be45d54cc5e1c2e3102e32b8c190346-liberation-fonts-ttf-1.07.1.tar.gz"
    FONT_LIBERATION_TARBALL="5c781723a0d9ed6188960defba8e91cf-liberation-fonts-ttf-2.00.1.tar.gz"
    FONT_LINLIBERTINEG_TARBALL="e7a384790b13c29113e22e596ade9687-LinLibertineG-20120116.zip"
else
    AC_MSG_RESULT([no])
    WITH_FONTS=NO
    SCPDEFS="$SCPDEFS -DWITHOUT_FONTS"
    FONT_DEJAVU_TARBALL=
    FONT_GENTIUM_TARBALL=
    FONT_LIBERATION_NARROW_TARBALL=
    FONT_LIBERATION_TARBALL=
    FONT_LINLIBERTINEG_TARBALL=
fi
AC_SUBST(FONT_DEJAVU_TARBALL)
AC_SUBST(FONT_GENTIUM_TARBALL)
AC_SUBST(FONT_LIBERATION_TARBALL)
AC_SUBST(FONT_LINLIBERTINEG_TARBALL)
AC_SUBST(FONT_LIBERATION_NARROW_TARBALL)
AC_SUBST(WITH_FONTS)


# The agfa monotype fonts options seems broken
# there is no assocaited entry in ooo.lst
# so we do not fetch the dependencies
# there is no tie-up with MORE_FONTS, but in dmake mode
# without more_fonts that option is meaningless
# so I suspect it is not really used anymore, commenting out for now
#AC_MSG_CHECKING([whether to include Agfa Monotype fonts])
#if test "$with_agfa_monotype_fonts" = "yes"; then
#    AC_MSG_RESULT([yes])
#    WITH_AGFA_MONOTYPE_FONTS=YES
#    SCPDEFS="$SCPDEFS -DWITH_AGFA_MONOTYPE_FONTS"
#    BUILD_TYPE="$BUILD_TYPE AGFA_MONOTYPE_FONTS"
#else
#    AC_MSG_RESULT([no])
#    WITH_AGFA_MONOTYPE_FONTS=NO
#fi
#AC_SUBST(WITH_AGFA_MONOTYPE_FONTS)

dnl ===================================================================
dnl Test whether to include ppds
dnl ===================================================================
AC_MSG_CHECKING([whether to include PPDs])
if test "$with_ppds" != "no"; then
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
    WITHOUT_PPDS=YES
    SCPDEFS="$SCPDEFS -DWITHOUT_PPDS"
fi
AC_SUBST(WITHOUT_PPDS)

dnl ===================================================================
dnl Test whether to include afms
dnl ===================================================================
AC_MSG_CHECKING([whether to include AFMs])
if test "$with_afms" != "no"; then
    AC_MSG_RESULT([yes])
    BUILD_TYPE="$BUILD_TYPE AFMS"
    AFMS_TARBALL="1756c4fa6c616ae15973c104cd8cb256-Adobe-Core35_AFMs-314.tar.gz"
else
    AC_MSG_RESULT([no])
    WITHOUT_AFMS=YES
    SCPDEFS="$SCPDEFS -DWITHOUT_AFMS"
    AFMS_TARBALL=
fi
AC_SUBST(WITHOUT_AFMS)
AC_SUBST(AFMS_TARBALL)

dnl ===================================================================
dnl Test whether to include extra galleries
dnl ===================================================================
AC_MSG_CHECKING([whether to include extra galleries])
if test "$enable_extra_gallery" = "" -o "$enable_extra_gallery" = "no"; then
    AC_MSG_RESULT([no])
    WITH_EXTRA_GALLERY=NO
    OOOP_GALLERY_PACK=""
else
    AC_MSG_RESULT([yes])
    WITH_EXTRA_GALLERY=YES
    BUILD_TYPE="$BUILD_TYPE EXTRA_GALLERY"
    SCPDEFS="$SCPDEFS -DWITH_EXTRA_GALLERY"
    OOOP_GALLERY_PACK="af9314c5972d95a5d6da23ffad818f68-OOOP-gallery-pack-2.8.0.0.zip"
fi
AC_SUBST(WITH_EXTRA_GALLERY)
AC_SUBST(OOOP_GALLERY_PACK)

dnl ===================================================================
dnl Test whether to include extra templates
dnl ===================================================================
AC_MSG_CHECKING([whether to include extra templates])
if test "$enable_extra_template" = "" -o "$enable_extra_template" = "no"; then
    AC_MSG_RESULT([no])
    WITH_EXTRA_TEMPLATE=NO
    OOOP_TEMPLATES_PACK=""
else
    AC_MSG_RESULT([yes])
    WITH_EXTRA_TEMPLATE=YES
    BUILD_TYPE="$BUILD_TYPE EXTRA_TEMPLATE"
    SCPDEFS="$SCPDEFS -DWITH_EXTRA_TEMPLATE"
    OOOP_TEMPLATES_PACK="1be202fbbbc13f10592a98f70a4a87fb-OOOP-templates-pack-2.9.0.0.zip"
fi
AC_SUBST(WITH_EXTRA_TEMPLATE)
AC_SUBST(OOOP_TEMPLATES_PACK)

dnl ===================================================================
dnl Test whether to include extra samples
dnl ===================================================================
AC_MSG_CHECKING([whether to include extra samples])
if test "$enable_extra_sample" = "" -o "$enable_extra_sample" = "no"; then
    AC_MSG_RESULT([no])
    WITH_EXTRA_SAMPLE=NO
    OOOP_SAMPLES_PACK=""
else
    AC_MSG_RESULT([yes])
    WITH_EXTRA_SAMPLE=YES
    BUILD_TYPE="$BUILD_TYPE EXTRA_SAMPLE"
    SCPDEFS="$SCPDEFS -DWITH_EXTRA_SAMPLE"
    OOOP_SAMPLES_PACK="a6bccacf44914969e6e7b2f8faf4132c-OOOP-samples-pack-2.7.0.0.zip"
fi
AC_SUBST(WITH_EXTRA_SAMPLE)
AC_SUBST(OOOP_SAMPLES_PACK)

dnl ===================================================================
dnl Test whether to include extra fonts
dnl ===================================================================
AC_MSG_CHECKING([whether to include extra fonts])
if test "$enable_extra_font" = "" -o "$enable_extra_font" = "no"; then
    AC_MSG_RESULT([no])
    WITH_EXTRA_FONT=NO
    OOOP_FONTS_PACK=""
else
    AC_MSG_RESULT([yes])
    WITH_EXTRA_FONT=YES
    BUILD_TYPE="$BUILD_TYPE EXTRA_FONT"
    SCPDEFS="$SCPDEFS -DWITH_EXTRA_FONT"
    OOOP_FONTS_PACK="a10aa597411643326e27d7fc128af12d-OOOP-fonts-pack-2.9.0.0.zip"
fi
AC_SUBST(WITH_EXTRA_FONT)
AC_SUBST(OOOP_FONTS_PACK)

dnl ===================================================================
dnl Test whether to download OxygenOffice branding and set custom settings
dnl ===================================================================
AC_MSG_CHECKING([whether to download OxygenOffice branding and set custom settings])
if test "$enable_oxygenoffice" = "" -o "$enable_oxygenoffice" = "no"; then
    AC_MSG_RESULT([no])
    ENABLE_OXYGENOFFICE=NO
else
    AC_MSG_RESULT([yes])
    ENABLE_OXYGENOFFICE=YES
    OXYGENOFFICE_PACK="18bf204479ff641d99a88cd71f6f25f7-oxygenoffice-001.zip"
    BUILD_TYPE="$BUILD_TYPE OXYGENOFFICE"
fi
AC_SUBST(ENABLE_OXYGENOFFICE)
AC_SUBST(OXYGENOFFICE_PACK)

dnl ===================================================================
dnl Test whether to build global menu support
dnl ===================================================================
AC_MSG_CHECKING([whether to build global menu support])
if test "$enable_lomenubar" = "" -o "$enable_lomenubar" = "no"; then
    AC_MSG_RESULT([no])
    ENABLE_LOMENUBAR="FALSE"
else
    AC_MSG_RESULT([yes])
    PKG_CHECK_MODULES(DBUSMENUGTK, dbusmenu-gtk-0.4,, AC_MSG_ERROR([requirements to build lo-menubar not met. Use --disable-lomenubar or install the missing packages]))
    ENABLE_LOMENUBAR="TRUE"
fi
AC_SUBST(ENABLE_LOMENUBAR)

dnl ===================================================================
dnl Test whether to enable online update service
dnl ===================================================================
AC_MSG_CHECKING([whether to enable online update])
ENABLE_ONLINE_UPDATE=
if test "$enable_online_update" = ""; then
    if test "$_os" = "WINNT" -o "$_os" = "Darwin"; then
        AC_MSG_RESULT([yes])
        ENABLE_ONLINE_UPDATE="TRUE"
    else
        AC_MSG_RESULT([no])
    fi
else
    if test "$enable_online_update" = "yes"; then
        AC_MSG_RESULT([yes])
        ENABLE_ONLINE_UPDATE="TRUE"
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_SUBST(ENABLE_ONLINE_UPDATE)

dnl ===================================================================
dnl Test whether to create MSI with LIMITUI=1 (silent install)
dnl ===================================================================
AC_MSG_CHECKING([whether to create MSI with LIMITUI=1 (silent install)])
if test "$enable_silent_msi" = "" -o "$enable_silent_msi" = "no"; then
    AC_MSG_RESULT([no])
    ENABLE_SILENT_MSI="FALSE"
else
    AC_MSG_RESULT([yes])
    ENABLE_SILENT_MSI="TRUE"
    SCPDEFS="$SCPDEFS -DENABLE_SILENT_MSI"
fi
AC_SUBST(ENABLE_SILENT_MSI)

AC_SUBST(SCPDEFS)

AC_MSG_CHECKING([whether and how to use Xinerama])
if test "$_os" = "Darwin"; then
    USE_XINERAMA=YES
    XINERAMA_LINK=dynamic
    AC_MSG_RESULT([yes])
elif test "$_os" = "Linux" -o "$_os" = "FreeBSD"; then
    if test "$x_libraries" = "default_x_libraries"; then
        XINERAMALIB=`$PKG_CONFIG --variable=libdir xinerama`
        if test "x$XINERAMALIB" = x; then
           XINERAMALIB="/usr/lib"
        fi
    else
        XINERAMALIB="$x_libraries"
    fi
    if test -e "$XINERAMALIB/libXinerama.so" -a -e "$XINERAMALIB/libXinerama.a"; then
        # we have both versions, let the user decide but use the dynamic one
        # per default
        USE_XINERAMA=YES
        if test -z "$with_static_xinerama" -o -n "$with_system_libs"; then
            XINERAMA_LINK=dynamic
        else
            XINERAMA_LINK=static
        fi
    elif test -e "$XINERAMALIB/libXinerama.so" -a ! -e "$XINERAMALIB/libXinerama.a"; then
        # we have only the dynamic version
        USE_XINERAMA=YES
        XINERAMA_LINK=dynamic
    elif test -e "$XINERAMALIB/libXinerama.a"; then
        # static version
        if echo $host_cpu | $GREP -E 'i[[3456]]86' 2>/dev/null >/dev/null; then
            USE_XINERAMA=YES
            XINERAMA_LINK=static
        else
            USE_XINERAMA=NO
            XINERAMA_LINK=none
        fi
    else
        # no Xinerama
        USE_XINERAMA=NO
        XINERAMA_LINK=none
    fi
    if test "$USE_XINERAMA" = "YES"; then
        AC_MSG_RESULT([yes, with $XINERAMA_LINK linking])
        AC_CHECK_HEADER(X11/extensions/Xinerama.h, [],
            [AC_MSG_ERROR(Xinerama header not found.)], [])
        XEXTLIBS=`$PKG_CONFIG --variable=libs xext`
        if test "x$XEXTLIB" = x; then
           XEXTLIBS="-L$XLIB -L$XINERAMALIB -lXext"
        fi
        XINERAMA_EXTRA_LIBS="$XEXTLIBS"
        if test "$_os" = "FreeBSD"; then
            XINERAMA_EXTRA_LIBS="$XINERAMA_EXTRA_LIBS -lXt"
        fi
        if test "$_os" = "Linux"; then
            XINERAMA_EXTRA_LIBS="$XINERAMA_EXTRA_LIBS -ldl"
        fi
        AC_CHECK_LIB([Xinerama], [XineramaIsActive], [:],
            [AC_MSG_ERROR(Xinerama not functional?)], [$XINERAMA_EXTRA_LIBS])
    else
        AC_MSG_RESULT([no, libXinerama not found or wrong architecture.])
    fi
else
    USE_XINERAMA=NO
    XINERAMA_LINK=none
    AC_MSG_RESULT([no])
fi
AC_SUBST(USE_XINERAMA)
AC_SUBST(XINERAMA_LINK)

dnl ===================================================================
dnl Checks for librsvg
dnl ===================================================================

dnl ENABLE_LIBRSVG is set to NO, SYSTEM or INTERNAL. The
dnl SYSTEM_LIBRSVG, SYSTEM_GDKPIXBUF etc are redundant.

dnl FIXME: RSVG is a hard dependency now as the impress masterpages use it
dnl FIXME: thus the --enable-rsvg=no is not really an option anymore and we
dnl should error out in that case. leaving as is for now as this might be an
dnl unwelcome buildbreaker otherwise
ENABLE_LIBRSVG=""
LIBRSVG_CFLAGS=""
LIBRSVG_LIBS=""

AC_MSG_CHECKING([what librsvg to use])

case "$enable_librsvg" in
no|disable)
    AC_MSG_RESULT([none])
    enable_librsvg=no
    ;;

""|yes|auto)
    if test $build_os = cygwin -o \
           $_os = Darwin; then
        dnl When building on/for these OSes always use the internal one,
        dnl if at all. Add more OSes above as needed.
        AC_MSG_RESULT([internal])
        enable_librsvg=internal
    elif test  $_os = iOS -o $_os = Android; then
        AC_MSG_RESULT([none])
        enable_librsvg=no
    else
        if test "$with_system_libs" = yes; then
            AC_MSG_RESULT([system])
            PKG_CHECK_MODULES(LIBRSVG, librsvg-2.0 >= 2.14)
            enable_librsvg=system
        elif test "$with_system_libs" = no; then
            AC_MSG_RESULT([internal])
            enable_librsvg=internal
        else
            AC_MSG_RESULT([checking further])
            PKG_CHECK_MODULES(LIBRSVG, librsvg-2.0 >= 2.14,, [:])
            if test -z "$LIBRSVG_PKG_ERRORS"; then
                enable_librsvg=system
            else
                enable_librsvg=internal
            fi
        fi
    fi
    ;;

internal)
    AC_MSG_RESULT([internal])
    ;;

fully-internal)
    AC_MSG_RESULT([fully internal])
    ;;

system)
    if test $_os = WINNT -a "$WITH_MINGW" != yes; then
        AC_MSG_ERROR([Must use internal librsvg when building with MSVC])
    fi
    AC_MSG_RESULT([system])
    PKG_CHECK_MODULES(LIBRSVG, librsvg-2.0 >= 2.14)
    ;;

*)
    AC_MSG_ERROR([Incorrect --enable-librsvg option])
    ;;
esac

dnl By now enable_librsvg should be "system", "internal", "fully-internal", or
dnl "no":
case $enable_librsvg in
system)
    ENABLE_LIBRSVG=SYSTEM
    SYSTEM_LIBRSVG=YES
    ;;

internal|fully-internal)
    ENABLE_LIBRSVG=INTERNAL
    SYSTEM_LIBRSVG=NO
    BUILD_TYPE="$BUILD_TYPE LIBRSVG"
	LIBRSVG_TARBALL="d7a242ca43e33e1b63d3073f9d46a6a8-librsvg-2.32.1.tar.gz"
    ;;

no)
    ENABLE_LIBRSVG=NO
    SYSTEM_LIBRSVG=NO
    ;;

*)
    AC_MSG_ERROR([Internal configure script error, invalid enable_librsvg value "$enable_librsvg"])
    ;;
esac

AC_SUBST(ENABLE_LIBRSVG)
AC_SUBST(LIBRSVG_TARBALL)
AC_SUBST(LIBRSVG_CFLAGS)
AC_SUBST(LIBRSVG_LIBS)
AC_SUBST(SYSTEM_LIBRSVG)

dnl ===================================================================
dnl Test whether to build cairo or rely on the system version
dnl ===================================================================


SYSTEM_CAIRO=""

if test "$test_cairo" = "yes" ; then
    AC_MSG_CHECKING([whether to use the system cairo])

    if test "$with_system_cairo" = "yes"; then
        SYSTEM_CAIRO=YES
        AC_MSG_RESULT([yes])

        PKG_CHECK_MODULES( CAIRO, cairo >= 1.0.2 )

        if test "$test_xrender" = "yes"; then
            if test "$with_system_xextensions_headers" != "no"; then
                AC_MSG_CHECKING([whether Xrender.h defines PictStandardA8])
                AC_LANG_PUSH([C])
                AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <X11/extensions/Xrender.h>]],[[
#ifdef PictStandardA8
#else
      return fail;
#endif
]])],[AC_MSG_RESULT([yes])],[AC_MSG_ERROR([no, X headers too old.])])

                AC_LANG_POP([C])
            fi
        fi
        libo_MINGW_CHECK_DLL([CAIRO], [libcairo])
        libo_MINGW_TRY_DLL([FONTCONFIG], [libfontconfig])
        libo_MINGW_TRY_DLL([FREETYPE], [libfreetype])
        libo_MINGW_TRY_DLL([PIXMAN], [libpixman])
        libo_MINGW_TRY_DLL([PNG15], [libpng15])
    else
        AC_MSG_RESULT([no])

        if test $_os = WINNT; then
            dnl We only need cairo for Windows if we
            dnl build librsvg or directx disabled
            if test "$ENABLE_LIBRSVG" != NO -o -z "$ENABLE_DIRECTX"; then
                BUILD_TYPE="$BUILD_TYPE CAIRO"
            fi
        else
            if test "$enable_cairo_canvas" = "yes"; then
                BUILD_TYPE="$BUILD_TYPE CAIRO"
            else
                if test "$with_system_cairo" != "yes"; then
                    if test "$ENABLE_LIBRSVG" != NO; then
                        BUILD_TYPE="$BUILD_TYPE CAIRO"
                    fi
                fi
            fi
        fi
    fi
fi

AC_SUBST(SYSTEM_CAIRO)
AC_SUBST(CAIRO_CFLAGS)
AC_SUBST(CAIRO_LIBS)
AC_SUBST([MINGW_CAIRO_DLL])
AC_SUBST([MINGW_FONTCONFIG_DLL])
AC_SUBST([MINGW_FREETYPE_DLL])
AC_SUBST([MINGW_PIXMAN_DLL])
AC_SUBST([MINGW_PNG15_DLL])


dnl ===================================================================
dnl Test whether to build gdk-pixbuf or rely on the system version
dnl ===================================================================

if test "$ENABLE_LIBRSVG" != NO; then
    AC_MSG_CHECKING([whether to use the system gdk-pixbuf])

    dnl As long as the only thing we need gdk-pixbuf for is below
    dnl librsvg, use the same --enable-librsvg (possibly implied
    dnl by --with-system-libs) to override this.

    if test "$SYSTEM_LIBRSVG" = YES; then
        SYSTEM_GDKPIXBUF=YES
    elif test "$enable_librsvg" = fully-internal; then
        SYSTEM_GDKPIXBUF=NO
    else
        case "$_os" in
        WINNT|Darwin|iOS|Android)
            SYSTEM_GDKPIXBUF=NO
            ;;
        *)
            SYSTEM_GDKPIXBUF=YES
            ;;
        esac
    fi

    if test "$SYSTEM_GDKPIXBUF" = YES; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
        BUILD_TYPE="$BUILD_TYPE GDK_PIXBUF"
    fi
else
    SYSTEM_GDKPIXBUF=NO
fi
AC_SUBST(SYSTEM_GDKPIXBUF)

dnl ===================================================================
dnl Test whether to build GLib or rely on the system version
dnl ===================================================================

if test "$ENABLE_LIBRSVG" != NO; then
    AC_MSG_CHECKING([whether to use the system GLib])

    dnl As long as the only thing we need GLib for is below
    dnl librsvg, use the same --enable-librsvg (possibly implied
    dnl by --with-system-libs) to override this.

    if test "$SYSTEM_LIBRSVG" = YES; then
        SYSTEM_GLIB=YES
    elif test "$enable_librsvg" = fully-internal; then
        SYSTEM_GLIB=NO
    else
        case "$_os" in
        WINNT|Darwin|iOS|Android)
            SYSTEM_GLIB=NO
            ;;
        *)
            SYSTEM_GLIB=YES
            ;;
        esac
    fi

    if test "$SYSTEM_GLIB" = "YES"; then
        AC_MSG_RESULT([yes])
        PKG_CHECK_MODULES( GLIB, glib-2.0 )
    else
        AC_MSG_RESULT([no])
        BUILD_TYPE="$BUILD_TYPE GLIB"
    fi
else
    GLIB_CFLAGS=
    GLIB_LIBS=
    SYSTEM_GLIB=NO
fi
AC_SUBST(SYSTEM_GLIB)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

dnl ===================================================================
dnl Test whether to use liblangtag
dnl ===================================================================
ENABLE_LIBLANGTAG=
SYSTEM_LIBLANGTAG=
AC_MSG_CHECKING([whether to use liblangtag])
if test "$enable_liblangtag" = "yes" -o \( "$enable_liblangtag" = "" -a $_os != Android -a $_os != iOS \); then
    ENABLE_LIBLANGTAG=YES
    AC_MSG_RESULT([yes])
    AC_MSG_CHECKING([whether to use system liblangtag])
    if test "$with_system_liblangtag" = yes; then
        SYSTEM_LIBLANGTAG=YES
        AC_MSG_RESULT([yes])
        PKG_CHECK_MODULES( LIBLANGTAG, liblangtag >= 0.3)
    else
        SYSTEM_LIBLANGTAG=NO
        AC_MSG_RESULT([no])
        BUILD_TYPE="$BUILD_TYPE LIBLANGTAG"
    fi
else
    ENABLE_LIBLANGTAG=NO
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_LIBLANGTAG)
AC_SUBST(SYSTEM_LIBLANGTAG)
AC_SUBST(LIBLANGTAG_CFLAGS)
AC_SUBST(LIBLANGTAG_LIBS)

dnl ===================================================================
dnl Test whether to build gettext runtime (libintl) or rely on the
dnl system version
dnl ===================================================================

AC_MSG_CHECKING([whether to use the system gettext runtime])

if test -z "$with_system_gettext"; then
    case "$_os" in
    WINNT|Darwin|iOS|Android)
        with_system_gettext=no
        ;;
    *)
        with_system_gettext=yes
        ;;
    esac
fi
if test "$with_system_gettext" = yes; then
    SYSTEM_GETTEXT=YES
    AC_MSG_RESULT([yes])
elif test "$with_system_gettext" = no; then
    SYSTEM_GETTEXT=NO
    dnl gettext is only used by the librsvg stack, so let us not build
    dnl it unless it is actually needed
    if test "$ENABLE_LIBRSVG" != NO; then
        BUILD_TYPE="$BUILD_TYPE GETTEXT"
    fi
    AC_MSG_RESULT([no])
else
    AC_MSG_ERROR([bad --with-system-gettext=$with_system_gettext])
fi
AC_SUBST(SYSTEM_GETTEXT)

dnl ===================================================================
dnl Test whether to build libcroco or rely on the system version
dnl ===================================================================

if test "$ENABLE_LIBRSVG" != NO; then
    AC_MSG_CHECKING([whether to use the system libcroco])

    dnl As long as the only thing we need libcroco for is below
    dnl librsvg, use the same --enable-librsvg (possibly implied
    dnl by --with-system-libs) to override this.

    if test "$SYSTEM_LIBRSVG" = YES; then
        SYSTEM_LIBCROCO=YES
    elif test "$enable_librsvg" = fully-internal; then
        SYSTEM_LIBCROCO=NO
    else
        case "$_os" in
        WINNT|Darwin|iOS|Android)
            SYSTEM_LIBCROCO=NO
            ;;
        *)
            SYSTEM_LIBCROCO=YES
            ;;
        esac
    fi

    if test "$SYSTEM_LIBCROCO" = YES; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
        BUILD_TYPE="$BUILD_TYPE LIBCROCO"
    fi
else
    SYSTEM_LIBCROCO=NO
fi
AC_SUBST(SYSTEM_LIBCROCO)

dnl ===================================================================
dnl Test whether to build Pango or rely on the system version
dnl ===================================================================

if test "$ENABLE_LIBRSVG" != NO; then
    AC_MSG_CHECKING([whether to use the system pango])

    dnl As long as the only thing we need Pango for is below
    dnl librsvg, use the same --enable-librsvg (possibly implied
    dnl by --with-system-libs) to override this.

    if test "$SYSTEM_LIBRSVG" = YES; then
        SYSTEM_PANGO=YES
    elif test "$enable_librsvg" = fully-internal; then
        SYSTEM_PANGO=NO
    else
        case "$_os" in
        WINNT|Darwin|iOS|Android)
            SYSTEM_PANGO=NO
            ;;
        *)
            SYSTEM_PANGO=YES
            ;;
        esac
    fi

    if test "$SYSTEM_PANGO" = YES; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
        BUILD_TYPE="$BUILD_TYPE PANGO"
    fi
else
    SYSTEM_PANGO=NO
fi
AC_SUBST(SYSTEM_PANGO)

dnl ===================================================================
dnl Test whether to build libgsf or rely on the system version
dnl ===================================================================

if test "$ENABLE_LIBRSVG" != NO; then
    AC_MSG_CHECKING([whether to use the system libgsf])

    dnl As long as the only thing we need libgsf for is below librsvg (is
    dnl it?), use the same --enable-librsvg (possibly implied by
    dnl --with-system-libs) to override this.

    if test "$SYSTEM_LIBRSVG" = YES; then
        SYSTEM_LIBGSF=YES
    elif test "$enable_librsvg" = fully-internal; then
        SYSTEM_LIBGSF=NO
    else
        case "$_os" in
        WINNT|Darwin|iOS|Android)
            SYSTEM_LIBGSF=NO
            ;;
        *)
            SYSTEM_LIBGSF=YES
            ;;
        esac
    fi

    if test "$SYSTEM_LIBGSF" = YES; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
        BUILD_TYPE="$BUILD_TYPE LIBGSF"
    fi
else
    SYSTEM_LIBGSF=NO
fi
AC_SUBST(SYSTEM_LIBGSF)

dnl ===================================================================
dnl Test whether to build libpng or rely on the system version
dnl ===================================================================

AC_MSG_CHECKING([whether to use the system libpng])

dnl How should and does this interact with the checks for libpng
dnl related to use of libpng in the quickstarter above? This needs to
dnl be unified.

if test "$with_system_libpng" = "auto"; then
    case "$_os" in
    WINNT|Darwin|iOS|Android)
        with_system_libpng="$with_system_libs"
        ;;
    *)
        with_system_libpng=yes
        ;;
    esac
fi

if test "$with_system_libpng" = yes; then
    SYSTEM_LIBPNG=YES
    AC_MSG_RESULT([yes])
else
    SYSTEM_LIBPNG=NO
    BUILD_TYPE="$BUILD_TYPE LIBPNG"
    PNG_TARBALL=9e5d864bce8f06751bbd99962ecf4aad-libpng-1.5.10.tar.gz
    AC_MSG_RESULT([no])
fi
AC_SUBST(SYSTEM_LIBPNG)
AC_SUBST(PNG_TARBALL)

dnl ===================================================================
dnl Check for runtime JVM search path
dnl ===================================================================
if test "$SOLAR_JAVA" != ""; then
    AC_MSG_CHECKING([whether to use specific JVM search path at runtime])
    if test -n "$with_jvm_path" -a "$with_jvm_path" != "no"; then
        AC_MSG_RESULT([yes])
        if ! test -d "$with_jvm_path"; then
            AC_MSG_ERROR(["$with_jvm_path" not a directory])
        fi
        if ! test -d "$with_jvm_path"jvm; then
            AC_MSG_ERROR(["$with_jvm_path"jvm not found, point with_jvm_path to \[/path/to/\]jvm])
        fi
        JVM_ONE_PATH_CHECK="$with_jvm_path"
        AC_SUBST(JVM_ONE_PATH_CHECK)
    else
        AC_MSG_RESULT([no])
    fi
fi

dnl ===================================================================
dnl Test for the presence of Ant and that it works
dnl ===================================================================

if test "$SOLAR_JAVA" != ""; then
    ANT_HOME=; export ANT_HOME
    WITH_ANT_HOME=; export WITH_ANT_HOME
    if test -z "$with_ant_home"; then
        AC_PATH_PROGS(ANT, [jakarta-ant ant ant.sh ant.bat ant.cmd])
    else
        if test "$_os" = "WINNT"; then
            with_ant_home=`cygpath -u "$with_ant_home"`
        fi
        AC_PATH_PROGS(ANT, [jakarta-ant ant ant.sh ant.bat ant.cmd],,$with_ant_home/bin:$PATH)
        WITH_ANT_HOME=$with_ant_home
        ANT_HOME=$with_ant_home
    fi

    if test -z "$ANT"; then
        AC_MSG_ERROR([Ant not found - Make sure it's in the path or use --with-ant-home])
    else
        # resolve relative or absolute symlink
        while test -h "$ANT"; do
            a_cwd=`pwd`
            a_basename=`basename "$ANT"`
            a_script=`ls -l "$ANT" | $SED "s/.*${a_basename} -> //g"`
            cd "`dirname "$ANT"`"
            cd "`dirname "$a_script"`"
            ANT="`pwd`"/"`basename "$a_script"`"
            cd "$a_cwd"
        done

        AC_MSG_CHECKING([if $ANT works])
        cat > conftest.java << EOF
        public class conftest {
            int testmethod(int a, int b) {
                    return a + b;
            }
        }
EOF

        cat > conftest.xml << EOF
        <project name="conftest" default="conftest">
        <target name="conftest">
            <javac srcdir="." includes="conftest.java">
            </javac>
        </target>
        </project>
EOF

        oldJAVA_HOME=$JAVA_HOME
        if test "$JAVACISGCJ" = "yes"; then
            JAVA_HOME=; export JAVA_HOME
            ant_gcj="-Dbuild.compiler=gcj"
        fi
        AC_TRY_COMMAND("$ANT" $ant_gcj -buildfile conftest.xml 1>&2)
        if test $? = 0 -a -f ./conftest.class; then
            AC_MSG_RESULT([Ant works])
            if test -z "$WITH_ANT_HOME"; then
                ANT_HOME=`"$ANT" -diagnostics | $EGREP "ant.home :" | $SED -e "s#ant.home : ##g"`
                if test -z "$ANT_HOME"; then
                    ANT_HOME=`echo "$ANT" | $SED -n "s/\/bin\/ant.*\$//p"`
                fi
            else
                ANT_HOME="$WITH_ANT_HOME"
            fi
        else
            echo "configure: Ant test failed" >&5
            cat conftest.java >&5
            cat conftest.xml >&5
            AC_MSG_WARN([Ant does not work - Some Java projects will not build!])
            ANT_HOME=""
            add_warning "Ant does not work - Some Java projects will not build!"
        fi
        JAVA_HOME=$oldJAVA_HOME
        rm -f conftest* core core.* *.core
    fi
    if test -z "$ANT_HOME"; then
        ANT_HOME="NO_ANT_HOME"
    else
        PathFormat "$ANT_HOME"
        ANT_HOME="$formatted_path"
        PathFormat "$ANT"
        ANT="$ANT"
    fi
    AC_SUBST(ANT_HOME)
    AC_SUBST(ANT)

    dnl Checking for ant.jar
    if test "$ANT_HOME" != "NO_ANT_HOME"; then
        AC_MSG_CHECKING([Ant lib directory])
        if test -f $ANT_HOME/lib/ant.jar; then
            ANT_LIB="$ANT_HOME/lib"
        else
            if test -f $ANT_HOME/ant.jar; then
                ANT_LIB="$ANT_HOME"
            else
                if test -f /usr/share/java/ant.jar; then
                    ANT_LIB=/usr/share/java
                else
                    if test -f /usr/share/ant-core/lib/ant.jar; then
                        ANT_LIB=/usr/share/ant-core/lib
                    else
                        if test -f $ANT_HOME/lib/ant/ant.jar; then
                            ANT_LIB="$ANT_HOME/lib/ant"
                        else
                            if test -f /usr/share/lib/ant/ant.jar; then
                                ANT_LIB=/usr/share/lib/ant
                            else
                                AC_MSG_ERROR([Ant libraries not found!])
                            fi
                        fi
                    fi
                fi
            fi
        fi
        PathFormat "$ANT_LIB"
        ANT_LIB="$formatted_path"
        AC_MSG_RESULT([Ant lib directory found.])
    fi
    AC_SUBST(ANT_LIB)

    ant_minver=1.6.0
    ant_minminor1=`echo $ant_minver | cut -d"." -f2`

    AC_MSG_CHECKING([whether Ant is >= $ant_minver])
    ant_version=`"$ANT" -version | $AWK '{ print $4; }'`
    ant_version_major=`echo $ant_version | cut -d. -f1`
    ant_version_minor=`echo $ant_version | cut -d. -f2`
    echo "configure: ant_version $ant_version " >&5
    echo "configure: ant_version_major $ant_version_major " >&5
    echo "configure: ant_version_minor $ant_version_minor " >&5
    if test "$ant_version_major" -ge "2"; then
        AC_MSG_RESULT([yes, $ant_version])
    elif test "$ant_version_major" = "1" -a "$ant_version_minor" -ge "$ant_minminor1"; then
        AC_MSG_RESULT([yes, $ant_version])
    else
        AC_MSG_ERROR([no, you need at least Ant >= $ant_minver])
    fi

    if test "$ENABLE_MEDIAWIKI" = "YES"; then
        AC_MSG_CHECKING([whether Ant supports mapper type="regexp"])
        rm -rf confdir
        mkdir confdir
        cat > conftest.java << EOF
            public class conftest {
                int testmethod(int a, int b) {
                    return a + b;
                }
            }
EOF

        cat > conftest.xml << EOF
            <project name="conftest" default="conftest">
            <target name="conftest" depends="copytest">
                <javac srcdir="." includes="conftest.java">
                </javac>
            </target>
            <target name="copytest">
                 <copy todir="confdir">
                 <fileset dir="confdir" includes="**/*.abc" casesensitive="yes"/>
                 <filterset/>
                 <mapper type="regexp" from="^(.*[/\\])foo([/\\].*)" to="\1baa\2"/>
                 </copy>
            </target>
            </project>
EOF

        if test "$JAVACISGCJ" = "yes"; then
            JAVA_HOME=; export JAVA_HOME
            ant_gcj="-Dbuild.compiler=gcj"
        fi
        AC_TRY_COMMAND("$ANT" $ant_gcj -buildfile conftest.xml 1>&2)
        if test $? = 0 -a -f ./conftest.class; then
            AC_MSG_RESULT([yes])
            rm -rf confdir
        else
            echo "configure: Ant test failed" >&5
            cat conftest.java >&5
            cat conftest.xml >&5
            rm -rf confdir
            AC_MSG_ERROR([no. Did you install ant-apache-regexp?])
        fi
    fi
    rm -f conftest* core core.* *.core
fi

OOO_JUNIT_JAR=
if test "$SOLAR_JAVA" != "" -a "$with_junit" != "no"; then
    AC_MSG_CHECKING([for JUnit 4])
    if test "$with_junit" = "yes"; then
        if test -e /usr/share/java/junit4.jar; then
            OOO_JUNIT_JAR=/usr/share/java/junit4.jar
        else
           if test -e /usr/share/lib/java/junit.jar; then
              OOO_JUNIT_JAR=/usr/share/lib/java/junit.jar
           else
              OOO_JUNIT_JAR=/usr/share/java/junit.jar
           fi
        fi
    else
        OOO_JUNIT_JAR=$with_junit
    fi
    if test "$_os" = "WINNT"; then
        OOO_JUNIT_JAR=`cygpath -m "$OOO_JUNIT_JAR"`
    fi
    "$JAVA_HOME/bin/jar" tf "$OOO_JUNIT_JAR" 2>&5 | \
        grep org/junit/Before.class > /dev/null 2>&5
    if test $? -eq 0; then
        # check if either class-path entry is available for hamcrest or
        # it's bundled
        if "$JAVA_HOME/bin/jar" tf "$OOO_JUNIT_JAR" |$GREP -q hamcrest || \
            "$UNZIP" -c "$OOO_JUNIT_JAR" META-INF/MANIFEST.MF |$GREP 'Class-Path:' | $GREP -q 'hamcrest'; then
            AC_MSG_RESULT([$OOO_JUNIT_JAR])
        else
            AC_MSG_ERROR([your junit jar neither sets a classpath nor includes hamcrest; please
provide a full junit jar or use --without-junit])
        fi
    else
        AC_MSG_RESULT([no])
        AC_MSG_ERROR([cannot find JUnit 4 jar; please install one in the default
location (/usr/share/java), specify its pathname via
--with-junit=..., or disable it via --without-junit])
    fi
    if test $OOO_JUNIT_JAR != ""; then
    BUILD_TYPE="$BUILD_TYPE QADEVOOO"
    fi
fi
AC_SUBST(OOO_JUNIT_JAR)

if test "$enable_odk" = no; then
    unset DOXYGEN
else
    if test "$with_doxygen" = no; then
        AC_MSG_CHECKING([for doxygen])
        unset DOXYGEN
        AC_MSG_RESULT([no])
    else
        if test "$with_doxygen" = yes; then
            AC_PATH_PROG([DOXYGEN], [doxygen])
            if test -z "$DOXYGEN"; then
                AC_MSG_RESULT([no])
                AC_MSG_ERROR([doxygen not found in \$PATH; specify its pathname via --with-doxygen=..., or disable its use via --without-doxygen])
            fi
        else
            AC_MSG_CHECKING([for doxygen])
            DOXYGEN=$with_doxygen
            AC_MSG_RESULT([$DOXYGEN])
        fi
    fi
fi
AC_SUBST([DOXYGEN])

dnl ===================================================================
dnl Product version
dnl ===================================================================
AC_MSG_CHECKING([for product version])
PRODUCTVERSION=AC_PACKAGE_VERSION
AC_MSG_RESULT([$PRODUCTVERSION])
AC_SUBST(PRODUCTVERSION)

dnl ===================================================================
dnl Dealing with l10n options
dnl ===================================================================
AC_MSG_CHECKING([which languages to be built])
# get list of all languages
# generate shell variable from completelangiso= from solenv/inc/langlist.mk
# the sed command does the following:
#   + if a line ends with a backslash, append the next line to it
#   + adds " on the beginning of the value (after =)
#   + adds " at the end of the value
#   + removes en-US; we want to put it on the beginning
#   + prints just the section starting with 'completelangiso=' and ending with the " at the end of line
[eval $(sed -e :a -e '/\\$/N; s/\\\n//; ta' -n -e 's/=/="/;s/\([^\\]\)$/\1"/;s/en-US//;/^completelangiso/p' solenv/inc/langlist.mk)]
ALL_LANGS="en-US $completelangiso"
# check the configured localizations
WITH_LANG="$with_lang"
if test -z "$WITH_LANG" -o "$WITH_LANG" = "en-US"; then
    AC_MSG_RESULT([en-US])
else
    AC_MSG_RESULT([$WITH_LANG])
    GIT_REPO_NAMES="$GIT_REPO_NAMES translations"
    GIT_NEEDED_SUBMODULES="translations $GIT_NEEDED_SUBMODULES"
fi
# check that the list is valid
for lang in $WITH_LANG; do
    test "$lang" = "ALL" && continue
    # need to check for the exact string, so add space before and after the list of all languages
    for vl in $ALL_LANGS; do
        if test "$vl" = "$lang"; then
           break
        fi
    done
    if test "$vl" != "$lang"; then
        AC_MSG_ERROR([invalid language: $lang; supported languages are: $ALL_LANGS])
    fi
done
# list with substituted ALL
WITH_LANG_LIST=`echo $WITH_LANG | sed "s/ALL/$ALL_LANGS/"`
# this variable is used only by bin/distro-install-* helper scripts
# they need a real list of languages
test -z "$WITH_LANG_LIST" && WITH_LANG_LIST="en-US"
test "$WITH_LANG" = "en-US" && WITH_LANG=
test -z "$WITH_LANG" || BUILD_TYPE="$BUILD_TYPE TRANSLATIONS"
AC_SUBST(ALL_LANGS)
AC_SUBST(WITH_LANG)
AC_SUBST(WITH_LANG_LIST)
AC_SUBST(GIT_REPO_NAMES)
AC_SUBST(GIT_NEEDED_SUBMODULES)


PathFormat "$SRC_ROOT/translations"
L10N_MODULE="$formatted_path"
WITH_POOR_HELP_LOCALIZATIONS=
if test -d "$SRC_ROOT/translations/source"; then
    for l in `ls -1 $SRC_ROOT/translations/source`; do
        if test ! -d "$SRC_ROOT/translations/source/$l/helpcontent2"; then
            WITH_POOR_HELP_LOCALIZATIONS="$WITH_POOR_HELP_LOCALIZATIONS $l"
        fi
    done
fi
AC_SUBST(L10N_MODULE)
AC_SUBST(WITH_POOR_HELP_LOCALIZATIONS)

dnl git-new-workdir
dnl ===================================================================
if test -n "${GIT_LINK_SRC}"; then
    for repo in ${GIT_REPO_NAMES}; do
        if ! test -d "${GIT_LINK_SRC}"/${repo}; then
            AC_MSG_ERROR([linked git: required repository does not exist: ${GIT_LINK_SRC}/${repo}])
        fi
    done
fi
AC_SUBST(GIT_LINK_SRC)

AC_MSG_CHECKING([for custom 'intro' progress bar color])
PROGRESSBARCOLOR=
if test -z "$with_intro_progressbar_color"; then
   PROGRESSBARCOLOR="255,255,255"
   AC_MSG_RESULT([none])
else
   PROGRESSBARCOLOR="$with_intro_progressbar_color"
   AC_MSG_RESULT([$PROGRESSBARCOLOR])
fi
AC_SUBST(PROGRESSBARCOLOR)

AC_MSG_CHECKING([for custom 'intro' progress bar size])
PROGRESSSIZE=
if test -z "$with_intro_progressbar_size"; then
   PROGRESSSIZE="407,6"
   AC_MSG_RESULT([none])
else
   PROGRESSSIZE="$with_intro_progressbar_size"
   AC_MSG_RESULT([$PROGRESSSIZE])
fi
AC_SUBST(PROGRESSSIZE)

AC_MSG_CHECKING([for custom 'intro' progress bar position])
PROGRESSPOSITION=
if test -z "$with_intro_progressbar_position"; then
   PROGRESSPOSITION="61,317"
   AC_MSG_RESULT([none])
else
   PROGRESSPOSITION="$with_intro_progressbar_position"
   AC_MSG_RESULT([$PROGRESSPOSITION])
fi
AC_SUBST(PROGRESSPOSITION)

AC_MSG_CHECKING([for custom 'intro' progress bar frame color])
PROGRESSFRAMECOLOR=
if test -z "$with_intro_progressbar_frame_color"; then
   PROGRESSFRAMECOLOR="20,136,3"
   AC_MSG_RESULT([none])
else
   PROGRESSFRAMECOLOR="$with_intro_progressbar_frame_color"
   AC_MSG_RESULT([$PROGRESSFRAMECOLOR])
fi
AC_SUBST(PROGRESSFRAMECOLOR)

AC_MSG_CHECKING([for custom 'intro' progress text color])
PROGRESSTEXTCOLOR=
if test -z "$with_intro_progressbar_text_color"; then
   PROGRESSTEXTCOLOR="255,255,255"
   AC_MSG_RESULT([none])
else
   PROGRESSTEXTCOLOR="$with_intro_progressbar_text_color"
   AC_MSG_RESULT([$PROGRESSTEXTCOLOR])
fi
AC_SUBST(PROGRESSTEXTCOLOR)

AC_MSG_CHECKING([for custom 'intro' progress text baseline])
PROGRESSTEXTBASELINE=
if test -z "$with_intro_progressbar_text_baseline"; then
   PROGRESSTEXTBASELINE="287"
   AC_MSG_RESULT([none])
else
   PROGRESSTEXTBASELINE="$with_intro_progressbar_text_baseline"
   AC_MSG_RESULT([$PROGRESSTEXTBASELINE])
fi
AC_SUBST(PROGRESSTEXTBASELINE)

AC_MSG_CHECKING([for alternative branding images directory])
INTRO_BITMAP=
ABOUT_BACKGROUND_SVG=
FLAT_LOGO_SVG=
STARTCENTER_LEFT_BITMAP=
STARTCENTER_RIGHT_BITMAP=
STARTCENTER_RTL_LEFT_BITMAP=
STARTCENTER_RTL_RIGHT_BITMAP=
STARTCENTER_SPACE_BITMAP=
if test -z "$with_branding" -o "$with_branding" = "no"; then
    AC_MSG_RESULT([none])
else
    INTRO_BITMAP="$with_branding/intro.png"
    FLAT_LOGO_SVG="$with_branding/flat_logo.svg"
    STARTCENTER_LEFT_BITMAP="$with_branding/backing_left.png"
    STARTCENTER_RIGHT_BITMAP="$with_branding/backing_right.png"
    STARTCENTER_RTL_LEFT_BITMAP="$with_branding/backing_rtl_left.png"
    STARTCENTER_RTL_RIGHT_BITMAP="$with_branding/backing_rtl_right.png"
    STARTCENTER_SPACE_BITMAP="$with_branding/backing_space.png"
    for i in $INTRO_BITMAP \
             $FLAT_LOGO_SVG \
             $STARTCENTER_LEFT_BITMAP \
             $STARTCENTER_RIGHT_BITMAP \
             $STARTCENTER_RTL_LEFT_BITMAP \
             $STARTCENTER_RTL_RIGHT_BITMAP \
             $STARTCENTER_SPACE_BITMAP; do
        if test ! -f $i; then
            AC_MSG_WARN([Required file $i does not exist!])
        fi
    done
    AC_MSG_RESULT([$with_branding])
fi

AC_MSG_CHECKING([for yet another 'intro' bitmap])
if test -z "$with_intro_bitmap" -o "$with_intro_bitmap" = "no"; then
    if test "$with_intro_bitmap" = "no"; then
        INTRO_BITMAP=
    fi
    AC_MSG_RESULT([none])
else
    case "$with_intro_bitmap" in
    *.png) INTRO_BITMAP="$with_intro_bitmap" ;;
    *)     AC_MSG_WARN([Intro bitmap should be a .png file!]) ;;
    esac
    AC_MSG_RESULT([$INTRO_BITMAP])
fi
AC_SUBST(INTRO_BITMAP)

AC_MSG_CHECKING([for yet another 'about' svg])
if test -z "$with_about_background_svg" -o "$with_about_background_svg" = "no" ; then
    if test "$with_about_background_svg" = "no" ; then
        ABOUT_BACKGROUND_SVG=
    fi
    AC_MSG_RESULT([none])
else
   case "$with_about_background_svg" in
   *.svg) ABOUT_BACKGROUND_SVG="$with_about_background_svg" ;;
   *)     AC_MSG_WARN([About background SVG should be a .svg file!]) ;;
   esac
   AC_MSG_RESULT([$ABOUT_BACKGROUND_SVG])
fi
AC_SUBST(ABOUT_BACKGROUND_SVG)

AC_MSG_CHECKING([for yet another 'flat_logo' svg])
if test -z "$with_flat_logo_svg" -o "$with_flat_logo_svg" = "no" ; then
    if test "$with_flat_logo_svg" = "no" ; then
        FLAT_LOGO_SVG=
    fi
    AC_MSG_RESULT([none])
else
   case "$with_flat_logo_svg" in
   *.svg) FLAT_LOGO_SVG="$with_flat_logo_svg" ;;
   *)     AC_MSG_WARN([Flat logo SVG should be a .svg file!]) ;;
   esac
   AC_MSG_RESULT([$ABOUT_BITMAP])
fi
AC_SUBST(FLAT_LOGO_SVG)

AC_MSG_CHECKING([for yet another 'start center left' bitmap])
if test -z "$with_startcenter_left_bitmap" -o "$with_startcenter_left_bitmap" = "no"; then
   if test "$with_startcenter_left_bitmap" = "no"; then
      STARTCENTER_LEFT_BITMAP=
   fi
   AC_MSG_RESULT([none])
else
   case "$with_startcenter_left_bitmap" in
      *.png) STARTCENTER_LEFT_BITMAP="$with_startcenter_left_bitmap" ;;
      *)     AC_MSG_WARN([Startcenter left bitmap should be a .png file!]) ;;
   esac
   AC_MSG_RESULT([$STARTCENTER_LEFT_BITMAP])
fi
AC_SUBST(STARTCENTER_LEFT_BITMAP)

AC_MSG_CHECKING([for yet another 'start center right' bitmap])
if test -z "$with_startcenter_right_bitmap" -o "$with_startcenter_right_bitmap" = "no"; then
   if test "$with_startcenter_right_bitmap" = "no"; then
      STARTCENTER_RIGHT_BITMAP=
   fi
   AC_MSG_RESULT([none])
else
   case "$with_startcenter_right_bitmap" in
      *.png) STARTCENTER_RIGHT_BITMAP="$with_startcenter_right_bitmap" ;;
      *)     AC_MSG_WARN([Startcenter right bitmap should be a .png file!]) ;;
   esac
   AC_MSG_RESULT([$STARTCENTER_RIGHT_BITMAP])
fi
AC_SUBST(STARTCENTER_RIGHT_BITMAP)

AC_MSG_CHECKING([for yet another 'start center rtl left' bitmap])
if test -z "$with_startcenter_rtl_left_bitmap" -o "$with_startcenter_rtl_left_bitmap" = "no"; then
   if test "$with_startcenter_rtl_left_bitmap" = "no"; then
      STARTCENTER_RTL_LEFT_BITMAP=
   fi
   AC_MSG_RESULT([none])
else
   case "$with_startcenter_rtl_left_bitmap" in
      *.png) STARTCENTER_RTL_LEFT_BITMAP="$with_startcenter_rtl_left_bitmap" ;;
      *)     AC_MSG_WARN([Startcenter rtl left bitmap should be a .png file!]) ;;
   esac
   AC_MSG_RESULT([$STARTCENTER_RTL_LEFT_BITMAP])
fi
AC_SUBST(STARTCENTER_RTL_LEFT_BITMAP)

AC_MSG_CHECKING([for yet another 'start center rtl right' bitmap])
if test -z "$with_startcenter_rtl_right_bitmap" -o "$with_startcenter_rtl_right_bitmap" = "no"; then
   if test "$with_startcenter_rtl_right_bitmap" = "no"; then
      STARTCENTER_RTL_RIGHT_BITMAP=
   fi
   AC_MSG_RESULT([none])
else
   case "$with_startcenter_rtl_right_bitmap" in
      *.png) STARTCENTER_RTL_RIGHT_BITMAP="$with_startcenter_rtl_right_bitmap" ;;
      *)     AC_MSG_WARN([Startcenter rtl right bitmap should be a .png file!]) ;;
   esac
   AC_MSG_RESULT([$STARTCENTER_RTL_RIGHT_BITMAP])
fi
AC_SUBST(STARTCENTER_RTL_RIGHT_BITMAP)

AC_MSG_CHECKING([for yet another 'start center space' bitmap])
if test -z "$with_startcenter_space_bitmap" -o "$with_startcenter_space_bitmap" = "no"; then
   if test "$with_startcenter_space_bitmap" = "no"; then
      STARTCENTER_SPACE_BITMAP=
   fi
   AC_MSG_RESULT([none])
else
   case "$with_startcenter_space_bitmap" in
      *.png) STARTCENTER_SPACE_BITMAP="$with_startcenter_space_bitmap" ;;
      *)     AC_MSG_WARN([Startcenter left bitmap should be a .png file!]) ;;
   esac
   AC_MSG_RESULT([$STARTCENTER_SPACE_BITMAP])
fi
AC_SUBST(STARTCENTER_SPACE_BITMAP)

OOO_VENDOR=
AC_MSG_CHECKING([for vendor])
if test -z "$with_vendor" -o "$with_vendor" = "no"; then
    AC_MSG_RESULT([not set])
else
    OOO_VENDOR="$with_vendor"
    AC_MSG_RESULT([$OOO_VENDOR])
fi
AC_SUBST(OOO_VENDOR)

UNIXWRAPPERNAME=
AC_MSG_CHECKING([for UNIX wrapper name])
if test -z "$with_unix_wrapper" -o "$with_unix_wrapper" = "no"  -o "$with_unix_wrapper" = "yes"; then
    AC_MSG_RESULT([not set])
else
    UNIXWRAPPERNAME="$with_unix_wrapper"
    AC_MSG_RESULT([$UNIXWRAPPERNAME])
fi
AC_SUBST(UNIXWRAPPERNAME)

AC_MSG_CHECKING([whether to install the compat oo* wrappers])
if test "$with_compat_oowrappers" = "yes"; then
    WITH_COMPAT_OOWRAPPERS=YES
    AC_MSG_RESULT(yes)
else
    WITH_COMPAT_OOWRAPPERS=
    AC_MSG_RESULT(no)
fi
AC_SUBST(WITH_COMPAT_OOWRAPPERS)

AC_MSG_CHECKING([for product name])
PRODUCTNAME=AC_PACKAGE_NAME
AC_MSG_RESULT([$PRODUCTNAME])
AC_SUBST(PRODUCTNAME)

INSTALLDIRNAME=`echo AC_PACKAGE_NAME | tr '[[:upper:]]' '[[:lower:]]'`
AC_MSG_CHECKING([for install dirname])
if test -n "$with_install_dirname" -a "$with_install_dirname" != "no" -a "$with_install_dirname" != "yes"; then
   INSTALLDIRNAME="$with_install_dirname"
fi
AC_MSG_RESULT([$INSTALLDIRNAME])
AC_SUBST(INSTALLDIRNAME)

AC_MSG_CHECKING([for prefix])
test "x$prefix" = xNONE && prefix=$ac_default_prefix
test "x$exec_prefix" = xNONE && exec_prefix=$prefix
PREFIXDIR="$prefix"
AC_MSG_RESULT([$PREFIXDIR])
AC_SUBST(PREFIXDIR)

AC_MSG_CHECKING([for libdir])
LIBDIR=[$(eval echo $(eval echo $libdir))]
AC_MSG_RESULT([$LIBDIR])
AC_SUBST(LIBDIR)

AC_MSG_CHECKING([for data dir])
DATADIR=[$(eval echo $(eval echo $datadir))]
AC_MSG_RESULT([$DATADIR])
AC_SUBST(DATADIR)

AC_MSG_CHECKING([for man dir])
MANDIR=[$(eval echo $(eval echo $mandir))]
AC_MSG_RESULT([$MANDIR])
AC_SUBST(MANDIR)

AC_MSG_CHECKING([for doc dir])
DOCDIR=[$(eval echo $(eval echo $docdir))]
AC_MSG_RESULT([$DOCDIR])
AC_SUBST(DOCDIR)

AC_MSG_CHECKING([for install dir])
INSTALLDIR="$LIBDIR/$INSTALLDIRNAME"
AC_MSG_RESULT([$INSTALLDIR])
AC_SUBST(INSTALLDIR)

AC_MSG_CHECKING([for dev-install dir])
DEVINSTALLDIR="$OUTDIR/installation"
AC_MSG_RESULT([$DEVINSTALLDIR])
AC_SUBST(DEVINSTALLDIR)

# ===================================================================
# De- or increase default verbosity of build process
# ===================================================================
AC_MSG_CHECKING([build verbosity])
if test -n "$enable_verbose"; then
    if test "$enable_verbose" = "yes"; then
        VERBOSE="TRUE"
        AC_MSG_RESULT([high])
    fi
    if test "$enable_verbose" = "no"; then
        VERBOSE="FALSE"
        AC_MSG_RESULT([low])
    fi
else
    AC_MSG_RESULT([not set])
fi
AC_SUBST(VERBOSE)

dnl ===================================================================
dnl Use zenity during build
dnl ===================================================================
AC_MSG_CHECKING([whether to use zenity during build])
if test "$enable_zenity" = "yes"; then
    AC_MSG_RESULT([yes])
    AC_PATH_PROGS( ZNTY, zenity )
    if test -z "$ZNTY"; then
        ENABLE_ZENITY=FALSE
    else
        ENABLE_ZENITY=TRUE
    fi
else
    ENABLE_ZENITY=FALSE
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_ZENITY)


dnl ===================================================================
dnl Hook up LibreOffice's nodep environmental variable to automake's equivalent
dnl --enable-dependency-tracking configure option
dnl ===================================================================
AC_MSG_CHECKING([whether to enable dependency tracking])
if test "$enable_dependency_tracking" = "no"; then
    nodep=TRUE
    AC_MSG_RESULT([no])
else
    AC_MSG_RESULT([yes])
fi
AC_SUBST(nodep)

dnl ===================================================================
dnl Number of CPUs to use during the build
dnl ===================================================================
AC_MSG_CHECKING([for number of processors to use])
if test -n "$with_num_cpus"; then
    BUILD_NCPUS=$with_num_cpus
else
    case `uname -s` in

    Darwin|FreeBSD|NetBSD|OpenBSD)
        BUILD_NCPUS=`sysctl -n hw.ncpu`
        ;;

    Linux)
        BUILD_NCPUS=`getconf _NPROCESSORS_ONLN`
    ;;
    # what else than above does profit here *and* has /proc?
    *)
        BUILD_NCPUS=`grep $'^processor\t*:' /proc/cpuinfo | wc -l`
        ;;
    esac

    # If we hit the catch-all case, but /proc/cpuinfo doesn't exist or has an
    # unexpected format, 'wc -l' will have returned 0.
    if test "$BUILD_NCPUS" -eq 0; then
        BUILD_NCPUS=1
    fi
fi
AC_MSG_RESULT([$BUILD_NCPUS])
AC_SUBST(BUILD_NCPUS)

#
# Set up ILIB for MSVC build
#
if test "$build_os" = "cygwin"; then
    ILIB=".;${SOLARVER}/$INPATH/lib"
    if test -n "$JAVA_HOME" -a "$JAVA_HOME" != "NO_JAVA_HOME"; then
        ILIB="$ILIB;$JAVA_HOME/lib"
    fi
    if test "$BITNESS_OVERRIDE" = 64; then
        ILIB="$ILIB;$COMPATH/lib/amd64"
        ILIB="$ILIB;$WINDOWS_SDK_HOME/lib64"
        if test "$WINDOWS_SDK_VERSION" = "80"; then
            ILIB="$ILIB;$WINDOWS_SDK_HOME/lib/win8/um/x64"
        fi
    else
        ILIB="$ILIB;$COMPATH/lib"
        ILIB="$ILIB;$WINDOWS_SDK_HOME/lib"
        if test "$WINDOWS_SDK_VERSION" = "80"; then
            ILIB="$ILIB;$WINDOWS_SDK_HOME/lib/win8/um/x86"
        fi
    fi
    ILIB="$ILIB;$DOTNET_FRAMEWORK_HOME/lib"

    if test "$ENABLE_DIRECTX" = "TRUE"; then
        ILIB="$ILIB;$DIRECTXSDK_LIB"
    fi
    AC_SUBST(ILIB)
fi


# ===================================================================
# Creating bigger shared library to link against
# ===================================================================
AC_MSG_CHECKING([whether to create huge library])
MERGELIBS=
if test "$enable_mergelibs" = "yes" ; then
    if test $WITH_BINFILTER = YES; then
        add_warning "--enable-mergelibs conflicts with --enable-binfilter. Disabling mergelibs."
        AC_MSG_RESULT([no])
    else
        if test $_os != Linux -a $_os != Android -a $_os != WINNT; then
            add_warning "--enable-mergelibs is not tested for this platform"
        fi
        MERGELIBS="TRUE"
        AC_MSG_RESULT([yes])
    fi
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(MERGELIBS)

dnl ===================================================================
dnl Number of parallel jobs to be executed by dmake
dnl ===================================================================
AC_MSG_CHECKING([for maximum of jobs per processor])
BUILD_MAX_JOBS="1"
if test "$with_max_jobs" != ""; then
    BUILD_MAX_JOBS="$with_max_jobs"
else
    if test "$enable_icecream" = "yes"; then
        BUILD_MAX_JOBS="10"
    fi
fi
AC_MSG_RESULT([$BUILD_MAX_JOBS])
AC_SUBST(BUILD_MAX_JOBS)

dnl ===================================================================
dnl icerun is a wrapper that stops us spawning tens of processes
dnl locally - for tools that can't be executed on the compile cluster
dnl this avoids a dozen javac's ganging up on your laptop to kill it.
dnl ===================================================================
ICECREAM_RUN=
if test "$enable_icecream" = "yes"; then
   ICECREAM_RUN=icerun
fi
AC_SUBST(ICECREAM_RUN)

# =====================================================================
# determine the parallelism for gnu make
# =====================================================================
AC_MSG_CHECKING([for maximum parallelism for gmake])
if test $BUILD_MAX_JOBS -gt $BUILD_NCPUS; then
    GMAKE_PARALLELISM="$BUILD_MAX_JOBS"
else
    GMAKE_PARALLELISM="$BUILD_NCPUS"
fi
GMAKE_MODULE_PARALLELISM="$BUILD_MAX_JOBS"
if test "$no_parallelism_make" = "YES"; then
    if test -z "$with_num_cpus"; then
        GMAKE_PARALLELISM="1"
        if test $GMAKE_MODULE_PARALLELISM -gt 1; then
            AC_MSG_WARN([gmake 3.81 crashes with parallelism > 1, reducing it to 1. upgrade to 3.82 to avoid this.])
            if test -z "$with_num_cpus"; then
                add_warning "gmake 3.81 crashes with parallelism > 1, reducing it to 1. upgrade to 3.82 to avoid this."
            fi
            GMAKE_MODULE_PARALLELISM="1"
        fi
    else
        GMAKE_PARALLELISM="$BUILD_NCPUS"
        add_warning "make 3.81 is prone to crashes with parallelism > 1. Since --with-num-cpus was explicitly given, it is honored, but do not complain when make segfault on you."
    fi
fi

# GMAKE_PARALLELISM is used in tail_build
# GMAKE_MODULE_PARALLELISM is used when building individual gbuildified module
AC_MSG_RESULT([per module:$GMAKE_MODULE_PARALLELISM, for tail_build:$GMAKE_PARALLELISM])
AC_SUBST(GMAKE_PARALLELISM)
AC_SUBST(GMAKE_MODULE_PARALLELISM)

dnl ===================================================================
dnl Number of parallel jobs to be executed during subsequenttest
dnl ===================================================================
AC_MSG_CHECKING([for maximum no. of jobs during subsequenttest])
CHECK_PARALLELISM=$GMAKE_PARALLELISM
if test "$with_check_jobs" != ""; then
    CHECK_PARALLELISM="$with_check_jobs"
fi
AC_MSG_RESULT([$CHECK_PARALLELISM])
AC_SUBST(CHECK_PARALLELISM)

dnl ===================================================================
dnl Setting up the environment.
dnl ===================================================================
echo "setting up the build environment variables..."

AC_SUBST(COMPATH)

if test "$build_os" = "cygwin"; then
    if test "$DISABLE_ACTIVEX" = "YES"; then
        ATL_LIB="$WINDOWS_SDK_HOME/lib" # Doesn't exist for VSE
        ATL_INCLUDE="$WINDOWS_SDK_HOME/include/atl"
        MFC_LIB="$WINDOWS_SDK_HOME/lib" # Doesn't exist for VSE
        MFC_INCLUDE="$WINDOWS_SDK_HOME/include/mfc"
    else
        ATL_LIB="$COMPATH/atlmfc/lib"
        ATL_INCLUDE="$COMPATH/atlmfc/include"
        MFC_LIB="$COMPATH/atlmfc/lib"
        MFC_INCLUDE="$WINDOWS_SDK_HOME/include/mfc"
    fi
    if test "$BITNESS_OVERRIDE" = 64; then
        ATL_LIB="$ATL_LIB/amd64"
        MFC_LIB="$MFC_LIB/amd64"
    fi
    PathFormat "/usr/bin/grep.exe"
    WIN_GREP="$formatted_path"
    PathFormat "/usr/bin/find.exe"
    WIN_FIND="$formatted_path"
    PathFormat "/usr/bin/ls.exe"
    WIN_LS="$formatted_path"
    PathFormat "/usr/bin/touch.exe"
    WIN_TOUCH="$formatted_path"
fi

AC_SUBST(ATL_INCLUDE)
AC_SUBST(ATL_LIB)
AC_SUBST(MFC_INCLUDE)
AC_SUBST(MFC_LIB)
AC_SUBST(WIN_FIND)
AC_SUBST(WIN_GREP)
AC_SUBST(WIN_LS)
AC_SUBST(WIN_TOUCH)

AC_SUBST(BUILD_TYPE)

AC_SUBST(SOLARINC)
AC_SUBST(SOLARLIB)

if test -z "$SOLARINC_FOR_BUILD"; then
    SOLARINC_FOR_BUILD="$SOLARINC"
fi
AC_SUBST(SOLARINC_FOR_BUILD)

if test -z "$SOLARLIB_FOR_BUILD"; then
    SOLARLIB_FOR_BUILD="$SOLARLIB"
fi
AC_SUBST(SOLARLIB_FOR_BUILD)

PathFormat "$PERL"
PERL="$formatted_path"
AC_SUBST(PERL)

if test "$build_os" = "cygwin"; then
    TEMP_DIRECTORY=`cygpath -m "/tmp"`
    if test -n "$TMP"; then
        TMP_DIRECTORY="$TMP"
    else
        TMP_DIRECTORY="$TEMP_DIRECTORY"
    fi
else
    if test -n "$TMPDIR"; then
        TEMP_DIRECTORY="$TMPDIR"
    else
        TEMP_DIRECTORY="/tmp"
    fi
fi
AC_SUBST(TEMP_DIRECTORY)
AC_SUBST(TMP_DIRECTORY)

# setup the PATH for the environment
if test -n "$LO_PATH_FOR_BUILD"; then
    LO_PATH="$LO_PATH_FOR_BUILD"
else
    LO_PATH="$PATH"

    case "$host_os" in

    aix*|dragonfly*|freebsd*|linux-gnu*|*netbsd*|openbsd*)
        if test "$SOLAR_JAVA" != "" -a "$JDK"!="gcj"; then
            pathmunge "$JAVA_HOME/bin" "after"
        fi
        ;;

    cygwin*)
        pathmunge "$DOTNET_FRAMEWORK_HOME/bin" "before"
        pathmunge "$ASM_HOME" "before"
        pathmunge "$WINDOWS_SDK_HOME/bin" "before"
        pathmunge "$CSC_PATH" "before"
        pathmunge "$MIDL_PATH" "before"
        pathmunge "$AL_PATH" "before"
        pathmunge "$MSPDB_PATH" "before"
        if test "$BITNESS_OVERRIDE" = 64; then
            pathmunge "$COMPATH/bin/amd64" "before"
            pathmunge "$WINDOWS_SDK_HOME/bin/x64" "before"
        else
            pathmunge "$COMPATH/bin" "before"
            pathmunge "$WINDOWS_SDK_HOME/bin/x86" "before"
        fi
        if test "$SOLAR_JAVA" != "" -a "$JDK"!="gcj"; then
            if test -d "$JAVA_HOME/jre/bin/client"; then
                pathmunge "$JAVA_HOME/jre/bin/client" "before"
            fi
            if test -d "$JAVA_HOME/jre/bin/hotspot"; then
                pathmunge "$JAVA_HOME/jre/bin/hotspot" "before"
            fi
            pathmunge "$JAVA_HOME/bin" "before"
        fi
        ;;

    solaris*)
        pathmunge "/usr/css/bin" "before"
        if test "$SOLAR_JAVA" != "" -a "$JDK"!="gcj"; then
            pathmunge "$JAVA_HOME/bin" "after"
        fi
        ;;
    esac

    pathmunge "$SRC_ROOT/solenv/$OUTPATH/bin" "before"
    pathmunge "$SRC_ROOT/solenv/bin" "before"
    pathmunge "." "before"
fi

AC_SUBST(LO_PATH)

# make sure config.guess is +x; we execute config.guess, so it has to be so
chmod +x ./config.guess

dnl Setting up the post_download check script
dnl Using autoconf here, so that the migration from the previous state
dnl (checks for the additional Windows downloads were directly in
dnl configure.ac) is as easy as possible
echo "setting up the post_download check script"
autoconf post_download.in > post_download
chmod +x post_download

# Generate a configuration timestamp we can use for deps
if test -f config_host.mk; then
    mv -f config_host.mk config_host.mk.last
else
    echo > config_host.mk.last
fi

AC_CONFIG_FILES([config_host.mk ooo.lst bin/repo-list])
AC_OUTPUT

# touch the config timestamp file set_soenv.stamp
if test ! -f set_soenv.stamp; then
    echo > set_soenv.stamp
elif diff config_host.mk config_host.mk.last >/dev/null 2>&1; then
    echo "Configuration unchanged - avoiding scp2 stamp update"
else
    echo > set_soenv.stamp
fi


if test "$STALE_MAKE" = "TRUE" -a "$build_os" = "cygwin"; then
cat << _EOS
****************************************************************************
WARNING:
Your make version is known to be horribly slow, and hard to debug
problems with. To get a reasonably functional make please do:

to install a pre-compiled binary make for cygwin

mkdir -p /opt/lo/bin
cd /opt/lo/bin
wget http://dev-www.libreoffice.org/bin/cygwin/make
chmod +x make

to install from source:
place yourself in a working directory of you choice.

git clone git://anongit.freedesktop.org/libreoffice/contrib/dev-tools
cd dev-tools/make-3.82-gbuild
./configure --prefix=/opt/lo
make
sudo make install

Then re-run autogen.sh

Note: autogen.sh will try to use /opt/lo/bin/make if the environment variable GNUMAKE is not already defined.
Alternatively, you can install the 'new' make where ever you want and make sure that `which make` finds it.

_EOS
fi

cat << _EOF
****************************************************************************
To build, issue:
$GNUMAKE

run: $GNUMAKE help
for details

_EOF

if test $_os != WINNT -a "$CROSS_COMPILING" != YES; then
    cat << _EOF

To install when the build is finished, issue:
$GNUMAKE install

If you want to develop LibreOffice, you might prefer:
$GNUMAKE dev-install

If you want to run the smoketest, issue:
$GNUMAKE check

_EOF
fi

cat warn

dnl vim:set shiftwidth=4 softtabstop=4 expandtab:
