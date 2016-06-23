dnl -*- Mode: Autoconf; tab-width: 4; indent-tabs-mode: nil; fill-column: 102 -*-
dnl configure.ac serves as input for the GNU autoconf package
dnl in order to create a configure script.

# The version number in the second argumemnt to AC_INIT should be four numbers separated by
# periods. Some parts of the code requires the first one to be less than 128 and the others to be less
# than 256. The four numbers can optionally be followed by a period and a free-form string containing
# no spaces or periods, like "frobozz-mumble-42" or "alpha0". If the free-form string ends with one or
# several non-alphanumeric characters, those are split off and used only for the
# ABOUTBOXPRODUCTVERSIONSUFFIX in openoffice.lst. Why that is necessary, no idea.

AC_INIT([LibreOffice],[5.3.0.0.alpha0+],[],[],[http://documentfoundation.org/])

AC_PREREQ([2.59])

if test -n "$BUILD_TYPE"; then
    AC_MSG_ERROR([You have sourced config_host.mk in this shell.  This may lead to trouble, please run in a fresh (login) shell.])
fi

save_CC=$CC
save_CXX=$CXX

BUILD_TYPE="LibO"
SCPDEFS=""
GIT_NEEDED_SUBMODULES=""
LO_PATH= # used by path_munge to construct a PATH variable

FilterLibs()
{
    filteredlibs=
    for f in $1; do
        case "$f" in
            # let's start with Fedora's paths for now
            -L/lib|-L/lib/|-L/lib64|-L/lib64/|-L/usr/lib|-L/usr/lib/|-L/usr/lib64|-L/usr/lib64/)
                # ignore it: on UNIXoids it is searched by default anyway
                # but if it's given explicitly then it may override other paths
                # (on Mac OS X it would be an error to use it instead of SDK)
                ;;
            *)
                filteredlibs="$filteredlibs $f"
                ;;
        esac
    done
}

PathFormat()
{
    formatted_path="$1"
    if test "$build_os" = "cygwin"; then
        pf_conv_to_dos=
        # spaces,parentheses,brackets,braces are problematic in pathname
        # so are backslashes
        case "$formatted_path" in
            *\ * | *\)* | *\(* | *\{* | *\}* | *\[* | *\]* | *\\* )
                pf_conv_to_dos="yes"
            ;;
        esac
        if test "$pf_conv_to_dos" = "yes"; then
            if test "$GNUMAKE_WIN_NATIVE" = "TRUE" ; then
                formatted_path=`cygpath -sm "$formatted_path"`
            else
                formatted_path=`cygpath -d "$formatted_path"`
            fi
            if test $? -ne 0;  then
                AC_MSG_ERROR([path conversion failed for "$1".])
            fi
        fi
        fp_count_colon=`echo "$formatted_path" | $GREP -c "[:]"`
        fp_count_slash=`echo "$formatted_path" | $GREP -c "[/]"`
        if test "$fp_count_slash$fp_count_colon" != "00"; then
            if test "$fp_count_colon" = "0"; then
                new_formatted_path=`realpath "$formatted_path"`
                if test $? -ne 0;  then
                    AC_MSG_WARN([realpath failed for "$1", not necessarily a problem.])
                else
                    formatted_path="$new_formatted_path"
                fi
            fi
            formatted_path=`cygpath -m "$formatted_path"`
            if test $? -ne 0;  then
                AC_MSG_ERROR([path conversion failed for "$1".])
            fi
        fi
    fi
}

AbsolutePath()
{
    # There appears to be no simple and portable method to get an absolute and
    # canonical path, so we try creating the directory if does not exist and
    # utilizing the shell and pwd.
    rel="$1"
    absolute_path=""
    test ! -e "$rel" && mkdir -p "$rel"
    if test -d "$rel" ; then
        cd "$rel" || AC_MSG_ERROR([absolute path resolution failed for "$rel".])
        absolute_path="$(pwd)"
        cd - > /dev/null
    else
        AC_MSG_ERROR([Failed to resolve absolute path.  "$rel" does not exist or is not a directory.])
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

dnl Some Mac User have the bad habbit of letting a lot fo crap
dnl accumulate in their PATH and even adding stuff in /usr/local/bin
dnl that confuse the build.
dnl For the ones that use LODE, let's be nice and protect them
dnl from themselves

mac_sanitize_path()
{
    mac_path="$LODE_HOME/opt/bin:/usr/bin:/bin:/usr/sbin:/sbin"
dnl a common but nevertheless necessary thing that may be in a fancy
dnl path location is git, so make sure we have it
    mac_git_path=`which git 2>/dev/null`
    if test -n "$mac_git_path" -a -x "$mac_git_path" -a "$mac_git_path" != "/usr/bin/git" ; then
        mac_path="$mac_path:`dirname $mac_git_path`"
    fi
    PATH="$mac_path"
    unset mac_path
    unset git_mac_path
}

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

AC_MSG_CHECKING([for product name])
PRODUCTNAME="AC_PACKAGE_NAME"
if test "$enable_release_build" = "" -o "$enable_release_build" = "no"; then
    PRODUCTNAME="${PRODUCTNAME}Dev"
fi
AC_MSG_RESULT([$PRODUCTNAME])
AC_SUBST(PRODUCTNAME)

dnl ===================================================================
dnl Our version is defined by the AC_INIT() at the top of this script.
dnl ===================================================================

set `echo AC_PACKAGE_VERSION | sed "s/\./ /g"`

LIBO_VERSION_MAJOR=$1
LIBO_VERSION_MINOR=$2
LIBO_VERSION_MICRO=$3
LIBO_VERSION_PATCH=$4

# The CFBundleShortVersionString in Info.plist consists of three integers, so encode the third
# as the micro version times 1000 plus the patch number. Unfortunately the LIBO_VERSION_SUFFIX can be anything so
# no way to encode that into an integer in general.
MACOSX_BUNDLE_SHORTVERSION=$LIBO_VERSION_MAJOR.$LIBO_VERSION_MINOR.`expr $LIBO_VERSION_MICRO '*' 1000 + $LIBO_VERSION_PATCH`

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
AC_SUBST(MACOSX_BUNDLE_SHORTVERSION)
AC_SUBST(LIBO_VERSION_SUFFIX)
AC_SUBST(LIBO_VERSION_SUFFIX_SUFFIX)

AC_DEFINE_UNQUOTED(LIBO_VERSION_MAJOR,$LIBO_VERSION_MAJOR)
AC_DEFINE_UNQUOTED(LIBO_VERSION_MINOR,$LIBO_VERSION_MINOR)
AC_DEFINE_UNQUOTED(LIBO_VERSION_MICRO,$LIBO_VERSION_MICRO)
AC_DEFINE_UNQUOTED(LIBO_VERSION_PATCH,$LIBO_VERSION_PATCH)

LIBO_THIS_YEAR=`date +%Y`
AC_DEFINE_UNQUOTED(LIBO_THIS_YEAR,$LIBO_THIS_YEAR)

dnl ===================================================================
dnl Product version
dnl ===================================================================
AC_MSG_CHECKING([for product version])
PRODUCTVERSION="$LIBO_VERSION_MAJOR.$LIBO_VERSION_MINOR"
AC_MSG_RESULT([$PRODUCTVERSION])
AC_SUBST(PRODUCTVERSION)

AC_PROG_EGREP
# AC_PROG_EGREP doesn't set GREP on all systems as well
AC_PATH_PROG(GREP, grep)

BUILDDIR=`pwd`
cd $srcdir
SRC_ROOT=`pwd`
cd $BUILDDIR
x_Cygwin=[\#]

dnl ======================================
dnl Required GObject introspection version
dnl ======================================
INTROSPECTION_REQUIRED_VERSION=1.32.0

dnl ===================================================================
dnl Search all the common names for GNU Make
dnl ===================================================================
AC_MSG_CHECKING([for GNU Make])

# try to use our own make if it is available and GNUMAKE was not already defined
if test -z "$GNUMAKE"; then
    if test -n "$LODE_HOME" -a -x "$LODE_HOME/opt/bin/make" ; then
        GNUMAKE="$LODE_HOME/opt/bin/make"
    elif test -x "/opt/lo/bin/make"; then
        GNUMAKE="/opt/lo/bin/make"
    fi
fi

GNUMAKE_WIN_NATIVE=
for a in "$MAKE" "$GNUMAKE" make gmake gnumake; do
    if test -n "$a"; then
        $a --version 2> /dev/null | grep GNU  2>&1 > /dev/null
        if test $? -eq 0;  then
            if test "$build_os" = "cygwin"; then
                if test -n "$($a -v | grep 'Built for Windows')" ; then
                    GNUMAKE="$(cygpath -m "$(which "$(cygpath -u $a)")")"
                    GNUMAKE_WIN_NATIVE="TRUE"
                else
                    GNUMAKE=`which $a`
                fi
            else
                GNUMAKE=`which $a`
            fi
            break
        fi
    fi
done
AC_MSG_RESULT($GNUMAKE)
if test -z "$GNUMAKE"; then
    AC_MSG_ERROR([not found. install GNU Make.])
else
    if test "$GNUMAKE_WIN_NATIVE" = "TRUE" ; then
        AC_MSG_NOTICE([Using a native Win32 GNU Make version.])
    fi
fi

win_short_path_for_make()
{
    local_short_path="$1"
    if test "$GNUMAKE_WIN_NATIVE" = "TRUE" ; then
        cygpath -sm "$local_short_path"
    else
        cygpath -u "$(cygpath -d "$local_short_path")"
    fi
}


if test "$build_os" = "cygwin"; then
    PathFormat "$SRC_ROOT"
    SRC_ROOT="$formatted_path"
    PathFormat "$BUILDDIR"
    BUILDDIR="$formatted_path"
    x_Cygwin=
    AC_MSG_CHECKING(for explicit COMSPEC)
    if test -z "$COMSPEC"; then
        AC_MSG_ERROR([COMSPEC not set in Environment, please set it an rerun])
    fi
fi

AC_SUBST(SRC_ROOT)
AC_SUBST(BUILDDIR)
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

# Set the ENABLE_LTO variable
# ===================================================================
AC_MSG_CHECKING([whether to use link-time optimization])
if test -n "$enable_lto" -a "$enable_lto" != "no"; then
    ENABLE_LTO="TRUE"
    AC_MSG_RESULT([yes])
    AC_DEFINE(STATIC_LINKING)
else
    ENABLE_LTO=""
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_LTO)

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
        Android NDK you are using. The default is 4.9 currently.]), ,)

AC_ARG_WITH(android-sdk,
    AS_HELP_STRING([--with-android-sdk],
        [Specify location of the Android SDK. Mandatory when building for Android,
        or when building the Impress Remote Android app.]),
,)

ANDROID_NDK_HOME=
if test -z "$with_android_ndk" -a -e "$SRC_ROOT/external/android-ndk" -a "$build" != "$host"; then
    with_android_ndk="$SRC_ROOT/external/android-ndk"
fi
if test -n "$with_android_ndk"; then
    ANDROID_NDK_HOME=$with_android_ndk

    # Set up a lot of pre-canned defaults

    if test ! -f $ANDROID_NDK_HOME/RELEASE.TXT; then
        if test ! -f $ANDROID_NDK_HOME/source.properties; then
            AC_MSG_ERROR([Unrecognized Android NDK. Missing RELEASE.TXT or source.properties file in $ANDROID_NDK_HOME.])
        fi
        ANDROID_NDK_VERSION=`sed -n -e 's/Pkg.Revision = //p' $ANDROID_NDK_HOME/source.properties`
    else
        ANDROID_NDK_VERSION=`cut -f1 -d' ' <$ANDROID_NDK_HOME/RELEASE.TXT`
    fi

    case $ANDROID_NDK_VERSION in
    r9*|r10*)
        ;;
    11.1.*|12.1.*)
        ;;
    *)
        AC_MSG_ERROR([Unsupported NDK version $ANDROID_NDK_VERSION, only r9*, r10*, 11.1.* and 12.1.* are supported])
        ;;
    esac

    if test $host_cpu = arm; then
        android_cpu=arm
        android_platform_prefix=$android_cpu-linux-androideabi
    elif test $host_cpu = aarch64; then
        android_cpu=aarch64
        android_platform_prefix=$android_cpu-linux-android
    elif test $host_cpu = mips; then
        android_cpu=mips
        android_platform_prefix=$android_cpu-linux-androideabi
    else
        # host_cpu is something like "i386" or "i686" I guess, NDK uses
        # "x86" in some contexts
        android_cpu=x86
        android_platform_prefix=$android_cpu
    fi

    if test -z "$with_android_ndk_toolchain_version"; then
        # Default to gcc 4.9
        with_android_ndk_toolchain_version=4.9
    fi

    case "$with_android_ndk_toolchain_version" in
    4.6|4.7|4.8|4.9)
        ANDROID_BINUTILS_DIR=$ANDROID_NDK_HOME/toolchains/$android_platform_prefix-$with_android_ndk_toolchain_version
        ANDROID_COMPILER_DIR=$ANDROID_BINUTILS_DIR
        ;;
    clang3.3|clang3.4)
        AC_MSG_WARN([Building with the Clang tool-chain is known to break in the bridges module, fix that please])
        ANDROID_BINUTILS_DIR=$ANDROID_NDK_HOME/toolchains/$android_platform_prefix-4.8
        ANDROID_COMPILER_DIR=$ANDROID_NDK_HOME/toolchains/llvm-${with_android_ndk_toolchain_version#clang}
        ANDROID_USING_CLANG=true
        ;;
    *)
        AC_MSG_ERROR([Unrecognized value for the --with-android-ndk-toolchain-version option])
    esac

    if test ! -d $ANDROID_BINUTILS_DIR; then
        AC_MSG_ERROR([No directory $ANDROID_BINUTILS_DIR])
    elif test $ANDROID_COMPILER_DIR != $ANDROID_BINUTILS_DIR -a ! -d $ANDROID_COMPILER_DIR; then
        AC_MSG_ERROR([No directory $ANDROID_COMPILER_DIR])
    fi

    # Check if there is a 64-bit tool-chain. Google provides a NDK with 64-bit tool-chain binaries in
    # NDK r8e and later, and for earlier NDKs it was possible to build one yourself. Using a 64-bit
    # linker is required if you compile large parts of the code with -g. A 32-bit linker just won't
    # manage to link the (app-specific) single huge .so that is built for the app in
    # android/source/ if there is debug information in a significant part of the object files.
    # (A 64-bit ld.gold grows to much over 10 gigabytes of virtual space when linking such a .so if
    # all objects have been built with debug information.)
    toolchain_system='*'
    case $build_os in
    linux-gnu*)
        ndk_build_os=linux
        ;;
    darwin*)
        ndk_build_os=darwin
        ;;
    *)
        AC_MSG_ERROR([We only support building for Android from Linux or OS X])
        ;;
    esac

    ANDROID_COMPILER_BIN=$ANDROID_COMPILER_DIR/prebuilt/$ndk_build_os-x86/bin
    ANDROID_BINUTILS_PREBUILT_ROOT=$ANDROID_BINUTILS_DIR/prebuilt/$ndk_build_os-x86
    if test $build_cpu = x86_64; then
        if test -d $ANDROID_COMPILER_DIR/prebuilt/$ndk_build_os-x86_64; then
            ANDROID_COMPILER_BIN=$ANDROID_COMPILER_DIR/prebuilt/$ndk_build_os-x86_64/bin
        fi
        if test -d $ANDROID_BINUTILS_DIR/prebuilt/$ndk_build_os-x86_64; then
            ANDROID_BINUTILS_PREBUILT_ROOT=$ANDROID_BINUTILS_DIR/prebuilt/$ndk_build_os-x86_64
        fi
    fi
    ANDROID_BINUTILS_BIN=$ANDROID_BINUTILS_PREBUILT_ROOT/bin

    # This stays empty if there is just one version of the toolchain in the NDK
    ANDROID_NDK_TOOLCHAIN_VERSION_SUBDIR=
    if test ! -d "$ANDROID_NDK_HOME/sources/cxx-stl/gnu-libstdc++/libs" ; then
        # nope, won't work if empty...
        # as is the case when using the ndk-bundle as installed with android studio
        ANDROID_NDK_TOOLCHAIN_VERSION_SUBDIR="${with_android_ndk_toolchain_version}/"
        if test -n "$ANDROID_USING_CLANG"; then
            ANDROID_NDK_TOOLCHAIN_VERSION_SUBDIR=4.8/
        fi
    fi

    ANDROID_API_LEVEL=15
    if test $host_cpu = arm; then
        android_gnu_prefix=arm-linux-androideabi
    elif test $host_cpu = aarch64; then
        android_gnu_prefix=aarch64-linux-android
        ANDROID_NDK_TOOLCHAIN_VERSION_SUBDIR="${with_android_ndk_toolchain_version}/"
        ANDROID_API_LEVEL=L
    elif test $host_cpu = mips; then
        android_gnu_prefix=mipsel-linux-android
    elif test $ANDROID_NDK_VERSION = r8; then
        # The prefix used for the x86 tool-chain changed between NDK r8 and r8b
        android_gnu_prefix=i686-android-linux
    else
        android_gnu_prefix=i686-linux-android
    fi

    ANDROID_ARCH=$android_cpu
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
        ANDROIDCFLAGS="$ANDROIDCFLAGS -Wl,--fix-cortex-a8"
    elif test $host_cpu = aarch64; then
        ANDROID_APP_ABI=arm64-v8a
        ANDROID_ARCH=arm64
    elif test $host_cpu = mips; then
        ANDROID_APP_ABI=mips
        ANDROIDCFLAGS=""
    else # x86
        ANDROID_APP_ABI=x86
        ANDROIDCFLAGS="-march=atom"
    fi
    ANDROIDCFLAGS="$ANDROIDCFLAGS -ffunction-sections -fdata-sections"
    ANDROIDCFLAGS="$ANDROIDCFLAGS -L$ANDROID_NDK_HOME/sources/cxx-stl/gnu-libstdc++/${ANDROID_NDK_TOOLCHAIN_VERSION_SUBDIR}libs/$ANDROID_APP_ABI"
    ANDROIDCFLAGS="$ANDROIDCFLAGS --sysroot=$ANDROID_NDK_HOME/platforms/android-${ANDROID_API_LEVEL}/arch-${ANDROID_ARCH}"
    export PKG_CONFIG_LIBDIR="$ANDROID_BINUTILS_PREBUILT_ROOT/lib/pkgconfig"

    if test -n "$ANDROID_USING_CLANG"; then
        ANDROIDCFLAGS="$ANDROIDCFLAGS -Qunused-arguments"
    else
        ANDROIDCFLAGS="$ANDROIDCFLAGS -Wno-psabi"
    fi

    test -z "$SYSBASE" && export SYSBASE=$ANDROID_NDK_HOME/platforms/android-${ANDROID_API_LEVEL}/arch-${ANDROID_ARCH}
    test -z "$AR" && AR=$ANDROID_BINUTILS_BIN/$android_gnu_prefix-ar
    test -z "$NM" && NM=$ANDROID_BINUTILS_BIN/$android_gnu_prefix-nm
    test -z "$OBJDUMP" && OBJDUMP=$ANDROID_BINUTILS_BIN/$android_gnu_prefix-objdump
    test -z "$RANLIB" && RANLIB=$ANDROID_BINUTILS_BIN/$android_gnu_prefix-ranlib
    test -z "$STRIP" && STRIP=$ANDROID_BINUTILS_BIN/$android_gnu_prefix-strip

    # When using the 4.6 or newer toolchain, use the gold linker
    case "$with_android_ndk_toolchain_version" in
    4.[[6789]]*|[[56789]].*|clang*)
        if test "$host_cpu" = arm -a "$ENABLE_LTO" != TRUE; then
            ANDROIDCFLAGS="$ANDROIDCFLAGS -fuse-ld=gold"
        fi
        ;;
    esac

    if test "$ENABLE_LTO" = TRUE; then
        # -flto comes from com_GCC_defs.mk, too, but we need to make sure it gets passed as part of
        # $CC and $CXX when building external libraries
         if test "$COM_IS_CLANG" = TRUE; then
            ANDROIDCFLAGS="$ANDROIDCFLAGS -flto"
        else
            ANDROIDCFLAGS="$ANDROIDCFLAGS -flto -fuse-linker-plugin -O2"
        fi
    fi

    # gdbserver can be in different locations
    if test -f $ANDROID_NDK_HOME/toolchains/arm-linux-androideabi-4.4.3/prebuilt/gdbserver; then
        ANDROID_NDK_GDBSERVER=$ANDROID_NDK_HOME/toolchains/arm-linux-androideabi-4.4.3/prebuilt/gdbserver
    elif test -f $ANDROID_NDK_HOME/prebuilt/android-$android_cpu/gdbserver/gdbserver; then
        ANDROID_NDK_GDBSERVER=$ANDROID_NDK_HOME/prebuilt/android-$android_cpu/gdbserver/gdbserver
    elif test $android_cpu = aarch64; then
        ANDROID_NDK_GDBSERVER=$ANDROID_NDK_HOME/prebuilt/android-arm64/gdbserver/gdbserver
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

    # remember to download the ownCloud Android library later
    BUILD_TYPE="$BUILD_TYPE OWNCLOUD_ANDROID_LIB"
fi
AC_SUBST(ANDROID_NDK_GDBSERVER)
AC_SUBST(ANDROID_APP_ABI)

dnl ===================================================================
dnl --with-android-sdk
dnl ===================================================================
ANDROID_SDK_HOME=
if test -z "$with_android_sdk" -a -e "$SRC_ROOT/external/android-sdk-linux" -a "$build" != "$host"; then
    with_android_sdk="$SRC_ROOT/external/android-sdk-linux"
fi
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

INSTROOTSUFFIX=
SDKDIRNAME=sdk

case "$host_os" in

solaris*)
    test_gtk=yes
    build_gstreamer_1_0=yes
    build_gstreamer_0_10=yes
    test_tde=yes
    test_freetype=yes
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
    build_gstreamer_1_0=yes
    build_gstreamer_0_10=yes
    test_tde=yes
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
        if test -n "$LODE_HOME" ; then
            mac_sanitize_path
            AC_MSG_NOTICE([sanitized the PATH to $PATH])
        fi
        INSTROOTSUFFIX=/$PRODUCTNAME.app/Contents
        SDKDIRNAME=AC_PACKAGE_NAME${PRODUCTVERSION}_SDK
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
    build_gstreamer_1_0=yes
    build_gstreamer_0_10=yes
    test_tde=yes
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
    build_gstreamer_1_0=yes
    build_gstreamer_0_10=yes
    test_tde=no
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
    test_freetype=yes
    PTHREAD_CFLAGS="-D_THREAD_SAFE"
    PTHREAD_LIBS="-pthread"
    _os=OpenBSD
    ;;

dragonfly*)
    test_gtk=yes
    build_gstreamer_1_0=yes
    build_gstreamer_0_10=yes
    test_tde=yes
    test_kde4=yes
    test_freetype=yes
    PTHREAD_LIBS="-pthread"
    _os=DragonFly
    ;;

linux-android*)
    build_gstreamer_1_0=no
    build_gstreamer_0_10=no
    enable_lotuswordpro=no
    enable_mpl_subset=yes
    enable_coinmp=yes
    enable_lpsolve=no
    enable_report_builder=no
    enable_odk=no
    enable_postgresql_sdbc=no
    enable_python=no
    with_theme="tango"
    test_cups=no
    test_dbus=no
    test_fontconfig=no
    test_freetype=no
    test_gtk=no
    test_tde=no
    test_kde4=no
    test_randr=no
    test_xrender=no
    _os=Android

    if test -z "$with_android_ndk"; then
        AC_MSG_ERROR([the --with-android-ndk option is mandatory, unless it is available at external/android-ndk/.])
    fi

    if test -z "$with_android_ndk_toolchain_version"; then
        AC_MSG_ERROR([the --with-android-ndk-toolchain-version option is mandatory])
    fi

    # Verify that the NDK and SDK options are proper
    if test ! -f "$ANDROID_NDK_HOME/platforms/android-${ANDROID_API_LEVEL}/arch-${ANDROID_ARCH}/usr/lib/libc.a"; then
        AC_MSG_ERROR([the --with-android-ndk option does not point to an Android NDK])
    fi

    AC_DEFINE(HAVE_FT_FACE_GETCHARVARIANTINDEX)
    BUILD_TYPE="$BUILD_TYPE CAIRO FONTCONFIG FREETYPE"
    ;;

emscripten*)
    build_gstreamer_1_0=no
    build_gstreamer_0_10=no
    enable_lotuswordpro=no
    enable_mpl_subset=yes
    enable_coinmp=yes
    enable_lpsolve=no
    enable_report_builder=no
    with_theme="tango"
    test_cups=no
    test_dbus=no
    test_fontconfig=no
    test_freetype=no
    test_gtk=no
    test_tde=no
    test_kde4=no
    test_randr=no
    test_xrender=no
    _os=Emscripten
    ;;

*)
    AC_MSG_ERROR([$host_os operating system is not suitable to build LibreOffice for!])
    ;;
esac

if echo "$host_os" | grep -q linux-android ; then
    if test -z "$with_android_sdk"; then
        AC_MSG_ERROR([the --with-android-sdk option is mandatory, unless it is available at external/android-sdk-linux/.])
    fi

    if test ! -d "$ANDROID_SDK_HOME/platforms"; then
        AC_MSG_ERROR([the --with-android-sdk option does not point to an Android SDK])
    fi

    BUILD_TOOLS_VERSION=`$SED -n -e 's/.*buildToolsVersion "\(.*\)"/\1/p' $SRC_ROOT/android/source/build.gradle`
    if test ! -d "$ANDROID_SDK_HOME/build-tools/$BUILD_TOOLS_VERSION"; then
        AC_MSG_WARN([android build-tools $BUILD_TOOLS_VERSION not found - install with
                         $ANDROID_SDK_HOME/tools/android update sdk -u --all --filter build-tools-$BUILD_TOOLS_VERSION
                    or adjust change $SRC_ROOT/android/source/build.gradle accordingly])
        add_warning "android build-tools $BUILD_TOOLS_VERSION not found - install with"
        add_warning "    $ANDROID_SDK_HOME/tools/android update sdk -u --all --filter build-tools-$BUILD_TOOLS_VERSION"
        add_warning "or adjust $SRC_ROOT/android/source/build.gradle accordingly"
    fi
    if test ! -f "$ANDROID_SDK_HOME/extras/android/m2repository/source.properties"; then
        AC_MSG_WARN([android support repository not found - install with
                         $ANDROID_SDK_HOME/tools/android update sdk -u --filter extra-android-m2repository
                     to allow the build to download the specified version of the android support libraries])
        add_warning "android support repository not found - install with"
        add_warning "    $ANDROID_SDK_HOME/tools/android update sdk -u --filter extra-android-m2repository"
        add_warning "to allow the build to download the specified version of the android support libraries"
    fi
fi

if test "$_os" = "AIX"; then
    AC_PATH_PROG(GAWK, gawk)
    if test -z "$GAWK"; then
        AC_MSG_ERROR([gawk not found in \$PATH])
    fi
fi

AC_SUBST(SDKDIRNAME)

AC_SUBST(WITH_MINGW)
AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_LIBS)

dnl The following checks for gcc, cc and then cl (if it weren't guarded for win32)
dnl Needs to precede the AC_SEARCH_LIBS call below, which apparently calls
dnl AC_PROG_CC internally.
if test "$_os" != "WINNT" -o "$WITH_MINGW" = "yes"; then
    # AC_PROG_CC sets CFLAGS to -g -O2 if not set, avoid that
    save_CFLAGS=$CFLAGS
    AC_PROG_CC
    CFLAGS=$save_CFLAGS
fi

if test $_os != "WINNT"; then
    save_LIBS="$LIBS"
    AC_SEARCH_LIBS([dlsym], [dl],
        [case "$ac_cv_search_dlsym" in -l*) DLOPEN_LIBS="$ac_cv_search_dlsym";; esac],
        [AC_MSG_ERROR([dlsym not found in either libc nor libdl])])
    LIBS="$save_LIBS"
fi
AC_SUBST(DLOPEN_LIBS)

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

AC_ARG_ENABLE(avmedia,
    AS_HELP_STRING([--disable-avmedia],
        [Disable displaying and inserting AV media in documents. Work in progress, use only if you are hacking on it.])
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
AC_ARG_ENABLE(coinmp,
    AS_HELP_STRING([--disable-coinmp],
        [Disable compilation of the CoinMP solver ])
)

AC_ARG_ENABLE(pdfimport,
    AS_HELP_STRING([--disable-pdfimport],
        [Disable building the PDF import feature.])
)

###############################################################################

dnl ---------- *** ----------

AC_ARG_ENABLE([hardlink-deliver],
    AS_HELP_STRING([--enable-hardlink-deliver],
        [Put files into deliver folder as hardlinks instead of copying them
        over. Saves space and speeds up build.])
)

AC_ARG_ENABLE(mergelibs,
    AS_HELP_STRING([--enable-mergelibs],
        [Enables linking of big, merged, library. Experimental feature, tested
        only for Linux at some stage in history, but possibly does not work even
        for Linux any more. This will link a core set of libraries into libmerged.])
)

AC_ARG_ENABLE(graphite,
    AS_HELP_STRING([--disable-graphite],
        [Disables the compilation of Graphite smart font rendering.])
)

AC_ARG_ENABLE(breakpad,
    AS_HELP_STRING([--enable-breakpad],
        [Enables breakpad for crash reporting.])
)

AC_ARG_ENABLE(orcus,
    AS_HELP_STRING([--enable-orcus],
        [Enables orcus for extra file import filters for Calc.])
)

AC_ARG_ENABLE(fetch-external,
    AS_HELP_STRING([--disable-fetch-external],
        [Disables fetching external tarballs from web sources.])
)

AC_ARG_ENABLE(pch,
    AS_HELP_STRING([--enable-pch],
        [Enables precompiled header support for C++. Forced default on Windows/VC build])
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

AC_ARG_ENABLE(sal-log,
    AS_HELP_STRING([--enable-sal-log],
        [Make SAL_INFO and SAL_WARN calls do something even in a non-debug build.]))

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
        [Generate debug information.
         By default, enabled for --enable-debug and --enable-dbgutil, disabled
         otherwise.]))

AC_ARG_ENABLE(runtime-optimizations,
    AS_HELP_STRING([--disable-runtime-optimizations],
        [Statically disable certain runtime optimizations (like rtl/alloc.h or
         JVM JIT) that are known to interact badly with certain dynamic analysis
         tools (like -fsanitize=address or Valgrind).  By default, disabled iff
         CC contains "-fsanitize=*".  (For Valgrind, those runtime optimizations
         are typically disabled dynamically via RUNNING_ON_VALGRIND.)]))

AC_ARG_ENABLE(compiler-plugins,
    AS_HELP_STRING([--enable-compiler-plugins],
        [Enable compiler plugins that will perform additional checks during
         building. Enabled automatically by --enable-dbgutil.]))

AC_ARG_ENABLE(ooenv,
    AS_HELP_STRING([--disable-ooenv],
        [Disable ooenv for the instdir installation.]))

AC_ARG_ENABLE(lto,
    AS_HELP_STRING([--enable-lto],
        [Enable link-time optimization. Suitable for (optimised) product builds. Building might take
         longer but libraries and executables are optimized for speed. For GCC, best to use the 'gold'
         linker. For MSVC, this option is broken at the moment. This is experimental work
         in progress that shouldn't be used unless you are working on it.)]))

AC_ARG_ENABLE(python,
    AS_HELP_STRING([--enable-python=<no/auto/system/internal/fully-internal>],
        [Enables or disables Python support at run-time and build-time.
         Also specifies what Python to use. 'auto' is the default.
         'fully-internal' even forces the internal version for uses of Python
         during the build.]))

AC_ARG_ENABLE(gtk,
    AS_HELP_STRING([--disable-gtk],
        [Determines whether to use Gtk+ vclplug on platforms where Gtk+ is available.]),
,enable_gtk=yes)

AC_ARG_ENABLE(gtk3,
    AS_HELP_STRING([--disable-gtk3],
        [Determines whether to use Gtk+ 3.0 vclplug on platforms where Gtk+ 3.0 is available.
         This is experimental and may not work.]),
,enable_gtk3=yes)

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
        [Split file lists for some optional features, e.g. pyuno, testtool.
         Has effect only with make distro-pack-install]),
,)

AC_ARG_ENABLE(cairo-canvas,
    AS_HELP_STRING([--disable-cairo-canvas],
        [Determines whether to build the Cairo canvas on platforms where Cairo is available.]),
,)

AC_ARG_ENABLE(dbus,
    AS_HELP_STRING([--disable-dbus],
        [Determines whether to enable features that depend on dbus.
         e.g. Presentation mode screensaver control, bluetooth presentation control, automatic font install]),
,enable_dbus=yes)

AC_ARG_ENABLE(sdremote,
    AS_HELP_STRING([--disable-sdremote],
        [Determines whether to enable Impress remote control (i.e. the server component).]),
,enable_sdremote=yes)

AC_ARG_ENABLE(sdremote-bluetooth,
    AS_HELP_STRING([--disable-sdremote-bluetooth],
        [Determines whether to build sdremote with bluetooth support.
         Requires dbus on Linux.]))

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

AC_ARG_ENABLE(kde4,
    AS_HELP_STRING([--enable-kde4],
        [Determines whether to use Qt4/KDE4 vclplug on platforms where Qt4 and
         KDE4 are available.]),
,)

AC_ARG_ENABLE(gui,
    AS_HELP_STRING([--disable-gui],
        [Disable X11/Wayland support to reduce dependencies. Maybe useful for
         headless servers, but soffice --headless works just fine without it.
         Work in progress, use only if you are hacking on it.]),
,enable_gui=yes)

AC_ARG_ENABLE(randr,
    AS_HELP_STRING([--disable-randr],
        [Disable RandR support in the vcl project.]),
,enable_randr=yes)

AC_ARG_ENABLE(gstreamer-1-0,
    AS_HELP_STRING([--disable-gstreamer-1-0],
        [Disable building with the new gstreamer 1.0 avmedia backend.]),
,enable_gstreamer_1_0=yes)

AC_ARG_ENABLE(gstreamer-0-10,
    AS_HELP_STRING([--enable-gstreamer-0-10],
        [Enable building with the gstreamer 0.10 avmedia backend.]),
,enable_gstreamer_0_10=no)

AC_ARG_ENABLE(vlc,
    AS_HELP_STRING([--enable-vlc],
        [Enable building with the (experimental) VLC avmedia backend.]),
,enable_vlc=no)

AC_ARG_ENABLE(neon,
    AS_HELP_STRING([--disable-neon],
        [Disable neon and the compilation of webdav binding.]),
,)

AC_ARG_ENABLE([eot],
    [AS_HELP_STRING([--enable-eot],
        [Enable support for Embedded OpenType fonts.])],
, [enable_eot=no])

AC_ARG_ENABLE(cve-tests,
    AS_HELP_STRING([--disable-cve-tests],
        [Prevent CVE tests to be executed]),
,)

AC_ARG_ENABLE(chart-tests,
    AS_HELP_STRING([--enable-chart-tests],
        [Executes chart XShape tests. In a perfect world these tests would be
         stable and everyone could run them, in reality it is best to run them
         only on a few machines that are known to work and maintained by people
         who can judge if a test failure is a regression or not.]),
,)

AC_ARG_ENABLE(build-unowinreg,
    AS_HELP_STRING([--enable-build-unowinreg],
        [Do not use the prebuilt unowinreg.dll. Build it instead. The MinGW C++
         compiler is needed on Linux.]),
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
         By default, unless on Windows, we will try to detect if ccache is available; in that case if
         CC/CXX are not yet set, and --enable-icecream is not given, we
         attempt to use ccache. --disable-ccache disables ccache completely.
]),
,)

AC_ARG_ENABLE(64-bit,
    AS_HELP_STRING([--enable-64-bit],
        [Build a 64-bit LibreOffice on platforms where the normal build is 32-bit.
         At the moment meaningful only for iOS and Windows. On Windows this option is
         experimental and possibly quite broken, and you should use it only if you are
         hacking on 64-bitness support.]), ,)

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
         LibreOffice. By default, it is enabled on Windows and Mac, disabled on Linux.
         If the value is "mar", the experimental Mozilla-like update will be
         enabled instead of the traditional update mechanism.]),
,)

AC_ARG_ENABLE(extension-update,
    AS_HELP_STRING([--disable-extension-update],
        [Disable possibility to update installed extensions.]),
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

AC_ARG_ENABLE(macosx-code-signing,
    AS_HELP_STRING([--enable-macosx-code-signing=<identity>],
        [Sign executables, dylibs, frameworks and the app bundle. If you
         don't provide an identity the first suitable certificate
         in your keychain is used.]),
,)

AC_ARG_ENABLE(macosx-package-signing,
    AS_HELP_STRING([--enable-macosx-package-signing=<identity>],
        [Create a .pkg suitable for uploading to the Mac App Store and sign
         it. If you don't provide an identity the first suitable certificate
         in your keychain is used.]),
,)

AC_ARG_ENABLE(macosx-sandbox,
    AS_HELP_STRING([--enable-macosx-sandbox],
        [Make the app bundle run in a sandbox. Requires code signing.
         Is required by apps distributed in the Mac App Store, and implies
         adherence to App Store rules.]),
,)

AC_ARG_WITH(macosx-bundle-identifier,
    AS_HELP_STRING([--with-macosx-bundle-identifier=tld.mumble.orifice.TheOffice],
        [Define the OS X bundle identifier. Default is the somewhat weird
         org.libreoffice.script ("script", huh?).]),
,with_macosx_bundle_identifier=org.libreoffice.script)

AC_ARG_WITH(macosx-app-name,
    AS_HELP_STRING([--with-macosx-app-name='My Own Office Suite'],
        [Define the OS X app name. Default is AC_PACKAGE_NAME.]),
,with_macosx_app_name=$PRODUCTNAME)

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
        [Enable the building and reused of tarball of binary build for some 'external' libraries.
        Some libraries can save their build result in a tarball
        stored in TARFILE_LOCATION. That binary tarball is
        uniquely identified by the source tarball,
        the content of the config_host.mk file and the content
        of the top-level directory in core for that library
        If this option is enabled, then if such a tarfile exist, it will be untarred
        instead of the source tarfile, and the build step will be skipped for that
        library.
        If a proper tarfile does not exist, then the normal source-based
        build is done for that library and a proper binary tarfile is created
        for the next time.]),
)

AC_ARG_ENABLE(gltf,
    AS_HELP_STRING([--disable-gltf],
        [Determines whether to build libraries related to glTF 3D model rendering.]))

AC_ARG_ENABLE(collada,
    AS_HELP_STRING([--disable-collada],
        [Disable collada support (Rendering 3D models stored in *.dae and *.kmz format).]))

AC_ARG_ENABLE(dconf,
    AS_HELP_STRING([--disable-dconf],
        [Disable the dconf configuration backend (enabled by default where
         available).]))

dnl ===================================================================
dnl Optional Packages (--with/without-)
dnl ===================================================================

AC_ARG_WITH(gcc-home,
    AS_HELP_STRING([--with-gcc-home],
        [Specify the location of gcc/g++ manually. This can be used in conjunction
         with --enable-icecream when icecream gcc/g++ wrappers are installed in a
         non-default path.]),
,)

AC_ARG_WITH(gnu-patch,
    AS_HELP_STRING([--with-gnu-patch],
        [Specify location of GNU patch on Solaris or FreeBSD.]),
,)

AC_ARG_WITH(build-platform-configure-options,
    AS_HELP_STRING([--with-build-platform-configure-options],
        [Specify options for the configure script run for the *build* platform in a cross-compilation]),
,)

AC_ARG_WITH(gnu-cp,
    AS_HELP_STRING([--with-gnu-cp],
        [Specify location of GNU cp on Solaris or FreeBSD.]),
,)

AC_ARG_WITH(external-tar,
    AS_HELP_STRING([--with-external-tar=<TARFILE_PATH>],
        [Specify an absolute path of where to find (and store) tarfiles.]),
    TARFILE_LOCATION=$withval ,
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
         build these internally from source ("build"),
         or to disable them ("no")]),
)

AC_ARG_WITH(theme,
    AS_HELP_STRING([--with-theme="theme1 theme2..."],
        [Choose which themes to include. By default those themes with an '*' are included.
         Possible choices: *breeze, crystal, *galaxy, *hicontrast, human, *oxygen, *sifr, *tango, *tango_testing.]),
,)

AC_ARG_WITH(helppack-integration,
    AS_HELP_STRING([--without-helppack-integration],
        [It will not integrate the helppacks to the installer
         of the product. Please use this switch to use the online help
         or separate help packages.]),
,)

AC_ARG_WITH(fonts,
    AS_HELP_STRING([--without-fonts],
        [LibreOffice includes some third-party fonts to provide a reliable basis for
         help content, templates, samples, etc. When these fonts are already
         known to be available on the system then you should use this option.]),
,)

AC_ARG_WITH(epm,
    AS_HELP_STRING([--with-epm],
        [Decides which epm to use. Default is to use the one from the system if
         one is built. When either this is not there or you say =internal epm
         will be built.]),
,)

AC_ARG_WITH(package-format,
    AS_HELP_STRING([--with-package-format],
        [Specify package format(s) for LibreOffice installation sets. The
         implicit --without-package-format leads to no installation sets being
         generated. Possible values: aix, archive, bsd, deb, dmg,
         installed, msi, pkg, and rpm.
         Example: --with-package-format='deb rpm']),
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

AC_ARG_WITH(system-bzip2,
    AS_HELP_STRING([--with-system-bzip2],
        [Use bzip2 already on system. Used only when --enable-online-update=mar]),,
    [with_system_bzip2="$with_system_libs"])

AC_ARG_WITH(system-headers,
    AS_HELP_STRING([--with-system-headers],
        [Use headers already on system -- enables all --with-system-* flags for
         external packages whose headers are the only entities used i.e.
         boost/odbc/sane-header(s).]),,
    [with_system_headers="$with_system_libs"])

AC_ARG_WITH(system-jars,
    AS_HELP_STRING([--without-system-jars],
        [When building with --with-system-libs, also the needed jars are expected
         on the system. Use this to disable that]),,
    [with_system_jars="$with_system_libs"])

AC_ARG_WITH(system-cairo,
    AS_HELP_STRING([--with-system-cairo],
        [Use cairo libraries already on system.  Happens automatically for
         (implicit) --enable-gtk and for --enable-gtk3.]))

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
    [with_system_jpeg="$with_system_libs"])

AC_ARG_WITH(system-libgltf,
    AS_HELP_STRING([--with-system-libgltf],
        [Use libgltf already on system.]),,
    [with_system_libgltf="$with_system_libs"])

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

AC_ARG_WITH(system-opencollada,
    AS_HELP_STRING([--with-system-opencollada],
        [Use openCOLLADA already on system.]),,
    [with_system_opencollada=no])

AC_ARG_WITH(system-collada2gltf,
    AS_HELP_STRING([--with-system-collada2gltf],
        [Use collada2gltf already on system.]),,
    [with_system_collada2gltf=no])

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

AC_ARG_ENABLE(bundle-mariadb,
    AS_HELP_STRING([--enable-bundle-mariadb],
        [When using MariaDB/MySQL libraries already on system, bundle them with the MariaDB Connector/LibreOffice extension.])
)

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
    AS_HELP_STRING([--with-libpq-path=<absolute path to your libpq installation>],
        [Use this PostgreSQL C interface (libpq) installation for building
         the PostgreSQL-SDBC extension.]),
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
    [with_system_odbc="auto"])

AC_ARG_WITH(system-sane,
    AS_HELP_STRING([--with-system-sane],
        [Use sane.h already on system.]),,
    [with_system_sane="$with_system_headers"])

AC_ARG_WITH(system-bluez,
    AS_HELP_STRING([--with-system-bluez],
        [Use bluetooth.h already on system.]),,
    [with_system_bluez="$with_system_headers"])

AC_ARG_WITH(system-curl,
    AS_HELP_STRING([--with-system-curl],
        [Use curl already on system.]),,
    [with_system_curl=auto])

AC_ARG_WITH(system-boost,
    AS_HELP_STRING([--with-system-boost],
        [Use boost already on system.]),,
    [with_system_boost="$with_system_headers"])

AC_ARG_WITH(system-glm,
    AS_HELP_STRING([--with-system-glm],
        [Use glm already on system.]),,
    [with_system_glm="$with_system_headers"])

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

AC_ARG_WITH(system-coinmp,
    AS_HELP_STRING([--with-system-coinmp],
        [Use CoinMP already on system.]),,
    [with_system_coinmp="$with_system_libs"])

AC_ARG_WITH(system-liblangtag,
    AS_HELP_STRING([--with-system-liblangtag],
        [Use liblangtag library already on system.]),,
    [with_system_liblangtag="$with_system_libs"])

AC_ARG_WITH(jpeg-turbo,
    AS_HELP_STRING([--with-jpeg-turbo],
        [Use internal libjpeg-turbo library.]),,
    [with_jpeg_turbo=auto])

AC_ARG_WITH(webdav,
    AS_HELP_STRING([--with-webdav],
        [Specify which library to use for webdav implementation.
         Possible values: "neon", "serf", "no". The default value is "neon".
         Example: --with-webdav="serf"]),
    WITH_WEBDAV=$withval,
    WITH_WEBDAV="neon")

AC_ARG_WITH(linker-hash-style,
    AS_HELP_STRING([--with-linker-hash-style],
        [Use linker with --hash-style=<style> when linking shared objects.
         Possible values: "sysv", "gnu", "both". The default value is "gnu"
         if supported on the build system, and "sysv" otherwise.]))

AC_ARG_WITH(jdk-home,
    AS_HELP_STRING([--with-jdk-home=<absolute path to JDK home>],
        [If you have installed JDK 1.3 or later on your system please supply the
         path here. Note that this is not the location of the java command but the
         location of the entire distribution.]),
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
    AS_HELP_STRING([--with-java=<java command>],
        [Specify the name of the Java interpreter command. Typically "java"
         which is the default.

         To build without support for Java components, applets, accessibility
         or the XML filters written in Java, use --without-java or --with-java=no.]),
    [ test -z "$with_java" -o "$with_java" = "yes" && with_java=java ],
    [ with_java=java ]
)

AC_ARG_WITH(jvm-path,
    AS_HELP_STRING([--with-jvm-path=<absolute path to parent of jvm home>],
        [Use a specific JVM search path at runtime.
         e.g. use --with-jvm-path=/usr/lib/ to find JRE/JDK in /usr/lib/jvm/]),
,)

AC_ARG_WITH(ant-home,
    AS_HELP_STRING([--with-ant-home=<absolute path to Ant home>],
        [If you have installed Jakarta Ant on your system, please supply the path here.
         Note that this is not the location of the Ant binary but the location
         of the entire distribution.]),
,)

AC_ARG_WITH(symbol-config,
    AS_HELP_STRING([--with-symbol-config],
        [Configuration for the crashreport symbol upload]),
        [],
        [with_symbol_config=no])

AC_ARG_WITH(export-validation,
    AS_HELP_STRING([--with-export-validation],
        [If you want the exported files to be validated. Right now limited to OOXML and ODF files.
         Note: You need executable scripts officeotron and odfvalidator that take the path to the file.
         See https://cgit.freedesktop.org/libreoffice/contrib/dev-tools/tree/export-validation]),
,)

AC_ARG_WITH(bffvalidator,
    AS_HELP_STRING([--with-bffvalidator=<absolute path to BFFValidator>],
        [Enables export validation for Microsoft Binary formats (doc, xls, ppt).
         Requires installed Microsoft Office Binary File Format Validator.
         Note: export-validation (--with-export-validation) is required to be turned on.
         See https://www.microsoft.com/en-us/download/details.aspx?id=26794]),
,with_bffvalidator=no)

AC_ARG_WITH(junit,
    AS_HELP_STRING([--with-junit=<absolute path to JUnit 4 jar>],
        [Specifies the JUnit 4 jar file to use for JUnit-based tests.
         --without-junit disables those tests. Not relevant in the --without-java case.]),
,with_junit=yes)

AC_ARG_WITH(hamcrest,
    AS_HELP_STRING([--with-hamcrest=<absolute path to hamcrest jar>],
        [Specifies the hamcrest jar file to use for JUnit-based tests.
         --without-junit disables those tests. Not relevant in the --without-java case.]),
,with_hamcrest=yes)

AC_ARG_WITH(perl-home,
    AS_HELP_STRING([--with-perl-home=<abs. path to Perl 5 home>],
        [If you have installed Perl 5 Distribution, on your system, please
         supply the path here. Note that this is not the location of the Perl
         binary but the location of the entire distribution.]),
,)

AC_ARG_WITH(doxygen,
    AS_HELP_STRING(
        [--with-doxygen=<absolute path to doxygen executable>],
        [Specifies the doxygen executable to use when generating ODK C/C++
         documentation. --without-doxygen disables generation of ODK C/C++
         documentation. Not relevant in the --disable-odk case.]),
,with_doxygen=yes)

AC_ARG_WITH(visual-studio,
    AS_HELP_STRING([--with-visual-studio=<2013/2015>],
        [Specify which Visual Studio version to use in case several are
         installed. If not specified, only 2013 is detected automatically
         because 2015 support is currently experimental.]),
,)

AC_ARG_WITH(windows-sdk,
    AS_HELP_STRING([--with-windows-sdk=<7.1(A)/8.0(A)/8.1(A)/10>],
        [Specify which Windows SDK, or "Windows Kit", version to use
         in case the one that came with the selected Visual Studio
         is not what you want for some reason. Note that not all compiler/SDK
         combinations are supported. The intent is that this option should not
         be needed.]),
,)

AC_ARG_WITH(lang,
    AS_HELP_STRING([--with-lang="es sw tu cs sk"],
        [Use this option to build LibreOffice with additional UI language support.
         English (US) is always included by default.
         Separate multiple languages with space.
         For all languages, use --with-lang=ALL.]),
,)

AC_ARG_WITH(locales,
    AS_HELP_STRING([--with-locales="en es pt fr zh kr ja"],
        [Use this option to limit the locale information built in.
         Separate multiple locales with space.
         Very experimental and might well break stuff.
         Just a desperate measure to shrink code and data size.
         By default all the locales available is included.
         This option is completely unrelated to --with-lang.])
    [
                          Affects also our character encoding conversion
                          tables for encodings mainly targeted for a
                          particular locale, like EUC-CN and EUC-TW for
                          zh, ISO-2022-JP for ja.

                          Affects also our add-on break iterator data for
                          some languages.

                          For the default, all locales, don't use this switch at all.
                          Specifying just the language part of a locale means all matching
                          locales will be included.
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

AC_ARG_WITH(iwyu,
    AS_HELP_STRING([--with-iwyu],
        [Use given IWYU binary path to check unneeded includes instead of building.
         Use only if you are hacking on it.]),
,)

AC_ARG_WITH(harfbuzz,
    AS_HELP_STRING([--with-harfbuzz],
        [Enable HarfBuzz support regardless of the platform.
         Experimental only. Use only if working on it.]),
,)

dnl ===================================================================
dnl Branding
dnl ===================================================================

AC_ARG_WITH(branding,
    AS_HELP_STRING([--with-branding=/path/to/images],
        [Use given path to retrieve branding images set.])
    [
                          Search for intro.png about.svg and flat_logo.svg.
                          If any is missing, default ones will be used instead.

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
    ],
,)


AC_ARG_WITH(extra-buildid,
    AS_HELP_STRING([--with-extra-buildid="Tinderbox: Win-x86@6, Branch:master, Date:2012-11-26_00.29.34"],
        [Show addition build identification in about dialog.]),
,)


AC_ARG_WITH(vendor,
    AS_HELP_STRING([--with-vendor="John the Builder"],
        [Set vendor of the build.]),
,)

AC_ARG_WITH(android-package-name,
    AS_HELP_STRING([--with-android-package-name="org.libreoffice"],
        [Set Android package name of the build.]),
,)

AC_ARG_WITH(compat-oowrappers,
    AS_HELP_STRING([--with-compat-oowrappers],
        [Install oo* wrappers in parallel with
         lo* ones to keep backward compatibility.
         Has effect only with make distro-pack-install]),
,)

AC_ARG_WITH(os-version,
    AS_HELP_STRING([--with-os-version=<OSVERSION>],
        [For FreeBSD users, use this option option to override the detected OSVERSION.]),
,)

AC_ARG_WITH(mingw-cross-compiler,
    AS_HELP_STRING([--with-mingw-cross-compiler=<mingw32-g++ command>],
        [Specify the MinGW cross-compiler to use.
         When building on the ODK on Unix and building unowinreg.dll,
         specify the MinGW C++ cross-compiler.]),
,)

AC_ARG_WITH(idlc-cpp,
    AS_HELP_STRING([--with-idlc-cpp=<cpp/ucpp>],
        [Specify the C Preprocessor to use for idlc. Default is ucpp.]),
,)

AC_ARG_WITH(build-version,
    AS_HELP_STRING([--with-build-version="Built by Jim"],
        [Allows the builder to add a custom version tag that will appear in the
         Help/About box for QA purposes.]),
with_build_version=$withval,
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

AC_ARG_WITH(all-tarballs,
    AS_HELP_STRING([--with-all-tarballs],
        [Download all external tarballs unconditionally]))

AC_ARG_WITH(gdrive-client-id,
    AS_HELP_STRING([--with-gdrive-client-id],
        [Provides the client id of the application for OAuth2 authentication
        on Google Drive. If either this or --with-gdrive-client-secret is
        empty, the feature will be disabled]),
)

AC_ARG_WITH(gdrive-client-secret,
    AS_HELP_STRING([--with-gdrive-client-secret],
        [Provides the client secret of the application for OAuth2
        authentication on Google Drive. If either this or
        --with-gdrive-client-id is empty, the feature will be disabled]),
)

AC_ARG_WITH(alfresco-cloud-client-id,
    AS_HELP_STRING([--with-alfresco-cloud-client-id],
        [Provides the client id of the application for OAuth2 authentication
        on Alfresco Cloud. If either this or --with-alfresco-cloud-client-secret is
        empty, the feature will be disabled]),
)

AC_ARG_WITH(alfresco-cloud-client-secret,
    AS_HELP_STRING([--with-alfresco-cloud-client-secret],
        [Provides the client secret of the application for OAuth2
        authentication on Alfresco Cloud. If either this or
        --with-alfresco-cloud-client-id is empty, the feature will be disabled]),
)

AC_ARG_WITH(onedrive-client-id,
    AS_HELP_STRING([--with-onedrive-client-id],
        [Provides the client id of the application for OAuth2 authentication
        on OneDrive. If either this or --with-onedrive-client-secret is
        empty, the feature will be disabled]),
)

AC_ARG_WITH(onedrive-client-secret,
    AS_HELP_STRING([--with-onedrive-client-secret],
        [Provides the client secret of the application for OAuth2
        authentication on OneDrive. If either this or
        --with-onedrive-client-id is empty, the feature will be disabled]),
)
dnl ===================================================================
dnl Do we want to use pre-build binary tarball for recompile
dnl ===================================================================

if test "$enable_library_bin_tar" = "yes" ; then
    USE_LIBRARY_BIN_TAR=TRUE
else
    USE_LIBRARY_BIN_TAR=
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
    AS_HELP_STRING([--with-macosx-sdk=<version>],
        [Use a specific SDK for building.])
    [
                          e. g.: --with-macosx-sdk=10.8

                          there are 3 options to control the MacOSX build:
                          --with-macosx-sdk (referred as 'sdk' below)
                          --with-macosx-version-min-required (referred as 'min' below)
                          --with-macosx-version-max-allowed (referred as 'max' below)

                          the connection between these value and the default they take is as follow:
                          ( ? means not specified on the command line, s means the SDK version found,
                          constraint: 8 <= x <= y <= z)

                          ==========================================
                           command line      || config result
                          ==========================================
                          min  | max  | sdk  || min  | max  | sdk  |
                          ?    | ?    | ?    || 10.8 | 10.s | 10.s |
                          ?    | ?    | 10.x || 10.8 | 10.x | 10.x |
                          ?    | 10.x | ?    || 10.8 | 10.s | 10.s |
                          ?    | 10.x | 10.y || 10.8 | 10.x | 10.y |
                          10.x | ?    | ?    || 10.x | 10.s | 10.s |
                          10.x | ?    | 10.y || 10.x | 10.y | 10.y |
                          10.x | 10.y | ?    || 10.x | 10.y | 10.y |
                          10.x | 10.y | 10.z || 10.x | 10.y | 10.z |


                          see: http://developer.apple.com/library/mac/#technotes/tn2064/_index.html
                          for a detailed technical explanation of these variables

                          Note: MACOSX_DEPLOYMENT_TARGET will be set to the value of 'min'.
    ],
,)

AC_ARG_WITH(macosx-version-min-required,
    AS_HELP_STRING([--with-macosx-version-min-required=<version>],
        [set the minimum OS version needed to run the built LibreOffice])
    [
                          e. g.: --with-macos-version-min-required=10.8
                          see --with-macosx-sdk for more info
    ],
,)

AC_ARG_WITH(macosx-version-max-allowed,
    AS_HELP_STRING([--with-macosx-version-max-allowed=<version>],
        [set the maximum allowed OS version the LibreOffice compilation can use APIs from])
    [
                          e. g.: --with-macos-version-max-allowed=10.8
                          see --with-macosx-sdk for more info
    ],
,)


dnl ===================================================================
dnl options for stuff used during cross-compilation build
dnl Not quite superseded by --with-build-platform-configure-options.
dnl TODO: check, if the "force" option is still needed anywhere.
dnl ===================================================================

AC_ARG_WITH(system-icu-for-build,
    AS_HELP_STRING([--with-system-icu-for-build=yes/no/force],
        [Use icu already on system for build tools (cross-compilation only).]))


dnl ===================================================================
dnl check for required programs (grep, awk, sed, bash)
dnl ===================================================================

pathmunge ()
{
    if test -n "$1"; then
        if test "$build_os" = "cygwin"; then
            if test "$GNUMAKE_WIN_NATIVE" = "TRUE" ; then
                PathFormat "$1"
                new_path=`cygpath -sm "$formatted_path"`
            else
                PathFormat "$1"
                new_path=`cygpath -u "$formatted_path"`
            fi
        else
            new_path="$1"
        fi
        if ! echo "$LO_PATH" | $EGREP -q "(^|:)$1($|:)"; then
            if test "$2" = "after"; then
                LO_PATH="$LO_PATH${P_SEP}$new_path"
            else
                LO_PATH="$new_path${P_SEP}$LO_PATH"
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
for a in $GNUTAR gtar gnutar bsdtar tar /usr/sfw/bin/gtar; do
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
if test x$STRIP_COMPONENTS = xunsupported; then
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
    AC_DEFINE(HAVE_FEATURE_MULTIUSER_ENVIRONMENT)
fi

DISABLE_EXPORT=''
if test "$enable_export" != no; then
    BUILD_TYPE="$BUILD_TYPE EXPORT"
else
    DISABLE_EXPORT='TRUE'
    SCPDEFS="$SCPDES -DDISABLE_EXPORT"
fi
AC_SUBST(DISABLE_EXPORT)

# Whether to build "avmedia" functionality or not.

if test -z "$enable_avmedia"; then
    enable_avmedia=yes
fi

if test "$enable_avmedia" = yes; then
    BUILD_TYPE="$BUILD_TYPE AVMEDIA"
    AC_DEFINE(HAVE_FEATURE_AVMEDIA)
else
    SCPDEFS="$SCPDEFS -DDISABLE_AVMEDIA"
fi

# Decide whether to build database connectivity stuff (including
# Base) or not. We probably don't want to on non-desktop OSes.

if test -z "$enable_database_connectivity"; then
    # --disable-database-connectivity is unfinished work in progress
    # and the iOS test app doesn't link if we actually try to use it.
    # if test $_os != iOS -a $_os != Android; then
        enable_database_connectivity=yes
    # fi
fi

if test "$enable_database_connectivity" = yes; then
    BUILD_TYPE="$BUILD_TYPE DBCONNECTIVITY"
    AC_DEFINE(HAVE_FEATURE_DBCONNECTIVITY)
fi

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
    AC_DEFINE(HAVE_FEATURE_SCRIPTING)
else
    DISABLE_SCRIPTING='TRUE'
    SCPDEFS="$SCPDEFS -DDISABLE_SCRIPTING"
fi

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

# remenber SYSBASE value
AC_SUBST(SYSBASE)

dnl ===================================================================
dnl  Sort out various gallery compilation options
dnl ===================================================================
AC_MSG_CHECKING([how to build and package galleries])
if test -n "${with_galleries}"; then
    if test "$with_galleries" = "build"; then
        WITH_GALLERY_BUILD=TRUE
        AC_MSG_RESULT([build from source images internally])
    elif test "$with_galleries" = "no"; then
        WITH_GALLERY_BUILD=
        AC_MSG_RESULT([disable non-internal gallery build])
    else
        AC_MSG_ERROR([unknown value --with-galleries=$with_galleries])
    fi
else
    if test $_os != iOS -a $_os != Android; then
        WITH_GALLERY_BUILD=TRUE
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
if test "$_os" = "WINNT" -a "$WITH_MINGW" != "yes"; then
    # on windows/VC build do not use ccache
    CCACHE=""
elif test "$enable_ccache" = "yes" -o \( "$enable_ccache" = "" -a "$enable_icecream" != "yes" \); then
    case "%$CC%$CXX%" in
    # If $CC and/or $CXX already contain "ccache" (possibly suffixed with some version number etc),
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
            AC_MSG_WARN([ccache's cache size is less than 1GB using it is counter-productive: Disabling auto-ccache detection])
            add_warning "ccache's cache size is less than 1GB using it is counter-productive: auto-ccache detection disabled"
        else
            # warn that ccache may be too small for debug build
            AC_MSG_WARN([ccache's cache size is less than 5GB using it may be counter-productive for debug or symbol-enabled build])
            add_warning "ccache's cache size is less than 5GB using it may be counter-productive for debug or symbol-enabled build"
        fi
    else
        if test $ccache_size -lt 5; then
            #warn that ccache may be too small for debug build
            AC_MSG_WARN([ccache's cache size is less than 5GB using it may be counter-productive for debug or symbol-enabled build])
            add_warning "ccache's cache size is less than 5GB using it may be counter-productive for debug or symbol-enabled build"
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

COMPATH=`dirname "$CC"`
if test "$COMPATH" = "."; then
    AC_PATH_PROGS(COMPATH, $CC)
    dnl double square bracket to get single because of M4 quote...
    COMPATH=`echo $COMPATH | $SED "s@/[[^/:]]*\\\$@@"`
fi
COMPATH=`echo $COMPATH | $SED "s@/[[Bb]][[Ii]][[Nn]]\\\$@@"`

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
dnl Export file validation
dnl ===================================================================
AC_MSG_CHECKING([whether to enable export file validation])
if test "$with_export_validation" = yes; then
    AC_MSG_RESULT([yes])
    AC_DEFINE(HAVE_EXPORT_VALIDATION)

    AC_PATH_PROGS(ODFVALIDATOR, odfvalidator)
    if test -z "$ODFVALIDATOR"; then
        # remember to download the ODF toolkit with validator later
        AC_MSG_NOTICE([no odfvalidator found, will download it])
        BUILD_TYPE="$BUILD_TYPE ODFVALIDATOR"
        ODFVALIDATOR="bin/odfvalidator.sh"

        # and fetch name of odfvalidator jar name from download.lst
        ODFVALIDATOR_JAR=`sed -ne "s/export *ODFVALIDATOR_JAR *:= *\(.*\) */\1/p" download.lst`
        AC_SUBST(ODFVALIDATOR_JAR)

        if test -z "$ODFVALIDATOR_JAR"; then
            AC_MSG_ERROR([odfvalidator jar location is not determined (--with-export-validation)])
        fi

        if test "$build_os" = "cygwin"; then
            # In case of CygWin it will be executed from Windows,
            # so we need to run bash and absolute path to validator
            # so instead of "odfvalidator" it will be
            # something like "bash.exe C:\cygwin\opt\lo\bin\odfvalidator"
            ODFVALIDATOR="bash.exe `cygpath -m "$ODFVALIDATOR"`"
        fi
    fi
    AC_SUBST(ODFVALIDATOR)

    if test "$build_os" = "cygwin"; then
        # In case of CygWin it will be executed from Windows,
        # so we need to run bash and absolute path to validator
        # so instead of "officeotron" it will be
        # something like "bash.exe C:\cygwin\opt\lo\bin\officeotron"
        AC_PATH_PROGS(OFFICEOTRON, officeotron)
        if test -z "$OFFICEOTRON"; then
            AC_MSG_ERROR([officeotron not found, but required by --with-export-validation])
        fi
        OFFICEOTRON="bash.exe `cygpath -m "$OFFICEOTRON"`"
    else
        AC_CHECK_PROGS(OFFICEOTRON, officeotron)
        if test -z "$OFFICEOTRON"; then
            AC_MSG_ERROR([officeotron not found, but required by --with-export-validation])
        fi
    fi
    OFFICEOTRON_VER=`$OFFICEOTRON --version | $AWK -F. '{ print \$1*10000+\$2*100+\$3 }'`
    if test 0"$OFFICEOTRON_VER" -lt 704; then
        AC_MSG_ERROR([officeotron too old])
    fi
    AC_SUBST(OFFICEOTRON)

else
    AC_MSG_RESULT([no])
fi

AC_MSG_CHECKING([for Microsoft Binary File Format Validator])
if test "$with_bffvalidator" != "no"; then
    AC_DEFINE(HAVE_BFFVALIDATOR)

    if test "$with_export_validation" != "yes"; then
        AC_MSG_ERROR([Please enable export validation (-with-export-validation)!])
    fi

    if test "$with_bffvalidator" = "yes"; then
        BFFVALIDATOR=`win_short_path_for_make "$PROGRAMFILES/Microsoft Office/BFFValidator/BFFValidator.exe"`
    else
        BFFVALIDATOR="$with_bffvalidator"
    fi

    if test "$build_os" = "cygwin"; then
        if test -n "$BFFVALIDATOR" -a -e "`cygpath $BFFVALIDATOR`"; then
            AC_MSG_RESULT($BFFVALIDATOR)
        else
            AC_MSG_ERROR([bffvalidator not found, but required by --with-bffvalidator])
        fi
    elif test -n "$BFFVALIDATOR"; then
        # We are not in CygWin but need to run Windows binary with wine
        AC_PATH_PROGS(WINE, wine)

        # so swap in a shell wrapper that converts paths transparently
        BFFVALIDATOR_EXE="$BFFVALIDATOR"
        BFFVALIDATOR="$SRC_ROOT/bin/bffvalidator.sh"
        AC_SUBST(BFFVALIDATOR_EXE)
        AC_MSG_RESULT($BFFVALIDATOR)
    else
        AC_MSG_ERROR([bffvalidator not found, but required by --with-bffvalidator])
    fi
    AC_SUBST(BFFVALIDATOR)
else
    AC_MSG_RESULT([no])
fi


dnl ===================================================================
dnl Check OS X SDK and compiler
dnl ===================================================================

if test $_os = Darwin; then

    # If no --with-macosx-sdk option is given, look for one

    # The intent is that for "most" Mac-based developers, a suitable
    # SDK will be found automatically without any configure options.

    # For developers with a current Xcode, the lowest-numbered SDK
    # higher than or equal to the minimum required should be found.

    AC_MSG_CHECKING([what Mac OS X SDK to use])

    for _macosx_sdk in $with_macosx_sdk 10.8 10.9 10.10 10.11; do
        MACOSX_SDK_PATH=`xcrun --sdk macosx${_macosx_sdk} --show-sdk-path 2> /dev/null`
        if test -d "$MACOSX_SDK_PATH"; then
            with_macosx_sdk="${_macosx_sdk}"
            break
        else
            MACOSX_SDK_PATH="`xcode-select -print-path`/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${_macosx_sdk}.sdk"
            if test -d "$MACOSX_SDK_PATH"; then
                with_macosx_sdk="${_macosx_sdk}"
                break
            fi
        fi
    done
    if test ! -d "$MACOSX_SDK_PATH"; then
        AC_MSG_ERROR([Could not figure out the location of a Mac OS X SDK and its version])
    fi
    AC_MSG_RESULT([SDK $with_macosx_sdk at $MACOSX_SDK_PATH])

    case $with_macosx_sdk in
    10.8)
        MACOSX_SDK_VERSION=1080
        ;;
    10.9)
        MACOSX_SDK_VERSION=1090
        ;;
    10.10)
        MACOSX_SDK_VERSION=101000
        ;;
    10.11)
        MACOSX_SDK_VERSION=101100
        ;;
    *)
        AC_MSG_ERROR([with-macosx-sdk $with_macosx_sdk is not a supported value, supported values are 10.8--11])
        ;;
    esac

    if test "$with_macosx_version_min_required" = "" ; then
        with_macosx_version_min_required="10.8";
    fi

    if test "$with_macosx_version_max_allowed" = "" ; then
        with_macosx_version_max_allowed="$with_macosx_sdk"
    fi

    # export this so that "xcrun" invocations later return matching values
    DEVELOPER_DIR="${MACOSX_SDK_PATH%/SDKs*}"
    DEVELOPER_DIR="${DEVELOPER_DIR%/Platforms*}"
    export DEVELOPER_DIR
    FRAMEWORKSHOME="$MACOSX_SDK_PATH/System/Library/Frameworks"
    MACOSX_DEPLOYMENT_TARGET="$with_macosx_version_min_required"

    case "$with_macosx_version_min_required" in
    10.8)
        MAC_OS_X_VERSION_MIN_REQUIRED="1080"
        ;;
    10.9)
        MAC_OS_X_VERSION_MIN_REQUIRED="1090"
        ;;
    10.10)
        MAC_OS_X_VERSION_MIN_REQUIRED="101000"
        ;;
    10.11)
        MAC_OS_X_VERSION_MIN_REQUIRED="101100"
        ;;
    *)
        AC_MSG_ERROR([with-macosx-version-min-required $with_macosx_version_min_required is not a supported value, supported values are 10.8--11])
        ;;
    esac

    LIBTOOL=libtool
    INSTALL_NAME_TOOL=install_name_tool
    if test -z "$save_CC"; then
        AC_MSG_CHECKING([what compiler to use])
        stdlib=-stdlib=libc++
        if test "$ENABLE_LTO" = TRUE; then
            lto=-flto
        fi
        CC="`xcrun -find clang` -m64 $lto -mmacosx-version-min=$with_macosx_version_min_required -isysroot $MACOSX_SDK_PATH"
        CXX="`xcrun -find clang++` -m64 $lto $stdlib -mmacosx-version-min=$with_macosx_version_min_required -isysroot $MACOSX_SDK_PATH"
        INSTALL_NAME_TOOL=`xcrun -find install_name_tool`
        AR=`xcrun -find ar`
        NM=`xcrun -find nm`
        STRIP=`xcrun -find strip`
        LIBTOOL=`xcrun -find libtool`
        RANLIB=`xcrun -find ranlib`
        AC_MSG_RESULT([$CC and $CXX])
    fi

    case "$with_macosx_version_max_allowed" in
    10.8)
        MAC_OS_X_VERSION_MAX_ALLOWED="1080"
        ;;
    10.9)
        MAC_OS_X_VERSION_MAX_ALLOWED="1090"
        ;;
    10.10)
        MAC_OS_X_VERSION_MAX_ALLOWED="101000"
        ;;
    10.11)
        MAC_OS_X_VERSION_MAX_ALLOWED="101100"
        ;;
    *)
        AC_MSG_ERROR([with-macosx-version-max-allowed $with_macosx_version_max_allowed is not a supported value, supported values are 10.8--11])
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
        AC_MSG_ERROR([the version maximum allowed cannot be greater than the sdk level])
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
        # the App Store, the "3rd Party Mac Developer" one. I think it works best to the
        # "Developer ID Application" one.

        identity=`security find-identity -p codesigning -v 2>/dev/null | grep 'Developer ID Application:' | $AWK '{print $2}' |head -1`
        if test -n "$identity"; then
            MACOSX_CODESIGNING_IDENTITY=$identity
            pretty_name=`security find-identity -p codesigning -v | grep "$MACOSX_CODESIGNING_IDENTITY" | sed -e 's/^[[^"]]*"//' -e 's/"//'`
            AC_MSG_RESULT([yes, using the identity $MACOSX_CODESIGNING_IDENTITY for $pretty_name])
        else
            AC_MSG_ERROR([cannot determine identity to use])
        fi
    elif test -n "$enable_macosx_code_signing" -a "$enable_macosx_code_signing" != no ; then
        MACOSX_CODESIGNING_IDENTITY=$enable_macosx_code_signing
        pretty_name=`security find-identity -p codesigning -v | grep "$MACOSX_CODESIGNING_IDENTITY" | sed -e 's/^[[^"]]*"//' -e 's/"//'`
        AC_MSG_RESULT([yes, using the identity $MACOSX_CODESIGNING_IDENTITY for $pretty_name])
    else
        AC_MSG_RESULT([no])
    fi

    AC_MSG_CHECKING([whether to create a Mac App Store package])

    if test -n "$enable_macosx_package_signing" -a -z "$MACOSX_CODESIGNING_IDENTITY"; then
        AC_MSG_ERROR([You forgot --enable-macosx-code-signing])
    elif test "$enable_macosx_package_signing" = yes; then
        # By default use the first suitable certificate.
        # It should be a "3rd Party Mac Developer Installer" one

        identity=`security find-identity -v 2>/dev/null | grep '3rd Party Mac Developer Installer:' | awk '{print $2}' |head -1`
        if test -n "$identity"; then
            MACOSX_PACKAGE_SIGNING_IDENTITY=$identity
            pretty_name=`security find-identity -v | grep "$MACOSX_PACKAGE_SIGNING_IDENTITY" | sed -e 's/^[[^"]]*"//' -e 's/"//'`
            AC_MSG_RESULT([yes, using the identity $MACOSX_PACKAGE_SIGNING_IDENTITY for $pretty_name])
        else
            AC_MSG_ERROR([Could not find any suitable '3rd Party Mac Developer Installer' certificate])
        fi
    elif test -n "$enable_macosx_package_signing"; then
        MACOSX_PACKAGE_SIGNING_IDENTITY=$enable_macosx_package_signing
        pretty_name=`security find-identity -v | grep "$MACOSX_PACKAGE_SIGNING_IDENTITY" | sed -e 's/^[[^"]]*"//' -e 's/"//'`
        AC_MSG_RESULT([yes, using the identity $MACOSX_PACKAGE_SIGNING_IDENTITY for $pretty_name])
    else
        AC_MSG_RESULT([no])
    fi

    if test -n "$MACOSX_CODESIGNING_IDENTITY" -a -n "$MACOSX_PACKAGE_SIGNING_IDENTITY" -a "$MACOSX_CODESIGNING_IDENTITY" = "$MACOSX_PACKAGE_SIGNING_IDENTITY"; then
        AC_MSG_ERROR([You should not use the same identity for code and package signing])
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

    AC_MSG_CHECKING([what OS X app name to use])
    MACOSX_APP_NAME="$with_macosx_app_name"
    AC_MSG_RESULT([$MACOSX_APP_NAME])

fi
AC_SUBST(MACOSX_SDK_PATH)
AC_SUBST(MACOSX_SDK_VERSION)
AC_SUBST(MACOSX_DEPLOYMENT_TARGET)
AC_SUBST(MAC_OS_X_VERSION_MIN_REQUIRED)
AC_SUBST(MAC_OS_X_VERSION_MAX_ALLOWED)
AC_SUBST(INSTALL_NAME_TOOL)
AC_SUBST(LIBTOOL) # Note that the OS X libtool command is unrelated to GNU libtool
AC_SUBST(MACOSX_CODESIGNING_IDENTITY)
AC_SUBST(MACOSX_PACKAGE_SIGNING_IDENTITY)
AC_SUBST(ENABLE_MACOSX_SANDBOX)
AC_SUBST(MACOSX_BUNDLE_IDENTIFIER)
AC_SUBST(MACOSX_APP_NAME)

dnl ===================================================================
dnl Check iOS SDK and compiler
dnl ===================================================================

if test $_os = iOS; then

    if test "$enable_64_bit" = "" -o "$enable_64_bit" = "no"; then
        :
    else
        BITNESS_OVERRIDE=64
    fi

    AC_MSG_CHECKING([what iOS SDK to use])

    if test "$enable_ios_simulator" = yes; then
        platform=iPhoneSimulator
    else
        platform=iPhoneOS
    fi

    xcode_developer=`xcode-select -print-path`

    current_sdk_ver=9.3
    for sdkver in 9.3 9.2; do
        t=$xcode_developer/Platforms/$platform.platform/Developer/SDKs/$platform$sdkver.sdk
        if test -d $t; then
            ios_sdk=$sdkver
            sysroot=$t
            break
        fi
    done

    if test -z "$sysroot"; then
        AC_MSG_ERROR([Could not find iOS SDK, expected something like $xcode_developer/Platforms/$platform.platform/Developer/SDKs/${platform}${current_sdk_ver}.sdk])
    fi

    AC_MSG_RESULT($sysroot)

    XCODEBUILD_SDK=`echo $platform | tr A-Z a-z`$ios_sdk

    if test "$enable_ios_simulator" = yes; then
        if test "$BITNESS_OVERRIDE" = 64; then
            XCODE_ARCHS=x86_64
            versionmin=-mios-simulator-version-min=7.0
        else
            XCODE_ARCHS=i386
            versionmin=-mios-simulator-version-min=7.0
        fi
    else
        platform=iPhoneOS
        if test "$BITNESS_OVERRIDE" = 64; then
            XCODE_ARCHS=arm64
        else
            XCODE_ARCHS=armv7
        fi
        versionmin=-miphoneos-version-min=7.0
    fi

    # LTO is not really recommended for iOS builds,
    # the link time will be astronomical
    if test "$ENABLE_LTO" = TRUE; then
        lto=-flto
    fi
    # Just add -fvisibility=hidden to CC and CXX directly so that the 3rd-party libs also
    # get compiled with it, to avoid ld warnings when linking all that together into one
    # executable.

    XCODE_CLANG_CXX_LIBRARY=libc++
    stdlib="-stdlib=$XCODE_CLANG_CXX_LIBRARY"

    CC="`xcrun -find clang` -arch $XCODE_ARCHS -fvisibility=hidden -isysroot $sysroot $lto $versionmin"
    CXX="`xcrun -find clang++` -arch $XCODE_ARCHS -fvisibility=hidden $stdlib -isysroot $sysroot $lto $versionmin"

    INSTALL_NAME_TOOL=`xcrun -find install_name_tool`
    AR=`xcrun -find ar`
    NM=`xcrun -find nm`
    STRIP=`xcrun -find strip`
    LIBTOOL=`xcrun -find libtool`
    RANLIB=`xcrun -find ranlib`
fi

AC_SUBST(XCODE_CLANG_CXX_LIBRARY)
AC_SUBST(XCODE_ARCHS)
AC_SUBST(XCODEBUILD_SDK)

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

if test $_os = Darwin; then
    LIBO_BIN_FOLDER=MacOS
    LIBO_ETC_FOLDER=Resources
    LIBO_LIBEXEC_FOLDER=MacOS
    LIBO_LIB_FOLDER=Frameworks
    LIBO_LIB_PYUNO_FOLDER=Resources
    LIBO_SHARE_FOLDER=Resources
    LIBO_SHARE_HELP_FOLDER=Resources/help
    LIBO_SHARE_JAVA_FOLDER=Resources/java
    LIBO_SHARE_PRESETS_FOLDER=Resources/presets
    LIBO_SHARE_READMES_FOLDER=Resources/readmes
    LIBO_SHARE_RESOURCE_FOLDER=Resources/resource
    LIBO_SHARE_SHELL_FOLDER=Resources/shell
    LIBO_URE_BIN_FOLDER=MacOS
    LIBO_URE_ETC_FOLDER=Resources/ure/etc
    LIBO_URE_LIB_FOLDER=Frameworks
    LIBO_URE_MISC_FOLDER=Resources/ure/share/misc
    LIBO_URE_SHARE_JAVA_FOLDER=Resources/java
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
    LIBO_SHARE_READMES_FOLDER=readmes
    LIBO_SHARE_RESOURCE_FOLDER=program/resource
    LIBO_SHARE_SHELL_FOLDER=program/shell
    LIBO_URE_BIN_FOLDER=program
    LIBO_URE_ETC_FOLDER=program
    LIBO_URE_LIB_FOLDER=program
    LIBO_URE_MISC_FOLDER=program
    LIBO_URE_SHARE_JAVA_FOLDER=program/classes
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
    LIBO_SHARE_READMES_FOLDER=readmes
    LIBO_SHARE_RESOURCE_FOLDER=program/resource
    LIBO_SHARE_SHELL_FOLDER=program/shell
    LIBO_URE_BIN_FOLDER=program
    LIBO_URE_ETC_FOLDER=program
    LIBO_URE_LIB_FOLDER=program
    LIBO_URE_MISC_FOLDER=program
    LIBO_URE_SHARE_JAVA_FOLDER=program/classes
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
AC_DEFINE_UNQUOTED(LIBO_URE_MISC_FOLDER,"$LIBO_URE_MISC_FOLDER")
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
AC_SUBST(LIBO_SHARE_READMES_FOLDER)
AC_SUBST(LIBO_SHARE_RESOURCE_FOLDER)
AC_SUBST(LIBO_URE_BIN_FOLDER)
AC_SUBST(LIBO_URE_ETC_FOLDER)
AC_SUBST(LIBO_URE_LIB_FOLDER)
AC_SUBST(LIBO_URE_MISC_FOLDER)
AC_SUBST(LIBO_URE_SHARE_JAVA_FOLDER)

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

if test "$cross_compiling" = "yes"; then
    export CROSS_COMPILING=TRUE
    SCPDEFS="$SCPDEFS -DCROSS_COMPILING"
else
    CROSS_COMPILING=
    BUILD_TYPE="$BUILD_TYPE NATIVE"
fi
AC_SUBST(CROSS_COMPILING)

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
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[ return 0; ]])],[ ISYSTEM="-isystem " ],[])
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
    2013)
        vsversion=12.0;;
    2015)
        vsversion=14.0;;
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
        # By default we prefer 2013/2015, in this order
        vsversions="12.0 14.0"
    fi
}

find_msvs()
{
    # Find Visual Studio 2013/2015
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

win_get_env_from_vsvars32bat()
{
    WRAPPERBATCHFILEPATH="`mktemp -t wrpXXXXXX.bat`"
    echo -e "@setlocal\r\n" >> $WRAPPERBATCHFILEPATH
    echo -e "@call \"`cygpath -w $VC_PRODUCT_DIR`/../Common7/Tools/vsvars32.bat\"\r\n" >> $WRAPPERBATCHFILEPATH
    echo -e "@echo %$1%\r\n" >> $WRAPPERBATCHFILEPATH
    echo -e "@endlocal\r\n" >> $WRAPPERBATCHFILEPATH
    chmod +x $WRAPPERBATCHFILEPATH
    _win_get_env_from_vsvars32bat="`$WRAPPERBATCHFILEPATH | $SED -e s/^M^J//`"
    rm -f $WRAPPERBATCHFILEPATH
    echo $_win_get_env_from_vsvars32bat
}

find_ucrt()
{
    reg_get_value_32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v10.0/InstallationFolder"
    if test -n "$regvalue"; then
        PathFormat "$regvalue"
        UCRTSDKDIR=$formatted_path
        reg_get_value_32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v10.0/ProductVersion"
        UCRTVERSION=$regvalue
    fi
    if test -z "$UCRTSDKDIR"; then
        if test -f "$VC_PRODUCT_DIR/../Common7/Tools/vsvars32.bat"; then
            PathFormat "`win_get_env_from_vsvars32bat "UniversalCRTSdkDir"`"
            UCRTSDKDIR=$formatted_path
            UCRTVERSION=`win_get_env_from_vsvars32bat "UCRTVersion"`
        fi
    fi
}

find_msvc()
{
    # Find Visual C++ 2013/2015
    # Args: $1 (optional) : The VS version year
    # Return values: $vctest, $vcyear, $vcnum, $vcnumwithdot

    unset vctest vcnum vcnumwithdot vcexpress

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
        12.0)
            vcyear=2013
            vcnum=120
            ;;
        14.0)
            vcyear=2015
            vcnum=140
            ;;
        esac
        reg_get_value_32 HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VCExpress/$vcnumwithdot/Setup/VC/ProductDir
        if test -n "$regvalue" -a "$regvalue" = "$vctest" ; then
            vcexpress=Express
        fi
    fi
}

SOLARINC=
SHOWINCLUDES_PREFIX=
MSBUILD_PATH=
DEVENV=
if test "$_os" = "WINNT"; then
    if test "$WITH_MINGW" != "yes"; then
        AC_MSG_CHECKING([Visual C++])

        find_msvc "$with_visual_studio"
        if test -z "$vctest"; then
            if test -n "$with_visual_studio"; then
                AC_MSG_ERROR([No Visual Studio $with_visual_studio installation found])
            else
                AC_MSG_ERROR([No Visual Studio 2013/2015 installation found])
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
                if test -f "$vctest/bin/x86_amd64/cl.exe" -a "$vcexpress" = "Express"; then
                    VC_PRODUCT_DIR=$vctest
                else
                    AC_MSG_ERROR([No compiler (cl.exe) in $vctest/bin/amd64/cl.exe or $vctest/bin/x86_amd64/cl.exe])
                fi
            fi
        fi

        VC_PRODUCT_DIR=`win_short_path_for_make "$VC_PRODUCT_DIR"`
        AC_MSG_RESULT([$VC_PRODUCT_DIR])

        AC_MSG_CHECKING([Is Visual C++ Express])
        if test "$vcexpress" = "Express" ; then
            AC_MSG_RESULT([Yes])
        else
            AC_MSG_RESULT([No])
        fi

        UCRTSDKDIR=
        UCRTVERSION=

        AC_MSG_CHECKING([whether UCRT is needed for this compiler version])
        if test "$vcnum" = "120"; then
            AC_MSG_RESULT([No])
        else
            AC_MSG_RESULT([Yes])
            AC_MSG_CHECKING([for UCRT location])
            find_ucrt
            if test -n "$UCRTSDKDIR"; then
                AC_MSG_RESULT([found])
                PathFormat "${UCRTSDKDIR}Include/$UCRTVERSION/ucrt"
                ucrtincpath_formatted=$formatted_path
                # SOLARINC is used for external modules and must be set too.
                # And no, it's not sufficient to set SOLARINC only, as configure
                # itself doesn't honour it.
                SOLARINC="$SOLARINC -I$ucrtincpath_formatted"
                CFLAGS="$CFLAGS -I$ucrtincpath_formatted"
                CXXFLAGS="$CXXFLAGS -I$ucrtincpath_formatted"
                CPPFLAGS="$CPPFLAGS -I$ucrtincpath_formatted"
            else
                AC_MSG_ERROR([not found])
            fi
        fi
        AC_SUBST(UCRTSDKDIR)
        AC_SUBST(UCRTVERSION)

        # Find the proper version of MSBuild.exe to use based on the VS version
        reg_get_value_32 HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/MSBuild/$vcnumwithdot/MSBuildOverrideTasksPath
        if test -n "$regvalue" ; then
            MSBUILD_PATH=`win_short_path_for_make "$regvalue"`
        fi

        # Find the version of devenv.exe
        DEVENV="$VC_PRODUCT_DIR/../Common7/IDE/devenv.exe"
        if test ! -e "$DEVENV" -a "$vcnum" = "120"; then
            # for Visual Studio 2013 Express, fall back
            DEVENV="$VC_PRODUCT_DIR/../Common7/IDE/WDExpress.exe"
        fi
        if test ! -e "$DEVENV"; then
            AC_MSG_ERROR([No devenv.exe found, Visual Studio installation broken?])
        fi

        dnl ===========================================================
        dnl  Check for the corresponding mspdb*.dll
        dnl ===========================================================

        MSPDB_PATH=

        if test "$BITNESS_OVERRIDE" = ""; then
            if test "$vcnum" = "120"; then
                MSPDB_PATH="$VC_PRODUCT_DIR/../VC/bin"
            else
                MSPDB_PATH="$VC_PRODUCT_DIR/../Common7/IDE"
            fi
        else
            if test "$vcexpress" = "Express"; then
                MSPDB_PATH="$VC_PRODUCT_DIR/bin"
            else
                MSPDB_PATH="$VC_PRODUCT_DIR/bin/amd64"
            fi
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
                if test "$vcexpress" = "Express"; then
                   if test -f "$VC_PRODUCT_DIR/bin/x86_amd64/cl.exe"; then
                        CC="$VC_PRODUCT_DIR/bin/x86_amd64/cl.exe"
                   fi
                else
                   if test -f "$VC_PRODUCT_DIR/bin/amd64/cl.exe"; then
                        CC="$VC_PRODUCT_DIR/bin/amd64/cl.exe"
                   fi
                fi
            fi

            # This gives us a posix path with 8.3 filename restrictions
            CC=`win_short_path_for_make "$CC"`
        fi

        if test -n "$CC"; then
            # Remove /cl.exe from CC case insensitive
            AC_MSG_RESULT([found Visual C++ $vcyear $vcexpress ($CC)])
            if test "$BITNESS_OVERRIDE" = ""; then
                COMPATH=`echo "$CC" | $SED -e 's@\/[[Bb]][[Ii]][[Nn]]\/[[cC]][[lL]]\.[[eE]][[xX]][[eE]].*@@' -e 's@^.* @@'`
            else
                if test -n "$VC_PRODUCT_DIR"; then
                    COMPATH=$VC_PRODUCT_DIR
                fi
            fi
            if test "$BITNESS_OVERRIDE" = ""; then
                dnl since MSVC 2012, default for x86 is -arch:SSE2:
                CC="$CC -arch:SSE"
            fi
            export INCLUDE=`cygpath -d "$COMPATH/Include"`

            PathFormat "$COMPATH"
            COMPATH="$formatted_path"

            VCVER=$vcnum
            MSVSVER=$vcyear

            # The WINDOWS_SDK_ACCEPTABLE_VERSIONS is mostly an educated guess...  Assuming newer ones
            # are always "better", we list them in reverse chronological order.

            case $vcnum in
            120)
                COMEX=15
                WINDOWS_SDK_ACCEPTABLE_VERSIONS="8.1A 8.1 8.0 7.1A"
                ;;
            140)
                COMEX=19
                WINDOWS_SDK_ACCEPTABLE_VERSIONS="10.0A 10.0 8.1A 8.1 8.0 7.1A"
                ;;
            esac

            # The expectation is that --with-windows-sdk should not need to be used
            if test -n "$with_windows_sdk"; then
                case " $WINDOWS_SDK_ACCEPTABLE_VERSIONS " in
                *" "$with_windows_sdk" "*)
                    WINDOWS_SDK_ACCEPTABLE_VERSIONS=$with_windows_sdk
                    ;;
                *)
                    AC_MSG_ERROR([Windows SDK $with_windows_sdk is not known to work with VS $MSVSVER])
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
        dnl Filter out -FIIntrin.h when CC points at clang-cl.exe and needs to
        dnl explicitly state that argument:
        my_CC=
        for i in $CC; do
            case $i in
            -FIIntrin.h)
                ;;
            *)
                my_CC="$my_CC $i"
                ;;
            esac
        done
        SHOWINCLUDES_PREFIX=`$my_CC $CFLAGS -c -showIncludes conftest.c 2>/dev/null | \
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
AC_SUBST(DEVENV)
PathFormat "$MSPDB_PATH"
MSPDB_PATH="$formatted_path"
AC_SUBST(SHOWINCLUDES_PREFIX)

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
    if test "$enable_activex" = "yes" -o "$enable_activex" = "" -a "$vcexpress" != Express; then
        DISABLE_ACTIVEX=""
        AC_MSG_RESULT([yes])
    else
        DISABLE_ACTIVEX="TRUE"
        AC_MSG_RESULT([no])
    fi

    AC_MSG_CHECKING([whether to use ATL])
    if test "$enable_atl" = "yes" -o "$enable_atl" = "" -a "$vcexpress" != Express; then
        DISABLE_ATL=""
        AC_DEFINE(HAVE_FEATURE_ATL, 1)
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

#
# dbghelp.dll
#
if test "$_os" = "WINNT"; then
    BUILD_TYPE="$BUILD_TYPE DBGHELP"
fi

#
# unowinreg.dll
#
UNOWINREG_DLL="185d60944ea767075d27247c3162b3bc-unowinreg.dll"
AC_SUBST(UNOWINREG_DLL)

COM_IS_CLANG=
AC_MSG_CHECKING([whether the compiler is actually Clang])
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
    #ifndef __clang__
    you lose
    #endif
    int foo=42;
    ]])],
    [AC_MSG_RESULT([yes])
     COM_IS_CLANG=TRUE],
    [AC_MSG_RESULT([no])])

if test "$COM_IS_CLANG" = TRUE; then
    AC_MSG_CHECKING([the Clang version])
    clang_version=`echo __clang_major__.__clang_minor__.__clang_patchlevel__ | ${CC%-cl.exe*} -E -P -`
    CLANG_FULL_VERSION=`echo __clang_version__ | ${CC%-cl.exe*} -E -P -`
    CLANGVER=`echo $clang_version | $AWK -F. '{ print \$1*10000+\$2*100+\$3 }'`
    AC_MSG_RESULT([Clang $CLANG_FULL_VERSION, $CLANGVER])
    AC_DEFINE_UNQUOTED(CLANG_VERSION,$CLANGVER)
    AC_DEFINE_UNQUOTED(CLANG_FULL_VERSION,$CLANG_FULL_VERSION)
fi
AC_SUBST(COM_IS_CLANG)

if test "$CCACHE" != "" -a "$COM_IS_CLANG" = TRUE; then
    if test -z "$CCACHE_CPP2"; then
        if test "$enable_ccache" = "" ; then
            AC_MSG_WARN([Using ccache with Clang without CCACHE_CPP2 set causes spurious warnings/errors.])
            add_warning "Using ccache with Clang without CCACHE_CPP2 set causes spurious warnings/errors. Disabling it."
            CCACHE=
        else
            AC_MSG_ERROR([Using --enable-ccache with Clang without CCACHE_CPP2 set causes spurious warnings/errors.])
        fi
    fi
fi

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

dnl ===================================================================
dnl  Test the gcc version
dnl ===================================================================
if test "$GCC" = "yes" -a -z "$COM_IS_CLANG"; then
    AC_MSG_CHECKING([the GCC version])
    _gcc_version=`$CC -dumpversion`
    GCC_VERSION=`echo $_gcc_version | $AWK -F. '{ print \$1*100+\$2 }'`

    AC_MSG_RESULT([gcc $_gcc_version])

    if test "$GCC_VERSION" -lt 0407; then
        AC_MSG_ERROR([GCC $_gcc_version is too old, must be at least GCC 4.7.0])
    fi
else
    # Explicitly force GCC_VERSION to be empty, even for Clang, to check incorrect uses.
    # GCC version should generally be checked only when handling GCC-specific bugs, for testing
    # things like features configure checks should be used, otherwise they may e.g. fail with Clang
    # (which reports itself as GCC 4.2.1).
    GCC_VERSION=
fi
AC_SUBST(GCC_VERSION)

dnl Set the ENABLE_DBGUTIL variable
dnl ===================================================================
AC_MSG_CHECKING([whether to build with additional debug utilities])
if test -n "$enable_dbgutil" -a "$enable_dbgutil" != "no"; then
    ENABLE_DBGUTIL="TRUE"
    # this is an extra var so it can have different default on different MSVC
    # versions (in case there are version specific problems with it)
    MSVC_USE_DEBUG_RUNTIME="TRUE"

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
    if test "$with_system_libgltf" = "yes"; then
        AC_MSG_ERROR([--with-system-libgltf conflicts with --enable-dbgutil])
    else
        with_system_libgltf=no
    fi
else
    ENABLE_DBGUTIL=""
    MSVC_USE_DEBUG_RUNTIME=""
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_DBGUTIL)
AC_SUBST(MSVC_USE_DEBUG_RUNTIME)

dnl Set the ENABLE_DEBUG variable.
dnl ===================================================================
AC_MSG_CHECKING([whether to do a debug build])
if test -n "$enable_debug" && test "$enable_debug" != "yes" && test "$enable_debug" != "no"; then
    AC_MSG_ERROR([--enable-debug now accepts only yes or no, use --enable-selective-debuginfo])
fi
if test -n "$ENABLE_DBGUTIL" -a "$enable_debug" = "no"; then
    AC_MSG_ERROR([--disable-debug cannot be used with --enable-dbgutil])
fi

if test -n "$ENABLE_DBGUTIL" -o \( -n "$enable_debug" -a "$enable_debug" != "no" \) ; then
    ENABLE_DEBUG="TRUE"
    if test -n "$ENABLE_DBGUTIL" ; then
        AC_MSG_RESULT([yes (dbgutil)])
    else
        AC_MSG_RESULT([yes])
    fi
else
    ENABLE_DEBUG=""
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_DEBUG)

if test "$enable_sal_log" = yes; then
    ENABLE_SAL_LOG=TRUE
fi
AC_SUBST(ENABLE_SAL_LOG)

dnl Selective debuginfo
ENABLE_DEBUGINFO_FOR=
if test -n "$ENABLE_DEBUG"; then
    AC_MSG_CHECKING([whether to use selective debuginfo])
    if test -n "$enable_selective_debuginfo" -a "$enable_selective_debuginfo" != "no"; then
        if test "$enable_selective_debuginfo" = "yes"; then
            AC_MSG_ERROR([--enable-selective-debuginfo requires a parameter])
        fi
        ENABLE_DEBUGINFO_FOR="$enable_selective_debuginfo"
        AC_MSG_RESULT([for "$enable_selective_debuginfo"])
    else
        ENABLE_DEBUGINFO_FOR=all
        AC_MSG_RESULT([no, for all])
    fi
else
    if test -n "$enable_selective_debuginfo"; then
        AC_MSG_ERROR([--enable-selective-debuginfo must be used together with either --enable-debug or --enable-dbgutil])
    fi
fi
AC_SUBST(ENABLE_DEBUGINFO_FOR)

dnl Check for enable symbols option
dnl ===================================================================
AC_MSG_CHECKING([whether to generate debug information])
if test -z "$enable_symbols"; then
    if test -n "$ENABLE_DEBUG$ENABLE_DBGUTIL"; then
        enable_symbols=yes
    else
        enable_symbols=no
    fi
fi
if test "$enable_symbols" != no; then
    ENABLE_SYMBOLS=TRUE
    AC_MSG_RESULT([yes])
else
    ENABLE_SYMBOLS=
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_SYMBOLS)

if test -n "$with_android_ndk" -a \( -n "$ENABLE_SYMBOLS" -o -n "$ENABLE_DEBUG" -o -n "$ENABLE_DBGUTIL" \) -a "$ENABLE_DEBUGINFO_FOR" = "all"; then
    # Building on Android with full symbols: without enough memory the linker never finishes currently.
    AC_MSG_CHECKING([whether enough memory is available for linking])
    mem_size=$(grep -o 'MemTotal: *.\+ kB' /proc/meminfo | sed 's/MemTotal: *\(.\+\) kB/\1/')
    # Check for 15GB, as Linux reports a bit less than the physical memory size.
    if test -n "$mem_size" -a $mem_size -lt 15728640; then
        AC_MSG_ERROR([building with full symbols and less than 16GB of memory is not supported])
    else
        AC_MSG_RESULT([yes])
    fi
fi

# Debug information format for iOS. Running dsymutil takes a long time... you really need a separate
# .dSYM only if running Instruments, I think. (Not for normal debugging in Xcode.) To enable a
# separate .dSYM, either use --enable-release-build or change manually to "DWARF with DSYM" in Xcode.
XCODE_DEBUG_INFORMATION_FORMAT=dwarf-with-dsym
if test "$enable_release_build" != yes -a \( -n "$ENABLE_SYMBOLS" -o -n "$ENABLE_DEBUG" -o -n "$ENABLE_DBGUTIL" \); then
    XCODE_DEBUG_INFORMATION_FORMAT=dwarf
fi
AC_SUBST(XCODE_DEBUG_INFORMATION_FORMAT)

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
# determine CPUNAME, OS, ...
# The USING_X11 flag tells whether the host os uses X by default. Can be overridden with the --without-x option.
#
case "$host_os" in

aix*)
    COM=GCC
    CPUNAME=POWERPC
    USING_X11=TRUE
    OS=AIX
    RTL_OS=AIX
    RTL_ARCH=PowerPC
    PLATFORMID=aix_powerpc
    P_SEP=:
    ;;

cygwin*)
    COM=MSC
    USING_X11=
    OS=WNT
    RTL_OS=Windows
    if test "$GNUMAKE_WIN_NATIVE" = "TRUE" ; then
        P_SEP=";"
    else
        P_SEP=:
    fi
    case "$host_cpu" in
    i*86|x86_64)
        if test "$BITNESS_OVERRIDE" = 64; then
            CPUNAME=X86_64
            RTL_ARCH=X86_64
            PLATFORMID=windows_x86_64
            WINDOWS_X64=1
            SCPDEFS="$SCPDEFS -DWINDOWS_X64"
        else
            CPUNAME=INTEL
            RTL_ARCH=x86
            PLATFORMID=windows_x86
        fi
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    SCPDEFS="$SCPDEFS -D_MSC_VER"
    ;;

darwin*)
    COM=GCC
    USING_X11=
    OS=MACOSX
    RTL_OS=MacOSX
    P_SEP=:

    case "$host_cpu" in
    arm*)
        CPUNAME=ARM
        RTL_ARCH=ARM_EABI
        PLATFORMID=ios_arm
        OS=IOS
        ;;
    i*86)
        AC_MSG_ERROR([Can't build 64-bit code in 32-bit OS])
        ;;
    x86_64)
        CPUNAME=X86_64
        RTL_ARCH=X86_64
        PLATFORMID=macosx_x86_64
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    ;;

dragonfly*)
    COM=GCC
    USING_X11=TRUE
    OS=DRAGONFLY
    RTL_OS=DragonFly
    P_SEP=:

    case "$host_cpu" in
    i*86)
        CPUNAME=INTEL
        RTL_ARCH=x86
        PLATFORMID=dragonfly_x86
        ;;
    x86_64)
        CPUNAME=X86_64
        RTL_ARCH=X86_64
        PLATFORMID=dragonfly_x86_64
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    ;;

freebsd*)
    COM=GCC
    USING_X11=TRUE
    RTL_OS=FreeBSD
    OS=FREEBSD
    P_SEP=:

    case "$host_cpu" in
    i*86)
        CPUNAME=INTEL
        RTL_ARCH=x86
        PLATFORMID=freebsd_x86
        ;;
    x86_64|amd64)
        CPUNAME=X86_64
        RTL_ARCH=X86_64
        PLATFORMID=freebsd_x86_64
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    ;;

kfreebsd*)
    COM=GCC
    USING_X11=TRUE
    OS=LINUX
    RTL_OS=kFreeBSD
    P_SEP=:

    case "$host_cpu" in

    i*86)
        CPUNAME=INTEL
        RTL_ARCH=x86
        PLATFORMID=kfreebsd_x86
        ;;
    x86_64)
        CPUNAME=X86_64
        RTL_ARCH=X86_64
        PLATFORMID=kfreebsd_x86_64
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    ;;

linux-gnu*)
    COM=GCC
    USING_X11=TRUE
    OS=LINUX
    RTL_OS=Linux
    P_SEP=:

    case "$host_cpu" in

    aarch64)
        CPUNAME=AARCH64
        PLATFORMID=linux_aarch64
        RTL_ARCH=AARCH64
        ;;
    alpha)
        CPUNAME=AXP
        RTL_ARCH=ALPHA
        PLATFORMID=linux_alpha
        ;;
    arm*)
        CPUNAME=ARM
        EPM_FLAGS="-a arm"
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
        CPUNAME=HPPA
        RTL_ARCH=HPPA
        EPM_FLAGS="-a hppa"
        PLATFORMID=linux_hppa
        ;;
    i*86)
        CPUNAME=INTEL
        RTL_ARCH=x86
        PLATFORMID=linux_x86
        ;;
    ia64)
        CPUNAME=IA64
        RTL_ARCH=IA64
        PLATFORMID=linux_ia64
        ;;
    mips)
        CPUNAME=GODSON
        RTL_ARCH=MIPS_EB
        EPM_FLAGS="-a mips"
        PLATFORMID=linux_mips_eb
        ;;
    mips64)
        CPUNAME=GODSON64
        RTL_ARCH=MIPS64_EB
        EPM_FLAGS="-a mips64"
        PLATFORMID=linux_mips64_eb
        ;;
    mips64el)
        CPUNAME=GODSON64
        RTL_ARCH=MIPS64_EL
        EPM_FLAGS="-a mips64el"
        PLATFORMID=linux_mips64_el
        ;;
    mipsel)
        CPUNAME=GODSON
        RTL_ARCH=MIPS_EL
        EPM_FLAGS="-a mipsel"
        PLATFORMID=linux_mips_el
        ;;
    m68k)
        CPUNAME=M68K
        RTL_ARCH=M68K
        PLATFORMID=linux_m68k
        ;;
    powerpc)
        CPUNAME=POWERPC
        RTL_ARCH=PowerPC
        PLATFORMID=linux_powerpc
        ;;
    powerpc64)
        CPUNAME=POWERPC64
        RTL_ARCH=PowerPC_64
        PLATFORMID=linux_powerpc64
        ;;
    powerpc64le)
        CPUNAME=POWERPC64
        RTL_ARCH=PowerPC_64_LE
        PLATFORMID=linux_powerpc64_le
        ;;
    sparc)
        CPUNAME=SPARC
        RTL_ARCH=SPARC
        PLATFORMID=linux_sparc
        ;;
    s390)
        CPUNAME=S390
        RTL_ARCH=S390
        PLATFORMID=linux_s390
        ;;
    s390x)
        CPUNAME=S390X
        RTL_ARCH=S390x
        PLATFORMID=linux_s390x
        ;;
    x86_64)
        CPUNAME=X86_64
        RTL_ARCH=X86_64
        PLATFORMID=linux_x86_64
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    ;;

linux-android*)
    COM=GCC
    USING_X11=
    OS=ANDROID
    RTL_OS=Android
    P_SEP=:

    case "$host_cpu" in

    arm|armel)
        CPUNAME=ARM
        RTL_ARCH=ARM_EABI
        PLATFORMID=android_arm_eabi
        ;;
    aarch64)
        CPUNAME=AARCH64
        RTL_ARCH=AARCH64
        PLATFORMID=android_aarch64
        ;;
    mips|mipsel)
        CPUNAME=GODSON # Weird, but maybe that's the LO convention?
        RTL_ARCH=MIPS_EL
        PLATFORMID=android_mips_el
        ;;
    i*86)
        CPUNAME=INTEL
        RTL_ARCH=x86
        PLATFORMID=android_x86
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    ;;

emscripten*)
    COM=emcc
    USING_X11=
    OS=EMSCRIPTEN
    RTL_OS=Emscripten
    P_SEP=:
    CPUNAME=INTEL
    RTL_ARCH=x86
    PLATFORMID=linux_x86
    ;;

mingw*)
    COM=GCC
    USING_X11=
    OS=WNT
    RTL_OS=Windows
    P_SEP=:

    case "$host_cpu" in
    i*86|x86_64)
        if test "$BITNESS_OVERRIDE" = 64; then
            CPUNAME=X86_64
            RTL_ARCH=X86_84
            PLATFORMID=windows_x86_64
            SOLARINC="$SOLARINC -I$SRC_ROOT/include/wntgccx"
        else
            CPUNAME=INTEL
            RTL_ARCH=x86
            PLATFORMID=windows_x86
            SOLARINC="$SOLARINC -I$SRC_ROOT/include/wntgcci"
        fi
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    ;;

*netbsd*)
    COM=GCC
    USING_X11=TRUE
    OS=NETBSD
    RTL_OS=NetBSD
    P_SEP=:

    case "$host_cpu" in
    i*86)
        CPUNAME=INTEL
        RTL_ARCH=x86
        PLATFORMID=netbsd_x86
        ;;
    powerpc)
        CPUNAME=POWERPC
        RTL_ARCH=PowerPC
        PLATFORMID=netbsd_powerpc
        ;;
    sparc)
        CPUNAME=SPARC
        RTL_ARCH=SPARC
        PLATFORMID=netbsd_sparc
        ;;
    x86_64)
        CPUNAME=X86_64
        RTL_ARCH=X86_64
        PLATFORMID=netbsd_x86_64
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    ;;

openbsd*)
    COM=GCC
    USING_X11=TRUE
    OS=OPENBSD
    RTL_OS=OpenBSD
    P_SEP=:

    case "$host_cpu" in
    i*86)
        CPUNAME=INTEL
        RTL_ARCH=x86
        PLATFORMID=openbsd_x86
        ;;
    x86_64)
        CPUNAME=X86_64
        RTL_ARCH=X86_64
        PLATFORMID=openbsd_x86_64
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    SOLARINC="$SOLARINC -I/usr/local/include"
    ;;

solaris*)
    COM=GCC
    USING_X11=TRUE
    OS=SOLARIS
    RTL_OS=Solaris
    P_SEP=:

    case "$host_cpu" in
    i*86)
        CPUNAME=INTEL
        RTL_ARCH=x86
        PLATFORMID=solaris_x86
        ;;
    sparc)
        CPUNAME=SPARC
        RTL_ARCH=SPARC
        PLATFORMID=solaris_sparc
        ;;
    sparc64)
        CPUNAME=SPARC64
        RTL_ARCH=SPARC64
        PLATFORMID=solaris_sparc64
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    SOLARINC="$SOLARINC -I/usr/local/include"
    ;;

*)
    AC_MSG_ERROR([$host_os operating system is not suitable to build LibreOffice for!])
    ;;
esac

if test "$with_x" = "no"; then
    AC_MSG_ERROR([Use --disable-gui instead. How can we get rid of this option? No idea where it comes from.])
fi

ENABLE_HEADLESS=""
if test "$enable_gui" = "no"; then
    if test "$USING_X11" != TRUE; then
        AC_MSG_ERROR([$host_os operating system is not suitable to build LibreOffice with --disable-gui.])
    fi
    USING_X11=
    ENABLE_HEADLESS=TRUE
    AC_DEFINE(HAVE_FEATURE_UI,0)
    test_cairo=yes
fi
AC_SUBST(ENABLE_HEADLESS)

WORKDIR="${BUILDDIR}/workdir"
INSTDIR="${BUILDDIR}/instdir"
INSTROOT="${INSTDIR}${INSTROOTSUFFIX}"
SOLARINC="-I. -I$SRC_ROOT/include $SOLARINC"
AC_SUBST(COM)
AC_SUBST(CPUNAME)
AC_SUBST(RTL_OS)
AC_SUBST(RTL_ARCH)
AC_SUBST(EPM_FLAGS)
AC_SUBST(USING_X11)
AC_SUBST([INSTDIR])
AC_SUBST([INSTROOT])
AC_SUBST(OS)
AC_SUBST(P_SEP)
AC_SUBST(WORKDIR)
AC_SUBST(PLATFORMID)
AC_SUBST(WINDOWS_X64)
AC_DEFINE_UNQUOTED(WORKDIR,"$WORKDIR")

dnl ===================================================================
dnl Test which package format to use
dnl ===================================================================
AC_MSG_CHECKING([which package format to use])
if test -n "$with_package_format" -a "$with_package_format" != no; then
    for i in $with_package_format; do
        case "$i" in
        aix | bsd | deb | pkg | rpm | archive | dmg | installed | msi)
            ;;
        *)
            AC_MSG_ERROR([unsupported format $i. Supported by EPM are:
aix - AIX software distribution
bsd - FreeBSD, NetBSD, or OpenBSD software distribution
deb - Debian software distribution
pkg - Solaris software distribution
rpm - RedHat software distribution

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
    AC_MSG_RESULT([$PKGFORMAT])
else
    PKGFORMAT=
    AC_MSG_RESULT([none])
fi
AC_SUBST(PKGFORMAT)

dnl ===================================================================
dnl Set up a different compiler to produce tools to run on the build
dnl machine when doing cross-compilation
dnl ===================================================================

m4_pattern_allow([PKG_CONFIG_FOR_BUILD])
m4_pattern_allow([PKG_CONFIG_LIBDIR])
if test "$cross_compiling" = "yes"; then
    AC_MSG_CHECKING([for BUILD platform configuration])
    echo
    rm -rf CONF-FOR-BUILD config_build.mk
    mkdir CONF-FOR-BUILD
    # Here must be listed all files needed when running the configure script. In particular, also
    # those expanded by the AC_CONFIG_FILES() call near the end of this configure.ac. For clarity,
    # keep them in the same order as there.
    (cd $SRC_ROOT && tar cf - \
        config.guess \
        bin/get_config_variables \
        solenv/bin/getcompver.awk \
        solenv/inc/langlist.mk \
        config_host.mk.in \
        config_host_lang.mk.in \
        Makefile.in \
        lo.xcent.in \
        bin/bffvalidator.sh.in \
        bin/odfvalidator.sh.in \
        instsetoo_native/util/openoffice.lst.in \
        config_host/*.in \
        sysui/desktop/macosx/Info.plist.in \
        ios/lo.xcconfig.in) \
    | (cd CONF-FOR-BUILD && tar xf -)
    cp configure CONF-FOR-BUILD
    test -d config_build && cp -p config_build/*.h CONF-FOR-BUILD/config_host 2>/dev/null
    (
    unset COM USING_X11 OS CPUNAME
    unset CC CXX SYSBASE CFLAGS
    unset AR NM OBJDUMP PKG_CONFIG RANLIB READELF STRIP
    unset CPPUNIT_CFLAGS CPPUNIT_LIBS
    unset LIBXML_CFLAGS LIBXML_LIBS LIBXSLT_CFLAGS LIBXSLT_LIBS XSLTPROC PKG_CONFIG_LIBDIR
    test -n "$CC_FOR_BUILD" && export CC="$CC_FOR_BUILD"
    test -n "$CXX_FOR_BUILD" && export CXX="$CXX_FOR_BUILD"
    test -n "$PKG_CONFIG_FOR_BUILD" && export PKG_CONFIG="$PKG_CONFIG_FOR_BUILD"
    cd CONF-FOR-BUILD
    sub_conf_opts=""
    test -n "$enable_ccache" && sub_conf_opts="$sub_conf_opts --enable-ccache=$enable_ccache"
    test -n "$with_ant_home" && sub_conf_opts="$sub_conf_opts --with-ant-home=$with_ant_home"
    test $with_junit = no && sub_conf_opts="$sub_conf_opts --without-junit"
    test -n "$TARFILE_LOCATION" && sub_conf_opts="$sub_conf_opts --with-external-tar=$TARFILE_LOCATION"
    test "$with_system_icu_for_build" = "yes" -o "$with_system_icu_for_build" = "force" && sub_conf_opts="$sub_conf_opts --with-system-icu"
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
        --disable-cups \
        --disable-graphite \
        --disable-gtk3 \
        --disable-pdfimport \
        --disable-postgresql-sdbc \
        --with-parallelism="$with_parallelism" \
        --without-doxygen \
        --without-java \
        $sub_conf_opts \
        --srcdir=$srcdir \
        2>&1 | sed -e 's/^/    /'
    test -f ./config_host.mk 2>/dev/null || exit
    cp config_host.mk ../config_build.mk
    cp config_host_lang.mk ../config_build_lang.mk
    mv config.log ../config.Build.log
    mkdir -p ../config_build
    mv config_host/*.h ../config_build
    . ./bin/get_config_variables CC CXX INSTDIR INSTROOT LIBO_BIN_FOLDER LIBO_LIB_FOLDER LIBO_URE_LIB_FOLDER OS PATH SDKDIRNAME SYSTEM_LIBXML SYSTEM_LIBXSLT WORKDIR

    for V in CC CXX LIBO_BIN_FOLDER LIBO_LIB_FOLDER LIBO_URE_LIB_FOLDER OS SDKDIRNAME SYSTEM_LIBXML SYSTEM_LIBXSLT; do
        VV='$'$V
        VV=`eval "echo $VV"`
        if test -n "$VV"; then
            line=${V}_FOR_BUILD='${'${V}_FOR_BUILD:-$VV'}'
            echo "$line" >>build-config
        fi
    done

    for V in INSTDIR INSTROOT WORKDIR; do
        VV='$'$V
        VV=`eval "echo $VV"`
        VV=`echo $VV | sed -e "s,/CONF-FOR-BUILD/\([[a-z]]*\),/\1_for_build,g"`
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
    perl -pi -e 's,/(workdir|instdir)(/|$),/\1_for_build\2,g;' \
             -e 's,/CONF-FOR-BUILD,,g;' config_build.mk

    eval `cat CONF-FOR-BUILD/build-config`

    AC_MSG_RESULT([checking for BUILD platform configuration... done])

    rm -rf CONF-FOR-BUILD
else
    OS_FOR_BUILD="$OS"
    CC_FOR_BUILD="$CC"
    CXX_FOR_BUILD="$CXX"
    INSTDIR_FOR_BUILD="$INSTDIR"
    INSTROOT_FOR_BUILD="$INSTROOT"
    LIBO_BIN_FOLDER_FOR_BUILD="$LIBO_BIN_FOLDER"
    LIBO_LIB_FOLDER_FOR_BUILD="$LIBO_LIB_FOLDER"
    LIBO_URE_LIB_FOLDER_FOR_BUILD="$LIBO_URE_LIB_FOLDER"
    SDKDIRNAME_FOR_BUILD="$SDKDIRNAME"
    WORKDIR_FOR_BUILD="$WORKDIR"
fi
AC_SUBST(OS_FOR_BUILD)
AC_SUBST(INSTDIR_FOR_BUILD)
AC_SUBST(INSTROOT_FOR_BUILD)
AC_SUBST(LIBO_BIN_FOLDER_FOR_BUILD)
AC_SUBST(LIBO_LIB_FOLDER_FOR_BUILD)
AC_SUBST(LIBO_URE_LIB_FOLDER_FOR_BUILD)
AC_SUBST(SDKDIRNAME_FOR_BUILD)
AC_SUBST(WORKDIR_FOR_BUILD)

dnl ===================================================================
dnl Check for syslog header
dnl ===================================================================
AC_CHECK_HEADER(syslog.h, AC_DEFINE(HAVE_SYSLOG_H))

# placeholder for future crash reporter feature
ENABLE_CRASHDUMP=""
AC_SUBST(ENABLE_CRASHDUMP)

dnl Set the ENABLE_WERROR variable. (Activate --enable-werror)
dnl ===================================================================
AC_MSG_CHECKING([whether to turn warnings to errors])
if test -n "$enable_werror" -a "$enable_werror" != "no"; then
    ENABLE_WERROR="TRUE"
    AC_MSG_RESULT([yes])
else
    if test -n "$LODE_HOME" -a -z "$enable_werror"; then
        ENABLE_WERROR="TRUE"
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_SUBST(ENABLE_WERROR)

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

# Determine whether to use ooenv for the instdir installation
# ===================================================================
if test $_os != "WINNT" -a $_os != "Darwin"; then
    AC_MSG_CHECKING([whether to use ooenv for the instdir installation])
    if test "$enable_ooenv" = "no"; then
        AC_MSG_RESULT([no])
    else
        ENABLE_OOENV="TRUE"
        AC_MSG_RESULT([yes])
    fi
fi
AC_SUBST(ENABLE_OOENV)

if test "$USING_X11" != TRUE; then
    # be sure to do not mess with unneeded stuff
    test_randr=no
    test_xrender=no
    test_cups=no
    test_dbus=no
    test_gtk=no
    build_gstreamer_1_0=no
    build_gstreamer_0_10=no
    test_tde=no
    test_kde4=no
    enable_cairo_canvas=no
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
    if test "$ac_cv_lib_cups_cupsPrintFiles" != "yes" -o "$ac_cv_header_cups_cups_h" != "yes"; then
        AC_MSG_ERROR([Could not find CUPS. Install libcups2-dev or cups-devel.])
    fi

else
    AC_MSG_RESULT([no])
fi

AC_SUBST(ENABLE_CUPS)

# fontconfig checks
if test "$test_fontconfig" = "yes"; then
    PKG_CHECK_MODULES([FONTCONFIG], [fontconfig >= 2.4.1])
    SYSTEM_FONTCONFIG=TRUE
    FilterLibs "${FONTCONFIG_LIBS}"
    FONTCONFIG_LIBS="${filteredlibs}"
fi
AC_SUBST(FONTCONFIG_CFLAGS)
AC_SUBST(FONTCONFIG_LIBS)
AC_SUBST([SYSTEM_FONTCONFIG])

dnl whether to find & fetch external tarballs?
dnl ===================================================================
if test -z "$TARFILE_LOCATION" -a -n "$LODE_HOME" ; then
   if test "$GNUMAKE_WIN_NATIVE" = "TRUE" ; then
       TARFILE_LOCATION="`cygpath -m $LODE_HOME/ext_tar`"
   else
       TARFILE_LOCATION="$LODE_HOME/ext_tar"
   fi
fi
if test -z "$TARFILE_LOCATION"; then
    if test -d "$SRC_ROOT/src" ; then
        mv "$SRC_ROOT/src" "$SRC_ROOT/external/tarballs"
        ln -s "$SRC_ROOT/external/tarballs" "$SRC_ROOT/src"
    fi
    TARFILE_LOCATION="$SRC_ROOT/external/tarballs"
else
    AbsolutePath "$TARFILE_LOCATION"
    PathFormat "${absolute_path}"
    TARFILE_LOCATION="${formatted_path}"
fi
AC_SUBST(TARFILE_LOCATION)

AC_MSG_CHECKING([whether we want to fetch tarballs])
if test "$enable_fetch_external" != "no"; then
    if test "$with_all_tarballs" = "yes"; then
        AC_MSG_RESULT(["yes, all of them"])
        DO_FETCH_TARBALLS="ALL"
    else
        AC_MSG_RESULT(["yes, if we use them"])
        DO_FETCH_TARBALLS="TRUE"
    fi
else
    AC_MSG_RESULT([no])
    DO_FETCH_TARBALLS=
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
    WITH_MYSPELL_DICTS=TRUE
    BUILD_TYPE="$BUILD_TYPE DICTIONARIES"
    GIT_NEEDED_SUBMODULES="dictionaries $GIT_NEEDED_SUBMODULES"
else
    AC_MSG_RESULT([no])
    WITH_MYSPELL_DICTS=
fi
AC_SUBST(WITH_MYSPELL_DICTS)

# There are no "system" myspell, hyphen or mythes dictionaries on OS X, Windows, Android or iOS.
if test $_os = Darwin -o $_os = WINNT -o $_os = iOS -o $_os = Android; then
    if test "$with_system_dicts" = yes; then
        AC_MSG_ERROR([There are no system dicts on this OS in the formats the 3rd-party libs we use expect]);
    fi
    with_system_dicts=no
fi

AC_MSG_CHECKING([whether to use dicts from external paths])
if test -z "$with_system_dicts" -o "$with_system_dicts" != "no"; then
    AC_MSG_RESULT([yes])
    SYSTEM_DICTS=TRUE
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
    SYSTEM_DICTS=
fi
AC_SUBST(SYSTEM_DICTS)
AC_SUBST(DICT_SYSTEM_DIR)
AC_SUBST(HYPH_SYSTEM_DIR)
AC_SUBST(THES_SYSTEM_DIR)

dnl ===================================================================
dnl enable pch by default on windows
dnl enable it explicitly otherwise
AC_MSG_CHECKING([whether to enable pch feature])
ENABLE_PCH=""
if test "$enable_pch" != "no"; then
    if test "$_os" = "WINNT" -a "$WITH_MINGW" != "yes"; then
        ENABLE_PCH="TRUE"
        AC_MSG_RESULT([yes])
    elif test -n "$enable_pch" && test "$GCC" = "yes"; then
        ENABLE_PCH="TRUE"
        AC_MSG_RESULT([yes])
    elif test -n "$enable_pch"; then
        AC_MSG_ERROR([Precompiled header not yet supported for your platform/compiler])
    else
        AC_MSG_RESULT([no])
    fi
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_PCH)

TAB=`printf '\t'`

AC_MSG_CHECKING([the GNU Make version])
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
    AC_MSG_CHECKING([for GNU Make bug 20033])
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
AC_MSG_CHECKING([whether GNU Make supports the 'file' function])
TESTGMAKEFILEFUNC="`mktemp -d -t tst.XXXXXX`"
if test "$GNUMAKE_WIN_NATIVE" = "TRUE" ; then
    TESTGMAKEFILEFUNC=`cygpath -m $TESTGMAKEFILEFUNC`
fi
$SED -e "s/<TAB>/$TAB/" > $TESTGMAKEFILEFUNC/Makefile << EOF
\$(file >test.txt,Success )

.PHONY: all
all:
<TAB>@cat test.txt

EOF
$GNUMAKE -C $TESTGMAKEFILEFUNC 2>/dev/null 1>&2
if test -f $TESTGMAKEFILEFUNC/test.txt; then
    HAVE_GNUMAKE_FILE_FUNC=TRUE
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
rm -rf $TESTGMAKEFILEFUNC
AC_SUBST(HAVE_GNUMAKE_FILE_FUNC)
AC_SUBST(GNUMAKE_WIN_NATIVE)

_make_ver_check=`$GNUMAKE --version | grep "Built for Windows"`
STALE_MAKE=
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

        AC_RUN_IFELSE([AC_LANG_PROGRAM(
            [
#include <stdio.h>
            ],[
printf ("");
            ])],
            [
                  HAVE_LD_HASH_STYLE=TRUE
                  WITH_LINKER_HASH_STYLE=$hash_style
            ],
            [HAVE_LD_HASH_STYLE=FALSE],
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
if `$PERL -e 'use Cwd; use Digest::MD5'>/dev/null 2>&1`; then
    AC_MSG_RESULT([all modules found])
else
    AC_MSG_RESULT([failed to find some modules])
    # Find out which modules are missing.
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

    # Use wrappers for LTO
    if test "$ENABLE_LTO" = "TRUE" -a "$COM_IS_CLANG" != "TRUE"; then
        AC_CHECK_TOOL(AR,gcc-ar)
        AC_CHECK_TOOL(NM,gcc-nm)
        AC_CHECK_TOOL(RANLIB,gcc-ranlib)
    else
        AC_CHECK_TOOL(AR,ar)
        AC_CHECK_TOOL(NM,nm)
        AC_CHECK_TOOL(RANLIB,ranlib)
    fi
    AC_CHECK_TOOL(OBJDUMP,objdump)
    AC_CHECK_TOOL(READELF,readelf)
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
AC_SUBST(READELF)
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
                AC_MSG_ERROR([yes, from unknown origin. This *will* break the build. Please modify your PATH variable so that $PKG_CONFIG is no longer found by configure scripts.])
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

    for x in `ls /proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft\ SDKs/Windows`; do
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

find_dotnetsdk46()
{
    unset frametest

    for ver in 4.6.1 4.6; do
        reg_get_value_64 "HKEY_LOCAL_MACHINE/SOFTWARE/Wow6432Node/Microsoft/Microsoft SDKs/NETFXSDK/$ver/KitsInstallationFolder"
        if test -n "$regvalue"; then
            frametest=$regvalue
            return
        fi
        reg_get_value_32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/NETFXSDK/$ver/KitsInstallationFolder"
        if test -n "$regvalue"; then
            frametest=$regvalue
            return
        fi
    done
}

find_winsdk_version()
{
    # Args: $1 : SDK version as in "6.0A", "7.0" etc
    # Return values: $winsdktest, $winsdklibsubdir

    unset winsdktest winsdklibsubdir

    # Why we look for them in this particular order I don't know. But OTOH I
    case "$1" in
    7.*)
        reg_get_value_32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v${1}/InstallationFolder"
        if test -n "$regvalue"; then
            winsdktest=$regvalue
            winsdklibsubdir=.
            return
        fi
        ;;
    8.1|8.1A)
        reg_get_value_32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Windows Kits/Installed Roots/KitsRoot81"
        if test -n "$regvalue"; then
            winsdktest=$regvalue
            winsdklibsubdir=winv6.3
            return
        fi
        ;;
    8.0|8.0A)
        reg_get_value_32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Windows Kits/Installed Roots/KitsRoot"
        if test -n "$regvalue"; then
            winsdktest=$regvalue
            winsdklibsubdir=win8
            return
        fi
        ;;
    10.0)
        reg_get_value_32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v${1}/InstallationFolder"
        if test -n "$regvalue"; then
            winsdktest=$regvalue
            reg_get_value_32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v${1}/ProductVersion"
            winsdklibsubdir=$regvalue
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

    for ver in $WINDOWS_SDK_ACCEPTABLE_VERSIONS; do
        find_winsdk_version $ver
        if test -n "$winsdktest"; then
            return
        fi
    done
}

find_msms()
{
    my_msm_file=Microsoft_VC${VCVER}_CRT_x86.msm
    AC_MSG_CHECKING([for $my_msm_file])
    msmdir=
    for ver in 12.0 14.0; do
        reg_get_value_32 HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/$ver/Setup/VS/MSMDir
        if test -n "$regvalue"; then
            if test -e "$regvalue/$my_msm_file"; then
                msmdir=$regvalue
                break
            fi
        fi
    done
    dnl Is the following fallback really necessary, or was it added in response
    dnl to never having started Visual Studio on a given machine, so the
    dnl registry keys checked above had presumably not yet been created?
    dnl Anyway, if it really is necessary, it might be worthwhile to extend it
    dnl to also check %CommonProgramFiles(X86)% (typically expanding to
    dnl "C:\Program Files (X86)\Common Files" compared to %CommonProgramFiles%
    dnl expanding to "C:\Program Files\Common Files"), which would need
    dnl something like $(perl -e 'print $ENV{"CommonProgramFiles(x86)"}') to
    dnl obtain its value from cygwin:
    if test -z "$msmdir"; then
        my_msm_dir="${COMMONPROGRAMFILES}/Merge Modules/"
        if test -e "$my_msm_dir/$my_msm_file"; then
            msmdir=$my_msm_dir
        fi
    fi
    if test -n "$msmdir"; then
        msmdir=`cygpath -m "$msmdir"`
        AC_MSG_RESULT([$msmdir])
    else
        if test "$ENABLE_RELEASE_BUILD" = "TRUE" ; then
            AC_MSG_ERROR([not found])
        else
            AC_MSG_WARN([not found])
        fi
    fi
}

find_msvc_x64_dlls()
{
    msvcdllpath="$VC_PRODUCT_DIR/redist/x64/Microsoft.VC${VCVER}.CRT"
    # http://blogs.msdn.com/b/vcblog/archive/2014/06/03/visual-studio-14-ctp.aspx
    # Refactored C Runtime (CRT): This CTP contains the first preview of the substantially refactored CRT.
    # msvcr140.dll no longer exists. It is replaced by a trio of DLLs: vcruntime140.dll, appcrt140.dll,
    # and desktopcrt140.dll.
    if test "$VCVER" = 140; then
       msvcdlls="msvcp${VCVER}.dll vcruntime${VCVER}.dll"
    else
       msvcdlls="msvcp${VCVER}.dll msvcr${VCVER}.dll"
    fi
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
    MIDL_PATH=`win_short_path_for_make "$MIDL_PATH"`

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

    CSC_PATH=`win_short_path_for_make "$CSC_PATH"`

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

    AL_PATH=`win_short_path_for_make "$AL_PATH"`

    dnl Check mscoree.lib / .NET Framework dir
    AC_MSG_CHECKING(.NET Framework find_dotnetsdk)
    find_dotnetsdk
    if test -f "$frametest/lib/mscoree.lib"; then
        DOTNET_FRAMEWORK_HOME="$frametest"
    else
        AC_MSG_CHECKING(.NET Framework find_winsdk)
        find_winsdk
        if test -f "$winsdktest/lib/mscoree.lib" -o -f "$winsdktest/lib/$winsdklibsubdir/um/$WINDOWS_SDK_ARCH/mscoree.lib"; then
            DOTNET_FRAMEWORK_HOME="$winsdktest"
        else
            AC_MSG_CHECKING(.NET Framework find_dotnetsdk46)
            find_dotnetsdk46
            PathFormat "$frametest"
            frametest="$formatted_path"
            AC_MSG_CHECKING(found: $frametest/um/$WINDOWS_SDK_ARCH/mscoree.lib)
            if test -f "$frametest/Lib/um/$WINDOWS_SDK_ARCH/mscoree.lib"; then
                 DOTNET_FRAMEWORK_HOME="$frametest"
            fi
        fi
    fi

    if test ! -f "$DOTNET_FRAMEWORK_HOME/lib/mscoree.lib" -a ! -f "$DOTNET_FRAMEWORK_HOME/lib/$winsdklibsubdir/um/$WINDOWS_SDK_ARCH/mscoree.lib" -a ! -f "$DOTNET_FRAMEWORK_HOME/Lib/um/$WINDOWS_SDK_ARCH/mscoree.lib"; then
        AC_MSG_ERROR([mscoree.lib not found])
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

dnl check for GNU C++ compiler version
if test "$GXX" = "yes" -a "$CXX" != "emcc"; then
    AC_MSG_CHECKING([the GNU C++ compiler version])

    _gpp_version=`$CXX -dumpversion`
    _gpp_majmin=`echo $_gpp_version | $AWK -F. '{ print \$1*100+\$2 }'`

    if test "$_gpp_majmin" -lt "401"; then
        AC_MSG_ERROR([You need to use GNU C++ compiler version >= 4.1 to build LibreOffice, you have $_gpp_version.])
    else
        AC_MSG_RESULT([checked (g++ $_gpp_version)])
    fi

    dnl see https://code.google.com/p/android/issues/detail?id=41770
    if test "$_gpp_majmin" -ge "401"; then
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

dnl ===================================================================
dnl Find pre-processors.(should do that _after_ messing with CC/CXX)
dnl ===================================================================

if test "$_os" != "WINNT" -o "$WITH_MINGW" = "yes"; then
    AC_PROG_CXXCPP

    dnl Check whether there's a C pre-processor.
    AC_PROG_CPP
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
    LFS_CFLAGS=''
fi
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
dnl Check whether to enable runtime optimizations
dnl ===================================================================
ENABLE_RUNTIME_OPTIMIZATIONS=
AC_MSG_CHECKING([whether to enable runtime optimizations])
if test -z "$enable_runtime_optimizations"; then
    for i in $CC; do
        case $i in
        -fsanitize=*)
            enable_runtime_optimizations=no
            break
            ;;
        esac
    done
fi
if test "$enable_runtime_optimizations" != no; then
    ENABLE_RUNTIME_OPTIMIZATIONS=TRUE
    AC_DEFINE(ENABLE_RUNTIME_OPTIMIZATIONS)
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST([ENABLE_RUNTIME_OPTIMIZATIONS])

dnl ===================================================================
dnl Check if valgrind headers are available
dnl ===================================================================
ENABLE_VALGRIND=
if test "$cross_compiling" != yes; then
    prev_cppflags=$CPPFLAGS
    # Is VALGRIND_CFLAGS something one is supposed to have in the environment,
    # or where does it come from?
    CPPFLAGS="$CPPFLAGS $VALGRIND_CFLAGS"
    AC_CHECK_HEADER([valgrind/valgrind.h],
        [ENABLE_VALGRIND=TRUE])
    CPPFLAGS=$prev_cppflags
fi
AC_SUBST([ENABLE_VALGRIND])
if test -z "$ENABLE_VALGRIND"; then
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
                MINGW_SHARED_GCCLIB=TRUE
                AC_MSG_RESULT([ $MINGW_GCCDLL])
            else
                MINGW_SHARED_GCCLIB=
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
                if test "$CROSS_COMPILING" = "TRUE"; then
                    dnl m4 escaping!
                    mingw_gxxdll_root=${mingw_gxxdll_root%-[[0-9]]}
                fi
                MINGW_SHARED_LIBSTDCPP=-l$mingw_gxxdll_root
                MINGW_SHARED_GXXLIB=TRUE
                AC_MSG_RESULT([$MINGW_GXXDLL])
            else
                MINGW_SHARED_GXXLIB=
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
    if test "$_os" = Darwin; then
        dnl On Mac OS X, both /usr/lib/libstdc++.6.dylib and
        dnl /usr/lib/libc++1.dylib delegate to /usr/lib/libc++abi.dylib for the
        dnl __cxa_guard_* functionality; up to 10.8, libc++abi.dylib is
        dnl apparently provided by a libcppabi project that has broken semantics
        dnl (cf. <http://www.opensource.apple.com/source/libcppabi/libcppabi-26/
        dnl src/cxa_guard.cxx>), while in 10.9 (verified at least on 10.9.4) it
        dnl is apparently provided by LLVM's libcxxabi project, which has good
        dnl semantics (cf. <http://llvm.org/svn/llvm-project/libcxxabi/trunk/
        dnl src/cxa_guard.cpp>):
        if test "$MAC_OS_X_VERSION_MIN_REQUIRED" -ge 1090; then
            HAVE_THREADSAFE_STATICS=TRUE
        fi
    elif test "$_os" != Android; then
        if test "$COM_IS_CLANG" = TRUE; then
            AC_LANG_PUSH([C++])
            AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <list>
#if defined __GLIBCXX__
#if __GLIBCXX__ < 20080606 && __GLIBCXX__ != 20080306
#error
#endif
#endif
                ]])],[HAVE_THREADSAFE_STATICS=TRUE],[])
            AC_LANG_POP([C++])
        else # known to work in GCC since version 4.3
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
    # MSVC has sprouted C++11 thread-safe statics in 2015
    # http://blogs.msdn.com/b/vcblog/archive/2015/06/19/c-11-14-17-features-in-vs-2015-rtm.aspx
    if test "$COM" = "MSC" -a "$VCVER" -ge "140"; then
        AC_DEFINE(HAVE_THREADSAFE_STATICS)
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_SUBST(HAVE_THREADSAFE_STATICS)

dnl ===================================================================
dnl visibility and other gcc features
dnl ===================================================================
if test "$GCC" = "yes" -o "$COM_IS_CLANG" = TRUE; then
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

    AC_MSG_CHECKING([whether $CC supports -fstack-protector-strong])
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -Werror -fstack-protector-strong"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[ return 0; ]])],[ HAVE_GCC_STACK_PROTECTOR_STRONG=TRUE ],[])
    CFLAGS=$save_CFLAGS
    if test "$HAVE_GCC_STACK_PROTECTOR_STRONG" = "TRUE"; then
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

    AC_MSG_CHECKING([whether $CXX defines __base_class_type_info in cxxabi.h])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            #include <cstddef>
            #include <cxxabi.h>
            std::size_t f() { return sizeof(__cxxabiv1::__base_class_type_info); }
        ])], [
            AC_DEFINE([HAVE_CXXABI_H_BASE_CLASS_TYPE_INFO],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    AC_LANG_POP([C++])

    AC_MSG_CHECKING([whether $CXX defines __class_type_info in cxxabi.h])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            #include <cstddef>
            #include <cxxabi.h>
            std::size_t f() { return sizeof(__cxxabiv1::__class_type_info); }
        ])], [
            AC_DEFINE([HAVE_CXXABI_H_CLASS_TYPE_INFO],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    AC_LANG_POP([C++])

    AC_MSG_CHECKING([whether $CXX declares __cxa_allocate_exception in cxxabi.h])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            #include <cxxabi.h>
            void * f() { return __cxxabiv1::__cxa_allocate_exception(0); }
        ])], [
            AC_DEFINE([HAVE_CXXABI_H_CXA_ALLOCATE_EXCEPTION],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    AC_LANG_POP([C++])

    AC_MSG_CHECKING([whether $CXX defines __cxa_eh_globals in cxxabi.h])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            #include <cstddef>
            #include <cxxabi.h>
            std::size_t f() { return sizeof(__cxxabiv1::__cxa_eh_globals); }
        ])], [
            AC_DEFINE([HAVE_CXXABI_H_CXA_EH_GLOBALS],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    AC_LANG_POP([C++])

    AC_MSG_CHECKING([whether $CXX defines __cxa_exceptions in cxxabi.h])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            #include <cstddef>
            #include <cxxabi.h>
            std::size_t f() { return sizeof(__cxxabiv1::__cxa_exceptions); }
        ])], [
            AC_DEFINE([HAVE_CXXABI_H_CXA_EXCEPTIONS],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    AC_LANG_POP([C++])

    AC_MSG_CHECKING([whether $CXX declares __cxa_get_globals in cxxabi.h])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            #include <cxxabi.h>
            void * f() { return __cxxabiv1::__cxa_get_globals(); }
        ])], [
            AC_DEFINE([HAVE_CXXABI_H_CXA_GET_GLOBALS],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    AC_LANG_POP([C++])

    AC_MSG_CHECKING([whether $CXX declares __cxa_throw in cxxabi.h])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            #include <cxxabi.h>
            void f() { __cxxabiv1::__cxa_throw(0, 0, 0); }
        ])], [
            AC_DEFINE([HAVE_CXXABI_H_CXA_THROW],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    AC_LANG_POP([C++])

    AC_MSG_CHECKING([whether $CXX defines __si_class_type_info in cxxabi.h])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            #include <cstddef>
            #include <cxxabi.h>
            std::size_t f() { return sizeof(__cxxabiv1::__si_class_type_info); }
        ])], [
            AC_DEFINE([HAVE_CXXABI_H_SI_CLASS_TYPE_INFO],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    AC_LANG_POP([C++])

    AC_MSG_CHECKING([whether $CXX defines __vmi_class_type_info in cxxabi.h])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            #include <cstddef>
            #include <cxxabi.h>
            std::size_t f() { return sizeof(__cxxabiv1::__vmi_class_type_info); }
        ])], [
            AC_DEFINE([HAVE_CXXABI_H_VMI_CLASS_TYPE_INFO],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    AC_LANG_POP([C++])

    AC_MSG_CHECKING([whether STL uses __attribute__((warn_unused))])
    AC_LANG_PUSH([C++])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS -Werror -Wunused"
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

AC_MSG_CHECKING([whether $CXX supports __attribute__((warn_unused))])
AC_LANG_PUSH([C++])
save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="$CXXFLAGS -Werror -Wunknown-pragmas"
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
        struct __attribute__((warn_unused)) dummy {};
    ])], [
        AC_DEFINE([HAVE_GCC_ATTRIBUTE_WARN_UNUSED],[1])
        AC_MSG_RESULT([yes])
    ], [AC_MSG_RESULT([no])])
CXXFLAGS=$save_CXXFLAGS
AC_LANG_POP([C++])

AC_SUBST(HAVE_GCC_AVX)
AC_SUBST(HAVE_GCC_STACK_PROTECTOR_STRONG)
AC_SUBST(HAVE_GCC_BUILTIN_ATOMIC)

dnl ===================================================================
dnl Identify the C++ library
dnl ===================================================================

AC_MSG_CHECKING([What the C++ library is])
AC_LANG_PUSH([C++])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <utility>
#ifndef __GLIBCXX__
foo bar
#endif
]])],
    [CPP_LIBRARY=GLIBCXX
     cpp_library_name="GNU libstdc++"
    ],
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <utility>
#ifndef _LIBCPP_VERSION
foo bar
#endif
]])],
    [CPP_LIBRARY=LIBCPP
     cpp_library_name="LLVM libc++"
    ],
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <utility>
#ifndef _MSC_VER
foo bar
#endif
]])],
    [CPP_LIBRARY=MSVCRT
     cpp_library_name="Microsoft"
    ],
    AC_MSG_ERROR([Could not figure out what C++ library this is]))))
AC_MSG_RESULT([$cpp_library_name])
AC_LANG_POP([C++])

dnl ===================================================================
dnl C++11
dnl ===================================================================

CXXFLAGS_CXX11=
if test "$COM" = MSC; then
    AC_MSG_CHECKING([whether $CXX supports C++11])
    AC_MSG_RESULT(yes)
    # MSVC supports (a subset of) CXX11 without any switch
elif test "$GCC" = "yes"; then
    HAVE_CXX11=
    AC_MSG_CHECKING([whether $CXX supports C++14 or C++11])
    for flag in -std=gnu++14 -std=gnu++1y -std=c++14 -std=c++1y -std=gnu++11 -std=gnu++0x -std=c++11 -std=c++0x ; do
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
            ]])],[CXXFLAGS_CXX11=$flag])
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
        AC_MSG_ERROR(no)
    fi
fi

dnl Test for temporarily incompatible libstdc++ 4.7.{0,1}, where
dnl <https://gcc.gnu.org/viewcvs/gcc?view=revision&revision=179528> introduced
dnl an additional member _M_size into C++11 std::list towards 4.7.0 and
dnl <https://gcc.gnu.org/viewcvs/gcc?view=revision&revision=189186> removed it
dnl again towards 4.7.2:
if test $CPP_LIBRARY = GLIBCXX; then
    AC_MSG_CHECKING([whether using C++11 causes libstdc++ 4.7.0/4.7.1 ABI breakage])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <list>
#if !defined __GLIBCXX__ || (__GLIBCXX__ != 20120322 && __GLIBCXX__ != 20120614)
    // according to <https://gcc.gnu.org/onlinedocs/libstdc++/manual/abi.html>:
    //   GCC 4.7.0: 20120322
    //   GCC 4.7.1: 20120614
    // and using a range check is not possible as the mapping between
    // __GLIBCXX__ values and GCC versions is not monotonic
/* ok */
#else
abi broken
#endif
        ]])], [AC_MSG_RESULT(no, ok)],
        [AC_MSG_ERROR(yes)])
    AC_LANG_POP([C++])
fi

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
    [ AC_MSG_ERROR(no)])
AC_LANG_POP([C++])
CXXFLAGS=$save_CXXFLAGS

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
     AC_MSG_ERROR(no)
    ])
])

AC_LANG_POP([C++])
CXXFLAGS=$save_CXXFLAGS

AC_SUBST(CXXFLAGS_CXX11)

AC_MSG_CHECKING([whether $CXX supports C++11 constexpr])
save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $CXXFLAGS_CXX11"
AC_LANG_PUSH([C++])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    struct S {
        int n_;
        constexpr S(int n): n_(n) {}
    };
    ]])], [cxx11_constexpr=yes], [cxx11_constexpr=no])
AC_LANG_POP([C++])
CXXFLAGS=$save_CXXFLAGS
AC_MSG_RESULT([$cxx11_constexpr])
if test "$cxx11_constexpr" = yes; then
    AC_DEFINE([HAVE_CXX11_CONSTEXPR])
fi

AC_MSG_CHECKING([whether $CXX supports C++14 constexpr])
save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $CXXFLAGS_CXX11"
AC_LANG_PUSH([C++])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    // A somewhat over-complicated way of checking for
    // <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66460> "ICE using __func__
    // in constexpr function":
    #include <cassert>
    template<typename T> inline constexpr T f(T x) { return x; }
    template<typename T> inline constexpr T g(T x) {
        assert(f(static_cast<int>(x)));
        return x;
    }
    enum E { e };
    auto v = g(E::e);

    struct S {
        int n_;
        constexpr bool f() {
            int n = n_;
            int i = 0;
            while (n > 0) { --n; ++i; }
            return i == 0;
        }
    };
    ]])], [cxx14_constexpr=yes], [cxx14_constexpr=no])
AC_LANG_POP([C++])
CXXFLAGS=$save_CXXFLAGS
AC_MSG_RESULT([$cxx14_constexpr])
if test "$cxx14_constexpr" = yes; then
    AC_DEFINE([HAVE_CXX14_CONSTEXPR])
fi

AC_MSG_CHECKING([whether $CXX supports C++11 ref-qualifier])
save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $CXXFLAGS_CXX11"
AC_LANG_PUSH([C++])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    struct S {
        void f() &;
        void f() &&;
    };
    ]])], [cxx11_ref_qualifier=yes], [cxx11_ref_qualifier=no])
AC_LANG_POP([C++])
CXXFLAGS=$save_CXXFLAGS
AC_MSG_RESULT([$cxx11_ref_qualifier])
if test "$cxx11_ref_qualifier" = yes; then
    AC_DEFINE([HAVE_CXX11_REF_QUALIFIER])
fi

AC_MSG_CHECKING([whether $CXX supports C++14 sized deallocation])
dnl At least Clang -fsanitize=address causes "multiple definition of
dnl `operator delete(void*, unsigned long)'" also defined in
dnl projects/compiler-rt/lib/asan/asan_new_delete.cc:
save_CXX=$CXX
if test "$COM_IS_CLANG" = TRUE; then
    my_CXX=
    for i in $CXX; do
        case $i in
        -fsanitize=address)
            ;;
        *)
            my_CXX="$my_CXX $i"
            ;;
        esac
    done
    CXX=$my_CXX
fi
save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $CXXFLAGS_CXX11"
AC_LANG_PUSH([C++])
AC_RUN_IFELSE([AC_LANG_PROGRAM([[
    #include <cstddef>
    #include <cstdlib>
    void operator delete(void *) throw () { std::exit(1); }
    void operator delete(void *, std::size_t) throw () { std::exit(0); }
    struct S { S() { throw 0; } };
    ]],[[
    try { new S; } catch (...) {}
    return 1;
    ]])], [cxx14_sized_deallocation=yes], [cxx14_sized_deallocation=no], [cxx14_sized_deallocation=no])
AC_LANG_POP([C++])
CXX=$save_CXX
CXXFLAGS=$save_CXXFLAGS
AC_MSG_RESULT([$cxx14_sized_deallocation])
if test "$cxx14_sized_deallocation" = yes; then
    AC_DEFINE([HAVE_CXX14_SIZED_DEALLOCATION])
fi

HAVE_GCC_PRAGMA_OPERATOR=
dnl _Pragma support (may require C++11)
if test "$GCC" = "yes" -o "$COM_IS_CLANG" = TRUE; then
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

    # Assume visibility is not broken with libc++. The below test is very much designed for libstdc++
    # only.
    if test "$HAVE_GCC_VISIBILITY_FEATURE" = "TRUE" -a "$CPP_LIBRARY" = GLIBCXX; then
        dnl gcc#19664, gcc#22482, rhbz#162935
        AC_MSG_CHECKING([if STL headers are visibility safe (GCC bug 22482)])
        AC_EGREP_HEADER(visibility push, string, stlvisok=yes, stlvisok=no)
        AC_MSG_RESULT([$stlvisok])
        if test "$stlvisok" = "no"; then
            AC_MSG_WARN([Your libstdc++ headers are not visibility safe. Disabling visibility])
            add_warning "Your libstdc++ headers are not visibility safe. Disabling visibility"
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
            dnl At least Clang -fsanitize=address and -fsanitize=undefined are
            dnl known to not work with -z defs (unsetting which makes the test
            dnl moot, though):
            my_linkflagsnoundefs=$LINKFLAGSNOUNDEFS
            if test "$COM_IS_CLANG" = TRUE; then
                for i in $CXX; do
                    case $i in
                    -fsanitize=*)
                        my_linkflagsnoundefs=
                        break
                        ;;
                    esac
                done
            fi
            if ! $CXX $CXXFLAGS $CPPFLAGS $LINKFLAGSSHL -fPIC -fvisibility-inlines-hidden conftestlib2.cc -L. -lconftest1 $my_linkflagsnoundefs -o libconftest2$DLLPOST >/dev/null 2>&5; then
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
        if ! $CXX $CXXFLAGS $CPPFLAGS -fpic -S visibility.cxx; then
            gccvisbroken=yes
        else
            case "$host_cpu" in
            i?86|x86_64)
                if test "$_os" = "Darwin" -o "$WITH_MINGW" = "yes"; then
                    gccvisbroken=no
                else
                    if $EGREP -q '@PLT|@GOT' visibility.s || test "$ENABLE_LTO" = "TRUE"; then
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
dnl Compiler plugins
dnl ===================================================================

COMPILER_PLUGINS=
# currently only Clang
if test "$COM_IS_CLANG" = "TRUE"; then
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
        AC_LANG_PUSH([C++])
        save_CPPFLAGS=$CPPFLAGS
        save_CXX=$CXX
        # compiler plugins must be built with "native" bitness of clang
        # because they link against clang libraries
        CXX=${COMPILER_PLUGINS_CXX-$(echo $CXX | sed -e s/-m64// -e s/-m32//)}
        CPPFLAGS="$CPPFLAGS $CXXFLAGS_CXX11 -I$CLANGDIR/include -I$CLANGDIR/tools/clang/include -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS"
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
        CXX=$save_CXX
        CPPFLAGS=$save_CPPFLAGS
        AC_LANG_POP([C++])
    fi
else
    if test "$enable_compiler_plugins" = "yes"; then
        AC_MSG_ERROR([Compiler plugins are currently supported only with the Clang compiler.])
    fi
fi
AC_SUBST(COMPILER_PLUGINS)
AC_SUBST(COMPILER_PLUGINS_CXX)
AC_SUBST(CLANGDIR)

# Plugin to help linker.
# Add something like LD_PLUGIN=/usr/lib64/LLVMgold.so to your autogen.input.
# This makes --enable-lto build with clang work.
AC_SUBST(LD_PLUGIN)

dnl ===================================================================
dnl allocator
dnl ===================================================================
AC_MSG_CHECKING([which memory allocator to use])
if test "$with_alloc" = "system"; then
    AC_MSG_RESULT([system])
    ALLOC="SYS_ALLOC"
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
    SCPDEFS="$SCPDEFS -DWITH_VC${VCVER}_REDIST"
fi

AC_SUBST(MSVC_DLL_PATH)
AC_SUBST(MSVC_DLLS)
AC_SUBST(MSM_PATH)

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
            for ver in 1.8 1.7 1.6; do
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
            test "$build_os" = "cygwin" && with_jdk_home=`win_short_path_for_make "$with_jdk_home"`
            howfound="you passed"
        fi
    fi

    # MacOS X: /usr/libexec/java_home helps to set the current JDK_HOME. Actually JDK_HOME should NOT be set where java (/usr/bin/java) is located.
    # /usr/bin/java -> /System/Library/Frameworks/JavaVM.framework/Versions/Current/Commands/java, but /usr does not contain the JDK libraries
    if test -z "$with_jdk_home" -a "$_os" = "Darwin" -a -x /usr/libexec/java_home; then
        with_jdk_home=`/usr/libexec/java_home`
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

    dnl Check that the JDK found is correct architecture (at least 2 reasons to
    dnl check: officebean needs to link -ljawt, and libjpipe.so needs to be
    dnl loaded by java to run JunitTests:
    if test "$build_os" = "cygwin"; then
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
        JAVAINTERPRETER=`win_short_path_for_make "$JAVAINTERPRETER"`
    elif test "$cross_compiling" != "yes"; then
        case $CPUNAME in
            AARCH64|AXP|X86_64|HPPA|IA64|POWERPC64|S390X|SPARC64|GODSON64)
                if test -f "$JAVAINTERPRETER" -a "`$JAVAINTERPRETER -version 2>&1 | $GREP -i 64-bit`" = "" >/dev/null; then
                    AC_MSG_WARN([You are building 64-bit binaries but the JDK $JAVAINTERPRETER is 32-bit])
                    AC_MSG_ERROR([You should pass the --with-jdk-home option pointing to a 64-bit JDK])
                fi
                ;;
            *) # assumption: everything else 32-bit
                if test -f "$JAVAINTERPRETER" -a "`$JAVAINTERPRETER -version 2>&1 | $GREP -i 64-bit`" != ""  >/dev/null; then
                    AC_MSG_WARN([You are building 32-bit binaries but the JDK $howfound is 64-bit])
                    AC_MSG_ERROR([You should pass the --with-jdk-home option pointing to a (32-bit) JDK])
                fi
                ;;
        esac
    fi
fi

HAVE_JAVA6=
HAVE_JAVA9=
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
            if test "$_jdk_ver" -ge 10600; then
                HAVE_JAVA6=TRUE
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
            if test "$_jdk_ver" -ge 10600; then
                HAVE_JAVA6=TRUE
            fi
            if test "$_jdk_ver" -ge 10900; then
                HAVE_JAVA9=TRUE
            fi

            AC_MSG_RESULT([checked (JDK $_jdk)])
            JAVA_HOME=`echo $JAVAINTERPRETER | $SED -n "s,//*bin//*java,,p"`
            if test "$_os" = "WINNT"; then
                JAVA_HOME=`echo $JAVA_HOME | $SED "s,\.[[eE]][[xX]][[eE]]$,,"`
            fi

            # set to limit VM usage for JunitTests
            JAVAIFLAGS=-Xmx64M
            # set to limit VM usage for javac
            JAVAFLAGS=-J-Xmx128M
        fi
    else
        AC_MSG_ERROR([Java not found. You need at least jdk-1.5, or gcj-4])
    fi
else
    dnl Java disabled
    JAVA_HOME=NO_JAVA_HOME ; export JAVA_HOME
fi
AC_SUBST([HAVE_JAVA6])
AC_SUBST([HAVE_JAVA9])

dnl ===================================================================
dnl Set target Java bytecode version
dnl ===================================================================
if test "$ENABLE_JAVA" != ""; then
    if test "$HAVE_JAVA9" = "TRUE"; then
        _java_target_ver="1.6"
    else
        _java_target_ver="1.5"
    fi
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
        JAVACOMPILER=`win_short_path_for_make "$JAVACOMPILER"`
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
        JAVADOC=`win_short_path_for_make "$JAVADOC"`
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
        dnl now that we probably have the path to the real javac, make a JAVA_HOME out of it..
        if test "$JAVA_HOME" != "/usr"; then
            if test "$_os" = "Darwin" -o "$OS_FOR_BUILD" = MACOSX; then
                dnl Leopard returns a non-suitable path with readlink - points to "Current" only
                JAVA_HOME=$(echo $JAVA_HOME | $SED -e s,/Current/Commands/javac$,/CurrentJDK/Home,)
                dnl Tiger already returns a JDK path..
                JAVA_HOME=$(echo $JAVA_HOME | $SED -e s,/CurrentJDK/Commands/javac$,/CurrentJDK/Home,)
            else
                JAVA_HOME=$(echo $JAVA_HOME | $SED -e s,/bin/javac$,,)
                dnl check that we have a directory as certain distros eg gentoo substitute javac for a script
                dnl that checks which version to run
                if test -f "$JAVA_HOME"; then
                    JAVA_HOME=""; # set JAVA_HOME to null if it's a file
                fi
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

if test -z "$JAWTLIB" -a -n "$ENABLE_JAVA" -a "$_os" != Android -a \
    "$_os" != Darwin
then
    AC_MSG_CHECKING([for JAWT lib])
    if test "$_os" = WINNT; then
        if test "$WITH_MINGW" = yes; then
            # The path is taken care of in gb_LinkTarget__use_jawt
            # in RepositoryExternal.mk:
            JAWTLIB=-ljawt
        else
            # The path to $JAVA_HOME/lib/$JAWTLIB is part of $ILIB:
            JAWTLIB=jawt.lib
        fi
    else
        case "$host_cpu" in
        arm*)
            my_java_arch=arm
            ;;
        i*86)
            my_java_arch=i386
            ;;
        m68k)
            my_java_arch=mk68
            ;;
        powerpc)
            my_java_arch=ppc
            ;;
        powerpc64)
            my_java_arch=ppc64
            ;;
        powerpc64le)
            AS_IF([test -e "$JAVA_HOME/jre/lib/ppc64le/libjawt.so"], [my_java_arch=ppc64le], [my_java_arch=ppc64])
            JAVA_ARCH=$my_java_arch
            ;;
        x86_64)
            my_java_arch=amd64
            ;;
        *)
            my_java_arch=$host_cpu
            ;;
        esac
        JAWTLIB="-L$JAVA_HOME/jre/lib/$my_java_arch -ljawt"
        AS_IF([test "$JAVA_ARCH" != ""], [AC_DEFINE_UNQUOTED([JAVA_ARCH], ["$JAVA_ARCH"])])
    fi
    AC_MSG_RESULT([$JAWTLIB])
fi
AC_SUBST(JAWTLIB)

if test -n "$ENABLE_JAVA" -a -z "$JAVAINC"; then
    case "$host_os" in

    aix*)
        JAVAINC="-I$JAVA_HOME/include"
        JAVAINC="$JAVAINC -I$JAVA_HOME/include/aix"
        test -d "$JAVA_HOME/include/native_thread" && JAVAINC="$JAVAINC -I$JAVA_HOME/include/native_thread"
        ;;

    cygwin*)
        JAVAINC="-I$JAVA_HOME/include/win32"
        JAVAINC="$JAVAINC -I$JAVA_HOME/include"
        ;;

    darwin*)
        if test -d "$JAVA_HOME/include/darwin"; then
            JAVAINC="-I$JAVA_HOME/include  -I$JAVA_HOME/include/darwin"
        else
            JAVAINC=${ISYSTEM}$FRAMEWORKSHOME/JavaVM.framework/Versions/Current/Headers
        fi
        ;;

    dragonfly*)
        JAVAINC="-I$JAVA_HOME/include"
        test -d "$JAVA_HOME/include/native_thread" && JAVAINC="$JAVAINC -I$JAVA_HOME/include/native_thread"
        ;;

    freebsd*)
        JAVAINC="-I$JAVA_HOME/include"
        JAVAINC="$JAVAINC -I$JAVA_HOME/include/freebsd"
        JAVAINC="$JAVAINC -I$JAVA_HOME/include/bsd"
        JAVAINC="$JAVAINC -I$JAVA_HOME/include/linux"
        test -d "$JAVA_HOME/include/native_thread" && JAVAINC="$JAVAINC -I$JAVA_HOME/include/native_thread"
        ;;

    k*bsd*-gnu*)
        JAVAINC="-I$JAVA_HOME/include"
        JAVAINC="$JAVAINC -I$JAVA_HOME/include/linux"
        test -d "$JAVA_HOME/include/native_thread" && JAVAINC="$JAVAINC -I$JAVA_HOME/include/native_thread"
        ;;

    linux-gnu*)
        JAVAINC="-I$JAVA_HOME/include"
        JAVAINC="$JAVAINC -I$JAVA_HOME/include/linux"
        test -d "$JAVA_HOME/include/native_thread" && JAVAINC="$JAVAINC -I$JAVA_HOME/include/native_thread"
        ;;

    mingw*)
        JAVAINC="-I$JAVA_HOME/include"
        ;;

    *netbsd*)
        JAVAINC="-I$JAVA_HOME/include"
        JAVAINC="$JAVAINC -I$JAVA_HOME/include/netbsd"
        test -d "$JAVA_HOME/include/native_thread" && JAVAINC="$JAVAINC -I$JAVA_HOME/include/native_thread"
       ;;

    openbsd*)
        JAVAINC="-I$JAVA_HOME/include"
        JAVAINC="$JAVAINC -I$JAVA_HOME/include/openbsd"
        test -d "$JAVA_HOME/include/native_thread" && JAVAINC="$JAVAINC -I$JAVA_HOME/include/native_thread"
        ;;

    solaris*)
        JAVAINC="-I$JAVA_HOME/include"
        JAVAINC="$JAVAINC -I$JAVA_HOME/include/solaris"
        test -d "$JAVA_HOME/include/native_thread" && JAVAINC="$JAVAINC -I$JAVA_HOME/include/native_thread"
        ;;
    esac
fi
SOLARINC="$SOLARINC $JAVAINC"

AC_SUBST(JAVACOMPILER)
AC_SUBST(JAVADOC)
AC_SUBST(JAVAINTERPRETER)
AC_SUBST(JAVAIFLAGS)
AC_SUBST(JAVAFLAGS)
AC_SUBST(JAVA_CLASSPATH_NOT_SET)
AC_SUBST(JAVA_HOME)
AC_SUBST(JAVA_SOURCE_VER)
AC_SUBST(JAVA_TARGET_VER)
AC_SUBST(JDK)

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
            AC_MSG_NOTICE([EPM will be built.])
            BUILD_TYPE="$BUILD_TYPE EPM"
            EPM=${WORKDIR}/UnpackedTarball/epm/epm
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
                AC_MSG_ERROR([Install ESP Package Manager (http://www.msweet.org/projects.php?Z2) and/or specify the path to the right epm])
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
    if echo "$PKGFORMAT" | $EGREP rpm 2>&1 >/dev/null || \
       echo "$PKGFORMAT" | $EGREP pkg 2>&1 >/dev/null; then
        if test "$with_epm" = "no" -a "$_os" != "Darwin"; then
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
    for i in $PKGFORMAT; do
        case "$i" in
        aix | bsd | deb | pkg | rpm | native | portable)
            AC_MSG_ERROR(
                [--with-package-format='$PKGFORMAT' requires --enable-epm])
            ;;
        esac
    done
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
    enable_lotuswordpro=no
    enable_coinmp=yes
    enable_lpsolve=no
    enable_postgresql_sdbc=no
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
if test "$GNUMAKE_WIN_NATIVE" = "TRUE" ; then
    GPERF=`cygpath -m $GPERF`
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
                AC_MSG_ERROR([doxygen not found in \$PATH; specify its pathname via --with-doxygen=..., or disable its use via --without-doxygen])
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
            BUILD_UNOWINREG=
        else
            AC_MSG_RESULT([yes])
            BUILD_UNOWINREG=TRUE
        fi
        if test "$_os" != "WINNT" -a "$BUILD_UNOWINREG" = "TRUE"; then
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
    BUILD_UNOWINREG=
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
    SYSTEM_ZLIB=TRUE
    if test "$_os" != "Emscripten"; then
        # Emscripten provides its own zlib, don't check for that
        AC_CHECK_HEADER(zlib.h, [],
            [AC_MSG_ERROR(zlib.h not found. install zlib)], [])
        AC_CHECK_LIB(z, deflate, [ ZLIB_LIBS=-lz ],
            [AC_MSG_ERROR(zlib not found or functional)], [])
    fi
else
    AC_MSG_RESULT([internal])
    SYSTEM_ZLIB=
    BUILD_TYPE="$BUILD_TYPE ZLIB"
    ZLIB_CFLAGS="-I${WORKDIR}/UnpackedTarball/zlib"
    ZLIB_LIBS="-L${WORKDIR}/LinkTarget/StaticLibrary -lzlib"
fi
AC_SUBST(ZLIB_CFLAGS)
AC_SUBST(ZLIB_LIBS)
AC_SUBST(SYSTEM_ZLIB)

dnl ===================================================================
dnl Check for system jpeg
dnl ===================================================================
AC_MSG_CHECKING([which libjpeg to use])
if test "$with_jpeg_turbo" = "auto"; then
    # TODO use jpeg-turbo on Darwin too
    case "$_os" in
    WINNT)
        with_jpeg_turbo=yes
        ;;
    *)
        with_jpeg_turbo=no
        ;;
    esac
fi

if test "$with_system_jpeg" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_JPEG=TRUE
    AC_CHECK_HEADER(jpeglib.h, [],
        [AC_MSG_ERROR(jpeg.h not found. install libjpeg)], [])
    AC_CHECK_LIB(jpeg, jpeg_resync_to_restart, [ JPEG3RDLIB=-ljpeg ],
    [AC_MSG_CHECKING(jpeg library not found or fuctional)], [])
    libo_MINGW_CHECK_DLL([libjpeg])
else
    SYSTEM_JPEG=
    if test "$with_jpeg_turbo" = "yes"; then
        AC_MSG_RESULT([internal, jpeg-turbo])
        BUILD_TYPE="$BUILD_TYPE JPEG_TURBO"
        AC_CHECK_PROGS(NASM, [nasm nasmw yasm])

        if test -z "$NASM" -a "$build_os" = "cygwin"; then
            if test -n "$LODE_HOME" -a -x "$LODE_HOME/opt/bin/nasm"; then
                NASM="$LODE_HOME/opt/bin/nasm"
            elif test -x "/opt/lo/bin/nasm"; then
                NASM="/opt/lo/bin/nasm"
            fi
        fi
        if test -z "$NASM"; then
cat << _EOS
****************************************************************************
You need nasm (Netwide Assembler) to build internal jpeg library.
To get one please do:

_EOS
        if test "$build_os" = "cygwin"; then
cat << _EOS
install a pre-compiled binary for Win32

 mkdir -p /opt/lo/bin
 cd /opt/lo/bin
 wget http://dev-www.libreoffice.org/bin/cygwin/nasm.exe
 chmod +x nasm
_EOS
            else
cat << _EOS
consult http://svn.code.sf.net/p/libjpeg-turbo/code/trunk/BUILDING.txt
_EOS
            fi
cat << _EOS

or get and install one from http://www.nasm.us/

Then re-run autogen.sh

Note: autogen.sh will try to use /opt/lo/bin/nasm if the environment variable NASM is not already defined.
Alternatively, you can install the 'new' nasm where ever you want and make sure that \`which nasm\` finds it.

_EOS
        AC_MSG_ERROR([no nasm (Netwide Assembler) found])
        fi
    else
        AC_MSG_RESULT([internal, jpeg])
        BUILD_TYPE="$BUILD_TYPE JPEG"
    fi
fi
AC_SUBST(NASM)
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
    SYSTEM_CLUCENE=TRUE
    PKG_CHECK_MODULES(CLUCENE, libclucene-core)
    CLUCENE_CFLAGS=[$(printf '%s' "$CLUCENE_CFLAGS" | sed -e 's@-I[^ ]*/CLucene/ext@@' -e "s/-I/${ISYSTEM?}/g")]
    FilterLibs "${CLUCENE_LIBS}"
    CLUCENE_LIBS="${filteredlibs}"
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
    SYSTEM_CLUCENE=
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
    SYSTEM_EXPAT=TRUE
    AC_CHECK_HEADER(expat.h, [],
        [AC_MSG_ERROR(expat.h not found. install expat)], [])
    AC_CHECK_LIB([expat], [XML_ParserCreate], [:],
        [AC_MSG_RESULT(expat library not found or functional.)], [])
    libo_MINGW_CHECK_DLL([libexpat])
else
    AC_MSG_RESULT([internal])
    SYSTEM_EXPAT=
    BUILD_TYPE="$BUILD_TYPE EXPAT"
fi
AC_SUBST(SYSTEM_EXPAT)

AC_MSG_CHECKING([whether to enable Embedded OpenType support])
if test "$_os" != "WINNT" -a "$_os" != "Darwin" -a "$enable_eot" = "yes"; then
    ENABLE_EOT="TRUE"
    AC_DEFINE([ENABLE_EOT])
    AC_MSG_RESULT([yes])

    libo_CHECK_SYSTEM_MODULE([libeot],[LIBEOT],[libeot >= 0.01])
else
    ENABLE_EOT=
    AC_MSG_RESULT([no])
fi
AC_SUBST([ENABLE_EOT])

dnl ===================================================================
dnl Check for system librevenge
dnl ===================================================================
AS_IF([test "$COM" = "MSC"],
      [librevenge_libdir="${WORKDIR}/LinkTarget/Library"],
      [librevenge_libdir="${WORKDIR}/UnpackedTarball/librevenge/src/lib/.libs"]
)
libo_CHECK_SYSTEM_MODULE([librevenge],[REVENGE],[librevenge-0.0 >= 0.0.1],["-I${WORKDIR}/UnpackedTarball/librevenge/inc"],["-L${librevenge_libdir} -lrevenge-0.0"])

dnl ===================================================================
dnl Check for system libe-book
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libebook],[EBOOK],[libe-book-0.1])
libo_PKG_VERSION([EBOOK], [libe-book-0.1], [0.1.2])

dnl ===================================================================
dnl Check for system libetonyek
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libetonyek],[ETONYEK],[libetonyek-0.1])
libo_PKG_VERSION([ETONYEK], [libetonyek-0.1], [0.1.5])

dnl ===================================================================
dnl Check for system libfreehand
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libfreehand],[FREEHAND],[libfreehand-0.1])

dnl ===================================================================
dnl Check for system libodfgen
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libodfgen],[ODFGEN],[libodfgen-0.1])

dnl ===================================================================
dnl Check for system libcdr
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libcdr],[CDR],[libcdr-0.1])

dnl ===================================================================
dnl Check for system libmspub
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libmspub],[MSPUB],[libmspub-0.1])

dnl ===================================================================
dnl Check for system libmwaw
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libmwaw],[MWAW],[libmwaw-0.3 >= 0.3.1])
libo_PKG_VERSION([MWAW], [libmwaw-0.3], [0.3.8])

dnl ===================================================================
dnl Check for system libpagemaker
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libpagemaker],[PAGEMAKER],[libpagemaker-0.0])

dnl ===================================================================
dnl Check for system libzmf
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libzmf],[ZMF],[libzmf-0.0])

dnl ===================================================================
dnl Check for system libvisio
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libvisio],[VISIO],[libvisio-0.1])

dnl ===================================================================
dnl Check for system libcmis
dnl ===================================================================
# libcmis requires curl and we can't build curl for iOS
if test $_os != iOS; then
    libo_CHECK_SYSTEM_MODULE([libcmis],[CMIS],[libcmis-0.5 >= 0.5.0])
    ENABLE_CMIS=TRUE
else
    ENABLE_CMIS=
fi
AC_SUBST(ENABLE_CMIS)

dnl ===================================================================
dnl Check for system libwpd
dnl ===================================================================
AS_IF([test "$COM" = "MSC"],
      [libwpd_libdir="${WORKDIR}/LinkTarget/Library"],
      [libwpd_libdir="${WORKDIR}/UnpackedTarball/libwpd/src/lib/.libs"]
)
libo_CHECK_SYSTEM_MODULE([libwpd],[WPD],[libwpd-0.10],["-I${WORKDIR}/UnpackedTarball/libwpd/inc"],["-L${libwpd_libdir} -lwpd-0.10"])

dnl ===================================================================
dnl Check for system lcms2
dnl ===================================================================
if test "$with_system_lcms2" = "yes"; then
    libo_MINGW_CHECK_DLL([liblcms2])
else
    SYSTEM_LCMS2=
fi
libo_CHECK_SYSTEM_MODULE([lcms2],[LCMS2],[lcms2],["-I${WORKDIR}/UnpackedTarball/lcms2/include"],["-L${WORKDIR}/UnpackedTarball/lcms2/src/.libs -llcms2"])
if test "$GCC" = "yes"; then
    LCMS2_CFLAGS="${LCMS2_CFLAGS} -Wno-long-long"
fi
if test "$COM" = "MSC"; then # override the above
    LCMS2_LIBS=${WORKDIR}/UnpackedTarball/lcms2/bin/lcms2.lib
fi

dnl ===================================================================
dnl Check for system cppunit
dnl ===================================================================
if test "$cross_compiling" != "yes"; then
    libo_CHECK_SYSTEM_MODULE([cppunit],[CPPUNIT],[cppunit >= 1.12.0])
fi

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
    FilterLibs "${FREETYPE_LIBS}"
    FREETYPE_LIBS="${filteredlibs}"
    SYSTEM_FREETYPE=TRUE
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
dnl Check for system libabw
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libabw],[ABW],[libabw-0.1])

dnl ===================================================================
dnl Check for system libwps
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libwps],[WPS],[libwps-0.4])
libo_PKG_VERSION([WPS], [libwps-0.4], [0.4.3])

dnl ===================================================================
dnl Check for system libwpg
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libwpg],[WPG],[libwpg-0.3])

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
    SYSTEM_LIBXSLT=TRUE
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
        FilterLibs "${LIBXSLT_LIBS}"
        LIBXSLT_LIBS="${filteredlibs}"
        PKG_CHECK_MODULES(LIBEXSLT, libexslt)
        LIBEXSLT_CFLAGS=$(printf '%s' "$LIBEXSLT_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
        FilterLibs "${LIBEXSLT_LIBS}"
        LIBEXSLT_LIBS="${filteredlibs}"
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
    SYSTEM_LIBXSLT=
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

# ===================================================================
# Check for system libxml
# ===================================================================
AC_MSG_CHECKING([which libxml to use])
if test "$with_system_libxml" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_LIBXML=TRUE
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
        FilterLibs "${LIBXML_LIBS}"
        LIBXML_LIBS="${filteredlibs}"
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
    SYSTEM_LIBXML=
    LIBXML_CFLAGS="-I${WORKDIR}/UnpackedTarball/xml2/include"
    if test "$COM" = "MSC"; then
        LIBXML_CFLAGS="${LIBXML_CFLAGS} -I${WORKDIR}/UnpackedTarball/icu/source/i18n -I${WORKDIR}/UnpackedTarball/icu/source/common"
    fi
    if test "$COM" = "MSC"; then
        LIBXML_LIBS="${WORKDIR}/UnpackedTarball/xml2/win32/bin.msvc/libxml2.lib"
    else
        LIBXML_LIBS="-L${WORKDIR}/UnpackedTarball/xml2/.libs -lxml2"
    fi
    BUILD_TYPE="$BUILD_TYPE LIBXML2"
fi
AC_SUBST(SYSTEM_LIBXML)
if test -z "$SYSTEM_LIBXML_FOR_BUILD"; then
    SYSTEM_LIBXML_FOR_BUILD="$SYSTEM_LIBXML"
fi
AC_SUBST(SYSTEM_LIBXML_FOR_BUILD)
AC_SUBST(LIBXML_CFLAGS)
AC_SUBST(LIBXML_LIBS)
AC_SUBST(XMLLINT)

# =====================================================================
# Checking for a Python interpreter with version >= 2.6.
# Build and runtime requires Python 3 compatible version (>= 2.6).
# Optionally user can pass an option to configure, i. e.
# ./configure PYTHON=/usr/bin/python
# =====================================================================
if test "$build_os" != "cygwin" -a "$enable_python" != fully-internal; then
    # This allows a lack of system python with no error, we use internal one in that case.
    AM_PATH_PYTHON([2.6],, [:])
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
        AC_MSG_RESULT([fully internal])
        enable_python=internal
    elif test "$cross_compiling" = yes; then
        AC_MSG_RESULT([system])
        enable_python=system
    else
        # Unset variables set by the above AM_PATH_PYTHON so that
        # we actually do check anew.
        unset PYTHON am_cv_pathless_PYTHON ac_cv_path_PYTHON am_cv_python_version am_cv_python_platform am_cv_python_pythondir am_cv_python_pyexecdir
        AM_PATH_PYTHON([3.3],, [:])
        if test "$PYTHON" = ":"; then
            if test -z "$PYTHON_FOR_BUILD"; then
                AC_MSG_RESULT([fully internal])
            else
                AC_MSG_RESULT([internal])
            fi
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
fully-internal)
    AC_MSG_RESULT([fully internal])
    enable_python=internal
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
        python_version=2.7
        PYTHON=python$python_version
        if test -d "$FRAMEWORKSHOME/Python.framework/Versions/${python_version}/include/python${python_version}"; then
            PYTHON_CFLAGS="-I$FRAMEWORKSHOME/Python.framework/Versions/${python_version}/include/python${python_version}"
            PYTHON_LIBS="-framework Python"
        else
            PYTHON_CFLAGS="`$PYTHON-config --includes`"
            PYTHON_LIBS="`$PYTHON-config --libs`"
        fi
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
        FilterLibs "${PYTHON_LIBS}"
        PYTHON_LIBS="${filteredlibs}"
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
    SYSTEM_PYTHON=TRUE

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
       ]])],[AC_MSG_RESULT([ok])],[AC_MSG_ERROR([Python >= 3.3 is needed when building with Python 3, or Python >= 2.6 when building with Python 2])],[AC_MSG_RESULT([skipped; cross-compiling])])
    CFLAGS=$save_CFLAGS
    AC_LANG_POP(C)

    dnl FIXME Check if the Python library can be linked with, too?
    ;;

internal)
    SYSTEM_PYTHON=
    PYTHON_VERSION_MAJOR=3
    # On windows we are supporting two different python versions: 3.3 and 3.5.
    # We msut do it as long as we support MSVC 2013. Python 3.3 can be removed
    # when MSVC 2013 support was dropped.
    if test "$COM" = "MSC" -a "$VCVER" = "120"; then
        PYTHON_VERSION_MINOR=3
    else
        PYTHON_VERSION_MINOR=5
    fi
    PYTHON_VERSION=${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}.0
    AC_DEFINE_UNQUOTED([PYTHON_VERSION_STRING], [L"${PYTHON_VERSION}"])
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
    SYSTEM_PYTHON=
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
        SYSTEM_MARIADB=TRUE
        #AC_PATH_PROG(MARIADBCONFIG, [mariadb_config])
        if test -z "$MARIADBCONFIG"; then
            AC_PATH_PROG(MARIADBCONFIG, [mysql_config])
            if test -z "$MARIADBCONFIG"; then
                AC_MSG_ERROR([mysql_config is missing. Install MySQL client library development package.])
                #AC_MSG_ERROR([mariadb_config and mysql_config are missing. Install MariaDB or MySQL client library development package.])
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
        if test "$COM_IS_CLANG" = TRUE; then
            MARIADB_CFLAGS=$(printf '%s' "$MARIADB_CFLAGS" | sed -e s/-fstack-protector-strong//)
        fi
        MARIADB_LIBS=`$MARIADBCONFIG --libs_r`
        dnl At least mariadb-5.5.34-3.fc20.x86_64 plus
        dnl mariadb-5.5.34-3.fc20.i686 reports 64-bit specific output even under
        dnl linux32:
        if test "$OS" = LINUX -a "$CPUNAME" = INTEL; then
            MARIADB_CFLAGS=$(printf '%s' "$MARIADB_CFLAGS" | sed -e s/-m64//)
            MARIADB_LIBS=$(printf '%s' "$MARIADB_LIBS" \
                | sed -e 's|/lib64/|/lib/|')
        fi
        FilterLibs "${MARIADB_LIBS}"
        MARIADB_LIBS="${filteredlibs}"
        AC_MSG_RESULT([includes '$MARIADB_CFLAGS', libraries '$MARIADB_LIBS'])
        AC_MSG_CHECKING([whether to bundle the MySQL/MariaDB client library])
        if test "$enable_bundle_mariadb" = "yes"; then
            AC_MSG_RESULT([yes])
            BUNDLE_MARIADB=TRUE
            LIBMARIADB=lib$(echo "${MARIADB_LIBS}" | sed -e 's/[[[:space:]]]\{1,\}-l\([[^[:space:]]]\{1,\}\)/\
\1\
/g' -e 's/^-l\([[^[:space:]]]\{1,\}\)[[[:space:]]]*/\
\1\
/g' | grep -E '(mysqlclient|mariadb)')
            if test "$_os" = "Darwin"; then
                LIBMARIADB=${LIBMARIADB}.dylib
            elif test "$_os" = "WINNT"; then
                LIBMARIADB=${LIBMARIADB}.dll
            else
                LIBMARIADB=${LIBMARIADB}.so
            fi
            LIBMARIADB_PATH=$($MARIADBCONFIG --variable=pkglibdir)
            AC_MSG_CHECKING([for $LIBMARIADB in $LIBMARIADB_PATH])
            if test -e "$LIBMARIADB_PATH/$LIBMARIADB"; then
                AC_MSG_RESULT([found.])
                PathFormat "$LIBMARIADB_PATH"
                LIBMARIADB_PATH="$formatted_path"
            else
                AC_MSG_ERROR([not found.])
            fi
        else
            AC_MSG_RESULT([no])
            BUNDLE_MARIADB=
        fi
    else
        AC_MSG_RESULT([internal])
        SYSTEM_MARIADB=
        MARIADB_CFLAGS="-I${WORKDIR}/UnpackedTarball/mariadb/include"
        MARIADB_LIBS="-L${WORKDIR}/LinkTarget/StaticLibrary -lmariadblib"
        BUILD_TYPE="$BUILD_TYPE MARIADB"
    fi

    AC_SUBST(SYSTEM_MARIADB)
    AC_SUBST(MARIADB_CFLAGS)
    AC_SUBST(MARIADB_LIBS)
    AC_SUBST(LIBMARIADB)
    AC_SUBST(LIBMARIADB_PATH)
    AC_SUBST(BUNDLE_MARIADB)

    AC_LANG_PUSH([C++])
    dnl ===================================================================
    dnl Check for system MySQL C++ Connector
    dnl ===================================================================
    # FIXME!
    # who thought this too-generic cppconn dir was a good idea?
    AC_MSG_CHECKING([MySQL Connector/C++])
    if test "$with_system_mysql_cppconn" = "yes"; then
        AC_MSG_RESULT([external])
        SYSTEM_MYSQL_CPPCONN=TRUE
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
      ]])],[AC_MSG_RESULT(OK)],[AC_MSG_ERROR([not suitable, we need >= 1.0.6])],[AC_MSG_ERROR([MySQL C++ Connecter not tested with cross-compilation])])

        AC_LANG_POP([C++])
        LIBS=$save_LIBS
    else
        AC_MSG_RESULT([internal])
        BUILD_TYPE="$BUILD_TYPE MYSQLCPPCONN"
        SYSTEM_MYSQL_CPPCONN=
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
        SYSTEM_HSQLDB=TRUE
        if test -z $HSQLDB_JAR; then
            HSQLDB_JAR=/usr/share/java/hsqldb.jar
        fi
        if ! test -f $HSQLDB_JAR; then
               AC_MSG_ERROR(hsqldb.jar not found.)
        fi
        AC_MSG_CHECKING([whether hsqldb is 1.8.0.x])
        export HSQLDB_JAR
        if $PERL -e \
           'use Archive::Zip;
            my $file = "$ENV{'HSQLDB_JAR'}";
            my $zip = Archive::Zip->new( $file );
            my $mf = $zip->contents ( "META-INF/MANIFEST.MF" );
            if ( $mf =~ m/Specification-Version: 1.8.*/ )
            {
                push @l, split(/\n/, $mf);
                foreach my $line (@l)
                {
                    if ($line =~ m/Specification-Version:/)
                    {
                        ($t, $version) = split (/:/,$line);
                        $version =~ s/^\s//;
                        ($a, $b, $c, $d) = split (/\./,$version);
                        if ($c == "0" && $d > "8")
                        {
                            exit 0;
                        }
                        else
                        {
                            exit 1;
                        }
                    }
                }
            }
            else
            {
                exit 1;
            }'; then
            AC_MSG_RESULT([yes])
        else
            AC_MSG_ERROR([no, you need hsqldb >= 1.8.0.9 but < 1.8.1])
        fi
    else
        AC_MSG_RESULT([internal])
        SYSTEM_HSQLDB=
        BUILD_TYPE="$BUILD_TYPE HSQLDB"
        AC_MSG_CHECKING([whether hsqldb should be built with JDBC 4.1])
        javanumver=`$JAVAINTERPRETER -version 2>&1 | $AWK -v num=true -f $SRC_ROOT/solenv/bin/getcompver.awk`
        if expr "$javanumver" '>=' 000100060000 > /dev/null; then
            AC_MSG_RESULT([yes])
            HSQLDB_USE_JDBC_4_1=TRUE
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

    postgres_interface=""
    if test "$with_system_postgresql" = "yes"; then
        postgres_interface="external PostgreSQL"
        SYSTEM_POSTGRESQL=TRUE
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
        FilterLibs "${POSTGRESQL_LIB}"
        POSTGRESQL_LIB="${filteredlibs}"
    else
        # if/when anything else than PostgreSQL uses Kerberos,
        # move this out of `test "x$enable_postgresql_sdbc" != "xno"'
        WITH_KRB5=
        WITH_GSSAPI=
        case "$_os" in
        Darwin)
            # MacOS X has system MIT Kerberos 5 since 10.4
            if test "$with_krb5" != "no"; then
                WITH_KRB5=TRUE
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
                WITH_GSSAPI=TRUE
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
                WITH_KRB5=TRUE
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
                WITH_GSSAPI=TRUE
                save_LIBS=$LIBS
                AC_SEARCH_LIBS(gss_init_sec_context, [gssapi_krb5 gss 'gssapi -lkrb5 -lcrypto'], [],
                    [AC_MSG_ERROR([could not find function 'gss_init_sec_context' required for GSSAPI])])
                GSSAPI_LIBS=$LIBS
                LIBS=$save_LIBS
            fi
            ;;
        *)
            if test "$with_krb5" = "yes"; then
                WITH_KRB5=TRUE
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
                WITH_GSSAPI=TRUE
                save_LIBS=$LIBS
                AC_SEARCH_LIBS(gss_init_sec_context, [gssapi_krb5 gss 'gssapi -lkrb5 -lcrypto'], [],
                    [AC_MSG_ERROR([could not find function 'gss_init_sec_context' required for GSSAPI])])
                LIBS=$save_LIBS
                GSSAPI_LIBS=$LIBS
            fi
        esac

        if test -n "$with_libpq_path"; then
            SYSTEM_POSTGRESQL=TRUE
            postgres_interface="external libpq"
            POSTGRESQL_LIB="-L${with_libpq_path}/lib/"
            POSTGRESQL_INC=-I"${with_libpq_path}/include/"
        else
            SYSTEM_POSTGRESQL=
            postgres_interface="internal"
            POSTGRESQL_LIB=""
            POSTGRESQL_INC="%OVERRIDE_ME%"
            BUILD_TYPE="$BUILD_TYPE POSTGRESQL"
        fi
    fi

    AC_MSG_CHECKING([PostgreSQL C interface])
    AC_MSG_RESULT([$postgres_interface])

    if test "${SYSTEM_POSTGRESQL}" = "TRUE"; then
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
    BUILD_POSTGRESQL_SDBC=TRUE
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
if test "$enable_firebird_sdbc" = "yes" ; then
    SCPDEFS="$SCPDEFS -DWITH_FIREBIRD_SDBC"

    dnl ===================================================================
    dnl Check for system Firebird
    dnl ===================================================================
    AC_MSG_CHECKING([which Firebird to use])
    if test "$with_system_firebird" = "yes"; then
        AC_MSG_RESULT([external])
        SYSTEM_FIREBIRD=TRUE
        AC_PATH_PROG(FIREBIRDCONFIG, [fb_config])
        if test -z "$FIREBIRDCONFIG"; then
            AC_MSG_NOTICE([No fb_config -- using pkg-config])
            PKG_CHECK_MODULES([FIREBIRD], [fbclient >= 3], [FIREBIRD_PKGNAME=fbclient], [
                PKG_CHECK_MODULES([FIREBIRD], [fbembed], [FIREBIRD_PKGNAME=fbembed])
            ])
            FIREBIRD_VERSION=`pkg-config --modversion "$FIREBIRD_PKGNAME"`
        else
            AC_MSG_NOTICE([fb_config found])
            FIREBIRD_VERSION=`$FIREBIRDCONFIG --version`
            AC_MSG_CHECKING([for Firebird Client library])
            FIREBIRD_CFLAGS=`$FIREBIRDCONFIG --cflags`
            FIREBIRD_LIBS=`$FIREBIRDCONFIG --embedlibs`
            FilterLibs "${FIREBIRD_LIBS}"
            FIREBIRD_LIBS="${filteredlibs}"
        fi
        AC_MSG_RESULT([includes `$FIREBIRD_CFLAGS', libraries `$FIREBIRD_LIBS'])
        AC_MSG_CHECKING([Firebird version])
        if test -n "${FIREBIRD_VERSION}"; then
            FIREBIRD_MAJOR=`echo $FIREBIRD_VERSION | cut -d"." -f1`
            FIREBIRD_MINOR=`echo $FIREBIRD_VERSION | cut -d"." -f2`
            if test "$FIREBIRD_MAJOR" -eq "2" -a "$FIREBIRD_MINOR" -eq "5"; then
                AC_MSG_RESULT([OK])
            elif test "$FIREBIRD_MAJOR" -eq "3" -a "$FIREBIRD_MINOR" -eq "0"; then
                AC_DEFINE(HAVE_FIREBIRD_30, 1)
                AC_MSG_RESULT([OK])
            else
                AC_MSG_ERROR([Ensure firebird 2.5.x or 3.0.x is installed])
            fi
        else
            __save_CFLAGS="${CFLAGS}"
            CFLAGS="${CFLAGS} ${FIREBIRD_CFLAGS}"
            AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <ibase.h>
#if defined(FB_API_VER) && FB_API_VER == 25
int fb_api_is_25(void) { return 0; }
#elif defined(FB_API_VER) && FB_API_VER == 30
int fb_api_is_30(void) { return 0; }
#else
#error "Wrong Firebird API version"
#endif]])],AC_MSG_RESULT([OK]),AC_MSG_ERROR([Ensure firebird 2.5.x or 3.0.x is installed]))
            if nm conftest.$OBJEXT | grep fb_api_is_30; then
                AC_DEFINE(HAVE_FIREBIRD_30, 1)
            fi
            CFLAGS="${__save_CFLAGS}"
        fi
        ENABLE_FIREBIRD_SDBC="TRUE"
    elif test "$enable_database_connectivity" != yes; then
        AC_MSG_RESULT([none])
    elif test "$cross_compiling" = "yes"; then
        AC_MSG_RESULT([none])
    else
        dnl We need libatomic-ops for any non X86/X64 system
        if test "${CPUNAME}" != INTEL -a "${CPUNAME}" != X86_64; then
            dnl ===================================================================
            dnl Check for system libatomic-ops
            dnl ===================================================================
            libo_CHECK_SYSTEM_MODULE([libatomic_ops],[ATOMIC_OPS],[atomic_ops >= 0.7.2])
            if test "$with_system_libatomic_ops" = "yes"; then
                SYSTEM_LIBATOMIC_OPS=TRUE
                AC_CHECK_HEADERS(atomic_ops.h, [],
                [AC_MSG_ERROR(atomic_ops.h not found. install libatomic-ops)], [])
            else
                SYSTEM_LIBATOMIC_OPS=
                LIBATOMIC_OPS_CFLAGS="-I${WORKDIR}/UnpackedTarball/libatomic_ops/include"
                LIBATOMIC_OPS_LIBS="-latomic_ops"
                BUILD_TYPE="$BUILD_TYPE LIBATOMIC_OPS"
            fi
        fi

        AC_MSG_RESULT([internal])
        SYSTEM_FIREBIRD=
        FIREBIRD_CFLAGS="-I${WORKDIR}/UnpackedTarball/firebird/include"
        FIREBIRD_LIBS="-lfbembed"

        BUILD_TYPE="$BUILD_TYPE FIREBIRD"
        ENABLE_FIREBIRD_SDBC="TRUE"
    fi
fi
AC_SUBST(ENABLE_FIREBIRD_SDBC)
AC_SUBST(SYSTEM_LIBATOMIC_OPS)
AC_SUBST(LIBATOMIC_OPS_CFLAGS)
AC_SUBST(LIBATOMIC_OPS_LIBS)
AC_SUBST(SYSTEM_FIREBIRD)
AC_SUBST(FIREBIRD_CFLAGS)
AC_SUBST(FIREBIRD_LIBS)
AC_SUBST(HAVE_FIREBIRD_30)
dnl AC_SUBST([TOMMATH_CFLAGS])
dnl AC_SUBST([TOMMATH_LIBS])

dnl ===================================================================
dnl Check for system curl
dnl ===================================================================
AC_MSG_CHECKING([which libcurl to use])
if test "$with_system_curl" = "auto"; then
    with_system_curl="$with_system_libs"
fi

if test "$with_system_curl" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_CURL=TRUE

    # First try PKGCONFIG and then fall back
    PKG_CHECK_MODULES(CURL, libcurl >= 7.19.4,, [:])

    if test -n "$CURL_PKG_ERRORS"; then
        AC_PATH_PROG(CURLCONFIG, curl-config)
        if test -z "$CURLCONFIG"; then
            AC_MSG_ERROR([curl development files not found])
        fi
        CURL_LIBS=`$CURLCONFIG --libs`
        FilterLibs "${CURL_LIBS}"
        CURL_LIBS="${filteredlibs}"
        CURL_CFLAGS=$("$CURLCONFIG" --cflags | sed -e "s/-I/${ISYSTEM?}/g")
        curl_version=`$CURLCONFIG --version | $SED -e 's/^libcurl //'`

        AC_MSG_CHECKING([whether libcurl is >= 7.19.4])
        case $curl_version in
        dnl brackets doubled below because Autoconf uses them as m4 quote characters,
        dnl so they need to be doubled to end up in the configure script
        7.19.4|7.19.[[5-9]]|7.[[2-9]]?.*|7.???.*|[[8-9]].*|[[1-9]][[0-9]].*)
            AC_MSG_RESULT([yes])
            ;;
        *)
            AC_MSG_ERROR([no, you have $curl_version])
            ;;
        esac
    fi

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
    ENABLE_CURL=TRUE
elif test $_os = iOS; then
    # Let's see if we need curl, I think not?
    AC_MSG_RESULT([none])
    ENABLE_CURL=
else
    AC_MSG_RESULT([internal])
    SYSTEM_CURL=
    BUILD_TYPE="$BUILD_TYPE CURL"
    ENABLE_CURL=TRUE
fi
AC_SUBST(SYSTEM_CURL)
AC_SUBST(CURL_CFLAGS)
AC_SUBST(CURL_LIBS)
AC_SUBST(ENABLE_CURL)

dnl ===================================================================
dnl Check for system boost
dnl ===================================================================
AC_MSG_CHECKING([which boost to use])
if test "$with_system_boost" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_BOOST=TRUE
    AX_BOOST_BASE(1.47)
    AX_BOOST_DATE_TIME
    AX_BOOST_FILESYSTEM
    AX_BOOST_IOSTREAMS
    mingw_boost_date_time_dll=`echo $BOOST_DATE_TIME_LIB | sed -e 's/^-l//' -e 's/\.dll$//'`
    libo_MINGW_TRY_DLL([$mingw_boost_date_time_dll])
    if test -z "$MINGW_BOOST_DATE_TIME_DLL"; then
        # if not found, try again with 'lib' prefix
        libo_MINGW_CHECK_DLL([lib$mingw_boost_date_time_dll])
    fi
    AC_LANG_PUSH([C++])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS $BOOST_CPPFLAGS $CXXFLAGS_CXX11"
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
    # this is in m4/ax_boost_base.m4
    FilterLibs "${BOOST_LDFLAGS}"
    BOOST_LDFLAGS="${filteredlibs}"
else
    AC_MSG_RESULT([internal])
    BUILD_TYPE="$BUILD_TYPE BOOST"
    SYSTEM_BOOST=
    if test "${COM}" = "GCC" -o "${COM_IS_CLANG}" = "TRUE"; then
        # use warning-suppressing wrapper headers
        BOOST_CPPFLAGS="-I${SRC_ROOT}/external/boost/include -I${WORKDIR}/UnpackedTarball/boost"
    else
        BOOST_CPPFLAGS="-I${WORKDIR}/UnpackedTarball/boost"
    fi
fi
AC_SUBST(SYSTEM_BOOST)

dnl ===================================================================
dnl Check for system mdds
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([mdds], [MDDS], [mdds-1.2 >= 1.2.0], ["-I${WORKDIR}/UnpackedTarball/mdds/include"])

dnl ===================================================================
dnl Check for system glm
dnl ===================================================================
AC_MSG_CHECKING([which glm to use])
if test "$with_system_glm" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_GLM=TRUE
    AC_LANG_PUSH([C++])
    AC_CHECK_HEADER([glm/glm.hpp], [],
       [AC_MSG_ERROR([glm/glm.hpp not found. install glm])], [])
    AC_LANG_POP([C++])
else
    AC_MSG_RESULT([internal])
    BUILD_TYPE="$BUILD_TYPE GLM"
    SYSTEM_GLM=
    GLM_CFLAGS="${ISYSTEM}${WORKDIR}/UnpackedTarball/glm"
fi
AC_SUBST([GLM_CFLAGS])
AC_SUBST([SYSTEM_GLM])

dnl ===================================================================
dnl Check for system glew
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([glew], [GLEW], [glew >= 1.10.0])
AS_IF([test "$with_system_glew" = "yes"],
    [PKG_CHECK_EXISTS([glew >= 1.12.0], [AC_DEFINE([HAVE_GLEW_1_12])])],
    [AC_DEFINE([HAVE_GLEW_1_12])])

dnl ===================================================================
dnl Check for system glyphy
dnl ===================================================================
dnl We currently use GLyphy only on Windows
if test $_os = WINNT; then
    libo_CHECK_SYSTEM_MODULE([glyphy], [GLYPHY], [glyphy >= 0.12.0], ["-I${WORKDIR}/UnpackedTarball/glyphy/src"])
fi

dnl ===================================================================
dnl Check for system odbc
dnl ===================================================================
AC_MSG_CHECKING([which odbc headers to use])
if test "$with_system_odbc" = "yes" -o '(' "$with_system_headers" = "yes" -a "$with_system_odbc" = "auto" ')' -o '(' "$_os" = "WINNT" -a  "$with_system_odbc" != "no" ')'; then
    AC_MSG_RESULT([external])
    SYSTEM_ODBC_HEADERS=TRUE

    if test "$build_os" = "cygwin"; then
        save_CPPFLAGS=$CPPFLAGS
        find_winsdk
        PathFormat "$winsdktest"
        CPPFLAGS="$CPPFLAGS -I$formatted_path/include/um -I$formatted_path/Include/$winsdklibsubdir/um -I$formatted_path/include -I$formatted_path/include/shared -I$formatted_path/include/$winsdklibsubdir/shared"
        AC_CHECK_HEADER(sqlext.h, [],
            [AC_MSG_ERROR(odbc not found. install odbc)],
            [#include <windows.h>])
        CPPFLAGS=$save_CPPFLAGS
    else
        AC_CHECK_HEADER(sqlext.h, [],
            [AC_MSG_ERROR(odbc not found. install odbc)],[])
    fi
elif test "$enable_database_connectivity" != yes; then
    AC_MSG_RESULT([none])
else
    AC_MSG_RESULT([internal])
    SYSTEM_ODBC_HEADERS=
fi
AC_SUBST(SYSTEM_ODBC_HEADERS)


dnl ===================================================================
dnl Check for system openldap
dnl ===================================================================

if test "$_os" != "WINNT" -a "$_os" != "iOS" -a "$_os" != "Android"; then
AC_MSG_CHECKING([which openldap library to use])
if test "$with_system_openldap" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_OPENLDAP=TRUE
    AC_CHECK_HEADERS(ldap.h, [], [AC_MSG_ERROR(ldap.h not found. install openldap libs)], [])
    AC_CHECK_LIB([ldap], [ldap_simple_bind_s], [:], [AC_MSG_ERROR(openldap lib not found or functional)], [])
    AC_CHECK_LIB([ldap], [ldap_set_option], [:], [AC_MSG_ERROR(openldap lib not found or functional)], [])
else
    AC_MSG_RESULT([internal])
    SYSTEM_OPENLDAP=
    BUILD_TYPE="$BUILD_TYPE OPENLDAP"
fi
fi
AC_SUBST(SYSTEM_OPENLDAP)

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
elif test $_os = iOS -o $_os = Android; then
    # We don't build NSS for iOS and Android
    AC_DEFINE(USE_TLS_OPENSSL)
    TLS=OPENSSL
else
    # default to using NSS, it results in smaller oox lib
    AC_DEFINE(USE_TLS_NSS)
    TLS=NSS
fi
AC_MSG_RESULT([$TLS])
AC_SUBST(TLS)

dnl ===================================================================
dnl Check for system NSS
dnl ===================================================================
if test $_os != iOS; then
    libo_CHECK_SYSTEM_MODULE([nss],[NSS],[nss >= 3.9.3 nspr >= 4.8])
fi
if test "$with_system_nss" = "yes"; then
    libo_MINGW_CHECK_DLL([libnspr4])
    libo_MINGW_CHECK_DLL([libplc4])
    libo_MINGW_CHECK_DLL([libplds4])
    libo_MINGW_CHECK_DLL([nss3])
    libo_MINGW_CHECK_DLL([nssutil3])
    libo_MINGW_CHECK_DLL([smime3])
    libo_MINGW_CHECK_DLL([ssl3])
fi

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

ICU_MAJOR=57
ICU_MINOR=1
ICU_RECLASSIFIED_PREPEND_SET_EMPTY="TRUE"
ICU_RECLASSIFIED_CONDITIONAL_JAPANESE_STARTER="TRUE"
ICU_RECLASSIFIED_HEBREW_LETTER="TRUE"
AC_MSG_CHECKING([which icu to use])
if test "$with_system_icu" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_ICU=TRUE
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

        if test "$ICU_MAJOR" -ge "49" -o \( "$ICU_MAJOR" = "4" -a "$ICU_MINOR" -ge "6" \); then
            AC_MSG_RESULT([OK, $ICU_VERSION])
        else
            AC_MSG_ERROR([not suitable, only >= 4.6 supported currently])
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
        if test "$ICU_MAJOR" -ge "49"; then
            ICU_RECLASSIFIED_PREPEND_SET_EMPTY="TRUE"
            ICU_RECLASSIFIED_CONDITIONAL_JAPANESE_STARTER="TRUE"
            ICU_RECLASSIFIED_HEBREW_LETTER="TRUE"
        else
            ICU_RECLASSIFIED_PREPEND_SET_EMPTY=
            ICU_RECLASSIFIED_CONDITIONAL_JAPANESE_STARTER=
            ICU_RECLASSIFIED_HEBREW_LETTER=
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
    SYSTEM_ICU=
    BUILD_TYPE="$BUILD_TYPE ICU"
    # surprisingly set these only for "internal" (to be used by various other
    # external libs): the system icu-config is quite unhelpful and spits out
    # dozens of weird flags and also default path -I/usr/include
    ICU_CFLAGS="-I${WORKDIR}/UnpackedTarball/icu/source/i18n -I${WORKDIR}/UnpackedTarball/icu/source/common"
    ICU_LIBS="-L${WORKDIR}/UnpackedTarball/icu/source/lib"
fi
AC_SUBST(SYSTEM_ICU)
AC_SUBST(SYSTEM_GENBRK)
AC_SUBST(SYSTEM_GENCCODE)
AC_SUBST(SYSTEM_GENCMN)
AC_SUBST(ICU_MAJOR)
AC_SUBST(ICU_MINOR)
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

dnl ==================================================================
dnl Breakpad
dnl ==================================================================
AC_MSG_CHECKING([whether to enable breakpad])
if test "$enable_breakpad" != yes; then
    AC_MSG_RESULT([no])
else
    AC_MSG_RESULT([yes])
    ENABLE_BREAKPAD="TRUE"
    AC_DEFINE(ENABLE_BREAKPAD)
    AC_DEFINE(HAVE_FEATURE_BREAKPAD, 1)
    BUILD_TYPE="$BUILD_TYPE BREAKPAD"

    AC_MSG_CHECKING([for crashreport config])
    if test "$with_symbol_config" = "no"; then
        BREAKPAD_SYMBOL_CONFIG="invalid"
        AC_MSG_RESULT([no])
    else
        BREAKPAD_SYMBOL_CONFIG="$with_symbol_config"
        AC_DEFINE(BREAKPAD_SYMBOL_CONFIG)
        AC_MSG_RESULT([yes])
    fi
    AC_SUBST(BREAKPAD_SYMBOL_CONFIG)
fi
AC_SUBST(ENABLE_BREAKPAD)

dnl ===================================================================
dnl Orcus
dnl ===================================================================

AC_MSG_CHECKING([whether to enable orcus])
if test -z "$enable_orcus" -o "$enable_orcus" != no; then
    AC_MSG_RESULT([yes])
    ENABLE_ORCUS="TRUE"
    AC_DEFINE(ENABLE_ORCUS)

    libo_CHECK_SYSTEM_MODULE([orcus],[ORCUS],[liborcus-0.11 >= 0.11.2])
    if test "$with_system_orcus" != "yes"; then
        if test "$SYSTEM_BOOST" = "TRUE"; then
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

else
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_ORCUS)

dnl ===================================================================
dnl HarfBuzz
dnl ===================================================================
AC_MSG_CHECKING([whether to enable HarfBuzz support])
if test "$with_harfbuzz" = "yes" -o \( $_os != WINNT -a $_os != Darwin -a $_os != iOS \); then
    AC_MSG_RESULT([yes])
    ENABLE_HARFBUZZ="TRUE"
    if $PKG_CONFIG --atleast-version 0.9.18 harfbuzz; then
        libo_CHECK_SYSTEM_MODULE([harfbuzz],[HARFBUZZ],[harfbuzz-icu >= 0.9.18],["-I${WORKDIR}/UnpackedTarball/harfbuzz/src"],["-L${WORKDIR}/UnpackedTarball/harfbuzz/src/.libs -lharfbuzz -lharfbuzz-icu"])
    else
        libo_CHECK_SYSTEM_MODULE([harfbuzz],[HARFBUZZ],[harfbuzz >= 0.9.10],[-I${WORKDIR}/UnpackedTarball/harfbuzz/src],["-L${WORKDIR}/UnpackedTarball/harfbuzz/src/.libs -lharfbuzz -lharfbuzz-icu"])
    fi
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_HARFBUZZ)

AC_MSG_CHECKING([whether to use X11])
dnl ***************************************
dnl testing for X libraries and includes...
dnl ***************************************
if test "$USING_X11" = TRUE; then
    AC_DEFINE(HAVE_FEATURE_X11)
fi
AC_MSG_RESULT([$USING_X11])

if test "$USING_X11" = TRUE; then
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

if test "$USING_X11" = TRUE; then
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

    # vcl needs ICE and SM
    AC_CHECK_HEADERS(X11/ICE/ICElib.h,[],[AC_MSG_ERROR([libICE headers not found])])
    AC_CHECK_LIB([ICE], [IceConnectionNumber], [:],
        [AC_MSG_ERROR(ICE library not found)])
    AC_CHECK_HEADERS(X11/SM/SMlib.h,[],[AC_MSG_ERROR([libSM headers not found])])
    AC_CHECK_LIB([SM], [SmcOpenConnection], [:],
        [AC_MSG_ERROR(SM library not found)])
fi

dnl ===================================================================
dnl Check for system Xrender
dnl ===================================================================
AC_MSG_CHECKING([whether to use Xrender])
if test "$USING_X11" = TRUE -a  "$test_xrender" = "yes"; then
    AC_MSG_RESULT([yes])
    PKG_CHECK_MODULES(XRENDER, xrender)
    XRENDER_CFLAGS=$(printf '%s' "$XRENDER_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
    FilterLibs "${XRENDER_LIBS}"
    XRENDER_LIBS="${filteredlibs}"
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
if test "$USING_X11" = TRUE -a "$test_randr" = "yes" -a \( "$enable_randr" = "yes" -o "$enable_randr" = "TRUE" \); then
    AC_MSG_RESULT([yes])
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
    FilterLibs "${XRANDR_LIBS}"
    XRANDR_LIBS="${filteredlibs}"
else
    ENABLE_RANDR=""
    AC_MSG_RESULT([no])
fi
AC_SUBST(XRANDR_CFLAGS)
AC_SUBST(XRANDR_LIBS)
AC_SUBST(ENABLE_RANDR)

if test "$enable_neon" = "no" -o "$enable_mpl_subset" = "yes"; then
    WITH_WEBDAV="serf"
fi
if test $_os = iOS -o $_os = Android; then
    WITH_WEBDAV="no"
fi
AC_MSG_CHECKING([for webdav library])
case "$WITH_WEBDAV" in
serf)
    AC_MSG_RESULT([serf])
    # Check for system apr-util
    libo_CHECK_SYSTEM_MODULE([apr],[APR],[apr-util-1],
                             ["-I${WORKDIR}/UnpackedTarball/apr/include -I${WORKDIR}/UnpackedTarball/apr_util/include"],
                             ["-L${WORKDIR}/UnpackedTarball/apr/.libs -lapr-1 -L${WORKDIR}/UnpackedTarball/apr_util/.libs -laprutil-1"])
    if test "$COM" = "MSC"; then
        APR_LIB_DIR="LibR"
        test -n "${MSVC_USE_DEBUG_RUNTIME}" && APR_LIB_DIR="LibD"
        APR_LIBS="${WORKDIR}/UnpackedTarball/apr/${APR_LIB_DIR}/apr-1.lib ${WORKDIR}/UnpackedTarball/apr_util/${APR_LIB_DIR}/aprutil-1.lib"
    fi

    # Check for system serf
    libo_CHECK_SYSTEM_MODULE([serf],[SERF],[serf-1 >= 1.1.0],["-I${WORKDIR}/UnpackedTarball/serf"],
                             ["-L${WORKDIR}/UnpackedTarball/serf/.libs -lserf-1"])
    if test "$COM" = "MSC"; then
        SERF_LIB_DIR="Release"
        test -n "${MSVC_USE_DEBUG_RUNTIME}" && SERF_LIB_DIR="Debug"
        SERF_LIBS="${WORKDIR}/UnpackedTarball/serf/${SERF_LIB_DIR}/serf-1.lib"
    fi
    ;;
neon)
    AC_MSG_RESULT([neon])
    # Check for system neon
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
    ;;
*)
    AC_MSG_RESULT([none, disabled])
    WITH_WEBDAV=""
    ;;
esac
AC_SUBST(WITH_WEBDAV)

dnl ===================================================================
dnl Check for disabling cve_tests
dnl ===================================================================
AC_MSG_CHECKING([whether to execute CVE tests])
# If not explicitly enabled or disabled, default
if test -z "$enable_cve_tests"; then
    case "$OS" in
    WNT)
        # Default cves off for windows with its wild and wonderful
        # varienty of AV software kicking in and panicing
        enable_cve_tests=no
        ;;
    *)
        # otherwise yes
        enable_cve_tests=yes
        ;;
    esac
fi
if test "$enable_cve_tests" = "no"; then
    AC_MSG_RESULT([no])
    DISABLE_CVE_TESTS=TRUE
    AC_SUBST(DISABLE_CVE_TESTS)
else
    AC_MSG_RESULT([yes])
fi

dnl ===================================================================
dnl Check for enabling chart XShape tests
dnl ===================================================================
AC_MSG_CHECKING([whether to execute chart XShape tests])
if test "$enable_chart_tests" = "yes"; then
    AC_MSG_RESULT([yes])
    ENABLE_CHART_TESTS=TRUE
    AC_SUBST(ENABLE_CHART_TESTS)
else
    AC_MSG_RESULT([no])
fi

dnl ===================================================================
dnl Check for system openssl
dnl ===================================================================
DISABLE_OPENSSL=
AC_MSG_CHECKING([whether to disable OpenSSL usage])
if test "$enable_openssl" = "yes"; then
    AC_MSG_RESULT([no])
    if test "$_os" = Darwin ; then
        # OpenSSL is deprecated when building for 10.7 or later.
        #
        # http://stackoverflow.com/questions/7406946/why-is-apple-deprecating-openssl-in-macos-10-7-lion
        # http://stackoverflow.com/questions/7475914/libcrypto-deprecated-on-mac-os-x-10-7-lion

        with_system_openssl=no
        libo_CHECK_SYSTEM_MODULE([openssl],[OPENSSL],[openssl])
    elif test "$_os" = "NetBSD" -o "$_os" = "OpenBSD" -o "$_os" = "DragonFly" \
            && test "$with_system_openssl" != "no"; then
        with_system_openssl=yes
        SYSTEM_OPENSSL=TRUE
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
    DISABLE_OPENSSL=TRUE

    # warn that although OpenSSL is disabled, system libraries may be depending on it
    AC_MSG_WARN([OpenSSL has been disabled. No code compiled by LibO will make use of it but system librares may create indirect dependencies])
    add_warning "OpenSSL has been disabled. No code compiled by LibO will make use of it but system librares may create indirect dependencies"
fi

AC_SUBST([DISABLE_OPENSSL])

dnl ===================================================================
dnl Check for building gnutls
dnl ===================================================================
AC_MSG_CHECKING([whether to use gnutls])
if test "$WITH_WEBDAV" = "neon" -a "$with_system_neon" = no -a "$enable_openssl" = "no"; then
    AC_MSG_RESULT([yes])
    AM_PATH_LIBGCRYPT()
    PKG_CHECK_MODULES(GNUTLS, [gnutls],,
        AC_MSG_ERROR([[Disabling OpenSSL was requested, but GNUTLS is not
                      available in the system to use as replacement.]]))
    FilterLibs "${LIBGCRYPT_LIBS}"
    LIBGCRYPT_LIBS="${filteredlibs}"
else
    AC_MSG_RESULT([no])
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
            [AC_MSG_ERROR(librdf too old. Need >= 1.0.16)], [])
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
dnl Check for system hunspell
dnl ===================================================================
AC_MSG_CHECKING([which libhunspell to use])
if test "$with_system_hunspell" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_HUNSPELL=TRUE
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
    libo_MINGW_CHECK_DLL([libhunspell-1.4])
    HUNSPELL_CFLAGS=$(printf '%s' "$HUNSPELL_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
    FilterLibs "${HUNSPELL_LIBS}"
    HUNSPELL_LIBS="${filteredlibs}"
else
    AC_MSG_RESULT([internal])
    SYSTEM_HUNSPELL=
    HUNSPELL_CFLAGS="-I${WORKDIR}/UnpackedTarball/hunspell/src/hunspell"
    if test "$COM" = "MSC"; then
        HUNSPELL_LIBS="${WORKDIR}/LinkTarget/StaticLibrary/hunspell.lib"
    else
        HUNSPELL_LIBS="-L${WORKDIR}/UnpackedTarball/hunspell/src/hunspell/.libs -lhunspell-1.4"
    fi
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
    SYSTEM_HYPH=TRUE
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
    SYSTEM_HYPH=
    BUILD_TYPE="$BUILD_TYPE HYPHEN"
    if test "$COM" = "MSC"; then
        HYPHEN_LIB="${WORKDIR}/LinkTarget/StaticLibrary/hyphen.lib"
    else
        HYPHEN_LIB="-L${WORKDIR}/UnpackedTarball/hyphen/.libs -lhyphen"
    fi
fi
AC_SUBST(SYSTEM_HYPH)
AC_SUBST(HYPHEN_LIB)

dnl ===================================================================
dnl Checking for mythes
dnl ===================================================================
AC_MSG_CHECKING([which mythes to use])
if test "$with_system_mythes" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_MYTHES=TRUE
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
    FilterLibs "${MYTHES_LIBS}"
    MYTHES_LIBS="${filteredlibs}"
else
    AC_MSG_RESULT([internal])
    SYSTEM_MYTHES=
    BUILD_TYPE="$BUILD_TYPE MYTHES"
    if test "$COM" = "MSC"; then
        MYTHES_LIBS="${WORKDIR}/LinkTarget/StaticLibrary/mythes.lib"
    else
        MYTHES_LIBS="-L${WORKDIR}/UnpackedTarball/mythes/.libs -lmythes-1.2"
    fi
fi
AC_SUBST(SYSTEM_MYTHES)
AC_SUBST(MYTHES_CFLAGS)
AC_SUBST(MYTHES_LIBS)

dnl ===================================================================
dnl How should we build the linear programming solver ?
dnl ===================================================================

ENABLE_COINMP=
AC_MSG_CHECKING([whether to build with CoinMP])
if test "$enable_coinmp" != "no"; then
    ENABLE_COINMP=TRUE
    AC_MSG_RESULT([yes])
    if test "$with_system_coinmp" = "yes"; then
        SYSTEM_COINMP=TRUE
        PKG_CHECK_MODULES( COINMP, coinmp )
        FilterLibs "${COINMP_LIBS}"
        COINMP_LIBS="${filteredlibs}"
    else
        BUILD_TYPE="$BUILD_TYPE COINMP"
    fi
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_COINMP)
AC_SUBST(SYSTEM_COINMP)
AC_SUBST(COINMP_CFLAGS)
AC_SUBST(COINMP_LIBS)

ENABLE_LPSOLVE=
AC_MSG_CHECKING([whether to build with lpsolve])
if test "$enable_lpsolve" != "no"; then
    ENABLE_LPSOLVE=TRUE
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_LPSOLVE)

if test "$ENABLE_LPSOLVE" = TRUE; then
    AC_MSG_CHECKING([which lpsolve to use])
    if test "$with_system_lpsolve" = "yes"; then
        AC_MSG_RESULT([external])
        SYSTEM_LPSOLVE=TRUE
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
        SYSTEM_LPSOLVE=
        BUILD_TYPE="$BUILD_TYPE LPSOLVE"
    fi
fi
AC_SUBST(SYSTEM_LPSOLVE)

dnl ===================================================================
dnl Checking for libexttextcat
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([libexttextcat],[LIBEXTTEXTCAT],[libexttextcat >= 3.4.1])
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

        WINDOWS_SDK_LIB_SUBDIR=$winsdklibsubdir
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
        elif test -f "$WINDOWS_SDK_HOME/Include/$winsdklibsubdir/um/adoint.h" \
             -a -f "$WINDOWS_SDK_HOME/Include/$winsdklibsubdir/um/SqlUcode.h" \
             -a -f "$WINDOWS_SDK_HOME/Include/$winsdklibsubdir/um/usp10.h"; then
            have_windows_sdk_headers=yes
        else
            have_windows_sdk_headers=no
        fi

        if test -f "$WINDOWS_SDK_HOME/lib/user32.lib"; then
            have_windows_sdk_libs=yes
        elif test -f "$WINDOWS_SDK_HOME/lib/$winsdklibsubdir/um/$WINDOWS_SDK_ARCH/user32.lib"; then
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
    elif echo $WINDOWS_SDK_HOME | grep "v7.1" >/dev/null 2>/dev/null; then
        WINDOWS_SDK_VERSION=70
        AC_MSG_RESULT([found Windows SDK 7 ($WINDOWS_SDK_HOME)])
    elif echo $WINDOWS_SDK_HOME | grep "8.0" >/dev/null 2>/dev/null; then
        WINDOWS_SDK_VERSION=80
        AC_MSG_RESULT([found Windows SDK 8.0 ($WINDOWS_SDK_HOME)])
        dnl compatibility warning if not explicitly choosing the 80 SDK:
        if test -z "$with_windows_sdk"; then
            AC_MSG_WARN([If a build should run on Windows XP,])
            AC_MSG_WARN([use --with-windows-sdk=7.1A])
            add_warning "If a build should run on Windows XP,"
            add_warning "use --with-windows-sdk=7.1A"
        fi
    elif echo $WINDOWS_SDK_HOME | grep "8.1" >/dev/null 2>/dev/null; then
        WINDOWS_SDK_VERSION=81
        AC_MSG_RESULT([found Windows SDK 8.1 ($WINDOWS_SDK_HOME)])
    elif echo $WINDOWS_SDK_HOME | grep "/10" >/dev/null 2>/dev/null; then
        WINDOWS_SDK_VERSION=10
        AC_MSG_RESULT([found Windows SDK 10.0 ($WINDOWS_SDK_HOME)])
    else
        AC_MSG_ERROR([Found legacy Windows Platform SDK ($WINDOWS_SDK_HOME)])
    fi
    PathFormat "$WINDOWS_SDK_HOME"
    WINDOWS_SDK_HOME="$formatted_path"
    if test "$build_os" = "cygwin"; then
        SOLARINC="$SOLARINC -I$WINDOWS_SDK_HOME/include -I$COMPATH/include"
        if test -d "$WINDOWS_SDK_HOME/include/um"; then
            SOLARINC="$SOLARINC -I$WINDOWS_SDK_HOME/include/um -I$WINDOWS_SDK_HOME/include/shared"
        elif test -d "$WINDOWS_SDK_HOME/Include/$winsdklibsubdir/um"; then
            SOLARINC="$SOLARINC -I$WINDOWS_SDK_HOME/Include/$winsdklibsubdir/um -I$WINDOWS_SDK_HOME/Include/$winsdklibsubdir/shared"
        fi
    fi

    dnl TODO: solenv/bin/modules/installer/windows/msiglobal.pm wants to use a
    dnl WiLangId.vbs that is included only in some SDKs (e.g., included in v7.1
    dnl but not in v8.0), so allow this to be overridden with a
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
            WINDOWS_SDK_WILANGID=$(cygpath -sm "C:/Program Files (x86)/Windows Kits/8.1/bin/$WINDOWS_SDK_ARCH/WiLangId.vbs")
        fi
        if ! test -e "$WINDOWS_SDK_WILANGID" ; then
            AC_MSG_WARN([WiLangId.vbs not found - building translated packages will fail])
            add_warning "WiLangId.vbs not found - building translated packages will fail"
        fi
    fi
fi
AC_SUBST(WINDOWS_SDK_HOME)
AC_SUBST(WINDOWS_SDK_LIB_SUBDIR)
AC_SUBST(WINDOWS_SDK_VERSION)
AC_SUBST(WINDOWS_SDK_WILANGID)

dnl =========================================
dnl Check for uuidgen
dnl =========================================
if test "$_os" = "WINNT" -a "$cross_compiling" != "yes"; then
    if test "$WITH_MINGW" = "yes"; then
        AC_PATH_PROG([UUIDGEN], [uuid])
        if test -z "$UUIDGEN"; then
            AC_MSG_WARN([uuid is needed for building installation sets])
        fi
    else
        # presence is already tested above in the WINDOWS_SDK_HOME check
        UUIDGEN=uuidgen.exe
        AC_SUBST(UUIDGEN)
    fi
else
    AC_PATH_PROG([UUIDGEN], [uuidgen])
    if test -z "$UUIDGEN"; then
        AC_MSG_WARN([uuid is needed for building installation sets])
    fi
fi

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
if test "$GNUMAKE_WIN_NATIVE" = "TRUE" ; then
    FLEX=`cygpath -m $FLEX`
fi
if test -z "$FLEX"; then
    AC_MSG_ERROR([no flex found in \$PATH, install it])
else
    AC_MSG_CHECKING([the flex version])
    _flex_version=$($FLEX --version | $SED -e 's/^.*\([[[:digit:]]]\{1,\}\.[[[:digit:]]]\{1,\}\.[[[:digit:]]]\{1,\}\).*$/\1/')
    if test $(echo $_flex_version | $AWK -F. '{printf("%d%03d%03d", $1, $2, $3)}') -lt 2005035; then
        AC_MSG_ERROR([failed ($FLEX $_flex_version found, but need at least 2.5.35)])
    fi
fi
AC_SUBST([FLEX])
dnl ***************************************
dnl Checking for patch
dnl ***************************************
AC_PATH_PROG(PATCH, patch)
if test -z "$PATCH"; then
    AC_MSG_ERROR(["patch" not found in \$PATH, install it])
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

if test "$GNUMAKE_WIN_NATIVE" = "TRUE" ; then
    GNUPATCH=`cygpath -m $GNUPATCH`
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

if test "$GNUMAKE_WIN_NATIVE" = "TRUE" ; then
    GNUCP=`cygpath -m $GNUCP`
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
        if test "$vcexpress" = "Express"; then
            assembler=ml64.exe
            assembler_bin=bin/x86_amd64
        else
            assembler=ml64.exe
            assembler_bin=bin/amd64
        fi
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
dnl We need touch with -h option support.
dnl ===================================================================
AC_PATH_PROG(TOUCH, touch)
test -z "$TOUCH" && AC_MSG_ERROR([touch is required])
touch warn
if ! "$TOUCH" -h warn 2>/dev/null > /dev/null; then
    AC_MSG_ERROR([touch version with -h option support is required to build, please install it and make sure it is the one found first in PATH],,)
fi

dnl ===================================================================
dnl Set vcl option: coordinate device in double or sal_Int32
dnl ===================================================================

AC_MSG_CHECKING([Type to use for Device Pixel coordinates])
if test "$_os" = "Darwin" -o  $_os = iOS ; then
    AC_DEFINE(VCL_FLOAT_DEVICE_PIXEL)
    AC_MSG_RESULT([double])
else
    AC_MSG_RESULT([sal_Int32])
fi

dnl ===================================================================
dnl Test which vclplugs have to be built.
dnl ===================================================================
R=""
if test "$USING_X11" != TRUE; then
    enable_gtk=no
    enable_gtk3=no
fi
GTK3_CFLAGS=""
GTK3_LIBS=""
ENABLE_GTK3=""
if test "x$enable_gtk3" = "xyes"; then
    if test "$with_system_cairo" = no; then
        AC_MSG_ERROR([System cairo required for gtk3 support, do not combine --enable-gtk3 with --without-system-cairo])
    fi
    : ${with_system_cairo:=yes}
    PKG_CHECK_MODULES(GTK3, gtk+-3.0 >= 3.8 gtk+-unix-print-3.0 gmodule-no-export-2.0 glib-2.0 >= 2.38 cairo, ENABLE_GTK3="TRUE", ENABLE_GTK3="")
    if test "x$ENABLE_GTK3" = "xTRUE"; then
        R="gtk3"
        dnl Avoid installed by unpackaged files for now.
        if test -z "$PKGFORMAT"; then
            GOBJECT_INTROSPECTION_CHECK(INTROSPECTION_REQUIRED_VERSION)
        fi
    else
        AC_MSG_ERROR([gtk3 or dependent libraries of the correct versions, not found])
    fi
    GTK3_CFLAGS=$(printf '%s' "$GTK3_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
    FilterLibs "${GTK3_LIBS}"
    GTK3_LIBS="${filteredlibs}"
fi
AC_SUBST(GTK3_LIBS)
AC_SUBST(GTK3_CFLAGS)
AC_SUBST(ENABLE_GTK3)

ENABLE_GTK=""
if test "x$enable_gtk" = "xyes"; then
    if test "$with_system_cairo" = no; then
        AC_MSG_ERROR([System cairo required for gtk support, do not use --without-system-cairo or use --disable-gtk])
    fi
    : ${with_system_cairo:=yes}
    ENABLE_GTK="TRUE"
    AC_DEFINE(ENABLE_GTK)
    R="gtk $R"
fi
AC_SUBST(ENABLE_GTK)

ENABLE_TDE=""
if test "x$enable_tde" = "xyes"; then
    ENABLE_TDE="TRUE"
    AC_DEFINE(ENABLE_TDE)
    R="$R tde"
fi
AC_SUBST(ENABLE_TDE)

ENABLE_KDE4=""
if test "x$enable_kde4" = "xyes"; then
    ENABLE_KDE4="TRUE"
    AC_DEFINE(ENABLE_KDE4)
    R="$R kde4"
fi
AC_SUBST(ENABLE_KDE4)


build_vcl_plugins="$R"
if test -z "$build_vcl_plugins"; then
    build_vcl_plugins="none"
fi
AC_MSG_NOTICE([VCLplugs to be built: $build_vcl_plugins])

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
    AC_DEFINE(ENABLE_DBUS)
    DBUS_CFLAGS=$(printf '%s' "$DBUS_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
    FilterLibs "${DBUS_LIBS}"
    DBUS_LIBS="${filteredlibs}"
else
    AC_MSG_RESULT([no])
fi

AC_SUBST(ENABLE_DBUS)
AC_SUBST(DBUS_CFLAGS)
AC_SUBST(DBUS_LIBS)

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
                    SYSTEM_BLUEZ=TRUE
                else
                    AC_MSG_RESULT([internal])
                    SYSTEM_BLUEZ=
                fi
            else
                AC_MSG_RESULT([no, dbus disabled])
                ENABLE_SDREMOTE_BLUETOOTH=
                SYSTEM_BLUEZ=
            fi
        else
            AC_MSG_RESULT([yes])
            ENABLE_SDREMOTE_BLUETOOTH=TRUE
            SYSTEM_BLUEZ=
        fi
    else
        AC_MSG_RESULT([no])
        ENABLE_SDREMOTE_BLUETOOTH=
        SYSTEM_BLUEZ=
    fi
else
    ENABLE_SDREMOTE=
    SYSTEM_BLUEZ=
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_SDREMOTE)
AC_SUBST(ENABLE_SDREMOTE_BLUETOOTH)
AC_SUBST(SYSTEM_BLUEZ)

dnl ===================================================================
dnl Check whether the gtk 2.0 libraries are available.
dnl ===================================================================

GTK_CFLAGS=""
GTK_LIBS=""
ENABLE_SYSTRAY_GTK=""
if test  "$test_gtk" = "yes"; then

    if test "$ENABLE_GTK" = "TRUE"; then
        PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.18.0 gdk-pixbuf-2.0 >= 2.2 ,,AC_MSG_ERROR([requirements to build the gtk-plugin not met. Use --disable-gtk or install the missing packages]))
        GTK_CFLAGS=$(printf '%s' "$GTK_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
        FilterLibs "${GTK_LIBS}"
        GTK_LIBS="${filteredlibs}"
        PKG_CHECK_MODULES(GTHREAD, gthread-2.0,,AC_MSG_ERROR([requirements to build the gtk-plugin not met. Use --disable-gtk or install the missing packages]))
        BUILD_TYPE="$BUILD_TYPE GTK"
        GTHREAD_CFLAGS=$(printf '%s' "$GTK_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
        FilterLibs "${GTHREAD_LIBS}"
        GTHREAD_LIBS="${filteredlibs}"

        if test "x$enable_systray" = "xyes"; then
            ENABLE_SYSTRAY_GTK="TRUE"
        fi

        AC_MSG_CHECKING([whether to enable Gtk print dialog support])
        PKG_CHECK_MODULES([GTK_PRINT], [gtk+-unix-print-2.0 >= 2.10.0],
                          [ENABLE_GTK_PRINT="TRUE"],
                          [ENABLE_GTK_PRINT=""])
        GTK_PRINT_CFLAGS=$(printf '%s' "$GTK_PRINT_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
        FilterLibs "${GTK_PRINT_LIBS}"
        GTK_PRINT_LIBS="${filteredlibs}"

        AC_MSG_CHECKING([whether to enable GIO support])
        if test "$_os" != "WINNT" -a "$_os" != "Darwin" -a "$enable_gio" = "yes"; then
            dnl Need at least 2.26 for the dbus support.
            PKG_CHECK_MODULES([GIO], [gio-2.0 >= 2.26],
                              [ENABLE_GIO="TRUE"], [ENABLE_GIO=""])
            if test "$ENABLE_GIO" = "TRUE"; then
                AC_DEFINE(ENABLE_GIO)
                GIO_CFLAGS=$(printf '%s' "$GIO_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
                FilterLibs "${GIO_LIBS}"
                GIO_LIBS="${filteredlibs}"
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
    PKG_CHECK_MODULES( TELEPATHY, telepathy-glib >= 0.18.0 glib-2.0 gobject-2.0 gthread-2.0 gio-2.0 gtk+-2.0 >= 2.18.0 )
    TELEPATHY_CFLAGS=$(printf '%s' "$TELEPATHY_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
    FilterLibs "${TELEPATHY_LIBS}"
    TELEPATHY_LIBS="${filteredlibs}"
else
    AC_MSG_RESULT([no])
fi

AC_SUBST(ENABLE_TELEPATHY)
AC_SUBST(TELEPATHY_CFLAGS)
AC_SUBST(TELEPATHY_LIBS)


dnl ===================================================================

SPLIT_APP_MODULES=""
if test "$enable_split_app_modules" = "yes"; then
    SPLIT_APP_MODULES="TRUE"
fi
AC_SUBST(SPLIT_APP_MODULES)

SPLIT_OPT_FEATURES=""
if test "$enable_split_opt_features" = "yes"; then
    SPLIT_OPT_FEATURES="TRUE"
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
    AC_DEFINE(ENABLE_CAIRO_CANVAS)
fi
AC_SUBST(ENABLE_CAIRO_CANVAS)

dnl ===================================================================
dnl Check whether the GStreamer libraries are available.
dnl It's possible to build avmedia with both GStreamer backends!
dnl ===================================================================

ENABLE_GSTREAMER_1_0=""

if test "$build_gstreamer_1_0" = "yes"; then

    AC_MSG_CHECKING([whether to enable the new GStreamer 1.0 avmedia backend])
    if test "$enable_avmedia" = yes -a "$enable_gstreamer_1_0" != no; then
        ENABLE_GSTREAMER_1_0="TRUE"
        AC_MSG_RESULT([yes])
        PKG_CHECK_MODULES( [GSTREAMER_1_0], [gstreamer-1.0 gstreamer-plugins-base-1.0 gstreamer-pbutils-1.0 gstreamer-video-1.0] )
        GSTREAMER_1_0_CFLAGS=$(printf '%s' "$GSTREAMER_1_0_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
        FilterLibs "${GSTREAMER_1_0_LIBS}"
        GSTREAMER_1_0_LIBS="${filteredlibs}"
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_SUBST(GSTREAMER_1_0_CFLAGS)
AC_SUBST(GSTREAMER_1_0_LIBS)
AC_SUBST(ENABLE_GSTREAMER_1_0)


ENABLE_GSTREAMER_0_10=""
if test "$build_gstreamer_0_10" = "yes"; then

    AC_MSG_CHECKING([whether to enable the GStreamer 0.10 avmedia backend])
    if test "$enable_avmedia" = yes -a "$enable_gstreamer_0_10" != no; then
        ENABLE_GSTREAMER_0_10="TRUE"
        AC_MSG_RESULT([yes])
        PKG_CHECK_MODULES( [GSTREAMER_0_10], [gstreamer-0.10 gstreamer-plugins-base-0.10 gstreamer-pbutils-0.10 gstreamer-interfaces-0.10],, [
            PKG_CHECK_MODULES( [GSTREAMER_0_10], [gstreamer-0.10 gstreamer-plugins-base-0.10 gstreamer-pbutils-0.10] )
        ])
        GSTREAMER_0_10_CFLAGS=$(printf '%s' "$GSTREAMER_0_10_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
        FilterLibs "${GSTREAMER_0_10_LIBS}"
        GSTREAMER_0_10_LIBS="${filteredlibs}"
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
if test "$enable_avmedia" = yes -a $_os != iOS -a $_os != Android -a "$enable_vlc" = yes; then
    ENABLE_VLC="TRUE"
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_VLC)

# this is for opengl transitions in impress slideshow
ENABLE_OPENGL=
if test $_os = iOS -o $_os = Android; then
   : # disable
elif test "$_os" = "Darwin"; then
    # We use frameworks on Mac OS X, no need for detail checks
    ENABLE_OPENGL=TRUE
    AC_DEFINE(HAVE_FEATURE_OPENGL,1)
    ENABLE_OPENGL_CANVAS=TRUE
elif test $_os = WINNT; then
    ENABLE_OPENGL=TRUE
    AC_DEFINE(HAVE_FEATURE_OPENGL,1)
    ENABLE_OPENGL_CANVAS=TRUE
else
    if test "$USING_X11" = TRUE; then
        AC_CHECK_LIB(GL, glBegin, [:], AC_MSG_ERROR([libGL required.]))
        ENABLE_OPENGL=TRUE
        AC_DEFINE(HAVE_FEATURE_OPENGL,1)
        ENABLE_OPENGL_CANVAS=TRUE
    fi
fi

AC_SUBST(ENABLE_OPENGL)
AC_SUBST(ENABLE_OPENGL_CANVAS)

dnl =================================================
dnl Check whether to build with OpenCL support.
dnl =================================================

if test $_os != iOS -a $_os != Android; then
    # OPENCL in BUILD_TYPE tells that OpenCL is potentially available on the platform (optional at run-time,
    # used through clew).
    BUILD_TYPE="$BUILD_TYPE OPENCL"
    AC_DEFINE(HAVE_FEATURE_OPENCL)
fi

dnl ===================================================================
dnl Check whether to enable glTF support
dnl ===================================================================
AC_MSG_CHECKING([whether to enable glTF support])
ENABLE_GLTF=
if test "x$enable_gltf" != "xno" -a $_os != iOS -a $_os != Android -a "$ENABLE_HEADLESS" = ""; then
    ENABLE_GLTF=TRUE
    AC_MSG_RESULT([yes])
    AC_DEFINE(HAVE_FEATURE_GLTF,1)
    if test "$with_system_libgltf" = "yes"; then
        SYSTEM_LIBGLTF=TRUE
        PKG_CHECK_MODULES( LIBGLTF, [libgltf-0.0 >= 0.0.1] )
        FilterLibs "${LIBGLTF_LIBS}"
        LIBGLTF_LIBS="${filteredlibs}"
    else
        BUILD_TYPE="$BUILD_TYPE LIBGLTF"
    fi
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_GLTF)
AC_SUBST(SYSTEM_LIBGLTF)
AC_SUBST(LIBGLTF_CFLAGS)
AC_SUBST(LIBGLTF_LIBS)

dnl ===================================================================
dnl Check whether to enable COLLADA support
dnl ===================================================================
AC_MSG_CHECKING([whether to enable COLLADA support])
ENABLE_COLLADA=
if test "$enable_collada" != "no" -a "$ENABLE_GLTF" = "TRUE"; then
    AC_MSG_RESULT([yes])
    ENABLE_COLLADA=TRUE
    AC_DEFINE(HAVE_FEATURE_COLLADA,1)

    AC_MSG_CHECKING([which OPENCOLLADA to use])
    if test "$with_system_opencollada" = "yes"; then
        AC_MSG_RESULT([external])
        SYSTEM_OPENCOLLADA=TRUE
        AS_IF([test -n "$OPENCOLLADA_CFLAGS"],[],[AC_MSG_ERROR([export OPENCOLLADA_CFLAGS])])
        AS_IF([test -n "$OPENCOLLADA_LIBS"],[],[AC_MSG_ERROR([export OPENCOLLADA_LIBS])])
        AC_LANG_PUSH([C++])
        save_CXXFLAGS=$CXXFLAGS
        save_CPPFLAGS=$CPPFLAGS
        CXXFLAGS="$CXXFLAGS $OPENCOLLADA_CFLAGS"
        CPPFLAGS="$CPPFLAGS $OPENCOLLADA_CFLAGS"
        AC_CHECK_HEADERS(
                COLLADABU.h \
                COLLADAFW.h \
                COLLADASaxFWLLoader.h \
                GeneratedSaxParser.h,
            [],
            [AC_MSG_ERROR([openCOLLADA headers not found. Install openCOLLADA])],
            [])
        CXXFLAGS=$save_CXXFLAGS
        CPPFLAGS=$save_CPPFLAGS
        AC_LANG_POP([C++])
        OPENCOLLADA_CFLAGS=$(printf '%s' "$OPENCOLLADA_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
        FilterLibs "${OPENCOLLADA_LIBS}"
        OPENCOLLADA_LIBS="${filteredlibs}"
    else
        AC_MSG_RESULT([internal])
        BUILD_TYPE="$BUILD_TYPE OPENCOLLADA"
    fi

    AC_MSG_CHECKING([which collada2gltf to use])
    if test "$with_system_collada2gltf" = "yes"; then
        if test "$with_system_opencollada" = "no"; then
            AC_MSG_ERROR([the combination of system collada2gltf and internal openCOLLADA is not allowed])
        fi
        AC_MSG_RESULT([external])
        SYSTEM_COLLADA2GLTF=TRUE
        AS_IF([test -n "$COLLADA2GLTF_CFLAGS"],[],[AC_MSG_ERROR([export COLLADA2GLTF_CFLAGS])])
        AS_IF([test -n "$COLLADA2GLTF_LIBS"],[],[AC_MSG_ERROR([export COLLADA2GLTF_LIBS])])
        AC_LANG_PUSH([C++])
        save_CXXFLAGS=$CXXFLAGS
        save_CPPFLAGS=$CPPFLAGS
        CXXFLAGS="$CXXFLAGS $COLLADA2GLTF_CFLAGS $OPENCOLLADA_CFLAGS $CXXFLAGS_CXX11"
        CPPFLAGS="$CPPFLAGS $COLLADA2GLTF_CFLAGS $OPENCOLLADA_CFLAGS $CXXFLAGS_CXX11"
        AC_CHECK_HEADERS(
                GLTF.h \
                encodingHelpers.h,
            [],
            [AC_MSG_ERROR([collada2gltf headers not found. Install collada2gltf])],
            [])
        CXXFLAGS=$save_CXXFLAGS
        CPPFLAGS=$save_CPPFLAGS
        AC_LANG_POP([C++])
        COLLADA2GLTF_CFLAGS=$(printf '%s' "$COLLADA2GLTF_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
        FilterLibs "${COLLADA2GLTF_LIBS}"
        COLLADA2GLTF_LIBS="${filteredlibs}"
    else
        AC_MSG_RESULT([internal])
        BUILD_TYPE="$BUILD_TYPE COLLADA2GLTF"
    fi
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_COLLADA)
AC_SUBST([OPENCOLLADA_CFLAGS])
AC_SUBST([OPENCOLLADA_LIBS])
AC_SUBST([SYSTEM_OPENCOLLADA])

AC_SUBST([COLLADA2GLTF_CFLAGS])
AC_SUBST([COLLADA2GLTF_LIBS])
AC_SUBST([SYSTEM_COLLADA2GLTF])

if test "$enable_dconf" != no; then
    PKG_CHECK_MODULES([DCONF], [dconf >= 0.15.2], [], [
        if test "$enable_dconf" = yes; then
            AC_MSG_ERROR([dconf not found])
        else
            enable_dconf=no
        fi])
fi
AC_MSG_CHECKING([whether to enable dconf])
if test "$enable_dconf" = no; then
    DCONF_CFLAGS=
    DCONF_LIBS=
    ENABLE_DCONF=
    AC_MSG_RESULT([no])
else
    ENABLE_DCONF=TRUE
    AC_DEFINE(ENABLE_DCONF)
    AC_MSG_RESULT([yes])
fi
AC_SUBST([DCONF_CFLAGS])
AC_SUBST([DCONF_LIBS])
AC_SUBST([ENABLE_DCONF])

# pdf import?
AC_MSG_CHECKING([whether to build the PDF import feature])
ENABLE_PDFIMPORT=
if test $_os != Android -a $_os != iOS -a \( -z "$enable_pdfimport" -o "$enable_pdfimport" = yes \); then
    AC_MSG_RESULT([yes])
    ENABLE_PDFIMPORT=TRUE

    dnl ===================================================================
    dnl Check for system poppler
    dnl ===================================================================
    AC_MSG_CHECKING([which PDF import backend to use])
    if test "$with_system_poppler" = "yes"; then
        AC_MSG_RESULT([external])
        SYSTEM_POPPLER=TRUE
        PKG_CHECK_MODULES( POPPLER, poppler >= 0.12.0 )
        AC_LANG_PUSH([C++])
        save_CXXFLAGS=$CXXFLAGS
        save_CPPFLAGS=$CPPFLAGS
        CXXFLAGS="$CXXFLAGS $POPPLER_CFLAGS"
        CPPFLAGS="$CPPFLAGS $POPPLER_CFLAGS"
        AC_CHECK_HEADER([cpp/poppler-version.h],
            [AC_DEFINE([HAVE_POPPLER_VERSION_H], 1)],
            [])
        CXXFLAGS=$save_CXXFLAGS
        CPPFLAGS=$save_CPPFLAGS
        AC_LANG_POP([C++])
        libo_MINGW_CHECK_DLL([libpoppler])
        POPPLER_CFLAGS=$(printf '%s' "$POPPLER_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")

        FilterLibs "${POPPLER_LIBS}"
        POPPLER_LIBS="${filteredlibs}"
    else
        AC_MSG_RESULT([internal])
        SYSTEM_POPPLER=
        BUILD_TYPE="$BUILD_TYPE POPPLER"
        AC_DEFINE([HAVE_POPPLER_VERSION_H], 1)
    fi
    AC_DEFINE([ENABLE_PDFIMPORT],1)
else
    AC_MSG_RESULT([no])
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

AC_MSG_CHECKING([whether to build the Report Builder])
if test "$enable_report_builder" != "no" -a "$with_java" != "no"; then
    AC_MSG_RESULT([yes])
    ENABLE_REPORTBUILDER=TRUE
    AC_MSG_CHECKING([which jfreereport libs to use])
    if test "$with_system_jfreereport" = "yes"; then
        SYSTEM_JFREEREPORT=TRUE
        AC_MSG_RESULT([external])
        if test -z $SAC_JAR; then
            SAC_JAR=/usr/share/java/sac.jar
        fi
        if ! test -f $SAC_JAR; then
             AC_MSG_ERROR(sac.jar not found.)
        fi

        if test -z $LIBXML_JAR; then
            if test -f /usr/share/java/libxml-1.0.0.jar; then
                LIBXML_JAR=/usr/share/java/libxml-1.0.0.jar
            elif test -f /usr/share/java/libxml.jar; then
                LIBXML_JAR=/usr/share/java/libxml.jar
            else
                AC_MSG_ERROR(libxml.jar replacement not found.)
            fi
        elif ! test -f $LIBXML_JAR; then
            AC_MSG_ERROR(libxml.jar not found.)
        fi

        if test -z $FLUTE_JAR; then
            if test -f/usr/share/java/flute-1.3.0.jar; then
                FLUTE_JAR=/usr/share/java/flute-1.3.0.jar
            elif test -f /usr/share/java/flute.jar; then
                FLUTE_JAR=/usr/share/java/flute.jar
            else
                AC_MSG_ERROR(flute-1.3.0.jar replacement not found.)
            fi
        elif ! test -f $FLUTE_JAR; then
            AC_MSG_ERROR(flute-1.3.0.jar not found.)
        fi

        if test -z $JFREEREPORT_JAR; then
            if test -f /usr/share/java/flow-engine-0.9.2.jar; then
                JFREEREPORT_JAR=/usr/share/java/flow-engine-0.9.2.jar
            elif test -f /usr/share/java/flow-engine.jar; then
                JFREEREPORT_JAR=/usr/share/java/flow-engine.jar
            else
                AC_MSG_ERROR(jfreereport.jar replacement not found.)
            fi
        elif ! test -f  $JFREEREPORT_JAR; then
                AC_MSG_ERROR(jfreereport.jar not found.)
        fi

        if test -z $LIBLAYOUT_JAR; then
            if test -f /usr/share/java/liblayout-0.2.9.jar; then
                LIBLAYOUT_JAR=/usr/share/java/liblayout-0.2.9.jar
            elif test -f /usr/share/java/liblayout.jar; then
                LIBLAYOUT_JAR=/usr/share/java/liblayout.jar
            else
                AC_MSG_ERROR(liblayout.jar replacement not found.)
            fi
        elif ! test -f $LIBLAYOUT_JAR; then
                AC_MSG_ERROR(liblayout.jar not found.)
        fi

        if test -z $LIBLOADER_JAR; then
            if test -f /usr/share/java/libloader-1.0.0.jar; then
                LIBLOADER_JAR=/usr/share/java/libloader-1.0.0.jar
            elif test -f /usr/share/java/libloader.jar; then
                LIBLOADER_JAR=/usr/share/java/libloader.jar
            else
                AC_MSG_ERROR(libloader.jar replacement not found.)
            fi
        elif ! test -f  $LIBLOADER_JAR; then
            AC_MSG_ERROR(libloader.jar not found.)
        fi

        if test -z $LIBFORMULA_JAR; then
            if test -f /usr/share/java/libformula-0.2.0.jar; then
                LIBFORMULA_JAR=/usr/share/java/libformula-0.2.0.jar
            elif test -f /usr/share/java/libformula.jar; then
                LIBFORMULA_JAR=/usr/share/java/libformula.jar
            else
                AC_MSG_ERROR(libformula.jar replacement not found.)
            fi
        elif ! test -f $LIBFORMULA_JAR; then
                AC_MSG_ERROR(libformula.jar not found.)
        fi

        if test -z $LIBREPOSITORY_JAR; then
            if test -f /usr/share/java/librepository-1.0.0.jar; then
                LIBREPOSITORY_JAR=/usr/share/java/librepository-1.0.0.jar
            elif test -f /usr/share/java/librepository.jar; then
                LIBREPOSITORY_JAR=/usr/share/java/librepository.jar
            else
                AC_MSG_ERROR(librepository.jar replacement not found.)
            fi
        elif ! test -f $LIBREPOSITORY_JAR; then
            AC_MSG_ERROR(librepository.jar not found.)
        fi

        if test -z $LIBFONTS_JAR; then
            if test -f /usr/share/java/libfonts-1.0.0.jar; then
                LIBFONTS_JAR=/usr/share/java/libfonts-1.0.0.jar
            elif test -f /usr/share/java/libfonts.jar; then
                LIBFONTS_JAR=/usr/share/java/libfonts.jar
            else
                AC_MSG_ERROR(libfonts.jar replacement not found.)
            fi
        elif ! test -f $LIBFONTS_JAR; then
                AC_MSG_ERROR(libfonts.jar not found.)
        fi

        if test -z $LIBSERIALIZER_JAR; then
            if test -f /usr/share/java/libserializer-1.0.0.jar; then
                LIBSERIALIZER_JAR=/usr/share/java/libserializer-1.0.0.jar
            elif test -f /usr/share/java/libserializer.jar; then
                LIBSERIALIZER_JAR=/usr/share/java/libserializer.jar
            else
                AC_MSG_ERROR(libserializer.jar replacement not found.)
            fi
        elif ! test -f $LIBSERIALIZER_JAR; then
                AC_MSG_ERROR(libserializer.jar not found.)
        fi

        if test -z $LIBBASE_JAR; then
            if test -f /usr/share/java/libbase-1.0.0.jar; then
                LIBBASE_JAR=/usr/share/java/libbase-1.0.0.jar
            elif test -f /usr/share/java/libbase.jar; then
                LIBBASE_JAR=/usr/share/java/libbase.jar
            else
                AC_MSG_ERROR(libbase.jar replacement not found.)
            fi
        elif ! test -f $LIBBASE_JAR; then
            AC_MSG_ERROR(libbase.jar not found.)
        fi

    else
        AC_MSG_RESULT([internal])
        SYSTEM_JFREEREPORT=
        BUILD_TYPE="$BUILD_TYPE JFREEREPORT"
    fi
else
    AC_MSG_RESULT([no])
    ENABLE_REPORTBUILDER=
    SYSTEM_JFREEREPORT=
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
COMMONS_LOGGING_VERSION=1.2
if test "$ENABLE_REPORTBUILDER" = "TRUE"; then
    AC_MSG_CHECKING([which Apache commons-* libs to use])
    if test "$with_system_apache_commons" = "yes"; then
        SYSTEM_APACHE_COMMONS=TRUE
        AC_MSG_RESULT([external])
        if test -z $COMMONS_LOGGING_JAR; then
            if test -f /usr/share/java/commons-logging-${COMMONS_LOGGING_VERSION}.jar; then
               COMMONS_LOGGING_JAR=/usr/share/java/commons-logging-${COMMONS_LOGGING_VERSION}.jar
           elif test -f /usr/share/java/commons-logging.jar; then
                COMMONS_LOGGING_JAR=/usr/share/java/commons-logging.jar
            else
                AC_MSG_ERROR(commons-logging.jar replacement not found.)
            fi
        elif ! test -f $COMMONS_LOGGING_JAR; then
            AC_MSG_ERROR(commons-logging.jar not found.)
        fi
    else
        AC_MSG_RESULT([internal])
        SYSTEM_APACHE_COMMONS=
        BUILD_TYPE="$BUILD_TYPE APACHE_COMMONS TOMCAT"
    fi
fi
AC_SUBST(SYSTEM_APACHE_COMMONS)
AC_SUBST(COMMONS_LOGGING_JAR)
AC_SUBST(COMMONS_LOGGING_VERSION)

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
        SYSTEM_BSH=TRUE
        if test -z $BSH_JAR; then
            BSH_JAR=/usr/share/java/bsh.jar
        fi
        if ! test -f $BSH_JAR; then
            AC_MSG_ERROR(bsh.jar not found.)
        fi
    else
        AC_MSG_RESULT([internal])
        SYSTEM_BSH=
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
        SYSTEM_RHINO=TRUE
        if test -z $RHINO_JAR; then
            RHINO_JAR=/usr/share/java/js.jar
        fi
        if ! test -f $RHINO_JAR; then
            AC_MSG_ERROR(js.jar not found.)
        fi
    else
        AC_MSG_RESULT([internal])
        SYSTEM_RHINO=
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

# This is only used in KDE3/KDE4/TDE checks to determine if /usr/lib64
# paths should be added to library search path. So lets put all 64-bit
# platforms there.
supports_multilib=
case "$host_cpu" in
x86_64 | powerpc64 | powerpc64le | s390x | aarch64)
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
    qt_test_library="libQtNetwork.so"
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
    if test "$kde_incdir" = "/usr/include"; then kde_incdir=; fi

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

    PKG_CHECK_MODULES([QT4],[QtNetwork QtGui])
    if ! test -z "$kde_incdir"; then
    	KDE4_CFLAGS="-I$kde_incdir $QT4_CFLAGS -DQT_CLEAN_NAMESPACE -DQT_THREAD_SUPPORT"
    else
    	KDE4_CFLAGS="$QT4_CFLAGS -DQT_CLEAN_NAMESPACE -DQT_THREAD_SUPPORT"
    fi

    KDE4_LIBS="-L$kde_libdir -lkio -lkfile -lkdeui -lkdecore -L$qt_lib_dir $QT4_LIBS"
    KDE4_CFLAGS=$(printf '%s' "$KDE4_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
    FilterLibs "$KDE4_LIBS"
    KDE4_LIBS="$filteredlibs"

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
]])],[AC_MSG_RESULT([yes])],[AC_MSG_ERROR([KDE version too old])],[AC_MSG_ERROR([KDE support not tested with cross-compilation])])
    CXXFLAGS=$save_CXXFLAGS
    AC_LANG_POP([C++])

    # Glib is needed for properly handling Qt event loop with Qt's Glib integration enabled.
    # Sets also KDE_GLIB_CFLAGS/KDE_GLIB_LIBS if successful.
    PKG_CHECK_MODULES(KDE_GLIB,[glib-2.0 >= 2.4],
        [
            KDE_HAVE_GLIB=1
            AC_DEFINE(KDE_HAVE_GLIB,1)
            KDE_GLIB_CFLAGS=$(printf '%s' "$KDE_GLIB_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
            FilterLibs "${KDE_GLIB_LIBS}"
            KDE_GLIB_LIBS="${filteredlibs}"

            qt4_fix_warning=

            AC_LANG_PUSH([C++])
            # tst_exclude_socket_notifiers.moc:70:28: runtime error: member access within address 0x60d00000bb20 which does not point to an object of type 'QObjectData'
            # 0x60d00000bb20: note: object is of type 'QObjectPrivate'
            #  02 00 80 3a  90 8a 4e d2 3a 00 00 00  f0 b4 b9 a7 ff 7f 00 00  00 00 00 00 00 00 00 00  20 d8 4e d2
            #               ^~~~~~~~~~~~~~~~~~~~~~~
            #               vptr for 'QObjectPrivate'
            save_CXX=$CXX
            CXX=$(printf %s "$CXX" \
                | sed -e 's/-fno-sanitize-recover\(=[[0-9A-Za-z,_-]]*\)*//')
            save_CXXFLAGS=$CXXFLAGS
            CXXFLAGS="$CXXFLAGS $KDE4_CFLAGS"
            save_LIBS=$LIBS
            LIBS="$LIBS $KDE4_LIBS"
            AC_MSG_CHECKING([whether Qt has fixed ExcludeSocketNotifiers])

            # Prepare meta object data
            TSTBASE="tst_exclude_socket_notifiers"
            TSTMOC="${SRC_ROOT}/vcl/unx/kde4/${TSTBASE}"
            ln -fs "${TSTMOC}.hxx"
            $MOC4 "${TSTBASE}.hxx" -o "${TSTBASE}.moc"

            AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <cstdlib>
#include "tst_exclude_socket_notifiers.moc"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    exit(tst_processEventsExcludeSocket());
    return 0;
}
            ]])],[
                AC_MSG_RESULT([yes])
            ],[
                AC_MSG_RESULT([no])
                AC_MSG_WARN([native KDE4 file pickers will be disabled at runtime])
                if test -z "$qt4_fix_warning"; then
                    add_warning "native KDE4 file pickers will be disabled at runtime, Qt4 fixes needed"
                fi
                qt4_fix_warning=1
                add_warning "  https://bugreports.qt-project.org/browse/QTBUG-37380 (needed)"
                ],[AC_MSG_ERROR([KDE4 file pickers not tested with cross-compilation])])

            # Remove meta object data
            rm -f "${TSTBASE}."*

            AC_MSG_CHECKING([whether Qt avoids QClipboard recursion caused by posted events])

            # Prepare meta object data
            TSTBASE="tst_exclude_posted_events"
            TSTMOC="${SRC_ROOT}/vcl/unx/kde4/${TSTBASE}"
            ln -fs "${TSTMOC}.hxx"
            $MOC4 "${TSTBASE}.hxx" -o "${TSTBASE}.moc"

            AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <cstdlib>
#include "tst_exclude_posted_events.moc"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    exit(tst_excludePostedEvents());
    return 0;
}
            ]])],[
                AC_MSG_RESULT([yes])
            ],[
                AC_MSG_RESULT([no])
                AC_MSG_WARN([native KDE4 file pickers will be disabled at runtime])
                if test -z "$qt4_fix_warning"; then
                    add_warning "native KDE4 file pickers will be disabled at runtime, Qt4 fixes needed"
                fi
                qt4_fix_warning=1
                add_warning "  https://bugreports.qt-project.org/browse/QTBUG-34614 (needed)"
            ],[AC_MSG_ERROR([KDE4 file pickers not tested with cross-compilation])])

            # Remove meta object data
            rm -f "${TSTBASE}."*

            if test -n "$qt4_fix_warning"; then
                add_warning "  https://bugreports.qt-project.org/browse/QTBUG-38585 (recommended)"
            fi

            LIBS=$save_LIBS
            CXXFLAGS=$save_CXXFLAGS
            CXX=$save_CXX
            AC_LANG_POP([C++])
        ],
        AC_MSG_WARN([[No Glib found, KDE4 support will not use native file pickers!]]))
fi
AC_SUBST(KDE4_CFLAGS)
AC_SUBST(KDE4_LIBS)
AC_SUBST(MOC4)
AC_SUBST(KDE_GLIB_CFLAGS)
AC_SUBST(KDE_GLIB_LIBS)
AC_SUBST(KDE_HAVE_GLIB)

dnl ===================================================================
dnl Test whether to include Evolution 2 support
dnl ===================================================================
AC_MSG_CHECKING([whether to enable evolution 2 support])
if test "$enable_evolution2" = "yes" -o "$enable_evolution2" = "TRUE"; then
    AC_MSG_RESULT([yes])
    PKG_CHECK_MODULES(GOBJECT, gobject-2.0)
    GOBJECT_CFLAGS=$(printf '%s' "$GOBJECT_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
    FilterLibs "${GOBJECT_LIBS}"
    GOBJECT_LIBS="${filteredlibs}"
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
       ]])],[AC_MSG_RESULT([yes])],[AC_MSG_ERROR([TDE version too old or too recent, please use another version of TDE or disable TDE address book support])],
       [AC_MSG_ERROR([TDE support not tested with cross compilation])])
    CXXFLAGS=$save_CXXFLAGS
    AC_LANG_POP([C++])
    ENABLE_TDEAB=TRUE
else
    AC_MSG_RESULT([no])
    ENABLE_TDEAB=
fi
AC_SUBST(ENABLE_TDEAB)

dnl ===================================================================
dnl Test which themes to include
dnl ===================================================================
AC_MSG_CHECKING([which themes to include])
# if none given use default subset of available themes
if test "x$with_theme" = "x" -o "x$with_theme" = "xyes"; then
    with_theme="breeze galaxy hicontrast oxygen sifr tango"
    test -z "$ENABLE_RELEASE_BUILD" && with_theme="$with_theme tango_testing"
fi

WITH_THEMES=""
if test "x$with_theme" != "xno"; then
    for theme in $with_theme; do
        case $theme in
        breeze|crystal|elementary|galaxy|hicontrast|human|oxygen|sifr|tango|tango_testing) real_theme="$theme" ;;
        default) real_theme=galaxy ;;
        *) AC_MSG_ERROR([Unknown value for --with-theme: $theme]) ;;
        esac
        WITH_THEMES=`echo "$WITH_THEMES $real_theme"|tr '\ ' '\n'|sort|uniq|tr '\n' '\ '`
    done
fi
AC_MSG_RESULT([$WITH_THEMES])
AC_SUBST([WITH_THEMES])
# FIXME: remove this, and the convenience default->galaxy remapping after a crace period
for theme in $with_theme; do
    case $theme in
    default) AC_MSG_WARN([--with-theme=default is deprecated and will be removed, use --with-theme=galaxy]) ;;
    *) ;;
    esac
done

dnl ===================================================================
dnl Test whether to integrate helppacks into the product's installer
dnl ===================================================================
AC_MSG_CHECKING([for helppack integration])
if test "$with_helppack_integration" = "no"; then
    WITH_HELPPACK_INTEGRATION=
    AC_MSG_RESULT([no integration])
else
    WITH_HELPPACK_INTEGRATION=TRUE
    SCPDEFS="$SCPDEFS -DWITH_HELPPACK_INTEGRATION"
    AC_MSG_RESULT([integration])
fi
AC_SUBST(WITH_HELPPACK_INTEGRATION)

###############################################################################
# Extensions checking
###############################################################################
AC_MSG_CHECKING([for extensions integration])
if test "x$enable_extension_integration" != "xno"; then
    WITH_EXTENSION_INTEGRATION=TRUE
    SCPDEFS="$SCPDEFS -DWITH_EXTENSION_INTEGRATION"
    AC_MSG_RESULT([yes, use integration])
else
    WITH_EXTENSION_INTEGRATION=
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
    WITH_EXTRA_EXTENSIONS=TRUE
    AC_MSG_RESULT([$SUNTEMPLATES_LANG])
fi
AC_SUBST(SUNTEMPLATES_DE_PACK)
AC_SUBST(SUNTEMPLATES_EN_US_PACK)
AC_SUBST(SUNTEMPLATES_ES_PACK)
AC_SUBST(SUNTEMPLATES_FR_PACK)
AC_SUBST(SUNTEMPLATES_HU_PACK)
AC_SUBST(SUNTEMPLATES_IT_PACK)

TEST_FONTS_MISSING=0

# $1  = font family
# $2+ = accepted font mappings
test_font_map()
{
    FONT="$1" ; shift
    AC_MSG_CHECKING([font mapping for '$FONT'])
    FONTFILE="$(basename `$FCMATCH -f '%{file}' "$FONT"`)"
    FONTFILE_LOWER="$(echo $FONTFILE | $AWK '{print tolower($0)}')"

    TESTEXPR="'${FONTFILE_LOWER}' = '$(echo $FONT | $AWK '{print tolower($0)}').ttf'"
    while test "$#" -ge 1 ; do
        MAPPING="$(echo $1 | $AWK '{print tolower($0)}')"; shift
        TESTEXPR="${TESTEXPR} -o '${FONTFILE_LOWER}' = '$MAPPING-regular.ttf'"
    done
    if test $TESTEXPR
    then
        AC_MSG_RESULT([ok])
    else
        AC_MSG_RESULT([unknown ($FONTFILE)])
        TEST_FONTS_MISSING=1
    fi
}

dnl ===================================================================
dnl Test whether to include fonts
dnl ===================================================================
AC_MSG_CHECKING([whether to include third-party fonts])
if test "$with_fonts" != "no"; then
    AC_MSG_RESULT([yes])
    WITH_FONTS=TRUE
    BUILD_TYPE="$BUILD_TYPE MORE_FONTS"
else
    AC_MSG_RESULT([no])
    WITH_FONTS=
    SCPDEFS="$SCPDEFS -DWITHOUT_FONTS"
    if test "$test_fontconfig" = "yes"; then
        AC_PATH_PROG([FCMATCH], [fc-match])
        if test -z "$FCMATCH"; then
            AC_MSG_WARN([Unable to query installed fonts - unit tests disabled.])
            TEST_FONTS_MISSING=1
        else
            test_font_map 'Calibri' 'Carlito'
            if test ${TEST_FONTS_MISSING} -eq 1
            then
                AC_MSG_WARN([Unknown font mappings - unit tests disabled.])
            fi
        fi
    else
        TEST_FONTS_MISSING=0
    fi
fi
AC_SUBST(WITH_FONTS)
AC_DEFINE_UNQUOTED([TEST_FONTS_MISSING], $TEST_FONTS_MISSING)

dnl ===================================================================
dnl Test whether to include extra galleries
dnl ===================================================================
AC_MSG_CHECKING([whether to include extra galleries])
if test "$enable_extra_gallery" = "" -o "$enable_extra_gallery" = "no"; then
    AC_MSG_RESULT([no])
    WITH_EXTRA_GALLERY=
    OOOP_GALLERY_PACK=""
else
    AC_MSG_RESULT([yes])
    WITH_EXTRA_GALLERY=TRUE
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
    WITH_EXTRA_TEMPLATE=
    OOOP_TEMPLATES_PACK=""
else
    AC_MSG_RESULT([yes])
    WITH_EXTRA_TEMPLATE=TRUE
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
    WITH_EXTRA_SAMPLE=
    OOOP_SAMPLES_PACK=""
else
    AC_MSG_RESULT([yes])
    WITH_EXTRA_SAMPLE=TRUE
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
    WITH_EXTRA_FONT=
    OOOP_FONTS_PACK=""
else
    AC_MSG_RESULT([yes])
    WITH_EXTRA_FONT=TRUE
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
ENABLE_ONLINE_UPDATE_MAR=
if test "$enable_online_update" = ""; then
    if test "$_os" = "WINNT" -o "$_os" = "Darwin"; then
        AC_MSG_RESULT([yes])
        ENABLE_ONLINE_UPDATE="TRUE"
    else
        AC_MSG_RESULT([no])
    fi
else
    if test "$enable_online_update" = "mar"; then
        AC_MSG_RESULT([yes - MAR-based online update])
        ENABLE_ONLINE_UPDATE_MAR="TRUE"
    elif test "$enable_online_update" = "yes"; then
        AC_MSG_RESULT([yes])
        ENABLE_ONLINE_UPDATE="TRUE"
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_SUBST(ENABLE_ONLINE_UPDATE)
AC_SUBST(ENABLE_ONLINE_UPDATE_MAR)

dnl ===================================================================
dnl Test whether we need bzip2
dnl ===================================================================
SYSTEM_BZIP2=
if test "$ENABLE_ONLINE_UPDATE_MAR" = "TRUE"; then
    AC_MSG_CHECKING([whether to use system bzip2])
    if test "$with_system_bzip2" = yes; then
        SYSTEM_BZIP2=TRUE
        AC_MSG_RESULT([yes])
        PKG_CHECK_MODULES(BZIP2, bzip2)
        FilterLibs "${BZIP2_LIBS}"
        BZIP2_LIBS="${filteredlibs}"
    else
        AC_MSG_RESULT([no])
        BUILD_TYPE="$BUILD_TYPE BZIP2"
    fi
fi
AC_SUBST(SYSTEM_BZIP2)
AC_SUBST(BZIP2_CFLAGS)
AC_SUBST(BZIP2_LIBS)

dnl ===================================================================
dnl Test whether to enable extension update
dnl ===================================================================
AC_MSG_CHECKING([whether to enable extension update])
ENABLE_EXTENSION_UPDATE=
if test "x$enable_extension_update" = "xno"; then
    AC_MSG_RESULT([no])
else
    AC_MSG_RESULT([yes])
    ENABLE_EXTENSION_UPDATE="TRUE"
    AC_DEFINE(ENABLE_EXTENSION_UPDATE)
    SCPDEFS="$SCPDEFS -DENABLE_EXTENSION_UPDATE"
fi
AC_SUBST(ENABLE_EXTENSION_UPDATE)


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
if test "$_os" = "Linux" -o "$_os" = "FreeBSD"; then
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
        USE_XINERAMA=TRUE
        if test -z "$with_static_xinerama" -o -n "$with_system_libs"; then
            XINERAMA_LINK=dynamic
        else
            XINERAMA_LINK=static
        fi
    elif test -e "$XINERAMALIB/libXinerama.so" -a ! -e "$XINERAMALIB/libXinerama.a"; then
        # we have only the dynamic version
        USE_XINERAMA=TRUE
        XINERAMA_LINK=dynamic
    elif test -e "$XINERAMALIB/libXinerama.a"; then
        # static version
        if echo $host_cpu | $GREP -E 'i[[3456]]86' 2>/dev/null >/dev/null; then
            USE_XINERAMA=TRUE
            XINERAMA_LINK=static
        else
            USE_XINERAMA=
            XINERAMA_LINK=none
        fi
    else
        # no Xinerama
        USE_XINERAMA=
        XINERAMA_LINK=none
    fi
    if test "$USE_XINERAMA" = "TRUE"; then
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
    USE_XINERAMA=
    XINERAMA_LINK=none
    AC_MSG_RESULT([no])
fi
AC_SUBST(USE_XINERAMA)
AC_SUBST(XINERAMA_LINK)

dnl ===================================================================
dnl Test whether to build cairo or rely on the system version
dnl ===================================================================

if test "$USING_X11" = TRUE; then
    # Used in vcl/Library_vclplug_gen.mk
    test_cairo=yes
fi

if test "$test_cairo" = "yes"; then
    AC_MSG_CHECKING([whether to use the system cairo])

    : ${with_system_cairo:=$with_system_libs}
    if test "$with_system_cairo" = "yes"; then
        SYSTEM_CAIRO=TRUE
        AC_MSG_RESULT([yes])

        PKG_CHECK_MODULES( CAIRO, cairo >= 1.2.0 )
        CAIRO_CFLAGS=$(printf '%s' "$CAIRO_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
        FilterLibs "${CAIRO_LIBS}"
        CAIRO_LIBS="${filteredlibs}"

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
        SYSTEM_CAIRO=
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
if test "$_os" = "WINNT"; then
    # Windows uses bundled mDNSResponder
    BUILD_TYPE="$BUILD_TYPE MDNSRESPONDER"
elif test "$_os" != "Darwin" -a "$enable_avahi" = "yes"; then
    PKG_CHECK_MODULES([AVAHI], [avahi-client >= 0.6.10],
                      [ENABLE_AVAHI="TRUE"])
    AC_DEFINE(HAVE_FEATURE_AVAHI)
    AVAHI_CFLAGS=$(printf '%s' "$AVAHI_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
    FilterLibs "${AVAHI_LIBS}"
    AVAHI_LIBS="${filteredlibs}"
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
if test "$enable_liblangtag" = "yes" -o \
        \( "$enable_liblangtag" = "" -a $_os != iOS -a $_os != Emscripten \); then
    ENABLE_LIBLANGTAG=TRUE
    AC_MSG_RESULT([yes])
    AC_MSG_CHECKING([whether to use system liblangtag])
    if test "$with_system_liblangtag" = yes; then
        SYSTEM_LIBLANGTAG=TRUE
        AC_MSG_RESULT([yes])
        PKG_CHECK_MODULES( LIBLANGTAG, liblangtag >= 0.4.0)
        dnl cf. <https://bitbucket.org/tagoh/liblangtag/commits/9324836a0d1c> "Fix a build issue with inline keyword"
        PKG_CHECK_EXISTS([liblangtag >= 0.5.5], [], [AC_DEFINE([LIBLANGTAG_INLINE_FIX])])
        LIBLANGTAG_CFLAGS=$(printf '%s' "$LIBLANGTAG_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
        FilterLibs "${LIBLANGTAG_LIBS}"
        LIBLANGTAG_LIBS="${filteredlibs}"
        libo_MINGW_CHECK_DLL([liblangtag])
    else
        SYSTEM_LIBLANGTAG=
        AC_MSG_RESULT([no])
        BUILD_TYPE="$BUILD_TYPE LIBLANGTAG"
        LIBLANGTAG_CFLAGS="-I${WORKDIR}/UnpackedTarball/langtag"
        if test "$COM" = "MSC"; then
            LIBLANGTAG_LIBS="${WORKDIR}/UnpackedTarball/langtag/liblangtag/.libs/liblangtag.lib"
        else
            LIBLANGTAG_LIBS="-L${WORKDIR}/UnpackedTarball/langtag/liblangtag/.libs -llangtag"
        fi
    fi
    AC_DEFINE(ENABLE_LIBLANGTAG)
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

libo_CHECK_SYSTEM_MODULE([libpng],[LIBPNG],[libpng],["-I${WORKDIR}/UnpackedTarball/png"],["-L${WORKDIR}/LinkTarget/StaticLibrary -lpng"])

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
    if test -z "$with_ant_home" -a -n "$LODE_HOME" ; then
        if test -x "$LODE_HOME/opt/ant/bin/ant" ; then
            if test "$_os" = "WINNT"; then
                with_ant_home="`cygpath -m $LODE_HOME/opt/ant`"
            else
                with_ant_home="$LODE_HOME/opt/ant"
            fi
        elif test -x  "$LODE_HOME/opt/bin/ant" ; then
            with_ant_home="$LODE_HOME/opt/ant"
        fi
    fi
    if test -z "$with_ant_home"; then
        AC_PATH_PROGS(ANT, [jakarta-ant ant ant.sh ant.bat ant.cmd])
    else
        if test "$_os" = "WINNT"; then
            # AC_PATH_PROGS needs unix path
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
        ANT="$formatted_path"
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

    rm -f conftest* core core.* *.core
fi

OOO_JUNIT_JAR=
if test "$ENABLE_JAVA" != "" -a "$with_junit" != "no"; then
    AC_MSG_CHECKING([for JUnit 4])
    if test "$with_junit" = "yes"; then
        if test -n "$LODE_HOME" -a -e "$LODE_HOME/opt/share/java/junit.jar" ; then
            OOO_JUNIT_JAR="$LODE_HOME/opt/share/java/junit.jar"
        elif test -e /usr/share/java/junit4.jar; then
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
    printf 'import org.junit.Before;' > conftest.java
    if "$JAVACOMPILER" -classpath "$OOO_JUNIT_JAR" conftest.java >&5 2>&5; then
        AC_MSG_RESULT([$OOO_JUNIT_JAR])
    else
        AC_MSG_ERROR(
[cannot find JUnit 4 jar; please install one in the default location (/usr/share/java),
 specify its pathname via --with-junit=..., or disable it via --without-junit])
    fi
    rm -f conftest.class conftest.java
    if test $OOO_JUNIT_JAR != ""; then
    BUILD_TYPE="$BUILD_TYPE QADEVOOO"
    fi
fi
AC_SUBST(OOO_JUNIT_JAR)

HAMCREST_JAR=
if test "$ENABLE_JAVA" != "" -a "$with_junit" != "no"; then
    AC_MSG_CHECKING([for included Hamcrest])
    printf 'import org.hamcrest.BaseDescription;' > conftest.java
    if "$JAVACOMPILER" -classpath "$OOO_JUNIT_JAR" conftest.java >&5 2>&5; then
        AC_MSG_RESULT([Included in $OOO_JUNIT_JAR])
    else
        AC_MSG_RESULT([Not included])
        AC_MSG_CHECKING([for standalone hamcrest jar.])
        if test "$with_hamcrest" = "yes"; then
            if test -e /usr/share/lib/java/hamcrest.jar; then
                HAMCREST_JAR=/usr/share/lib/java/hamcrest.jar
            elif test -e /usr/share/java/hamcrest/core.jar; then
                HAMCREST_JAR=/usr/share/java/hamcrest/core.jar
            else
                HAMCREST_JAR=/usr/share/java/hamcrest.jar
            fi
        else
            HAMCREST_JAR=$with_hamcrest
        fi
        if test "$_os" = "WINNT"; then
            HAMCREST_JAR=`cygpath -m "$HAMCREST_JAR"`
        fi
        if "$JAVACOMPILER" -classpath "$HAMCREST_JAR" conftest.java >&5 2>&5; then
            AC_MSG_RESULT([$HAMCREST_JAR])
        else
            AC_MSG_ERROR([junit does not contain hamcrest; please use a junit jar that includes hamcrest, install a hamcrest jar in the default location (/usr/share/java),
                          specify its path with --with-hamcrest=..., or disable junit with --without-junit])
        fi
    fi
    rm -f conftest.class conftest.java
fi
AC_SUBST(HAMCREST_JAR)


AC_SUBST(SCPDEFS)

#
# check for wget and curl
#
WGET=
CURL=

if test "$enable_fetch_external" != "no"; then

CURL=`which curl 2>/dev/null`

for i in wget /usr/bin/wget /usr/local/bin/wget /usr/sfw/bin/wget /opt/sfw/bin/wget /opt/local/bin/wget; do
    # wget new enough?
    $i --help 2> /dev/null | $GREP no-use-server-timestamps 2>&1 > /dev/null
    if test $? -eq 0; then
        WGET=$i
        break
    fi
done

if test -z "$WGET" -a -z "$CURL"; then
    AC_MSG_ERROR([neither wget nor curl found!])
fi

fi

AC_SUBST(WGET)
AC_SUBST(CURL)

#
# check for md5sum
#
MD5SUM=

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
    AC_MSG_ERROR([no md5sum found!])
fi

AC_SUBST(MD5SUM)

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

if test -n "$with_locales"; then
    WITH_LOCALES="$with_locales"

    just_langs="`echo $WITH_LOCALES | sed -e 's/_[A-Z]*//g'`"
    # Only languages and scripts for which we actually have ifdefs need to be handled. Also see
    # config_host/config_locales.h.in
    for locale in $WITH_LOCALES; do
        lang=${locale%_*}

        AC_DEFINE_UNQUOTED(WITH_LOCALE_$lang, 1)

        case $lang in
        hi|mr*ne)
            AC_DEFINE(WITH_LOCALE_FOR_SCRIPT_Deva)
            ;;
        bg|ru)
            AC_DEFINE(WITH_LOCALE_FOR_SCRIPT_Cyrl)
            ;;
        esac
    done
else
    AC_DEFINE(WITH_LOCALE_ALL)
fi
AC_SUBST(WITH_LOCALES)

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
brand_files="$BRAND_INTRO_IMAGES about.svg"

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

if echo "$host_os" | grep -q linux-android ; then
    ANDROID_PACKAGE_NAME=
    AC_MSG_CHECKING([for Android package name])
    if test -z "$with_android_package_name" -o "$with_android_package_name" = "no"; then
        if test -n "$ENABLE_DEBUG"; then
            # Default to the package name that makes ndk-gdb happy.
            ANDROID_PACKAGE_NAME="org.libreoffice"
        else
            ANDROID_PACKAGE_NAME="org.example.libreoffice"
        fi

        AC_MSG_RESULT([not set, using $ANDROID_PACKAGE_NAME])
    else
        ANDROID_PACKAGE_NAME="$with_android_package_name"
        AC_MSG_RESULT([$ANDROID_PACKAGE_NAME])
    fi
    AC_SUBST(ANDROID_PACKAGE_NAME)
fi

AC_MSG_CHECKING([whether to install the compat oo* wrappers])
if test "$with_compat_oowrappers" = "yes"; then
    WITH_COMPAT_OOWRAPPERS=TRUE
    AC_MSG_RESULT(yes)
else
    WITH_COMPAT_OOWRAPPERS=
    AC_MSG_RESULT(no)
fi
AC_SUBST(WITH_COMPAT_OOWRAPPERS)

INSTALLDIRNAME=`echo AC_PACKAGE_NAME | $AWK '{print tolower($0)}'`
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

TESTINSTALLDIR="${BUILDDIR}/test-install"
AC_SUBST(TESTINSTALLDIR)


# ===================================================================
# OAuth2 id and secrets
# ===================================================================

AC_MSG_CHECKING([for Google Drive client id and secret])
GDRIVE_CLIENT_ID="\"$with_gdrive_client_id\""
GDRIVE_CLIENT_SECRET="\"$with_gdrive_client_secret\""
if test "$with_gdrive_client_id" = "no" -o -z "$with_gdrive_client_id"; then
    GDRIVE_CLIENT_ID="\"\""
fi

if test "$with_gdrive_client_secret" = "no" -o -z "$with_gdrive_client_secret"; then
    GDRIVE_CLIENT_SECRET="\"\""
fi

if test -z "$GDRIVE_CLIENT_ID" -o -z "$GDRIVE_CLIENT_SECRET"; then
    AC_MSG_RESULT([not set])
else
    AC_MSG_RESULT([set])
fi

AC_DEFINE_UNQUOTED(GDRIVE_CLIENT_ID, $GDRIVE_CLIENT_ID)
AC_DEFINE_UNQUOTED(GDRIVE_CLIENT_SECRET, $GDRIVE_CLIENT_SECRET)

AC_MSG_CHECKING([for Alfresco Cloud client id and secret])
ALFRESCO_CLOUD_CLIENT_ID="\"$with_alfresco_cloud_client_id\""
ALFRESCO_CLOUD_CLIENT_SECRET="\"$with_alfresco_cloud_client_secret\""
if test "$with_alfresco_cloud_client_id" = "no" -o -z "$with_alfresco_cloud_client_id"; then
    ALFRESCO_CLOUD_CLIENT_ID="\"\""
fi

if test "$with_alfresco_cloud_client_secret" = "no" -o -z "$with_alfresco_cloud_client_secret"; then
    ALFRESCO_CLOUD_CLIENT_SECRET="\"\""
fi

if test -z "$ALFRESCO_CLOUD_CLIENT_ID" -o -z "$ALFRESCO_CLOUD_CLIENT_SECRET"; then
    AC_MSG_RESULT([not set])
else
    AC_MSG_RESULT([set])
fi
AC_DEFINE_UNQUOTED(ALFRESCO_CLOUD_CLIENT_ID, $ALFRESCO_CLOUD_CLIENT_ID)
AC_DEFINE_UNQUOTED(ALFRESCO_CLOUD_CLIENT_SECRET, $ALFRESCO_CLOUD_CLIENT_SECRET)

AC_MSG_CHECKING([for OneDrive client id and secret])
ONEDRIVE_CLIENT_ID="\"$with_onedrive_client_id\""
ONEDRIVE_CLIENT_SECRET="\"$with_onedrive_client_secret\""
if test "$with_onedrive_client_id" = "no" -o -z "$with_onedrive_client_id"; then
    ONEDRIVE_CLIENT_ID="\"\""
fi

if test "$with_onedrive_client_secret" = "no" -o -z "$with_onedrive_client_secret"; then
    ONEDRIVE_CLIENT_SECRET="\"\""
fi

if test -z "$ONEDRIVE_CLIENT_ID" -o -z "$ONEDRIVE_CLIENT_SECRET"; then
    AC_MSG_RESULT([not set])
else
    AC_MSG_RESULT([set])
fi
AC_DEFINE_UNQUOTED(ONEDRIVE_CLIENT_ID, $ONEDRIVE_CLIENT_ID)
AC_DEFINE_UNQUOTED(ONEDRIVE_CLIENT_SECRET, $ONEDRIVE_CLIENT_SECRET)


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
        PARALLELISM=0
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
        # unexpected format, 'wc -l' will have returned 0 (and we won't use -j at all).
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

if test $PARALLELISM -eq 0; then
    AC_MSG_RESULT([explicit make -j option needed])
else
    AC_MSG_RESULT([$PARALLELISM])
fi
AC_SUBST(PARALLELISM)

IWYU_PATH="$with_iwyu"
AC_SUBST(IWYU_PATH)
if test ! -z "$IWYU_PATH"; then
    if test ! -f "$IWYU_PATH"; then
        AC_MSG_ERROR([cannot find include-what-you-use binary specified by --with-iwyu])
    fi
fi

#
# Set up ILIB for MSVC build
#
ILIB1=
if test "$build_os" = "cygwin"; then
    ILIB="."
    if test -n "$JAVA_HOME" -a "$JAVA_HOME" != "NO_JAVA_HOME"; then
        ILIB="$ILIB;$JAVA_HOME/lib"
    fi
    ILIB1=-link
    if test "$BITNESS_OVERRIDE" = 64; then
        ILIB="$ILIB;$COMPATH/lib/amd64"
        ILIB1="$ILIB1 -LIBPATH:$COMPATH/lib/amd64"
        ILIB="$ILIB;$WINDOWS_SDK_HOME/lib/x64"
        ILIB1="$ILIB1 -LIBPATH:$WINDOWS_SDK_HOME/lib/x64"
        if test $WINDOWS_SDK_VERSION = 80 -o $WINDOWS_SDK_VERSION = 81 -o $WINDOWS_SDK_VERSION = 10; then
            ILIB="$ILIB;$WINDOWS_SDK_HOME/lib/$winsdklibsubdir/um/x64"
            ILIB1="$ILIB1 -LIBPATH:$WINDOWS_SDK_HOME/lib/$winsdklibsubdir/um/x64"
        fi
        if test $VCVER = 140; then
            PathFormat "${UCRTSDKDIR}lib/$UCRTVERSION/ucrt/x64"
            ucrtlibpath_formatted=$formatted_path
            ILIB="$ILIB;$ucrtlibpath_formatted"
        fi
    else
        ILIB="$ILIB;$COMPATH/lib"
        ILIB1="$ILIB1 -LIBPATH:$COMPATH/lib"
        ILIB="$ILIB;$WINDOWS_SDK_HOME/lib"
        ILIB1="$ILIB1 -LIBPATH:$WINDOWS_SDK_HOME/lib"
        if test $WINDOWS_SDK_VERSION = 80 -o $WINDOWS_SDK_VERSION = 81 -o $WINDOWS_SDK_VERSION = 10; then
            ILIB="$ILIB;$WINDOWS_SDK_HOME/lib/$winsdklibsubdir/um/x86"
            ILIB1="$ILIB1 -LIBPATH:$WINDOWS_SDK_HOME/lib/$winsdklibsubdir/um/x86"
        fi
        if test $VCVER = 140; then
            PathFormat "${UCRTSDKDIR}lib/$UCRTVERSION/ucrt/x86"
            ucrtlibpath_formatted=$formatted_path
            ILIB="$ILIB;$ucrtlibpath_formatted"
        fi
    fi
    if test -f "$DOTNET_FRAMEWORK_HOME/lib/mscoree.lib"; then
        ILIB="$ILIB;$DOTNET_FRAMEWORK_HOME/lib"
    else
        ILIB="$ILIB;$DOTNET_FRAMEWORK_HOME/Lib/um/$WINDOWS_SDK_ARCH"
    fi

    AC_SUBST(ILIB)
fi

AC_MSG_CHECKING(
    [whether C++11 use of const_iterator in standard containers is broken])
save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $CXXFLAGS_CXX11"
AC_LANG_PUSH([C++])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    #include <list>
    ]],[[
        std::list<int> l;
        l.erase(l.cbegin());
    ]])],
    [broken=no], [broken=yes])
AC_LANG_POP([C++])
LIBS=$save_LIBS
CXXFLAGS=$save_CXXFLAGS
AC_MSG_RESULT([$broken])
if test "$broken" = yes; then
    AC_DEFINE([HAVE_BROKEN_CONST_ITERATORS])
fi


AC_MSG_CHECKING([whether $CXX has broken static initializer_list support])
save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $CXXFLAGS_CXX11"
save_LIBS=$LIBS
if test -n "$ILIB1"; then
    LIBS="$LIBS $ILIB1"
fi
AC_LANG_PUSH([C++])
AC_RUN_IFELSE([AC_LANG_PROGRAM([[
    // Exit with failure if the static initializer_list is stored on the
    // stack (as done by Clang < 3.4):
    #include <initializer_list>
    struct S {};
    bool g(void const * p1, void const * p2) {
        int n;
        return !((p1 > p2 && p2 > &n) || (p1 < p2 && p2 < &n));
    }
    bool f(void const * p1) {
        static std::initializer_list<S> s { S() };
        return g(p1, s.begin());
    }
    ]],[[
        int n;
        return f(&n) ? 0 : 1;
    ]])], [broken=no], [broken=yes],[broken='assuming not (cross-compiling)'])
AC_LANG_POP([C++])
LIBS=$save_LIBS
CXXFLAGS=$save_CXXFLAGS
AC_MSG_RESULT([$broken])
if test "$broken" = yes; then
    AC_DEFINE([HAVE_BROKEN_STATIC_INITIALIZER_LIST])
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
    MERGELIBS="TRUE"
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST([MERGELIBS])

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
dnl Setup the ICECC_VERSION for the build the same way it was set for
dnl configure, so that CC/CXX and ICECC_VERSION are in sync
dnl ===================================================================
x_ICECC_VERSION=[\#]
if test -n "$ICECC_VERSION" ; then
    x_ICECC_VERSION=
fi
AC_SUBST(x_ICECC_VERSION)
AC_SUBST(ICECC_VERSION)

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
    if test "$WITH_WEBDAV" = "neon"; then
        AC_MSG_ERROR([need --with-webdav=serf or --without-webdav - webdav support.])
    fi
    if test "x$enable_ext_mariadb_connector" = "xyes"; then
        AC_MSG_ERROR([need to --disable-ext-mariadb-connector - mariadb/mysql support.])
    fi
    if test -n "$ENABLE_PDFIMPORT"; then
        if test "x$SYSTEM_POPPLER" = "x"; then
            AC_MSG_ERROR([need to disable PDF import via poppler or use system library])
        fi
    fi
    # cf. m4/libo_check_extension.m4
    if test "x$WITH_EXTRA_EXTENSIONS" != "x"; then
        AC_MSG_ERROR([need to disable extra extensions '$WITH_EXTRA_EXTENSIONS'])
    fi
    for theme in $WITH_THEMES; do
        case $theme in
        breeze|crystal|default|hicontrast|human|oxygen|sifr)
            AC_MSG_ERROR([need to disable icon themes from '$WITH_THEMES': $theme present, use --with-theme=tango]) ;;
        *) : ;;
        esac
    done

    ENABLE_OPENGL=

    if test "$enable_lpsolve" != "no" -o "x$ENABLE_LPSOLVE" = "xTRUE"; then
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
AC_MSG_NOTICE([setting up the build environment variables...])

AC_SUBST(COMPATH)

if test "$build_os" = "cygwin"; then
    if test -d "$COMPATH/atlmfc/lib"; then
        ATL_LIB="$COMPATH/atlmfc/lib"
        ATL_INCLUDE="$COMPATH/atlmfc/include"
    else
        ATL_LIB="$WINDOWS_SDK_HOME/lib" # Doesn't exist for VSE
        ATL_INCLUDE="$WINDOWS_SDK_HOME/include/atl"
    fi
    if test "$BITNESS_OVERRIDE" = 64; then
        ATL_LIB="$ATL_LIB/amd64"
    fi
    # sort.exe and find.exe also exist in C:/Windows/system32 so need /usr/bin/
    PathFormat "/usr/bin/find.exe"
    FIND="$formatted_path"
    PathFormat "/usr/bin/sort.exe"
    SORT="$formatted_path"
    PathFormat "/usr/bin/grep.exe"
    WIN_GREP="$formatted_path"
    PathFormat "/usr/bin/ls.exe"
    WIN_LS="$formatted_path"
    PathFormat "/usr/bin/touch.exe"
    WIN_TOUCH="$formatted_path"
else
    FIND=find
    SORT=sort
fi

AC_SUBST(ATL_INCLUDE)
AC_SUBST(ATL_LIB)
AC_SUBST(FIND)
AC_SUBST(SORT)
AC_SUBST(WIN_GREP)
AC_SUBST(WIN_LS)
AC_SUBST(WIN_TOUCH)

AC_SUBST(BUILD_TYPE)

AC_SUBST(SOLARINC)

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
        # Win32 make needs native paths
        if test "$GNUMAKE_WIN_NATIVE" = "TRUE" ; then
            LO_PATH=`cygpath -p -m "$PATH"`
        fi
        if test "$BITNESS_OVERRIDE" = 64; then
            # needed for msi packaging
            pathmunge "$WINDOWS_SDK_HOME/bin/x86" "before"
        fi
        # .NET 4.6 and higher don't have bin directory
        if test -f "$DOTNET_FRAMEWORK_HOME/bin"; then
            pathmunge "$DOTNET_FRAMEWORK_HOME/bin" "before"
        fi
        pathmunge "$ASM_HOME" "before"
        pathmunge "$WINDOWS_SDK_HOME/bin" "before"
        pathmunge "$CSC_PATH" "before"
        pathmunge "$MIDL_PATH" "before"
        pathmunge "$AL_PATH" "before"
        pathmunge "$MSPDB_PATH" "before"
        if test -n "$MSBUILD_PATH" ; then
            pathmunge "$MSBUILD_PATH" "before"
        fi
        if test "$BITNESS_OVERRIDE" = 64; then
            if test "$vcexpress" = "Express"; then
                pathmunge "$COMPATH/bin/x86_amd64" "before"
                pathmunge "$WINDOWS_SDK_HOME/bin/x64" "before"
            else
                pathmunge "$COMPATH/bin/amd64" "before"
                pathmunge "$WINDOWS_SDK_HOME/bin/x64" "before"
            fi
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
fi

AC_SUBST(LO_PATH)

# Generate a configuration md5 we can use for deps
if test -f config_host.mk; then
    config_md5=`$MD5SUM config_host.mk | sed "s/ .*//"`
fi
if test -f config_host_lang.mk; then
    config_lang_md5=`$MD5SUM config_host_lang.mk | sed "s/ .*//"`
fi

AC_CONFIG_FILES([config_host.mk
                 config_host_lang.mk
                 Makefile
                 lo.xcent
                 bin/bffvalidator.sh
                 bin/odfvalidator.sh
                 instsetoo_native/util/openoffice.lst
                 sysui/desktop/macosx/Info.plist
                 ios/lo.xcconfig])
AC_CONFIG_HEADERS([config_host/config_buildid.h])
AC_CONFIG_HEADERS([config_host/config_clang.h])
AC_CONFIG_HEADERS([config_host/config_dconf.h])
AC_CONFIG_HEADERS([config_host/config_eot.h])
AC_CONFIG_HEADERS([config_host/config_extension_update.h])
AC_CONFIG_HEADERS([config_host/config_cairo_canvas.h])
AC_CONFIG_HEADERS([config_host/config_cxxabi.h])
AC_CONFIG_HEADERS([config_host/config_dbus.h])
AC_CONFIG_HEADERS([config_host/config_features.h])
AC_CONFIG_HEADERS([config_host/config_firebird.h])
AC_CONFIG_HEADERS([config_host/config_folders.h])
AC_CONFIG_HEADERS([config_host/config_gio.h])
AC_CONFIG_HEADERS([config_host/config_global.h])
AC_CONFIG_HEADERS([config_host/config_graphite.h])
AC_CONFIG_HEADERS([config_host/config_java.h])
AC_CONFIG_HEADERS([config_host/config_lgpl.h])
AC_CONFIG_HEADERS([config_host/config_liblangtag.h])
AC_CONFIG_HEADERS([config_host/config_locales.h])
AC_CONFIG_HEADERS([config_host/config_mpl.h])
AC_CONFIG_HEADERS([config_host/config_orcus.h])
AC_CONFIG_HEADERS([config_host/config_kde4.h])
AC_CONFIG_HEADERS([config_host/config_oox.h])
AC_CONFIG_HEADERS([config_host/config_opengl.h])
AC_CONFIG_HEADERS([config_host/config_options.h])
AC_CONFIG_HEADERS([config_host/config_test.h])
AC_CONFIG_HEADERS([config_host/config_telepathy.h])
AC_CONFIG_HEADERS([config_host/config_typesizes.h])
AC_CONFIG_HEADERS([config_host/config_vcl.h])
AC_CONFIG_HEADERS([config_host/config_vclplug.h])
AC_CONFIG_HEADERS([config_host/config_version.h])
AC_CONFIG_HEADERS([config_host/config_oauth2.h])
AC_CONFIG_HEADERS([config_host/config_poppler.h])
AC_CONFIG_HEADERS([config_host/config_python.h])
AC_CONFIG_HEADERS([config_host/config_writerperfect.h])
AC_OUTPUT

if test "$CROSS_COMPILING" = TRUE; then
    (echo; echo export BUILD_TYPE_FOR_HOST=$BUILD_TYPE) >>config_build.mk
fi

# touch the config timestamp file
if test ! -f config_host.mk.stamp; then
    echo > config_host.mk.stamp
elif test "$config_md5" = `$MD5SUM config_host.mk | sed "s/ .*//"`; then
    echo "Host Configuration unchanged - avoiding scp2 stamp update"
else
    echo > config_host.mk.stamp
fi

# touch the config lang timestamp file
if test ! -f config_host_lang.mk.stamp; then
    echo > config_host_lang.mk.stamp
elif test "$config_lang_md5" = `$MD5SUM config_host_lang.mk | sed "s/ .*//"`; then
    echo "Language Configuration unchanged - avoiding scp2 stamp update"
else
    echo > config_host_lang.mk.stamp
fi


if test "$STALE_MAKE" = "TRUE" -a "$build_os" = "cygwin"; then

cat << _EOS
****************************************************************************
WARNING:
Your make version is known to be horribly slow, and hard to debug
problems with. To get a reasonably functional make please do:

to install a pre-compiled binary make for Win32

 mkdir -p /opt/lo/bin
 cd /opt/lo/bin
 wget http://dev-www.libreoffice.org/bin/cygwin/make-85047eb-msvc.exe
 cp make-85047eb-msvc.exe make
 chmod +x make

to install from source:
place yourself in a working directory of you choice.

 git clone git://git.savannah.gnu.org/make.git

 [go to Start menu, click "All Programs", click "Visual Studio 2013", click "Visual Studio Tools", double-click "VS2013 x86 Native Tools Command Prompt" or "VS2013 x64 Native Tools Command Prompt"]
 set PATH=%PATH%;C:\Cygwin\bin
 [or Cygwin64, if that is what you have]
 cd path-to-make-repo-you-cloned-above
 build_w32.bat --without-guile

should result in a WinRel/gnumake.exe.
Copy it to the Cygwin /opt/lo/bin directory as make.exe

Then re-run autogen.sh

Note: autogen.sh will try to use /opt/lo/bin/make if the environment variable GNUMAKE is not already defined.
Alternatively, you can install the 'new' make where ever you want and make sure that `which make` finds it.

_EOS
fi

cat << _EOF
****************************************************************************

To build, run:
$GNUMAKE

To view some help, run:
$GNUMAKE help

_EOF

if test $_os != WINNT -a "$CROSS_COMPILING" != TRUE; then
    cat << _EOF
After the build has finished, you can immediately run it using:
_EOF

    if test $_os = Darwin; then
        echo open instdir/$PRODUCTNAME.app
    else
        echo instdir/program/soffice
    fi
    cat << _EOF

If you want to run the smoketest, run:
$GNUMAKE check

_EOF
fi

if test -f warn; then
    cat warn
    rm warn
fi

dnl vim:set shiftwidth=4 softtabstop=4 expandtab:
