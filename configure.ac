dnl -*- Mode: Autoconf; tab-width: 4; indent-tabs-mode: nil; fill-column: 102 -*-
dnl configure.ac serves as input for the GNU autoconf package
dnl in order to create a configure script.

# The version number in the second argumemnt to AC_INIT should be four numbers separated by
# periods. Some parts of the code requires the first one to be less than 128 and the others to be less
# than 256. The four numbers can optionally be followed by a period and a free-form string containing
# no spaces or periods, like "frobozz-mumble-42" or "alpha0". If the free-form string ends with one or
# several non-alphanumeric characters, those are split off and used only for the
# ABOUTBOXPRODUCTVERSIONSUFFIX in openoffice.lst. Why that is necessary, no idea.

AC_INIT([LibreOffice],[4.2.0.0.alpha0+],[],[],[http://documentfoundation.org/])

AC_PREREQ([2.59])
save_CC=$CC
save_CXX=$CXX

BUILD_TYPE="LibO"
SCPDEFS=""
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

rm -f warn
have_WARNINGS="no"
add_warning()
{
    if test "$have_WARNINGS" = "no"; then
        echo "*************************************" > warn
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

if test -n "$SOLARENV"; then
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

BUILDDIR=`pwd`
cd $srcdir
SRC_ROOT=`pwd`
cd $BUILDDIR
EXEEXT_FOR_BUILD=
x_Cygwin=[\#]

if test "$build_os" = "cygwin"; then
    EXEEXT_FOR_BUILD=.exe
    PathFormat "$SRC_ROOT"
    SRC_ROOT="$formatted_path"
    PathFormat "$BUILDDIR"
    BUILDDIR="$formatted_path"
    x_Cygwin=
fi

AC_SUBST(SRC_ROOT)
AC_SUBST(BUILDDIR)
AC_SUBST(EXEEXT_FOR_BUILD)
AC_SUBST(x_Cygwin)
AC_DEFINE_UNQUOTED(SRCDIR,"$SRC_ROOT")
AC_DEFINE_UNQUOTED(BUILDDIR,"$BUILDDIR")

if test "z$EUID" = "z0" -a "`uname -o 2>/dev/null`" = "Cygwin"; then
    AC_MSG_ERROR([You must build LibreOffice as a normal user - not using an administrative account])
fi

# need sed in os checks...
AC_PATH_PROGS(SED, sed)
if test -z "$SED"; then
    AC_MSG_ERROR([install sed to run this script])
fi

dnl ===================================================================
dnl When building for Android, --with-android-ndk,
dnl --with-android-ndk-toolchain-version and --with-android-sdk are
dnl mandatory
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

    if test ! -f $ANDROID_NDK_HOME/RELEASE.TXT; then
        AC_MSG_ERROR([Unrecognized Android NDK. Only r8* versions supported.])
    fi
    ANDROID_NDK_VERSION=`cut -f1 -d' ' <$ANDROID_NDK_HOME/RELEASE.TXT`

    case $ANDROID_NDK_VERSION in
    r8*|r9)
        ;;
    *)
        AC_MSG_ERROR([Unsupported NDK version $ANDROID_NDK_VERSION, only r8* and r9 versions are supported])
        ;;
    esac

    if test $host_cpu = arm; then
        android_cpu=arm
        android_platform_prefix=$android_cpu-linux-androideabi
    elif test $host_cpu = mips; then
        android_cpu=mips
        android_platform_prefix=$android_cpu-linux-androideabi
    else
        # host_cpu is something like "i386" or "i686" I guess, NDK uses
        # "x86" in some contexts
        android_cpu=x86
        android_platform_prefix=$android_cpu
    fi

    case "$with_android_ndk_toolchain_version" in
    4.6|4.7|4.8)
        ANDROID_BINUTILS_DIR=$ANDROID_NDK_HOME/toolchains/$android_platform_prefix-$with_android_ndk_toolchain_version
        ANDROID_COMPILER_DIR=$ANDROID_BINUTILS_DIR
        ;;
    clang3.1|clang3.2)
        AC_MSG_WARN([Building with the Clang tool-chain is known to break in the bridges module, fix that please])
        ANDROID_BINUTILS_DIR=$ANDROID_NDK_HOME/toolchains/$android_platform_prefix-4.6
        ANDROID_COMPILER_DIR=$ANDROID_NDK_HOME/toolchains/llvm-${with_android_ndk_toolchain_version#clang}
        ANDROID_USING_CLANG=true
        ;;
    esac

    if test ! -d $ANDROID_BINUTILS_DIR; then
        AC_MSG_ERROR([No directory $ANDROID_BINUTILS_DIR])
    elif test $ANDROID_COMPILER_DIR != $ANDROID_BINUTILS_DIR -a ! -d $ANDROID_COMPILER_DIR; then
        AC_MSG_ERROR([No directory $ANDROID_COMPILER_DIR])
    fi

    # Check if there is a 64-bit tool-chain. Google provides a NDK with 64-bit tool-chain binaries in
    # NDK r8e and later, and for earlier NDKs it was possible to build one yourself. Using a 64-bit
    # linker is required if you compile large parts of the code with -g. A 32-bit linker just won't
    # manage to link the (app-specific) single huge .so that is built for the apps in
    # android/experimental/* if there is debug information in a significant part of the object files.
    # (A 64-bit ld.gold grows to much over 10 gigabytes of virtual space when linking such a .so if
    # all objects have been built with debug information.)
    toolchain_system='*'
    if test $build_os = linux-gnu; then
        ANDROID_COMPILER_BIN=$ANDROID_COMPILER_DIR/prebuilt/linux-x86/bin
        ANDROID_BINUTILS_PREBUILT_ROOT=$ANDROID_BINUTILS_DIR/prebuilt/linux-x86
        if test $build_cpu = x86_64; then
            if test -d $ANDROID_COMPILER_DIR/prebuilt/linux-x86_64; then
                ANDROID_COMPILER_BIN=$ANDROID_COMPILER_DIR/prebuilt/linux-x86_64/bin
            fi
            if test -d $ANDROID_BINUTILS_DIR/prebuilt/linux-x86_64; then
                ANDROID_BINUTILS_PREBUILT_ROOT=$ANDROID_BINUTILS_DIR/prebuilt/linux-x86_64
            fi
        fi
        ANDROID_BINUTILS_BIN=$ANDROID_BINUTILS_PREBUILT_ROOT/bin
    fi

    # This stays empty if there is just one version of the toolchain in the NDK
    ANDROID_NDK_TOOLCHAIN_VERSION_SUBDIR=
    case "`echo $ANDROID_NDK_HOME/toolchains/$android_cpu*-*/prebuilt/*/bin`" in
    */bin\ */bin*)
        # Trailing slash intentional and necessary, compare to how this is used
        if test -n "$ANDROID_USING_CLANG"; then
            ANDROID_NDK_TOOLCHAIN_VERSION_SUBDIR=4.6/
        else
            ANDROID_NDK_TOOLCHAIN_VERSION_SUBDIR="${with_android_ndk_toolchain_version}/"
        fi
        ;;
    esac

    if test $host_cpu = arm; then
       android_gnu_prefix=arm-linux-androideabi
    elif test $host_cpu = mips; then
       android_gnu_prefix=mipsel-linux-android
    elif test $ANDROID_NDK_VERSION = r8; then
        # The prefix used for the x86 tool-chain changed between NDK r8 and r8b
        android_gnu_prefix=i686-android-linux
    else
        android_gnu_prefix=i686-linux-android
    fi

    test -z "$SYSBASE" && export SYSBASE=$ANDROID_NDK_HOME/platforms/android-9/arch-$android_cpu
    test -z "$AR" && AR=$ANDROID_BINUTILS_BIN/$android_gnu_prefix-ar
    test -z "$NM" && NM=$ANDROID_BINUTILS_BIN/$android_gnu_prefix-nm
    test -z "$OBJDUMP" && OBJDUMP=$ANDROID_BINUTILS_BIN/$android_gnu_prefix-objdump
    test -z "$RANLIB" && RANLIB=$ANDROID_BINUTILS_BIN/$android_gnu_prefix-ranlib
    test -z "$STRIP" && STRIP=$ANDROID_BINUTILS_BIN/$android_gnu_prefix-strip

    if test $host_cpu = arm; then
        ANDROID_APP_ABI=armeabi-v7a
        if test -n "$ANDROID_USING_CLANG"; then
            ANDROIDCFLAGS="-gcc-toolchain $ANDROID_BINUTILS_PREBUILT_ROOT"
            ANDROIDCFLAGS="$ANDROIDCFLAGS -target armv7-none-linux-androideabi"
            ANDROIDCFLAGS="$ANDROIDCFLAGS -no-canonical-prefixes"
        else
            :
        fi
        ANDROIDCFLAGS="$ANDROIDCFLAGS -mthumb"
        ANDROIDCFLAGS="$ANDROIDCFLAGS -march=armv7-a -mfloat-abi=softfp -mfpu=neon"
    elif test $host_cpu = mips; then
        ANDROID_APP_ABI=mips
        ANDROIDCFLAGS=""
    else # x86
        ANDROID_APP_ABI=x86
        ANDROIDCFLAGS="-march=atom"
    fi
    ANDROIDCFLAGS="$ANDROIDCFLAGS -ffunction-sections"
    ANDROIDCFLAGS="$ANDROIDCFLAGS -L$ANDROID_NDK_HOME/sources/cxx-stl/gnu-libstdc++/${ANDROID_NDK_TOOLCHAIN_VERSION_SUBDIR}libs/$ANDROID_APP_ABI"
    ANDROIDCFLAGS="$ANDROIDCFLAGS --sysroot $ANDROID_NDK_HOME/platforms/android-9/arch-$android_cpu"
    ANDROIDCFLAGS="$ANDROIDCFLAGS -Wl,--fix-cortex-a8"

    if test -n "$ANDROID_USING_CLANG"; then
        ANDROIDCFLAGS="$ANDROIDCFLAGS -Qunused-arguments"
    else
        ANDROIDCFLAGS="$ANDROIDCFLAGS -Wno-psabi"
    fi

    # When using the 4.6 or newer toolchain, use the gold linker
    case "$with_android_ndk_toolchain_version" in
    4.[[6789]]*|[[56789]].*|clang*)
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

    if test -z "$CC"; then
        case "$with_android_ndk_toolchain_version" in
        4.*)
            CC="$ANDROID_COMPILER_BIN/$android_gnu_prefix-gcc $ANDROIDCFLAGS"
            ;;
        clang*)
            CC="$ANDROID_COMPILER_BIN/clang $ANDROIDCFLAGS"
        esac
    fi
    if test -z "$CXX"; then
        case "$with_android_ndk_toolchain_version" in
        4.*)
            CXX="$ANDROID_COMPILER_BIN/$android_gnu_prefix-g++ $ANDROIDCXXFLAGS"
            ;;
        clang*)
            CXX="$ANDROID_COMPILER_BIN/clang++ $ANDROIDCXXFLAGS"
            ;;
        esac
    fi
fi
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
test_cairo=no

# Default values, as such probably valid just for Linux, set
# differently below just for Mac OSX,but at least better than
# hardcoding these as we used to do. Much of this is duplicated also
# in solenv for old build system and for gbuild, ideally we should
# perhaps define stuff like this only here in configure.ac?

LINKFLAGSSHL="-shared"
PICSWITCH="-fpic"
DLLPOST=".so"

LINKFLAGSNOUNDEFS="-Wl,-z,defs"

# Most platforms use GNU libstdc++
CPP_LIBRARY="GLIBCXX"
AC_SUBST(CPP_LIBRARY)

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

    DLLPOST=".dll"
    LINKFLAGSNOUNDEFS=

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
    enable_lotuswordpro=no
    enable_neon=no
    enable_mpl_subset=yes
    enable_opengl=no
    enable_lpsolve=no
    enable_report_builder=no
    with_theme="tango"
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
    _os=Android

    if test -z "$with_android_ndk"; then
        AC_MSG_ERROR([the --with-android-ndk option is mandatory])
    fi

    if test -z "$with_android_ndk_toolchain_version"; then
        AC_MSG_ERROR([the --with-android-ndk-toolchain-version option is mandatory])
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

    AC_DEFINE(HAVE_FT_FACE_GETCHARVARIANTINDEX)
    BUILD_TYPE="$BUILD_TYPE FONTCONFIG FREETYPE"
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

AC_SUBST(WITH_MINGW)
AC_SUBST(OSVERSION)
AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_LIBS)

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

AC_ARG_ENABLE(export,
    AS_HELP_STRING([--disable-export],
        [Disable (some) code for document export. Useful when building viewer-only apps that lack
         save/export functionality, to avoid having an excessive amount of code and data used
         only for exporrt linked in. Work in progress, use only if you are hacking on it.])
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

# This is mainly for Android and iOS, but could potentially be used in some
# special case otherwise, too, so factored out as a separate setting

AC_ARG_ENABLE(dynamic-loading,
    AS_HELP_STRING([--disable-dynamic-loading],
        [Disable any use of dynamic loading of code. Work in progress, use only if you are hacking on it.])
)

AC_ARG_ENABLE(ext-mariadb-connector,
    AS_HELP_STRING([--enable-ext-mariadb-connector],
        [Enable the build of the MariaDB/MySQL Connector extension.])
)

AC_ARG_ENABLE(ext-presenter-minimizer,
    AS_HELP_STRING([--disable-ext-presenter-minimizer],
        [Disable the Presentation Minimizer extension.])
)

AC_ARG_ENABLE(report-builder,
    AS_HELP_STRING([--disable-report-builder],
        [Disable the Report Builder.])
)

AC_ARG_ENABLE(ext-wiki-publisher,
    AS_HELP_STRING([--enable-ext-wiki-publisher],
        [Enable the Wiki Publisher extension.])
)

AC_ARG_ENABLE(lpsolve,
    AS_HELP_STRING([--disable-lpsolve],
        [Disable compilation of the lp solve solver ])
)

###############################################################################

dnl ---------- *** ----------

AC_ARG_ENABLE([hardlink-deliver],
    AS_HELP_STRING([--enable-hardlink-deliver],
        [Put files into deliver folder as hardlinks instead of copying them
        over. Saves space and speeds up build.])
)

AC_ARG_ENABLE(mergelibs,
    AS_HELP_STRING([--enable-mergelibs=<all/yes>],
        [Enables linking of big, merged, library. Experimental feature tested
        only on Linux and Android. All will link a lot more libraries into libmerged
        while yes or no argument will simply enable a core set of libraries.])
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
        [Enables precompiled header support for C++.])
)

AC_ARG_ENABLE(win-mozab-driver,
    AS_HELP_STRING([--enable-win-mozab-driver],
        [LibreOffice includes a driver to connect to Mozilla
         address books under Windows, to build with this version, use this option.])
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

AC_ARG_ENABLE(mpl-subset,
    AS_HELP_STRING([--enable-mpl-subset],
        [Don't compile any pieces which are not MPL or more liberally licensed])
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
        This switch is mandatory when using an Express edition of Visual Studio.])
)

AC_ARG_ENABLE(atl,
    AS_HELP_STRING([--disable-atl],
        [Disable the use of ATL for a Windows build.])
    [
        This switch is mandatory when using an Express edition of Visual Studio.
    ],
,)

AC_ARG_ENABLE(avahi,
    AS_HELP_STRING([--enable-avahi],
        [Determines whether to use Avahi to advertise Impress to remote controls.]),
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

AC_ARG_ENABLE(dbgutil,
    AS_HELP_STRING([--enable-dbgutil],
        [Provide debugging support from --enable-debug and include additional debugging
         utilities such as object counting or more expensive checks.
         This is the recommended option for developers.
         Note that this makes the build ABI incompatible, it is not possible to mix object
         files or libraries from a --enable-dbgutil and a --disable-dbgutil build.]))

AC_ARG_ENABLE(debug,
    AS_HELP_STRING([--enable-debug],
        [Include debugging information, disable compiler optimization and inlining plus
         extra debugging code like assertions. Extra large build! (enables -g compiler flag).]))

AC_ARG_ENABLE(selective-debuginfo,
    AS_HELP_STRING([--enable-selective-debuginfo],
        [If --enable-debug or --enable-dbgutil is used, build debugging information
         (-g compiler flag) only for the specified gbuild build targets
         (where all means everything, - prepended means not to enable, / appended means
         everything in the directory; there is no ordering, more specific overrides
         more general, and disabling takes precedence).
         Example: --enable-selective-debuginfo="all -sw/ -Library_sc".]))

AC_ARG_ENABLE(symbols,
    AS_HELP_STRING([--enable-symbols],
        [Include debugging symbols in output while preserve optimization.
         This enables -g compiler flag for GCC or equivalent,
         without changing anything else compared to productive code.]))

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
         Building takes longer but libraries and executables are optimized for speed.
         (For GCC, best to use GCC 4.6 or later and 'gold' linker.
          Also possible for Clang.
          Experimental work in progress,
          don't use unless you are working on this.)]))

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

AC_ARG_ENABLE(opengl,
    AS_HELP_STRING([--disable-opengl],
        [Determines whether to build the OpenGL 3D slide transitions component.]))

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

AC_ARG_ENABLE(sdremote-bluetooth,
    AS_HELP_STRING([--disable-sdremote-bluetooth],
        [Determines whether to build sdremote with bluetooth support.
         Requires dbus on Linux.]))

AC_ARG_ENABLE(gconf,
    AS_HELP_STRING([--disable-gconf],
        [Determines whether to use the GConf support.]),
,enable_gconf=yes)

AC_ARG_ENABLE(gnome-vfs,
    AS_HELP_STRING([--enable-gnome-vfs],
        [Determines whether to use the Gnome Virtual Filing System on platforms
         where that VFS is available.]),
,enable_gnome_vfs=no)

AC_ARG_ENABLE(gio,
    AS_HELP_STRING([--disable-gio],
        [Determines whether to use the GIO support.]),
,enable_gio=yes)

AC_ARG_ENABLE(telepathy,
    AS_HELP_STRING([--enable-telepathy],
        [Determines whether to enable Telepathy for collaboration.]),
,enable_telepathy=no)

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

AC_ARG_ENABLE(vlc,
    AS_HELP_STRING([--enable-vlc],
        [Enable building with the VLC avmedia backend.]),
,enable_vlc=no)

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

AC_ARG_ENABLE(windows-build-signing,
    AS_HELP_STRING([--enable-windows-build-signing],
        [Enable signing of windows binaries (*.exe, *.dll)]),
,)

AC_ARG_ENABLE(silent-msi,
    AS_HELP_STRING([--enable-silent-msi],
        [Enable MSI with LIMITUI=1 (silent install).]),
,)

AC_ARG_ENABLE(desktop-gui-elements,
    AS_HELP_STRING([--enable-desktop-gui-elements],
        [Enable display and use of desktop style GUI elements
         even on a non-desktop platform.]),
,)

AC_ARG_ENABLE(macosx-code-signing,
    AS_HELP_STRING([--enable-macosx-code-signing<=identity>],
        [Sign executables, dylibs, frameworks and the app bundle. If you
         don't provide an identity the first suitable certificate
         in your keychain is used.]),
,)

AC_ARG_ENABLE(macosx-sandbox,
    AS_HELP_STRING([--enable-macosx-sandbox],
        [Make the app bundle run in a sandbox. Requires code signing.
         Is required by apps distributed in the Mac App Store, and implies
         adherence to App Store rules.]),
,)

AC_ARG_ENABLE(canonical-installation-tree-structure,
    AS_HELP_STRING([--enable-canonical-installation-tree-structure],
        [Make the installation tree structure be more canonical for
         the platform. Implemented only for OS X. Experimental.]),
,)


AC_ARG_WITH(macosx-bundle-identifier,
    AS_HELP_STRING([--with-macosx-bundle-identifier=tld.mumble.orifice.TheOffice],
        [Define the OS X bundle identifier. Default is the somewhat weird
         org.libreoffice.script ("script", huh?).]),
,with_macosx_bundle_identifier=org.libreoffice.script)

AC_ARG_ENABLE(libc++,
    AS_HELP_STRING([--enable-libc++],
        [Use the libc++ C++ library instead of GNU libstdc++ on OS X. Only effective
         if --with-macosx-version-min-required is 10.7 or later. Experimental work in
         progress, very likely breaks something, don't use unless you plan to fix that.]),
,)

AC_ARG_ENABLE(ios-simulator,
    AS_HELP_STRING([--enable-ios-simulator],
        [Build for the iOS Simulator, not iOS device.]),
,)

AC_ARG_ENABLE(readonly-installset,
    AS_HELP_STRING([--enable-readonly-installset],
        [Prevents any attempts by LibreOffice to write into its installation. That means
         at least that no "system-wide" extensions can be added. Experimental work in
         progress.]),
,)

AC_ARG_ENABLE(postgresql-sdbc,
    AS_HELP_STRING([--disable-postgresql-sdbc],
        [Disable the build of the PostgreSQL-SDBC driver.])
)

AC_ARG_ENABLE(lotuswordpro,
    AS_HELP_STRING([--disable-lotuswordpro],
        [Disable the build of the Lotus Word Pro filter.]),
,enable_lotuswordpro=yes)

AC_ARG_ENABLE(firebird-sdbc,
    AS_HELP_STRING([--disable-firebird-sdbc],
        [Disable the build of the Firebird-SDBC driver if it doesn't compile for you.]),
,enable_firebird_sdbc=yes)

AC_ARG_ENABLE(coretext,
    AS_HELP_STRING([--disable-coretext],
        [Don't use CoreText on Mac (but the obsolete ATSU instead).
         In a 64-bit LibreOffice CoreText is by necessity always used.]),
)

AC_ARG_ENABLE(winegcc,
    AS_HELP_STRING([--enable-winegcc],
        [Enable use of winegcc during the build, in order to create msi* tools
         needed for MinGW cross-compilation.]),
)

AC_ARG_ENABLE(liblangtag,
    AS_HELP_STRING([--disable-liblangtag],
        [Disable use of liblangtag, and instead use an own simple
         implementation.]),
)

AC_ARG_ENABLE(bogus-pkg-config,
    AS_HELP_STRING([--enable-bogus-pkg-config],
        [MACOSX only: on MacOSX pkg-config can cause trouble. by default if one is found in the PATH, an error is issued. This flag turn that error into a warning.]),
)

AC_ARG_ENABLE(openssl,
    AS_HELP_STRING([--disable-openssl],
        [Disable using libssl/libcrypto from OpenSSL. If disabled,
         components will either use GNUTLS or NSS. Work in progress,
         use only if you are hacking on it.]),
,enable_openssl=yes)

AC_ARG_ENABLE(library-bin-tar,
    AS_HELP_STRING([--enable-library-bin-tar],
        [Enable the building and reused of tarball of binary build for some 'external' libraries
        Some libraries can save their build result in a tarball
        stored in TARFILE_LOCATION. that binary tarball is
        uniquely identified by the source tarbal,
        the content of the config_host.mk file and the content
        of the top-level directory in core for that library
        If this option is enabled, then if such a tarfile exist, it will be untarred
        isntead of the source tarfile, and the build step will be skipped for that
        library.
        If a proper tarfile does not exist, then the normal source-based
        build is done for taht library and a proper binary tarfile is created
        for the next time.]),
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

AC_ARG_WITH(referenced-git,
    AS_HELP_STRING([--with-referenced-git=<OTHER_CHECKOUT_DIR>],
        [Specify another checkout directory to reference. This makes use of
                 git submodule update --reference, and saves a lot of diskspace
                 when having multiple trees side-by-side.]),
    GIT_REFERENCE_SRC=$withval ,
)

AC_ARG_WITH(linked-git,
    AS_HELP_STRING([--with-linked-git=<submodules repo basedir>],
        [Specify a directory where the repositories of submodules are located.
         This uses a method similar to git-new-workdir to get submodules.]),
    GIT_LINK_SRC=$withval ,
)

AC_ARG_WITH(galleries,
    AS_HELP_STRING([--with-galleries],
        [Specify how galleries should be built. It is possible either to
         build these internally from source ("build"), or to (optionally
         download and) unpack them them from a package ("package"), or
         to disable them ("no")]),
)

AC_ARG_WITH(theme,
    AS_HELP_STRING([--with-theme="theme1 theme2..."],
        [Choose which themes to include. By default those themes with an '*' are included.
         Possible choices: *crystal, *default (galaxy), *hicontrast, human, industrial, *oxygen, *sifr, *tango, *tango_testing.]),
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

AC_ARG_WITH(tls,
    AS_HELP_STRING([--with-tls],
        [Decides which TLS/SSL and cryptographic implementations to use for
         LibreOffice's code. Notice that this doesn't apply for depending
         libraries like "neon", for example. Default is to use OpenSSL
         although NSS is also possible. Notice that selecting NSS restricts
         the usage of OpenSSL in LO's code but selecting OpenSSL doesn't
         restrict by now the usage of NSS in LO's code. Possible values:
         openssl, nss. Example: --with-tls="nss"]),
,)

AC_ARG_WITH(system-libs,
    AS_HELP_STRING([--with-system-libs],
        [Use libraries already on system -- enables all --with-system-* flags.]),
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

AC_ARG_WITH(system-cairo,
    AS_HELP_STRING([--with-system-cairo],
        [Use Cairo libraries already on system.]),,
    [with_system_cairo="$with_system_libs"])

AC_ARG_WITH(myspell-dicts,
    AS_HELP_STRING([--with-myspell-dicts],
        [Adds myspell dictionaries to the LibreOffice installation set]),
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
        [Use system poppler (only needed for PDF import).]),,
    [with_system_poppler="$with_system_libs"])

AC_ARG_WITH(system-apache-commons,
    AS_HELP_STRING([--with-system-apache-commons],
        [Use Apache commons libraries already on system.]),,
    [with_system_apache_commons="$with_system_jars"])

AC_ARG_WITH(system-mariadb,
    AS_HELP_STRING([--with-system-mariadb],
        [Use MariaDB/MySQL libraries already on system, for building the MariaDB Connector/LibreOffice
         extension.]),,
    [with_system_mariadb="$with_system_libs"])

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
                                                  your libpq installation>
    ],
,)

AC_ARG_WITH(system-firebird,
    AS_HELP_STRING([--with-system-firebird],
        [Use Firebird libraries already on system, for building the Firebird-SDBC
         driver. If fb_config is not in PATH, use FBCONFIG to point to it.]),,
    [with_system_firebird="$with_system_libs"])

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

AC_ARG_WITH(system-vigra,
    AS_HELP_STRING([--with-system-vigra],
        [Use vigra already on system.]),,
    [with_system_vigra="$with_system_headers"])

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

AC_ARG_WITH(system-liblangtag,
    AS_HELP_STRING([--with-system-liblangtag],
        [Use liblangtag library already on system.]),,
    [with_system_liblangtag="$with_system_libs"])

AC_ARG_WITH(system-npapi-headers,
    AS_HELP_STRING([--with-system-npapi-headers],
        [Use NPAPI headers provided by system instead of bundled ones. Used in
         extensions/source/nsplugin (ENABLE_NPAPI_INTO_BROWSER=TRUE) and
         extensions/source/plugin (ENABLE_NPAPI_FROM_BROWSER=TRUE)]),,
    [with_system_npapi_headers="$with_system_headers"]
)

AC_ARG_WITH(linker-hash-style,
    AS_HELP_STRING([--with-linker-hash-style],
        [Use linker with --hash-style=<style> when linking shared objects.
         Possible values: "sysv", "gnu", "both". The default value is "gnu"
         if supported on the build system, and "sysv" otherwise.]))

AC_ARG_WITH(jdk-home,
    AS_HELP_STRING([--with-jdk-home],
        [If you have installed JDK 1.3 or later on your system please supply the
         path here. Note that this is not the location of the java command but the
         location of the entire distribution.])
    [
                          Usage:     --with-jdk-home=<absolute path to JDK home>
    ],
,)

AC_ARG_WITH(help,
    AS_HELP_STRING([--with-help],
        [Enable the build of help. There is a special parameter "common" that
         can be used to bundle only the common part, .e.g help-specific icons.
         This is useful when you build the helpcontent separately.])
    [
                          Usage:     --with-help    build the entire local help
                                 --without-help     no local help (default)
                                 --with-help=common bundle common files for the local
                                                    help but do not build the whole help
    ],
,)

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

AC_ARG_WITH(visual-studio,
    AS_HELP_STRING([--with-visual-studio=<2012/2010>],
        [Specify which Visual Studio version to use in case several are
         are installed. If not specified, the order of preference is
         2012, 2010 (including Express editions).])
    [
                          Usage:     --with-visual-studio=<2012/2010>
    ],
,)

AC_ARG_WITH(windows-sdk,
    AS_HELP_STRING([--with-windows-sdk=<6.0(A)/7.0(A)/7.1(A)/8.0(A)>],
        [Specify which Windows SDK, or "Windows Kit", version to use
         in case the one that came with the selected Visual Studio
         is not what you want for some reason. Note that not all compiler/SDK
         combinations are supported. The intent is that this option should not
         be needed.])
    [
                          Usage:     --with-windows-sdk=6.0(A)/7.0(A)/7.1(A)/8.0(A)>
    ],
,)

AC_ARG_WITH(opencl-sdk,
    AS_HELP_STRING([--with-opencl-sdk],
        [If you have the AMD HSA / opencl SDK installed (cf.
         http://developer.amd.com/tools-and-sdks/heterogeneous-computing/amd-accelerated-parallel-processing-app-sdk/downloads/ )
         Pass the absolute path to where that SDK is unpacked.])
    [
                          Usage:     --with-opencl-sdk=/opt/AMDAPP
    ],
,)

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
    AS_HELP_STRING([--with-branding],
        [Use given path to retrieve branding images set.])
    [
                          Search for intro.png about.svg flat_logo.svg
                          backing_left.png backing_right.png
                          backing_rtl_left.png backing_rtl_right.png
                          backing_space.png. If any is missing, default
                          ones will be used instead.

                          Search also progress.conf for progress
                          settings on intro screen :

                          PROGRESSBARCOLOR="255,255,255" Set color of
                          progress bar. Comma separated RGB decimal values.
                          PROGRESSSIZE="407,6" Set size of progress bar.
                          Comma separated decimal values (width, height).
                          PROGRESSPOSITION="61,317" Set position of progress
                          bar from left,top. Comma separated decimal values.
                          PROGRESSFRAMECOLOR="20,136,3" Set color of progress
                          bar frame. Comma separated RGB decimal values.
                          PROGRESSTEXTCOLOR="255,255,255" Set color of progress
                          bar text. Comma separated RGB decimal values.
                          PROGRESSTEXTBASELINE="287" Set vertical position of
                          progress bar text from top. Decimal value.

                          Default values will be used if not found.

                          Usage:     --with-branding=/path/to/images
    ],
,)


AC_ARG_WITH(extra-buildid,
    AS_HELP_STRING([--with-extra-buildid],
        [Show addition build identification in about dialog.])
    [
                          Usage:     --with-extra-buildid="Tinderbox: Win-x86@6, Branch:master, Date:2012-11-26_00.29.34"
    ],
,)


AC_ARG_WITH(vendor,
    AS_HELP_STRING([--with-vendor],
        [Set vendor of the build.])
    [
                          Usage:     --with-vendor="John the Builder"
    ],
,)

AC_ARG_WITH(compat-oowrappers,
    AS_HELP_STRING([--with-compat-oowrappers],
        [Install oo* wrappers in parallel with
         lo* ones to keep backward compatibility.
         Has effect only with make distro-pack-install]),
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

AC_ARG_WITH(parallelism,
    AS_HELP_STRING([--with-parallelism],
        [Number of jobs to run simultaneously during build. Parallel builds can
        save a lot of time on multi-cpu machines. Defaults to the number of
        CPUs on the machine, unless you configure --enable-icecream - then to
        10.]),
,)

AC_ARG_WITH(check-jobs,
    AS_HELP_STRING([--with-check-jobs],
        [Maximum number of jobs that will be issued at the same time during 'make subsequenttest'.
         Default value is the same as 'parallelism'. Useful because the subsequent tests are very lightweight.]),
,)

AC_ARG_WITH(all-tarballs,
    AS_HELP_STRING([--with-all-tarballs],
        [Download all external tarballs unconditionally]))

dnl ===================================================================
dnl Do we want to use pre-build binary tarball for recompile
dnl ===================================================================

if test "$enable_library_bin_tar" = "yes" ; then
   USE_LIBRARY_BIN_TAR=YES
else
   USE_LIBRARY_BIN_TAR=NO
fi
AC_SUBST(USE_LIBRARY_BIN_TAR)

dnl ===================================================================
dnl Test whether build target is Release Build
dnl ===================================================================
AC_MSG_CHECKING([whether build target is Release Build])
if test "$enable_release_build" = "" -o "$enable_release_build" = "no"; then
    AC_MSG_RESULT([no])
    ENABLE_RELEASE_BUILD=
else
    AC_MSG_RESULT([yes])
    ENABLE_RELEASE_BUILD=TRUE
fi
AC_SUBST(ENABLE_RELEASE_BUILD)

dnl ===================================================================
dnl Test whether to sign Windows Build
dnl ===================================================================
AC_MSG_CHECKING([whether to sign windows build])
if test "$enable_windows_build_signing" = "yes" -a "$_os" = "WINNT" -a "$WITH_MINGW" != "YES" ; then
    AC_MSG_RESULT([yes])
    WINDOWS_BUILD_SIGNING="TRUE"
else
    AC_MSG_RESULT([no])
    WINDOWS_BUILD_SIGNING="FALSE"
fi
AC_SUBST(WINDOWS_BUILD_SIGNING)

dnl ===================================================================
dnl MacOSX build and runtime environment options
dnl ===================================================================

AC_ARG_WITH(macosx-sdk,
    AS_HELP_STRING([--with-macosx-sdk],
        [Use a specific SDK for building.])
    [
                          Usage:     --with-macosx-sdk=<version>

                          e. g.: --with-macosx-sdk=10.6

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
                          ?    | ?    | ?    || 10.6 | 10.s | 10.s |
                          ?    | ?    | 10.x || 10.6 | 10.x | 10.x |
                          ?    | 10.x | ?    || 10.6 | 10.s | 10.s |
                          ?    | 10.x | 10.y || 10.6 | 10.x | 10.y |
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

                          e. g.: --with-macos-version-min-required=10.6
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
dnl These are superseded by --with-build-platform-configure-options.
dnl Or are they? Look for the complicated tests for "$cross_compiling" = "yes" -a \(
dnl "$with_system_icu_for_build" = "yes" -o "$with_system_icu_for_build" = "force" \)
dnl far below. Could whoever understands MinGW cross-compilation clean up this please.
dnl ===================================================================

AC_ARG_WITH(system-boost-for-build,
    AS_HELP_STRING([--with-system-boost-for-build],
        [Use boost already on system for build tools (cross-compilation only).]))

AC_ARG_WITH(system-cppunit-for-build,
    AS_HELP_STRING([--with-system-cppunit-for-build],
        [Use cppunit already on system for build tools (cross-compilation only).]))

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
    AC_DEFINE(HAVE_FEATURE_DESKTOP)
    AC_DEFINE(HAVE_FEATURE_DESKTOP_GUI_ELEMENTS)
    AC_DEFINE(HAVE_FEATURE_MULTIUSER_ENVIRONMENT)
elif test "$enable_desktop_gui_elements" = yes; then
    AC_DEFINE(HAVE_FEATURE_DESKTOP_GUI_ELEMENTS)
fi

DISABLE_EXPORT=''
if test "$enable_export" != no; then
    BUILD_TYPE="$BUILD_TYPE EXPORT"
else
    DISABLE_EXPORT='TRUE'
    SCPDEFS="$SCPDES -DDISABLE_EXPORT"
fi
AC_SUBST(DISABLE_EXPORT)

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
    # For iOS and Android disable extensions unless specifically overridden with --enable-extensions.
    if test $_os != iOS -a $_os != Android; then
        enable_extensions=yes
    fi
fi

if test "$enable_extensions" = yes; then
    BUILD_TYPE="$BUILD_TYPE EXTENSIONS"
    AC_DEFINE(HAVE_FEATURE_EXTENSIONS)
fi

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
    SOLARVER=${BUILDDIR}/solver
fi

# remenber SYSBASE value
AC_SUBST(SYSBASE)

dnl ===================================================================
dnl  Sort out various gallery compilation options
dnl ===================================================================
AC_MSG_CHECKING([how to build and package galleries])
if test -n "${with_galleries}"; then
    if test "$with_galleries" = "build"; then
        WITH_GALLERY_BUILD=YES
        AC_MSG_RESULT([build from source images internally])
    elif test "$with_galleries" = "package"; then
        WITH_GALLERY_BUILD=PACKAGE
        AC_MSG_ERROR([FIXME - implement build from pre-compiled package])
    elif test "$with_galleries" = "no"; then
        WITH_GALLERY_BUILD=
        AC_MSG_RESULT([disable non-internal gallery build])
    else
        AC_MSG_ERROR([unknown value --with-galleries=$with_galleries])
    fi
else
   if test $_os != iOS -a $_os != Android; then
        WITH_GALLERY_BUILD=YES
        AC_MSG_RESULT([internal src images for desktop])
   else
        WITH_GALLERY_BUILD=
        AC_MSG_RESULT([disable src imgage build])
   fi
fi
AC_SUBST(WITH_GALLERY_BUILD)

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
if test "$_os" != "WINNT" -a "$WITH_MINGW" != "yes"; then
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

dnl ===================================================================
dnl Java support
dnl ===================================================================
AC_MSG_CHECKING([whether to build with Java support])
if test "$with_java" != "no"; then
    if test "$DISABLE_SCRIPTING" = TRUE; then
        AC_MSG_RESULT([no, overridden by --disable-scripting])
        ENABLE_JAVA=""
        with_java=no
    else
        AC_MSG_RESULT([yes])
        ENABLE_JAVA="TRUE"
        AC_DEFINE(HAVE_FEATURE_JAVA)
    fi
else
    AC_MSG_RESULT([no])
    ENABLE_JAVA=""
fi

AC_SUBST(ENABLE_JAVA)

dnl ENABLE_JAVA="TRUE" if we want there to be *run-time* (and build-time) support for Java

dnl ENABLE_JAVA="" indicate no Java support at all

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
dnl Check OS X SDK and compiler
dnl ===================================================================

if test $_os = Darwin; then

    if test "$enable_64_bit" = "" -o "$enable_64_bit" = "no"; then
        bitness=-m32
    else
        bitness=-m64
        BITNESS_OVERRIDE=64
    fi

    # If no --with-macosx-sdk option is given, look for 10.6, 10.7,
    # 10.8 and 10.9 SDKs, in that order. If not found in some (old)
    # default locations, try the xcode-select tool.

    # The intent is that for "most" Mac-based developers, a suitable
    # SDK will be found automatically without any configure options.

    # For developers still using Xcode 3, in /Developer, either
    # because it is the only Xcode they have, or they have that in
    # addition to Xcode 4 in /Applications/Xcode.app, the 10.6 SDK
    # should be found.

    # For developers with a current Xcode, the lowest-numbered SDK
    # should be found.

    AC_MSG_CHECKING([what Mac OS X SDK to use])

    if test -z "$with_macosx_sdk"; then
        if test -d /Developer/SDKs/MacOSX10.6.sdk; then
            with_macosx_sdk=10.6
        elif test -d /Developer-old/SDKs/MacOSX10.6.sdk; then
            with_macosx_sdk=10.6
        elif test -d /Xcode3/SDKs/MacOSX10.6.sdk; then
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
            elif test -d "$xcodepath/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk"; then
                with_macosx_sdk=10.9
            fi
        fi
        if test -z "$with_macosx_sdk"; then
            AC_MSG_ERROR([Could not figure out the location of a Mac OS X SDK and its version])
        fi
    fi

    case $with_macosx_sdk in
    10.6)
        MACOSX_SDK_VERSION=1060
        ;;
    10.7)
        MACOSX_SDK_VERSION=1070
        ;;
    10.8)
        MACOSX_SDK_VERSION=1080
        ;;
    10.9)
        MACOSX_SDK_VERSION=1090
        ;;
    *)
        AC_MSG_ERROR([with-macosx-sdk $with_macosx_sdk is not a supported value, supported value are 10.6--8])
        ;;
    esac

    # Next find it (again, if we deduced its version above by finding
    # it... but we need to look for it once more in case
    # --with-macosx-sdk was given so that the above search did not
    # happen).
    if test -z "$MACOSX_SDK_PATH"; then
        case $with_macosx_sdk in
        10.6)
            if test -d /Developer/SDKs/MacOSX10.6.sdk; then
                MACOSX_SDK_PATH=/Developer/SDKs/MacOSX10.6.sdk
            elif test -d /Developer-old/SDKs/MacOSX10.6.sdk; then
                MACOSX_SDK_PATH=/Developer-old/SDKs/MacOSX10.6.sdk
            elif test -d /Xcode3/SDKs/MacOSX10.6.sdk; then
                MACOSX_SDK_PATH=/Xcode3/SDKs/MacOSX10.6.sdk
            elif test -x /usr/bin/xcode-select; then
                xcodepath="`xcode-select -print-path`"
                if test -d "$xcodepath/Platforms/MacOSX.platform/Developer/SDKs/MacOSX$with_macosx_sdk.sdk"; then
                    MACOSX_SDK_PATH="$xcodepath/Platforms/MacOSX.platform/Developer/SDKs/MacOSX$with_macosx_sdk.sdk"
                fi
            fi
            ;;
        10.7)
            if test -d /Developer/SDKs/MacOSX$with_macosx_sdk.sdk; then
                MACOSX_SDK_PATH=/Developer/SDKs/MacOSX$with_macosx_sdk.sdk
            elif test -x /usr/bin/xcode-select; then
                xcodepath="`xcode-select -print-path`"
                if test -d "$xcodepath/Platforms/MacOSX.platform/Developer/SDKs/MacOSX$with_macosx_sdk.sdk"; then
                    MACOSX_SDK_PATH="$xcodepath/Platforms/MacOSX.platform/Developer/SDKs/MacOSX$with_macosx_sdk.sdk"
                fi
            fi
            ;;
        10.8|10.9)
            xcodepath="`xcode-select -print-path`"
            if test -d "$xcodepath/Platforms/MacOSX.platform/Developer/SDKs/MacOSX$with_macosx_sdk.sdk"; then
                MACOSX_SDK_PATH="$xcodepath/Platforms/MacOSX.platform/Developer/SDKs/MacOSX$with_macosx_sdk.sdk"
            fi
            ;;
        esac
        if test -z "$MACOSX_SDK_PATH"; then
            AC_MSG_ERROR([Could not figure out the location of Mac OS X $with_macosx_sdk SDK])
        fi
    fi
    AC_MSG_RESULT([SDK $with_macosx_sdk at $MACOSX_SDK_PATH])

    if test "$with_macosx_version_min_required" = ""; then
        with_macosx_version_min_required="10.6"
    fi

    if test "$with_macosx_version_max_allowed" = ""; then
        with_macosx_version_max_allowed="$with_macosx_sdk"
    fi

    FRAMEWORKSHOME="$MACOSX_SDK_PATH/System/Library/Frameworks"
    MACOSX_DEPLOYMENT_TARGET="$with_macosx_version_min_required"

    case "$with_macosx_version_min_required" in
    10.6)
        MAC_OS_X_VERSION_MIN_REQUIRED="1060"
        ;;
    10.7)
        MAC_OS_X_VERSION_MIN_REQUIRED="1070"
        ;;
    10.8)
        MAC_OS_X_VERSION_MIN_REQUIRED="1080"
        ;;
    10.9)
        MAC_OS_X_VERSION_MIN_REQUIRED="1090"
        ;;
    *)
        AC_MSG_ERROR([with-macosx-version-min-required $with_macosx_version_min_required is not a supported value, supported value are 10.6--9])
        ;;
    esac

    if test "$BITNESS_OVERRIDE" = ""; then
        case "$with_macosx_version_min_required" in
        10.6)
            case "$with_macosx_sdk" in
            10.6)
                ;;
            *)
                AC_MSG_WARN([Building with a SDK > 10.6 possibly breaks 10.6 compatibility. Do not use for the TDF build])
                add_warning "Building with a SDK > 10.6 possibly breaks 10.6 compatibility. Do not use for the TDF build"
                ;;
            esac
            ;;
        *)
            AC_MSG_WARN([Building with a minimum version requirement > 10.6 breaks 10.6 compatibility. Do not use for the TDF build])
            add_warning "Building with a minimum version requirement > 10.6 breaks 10.6 compatibility. Do not use for the TDF build"
            ;;
        esac
    fi

    # If no CC and CXX environment vars, try to guess where the compiler is
    LIBTOOL=libtool
    if test -z "$save_CC"; then
        AC_MSG_CHECKING([what compiler to use])
        case $with_macosx_sdk in
        10.6)
            if test "$enable_libc__" = yes; then
                AC_MSG_ERROR([--enable-libc++ requires --with-macosx-version-min-required >= 10.7])
            fi
            # did someone copy her 10.6 sdk into xcode 4 (needed on Mountain Lion)?
            if test "$(echo $MACOSX_SDK_PATH | cut -c1-23)" = "/Applications/Xcode.app"; then
                CC="`xcrun -find gcc` $bitness -mmacosx-version-min=$with_macosx_version_min_required -isysroot $MACOSX_SDK_PATH"
                CXX="`xcrun -find g++` $bitness -mmacosx-version-min=$with_macosx_version_min_required -isysroot $MACOSX_SDK_PATH"
                XCRUN=xcrun
            else
                CC="gcc-4.2 $bitness -mmacosx-version-min=$with_macosx_version_min_required -isysroot $MACOSX_SDK_PATH"
                CXX="g++-4.2 $bitness -mmacosx-version-min=$with_macosx_version_min_required -isysroot $MACOSX_SDK_PATH"
            fi
            LIBTOOL=libtool
            ;;
        10.7|10.8|10.9)
            if test "$enable_libc__" = yes; then
                if test "$with_macosx_version_min_required" = 10.6; then
                    AC_MSG_ERROR([--enable-libc++ requires --with-macosx-version-min-required >= 10.7])
                fi
                # Use libc++ instead of libstdc++ when possible
                stdlib=-stdlib=libc++
                CPP_LIBRARY="LIBCPP"
            fi
            if test "$ENABLE_LTO" = TRUE; then
                lto=-flto
            fi
            CC="`xcrun -find clang` $bitness $lto -mmacosx-version-min=$with_macosx_version_min_required -isysroot $MACOSX_SDK_PATH"
            CXX="`xcrun -find clang++` $bitness $lto $stdlib -mmacosx-version-min=$with_macosx_version_min_required -isysroot $MACOSX_SDK_PATH"
            XCRUN=xcrun
            AR=`xcrun -find ar`
            LIBTOOL=`xcrun -find libtool`
            RANLIB=`xcrun -find ranlib`
            ;;
        esac
        AC_MSG_RESULT([$CC and $CXX])
    else
        # In case somebody insists on passing in CXX, and perhaps CFLAGS or CXXFLAGS to choose
        # compiler. Not really something we encourage or support.
        case "$CXX $CFLAGS $CXXFLAGS" in
        *-stdlib=libc++*)
            CPP_LIBRARY="LIBCPP"
            ;;
        esac
    fi

    case "$with_macosx_version_max_allowed" in
    10.6)
        MAC_OS_X_VERSION_MAX_ALLOWED="1060"
        ;;
    10.7)
        MAC_OS_X_VERSION_MAX_ALLOWED="1070"
        ;;
    10.8)
        MAC_OS_X_VERSION_MAX_ALLOWED="1080"
        ;;
    10.9)
        MAC_OS_X_VERSION_MAX_ALLOWED="1090"
        ;;
    *)
        AC_MSG_ERROR([with-macosx-version-max-allowed $with_macosx_version_max_allowed is not a supported value, supported value are 10.6--8])
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

    AC_MSG_CHECKING([whether to do code signing])

    if test "$enable_macosx_code_signing" = yes; then
        # By default use the first suitable certificate (?).

        # http://stackoverflow.com/questions/13196291/difference-between-mac-developer-and-3rd-party-mac-developer-application
        # says that the "Mac Developer" certificate is useful just for self-testing. For distribution
        # outside the Mac App Store, use the "Developer ID Application" one, and for distribution in
        # the App Store, the "3rd Party Mac Developer" one. I think it works best to the the
        # "Developer ID Application" one.

        identity=`security find-identity -p codesigning -v 2>/dev/null | grep 'Developer ID Application:' | awk '{print $2}' |head -1`
        if test -n "$identity"; then
            MACOSX_CODESIGNING_IDENTITY=$identity
            pretty_name=`security find-identity -p codesigning -v | grep "$MACOSX_CODESIGNING_IDENTITY" | sed -e 's/^[[^"]]*"//' -e 's/"//'`
            AC_MSG_RESULT([yes, using the identity $MACOSX_CODESIGNING_IDENTITY for $pretty_name])
        fi
    elif test -n "$enable_macosx_code_signing" -a "$enable_macosx_code_signing" != no ; then
        MACOSX_CODESIGNING_IDENTITY=$enable_macosx_code_signing
        pretty_name=`security find-identity -p codesigning -v | grep "$MACOSX_CODESIGNING_IDENTITY" | sed -e 's/^[[^"]]*"//' -e 's/"//'`
        AC_MSG_RESULT([yes, using the identity $MACOSX_CODESIGNING_IDENTITY for $pretty_name])
    else
        AC_MSG_RESULT([no])
    fi

    AC_MSG_CHECKING([whether to sandbox the application])

    if test -z "$MACOSX_CODESIGNING_IDENTITY" -a "$enable_macosx_sandbox" = yes; then
        AC_MSG_ERROR([OS X sandboxing requires code signing])
    elif test -n "$ENABLE_JAVA" -a "$enable_macosx_sandbox" = yes; then
        AC_MSG_ERROR([OS X sandboxing (actually App Store rules) disallows use of Java])
    elif test -n "$MACOSX_CODESIGNING_IDENTITY" -a "$enable_macosx_sandbox" = yes; then
        ENABLE_MACOSX_SANDBOX=TRUE
        AC_DEFINE(HAVE_FEATURE_MACOSX_SANDBOX)
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
    fi

    AC_MSG_CHECKING([what OS X app bundle identifier to use])

    MACOSX_BUNDLE_IDENTIFIER=$with_macosx_bundle_identifier

    AC_MSG_RESULT([$MACOSX_BUNDLE_IDENTIFIER])
fi
AC_SUBST(MACOSX_SDK_PATH)
AC_SUBST(MACOSX_SDK_VERSION)
AC_SUBST(MACOSX_DEPLOYMENT_TARGET)
AC_SUBST(MAC_OS_X_VERSION_MIN_REQUIRED)
AC_SUBST(MAC_OS_X_VERSION_MAX_ALLOWED)
AC_SUBST(XCRUN)
AC_SUBST(LIBTOOL) # Note that the OS X libtool command is unrelated to GNU libtool
AC_SUBST(MACOSX_CODESIGNING_IDENTITY)
AC_SUBST(ENABLE_MACOSX_SANDBOX)
AC_SUBST(MACOSX_BUNDLE_IDENTIFIER)

dnl ===================================================================
dnl Check iOS SDK and compiler
dnl ===================================================================

if test $_os = iOS; then

    AC_MSG_CHECKING([what iOS SDK to use])

    xcode_developer=`xcode-select -print-path`

    if test "$enable_ios_simulator" = yes; then
        platform=iPhoneSimulator
        versionmin=-mmacosx-version-min=10.7
        arch=i386
    else
        platform=iPhoneOS
        versionmin=-miphoneos-version-min=5.0
        arch=armv7
    fi

    pref_sdk_ver=6.1
    for I in 6.1 6.0 7.0; do
        t=$xcode_developer/Platforms/$platform.platform/Developer/SDKs/$platform$I.sdk
        if test -d $t; then
            sysroot=$t
            break
        fi
    done

    if test -z "$sysroot"; then
        AC_MSG_ERROR([Could not find iOS SDK, expected something like $xcode_developer/Platforms/$platform.platform/Developer/SDKs/${platform}${pref_sdk_ver}])
    fi

    AC_MSG_RESULT($sysroot)

    if test "$ENABLE_LTO" = TRUE; then
        lto=-flto
    fi
    CC="`xcrun -find clang` -arch $arch -isysroot $sysroot $lto $versionmin"
    CXX="`xcrun -find clang++` -arch $arch -isysroot $sysroot $lto $versionmin"
    XCRUN=xcrun
    AR=`xcrun -find ar`
    LIBTOOL=`xcrun -find libtool`
    RANLIB=`xcrun -find ranlib`
fi

AC_MSG_CHECKING([whether to treat the installation as read-only])

if test \( -z "$enable_readonly_installset" -a "$ENABLE_MACOSX_SANDBOX" = TRUE \) -o \
        "$enable_extensions" != yes; then
    enable_readonly_installset=yes
fi
if test "$enable_readonly_installset" = yes; then
    AC_MSG_RESULT([yes])
    AC_DEFINE(HAVE_FEATURE_READONLY_INSTALLSET)
else
    AC_MSG_RESULT([no])
fi

dnl ===================================================================
dnl Structure of install set
dnl ===================================================================

if test $_os = Darwin -a "$enable_canonical_installation_tree_structure" = yes; then
    LIBO_BIN_FOLDER=MacOS
    LIBO_ETC_FOLDER=Resources
    LIBO_LIBEXEC_FOLDER=MacOS
    LIBO_LIB_FOLDER=Frameworks
    LIBO_SHARE_FOLDER=Resources
    LIBO_SHARE_HELP_FOLDER=Resources/help
    LIBO_SHARE_JAVA_FOLDER=Resources/java
    LIBO_SHARE_PRESETS_FOLDER=Resources/presets
    LIBO_LIB_PYUNO_FOLDER=Resources
    LIBO_SHARE_RESOURCE_FOLDER=Resources/resource
    LIBO_SHARE_SHELL_FOLDER=Resources/shell
    LIBO_URE_BIN_FOLDER=MacOS
    LIBO_URE_ETC_FOLDER=Resources
    LIBO_URE_LIB_FOLDER=Frameworks
    LIBO_URE_SHARE_FOLDER=Resources/ure
    LIBO_URE_SHARE_JAVA_FOLDER=Resources/java
    ENABLE_MACOSX_MACLIKE_APP_STRUCTURE=TRUE
    AC_DEFINE(HAVE_FEATURE_MACOSX_MACLIKE_APP_STRUCTURE)
elif test $_os = Darwin; then
    LIBO_BIN_FOLDER=MacOS
    LIBO_ETC_FOLDER=MacOS
    LIBO_LIBEXEC_FOLDER=MacOS
    LIBO_LIB_FOLDER=MacOS
    LIBO_LIB_PYUNO_FOLDER=MacOS
    LIBO_SHARE_FOLDER=share
    LIBO_SHARE_HELP_FOLDER=help
    LIBO_SHARE_JAVA_FOLDER=MacOS/classes
    LIBO_SHARE_PRESETS_FOLDER=presets
    LIBO_SHARE_RESOURCE_FOLDER=MacOS/resource
    LIBO_SHARE_SHELL_FOLDER=MacOS/shell
    LIBO_URE_BIN_FOLDER=ure-link/bin
    LIBO_URE_ETC_FOLDER=ure-link/bin
    LIBO_URE_LIB_FOLDER=ure-link/lib
    LIBO_URE_SHARE_FOLDER=ure-link/share
    LIBO_URE_SHARE_JAVA_FOLDER=ure-link/share/java
elif test $_os = WINNT; then
    LIBO_BIN_FOLDER=program
    LIBO_ETC_FOLDER=program
    LIBO_LIBEXEC_FOLDER=program
    LIBO_LIB_FOLDER=program
    LIBO_LIB_PYUNO_FOLDER=program
    LIBO_SHARE_FOLDER=share
    LIBO_SHARE_HELP_FOLDER=help
    LIBO_SHARE_JAVA_FOLDER=program/classes
    LIBO_SHARE_PRESETS_FOLDER=presets
    LIBO_SHARE_RESOURCE_FOLDER=program/resource
    LIBO_SHARE_SHELL_FOLDER=program/shell
    LIBO_URE_BIN_FOLDER=URE/bin
    LIBO_URE_ETC_FOLDER=URE/bin
    LIBO_URE_LIB_FOLDER=URE/bin
    LIBO_URE_SHARE_FOLDER=URE
    LIBO_URE_SHARE_JAVA_FOLDER=URE/java
else
    LIBO_BIN_FOLDER=program
    LIBO_ETC_FOLDER=program
    LIBO_LIBEXEC_FOLDER=program
    LIBO_LIB_FOLDER=program
    LIBO_LIB_PYUNO_FOLDER=program
    LIBO_SHARE_FOLDER=share
    LIBO_SHARE_HELP_FOLDER=help
    LIBO_SHARE_JAVA_FOLDER=program/classes
    LIBO_SHARE_PRESETS_FOLDER=presets
    LIBO_SHARE_RESOURCE_FOLDER=program/resource
    LIBO_SHARE_SHELL_FOLDER=program/shell
    LIBO_URE_BIN_FOLDER=ure/bin
    LIBO_URE_ETC_FOLDER=ure/bin
    LIBO_URE_LIB_FOLDER=ure/lib
    LIBO_URE_SHARE_FOLDER=ure/share
    LIBO_URE_SHARE_JAVA_FOLDER=ure/share/java
fi
AC_DEFINE_UNQUOTED(LIBO_BIN_FOLDER,"$LIBO_BIN_FOLDER")
AC_DEFINE_UNQUOTED(LIBO_ETC_FOLDER,"$LIBO_ETC_FOLDER")
AC_DEFINE_UNQUOTED(LIBO_LIBEXEC_FOLDER,"$LIBO_LIBEXEC_FOLDER")
AC_DEFINE_UNQUOTED(LIBO_LIB_FOLDER,"$LIBO_LIB_FOLDER")
AC_DEFINE_UNQUOTED(LIBO_LIB_PYUNO_FOLDER,"$LIBO_LIB_PYUNO_FOLDER")
AC_DEFINE_UNQUOTED(LIBO_SHARE_FOLDER,"$LIBO_SHARE_FOLDER")
AC_DEFINE_UNQUOTED(LIBO_SHARE_HELP_FOLDER,"$LIBO_SHARE_HELP_FOLDER")
AC_DEFINE_UNQUOTED(LIBO_SHARE_JAVA_FOLDER,"$LIBO_SHARE_JAVA_FOLDER")
AC_DEFINE_UNQUOTED(LIBO_SHARE_PRESETS_FOLDER,"$LIBO_SHARE_PRESETS_FOLDER")
AC_DEFINE_UNQUOTED(LIBO_SHARE_RESOURCE_FOLDER,"$LIBO_SHARE_RESOURCE_FOLDER")
AC_DEFINE_UNQUOTED(LIBO_SHARE_SHELL_FOLDER,"$LIBO_SHARE_SHELL_FOLDER")
AC_DEFINE_UNQUOTED(LIBO_URE_BIN_FOLDER,"$LIBO_URE_BIN_FOLDER")
AC_DEFINE_UNQUOTED(LIBO_URE_ETC_FOLDER,"$LIBO_URE_ETC_FOLDER")
AC_DEFINE_UNQUOTED(LIBO_URE_LIB_FOLDER,"$LIBO_URE_LIB_FOLDER")
AC_DEFINE_UNQUOTED(LIBO_URE_SHARE_FOLDER,"$LIBO_URE_SHARE_FOLDER")
AC_DEFINE_UNQUOTED(LIBO_URE_SHARE_JAVA_FOLDER,"$LIBO_URE_SHARE_JAVA_FOLDER")

# Not all of them needed in config_host.mk, add more if need arises
AC_SUBST(LIBO_BIN_FOLDER)
AC_SUBST(LIBO_ETC_FOLDER)
AC_SUBST(LIBO_LIB_FOLDER)
AC_SUBST(LIBO_LIB_PYUNO_FOLDER)
AC_SUBST(LIBO_SHARE_FOLDER)
AC_SUBST(LIBO_SHARE_HELP_FOLDER)
AC_SUBST(LIBO_SHARE_JAVA_FOLDER)
AC_SUBST(LIBO_SHARE_PRESETS_FOLDER)
AC_SUBST(LIBO_SHARE_RESOURCE_FOLDER)
AC_SUBST(LIBO_URE_BIN_FOLDER)
AC_SUBST(LIBO_URE_LIB_FOLDER)
AC_SUBST(LIBO_URE_SHARE_FOLDER)
AC_SUBST(LIBO_URE_SHARE_JAVA_FOLDER)
AC_SUBST(ENABLE_MACOSX_MACLIKE_APP_STRUCTURE)

dnl ===================================================================
dnl Windows specific tests and stuff
dnl ===================================================================

# Get a value from the 32-bit side of the Registry
reg_get_value_32()
{
    # Return value: $regvalue
    unset regvalue
    _regvalue=`cat "/proc/registry32/$1" 2> /dev/null`

    if test $? -eq 0; then
        regvalue=$_regvalue
    fi
}

# Get a value from the 64-bit side of the Registry
reg_get_value_64()
{
    # Return value: $regvalue
    unset regvalue
    _regvalue=`cat "/proc/registry64/$1" 2> /dev/null`

    if test $? -eq 0; then
        regvalue=$_regvalue
    fi
}

if test "$_os" = "WINNT"; then
    AC_MSG_CHECKING([whether to build a 64-bit LibreOffice])
    if test "$enable_64_bit" = "" -o "$enable_64_bit" = "no"; then
        AC_MSG_RESULT([no])
        WINDOWS_SDK_ARCH="x86"
    else
        AC_MSG_RESULT([yes])
        WINDOWS_SDK_ARCH="x64"
        BITNESS_OVERRIDE=64
    fi
fi

if test "$_os" = "WINNT" -a "$WITH_MINGW" != yes; then
    AC_MSG_CHECKING([whether to use DirectX])
    if test "$enable_directx" = "yes" -o "$enable_directx" = ""; then
        ENABLE_DIRECTX="TRUE"
        AC_MSG_RESULT([yes])
    else
        ENABLE_DIRECTX=""
        AC_MSG_RESULT([no])
    fi

    AC_MSG_CHECKING([whether to use ActiveX])
    if test "$enable_activex" = "yes" -o "$enable_activex" = ""; then
        DISABLE_ACTIVEX=""
        SCPDEFS="$SCPDEFS -DWITH_ACTIVEX_COMPONENT"
        AC_MSG_RESULT([yes])
    else
        DISABLE_ACTIVEX="TRUE"
        AC_MSG_RESULT([no])
    fi

    AC_MSG_CHECKING([whether to use ATL])
    if test "$enable_atl" = "yes" -o "$enable_atl" = ""; then
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
        AC_DEFINE_UNQUOTED(CLANG_FULL_VERSION,$CLANG_FULL_VERSION)
    fi
fi
AC_SUBST(COM_GCC_IS_CLANG)

if test "$CCACHE" != "" -a "$COM_GCC_IS_CLANG" = TRUE; then
    if test -z "$CCACHE_CPP2"; then
            AC_MSG_WARN([Using ccache with Clang without CCACHE_CPP2 set causes spurious warnings.])
            add_warning "Using ccache with Clang without CCACHE_CPP2 set causes spurious warnings."
    fi
fi

dnl ===================================================================
dnl  Test the gcc version
dnl ===================================================================
if test "$GCC" = "yes" -a -z "$COM_GCC_IS_CLANG"; then
    AC_MSG_CHECKING([the GCC version])
    _gcc_version=`$CC -dumpversion`
    GCC_VERSION=`echo $_gcc_version | $AWK -F. '{ print \$1*100+\$2 }'`

    AC_MSG_RESULT([gcc $_gcc_version])
    if test "$GCC_VERSION" -lt 0401; then
        AC_MSG_ERROR([GCC $_gcc_version is too old, must be at least GCC 4.1.0])
    fi
else
    # Explicitly force GCC_VERSION to be empty, even for Clang, to check incorrect uses.
    # GCC version should generally be checked only when handling GCC-specific bugs, for testing
    # things like features configure checks should be used, otherwise they may e.g. fail with Clang
    # (which reports itself as GCC 4.2.1).
    GCC_VERSION=
fi
AC_SUBST(GCC_VERSION)

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

# Use -isystem (gcc) if possible, to avoid warnigs in 3rd party headers.
# NOTE: must _not_ be used for bundled external libraries!
ISYSTEM=
if test "$GCC" = "yes"; then
    AC_MSG_CHECKING( for -isystem )
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -Werror"
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[ return 0; ]])],[ ISYSTEM=-isystem ],[])
    CFLAGS=$save_CFLAGS
    if test -n "$ISYSTEM"; then
        AC_MSG_RESULT(yes)
    else
        AC_MSG_RESULT(no)
    fi
fi
if test -z "$ISYSTEM"; then
    # fall back to using -I
    ISYSTEM=-I
fi
AC_SUBST(ISYSTEM)

dnl ===================================================================
dnl  Check which Visual Studio or MinGW compiler is used
dnl ===================================================================

map_vs_year_to_version()
{
    # Return value: $vsversion

    unset vsversion

    case $1 in
    2010)
        vsversion=10.0;;
    2012)
        vsversion=11.0;;
    *)
        AC_MSG_ERROR([Assertion failure - invalid argument "$1" to map_vs_year_to_version()]);;
    esac
}

vs_versions_to_check()
{
    # Args: $1 (optional) : versions to check, in the order of preference
    # Return value: $vsversions

    unset vsversions

    if test -n "$1"; then
        map_vs_year_to_version "$1"
        vsversions=$vsversion
    else
        # By default we prefer 2012, then 2010
        vsversions="11.0 10.0"
    fi
}

find_msvs()
{
    # Find Visual Studio 2012/2010
    # Args: $1 (optional) : versions to check, in the order of preference
    # Return value: $vstest

    unset vstest

    vs_versions_to_check "$1"

    for ver in $vsversions; do
        reg_get_value_32 HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/$ver/Setup/VS/ProductDir
        if test -n "$regvalue"; then
            vstest=$regvalue
            break
        fi
        reg_get_value_32 HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VCExpress/$ver/Setup/VS/ProductDir
        if test -n "$regvalue"; then
            vstest=$regvalue
            break
        fi
    done
}

find_msvc()
{
    # Find Visual C++ 2012/2010
    # Args: $1 (optional) : The VS version year
    # Return values: $vctest, $vcyear, $vcnum, $vcnumwithdot

    unset vctest vcnum vcnumwithdot

    vs_versions_to_check "$1"

    for ver in $vsversions; do
        reg_get_value_32 HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/$ver/Setup/VC/ProductDir
        if test -n "$regvalue"; then
            vctest=$regvalue
            break
        fi
        reg_get_value_32 HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VCExpress/$ver/Setup/VC/ProductDir
        if test -n "$regvalue"; then
            vctest=$regvalue
            break
        fi
    done
    if test -n "$vctest"; then
        vcnumwithdot=$ver
        case "$vcnumwithdot" in
        10.0)
            vcyear=2010
            vcnum=100
            ;;
        11.0)
            vcyear=2012
            vcnum=110
            ;;
        esac
    fi
}

SHOWINCLUDES_PREFIX=
if test "$_os" = "WINNT"; then
    if test "$WITH_MINGW" != "yes"; then
        AC_MSG_CHECKING([Visual C++])

        find_msvc "$with_visual_studio"

        if test -z "$vctest"; then
            if test -n "$with_visual_studio"; then
                AC_MSG_ERROR([No Visual Studio $with_visual_studio installation found])
            else
                AC_MSG_ERROR([No Visual Studio 2012 or 2010 installation found])
            fi
        fi

        if test "$BITNESS_OVERRIDE" = ""; then
            if test -f "$vctest/bin/cl.exe"; then
                VC_PRODUCT_DIR=$vctest
            else
                AC_MSG_ERROR([No compiler (cl.exe) in $vctest/bin/cl.exe])
            fi
        else
            if test -f "$vctest/bin/amd64/cl.exe"; then
                VC_PRODUCT_DIR=$vctest
            else
                AC_MSG_ERROR([No compiler (cl.exe) in $vctest/bin/amd64/cl.exe])
            fi
        fi

        VC_PRODUCT_DIR=`cygpath -d "$VC_PRODUCT_DIR"`
        VC_PRODUCT_DIR=`cygpath -u "$VC_PRODUCT_DIR"`
        AC_MSG_RESULT([$VC_PRODUCT_DIR])

        dnl ===========================================================
        dnl  Check for the corresponding mspdb*.dll
        dnl ===========================================================

        MSPDB_PATH=

        if test "$BITNESS_OVERRIDE" == ""; then
            MSPDB_PATH="$VC_PRODUCT_DIR/../Common7/IDE"
        else
            MSPDB_PATH="$VC_PRODUCT_DIR/bin/amd64"
        fi

        mspdbnum=$vcnum

        if test ! -e "$MSPDB_PATH/mspdb${mspdbnum}.dll"; then
            AC_MSG_ERROR([No mspdb${mspdbnum}.dll in $MSPDB_PATH, Visual Studio installation broken?])
        fi

        MSPDB_PATH=`cygpath -d "$MSPDB_PATH"`
        MSPDB_PATH=`cygpath -u "$MSPDB_PATH"`

        dnl The path needs to be added before cl is called
        PATH="$MSPDB_PATH:$PATH"

        AC_MSG_CHECKING([cl.exe])

        # Is there really ever a need to pass CC explicitly? Surely we can hope to get all the
        # automagical niceness to work OK? If somebody has some unsupported compiler in some weird
        # location, isn't it likely that lots of other things needs changes, too, and just setting CC
        # is not enough?

        if test -z "$CC"; then
            if test "$BITNESS_OVERRIDE" = ""; then
                if test -f "$VC_PRODUCT_DIR/bin/cl.exe"; then
                    CC="$VC_PRODUCT_DIR/bin/cl.exe"
                fi
            else
                if test -f "$VC_PRODUCT_DIR/bin/amd64/cl.exe"; then
                    CC="$VC_PRODUCT_DIR/bin/amd64/cl.exe"
                fi
            fi

            # This gives us a posix path with 8.3 filename restrictions
            CC=`cygpath -d "$CC"`
            CC=`cygpath -u "$CC"`
        fi

        if test -n "$CC"; then
            # Remove /cl.exe from CC case insensitive
            AC_MSG_RESULT([found Visual C++ $vcyear ($CC)])
            if test "$BITNESS_OVERRIDE" = ""; then
                COMPATH=`echo $CC | $SED -e 's@\/[[Bb]][[Ii]][[Nn]]\/[[cC]][[lL]]\.[[eE]][[xX]][[eE]].*@@' -e 's@^.* @@'`
            else
                if test -n "$VC_PRODUCT_DIR"; then
                    # Huh, why not just an assignment?
                    COMPATH=`echo $VC_PRODUCT_DIR`
                fi
            fi
            export INCLUDE=`cygpath -d "$COMPATH/Include"`

            PathFormat "$COMPATH"
            COMPATH="$formatted_path"

            VCVER=$vcnum
            MSVSVER=$vcyear

            # The WINDOWS_SDK_ACCEPTABLE_VERSIONS is mostly an educated guess...  Assuming newer ones
            # are always "better", we list them in reverse chronological order.

            case $vcnum in
            100)
                COMEX=13
                WINDOWS_SDK_ACCEPTABLE_VERSIONS="7.1A 7.1 7.0A 6.0A"
                ;;
            110)
                COMEX=14
                WINDOWS_SDK_ACCEPTABLE_VERSIONS="8.0 7.1A"
                ;;
            esac

            # The expectation is that --with-windows-sdk should not need to be used
            if test -n "$with_windows_sdk"; then
                case " $WINDOWS_SDK_ACCEPTABLE_VERSIONS " in
                *" "$with_windows_sdk" "*)
                    WINDOWS_SDK_ACCEPTABLE_VERSIONS=$with_windows_sdk
                    ;;
                *)
                    AC_MSG_ERROR([Windows SDK $with_windows_sdk is not known to work witn VS $MSVSVER])
                    ;;
                esac
            fi

            # Make AC_COMPILE_IFELSE etc. work (set by AC_PROG_C, which we don't use for MSVC)
            ac_objext=obj
            ac_exeext=exe

        else
            AC_MSG_ERROR([Visual C++ not found after all, huh])
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
        # unfinished work and highly experimental.

        BUILD_X64=
        CXX_X64_BINARY=
        LINK_X64_BINARY=

        if test "$BITNESS_OVERRIDE" = ""; then
            AC_MSG_CHECKING([for a x64 compiler and libraries for 64-bit Explorer extensions])
            if test -f "$VC_PRODUCT_DIR/atlmfc/lib/amd64/atls.lib"; then
                # Prefer native x64 compiler to cross-compiler, in case we are running
                # the build on a 64-bit OS.
                if "$VC_PRODUCT_DIR/bin/amd64/cl.exe" -? </dev/null >/dev/null 2>&1; then
                    BUILD_X64=TRUE
                    CXX_X64_BINARY="$VC_PRODUCT_DIR/bin/amd64/cl.exe"
                    LINK_X64_BINARY="$VC_PRODUCT_DIR/bin/amd64/link.exe"
                elif "$VC_PRODUCT_DIR/bin/x86_amd64/cl.exe" -? </dev/null >/dev/null 2>&1; then
                    BUILD_X64=TRUE
                    CXX_X64_BINARY="$VC_PRODUCT_DIR/bin/x86_amd64/cl.exe"
                    LINK_X64_BINARY="$VC_PRODUCT_DIR/bin/x86_amd64/link.exe"
                fi
            fi
            if test "$BUILD_X64" = TRUE; then
                AC_MSG_RESULT([found])
            else
                AC_MSG_RESULT([not found])
                AC_MSG_WARN([Installation set will not contain 64-bit Explorer extensions])
            fi
        fi
        AC_SUBST(BUILD_X64)

        # These are passed to the environment and then used in gbuild/platform/com_MSC_class.mk
        AC_SUBST(CXX_X64_BINARY)
        AC_SUBST(LINK_X64_BINARY)
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
AC_SUBST(VCVER)
PathFormat "$MSPDB_PATH"
MSPDB_PATH="$formatted_path"
AC_SUBST(SHOWINCLUDES_PREFIX)

#
# dbghelp.dll
#
if test "$_os" == "WINNT"; then
    BUILD_TYPE="$BUILD_TYPE DBGHELP"
fi

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
    # this is an extra var so it can have different default on different MSVC
    # versions (in case there are version specific problems with it)
    MSVC_USE_DEBUG_RUNTIME="TRUE"
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
    if test "$with_system_orcus" = "yes"; then
        AC_MSG_ERROR([--with-system-orcus conflicts with --enable-dbgutil])
    else
        with_system_orcus=no
    fi
    if test "$with_system_libcmis" = "yes"; then
        AC_MSG_ERROR([--with-system-libcmis conflicts with --enable-dbgutil])
    else
        with_system_libcmis=no
    fi
else
    ENABLE_DBGUTIL=""
    MSVC_USE_DEBUG_RUNTIME=""
    # PRODUCT is old concept, still used by build.pl .
    PRODUCT="full"
    PROEXT=".pro"
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_DBGUTIL)
AC_SUBST(MSVC_USE_DEBUG_RUNTIME)
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

dnl Check for enable symbols option
dnl ===================================================================
AC_MSG_CHECKING([whether to include symbols while preserve optimization])
if test -n "$enable_symbols" -a "$enable_symbols" != "no"; then
    ENABLE_SYMBOLS="TRUE"
    if test -n "$ENABLE_DBGUTIL"; then
        AC_MSG_ERROR([--enable-dbgutil cannot be used with --enable-symbols])
    elif test -n "$ENABLE_DEBUG"; then
        AC_MSG_ERROR([--enable-debug cannot be used with --enable-symbols])
    fi
    AC_MSG_RESULT([yes])
else
    if test -n "$enable_symbols" -a "$enable_symbols" = "no"; then
        ENABLE_SYMBOLS="FALSE"
    else
        ENABLE_SYMBOLS=
    fi
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_SYMBOLS)

# Check for explicit A/C/CXX/OBJC/OBJCXX/LDFLAGS.
# By default use the ones specified by our build system,
# but explicit override is possible.
AC_MSG_CHECKING(for explicit AFLAGS)
if test -n "$AFLAGS"; then
    AC_MSG_RESULT([$AFLAGS])
    x_AFLAGS=
else
    AC_MSG_RESULT(no)
    x_AFLAGS=[\#]
fi
AC_MSG_CHECKING(for explicit CFLAGS)
if test -n "$CFLAGS"; then
    AC_MSG_RESULT([$CFLAGS])
    x_CFLAGS=
else
    AC_MSG_RESULT(no)
    x_CFLAGS=[\#]
fi
AC_MSG_CHECKING(for explicit CXXFLAGS)
if test -n "$CXXFLAGS"; then
    AC_MSG_RESULT([$CXXFLAGS])
    x_CXXFLAGS=
else
    AC_MSG_RESULT(no)
    x_CXXFLAGS=[\#]
fi
AC_MSG_CHECKING(for explicit OBJCFLAGS)
if test -n "$OBJCFLAGS"; then
    AC_MSG_RESULT([$OBJCFLAGS])
    x_OBJCFLAGS=
else
    AC_MSG_RESULT(no)
    x_OBJCFLAGS=[\#]
fi
AC_MSG_CHECKING(for explicit OBJCXXFLAGS)
if test -n "$OBJCXXFLAGS"; then
    AC_MSG_RESULT([$OBJCXXFLAGS])
    x_OBJCXXFLAGS=
else
    AC_MSG_RESULT(no)
    x_OBJCXXFLAGS=[\#]
fi
AC_MSG_CHECKING(for explicit LDFLAGS)
if test -n "$LDFLAGS"; then
    AC_MSG_RESULT([$LDFLAGS])
    x_LDFLAGS=
else
    AC_MSG_RESULT(no)
    x_LDFLAGS=[\#]
fi
AC_SUBST(AFLAGS)
AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(OBJCFLAGS)
AC_SUBST(OBJCXXFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(x_AFLAGS)
AC_SUBST(x_CFLAGS)
AC_SUBST(x_CXXFLAGS)
AC_SUBST(x_OBJCFLAGS)
AC_SUBST(x_OBJCXXFLAGS)
AC_SUBST(x_LDFLAGS)

#
# determine CPU, CPUNAME, GUIBASE, ...
#
LIB64="lib"
SOLARINC=

case "$host_os" in

aix*)
    COM=GCC
    CPU=P
    CPUNAME=POWERPC
    GUIBASE=unx
    OS=AIX
    RTL_OS=AIX
    RTL_ARCH=PowerPC
    PLATFORMID=aix_powerpc
    OUTPATH=unxaigppc
    P_SEP=:
    SOLARLIB="-L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    ;;

cygwin*)
    COM=MSC
    GUIBASE=not-used
    OS=WNT
    RTL_OS=Windows
    P_SEP=";"
    CPP_LIBRARY="MSVCRT"

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
    SOLARLIB="-L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    ;;

darwin*)
    COM=GCC
    GUIBASE=not-used
    OS=MACOSX
    RTL_OS=MacOSX
    P_SEP=:

    case "$host_cpu" in
    arm*)
        CPU=R
        CPUNAME=ARM
        RTL_ARCH=ARM_EABI
        PLATFORMID=ios_arm
        OUTPATH=unxiosr
        OS=IOS
        SDKDIR=sdk
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
    SOLARLIB="-L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    ;;

freebsd*)
    COM=GCC
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
    SOLARLIB="-L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    ;;

kfreebsd*)
    COM=GCC
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
    SOLARLIB="-L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    ;;

linux-gnu*)
    COM=GCC
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
    SOLARLIB="-L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    ;;

linux-android*)
    COM=GCC
    GUIBASE=not-used
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
    SOLARLIB="-L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    ;;

mingw*)
    COM=GCC
    GUIBASE=not-used
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
            SOLARINC="$SOLARINC -I$SRC_ROOT/include/wntgccx"
        else
            CPU=I
            CPUNAME=INTEL
            RTL_ARCH=x86
            PLATFORMID=windows_x86
            OUTPATH=wntgcci$COMEX
            SOLARINC="$SOLARINC -I$SRC_ROOT/include/wntgcci"
        fi
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac

    SOLARLIB="-L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    ;;

*netbsd*)

    COM=GCC
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
    SOLARLIB="-L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    ;;

openbsd*)
    COM=GCC
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
    SOLARLIB="-L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    SOLARLIB="$SOLARLIB -L/usr/local/lib"

    ;;

solaris*)

    COM=GCC
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
    SOLARLIB="-L${SOLARVER}/${OUTPATH}${PROEXT}/lib"
    SOLARLIB="$SOLARLIB -L$COMPATH/lib"
    SOLARLIB="$SOLARLIB -L/usr/local/bin -L/usr/dt/lib -L/usr/openwin/lib"
    ;;

*)
    AC_MSG_ERROR([$host_os operating system is not suitable to build LibreOffice for!])
    ;;
esac

if test "$enable_headless" = "yes"; then
    if test "$GUIBASE" != "unx"; then
        AC_MSG_ERROR([$host_os operating system is not suitable to build LibreOffice with --enable-headless])
    fi
    GUIBASE=not-used
fi

INPATH="${OUTPATH}${PROEXT}"
if test -n "${with_solver_and_workdir_root}"; then
    PathFormat "${with_solver_and_workdir_root}"
    # IsValidFilePath in /sal/osl/w32/file_url.cxx rejects "X:\\" breaking idlc
    if echo ${formatted_path} | $GREP -q '/$'; then
        WORKDIR=${formatted_path}workdir/${INPATH}
        INSTDIR=${formatted_path}instdir/${INPATH}
    else
        WORKDIR=${formatted_path}/workdir/${INPATH}
        INSTDIR=${formatted_path}/instdir/${INPATH}
    fi
else
    WORKDIR=${BUILDDIR}/workdir/${INPATH}
    INSTDIR=${BUILDDIR}/instdir/${INPATH}
fi
OUTDIR="${SOLARVER}/${INPATH}"
SOLARINC="-I. -I$SRC_ROOT/include $SOLARINC"
AC_SUBST(COM)
AC_SUBST(CPU)
AC_SUBST(CPUNAME)
AC_SUBST(RTL_OS)
AC_SUBST(RTL_ARCH)
AC_SUBST(EPM_FLAGS)
AC_SUBST(GUIBASE)
AC_SUBST(INPATH)
AC_SUBST([INSTDIR])
AC_SUBST(OS)
AC_SUBST(OUTDIR)
AC_SUBST(OUTPATH)
AC_SUBST(P_SEP)
AC_SUBST(SOLARVER)
AC_SUBST(WORKDIR)
AC_SUBST(PLATFORMID)
AC_DEFINE_UNQUOTED(OUTDIR,"$OUTDIR")
AC_DEFINE_UNQUOTED(WORKDIR,"$WORKDIR")

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
    (cd $SRC_ROOT && tar cf - \
        config.guess \
        config_host.mk.in \
        Makefile.in \
        lo.xcent.in \
        config_host/*.in \
        bin/get_config_variables \
        solenv/bin/getcompver.awk \
        solenv/inc/langlist.mk \
        instsetoo_native/util/openoffice.lst.in) \
    | (cd CONF-FOR-BUILD && tar xf -)
    cp configure CONF-FOR-BUILD
    test -d config_build && cp -p config_build/*.h CONF-FOR-BUILD/config_host
    (
    unset COM GUIBASE OS CPU CPUNAME
    unset CC CXX SYSBASE CFLAGS
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
    test $with_junit = no && sub_conf_opts="$sub_conf_opts --without-junit"
    test -n "$TARFILE_LOCATION" && sub_conf_opts="$sub_conf_opts --with-external-tar=$TARFILE_LOCATION"
    test -n "${with_solver_and_workdir_root}" && sub_conf_opts="$sub_conf_opts --with-solver-and-workdir-root=${with_solver_and_workdir_root}"
    test -n "$with_system_boost_for_build" && sub_conf_opts="$sub_conf_opts --with-system-boost"
    test -n "$with_system_cppunit_for_build" && sub_conf_opts="$sub_conf_opts --with-system-cppunit"
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
        --disable-postgresql-sdbc \
        --with-parallelism="$with_parallelism" \
        --without-doxygen \
        --without-java \
        ENABLE_PDFIMPORT=FALSE \
        $sub_conf_opts \
        --srcdir=$srcdir \
        2>&1 | sed -e 's/^/    /'
    test -f ./config_host.mk 2>/dev/null || exit
    cp config_host.mk ../config_build.mk
    mv config.log ../config.Build.log
    mkdir -p ../config_build
    mv config_host/*.h ../config_build
    . ./bin/get_config_variables OS PATH CC CXX INPATH SYSTEM_LIBXSLT OUTDIR WORKDIR

    for V in OS CC CXX INPATH SYSTEM_LIBXSLT; do
        VV='$'$V
        VV=`eval "echo $VV"`
        if test -n "$VV"; then
            line=${V}_FOR_BUILD='${'${V}_FOR_BUILD:-$VV'}'
            echo "$line" >>build-config
        fi
    done

    for V in OUTDIR WORKDIR; do
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
    OS_FOR_BUILD="$OS"
    CC_FOR_BUILD="$CC"
    CXX_FOR_BUILD="$CXX"
    INPATH_FOR_BUILD="$INPATH"
    OUTDIR_FOR_BUILD="$OUTDIR"
    WORKDIR_FOR_BUILD="$WORKDIR"
fi
AC_SUBST(OS_FOR_BUILD)
AC_SUBST(INPATH_FOR_BUILD)
AC_SUBST(OUTDIR_FOR_BUILD)
AC_SUBST(WORKDIR_FOR_BUILD)

dnl ===================================================================
dnl Our version is defined by the AC_INIT() at the top of this script.
dnl ===================================================================

set `echo AC_PACKAGE_VERSION | sed "s/\./ /g"`

LIBO_VERSION_MAJOR=$1
LIBO_VERSION_MINOR=$2
LIBO_VERSION_MICRO=$3
LIBO_VERSION_PATCH=$4

LIBO_VERSION_SUFFIX=$5
# Split out LIBO_VERSION_SUFFIX_SUFFIX... horrible crack. But apparently wanted separately in
# openoffice.lst as ABOUTBOXPRODUCTVERSIONSUFFIX. Note that the double brackets are for m4's sake,
# they get undoubled before actually passed to sed.
LIBO_VERSION_SUFFIX_SUFFIX=`echo "$LIBO_VERSION_SUFFIX" | sed -e 's/.*[[a-zA-Z0-9]]\([[^a-zA-Z0-9]]*\)$/\1/'`
test -n "$LIBO_VERSION_SUFFIX_SUFFIX" && LIBO_VERSION_SUFFIX="${LIBO_VERSION_SUFFIX%${LIBO_VERSION_SUFFIX_SUFFIX}}"
# LIBO_VERSION_SUFFIX, if non-empty, should include the period separator
test -n "$LIBO_VERSION_SUFFIX" && LIBO_VERSION_SUFFIX=".$LIBO_VERSION_SUFFIX"

AC_SUBST(LIBO_VERSION_MAJOR)
AC_SUBST(LIBO_VERSION_MINOR)
AC_SUBST(LIBO_VERSION_MICRO)
AC_SUBST(LIBO_VERSION_PATCH)
AC_SUBST(LIBO_VERSION_SUFFIX)
AC_SUBST(LIBO_VERSION_SUFFIX_SUFFIX)

AC_DEFINE_UNQUOTED(LIBO_VERSION_MAJOR,$LIBO_VERSION_MAJOR)
AC_DEFINE_UNQUOTED(LIBO_VERSION_MINOR,$LIBO_VERSION_MINOR)
AC_DEFINE_UNQUOTED(LIBO_VERSION_MICRO,$LIBO_VERSION_MICRO)
AC_DEFINE_UNQUOTED(LIBO_VERSION_PATCH,$LIBO_VERSION_PATCH)

LIBO_THIS_YEAR=`date +%Y`
AC_DEFINE_UNQUOTED(LIBO_THIS_YEAR,$LIBO_THIS_YEAR)

# This UPD silly thing must go away soon
UPD="${LIBO_VERSION_MAJOR}${LIBO_VERSION_MINOR}0"
AC_SUBST(UPD)

# This too should go away
SOURCEVERSION="OOO$UPD"
AC_SUBST(SOURCEVERSION)

dnl ===================================================================
dnl Check for syslog header
dnl ===================================================================
AC_CHECK_HEADER(syslog.h, AC_DEFINE(HAVE_SYSLOG_H))

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
if test $_os = Darwin; then
    enable_linkoo=no
fi

if test "$enable_linkoo" = "no"; then
    DISABLE_LINKOO="TRUE"
    AC_MSG_RESULT([no])
else
    DISABLE_LINKOO=
    AC_MSG_RESULT([yes])
fi
AC_SUBST(DISABLE_LINKOO)

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
    enable_cairo_canvas=no
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
    SYSTEM_FONTCONFIG=YES
fi
AC_SUBST(FONTCONFIG_CFLAGS)
AC_SUBST(FONTCONFIG_LIBS)
AC_SUBST([SYSTEM_FONTCONFIG])

dnl whether to find & fetch external tarballs?
dnl ===================================================================
if test -z "$TARFILE_LOCATION"; then
    TARFILE_LOCATION="$SRC_ROOT/src"
fi
AC_SUBST(TARFILE_LOCATION)

AC_MSG_CHECKING([whether we want to fetch tarballs])
if test "$enable_fetch_external" != "no"; then
    if test "$with_all_tarballs" = "yes"; then
        AC_MSG_RESULT(["yes, all of them"])
        DO_FETCH_TARBALLS="ALL"
    else
        AC_MSG_RESULT(["yes, if we use them"])
        DO_FETCH_TARBALLS="YES"
    fi
else
    AC_MSG_RESULT([no])
    DO_FETCH_TARBALLS="NO"
fi
AC_SUBST(DO_FETCH_TARBALLS)

AC_MSG_CHECKING([whether to build help])
HELP_COMMON_ONLY=FALSE
if test -n "$with_help" -a "$with_help" != "no" -a $_os != iOS -a $_os != Android; then
    BUILD_TYPE="$BUILD_TYPE HELP"
    GIT_NEEDED_SUBMODULES="helpcontent2 $GIT_NEEDED_SUBMODULES"
    if test "$with_help" = "common" ; then
        HELP_COMMON_ONLY=TRUE
        AC_MSG_RESULT([common only])
    else
        SCPDEFS="$SCPDEFS -DWITH_HELP"
        AC_MSG_RESULT([yes])
    fi
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(HELP_COMMON_ONLY)

dnl Test whether to include MySpell dictionaries
dnl ===================================================================
AC_MSG_CHECKING([whether to include MySpell dictionaries])
if test "$with_myspell_dicts" = "yes"; then
    AC_MSG_RESULT([yes])
    WITH_MYSPELL_DICTS=YES
    BUILD_TYPE="$BUILD_TYPE DICTIONARIES"
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
AC_MSG_CHECKING([whether to enable pch feature])
if test -n "$enable_pch" && test "$enable_pch" != "no"; then
    if test "$_os" = "WINNT" -a "$WITH_MINGW" != "yes"; then
        ENABLE_PCH="TRUE"
        AC_MSG_RESULT([yes])
    elif test "$GCC" = "yes"; then
        ENABLE_PCH="TRUE"
        AC_MSG_RESULT([yes])
    else
        ENABLE_PCH=""
        AC_MSG_ERROR([Precompiled header not yet supported for your platform/compiler])
    fi
else
    ENABLE_PCH=""
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_PCH)

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
            GNUMAKE=`which $a`
            break
        fi
    fi
done
AC_MSG_RESULT($GNUMAKE)
if test -z "$GNUMAKE"; then
    AC_MSG_ERROR([not found. install GNU make.])
fi

TAB=`printf '\t'`

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
    AC_CHECK_PROGS(SHA1SUM, sha1sum sha1 shasum openssl)
    if test -z "$SHA1SUM"; then
        AC_MSG_ERROR([install the approproate SHA-1 checksumming program for this OS])
    elif test "$SHA1SUM" = "openssl"; then
        SHA1SUM="openssl sha1"
    fi
    AC_MSG_CHECKING([for GNU make bug 20033])
    TESTGMAKEBUG20033=`mktemp -d tmp.XXXXXX`
    $SED -e "s/<TAB>/$TAB/g" > $TESTGMAKEBUG20033/Makefile << EOF
A := \$(wildcard *.a)

.PHONY: all
all: \$(A:.a=.b)
<TAB>@echo survived bug20033.

.PHONY: setup
setup:
<TAB>@touch 1.a 2.a 3.a 4.a 5.a 6.a

define d1
@echo lala \$(1)
@sleep 1
endef

define d2
@echo tyty \$(1)
@sleep 1
endef

%.b : %.a
<TAB>\$(eval CHECKSUM := \$(word 1,\$(shell cat \$^ | $SHA1SUM))) \$(if \$(wildcard \$(CACHEDIR)/\$(CHECKSUM)),\
<TAB>\$(call d1,\$(CHECKSUM)),\
<TAB>\$(call d2,\$(CHECKSUM)))
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
$SED -e "s/<TAB>/$TAB/" > $TESTGMAKEFILEFUNC/Makefile << EOF
\$(file >test.txt,Success )

.PHONY: all
all:
<TAB>@cat test.txt

EOF
$GNUMAKE -C $TESTGMAKEFILEFUNC 2>/dev/null 1>&2
if test -f $TESTGMAKEFILEFUNC/test.txt; then
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
if `$PERL -e 'use Archive::Zip; use Cwd; use Digest::MD5'>/dev/null 2>&1`; then
    AC_MSG_RESULT([all modules found])
else
    AC_MSG_RESULT([failed to find some modules])
    # Find out which modules are missing.
    missing_perl_modules=
    if ! `$PERL -e 'use Archive::Zip;'>/dev/null 2>&1`; then
       missing_perl_modules=Archive::Zip
    fi
    if ! `$PERL -e 'use Cwd;'>/dev/null 2>&1`; then
       missing_perl_modules="$missing_perl_modules Cwd"
    fi
    if ! `$PERL -e 'use Digest::MD5;'>/dev/null 2>&1`; then
       missing_perl_modules="$missing_perl_modules Digest::MD5"
    fi
       AC_MSG_ERROR([
    The missing Perl modules are: $missing_perl_modules
    Install them as superuser/administrator with "cpan -i $missing_perl_modules"])
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
dnl ===================================================================

if test $_os = Darwin; then
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

find_csc()
{
    # Return value: $csctest

    unset csctest

    reg_get_value_32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/NET Framework Setup/NDP/v4/Client/InstallPath"
    if test -n "$regvalue"; then
        csctest=$regvalue
        return
    fi
}

find_al()
{
    # Return value: $altest

    unset altest

    for x in `ls /proc/registry/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft\ SDKs/Windows`; do
        reg_get_value_32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/$x/WinSDK-NetFx40Tools/InstallationFolder"
        if test -n "$regvalue" -a \( -f "$regvalue/al.exe" -o -f "$regvalue/bin/al.exe" \); then
            altest=$regvalue
            return
        fi
    done
}

find_dotnetsdk()
{
    # Return value: $frametest (that's a silly name...)

    unset frametest

    for ver in 1.1 2.0; do
        reg_get_value_32 HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/.NETFramework/sdkInstallRootv$ver
        if test -n "$regvalue"; then
            frametest=$regvalue
            return
        fi
    done
}

find_winsdk_version()
{
    # Args: $1 : SDK version as in "6.0A", "7.0" etc
    # Return value: $winsdktest

    unset winsdktest

    # Why we look for them in this particular order I don't know. But OTOH I
    case "$1" in
    6.0*|7.*)
        reg_get_value_32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v${1}/InstallationFolder"
        if test -n "$regvalue"; then
            winsdktest=$regvalue
            return
        fi
        ;;
    8.*)
        reg_get_value_32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Windows Kits/Installed Roots/KitsRoot"
        if test -n "$regvalue"; then
            winsdktest=$regvalue
            return
        fi
        ;;
    esac
}

find_winsdk()
{
    # Args: $1 (optional) : list of acceptable SDK versions
    # Return value: $winsdktest

    unset winsdktest

    if test -n "$1"; then
        sdkversions=$1
    else
        sdkversions="$WINDOWS_SDK_ACCEPTABLE_VERSIONS"
    fi

    for ver in $sdkversions; do
        find_winsdk_version $ver
        if test -n "$winsdktest"; then
            return
        fi
    done
}

find_msms()
{
    for ver in 10.0 11.0; do
        reg_get_value_32 HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/$ver/Setup/VS/MSMDir
        if test -n "$regvalue"; then
            if test -e "$regvalue/Microsoft_VC${VCVER}_CRT_x86.msm"; then
                msmdir=$regvalue
                break
            fi
        fi
    done
    if test -z "$msmdir"; then
        AC_MSG_NOTICE([no registry entry for Merge Module directory - trying "$COMMONPROGRAMFILES\Merge Modules"])
        msmdir="$COMMONPROGRAMFILES\Merge Modules"
    fi
    msmdir=`cygpath -d "$msmdir"`
    msmdir=`cygpath -u "$msmdir"`
    if test -z "$msmdir"; then
        if test "$ENABLE_RELEASE_BUILD" = "TRUE" ; then
            AC_MSG_ERROR([Merge modules not found in $msmdir])
        else
            AC_MSG_WARN([Merge modules not found in $msmdir])
            msmdir=""
        fi
    else
        msms="Microsoft_VC${VCVER}_CRT_x86.msm"
    fi
}

find_msvc_x64_dlls()
{
    msvcdllpath="$VC_PRODUCT_DIR/redist/x64/Microsoft.VC${VCVER}.CRT"
    msvcdlls="msvcp${VCVER}.dll msvcr${VCVER}.dll"
    for dll in $msvcdlls; do
        if ! test -f "$msvcdllpath/$dll"; then
            AC_MSG_ERROR([can not find $dll in $msvcdllpath])
        fi
    done
}

if test "$build_os" = "cygwin"; then
    dnl Check midl.exe
    AC_MSG_CHECKING([for midl.exe])

    find_winsdk
    if test -f "$winsdktest/Bin/midl.exe"; then
        MIDL_PATH="$winsdktest/Bin"
    elif test -f "$winsdktest/Bin/$WINDOWS_SDK_ARCH/midl.exe"; then
        MIDL_PATH="$winsdktest/Bin/$WINDOWS_SDK_ARCH"
    fi
    if test ! -f "$MIDL_PATH/midl.exe"; then
        AC_MSG_ERROR([midl.exe not found in $winsdktest/Bin/$WINDOWS_SDK_ARCH, Windows SDK installation broken?])
    else
        AC_MSG_RESULT([$MIDL_PATH/midl.exe])
    fi

    # Convert to posix path with 8.3 filename restrictions ( No spaces )
    MIDL_PATH=`cygpath -d "$MIDL_PATH"`
    MIDL_PATH=`cygpath -u "$MIDL_PATH"`

    dnl Check csc.exe
    AC_MSG_CHECKING([for csc.exe])
    find_csc
    if test -f "$csctest/csc.exe"; then
        CSC_PATH="$csctest"
    fi
    if test ! -f "$CSC_PATH/csc.exe"; then
        AC_MSG_ERROR([csc.exe not found as $CSC_PATH/csc.exe])
    else
        AC_MSG_RESULT([$CSC_PATH/csc.exe])
    fi

    CSC_PATH=`cygpath -d "$CSC_PATH"`
    CSC_PATH=`cygpath -u "$CSC_PATH"`

    dnl Check al.exe
    AC_MSG_CHECKING([for al.exe])
    find_winsdk
    if test -f "$winsdktest/Bin/al.exe"; then
        AL_PATH="$winsdktest/Bin"
    elif test -f "$winsdktest/Bin/$WINDOWS_SDK_ARCH/al.exe"; then
        AL_PATH="$winsdktest/Bin/$WINDOWS_SDK_ARCH"
    fi

    if test -z "$AL_PATH"; then
        find_al
        if test -f "$altest/bin/al.exe"; then
            AL_PATH="$altest/bin"
        elif test -f "$altest/al.exe"; then
            AL_PATH="$altest"
        fi
    fi
    if test ! -f "$AL_PATH/al.exe"; then
        AC_MSG_ERROR([al.exe not found as $AL_PATH/al.exe])
    else
        AC_MSG_RESULT([$AL_PATH/al.exe])
    fi

    AL_PATH=`cygpath -d "$AL_PATH"`
    AL_PATH=`cygpath -u "$AL_PATH"`

    dnl Check mscoree.lib / .NET Framework dir
    AC_MSG_CHECKING(.NET Framework)
    find_dotnetsdk
    if test -f "$frametest/lib/mscoree.lib"; then
        DOTNET_FRAMEWORK_HOME="$frametest"
    else
        find_winsdk
        if test -f "$winsdktest/lib/mscoree.lib" -o -f "$winsdktest/lib/win8/um/$WINDOWS_SDK_ARCH/mscoree.lib"; then
            DOTNET_FRAMEWORK_HOME="$winsdktest"
        fi
    fi

    if test ! -f "$DOTNET_FRAMEWORK_HOME/lib/mscoree.lib" -a ! -f "$DOTNET_FRAMEWORK_HOME/lib/win8/um/$WINDOWS_SDK_ARCH/mscoree.lib"; then
        AC_MSG_ERROR([mscoree.lib (.NET Framework) not found])
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

    AC_MSG_RESULT([checked (g++ $_gpp_version)])

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
    dnl see https://code.google.com/p/android/issues/detail?id=41770
    if test "$_gpp_majmin" -ge "407"; then
            glibcxx_threads=no
            AC_LANG_PUSH([C++])
            AC_REQUIRE_CPP
            AC_MSG_CHECKING([whether $CXX is broken with boost.thread])
            AC_PREPROC_IFELSE([AC_LANG_PROGRAM([[
                #include <bits/c++config.h>]],[[
                #if !defined(_GLIBCXX_HAVE_GTHR_DEFAULT) \
                && !defined(_GLIBCXX__PTHREADS) \
                && !defined(_GLIBCXX_HAS_GTHREADS)
                choke me
                #endif
            ]])],[AC_MSG_RESULT([yes])
            glibcxx_threads=yes],[AC_MSG_RESULT([no])])
            AC_LANG_POP([C++])
            if test $glibcxx_threads = yes; then
                  BOOST_CXXFLAGS="-D_GLIBCXX_HAS_GTHREADS"
            fi
     fi
fi
AC_SUBST(BOOST_CXXFLAGS)

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

    SAL_TYPES_SIZEOFSHORT=$ac_cv_sizeof_short
    SAL_TYPES_SIZEOFINT=$ac_cv_sizeof_int
    SAL_TYPES_SIZEOFLONG=$ac_cv_sizeof_long
    SAL_TYPES_SIZEOFLONGLONG=$ac_cv_sizeof_long_long
    SAL_TYPES_SIZEOFPOINTER=$ac_cv_sizeof_voidp

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
            Linux-i686)
                test -z "$ac_cv_alignof_short" && ac_cv_alignof_short=2
                test -z "$ac_cv_alignof_int" && ac_cv_alignof_int=4
                test -z "$ac_cv_alignof_long" && ac_cv_alignof_long=4
                test -z "$ac_cv_alignof_double" && ac_cv_alignof_double=4
                ;;
            Linux-x86_64)
                test -z "$ac_cv_alignof_short" && ac_cv_alignof_short=2
                test -z "$ac_cv_alignof_int" && ac_cv_alignof_int=4
                test -z "$ac_cv_alignof_long" && ac_cv_alignof_long=8
                test -z "$ac_cv_alignof_double" && ac_cv_alignof_double=8
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

    SAL_TYPES_ALIGNMENT2=$ac_cv_alignof_short
    SAL_TYPES_ALIGNMENT4=$ac_cv_alignof_int
    if test $ac_cv_sizeof_long -eq 8; then
        SAL_TYPES_ALIGNMENT8=$ac_cv_alignof_long
    elif test $ac_cv_sizeof_double -eq 8; then
        SAL_TYPES_ALIGNMENT8=$ac_cv_alignof_double
    else
        AC_MSG_ERROR([Cannot find alignment of 8 byte types.])
    fi

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
    SAL_TYPES_SIZEOFSHORT=2
    SAL_TYPES_SIZEOFINT=4
    SAL_TYPES_SIZEOFLONG=4
    SAL_TYPES_SIZEOFLONGLONG=8
    if test "$BITNESS_OVERRIDE" = ""; then
        SAL_TYPES_SIZEOFPOINTER=4
    else
        SAL_TYPES_SIZEOFPOINTER=8
    fi
    SAL_TYPES_ALIGNMENT2=2
    SAL_TYPES_ALIGNMENT4=4
    SAL_TYPES_ALIGNMENT8=8
    WORDS_BIGENDIAN=no
    LFS_CFLAGS=''
fi
AC_SUBST(WORDS_BIGENDIAN)
AC_SUBST(LFS_CFLAGS)

AC_DEFINE_UNQUOTED(SAL_TYPES_SIZEOFSHORT,$SAL_TYPES_SIZEOFSHORT)
AC_DEFINE_UNQUOTED(SAL_TYPES_SIZEOFINT,$SAL_TYPES_SIZEOFINT)
AC_DEFINE_UNQUOTED(SAL_TYPES_SIZEOFLONG,$SAL_TYPES_SIZEOFLONG)
AC_DEFINE_UNQUOTED(SAL_TYPES_SIZEOFLONGLONG,$SAL_TYPES_SIZEOFLONGLONG)
AC_DEFINE_UNQUOTED(SAL_TYPES_SIZEOFPOINTER,$SAL_TYPES_SIZEOFPOINTER)
AC_DEFINE_UNQUOTED(SAL_TYPES_ALIGNMENT2,$SAL_TYPES_ALIGNMENT2)
AC_DEFINE_UNQUOTED(SAL_TYPES_ALIGNMENT4,$SAL_TYPES_ALIGNMENT4)
AC_DEFINE_UNQUOTED(SAL_TYPES_ALIGNMENT8,$SAL_TYPES_ALIGNMENT8)

dnl ===================================================================
dnl Check if valgrind headers are available
dnl ===================================================================
if test "$cross_compiling" = yes; then
    ENABLE_VALGRIND=FALSE
else
    prev_cppflags=$CPPFLAGS
    CPPFLAGS="$CPPFLAGS $VALGRIND_CFLAGS"
    AC_CHECK_HEADER([valgrind/valgrind.h],
        [ENABLE_VALGRIND=TRUE],[ENABLE_VALGRIND=FALSE])
    CPPFLAGS=$prev_cppflags
fi
AC_SUBST([ENABLE_VALGRIND])
if test "$ENABLE_VALGRIND" = FALSE; then
    VALGRIND_CFLAGS=
fi
AC_SUBST([VALGRIND_CFLAGS])


dnl ===================================================================
dnl Check if SDT probes (for systemtap, gdb, dtrace) are available
dnl ===================================================================

# We need at least the sys/sdt.h include header.
AC_CHECK_HEADER([sys/sdt.h], [SDT_H_FOUND='TRUE'], [SDT_H_FOUND='FALSE'])
if test "$SDT_H_FOUND" = "TRUE"; then
  # Found sys/sdt.h header, now make sure the c++ compiler works.
  # Old g++ versions had problems with probes in constructors/destructors.
  AC_MSG_CHECKING([working sys/sdt.h and c++ support])
  AC_LANG_PUSH([C++])
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[
    #include <sys/sdt.h>
    class ProbeClass
    {
    private:
      int& ref;
      const char *name;

    public:
      ProbeClass(int& v, const char *n) : ref(v), name(n)
      {
        DTRACE_PROBE2(_test_, cons, name, ref);
      }

      void method(int min)
      {
        DTRACE_PROBE3(_test_, meth, name, ref, min);
        ref -= min;
      }

      ~ProbeClass()
      {
        DTRACE_PROBE2(_test_, dest, name, ref);
      }
    };
  ]],[[
    int i = 64;
    DTRACE_PROBE1(_test_, call, i);
    ProbeClass inst = ProbeClass(i, "call");
    inst.method(24);
  ]])], [AC_MSG_RESULT([yes]); AC_DEFINE([USE_SDT_PROBES])],
        [AC_MSG_RESULT([no, sdt.h or c++ compiler too old])])
  AC_LANG_POP([C++])
fi
AC_CONFIG_HEADERS([config_host/config_probes.h])

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
        dnl The prefix where Clang resides, override to where Clang resides if
        dnl using a source build:
        if test -z "$CLANGDIR"; then
            CLANGDIR=/usr
        fi
        dnl The build directory (different from CLANGDIR if using a Clang out-
        dnl of-source build):
        if test -z "$CLANGBUILD"; then
            CLANGBUILD=/usr
        fi
        AC_LANG_PUSH([C++])
        save_CPPFLAGS=$CPPFLAGS
        CPPFLAGS="$CPPFLAGS -I$CLANGDIR/include -I$CLANGDIR/tools/clang/include -I$CLANGBUILD/include -I$CLANGBUILD/tools/clang/include -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS"
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
AC_SUBST(CLANGDIR)
AC_SUBST(CLANGBUILD)

dnl ===================================================================
dnl Set the MinGW sys-root
dnl ===================================================================
if test "$WITH_MINGW" = "yes"; then
    AC_MSG_CHECKING([for MinGW sysroot])
    sysroot=`$CC -print-sysroot`
    AS_IF([test -d "$sysroot"/mingw],
          [MINGW_SYSROOT="$sysroot"/mingw
           AC_MSG_RESULT([$MINGW_SYSROOT])],
          [AC_MSG_RESULT([not found])
           AC_MSG_ERROR([cannot determine MinGW sysroot])])
fi
AC_SUBST([MINGW_DLLS])
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
                if test "$CROSS_COMPILING" = "YES"; then
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

    AC_SUBST(MINGW_SHARED_GCCLIB)
    AC_SUBST(MINGW_SHARED_GXXLIB)
    AC_SUBST(MINGW_SHARED_LIBSTDCPP)
    AC_SUBST(MINGW_GCCDLL)
    AC_SUBST(MINGW_GXXDLL)
fi

if test "$WITH_MINGW" = "yes"; then
    AC_MSG_CHECKING([for fMergeNeutralItems in SCRIPT_CONTROL])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
    [
    #include <usp10.h>
    ],
    [
    SCRIPT_CONTROL c;
    c.fMergeNeutralItems = 1;
    ])],
    [
    AC_MSG_RESULT(yes)
    AC_DEFINE(HAVE_FMERGENEUTRALITEMS)
    ],
    [AC_MSG_RESULT(no)])
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
    dnl it working in GCC >= 4.3, so conservative way to check here is to use
    dnl GCC_VERSION for GCC but resort to __GLIBCXX__ corresponding to libstdc++
    dnl shipped with GCC 4.3.0 (cf. <http://gcc.gnu.org/onlinedocs/libstdc++/
    dnl manual/abi.html#abi.versioning.history>; 4.3.0 is 20080306, 4.2.4 is
    dnl 20080519, 4.3.1 is 20080606) for Clang (for which GCC_VERSION is notoriously
    dnl "too old"):
    if test "$_os" != Darwin -a "$_os" != Android; then
        if test "$COM_GCC_IS_CLANG" = TRUE; then
            AC_LANG_PUSH([C++])
            AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <list>
#if !defined __GLIBCXX__ ||(__GLIBCXX__ < 20080606 && __GLIBCXX__ != 20080306)
#error
#endif
                ]])],[HAVE_THREADSAFE_STATICS=TRUE],[])
            AC_LANG_POP([C++])
        elif test "${GCC_VERSION?}" -ge 0403; then
            HAVE_THREADSAFE_STATICS=TRUE
        fi
    fi
    if test "$HAVE_THREADSAFE_STATICS" = TRUE; then
        AC_DEFINE(HAVE_THREADSAFE_STATICS)
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([broken (i.e., no)])
    fi
else
    AC_MSG_RESULT([unknown (assuming no)])
fi
AC_SUBST(HAVE_THREADSAFE_STATICS)

dnl ===================================================================
dnl visibility and other gcc features
dnl ===================================================================
if test "$GCC" = "yes"; then
    AC_MSG_CHECKING([whether $CC supports -fvisibility=hidden])
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -Werror -fvisibility=hidden"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[ return 0; ]])],[ HAVE_GCC_VISIBILITY_FEATURE=TRUE ],[])
    CFLAGS=$save_CFLAGS

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
        AC_DEFINE(HAVE_GCC_BUILTIN_ATOMIC)
    else
        AC_MSG_RESULT([no])
    fi

    AC_MSG_CHECKING(
        [whether $CC supports pragma GCC diagnostic error/ignored/warning])
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -Werror -Wunknown-pragmas -Wunused-parameter"
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            #pragma GCC diagnostic ignored "-Wunused-parameter"
            void dummy(int n) {}
        ])], [
            AC_DEFINE([HAVE_GCC_PRAGMA_DIAGNOSTIC_MODIFY],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    CFLAGS=$save_CFLAGS

    AC_MSG_CHECKING([whether $CC supports pragma GCC diagnostic push/pop])
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -Werror -Wunknown-pragmas -Wunused-parameter"
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wunused-parameter"
            void dummy(int n) {}
            #pragma GCC diagnostic pop
        ])], [
            AC_DEFINE([HAVE_GCC_PRAGMA_DIAGNOSTIC_SCOPE],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    CFLAGS=$save_CFLAGS

    AC_MSG_CHECKING([whether $CC supports __attribute__((deprecated(message)))])
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -Werror"
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            __attribute__((deprecated("test"))) void f();
        ])], [
            AC_DEFINE([HAVE_GCC_DEPRECATED_MESSAGE],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    CFLAGS=$save_CFLAGS

    AC_MSG_CHECKING([whether $CXX declares __cxa_allocate_exception in cxxabi.h])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            #include <cxxabi.h>
            void * f() { return __cxxabiv1::__cxa_allocate_exception(0); }
        ])], [
            AC_DEFINE([HAVE_GCC_CXXABI_H_CXA_ALLOCATE_EXCEPTION],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    AC_LANG_POP([C++])

    AC_MSG_CHECKING([whether $CXX declares __cxa_get_globals in cxxabi.h])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            #include <cxxabi.h>
            void * f() { return __cxxabiv1::__cxa_get_globals(); }
        ])], [
            AC_DEFINE([HAVE_GCC_CXXABI_H_CXA_GET_GLOBALS],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    AC_LANG_POP([C++])

    AC_MSG_CHECKING([whether $CXX declares __cxa_throw in cxxabi.h])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            #include <cxxabi.h>
            void f() { __cxxabiv1::__cxa_throw(0, 0, 0); }
        ])], [
            AC_DEFINE([HAVE_GCC_CXXABI_H_CXA_THROW],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    AC_LANG_POP([C++])

    AC_MSG_CHECKING([whether $CXX supports __attribute__((warn_unused))])
    AC_LANG_PUSH([C++])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CFLAGS -Werror -Wunknown-pragmas"
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            struct __attribute__((warn_unused)) dummy {};
        ])], [
            AC_DEFINE([HAVE_GCC_ATTRIBUTE_WARN_UNUSED],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    CXXFLAGS=$save_CXXFLAGS
    AC_LANG_POP([C++])

    AC_MSG_CHECKING([whether STL uses __attribute__((warn_unused))])
    AC_LANG_PUSH([C++])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CFLAGS -Werror -Wunused"
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            #include <string>
            void f() { std::string s; }
        ])], [
            AC_MSG_RESULT([no])
        ], [
            AC_DEFINE([HAVE_GCC_ATTRIBUTE_WARN_UNUSED_STL],[1])
            AC_MSG_RESULT([yes])])
    CXXFLAGS=$save_CXXFLAGS
    AC_LANG_POP([C++])
fi

AC_SUBST(HAVE_GCC_NO_LONG_DOUBLE)
AC_SUBST(HAVE_GCC_AVX)
AC_SUBST(HAVE_GCC_BUILTIN_ATOMIC)

dnl ===================================================================
dnl C++11
dnl ===================================================================

CXXFLAGS_CXX11=
HAVE_CXX11=
if test "$COM" = MSC; then
    AC_MSG_CHECKING([whether $CXX supports C++11])
    AC_MSG_RESULT(yes)
    # MSVC supports (a subset of) CXX11 without any switch
    HAVE_CXX11=TRUE
    CXXFLAGS_CXX11=
elif test "$GCC" = "yes"; then
    CXXFLAGS_CXX11=
    AC_MSG_CHECKING([whether $CXX supports C++11])
    for flag in -std=gnu++11 -std=gnu++0x -std=c++11 -std=c++0x ; do
        save_CXXFLAGS=$CXXFLAGS
        CXXFLAGS="$CXXFLAGS $flag -Werror"
        AC_LANG_PUSH([C++])
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[void f() {}]])],[CXXFLAGS_CXX11=$flag])
        AC_LANG_POP([C++])
        CXXFLAGS=$save_CXXFLAGS
        if test -n "$CXXFLAGS_CXX11"; then
            HAVE_CXX11=TRUE
            break
        fi
    done
    if test "$HAVE_CXX11" = TRUE; then
        AC_MSG_RESULT([yes ($CXXFLAGS_CXX11)])
    else
        AC_MSG_RESULT(no)
    fi
fi

if test "$HAVE_CXX11" = TRUE; then
    AC_MSG_CHECKING([whether using C++11 causes libstdc++ 4.7.0/4.7.1 ABI breakage])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <list>
#if !defined(__GLIBCXX__)
/* ok */
#elif __GLIBCXX__ < 20111004
/* before breakage */
#elif __GLIBCXX__ > 20120703
/* after breakage */
#else
abi broken
#endif
        ]])], [AC_MSG_RESULT(no, ok)],
        [AC_MSG_RESULT(yes, disabling C++11)
         HAVE_CXX11=])
    AC_LANG_POP([C++])
fi

if test "$HAVE_CXX11" = TRUE; then
    AC_MSG_CHECKING([whether $CXX supports C++11 without Language Defect 757])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS $CXXFLAGS_CXX11"
    AC_LANG_PUSH([C++])

    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <stddef.h>

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
]])
        ], [ AC_MSG_RESULT(yes) ],
        [ AC_MSG_RESULT(no)
          HAVE_CXX11=
        ])
    AC_LANG_POP([C++])
    CXXFLAGS=$save_CXXFLAGS
fi

if test "$HAVE_CXX11" = TRUE; then
    AC_MSG_CHECKING([whether $CXX supports C++11 without __float128 compile error])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS $CXXFLAGS_CXX11"
    AC_LANG_PUSH([C++])

    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <vector>
    // some Clang fail when compiling against libstdc++ headers with -std=gnu++0x
    // (__float128)
]])
    ],[ AC_MSG_RESULT(yes) ],
    [
        AC_MSG_RESULT(no)
        # The only reason why libstdc++ headers fail with Clang in C++11 mode is because
        # they use the __float128 type that Clang doesn't know (libstdc++ checks whether
        # __float128 is available during its build, but it's usually built using GCC,
        # and so c++config.h hardcodes __float128 being supported). As the only place
        # where __float128 is actually used is in a template specialization,
        # -D__float128=void will avoid the problem there while still causing a problem
        # if somebody actually uses the type.
        AC_MSG_CHECKING([whether -D__float128=void workaround helps])
        CXXFLAGS="$CXXFLAGS -D__float128=void"
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <vector>
    // some Clang fail when compiling against libstdc++ headers with -std=gnu++0x
    // (__float128)
]])
        ],
        [
         AC_MSG_RESULT(yes)
         CXXFLAGS_CXX11="$CXXFLAGS_CXX11 -D__float128=void"
        ],
        [
         AC_MSG_RESULT(no)
         HAVE_CXX11=
        ])
    ])

    AC_LANG_POP([C++])
    CXXFLAGS=$save_CXXFLAGS
fi

if test "$HAVE_CXX11" = "TRUE"; then
    AC_DEFINE(HAVE_CXX11)
elif test -n "$CXXFLAGS_CXX11"; then
    AC_MSG_NOTICE([Disabling C++11 support])
    CXXFLAGS_CXX11=
elif test "$CPP_LIBRARY" = LIBCPP -a $_os = Darwin; then
    AC_MSG_ERROR([Selected libc++ but C++11 support broken])
fi

AC_SUBST(CXXFLAGS_CXX11)
AC_SUBST(HAVE_CXX11)

dnl ==================================
dnl Check for C++11 "= delete" support
dnl ==================================

if test "$HAVE_CXX11" = "TRUE"; then
    AC_MSG_CHECKING([whether $CXX supports C++11 = delete syntax])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS $CXXFLAGS_CXX11"
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
struct A
    {
    void test() = delete;
    };
]])],[HAVE_CXX11_DELETE=TRUE],[])

    AC_LANG_POP([C++])
    CXXFLAGS=$save_CXXFLAGS
    if test "$HAVE_CXX11_DELETE" = "TRUE"; then
        AC_MSG_RESULT([yes])
        AC_DEFINE([HAVE_CXX11_DELETE])
    else
        AC_MSG_RESULT([no])
    fi
fi

dnl ==================================
dnl Check for C++11 "override" support
dnl ==================================

if test "$HAVE_CXX11" = "TRUE"; then
    AC_MSG_CHECKING([whether $CXX supports C++11 "override" syntax])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS $CXXFLAGS_CXX11"
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
struct A
{
        virtual void test();
};

struct B : A
{
        virtual void test() override;
};
]])],[HAVE_CXX11_OVERRIDE=TRUE],[])

    AC_LANG_POP([C++])
    CXXFLAGS=$save_CXXFLAGS
    if test "$HAVE_CXX11_OVERRIDE" = "TRUE"; then
        AC_MSG_RESULT([yes])
        AC_DEFINE([HAVE_CXX11_OVERRIDE])
    else
        AC_MSG_RESULT([no])
    fi
fi

dnl ==================================
dnl Check for C++11 "final" support
dnl ==================================

if test "$HAVE_CXX11" = "TRUE"; then
    AC_MSG_CHECKING([whether $CXX supports C++11 "final" syntax])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS $CXXFLAGS_CXX11"
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
// First check that this correct program that uses "final" compiles
struct A final
{
};

struct B
{
        virtual void test();
};

struct C : B
{
        void test() final;
};
]])],[have_final=yes],[])

    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
// Then check that the "final" works as expected,
// that this program fails to compile
struct A final
{
};

struct B : A
{
};
]])],[],[final_class_works=yes])

    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
// Also this should fail to compile
struct B
{
        virtual void test();
};

struct C : B
{
        void test() final;
};

struct D : C
{
        void test();
};
]])],[],[final_method_works=yes])
    AC_LANG_POP([C++])

    CXXFLAGS=$save_CXXFLAGS

    if test "$have_final" = yes -a "$final_class_works" = yes -a "$final_method_works" = yes; then
        AC_MSG_RESULT([yes])
        AC_DEFINE([HAVE_CXX11_FINAL])
    else
        AC_MSG_RESULT([no])
    fi
fi

dnl ===================================================================
dnl Check for C++11 perfect forwarding support
dnl ===================================================================
if test "$HAVE_CXX11" = "TRUE"; then
    AC_MSG_CHECKING([whether $CXX supports C++11 perfect forwarding])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS $CXXFLAGS_CXX11"
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
        #include <utility>
        template<typename T, typename... Args> T * f(Args &&... v) {
            return new T(std::forward<Args>(v)...);
        }
        ]], [[
        f<int>(0);
        ]])], [perfect_forwarding=yes], [perfect_forwarding=no])
    AC_LANG_POP([C++])
    CXXFLAGS=$save_CXXFLAGS
    AC_MSG_RESULT([$perfect_forwarding])
    if test "$perfect_forwarding" = yes; then
        AC_DEFINE([HAVE_CXX11_PERFECT_FORWARDING])
    fi
fi

HAVE_GCC_PRAGMA_OPERATOR=
dnl _Pragma support (may require C++11)
if test "$GCC" = "yes"; then
    AC_MSG_CHECKING([whether $CXX supports _Pragma operator])
    AC_LANG_PUSH([C++])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS $CXXFLAGS_CXX11 -Werror"
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            _Pragma("GCC diagnostic ignored \"-Wformat\"")
        ])], [
            AC_DEFINE([HAVE_GCC_PRAGMA_OPERATOR],[1])
            HAVE_GCC_PRAGMA_OPERATOR=1
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    AC_LANG_POP([C++])
    CXXFLAGS=$save_CXXFLAGS
fi
AC_SUBST(HAVE_GCC_PRAGMA_OPERATOR)

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

    # As the below test checks things when linking self-compiled dynamic libraries, it presumably is irrelevant
    # when we don't make any dynamic libraries?
    if test "$DISABLE_DYNLOADING" = "" -a "$HAVE_GCC_VISIBILITY_FEATURE" = "TRUE" -a "$HAVE_GCC_VISIBILITY_BROKEN" != "TRUE"; then
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
                if test "$_os" = "Darwin" -o "$WITH_MINGW" = "yes"; then
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

JITC_PROCESSOR_TYPE=""
if test "$_os" = "Linux" -a "$host_cpu" = "powerpc"; then
    # IBMs JDK needs this...
    JITC_PROCESSOR_TYPE=6
    export JITC_PROCESSOR_TYPE
fi
AC_SUBST([JITC_PROCESSOR_TYPE])

# Misc Windows Stuff
if test $_os = "WINNT" -a "$WITH_MINGW" != yes; then
    find_msvc_x64_dlls
    find_msms
    MSVC_DLL_PATH="$msvcdllpath"
    MSVC_DLLS="$msvcdlls"
    MSM_PATH="$msmdir"
    MERGE_MODULES="$msms"
    SCPDEFS="$SCPDEFS -DWITH_VC${VCVER}_REDIST"
fi

AC_SUBST(MSVC_DLL_PATH)
AC_SUBST(MSVC_DLLS)
AC_SUBST(MSM_PATH)
AC_SUBST(MERGE_MODULES)

dnl ===================================================================
dnl Checks for Java
dnl ===================================================================
if test "$ENABLE_JAVA" != ""; then

    # Windows-specific tests
    if test "$build_os" = "cygwin"; then
        if test "$BITNESS_OVERRIDE" = 64; then
            bitness=64
        else
            bitness=32
        fi

        if test -z "$with_jdk_home"; then
            for ver in 1.7 1.6; do
               reg_get_value_$bitness "HKEY_LOCAL_MACHINE/SOFTWARE/JavaSoft/Java Development Kit/$ver/JavaHome"
               if test -n "$regvalue"; then
                   _jdk_home=$regvalue
                   break
               fi
            done
            if test -f "$_jdk_home/lib/jvm.lib" -a -f "$_jdk_home/bin/java.exe"; then
                with_jdk_home="$_jdk_home"
                howfound="found automatically"
            else
                AC_MSG_ERROR([No JDK found, pass the --with-jdk-home option pointing to a $bitness-bit JDK])
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

    if test "$build_os" = "cygwin"; then
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
    elif test $_os = Darwin -a "$BITNESS_OVERRIDE" = ""; then
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

if test "$ENABLE_JAVA" != ""; then
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
            if test "$_jdk_ver" -gt 10600; then
                JAVA_CLASSPATH_NOT_SET="1"
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
dnl Set target Java bytecode version
dnl ===================================================================
if test "$ENABLE_JAVA" != ""; then
    _java_target_ver="1.5"

    JAVA_SOURCE_VER="$_java_target_ver"
    JAVA_TARGET_VER="$_java_target_ver"
fi

dnl ===================================================================
dnl Checks for javac
dnl ===================================================================
if test "$ENABLE_JAVA" != ""; then
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
    if test "$build_os" = "cygwin"; then
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
if test "$ENABLE_JAVA" != ""; then
    if test `$JAVACOMPILER --version 2>&1 | $GREP -c "GCC"` -gt 0; then
        JAVACISGCJ="yes"
    fi
fi
AC_SUBST(JAVACISGCJ)

dnl ===================================================================
dnl Checks for javadoc
dnl ===================================================================
if test "$ENABLE_JAVA" != ""; then
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
    if test "$build_os" = "cygwin"; then
        if test x`echo "$JAVADOC" | $GREP -i '\.exe$'` = x; then
            JAVADOC="${JAVADOC}.exe"
        fi
        JAVADOC=`cygpath -d "$JAVADOC"`
        JAVADOC=`cygpath -u "$JAVADOC"`
    fi

    if test `$JAVADOC --version 2>&1 | $GREP -c "gjdoc"` -gt 0; then
    JAVADOCISGJDOC="yes"
    fi
fi
AC_SUBST(JAVADOCISGJDOC)

if test "$ENABLE_JAVA" != ""; then
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

if test "$ENABLE_JAVA" != ""; then
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


if test -n "$ENABLE_JAVA" -a \( -z "$JAVALIB" -o -z "$JAVAINC" \); then
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
        JAVAINC="-I$JAVA_HOME/include"
        JAVAINC="$JAVAINC -I$JAVA_HOME/include/aix"
        test -d "$JAVA_HOME/include/native_thread" && JAVAINC="$JAVAINC -I$JAVA_HOME/include/native_thread"

        case "$JDK" in
        [Ii][Bb][Mm]*)
            JAVA_ARCH=
            JRE_BASE_DIR="$JAVA_HOME/jre/bin"
            JAVALIB="-L$JRE_BASE_DIR -L$JRE_BASE_DIR/classic"
            ;;
        *)
            ;;
        esac

        ;;

    cygwin*)
        x_JDKLIB=
        JDKLIB="$JAVA_HOME/lib"
        JAVAINC="-I$JAVA_HOME/include/win32"
        JAVAINC="$JAVAINC -I$JAVA_HOME/include"
        ;;

    darwin*)
        x_JAVALIB=[\#]
        JAVAINC="${ISYSTEM}$FRAMEWORKSHOME/JavaVM.framework/Versions/Current/Headers ${ISYSTEM}$FRAMEWORKSHOME/JavaVM/Headers"
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
        JAVAINC="-I$JAVA_HOME/include"
        test -d "$JAVA_HOME/include/native_thread" && JAVAINC="$JAVAINC -I$JAVA_HOME/include/native_thread"
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
        JAVAINC="-I$JAVA_HOME/include"
        JAVAINC="$JAVAINC -I$JAVA_HOME/include/freebsd"
        JAVAINC="$JAVAINC -I$JAVA_HOME/include/bsd"
        JAVAINC="$JAVAINC -I$JAVA_HOME/include/linux"
        test -d "$JAVA_HOME/include/native_thread" && JAVAINC="$JAVAINC -I$JAVA_HOME/include/native_thread"
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
        JAVAINC="-I$JAVA_HOME/include"
        test -d "$JAVA_HOME/include/native_thread" && JAVAINC="$JAVAINC -I$JAVA_HOME/include/native_thread"
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
        JAVAINC="-I$JAVA_HOME/include"
        JAVAINC="$JAVAINC -I$JAVA_HOME/include/linux"
        test -d "$JAVA_HOME/include/native_thread" && JAVAINC="$JAVAINC -I$JAVA_HOME/include/native_thread"
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
        JAVAINC="-I$JAVA_HOME/include"
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
        JAVAINC="-I$JAVA_HOME/include"
        JAVAINC="$JAVAINC -I$JAVA_HOME/include/netbsd"
        test -d "$JAVA_HOME/include/native_thread" && JAVAINC="$JAVAINC -I$JAVA_HOME/include/native_thread"
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
        JAVAINC="-I$JAVA_HOME/include"
        JAVAINC="$JAVAINC -I$JAVA_HOME/include/openbsd"
        test -d "$JAVA_HOME/include/native_thread" && JAVAINC="$JAVAINC -I$JAVA_HOME/include/native_thread"
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
        JAVAINC="-I$JAVA_HOME/include"
        JAVAINC="$JAVAINC -I$JAVA_HOME/include/solaris"
        test -d "$JAVA_HOME/include/native_thread" && JAVAINC="$JAVAINC -I$JAVA_HOME/include/native_thread"
        ;;
    esac

    if test -n "$JAVA_ARCH"; then
        JRE_BASE_DIR="$JAVA_HOME/jre/lib/$JAVA_ARCH"
        JAVALIB="-L$JAVA_HOME/$LIB64"
        test -d "$JRE_BASE_DIR" && JAVALIB="$JAVALIB -L$JRE_BASE_DIR"
        test -d "$JRE_BASE_DIR/$JAVA_TOOLKIT" && JAVALIB="$JAVALIB -L$JRE_BASE_DIR/$JAVA_TOOLKIT"
        test -d "$JRE_BASE_DIR/native_thread" && JAVALIB="$JAVALIB -L$JRE_BASE_DIR/native_thread"
    fi
elif test -z "$ENABLE_JAVA"; then
    x_JAVALIB=[\#]
    x_JDKLIB=[\#]
fi
SOLARINC="$SOLARINC $JAVAINC"

AC_SUBST(JAVALIB)
AC_SUBST(JAVACOMPILER)
AC_SUBST(JAVADOC)
AC_SUBST(JAVAINTERPRETER)
AC_SUBST(JAVAIFLAGS)
AC_SUBST(JAVA_CLASSPATH_NOT_SET)
AC_SUBST(JAVA_HOME)
AC_SUBST(JAVA_SOURCE_VER)
AC_SUBST(JAVA_TARGET_VER)
AC_SUBST(JDK)
AC_SUBST(JDKLIB)
AC_SUBST(x_JAVALIB)
AC_SUBST(x_JDKLIB)


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
        if test $_os = Darwin; then
            EPM=internal
        elif test -n "$with_epm"; then
            EPM=$with_epm
        else
            AC_PATH_PROG(EPM, epm, no)
        fi
        if test "$EPM" = "no" -o "$EPM" = "internal"; then
            echo "EPM will be built."
            BUILD_TYPE="$BUILD_TYPE EPM"
        else
            # Gentoo has some epm which is something different...
            AC_MSG_CHECKING([whether the found epm is the right epm])
            if $EPM | grep "ESP Package Manager" >/dev/null 2>/dev/null; then
                AC_MSG_RESULT([yes])
            else
                AC_MSG_ERROR([no. Install ESP Package Manager (http://www.msweet.org/projects.php?Z2) and/or specify the path to the right epm])
            fi
            AC_MSG_CHECKING([epm version])
            EPM_VERSION=`$EPM | grep 'ESP Package Manager' | cut -d' ' -f4 | $SED -e s/v//`
            if test "`echo $EPM_VERSION | cut -d'.' -f1`" -gt "3" || \
               test "`echo $EPM_VERSION | cut -d'.' -f1`" -eq "3" -a "`echo $EPM_VERSION | cut -d'.' -f2`" -ge "7"; then
                AC_MSG_RESULT([OK, >= 3.7])
            else
                AC_MSG_RESULT([too old. epm >= 3.7 is required.])
                echo "EPM will be built."
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
            SCPDEFS="$SCPDEFS -DWITH_RPM"
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
    if echo "$PKGFORMAT" | $EGREP osx 2>&1 >/dev/null; then
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

dnl ===================================================================
dnl We need winegcc when building MinGW build to be able to cross-build msi tools
dnl ===================================================================
WINEGCC=
if test "$enable_winegcc" = "yes"; then
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

if test $_os = iOS; then
    enable_mpl_subset=yes
    enable_opengl=no
    enable_lotuswordpro=no
    enable_lpsolve=no
    enable_postgresql_sdbc=no
    enable_lotuswordpro=no
    enable_neon=no
    enable_extension_integration=no
    enable_report_builder=no
    with_theme="tango"
    with_ppds=no
fi

ENABLE_LWP=
if test "$enable_lotuswordpro" = "yes"; then
   ENABLE_LWP="TRUE"
   SCPDEFS="$SCPDEFS -DDISABLE_LWP"
fi
AC_SUBST(ENABLE_LWP)

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
                if test "$enable_odk" = "" ; then
                    enable_odk="no"
                else
                    AC_MSG_ERROR([doxygen not found in \$PATH; specify its pathname via --with-doxygen=..., or disable its use via --without-doxygen])
                fi
            fi
        else
            AC_MSG_CHECKING([for doxygen])
            DOXYGEN=$with_doxygen
            AC_MSG_RESULT([$DOXYGEN])
        fi
        if test -n "$DOXYGEN"; then
           DOXYGEN_VERSION=`$DOXYGEN --version 2>/dev/null`
           DOXYGEN_NUMVERSION=`echo $DOXYGEN_VERSION | $AWK -F. '{ print \$1*10000 + \$2*100 + \$3 }'`
           if ! test "$DOXYGEN_NUMVERSION" -ge "10804" ; then
              AC_MSG_ERROR([found doxygen is too old; need at least version 1.8.4 or specify --without-doxygen])
           fi
        fi
    fi
fi
AC_SUBST([DOXYGEN])

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

dnl we want to use libo_CHECK_SYSTEM_MODULE here too, but OS X is too stupid
dnl and has no pkg-config for it at least on some tinderboxes,
dnl so leaving that out for now
dnl libo_CHECK_SYSTEM_MODULE([zlib],[ZLIB],[zlib])
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
    ZLIB_CFLAGS="-I${WORKDIR}/UnpackedTarball/zlib"
    ZLIB_LIBS="-L${OUTDIR}/lib -lzlib"
fi
AC_SUBST(ZLIB_CFLAGS)
AC_SUBST(ZLIB_LIBS)
AC_SUBST(SYSTEM_ZLIB)

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
    libo_MINGW_CHECK_DLL([libjpeg])
else
    AC_MSG_RESULT([internal])
    SYSTEM_JPEG=NO
    BUILD_TYPE="$BUILD_TYPE JPEG"
fi
AC_SUBST(SYSTEM_JPEG)


dnl ===================================================================
dnl Check for system clucene
dnl ===================================================================
dnl we should rather be using
dnl libo_CHECK_SYSTEM_MODULE([clucence],[CLUCENE],[liblucence-core]) here
dnl but the contribs-lib check seems tricky
AC_MSG_CHECKING([which clucene to use])
if test "$with_system_clucene" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_CLUCENE=YES
    PKG_CHECK_MODULES(CLUCENE, libclucene-core)
    CLUCENE_CFLAGS=[$(printf '%s' "$CLUCENE_CFLAGS" | sed -e 's@-I[^ ]*/CLucene/ext@@' -e "s/-I/${ISYSTEM?}/g")]
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
    BUILD_TYPE="$BUILD_TYPE CLUCENE"
fi
AC_SUBST(SYSTEM_CLUCENE)
AC_SUBST(CLUCENE_CFLAGS)
AC_SUBST(CLUCENE_LIBS)

dnl ===================================================================
dnl Check for system expat
dnl ===================================================================
dnl we should use libo_CHECK_SYSTEM_MODULE, but e.g. the mingw tinderbox has no
dnl proper pkg-config for it. It should really be fixed on the tinderbox though.
AC_MSG_CHECKING([which expat to use])
if test "$with_system_expat" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_EXPAT=YES
    AC_CHECK_HEADER(expat.h, [],
        [AC_MSG_ERROR(expat.h not found. install expat)], [])
    AC_CHECK_LIB([expat], [XML_ParserCreate], [:],
        [AC_MSG_RESULT(expat library not found or functional.)], [])
    libo_MINGW_CHECK_DLL([libexpat])
else
    AC_MSG_RESULT([internal])
    SYSTEM_EXPAT=NO
    BUILD_TYPE="$BUILD_TYPE EXPAT"
fi
AC_SUBST(SYSTEM_EXPAT)

dnl ===================================================================
dnl Check for system libodfgen
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libodfgen],[ODFGEN],[libodfgen-0.0])

dnl ===================================================================
dnl Check for system libcdr
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libcdr],[CDR],[libcdr-0.0 >= 0.0.5])

dnl ===================================================================
dnl Check for system libmspub
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libmspub],[MSPUB],[libmspub-0.0])

dnl ===================================================================
dnl Check for system libmwaw
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libmwaw],[MWAW],[libmwaw-0.1])

dnl ===================================================================
dnl Check for system libvisio
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libvisio],[VISIO],[libvisio-0.0])

dnl ===================================================================
dnl Check for system libcmis
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libcmis],[CMIS],[libcmis-0.4 >= 0.4.0])

dnl ===================================================================
dnl Check for system libwpd
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libwpd],[WPD],[libwpd-0.9 >= 0.9.5 libwpd-stream-0.9 >= 0.9.5],["-I${WORKDIR}/UnpackedTarball/libwpd/inc"],["-L${OUTDIR}/lib -lwpdlib"])

dnl ===================================================================
dnl Check for system lcms2
dnl ===================================================================
if test "$with_system_lcms2" = "yes"; then
    libo_MINGW_CHECK_DLL([liblcms2])
else
    SYSTEM_LCMS2=NO
fi
libo_CHECK_SYSTEM_MODULE([lcms2],[LCMS2],[lcms2],["-I${WORKDIR}/UnpackedTarball/lcms2/include"],["-L${OUTDIR}/lib -llcms2"])
if test "$GCC" = "yes"; then
    LCMS2_CFLAGS="${LCMS2_CFLAGS} -Wno-long-long"
fi

dnl ===================================================================
dnl Check for system cppunit
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([cppunit],[CPPUNIT],[cppunit >= 1.12.0])

dnl ===================================================================
dnl Check whether freetype is available
dnl ===================================================================
if test  "$test_freetype" = "yes"; then
    AC_MSG_CHECKING([whether freetype is available])
    # FreeType has 3 different kinds of versions
    # * release, like 2.4.10
    # * libtool, like 13.0.7 (this what pkg-config returns)
    # * soname
    # FreeType's docs/VERSION.DLL provides a table mapping between the three
    #
    # 9.9.3 is 2.2.0
    PKG_CHECK_MODULES(FREETYPE, freetype2 >= 9.9.3)
    FREETYPE_CFLAGS=$(printf '%s' "$FREETYPE_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
    SYSTEM_FREETYPE=YES
    _save_libs="$LIBS"
    _save_cflags="$CFLAGS"
    LIBS="$LIBS $FREETYPE_LIBS"
    CFLAGS="$CFLAGS $FREETYPE_CFLAGS"
    AC_CHECK_FUNC(FT_Face_GetCharVariantIndex, AC_DEFINE(HAVE_FT_FACE_GETCHARVARIANTINDEX), [])
    LIBS="$_save_libs"
    CFLAGS="$_save_cflags"
fi
AC_SUBST(FREETYPE_CFLAGS)
AC_SUBST(FREETYPE_LIBS)
AC_SUBST([SYSTEM_FREETYPE])

dnl ===================================================================
dnl Check for system libwps
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libwps],[WPS],[libwps-0.2],["-I${WORKDIR}/UnpackedTarball/libwps/inc"],["-L${OUTDIR}/lib -lwpslib"])

dnl ===================================================================
dnl Check for system libwpg
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libwpg],[WPG],[libwpg-0.2],["-I${WORKDIR}/UnpackedTarball/libwpg/inc"],["-L${OUTDIR}/lib -lwpglib"])

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
        LIBXSLT_CFLAGS=$(printf '%s' "$LIBXSLT_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
        PKG_CHECK_MODULES(LIBEXSLT, libexslt)
        LIBEXSLT_CFLAGS=$(printf '%s' "$LIBEXSLT_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
    fi

    dnl Check for xsltproc
    AC_PATH_PROG(XSLTPROC, xsltproc, no)
    if test "$XSLTPROC" = "no"; then
        AC_MSG_ERROR([xsltproc is required])
    fi

    libo_MINGW_CHECK_DLL([libxslt])
    libo_MINGW_CHECK_DLL([libexslt])
    libo_MINGW_TRY_DLL([iconv])
else
    AC_MSG_RESULT([internal])
    SYSTEM_LIBXSLT=NO
    LIBXSLT_MAJOR=1
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
AC_SUBST(LIBXSLT_MAJOR)
AC_SUBST(XSLTPROC)

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
        LIBXML_CFLAGS=$(printf '%s' "$LIBXML_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
    fi

    dnl Check for xmllint
    AC_PATH_PROG(XMLLINT, xmllint, no)
    if test "$XMLLINT" = "no"; then
        AC_MSG_ERROR([xmllint is required])
    fi

    libo_MINGW_CHECK_DLL([libxml2])
    libo_MINGW_TRY_DLL([zlib1])
else
    AC_MSG_RESULT([internal])
    SYSTEM_LIBXML=NO
    BUILD_TYPE="$BUILD_TYPE LIBXML2"
fi
AC_SUBST(SYSTEM_LIBXML)
AC_SUBST(LIBXML_CFLAGS)
AC_SUBST(LIBXML_LIBS)
AC_SUBST(XMLLINT)

# =====================================================================
# Checking for a Python interpreter with version >= 2.5 needed to build LO.
# runtime requires Python 3 compatible version (>= 2.6)
# Optionally user can pass an option to configure, i. e.
# ./configure PYTHON=/usr/bin/python
# =====================================================================
if test "$build_os" != "cygwin"; then
    # This allows a lack of system python with no error, we use internal one in that case.
    AM_PATH_PYTHON([2.5],, [:])
    # Clean PYTHON_VERSION checked below if cross-compiling
    PYTHON_VERSION=""
    if test "$PYTHON" != ":"; then
        PYTHON_FOR_BUILD=$PYTHON
    fi
fi
AC_SUBST(PYTHON_FOR_BUILD)

# Checks for Python to use for Pyuno
AC_MSG_CHECKING([which Python to use for Pyuno])
case "$enable_python" in
no|disable)
    if test -z $PYTHON_FOR_BUILD; then
        # Python is required to build LibreOffice. In theory we could separate the build-time Python
        # requirement from the choice whether to include Python stuff in the installer, but why
        # bother?
        AC_MSG_ERROR([Python is required at build time.])
    fi
    enable_python=no
    AC_MSG_RESULT([none])
    ;;
""|yes|auto)
    if test "$DISABLE_SCRIPTING" = TRUE -a -n "$PYTHON_FOR_BUILD"; then
        AC_MSG_RESULT([no, overridden by --disable-scripting])
        enable_python=no
    elif test $build_os = cygwin; then
        dnl When building on Windows we don't attempt to use any installed
        dnl "system"  Python.
        dnl
        dnl (When cross-compiling to Windows from Linux using the mingw32-cross
        dnl compiler from OBS, use mingw32-python from OBS, and ditto for other
        dnl MinGW cross-compilation setups.)
        AC_MSG_RESULT([internal])
        enable_python=internal
    elif test "$cross_compiling" = yes; then
        AC_MSG_RESULT([system])
        enable_python=system
    else
        # Unset variables set by the above AM_PATH_PYTHON so that
        # we actually do check anew.
        unset PYTHON am_cv_pathless_PYTHON ac_cv_path_PYTHON am_cv_python_version am_cv_python_platform am_cv_python_pythondir am_cv_python_pyexecdir
        AM_PATH_PYTHON([3.3],, [:])
        if test "$PYTHON" = :; then
            AC_MSG_RESULT([internal])
            enable_python=internal
        else
            AC_MSG_RESULT([system])
            enable_python=system
        fi
    fi
    ;;
internal)
    AC_MSG_RESULT([internal])
    ;;
system)
    AC_MSG_RESULT([system])
    ;;
*)
    AC_MSG_ERROR([Incorrect --enable-python option])
    ;;
esac

if test $enable_python != no; then
    BUILD_TYPE="$BUILD_TYPE PYUNO"
fi

if test $enable_python = system; then
    if test -z "$PYTHON_CFLAGS" -a $_os = Darwin; then
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
    fi
    if test -n "$PYTHON_CFLAGS" -a -n "$PYTHON_LIBS"; then
        # Fallback: Accept these in the environment, or as set above
        # for MacOSX.
        :
    elif test "$cross_compiling" != yes; then
        # Unset variables set by the above AM_PATH_PYTHON so that
        # we actually do check anew.
        unset PYTHON am_cv_pathless_PYTHON ac_cv_path_PYTHON am_cv_python_version am_cv_python_platform am_cv_python_pythondir am_cv_python_pyexecdir
        # This causes an error if no python command is found
        AM_PATH_PYTHON([3.3])
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
                PYTHON_LIBS="-L${MINGW_SYSROOT}/lib -lpython$python_version $python_libs"
                AC_MSG_CHECKING([for python.exe])
                libo_MINGW_CHECK_DLL([libpython$python_version])
                libo_MINGW_CHECK_DLL([libreadline6])
                libo_MINGW_CHECK_DLL([libtermcap])
                # could we somehow extract the really mingw python version from
                # actual distro package?
                # 2.6.2 currently on OpenSUSE 12.1?
                # rpm -q mingw32-python => mingw32-python-2.6.2-17.17.noarch
                PYTHON_VERSION=$python_version.2
                PYTHON_VERSION_MAJOR=`echo $python_version | cut -d . -f 1`
                PYTHON_VERSION_MINOR=`echo $python_version | cut -d . -f 2`
                break
            fi
        done
        AC_MSG_CHECKING([for python version])
        AS_IF([test -n "$PYTHON_VERSION"],
              [AC_MSG_RESULT([$PYTHON_VERSION])],
              [AC_MSG_RESULT([not found])
               AC_MSG_ERROR([no usable python found])])
        test -n "$PYTHON_CFLAGS" && break
    fi
    # let the PYTHON_FOR_BUILD match the same python installation that
    # provides PYTHON_CFLAGS/PYTHON_LDFLAGS for pyuno, which should be
    # better for PythonTests.
    PYTHON_FOR_BUILD=$PYTHON
fi

dnl By now enable_python should be "system", "internal" or "no"
case $enable_python in
system)
    SYSTEM_PYTHON=YES

    if test "$CROSS_COMPILING" != YES; then
        dnl Check if the headers really work
        save_CPPFLAGS="$CPPFLAGS"
        CPPFLAGS="$CPPFLAGS $PYTHON_CFLAGS"
        AC_CHECK_HEADER(Python.h, [],
           [AC_MSG_ERROR([Python headers not found. You probably want to set both the PYTHON_CFLAGS and PYTHON_LIBS environment variables.])],
           [])
        CPPFLAGS="$save_CPPFLAGS"

        AC_LANG_PUSH(C)
        CFLAGS="$CFLAGS $PYTHON_CFLAGS"
        AC_MSG_CHECKING([for correct python library version])
           AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <Python.h>

int main(int argc, char **argv) {
       if ((PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION >= 6) ||
           (PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 3)) return 0;
       else return 1;
}
           ]])],[AC_MSG_RESULT([ok])],[AC_MSG_ERROR([Python >= 3.3 is needed when building with Python 3, or Python >= 2.6 when building with Python 2])],[])
        CFLAGS=$save_CFLAGS
        AC_LANG_POP(C)

        dnl FIXME Check if the Python library can be linked with, too?
    fi
    ;;

internal)
    SYSTEM_PYTHON=NO
    PYTHON_VERSION_MAJOR=3
    PYTHON_VERSION_MINOR=3
    PYTHON_VERSION=${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}.0
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
    DISABLE_PYTHON=TRUE
    SYSTEM_PYTHON=NO
    ;;
*)
    AC_MSG_ERROR([Internal configure script error, invalid enable_python value "$enable_python"])
    ;;
esac

AC_SUBST(DISABLE_PYTHON)
AC_SUBST(SYSTEM_PYTHON)
AC_SUBST(PYTHON_CFLAGS)
AC_SUBST(PYTHON_LIBS)
AC_SUBST(PYTHON_VERSION)
AC_SUBST(PYTHON_VERSION_MAJOR)
AC_SUBST(PYTHON_VERSION_MINOR)

AC_MSG_CHECKING([whether to build the MariaDB/MySQL Connector extension])
if test "x$enable_ext_mariadb_connector" = "xyes" -a "x$enable_extension_integration" != "xno"; then
    AC_MSG_RESULT([yes])
    ENABLE_MARIADBC=TRUE
    MARIADBC_MAJOR=1
    MARIADBC_MINOR=0
    MARIADBC_MICRO=2
    BUILD_TYPE="$BUILD_TYPE MARIADBC"
else
    AC_MSG_RESULT([no])
    ENABLE_MARIADBC=
fi
AC_SUBST(ENABLE_MARIADBC)
AC_SUBST(MARIADBC_MAJOR)
AC_SUBST(MARIADBC_MINOR)
AC_SUBST(MARIADBC_MICRO)

if test "$ENABLE_MARIADBC" = "TRUE"; then

    SCPDEFS="$SCPDEFS -DWITH_EXTENSION_MARIADBC"

    dnl ===================================================================
    dnl Check for system MariaDB
    dnl ===================================================================
    AC_MSG_CHECKING([which MariaDB to use])
    if test "$with_system_mariadb" = "yes"; then
        AC_MSG_RESULT([external])
        SYSTEM_MARIADB=YES
        AC_PATH_PROG(MARIADBCONFIG, [mariadb_config])
        if test -z "$MARIADBCONFIG"; then
            AC_PATH_PROG(MARIADBCONFIG, [mysql_config])
            if test -z "$MARIADBCONFIG"; then
                AC_MSG_ERROR([mariadb_config and mysql_config are missing. Install MariaDB or MySQL package.])
            fi
        fi
        AC_MSG_CHECKING([MariaDB version])
        MARIADB_VERSION=`$MARIADBCONFIG --version`
        MARIADB_MAJOR=`$MARIADBCONFIG --version | cut -d"." -f1`
        if test "$MARIADB_MAJOR" -ge "5"; then
            AC_MSG_RESULT([OK])
        else
            AC_MSG_ERROR([too old, use 5.0.x or later])
        fi
        AC_MSG_CHECKING([for MariaDB Client library])
        MARIADB_CFLAGS=`$MARIADBCONFIG --cflags`
        MARIADB_LIBS=`$MARIADBCONFIG --libs`
        AC_MSG_RESULT([includes $MARIADB_CFLAGS, libraries $MARIADB_LIBS])
    else
        AC_MSG_RESULT([internal])
        SYSTEM_MARIADB=NO
        MARIADB_CFLAGS="-I${WORKDIR}/UnpackedTarball/mariadb/include"
        MARIADB_LIBS="-L${OUTDIR}/lib -lmariadblib"
        BUILD_TYPE="$BUILD_TYPE MARIADB"
    fi

    AC_SUBST(SYSTEM_MARIADB)
    AC_SUBST(MARIADB_CFLAGS)
    AC_SUBST(MARIADB_LIBS)

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
        BUILD_TYPE="$BUILD_TYPE MYSQLCPPCONN"
        SYSTEM_MYSQL_CPPCONN=NO
    fi
    AC_LANG_POP([C++])
fi
AC_SUBST(SYSTEM_MYSQL_CPPCONN)

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

    if test "$with_krb5" = "yes" -a "$enable_openssl" = "no"; then
        AC_MSG_ERROR([krb5 needs OpenSSL, but --disable-openssl was given.])
    fi
    if test "$with_gssapi" = "yes" -a "$enable_openssl" = "no"; then
        AC_MSG_ERROR([GSSAPI needs OpenSSL, but --disable-openssl was given.])
    fi

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
                    [AC_MSG_ERROR([could not find function 'com_err' required for Kerberos 5, try installing libcom_err devel package])])
                KRB5_LIBS=$LIBS
                LIBS=$save_LIBS
                AC_SEARCH_LIBS(krb5_sendauth, [krb5 'krb5 -lcrypto -ldes -lasn1 -lroken'], [],
                    [AC_MSG_ERROR([could not find function 'krb5_sendauth' required for Kerberos 5, try installing krb5 devel package])])
                KRB5_LIBS="$KRB5_LIBS $LIBS"
                LIBS=$save_LIBS
            fi
            if test "$with_gssapi" != "no"; then
              WITH_GSSAPI=YES
              save_LIBS=$LIBS
              AC_SEARCH_LIBS(gss_init_sec_context, [gssapi_krb5 gss 'gssapi -lkrb5 -lcrypto'], [],
                    [AC_MSG_ERROR([could not find function 'gss_init_sec_context' required for GSSAPI])])
              GSSAPI_LIBS=$LIBS
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
                KRB5_LIBS=$LIBS
                LIBS=$save_LIBS
                AC_SEARCH_LIBS(krb5_sendauth, [krb5 'krb5 -lcrypto -ldes -lasn1 -lroken'], [],
                    [AC_MSG_ERROR([could not find function 'krb5_sendauth' required for Kerberos 5])])
                KRB5_LIBS="$KRB5_LIBS $LIBS"
                LIBS=$save_LIBS
            fi
            if test "$with_gssapi" != "no"; then
                WITH_GSSAPI=YES
                save_LIBS=$LIBS
                AC_SEARCH_LIBS(gss_init_sec_context, [gssapi_krb5 gss 'gssapi -lkrb5 -lcrypto'], [],
                    [AC_MSG_ERROR([could not find function 'gss_init_sec_context' required for GSSAPI])])
                GSSAPI_LIBS=$LIBS
                LIBS=$save_LIBS
            fi
            ;;
        *)
            if test "$with_krb5" = "yes"; then
                WITH_KRB5=YES
                save_LIBS=$LIBS
                AC_SEARCH_LIBS(com_err, [com_err 'com_err -lssl -lcrypto' krb5 'krb5 -lcrypto -ldes -lasn1 -lroken'], [],
                    [AC_MSG_ERROR([could not find function 'com_err' required for Kerberos 5])])
                KRB5_LIBS=$LIBS
                LIBS=$save_LIBS
                AC_SEARCH_LIBS(krb5_sendauth, [krb5 'krb5 -lcrypto -ldes -lasn1 -lroken'], [],
                    [AC_MSG_ERROR([could not find function 'krb5_sendauth' required for Kerberos 5])])
                KRB5_LIBS="$KRB5_LIBS $LIBS"
                LIBS=$save_LIBS
            fi
            if test "$with_gssapi" = "yes"; then
                WITH_GSSAPI=YES
                save_LIBS=$LIBS
                AC_SEARCH_LIBS(gss_init_sec_context, [gssapi_krb5 gss 'gssapi -lkrb5 -lcrypto'], [],
                    [AC_MSG_ERROR([could not find function 'gss_init_sec_context' required for GSSAPI])])
                LIBS=$save_LIBS
                GSSAPI_LIBS=$LIBS
            fi
        esac

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
AC_SUBST(GSSAPI_LIBS)
AC_SUBST(KRB5_LIBS)
AC_SUBST(BUILD_POSTGRESQL_SDBC)
AC_SUBST(SYSTEM_POSTGRESQL)
AC_SUBST(POSTGRESQL_INC)
AC_SUBST(POSTGRESQL_LIB)

dnl ===================================================================
dnl Check for Firebird stuff
dnl ===================================================================
ENABLE_FIREBIRD_SDBC=""
if test "x$enable_firebird_sdbc" = "xyes"; then
    SCPDEFS="$SCPDEFS -DWITH_FIREBIRD_SDBC"

    dnl ===================================================================
    dnl Check for system Firebird
    dnl ===================================================================
    AC_MSG_CHECKING([which Firebird to use])
    if test "$with_system_firebird" = "yes"; then
        AC_MSG_RESULT([external])
        SYSTEM_FIREBIRD=YES
        AC_PATH_PROG(FIREBIRDCONFIG, [fb_config])
        if test -z "$FIREBIRDCONFIG"; then
            AC_MSG_NOTICE([No fb_config -- using pkg-config])
            PKG_CHECK_MODULES(FIREBIRD, fbembed)
            FIREBIRD_VERSION=`pkg-config --modversion fbembed`
        else
            AC_MSG_NOTICE([fb_config found])
            FIREBIRD_VERSION=`$FIREBIRDCONFIG --version`
            AC_MSG_CHECKING([for Firebird Client library])
            FIREBIRD_CFLAGS=`$FIREBIRDCONFIG --cflags`
            FIREBIRD_LIBS=`$FIREBIRDCONFIG --embedlibs`
        fi
        AC_MSG_RESULT([includes `$FIREBIRD_CFLAGS', libraries `$FIREBIRD_LIBS'])
        AC_MSG_CHECKING([Firebird version])
        if test -n "${FIREBIRD_VERSION}"; then
            FIREBIRD_MAJOR=`echo $FIREBIRD_VERSION | cut -d"." -f1`
            FIREBIRD_MINOR=`echo $FIREBIRD_VERSION | cut -d"." -f2`
            if test "$FIREBIRD_MAJOR" -eq "2" -a "$FIREBIRD_MINOR" -eq "5"; then
                AC_MSG_RESULT([OK])
            else
                AC_MSG_ERROR([Ensure firebird 2.5.x is installed])
            fi
        else
            __save_CFLAGS="${CFLAGS}"
            CFLAGS="${CFLAGS} ${FIREBIRD_CFLAGS}"
            AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <ibase.h>
#if defined(FB_API_VER) && FB_API_VER == 25
#else
#error "Wrong Firebird API version"
#endif]])],AC_MSG_RESULT([OK]),AC_MSG_ERROR([Ensure firebird 2.5.x is installed]))
            CFLAGS="${__save_CFLAGS}"
        fi
        ENABLE_FIREBIRD_SDBC="TRUE"
    elif test "$enable_database_connectivity" != yes; then
        AC_MSG_RESULT([none])
    elif test "$cross_compiling" = "yes"; then
        AC_MSG_RESULT([none])
    else
        dnl ===================================================================
        dnl Check for system libatomic-ops
        dnl ===================================================================
        libo_CHECK_SYSTEM_MODULE([libatomic_ops],[ATOMIC_OPS],[atomic_ops >= 0.7.2])
        if test "$with_system_libatomic_ops" = "yes"; then
            SYSTEM_LIBATOMIC_OPS=YES
            AC_CHECK_HEADERS(atomic_ops.h, [],
            [AC_MSG_ERROR(atomic_ops.h not found. install libatomic-ops)], [])
        fi

        AC_MSG_RESULT([internal])
        SYSTEM_FIREBIRD=NO
        FIREBIRD_CFLAGS="-I${WORKDIR}/UnpackedTarball/firebird/include"
        FIREBIRD_LIBS="-lfbembed"

        BUILD_TYPE="$BUILD_TYPE FIREBIRD"
        ENABLE_FIREBIRD_SDBC="TRUE"
    fi

fi
AC_SUBST(ENABLE_FIREBIRD_SDBC)
AC_SUBST(SYSTEM_FIREBIRD)
AC_SUBST(FIREBIRD_CFLAGS)
AC_SUBST(FIREBIRD_LIBS)
dnl AC_SUBST([TOMMATH_CFLAGS])
dnl AC_SUBST([TOMMATH_LIBS])

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
        CURL_CFLAGS=$("$CURLCONFIG" --cflags | sed -e "s/-I/${ISYSTEM?}/g")
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

    libo_MINGW_CHECK_DLL([libcurl])
    libo_MINGW_TRY_DLL([libintl])
    libo_MINGW_TRY_DLL([libidn])
    libo_MINGW_TRY_DLL([libnspr4])
    libo_MINGW_TRY_DLL([nssutil3])
    libo_MINGW_TRY_DLL([libplc4])
    libo_MINGW_TRY_DLL([libplds4])
    libo_MINGW_TRY_DLL([nss3])
    libo_MINGW_TRY_DLL([ssl3])
    libo_MINGW_TRY_DLL([libgpg-error])
    libo_MINGW_TRY_DLL([libgcrypt])
    libo_MINGW_TRY_DLL([libssh2])
else
    AC_MSG_RESULT([internal])
    SYSTEM_CURL=NO
    BUILD_TYPE="$BUILD_TYPE CURL"
fi
AC_SUBST(SYSTEM_CURL)
AC_SUBST(CURL_CFLAGS)
AC_SUBST(CURL_LIBS)

dnl ===================================================================
dnl Check for system boost
dnl ===================================================================
AC_MSG_CHECKING([which boost to use])
if test "$with_system_boost" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_BOOST=YES
    AX_BOOST_BASE(1.47)
    AX_BOOST_DATE_TIME
    mingw_boost_date_time_dll=`echo $BOOST_DATE_TIME_LIB | sed -e 's/^-l//' -e 's/\.dll$//'`
    libo_MINGW_TRY_DLL([$mingw_boost_date_time_dll])
    if test -z "$MINGW_BOOST_DATE_TIME_DLL"; then
        # if not found, try again with 'lib' prefix
        libo_MINGW_CHECK_DLL([lib$mingw_boost_date_time_dll])
    fi
    AC_LANG_PUSH([C++])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS $BOOST_CPPFLAGS"
    AC_CHECK_HEADER(boost/shared_ptr.hpp, [],
       [AC_MSG_ERROR(boost/shared_ptr.hpp not found. install boost)], [])
    AC_CHECK_HEADER(boost/spirit/include/classic_core.hpp, [],
       [AC_MSG_ERROR(boost/spirit/include/classic_core.hpp not found. install boost >= 1.36)], [])
    AC_CHECK_HEADER(boost/function.hpp, [],
       [AC_MSG_ERROR(boost/function.hpp not found. install boost)], [])
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
else
    AC_MSG_RESULT([internal])
    BUILD_TYPE="$BUILD_TYPE BOOST"
    SYSTEM_BOOST=NO
fi
AC_SUBST(SYSTEM_BOOST)

dnl ===================================================================
dnl Check for system mdds
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([mdds], [MDDS], [mdds >= 0.9.0])

dnl ===================================================================
dnl Determine which hash container mdds shall use
dnl ===================================================================
AC_MSG_CHECKING([which hash container mdds shall use])
if test "x$HAVE_CXX11" = "xTRUE" -a "x$SYSTEM_MDDS" = "xYES"; then
    MDDS_CPPFLAGS=$CXXFLAGS_CXX11
    AC_MSG_RESULT([std::unordered_map])
else
    MDDS_CPPFLAGS="-DMDDS_HASH_CONTAINER_BOOST"
    AC_MSG_RESULT([boost::unordered_map])
fi
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
    BUILD_TYPE="$BUILD_TYPE VIGRA"
    SYSTEM_VIGRA=NO
fi
AC_SUBST(SYSTEM_VIGRA)

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
    BUILD_TYPE="$BUILD_TYPE OPENLDAP"
fi
fi
AC_SUBST(SYSTEM_OPENLDAP)

dnl ===================================================================
dnl Check for mozilla ab connectivity for windows
dnl ===================================================================

if test "$_os" = "WINNT"; then
    AC_MSG_CHECKING([whether to enable build of Mozilla addressbook connectivity driver for Windows])
    if test "$enable_win_mozab_driver" = "yes" -a "$WITH_MINGW" != "YES" ; then
        WITH_MOZAB4WIN=YES
        AC_MSG_RESULT([yes, internal (old windows mozab driver)])
        BUILD_TYPE="$BUILD_TYPE MOZ"
        MSVC80_DLLS="msvcp80.dll msvcr80.dll Microsoft.VC80.CRT.manifest"
        MSVC80_DLL_PATH=`cygpath -u "$TARFILE_LOCATION"`
        for dll in $MSVC80_DLLS; do
            if ! test -f "$MSVC80_DLL_PATH/$dll"; then
                AC_MSG_ERROR([can not find $dll in $MSVC80_DLL_PATH needed for the pre-built Mozilla libraries])
            fi
        done
    else
        AC_MSG_RESULT([no])
        WITH_MOZAB4WIN=NO
    fi
fi
AC_SUBST(WITH_MOZAB4WIN)
AC_SUBST(MSVC80_DLLS)
AC_SUBST(MSVC80_DLL_PATH)

dnl ===================================================================
dnl Check for TLS/SSL and cryptographic implementation to use
dnl ===================================================================
AC_MSG_CHECKING([which TLS/SSL and cryptographic implementation to use])
if test -n "$with_tls"; then
    case $with_tls in
    openssl)
        AC_DEFINE(USE_TLS_OPENSSL)
        TLS=OPENSSL

        if test "$enable_openssl" != "yes"; then
            AC_MSG_ERROR(["Disabling OpenSSL was requested, but the requested TLS to use is actually OpenSSL."])
        fi

        # warn that OpenSSL has been selected but not all TLS code has this option
        AC_MSG_WARN([TLS/SSL implementation to use is OpenSSL but some code may still be depending on NSS or GNUTLS])
        add_warning "TLS/SSL implementation to use is OpenSSL but some code may still be depending on NSS or GNUTLS"
        ;;
    nss)
        AC_DEFINE(USE_TLS_NSS)
        TLS=NSS
        ;;
    *)
        AC_MSG_ERROR([unsupported implementation $with_tls. Supported are:
openssl - OpenSSL
nss - Mozilla's Network Security Services (NSS)
    ])
        ;;
    esac
else
    if test "$enable_openssl" = "yes"; then
        AC_DEFINE(USE_TLS_OPENSSL)
        TLS=OPENSSL
    else
        AC_DEFINE(USE_TLS_NSS)
        TLS=NSS
    fi
fi
AC_MSG_RESULT([$TLS])
AC_SUBST(TLS)

dnl ===================================================================
dnl Check for system NSS
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([nss],[NSS],[nss >= 3.9.3 nspr >= 4.8])
if test "$with_system_nss" = "yes"; then
    libo_MINGW_CHECK_DLL([libnspr4])
    libo_MINGW_CHECK_DLL([libplc4])
    libo_MINGW_CHECK_DLL([libplds4])
    libo_MINGW_CHECK_DLL([nss3])
    libo_MINGW_CHECK_DLL([nssutil3])
    libo_MINGW_CHECK_DLL([smime3])
    libo_MINGW_CHECK_DLL([ssl3])
else
    NSS_MAJOR="3"
    NSS_MINOR="13"
    NSS_PATCH="5"
fi
AC_SUBST(NSS_MAJOR)
AC_SUBST(NSS_MINOR)
AC_SUBST(NSS_PATCH)

dnl ===================================================================
dnl Check for system mozilla headers
dnl ===================================================================
HAVE_NON_CONST_NPP_GETMIMEDESCRIPTION=
AC_MSG_CHECKING([which NPAPI headers to use])

if test "$with_system_npapi_headers" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_NPAPI_HEADERS=YES
    # First try npapi-sdk:
    PKG_CHECK_MODULES(NPAPI_HEADERS, npapi-sdk, [LOCATED=yes], [LOCATED=no])
    # Then go with libxul:
    if test "x$LOCATED" != "xyes"; then
        PKG_CHECK_MODULES(NPAPI_HEADERS, libxul, [LOCATED=yes], [LOCATED=no])
    fi
    if test "x$LOCATED" != "xyes"; then
        PKG_CHECK_MODULES(NPAPI_HEADERS, mozilla-plugin, [LOCATED=yes], [LOCATED=no])
    fi
    # if still not found bail out
    if test "x$LOCATED" != "xyes"; then
        AC_MSG_ERROR([npapi.h header file not found])
    fi

    AC_LANG_PUSH([C])
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS $NPAPI_HEADERS_CFLAGS"
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
    NPAPI_HEADERS_CFLAGS=$(printf '%s' "$NPAPI_HEADERS_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
else
    AC_MSG_RESULT([internal])
        dnl ...but will not be built/used unless ENABLE_NPAPI_FROM_BROWSER or
        dnl ENABLE_NPAPI_INTO_BROWSER is TRUE
    SYSTEM_NPAPI_HEADERS=NO
fi
AC_SUBST(NPAPI_HEADERS_CFLAGS)
AC_SUBST(SYSTEM_NPAPI_HEADERS)
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

ICU_MAJOR=51
ICU_MINOR=1
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

    if test "$cross_compiling" != "yes" -o "$WITH_MINGW" = "yes"; then
        ICUPATH="$PATH"
        if test "$WITH_MINGW" = "yes"; then
            ICUPATH=`$CC -print-sysroot`/mingw/bin:"$ICUPATH"
        fi
        AC_PATH_PROG(ICUCONFIG,icu-config,,[$ICUPATH])

        AC_MSG_CHECKING([ICU version])
        ICU_VERSION=`$ICUCONFIG --noverify --version 2>/dev/null || $ICUCONFIG --version`
        ICU_MAJOR=`echo $ICU_VERSION | cut -d"." -f1`
        ICU_MINOR=`echo $ICU_VERSION | cut -d"." -f2`

        if test "$ICU_MAJOR" -ge "49" -o \( "$ICU_MAJOR" = "4" -a "$ICU_MINOR" -ge "2" \); then
            AC_MSG_RESULT([OK, $ICU_VERSION])
        else
            AC_MSG_ERROR([not suitable, only >= 4.2 supported currently])
        fi
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
            ICU_RECLASSIFIED_CONDITIONAL_JAPANESE_STARTER="YES"
            ICU_RECLASSIFIED_HEBREW_LETTER="YES"
        else
            ICU_RECLASSIFIED_PREPEND_SET_EMPTY="NO"
            ICU_RECLASSIFIED_CONDITIONAL_JAPANESE_STARTER="NO"
            ICU_RECLASSIFIED_HEBREW_LETTER="NO"
        fi
    fi

    if test "$cross_compiling" = "yes"; then
       if test "$ICU_MAJOR" -ge "50"; then
            AC_MSG_RESULT([Ignore ICU_MINOR as obviously the libraries don't include the minor version in their names any more])
            ICU_MINOR=""
       fi
    fi

    libo_MINGW_CHECK_DLL([icudata][$ICU_MAJOR][$ICU_MINOR])
    libo_MINGW_CHECK_DLL([icui18n][$ICU_MAJOR][$ICU_MINOR])
    libo_MINGW_CHECK_DLL([icuuc][$ICU_MAJOR][$ICU_MINOR])
else
    AC_MSG_RESULT([internal])
    SYSTEM_ICU="NO"
    BUILD_TYPE="$BUILD_TYPE ICU"
    # surprisingly set these only for "internal" (to be used by various other
    # external libs): the system icu-config is quite unhelpful and spits out
    # dozens of weird flags and also default path -I/usr/include
    ICU_CFLAGS="-I${WORKDIR}/UnpackedTarball/icu/source/i18n -I${WORKDIR}/UnpackedTarball/icu/source/common"
    ICU_LIBS=""
fi
AC_SUBST(SYSTEM_ICU)
AC_SUBST(SYSTEM_GENBRK)
AC_SUBST(SYSTEM_GENCCODE)
AC_SUBST(SYSTEM_GENCMN)
AC_SUBST(ICU_MAJOR)
AC_SUBST(ICU_MINOR)
AC_SUBST(ICU_RECLASSIFIED_CLOSE_PARENTHESIS)
AC_SUBST(ICU_RECLASSIFIED_PREPEND_SET_EMPTY)
AC_SUBST(ICU_RECLASSIFIED_CONDITIONAL_JAPANESE_STARTER)
AC_SUBST(ICU_RECLASSIFIED_HEBREW_LETTER)
AC_SUBST(ICU_CFLAGS)
AC_SUBST(ICU_LIBS)

dnl ===================================================================
dnl Graphite
dnl ===================================================================

AC_MSG_CHECKING([whether to enable graphite support])
if test $_os != Darwin -a $_os != Android -a $_os != iOS -a \( -z "$enable_graphite" -o "$enable_graphite" != no \); then
    AC_MSG_RESULT([yes])
    ENABLE_GRAPHITE="TRUE"
    AC_DEFINE(ENABLE_GRAPHITE)
    libo_CHECK_SYSTEM_MODULE([graphite],[GRAPHITE],[graphite2 >= 0.9.3])
    if test "$with_system_graphite" = "yes"; then
        libo_MINGW_CHECK_DLL([libgraphite2])
    fi
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_GRAPHITE)

dnl ===================================================================
dnl HarfBuzz
dnl ===================================================================
AC_MSG_CHECKING([whether to enable HarfBuzz support])
if test $_os != WINNT -a $_os != Darwin -a $_os != iOS; then
    AC_MSG_RESULT([yes])
    ENABLE_HARFBUZZ="TRUE"
    if $PKG_CONFIG --atleast-version 0.9.18 harfbuzz; then
        libo_CHECK_SYSTEM_MODULE([harfbuzz],[HARFBUZZ],[harfbuzz-icu >= 0.9.18])
    else
        libo_CHECK_SYSTEM_MODULE([harfbuzz],[HARFBUZZ],[harfbuzz >= 0.9.10])
    fi
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_HARFBUZZ)

dnl ===================================================================
dnl Check for NPAPI interface to plug browser plugins into LibreOffice documents
dnl ===================================================================
AC_MSG_CHECKING([whether to plug browser plugins into LibreOffice documents])
# Obviously no such thing on iOS or Android. Also not possible when building
# 64-bit OS X code as the plugin code uses QuickTime and Carbon.
if test "$_os" != Android -a "$_os" != iOS -a \( $_os != Darwin -o "$BITNESS_OVERRIDE" = "" \) -a \
        "$enable_headless" != yes
then
    AC_MSG_RESULT([yes])
    ENABLE_NPAPI_FROM_BROWSER=TRUE
else
    AC_MSG_RESULT([no])
    ENABLE_NPAPI_FROM_BROWSER=
fi
AC_SUBST(ENABLE_NPAPI_FROM_BROWSER)

dnl ===================================================================
dnl Check for NPAPI interface to plug LibreOffice into browser windows
dnl ===================================================================
AC_MSG_CHECKING([whether to plug LibreOffice into browser windows])
if test "$_os" == WINNT -o "$_os" != Android -a "$_os" != Darwin -a "$_os" != iOS -a \
        "$enable_headless" != yes -a "$enable_gtk" != no
then
    AC_MSG_RESULT([yes])
    ENABLE_NPAPI_INTO_BROWSER=TRUE
else
    AC_MSG_RESULT([no])
    ENABLE_NPAPI_INTO_BROWSER=
fi
AC_SUBST(ENABLE_NPAPI_INTO_BROWSER)

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

    if test -z "$x_includes"; then
        x_includes="default_x_includes"
    fi
    if test -z "$x_libraries"; then
        x_libraries="default_x_libraries"
    fi
    CFLAGS="$CFLAGS $X_CFLAGS"
    LDFLAGS="$LDFLAGS $X_LDFLAGS $X_LIBS"
    AC_CHECK_LIB(X11, XOpenDisplay, x_libs="-lX11 $X_EXTRA_LIBS", [AC_MSG_ERROR([X Development libraries not found])])
else
    x_includes="no_x_includes"
    x_libraries="no_x_libraries"
fi

if test "$WANT_X11" = "yes"; then
    dnl ===================================================================
    dnl Check for Composite.h for Mozilla plugin
    dnl ===================================================================
    AC_CHECK_HEADERS(X11/Composite.h,[],[AC_MSG_ERROR([Xt headers not found])],
     [#include <X11/Intrinsic.h>])

    dnl ===================================================================
    dnl Check for extension headers
    dnl ===================================================================
    AC_CHECK_HEADERS(X11/extensions/shape.h,[],[AC_MSG_ERROR([libXext headers not found])],
     [#include <X11/extensions/shape.h>])
fi

dnl ===================================================================
dnl Check for system Xrender
dnl ===================================================================
AC_MSG_CHECKING([whether to use Xrender])
if test "$WANT_X11" = "yes" -a  "$test_xrender" = "yes"; then
    AC_MSG_RESULT([yes])
    PKG_CHECK_MODULES(XRENDER, xrender)
    XRENDER_CFLAGS=$(printf '%s' "$XRENDER_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
    AC_CHECK_LIB([Xrender], [XRenderQueryVersion], [:],
      [AC_MSG_ERROR(libXrender not found or functional)], [])
    AC_CHECK_HEADER(X11/extensions/Xrender.h, [],
      [AC_MSG_ERROR(Xrender not found. install X)], [])
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
        XRANDR_CFLAGS=$(printf '%s' "$XRANDR_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
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
libo_CHECK_SYSTEM_MODULE([neon],[NEON],[neon >= 0.26.0])
if test "$with_system_neon" = "yes"; then
    NEON_VERSION="`$PKG_CONFIG --modversion neon | $SED 's/\.//g'`"
    libo_MINGW_CHECK_DLL([libneon])
    libo_MINGW_TRY_DLL([libtasn1])
    libo_MINGW_TRY_DLL([libgnutls])
else
    NEON_VERSION=0295
fi
AC_SUBST(NEON_VERSION)
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
DISABLE_OPENSSL="NO"
AC_MSG_CHECKING([whether to disable OpenSSL usage])
if test "$enable_openssl" = "yes"; then
    AC_MSG_RESULT([no])
    if test "$_os" = Darwin -a "${MAC_OS_X_VERSION_MIN_REQUIRED:-0}" -ge 1070; then
        # OpenSSL is deprecated when building for 10.7 or later.
        #
        # http://stackoverflow.com/questions/7406946/why-is-apple-deprecating-openssl-in-macos-10-7-lion
        # http://stackoverflow.com/questions/7475914/libcrypto-deprecated-on-mac-os-x-10-7-lion
        #
        # They apparently got fed up with the unstable ABI of
        # OpenSSL, or something, and recommend you either move to
        # their "Common Crypto Architecture", or, if you have to stay
        # with OpenSSL, just build it yourself statically and bundle
        # with your application.

        with_system_openssl=no
        libo_CHECK_SYSTEM_MODULE([openssl],[OPENSSL],[openssl])
    elif test "$_os" = "Darwin" -o "$_os" = "NetBSD" -o "$_os" = "OpenBSD" -o \
            "$_os" = "DragonFly" && test "$with_system_openssl" != "no"; then
        # Mac OS builds should get out without extra stuff is the Mac porters'
        # wish. And pkg-config is although Xcode ships a .pc for openssl
        with_system_openssl=yes
        SYSTEM_OPENSSL=YES
        OPENSSL_CFLAGS=
        OPENSSL_LIBS="-lssl -lcrypto"
    else
        libo_CHECK_SYSTEM_MODULE([openssl],[OPENSSL],[openssl])
    fi
    if test "$with_system_openssl" = "yes"; then
        libo_MINGW_CHECK_DLL([libssl])
        libo_MINGW_CHECK_DLL([libcrypto])
        AC_MSG_CHECKING([whether openssl supports SHA512])
        AC_LANG_PUSH([C])
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <openssl/sha.h>]],[[
            SHA512_CTX context;
]])],[AC_MSG_RESULT([yes])],[AC_MSG_ERROR([no, openssl too old. Need >= 0.9.8.])])
        AC_LANG_POP(C)
    fi
else
    AC_MSG_RESULT([yes])
    DISABLE_OPENSSL=YES

    # warn that although OpenSSL is disabled, system libraries may be depending on it
    AC_MSG_WARN([OpenSSL has been disabled. No code compiled by LibO will make use of it but system librares may create indirect dependencies])
    add_warning "OpenSSL has been disabled. No code compiled by LibO will make use of it but system librares may create indirect dependencies"
fi

AC_SUBST([DISABLE_OPENSSL])

dnl ===================================================================
dnl Check for building gnutls
dnl ===================================================================
AC_MSG_CHECKING([whether to use gnutls])
if test "$enable_neon" != "no" && test "$enable_openssl" = "no"; then
    AC_MSG_RESULT([yes])
    AM_PATH_LIBGCRYPT()
    PKG_CHECK_MODULES(GNUTLS, [gnutls],,
        AC_MSG_ERROR([[Disabling OpenSSL was requested, but GNUTLS is not
                      available in the system to use as replacement.]]))
fi

AC_SUBST([LIBGCRYPT_CFLAGS])
AC_SUBST([LIBGCRYPT_LIBS])

dnl ===================================================================
dnl Check for system redland
dnl ===================================================================
dnl redland: versions before 1.0.8 write RDF/XML that is useless for ODF (@xml:base)
dnl raptor2: need at least 2.0.7 for CVE-2012-0037
libo_CHECK_SYSTEM_MODULE([redland],[REDLAND],[redland >= 1.0.8 raptor2 >= 2.0.7])
if test "$with_system_redland" = "yes"; then
    libo_MINGW_CHECK_DLL([librdf])
    AC_CHECK_LIB([rdf], [librdf_world_set_raptor_init_handler], [:],
            [AC_MSG_ERROR(librdf too old. Need >= 1.16)], [])
    libo_MINGW_TRY_DLL([libraptor])
    libo_MINGW_TRY_DLL([librasqal])
    libo_MINGW_TRY_DLL([libsqlite3])
else
    RAPTOR_MAJOR="0"
    RASQAL_MAJOR="3"
    REDLAND_MAJOR="0"
fi
AC_SUBST(RAPTOR_MAJOR)
AC_SUBST(RASQAL_MAJOR)
AC_SUBST(REDLAND_MAJOR)

dnl ===================================================================
dnl Check for system orcus
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([orcus],[ORCUS],[liborcus-0.6 >= 0.5.0])
if test "$with_system_orcus" != "yes"; then
    if test "$SYSTEM_BOOST" = "YES"; then
        # ===========================================================
        # Determine if we are going to need to link with Boost.System
        # ===========================================================
        dnl This seems to be necessary since boost 1.50 (1.48 does not need it,
        dnl 1.49 is untested). The macro BOOST_THREAD_DONT_USE_SYSTEM mentioned
        dnl in documentation has no effect.
        AC_MSG_CHECKING([if we need to link with Boost.System])
        AC_LANG_PUSH([C++])
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
                @%:@include <boost/version.hpp>
            ]],[[
                #if BOOST_VERSION >= 105000
                #   error yes, we need to link with Boost.System
                #endif
            ]])],[
                AC_MSG_RESULT([no])
            ],[
                AC_MSG_RESULT([yes])
                AX_BOOST_SYSTEM
        ])
        AC_LANG_POP([C++])
    fi
fi
dnl FIXME by renaming SYSTEM_LIBORCUS to SYSTEM_ORCUS in the build system world
SYSTEM_LIBORCUS=$SYSTEM_ORCUS
AC_SUBST([BOOST_SYSTEM_LIB])
AC_SUBST(SYSTEM_LIBORCUS)

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
    libo_MINGW_CHECK_DLL([libhunspell-1.3])
    HUNSPELL_CFLAGS=$(printf '%s' "$HUNSPELL_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
else
    AC_MSG_RESULT([internal])
    SYSTEM_HUNSPELL=NO
    HUNSPELL_CFLAGS="-I${WORKDIR}/UnpackedTarball/hunspell/src/hunspell"
    HUNSPELL_LIBS="-L${OUTDIR}/lib -lhunspell"
    BUILD_TYPE="$BUILD_TYPE HUNSPELL"
fi
AC_SUBST(SYSTEM_HUNSPELL)
AC_SUBST(HUNSPELL_CFLAGS)
AC_SUBST(HUNSPELL_LIBS)

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
    libo_MINGW_CHECK_DLL([libhyphen])
else
    AC_MSG_RESULT([internal])
    SYSTEM_HYPH=NO
    BUILD_TYPE="$BUILD_TYPE HYPHEN"
fi
AC_SUBST(SYSTEM_HYPH)
AC_SUBST(HYPHEN_LIB)

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
    libo_MINGW_CHECK_DLL([libmythes-1.2])
    MYTHES_CFLAGS=$(printf '%s' "$MYTHES_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
else
    AC_MSG_RESULT([internal])
    SYSTEM_MYTHES=NO
    BUILD_TYPE="$BUILD_TYPE MYTHES"
fi
AC_SUBST(SYSTEM_MYTHES)
AC_SUBST(MYTHES_CFLAGS)
AC_SUBST(MYTHES_LIBS)

dnl ===================================================================
dnl How should we build the linear programming solver ?
dnl ===================================================================
ENABLE_LPSOLVE=
AC_MSG_CHECKING([whether to build with lpsolve])
if test "$enable_lpsolve" != "no"; then
    ENABLE_LPSOLVE=TRUE
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_LPSOLVE)

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
    libo_MINGW_CHECK_DLL([lpsolve55])
else
    AC_MSG_RESULT([internal])
    SYSTEM_LPSOLVE=NO
    BUILD_TYPE="$BUILD_TYPE LPSOLVE"
fi
AC_SUBST(SYSTEM_LPSOLVE)



dnl ===================================================================
dnl Checking for libexttextcat
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libexttextcat],[LIBEXTTEXTCAT],[libexttextcat >= 3.1.1])
if test "$with_system_libexttextcat" = "yes"; then
    SYSTEM_LIBEXTTEXTCAT_DATA=file://`$PKG_CONFIG --variable=pkgdatadir libexttextcat`
fi
AC_SUBST(SYSTEM_LIBEXTTEXTCAT_DATA)

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
    AC_CHECK_FUNCS(readdir_r, HAVE_READDIR_R=YES, [HAVE_READDIR_R=NO])
    if test "$HAVE_READDIR_R" = "YES"; then
        SYSTEM_LIBC=YES
    fi
else
    HAVE_READDIR_R=NO
    SYSTEM_LIBC=YES
fi

if test "$HAVE_READDIR_R" = NO; then
    BUILD_TYPE="$BUILD_TYPE GLIBC"
fi

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
        find_winsdk
        WINDOWS_SDK_HOME=$winsdktest

        # normalize if found
        if test -n "$WINDOWS_SDK_HOME"; then
            WINDOWS_SDK_HOME=`cygpath -d "$WINDOWS_SDK_HOME"`
            WINDOWS_SDK_HOME=`cygpath -u "$WINDOWS_SDK_HOME"`
        fi
    fi

    if test -n "$WINDOWS_SDK_HOME"; then
        # Remove a possible trailing backslash
        WINDOWS_SDK_HOME=`echo $WINDOWS_SDK_HOME | $SED 's/\/$//'`

        if test -f "$WINDOWS_SDK_HOME/Include/adoint.h" \
             -a -f "$WINDOWS_SDK_HOME/Include/SqlUcode.h" \
             -a -f "$WINDOWS_SDK_HOME/Include/usp10.h"; then
            have_windows_sdk_headers=yes
        elif test -f "$WINDOWS_SDK_HOME/Include/um/adoint.h" \
             -a -f "$WINDOWS_SDK_HOME/Include/um/SqlUcode.h" \
             -a -f "$WINDOWS_SDK_HOME/Include/um/usp10.h"; then
            have_windows_sdk_headers=yes
        else
            have_windows_sdk_headers=no
        fi

        if test -f "$WINDOWS_SDK_HOME/lib/user32.lib"; then
            have_windows_sdk_libs=yes
        elif test -f "$WINDOWS_SDK_HOME/lib/win8/um/$WINDOWS_SDK_ARCH/user32.lib"; then
            have_windows_sdk_libs=yes
        else
            have_windows_sdk_libs=no
        fi

        if test $have_windows_sdk_headers = no -o $have_windows_sdk_libs = no; then
            AC_MSG_ERROR([Some (all?) Windows SDK files not found, please check if all needed parts of
the  Windows SDK are installed.])
        fi

        if test ! -f "$WINDOWS_SDK_HOME/bin/msiinfo.exe" \
             -o ! -f "$WINDOWS_SDK_HOME/bin/msidb.exe" \
             -o ! -f "$WINDOWS_SDK_HOME/bin/uuidgen.exe" \
             -o ! -f "$WINDOWS_SDK_HOME/bin/msitran.exe"; then :
        elif test ! -f "$WINDOWS_SDK_HOME/bin/x86/msiinfo.exe" \
             -o ! -f "$WINDOWS_SDK_HOME/bin/x86/msidb.exe" \
             -o ! -f "$WINDOWS_SDK_HOME/bin/x86/uuidgen.exe" \
             -o ! -f "$WINDOWS_SDK_HOME/bin/x86/msitran.exe"; then :
        else
            AC_MSG_ERROR([Some (all?) Windows Installer tools in the Windows SDK are missing, please install.])
        fi
    fi

    if test -z "$WINDOWS_SDK_HOME"; then
        AC_MSG_RESULT([no, hoping the necessary headers and libraries will be found anyway!?])
    elif echo $WINDOWS_SDK_HOME | grep "v6.0" >/dev/null 2>/dev/null; then
        WINDOWS_SDK_VERSION=60
        AC_MSG_RESULT([found Windows SDK 6.0 ($WINDOWS_SDK_HOME)])
    elif echo $WINDOWS_SDK_HOME | grep "v6.1" >/dev/null 2>/dev/null; then
        WINDOWS_SDK_VERSION=61
        AC_MSG_RESULT([found Windows SDK 6.1 ($WINDOWS_SDK_HOME)])
    elif echo $WINDOWS_SDK_HOME | grep "v7" >/dev/null 2>/dev/null; then
        WINDOWS_SDK_VERSION=70
        AC_MSG_RESULT([found Windows SDK 7 ($WINDOWS_SDK_HOME)])
    elif echo $WINDOWS_SDK_HOME | grep "8.0" >/dev/null 2>/dev/null; then
        WINDOWS_SDK_VERSION=80
        AC_MSG_RESULT([found Windows SDK 8.0 ($WINDOWS_SDK_HOME)])
        # compatibility warning if usind VS 2012 and not explicitly choosing the 80 SDK
        if test "$VCVER" = "110" -a -z "$with_windows_sdk"; then
            AC_MSG_WARN([If a build created with VS 2012 should run on Windows XP,])
            AC_MSG_WARN([use --with-windows-sdk=7.1A (requires VS 2012 Update 1 or newer)])
            add_warning "If a build created with VS 2012 should run on Windows XP,"
            add_warning "use --with-windows-sdk=7.1A (requires VS 2012 Update 1 or newer)"
        fi
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

    dnl TODO: solenv/bin/modules/installer/windows/msiglobal.pm wants to use a
    dnl WiLangId.vbs that is included only in some SDKs (e.g., included in v7.1
    dnl but not in v8.0), so allow this to be overriden with a
    dnl WINDOWS_SDK_WILANGID for now; a full-blown --with-windows-sdk-wilangid
    dnl and configuration error if no WiLangId.vbs is found would arguably be
    dnl better, but I do not know under which conditions exactly it is needed by
    dnl msiglobal.pm:
    if test -z "$WINDOWS_SDK_WILANGID" -a -n "$WINDOWS_SDK_HOME"; then
        WINDOWS_SDK_WILANGID=$WINDOWS_SDK_HOME/Samples/sysmgmt/msi/scripts/WiLangId.vbs
        if ! test -e "$WINDOWS_SDK_WILANGID" ; then
            WINDOWS_SDK_WILANGID=$WINDOWS_SDK_HOME/bin/$WINDOWS_SDK_ARCH/WiLangId.vbs
        fi
        if ! test -e "$WINDOWS_SDK_WILANGID" ; then
            WINDOWS_SDK_WILANGID=$(cygpath -sm "C:/Program Files (x86)/Windows Kits/8.0/bin/$WINDOWS_SDK_ARCH/WiLangId.vbs")
        fi
        if ! test -e "$WINDOWS_SDK_WILANGID" ; then
            AC_MSG_WARN([WiLangId.vbs not found - building translated packages will fail])
            add_warning "WiLangId.vbs not found - building translated packages will fail"
        fi
    fi
fi
AC_SUBST(WINDOWS_SDK_HOME)
AC_SUBST(WINDOWS_SDK_VERSION)
AC_SUBST(WINDOWS_SDK_WILANGID)

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
        dnl A standard installation of the DirectX SDK sets $DXSDK_DIR
        if test -n "$DXSDK_DIR"; then
            DIRECTXSDK_HOME=`cygpath -d "$DXSDK_DIR"`
            DIRECTXSDK_HOME=`cygpath -u "$DIRECTXSDK_HOME"`
        fi
        # At this point $DIRECTXSDK_HOME might still be undefined. This will lead to
        # the "DirectX SDK not found" error later.
        # (Where?)

        # Remove a possible trailing backslash
        DIRECTXSDK_HOME=`echo $DIRECTXSDK_HOME | $SED 's/\/$//'`
    fi

    if test -f "$DIRECTXSDK_HOME/Include/ddraw.h" -o -f "$DIRECTXSDK_HOME/Include/d3d9.h"; then
        HAVE_DIRECTXSDK_H="yes"
    else
        HAVE_DIRECTXSDK_H="no"
    fi

    if test "$BITNESS_OVERRIDE" = 64; then
        DIRECTXSDK_LIB="$DIRECTXSDK_HOME/lib/x64"
    else
        DIRECTXSDK_LIB="$DIRECTXSDK_HOME/lib/x86"
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
    if test -n "$DIRECTXSDK_HOME"; then
        PathFormat "$DIRECTXSDK_HOME"
        DIRECTXSDK_HOME="$formatted_path"
        SOLARINC="$SOLARINC -I$DIRECTXSDK_HOME/include"
    fi
fi
AC_SUBST(DIRECTXSDK_HOME)

dnl ***************************************
dnl Checking for bison and flex
dnl ***************************************
AC_PATH_PROG(BISON, bison)
if test -z "$BISON"; then
    AC_MSG_ERROR([no bison found in \$PATH, install it])
else
    AC_MSG_CHECKING([the bison version])
    _bison_version=`$BISON --version | grep GNU | $SED -e 's@^[[^0-9]]*@@' -e 's@ .*@@' -e 's@,.*@@'`
    _bison_longver=`echo $_bison_version | $AWK -F. '{ print \$1*1000+\$2}'`
    # Accept newer than 2.0
    if test "$_bison_longver" -lt 2000; then
        AC_MSG_ERROR([failed ($BISON $_bison_version need 2.0+)])
    fi
fi

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

    AC_MSG_CHECKING([$VC_PRODUCT_DIR/$assembler_bin/$assembler])
    if test -f "$VC_PRODUCT_DIR/$assembler_bin/$assembler"; then
        ASM_HOME=$VC_PRODUCT_DIR/$assembler_bin
        AC_MSG_RESULT([found])
        ML_EXE="$VC_PRODUCT_DIR/$assembler_bin/$assembler"
    else
        AC_MSG_ERROR([Configure did not find $assembler assembler.])
    fi

    PathFormat "$ASM_HOME"
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
    AC_MSG_ERROR([Zip version 3.0 or newer is required to build, please install it and make sure it is the one found first in PATH],,)
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
    GTK3_CFLAGS=$(printf '%s' "$GTK3_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
fi
AC_SUBST(GTK3_LIBS)
AC_SUBST(GTK3_CFLAGS)
AC_SUBST(ENABLE_GTK3)

AC_MSG_CHECKING([which VCLplugs shall be built])
if test "$GUIBASE" != "unx" -o "$enable_headless" = "yes"; then
    enable_gtk=no
fi
ENABLE_GTK=""
if test "x$enable_gtk" = "xyes"; then
    ENABLE_GTK="TRUE"
    AC_DEFINE(ENABLE_GTK)
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
    AC_DEFINE(ENABLE_TDE)
    R="$R tde"
fi
AC_SUBST(ENABLE_TDE)

ENABLE_KDE=""
if test "x$enable_kde" = "xyes"; then
    ENABLE_KDE="TRUE"
    AC_DEFINE(ENABLE_KDE)
    R="$R kde"
fi
AC_SUBST(ENABLE_KDE)

ENABLE_KDE4=""
if test "x$enable_kde4" = "xyes"; then
    ENABLE_KDE4="TRUE"
    AC_DEFINE(ENABLE_KDE4)
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

ENABLE_GCONF=
AC_MSG_CHECKING([whether to enable GConf support])
if test "$_os" != "WINNT" -a "$_os" != "Darwin" -a "$_os" != "iOS" -a "$enable_gconf" = "yes"; then
    ENABLE_GCONF="TRUE"
    AC_MSG_RESULT([yes])
    PKG_CHECK_MODULES( GCONF, gconf-2.0 gobject-2.0 )
    GCONF_CFLAGS=$(printf '%s' "$GCONF_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
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
    GNOMEVFS_CFLAGS=$(printf '%s' "$GNOMEVFS_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
    AC_DEFINE(ENABLE_GNOME_VFS)
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
    DBUS_CFLAGS=$(printf '%s' "$DBUS_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
else
    AC_MSG_RESULT([no])
fi

AC_SUBST(ENABLE_DBUS)
AC_SUBST(DBUS_CFLAGS)
AC_SUBST(DBUS_LIBS)

AC_MSG_CHECKING([whether to enable font install via packagekit])
if test "$ENABLE_DBUS" = "TRUE"; then
    if test -n "$enable_packagekit" -a "$enable_packagekit" != "no"; then
        ENABLE_PACKAGEKIT=TRUE
        AC_MSG_RESULT([yes])
    else
        ENABLE_PACKAGEKIT=
        AC_MSG_RESULT([no])
    fi
else
    AC_MSG_RESULT([no, dbus disabled.])
fi
AC_SUBST(ENABLE_PACKAGEKIT)

AC_MSG_CHECKING([whether to enable Impress remote control])
if test -n "$enable_sdremote" -a "$enable_sdremote" != "no"; then
    AC_MSG_RESULT([yes])
    ENABLE_SDREMOTE=TRUE
    AC_MSG_CHECKING([whether to enable Bluetooth support in Impress remote control])

    # If not explicitly enabled or disabled, default
    if test -z "$enable_sdremote_bluetooth"; then
        case "$OS" in
        LINUX|MACOSX|WNT)
            # Default to yes for these
            enable_sdremote_bluetooth=yes
            ;;
        *)
            # otherwise no
            enable_sdremote_bluetooth=no
            ;;
        esac
    fi
    # $enable_sdremote_bluetooth is guaranteed non-empty now

    if test "$enable_sdremote_bluetooth" != "no"; then
        if test "$OS" = "LINUX"; then
            if test "$ENABLE_DBUS" = "TRUE"; then
                AC_MSG_RESULT([yes])
                ENABLE_SDREMOTE_BLUETOOTH=TRUE
                dnl ===================================================================
                dnl Check for system bluez
                dnl ===================================================================
                AC_MSG_CHECKING([which Bluetooth header to use])
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
                ENABLE_SDREMOTE_BLUETOOTH=
            fi
        else
            AC_MSG_RESULT([yes])
            ENABLE_SDREMOTE_BLUETOOTH=TRUE
        fi
    else
        AC_MSG_RESULT([no])
        ENABLE_SDREMOTE_BLUETOOTH=
    fi
else
    ENABLE_SDREMOTE=
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_SDREMOTE)
AC_SUBST(ENABLE_SDREMOTE_BLUETOOTH)

dnl ===================================================================
dnl Check whether the gtk 2.0 libraries are available.
dnl ===================================================================

GTK_CFLAGS=""
GTK_LIBS=""
ENABLE_SYSTRAY_GTK=""
if test  "$test_gtk" = "yes"; then

    if test "$ENABLE_GTK" = "TRUE"; then
        PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.4 gdk-pixbuf-xlib-2.0 >= 2.2 ,,AC_MSG_ERROR([requirements to build the gtk-plugin not met. Use --disable-gtk or install the missing packages]))
        GTK_CFLAGS=$(printf '%s' "$GTK_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
        PKG_CHECK_MODULES(GTHREAD, gthread-2.0,,AC_MSG_ERROR([requirements to build the gtk-plugin not met. Use --disable-gtk or install the missing packages]))
        BUILD_TYPE="$BUILD_TYPE GTK"
        GTHREAD_CFLAGS=$(printf '%s' "$GTK_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")

    AC_MSG_CHECKING([whether to enable the systray quickstarter])
        if test "x$enable_systray" = "xyes"; then
            AC_MSG_RESULT([yes])
            PKG_CHECK_MODULES( GTK210, gtk+-2.0 >= 2.10.0,
                            [ ENABLE_SYSTRAY_GTK="TRUE" ],
                            [ ENABLE_SYSTRAY_GTK="" ])
        else
            AC_MSG_RESULT([no])
        fi

        AC_MSG_CHECKING([whether to enable Gtk print dialog support])
        PKG_CHECK_MODULES([GTK_PRINT], [gtk+-unix-print-2.0 >= 2.10.0],
                          [ENABLE_GTK_PRINT="TRUE"],
                          [ENABLE_GTK_PRINT=""])
        GTK_PRINT_CFLAGS=$(printf '%s' "$GTK_PRINT_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")

        AC_MSG_CHECKING([whether to enable GIO support])
        if test "$_os" != "WINNT" -a "$_os" != "Darwin" -a "$enable_gio" = "yes"; then
            if test "$ENABLE_GNOMEVFS" = "TRUE"; then
                AC_MSG_ERROR([please use --enable-gio only together with --disable-gnome-vfs.])
            fi
            dnl Need at least 2.26 for the dbus support.
            PKG_CHECK_MODULES([GIO], [gio-2.0 >= 2.26],
                              [ENABLE_GIO="TRUE"], [ENABLE_GIO=""])
            if test "$ENABLE_GIO" = "TRUE"; then
                AC_DEFINE(ENABLE_GIO)
                GIO_CFLAGS=$(printf '%s' "$GIO_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
            fi
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
    AC_DEFINE(ENABLE_TELEPATHY)
    AC_MSG_RESULT([yes])
    PKG_CHECK_MODULES( TELEPATHY, telepathy-glib >= 0.18.0 glib-2.0 gobject-2.0 gthread-2.0 gio-2.0 gtk+-2.0 >= 2.4 )
    TELEPATHY_CFLAGS=$(printf '%s' "$TELEPATHY_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
else
    AC_MSG_RESULT([no])
fi

AC_SUBST(ENABLE_TELEPATHY)
AC_SUBST(TELEPATHY_CFLAGS)
AC_SUBST(TELEPATHY_LIBS)


dnl ===================================================================

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

if test $_os = Darwin -o $_os = WINNT -o $_os = iOS; then
    if test "$enable_cairo_canvas" = yes; then
        AC_MSG_ERROR([The cairo canvas should not be used for this platform])
    fi
    enable_cairo_canvas=no
elif test -z "$enable_cairo_canvas"; then
    enable_cairo_canvas=yes
fi

ENABLE_CAIRO_CANVAS=""
if test "$enable_cairo_canvas" = "yes"; then
    test_cairo=yes
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
        GSTREAMER_CFLAGS=$(printf '%s' "$GSTREAMER_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
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
        GSTREAMER_0_10_CFLAGS=$(printf '%s' "$GSTREAMER_0_10_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
    else
        AC_MSG_RESULT([no])
    fi
fi

AC_SUBST(GSTREAMER_0_10_CFLAGS)
AC_SUBST(GSTREAMER_0_10_LIBS)
AC_SUBST(ENABLE_GSTREAMER_0_10)

dnl ===================================================================
dnl Check whether to build the VLC avmedia backend
dnl ===================================================================

ENABLE_VLC=""

AC_MSG_CHECKING([whether to enable the VLC avmedia backend])
if test $_os != iOS -a $_os != Android -a "$enable_vlc" = yes; then
    ENABLE_VLC="TRUE"
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_VLC)

dnl ===================================================================
dnl Check whether the OpenGL libraries are available
dnl ===================================================================

AC_MSG_CHECKING([whether to build the OpenGL Transitions component])
ENABLE_OPENGL=

if test -z "$enable_opengl"; then
    if test $_os = WINNT; then
        # By default disable the OpenGL transitions for Windows (the code compiles but works very
        # badly and crashes). In other words, use --enable-opengl only if you plan to hack on that
        # code for Windows.
        enable_opengl=no
    elif test $_os = iOS; then
        # As such with some suitable minor tweaks the Mac OpenGL transitions code would presumably
        # build fine for iOS, too, but let's leave that for later
        enable_opengl=no
    elif test "$enable_headless" = "yes"; then
        enable_opengl=no
    else
        enable_opengl=yes
    fi
fi

if test "x$enable_opengl" = "xno"; then
    AC_MSG_RESULT([no])
elif test "$_os" = "Darwin"; then
    # We use frameworks on Mac OS X, no need for detail checks
    ENABLE_OPENGL=TRUE
    SYSTEM_MESA_HEADERS=YES
    AC_MSG_RESULT([yes])
elif test $_os = WINNT; then
    # Experimental: try to use OpenGL on Windows
    ENABLE_OPENGL=TRUE
    # We need the internal "Mesa" headers.
    SYSTEM_MESA_HEADERS=NO
    BUILD_TYPE="$BUILD_TYPE MESA"
    AC_MSG_RESULT([yes])
else
    save_LDFLAGS=$LDFLAGS
    LDFLAGS="$LDFLAGS -lm"
    AC_MSG_RESULT([yes])
    AC_CHECK_LIB([GL], [main], [:],
              [AC_MSG_ERROR(libGL not installed or functional)], [])
    LDFLAGS="$LDFLAGS -lGL"
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
fi

AC_SUBST(SYSTEM_MESA_HEADERS)
AC_SUBST(ENABLE_OPENGL)

dnl =================================================
dnl Check whether the OpenCL libraries are available
dnl =================================================

OPENCL_LIBS=
OPENCL_CFLAGS=
ENABLE_OPENCL=
AC_MSG_CHECKING([OpenCL])
if test \( -z "$with_opencl_sdk" -o "$with_opencl_sdk" = yes \) -a $_os = Darwin -a "$with_macosx_sdk" != 10.6; then
    # OS X SDK > 10.6
    AC_MSG_RESULT([yes, always on OS X > 10.6])
    ENABLE_OPENCL=TRUE
    OPENCL_CFLAGS=
    OPENCL_LIBS="-framework OpenCL"
    AC_DEFINE(HAVE_FEATURE_OPENCL)
elif test -z "$with_opencl_sdk" -o "$with_opencl_sdk" = no; then
    AC_MSG_RESULT([no])
else
    if test -d "$with_opencl_sdk/include"; then
        ENABLE_OPENCL=TRUE
        if test "$_os" = "WINNT"; then
            PathFormat "$with_opencl_sdk"
            OPENCL_CFLAGS="-I$formatted_path/include"
            OPENCL_LIBS="-LIBPATH:$formatted_path/lib/x86 OpenCL.lib"
        else
            OPENCL_CFLAGS="-I$with_opencl_sdk/include"
            OPENCL_LIBS="-L$with_opencl_sdk/lib/x86 -lOpenCL"
        fi
        AC_MSG_RESULT([found at path $with_opencl_sdk])
        AC_DEFINE(HAVE_FEATURE_OPENCL)
    else
        AC_MSG_ERROR([no headers found found at $with_opencl_sdk/include ])
    fi
fi
AC_SUBST(OPENCL_CFLAGS)
AC_SUBST(OPENCL_LIBS)
AC_SUBST(ENABLE_OPENCL)

# presenter minimizer extension?
AC_MSG_CHECKING([whether to build the Presentation Minimizer extension])
if test "x$enable_ext_presenter_minimizer" != "xno" -a "x$enable_extension_integration" != "xno"; then
    AC_MSG_RESULT([yes])
    ENABLE_MINIMIZER=TRUE
    SCPDEFS="$SCPDEFS -DWITH_EXTENSION_MINIMIZER"
else
    AC_MSG_RESULT([no])
    ENABLE_MINIMIZER=
fi
AC_SUBST(ENABLE_MINIMIZER)

# pdf import?
AC_MSG_CHECKING([whether to build the PDF import])
if test "$_os" != Android -a "$_os" != iOS -a "$ENABLE_PDFIMPORT" != FALSE; then
    AC_MSG_RESULT([yes])
    ENABLE_PDFIMPORT=TRUE

    dnl ===================================================================
    dnl Check for system poppler
    dnl ===================================================================
    AC_MSG_CHECKING([which pdf backend to use])
    if test "$with_system_poppler" = "yes"; then
        AC_MSG_RESULT([external])
        SYSTEM_POPPLER=YES
        PKG_CHECK_MODULES( POPPLER, poppler >= 0.8.0 )
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
        libo_MINGW_CHECK_DLL([libpoppler])
        POPPLER_CFLAGS=$(printf '%s' "$POPPLER_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
    else
        AC_MSG_RESULT([internal])
        SYSTEM_POPPLER=NO
        BUILD_TYPE="$BUILD_TYPE POPPLER"
    fi
else
    AC_MSG_RESULT([no])
    ENABLE_PDFIMPORT=FALSE
fi
AC_SUBST(ENABLE_PDFIMPORT)
AC_SUBST(SYSTEM_POPPLER)
AC_SUBST(POPPLER_CFLAGS)
AC_SUBST(POPPLER_LIBS)

AC_MSG_CHECKING([whether to build the Wiki Publisher extension])
if test "x$enable_ext_wiki_publisher" = "xyes" -a "x$enable_extension_integration" != "xno" -a "$with_java" != "no"; then
    AC_MSG_RESULT([yes])
    ENABLE_MEDIAWIKI=TRUE
    BUILD_TYPE="$BUILD_TYPE XSLTML"
    if test  "x$with_java" = "xno"; then
        AC_MSG_ERROR([Wiki Publisher requires Java! Enable Java if you want to build it.])
    fi
else
    AC_MSG_RESULT([no])
    ENABLE_MEDIAWIKI=
    SCPDEFS="$SCPDEFS -DWITHOUT_EXTENSION_MEDIAWIKI"
fi
AC_SUBST(ENABLE_MEDIAWIKI)

if test "$ENABLE_MEDIAWIKI" = "TRUE"; then
    AC_MSG_CHECKING([which Servlet API Jar to use])
    if test "$with_system_servlet_api" = "yes"; then
        AC_MSG_RESULT([external])
        SYSTEM_SERVLETAPI=YES
        if test -z "$SERVLETAPI_JAR"; then
            for version in '' -3.3 -3.2 -3.1 -3.0 -2.5 -2.4; do
                if test -r "/usr/share/java/servlet-api${version}.jar"; then
                    SERVLETAPI_JAR=/usr/share/java/servlet-api${version}.jar
                fi
            done
        fi
        AC_CHECK_FILE($SERVLETAPI_JAR, [],
              [AC_MSG_ERROR(servlet-api.jar not found.)], [])
    else
        AC_MSG_RESULT([internal])
        SYSTEM_SERVLETAPI=NO
        BUILD_TYPE="$BUILD_TYPE TOMCAT"
    fi
fi
AC_SUBST(SYSTEM_SERVLETAPI)
AC_SUBST(SERVLETAPI_JAR)

AC_MSG_CHECKING([whether to build the Report Builder extension])
if test "$enable_report_builder" != "no" -a "$with_java" != "no"; then
    AC_MSG_RESULT([yes])
    ENABLE_REPORTBUILDER=TRUE
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
    fi
else
    AC_MSG_RESULT([no])
    ENABLE_REPORTBUILDER=
    SYSTEM_JFREEREPORT=NO
fi
AC_SUBST(ENABLE_REPORTBUILDER)
AC_SUBST(SYSTEM_JFREEREPORT)
AC_SUBST(SAC_JAR)
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
if test "$ENABLE_MEDIAWIKI" = "TRUE" -o "$ENABLE_REPORTBUILDER" = "TRUE"; then
    AC_MSG_CHECKING([which Apache commons-* libs to use])
    if test "$with_system_apache_commons" = "yes"; then
        SYSTEM_APACHE_COMMONS=YES
        AC_MSG_RESULT([external])
        if test "$ENABLE_MEDIAWIKI" = "TRUE"; then
            if test -z $COMMONS_CODEC_JAR; then
                AC_CHECK_FILE(/usr/share/java/commons-codec-1.6.jar,
                    [ COMMONS_CODEC_JAR=/usr/share/java/commons-codec-1.6.jar ],
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
                AC_CHECK_FILE(/usr/share/java/commons-lang-2.4.jar,
                    [ COMMONS_LANG_JAR=/usr/share/java/commons-lang-2.4.jar ],
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
        if test "$ENABLE_MEDIAWIKI" = "TRUE" -o "$ENABLE_REPORTBUILDER" = "TRUE"; then
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
        BUILD_TYPE="$BUILD_TYPE APACHE_COMMONS TOMCAT"
    fi
fi
AC_SUBST(SYSTEM_APACHE_COMMONS)
AC_SUBST(COMMONS_CODEC_JAR)
AC_SUBST(COMMONS_LANG_JAR)
AC_SUBST(COMMONS_HTTPCLIENT_JAR)
AC_SUBST(COMMONS_LOGGING_JAR)

# scripting provider for BeanShell?
AC_MSG_CHECKING([whether to build support for scripts in BeanShell])
if test "${enable_scripting_beanshell}" != "no" -a "x$with_java" != "xno"; then
   AC_MSG_RESULT([yes])
   ENABLE_SCRIPTING_BEANSHELL=TRUE

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
       BUILD_TYPE="$BUILD_TYPE BSH"
   fi
else
   AC_MSG_RESULT([no])
   ENABLE_SCRIPTING_BEANSHELL=
   SCPDEFS="$SCPDEFS -DWITHOUT_SCRIPTING_BEANSHELL"
fi
AC_SUBST(ENABLE_SCRIPTING_BEANSHELL)
AC_SUBST(SYSTEM_BSH)
AC_SUBST(BSH_JAR)

# scripting provider for JavaScript?
AC_MSG_CHECKING([whether to build support for scripts in JavaScript])
if test "${enable_scripting_javascript}" != "no" -a "x$with_java" != "xno"; then
   AC_MSG_RESULT([yes])
   ENABLE_SCRIPTING_JAVASCRIPT=TRUE

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
       BUILD_TYPE="$BUILD_TYPE RHINO"
   fi
else
   AC_MSG_RESULT([no])
   ENABLE_SCRIPTING_JAVASCRIPT=
   SCPDEFS="$SCPDEFS -DWITHOUT_SCRIPTING_JAVASCRIPT"
fi
AC_SUBST(ENABLE_SCRIPTING_JAVASCRIPT)
AC_SUBST(SYSTEM_RHINO)
AC_SUBST(RHINO_JAR)

supports_multilib=
case "$host_cpu" in
x86_64 | powerpc64 | s390x)
    if test "$SAL_TYPES_SIZEOFLONG" = "8"; then
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
    TDE_LIBS="-L$tde_libdir -L$tqt_libdir -ltdeio -ltdeui -ltdecore -ltqt -ltqt-mt"
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
    KDE_CFLAGS=$(printf '%s' "$KDE_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
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
    if test "$QMAKE4" = "no"; then
        AC_PATH_PROG( QMAKE4, qmake, no, [`dirname $qt_header_dir`/bin:$QT4DIR/bin:$PATH] )
        if test "$QMAKE4" = "no"; then
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
    KDE4_CFLAGS=$(printf '%s' "$KDE4_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")

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

    # Glib is needed for properly handling Qt event loop with Qt's Glib integration enabled.
    # Sets also KDE_GLIB_CFLAGS/KDE_GLIB_LIBS if successful.
    PKG_CHECK_MODULES(KDE_GLIB,[glib-2.0 >= 2.4],
        [
        KDE_HAVE_GLIB=1
        AC_DEFINE(KDE_HAVE_GLIB,1)
        KDE_GLIB_CFLAGS=$(printf '%s' "$KDE_GLIB_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
        ],
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
ENABLE_LOCKDOWN=
if test -n "$enable_lockdown" -a "$enable_lockdown" != "no"; then
    ENABLE_LOCKDOWN=TRUE
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
    GOBJECT_CFLAGS=$(printf '%s' "$GOBJECT_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
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
dnl Test which themes to include
dnl ===================================================================
AC_MSG_CHECKING([which themes to include])
# if none given use default subset of available themes
if test "x$with_theme" = "x" -o "x$with_theme" = "xyes"; then
    with_theme="crystal default hicontrast oxygen tango sifr"
    test -z "$ENABLE_RELEASE_BUILD" && with_theme="$with_theme tango_testing"
fi

WITH_THEMES=""
if test "x$with_theme" != "xno"; then
    for theme in $with_theme; do
        case $theme in
            crystal|default|galaxy|hicontrast|human|industrial|oxygen|sifr|tango|tango_testing) : ;;
            *) AC_MSG_ERROR([Unknown value for --with-theme: $theme]) ;;
        esac
        WITH_THEMES="$WITH_THEMES $theme"
    done
fi
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
ENABLE_CORETEXT=
if test "$_os" = "Darwin"; then
    AC_MSG_CHECKING([whether to use CoreText framework])
    if test "$enable_coretext" != no -o "$BITNESS_OVERRIDE" = 64; then
        AC_MSG_RESULT([yes])
        ENABLE_CORETEXT=TRUE
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_SUBST(ENABLE_CORETEXT)

###############################################################################
# Extensions checking
###############################################################################
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

libo_CHECK_EXTENSION([Watch Window],[WATCH_WINDOW],[watch_window],[watch-window],[23bd75552206dfcd8fd4e29137dcac84-WatchWindow_1.2.0.0.oxt])
libo_CHECK_EXTENSION([SmART Gallery (Diagram)],[DIAGRAM],[diagram],[diagram],[8d74685d41f8bffe8c3e71fe8deac09d-SmART_0.9.5.oxt])
libo_CHECK_EXTENSION([Validator],[VALIDATOR],[validator],[validator],[bbdd5639ada63e3130761daaecae1a10-Validator_1.1.0.0.oxt])
libo_CHECK_EXTENSION([Barcode],[BARCODE],[barcode],[barcode],[3ed18025a766f1e955707b969c8113a5-Barcode_1.3.5.0.oxt])
libo_CHECK_EXTENSION([ConvertTextToNumber],[CT2N],[ct2n],[ct2n],[])
libo_CHECK_EXTENSION([Numbertext],[NUMBERTEXT],[numbertext],[numbertext],[b7cae45ad2c23551fd6ccb8ae2c1f59e-numbertext_0.9.5.oxt])
libo_CHECK_EXTENSION([Hungarian Cross-reference Toolbar],[HUNART],[hunart],[hunart],[b632bdd25649cc4067bcb410bae23d2b-hunart_0.3.oxt])
libo_CHECK_EXTENSION([Typography Toolbar],[TYPO],[typo],[typo],[9d60b6cfa3ef1926848710bbcd11115b-typo_0.4.2.oxt])
libo_CHECK_EXTENSION([Google Docs],[GOOGLE_DOCS],[google_docs],[google-docs],[27211596cf0ad97cab7321239406fde0-gdocs_3.0.1_modified.oxt])
if test "x$with_java" != "xno"; then
    libo_CHECK_EXTENSION([NLPSolver],[NLPSOLVER],[nlpsolver],[nlpsolver],[])
    libo_CHECK_EXTENSION([LanguageTool],[LANGUAGETOOL],[languagetool],[languagetool],[])
fi

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
else
    AC_MSG_RESULT([no])
    WITH_FONTS=NO
    SCPDEFS="$SCPDEFS -DWITHOUT_FONTS"
fi
AC_SUBST(WITH_FONTS)

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
else
    AC_MSG_RESULT([no])
    WITHOUT_AFMS=YES
    SCPDEFS="$SCPDEFS -DWITHOUT_AFMS"
fi
AC_SUBST(WITHOUT_AFMS)

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
    ENABLE_SILENT_MSI=
else
    AC_MSG_RESULT([yes])
    ENABLE_SILENT_MSI=TRUE
    SCPDEFS="$SCPDEFS -DENABLE_SILENT_MSI"
fi
AC_SUBST(ENABLE_SILENT_MSI)

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
dnl Test whether to build cairo or rely on the system version
dnl ===================================================================

if test "$GUIBASE" = "unx"; then
    # Used in vcl/Library_vclplug_gen.mk
    test_cairo=yes
fi

if test "$test_cairo" = "yes"; then
    AC_MSG_CHECKING([whether to use the system cairo])

    if test "$with_system_cairo" = "yes"; then
        SYSTEM_CAIRO=YES
        AC_MSG_RESULT([yes])

        PKG_CHECK_MODULES( CAIRO, cairo >= 1.0.2 )
        CAIRO_CFLAGS=$(printf '%s' "$CAIRO_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")

        if test "$test_xrender" = "yes"; then
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
        libo_MINGW_CHECK_DLL([libcairo])
        libo_MINGW_TRY_DLL([libfontconfig])
        libo_MINGW_TRY_DLL([libfreetype])
        libo_MINGW_TRY_DLL([libpixman])
        libo_MINGW_TRY_DLL([libpng15])
    else
        SYSTEM_CAIRO=NO
        AC_MSG_RESULT([no])

        BUILD_TYPE="$BUILD_TYPE CAIRO"
        SCPDEFS="$SCPDEFS -DNEED_CAIRO"
    fi
fi

AC_SUBST(SYSTEM_CAIRO)
AC_SUBST(CAIRO_CFLAGS)
AC_SUBST(CAIRO_LIBS)

dnl ===================================================================
dnl Test whether to use avahi
dnl ===================================================================
if test "$_os" != "WINNT" -a "$_os" != "Darwin" -a "$enable_avahi" = "yes"; then
    PKG_CHECK_MODULES([AVAHI], [avahi-client >= 0.6.10],
                      [ENABLE_AVAHI="TRUE"], [ENABLE_AVAHI=""])
    if test "$ENABLE_AVAHI" = "TRUE"; then
        AC_DEFINE(HAVE_FEATURE_AVAHI)
        AVAHI_CFLAGS=$(printf '%s' "$AVAHI_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
    fi
fi

AC_SUBST(ENABLE_AVAHI)
AC_SUBST(AVAHI_CFLAGS)
AC_SUBST(AVAHI_LIBS)

dnl ===================================================================
dnl Test whether to use liblangtag
dnl ===================================================================
ENABLE_LIBLANGTAG=
SYSTEM_LIBLANGTAG=
AC_MSG_CHECKING([whether to use liblangtag])
if test "$enable_liblangtag" = "yes" -o \( "$enable_liblangtag" = "" -a $_os != Android -a $_os != iOS \); then
    ENABLE_LIBLANGTAG=TRUE
    AC_MSG_RESULT([yes])
    AC_MSG_CHECKING([whether to use system liblangtag])
    if test "$with_system_liblangtag" = yes; then
        SYSTEM_LIBLANGTAG=YES
        AC_MSG_RESULT([yes])
        PKG_CHECK_MODULES( LIBLANGTAG, liblangtag >= 0.4.0)
        LIBLANGTAG_CFLAGS=$(printf '%s' "$LIBLANGTAG_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
        libo_MINGW_CHECK_DLL([liblangtag])
    else
        SYSTEM_LIBLANGTAG=NO
        AC_MSG_RESULT([no])
        BUILD_TYPE="$BUILD_TYPE LIBLANGTAG"
    fi
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_LIBLANGTAG)
AC_SUBST(SYSTEM_LIBLANGTAG)
AC_SUBST(LIBLANGTAG_CFLAGS)
AC_SUBST(LIBLANGTAG_LIBS)

dnl ===================================================================
dnl Test whether to build libpng or rely on the system version
dnl ===================================================================

libo_CHECK_SYSTEM_MODULE([libpng],[LIBPNG],[libpng],["-I${WORKDIR}/UnpackedTarball/png"],["-L${OUTDIR}/lib -lpng"])

dnl ===================================================================
dnl Check for runtime JVM search path
dnl ===================================================================
if test "$ENABLE_JAVA" != ""; then
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

if test "$ENABLE_JAVA" != ""; then
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

    if test "$ENABLE_MEDIAWIKI" = "TRUE"; then
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
if test "$ENABLE_JAVA" != "" -a "$with_junit" != "no"; then
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


AC_SUBST(SCPDEFS)

#
# check for wget and md5sum
#
WGET=
MD5SUM=
CURL=

if test "$enable_fetch_external" != "no"; then

CURL=`which curl 2>/dev/null`

for i in wget /usr/bin/wget /usr/local/bin/wget /usr/sfw/bin/wget /opt/sfw/bin/wget /opt/local/bin/wget; do
    eval "$i --version" > /dev/null 2>&1
    ret=$?
    if test $ret -eq 0; then
        WGET=$i
        break
    fi
done

if test -z "$WGET" -a -z "$CURL"; then
    AC_MSG_ERROR([neither wget nor curl found!])
fi

for i in md5 md5sum /usr/local/bin/md5sum gmd5sum /usr/sfw/bin/md5sum /opt/sfw/bin/gmd5sum /opt/local/bin/md5sum; do
    if test "$i" = "md5"; then
        eval "$i -x" > /dev/null 2>&1
    else
        eval "$i --version" > /dev/null 2>&1
    fi
    ret=$?
    if test $ret -eq 0; then
        MD5SUM=$i
        break
    fi
done

if test "$MD5SUM" = "md5"; then
    if md5 -r < /dev/null > /dev/null 2>/dev/null; then
        MD5SUM="$MD5SUM -r"
    elif md5 -n < /dev/null > /dev/null 2>/dev/null; then
        MD5SUM="$MD5SUM -n"
    fi
fi

if test -z "$MD5SUM"; then
    AC_MSG_ERROR([no md5sum: found!])
fi

fi

AC_SUBST(WGET)
AC_SUBST(CURL)
AC_SUBST(MD5SUM)

dnl ===================================================================
dnl Product version
dnl ===================================================================
AC_MSG_CHECKING([for product version])
PRODUCTVERSION="$LIBO_VERSION_MAJOR.$LIBO_VERSION_MINOR"
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
[eval $(sed -e :a -e '/\\$/N; s/\\\n//; ta' -n -e 's/=/="/;s/\([^\\]\)$/\1"/;s/en-US//;/^completelangiso/p' $SRC_ROOT/solenv/inc/langlist.mk)]
ALL_LANGS="en-US $completelangiso"
# check the configured localizations
WITH_LANG="$with_lang"
if test -z "$WITH_LANG" -o "$WITH_LANG" = "en-US"; then
    AC_MSG_RESULT([en-US])
else
    AC_MSG_RESULT([$WITH_LANG])
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
        # if you're reading this - you prolly quoted your languages remove the quotes ...
        AC_MSG_ERROR([invalid language: '$lang' (vs '$v1'); supported languages are: $ALL_LANGS])
    fi
done
if test -n "$WITH_LANG" -a "$WITH_LANG" != "ALL"; then
    echo $WITH_LANG | grep -q en-US
    test $? -ne 1 || WITH_LANG=`echo $WITH_LANG en-US`
fi
# list with substituted ALL
WITH_LANG_LIST=`echo $WITH_LANG | sed "s/ALL/$ALL_LANGS/"`
test -z "$WITH_LANG_LIST" && WITH_LANG_LIST="en-US"
test "$WITH_LANG" = "en-US" && WITH_LANG=
if test "$enable_release_build" = "" -o "$enable_release_build" = "no"; then
    test "$WITH_LANG_LIST" = "en-US" || WITH_LANG_LIST=`echo $WITH_LANG_LIST qtz`
    ALL_LANGS=`echo $ALL_LANGS qtz`
fi
AC_SUBST(ALL_LANGS)
AC_SUBST(WITH_LANG)
AC_SUBST(WITH_LANG_LIST)
AC_SUBST(GIT_NEEDED_SUBMODULES)

WITH_POOR_HELP_LOCALIZATIONS=
if test -d "$SRC_ROOT/translations/source"; then
    for l in `ls -1 $SRC_ROOT/translations/source`; do
        if test ! -d "$SRC_ROOT/translations/source/$l/helpcontent2"; then
            WITH_POOR_HELP_LOCALIZATIONS="$WITH_POOR_HELP_LOCALIZATIONS $l"
        fi
    done
fi
AC_SUBST(WITH_POOR_HELP_LOCALIZATIONS)

dnl git submodule update --reference
dnl ===================================================================
if test -n "${GIT_REFERENCE_SRC}"; then
    for repo in ${GIT_NEEDED_SUBMODULES}; do
        if ! test -d "${GIT_REFERENCE_SRC}"/${repo}; then
            AC_MSG_ERROR([referenced git: required repository does not exist: ${GIT_REFERENCE_SRC}/${repo}])
        fi
    done
fi
AC_SUBST(GIT_REFERENCE_SRC)

dnl git submodules linked dirs
dnl ===================================================================
if test -n "${GIT_LINK_SRC}"; then
    for repo in ${GIT_NEEDED_SUBMODULES}; do
        if ! test -d "${GIT_LINK_SRC}"/${repo}; then
            AC_MSG_ERROR([linked git: required repository does not exist: ${GIT_LINK_SRC}/${repo}])
        fi
    done
fi
AC_SUBST(GIT_LINK_SRC)

dnl branding
dnl ===================================================================
AC_MSG_CHECKING([for alternative branding images directory])
# initialize mapped arrays
BRAND_INTRO_IMAGES="flat_logo.svg intro.png"
brand_files="$BRAND_INTRO_IMAGES about.svg \
    backing_left.png backing_right.png \
    backing_rtl_left.png backing_rtl_right.png \
    backing_space.png"

if test -z "$with_branding" -o "$with_branding" = "no"; then
    AC_MSG_RESULT([none])
    DEFAULT_BRAND_IMAGES="$brand_files"
else
    if ! test -d $with_branding ; then
        AC_MSG_ERROR([No directory $with_branding, falling back to default branding])
    else
        AC_MSG_RESULT([$with_branding])
        CUSTOM_BRAND_DIR="$with_branding"
        for lfile in $brand_files
        do
            if ! test -f $with_branding/$lfile ; then
                AC_MSG_WARN([Branded file $lfile does not exist, using the default one])
                DEFAULT_BRAND_IMAGES="$DEFAULT_BRAND_IMAGES $lfile"
            else
                CUSTOM_BRAND_IMAGES="$CUSTOM_BRAND_IMAGES $lfile"
            fi
        done
        check_for_progress="yes"
    fi
fi
AC_SUBST([BRAND_INTRO_IMAGES])
AC_SUBST([CUSTOM_BRAND_DIR])
AC_SUBST([CUSTOM_BRAND_IMAGES])
AC_SUBST([DEFAULT_BRAND_IMAGES])


AC_MSG_CHECKING([for 'intro' progress settings])
PROGRESSBARCOLOR=
PROGRESSSIZE=
PROGRESSPOSITION=
PROGRESSFRAMECOLOR=
PROGRESSTEXTCOLOR=
PROGRESSTEXTBASELINE=

if test "$check_for_progress" = "yes" -a -f "$with_branding/progress.conf" ; then
    source "$with_branding/progress.conf"
    AC_MSG_RESULT([settings found in $with_branding/progress.conf])
else
    AC_MSG_RESULT([none])
fi

AC_SUBST(PROGRESSBARCOLOR)
AC_SUBST(PROGRESSSIZE)
AC_SUBST(PROGRESSPOSITION)
AC_SUBST(PROGRESSFRAMECOLOR)
AC_SUBST(PROGRESSTEXTCOLOR)
AC_SUBST(PROGRESSTEXTBASELINE)


AC_MSG_CHECKING([for extra build ID])
if test -n "$with_extra_buildid" -a "$with_extra_buildid" != "yes" ; then
    EXTRA_BUILDID="$with_extra_buildid"
fi
# in tinderboxes, it is easier to set EXTRA_BUILDID via the environment variable instead of configure switch
if test -n "$EXTRA_BUILDID" ; then
    AC_MSG_RESULT([$EXTRA_BUILDID])
else
    AC_MSG_RESULT([not set])
fi
AC_DEFINE_UNQUOTED([EXTRA_BUILDID], ["$EXTRA_BUILDID"])

OOO_VENDOR=
AC_MSG_CHECKING([for vendor])
if test -z "$with_vendor" -o "$with_vendor" = "no"; then
    OOO_VENDOR="$USERNAME"

    if test -z "$OOO_VENDOR"; then
        OOO_VENDOR="$USER"
    fi

    if test -z "$OOO_VENDOR"; then
        OOO_VENDOR="`id -u -n`"
    fi

    AC_MSG_RESULT([not set, using $OOO_VENDOR])
else
    OOO_VENDOR="$with_vendor"
    AC_MSG_RESULT([$OOO_VENDOR])
fi
AC_SUBST(OOO_VENDOR)

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
if test "$enable_release_build" = "" -o "$enable_release_build" = "no"; then
    PRODUCTNAME=AC_PACKAGE_NAME"Dev"
else
    PRODUCTNAME=AC_PACKAGE_NAME
fi
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

LIBDIR=[$(eval echo $(eval echo $libdir))]
AC_SUBST(LIBDIR)

DATADIR=[$(eval echo $(eval echo $datadir))]
AC_SUBST(DATADIR)

MANDIR=[$(eval echo $(eval echo $mandir))]
AC_SUBST(MANDIR)

DOCDIR=[$(eval echo $(eval echo $docdir))]
AC_SUBST(DOCDIR)

INSTALLDIR="$LIBDIR/$INSTALLDIRNAME"
AC_SUBST(INSTALLDIR)

DEVINSTALLDIR="$OUTDIR/installation"
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
# plain --with-parallelism is just the default
if test -n "$with_parallelism" -a "$with_parallelism" != "yes"; then
    if test "$with_parallelism" = "no"; then
        PARALLELISM=1
    else
        PARALLELISM=$with_parallelism
    fi
else
    if test "$enable_icecream" = "yes"; then
        PARALLELISM="10"
    else
        case `uname -s` in

        Darwin|FreeBSD|NetBSD|OpenBSD)
            PARALLELISM=`sysctl -n hw.ncpu`
            ;;

        Linux)
            PARALLELISM=`getconf _NPROCESSORS_ONLN`
        ;;
        # what else than above does profit here *and* has /proc?
        *)
            PARALLELISM=`grep $'^processor\t*:' /proc/cpuinfo | wc -l`
            ;;
        esac

        # If we hit the catch-all case, but /proc/cpuinfo doesn't exist or has an
        # unexpected format, 'wc -l' will have returned 0.
        if test "$PARALLELISM" -eq 0; then
            PARALLELISM=1
        fi
    fi
fi

if test "$no_parallelism_make" = "YES" && test $PARALLELISM -gt 1; then
    if test -z "$with_parallelism"; then
            AC_MSG_WARN([gmake 3.81 crashes with parallelism > 1, reducing it to 1. upgrade to 3.82 to avoid this.])
            add_warning "gmake 3.81 crashes with parallelism > 1, reducing it to 1. upgrade to 3.82 to avoid this."
            PARALLELISM="1"
    else
        add_warning "make 3.81 is prone to crashes with parallelism > 1. Since --with-parallelism was explicitly given, it is honored, but do not complain when make segfaults on you."
    fi
fi

AC_MSG_RESULT([$PARALLELISM])
AC_SUBST(PARALLELISM)

# ===================================================================
# Number of parallel jobs to be executed during subsequenttest
# ===================================================================
AC_MSG_CHECKING([for maximum no. of jobs during subsequenttest])
CHECK_PARALLELISM=$PARALLELISM
if test "$with_check_jobs" != ""; then
    CHECK_PARALLELISM="$with_check_jobs"
fi
AC_MSG_RESULT([$CHECK_PARALLELISM])
AC_SUBST(CHECK_PARALLELISM)

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
        ILIB="$ILIB;$WINDOWS_SDK_HOME/lib/x64"
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

if test $_os = iOS -o $_os = Android; then
    # Never any point in mergelibs for these as we build just static
    # libraries anyway...
    enable_mergelibs=no
fi

if test -n "$enable_mergelibs" -a "$enable_mergelibs" != "no"; then
    if test $_os != Linux -a $_os != WINNT; then
        add_warning "--enable-mergelibs is not tested for this platform"
    fi
    if test "$enable_mergelibs" = "all"; then
        MERGELIBS="ALL"
        URELIBS="TRUE"
    else
        MERGELIBS="CORE"
    fi
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST([MERGELIBS])
AC_SUBST([URELIBS])

# ===================================================================
# Create hardlinks on deliver instead of copying for smaller size and speed up
# ===================================================================
AC_MSG_CHECKING([whether to create hardlinks for delivering files])
HARDLINKDELIVER=
if test "$enable_hardlink_deliver" = "yes"; then
    HARDLINKDELIVER="TRUE"
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(HARDLINKDELIVER)

dnl ===================================================================
dnl icerun is a wrapper that stops us spawning tens of processes
dnl locally - for tools that can't be executed on the compile cluster
dnl this avoids a dozen javac's ganging up on your laptop to kill it.
dnl ===================================================================
AC_MSG_CHECKING([whether to use icerun wrapper])
ICECREAM_RUN=
if test "$enable_icecream" = "yes" && which icerun >/dev/null 2>&1 ; then
    ICECREAM_RUN=icerun
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(ICECREAM_RUN)

dnl ===================================================================

AC_MSG_CHECKING([MPL subset])
MPL_SUBSET=

if test "$enable_mpl_subset" = "yes"; then
   warn_report=false
   if test "$enable_report_builder" != "no" -a "$with_java" != "no"; then
        warn_report=true
   elif test "$ENABLE_REPORTBUILDER" = "TRUE"; then
        warn_report=true
   fi
   if test "$warn_report" = "true"; then
        AC_MSG_ERROR([need to --disable-report-builder - extended database report builder.])
   fi
   if test "x$enable_postgresql_sdbc" != "xno"; then
        AC_MSG_ERROR([need to --disable-postgresql-sdbc - the postgress database backend.])
   fi
   if test "$enable_lotuswordpro" = "yes"; then
        AC_MSG_ERROR([need to --disable-lotuswordpro - a Lotus Word Pro file format import filter.])
   fi
   if test "$enable_neon" != "no" -o "x$DISABLE_NEON" != "xTRUE"; then
        AC_MSG_ERROR([need to --disable-neon - webdav support.])
   fi
   if test "x$enable_ext_mariadb_connector" = "xyes"; then
        AC_MSG_ERROR([need to --disable-ext-mariadb-connector - mariadb/mysql support.])
   fi
   if test "x$ENABLE_PDFIMPORT" != "xFALSE"; then
        if test "x$SYSTEM_POPPLER" != "xNO"; then
                AC_MSG_ERROR([need to disable PDF import via poppler or use system library])
        fi
   fi
   # cf. m4/libo_check_extension.m4
   if test "x$WITH_EXTRA_EXTENSIONS" != "x"; then
        AC_MSG_ERROR([need to disable extra extensions '$WITH_EXTRA_EXTENSIONS'])
   fi
   for theme in $WITH_THEMES; do
        case $theme in
            crystal|default|hicontrast|human|oxygen|sifr)
                AC_MSG_ERROR([need to disable icon themes from '$WITH_THEMES': $theme present, use --with-theme=tango]) ;;
            *) : ;;
        esac
   done
   if test "$enable_opengl" != "no" -o "x$ENABLE_OPENGL" == "xTRUE"; then
        AC_MSG_ERROR([need to --disable-opengl - GL transitions support.])
   fi
   if test "$enable_lpsolve" != "no" -o "x$ENABLE_LPSOLVE" == "xTRUE"; then
        AC_MSG_ERROR([need to --disable-lpsolve - calc linear programming solver.])
   fi

   MPL_SUBSET="TRUE"
   AC_DEFINE(MPL_HAVE_SUBSET)
   AC_MSG_RESULT([only])
else
   AC_MSG_RESULT([no restrictions])
fi
AC_SUBST(MPL_SUBSET)


dnl ===================================================================
dnl Setting up the environment.
dnl ===================================================================
echo "setting up the build environment variables..."

AC_SUBST(COMPATH)

if test "$build_os" = "cygwin"; then
    if test "$DISABLE_ACTIVEX" = "TRUE"; then
        ATL_LIB="$WINDOWS_SDK_HOME/lib" # Doesn't exist for VSE
        ATL_INCLUDE="$WINDOWS_SDK_HOME/include/atl"
    else
        ATL_LIB="$COMPATH/atlmfc/lib"
        ATL_INCLUDE="$COMPATH/atlmfc/include"
    fi
    if test "$BITNESS_OVERRIDE" = 64; then
        ATL_LIB="$ATL_LIB/amd64"
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
AC_SUBST(WIN_FIND)
AC_SUBST(WIN_GREP)
AC_SUBST(WIN_LS)
AC_SUBST(WIN_TOUCH)

AC_SUBST(BUILD_TYPE)

AC_SUBST(SOLARINC)
AC_SUBST(SOLARLIB)

PathFormat "$PERL"
PERL="$formatted_path"
AC_SUBST(PERL)

if test -n "$TMPDIR"; then
    TEMP_DIRECTORY="$TMPDIR"
else
    TEMP_DIRECTORY="/tmp"
fi
if test "$build_os" = "cygwin"; then
    TEMP_DIRECTORY=`cygpath -m "$TEMP_DIRECTORY"`
fi
AC_SUBST(TEMP_DIRECTORY)

# setup the PATH for the environment
if test -n "$LO_PATH_FOR_BUILD"; then
    LO_PATH="$LO_PATH_FOR_BUILD"
else
    LO_PATH="$PATH"

    case "$host_os" in

    aix*|dragonfly*|freebsd*|linux-gnu*|*netbsd*|openbsd*)
        if test "$ENABLE_JAVA" != "" -a "$JDK"!="gcj"; then
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
        if test "$ENABLE_JAVA" != "" -a "$JDK"!="gcj"; then
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
        if test "$ENABLE_JAVA" != "" -a "$JDK"!="gcj"; then
            pathmunge "$JAVA_HOME/bin" "after"
        fi
        ;;
    esac

    pathmunge "$SRC_ROOT/solenv/$OUTPATH/bin" "before"
    pathmunge "$SRC_ROOT/solenv/bin" "before"
    pathmunge "." "before"
fi

AC_SUBST(LO_PATH)

# Generate a configuration md5 we can use for deps
if test -f config_host.mk; then
    config_md5=`$MD5SUM config_host.mk | sed "s/ .*//"`
fi

AC_CONFIG_FILES([config_host.mk Makefile lo.xcent instsetoo_native/util/openoffice.lst])
AC_CONFIG_HEADERS([config_host/config_buildid.h])
AC_CONFIG_HEADERS([config_host/config_clang.h])
AC_CONFIG_HEADERS([config_host/config_features.h])
AC_CONFIG_HEADERS([config_host/config_folders.h])
AC_CONFIG_HEADERS([config_host/config_gcc.h])
AC_CONFIG_HEADERS([config_host/config_global.h])
AC_CONFIG_HEADERS([config_host/config_graphite.h])
AC_CONFIG_HEADERS([config_host/config_lgpl.h])
AC_CONFIG_HEADERS([config_host/config_mpl.h])
AC_CONFIG_HEADERS([config_host/config_kde4.h])
AC_CONFIG_HEADERS([config_host/config_mingw.h])
AC_CONFIG_HEADERS([config_host/config_oox.h])
AC_CONFIG_HEADERS([config_host/config_telepathy.h])
AC_CONFIG_HEADERS([config_host/config_typesizes.h])
AC_CONFIG_HEADERS([config_host/config_vclplug.h])
AC_CONFIG_HEADERS([config_host/config_version.h])
AC_OUTPUT

if test "$CROSS_COMPILING" = YES; then
    (echo; echo export BUILD_TYPE_FOR_HOST=$BUILD_TYPE) >>config_build.mk
fi

# touch the config timestamp file
if test ! -f config_host.mk.stamp; then
    echo > config_host.mk.stamp
elif test "$config_md5" = `$MD5SUM config_host.mk | sed "s/ .*//"`; then
    echo "Configuration unchanged - avoiding scp2 stamp update"
else
    echo > config_host.mk.stamp
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

if test -f warn; then
    cat warn
    rm warn
fi

dnl vim:set shiftwidth=4 softtabstop=4 expandtab:
