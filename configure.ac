dnl -*- Mode: Autoconf; tab-width: 4; indent-tabs-mode: nil; fill-column: 102 -*-
dnl configure.ac serves as input for the GNU autoconf package
dnl in order to create a configure script.

# The version number in the second argument to AC_INIT should be four numbers separated by
# periods. Some parts of the code requires the first one to be less than 128 and the others to be less
# than 256. The four numbers can optionally be followed by a period and a free-form string containing
# no spaces or periods, like "frobozz-mumble-42" or "alpha0". If the free-form string ends with one or
# several non-alphanumeric characters, those are split off and used only for the
# ABOUTBOXPRODUCTVERSIONSUFFIX in openoffice.lst. Why that is necessary, no idea.

AC_INIT([Collabora Office],[6.4.10.52],[],[],[https://collaboraoffice.com/])

dnl libnumbertext needs autoconf 2.68, but that can pick up autoconf268 just fine if it is installed
dnl whereas aclocal (as run by autogen.sh) insists on using autoconf and fails hard
dnl so check for the version of autoconf that is actually used to create the configure script
AC_PREREQ([2.59])
m4_if(m4_version_compare(m4_defn([AC_AUTOCONF_VERSION]), [2.68]), -1,
    [AC_MSG_ERROR([at least autoconf version 2.68 is needed (you can use AUTOCONF environment variable to point to a suitable one)])])

if test -n "$BUILD_TYPE"; then
    AC_MSG_ERROR([You have sourced config_host.mk in this shell.  This may lead to trouble, please run in a fresh (login) shell.])
fi

save_CC=$CC
save_CXX=$CXX

first_arg_basename()
{
    for i in $1; do
        basename "$i"
        break
    done
}

CC_BASE=`first_arg_basename "$CC"`
CXX_BASE=`first_arg_basename "$CXX"`

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
                # (on macOS it would be an error to use it instead of SDK)
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
        fp_count_space=`echo "$formatted_path" | $GREP -c "[ ]"`
        if test "$fp_count_space" != "0"; then
            AC_MSG_ERROR([converted path "$formatted_path" still contains spaces. Short filenames (8.3 filenames) support was disabled on this system?])
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
        if which tput >/dev/null && test "`tput colors 2>/dev/null || echo 0`" -ge 8; then
            dnl <esc> as actual byte (U+1b), [ escaped using quadrigraph @<:@
            COLORWARN='*@<:@1;33;40m WARNING @<:@0m:'
        else
            COLORWARN="* WARNING :"
        fi
    fi
    echo "$COLORWARN $@" >> warn
}

dnl Some Mac User have the bad habit of letting a lot of crap
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
dnl a not so common but nevertheless quite helpful thing that may be in a fancy
dnl path location is gpg, so make sure we find it
    mac_gpg_path=`which gpg 2>/dev/null`
    if test -n "$mac_gpg_path" -a -x "$mac_gpg_path" -a "$mac_gpg_path" != "/usr/bin/gpg" ; then
        mac_path="$mac_path:`dirname $mac_gpg_path`"
    fi
    PATH="$mac_path"
    unset mac_path
    unset mac_git_path
    unset mac_gpg_path
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
if test -n "$with_product_name" -a "$with_product_name" != no; then
    PRODUCTNAME="$with_product_name"
fi
if test "$enable_release_build" = "" -o "$enable_release_build" = "no"; then
    PRODUCTNAME="${PRODUCTNAME}Dev"
    PRODUCTNAME=AC_PACKAGE_NAME
fi
AC_MSG_RESULT([$PRODUCTNAME])
AC_SUBST(PRODUCTNAME)
PRODUCTNAME_WITHOUT_SPACES=$(printf %s "$PRODUCTNAME" | sed 's/ //g')
AC_SUBST(PRODUCTNAME_WITHOUT_SPACES)

dnl ===================================================================
dnl Our version is defined by the AC_INIT() at the top of this script.
dnl ===================================================================

AC_MSG_CHECKING([for package version])
if test -n "$with_package_version" -a "$with_package_version" != no; then
    PACKAGE_VERSION="$with_package_version"
fi
AC_MSG_RESULT([$PACKAGE_VERSION])

set `echo "$PACKAGE_VERSION" | sed "s/\./ /g"`

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
        AC_MSG_ERROR([COMSPEC not set in environment, please set it and rerun])
    else
        AC_MSG_RESULT([found: $COMSPEC])
    fi
fi

AC_SUBST(SRC_ROOT)
AC_SUBST(BUILDDIR)
AC_SUBST(x_Cygwin)
AC_DEFINE_UNQUOTED(SRCDIR,"$SRC_ROOT")
AC_DEFINE_UNQUOTED(SRC_ROOT,"$SRC_ROOT")
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

AC_ARG_ENABLE(fuzz-options,
    AS_HELP_STRING([--enable-fuzz-options],
        [Randomly enable or disable each of those configurable options
         that are supposed to be freely selectable without interdependencies,
         or where bad interaction from interdependencies is automatically avoided.])
)

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
        Android NDK you are using. The default (and only supported version currently) is "clang5.0"]),,
        with_android_ndk_toolchain_version=clang5.0)

AC_ARG_WITH(android-sdk,
    AS_HELP_STRING([--with-android-sdk],
        [Specify location of the Android SDK. Mandatory when building for Android.]),
,)

AC_ARG_WITH(android-api-level,
    AS_HELP_STRING([--with-android-api-level],
        [Specify the API level when building for Android. Defaults to 16 for ARM and x86 and to 21 for ARM64 and x86-64]),
,)

ANDROID_NDK_HOME=
if test -z "$with_android_ndk" -a -e "$SRC_ROOT/external/android-ndk" -a "$build" != "$host"; then
    with_android_ndk="$SRC_ROOT/external/android-ndk"
fi
if test -n "$with_android_ndk"; then
    eval ANDROID_NDK_HOME=$with_android_ndk

    # Set up a lot of pre-canned defaults

    if test ! -f $ANDROID_NDK_HOME/RELEASE.TXT; then
        if test ! -f $ANDROID_NDK_HOME/source.properties; then
            AC_MSG_ERROR([Unrecognized Android NDK. Missing RELEASE.TXT or source.properties file in $ANDROID_NDK_HOME.])
        fi
        ANDROID_NDK_VERSION=`sed -n -e 's/Pkg.Revision = //p' $ANDROID_NDK_HOME/source.properties`
    else
        ANDROID_NDK_VERSION=`cut -f1 -d' ' <$ANDROID_NDK_HOME/RELEASE.TXT`
    fi
    if test -z "$ANDROID_NDK_VERSION";  then
        AC_MSG_ERROR([Failed to determine Android NDK version. Please check your installation.])
    fi
    case $ANDROID_NDK_VERSION in
    r9*|r10*)
        AC_MSG_ERROR([Building for Android is only supported with NDK versions above 16.x*])
        ;;
    11.1.*|12.1.*|13.1.*|14.1.*)
        AC_MSG_ERROR([Building for Android is only supported with NDK versions above 16.x.*])
        ;;
    16.*|17.*|18.*|19.*|20.*)
        ;;
    *)
        AC_MSG_WARN([Untested Android NDK version $ANDROID_NDK_VERSION, only versions 16.* til 20.* have been used successfully. Proceed at your own risk.])
        add_warning "Untested Android NDK version $ANDROID_NDK_VERSION, only versions 16.* til 20.* have been used successfully. Proceed at your own risk."
        ;;
    esac

    ANDROID_API_LEVEL=16
    if test -n "$with_android_api_level" ; then
        ANDROID_API_LEVEL="$with_android_api_level"
    fi

    android_cpu=$host_cpu
    if test $host_cpu = arm; then
        android_platform_prefix=arm-linux-androideabi
        android_gnu_prefix=$android_platform_prefix
        LLVM_TRIPLE=armv7a-linux-androideabi
        ANDROID_APP_ABI=armeabi-v7a
        ANDROIDCFLAGS="-mthumb -march=armv7-a -mfloat-abi=softfp -mfpu=neon -Wl,--fix-cortex-a8"
    elif test $host_cpu = aarch64; then
        android_platform_prefix=aarch64-linux-android
        android_gnu_prefix=$android_platform_prefix
        LLVM_TRIPLE=$android_platform_prefix
        # minimum android version that supports aarch64
        if test "$ANDROID_API_LEVEL" -lt "21" ; then
            ANDROID_API_LEVEL=21
        fi
        ANDROID_APP_ABI=arm64-v8a
    elif test $host_cpu = x86_64; then
        android_platform_prefix=x86_64-linux-android
        android_gnu_prefix=$android_platform_prefix
        LLVM_TRIPLE=$android_platform_prefix
        # minimum android version that supports x86_64
        ANDROID_API_LEVEL=21
        ANDROID_APP_ABI=x86_64
    else
        # host_cpu is something like "i386" or "i686" I guess, NDK uses
        # "x86" in some contexts
        android_cpu=x86
        android_platform_prefix=$android_cpu
        android_gnu_prefix=i686-linux-android
        LLVM_TRIPLE=$android_gnu_prefix
        ANDROID_APP_ABI=x86
    fi

    case "$with_android_ndk_toolchain_version" in
    clang5.0)
        ANDROID_GCC_TOOLCHAIN_VERSION=4.9
        ;;
    *)
        AC_MSG_ERROR([Unrecognized value for the --with-android-ndk-toolchain-version option. Building for Android is only supported with Clang 5.*])
    esac

    AC_MSG_NOTICE([using the Android API level... $ANDROID_API_LEVEL])

    # NDK 15 or later toolchain is 64bit-only, except for Windows that we don't support. Using a 64-bit
    # linker is required if you compile large parts of the code with -g. A 32-bit linker just won't
    # manage to link the (app-specific) single huge .so that is built for the app in
    # android/source/ if there is debug information in a significant part of the object files.
    # (A 64-bit ld.gold grows too much over 10 gigabytes of virtual space when linking such a .so if
    # all objects have been built with debug information.)
    case $build_os in
    linux-gnu*)
        android_HOST_TAG=linux-x86_64
        ;;
    darwin*)
        android_HOST_TAG=darwin-x86_64
        ;;
    *)
        AC_MSG_ERROR([We only support building for Android from Linux or macOS])
        # ndk would also support windows and windows-x86_64
        ;;
    esac
    android_TOOLCHAIN=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/$android_HOST_TAG
    ANDROID_COMPILER_BIN=$android_TOOLCHAIN/bin
    dnl TODO: NSS build uses it...
    ANDROID_BINUTILS_PREBUILT_ROOT=$ANDROID_NDK_HOME/toolchains/$android_platform_prefix-$ANDROID_GCC_TOOLCHAIN_VERSION/prebuilt/$android_HOST_TAG
    AC_SUBST(ANDROID_BINUTILS_PREBUILT_ROOT)

    test -z "$AR" && AR=$ANDROID_COMPILER_BIN/$android_gnu_prefix-ar
    test -z "$NM" && NM=$ANDROID_COMPILER_BIN/$android_gnu_prefix-nm
    test -z "$OBJDUMP" && OBJDUMP=$ANDROID_COMPILER_BIN/$android_gnu_prefix-objdump
    test -z "$RANLIB" && RANLIB=$ANDROID_COMPILER_BIN/$android_gnu_prefix-ranlib
    test -z "$STRIP" && STRIP=$ANDROID_COMPILER_BIN/$android_gnu_prefix-strip

    ANDROIDCFLAGS="$ANDROIDCFLAGS -target ${LLVM_TRIPLE}${ANDROID_API_LEVEL}"
    ANDROIDCFLAGS="$ANDROIDCFLAGS -no-canonical-prefixes -ffunction-sections -fdata-sections -Qunused-arguments"
    if test "$ENABLE_LTO" = TRUE; then
        # -flto comes from com_GCC_defs.mk, too, but we need to make sure it gets passed as part of
        # $CC and $CXX when building external libraries
        ANDROIDCFLAGS="$ANDROIDCFLAGS -flto -fuse-linker-plugin -O2"
    fi

    ANDROIDCXXFLAGS="$ANDROIDCFLAGS -stdlib=libc++"

    if test -z "$CC"; then
        CC="$ANDROID_COMPILER_BIN/clang $ANDROIDCFLAGS"
        CC_BASE="clang"
    fi
    if test -z "$CXX"; then
        CXX="$ANDROID_COMPILER_BIN/clang++ $ANDROIDCXXFLAGS"
        CXX_BASE="clang++"
    fi

    # remember to download the ownCloud Android library later
    BUILD_TYPE="$BUILD_TYPE OWNCLOUD_ANDROID_LIB"
fi
AC_SUBST(ANDROID_NDK_HOME)
AC_SUBST(ANDROID_APP_ABI)
AC_SUBST(ANDROID_GCC_TOOLCHAIN_VERSION)

dnl ===================================================================
dnl --with-android-sdk
dnl ===================================================================
ANDROID_SDK_HOME=
if test -z "$with_android_sdk" -a -e "$SRC_ROOT/external/android-sdk-linux" -a "$build" != "$host"; then
    with_android_sdk="$SRC_ROOT/external/android-sdk-linux"
fi
if test -n "$with_android_sdk"; then
    eval ANDROID_SDK_HOME=$with_android_sdk
    PATH="$ANDROID_SDK_HOME/platform-tools:$ANDROID_SDK_HOME/tools:$PATH"
fi
AC_SUBST(ANDROID_SDK_HOME)

AC_ARG_ENABLE([android-lok],
    AS_HELP_STRING([--enable-android-lok],
        [The Android app from the android/ subdir needs several tweaks all
         over the place that break the LOK when used in the Online-based
         Android app.  This switch indicates that the intent of this build is
         actually the Online-based, non-modified LOK.])
)
ENABLE_ANDROID_LOK=
if test -n "$ANDROID_NDK_HOME" ; then
    if test "$enable_android_lok" = yes; then
        ENABLE_ANDROID_LOK=TRUE
        AC_DEFINE(HAVE_FEATURE_ANDROID_LOK)
        AC_MSG_NOTICE([building the Android version... for the Online-based Android app])
    else
        AC_MSG_NOTICE([building the Android version... for the app from the android/ subdir])
    fi
fi
AC_SUBST([ENABLE_ANDROID_LOK])

libo_FUZZ_ARG_ENABLE([android-editing],
    AS_HELP_STRING([--enable-android-editing],
        [Enable the experimental editing feature on Android.])
)
ENABLE_ANDROID_EDITING=
if test "$enable_android_editing" = yes; then
    ENABLE_ANDROID_EDITING=TRUE
fi
AC_SUBST([ENABLE_ANDROID_EDITING])

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
test_gdb_index=no
test_split_debug=no

# Default values, as such probably valid just for Linux, set
# differently below just for Mac OSX, but at least better than
# hardcoding these as we used to do. Much of this is duplicated also
# in solenv for old build system and for gbuild, ideally we should
# perhaps define stuff like this only here in configure.ac?

LINKFLAGSSHL="-shared"
PICSWITCH="-fpic"
DLLPOST=".so"

LINKFLAGSNOUNDEFS="-Wl,-z,defs"

INSTROOTBASESUFFIX=
INSTROOTCONTENTSUFFIX=
SDKDIRNAME=sdk

HOST_PLATFORM="$host"

host_cpu_for_clang="$host_cpu"

case "$host_os" in

solaris*)
    build_gstreamer_1_0=yes
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
    build_gstreamer_1_0=yes
    test_kf5=yes
    test_gtk3_kde5=yes
    test_gdb_index=yes
    test_split_debug=yes
    if test "$enable_fuzzers" != yes; then
        test_freetype=yes
        test_fontconfig=yes
    else
        test_freetype=no
        test_fontconfig=no
        BUILD_TYPE="$BUILD_TYPE FONTCONFIG FREETYPE"
    fi
    _os=Linux
    ;;

gnu)
    test_randr=no
    test_xrender=no
    _os=GNU
     ;;

cygwin*|interix*)

    # When building on Windows normally with MSVC under Cygwin,
    # configure thinks that the host platform (the platform the
    # built code will run on) is Cygwin, even if it obviously is
    # Windows, which in Autoconf terminology is called
    # "mingw32". (Which is misleading as MinGW is the name of the
    # tool-chain, not an operating system.)

    # Somewhat confusing, yes. But this configure script doesn't
    # look at $host etc that much, it mostly uses its own $_os
    # variable, set here in this case statement.

    test_cups=no
    test_dbus=no
    test_randr=no
    test_xrender=no
    test_freetype=no
    test_fontconfig=no
    _os=WINNT

    DLLPOST=".dll"
    LINKFLAGSNOUNDEFS=
    ;;

darwin*|macos*) # macOS
    test_randr=no
    test_xrender=no
    test_freetype=no
    test_fontconfig=no
    test_dbus=no
    if test -n "$LODE_HOME" ; then
        mac_sanitize_path
        AC_MSG_NOTICE([sanitized the PATH to $PATH])
    fi
    _os=Darwin
    INSTROOTBASESUFFIX=/$PRODUCTNAME_WITHOUT_SPACES.app
    INSTROOTCONTENTSUFFIX=/Contents
    SDKDIRNAME=${PRODUCTNAME_WITHOUT_SPACES}${PRODUCTVERSION}_SDK
    # See comment above the case "$host_os"
    LINKFLAGSSHL="-dynamiclib -single_module"

    # -fPIC is default
    PICSWITCH=""

    DLLPOST=".dylib"

    # -undefined error is the default
    LINKFLAGSNOUNDEFS=""
    case "$host_cpu" in
    aarch64|arm64)
        case "$host_os" in
        macos*)
            # HOST_PLATFORM is used for external projects and their configury occasionally doesn't like
            # the "macos" part so be sure to use aarch64-apple-darwin for now.
            HOST_PLATFORM=aarch64-apple-darwin
            ;;
        esac

        # Apple's Clang uses "arm64"
        host_cpu_for_clang=arm64
    esac
;;

ios*) # iOS
    test_randr=no
    test_xrender=no
    test_freetype=no
    test_fontconfig=no
    test_dbus=no
    if test -n "$LODE_HOME" ; then
        mac_sanitize_path
        AC_MSG_NOTICE([sanitized the PATH to $PATH])
    fi
    build_for_ios=YES
    _os=iOS
    test_cups=no
    enable_mpl_subset=yes
    enable_lotuswordpro=no
    enable_coinmp=no
    enable_lpsolve=no
    enable_postgresql_sdbc=no
    enable_extension_integration=no
    enable_report_builder=no
    with_ppds=no
    if test "$enable_ios_simulator" = "yes"; then
        host=x86_64-apple-darwin
    fi
    # See comment above the case "$host_os"
    LINKFLAGSSHL="-dynamiclib -single_module"

    # -fPIC is default
    PICSWITCH=""

    DLLPOST=".dylib"

    # -undefined error is the default
    LINKFLAGSNOUNDEFS=""

    # HOST_PLATFORM is used for external projects and their configury typically doesn't like the "ios"
    # part, so use aarch64-apple-darwin for now.
    HOST_PLATFORM=aarch64-apple-darwin

    # Apple's Clang uses "arm64"
    host_cpu_for_clang=arm64
;;

freebsd*)
    build_gstreamer_1_0=yes
    test_kf5=yes
    test_gtk3_kde5=yes
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
    build_gstreamer_1_0=yes
    test_kf5=yes
    test_gtk3_kde5=yes
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
    test_freetype=yes
    PTHREAD_CFLAGS="-D_THREAD_SAFE"
    PTHREAD_LIBS="-pthread"
    _os=OpenBSD
    ;;

dragonfly*)
    build_gstreamer_1_0=yes
    test_kf5=yes
    test_gtk3_kde5=yes
    test_freetype=yes
    PTHREAD_LIBS="-pthread"
    _os=DragonFly
    ;;

linux-android*)
    build_gstreamer_1_0=no
    enable_lotuswordpro=no
    enable_mpl_subset=yes
    enable_coinmp=yes
    enable_lpsolve=no
    enable_report_builder=no
    enable_odk=no
    enable_postgresql_sdbc=no
    enable_python=no
    test_cups=no
    test_dbus=no
    test_fontconfig=no
    test_freetype=no
    test_kf5=no
    test_qt5=no
    test_gtk3_kde5=no
    test_randr=no
    test_xrender=no
    _os=Android

    AC_DEFINE(HAVE_FT_FACE_GETCHARVARIANTINDEX)
    BUILD_TYPE="$BUILD_TYPE CAIRO FONTCONFIG FREETYPE"
    ;;

haiku*)
    test_cups=no
    test_dbus=no
    test_randr=no
    test_xrender=no
    test_freetype=yes
    enable_odk=no
    enable_gstreamer_1_0=no
    enable_vlc=no
    enable_coinmp=no
    enable_pdfium=no
    enable_sdremote=no
    enable_postgresql_sdbc=no
    enable_firebird_sdbc=no
    _os=Haiku
    ;;

*)
    AC_MSG_ERROR([$host_os operating system is not suitable to build LibreOffice for!])
    ;;
esac

AC_SUBST(HOST_PLATFORM)

if test "$_os" = "Android" ; then
    # Verify that the NDK and SDK options are proper
    if test -z "$with_android_ndk"; then
        AC_MSG_ERROR([the --with-android-ndk option is mandatory, unless it is available at external/android-ndk/.])
    elif test ! -f "$ANDROID_NDK_HOME/meta/abis.json"; then
        AC_MSG_ERROR([the --with-android-ndk option does not point to an Android NDK])
    fi

    if test -z "$ANDROID_SDK_HOME"; then
        AC_MSG_ERROR([the --with-android-sdk option is mandatory, unless it is available at external/android-sdk-linux/.])
    elif test ! -d "$ANDROID_SDK_HOME/platforms"; then
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

AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_LIBS)

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

dnl These are potentially set for MSVC, in the code checking for UCRT below:
my_original_CFLAGS=$CFLAGS
my_original_CXXFLAGS=$CXXFLAGS
my_original_CPPFLAGS=$CPPFLAGS

dnl The following checks for gcc, cc and then cl (if it weren't guarded for win32)
dnl Needs to precede the AC_C_BIGENDIAN and AC_SEARCH_LIBS calls below, which apparently call
dnl AC_PROG_CC internally.
if test "$_os" != "WINNT"; then
    # AC_PROG_CC sets CFLAGS to -g -O2 if not set, avoid that
    save_CFLAGS=$CFLAGS
    AC_PROG_CC
    CFLAGS=$save_CFLAGS
    if test -z "$CC_BASE"; then
        CC_BASE=`first_arg_basename "$CC"`
    fi
fi

if test "$_os" != "WINNT"; then
    AC_C_BIGENDIAN([ENDIANNESS=big], [ENDIANNESS=little])
else
    ENDIANNESS=little
fi
AC_SUBST(ENDIANNESS)

if test $_os != "WINNT"; then
    save_LIBS="$LIBS"
    AC_SEARCH_LIBS([dlsym], [dl],
        [case "$ac_cv_search_dlsym" in -l*) DLOPEN_LIBS="$ac_cv_search_dlsym";; esac],
        [AC_MSG_ERROR([dlsym not found in either libc nor libdl])])
    LIBS="$save_LIBS"
fi
AC_SUBST(DLOPEN_LIBS)

AC_ARG_ENABLE(ios-simulator,
    AS_HELP_STRING([--enable-ios-simulator],
        [build i386 or x86_64 for ios simulator])
)

AC_ARG_ENABLE(ios-libreofficelight-app,
    AS_HELP_STRING([--enable-ios-libreofficelight-app],
        [When building for iOS, build stuff relevant only for the 'LibreOfficeLight' app
         (in ios/LibreOfficeLight). Note that this app is not known to work in any useful manner,
         and also that its actual build (in Xcode) requires some obvious modifications to the project.])
)

ENABLE_IOS_LIBREOFFICELIGHT_APP=
if test "$enable_ios_libreofficelight_app" = yes; then
    ENABLE_IOS_LIBREOFFICELIGHT_APP=TRUE
fi
AC_SUBST(ENABLE_IOS_LIBREOFFICELIGHT_APP)

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

libo_FUZZ_ARG_ENABLE(extension-integration,
    AS_HELP_STRING([--disable-extension-integration],
        [Disable integration of the built extensions in the installer of the
         product. Use this switch to disable the integration.])
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
libo_FUZZ_ARG_ENABLE(extensions,
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

libo_FUZZ_ARG_ENABLE(report-builder,
    AS_HELP_STRING([--disable-report-builder],
        [Disable the Report Builder.])
)

libo_FUZZ_ARG_ENABLE(ext-wiki-publisher,
    AS_HELP_STRING([--enable-ext-wiki-publisher],
        [Enable the Wiki Publisher extension.])
)

libo_FUZZ_ARG_ENABLE(lpsolve,
    AS_HELP_STRING([--disable-lpsolve],
        [Disable compilation of the lp solve solver ])
)
libo_FUZZ_ARG_ENABLE(coinmp,
    AS_HELP_STRING([--disable-coinmp],
        [Disable compilation of the CoinMP solver ])
)

libo_FUZZ_ARG_ENABLE(pdfimport,
    AS_HELP_STRING([--disable-pdfimport],
        [Disable building the PDF import feature.])
)

libo_FUZZ_ARG_ENABLE(pdfium,
    AS_HELP_STRING([--disable-pdfium],
        [Disable building PDFium.])
)

###############################################################################

dnl ---------- *** ----------

libo_FUZZ_ARG_ENABLE(mergelibs,
    AS_HELP_STRING([--enable-mergelibs],
        [Merge several of the smaller libraries into one big, "merged", one.])
)

libo_FUZZ_ARG_ENABLE(breakpad,
    AS_HELP_STRING([--enable-breakpad],
        [Enables breakpad for crash reporting.])
)

libo_FUZZ_ARG_ENABLE(crashdump,
    AS_HELP_STRING([--disable-crashdump],
        [Disable dump.ini and dump-file, when --enable-breakpad])
)

AC_ARG_ENABLE(fetch-external,
    AS_HELP_STRING([--disable-fetch-external],
        [Disables fetching external tarballs from web sources.])
)

AC_ARG_ENABLE(fuzzers,
    AS_HELP_STRING([--enable-fuzzers],
        [Enables building libfuzzer targets for fuzz testing.])
)

libo_FUZZ_ARG_ENABLE(pch,
    AS_HELP_STRING([--enable-pch=<yes/no/system/base/normal/full>],
        [Enables precompiled header support for C++. Forced default on Windows/VC build.
         Using 'system' will include only external headers, 'base' will add also headers
         from base modules, 'normal' will also add all headers except from the module built,
         'full' will use all suitable headers even from a module itself.])
)

libo_FUZZ_ARG_ENABLE(epm,
    AS_HELP_STRING([--enable-epm],
        [LibreOffice includes self-packaging code, that requires epm, however epm is
         useless for large scale package building.])
)

libo_FUZZ_ARG_ENABLE(odk,
    AS_HELP_STRING([--disable-odk],
        [LibreOffice includes an ODK, office development kit which some packagers may
         wish to build without.])
)

AC_ARG_ENABLE(mpl-subset,
    AS_HELP_STRING([--enable-mpl-subset],
        [Don't compile any pieces which are not MPL or more liberally licensed])
)

libo_FUZZ_ARG_ENABLE(evolution2,
    AS_HELP_STRING([--enable-evolution2],
        [Allows the built-in evolution 2 addressbook connectivity build to be
         enabled.])
)

AC_ARG_ENABLE(avahi,
    AS_HELP_STRING([--enable-avahi],
        [Determines whether to use Avahi to advertise Impress to remote controls.])
)

libo_FUZZ_ARG_ENABLE(werror,
    AS_HELP_STRING([--enable-werror],
        [Turn warnings to errors. (Has no effect in modules where the treating
         of warnings as errors is disabled explicitly.)]),
,)

libo_FUZZ_ARG_ENABLE(assert-always-abort,
    AS_HELP_STRING([--enable-assert-always-abort],
        [make assert() failures abort even when building without --enable-debug or --enable-dbgutil.]),
,)

libo_FUZZ_ARG_ENABLE(dbgutil,
    AS_HELP_STRING([--enable-dbgutil],
        [Provide debugging support from --enable-debug and include additional debugging
         utilities such as object counting or more expensive checks.
         This is the recommended option for developers.
         Note that this makes the build ABI incompatible, it is not possible to mix object
         files or libraries from a --enable-dbgutil and a --disable-dbgutil build.]))

libo_FUZZ_ARG_ENABLE(debug,
    AS_HELP_STRING([--enable-debug],
        [Include debugging information, disable compiler optimization and inlining plus
         extra debugging code like assertions. Extra large build! (enables -g compiler flag).]))

libo_FUZZ_ARG_ENABLE(split-debug,
    AS_HELP_STRING([--disable-split-debug],
        [Disable using split debug information (-gsplit-dwarf compile flag). Split debug information
         saves disk space and build time, but requires tools that support it (both build tools and debuggers).]))

libo_FUZZ_ARG_ENABLE(gdb-index,
    AS_HELP_STRING([--disable-gdb-index],
        [Disables creating debug information in the gdb index format, which makes gdb start faster.
         The feature requires the gold or lld linker.]))

libo_FUZZ_ARG_ENABLE(sal-log,
    AS_HELP_STRING([--enable-sal-log],
        [Make SAL_INFO and SAL_WARN calls do something even in a non-debug build.]))

libo_FUZZ_ARG_ENABLE(symbols,
    AS_HELP_STRING([--enable-symbols],
        [Generate debug information.
         By default, enabled for --enable-debug and --enable-dbgutil, disabled
         otherwise. It is possible to explicitly specify gbuild build targets
         (where 'all' means everything, '-' prepended means to not enable, '/' appended means
         everything in the directory; there is no ordering, more specific overrides
         more general, and disabling takes precedence).
         Example: --enable-symbols="all -sw/ -Library_sc".]))

libo_FUZZ_ARG_ENABLE(optimized,
    AS_HELP_STRING([--disable-optimized],
        [Whether to compile with optimization flags.
         By default, disabled for --enable-debug and --enable-dbgutil, enabled
         otherwise.]))

libo_FUZZ_ARG_ENABLE(runtime-optimizations,
    AS_HELP_STRING([--disable-runtime-optimizations],
        [Statically disable certain runtime optimizations (like rtl/alloc.h or
         JVM JIT) that are known to interact badly with certain dynamic analysis
         tools (like -fsanitize=address or Valgrind).  By default, disabled iff
         CC contains "-fsanitize=*".  (For Valgrind, those runtime optimizations
         are typically disabled dynamically via RUNNING_ON_VALGRIND.)]))

AC_ARG_WITH(valgrind,
    AS_HELP_STRING([--with-valgrind],
        [Make availability of Valgrind headers a hard requirement.]))

libo_FUZZ_ARG_ENABLE(compiler-plugins,
    AS_HELP_STRING([--enable-compiler-plugins],
        [Enable compiler plugins that will perform additional checks during
         building. Enabled automatically by --enable-dbgutil.
         Use --enable-compiler-plugins=debug to also enable debug code in the plugins.]))
COMPILER_PLUGINS_DEBUG=
if test "$enable_compiler_plugins" = debug; then
    enable_compiler_plugins=yes
    COMPILER_PLUGINS_DEBUG=TRUE
fi

libo_FUZZ_ARG_ENABLE(ooenv,
    AS_HELP_STRING([--disable-ooenv],
        [Disable ooenv for the instdir installation.]))

libo_FUZZ_ARG_ENABLE(libnumbertext,
    AS_HELP_STRING([--disable-libnumbertext],
        [Disable use of numbertext external library.]))

AC_ARG_ENABLE(lto,
    AS_HELP_STRING([--enable-lto],
        [Enable link-time optimization. Suitable for (optimised) product builds. Building might take
         longer but libraries and executables are optimized for speed. For GCC, best to use the 'gold'
         linker. For MSVC, this option is broken at the moment. This is experimental work
         in progress that shouldn't be used unless you are working on it.)]))

AC_ARG_ENABLE(python,
    AS_HELP_STRING([--enable-python=<no/auto/system/internal/fully-internal>],
        [Enables or disables Python support at run-time.
         Also specifies what Python to use. 'auto' is the default.
         'fully-internal' even forces the internal version for uses of Python
         during the build.]))

libo_FUZZ_ARG_ENABLE(gtk3,
    AS_HELP_STRING([--disable-gtk3],
        [Determines whether to use Gtk+ 3.0 vclplug on platforms where Gtk+ 3.0 is available.]),
,test "${enable_gtk3+set}" = set || enable_gtk3=yes)

AC_ARG_ENABLE(introspection,
    AS_HELP_STRING([--enable-introspection],
        [Generate files for GObject introspection.  Requires --enable-gtk3.  (Typically used by
         Linux distributions.)]))

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

libo_FUZZ_ARG_ENABLE(cairo-canvas,
    AS_HELP_STRING([--disable-cairo-canvas],
        [Determines whether to build the Cairo canvas on platforms where Cairo is available.]),
,)

libo_FUZZ_ARG_ENABLE(dbus,
    AS_HELP_STRING([--disable-dbus],
        [Determines whether to enable features that depend on dbus.
         e.g. Presentation mode screensaver control, bluetooth presentation control, automatic font install]),
,test "${enable_dbus+set}" = set || enable_dbus=yes)

libo_FUZZ_ARG_ENABLE(sdremote,
    AS_HELP_STRING([--disable-sdremote],
        [Determines whether to enable Impress remote control (i.e. the server component).]),
,test "${enable_sdremote+set}" = set || enable_sdremote=yes)

libo_FUZZ_ARG_ENABLE(sdremote-bluetooth,
    AS_HELP_STRING([--disable-sdremote-bluetooth],
        [Determines whether to build sdremote with bluetooth support.
         Requires dbus on Linux.]))

libo_FUZZ_ARG_ENABLE(gio,
    AS_HELP_STRING([--disable-gio],
        [Determines whether to use the GIO support.]),
,test "${enable_gio+set}" = set || enable_gio=yes)

AC_ARG_ENABLE(qt5,
    AS_HELP_STRING([--enable-qt5],
        [Determines whether to use Qt5 vclplug on platforms where Qt5 is
         available.]),
,)

AC_ARG_ENABLE(kf5,
    AS_HELP_STRING([--enable-kf5],
        [Determines whether to use Qt5/KF5 vclplug on platforms where Qt5 and
         KF5 are available.]),
,)

AC_ARG_ENABLE(kde5,
    AS_HELP_STRING([--enable-kde5],
        [Compatibility switch for the kde5 => kf5 rename. Use --enable-kf5!])
,)

AC_ARG_ENABLE(gtk3_kde5,
    AS_HELP_STRING([--enable-gtk3-kde5],
        [Determines whether to use Gtk3 vclplug with KF5 file dialogs on
         platforms where Gtk3, Qt5 and Plasma is available.]),
,)

AC_ARG_ENABLE(gui,
    AS_HELP_STRING([--disable-gui],
        [Disable use of X11 or Wayland to reduce dependencies (e.g. for building LibreOfficeKit).]),
,enable_gui=yes)

libo_FUZZ_ARG_ENABLE(randr,
    AS_HELP_STRING([--disable-randr],
        [Disable RandR support in the vcl project.]),
,test "${enable_randr+set}" = set || enable_randr=yes)

libo_FUZZ_ARG_ENABLE(gstreamer-1-0,
    AS_HELP_STRING([--disable-gstreamer-1-0],
        [Disable building with the gstreamer 1.0 avmedia backend.]),
,test "${enable_gstreamer_1_0+set}" = set || enable_gstreamer_1_0=yes)

libo_FUZZ_ARG_ENABLE(vlc,
    AS_HELP_STRING([--enable-vlc],
        [Enable building with the (experimental) VLC avmedia backend.]),
,test "${enable_vlc+set}" = set || enable_vlc=no)

libo_FUZZ_ARG_ENABLE(neon,
    AS_HELP_STRING([--disable-neon],
        [Disable neon and the compilation of webdav binding.]),
,)

libo_FUZZ_ARG_ENABLE([eot],
    [AS_HELP_STRING([--enable-eot],
        [Enable support for Embedded OpenType fonts.])],
,test "${enable_eot+set}" = set || enable_eot=no)

libo_FUZZ_ARG_ENABLE(cve-tests,
    AS_HELP_STRING([--disable-cve-tests],
        [Prevent CVE tests to be executed]),
,)

libo_FUZZ_ARG_ENABLE(chart-tests,
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

AC_ARG_ENABLE(build-opensymbol,
    AS_HELP_STRING([--enable-build-opensymbol],
        [Do not use the prebuilt opens___.ttf. Build it instead. This needs
         fontforge installed.]),
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

AC_ARG_ENABLE(ld,
    AS_HELP_STRING([--enable-ld=<linker>],
        [Use the specified linker. Both 'gold' and 'lld' linkers generally use less memory and link faster.
         By default tries to use the best linker possible, use --disable-ld to use the default linker.]),
,)

libo_FUZZ_ARG_ENABLE(cups,
    AS_HELP_STRING([--disable-cups],
        [Do not build cups support.])
)

AC_ARG_ENABLE(ccache,
    AS_HELP_STRING([--disable-ccache],
        [Do not try to use ccache automatically.
         By default, unless on Windows, we will try to detect if ccache is available; in that case if
         CC/CXX are not yet set, and --enable-icecream is not given, we
         attempt to use ccache. --disable-ccache disables ccache completely.
         Additionally ccache's depend mode is enabled if possible,
         use --enable-ccache=nodepend to enable ccache without depend mode.
]),
,)

AC_ARG_ENABLE(64-bit,
    AS_HELP_STRING([--enable-64-bit],
        [Build a 64-bit LibreOffice on platforms where the normal build is 32-bit.
         At the moment meaningful only for Windows.]), ,)

libo_FUZZ_ARG_ENABLE(online-update,
    AS_HELP_STRING([--enable-online-update],
        [Enable the online update service that will check for new versions of
         LibreOffice. By default, it is enabled on Windows and Mac, disabled on Linux.
         If the value is "mar", the experimental Mozilla-like update will be
         enabled instead of the traditional update mechanism.]),
,)

AC_ARG_WITH(update-config,
    AS_HELP_STRING([--with-update-config=/tmp/update.ini],
                   [Path to the update config ini file]))

libo_FUZZ_ARG_ENABLE(extension-update,
    AS_HELP_STRING([--disable-extension-update],
        [Disable possibility to update installed extensions.]),
,)

libo_FUZZ_ARG_ENABLE(release-build,
    AS_HELP_STRING([--enable-release-build],
        [Enable release build. Note that the "release build" choice is orthogonal to
         whether symbols are present, debug info is generated, or optimization
         is done.
         See http://wiki.documentfoundation.org/Development/DevBuild]),
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
        [Define the macOS bundle identifier. Default is the somewhat weird
         org.libreoffice.script ("script", huh?).]),
,with_macosx_bundle_identifier=org.libreoffice.script)

AC_ARG_WITH(product-name,
    AS_HELP_STRING([--with-product-name='My Own Office Suite'],
        [Define the product name. Default is AC_PACKAGE_NAME.]),
,with_product_name=$PRODUCTNAME)

AC_ARG_WITH(package-version,
    AS_HELP_STRING([--with-package-version='3.1.4.5'],
        [Define the package version. Default is AC_PACKAGE_VERSION. Use only if you distribute an own build for macOS.]),
,)

libo_FUZZ_ARG_ENABLE(readonly-installset,
    AS_HELP_STRING([--enable-readonly-installset],
        [Prevents any attempts by LibreOffice to write into its installation. That means
         at least that no "system-wide" extensions can be added. Partly experimental work in
         progress, probably not fully implemented (but is useful for sandboxed macOS builds).]),
,)

libo_FUZZ_ARG_ENABLE(postgresql-sdbc,
    AS_HELP_STRING([--disable-postgresql-sdbc],
        [Disable the build of the PostgreSQL-SDBC driver.])
)

libo_FUZZ_ARG_ENABLE(lotuswordpro,
    AS_HELP_STRING([--disable-lotuswordpro],
        [Disable the build of the Lotus Word Pro filter.]),
,test "${enable_lotuswordpro+set}" = set || enable_lotuswordpro=yes)

libo_FUZZ_ARG_ENABLE(firebird-sdbc,
    AS_HELP_STRING([--disable-firebird-sdbc],
        [Disable the build of the Firebird-SDBC driver if it doesn't compile for you.]),
,test "${enable_firebird_sdbc+set}" = set || enable_firebird_sdbc=yes)

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

libo_FUZZ_ARG_ENABLE(cipher-openssl-backend,
    AS_HELP_STRING([--enable-cipher-openssl-backend],
        [Enable using OpenSSL as the actual implementation of the rtl/cipher.h functionality.
         Requires --enable-openssl.]))

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

AC_ARG_ENABLE(dconf,
    AS_HELP_STRING([--disable-dconf],
        [Disable the dconf configuration backend (enabled by default where
         available).]))

libo_FUZZ_ARG_ENABLE(formula-logger,
    AS_HELP_STRING(
        [--enable-formula-logger],
        [Enable formula logger for logging formula calculation flow in Calc.]
    )
)

AC_ARG_ENABLE(ldap,
    AS_HELP_STRING([--disable-ldap],
        [Disable LDAP support.]),
,enable_ldap=yes)

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
         Possible choices: *breeze, *breeze_dark, *breeze_dark_svg, *breeze_svg, *colibre, *colibre_svg, *elementary,
         *elementary_svg, *karasa_jaga, *karasa_jaga_svg, *sifr, *sifr_dark, *sifr_dark_svg, *sifr_svg, *tango.]),
,)

libo_FUZZ_ARG_WITH(helppack-integration,
    AS_HELP_STRING([--without-helppack-integration],
        [It will not integrate the helppacks to the installer
         of the product. Please use this switch to use the online help
         or separate help packages.]),
,)

libo_FUZZ_ARG_WITH(fonts,
    AS_HELP_STRING([--without-fonts],
        [LibreOffice includes some third-party fonts to provide a reliable basis for
         help content, templates, samples, etc. When these fonts are already
         known to be available on the system then you should use this option.]),
,)

AC_ARG_ENABLE(noto-font,
    AS_HELP_STRING([--enable-noto-font],
        [Add more Google Noto fonts.]),
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
         (implicit) --enable-gtk3.]))

AC_ARG_WITH(system-epoxy,
    AS_HELP_STRING([--with-system-epoxy],
        [Use epoxy libraries already on system.  Happens automatically for
         (implicit) --enable-gtk3.]),,
       [with_system_epoxy="$with_system_libs"])

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

libo_FUZZ_ARG_ENABLE(poppler,
    AS_HELP_STRING([--disable-poppler],
        [Disable building Poppler.])
)

AC_ARG_WITH(system-poppler,
    AS_HELP_STRING([--with-system-poppler],
        [Use system poppler (only needed for PDF import).]),,
    [with_system_poppler="$with_system_libs"])

AC_ARG_WITH(system-gpgmepp,
    AS_HELP_STRING([--with-system-gpgmepp],
        [Use gpgmepp already on system]),,
    [with_system_gpgmepp="$with_system_libs"])

AC_ARG_WITH(system-apache-commons,
    AS_HELP_STRING([--with-system-apache-commons],
        [Use Apache commons libraries already on system.]),,
    [with_system_apache_commons="$with_system_jars"])

AC_ARG_WITH(system-mariadb,
    AS_HELP_STRING([--with-system-mariadb],
        [Use MariaDB/MySQL libraries already on system.]),,
    [with_system_mariadb="$with_system_libs"])

AC_ARG_ENABLE(bundle-mariadb,
    AS_HELP_STRING([--enable-bundle-mariadb],
        [When using MariaDB/MySQL libraries already on system, bundle them with the MariaDB Connector/LibreOffice.])
)

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

AC_ARG_WITH(system-libtommath,
            AS_HELP_STRING([--with-system-libtommath],
                           [Use libtommath already on system]),,
            [with_system_libtommath="$with_system_libs"])

AC_ARG_WITH(system-hsqldb,
    AS_HELP_STRING([--with-system-hsqldb],
        [Use hsqldb already on system.]))

AC_ARG_WITH(hsqldb-jar,
    AS_HELP_STRING([--with-hsqldb-jar=JARFILE],
        [Specify path to jarfile manually.]),
    HSQLDB_JAR=$withval)

libo_FUZZ_ARG_ENABLE(scripting-beanshell,
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

libo_FUZZ_ARG_ENABLE(scripting-javascript,
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

libo_FUZZ_ARG_ENABLE(qrcodegen,
    AS_HELP_STRING([--disable-qrcodegen],
        [Disable use of qrcodegen external library.]))

AC_ARG_WITH(system-qrcodegen,
    AS_HELP_STRING([--with-system-qrcodegen],
        [Use libqrcodegen already on system.]),,
    [with_system_qrcodegen="$with_system_libs"])

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
        [If you have installed JDK 8 or later on your system please supply the
         path here. Note that this is not the location of the java command but the
         location of the entire distribution.]),
,)

AC_ARG_WITH(help,
    AS_HELP_STRING([--with-help],
        [Enable the build of help. There is a special parameter "common" that
         can be used to bundle only the common part, .e.g help-specific icons.
         This is useful when you build the helpcontent separately.])
    [
                          Usage:     --with-help    build the old local help
                                 --without-help     no local help (default)
                                 --with-help=html   build the new HTML local help
                                 --with-help=online build the new HTML online help
    ],
,)

AC_ARG_WITH(omindex,
   AS_HELP_STRING([--with-omindex],
	[Enable the support of xapian-omega index for online help.])
   [
                         Usage: --with-omindex=server prepare the pages for omindex
				but let xapian-omega be built in server.
				--with-omindex=noxap do not prepare online pages
                                for xapian-omega
  ],
,)

libo_FUZZ_ARG_WITH(java,
    AS_HELP_STRING([--with-java=<java command>],
        [Specify the name of the Java interpreter command. Typically "java"
         which is the default.

         To build without support for Java components, applets, accessibility
         or the XML filters written in Java, use --without-java or --with-java=no.]),
    [ test -z "$with_java" -o "$with_java" = "yes" && with_java=java ],
    [ test -z "$with_java" -o "$with_java" = "yes" && with_java=java ]
)

AC_ARG_WITH(jvm-path,
    AS_HELP_STRING([--with-jvm-path=<absolute path to parent of jvm home>],
        [Use a specific JVM search path at runtime.
         e.g. use --with-jvm-path=/usr/lib/ to find JRE/JDK in /usr/lib/jvm/]),
,)

AC_ARG_WITH(ant-home,
    AS_HELP_STRING([--with-ant-home=<absolute path to Ant home>],
        [If you have installed Apache Ant on your system, please supply the path here.
         Note that this is not the location of the Ant binary but the location
         of the entire distribution.]),
,)

AC_ARG_WITH(symbol-config,
    AS_HELP_STRING([--with-symbol-config],
        [Configuration for the crashreport symbol upload]),
        [],
        [with_symbol_config=no])

AC_ARG_WITH(export-validation,
    AS_HELP_STRING([--without-export-validation],
        [Disable validating OOXML and ODF files as exported from in-tree tests.]),
,with_export_validation=auto)

AC_ARG_WITH(bffvalidator,
    AS_HELP_STRING([--with-bffvalidator=<absolute path to BFFValidator>],
        [Enables export validation for Microsoft Binary formats (doc, xls, ppt).
         Requires installed Microsoft Office Binary File Format Validator.
         Note: export-validation (--with-export-validation) is required to be turned on.
         See https://www.microsoft.com/en-us/download/details.aspx?id=26794]),
,with_bffvalidator=no)

libo_FUZZ_ARG_WITH(junit,
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

libo_FUZZ_ARG_WITH(doxygen,
    AS_HELP_STRING(
        [--with-doxygen=<absolute path to doxygen executable>],
        [Specifies the doxygen executable to use when generating ODK C/C++
         documentation. --without-doxygen disables generation of ODK C/C++
         documentation. Not relevant in the --disable-odk case.]),
,with_doxygen=yes)

AC_ARG_WITH(visual-studio,
    AS_HELP_STRING([--with-visual-studio=<2017>],
        [Specify which Visual Studio version to use in case several are
         installed. Currently only 2017 is supported.]),
,)

AC_ARG_WITH(windows-sdk,
    AS_HELP_STRING([--with-windows-sdk=<8.0(A)/8.1(A)/10>],
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
libo_FUZZ_ARG_WITH(krb5,
    AS_HELP_STRING([--with-krb5],
        [Enable MIT Kerberos 5 support in modules that support it.
         By default automatically enabled on platforms
         where a good system Kerberos 5 is available.]),
,)

libo_FUZZ_ARG_WITH(gssapi,
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

libo_FUZZ_ARG_WITH(lxml,
    AS_HELP_STRING([--without-lxml],
        [gla11y will use python lxml when available, potentially building a local copy if necessary.
         --without-lxml tells it to not use python lxml at all, which means that gla11y will only
         report widget classes and ids.]),
,)

libo_FUZZ_ARG_WITH(latest-c++,
    AS_HELP_STRING([--with-latest-c++],
        [Try to enable the latest features of the C++ compiler, even if they are not yet part of a
         published standard.]),,
        [with_latest_c__=no])

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
                          PROGRESSTEXTCOLOR="0,0,0" Set color of progress
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
        [For FreeBSD users, use this option to override the detected OSVERSION.]),
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

AC_ARG_WITH(parallelism,
    AS_HELP_STRING([--with-parallelism],
        [Number of jobs to run simultaneously during build. Parallel builds can
        save a lot of time on multi-cpu machines. Defaults to the number of
        CPUs on the machine, unless you configure --enable-icecream - then to
        40.]),
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
if test "$enable_windows_build_signing" = "yes" -a "$_os" = "WINNT"; then
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
        [Prefer a specific SDK for building.])
    [
                          If the requested SDK is not available, a search for the oldest one will be done.
                          With current Xcode versions, only the latest SDK is included, so this option is
                          not terribly useful. It works fine to build with a new SDK and run the result
                          on an older OS.

                          e. g.: --with-macosx-sdk=10.10

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
                          min  | max  | sdk  || min   | max  | sdk  |
                          ?    | ?    | ?    || 10.10 | 10.s | 10.s |
                          ?    | ?    | 10.x || 10.10 | 10.x | 10.x |
                          ?    | 10.x | ?    || 10.10 | 10.s | 10.s |
                          ?    | 10.x | 10.y || 10.10 | 10.x | 10.y |
                          10.x | ?    | ?    || 10.x  | 10.s | 10.s |
                          10.x | ?    | 10.y || 10.x  | 10.y | 10.y |
                          10.x | 10.y | ?    || 10.x  | 10.y | 10.y |
                          10.x | 10.y | 10.z || 10.x  | 10.y | 10.z |


                          see: http://developer.apple.com/library/mac/#technotes/tn2064/_index.html
                          for a detailed technical explanation of these variables

                          Note: MACOSX_DEPLOYMENT_TARGET will be set to the value of 'min'.
    ],
,)

AC_ARG_WITH(macosx-version-min-required,
    AS_HELP_STRING([--with-macosx-version-min-required=<version>],
        [set the minimum OS version needed to run the built LibreOffice])
    [
                          e. g.: --with-macos-version-min-required=10.10
                          see --with-macosx-sdk for more info
    ],
,)

AC_ARG_WITH(macosx-version-max-allowed,
    AS_HELP_STRING([--with-macosx-version-max-allowed=<version>],
        [set the maximum allowed OS version the LibreOffice compilation can use APIs from])
    [
                          e. g.: --with-macos-version-max-allowed=10.10
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
dnl Check for incompatible options set by fuzzing, and reset those
dnl automatically to working combinations
dnl ===================================================================

if test "$libo_fuzzed_enable_dbus" = yes -a "$libo_fuzzed_enable_avahi" -a \
        "$enable_dbus" != "$enable_avahi"; then
    AC_MSG_NOTICE([Resetting --enable-avahi=$enable_dbus])
    enable_avahi=$enable_dbus
fi

add_lopath_after ()
{
    if ! echo "$LO_PATH" | $EGREP -q "(^|${P_SEP})$1($|${P_SEP})"; then
        LO_PATH="${LO_PATH:+$LO_PATH$P_SEP}$1"
    fi
}

add_lopath_before ()
{
    local IFS=${P_SEP}
    local path_cleanup
    local dir
    for dir in $LO_PATH ; do
        if test "$dir" != "$1" ; then
            path_cleanup=${path_cleanup:+$path_cleanup$P_SEP}$dir
        fi
    done
    LO_PATH="$1${path_cleanup:+$P_SEP$path_cleanup}"
}

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
        if test "$2" = "after"; then
            add_lopath_after "$new_path"
        else
            add_lopath_before "$new_path"
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

if test $_os != iOS -a $_os != Android -a "$enable_fuzzers" != "yes"; then
    BUILD_TYPE="$BUILD_TYPE DESKTOP"
    AC_DEFINE(HAVE_FEATURE_DESKTOP)
    AC_DEFINE(HAVE_FEATURE_MULTIUSER_ENVIRONMENT)
fi

# Whether to build "avmedia" functionality or not.

if test -z "$enable_avmedia"; then
    enable_avmedia=yes
fi

BUILD_TYPE="$BUILD_TYPE AVMEDIA"
if test "$enable_avmedia" = yes; then
    AC_DEFINE(HAVE_FEATURE_AVMEDIA)
else
    USE_AVMEDIA_DUMMY='TRUE'
fi
AC_SUBST(USE_AVMEDIA_DUMMY)

# Decide whether to build database connectivity stuff (including
# Base) or not. We probably don't want to on non-desktop OSes.
if test -z "$enable_database_connectivity"; then
    # --disable-database-connectivity is unfinished work in progress
    # and the iOS test app doesn't link if we actually try to use it.
    # if test $_os != iOS -a $_os != Android; then
    if test $_os != iOS; then
        enable_database_connectivity=yes
    fi
fi

if test "$enable_database_connectivity" = yes; then
    BUILD_TYPE="$BUILD_TYPE DBCONNECTIVITY"
    AC_DEFINE(HAVE_FEATURE_DBCONNECTIVITY)
fi

if test -z "$enable_extensions"; then
    # For iOS and Android disable extensions unless specifically overridden with --enable-extensions.
    if test $_os != iOS; then
        enable_extensions=yes
    fi
fi

if test "$enable_extensions" = yes; then
    BUILD_TYPE="$BUILD_TYPE EXTENSIONS"
    AC_DEFINE(HAVE_FEATURE_EXTENSIONS)
fi

if test -z "$enable_scripting"; then
    enable_scripting=yes
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
    # Otherwise enable it unless specifically disabled
    enable_dynamic_loading=yes
fi

DISABLE_DYNLOADING=''
if test "$enable_dynamic_loading" = yes; then
    BUILD_TYPE="$BUILD_TYPE DYNLOADING"
else
    DISABLE_DYNLOADING='TRUE'
fi
AC_SUBST(DISABLE_DYNLOADING)

# remember SYSBASE value
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
        AC_MSG_RESULT([disable src image build])
    fi
fi
AC_SUBST(WITH_GALLERY_BUILD)

dnl ===================================================================
dnl  Checks if ccache is available
dnl ===================================================================
CCACHE_DEPEND_MODE=
if test "$_os" = "WINNT"; then
    # on windows/VC build do not use ccache
    CCACHE=""
elif test "$enable_ccache" = "no"; then
    CCACHE=""
elif test -n "$enable_ccache" -o \( "$enable_ccache" = "" -a "$enable_icecream" != "yes" \); then
    case "%$CC%$CXX%" in
    # If $CC and/or $CXX already contain "ccache" (possibly suffixed with some version number etc),
    # assume that's good then
    *%ccache[[-_' ']]*|*/ccache[[-_' ']]*)
        AC_MSG_NOTICE([ccache seems to be included in a pre-defined CC and/or CXX])
        CCACHE_DEPEND_MODE=1
        ;;
    *)
        AC_PATH_PROG([CCACHE],[ccache],[not found])
        if test "$CCACHE" = "not found"; then
            CCACHE=""
        else
            CCACHE_DEPEND_MODE=1
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
                    CCACHE_DEPEND_MODE=
                fi
            fi
        fi
        ;;
    esac
else
    CCACHE=""
fi
if test "$enable_ccache" = "nodepend"; then
    CCACHE_DEPEND_MODE=""
fi
AC_SUBST(CCACHE_DEPEND_MODE)

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
if test "$_os" != "WINNT"; then
    GCC_HOME_SET="true"
    AC_MSG_CHECKING([gcc home])
    if test -z "$with_gcc_home"; then
        if test "$enable_icecream" = "yes"; then
            if test -d "/usr/lib/icecc/bin"; then
                GCC_HOME="/usr/lib/icecc/"
            elif test -d "/usr/libexec/icecc/bin"; then
                GCC_HOME="/usr/libexec/icecc/"
            elif test -d "/opt/icecream/bin"; then
                GCC_HOME="/opt/icecream/"
            else
                AC_MSG_ERROR([Could not figure out the location of icecream GCC wrappers, manually use --with-gcc-home])

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
            CC_BASE="gcc"
        fi
        if test -z "$CXX"; then
            CXX="$GCC_HOME/bin/g++"
            CXX_BASE="g++"
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
dnl Check macOS SDK and compiler
dnl ===================================================================

HAVE_MACOS_LD_PLATFORMVERSION=
if test $_os = Darwin; then

    # If no --with-macosx-sdk option is given, look for one

    # The intent is that for "most" Mac-based developers, a suitable
    # SDK will be found automatically without any configure options.

    # For developers with a current Xcode, the lowest-numbered SDK
    # higher than or equal to the minimum required should be found.

    AC_MSG_CHECKING([what macOS SDK to use])
    for _macosx_sdk in ${with_macosx_sdk-12.0 11.3 11.1 11.0 10.15 10.14 10.13 10.12}; do
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
        AC_MSG_ERROR([Could not find an appropriate macOS SDK])
    fi

    AC_MSG_RESULT([SDK $with_macosx_sdk at $MACOSX_SDK_PATH])

    case $with_macosx_sdk in
    10.12)
        MACOSX_SDK_VERSION=101200
        ;;
    10.13)
        MACOSX_SDK_VERSION=101300
        ;;
    10.14)
        MACOSX_SDK_VERSION=101400
        ;;
    10.15)
        MACOSX_SDK_VERSION=101500
        ;;
    11.0)
        MACOSX_SDK_VERSION=110000
        ;;
    11.1)
        MACOSX_SDK_VERSION=110100
        ;;
    11.3)
        MACOSX_SDK_VERSION=110300
        ;;
    12.0)
        MACOSX_SDK_VERSION=120000
        ;;
    *)
        AC_MSG_ERROR([with-macosx-sdk $with_macosx_sdk is not a supported value, supported values are 10.12--12.0])
        ;;
    esac

    if test "$host_cpu" = arm64 -a $MACOSX_SDK_VERSION -lt 110000; then
        AC_MSG_ERROR([with-macosx-sdk $with_macosx_sdk is not a supported value for Apple Silicon])
    fi

    if test "$with_macosx_version_min_required" = "" ; then
        if test "$host_cpu" = x86_64; then
            with_macosx_version_min_required="10.10";
        else
            with_macosx_version_min_required="11.0";
        fi
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

    AC_MSG_CHECKING([whether Xcode is new enough])
    my_xcode_ver1=$(xcrun xcodebuild -version | head -n 1)
    my_xcode_ver2=${my_xcode_ver1#Xcode }
    my_xcode_ver3=$(printf %s "$my_xcode_ver2" | $AWK -F. '{ print $1*100+($2<100?$2:99) }')
    if test "$my_xcode_ver3" -ge 903; then
        AC_MSG_RESULT([yes ($my_xcode_ver2)])
    else
        AC_MSG_ERROR(["$my_xcode_ver1" is too old or unrecognized, must be at least Xcode 9.3])
    fi

    case "$with_macosx_version_min_required" in
    10.10)
        MAC_OS_X_VERSION_MIN_REQUIRED="101000"
        ;;
    10.11)
        MAC_OS_X_VERSION_MIN_REQUIRED="101100"
        ;;
    10.12)
        MAC_OS_X_VERSION_MIN_REQUIRED="101200"
        ;;
    10.13)
        MAC_OS_X_VERSION_MIN_REQUIRED="101300"
        ;;
    10.14)
        MAC_OS_X_VERSION_MIN_REQUIRED="101400"
        ;;
    10.15)
        MAC_OS_X_VERSION_MIN_REQUIRED="101500"
        ;;
    10.16)
        MAC_OS_X_VERSION_MIN_REQUIRED="101600"
        ;;
    11.0)
        MAC_OS_X_VERSION_MIN_REQUIRED="110000"
        ;;
    11.1)
        MAC_OS_X_VERSION_MIN_REQUIRED="110100"
        ;;
    11.3)
        MAC_OS_X_VERSION_MIN_REQUIRED="110300"
        ;;
    12.0)
        MAC_OS_X_VERSION_MIN_REQUIRED="120000"
        ;;
    *)
        AC_MSG_ERROR([with-macosx-version-min-required $with_macosx_version_min_required is not a supported value, supported values are 10.10--12.0])
        ;;
    esac
    MAC_OS_X_VERSION_MIN_REQUIRED_DOTS=$with_macosx_version_min_required

    LIBTOOL=/usr/bin/libtool
    INSTALL_NAME_TOOL=install_name_tool
    if test -z "$save_CC"; then
        stdlib=-stdlib=libc++
        if test "$ENABLE_LTO" = TRUE; then
            lto=-flto
        fi

        AC_MSG_CHECKING([what C compiler to use])
        CC="`xcrun -find clang`"
        CC_BASE=`first_arg_basename "$CC"`
        if test "$host_cpu" = x86_64; then
            CC+=" -target x86_64-apple-macos"
        else
            CC+=" -target arm64-apple-macos"
        fi
        CC+=" $lto -mmacosx-version-min=$with_macosx_version_min_required -isysroot $MACOSX_SDK_PATH"
        AC_MSG_RESULT([$CC])

        AC_MSG_CHECKING([what C++ compiler to use])
        CXX="`xcrun -find clang++`"
        CXX_BASE=`first_arg_basename "$CXX"`
        if test "$host_cpu" = x86_64; then
            CXX+=" -target x86_64-apple-macos"
        else
            CXX+=" -target arm64-apple-macos"
        fi
        CXX+=" $lto $stdlib -mmacosx-version-min=$with_macosx_version_min_required -isysroot $MACOSX_SDK_PATH"
        AC_MSG_RESULT([$CXX])

        INSTALL_NAME_TOOL=`xcrun -find install_name_tool`
        AR=`xcrun -find ar`
        NM=`xcrun -find nm`
        STRIP=`xcrun -find strip`
        LIBTOOL=`xcrun -find libtool`
        RANLIB=`xcrun -find ranlib`
    fi

    case "$with_macosx_version_max_allowed" in
    10.10)
        MAC_OS_X_VERSION_MAX_ALLOWED="101000"
        ;;
    10.11)
        MAC_OS_X_VERSION_MAX_ALLOWED="101100"
        ;;
    10.12)
        MAC_OS_X_VERSION_MAX_ALLOWED="101200"
        ;;
    10.13)
        MAC_OS_X_VERSION_MAX_ALLOWED="101300"
        ;;
    10.14)
        MAC_OS_X_VERSION_MAX_ALLOWED="101400"
        ;;
    10.15)
        MAC_OS_X_VERSION_MAX_ALLOWED="101500"
        ;;
    11.0)
        MAC_OS_X_VERSION_MAX_ALLOWED="110000"
        ;;
    11.1)
        MAC_OS_X_VERSION_MAX_ALLOWED="110100"
        ;;
    11.3)
        MAC_OS_X_VERSION_MAX_ALLOWED="110100"
        ;;
    12.0)
        MAC_OS_X_VERSION_MAX_ALLOWED="120000"
        ;;
    *)
        AC_MSG_ERROR([with-macosx-version-max-allowed $with_macosx_version_max_allowed is not a supported value, supported values are 10.10--12.0])
        ;;
    esac

    AC_MSG_CHECKING([that macosx-version-min-required is coherent with macosx-version-max-allowed])
    if test $MAC_OS_X_VERSION_MIN_REQUIRED -gt $MAC_OS_X_VERSION_MAX_ALLOWED; then
        AC_MSG_ERROR([the version minimum required, $MAC_OS_X_VERSION_MIN_REQUIRED, must be <= the version maximum allowed, $MAC_OS_X_VERSION_MAX_ALLOWED])
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

    if test -n "$ENABLE_JAVA" -a "$enable_macosx_sandbox" = yes; then
        AC_MSG_ERROR([macOS sandboxing (actually App Store rules) disallows use of Java])
    elif test "$enable_macosx_sandbox" = yes; then
        ENABLE_MACOSX_SANDBOX=TRUE
        AC_DEFINE(HAVE_FEATURE_MACOSX_SANDBOX)
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
    fi

    AC_MSG_CHECKING([what macOS app bundle identifier to use])
    MACOSX_BUNDLE_IDENTIFIER=$with_macosx_bundle_identifier
    AC_MSG_RESULT([$MACOSX_BUNDLE_IDENTIFIER])

    AC_MSG_CHECKING([whether the linker supports -platform_version])
    save_LDFLAGS=$LDFLAGS
    LDFLAGS="$LDFLAGS -Xlinker -platform_version -Xlinker macos \
        -Xlinker $MAC_OS_X_VERSION_MIN_REQUIRED_DOTS -Xlinker 0.0.0"
    AC_LINK_IFELSE([AC_LANG_PROGRAM()],
        [AC_MSG_RESULT([yes])
         HAVE_MACOS_LD_PLATFORMVERSION=TRUE],
        [AC_MSG_RESULT([no])])
    LDFLAGS=$save_LDFLAGS
fi
AC_SUBST(MACOSX_SDK_PATH)
AC_SUBST(MACOSX_DEPLOYMENT_TARGET)
AC_SUBST(MAC_OS_X_VERSION_MIN_REQUIRED)
AC_SUBST(MAC_OS_X_VERSION_MIN_REQUIRED_DOTS)
AC_SUBST(MAC_OS_X_VERSION_MAX_ALLOWED)
AC_SUBST(INSTALL_NAME_TOOL)
AC_SUBST(LIBTOOL) # Note that the macOS libtool command is unrelated to GNU libtool
AC_SUBST(MACOSX_CODESIGNING_IDENTITY)
AC_SUBST(MACOSX_PACKAGE_SIGNING_IDENTITY)
AC_SUBST(ENABLE_MACOSX_SANDBOX)
AC_SUBST(MACOSX_BUNDLE_IDENTIFIER)
AC_SUBST(HAVE_MACOS_LD_PLATFORMVERSION)

dnl ===================================================================
dnl Check iOS SDK and compiler
dnl ===================================================================

if test $_os = iOS; then
    AC_MSG_CHECKING([what iOS SDK to use])
    current_sdk_ver=15.0
    older_sdk_vers="14.5 14.0"
    if test "$enable_ios_simulator" = "yes"; then
        platform=iPhoneSimulator
        versionmin=-mios-simulator-version-min=12.2
    else
        platform=iPhoneOS
        versionmin=-miphoneos-version-min=12.2
    fi
    xcode_developer=`xcode-select -print-path`

    for sdkver in $current_sdk_ver $older_sdk_vers; do
        t=$xcode_developer/Platforms/$platform.platform/Developer/SDKs/$platform$sdkver.sdk
        if test -d $t; then
            sysroot=$t
            break
        fi
    done

    if test -z "$sysroot"; then
        AC_MSG_ERROR([Could not find iOS SDK, expected something like $xcode_developer/Platforms/$platform.platform/Developer/SDKs/${platform}${current_sdk_ver}.sdk])
    fi

    AC_MSG_RESULT($sysroot)

    # LTO is not really recommended for iOS builds,
    # the link time will be astronomical
    if test "$ENABLE_LTO" = TRUE; then
        lto=-flto
    fi

    stdlib="-stdlib=libc++"

    AC_MSG_CHECKING([what C compiler to use])
    CC="`xcrun -find clang`"
    CC_BASE=`first_arg_basename "$CC"`
    CC+=" -arch $host_cpu_for_clang -isysroot $sysroot $lto $versionmin"
    AC_MSG_RESULT([$CC])

    AC_MSG_CHECKING([what C++ compiler to use])
    CXX="`xcrun -find clang++`"
    CXX_BASE=`first_arg_basename "$CXX"`
    CXX+=" -arch $host_cpu_for_clang $stdlib -isysroot $sysroot $lto $versionmin"
    AC_MSG_RESULT([$CXX])

    INSTALL_NAME_TOOL=`xcrun -find install_name_tool`
    AR=`xcrun -find ar`
    NM=`xcrun -find nm`
    STRIP=`xcrun -find strip`
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
    if test "$enable_fuzzers" != yes; then
        LIBO_SHARE_RESOURCE_FOLDER=program/resource
    else
        LIBO_SHARE_RESOURCE_FOLDER=resource
    fi
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

reg_get_value()
{
    # Return value: $regvalue
    unset regvalue

    local _regentry="/proc/registry${1}/${2}"
    if test -f "$_regentry"; then
        # Stop bash complaining about \0 bytes in input, as it can't handle them.
        # Registry keys read via /proc/registry* are always \0 terminated!
        local _regvalue=$(tr -d '\0' < "$_regentry")
        if test $? -eq 0; then
            regvalue=$_regvalue
        fi
    fi
}

# Get a value from the 32-bit side of the Registry
reg_get_value_32()
{
    reg_get_value "32" "$1"
}

# Get a value from the 64-bit side of the Registry
reg_get_value_64()
{
    reg_get_value "64" "$1"
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
AC_SUBST(WINDOWS_SDK_ARCH)
if test "$_os" = "iOS" -o "$build_cpu" != "$host_cpu"; then
    cross_compiling="yes"
fi
if test "$cross_compiling" = "yes"; then
    export CROSS_COMPILING=TRUE
else
    CROSS_COMPILING=
    BUILD_TYPE="$BUILD_TYPE NATIVE"
fi
AC_SUBST(CROSS_COMPILING)

# Use -isystem (gcc) if possible, to avoid warnings in 3rd party headers.
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
dnl  Check which Visual Studio compiler is used
dnl ===================================================================

map_vs_year_to_version()
{
    # Return value: $vsversion

    unset vsversion

    case $1 in
    2017)
        vsversion=15;;
    2019)
        vsversion=16;;
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
        # We accept only 2017
        vsversions="15"
    fi
}

win_get_env_from_vsvars32bat()
{
    WRAPPERBATCHFILEPATH="`mktemp -t wrpXXXXXX.bat`"
    # Also seems to be located in another directory under the same name: vsvars32.bat
    # https://github.com/bazelbuild/bazel/blob/master/src/main/native/build_windows_jni.sh#L56-L57
    printf '@call "%s/../Common7/Tools/VsDevCmd.bat" /no_logo\r\n' "$(cygpath -w $VC_PRODUCT_DIR)" > $WRAPPERBATCHFILEPATH
    printf '@setlocal\r\n@echo %%%s%%\r\n@endlocal\r\n' "$1" >> $WRAPPERBATCHFILEPATH
    chmod +x $WRAPPERBATCHFILEPATH
    _win_get_env_from_vsvars32bat=$("$WRAPPERBATCHFILEPATH" | tr -d '\r')
    rm -f $WRAPPERBATCHFILEPATH
    printf '%s' "$_win_get_env_from_vsvars32bat"
}

find_ucrt()
{
    reg_get_value_32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v10.0/InstallationFolder"
    if test -n "$regvalue"; then
        PathFormat "$regvalue"
        UCRTSDKDIR=$formatted_path
        reg_get_value_32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v10.0/ProductVersion"
        UCRTVERSION=$regvalue
        # Rest if not exist
        if ! test -d "${UCRTSDKDIR}Include/$UCRTVERSION/ucrt"; then
          UCRTSDKDIR=
        fi
    fi
    if test -z "$UCRTSDKDIR"; then
        ide_env_dir="$VC_PRODUCT_DIR/../Common7/Tools/"
        ide_env_file="${ide_env_dir}VsDevCmd.bat"
        if test -f "$ide_env_file"; then
            PathFormat "$(win_get_env_from_vsvars32bat UniversalCRTSdkDir)"
            UCRTSDKDIR=$formatted_path
            UCRTVERSION=$(win_get_env_from_vsvars32bat UCRTVersion)
            dnl Hack needed at least by tml:
            if test "$UCRTVERSION" = 10.0.15063.0 \
                -a ! -f "${UCRTSDKDIR}Include/10.0.15063.0/um/sqlext.h" \
                -a -f "${UCRTSDKDIR}Include/10.0.14393.0/um/sqlext.h"
            then
                UCRTVERSION=10.0.14393.0
            fi
        else
          AC_MSG_ERROR([No UCRT found])
        fi
    fi
}

find_msvc()
{
    # Find Visual C++ 2017/2019
    # Args: $1 (optional) : The VS version year
    # Return values: $vctest, $vcyear, $vcnum, $vcnumwithdot, $vcbuildnumber

    unset vctest vcnum vcnumwithdot vcbuildnumber

    vs_versions_to_check "$1"
    vswhere="$(perl -e 'print $ENV{"ProgramFiles(x86)"}')"
    vswhere+="\\Microsoft Visual Studio\\Installer\\vswhere.exe"
    PathFormat "$vswhere"
    vswhere=$formatted_path
    for ver in $vsversions; do
        vswhereoutput=`$vswhere -version "@<:@ $ver , $(expr $ver + 1) @:}@" -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath | head -1`
        if test -n "$vswhereoutput"; then
            PathFormat "$vswhereoutput"
            vctest=$formatted_path
            break
        fi
    done

    # This fallback is needed on *some* (but not all) VS 2017 installations, where vswhere turns up
    # with the empty list for unclear reason. The below hack does not work for VS 2019 anyway, so
    # should be removed when upgrading baseline.
    if ! test -n "$vctest"; then
        for ver in $vsversions; do
            reg_get_value_32 HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/$ver.0/Setup/VC/ProductDir
            if test -n "$regvalue"; then
                vctest=$regvalue
                break
            fi
            reg_get_value_32 HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/SxS/VS7/$ver.0
            if test -n "$regvalue"; then
                AC_MSG_RESULT([found: $regvalue])
                PathFormat "$regvalue"
                vctest=$formatted_path
                break
            fi
        done
    fi

    if test -n "$vctest"; then
        vcnumwithdot="$ver.0"
        case "$vcnumwithdot" in
        15.0)
            vcyear=2017
            vcnum=150
            ;;
        16.0)
            vcyear=2019
            vcnum=160
            ;;
        esac
        vcbuildnumber=`ls $vctest/VC/Tools/MSVC -A1r | head -1`

    fi
}

SOLARINC=
MSBUILD_PATH=
DEVENV=
if test "$_os" = "WINNT"; then
    AC_MSG_CHECKING([Visual C++])
    find_msvc "$with_visual_studio"
    if test -z "$vctest"; then
        if test -n "$with_visual_studio"; then
            AC_MSG_ERROR([no Visual Studio $with_visual_studio installation found])
        else
            AC_MSG_ERROR([no Visual Studio 2017 installation found])
        fi
    fi

    if test "$BITNESS_OVERRIDE" = ""; then
        if test -f "$vctest/VC/Tools/MSVC/$vcbuildnumber/bin/HostX86/x86/cl.exe"; then
            VC_PRODUCT_DIR=$vctest/VC
        else
            AC_MSG_ERROR([no compiler (cl.exe) in $vctest/VC/Tools/MSVC/$vcbuildnumber/bin/HostX86/x86])
        fi
    else
        if test -f "$vctest/VC/Tools/MSVC/$vcbuildnumber/bin/HostX64/x64/cl.exe"; then
            VC_PRODUCT_DIR=$vctest/VC
        else
            AC_MSG_ERROR([no compiler (cl.exe) in $vctest/VC/Tools/MSVC/$vcbuildnumber/bin/HostX64/x64])
        fi
    fi
    AC_MSG_RESULT([$VC_PRODUCT_DIR])

    AC_MSG_CHECKING([for short pathname of VC product directory])
    VC_PRODUCT_DIR=`win_short_path_for_make "$VC_PRODUCT_DIR"`
    AC_MSG_RESULT([$VC_PRODUCT_DIR])

    UCRTSDKDIR=
    UCRTVERSION=

    AC_MSG_CHECKING([for UCRT location])
    find_ucrt
    # find_ucrt errors out if it doesn't find it
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

    AC_SUBST(UCRTSDKDIR)
    AC_SUBST(UCRTVERSION)

    AC_MSG_CHECKING([for MSBuild.exe location for: $vcnumwithdot])
    # Find the proper version of MSBuild.exe to use based on the VS version
    reg_get_value_32 HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/MSBuild/$vcnumwithdot/MSBuildOverrideTasksPath
    if test -n "$regvalue" ; then
        AC_MSG_RESULT([found: $regvalue])
        MSBUILD_PATH=`win_short_path_for_make "$regvalue"`
    else
        if test "$vcnumwithdot" = "16.0"; then
            if test "$BITNESS_OVERRIDE" = ""; then
                regvalue="$VC_PRODUCT_DIR/../MSBuild/Current/Bin"
            else
                regvalue="$VC_PRODUCT_DIR/../MSBuild/Current/Bin/amd64"
            fi
        else
            if test "$BITNESS_OVERRIDE" = ""; then
                regvalue="$VC_PRODUCT_DIR/../MSBuild/$vcnumwithdot/Bin"
            else
                regvalue="$VC_PRODUCT_DIR/../MSBuild/$vcnumwithdot/Bin/amd64"
            fi
        fi
        MSBUILD_PATH=`win_short_path_for_make "$regvalue"`
        AC_MSG_RESULT([$regvalue])
    fi

    # Find the version of devenv.exe
    # MSVC 2017 devenv does not start properly from a DOS 8.3 path
    DEVENV=$(cygpath -lm "$VC_PRODUCT_DIR/../Common7/IDE/devenv.exe")
    if test ! -e "$DEVENV"; then
        AC_MSG_ERROR([No devenv.exe found, Visual Studio installation broken?])
    fi

    dnl ===========================================================
    dnl  Check for the corresponding mspdb*.dll
    dnl ===========================================================

    VC_HOST_DIR=
    MSPDB_PATH=
    CL_PATH=

    if "$VC_PRODUCT_DIR/Tools/MSVC/$vcbuildnumber/bin/HostX64/x64/cl.exe" -? </dev/null >/dev/null 2>&1; then
        VC_HOST_DIR="HostX64"
        MSPDB_PATH="$VC_PRODUCT_DIR/Tools/MSVC/$vcbuildnumber/bin/$VC_HOST_DIR/x64"
    else
        VC_HOST_DIR="HostX86"
        MSPDB_PATH="$VC_PRODUCT_DIR/Tools/MSVC/$vcbuildnumber/bin/$VC_HOST_DIR/x86"
    fi

    if test "$BITNESS_OVERRIDE" = ""; then
        CL_PATH="$VC_PRODUCT_DIR/Tools/MSVC/$vcbuildnumber/bin/$VC_HOST_DIR/x86"
    else
        CL_PATH="$VC_PRODUCT_DIR/Tools/MSVC/$vcbuildnumber/bin/$VC_HOST_DIR/x64"
    fi

    # MSVC 15.0 has libraries from 14.0?
    mspdbnum="140"

    if test ! -e "$MSPDB_PATH/mspdb${mspdbnum}.dll"; then
        AC_MSG_ERROR([No mspdb${mspdbnum}.dll in $MSPDB_PATH, Visual Studio installation broken?])
    fi

    dnl The path needs to be added before cl is called
    TEMP_PATH=`cygpath -d "$MSPDB_PATH"`
    PATH="`cygpath -u "$TEMP_PATH"`:$PATH"

    AC_MSG_CHECKING([cl.exe])

    # Is there really ever a need to pass CC explicitly? Surely we can hope to get all the
    # automagical niceness to work OK? If somebody has some unsupported compiler in some weird
    # location, isn't it likely that lots of other things needs changes, too, and just setting CC
    # is not enough?

    dnl Save the true MSVC cl.exe for use when CC/CXX is actually clang-cl,
    dnl needed when building CLR code:
    if test -z "$MSVC_CXX"; then
        if test -f "$CL_PATH/cl.exe"; then
            MSVC_CXX="$CL_PATH/cl.exe"
        fi

        # This gives us a posix path with 8.3 filename restrictions
        MSVC_CXX=`win_short_path_for_make "$MSVC_CXX"`
    fi

    if test -z "$CC"; then
        CC=$MSVC_CXX
        CC_BASE=`first_arg_basename "$CC"`
    fi
    if test "$BITNESS_OVERRIDE" = ""; then
        dnl since MSVC 2012, default for x86 is -arch:SSE2:
        MSVC_CXX="$MSVC_CXX -arch:SSE"
    fi
    if test -z "$CXX"; then
        CXX=$MSVC_CXX
        CXX_BASE=`first_arg_basename "$CXX"`
    fi

    if test -n "$CC"; then
        # Remove /cl.exe from CC case insensitive
        AC_MSG_RESULT([found Visual C++ $vcyear ($CC)])
        if test "$BITNESS_OVERRIDE" = ""; then
           COMPATH="$VC_PRODUCT_DIR"
        else
            if test -n "$VC_PRODUCT_DIR"; then
                COMPATH=$VC_PRODUCT_DIR
            fi
        fi
        if test "$BITNESS_OVERRIDE" = ""; then
            dnl since MSVC 2012, default for x86 is -arch:SSE2:
            CC="$CC -arch:SSE"
        fi

        COMPATH="$COMPATH/Tools/MSVC/$vcbuildnumber"

        export INCLUDE=`cygpath -d "$COMPATH\Include"`

        PathFormat "$COMPATH"
        COMPATH=`win_short_path_for_make "$formatted_path"`

        VCVER=$vcnum
        MSVSVER=$vcyear

        # The WINDOWS_SDK_ACCEPTABLE_VERSIONS is mostly an educated guess...  Assuming newer ones
        # are always "better", we list them in reverse chronological order.

        case "$vcnum" in
        150|160)
            WINDOWS_SDK_ACCEPTABLE_VERSIONS="10.0 8.1A 8.1 8.0"
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

    AC_MSG_CHECKING([$CC_BASE is at least Visual Studio 2017 version 15.7])
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
        // See <https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros> for mapping
        // between Visual Studio versions and _MSC_VER:
        #if _MSC_VER < 1914
        #error
        #endif
    ]])],[AC_MSG_RESULT([yes])],[AC_MSG_ERROR([no])])

    # Check for 64-bit (cross-)compiler to use to build the 64-bit
    # version of the Explorer extension (and maybe other small
    # bits, too) needed when installing a 32-bit LibreOffice on a
    # 64-bit OS. The 64-bit Explorer extension is a feature that
    # has been present since long in OOo. Don't confuse it with
    # building LibreOffice itself as 64-bit code.

    BUILD_X64=
    CXX_X64_BINARY=

    if test "$BITNESS_OVERRIDE" = ""; then
        AC_MSG_CHECKING([for a x64 compiler and libraries for 64-bit Explorer extensions])
        if test -f "$VC_PRODUCT_DIR/Tools/MSVC/$vcbuildnumber/atlmfc/lib/x64/atls.lib" || \
             test -f "$VC_PRODUCT_DIR/Tools/MSVC/$vcbuildnumber/atlmfc/lib/spectre/x64/atls.lib"; then
            if "$VC_PRODUCT_DIR/Tools/MSVC/$vcbuildnumber/bin/$VC_HOST_DIR/x64/cl.exe" -? </dev/null >/dev/null 2>&1; then
                BUILD_X64=TRUE
                CXX_X64_BINARY=`win_short_path_for_make "$VC_PRODUCT_DIR/Tools/MSVC/$vcbuildnumber/bin/$VC_HOST_DIR/x64/cl.exe"`
            fi
        fi
        if test "$BUILD_X64" = TRUE; then
            AC_MSG_RESULT([found])
        else
            AC_MSG_RESULT([not found])
            AC_MSG_WARN([Installation set will not contain 64-bit Explorer extensions])
        fi
    else
        CXX_X64_BINARY=$CXX
    fi
    AC_SUBST(BUILD_X64)

    # These are passed to the environment and then used in gbuild/platform/com_MSC_class.mk
    AC_SUBST(CXX_X64_BINARY)

    # Check for 32-bit compiler to use to build the 32-bit TWAIN shim
    # needed to support TWAIN scan on both 32- and 64-bit systems

    BUILD_X86=

    if test "$BITNESS_OVERRIDE" = "64"; then
        AC_MSG_CHECKING([for a x86 compiler and libraries for 32-bit binaries required for TWAIN support])
        if test -n "$CXX_X86_BINARY"; then
            BUILD_X86=TRUE
            AC_MSG_RESULT([preset])
        elif "$VC_PRODUCT_DIR/Tools/MSVC/$vcbuildnumber/bin/$VC_HOST_DIR/x86/cl.exe" -? </dev/null >/dev/null 2>&1; then
            BUILD_X86=TRUE
            CXX_X86_BINARY=`win_short_path_for_make "$VC_PRODUCT_DIR/Tools/MSVC/$vcbuildnumber/bin/$VC_HOST_DIR/x86/cl.exe"`
            CXX_X86_BINARY+=" /arch:SSE"
            AC_MSG_RESULT([found])
        else
            CXX_X86_BINARY=
            AC_MSG_RESULT([not found])
            AC_MSG_WARN([Installation set will not contain 32-bit binaries required for TWAIN support])
        fi
    else
        BUILD_X86=TRUE
        CXX_X86_BINARY=$MSVC_CXX
    fi
    AC_SUBST(BUILD_X86)
    AC_SUBST(CXX_X86_BINARY)
fi
AC_SUBST(VCVER)
AC_SUBST(DEVENV)
AC_SUBST(MSVC_CXX)

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
AC_SUBST(COM_IS_CLANG)

CC_PLAIN=$CC
CLANGVER=
if test "$COM_IS_CLANG" = TRUE; then
    AC_MSG_CHECKING([whether Clang is new enough])
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
        #if !defined __apple_build_version__
        #error
        #endif
        ]])],
        [my_apple_clang=yes],[my_apple_clang=])
    if test "$my_apple_clang" = yes; then
        AC_MSG_RESULT([assumed yes (Apple Clang)])
    else
        if test "$_os" = WINNT; then
            dnl In which case, assume clang-cl:
            my_args="/EP /TC"
            dnl Filter out -FIIntrin.h, which needs to be explicitly stated for
            dnl clang-cl:
            CC_PLAIN=
            for i in $CC; do
                case $i in
                -FIIntrin.h)
                    ;;
                *)
                    CC_PLAIN="$CC_PLAIN $i"
                    ;;
                esac
            done
        else
            my_args="-E -P"
        fi
        clang_version=`echo __clang_major__.__clang_minor__.__clang_patchlevel__ | $CC_PLAIN $my_args - | sed 's/ //g'`
        CLANG_FULL_VERSION=`echo __clang_version__ | $CC_PLAIN $my_args -`
        CLANGVER=`echo $clang_version \
            | $AWK -F. '{ print \$1*10000+(\$2<100?\$2:99)*100+(\$3<100?\$3:99) }'`
        if test "$CLANGVER" -ge 50002; then
            AC_MSG_RESULT([yes ($clang_version)])
        else
            AC_MSG_ERROR(["$CLANG_FULL_VERSION" is too old or unrecognized, must be at least Clang 5.0.2])
        fi
        AC_DEFINE_UNQUOTED(CLANG_VERSION,$CLANGVER)
        AC_DEFINE_UNQUOTED(CLANG_FULL_VERSION,$CLANG_FULL_VERSION)
    fi
fi

SHOWINCLUDES_PREFIX=
if test "$_os" = WINNT; then
    dnl We need to guess the prefix of the -showIncludes output, it can be
    dnl localized
    AC_MSG_CHECKING([the dependency generation prefix (cl.exe -showIncludes)])
    echo "#include <stdlib.h>" > conftest.c
    SHOWINCLUDES_PREFIX=`$CC_PLAIN $CFLAGS -c -showIncludes conftest.c 2>/dev/null | \
        grep 'stdlib\.h' | head -n1 | sed 's/ [[[:alpha:]]]:.*//'`
    rm -f conftest.c conftest.obj
    if test -z "$SHOWINCLUDES_PREFIX"; then
        AC_MSG_ERROR([cannot determine the -showIncludes prefix])
    else
        AC_MSG_RESULT(["$SHOWINCLUDES_PREFIX"])
    fi
fi
AC_SUBST(SHOWINCLUDES_PREFIX)

#
# prefix C with ccache if needed
#
if test "$CCACHE" != ""; then
    AC_MSG_CHECKING([whether $CC_BASE is already ccached])

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
        CC_BASE="ccache $CC_BASE"
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
if test "$GCC" = "yes"; then
    AC_MSG_CHECKING([whether $CC_BASE supports -ggdb2])
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -Werror -ggdb2"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[ return 0; ]])],[ HAVE_GCC_GGDB2=TRUE ],[])
    CFLAGS=$save_CFLAGS
    if test "$HAVE_GCC_GGDB2" = "TRUE"; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
    fi

    if test "$host_cpu" = "m68k"; then
        AC_MSG_CHECKING([whether $CC_BASE supports -mlong-jump-table-offsets])
        save_CFLAGS=$CFLAGS
        CFLAGS="$CFLAGS -Werror -mlong-jump-table-offsets"
        AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[ return 0; ]])],[ HAVE_GCC_LONG_JUMP_TABLE_OFFSETS=TRUE ],[])
        CFLAGS=$save_CFLAGS
        if test "$HAVE_GCC_LONG_JUMP_TABLE_OFFSETS" = "TRUE"; then
            AC_MSG_RESULT([yes])
        else
            AC_MSG_ERROR([no])
        fi
    fi
fi
AC_SUBST(HAVE_GCC_GGDB2)

dnl ===================================================================
dnl  Test the gcc version
dnl ===================================================================
if test "$GCC" = "yes" -a -z "$COM_IS_CLANG"; then
    AC_MSG_CHECKING([the GCC version])
    _gcc_version=`$CC -dumpversion`
    gcc_full_version=$(printf '%s' "$_gcc_version" | \
        $AWK -F. '{ print $1*10000+$2*100+(NF<3?1:$3) }')
    GCC_VERSION=`echo $_gcc_version | $AWK -F. '{ print \$1*100+\$2 }'`

    AC_MSG_RESULT([gcc $_gcc_version ($gcc_full_version)])

    if test "$gcc_full_version" -lt 70000; then
        AC_MSG_ERROR([GCC $_gcc_version is too old, must be at least GCC 7.0.0])
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
    if test "$with_system_hunspell" = "yes"; then
        AC_MSG_ERROR([--with-system-hunspell conflicts with --enable-dbgutil])
    else
        with_system_hunspell=no
    fi
    if test "$with_system_gpgmepp" = "yes"; then
        AC_MSG_ERROR([--with-system-gpgmepp conflicts with --enable-dbgutil])
    else
        with_system_gpgmepp=no
    fi
    # As mixing system libwps and non-system libnumbertext or vice versa likely causes trouble (see
    # 603074c5f2b84de8a24593faf807da784b040625 "Pass _GLIBCXX_DEBUG into external/libwps" and the
    # mail thread starting at <https://gcc.gnu.org/ml/gcc/2018-05/msg00057.html> "libstdc++: ODR
    # violation when using std::regex with and without -D_GLIBCXX_DEBUG"), simply make sure neither
    # of those two is using the system variant:
    if test "$with_system_libnumbertext" = "yes"; then
        AC_MSG_ERROR([--with-system-libnumbertext conflicts with --enable-dbgutil])
    else
        with_system_libnumbertext=no
    fi
    if test "$with_system_libwps" = "yes"; then
        AC_MSG_ERROR([--with-system-libwps conflicts with --enable-dbgutil])
    else
        with_system_libwps=no
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
if test -n "$enable_debug" && test "$enable_debug" != "yes" && test "$enable_debug" != "no"; then
    AC_MSG_ERROR([--enable-debug now accepts only yes or no, use --enable-symbols])
fi
if test -n "$ENABLE_DBGUTIL" -a "$enable_debug" = "no"; then
    if test -z "$libo_fuzzed_enable_debug"; then
        AC_MSG_ERROR([--disable-debug cannot be used with --enable-dbgutil])
    else
        AC_MSG_NOTICE([Resetting --enable-debug=yes])
        enable_debug=yes
    fi
fi

AC_MSG_CHECKING([whether to do a debug build])
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

dnl ===================================================================
dnl Select the linker to use (gold/lld/ld.bfd).
dnl This is done only after compiler checks (need to know if Clang is
dnl used, for different defaults) and after checking if a debug build
dnl is wanted (non-debug builds get the default linker if not explicitly
dnl specified otherwise).
dnl All checks for linker features/options should come after this.
dnl ===================================================================
check_use_ld()
{
    use_ld=$1
    use_ld_fail_if_error=$2
    use_ld_ok=
    AC_MSG_CHECKING([for -fuse-ld=$use_ld linker support])
    use_ld_ldflags_save="$LDFLAGS"
    LDFLAGS="$LDFLAGS -fuse-ld=$use_ld"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([
#include <stdio.h>
        ],[
printf ("hello world\n");
        ])], USE_LD=$use_ld, [])
    if test -n "$USE_LD"; then
        AC_MSG_RESULT( yes )
        use_ld_ok=yes
    else
        if test -n "$use_ld_fail_if_error"; then
            AC_MSG_ERROR( no )
        else
            AC_MSG_RESULT( no )
        fi
    fi
    if test -n "$use_ld_ok"; then
        dnl keep the value of LDFLAGS
        return 0
    fi
    LDFLAGS="$use_ld_ldflags_save"
    return 1
}
USE_LD=
if test "$enable_ld" != "no"; then
    if test "$GCC" = "yes"; then
        if test -n "$enable_ld"; then
            check_use_ld "$enable_ld" fail_if_error
        elif test -z "$ENABLE_DEBUG$ENABLE_DBGUTIL"; then
            dnl non-debug builds default to the default linker
            true
        elif test -n "$COM_IS_CLANG"; then
            check_use_ld lld
            if test $? -ne 0; then
                check_use_ld gold
            fi
        else
            # For gcc first try gold, new versions also support lld.
            check_use_ld gold
            if test $? -ne 0; then
                check_use_ld lld
            fi
        fi
        ld_output=$(echo 'int main() { return 0; }' | $CC -Wl,-v -x c -o conftest.out - $CFLAGS $LDFLAGS 2>/dev/null)
        rm conftest.out
        ld_used=$(echo "$ld_output" | grep -E '(^GNU gold|^GNU ld|^LLD)')
        if test -z "$ld_used"; then
            ld_used="unknown"
        fi
        AC_MSG_CHECKING([for linker that is used])
        AC_MSG_RESULT([$ld_used])
        if test -n "$ENABLE_DEBUG$ENABLE_DBGUTIL"; then
            if echo "$ld_used" | grep -q "^GNU ld"; then
                AC_MSG_WARN([The default GNU linker is slow, consider using the LLD or the GNU gold linker.])
                add_warning "The default GNU linker is slow, consider using the LLD or the GNU gold linker."
            fi
        fi
    else
        if test "$enable_ld" = "yes"; then
            AC_MSG_ERROR([--enable-ld not supported])
        fi
    fi
fi
AC_SUBST(USE_LD)

HAVE_LD_BSYMBOLIC_FUNCTIONS=
if test "$GCC" = "yes"; then
    AC_MSG_CHECKING([for -Bsymbolic-functions linker support])
    bsymbolic_functions_ldflags_save=$LDFLAGS
    LDFLAGS="$LDFLAGS -Wl,-Bsymbolic-functions"
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


HAVE_GSPLIT_DWARF=
if test "$enable_split_debug" != no; then
    dnl Currently by default enabled only on Linux, feel free to set test_split_debug above also for other platforms.
    if test "$enable_split_debug" = yes -o \( "$test_split_debug" = "yes" -a -n "$ENABLE_DEBUG$ENABLE_DBGUTIL" \); then
        AC_MSG_CHECKING([whether $CC_BASE supports -gsplit-dwarf])
        save_CFLAGS=$CFLAGS
        CFLAGS="$CFLAGS -Werror -gsplit-dwarf"
        AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[ return 0; ]])],[ HAVE_GCC_SPLIT_DWARF=TRUE ],[])
        CFLAGS=$save_CFLAGS
        if test "$HAVE_GCC_SPLIT_DWARF" = "TRUE"; then
            AC_MSG_RESULT([yes])
        else
            if test "$enable_split_debug" = yes; then
                AC_MSG_ERROR([no])
            else
                AC_MSG_RESULT([no])
            fi
        fi
    fi
    if test -z "$HAVE_GCC_SPLIT_DWARF" -a "$test_split_debug" = "yes" -a -n "$ENABLE_DEBUG$ENABLE_DBGUTIL"; then
        AC_MSG_WARN([Compiler is not capable of creating split debug info, linking will require more time and disk space.])
        add_warning "Compiler is not capable of creating split debug info, linking will require more time and disk space."
    fi
fi
AC_SUBST(HAVE_GCC_SPLIT_DWARF)

ENABLE_GDB_INDEX=
if test "$enable_gdb_index" != "no"; then
    dnl Currently by default enabled only on Linux, feel free to set test_gdb_index above also for other platforms.
    if test "$enable_gdb_index" = yes -o \( "$test_gdb_index" = "yes" -o -n "$ENABLE_DEBUG$ENABLE_DBGUTIL" \); then
        AC_MSG_CHECKING([whether $CC_BASE supports -ggnu-pubnames])
        save_CFLAGS=$CFLAGS
        CFLAGS="$CFLAGS -Werror -ggnu-pubnames"
        have_ggnu_pubnames=
        AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[ return 0; ]])],[have_ggnu_pubnames=TRUE],[have_ggnu_pubnames=])
        if test "$have_ggnu_pubnames" != "TRUE"; then
            if test "$enable_gdb_index" = "yes"; then
                AC_MSG_ERROR( no, --enable-gdb-index not supported )
            else
                AC_MSG_RESULT( no )
            fi
        else
            AC_MSG_RESULT( yes )
            AC_MSG_CHECKING([whether $CC_BASE supports -Wl,--gdb-index])
            ldflags_save=$LDFLAGS
            LDFLAGS="$LDFLAGS -Wl,--gdb-index"
            AC_LINK_IFELSE([AC_LANG_PROGRAM([
#include <stdio.h>
                ],[
printf ("hello world\n");
                ])], ENABLE_GDB_INDEX=TRUE, [])
            if test "$ENABLE_GDB_INDEX" = "TRUE"; then
                AC_MSG_RESULT( yes )
            else
                if test "$enable_gdb_index" = "yes"; then
                    AC_MSG_ERROR( no )
                else
                    AC_MSG_RESULT( no )
                fi
            fi
            LDFLAGS=$ldflags_save
        fi
        CFLAGS=$save_CFLAGS
        fi
    if test -z "$ENABLE_GDB_INDEX" -a "$test_gdb_index" = "yes" -a -n "$ENABLE_DEBUG$ENABLE_DBGUTIL"; then
        AC_MSG_WARN([Linker is not capable of creating gdb index, debugger startup will be slow.])
        add_warning "Linker is not capable of creating gdb index, debugger startup will be slow."
    fi
fi
AC_SUBST(ENABLE_GDB_INDEX)

if test "$enable_sal_log" = yes; then
    ENABLE_SAL_LOG=TRUE
fi
AC_SUBST(ENABLE_SAL_LOG)

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
if test "$enable_symbols" = yes; then
    ENABLE_SYMBOLS_FOR=all
    AC_MSG_RESULT([yes])
elif test "$enable_symbols" = no; then
    ENABLE_SYMBOLS_FOR=
    AC_MSG_RESULT([no])
else
    # Selective debuginfo.
    ENABLE_SYMBOLS_FOR="$enable_symbols"
    AC_MSG_RESULT([for "$enable_symbols"])
fi
AC_SUBST(ENABLE_SYMBOLS_FOR)

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

AC_MSG_CHECKING([whether to compile with optimization flags])
if test -z "$enable_optimized"; then
    if test -n "$ENABLE_DEBUG$ENABLE_DBGUTIL"; then
        enable_optimized=no
    else
        enable_optimized=yes
    fi
fi
if test "$enable_optimized" != no; then
    ENABLE_OPTIMIZED=TRUE
    AC_MSG_RESULT([yes])
else
    ENABLE_OPTIMIZED=
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_OPTIMIZED)

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

darwin*|macos*)
    COM=GCC
    USING_X11=
    OS=MACOSX
    RTL_OS=MacOSX
    P_SEP=:

    case "$host_cpu" in
    aarch64|arm64)
        if test "$enable_ios_simulator" = "yes"; then
            OS=iOS
        else
            CPUNAME=AARCH64
            RTL_ARCH=AARCH64
            PLATFORMID=macosx_arm64
        fi
        ;;
    x86_64)
        if test "$enable_ios_simulator" = "yes"; then
            OS=iOS
        fi
        CPUNAME=X86_64
        RTL_ARCH=X86_64
        PLATFORMID=macosx_x86_64
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    ;;

ios*)
    COM=GCC
    USING_X11=
    OS=iOS
    RTL_OS=iOS
    P_SEP=:

    case "$host_cpu" in
    aarch64|arm64)
        if test "$enable_ios_simulator" = "yes"; then
            AC_MSG_ERROR([iOS simulator is only available in macOS not iOS])
        fi
        ;;
    *)
        AC_MSG_ERROR([Unsupported host_cpu $host_cpu for host_os $host_os])
        ;;
    esac
    CPUNAME=AARCH64
    RTL_ARCH=AARCH64
    PLATFORMID=ios_arm64
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

haiku*)
    COM=GCC
    USING_X11=
    GUIBASE=haiku
    RTL_OS=Haiku
    OS=HAIKU
    P_SEP=:

    case "$host_cpu" in
    i*86)
        CPUNAME=INTEL
        RTL_ARCH=x86
        PLATFORMID=haiku_x86
        ;;
    x86_64|amd64)
        CPUNAME=X86_64
        RTL_ARCH=X86_64
        PLATFORMID=haiku_x86_64
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
        EPM_FLAGS="-a arm64"
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
    sparc64)
        CPUNAME=SPARC64
        RTL_ARCH=SPARC64
        PLATFORMID=linux_sparc64
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
    i*86)
        CPUNAME=INTEL
        RTL_ARCH=x86
        PLATFORMID=android_x86
        ;;
    x86_64)
        CPUNAME=X86_64
        RTL_ARCH=X86_64
        PLATFORMID=android_x86_64
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

DISABLE_GUI=""
if test "$enable_gui" = "no"; then
    if test "$USING_X11" != TRUE; then
        AC_MSG_ERROR([$host_os operating system is not suitable to build LibreOffice with --disable-gui.])
    fi
    USING_X11=
    DISABLE_GUI=TRUE
    AC_DEFINE(HAVE_FEATURE_UI,0)
    test_cairo=yes
fi
AC_SUBST(DISABLE_GUI)

WORKDIR="${BUILDDIR}/workdir"
INSTDIR="${BUILDDIR}/instdir"
INSTROOTBASE=${INSTDIR}${INSTROOTBASESUFFIX}
INSTROOT=${INSTROOTBASE}${INSTROOTCONTENTSUFFIX}
SOLARINC="-I$SRC_ROOT/include $SOLARINC"
AC_SUBST(COM)
AC_SUBST(CPUNAME)
AC_SUBST(RTL_OS)
AC_SUBST(RTL_ARCH)
AC_SUBST(EPM_FLAGS)
AC_SUBST(USING_X11)
AC_SUBST([INSTDIR])
AC_SUBST([INSTROOT])
AC_SUBST([INSTROOTBASE])
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
dmg - macOS .dmg
installed - installation tree
msi - Windows .msi
        ])
            ;;
        esac
    done
    # fakeroot is needed to ensure correct file ownerships/permissions
    # inside deb packages and tar archives created on Linux and Solaris.
    if test "$OS" = "LINUX" || test "$OS" = "SOLARIS"; then
        AC_PATH_PROG(FAKEROOT, fakeroot, no)
        if test "$FAKEROOT" = "no"; then
            AC_MSG_ERROR(
                [--with-package-format='$with_package_format' requires fakeroot. Install fakeroot.])
        fi
    fi
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
        download.lst \
        config_host.mk.in \
        config_host_lang.mk.in \
        Makefile.in \
        bin/bffvalidator.sh.in \
        bin/odfvalidator.sh.in \
        bin/officeotron.sh.in \
        instsetoo_native/util/openoffice.lst.in \
        setup_native/source/packinfo/finals_instsetoo.txt.in \
        config_host/*.in \
        sysui/desktop/macosx/Info.plist.in) \
    | (cd CONF-FOR-BUILD && tar xf -)
    cp configure CONF-FOR-BUILD
    test -d config_build && cp -p config_build/*.h CONF-FOR-BUILD/config_host 2>/dev/null
    (
    unset COM USING_X11 OS CPUNAME
    unset CC CXX SYSBASE CFLAGS
    unset AR NM OBJDUMP PKG_CONFIG RANLIB READELF STRIP
    unset CPPUNIT_CFLAGS CPPUNIT_LIBS
    unset LIBXML_CFLAGS LIBXML_LIBS LIBXSLT_CFLAGS LIBXSLT_LIBS XSLTPROC PKG_CONFIG_LIBDIR
    if test -n "$CC_FOR_BUILD"; then
        export CC="$CC_FOR_BUILD"
        CC_BASE=`first_arg_basename "$CC"`
    fi
    if test -n "$CXX_FOR_BUILD"; then
        export CXX="$CXX_FOR_BUILD"
        CXX_BASE=`first_arg_basename "$CXX"`
    fi
    test -n "$PKG_CONFIG_FOR_BUILD" && export PKG_CONFIG="$PKG_CONFIG_FOR_BUILD"
    cd CONF-FOR-BUILD
    sub_conf_opts=""
    test -n "$enable_ccache" && sub_conf_opts="$sub_conf_opts --enable-ccache=$enable_ccache"
    test -n "$with_ant_home" && sub_conf_opts="$sub_conf_opts --with-ant-home=$with_ant_home"
    test $with_junit = no && sub_conf_opts="$sub_conf_opts --without-junit"
    test -n "$TARFILE_LOCATION" && sub_conf_opts="$sub_conf_opts --with-external-tar=$TARFILE_LOCATION"
    test "$with_system_icu_for_build" = "yes" -o "$with_system_icu_for_build" = "force" && sub_conf_opts="$sub_conf_opts --with-system-icu"
    test "$build_for_ios" = "YES" && sub_conf_opts="$sub_conf_opts build_for_ios=YES"
    sub_conf_opts="$sub_conf_opts $with_build_platform_configure_options"
    # Don't bother having configure look for stuff not needed for the build platform anyway
    ./configure \
        --disable-cups \
        --disable-gstreamer-1-0 \
        --disable-gtk3 \
        --disable-pdfimport \
        --disable-postgresql-sdbc \
        --enable-icecream="$enable_icecream" \
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
    . ./bin/get_config_variables CC CXX INSTDIR INSTROOT LIBO_BIN_FOLDER LIBO_LIB_FOLDER LIBO_URE_LIB_FOLDER LIBO_URE_MISC_FOLDER OS PATH SDKDIRNAME SYSTEM_LIBXML SYSTEM_LIBXSLT WORKDIR

    for V in CC CXX LIBO_BIN_FOLDER LIBO_LIB_FOLDER LIBO_URE_LIB_FOLDER LIBO_URE_MISC_FOLDER OS SDKDIRNAME SYSTEM_LIBXML SYSTEM_LIBXSLT; do
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
    LIBO_URE_MISC_FOLDER_FOR_BUILD="$LIBO_URE_MISC_FOLDER"
    SDKDIRNAME_FOR_BUILD="$SDKDIRNAME"
    WORKDIR_FOR_BUILD="$WORKDIR"
fi
AC_SUBST(OS_FOR_BUILD)
AC_SUBST(INSTDIR_FOR_BUILD)
AC_SUBST(INSTROOT_FOR_BUILD)
AC_SUBST(LIBO_BIN_FOLDER_FOR_BUILD)
AC_SUBST(LIBO_LIB_FOLDER_FOR_BUILD)
AC_SUBST(LIBO_URE_LIB_FOLDER_FOR_BUILD)
AC_SUBST(LIBO_URE_MISC_FOLDER_FOR_BUILD)
AC_SUBST(SDKDIRNAME_FOR_BUILD)
AC_SUBST(WORKDIR_FOR_BUILD)

dnl ===================================================================
dnl Check for syslog header
dnl ===================================================================
AC_CHECK_HEADER(syslog.h, AC_DEFINE(HAVE_SYSLOG_H))

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

dnl Check for --enable-assert-always-abort, set ASSERT_ALWAYS_ABORT
dnl ===================================================================
AC_MSG_CHECKING([whether to have assert() failures abort even without --enable-debug])
if test -z "$enable_assert_always_abort"; then
   if test "$ENABLE_DEBUG" = TRUE; then
       enable_assert_always_abort=yes
   else
       enable_assert_always_abort=no
   fi
fi
if test "$enable_assert_always_abort" = "yes"; then
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
    build_gstreamer_1_0=no
    test_kf5=no
    test_qt5=no
    test_gtk3_kde5=no
    enable_cairo_canvas=no
fi

if test "$OS" = "HAIKU"; then
    enable_cairo_canvas=yes
    test_kf5=yes
fi

if test "$test_kf5" = "yes" -a "$enable_kde5" = "yes"; then
    AC_MSG_WARN([The kde5 VCL plugin was renamed to kf5. Please update your configuration to use --enable-kf5, as --enable-kde5 will be removed after the next major release!])
    add_warning "The kde5 VCL plugin was renamed to kf5. Please update your configuration to use --enable-kf5, as --enable-kde5 will be removed after the next major release!"
    enable_kf5=yes
fi

if test "$test_kf5" = "yes"; then
    test_qt5=yes
fi

if test "$test_kf5" = "yes" -a "$enable_kf5" = "yes"; then
    if test "$enable_qt5" = "no"; then
        AC_MSG_ERROR([KF5 support depends on QT5, so it conflicts with --disable-qt5])
    else
        enable_qt5=yes
    fi
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
        AC_MSG_RESULT([yes, all of them])
        DO_FETCH_TARBALLS="ALL"
    else
        AC_MSG_RESULT([yes, if we use them])
        DO_FETCH_TARBALLS="TRUE"
    fi
else
    AC_MSG_RESULT([no])
    DO_FETCH_TARBALLS=
fi
AC_SUBST(DO_FETCH_TARBALLS)

AC_MSG_CHECKING([whether to build help])
if test -n "$with_help" -a "$with_help" != "no" -a $_os != iOS -a $_os != Android; then
    BUILD_TYPE="$BUILD_TYPE HELP"
    GIT_NEEDED_SUBMODULES="helpcontent2 $GIT_NEEDED_SUBMODULES"
    case "$with_help" in
    "html")
        ENABLE_HTMLHELP=TRUE
        SCPDEFS="$SCPDEFS -DWITH_HELP"
        AC_MSG_RESULT([HTML])
        ;;
    "online")
        ENABLE_HTMLHELP=TRUE
        HELP_ONLINE=TRUE
        AC_MSG_RESULT([HTML])
        ;;
    yes)
        SCPDEFS="$SCPDEFS -DWITH_HELP"
        AC_MSG_RESULT([yes])
        ;;
    *)
        AC_MSG_ERROR([Unknown --with-help=$with_help])
        ;;
    esac
else
    AC_MSG_RESULT([no])
fi
AC_SUBST([ENABLE_HTMLHELP])
AC_SUBST([HELP_ONLINE])

AC_MSG_CHECKING([whether to enable xapian-omega support for help])
if test -n "$with_omindex" -a "$with_omindex" != "no" -a $_os != iOS -a $_os != Android; then
    BUILD_TYPE="$BUILD_TYPE HELP"
    GIT_NEEDED_SUBMODULES="helpcontent2 $GIT_NEEDED_SUBMODULES"
    case "$with_omindex" in
    "server")
        ENABLE_HTMLHELP=TRUE
        HELP_ONLINE=TRUE
        HELP_OMINDEX_PAGE=TRUE
        AC_MSG_RESULT([SERVER])
        ;;
    "noxap")
        ENABLE_HTMLHELP=TRUE
        HELP_ONLINE=TRUE
        HELP_OMINDEX_PAGE=FALSE
        AC_MSG_RESULT([NOXAP])
        ;;
    *)
        AC_MSG_ERROR([Unknown --with-omindex=$with_omindex])
        ;;
    esac
else
    HELP_OMINDEX_PAGE=FALSE
    AC_MSG_RESULT([no])
fi
AC_SUBST([ENABLE_HTMLHELP])
AC_SUBST([HELP_OMINDEX_PAGE])
AC_SUBST([HELP_ONLINE])


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

# There are no "system" myspell, hyphen or mythes dictionaries on macOS, Windows, Android or iOS.
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
dnl Precompiled headers.
ENABLE_PCH=""
AC_MSG_CHECKING([whether to enable pch feature])
if test -z "$enable_pch"; then
    if test "$_os" = "WINNT"; then
        # Enabled by default on Windows.
        enable_pch=yes
    else
        enable_pch=no
    fi
fi
if test "$enable_pch" != "no" -a "$_os" != "WINNT" -a "$GCC" != "yes" ; then
    AC_MSG_ERROR([Precompiled header not yet supported for your platform/compiler])
fi
if test "$enable_pch" = "system"; then
    ENABLE_PCH="1"
    AC_MSG_RESULT([yes (system headers)])
elif test "$enable_pch" = "base"; then
    ENABLE_PCH="2"
    AC_MSG_RESULT([yes (system and base headers)])
elif test "$enable_pch" = "normal"; then
    ENABLE_PCH="3"
    AC_MSG_RESULT([yes (normal)])
elif test "$enable_pch" = "full"; then
    ENABLE_PCH="4"
    AC_MSG_RESULT([yes (full)])
elif test "$enable_pch" = "yes"; then
    # Pick a suitable default.
    if test "$GCC" = "yes"; then
        # With Clang and GCC higher levels do not seem to make a noticeable improvement,
        # while making the PCHs larger and rebuilds more likely.
        ENABLE_PCH="2"
        AC_MSG_RESULT([yes (system and base headers)])
    else
        # With MSVC the highest level makes a significant difference,
        # and it was the default when there used to be no PCH levels.
        ENABLE_PCH="4"
        AC_MSG_RESULT([yes (full)])
    fi
elif test "$enable_pch" = "no"; then
    AC_MSG_RESULT([no])
else
    AC_MSG_ERROR([Unknown value for --enable-pch])
fi
AC_SUBST(ENABLE_PCH)
# ccache 3.7.1 and older do not properly detect/handle -include .gch in CCACHE_DEPEND mode
if test -n "$ENABLE_PCH" -a -n "$CCACHE_DEPEND_MODE" -a "$GCC" = "yes" -a "$COM_IS_CLANG" != "TRUE"; then
    AC_PATH_PROG([CCACHE_BIN],[ccache],[not found])
    if test "$CCACHE_BIN" != "not found"; then
        AC_MSG_CHECKING([ccache version])
        CCACHE_VERSION=`"$CCACHE_BIN" -V | "$AWK" '/^ccache version/{print $3}'`
        CCACHE_NUMVER=`echo $CCACHE_VERSION | $AWK -F. '{ print \$1*10000+\$2*100+\$3 }'`
        AC_MSG_RESULT([$CCACHE_VERSION])
        AC_MSG_CHECKING([whether ccache depend mode works properly with GCC PCH])
        if test "$CCACHE_NUMVER" -gt "030701"; then
            AC_MSG_RESULT([yes])
        else
            AC_MSG_RESULT([no (not newer than 3.7.1)])
            CCACHE_DEPEND_MODE=
        fi
    fi
fi

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
        AC_MSG_ERROR([install the appropriate SHA-1 checksumming program for this OS])
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
AC_MSG_CHECKING([for --hash-style gcc linker support])
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
        AC_MSG_ERROR([found Perl $_perl_version, use Perl 5])
    fi
    AC_MSG_RESULT([Perl $_perl_version])
else
    AC_MSG_ERROR([Perl not found, install Perl 5])
fi

dnl ===================================================================
dnl Testing for required Perl modules
dnl ===================================================================

AC_MSG_CHECKING([for required Perl modules])
perl_use_string="use Cwd ; use Digest::MD5"
if test "$_os" = "WINNT"; then
    if test -n "$PKGFORMAT"; then
        for i in $PKGFORMAT; do
            case "$i" in
            msi)
                # for getting fonts versions to use in MSI
                perl_use_string="$perl_use_string ; use Font::TTF::Font"
                ;;
            esac
        done
    fi
fi
if test "$with_system_hsqldb" = "yes"; then
    perl_use_string="$perl_use_string ; use Archive::Zip"
fi
if $PERL -e "$perl_use_string">/dev/null 2>&1; then
    AC_MSG_RESULT([all modules found])
else
    AC_MSG_RESULT([failed to find some modules])
    # Find out which modules are missing.
    for i in $perl_use_string; do
        if test "$i" != "use" -a "$i" != ";"; then
            if ! $PERL -e "use $i;">/dev/null 2>&1; then
                missing_perl_modules="$missing_perl_modules $i"
            fi
        fi
    done
    AC_MSG_ERROR([
    The missing Perl modules are: $missing_perl_modules
    Install them as superuser/administrator with "cpan -i $missing_perl_modules"])
fi

dnl ===================================================================
dnl Check for pkg-config
dnl ===================================================================
if test "$_os" != "WINNT"; then
    PKG_PROG_PKG_CONFIG
fi

if test "$_os" != "WINNT"; then

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
dnl pkg-config checks on macOS
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

    # We need this check to detect 4.6.1 or above.
    for ver in 4.8 4.7.2 4.7.1 4.7 4.6.2 4.6.1; do
        reg_get_value_32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/NETFXSDK/$ver/WinSDK-NetFx40Tools/InstallationFolder"
        if test -n "$regvalue" -a \( -f "$regvalue/al.exe" -o -f "$regvalue/bin/al.exe" \); then
            altest=$regvalue
            return
        fi
    done

    for x in `ls /proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft\ SDKs/Windows`; do
        reg_get_value_32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/$x/WinSDK-NetFx40Tools/InstallationFolder"
        if test -n "$regvalue" -a \( -f "$regvalue/al.exe" -o -f "$regvalue/bin/al.exe" \); then
            altest=$regvalue
            return
        fi
    done


}

find_dotnetsdk46()
{
    unset frametest

    for ver in 4.8 4.7.2 4.7.1 4.7 4.6.2 4.6.1 4.6; do
        reg_get_value_32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/NETFXSDK/$ver/KitsInstallationFolder"
        if test -n "$regvalue"; then
            frametest=$regvalue
            return
        fi
    done
}

find_winsdk_version()
{
    # Args: $1 : SDK version as in "8.0", "8.1A" etc
    # Return values: $winsdktest, $winsdkbinsubdir, $winsdklibsubdir

    unset winsdktest winsdkbinsubdir winsdklibsubdir

    case "$1" in
    8.0|8.0A)
        reg_get_value_32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Windows Kits/Installed Roots/KitsRoot"
        if test -n "$regvalue"; then
            winsdktest=$regvalue
            winsdklibsubdir=win8
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
    10.0)
        reg_get_value_32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v${1}/InstallationFolder"
        if test -n "$regvalue"; then
            winsdktest=$regvalue
            reg_get_value_32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v${1}/ProductVersion"
            if test -n "$regvalue"; then
                winsdkbinsubdir="$regvalue".0
                winsdklibsubdir=$winsdkbinsubdir
                tmppath="$winsdktest\\Include\\$winsdklibsubdir"
                # test exist the SDK path
                if test -d "$tmppath"; then
                   # when path is convertible to a short path then path is okay
                   cygpath -d "$tmppath" >/dev/null 2>&1
                   if test $? -ne 0; then
                      AC_MSG_ERROR([Windows SDK doesn't have a 8.3 name, see NtfsDisable8dot3NameCreation])
                   fi
                else
                   AC_MSG_ERROR([The Windows SDK not found, check the installation])
                fi
            fi
            return
        fi
        ;;
    esac
}

find_winsdk()
{
    # Return value: From find_winsdk_version

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
    AC_MSG_CHECKING([for MSVC merge modules directory])
    my_msm_files=Microsoft_VC${VCVER}_CRT_x86.msm
    case "$VCVER" in
        150|160)
        my_msm_files="Microsoft_VC141_CRT_x86.msm Microsoft_VC142_CRT_x86.msm ${my_msm_files}"
        ;;
    esac
    for f in $my_msm_files; do
        echo "$as_me:$LINENO: searching for $f" >&5
    done

    msmdir=
    for ver in 14.0 15.0; do
        reg_get_value_32 HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/$ver/Setup/VS/MSMDir
        if test -n "$regvalue"; then
            for f in ${my_msm_files}; do
                if test -e "$regvalue/${f}"; then
                    msmdir=$regvalue
                    break
                fi
            done
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
        for f in ${my_msm_files}; do
            if test -e "$my_msm_dir/${f}"; then
                msmdir=$my_msm_dir
            fi
        done
    fi

    dnl Starting from MSVC 15.0, merge modules are located in different directory
    case "$VCVER" in
    150|160)
        for l in `ls -1 $VC_PRODUCT_DIR/redist/MSVC/`; do
            echo "$as_me:$LINENO: looking in $VC_PRODUCT_DIR/redist/MSVC/$l/MergeModules])" >&5
            my_msm_dir="$VC_PRODUCT_DIR/redist/MSVC/$l/MergeModules/"
            for f in ${my_msm_files}; do
                if test -e "$my_msm_dir/${f}"; then
                    msmdir=$my_msm_dir
                    break
                fi
            done
        done
        ;;
    esac

    if test -n "$msmdir"; then
        msmdir=`cygpath -m "$msmdir"`
        AC_MSG_RESULT([$msmdir])
    else
        if test "$ENABLE_RELEASE_BUILD" = "TRUE" ; then
            AC_MSG_FAILURE([not found])
        else
            AC_MSG_WARN([not found (check config.log)])
            add_warning "MSM none of ${my_msm_files} found"
        fi
    fi
}

find_msvc_x64_dlls()
{
    AC_MSG_CHECKING([for MSVC x64 DLL path])
    msvcdllpath="$VC_PRODUCT_DIR/redist/x64/Microsoft.VC${VCVER}.CRT"
    case "$VCVER" in
    150|160)
        for l in `ls -1 $VC_PRODUCT_DIR/redist/MSVC/`; do
            echo "$as_me:$LINENO: testing $VC_PRODUCT_DIR/redist/MSVC/$l/x64/Microsoft.VC141.CRT" >&5
            if test -d "$VC_PRODUCT_DIR/redist/MSVC/$l/x64/Microsoft.VC141.CRT"; then
                msvcdllpath="$VC_PRODUCT_DIR/redist/MSVC/$l/x64/Microsoft.VC141.CRT"
                break
            fi
            echo "$as_me:$LINENO: testing $VC_PRODUCT_DIR/redist/MSVC/$l/x64/Microsoft.VC142.CRT" >&5
            if test -d "$VC_PRODUCT_DIR/redist/MSVC/$l/x64/Microsoft.VC142.CRT"; then
                msvcdllpath="$VC_PRODUCT_DIR/redist/MSVC/$l/x64/Microsoft.VC142.CRT"
                break
            fi
            echo "$as_me:$LINENO: testing $VC_PRODUCT_DIR/redist/MSVC/$l/x64/Microsoft.VC150.CRT" >&5
            if test -d "$VC_PRODUCT_DIR/redist/MSVC/$l/onecore/x64/Microsoft.VC150.CRT"; then
                msvcdllpath="$VC_PRODUCT_DIR/redist/MSVC/$l/onecore/x64/Microsoft.VC150.CRT"
                break
            fi
        done
        ;;
    esac
    AC_MSG_RESULT([$msvcdllpath])
    AC_MSG_CHECKING([for MSVC x64 DLLs])
    msvcdlls="msvcp140.dll vcruntime140.dll"
    for dll in $msvcdlls; do
        if ! test -f "$msvcdllpath/$dll"; then
            AC_MSG_FAILURE([missing $dll])
        fi
    done
    AC_MSG_RESULT([found all ($msvcdlls)])
}

dnl =========================================
dnl Check for the Windows  SDK.
dnl =========================================
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
    fi

    if test -z "$WINDOWS_SDK_HOME"; then
        AC_MSG_RESULT([no, hoping the necessary headers and libraries will be found anyway!?])
    elif echo $WINDOWS_SDK_HOME | grep "8.0" >/dev/null 2>/dev/null; then
        WINDOWS_SDK_VERSION=80
        AC_MSG_RESULT([found Windows SDK 8.0 ($WINDOWS_SDK_HOME)])
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
        SOLARINC="$SOLARINC -I$WINDOWS_SDK_HOME/include -I$COMPATH/Include"
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
            WINDOWS_SDK_WILANGID="${WINDOWS_SDK_HOME}/bin/${WINDOWS_SDK_LIB_SUBDIR}/${WINDOWS_SDK_ARCH}/WiLangId.vbs"
        fi
        if ! test -e "$WINDOWS_SDK_WILANGID" ; then
            WINDOWS_SDK_WILANGID=$WINDOWS_SDK_HOME/bin/$WINDOWS_SDK_ARCH/WiLangId.vbs
        fi
        if ! test -e "$WINDOWS_SDK_WILANGID" ; then
            WINDOWS_SDK_WILANGID=$(cygpath -sm "C:/Program Files (x86)/Windows Kits/8.1/bin/$WINDOWS_SDK_ARCH/WiLangId.vbs")
        fi
    fi
    if test -n "$with_lang" -a "$with_lang" != "en-US"; then
        if test -n "$with_package_format" -a "$with_package_format" != no; then
            for i in "$with_package_format"; do
                if test "$i" = "msi"; then
                    if ! test -e "$WINDOWS_SDK_WILANGID" ; then
                        AC_MSG_ERROR([WiLangId.vbs not found - building translated packages will fail])
                    fi
                fi
            done
        fi
    fi
fi
AC_SUBST(WINDOWS_SDK_HOME)
AC_SUBST(WINDOWS_SDK_LIB_SUBDIR)
AC_SUBST(WINDOWS_SDK_VERSION)
AC_SUBST(WINDOWS_SDK_WILANGID)

if test "$build_os" = "cygwin"; then
    dnl Check midl.exe; this being the first check for a tool in the SDK bin
    dnl dir, it also determines that dir's path w/o an arch segment if any,
    dnl WINDOWS_SDK_BINDIR_NO_ARCH:
    AC_MSG_CHECKING([for midl.exe])

    find_winsdk
    if test -n "$winsdkbinsubdir" \
        -a -f "$winsdktest/Bin/$winsdkbinsubdir/$WINDOWS_SDK_ARCH/midl.exe"
    then
        MIDL_PATH=$winsdktest/Bin/$winsdkbinsubdir/$WINDOWS_SDK_ARCH
        WINDOWS_SDK_BINDIR_NO_ARCH=$WINDOWS_SDK_HOME/Bin/$winsdkbinsubdir
    elif test -f "$winsdktest/Bin/$WINDOWS_SDK_ARCH/midl.exe"; then
        MIDL_PATH=$winsdktest/Bin/$WINDOWS_SDK_ARCH
        WINDOWS_SDK_BINDIR_NO_ARCH=$WINDOWS_SDK_HOME/Bin
    elif test -f "$winsdktest/Bin/midl.exe"; then
        MIDL_PATH=$winsdktest/Bin
        WINDOWS_SDK_BINDIR_NO_ARCH=$WINDOWS_SDK_HOME/Bin
    fi
    if test ! -f "$MIDL_PATH/midl.exe"; then
        AC_MSG_ERROR([midl.exe not found in $winsdktest/Bin/$WINDOWS_SDK_ARCH, Windows SDK installation broken?])
    else
        AC_MSG_RESULT([$MIDL_PATH/midl.exe])
    fi

    # Convert to posix path with 8.3 filename restrictions ( No spaces )
    MIDL_PATH=`win_short_path_for_make "$MIDL_PATH"`

    if test -f "$WINDOWS_SDK_BINDIR_NO_ARCH/msiinfo.exe" \
         -a -f "$WINDOWS_SDK_BINDIR_NO_ARCH/msidb.exe" \
         -a -f "$WINDOWS_SDK_BINDIR_NO_ARCH/uuidgen.exe" \
         -a -f "$WINDOWS_SDK_BINDIR_NO_ARCH/msitran.exe"; then :
    elif test -f "$WINDOWS_SDK_BINDIR_NO_ARCH/x86/msiinfo.exe" \
         -a -f "$WINDOWS_SDK_BINDIR_NO_ARCH/x86/msidb.exe" \
         -a -f "$WINDOWS_SDK_BINDIR_NO_ARCH/x86/uuidgen.exe" \
         -a -f "$WINDOWS_SDK_BINDIR_NO_ARCH/x86/msitran.exe"; then :
    elif test -f "$WINDOWS_SDK_HOME/bin/x86/msiinfo.exe" \
         -a -f "$WINDOWS_SDK_HOME/bin/x86/msidb.exe" \
         -a -f "$WINDOWS_SDK_BINDIR_NO_ARCH/x86/uuidgen.exe" \
         -a -f "$WINDOWS_SDK_HOME/bin/x86/msitran.exe"; then :
    else
        AC_MSG_ERROR([Some (all?) Windows Installer tools in the Windows SDK are missing, please install.])
    fi

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
    if test -n "$winsdkbinsubdir" \
        -a -f "$winsdktest/Bin/$winsdkbinsubdir/$WINDOWS_SDK_ARCH/al.exe"
    then
        AL_PATH="$winsdktest/Bin/$winsdkbinsubdir/$WINDOWS_SDK_ARCH"
    elif test -f "$winsdktest/Bin/$WINDOWS_SDK_ARCH/al.exe"; then
        AL_PATH="$winsdktest/Bin/$WINDOWS_SDK_ARCH"
    elif test -f "$winsdktest/Bin/al.exe"; then
        AL_PATH="$winsdktest/Bin"
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
    AC_MSG_CHECKING(.NET Framework)
    find_dotnetsdk46
    PathFormat "$frametest"
    frametest="$formatted_path"
    if test -f "$frametest/Lib/um/$WINDOWS_SDK_ARCH/mscoree.lib"; then
        DOTNET_FRAMEWORK_HOME="$frametest"
    else
        find_winsdk
        if test -f "$winsdktest/lib/mscoree.lib" -o -f "$winsdktest/lib/$winsdklibsubdir/um/$WINDOWS_SDK_ARCH/mscoree.lib"; then
            DOTNET_FRAMEWORK_HOME="$winsdktest"
        fi
    fi
    if test ! -f "$DOTNET_FRAMEWORK_HOME/lib/mscoree.lib" -a ! -f "$DOTNET_FRAMEWORK_HOME/lib/$winsdklibsubdir/um/$WINDOWS_SDK_ARCH/mscoree.lib" -a ! -f "$DOTNET_FRAMEWORK_HOME/Lib/um/$WINDOWS_SDK_ARCH/mscoree.lib"; then
        AC_MSG_ERROR([mscoree.lib not found])
    fi
    AC_MSG_RESULT([found: $DOTNET_FRAMEWORK_HOME])

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
if test "$_os" != "WINNT"; then
    AC_HEADER_STDC
fi

dnl ===================================================================
dnl Testing for C++ compiler and version...
dnl ===================================================================

if test "$_os" != "WINNT"; then
    # AC_PROG_CXX sets CXXFLAGS to -g -O2 if not set, avoid that
    save_CXXFLAGS=$CXXFLAGS
    AC_PROG_CXX
    CXXFLAGS=$save_CXXFLAGS
    if test -z "$CXX_BASE"; then
        CXX_BASE=`first_arg_basename "$CXX"`
    fi
fi

dnl check for GNU C++ compiler version
if test "$GXX" = "yes" -a -z "$COM_IS_CLANG"; then
    AC_MSG_CHECKING([the GNU C++ compiler version])

    _gpp_version=`$CXX -dumpversion`
    _gpp_majmin=`echo $_gpp_version | $AWK -F. '{ print \$1*100+\$2 }'`

    if test "$_gpp_majmin" -lt "700"; then
        AC_MSG_ERROR([You need to use GNU C++ compiler version >= 7.0 to build LibreOffice, you have $_gpp_version.])
    else
        AC_MSG_RESULT([ok (g++ $_gpp_version)])
    fi

    dnl see https://code.google.com/p/android/issues/detail?id=41770
        glibcxx_threads=no
        AC_LANG_PUSH([C++])
        AC_REQUIRE_CPP
        AC_MSG_CHECKING([whether $CXX_BASE is broken with boost.thread])
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
AC_SUBST(BOOST_CXXFLAGS)

#
# prefx CXX with ccache if needed
#
if test "$CCACHE" != ""; then
    AC_MSG_CHECKING([whether $CXX_BASE is already ccached])
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
        CXX_BASE="ccache $CXX_BASE"
        AC_MSG_RESULT([no])
    fi
    CXXFLAGS=$save_CXXFLAGS
    AC_LANG_POP([C++])
fi

dnl ===================================================================
dnl Find pre-processors.(should do that _after_ messing with CC/CXX)
dnl ===================================================================

if test "$_os" != "WINNT"; then
    AC_PROG_CXXCPP

    dnl Check whether there's a C pre-processor.
    AC_PROG_CPP
fi


dnl ===================================================================
dnl Find integral type sizes and alignments
dnl ===================================================================

if test "$_os" != "WINNT"; then

if test "$_os" = "iOS"; then
    AC_MSG_CHECKING([iOS setting sizes long, short, int, long long, double, voidp])
    ac_cv_sizeof_long=8
    ac_cv_sizeof_short=2
    ac_cv_sizeof_int=4
    ac_cv_sizeof_long_long=8
    ac_cv_sizeof_double=8
    ac_cv_sizeof_voidp=8
else
    AC_CHECK_SIZEOF(long)
    AC_CHECK_SIZEOF(short)
    AC_CHECK_SIZEOF(int)
    AC_CHECK_SIZEOF(long long)
    AC_CHECK_SIZEOF(double)
    AC_CHECK_SIZEOF(void*)
fi

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
if test "$cross_compiling" != yes -a "$with_valgrind" != no; then
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
    if test "$with_valgrind" = yes; then
        AC_MSG_ERROR([--with-valgrind specified but no Valgrind headers found])
    fi
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
dnl GCC features
dnl ===================================================================
HAVE_GCC_STACK_CLASH_PROTECTION=
if test "$GCC" = "yes" -o "$COM_IS_CLANG" = TRUE; then
    AC_MSG_CHECKING([whether $CC_BASE supports -fstack-clash-protection])
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -Werror -fstack-clash-protection"
    AC_LINK_IFELSE(
        [AC_LANG_PROGRAM(, [[return 0;]])],
        [AC_MSG_RESULT([yes]); HAVE_GCC_STACK_CLASH_PROTECTION=TRUE],
        [AC_MSG_RESULT([no])])
    CFLAGS=$save_CFLAGS

    AC_MSG_CHECKING([whether $CC_BASE supports -mno-avx])
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -Werror -mno-avx"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[ return 0; ]])],[ HAVE_GCC_AVX=TRUE ],[])
    CFLAGS=$save_CFLAGS
    if test "$HAVE_GCC_AVX" = "TRUE"; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
    fi

    AC_MSG_CHECKING([whether $CC_BASE supports atomic functions])
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

    AC_MSG_CHECKING([whether $CC_BASE supports __builtin_ffs])
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[ return __builtin_ffs(1); ]])],[HAVE_GCC_BUILTIN_FFS=TRUE],[])
    if test "$HAVE_GCC_BUILTIN_FFS" = "TRUE"; then
        AC_MSG_RESULT([yes])
        AC_DEFINE(HAVE_GCC_BUILTIN_FFS)
    else
        AC_MSG_RESULT([no])
    fi

    AC_MSG_CHECKING([whether $CC_BASE supports __attribute__((deprecated(message)))])
    save_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -Werror"
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            __attribute__((deprecated("test"))) void f();
        ])], [
            AC_DEFINE([HAVE_GCC_DEPRECATED_MESSAGE],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    CFLAGS=$save_CFLAGS

    AC_MSG_CHECKING([whether $CXX_BASE defines __base_class_type_info in cxxabi.h])
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

    AC_MSG_CHECKING([whether $CXX_BASE defines __class_type_info in cxxabi.h])
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

    AC_MSG_CHECKING([whether $CXX_BASE declares __cxa_allocate_exception in cxxabi.h])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            #include <cxxabi.h>
            void * f() { return __cxxabiv1::__cxa_allocate_exception(0); }
        ])], [
            AC_DEFINE([HAVE_CXXABI_H_CXA_ALLOCATE_EXCEPTION],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    AC_LANG_POP([C++])

    AC_MSG_CHECKING([whether $CXX_BASE defines __cxa_eh_globals in cxxabi.h])
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

    AC_MSG_CHECKING([whether $CXX_BASE defines __cxa_exceptions in cxxabi.h])
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

    AC_MSG_CHECKING([whether $CXX_BASE declares __cxa_get_globals in cxxabi.h])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            #include <cxxabi.h>
            void * f() { return __cxxabiv1::__cxa_get_globals(); }
        ])], [
            AC_DEFINE([HAVE_CXXABI_H_CXA_GET_GLOBALS],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    AC_LANG_POP([C++])

    AC_MSG_CHECKING([whether $CXX_BASE declares __cxa_current_exception_type in cxxabi.h])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            #include <cxxabi.h>
            void * f() { return __cxxabiv1::__cxa_current_exception_type(); }
        ])], [
            AC_DEFINE([HAVE_CXXABI_H_CXA_CURRENT_EXCEPTION_TYPE],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    AC_LANG_POP([C++])

    AC_MSG_CHECKING([whether $CXX_BASE declares __cxa_throw in cxxabi.h])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            #include <cxxabi.h>
            void f() { __cxxabiv1::__cxa_throw(0, 0, 0); }
        ])], [
            AC_DEFINE([HAVE_CXXABI_H_CXA_THROW],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    AC_LANG_POP([C++])

    AC_MSG_CHECKING([whether $CXX_BASE defines __si_class_type_info in cxxabi.h])
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

    AC_MSG_CHECKING([whether $CXX_BASE defines __vmi_class_type_info in cxxabi.h])
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

    dnl Available in GCC 4.9 and at least since Clang 3.4:
    AC_MSG_CHECKING([whether $CXX_BASE supports __attribute__((warn_unused))])
    AC_LANG_PUSH([C++])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS -Werror"
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            struct __attribute__((warn_unused)) dummy {};
        ])], [
            AC_DEFINE([HAVE_GCC_ATTRIBUTE_WARN_UNUSED],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    CXXFLAGS=$save_CXXFLAGS
AC_LANG_POP([C++])
fi

AC_SUBST(HAVE_GCC_AVX)
AC_SUBST(HAVE_GCC_BUILTIN_ATOMIC)
AC_SUBST(HAVE_GCC_BUILTIN_FFS)
AC_SUBST(HAVE_GCC_STACK_CLASH_PROTECTION)

dnl ===================================================================
dnl Identify the C++ library
dnl ===================================================================

AC_MSG_CHECKING([what the C++ library is])
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
     AC_DEFINE([HAVE_LIBCXX])
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
dnl Check for gperf
dnl ===================================================================
AC_PATH_PROG(GPERF, gperf)
if test -z "$GPERF"; then
    AC_MSG_ERROR([gperf not found but needed. Install it.])
fi
if test "$GNUMAKE_WIN_NATIVE" = "TRUE" ; then
    GPERF=`cygpath -m $GPERF`
fi
AC_MSG_CHECKING([whether gperf is new enough])
my_gperf_ver1=$($GPERF --version | head -n 1)
my_gperf_ver2=${my_gperf_ver1#GNU gperf }
my_gperf_ver3=$(printf %s "$my_gperf_ver2" | $AWK -F. '{ print $1*100+($2<100?$2:99) }')
if test "$my_gperf_ver3" -ge 301; then
    AC_MSG_RESULT([yes ($my_gperf_ver2)])
else
    AC_MSG_ERROR(["$my_gperf_ver1" is too old or unrecognized, must be at least gperf 3.1])
fi
AC_SUBST(GPERF)

dnl ===================================================================
dnl Check for system libcmis
dnl ===================================================================
# libcmis requires curl and we can't build curl for iOS
if test $_os != iOS; then
    libo_CHECK_SYSTEM_MODULE([libcmis],[LIBCMIS],[libcmis-0.5 >= 0.5.2])
    ENABLE_LIBCMIS=TRUE
else
    ENABLE_LIBCMIS=
fi
AC_SUBST(ENABLE_LIBCMIS)

dnl ===================================================================
dnl C++11
dnl ===================================================================

AC_MSG_CHECKING([whether $CXX_BASE supports C++17])
CXXFLAGS_CXX11=
if test "$COM" = MSC -a "$COM_IS_CLANG" != TRUE; then
    if test "$with_latest_c__" = yes; then
        CXXFLAGS_CXX11=-std:c++latest
    else
        CXXFLAGS_CXX11=-std:c++17
    fi
    CXXFLAGS_CXX11="$CXXFLAGS_CXX11 -Zc:__cplusplus"
elif test "$GCC" = "yes" -o "$COM_IS_CLANG" = TRUE; then
    my_flags='-std=c++17 -std=c++1z'
    if test "$with_latest_c__" = yes; then
        my_flags="-std=gnu++2a -std=c++2a $my_flags"
    fi
    for flag in $my_flags; do
        if test "$COM" = MSC; then
            flag="-Xclang $flag"
        fi
        save_CXXFLAGS=$CXXFLAGS
        CXXFLAGS="$CXXFLAGS $flag -Werror"
        if test "$SYSTEM_LIBCMIS" = TRUE; then
            CXXFLAGS="$CXXFLAGS -DSYSTEM_LIBCMIS $LIBCMIS_CFLAGS"
        fi
        AC_LANG_PUSH([C++])
        dnl Clang 3.9 supports __float128 since
        dnl <http://llvm.org/viewvc/llvm-project?view=revision&revision=268898> "Enable support for
        dnl __float128 in Clang and enable it on pertinent platforms", but Clang 3.8 may need a
        dnl hacky workaround to be able to include <vector> (as is done in the main check whether
        dnl $flag is supported below, so check this first):
        my_float128hack=
        my_float128hack_impl=-D__float128=void
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
            #include <vector>
            // some Clang fail when compiling against libstdc++ headers with -std=gnu++0x
            // (__float128)
            ]])
        ],,[
            dnl The only reason why libstdc++ headers fail with Clang in C++11 mode is because they
            dnl use the __float128 type that Clang doesn't know (libstdc++ checks whether __float128
            dnl is available during its build, but it's usually built using GCC, and so c++config.h
            dnl hardcodes __float128 being supported). At least for some older libstdc++, the only
            dnl place where __float128 is used is in a template specialization, -D__float128=void
            dnl will avoid the problem there while still causing a problem if somebody actually uses
            dnl the type. (But some later libstdc++ are known to use __float128 also in algorithm ->
            dnl bits/stl_alog.h -> cstdlib -> bits/std_abs.h, in a way that unfortunately cannot be
            dnl "fixed" with this hack):
            CXXFLAGS="$CXXFLAGS $my_float128hack_impl"
            AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
                #include <vector>
                // some Clang fail when compiling against libstdc++ headers with -std=gnu++0x
                // (__float128)
                ]])
            ],[my_float128hack=$my_float128hack_impl])
        ])
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
            #include <algorithm>
            #include <functional>
            #include <vector>

            #if defined SYSTEM_LIBCMIS
            // See ucb/source/ucp/cmis/auth_provider.hxx:
            #if !defined __clang__
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wdeprecated"
            #pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
            #endif
            #include <libcmis/libcmis.hxx>
            #if !defined __clang__
            #pragma GCC diagnostic pop
            #endif
            #endif

            void f(std::vector<int> & v, std::function<bool(int, int)> fn) {
                std::sort(v.begin(), v.end(), fn);
            }
            ]])],[CXXFLAGS_CXX11=$flag $my_float128hack])
        AC_LANG_POP([C++])
        CXXFLAGS=$save_CXXFLAGS
        if test -n "$CXXFLAGS_CXX11"; then
            break
        fi
    done
fi
if test -n "$CXXFLAGS_CXX11"; then
    AC_MSG_RESULT([yes ($CXXFLAGS_CXX11)])
else
    AC_MSG_ERROR(no)
fi
AC_SUBST(CXXFLAGS_CXX11)

if test "$GCC" = "yes"; then
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS $CXXFLAGS_CXX11"
    CHECK_L_ATOMIC
    CXXFLAGS=$save_CXXFLAGS
    AC_SUBST(ATOMIC_LIB)
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

AC_MSG_CHECKING([whether $CXX_BASE supports C++11 without Language Defect 757])
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

dnl _Pragma support (may require C++11)
if test "$GCC" = "yes" -o "$COM_IS_CLANG" = TRUE; then
    AC_MSG_CHECKING([whether $CXX_BASE supports _Pragma operator])
    AC_LANG_PUSH([C++])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS $CXXFLAGS_CXX11 -Werror"
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
            _Pragma("GCC diagnostic ignored \"-Wformat\"")
        ])], [
            AC_DEFINE([HAVE_GCC_PRAGMA_OPERATOR],[1])
            AC_MSG_RESULT([yes])
        ], [AC_MSG_RESULT([no])])
    AC_LANG_POP([C++])
    CXXFLAGS=$save_CXXFLAGS
fi

HAVE_GCC_FNO_SIZED_DEALLOCATION=
if test "$GCC" = yes; then
    AC_MSG_CHECKING([whether $CXX_BASE supports -fno-sized-deallocation])
    AC_LANG_PUSH([C++])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS -fno-sized-deallocation"
    AC_LINK_IFELSE([AC_LANG_PROGRAM()],[HAVE_GCC_FNO_SIZED_DEALLOCATION=TRUE])
    CXXFLAGS=$save_CXXFLAGS
    AC_LANG_POP([C++])
    if test "$HAVE_GCC_FNO_SIZED_DEALLOCATION" = TRUE; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_SUBST([HAVE_GCC_FNO_SIZED_DEALLOCATION])

AC_MSG_CHECKING([whether $CXX_BASE supports guaranteed copy elision])
AC_LANG_PUSH([C++])
save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $CXXFLAGS_CXX11"
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
        // At least VS 2017 15.8.1 defines __cpp_guaranteed_copy_elision as 201606L without actually
        // supporting it:
        #if !defined __cpp_guaranteed_copy_elision || (defined _MSC_VER && !defined __clang__)
        struct S {
        private:
            S(S const &);
        public:
            S();
            ~S();
        };
        S copy();
        void f() { S c(copy()); }
        #endif
    ])], [
        AC_DEFINE([HAVE_CPP_GUARANTEED_COPY_ELISION],[1])
        AC_MSG_RESULT([yes])
    ], [AC_MSG_RESULT([no])])
CXXFLAGS=$save_CXXFLAGS
AC_LANG_POP([C++])

AC_MSG_CHECKING([whether $CXX_BASE supports C++2a constinit sorted vectors])
AC_LANG_PUSH([C++])
save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $CXXFLAGS_CXX11"
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
        #include <algorithm>
        #include <initializer_list>
        #include <vector>
        template<typename T> class S {
        private:
            std::vector<T> v_;
        public:
            constexpr S(std::initializer_list<T> i): v_(i) { std::sort(v_.begin(), v_.end()); }
        };
        constinit S<int> s{3, 2, 1};
    ])], [
        AC_DEFINE([HAVE_CPP_CONSTINIT_SORTED_VECTOR],[1])
        AC_MSG_RESULT([yes])
    ], [AC_MSG_RESULT([no])])
CXXFLAGS=$save_CXXFLAGS
AC_LANG_POP([C++])

AC_MSG_CHECKING([whether $CXX_BASE supports C++2a <span> with unsigned size_type])
AC_LANG_PUSH([C++])
save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $CXXFLAGS_CXX11"
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
        #include <span>
        #include <type_traits>
        // Don't check size_type directly, as it was called index_type before P1872R0:
        void f(std::span<int> s) { static_assert(std::is_unsigned_v<decltype(s.size())>); };
    ])], [
        AC_DEFINE([HAVE_CPP_SPAN],[1])
        AC_MSG_RESULT([yes])
    ], [AC_MSG_RESULT([no])])
CXXFLAGS=$save_CXXFLAGS
AC_LANG_POP([C++])

AC_MSG_CHECKING([whether $CXX_BASE has GCC bug 87150])
AC_LANG_PUSH([C++])
save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $CXXFLAGS_CXX11"
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
        struct S1 { S1(S1 &&); };
        struct S2: S1 {};
        S1 f(S2 s) { return s; }
    ])], [
        AC_DEFINE([HAVE_GCC_BUG_87150],[1])
        AC_MSG_RESULT([yes])
    ], [AC_MSG_RESULT([no])])
CXXFLAGS=$save_CXXFLAGS
AC_LANG_POP([C++])

dnl At least GCC 8.2 with -O2 (i.e., --enable-optimized) causes a false-positive -Wmaybe-
dnl uninitialized warning for code like
dnl
dnl   OString f();
dnl   boost::optional<OString> * g(bool b) {
dnl       boost::optional<OString> o;
dnl       if (b) o = f();
dnl       return new boost::optional<OString>(o);
dnl   }
dnl
dnl (as is e.g. present, in a slightly more elaborate form, in
dnl librdf_TypeConverter::extractNode_NoLock in unoxml/source/rdf/librdf_repository.cxx); the below
dnl code is meant to be a faithfully stripped-down and self-contained version of the above code:
HAVE_BROKEN_GCC_WMAYBE_UNINITIALIZED=
if test "$GCC" = yes && test "$COM_IS_CLANG" != TRUE; then
    AC_MSG_CHECKING([whether $CXX_BASE might report false -Werror=maybe-uninitialized])
    AC_LANG_PUSH([C++])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS $CXXFLAGS_CXX11 -Werror -Wmaybe-uninitialized"
    if test "$ENABLE_OPTIMIZED" = TRUE; then
        CXXFLAGS="$CXXFLAGS -O2"
    else
        CXXFLAGS="$CXXFLAGS -O0"
    fi
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
            #include <new>
            void f1(int);
            struct S1 {
                ~S1() { f1(n); }
                int n = 0;
            };
            struct S2 {
                S2() {}
                S2(S2 const & s) { if (s.init) set(*reinterpret_cast<S1 const *>(s.stg)); }
                ~S2() { if (init) reinterpret_cast<S1 *>(stg)->S1::~S1(); }
                void set(S1 s) {
                    new (stg) S1(s);
                    init = true;
                }
                bool init = false;
                char stg[sizeof (S1)];
            } ;
            S1 f2();
            S2 * f3(bool b) {
                S2 o;
                if (b) o.set(f2());
                return new S2(o);
            }
        ]])], [AC_MSG_RESULT([no])], [
            HAVE_BROKEN_GCC_WMAYBE_UNINITIALIZED=TRUE
            AC_MSG_RESULT([yes])
        ])
    CXXFLAGS=$save_CXXFLAGS
    AC_LANG_POP([C++])
fi
AC_SUBST([HAVE_BROKEN_GCC_WMAYBE_UNINITIALIZED])

dnl ===================================================================
dnl CPU Intrinsics support - SSE, AVX
dnl ===================================================================

INTRINSICS_CXXFLAGS=""

if test "$GCC" = "yes"; then
    AC_MSG_CHECKING([whether $CXX can compile SSE2 intrinsics])
    AC_LANG_PUSH([C++])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS -msse2"
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
        #include <emmintrin.h>
        int main () {
            volatile __m128i a = _mm_set1_epi32 (0), b = _mm_set1_epi32 (0), c;
            c = _mm_xor_si128 (a, b);
            return 0;
        }
        ])],
        [can_compile_sse2=yes],
        [can_compile_sse2=no])
    AC_LANG_POP([C++])
    CXXFLAGS=$save_CXXFLAGS
    AC_MSG_RESULT([${can_compile_sse2}])
    if test "${can_compile_sse2}" = "yes" ; then
        INTRINSICS_CXXFLAGS="-msse2"
    else
        AC_MSG_WARN([cannot compile SSE2 intrinsics])
    fi

    AC_MSG_CHECKING([whether $CXX can compile SSSE3 intrinsics])
    AC_LANG_PUSH([C++])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS -mssse3"
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
        #include <tmmintrin.h>
        int main () {
            volatile __m128i a = _mm_set1_epi32 (0), b = _mm_set1_epi32 (0), c;
            c = _mm_maddubs_epi16 (a, b);
            return 0;
        }
        ])],
        [can_compile_ssse3=yes],
        [can_compile_ssse3=no])
    AC_LANG_POP([C++])
    CXXFLAGS=$save_CXXFLAGS
    AC_MSG_RESULT([${can_compile_ssse3}])
    if test "${can_compile_ssse3}" = "yes" ; then
        INTRINSICS_CXXFLAGS="-mssse3"
    else
        AC_MSG_WARN([cannot compile SSSE3 intrinsics])
    fi

    AC_MSG_CHECKING([whether $CXX can compile AVX intrinsics])
    AC_LANG_PUSH([C++])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS -mavx"
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
        #include <immintrin.h>
        int main () {
            volatile __m256 a = _mm256_set1_ps (0.0f), b = _mm256_set1_ps (0.0f), c;
            c = _mm256_xor_ps(a, b);
            return 0;
        }
        ])],
        [can_compile_avx=yes],
        [can_compile_avx=no])
    AC_LANG_POP([C++])
    CXXFLAGS=$save_CXXFLAGS
    AC_MSG_RESULT([${can_compile_avx}])
    if test "${can_compile_avx}" = "yes" ; then
        INTRINSICS_CXXFLAGS="-mavx"
    else
        AC_MSG_WARN([cannot compile AVX intrinsics])
    fi

    AC_MSG_CHECKING([whether $CXX can compile AVX2 intrinsics])
    AC_LANG_PUSH([C++])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS -mavx2"
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
        #include <immintrin.h>
        int main () {
            volatile __m256i a = _mm256_set1_epi32 (0), b = _mm256_set1_epi32 (0), c;
            c = _mm256_maddubs_epi16(a, b);
            return 0;
        }
        ])],
        [can_compile_avx2=yes],
        [can_compile_avx2=no])
    AC_LANG_POP([C++])
    CXXFLAGS=$save_CXXFLAGS
    AC_MSG_RESULT([${can_compile_avx2}])
    if test "${can_compile_avx2}" = "yes" ; then
        INTRINSICS_CXXFLAGS="-mavx2"
    else
        AC_MSG_WARN([cannot compile AVX2 intrinsics])
    fi
fi

AC_SUBST([INTRINSICS_CXXFLAGS])

dnl ===================================================================
dnl system stl sanity tests
dnl ===================================================================
if test "$_os" != "WINNT"; then

    AC_LANG_PUSH([C++])

    save_CPPFLAGS="$CPPFLAGS"
    if test -n "$MACOSX_SDK_PATH"; then
        CPPFLAGS="-isysroot $MACOSX_SDK_PATH $CPPFLAGS"
    fi

    # Assume visibility is not broken with libc++. The below test is very much designed for libstdc++
    # only.
    if test "$CPP_LIBRARY" = GLIBCXX; then
        dnl gcc#19664, gcc#22482, rhbz#162935
        AC_MSG_CHECKING([if STL headers are visibility safe (GCC bug 22482)])
        AC_EGREP_HEADER(visibility push, string, stlvisok=yes, stlvisok=no)
        AC_MSG_RESULT([$stlvisok])
        if test "$stlvisok" = "no"; then
            AC_MSG_ERROR([Your libstdc++ headers are not visibility safe. This is no longer supported.])
        fi
    fi

    # As the below test checks things when linking self-compiled dynamic libraries, it presumably is irrelevant
    # when we don't make any dynamic libraries?
    if test "$DISABLE_DYNLOADING" = ""; then
        AC_MSG_CHECKING([if $CXX_BASE is -fvisibility-inlines-hidden safe (Clang bug 11250)])
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
                for i in $CXX $CXXFLAGS; do
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
            AC_MSG_ERROR([Your gcc/clang is not -fvisibility-inlines-hidden safe. This is no longer supported.])
        fi
    fi

   AC_MSG_CHECKING([if $CXX_BASE has a visibility bug with class-level attributes (GCC bug 26905)])
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
            if test "$_os" = "Darwin" -o "$_os" = "iOS"; then
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
        AC_MSG_ERROR([Your gcc is not -fvisibility=hidden safe. This is no longer supported.])
    fi

    CPPFLAGS="$save_CPPFLAGS"

    AC_LANG_POP([C++])
fi

dnl ===================================================================
dnl  Clang++ tests
dnl ===================================================================

HAVE_GCC_FNO_ENFORCE_EH_SPECS=
if test "$GCC" = "yes"; then
    AC_MSG_CHECKING([whether $CXX_BASE supports -fno-enforce-eh-specs])
    AC_LANG_PUSH([C++])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CFLAGS -Werror -fno-enforce-eh-specs"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[ return 0; ]])],[ HAVE_GCC_FNO_ENFORCE_EH_SPECS=TRUE ],[])
    CXXFLAGS=$save_CXXFLAGS
    AC_LANG_POP([C++])
    if test "$HAVE_GCC_FNO_ENFORCE_EH_SPECS" = "TRUE"; then
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_SUBST(HAVE_GCC_FNO_ENFORCE_EH_SPECS)

dnl ===================================================================
dnl Compiler plugins
dnl ===================================================================

COMPILER_PLUGINS=
# currently only Clang

if test "$COM_IS_CLANG" != "TRUE"; then
    if test "$libo_fuzzed_enable_compiler_plugins" = yes -a "$enable_compiler_plugins" = yes; then
        AC_MSG_NOTICE([Resetting --enable-compiler-plugins=no])
        enable_compiler_plugins=no
    fi
fi

if test "$COM_IS_CLANG" = "TRUE"; then
    if test -n "$enable_compiler_plugins"; then
        compiler_plugins="$enable_compiler_plugins"
    elif test -n "$ENABLE_DBGUTIL"; then
        compiler_plugins=test
    else
        compiler_plugins=no
    fi
    if test "$compiler_plugins" != no -a "$my_apple_clang" != yes; then
        if test "$CLANGVER" -lt 50002; then
            if test "$compiler_plugins" = yes; then
                AC_MSG_ERROR([Clang $CLANGVER is too old to build compiler plugins; need >= 5.0.2.])
            else
                compiler_plugins=no
            fi
        fi
    fi
    if test "$compiler_plugins" != "no"; then
        dnl The prefix where Clang resides, override to where Clang resides if
        dnl using a source build:
        if test -z "$CLANGDIR"; then
            CLANGDIR=$(dirname $(dirname $($CXX -print-prog-name=$(basename $CXX))))
        fi
        # Assume Clang is self-built, but allow overriding COMPILER_PLUGINS_CXX to the compiler Clang was built with.
        if test -z "$COMPILER_PLUGINS_CXX"; then
            COMPILER_PLUGINS_CXX=[$(echo $CXX | sed -e 's/-fsanitize=[^ ]*//g')]
        fi
        clangbindir=$CLANGDIR/bin
        if test "$build_os" = "cygwin"; then
            clangbindir=$(cygpath -u "$clangbindir")
        fi
        AC_PATH_PROG(LLVM_CONFIG, llvm-config,[],"$clangbindir" $PATH)
        if test -n "$LLVM_CONFIG"; then
            COMPILER_PLUGINS_CXXFLAGS=$($LLVM_CONFIG --cxxflags)
            COMPILER_PLUGINS_LINKFLAGS=$($LLVM_CONFIG --ldflags --libs --system-libs | tr '\n' ' ')
            if test -z "$CLANGLIBDIR"; then
                CLANGLIBDIR=$($LLVM_CONFIG --libdir)
            fi
            # Try if clang is built from source (in which case its includes are not together with llvm includes).
            # src-root is [llvm-toplevel-src-dir]/llvm, clang is [llvm-toplevel-src-dir]/clang
            clangsrcdir=$(dirname $($LLVM_CONFIG --src-root))
            if test -n "$clangsrcdir" -a -d "$clangsrcdir" -a -d "$clangsrcdir/clang/include"; then
                COMPILER_PLUGINS_CXXFLAGS="$COMPILER_PLUGINS_CXXFLAGS -I$clangsrcdir/clang/include"
            fi
            # obj-root is [llvm-toplevel-obj-dir]/, clang is [llvm-toplevel-obj-dir]/tools/clang
            clangobjdir=$($LLVM_CONFIG --obj-root)
            if test -n "$clangobjdir" -a -d "$clangobjdir" -a -d "$clangobjdir/tools/clang/include"; then
                COMPILER_PLUGINS_CXXFLAGS="$COMPILER_PLUGINS_CXXFLAGS -I$clangobjdir/tools/clang/include"
            fi
        fi
        AC_MSG_NOTICE([compiler plugins compile flags: $COMPILER_PLUGINS_CXXFLAGS])
        AC_LANG_PUSH([C++])
        save_CXX=$CXX
        save_CXXCPP=$CXXCPP
        save_CPPFLAGS=$CPPFLAGS
        save_CXXFLAGS=$CXXFLAGS
        save_LDFLAGS=$LDFLAGS
        save_LIBS=$LIBS
        CXX=$COMPILER_PLUGINS_CXX
        CXXCPP="$COMPILER_PLUGINS_CXX -E"
        CPPFLAGS="$COMPILER_PLUGINS_CXXFLAGS"
        CXXFLAGS="$COMPILER_PLUGINS_CXXFLAGS"
        AC_CHECK_HEADER(clang/Basic/SourceLocation.h,
            [COMPILER_PLUGINS=TRUE],
            [
            if test "$compiler_plugins" = "yes"; then
                AC_MSG_ERROR([Cannot find Clang headers to build compiler plugins.])
            else
                AC_MSG_WARN([Cannot find Clang headers to build compiler plugins, plugins disabled])
                add_warning "Cannot find Clang headers to build compiler plugins, plugins disabled."
            fi
            ])
        dnl TODO: Windows doesn't use LO_CLANG_SHARED_PLUGINS for now, see corresponding TODO
        dnl comment in compilerplugins/Makefile-clang.mk:
        if test -n "$COMPILER_PLUGINS" && test "$_os" != "WINNT"; then
            LDFLAGS=""
            AC_MSG_CHECKING([for clang libraries to use])
            if test -z "$CLANGTOOLLIBS"; then
                LIBS="-lclangTooling -lclangDriver -lclangFrontend -lclangParse -lclangSema -lclangEdit \
 -lclangAnalysis -lclangAST -lclangLex -lclangSerialization -lclangBasic $COMPILER_PLUGINS_LINKFLAGS"
                AC_LINK_IFELSE([
                    AC_LANG_PROGRAM([[#include "clang/Basic/SourceLocation.h"]],
                        [[ clang::FullSourceLoc().dump(); ]])
                ],[CLANGTOOLLIBS="$LIBS"],[])
            fi
            if test -z "$CLANGTOOLLIBS"; then
                LIBS="-lclang-cpp $COMPILER_PLUGINS_LINKFLAGS"
                AC_LINK_IFELSE([
                    AC_LANG_PROGRAM([[#include "clang/Basic/SourceLocation.h"]],
                        [[ clang::FullSourceLoc().dump(); ]])
                ],[CLANGTOOLLIBS="$LIBS"],[])
            fi
            AC_MSG_RESULT([$CLANGTOOLLIBS])
            if test -z "$CLANGTOOLLIBS"; then
                if test "$compiler_plugins" = "yes"; then
                    AC_MSG_ERROR([Cannot find Clang libraries to build compiler plugins.])
                else
                    AC_MSG_WARN([Cannot find Clang libraries to build compiler plugins, plugins disabled])
                    add_warning "Cannot find Clang libraries to build compiler plugins, plugins disabled."
                fi
                COMPILER_PLUGINS=
            fi
            if test -n "$COMPILER_PLUGINS"; then
                if test -z "$CLANGSYSINCLUDE"; then
                    if test -n "$LLVM_CONFIG"; then
                        # Path to the clang system headers (no idea if there's a better way to get it).
                        CLANGSYSINCLUDE=$($LLVM_CONFIG --libdir)/clang/$($LLVM_CONFIG --version | sed 's/svn//')/include
                    fi
                fi
            fi
        fi
        CXX=$save_CXX
        CXXCPP=$save_CXXCPP
        CPPFLAGS=$save_CPPFLAGS
        CXXFLAGS=$save_CXXFLAGS
        LDFLAGS=$save_LDFLAGS
        LIBS="$save_LIBS"
        AC_LANG_POP([C++])
    fi
else
    if test "$enable_compiler_plugins" = "yes"; then
        AC_MSG_ERROR([Compiler plugins are currently supported only with the Clang compiler.])
    fi
fi
AC_SUBST(COMPILER_PLUGINS)
AC_SUBST(COMPILER_PLUGINS_CXX)
AC_SUBST(COMPILER_PLUGINS_CXXFLAGS)
AC_SUBST(COMPILER_PLUGINS_CXX_LINKFLAGS)
AC_SUBST(COMPILER_PLUGINS_DEBUG)
AC_SUBST(COMPILER_PLUGINS_TOOLING_ARGS)
AC_SUBST(CLANGDIR)
AC_SUBST(CLANGLIBDIR)
AC_SUBST(CLANGTOOLLIBS)
AC_SUBST(CLANGSYSINCLUDE)

# Plugin to help linker.
# Add something like LD_PLUGIN=/usr/lib64/LLVMgold.so to your autogen.input.
# This makes --enable-lto build with clang work.
AC_SUBST(LD_PLUGIN)

AC_CHECK_FUNCS(posix_fallocate, HAVE_POSIX_FALLOCATE=YES, [HAVE_POSIX_FALLOCATE=NO])
AC_SUBST(HAVE_POSIX_FALLOCATE)

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
AC_ARG_WITH(ucrt-dir,
    AS_HELP_STRING([--with-ucrt-dir],
        [path to the directory with the arch-specific MSU packages of the Windows Universal CRT redistributables
        (MS KB 2999226) for packaging into the installsets (without those the target system needs to install
        the UCRT or Visual C++ Runtimes manually). The directory must contain the following 6 files:
            Windows6.1-KB2999226-x64.msu
            Windows6.1-KB2999226-x86.msu
            Windows8.1-KB2999226-x64.msu
            Windows8.1-KB2999226-x86.msu
            Windows8-RT-KB2999226-x64.msu
            Windows8-RT-KB2999226-x86.msu
        A zip archive including those files is available from Microsoft site:
        https://www.microsoft.com/en-us/download/details.aspx?id=48234]),
,)
UCRT_REDISTDIR="$with_ucrt_dir"
if test $_os = "WINNT"; then
    find_msvc_x64_dlls
    find_msms
    MSVC_DLL_PATH=`win_short_path_for_make "$msvcdllpath"`
    MSVC_DLLS="$msvcdlls"
    MSM_PATH=`win_short_path_for_make "$msmdir"`
    # MSVC 15.3 changed it to VC141
    if echo "$msvcdllpath" | grep -q "VC141.CRT$"; then
        SCPDEFS="$SCPDEFS -DWITH_VC141_REDIST"
    else
        SCPDEFS="$SCPDEFS -DWITH_VC${VCVER}_REDIST"
    fi

    if test "$UCRT_REDISTDIR" = "no"; then
        dnl explicitly disabled
        UCRT_REDISTDIR=""
    else
        if ! test -f "$UCRT_REDISTDIR/Windows6.1-KB2999226-x64.msu" -a \
                  -f "$UCRT_REDISTDIR/Windows6.1-KB2999226-x86.msu" -a \
                  -f "$UCRT_REDISTDIR/Windows8.1-KB2999226-x64.msu" -a \
                  -f "$UCRT_REDISTDIR/Windows8.1-KB2999226-x86.msu" -a \
                  -f "$UCRT_REDISTDIR/Windows8-RT-KB2999226-x64.msu" -a \
                  -f "$UCRT_REDISTDIR/Windows8-RT-KB2999226-x86.msu"; then
            UCRT_REDISTDIR=""
            if test -n "$PKGFORMAT"; then
               for i in $PKGFORMAT; do
                   case "$i" in
                   msi)
                       AC_MSG_WARN([--without-ucrt-dir not specified or MSUs not found - installer will have runtime dependency])
                       add_warning "--without-ucrt-dir not specified or MSUs not found - installer will have runtime dependency"
                       ;;
                   esac
               done
            fi
        fi
    fi
fi

AC_SUBST(UCRT_REDISTDIR)
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
            dnl See <https://docs.oracle.com/javase/9/migrate/toc.htm#JSMIG-GUID-EEED398E-AE37-4D12-
            dnl AB10-49F82F720027> section "Windows Registry Key Changes":
            reg_get_value "$bitness" "HKEY_LOCAL_MACHINE/SOFTWARE/JavaSoft/JDK/CurrentVersion"
            if test -n "$regvalue"; then
                ver=$regvalue
                reg_get_value "$bitness" "HKEY_LOCAL_MACHINE/SOFTWARE/JavaSoft/JDK/$ver/JavaHome"
                _jdk_home=$regvalue
            fi
            if test -z "$with_jdk_home"; then
                for ver in 1.8; do
                    reg_get_value "$bitness" "HKEY_LOCAL_MACHINE/SOFTWARE/JavaSoft/Java Development Kit/$ver/JavaHome"
                    if test -n "$regvalue"; then
                        _jdk_home=$regvalue
                        break
                    fi
                done
            fi
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

    # macOS: /usr/libexec/java_home helps to set the current JDK_HOME. Actually JDK_HOME should NOT be set where java (/usr/bin/java) is located.
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
            AC_MSG_ERROR([No valid check available. Please check the block for your desired java in configure.ac])
        elif test `$JAVAINTERPRETER -version 2>&1 | $AWK '{ print }' | $GREP -c "BEA"` -gt 0; then
            AC_MSG_ERROR([No valid check available. Please check the block for your desired java in configure.ac])
        elif test `$JAVAINTERPRETER -version 2>&1 | $AWK '{ print }' | $GREP -c "IBM"` -gt 0; then
            JDK=ibm

            dnl IBM JDK specific tests
            _jdk=`$JAVAINTERPRETER -version 2>&1 | $AWK -F'"' '{ print \$2 }' | $SED s/[[-A-Za-z]]*//`
            _jdk_ver=`echo "$_jdk" | $AWK -F. '{ print (($1 * 100) + $2) * 100 + $3;}'`

            if test "$_jdk_ver" -lt 10800; then
                AC_MSG_ERROR([IBM JDK is too old, you need at least 8])
            fi

            AC_MSG_RESULT([found (IBM JDK $_jdk)])

            if test "$with_jdk_home" = ""; then
                AC_MSG_ERROR([In order to successfully build LibreOffice using the IBM JDK,
you must use the "--with-jdk-home" configure option explicitly])
            fi

            JAVA_HOME=$with_jdk_home
        else
            JDK=sun

            dnl Sun JDK specific tests
            _jdk=`$JAVAINTERPRETER -version 2>&1 | $AWK -F'"' '{ print \$2 }' | $SED '/^$/d' | $SED s/[[-A-Za-z]]*//`
            _jdk_ver=`echo "$_jdk" | $AWK -F. '{ print (($1 * 100) + $2) * 100 + $3;}'`

            if test "$_jdk_ver" -lt 10800; then
                AC_MSG_ERROR([JDK is too old, you need at least 8])
            fi
            if test "$_jdk_ver" -gt 10800; then
                JAVA_CLASSPATH_NOT_SET=TRUE
            fi

            AC_MSG_RESULT([found (JDK $_jdk)])
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
        AC_MSG_ERROR([Java not found. You need at least JDK 8])
    fi
else
    dnl Java disabled
    JAVA_HOME=
    export JAVA_HOME
fi

dnl ===================================================================
dnl Checks for javac
dnl ===================================================================
if test "$ENABLE_JAVA" != ""; then
    javacompiler="javac"
    : ${JAVA_SOURCE_VER=8}
    : ${JAVA_TARGET_VER=8}
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
fi

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
           # try to recover first by looking whether we have an alternative
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
        dnl now that we probably have the path to the real javac, make a JAVA_HOME out of it...
        if test "$JAVA_HOME" != "/usr"; then
            if test "$_os" = "Darwin" -o "$OS_FOR_BUILD" = MACOSX; then
                dnl Leopard returns a non-suitable path with readlink - points to "Current" only
                JAVA_HOME=$(echo $JAVA_HOME | $SED -e s,/Current/Commands/javac$,/CurrentJDK/Home,)
                dnl Tiger already returns a JDK path...
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

    # now check if $JAVA_HOME is really valid
    if test "$_os" = "Darwin" -o "$OS_FOR_BUILD" = MACOSX; then
        case "${JAVA_HOME}" in
            /Library/Java/JavaVirtualMachines/*)
                ;;
            *)
                AC_MSG_ERROR([JDK in $JAVA_HOME cannot be used in CppUnit tests - install Oracle JDK])
                ;;
        esac
        if test ! -f "$JAVA_HOME/lib/jvm.cfg" -a "x$with_jdk_home" = "x"; then
            JAVA_HOME_OK="NO"
        fi
    elif test ! -d "$JAVA_HOME/jre" -a "x$with_jdk_home" = "x"; then
        JAVA_HOME_OK="NO"
    fi
    if test "$JAVA_HOME_OK" = "NO"; then
        AC_MSG_WARN([JAVA_HOME was not explicitly informed with --with-jdk-home. the configure script])
        AC_MSG_WARN([attempted to find JAVA_HOME automatically, but apparently it failed])
        AC_MSG_WARN([in case JAVA_HOME is incorrectly set, some projects will not be built correctly])
        add_warning "JAVA_HOME was not explicitly informed with --with-jdk-home. the configure script"
        add_warning "attempted to find JAVA_HOME automatically, but apparently it failed"
        add_warning "in case JAVA_HOME is incorrectly set, some projects will not be built correctly"
    fi
    PathFormat "$JAVA_HOME"
    JAVA_HOME="$formatted_path"
fi

if test -z "$JAWTLIB" -a -n "$ENABLE_JAVA" -a "$_os" != Android -a \
    "$_os" != Darwin
then
    AC_MSG_CHECKING([for JAWT lib])
    if test "$_os" = WINNT; then
        # The path to $JAVA_HOME/lib/$JAWTLIB is part of $ILIB:
        JAWTLIB=jawt.lib
    else
        case "$host_cpu" in
        arm*)
            AS_IF([test -e "$JAVA_HOME/jre/lib/aarch32/libjawt.so"], [my_java_arch=aarch32], [my_java_arch=arm])
            JAVA_ARCH=$my_java_arch
            ;;
        i*86)
            my_java_arch=i386
            ;;
        m68k)
            my_java_arch=m68k
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
        sparc64)
            my_java_arch=sparcv9
            ;;
        x86_64)
            my_java_arch=amd64
            ;;
        *)
            my_java_arch=$host_cpu
            ;;
        esac
        # This is where JDK9 puts the library
        if test -e "$JAVA_HOME/lib/libjawt.so"; then
            JAWTLIB="-L$JAVA_HOME/lib/ -ljawt"
        else
            JAWTLIB="-L$JAVA_HOME/jre/lib/$my_java_arch -ljawt"
        fi
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


dnl ===================================================================
dnl Export file validation
dnl ===================================================================
AC_MSG_CHECKING([whether to enable export file validation])
if test "$with_export_validation" != "no"; then
    if test -z "$ENABLE_JAVA"; then
        if test "$with_export_validation" = "yes"; then
            AC_MSG_ERROR([requested, but Java is disabled])
        else
            AC_MSG_RESULT([no, as Java is disabled])
        fi
    elif ! test -d "${SRC_ROOT}/schema"; then
        if test "$with_export_validation" = "yes"; then
            AC_MSG_ERROR([requested, but schema directory is missing (it is excluded from tarballs)])
        else
            AC_MSG_RESULT([no, schema directory is missing (it is excluded from tarballs)])
        fi
    else
        AC_MSG_RESULT([yes])
        AC_DEFINE(HAVE_EXPORT_VALIDATION)

        AC_PATH_PROGS(ODFVALIDATOR, odfvalidator)
        if test -z "$ODFVALIDATOR"; then
            # remember to download the ODF toolkit with validator later
            AC_MSG_NOTICE([no odfvalidator found, will download it])
            BUILD_TYPE="$BUILD_TYPE ODFVALIDATOR"
            ODFVALIDATOR="$BUILDDIR/bin/odfvalidator.sh"

            # and fetch name of odfvalidator jar name from download.lst
            ODFVALIDATOR_JAR=`$SED -n -e "s/export *ODFVALIDATOR_JAR *:= *\(.*\) */\1/p" $SRC_ROOT/download.lst`
            AC_SUBST(ODFVALIDATOR_JAR)

            if test -z "$ODFVALIDATOR_JAR"; then
                AC_MSG_ERROR([cannot determine odfvalidator jar location (--with-export-validation)])
            fi
        fi
        if test "$build_os" = "cygwin"; then
            # In case of Cygwin it will be executed from Windows,
            # so we need to run bash and absolute path to validator
            # so instead of "odfvalidator" it will be
            # something like "bash.exe C:\cygwin\opt\lo\bin\odfvalidator"
            ODFVALIDATOR="bash.exe `cygpath -m "$ODFVALIDATOR"`"
        else
            ODFVALIDATOR="sh $ODFVALIDATOR"
        fi
        AC_SUBST(ODFVALIDATOR)


        AC_PATH_PROGS(OFFICEOTRON, officeotron)
        if test -z "$OFFICEOTRON"; then
            # remember to download the officeotron with validator later
            AC_MSG_NOTICE([no officeotron found, will download it])
            BUILD_TYPE="$BUILD_TYPE OFFICEOTRON"
            OFFICEOTRON="$BUILDDIR/bin/officeotron.sh"

            # and fetch name of officeotron jar name from download.lst
            OFFICEOTRON_JAR=`$SED -n -e "s/export *OFFICEOTRON_JAR *:= *\(.*\) */\1/p" $SRC_ROOT/download.lst`
            AC_SUBST(OFFICEOTRON_JAR)

            if test -z "$OFFICEOTRON_JAR"; then
                AC_MSG_ERROR([cannot determine officeotron jar location (--with-export-validation)])
            fi
        else
            # check version of existing officeotron
            OFFICEOTRON_VER=`$OFFICEOTRON --version | $AWK -F. '{ print \$1*10000+\$2*100+\$3 }'`
            if test 0"$OFFICEOTRON_VER" -lt 704; then
                AC_MSG_ERROR([officeotron too old])
            fi
        fi
        if test "$build_os" = "cygwin"; then
            # In case of Cygwin it will be executed from Windows,
            # so we need to run bash and absolute path to validator
            # so instead of "odfvalidator" it will be
            # something like "bash.exe C:\cygwin\opt\lo\bin\odfvalidator"
            OFFICEOTRON="bash.exe `cygpath -m "$OFFICEOTRON"`"
        else
            OFFICEOTRON="sh $OFFICEOTRON"
        fi
    fi
    AC_SUBST(OFFICEOTRON)
else
    AC_MSG_RESULT([no])
fi

AC_MSG_CHECKING([for Microsoft Binary File Format Validator])
if test "$with_bffvalidator" != "no"; then
    AC_DEFINE(HAVE_BFFVALIDATOR)

    if test "$with_export_validation" = "no"; then
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
        # We are not in Cygwin but need to run Windows binary with wine
        AC_PATH_PROGS(WINE, wine)

        # so swap in a shell wrapper that converts paths transparently
        BFFVALIDATOR_EXE="$BFFVALIDATOR"
        BFFVALIDATOR="sh $BUILDDIR/bin/bffvalidator.sh"
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
dnl Check for C preprocessor to use
dnl ===================================================================
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
                if grep "Patched for .*Office" $EPM >/dev/null 2>/dev/null; then
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
                    AC_MSG_WARN([the ${_pt}s will not be relocatable])
                    add_warning "the ${_pt}s will not be relocatable"
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

ENABLE_LWP=
if test "$enable_lotuswordpro" = "yes"; then
    ENABLE_LWP="TRUE"
fi
AC_SUBST(ENABLE_LWP)

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
            if $DOXYGEN -g - | grep -q "HAVE_DOT *= *YES"; then
                if ! dot -V 2>/dev/null; then
                    AC_MSG_ERROR([dot not found in \$PATH but doxygen defaults to HAVE_DOT=YES; install graphviz or disable its use via --without-doxygen])
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
        if test "$enable_fuzzers" != "yes"; then
            with_system_zlib=yes
        else
            with_system_zlib=no
        fi
        ;;
    esac
fi

dnl we want to use libo_CHECK_SYSTEM_MODULE here too, but macOS is too stupid
dnl and has no pkg-config for it at least on some tinderboxes,
dnl so leaving that out for now
dnl libo_CHECK_SYSTEM_MODULE([zlib],[ZLIB],[zlib])
AC_MSG_CHECKING([which zlib to use])
if test "$with_system_zlib" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_ZLIB=TRUE
    AC_CHECK_HEADER(zlib.h, [],
        [AC_MSG_ERROR(zlib.h not found. install zlib)], [])
    AC_CHECK_LIB(z, deflate, [ ZLIB_LIBS=-lz ],
        [AC_MSG_ERROR(zlib not found or functional)], [])
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
if test "$with_system_jpeg" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_LIBJPEG=TRUE
    AC_CHECK_HEADER(jpeglib.h, [ LIBJPEG_CFLAGS= ],
        [AC_MSG_ERROR(jpeg.h not found. install libjpeg)], [])
    AC_CHECK_LIB(jpeg, jpeg_resync_to_restart, [ LIBJPEG_LIBS="-ljpeg" ],
        [AC_MSG_ERROR(jpeg library not found or functional)], [])
else
    SYSTEM_LIBJPEG=
    AC_MSG_RESULT([internal, libjpeg-turbo])
    BUILD_TYPE="$BUILD_TYPE LIBJPEG_TURBO"
    LIBJPEG_CFLAGS="-I${WORKDIR}/UnpackedTarball/libjpeg-turbo"
    if test "$COM" = "MSC"; then
        LIBJPEG_LIBS="${WORKDIR}/UnpackedTarball/libjpeg-turbo/.libs/libjpeg.lib"
    else
        LIBJPEG_LIBS="-L${WORKDIR}/UnpackedTarball/libjpeg-turbo/.libs -ljpeg"
    fi

    case "$host_cpu" in
    x86_64 | amd64 | i*86 | x86 | ia32)
        AC_CHECK_PROGS(NASM, [nasm nasmw yasm])
        if test -z "$NASM" -a "$build_os" = "cygwin"; then
            if test -n "$LODE_HOME" -a -x "$LODE_HOME/opt/bin/nasm"; then
                NASM="$LODE_HOME/opt/bin/nasm"
            elif test -x "/opt/lo/bin/nasm"; then
                NASM="/opt/lo/bin/nasm"
            fi
        fi

        if test -n "$NASM"; then
            AC_MSG_CHECKING([for object file format of host system])
            case "$host_os" in
              cygwin* | mingw* | pw32* | interix*)
                case "$host_cpu" in
                  x86_64)
                    objfmt='Win64-COFF'
                    ;;
                  *)
                    objfmt='Win32-COFF'
                    ;;
                esac
              ;;
              msdosdjgpp* | go32*)
                objfmt='COFF'
              ;;
              os2-emx*) # not tested
                objfmt='MSOMF' # obj
              ;;
              linux*coff* | linux*oldld*)
                objfmt='COFF' # ???
              ;;
              linux*aout*)
                objfmt='a.out'
              ;;
              linux*)
                case "$host_cpu" in
                  x86_64)
                    objfmt='ELF64'
                    ;;
                  *)
                    objfmt='ELF'
                    ;;
                esac
              ;;
              kfreebsd* | freebsd* | netbsd* | openbsd*)
                if echo __ELF__ | $CC -E - | grep __ELF__ > /dev/null; then
                  objfmt='BSD-a.out'
                else
                  case "$host_cpu" in
                    x86_64 | amd64)
                      objfmt='ELF64'
                      ;;
                    *)
                      objfmt='ELF'
                      ;;
                  esac
                fi
              ;;
              solaris* | sunos* | sysv* | sco*)
                case "$host_cpu" in
                  x86_64)
                    objfmt='ELF64'
                    ;;
                  *)
                    objfmt='ELF'
                    ;;
                esac
              ;;
              darwin* | rhapsody* | nextstep* | openstep* | macos*)
                case "$host_cpu" in
                  x86_64)
                    objfmt='Mach-O64'
                    ;;
                  *)
                    objfmt='Mach-O'
                    ;;
                esac
              ;;
              *)
                objfmt='ELF ?'
              ;;
            esac

            AC_MSG_RESULT([$objfmt])
            if test "$objfmt" = 'ELF ?'; then
              objfmt='ELF'
              AC_MSG_WARN([unexpected host system. assumed that the format is $objfmt.])
            fi

            AC_MSG_CHECKING([for object file format specifier (NAFLAGS) ])
            case "$objfmt" in
              MSOMF)      NAFLAGS='-fobj -DOBJ32';;
              Win32-COFF) NAFLAGS='-fwin32 -DWIN32';;
              Win64-COFF) NAFLAGS='-fwin64 -DWIN64 -D__x86_64__';;
              COFF)       NAFLAGS='-fcoff -DCOFF';;
              a.out)      NAFLAGS='-faout -DAOUT';;
              BSD-a.out)  NAFLAGS='-faoutb -DAOUT';;
              ELF)        NAFLAGS='-felf -DELF';;
              ELF64)      NAFLAGS='-felf64 -DELF -D__x86_64__';;
              RDF)        NAFLAGS='-frdf -DRDF';;
              Mach-O)     NAFLAGS='-fmacho -DMACHO';;
              Mach-O64)   NAFLAGS='-fmacho64 -DMACHO -D__x86_64__';;
            esac
            AC_MSG_RESULT([$NAFLAGS])

            AC_MSG_CHECKING([whether the assembler ($NASM $NAFLAGS) works])
            cat > conftest.asm << EOF
            [%line __oline__ "configure"
                    section .text
                    global  _main,main
            _main:
            main:   xor     eax,eax
                    ret
            ]
EOF
            try_nasm='$NASM $NAFLAGS -o conftest.o conftest.asm'
            if AC_TRY_EVAL(try_nasm) && test -s conftest.o; then
              AC_MSG_RESULT(yes)
            else
              echo "configure: failed program was:" >&AC_FD_CC
              cat conftest.asm >&AC_FD_CC
              rm -rf conftest*
              AC_MSG_RESULT(no)
              AC_MSG_WARN([installation or configuration problem: assembler cannot create object files.])
              NASM=""
            fi

        fi

        if test -z "$NASM"; then
cat << _EOS
****************************************************************************
You need yasm or nasm (Netwide Assembler) to build the internal jpeg library optimally.
To get one please:

_EOS
            if test "$build_os" = "cygwin"; then
cat << _EOS
install a pre-compiled binary for Win32

mkdir -p /opt/lo/bin
cd /opt/lo/bin
wget https://dev-www.libreoffice.org/bin/cygwin/nasm.exe
chmod +x nasm

or get and install one from http://www.nasm.us/

Then re-run autogen.sh

Note: autogen.sh will try to use /opt/lo/bin/nasm if the environment variable NASM is not already defined.
Alternatively, you can install the 'new' nasm where ever you want and make sure that \`which nasm\` finds it.

_EOS
            else
cat << _EOS
consult https://github.com/libjpeg-turbo/libjpeg-turbo/blob/master/BUILDING.md

_EOS
            fi
            AC_MSG_WARN([no suitable nasm (Netwide Assembler) found])
            add_warning "no suitable nasm (Netwide Assembler) found for internal libjpeg-turbo"
        fi
      ;;
    esac
fi

AC_SUBST(NASM)
AC_SUBST(LIBJPEG_CFLAGS)
AC_SUBST(LIBJPEG_LIBS)
AC_SUBST(SYSTEM_LIBJPEG)

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
libo_CHECK_SYSTEM_MODULE([expat], [EXPAT], [expat])

dnl ===================================================================
dnl Check for system xmlsec
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([xmlsec], [XMLSEC], [xmlsec1-nss >= 1.2.28])

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
dnl Check for DLP libs
dnl ===================================================================
AS_IF([test "$COM" = "MSC"],
      [librevenge_libdir="${WORKDIR}/LinkTarget/Library"],
      [librevenge_libdir="${WORKDIR}/UnpackedTarball/librevenge/src/lib/.libs"]
)
libo_CHECK_SYSTEM_MODULE([librevenge],[REVENGE],[librevenge-0.0 >= 0.0.1],["-I${WORKDIR}/UnpackedTarball/librevenge/inc"],["-L${librevenge_libdir} -lrevenge-0.0"])

libo_CHECK_SYSTEM_MODULE([libodfgen],[ODFGEN],[libodfgen-0.1])

libo_CHECK_SYSTEM_MODULE([libepubgen],[EPUBGEN],[libepubgen-0.1])

AS_IF([test "$COM" = "MSC"],
      [libwpd_libdir="${WORKDIR}/LinkTarget/Library"],
      [libwpd_libdir="${WORKDIR}/UnpackedTarball/libwpd/src/lib/.libs"]
)
libo_CHECK_SYSTEM_MODULE([libwpd],[WPD],[libwpd-0.10],["-I${WORKDIR}/UnpackedTarball/libwpd/inc"],["-L${libwpd_libdir} -lwpd-0.10"])

libo_CHECK_SYSTEM_MODULE([libwpg],[WPG],[libwpg-0.3])

libo_CHECK_SYSTEM_MODULE([libwps],[WPS],[libwps-0.4])
libo_PKG_VERSION([WPS], [libwps-0.4], [0.4.10])

libo_CHECK_SYSTEM_MODULE([libvisio],[VISIO],[libvisio-0.1])

libo_CHECK_SYSTEM_MODULE([libcdr],[CDR],[libcdr-0.1])

libo_CHECK_SYSTEM_MODULE([libmspub],[MSPUB],[libmspub-0.1])

libo_CHECK_SYSTEM_MODULE([libmwaw],[MWAW],[libmwaw-0.3 >= 0.3.1])
libo_PKG_VERSION([MWAW], [libmwaw-0.3], [0.3.16])

libo_CHECK_SYSTEM_MODULE([libetonyek],[ETONYEK],[libetonyek-0.1])
libo_PKG_VERSION([ETONYEK], [libetonyek-0.1], [0.1.10])

libo_CHECK_SYSTEM_MODULE([libfreehand],[FREEHAND],[libfreehand-0.1])

libo_CHECK_SYSTEM_MODULE([libebook],[EBOOK],[libe-book-0.1])
libo_PKG_VERSION([EBOOK], [libe-book-0.1], [0.1.2])

libo_CHECK_SYSTEM_MODULE([libabw],[ABW],[libabw-0.1])

libo_CHECK_SYSTEM_MODULE([libpagemaker],[PAGEMAKER],[libpagemaker-0.0])

libo_CHECK_SYSTEM_MODULE([libqxp],[QXP],[libqxp-0.0])

libo_CHECK_SYSTEM_MODULE([libzmf],[ZMF],[libzmf-0.0])

libo_CHECK_SYSTEM_MODULE([libstaroffice],[STAROFFICE],[libstaroffice-0.0])
libo_PKG_VERSION([STAROFFICE], [libstaroffice-0.0], [0.0.6])

dnl ===================================================================
dnl Check for system lcms2
dnl ===================================================================
if test "$with_system_lcms2" != "yes"; then
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
if test "$_os" != "Android" ; then
    libo_CHECK_SYSTEM_MODULE([cppunit],[CPPUNIT],[cppunit >= 1.14.0])
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
else
    FREETYPE_CFLAGS="${ISYSTEM}${WORKDIR}/UnpackedTarball/freetype/include"
    if test "x$ac_config_site_64bit_host" = xYES; then
        FREETYPE_LIBS="-L${WORKDIR}/UnpackedTarball/freetype/instdir/lib64 -lfreetype"
    else
        FREETYPE_LIBS="-L${WORKDIR}/UnpackedTarball/freetype/instdir/lib -lfreetype"
    fi
fi
AC_SUBST(FREETYPE_CFLAGS)
AC_SUBST(FREETYPE_LIBS)
AC_SUBST([SYSTEM_FREETYPE])

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
        if test "$enable_fuzzers" != "yes"; then
            with_system_libxml=yes
        else
            with_system_libxml=no
        fi
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
        LIBEXSLT_LIBS=$(printf '%s' "${filteredlibs}" | sed -e "s/-lgpg-error//"  -e "s/-lgcrypt//")
    fi

    dnl Check for xsltproc
    AC_PATH_PROG(XSLTPROC, xsltproc, no)
    if test "$XSLTPROC" = "no"; then
        AC_MSG_ERROR([xsltproc is required])
    fi
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
else
    AC_MSG_RESULT([internal])
    SYSTEM_LIBXML=
    LIBXML_CFLAGS="-I${WORKDIR}/UnpackedTarball/libxml2/include"
    if test "$COM" = "MSC"; then
        LIBXML_CFLAGS="${LIBXML_CFLAGS} -I${WORKDIR}/UnpackedTarball/icu/source/i18n -I${WORKDIR}/UnpackedTarball/icu/source/common"
    fi
    if test "$COM" = "MSC"; then
        LIBXML_LIBS="${WORKDIR}/UnpackedTarball/libxml2/win32/bin.msvc/libxml2.lib"
    else
        LIBXML_LIBS="-L${WORKDIR}/UnpackedTarball/libxml2/.libs -lxml2"
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
# Checking for a Python interpreter with version >= 2.7.
# Build and runtime requires Python 3 compatible version (>= 2.7).
# Optionally user can pass an option to configure, i. e.
# ./configure PYTHON=/usr/bin/python
# =====================================================================
if test $_os = Darwin -a "$enable_python" != no -a "$enable_python" != fully-internal -a "$enable_python" != internal; then
    # Only allowed choices for macOS are 'no', 'internal' (default), and 'fully-internal'
    enable_python=internal
fi
if test "$build_os" != "cygwin" -a "$enable_python" != fully-internal; then
    if test -n "$PYTHON"; then
        PYTHON_FOR_BUILD=$PYTHON
    else
        # This allows a lack of system python with no error, we use internal one in that case.
        AM_PATH_PYTHON([2.7],, [:])
        # Clean PYTHON_VERSION checked below if cross-compiling
        PYTHON_VERSION=""
        if test "$PYTHON" != ":"; then
            PYTHON_FOR_BUILD=$PYTHON
        fi
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
        AC_MSG_RESULT([fully internal])
        enable_python=internal
    elif test "$cross_compiling" = yes; then
        AC_MSG_RESULT([system])
        enable_python=system
    else
        # Unset variables set by the above AM_PATH_PYTHON so that
        # we actually do check anew.
        AC_MSG_RESULT([])
        unset PYTHON am_cv_pathless_PYTHON ac_cv_path_PYTHON am_cv_python_version am_cv_python_platform am_cv_python_pythondir am_cv_python_pyexecdir
        AM_PATH_PYTHON([3.3],, [:])
        AC_MSG_CHECKING([which Python to use for Pyuno])
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
    if test "$_os" = Darwin; then
        AC_MSG_ERROR([--enable-python=system doesn't work on macOS: /System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7/unicodeobject.h is known to contain uses of "register", which is removed from C++17])
    fi
    ;;
*)
    AC_MSG_ERROR([Incorrect --enable-python option])
    ;;
esac

if test $enable_python != no; then
    BUILD_TYPE="$BUILD_TYPE PYUNO"
fi

if test $enable_python = system; then
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
        elif $PKG_CONFIG --exists python-$python_version-embed; then
            PYTHON_CFLAGS="`$PKG_CONFIG --cflags python-$python_version-embed`"
            PYTHON_LIBS="`$PKG_CONFIG --libs python-$python_version-embed` $python_libs"
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
        AC_MSG_CHECKING([for python version])
        AS_IF([test -n "$PYTHON_VERSION"],
              [AC_MSG_RESULT([$PYTHON_VERSION])],
              [AC_MSG_RESULT([not found])
               AC_MSG_ERROR([no usable python found])])
        test -n "$PYTHON_CFLAGS" && break
    fi

    dnl Check if the headers really work
    save_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $PYTHON_CFLAGS"
    AC_CHECK_HEADER(Python.h)
    CPPFLAGS="$save_CPPFLAGS"

    # let the PYTHON_FOR_BUILD match the same python installation that
    # provides PYTHON_CFLAGS/PYTHON_LDFLAGS for pyuno, which should be
    # better for PythonTests.
    PYTHON_FOR_BUILD=$PYTHON
fi

if test "$with_lxml" != no; then
    if test -z "$PYTHON_FOR_BUILD"; then
        case $build_os in
            cygwin)
                AC_MSG_WARN([No system-provided python lxml, gla11y will only report widget classes and ids])
                ;;
            *)
                if test "$cross_compiling" != yes ; then
                    BUILD_TYPE="$BUILD_TYPE LXML"
                fi
                ;;
        esac
    else
        AC_MSG_CHECKING([for python lxml])
        if $PYTHON_FOR_BUILD -c "import lxml.etree as ET" 2> /dev/null ; then
            AC_MSG_RESULT([yes])
        else
            case $build_os in
                cygwin)
                    AC_MSG_RESULT([no, gla11y will only report widget classes and ids])
                    ;;
                *)
                    if test "$cross_compiling" != yes -a "x$ac_cv_header_Python_h" = "xyes"; then
                        if test -n ${SYSTEM_LIBXSLT} -o -n ${SYSTEM_LIBXML}; then
                            AC_MSG_RESULT([no, and no system libxml/libxslt, gla11y will only report widget classes and ids])
                        else
                            BUILD_TYPE="$BUILD_TYPE LXML"
                            AC_MSG_RESULT([no, using internal lxml])
                        fi
                    else
                        AC_MSG_RESULT([no, and system does not provide python development headers, gla11y will only report widget classes and ids])
                    fi
                    ;;
            esac
        fi
    fi
fi

dnl By now enable_python should be "system", "internal" or "no"
case $enable_python in
system)
    SYSTEM_PYTHON=TRUE

    if test "x$ac_cv_header_Python_h" != "xyes"; then
       AC_MSG_ERROR([Python headers not found. You probably want to set both the PYTHON_CFLAGS and PYTHON_LIBS environment variables.])
    fi

    AC_LANG_PUSH(C)
    CFLAGS="$CFLAGS $PYTHON_CFLAGS"
    AC_MSG_CHECKING([for correct python library version])
       AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <Python.h>

int main(int argc, char **argv) {
   if ((PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION >= 7) ||
       (PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 3)) return 0;
   else return 1;
}
       ]])],[AC_MSG_RESULT([ok])],[AC_MSG_ERROR([Python >= 3.3 is needed when building with Python 3, or Python >= 2.7 when building with Python 2])],[AC_MSG_RESULT([skipped; cross-compiling])])
    CFLAGS=$save_CFLAGS
    AC_LANG_POP(C)

    dnl FIXME Check if the Python library can be linked with, too?
    ;;

internal)
    SYSTEM_PYTHON=
    PYTHON_VERSION_MAJOR=3
    PYTHON_VERSION_MINOR=7
    PYTHON_VERSION=${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}.10
    if ! grep -q -i python.*${PYTHON_VERSION} ${SRC_ROOT}/download.lst; then
        AC_MSG_ERROR([PYTHON_VERSION ${PYTHON_VERSION} but no matching file in download.lst])
    fi
    AC_DEFINE_UNQUOTED([PYTHON_VERSION_STRING], [L"3.7.7"])
    BUILD_TYPE="$BUILD_TYPE PYTHON"
    if test "$OS" = LINUX; then
        BUILD_TYPE="$BUILD_TYPE LIBFFI"
    fi
    # Embedded Python dies without Home set
    if test "$HOME" = ""; then
        export HOME=""
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

ENABLE_MARIADBC=TRUE
if test "$_os" = "Android" -o "$_os" = "iOS" -o "$enable_mpl_subset" = "yes"; then
    ENABLE_MARIADBC=
fi
MARIADBC_MAJOR=1
MARIADBC_MINOR=0
MARIADBC_MICRO=2
if test "$ENABLE_MARIADBC" = "TRUE"; then
    BUILD_TYPE="$BUILD_TYPE MARIADBC"
fi

AC_SUBST(ENABLE_MARIADBC)
AC_SUBST(MARIADBC_MAJOR)
AC_SUBST(MARIADBC_MINOR)
AC_SUBST(MARIADBC_MICRO)

if test "$ENABLE_MARIADBC" = "TRUE"; then
    dnl ===================================================================
    dnl Check for system MariaDB
    dnl ===================================================================
    AC_MSG_CHECKING([which MariaDB to use])
    if test "$with_system_mariadb" = "yes"; then
        AC_MSG_RESULT([external])
        SYSTEM_MARIADB_CONNECTOR_C=TRUE
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
            BUNDLE_MARIADB_CONNECTOR_C=TRUE
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
            BUNDLE_MARIADB_CONNECTOR_C=
        fi
    else
        AC_MSG_RESULT([internal])
        SYSTEM_MARIADB_CONNECTOR_C=
        MARIADB_CFLAGS="-I${WORKDIR}/UnpackedTarball/mariadb-connector-c/include"
        MARIADB_LIBS="-L${WORKDIR}/LinkTarget/StaticLibrary -lmariadb-connector-c"
        BUILD_TYPE="$BUILD_TYPE MARIADB_CONNECTOR_C"
    fi

    AC_SUBST(SYSTEM_MARIADB_CONNECTOR_C)
    AC_SUBST(MARIADB_CFLAGS)
    AC_SUBST(MARIADB_LIBS)
    AC_SUBST(LIBMARIADB)
    AC_SUBST(LIBMARIADB_PATH)
    AC_SUBST(BUNDLE_MARIADB_CONNECTOR_C)
fi

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
        NEED_ANT=TRUE
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
AC_MSG_CHECKING([whether to build the PostgreSQL SDBC driver])
if test "x$enable_postgresql_sdbc" != "xno"; then
    AC_MSG_RESULT([yes])
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
        if test -n "$PGCONFIG"; then
            POSTGRESQL_INC=-I$(${PGCONFIG} --includedir)
            POSTGRESQL_LIB="-L$(${PGCONFIG} --libdir)"
        else
            PKG_CHECK_MODULES(POSTGRESQL, libpq, [
              POSTGRESQL_INC=$POSTGRESQL_CFLAGS
              POSTGRESQL_LIB=$POSTGRESQL_LIBS
            ],[
              AC_MSG_ERROR([pg_config or 'pkg-config libpq' needed; set PGCONFIG if not in PATH])
            ])
        fi
        FilterLibs "${POSTGRESQL_LIB}"
        POSTGRESQL_LIB="${filteredlibs}"
    else
        # if/when anything else than PostgreSQL uses Kerberos,
        # move this out of `test "x$enable_postgresql_sdbc" != "xno"'
        WITH_KRB5=
        WITH_GSSAPI=
        case "$_os" in
        Darwin)
            # macOS has system MIT Kerberos 5 since 10.4
            if test "$with_krb5" != "no"; then
                WITH_KRB5=TRUE
                save_LIBS=$LIBS
                # Not sure whether it makes any sense here to search multiple potential libraries; it is not likely
                # that the libraries where these functions are located on macOS will change, is it?
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
else
    AC_MSG_RESULT([no])
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
ENABLE_FIREBIRD_SDBC=
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
            if test "$FIREBIRD_MAJOR" -eq "3" -a "$FIREBIRD_MINOR" -eq "0"; then
                AC_MSG_RESULT([OK])
            else
                AC_MSG_ERROR([Ensure firebird 3.0.x is installed])
            fi
        else
            __save_CFLAGS="${CFLAGS}"
            CFLAGS="${CFLAGS} ${FIREBIRD_CFLAGS}"
            AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <ibase.h>
#if defined(FB_API_VER) && FB_API_VER == 30
int fb_api_is_30(void) { return 0; }
#else
#error "Wrong Firebird API version"
#endif]])],AC_MSG_RESULT([OK]),AC_MSG_ERROR([Ensure firebird 3.0.x is installed]))
            CFLAGS="${__save_CFLAGS}"
        fi
        ENABLE_FIREBIRD_SDBC=TRUE
        AC_DEFINE([ENABLE_FIREBIRD_SDBC],1)
    elif test "$enable_database_connectivity" != yes; then
        AC_MSG_RESULT([none])
    elif test "$cross_compiling" = "yes"; then
        AC_MSG_RESULT([none])
    else
        dnl Embedded Firebird has version 3.0
        AC_DEFINE(HAVE_FIREBIRD_30, 1)
        dnl We need libatomic_ops for any non X86/X64 system
        if test "${CPUNAME}" != INTEL -a "${CPUNAME}" != X86_64; then
            dnl ===================================================================
            dnl Check for system libatomic_ops
            dnl ===================================================================
            libo_CHECK_SYSTEM_MODULE([libatomic_ops],[ATOMIC_OPS],[atomic_ops >= 0.7.2])
            if test "$with_system_libatomic_ops" = "yes"; then
                SYSTEM_LIBATOMIC_OPS=TRUE
                AC_CHECK_HEADERS(atomic_ops.h, [],
                [AC_MSG_ERROR(atomic_ops.h not found. install libatomic_ops)], [])
            else
                SYSTEM_LIBATOMIC_OPS=
                LIBATOMIC_OPS_CFLAGS="-I${WORKDIR}/UnpackedTarball/libatomic_ops/include"
                LIBATOMIC_OPS_LIBS="-latomic_ops"
                BUILD_TYPE="$BUILD_TYPE LIBATOMIC_OPS"
            fi
        fi

        AC_MSG_RESULT([internal])
        SYSTEM_FIREBIRD=
        FIREBIRD_CFLAGS="-I${WORKDIR}/UnpackedTarball/firebird/gen/Release/firebird/include"
        FIREBIRD_LIBS="-lfbclient"

        if test "$with_system_libtommath" = "yes"; then
            SYSTEM_LIBTOMMATH=TRUE
            dnl check for tommath presence
            save_LIBS=$LIBS
            AC_CHECK_HEADER(tommath.h,,AC_MSG_ERROR(Include file for tommath not found - please install development tommath package))
            AC_CHECK_LIB(tommath, mp_init, TOMMATH_LIBS=-ltommath, AC_MSG_ERROR(Library tommath not found - please install development tommath package))
            LIBS=$save_LIBS
        else
            SYSTEM_LIBTOMMATH=
            LIBTOMMATH_CFLAGS="-I${WORKDIR}/UnpackedTarball/libtommath"
            LIBTOMMATH_LIBS="-ltommath"
            BUILD_TYPE="$BUILD_TYPE LIBTOMMATH"
        fi

        BUILD_TYPE="$BUILD_TYPE FIREBIRD"
        ENABLE_FIREBIRD_SDBC=TRUE
        AC_DEFINE([ENABLE_FIREBIRD_SDBC],1)
    fi
fi
AC_SUBST(ENABLE_FIREBIRD_SDBC)
AC_SUBST(SYSTEM_LIBATOMIC_OPS)
AC_SUBST(LIBATOMIC_OPS_CFLAGS)
AC_SUBST(LIBATOMIC_OPS_LIBS)
AC_SUBST(SYSTEM_FIREBIRD)
AC_SUBST(FIREBIRD_CFLAGS)
AC_SUBST(FIREBIRD_LIBS)
AC_SUBST([TOMMATH_CFLAGS])
AC_SUBST([TOMMATH_LIBS])

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

    ENABLE_CURL=TRUE
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
    AX_BOOST_LOCALE
    AC_LANG_PUSH([C++])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS $BOOST_CPPFLAGS $CXXFLAGS_CXX11"
    AC_CHECK_HEADER(boost/shared_ptr.hpp, [],
       [AC_MSG_ERROR(boost/shared_ptr.hpp not found. install boost)], [])
    AC_CHECK_HEADER(boost/spirit/include/classic_core.hpp, [],
       [AC_MSG_ERROR(boost/spirit/include/classic_core.hpp not found. install boost >= 1.36)], [])
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
libo_CHECK_SYSTEM_MODULE([mdds], [MDDS], [mdds-1.5 >= 1.5.0], ["-I${WORKDIR}/UnpackedTarball/mdds/include"])

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
dnl Enable LDAP support
dnl ===================================================================

if test "$_os" != "WINNT" -a "$_os" != "iOS" -a "$_os" != "Android"; then
AC_MSG_CHECKING([whether to enable LDAP support])
    if test "$enable_ldap" != "yes"; then
        AC_MSG_RESULT([no])
        ENABLE_LDAP=""
        enable_ldap=no
    else
        AC_MSG_RESULT([yes])
        ENABLE_LDAP="TRUE"
        AC_DEFINE(HAVE_FEATURE_LDAP)
    fi
fi
AC_SUBST(ENABLE_LDAP)

dnl ===================================================================
dnl Check for system openldap
dnl ===================================================================

if test "$_os" != "WINNT" -a "$_os" != "iOS" -a "$_os" != "Android" -a "$ENABLE_LDAP" != ""; then
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
dnl Check for system NSS
dnl ===================================================================
if test "$enable_fuzzers" != "yes"; then
    libo_CHECK_SYSTEM_MODULE([nss],[NSS],[nss >= 3.9.3 nspr >= 4.8])
    AC_DEFINE(HAVE_FEATURE_NSS)
    ENABLE_NSS="TRUE"
    AC_DEFINE(ENABLE_NSS)
elif test $_os != iOS ; then
    with_tls=openssl
fi
AC_SUBST(ENABLE_NSS)

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
        AC_MSG_WARN([TLS/SSL implementation to use is OpenSSL but some code may still depend on NSS or GNUTLS])
        add_warning "TLS/SSL implementation to use is OpenSSL but some code may still depend on NSS or GNUTLS"
        ;;
    nss)
        AC_DEFINE(USE_TLS_NSS)
        TLS=NSS
        ;;
    no)
        AC_MSG_WARN([Skipping TLS/SSL])
        ;;
    *)
        AC_MSG_ERROR([unsupported implementation $with_tls. Supported are:
openssl - OpenSSL
nss - Mozilla's Network Security Services (NSS)
    ])
        ;;
    esac
else
    # default to using NSS, it results in smaller oox lib
    AC_DEFINE(USE_TLS_NSS)
    TLS=NSS
fi
AC_MSG_RESULT([$TLS])
AC_SUBST(TLS)

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

ICU_MAJOR=65
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
    AC_PREPROC_IFELSE([AC_LANG_SOURCE([[unicode/rbbi.h]])],[AC_MSG_RESULT([yes])],[AC_MSG_ERROR([icu headers not found])])
    AC_LANG_POP([C++])

    if test "$cross_compiling" != "yes"; then
        PKG_CHECK_MODULES(ICU, icu-i18n >= 4.6)
        ICU_VERSION=`$PKG_CONFIG --modversion icu-i18n 2>/dev/null`
        ICU_MAJOR=`echo $ICU_VERSION | cut -d"." -f1`
        ICU_MINOR=`echo $ICU_VERSION | cut -d"." -f2`
    fi

    if test "$cross_compiling" = "yes" -a \( "$with_system_icu_for_build" = "yes" -o "$with_system_icu_for_build" = "force" \); then
        ICU_VERSION_FOR_BUILD=`$PKG_CONFIG --modversion icu-i18n 2>/dev/null`
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
if test "$ICU_MAJOR" -ge "59"; then
    # As of ICU 59 it defaults to typedef char16_t UChar; which is available
    # with -std=c++11 but not all external libraries can be built with that,
    # for those use a bit-compatible typedef uint16_t UChar; see
    # icu/source/common/unicode/umachine.h
    ICU_UCHAR_TYPE="-DUCHAR_TYPE=uint16_t"
else
    ICU_UCHAR_TYPE=""
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
AC_SUBST(ICU_UCHAR_TYPE)

dnl ==================================================================
dnl Breakpad
dnl ==================================================================
DEFAULT_CRASHDUMP_VALUE="true"
AC_MSG_CHECKING([whether to enable breakpad])
if test "$enable_breakpad" != yes; then
    AC_MSG_RESULT([no])
else
    AC_MSG_RESULT([yes])
    ENABLE_BREAKPAD="TRUE"
    AC_DEFINE(ENABLE_BREAKPAD)
    AC_DEFINE(HAVE_FEATURE_BREAKPAD, 1)
    BUILD_TYPE="$BUILD_TYPE BREAKPAD"

    AC_MSG_CHECKING([for disable crash dump])
    if test "$enable_crashdump" = no; then
        DEFAULT_CRASHDUMP_VALUE="false"
        AC_MSG_RESULT([yes])
    else
       AC_MSG_RESULT([no])
    fi

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
AC_SUBST(DEFAULT_CRASHDUMP_VALUE)

dnl ==================================================================
dnl libfuzzer
dnl ==================================================================
AC_MSG_CHECKING([whether to enable fuzzers])
if test "$enable_fuzzers" != yes; then
    AC_MSG_RESULT([no])
else
    AC_MSG_RESULT([yes])
    ENABLE_FUZZERS="TRUE"
    AC_DEFINE([ENABLE_FUZZERS],1)
    BUILD_TYPE="$BUILD_TYPE FUZZERS"
fi
AC_SUBST(ENABLE_FUZZERS)

dnl ===================================================================
dnl Orcus
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([orcus],[ORCUS],[liborcus-0.15 >= 0.15.0])
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

dnl ===================================================================
dnl HarfBuzz
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([graphite],[GRAPHITE],[graphite2 >= 0.9.3],
                         ["-I${WORKDIR}/UnpackedTarball/graphite/include -DGRAPHITE2_STATIC"],
                         ["-L${WORKDIR}/LinkTarget/StaticLibrary -lgraphite"])

libo_CHECK_SYSTEM_MODULE([harfbuzz],[HARFBUZZ],[harfbuzz-icu >= 0.9.42],
                         ["-I${WORKDIR}/UnpackedTarball/harfbuzz/src"],
                         ["-L${WORKDIR}/UnpackedTarball/harfbuzz/src/.libs -lharfbuzz"])

if test "$COM" = "MSC"; then # override the above
    GRAPHITE_LIBS="${WORKDIR}/LinkTarget/StaticLibrary/graphite.lib"
    HARFBUZZ_LIBS="${WORKDIR}/UnpackedTarball/harfbuzz/src/.libs/libharfbuzz.lib"
fi

if test "$with_system_harfbuzz" = "yes"; then
    if test "$with_system_graphite" = "no"; then
        AC_MSG_ERROR([--with-system-graphite must be used when --with-system-harfbuzz is used])
    fi
    AC_MSG_CHECKING([whether system Harfbuzz is built with Graphite support])
    _save_libs="$LIBS"
    _save_cflags="$CFLAGS"
    LIBS="$LIBS $HARFBUZZ_LIBS"
    CFLAGS="$CFLAGS $HARFBUZZ_CFLAGS"
    AC_CHECK_FUNC(hb_graphite2_face_get_gr_face,,[AC_MSG_ERROR([Harfbuzz needs to be built with Graphite support.])])
    LIBS="$_save_libs"
    CFLAGS="$_save_cflags"
else
    if test "$with_system_graphite" = "yes"; then
        AC_MSG_ERROR([--without-system-graphite must be used when --without-system-harfbuzz is used])
    fi
fi

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
    if test -z "$WITH_WEBDAV"; then
        WITH_WEBDAV="serf"
    fi
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
    libo_CHECK_SYSTEM_MODULE([serf],[SERF],[serf-1 >= 1.3.9])
    ;;
neon)
    AC_MSG_RESULT([neon])
    # Check for system neon
    libo_CHECK_SYSTEM_MODULE([neon],[NEON],[neon >= 0.26.0])
    if test "$with_system_neon" = "yes"; then
        NEON_VERSION="`$PKG_CONFIG --modversion neon | $SED 's/\.//g'`"
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
        # Default cves off for Windows with its wild and wonderful
        # variety of AV software kicking in and panicking
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
if test "$enable_chart_tests" = "yes" -o '(' "$_os" = "WINNT" -a "$enable_chart_tests" != "no" ')'; then
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

    # warn that although OpenSSL is disabled, system libraries may depend on it
    AC_MSG_WARN([OpenSSL has been disabled. No code compiled here will make use of it but system libraries may create indirect dependencies])
    add_warning "OpenSSL has been disabled. No code compiled here will make use of it but system libraries may create indirect dependencies"
fi

AC_SUBST([DISABLE_OPENSSL])

if test "$enable_cipher_openssl_backend" = yes && test "$DISABLE_OPENSSL" = TRUE; then
    if test "$libo_fuzzed_enable_cipher_openssl_backend" = yes; then
        AC_MSG_NOTICE([Resetting --enable-cipher-openssl-backend=no])
        enable_cipher_openssl_backend=no
    else
        AC_MSG_ERROR([--enable-cipher-openssl-backend needs OpenSSL, but --disable-openssl was given.])
    fi
fi
AC_MSG_CHECKING([whether to enable the OpenSSL backend for rtl/cipher.h])
ENABLE_CIPHER_OPENSSL_BACKEND=
if test "$enable_cipher_openssl_backend" = yes; then
    AC_MSG_RESULT([yes])
    ENABLE_CIPHER_OPENSSL_BACKEND=TRUE
else
    AC_MSG_RESULT([no])
fi
AC_SUBST([ENABLE_CIPHER_OPENSSL_BACKEND])

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
    AC_CHECK_LIB([rdf], [librdf_world_set_raptor_init_handler], [:],
            [AC_MSG_ERROR(librdf too old. Need >= 1.0.16)], [])
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
        HUNSPELL_LIBS="-L${WORKDIR}/UnpackedTarball/hunspell/src/hunspell/.libs -lhunspell-1.7"
    fi
    BUILD_TYPE="$BUILD_TYPE HUNSPELL"
fi
AC_SUBST(SYSTEM_HUNSPELL)
AC_SUBST(HUNSPELL_CFLAGS)
AC_SUBST(HUNSPELL_LIBS)

dnl ===================================================================
dnl Check for system qrcodegen
dnl ===================================================================
AC_MSG_CHECKING([whether to use libqrcodegen])
if test "$enable_qrcodegen" = "no"; then
    AC_MSG_RESULT([no])
    ENABLE_QRCODEGEN=
    SYSTEM_QRCODEGEN=
else
    AC_MSG_RESULT([yes])
    ENABLE_QRCODEGEN=TRUE
    AC_MSG_CHECKING([which libqrcodegen to use])
    if test "$with_system_qrcodegen" = "yes"; then
        AC_MSG_RESULT([external])
        SYSTEM_QRCODEGEN=TRUE
        AC_LANG_PUSH([C++])
        AC_CHECK_HEADER(qrcodegen/QrCode.hpp, [],
            [AC_MSG_ERROR(qrcodegen headers not found.)], [#include <stdexcept>])
        AC_CHECK_LIB([qrcodegencpp], [main], [:],
            [ AC_MSG_ERROR(qrcodegen C++ library not found.) ], [])
        QRCODEGEN_LIBS=-lqrcodegencpp
        AC_LANG_POP([C++])
        QRCODEGEN_CFLAGS=$(printf '%s' "$QRCODEGEN_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
        FilterLibs "${QRCODEGEN_LIBS}"
        QRCODEGEN_LIBS="${filteredlibs}"
    else
        AC_MSG_RESULT([internal])
        SYSTEM_QRCODEGEN=
        BUILD_TYPE="$BUILD_TYPE QRCODEGEN"
    fi
    if test "$ENABLE_QRCODEGEN" = TRUE; then
        AC_DEFINE(ENABLE_QRCODEGEN)
    fi
fi
AC_SUBST(SYSTEM_QRCODEGEN)
AC_SUBST(ENABLE_QRCODEGEN)
AC_SUBST(QRCODEGEN_CFLAGS)
AC_SUBST(QRCODEGEN_LIBS)

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
        PKG_CHECK_MODULES(COINMP, coinmp coinutils)
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
        # some systems need this. Like Ubuntu...
        AC_CHECK_LIB(m, floor)
        AC_CHECK_LIB(dl, dlopen)
        AC_CHECK_LIB([lpsolve55], [make_lp], [:],
            [ AC_MSG_ERROR(lpsolve library not found or too old.)], [])
        LIBS=$save_LIBS
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

dnl ===================================================================
dnl Checking for libnumbertext
dnl ===================================================================
AC_MSG_CHECKING([whether to use libnumbertext])
if test "$enable_libnumbertext" = "no"; then
    AC_MSG_RESULT([no])
    ENABLE_LIBNUMBERTEXT=
    SYSTEM_LIBNUMBERTEXT=
else
    AC_MSG_RESULT([yes])
    ENABLE_LIBNUMBERTEXT=TRUE
    libo_CHECK_SYSTEM_MODULE([libnumbertext],[LIBNUMBERTEXT],[libnumbertext >= 1.0.0])
    if test "$with_system_libnumbertext" = "yes"; then
        SYSTEM_LIBNUMBERTEXT_DATA=file://`$PKG_CONFIG --variable=pkgdatadir libnumbertext`
        SYSTEM_LIBNUMBERTEXT=YES
    else
        SYSTEM_LIBNUMBERTEXT=
        AC_LANG_PUSH([C++])
        save_CPPFLAGS=$CPPFLAGS
        CPPFLAGS="$CPPFLAGS $CXXFLAGS_CXX11"
        AC_CHECK_HEADERS([codecvt regex])
        AS_IF([test "x$ac_cv_header_codecvt" != xyes -o "x$ac_cv_header_regex" != xyes],
                [ ENABLE_LIBNUMBERTEXT=''
                  LIBNUMBERTEXT_CFLAGS=''
                  AC_MSG_WARN([No system-provided libnumbertext or codecvt/regex C++11 headers (min. libstdc++ 4.9).
                               Enable libnumbertext fallback (missing number to number name conversion).])
                ])
        CPPFLAGS=$save_CPPFLAGS
        AC_LANG_POP([C++])
    fi
    if test "$ENABLE_LIBNUMBERTEXT" = TRUE; then
        AC_DEFINE(ENABLE_LIBNUMBERTEXT)
    fi
fi
AC_SUBST(SYSTEM_LIBNUMBERTEXT)
AC_SUBST(SYSTEM_LIBNUMBERTEXT_DATA)
AC_SUBST(ENABLE_LIBNUMBERTEXT)
AC_SUBST(LIBNUMBERTEXT_CFLAGS)

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
dnl Check for uuidgen
dnl =========================================
if test "$_os" = "WINNT" -a "$cross_compiling" != "yes"; then
    # presence is already tested above in the WINDOWS_SDK_HOME check
    UUIDGEN=uuidgen.exe
    AC_SUBST(UUIDGEN)
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
    if test $(echo $_flex_version | $AWK -F. '{printf("%d%03d%03d", $1, $2, $3)}') -lt 2006000; then
        AC_MSG_ERROR([failed ($FLEX $_flex_version found, but need at least 2.6.0)])
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

dnl On Solaris, FreeBSD or macOS, check if --with-gnu-patch was used
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
if test "$_os" = "WINNT"; then
    if test "$BITNESS_OVERRIDE" = ""; then
        assembler=ml.exe
    else
        assembler=ml64.exe
    fi

    AC_MSG_CHECKING([for the MSVC assembler ($assembler)])
    if test -f "$CL_PATH/$assembler"; then
        ML_EXE=`win_short_path_for_make "$CL_PATH/$assembler"`
        AC_MSG_RESULT([$ML_EXE])
    else
        AC_MSG_ERROR([not found])
    fi
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
dnl Check for system epoxy
dnl ===================================================================
libo_CHECK_SYSTEM_MODULE([epoxy], [EPOXY], [epoxy >= 1.2], ["-I${WORKDIR}/UnpackedTarball/epoxy/include"])

dnl ===================================================================
dnl Set vcl option: coordinate device in double or sal_Int32
dnl ===================================================================

dnl disabled for now, we don't want subtle differences between OSs
dnl AC_MSG_CHECKING([Type to use for Device Pixel coordinates])
dnl if test "$_os" = "Darwin" -o  $_os = iOS ; then
dnl     AC_DEFINE(VCL_FLOAT_DEVICE_PIXEL)
dnl     AC_MSG_RESULT([double])
dnl else
dnl     AC_MSG_RESULT([sal_Int32])
dnl fi

dnl ===================================================================
dnl Test which vclplugs have to be built.
dnl ===================================================================
R=""
if test "$USING_X11" != TRUE; then
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
    PKG_CHECK_MODULES(GTK3, gtk+-3.0 >= 3.18 gtk+-unix-print-3.0 gmodule-no-export-2.0 glib-2.0 >= 2.38 cairo, ENABLE_GTK3="TRUE", ENABLE_GTK3="")
    if test "x$ENABLE_GTK3" = "xTRUE"; then
        AC_DEFINE(ENABLE_GTK3)
        R="gtk3"
    else
        AC_MSG_ERROR([gtk3 or dependent libraries of the correct versions, not found])
    fi
    GTK3_CFLAGS=$(printf '%s' "$GTK3_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
    FilterLibs "${GTK3_LIBS}"
    GTK3_LIBS="${filteredlibs}"

    dnl We require egl only for the gtk3 plugin. Otherwise we use glx.
    if test "$with_system_epoxy" != "yes"; then
        AC_CHECK_LIB(EGL, eglMakeCurrent, [:], AC_MSG_ERROR([libEGL required.]))
        AC_CHECK_HEADER(EGL/eglplatform.h, [],
                        [AC_MSG_ERROR(EGL headers not found. install mesa-libEGL-devel)], [])
    fi
fi
AC_SUBST(GTK3_LIBS)
AC_SUBST(GTK3_CFLAGS)
AC_SUBST(ENABLE_GTK3)

if test "$enable_introspection" = yes; then
    if test "$ENABLE_GTK3" = TRUE; then
        GOBJECT_INTROSPECTION_REQUIRE(INTROSPECTION_REQUIRED_VERSION)
    else
        AC_MSG_ERROR([--enable-introspection requires --enable-gtk3])
    fi
fi

ENABLE_QT5=""
if test "x$enable_qt5" = "xyes"; then
    ENABLE_QT5="TRUE"
    AC_DEFINE(ENABLE_QT5)
    R="$R qt5"
fi
AC_SUBST(ENABLE_QT5)

ENABLE_KF5=""
if test "x$enable_kf5" = "xyes"; then
    ENABLE_KF5="TRUE"
    AC_DEFINE(ENABLE_KF5)
    R="$R kf5"
fi
AC_SUBST(ENABLE_KF5)

ENABLE_GTK3_KDE5=""
if test "x$enable_gtk3_kde5" = "xyes"; then
    ENABLE_GTK3_KDE5="TRUE"
    AC_DEFINE(ENABLE_GTK3_KDE5)
    R="$R gtk3_kde5"
fi
AC_SUBST(ENABLE_GTK3_KDE5)

if test "$_os" = "WINNT"; then
    R="$R win"
elif test "$_os" = "Darwin"; then
    R="$R osx"
elif test "$_os" = "iOS"; then
    R="ios (builtin)"
fi

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
DBUS_GLIB_CFLAGS=""
DBUS_GLIB_LIBS=""
DBUS_HAVE_GLIB=""

if test "$enable_dbus" = "no"; then
    test_dbus=no
fi

AC_MSG_CHECKING([whether to enable DBUS support])
if test "$test_dbus" = "yes"; then
    ENABLE_DBUS="TRUE"
    AC_MSG_RESULT([yes])
    PKG_CHECK_MODULES(DBUS, dbus-1 >= 0.60)
    AC_DEFINE(ENABLE_DBUS)
    DBUS_CFLAGS=$(printf '%s' "$DBUS_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
    FilterLibs "${DBUS_LIBS}"
    DBUS_LIBS="${filteredlibs}"

    # Glib is needed for BluetoothServer
    # Sets also DBUS_GLIB_CFLAGS/DBUS_GLIB_LIBS if successful.
    PKG_CHECK_MODULES(DBUS_GLIB,[glib-2.0 >= 2.4],
        [
            DBUS_HAVE_GLIB="TRUE"
            AC_DEFINE(DBUS_HAVE_GLIB,1)
        ],
        AC_MSG_WARN([[No Glib found, Bluetooth support will be disabled]])
    )
else
    AC_MSG_RESULT([no])
fi

AC_SUBST(ENABLE_DBUS)
AC_SUBST(DBUS_CFLAGS)
AC_SUBST(DBUS_LIBS)
AC_SUBST(DBUS_GLIB_CFLAGS)
AC_SUBST(DBUS_GLIB_LIBS)
AC_SUBST(DBUS_HAVE_GLIB)

AC_MSG_CHECKING([whether to enable Impress remote control])
if test -n "$enable_sdremote" -a "$enable_sdremote" != "no"; then
    AC_MSG_RESULT([yes])
    ENABLE_SDREMOTE=TRUE
    AC_MSG_CHECKING([whether to enable Bluetooth support in Impress remote control])

    if test $OS = MACOSX && test "$MAC_OS_X_VERSION_MAX_ALLOWED" -ge 101500; then
        # The Bluetooth code doesn't compile with macOS SDK 10.15
        enable_sdremote_bluetooth=no
    fi
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
            if test "$ENABLE_DBUS" = "TRUE" -a "$DBUS_HAVE_GLIB" = "TRUE"; then
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
dnl Check whether to enable GIO support
dnl ===================================================================
if test "$ENABLE_GTK3" = "TRUE"; then
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
AC_SUBST(ENABLE_GIO)
AC_SUBST(GIO_CFLAGS)
AC_SUBST(GIO_LIBS)


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
dnl ===================================================================

ENABLE_GSTREAMER_1_0=""

if test "$build_gstreamer_1_0" = "yes"; then

    AC_MSG_CHECKING([whether to enable the GStreamer 1.0 avmedia backend])
    if test "$enable_avmedia" = yes -a "$enable_gstreamer_1_0" != no; then
        ENABLE_GSTREAMER_1_0="TRUE"
        AC_MSG_RESULT([yes])
        PKG_CHECK_MODULES( [GSTREAMER_1_0], [gstreamer-1.0 gstreamer-plugins-base-1.0 gstreamer-pbutils-1.0 gstreamer-video-1.0] )
        GSTREAMER_1_0_CFLAGS=$(printf '%s' "$GSTREAMER_1_0_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
        FilterLibs "${GSTREAMER_1_0_LIBS}"
        GSTREAMER_1_0_LIBS="${filteredlibs}"
        AC_DEFINE(ENABLE_GSTREAMER_1_0)
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_SUBST(GSTREAMER_1_0_CFLAGS)
AC_SUBST(GSTREAMER_1_0_LIBS)
AC_SUBST(ENABLE_GSTREAMER_1_0)

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

ENABLE_OPENGL_TRANSITIONS=
ENABLE_OPENGL_CANVAS=
if test $_os = iOS -o $_os = Android -o "$ENABLE_FUZZERS" = "TRUE"; then
   : # disable
elif test "$_os" = "Darwin"; then
    # We use frameworks on macOS, no need for detail checks
    ENABLE_OPENGL_TRANSITIONS=TRUE
    AC_DEFINE(HAVE_FEATURE_OPENGL,1)
    ENABLE_OPENGL_CANVAS=TRUE
elif test $_os = WINNT; then
    ENABLE_OPENGL_TRANSITIONS=TRUE
    AC_DEFINE(HAVE_FEATURE_OPENGL,1)
    ENABLE_OPENGL_CANVAS=TRUE
else
    if test "$USING_X11" = TRUE; then
        AC_CHECK_LIB(GL, glBegin, [:], AC_MSG_ERROR([libGL required.]))
        ENABLE_OPENGL_TRANSITIONS=TRUE
        AC_DEFINE(HAVE_FEATURE_OPENGL,1)
        ENABLE_OPENGL_CANVAS=TRUE
    fi
fi

AC_SUBST(ENABLE_OPENGL_TRANSITIONS)
AC_SUBST(ENABLE_OPENGL_CANVAS)

dnl =================================================
dnl Check whether to build with OpenCL support.
dnl =================================================

if test $_os != iOS -a $_os != Android -a "$ENABLE_FUZZERS" != "TRUE"; then
    # OPENCL in BUILD_TYPE and HAVE_FEATURE_OPENCL tell that OpenCL is potentially available on the
    # platform (optional at run-time, used through clew).
    BUILD_TYPE="$BUILD_TYPE OPENCL"
    AC_DEFINE(HAVE_FEATURE_OPENCL)
fi

dnl =================================================
dnl Check whether to build with dconf support.
dnl =================================================

if test $_os != Android -a $_os != iOS -a "$enable_dconf" != no; then
    PKG_CHECK_MODULES([DCONF], [dconf >= 0.15.2], [], [
        if test "$enable_dconf" = yes; then
            AC_MSG_ERROR([dconf not found])
        else
            enable_dconf=no
        fi])
fi
AC_MSG_CHECKING([whether to enable dconf])
if test $_os = Android -o $_os = iOS -o "$enable_dconf" = no; then
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
if test \( -z "$enable_pdfimport" -o "$enable_pdfimport" = yes \); then
    AC_MSG_RESULT([yes])
    ENABLE_PDFIMPORT=TRUE
    AC_DEFINE(HAVE_FEATURE_PDFIMPORT)
else
    AC_MSG_RESULT([no])
fi

# Pdfium?
AC_MSG_CHECKING([whether to build PDFium])
ENABLE_PDFIUM=
if test \( -z "$enable_pdfium" -a "$ENABLE_PDFIMPORT" = "TRUE" \) -o "$enable_pdfium" = yes; then
    AC_MSG_RESULT([yes])
    ENABLE_PDFIUM=TRUE
    AC_DEFINE(HAVE_FEATURE_PDFIUM)
    BUILD_TYPE="$BUILD_TYPE PDFIUM"
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_PDFIUM)

dnl ===================================================================
dnl Check for poppler
dnl ===================================================================
ENABLE_POPPLER=
AC_MSG_CHECKING([whether to build Poppler])
if test \( -z "$enable_poppler" -a "$ENABLE_PDFIMPORT" = "TRUE" \) -o "$enable_poppler" = yes; then
    AC_MSG_RESULT([yes])
    ENABLE_POPPLER=TRUE
    AC_DEFINE(HAVE_FEATURE_POPPLER)
else
    AC_MSG_RESULT([no])
fi
AC_SUBST(ENABLE_POPPLER)

if test "$ENABLE_PDFIMPORT" = "TRUE" -a "$ENABLE_POPPLER" != "TRUE" -a "$ENABLE_PDFIUM" != "TRUE"; then
    AC_MSG_ERROR([Cannot import PDF without either Pdfium or Poppler; please enable either of them.])
fi

if test "$ENABLE_PDFIMPORT" != "TRUE" -a \( "$ENABLE_POPPLER" = "TRUE" -o "$ENABLE_PDFIUM" = "TRUE" \); then
    AC_MSG_ERROR([Cannot enable Pdfium or Poppler when PDF importing is disabled; please enable PDF import first.])
fi

if test "$ENABLE_PDFIMPORT" = "TRUE" -a "$ENABLE_POPPLER" = "TRUE"; then
    dnl ===================================================================
    dnl Check for system poppler
    dnl ===================================================================
    AC_MSG_CHECKING([which PDF import poppler to use])
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
fi
AC_SUBST(ENABLE_PDFIMPORT)
AC_SUBST(SYSTEM_POPPLER)
AC_SUBST(POPPLER_CFLAGS)
AC_SUBST(POPPLER_LIBS)

SYSTEM_GPGMEPP=

if test "$build_for_ios" = "YES"; then
    AC_MSG_CHECKING([whether gpgmepp should be disabled due to iOS])
    AC_MSG_RESULT([yes])
elif test "$enable_mpl_subset" = "yes"; then
    AC_MSG_CHECKING([whether gpgmepp should be disabled due to building just MPL])
    AC_MSG_RESULT([yes])
elif test "$enable_fuzzers" = "yes"; then
    AC_MSG_CHECKING([whether gpgmepp should be disabled due to oss-fuzz])
    AC_MSG_RESULT([yes])
elif test "$_os" = "Linux" -o "$_os" = "Darwin" -o "$_os" = "WINNT" ; then
    dnl ===================================================================
    dnl Check for system gpgme
    dnl ===================================================================
    AC_MSG_CHECKING([which gpgmepp to use])
    if test "$with_system_gpgmepp" = "yes"; then
        AC_MSG_RESULT([external])
        SYSTEM_GPGMEPP=TRUE

        # C++ library doesn't come with fancy gpgmepp-config, check for headers the old-fashioned way
        AC_CHECK_HEADER(gpgme++/gpgmepp_version.h, [ GPGMEPP_CFLAGS=-I/usr/include/gpgme++ ],
            [AC_MSG_ERROR([gpgmepp headers not found, install gpgmepp development package])], [])
        # progress_callback is the only func with plain C linkage
        # checking for it also filters out older, KDE-dependent libgpgmepp versions
        AC_CHECK_LIB(gpgmepp, progress_callback, [ GPGMEPP_LIBS=-lgpgmepp ],
            [AC_MSG_ERROR(gpgmepp not found or not functional)], [])
        AC_CHECK_HEADER(gpgme.h, [],
            [AC_MSG_ERROR([gpgme headers not found, install gpgme development package])], [])
    else
        AC_MSG_RESULT([internal])
        BUILD_TYPE="$BUILD_TYPE LIBGPGERROR LIBASSUAN GPGMEPP"
        AC_DEFINE([GPGME_CAN_EXPORT_MINIMAL_KEY])

        GPG_ERROR_CFLAGS="-I${WORKDIR}/UnpackedTarball/libgpg-error/src"
        LIBASSUAN_CFLAGS="-I${WORKDIR}/UnpackedTarball/libassuan/src"
        if test "$_os" != "WINNT"; then
            GPG_ERROR_LIBS="-L${WORKDIR}/UnpackedTarball/libgpg-error/src/.libs -lgpg-error"
            LIBASSUAN_LIBS="-L${WORKDIR}/UnpackedTarball/libassuan/src/.libs -lassuan"
        elif test "$host_cpu" = "i686" -a "$WINDOWS_SDK_ARCH" = "x64"; then
            AC_MSG_ERROR(gpgme cannot be built on cygwin32 for Win64.)
        fi
    fi
    ENABLE_GPGMEPP=TRUE
    AC_DEFINE([HAVE_FEATURE_GPGME])
    AC_PATH_PROG(GPG, gpg)
    # TODO: Windows's cygwin gpg does not seem to work with our gpgme,
    # so let's exclude that manually for the moment
    if test -n "$GPG" -a "$_os" != "WINNT"; then
        # make sure we not only have a working gpgme, but a full working
        # gpg installation to run OpenPGP signature verification
        AC_DEFINE([HAVE_FEATURE_GPGVERIFY])
    fi
    if test "$_os" = "Linux"; then
      uid=`id -u`
      AC_MSG_CHECKING([for /run/user/$uid])
      if test -d /run/user/$uid; then
        AC_MSG_RESULT([yes])
        AC_PATH_PROG(GPGCONF, gpgconf)

        # Older versions of gpgconf are not working as expected, since
        # `gpgconf --remove-socketdir` fails to exit any gpg-agent daemon operating
        # on that socket dir that has (indirectly) been started by the tests in xmlsecurity/qa/unit/signing/signing.cxx
        # (see commit message of f0305ec0a7d199e605511844d9d6af98b66d4bfd%5E )
        AC_MSG_CHECKING([whether version of gpgconf is suitable ... ])
        GPGCONF_VERSION=`"$GPGCONF" --version | "$AWK" '/^gpgconf \(GnuPG\)/{print $3}'`
        GPGCONF_NUMVER=`echo $GPGCONF_VERSION | $AWK -F. '{ print \$1*10000+\$2*100+\$3 }'`
        if test "$GPGCONF_VERSION" = "2.2_OOo" -o "$GPGCONF_NUMVER" -ge "020200"; then
          AC_MSG_RESULT([yes, $GPGCONF_VERSION])
          AC_MSG_CHECKING([for gpgconf --create-socketdir... ])
          if $GPGCONF --dump-options > /dev/null ; then
            if $GPGCONF --dump-options | grep -q create-socketdir ; then
              AC_MSG_RESULT([yes])
              AC_DEFINE([HAVE_GPGCONF_SOCKETDIR])
              AC_DEFINE_UNQUOTED([GPGME_GPGCONF], ["$GPGCONF"])
            else
              AC_MSG_RESULT([no])
            fi
          else
            AC_MSG_RESULT([no. missing or broken gpgconf?])
          fi
        else
          AC_MSG_RESULT([no, $GPGCONF_VERSION])
        fi
      else
        AC_MSG_RESULT([no])
     fi
   fi
fi
AC_SUBST(ENABLE_GPGMEPP)
AC_SUBST(SYSTEM_GPGMEPP)
AC_SUBST(GPG_ERROR_CFLAGS)
AC_SUBST(GPG_ERROR_LIBS)
AC_SUBST(LIBASSUAN_CFLAGS)
AC_SUBST(LIBASSUAN_LIBS)
AC_SUBST(GPGMEPP_CFLAGS)
AC_SUBST(GPGMEPP_LIBS)

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
        NEED_ANT=TRUE
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
        BUILD_TYPE="$BUILD_TYPE APACHE_COMMONS"
        NEED_ANT=TRUE
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
        NEED_ANT=TRUE
    fi
else
    AC_MSG_RESULT([no])
    ENABLE_SCRIPTING_JAVASCRIPT=
    SCPDEFS="$SCPDEFS -DWITHOUT_SCRIPTING_JAVASCRIPT"
fi
AC_SUBST(ENABLE_SCRIPTING_JAVASCRIPT)
AC_SUBST(SYSTEM_RHINO)
AC_SUBST(RHINO_JAR)

# This is only used in Qt5/KF5 checks to determine if /usr/lib64
# paths should be added to library search path. So lets put all 64-bit
# platforms there.
supports_multilib=
case "$host_cpu" in
x86_64 | powerpc64 | powerpc64le | s390x | aarch64 | mips64 | mips64el)
    if test "$SAL_TYPES_SIZEOFLONG" = "8"; then
        supports_multilib="yes"
    fi
    ;;
*)
    ;;
esac

dnl ===================================================================
dnl QT5 Integration
dnl ===================================================================

QT5_CFLAGS=""
QT5_LIBS=""
QMAKE5="qmake"
MOC5="moc"
QT5_GOBJECT_CFLAGS=""
QT5_GOBJECT_LIBS=""
QT5_HAVE_GOBJECT=""
if test \( "$test_kf5" = "yes" -a "$ENABLE_KF5" = "TRUE" \) -o \
        \( "$test_qt5" = "yes" -a "$ENABLE_QT5" = "TRUE" \) -o \
        \( "$test_gtk3_kde5" = "yes" -a "$ENABLE_GTK3_KDE5" = "TRUE" \)
then
    qt5_incdirs="$QT5INC /usr/include/qt5 /usr/include $x_includes"
    qt5_libdirs="$QT5LIB /usr/lib/qt5 /usr/lib $x_libraries"

    if test -n "$supports_multilib"; then
        qt5_libdirs="$qt5_libdirs /usr/lib64/qt5 /usr/lib64/qt /usr/lib64"
    fi

    qt5_test_include="QtWidgets/qapplication.h"
    qt5_test_library="libQt5Widgets.so"

    dnl Check for qmake5
    AC_PATH_PROGS( QMAKE5, [qmake-qt5 qmake], no, [$QT5DIR/bin:$PATH])
    if test "$QMAKE5" = "no"; then
        AC_MSG_ERROR([Qmake not found.  Please specify the root of your Qt5 installation by exporting QT5DIR before running "configure".])
    else
        qmake5_test_ver="`$QMAKE5 -v 2>&1 | $SED -n -e 's/^Using Qt version \(5\.[[0-9.]]\+\).*$/\1/p'`"
        if test -z "$qmake5_test_ver"; then
            AC_MSG_ERROR([Wrong qmake for Qt5 found. Please specify the root of your Qt5 installation by exporting QT5DIR before running "configure".])
        fi
        qmake5_minor_version="`echo $qmake5_test_ver | cut -d. -f2`"
        qt5_minimal_minor="6"
        if test "$qmake5_minor_version" -lt "$qt5_minimal_minor"; then
            AC_MSG_ERROR([The minimal supported Qt5 version is 5.${qt5_minimal_minor}, but your 'qmake -v' reports Qt5 version $qmake5_test_ver.])
        else
            AC_MSG_NOTICE([Detected Qt5 version: $qmake5_test_ver])
        fi
    fi

    qt5_incdirs="`$QMAKE5 -query QT_INSTALL_HEADERS` $qt5_incdirs"
    qt5_libdirs="`$QMAKE5 -query QT_INSTALL_LIBS` $qt5_libdirs"

    AC_MSG_CHECKING([for Qt5 headers])
    qt5_incdir="no"
    for inc_dir in $qt5_incdirs; do
        if test -r "$inc_dir/$qt5_test_include"; then
            qt5_incdir="$inc_dir"
            break
        fi
    done
    AC_MSG_RESULT([$qt5_incdir])
    if test "x$qt5_incdir" = "xno"; then
        AC_MSG_ERROR([Qt5 headers not found.  Please specify the root of your Qt5 installation by exporting QT5DIR before running "configure".])
    fi

    AC_MSG_CHECKING([for Qt5 libraries])
    qt5_libdir="no"
    for lib_dir in $qt5_libdirs; do
        if test -r "$lib_dir/$qt5_test_library"; then
            qt5_libdir="$lib_dir"
            break
        fi
    done
    AC_MSG_RESULT([$qt5_libdir])
    if test "x$qt5_libdir" = "xno"; then
        AC_MSG_ERROR([Qt5 libraries not found.  Please specify the root of your Qt5 installation by exporting QT5DIR before running "configure".])
    fi

    QT5_CFLAGS="-I$qt5_incdir -DQT_CLEAN_NAMESPACE -DQT_THREAD_SUPPORT -DQT_NO_VERSION_TAGGING"
    QT5_CFLAGS=$(printf '%s' "$QT5_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")
    QT5_LIBS="-L$qt5_libdir -lQt5Core -lQt5Gui -lQt5Widgets -lQt5Network"

    if test "$USING_X11" = TRUE; then
        PKG_CHECK_MODULES(QT5_XCB,[xcb],,[AC_MSG_ERROR([XCB not found, which is needed for correct app grouping in X11.])])
        PKG_CHECK_MODULES(QT5_XCB_ICCCM,[xcb-icccm],[
            QT5_HAVE_XCB_ICCCM=1
            AC_DEFINE(QT5_HAVE_XCB_ICCCM)
        ],[
            AC_MSG_WARN([XCB ICCCM not found, which is needed for old Qt versions (< 5.12) on some WMs to correctly group dialogs (like QTBUG-46626)])
            add_warning "XCB ICCCM not found, which is needed for Qt versions (< 5.12) on some WMs to correctly group dialogs (like QTBUG-46626)"
        ])
        QT5_CFLAGS="$QT5_CFLAGS $QT5_XCB_CFLAGS $QT5_XCB_ICCCM_CFLAGS"
        QT5_LIBS="$QT5_LIBS $QT5_XCB_LIBS $QT5_XCB_ICCCM_LIBS -lQt5X11Extras"
        QT5_USING_X11=1
        AC_DEFINE(QT5_USING_X11)
    fi

    dnl Check for Meta Object Compiler

    AC_PATH_PROGS( MOC5, [moc-qt5 moc], no, [`dirname $qt5_libdir`/bin:$QT5DIR/bin:$PATH])
    if test "$MOC5" = "no"; then
        AC_MSG_ERROR([Qt Meta Object Compiler not found.  Please specify
the root of your Qt installation by exporting QT5DIR before running "configure".])
    fi

    if test "$build_gstreamer_1_0" = "yes"; then
        PKG_CHECK_MODULES(QT5_GOBJECT,[gobject-2.0], [
                QT5_HAVE_GOBJECT=1
                AC_DEFINE(QT5_HAVE_GOBJECT)
            ],
            AC_MSG_WARN([[No GObject found, can't use QWidget GStreamer sink on wayland!]])
        )
    fi
fi
AC_SUBST(QT5_CFLAGS)
AC_SUBST(QT5_LIBS)
AC_SUBST(MOC5)
AC_SUBST(QT5_GOBJECT_CFLAGS)
AC_SUBST(QT5_GOBJECT_LIBS)
AC_SUBST(QT5_HAVE_GOBJECT)

dnl ===================================================================
dnl KF5 Integration
dnl ===================================================================

KF5_CFLAGS=""
KF5_LIBS=""
KF5_CONFIG="kf5-config"
if test \( "$test_kf5" = "yes" -a "$ENABLE_KF5" = "TRUE" \) -o \
        \( "$test_gtk3_kde5" = "yes" -a "$ENABLE_GTK3_KDE5" = "TRUE" \)
then
    if test "$OS" = "HAIKU"; then
        haiku_arch="`echo $RTL_ARCH | tr X x`"
        kf5_haiku_incdirs="`findpaths -c ' ' -a $haiku_arch B_FIND_PATH_HEADERS_DIRECTORY`"
        kf5_haiku_libdirs="`findpaths -c ' ' -a $haiku_arch B_FIND_PATH_DEVELOP_LIB_DIRECTORY`"
    fi

    kf5_incdirs="$KF5INC /usr/include $kf5_haiku_incdirs $x_includes"
    kf5_libdirs="$KF5LIB /usr/lib /usr/lib/kf5 /usr/lib/kf5/devel $kf5_haiku_libdirs $x_libraries"
    if test -n "$supports_multilib"; then
        kf5_libdirs="$kf5_libdirs /usr/lib64 /usr/lib64/kf5 /usr/lib64/kf5/devel"
    fi

    kf5_test_include="KF5/kcoreaddons_version.h"
    kf5_test_library="libKF5CoreAddons.so"
    kf5_libdirs="$qt5_libdir $kf5_libdirs"

    dnl kf5 KDE4 support compatibility installed
    AC_PATH_PROG( KF5_CONFIG, $KF5_CONFIG, no, )
    if test "$KF5_CONFIG" != "no"; then
        kf5_incdirs="`$KF5_CONFIG --path include` $kf5_incdirs"
        kf5_libdirs="`$KF5_CONFIG --path lib` $kf5_libdirs"
    fi

    dnl Check for KF5 headers
    AC_MSG_CHECKING([for KF5 headers])
    kf5_incdir="no"
    for kf5_check in $kf5_incdirs; do
        if test -r "$kf5_check/$kf5_test_include"; then
            kf5_incdir="$kf5_check/KF5"
            break
        fi
    done
    AC_MSG_RESULT([$kf5_incdir])
    if test "x$kf5_incdir" = "xno"; then
        AC_MSG_ERROR([KF5 headers not found.  Please specify the root of your KF5 installation by exporting KF5DIR before running "configure".])
    fi

    dnl Check for KF5 libraries
    AC_MSG_CHECKING([for KF5 libraries])
    kf5_libdir="no"
    for kf5_check in $kf5_libdirs; do
        if test -r "$kf5_check/$kf5_test_library"; then
            kf5_libdir="$kf5_check"
            break
        fi
    done

    AC_MSG_RESULT([$kf5_libdir])
    if test "x$kf5_libdir" = "xno"; then
        AC_MSG_ERROR([KF5 libraries not found.  Please specify the root of your KF5 installation by exporting KF5DIR before running "configure".])
    fi

    KF5_CFLAGS="-I$kf5_incdir -I$kf5_incdir/KCoreAddons -I$kf5_incdir/KI18n -I$kf5_incdir/KConfigCore -I$kf5_incdir/KWindowSystem -I$kf5_incdir/KIOCore -I$kf5_incdir/KIOWidgets -I$kf5_incdir/KIOFileWidgets -I$qt5_incdir -I$qt5_incdir/QtCore -I$qt5_incdir/QtGui -I$qt5_incdir/QtWidgets -I$qt5_incdir/QtNetwork -DQT_CLEAN_NAMESPACE -DQT_THREAD_SUPPORT -DQT_NO_VERSION_TAGGING"
    KF5_LIBS="-L$kf5_libdir -lKF5CoreAddons -lKF5I18n -lKF5ConfigCore -lKF5WindowSystem -lKF5KIOCore -lKF5KIOWidgets -lKF5KIOFileWidgets -L$qt5_libdir -lQt5Core -lQt5Gui -lQt5Widgets -lQt5Network"
    KF5_CFLAGS=$(printf '%s' "$KF5_CFLAGS" | sed -e "s/-I/${ISYSTEM?}/g")

    if test "$USING_X11" = TRUE; then
        KF5_LIBS="$KF5_LIBS -lQt5X11Extras"
    fi

    AC_LANG_PUSH([C++])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS $KF5_CFLAGS"
    AC_MSG_CHECKING([whether KDE is >= 5.0])
       AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <kcoreaddons_version.h>

int main(int argc, char **argv) {
       if (KCOREADDONS_VERSION_MAJOR == 5 && KCOREADDONS_VERSION_MINOR >= 0) return 0;
       else return 1;
}
       ]])],[AC_MSG_RESULT([yes])],[AC_MSG_ERROR([KDE version too old])],[])
    CXXFLAGS=$save_CXXFLAGS
    AC_LANG_POP([C++])
fi
AC_SUBST(KF5_CFLAGS)
AC_SUBST(KF5_LIBS)

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
dnl Test which themes to include
dnl ===================================================================
AC_MSG_CHECKING([which themes to include])
# if none given use default subset of available themes
if test "x$with_theme" = "x" -o "x$with_theme" = "xyes"; then
    with_theme="breeze breeze_dark breeze_dark_svg breeze_svg colibre colibre_svg elementary elementary_svg karasa_jaga karasa_jaga_svg sifr sifr_svg sifr_dark sifr_dark_svg tango"
fi

WITH_THEMES=""
if test "x$with_theme" != "xno"; then
    for theme in $with_theme; do
        case $theme in
        breeze|breeze_dark|breeze_dark_svg|breeze_svg|colibre|colibre_svg|elementary|elementary_svg|karasa_jaga|karasa_jaga_svg|sifr|sifr_svg|sifr_dark|sifr_dark_svg|tango) real_theme="$theme" ;;
        default) real_theme=colibre ;;
        *) AC_MSG_ERROR([Unknown value for --with-theme: $theme]) ;;
        esac
        WITH_THEMES=`echo "$WITH_THEMES $real_theme"|tr '\ ' '\n'|sort|uniq|tr '\n' '\ '`
    done
fi
AC_MSG_RESULT([$WITH_THEMES])
AC_SUBST([WITH_THEMES])
# FIXME: remove this, and the convenience default->colibre remapping after a grace period
for theme in $with_theme; do
    case $theme in
    default) AC_MSG_WARN([--with-theme=default is deprecated and will be removed, use --with-theme=colibre]) ;;
    *) ;;
    esac
done

dnl ===================================================================
dnl Test whether to integrate helppacks into the product's installer
dnl ===================================================================
AC_MSG_CHECKING([for helppack integration])
if test "$with_helppack_integration" = "no"; then
    AC_MSG_RESULT([no integration])
else
    SCPDEFS="$SCPDEFS -DWITH_HELPPACK_INTEGRATION"
    AC_MSG_RESULT([integration])
fi

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

libo_CHECK_EXTENSION([ConvertTextToNumber],[CT2N],[ct2n],[ct2n],[])
libo_CHECK_EXTENSION([Numbertext],[NUMBERTEXT],[numbertext],[numbertext],[b7cae45ad2c23551fd6ccb8ae2c1f59e-numbertext_0.9.5.oxt])
if test "x$with_java" != "xno"; then
    libo_CHECK_EXTENSION([NLPSolver],[NLPSOLVER],[nlpsolver],[nlpsolver],[])
    libo_CHECK_EXTENSION([LanguageTool],[LANGUAGETOOL],[languagetool],[languagetool],[])
fi

AC_MSG_CHECKING([whether to build opens___.ttf])
if test "$enable_build_opensymbol" = "yes"; then
    AC_MSG_RESULT([yes])
    AC_PATH_PROG(FONTFORGE, fontforge)
    if test -z "$FONTFORGE"; then
        AC_MSG_ERROR([fontforge not installed])
    fi
else
    AC_MSG_RESULT([no])
    OPENSYMBOL_TTF=884ed41809687c3e168fc7c19b16585149ff058eca79acbf3ee784f6630704cc-opens___.ttf
    BUILD_TYPE="$BUILD_TYPE OPENSYMBOL"
fi
AC_SUBST(OPENSYMBOL_TTF)
AC_SUBST(FONTFORGE)

dnl ===================================================================
dnl Test whether to include fonts
dnl ===================================================================
AC_MSG_CHECKING([whether to include third-party fonts])
if test "$with_fonts" != "no"; then
    AC_MSG_RESULT([yes])
    WITH_FONTS=TRUE
    BUILD_TYPE="$BUILD_TYPE MORE_FONTS"
    AC_DEFINE(HAVE_MORE_FONTS)
else
    AC_MSG_RESULT([no])
    WITH_FONTS=
    SCPDEFS="$SCPDEFS -DWITHOUT_FONTS"
fi
AC_SUBST(WITH_FONTS)

dnl Test whether to include more Google Noto fonts
dnl ===================================================================
AC_MSG_CHECKING([whether to include more Google Noto fonts])
if test "$enable_noto_font" = "" -o "$enable_noto_font" = "no" -o "$with_fonts" = "no"; then
    AC_MSG_RESULT([no])
    WITH_NOTO_FONT=
else
    AC_MSG_RESULT([yes])
    WITH_NOTO_FONT=TRUE
    BUILD_TYPE="$BUILD_TYPE NOTO_FONT"
    SCPDEFS="$SCPDEFS -DWITH_NOTO_FONT"
fi
AC_SUBST(WITH_NOTO_FONT)

dnl ===================================================================
dnl Test whether to enable online update service
dnl ===================================================================
AC_MSG_CHECKING([whether to enable online update])
ENABLE_ONLINE_UPDATE=
ENABLE_ONLINE_UPDATE_MAR=
UPDATE_CONFIG=
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
        if test "$with_update_config" = ""; then
            AC_MSG_ERROR([mar based online updater needs an update config specified with "with-update-config])
        fi
        UPDATE_CONFIG="$with_update_config"
        AC_DEFINE(HAVE_FEATURE_UPDATE_MAR)
    elif test "$enable_online_update" = "yes"; then
        AC_MSG_RESULT([yes])
        ENABLE_ONLINE_UPDATE="TRUE"
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_SUBST(ENABLE_ONLINE_UPDATE)
AC_SUBST(ENABLE_ONLINE_UPDATE_MAR)
AC_SUBST(UPDATE_CONFIG)

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

        PKG_CHECK_MODULES( CAIRO, cairo >= 1.8.0 )
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
    else
        SYSTEM_CAIRO=
        AC_MSG_RESULT([no])

        BUILD_TYPE="$BUILD_TYPE CAIRO"
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
SYSTEM_LIBLANGTAG=
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
else
    SYSTEM_LIBLANGTAG=
    AC_MSG_RESULT([no])
    BUILD_TYPE="$BUILD_TYPE LIBLANGTAG"
    LIBLANGTAG_CFLAGS="-I${WORKDIR}/UnpackedTarball/liblangtag"
    if test "$COM" = "MSC"; then
        LIBLANGTAG_LIBS="${WORKDIR}/UnpackedTarball/liblangtag/liblangtag/.libs/liblangtag.lib"
    else
        LIBLANGTAG_LIBS="-L${WORKDIR}/UnpackedTarball/liblangtag/liblangtag/.libs -llangtag"
    fi
fi
AC_SUBST(SYSTEM_LIBLANGTAG)
AC_SUBST(LIBLANGTAG_CFLAGS)
AC_SUBST(LIBLANGTAG_LIBS)

dnl ===================================================================
dnl Test whether to build libpng or rely on the system version
dnl ===================================================================

libo_CHECK_SYSTEM_MODULE([libpng],[LIBPNG],[libpng],["-I${WORKDIR}/UnpackedTarball/libpng"],["-L${WORKDIR}/LinkTarget/StaticLibrary -llibpng"])

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

if test "$ENABLE_JAVA" != "" -a "$NEED_ANT" = "TRUE"; then
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
        AC_PATH_PROGS(ANT, [ant ant.sh ant.bat ant.cmd])
    else
        if test "$_os" = "WINNT"; then
            # AC_PATH_PROGS needs unix path
            with_ant_home=`cygpath -u "$with_ant_home"`
        fi
        AbsolutePath "$with_ant_home"
        with_ant_home=$absolute_path
        AC_PATH_PROGS(ANT, [ant ant.sh ant.bat ant.cmd],,$with_ant_home/bin:$PATH)
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
        mkdir -p conftest.dir
        a_cwd=$(pwd)
        cd conftest.dir
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

        AC_TRY_COMMAND("$ANT" -buildfile conftest.xml 1>&2)
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
            AC_MSG_ERROR([Ant does not work - Some Java projects will not build!])
        fi
        cd "$a_cwd"
        rm -fr conftest.dir
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
    ant_version=`"$ANT" -version | $AWK '$3 == "version" { print $4; }'`
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
            elif test -e /usr/share/java/hamcrest/hamcrest.jar; then
                HAMCREST_JAR=/usr/share/java/hamcrest/hamcrest.jar
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
# check for sha256sum
#
SHA256SUM=

for i in shasum /usr/local/bin/shasum /usr/sfw/bin/shasum /opt/sfw/bin/shasum /opt/local/bin/shasum; do
    eval "$i -a 256 --version" > /dev/null 2>&1
    ret=$?
    if test $ret -eq 0; then
        SHA256SUM="$i -a 256"
        break
    fi
done

if test -z "$SHA256SUM"; then
    for i in sha256sum /usr/local/bin/sha256sum /usr/sfw/bin/sha256sum /opt/sfw/bin/sha256sum /opt/local/bin/sha256sum; do
        eval "$i --version" > /dev/null 2>&1
        ret=$?
        if test $ret -eq 0; then
            SHA256SUM=$i
            break
        fi
    done
fi

if test -z "$SHA256SUM"; then
    AC_MSG_ERROR([no sha256sum found!])
fi

AC_SUBST(SHA256SUM)

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

# Check for --without-lang which turns up as $with_lang being "no". Luckily there is no language with code "no".
# (Norwegian is "nb" and "nn".)
if test "$WITH_LANG" = "no"; then
    WITH_LANG=
fi

if test -z "$WITH_LANG" -o "$WITH_LANG" = "en-US"; then
    AC_MSG_RESULT([en-US])
else
    AC_MSG_RESULT([$WITH_LANG])
    GIT_NEEDED_SUBMODULES="translations $GIT_NEEDED_SUBMODULES"
    if test -z "$MSGFMT"; then
        if test -n "$LODE_HOME" -a -x "$LODE_HOME/opt/bin/msgfmt" ; then
            MSGFMT="$LODE_HOME/opt/bin/msgfmt"
        elif test -x "/opt/lo/bin/msgfmt"; then
            MSGFMT="/opt/lo/bin/msgfmt"
        else
            AC_CHECK_PROGS(MSGFMT, [msgfmt])
            if test -z "$MSGFMT"; then
                AC_MSG_ERROR([msgfmt not found. Install GNU gettext, or re-run without languages.])
            fi
        fi
    fi
    if test -z "$MSGUNIQ"; then
        if test -n "$LODE_HOME" -a -x "$LODE_HOME/opt/bin/msguniq" ; then
            MSGUNIQ="$LODE_HOME/opt/bin/msguniq"
        elif test -x "/opt/lo/bin/msguniq"; then
            MSGUNIQ="/opt/lo/bin/msguniq"
        else
            AC_CHECK_PROGS(MSGUNIQ, [msguniq])
            if test -z "$MSGUNIQ"; then
                AC_MSG_ERROR([msguniq not found. Install GNU gettext, or re-run without languages.])
            fi
        fi
    fi
fi
AC_SUBST(MSGFMT)
AC_SUBST(MSGUNIQ)
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
AC_DEFINE_UNQUOTED(WITH_LANG,"$WITH_LANG")
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
BRAND_INTRO_IMAGES="flat_logo.svg intro.png intro-highres.png"
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
AC_DEFINE_UNQUOTED(OOO_VENDOR,"$OOO_VENDOR")
AC_SUBST(OOO_VENDOR)

if test "$_os" = "Android" ; then
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

INSTALLDIRNAME=`echo AC_PACKAGE_NAME | $AWK '{gsub(" ", "", $0);print tolower($0)}'`
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

BINDIR=[$(eval echo $(eval echo $bindir))]
AC_SUBST(BINDIR)

INSTALLDIR="$LIBDIR/$INSTALLDIRNAME"
AC_SUBST(INSTALLDIR)

TESTINSTALLDIR="${BUILDDIR}/test-install"
AC_SUBST(TESTINSTALLDIR)


# ===================================================================
# OAuth2 id and secrets
# ===================================================================

AC_MSG_CHECKING([for Google Drive client id and secret])
if test "$with_gdrive_client_id" = "no" -o -z "$with_gdrive_client_id"; then
    AC_MSG_RESULT([not set])
    GDRIVE_CLIENT_ID="\"\""
    GDRIVE_CLIENT_SECRET="\"\""
else
    AC_MSG_RESULT([set])
    GDRIVE_CLIENT_ID="\"$with_gdrive_client_id\""
    GDRIVE_CLIENT_SECRET="\"$with_gdrive_client_secret\""
fi
AC_DEFINE_UNQUOTED(GDRIVE_CLIENT_ID, $GDRIVE_CLIENT_ID)
AC_DEFINE_UNQUOTED(GDRIVE_CLIENT_SECRET, $GDRIVE_CLIENT_SECRET)

AC_MSG_CHECKING([for Alfresco Cloud client id and secret])
if test "$with_alfresco_cloud_client_id" = "no" -o -z "$with_alfresco_cloud_client_id"; then
    AC_MSG_RESULT([not set])
    ALFRESCO_CLOUD_CLIENT_ID="\"\""
    ALFRESCO_CLOUD_CLIENT_SECRET="\"\""
else
    AC_MSG_RESULT([set])
    ALFRESCO_CLOUD_CLIENT_ID="\"$with_alfresco_cloud_client_id\""
    ALFRESCO_CLOUD_CLIENT_SECRET="\"$with_alfresco_cloud_client_secret\""
fi
AC_DEFINE_UNQUOTED(ALFRESCO_CLOUD_CLIENT_ID, $ALFRESCO_CLOUD_CLIENT_ID)
AC_DEFINE_UNQUOTED(ALFRESCO_CLOUD_CLIENT_SECRET, $ALFRESCO_CLOUD_CLIENT_SECRET)

AC_MSG_CHECKING([for OneDrive client id and secret])
if test "$with_onedrive_client_id" = "no" -o -z "$with_onedrive_client_id"; then
    AC_MSG_RESULT([not set])
    ONEDRIVE_CLIENT_ID="\"\""
    ONEDRIVE_CLIENT_SECRET="\"\""
else
    AC_MSG_RESULT([set])
    ONEDRIVE_CLIENT_ID="\"$with_onedrive_client_id\""
    ONEDRIVE_CLIENT_SECRET="\"$with_onedrive_client_secret\""
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
        PARALLELISM="40"
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
    if test -n "$JAVA_HOME"; then
        ILIB="$ILIB;$JAVA_HOME/lib"
    fi
    ILIB1=-link
    if test "$BITNESS_OVERRIDE" = 64; then
        ILIB="$ILIB;$COMPATH/lib/x64"
        ILIB1="$ILIB1 -LIBPATH:$COMPATH/lib/x64"
        ILIB="$ILIB;$WINDOWS_SDK_HOME/lib/x64"
        ILIB1="$ILIB1 -LIBPATH:$WINDOWS_SDK_HOME/lib/x64"
        if test $WINDOWS_SDK_VERSION = 80 -o $WINDOWS_SDK_VERSION = 81 -o $WINDOWS_SDK_VERSION = 10; then
            ILIB="$ILIB;$WINDOWS_SDK_HOME/lib/$winsdklibsubdir/um/x64"
            ILIB1="$ILIB1 -LIBPATH:$WINDOWS_SDK_HOME/lib/$winsdklibsubdir/um/x64"
        fi
        PathFormat "${UCRTSDKDIR}lib/$UCRTVERSION/ucrt/x64"
        ucrtlibpath_formatted=$formatted_path
        ILIB="$ILIB;$ucrtlibpath_formatted"
        ILIB1="$ILIB1 -LIBPATH:$ucrtlibpath_formatted"
    else
        ILIB="$ILIB;$COMPATH/lib/x86"
        ILIB1="$ILIB1 -LIBPATH:$COMPATH/lib/x86"
        ILIB="$ILIB;$WINDOWS_SDK_HOME/lib"
        ILIB1="$ILIB1 -LIBPATH:$WINDOWS_SDK_HOME/lib"
        if test $WINDOWS_SDK_VERSION = 80 -o $WINDOWS_SDK_VERSION = 81 -o $WINDOWS_SDK_VERSION = 10; then
            ILIB="$ILIB;$WINDOWS_SDK_HOME/lib/$winsdklibsubdir/um/x86"
            ILIB1="$ILIB1 -LIBPATH:$WINDOWS_SDK_HOME/lib/$winsdklibsubdir/um/x86"
        fi
        PathFormat "${UCRTSDKDIR}lib/$UCRTVERSION/ucrt/x86"
        ucrtlibpath_formatted=$formatted_path
        ILIB="$ILIB;$ucrtlibpath_formatted"
        ILIB1="$ILIB1 -LIBPATH:$ucrtlibpath_formatted"
    fi
    if test -f "$DOTNET_FRAMEWORK_HOME/lib/mscoree.lib"; then
        ILIB="$ILIB;$DOTNET_FRAMEWORK_HOME/lib"
    else
        ILIB="$ILIB;$DOTNET_FRAMEWORK_HOME/Lib/um/$WINDOWS_SDK_ARCH"
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
    MERGELIBS="TRUE"
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST([MERGELIBS])

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
        AC_MSG_ERROR([need to --disable-postgresql-sdbc - the PostgreSQL database backend.])
    fi
    if test "$enable_lotuswordpro" = "yes"; then
        AC_MSG_ERROR([need to --disable-lotuswordpro - a Lotus Word Pro file format import filter.])
    fi
    if test "$WITH_WEBDAV" = "neon"; then
        AC_MSG_ERROR([need --with-webdav=serf or --without-webdav - webdav support.])
    fi
    if test -n "$ENABLE_POPPLER"; then
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
        breeze|breeze_dark|breeze_svg|elementary|elementary_svg|karasa_jaga|karasa_jaga_svg) #blacklist of icon themes under GPL or LGPL
            AC_MSG_ERROR([need to disable icon themes from '$WITH_THEMES': $theme present, use --with-theme=tango]) ;;
        *) : ;;
        esac
    done

    ENABLE_OPENGL_TRANSITIONS=

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

AC_MSG_CHECKING([formula logger])
ENABLE_FORMULA_LOGGER=

if test "x$enable_formula_logger" = "xyes"; then
    AC_MSG_RESULT([yes])
    AC_DEFINE(ENABLE_FORMULA_LOGGER)
    ENABLE_FORMULA_LOGGER=TRUE
elif test -n "$ENABLE_DBGUTIL" ; then
    AC_MSG_RESULT([yes])
    AC_DEFINE(ENABLE_FORMULA_LOGGER)
    ENABLE_FORMULA_LOGGER=TRUE
else
    AC_MSG_RESULT([no])
fi

AC_SUBST(ENABLE_FORMULA_LOGGER)

dnl ===================================================================
dnl Setting up the environment.
dnl ===================================================================
AC_MSG_NOTICE([setting up the build environment variables...])

AC_SUBST(COMPATH)

if test "$build_os" = "cygwin"; then
    if test -d "$COMPATH/atlmfc/lib/spectre"; then
        ATL_LIB="$COMPATH/atlmfc/lib/spectre"
        ATL_INCLUDE="$COMPATH/atlmfc/include"
    elif test -d "$COMPATH/atlmfc/lib"; then
        ATL_LIB="$COMPATH/atlmfc/lib"
        ATL_INCLUDE="$COMPATH/atlmfc/include"
    else
        ATL_LIB="$WINDOWS_SDK_HOME/lib" # Doesn't exist for VSE
        ATL_INCLUDE="$WINDOWS_SDK_HOME/include/atl"
    fi
    if test "$BITNESS_OVERRIDE" = 64; then
        ATL_LIB="$ATL_LIB/x64"
    else
        ATL_LIB="$ATL_LIB/x86"
    fi
    ATL_LIB=`win_short_path_for_make "$ATL_LIB"`
    ATL_INCLUDE=`win_short_path_for_make "$ATL_INCLUDE"`

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
        if test "$ENABLE_JAVA" != ""; then
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
            pathmunge "$WINDOWS_SDK_BINDIR_NO_ARCH/x86" "before"
        fi
        # .NET 4.6 and higher don't have bin directory
        if test -f "$DOTNET_FRAMEWORK_HOME/bin"; then
            pathmunge "$DOTNET_FRAMEWORK_HOME/bin" "before"
        fi
        pathmunge "$WINDOWS_SDK_HOME/bin" "before"
        pathmunge "$CSC_PATH" "before"
        pathmunge "$MIDL_PATH" "before"
        pathmunge "$AL_PATH" "before"
        pathmunge "$MSPDB_PATH" "before"
        if test "$MSPDB_PATH" != "$CL_PATH" ; then
            pathmunge "$CL_PATH" "before"
        fi
        if test -n "$MSBUILD_PATH" ; then
            pathmunge "$MSBUILD_PATH" "before"
        fi
        if test "$BITNESS_OVERRIDE" = 64; then
            pathmunge "$COMPATH/bin/amd64" "before"
            pathmunge "$WINDOWS_SDK_BINDIR_NO_ARCH/x64" "before"
        else
            pathmunge "$COMPATH/bin" "before"
            pathmunge "$WINDOWS_SDK_BINDIR_NO_ARCH/x86" "before"
        fi
        if test "$ENABLE_JAVA" != ""; then
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
        if test "$ENABLE_JAVA" != ""; then
            pathmunge "$JAVA_HOME/bin" "after"
        fi
        ;;
    esac
fi

AC_SUBST(LO_PATH)

libo_FUZZ_SUMMARY

# Generate a configuration sha256 we can use for deps
if test -f config_host.mk; then
    config_sha256=`$SHA256SUM config_host.mk | sed "s/ .*//"`
fi
if test -f config_host_lang.mk; then
    config_lang_sha256=`$SHA256SUM config_host_lang.mk | sed "s/ .*//"`
fi

CFLAGS=$my_original_CFLAGS
CXXFLAGS=$my_original_CXXFLAGS
CPPFLAGS=$my_original_CPPFLAGS

AC_CONFIG_LINKS([include:include])
AC_CONFIG_FILES([config_host.mk
                 config_host_lang.mk
                 Makefile
                 bin/bffvalidator.sh
                 bin/odfvalidator.sh
                 bin/officeotron.sh
                 instsetoo_native/util/openoffice.lst
                 setup_native/source/packinfo/finals_instsetoo.txt
                 sysui/desktop/macosx/Info.plist])
AC_CONFIG_HEADERS([config_host/config_buildid.h])
AC_CONFIG_HEADERS([config_host/config_clang.h])
AC_CONFIG_HEADERS([config_host/config_dconf.h])
AC_CONFIG_HEADERS([config_host/config_eot.h])
AC_CONFIG_HEADERS([config_host/config_extensions.h])
AC_CONFIG_HEADERS([config_host/config_cairo_canvas.h])
AC_CONFIG_HEADERS([config_host/config_cxxabi.h])
AC_CONFIG_HEADERS([config_host/config_dbus.h])
AC_CONFIG_HEADERS([config_host/config_features.h])
AC_CONFIG_HEADERS([config_host/config_feature_desktop.h])
AC_CONFIG_HEADERS([config_host/config_feature_opencl.h])
AC_CONFIG_HEADERS([config_host/config_firebird.h])
AC_CONFIG_HEADERS([config_host/config_folders.h])
AC_CONFIG_HEADERS([config_host/config_fuzzers.h])
AC_CONFIG_HEADERS([config_host/config_gio.h])
AC_CONFIG_HEADERS([config_host/config_global.h])
AC_CONFIG_HEADERS([config_host/config_gpgme.h])
AC_CONFIG_HEADERS([config_host/config_java.h])
AC_CONFIG_HEADERS([config_host/config_langs.h])
AC_CONFIG_HEADERS([config_host/config_lgpl.h])
AC_CONFIG_HEADERS([config_host/config_libcxx.h])
AC_CONFIG_HEADERS([config_host/config_liblangtag.h])
AC_CONFIG_HEADERS([config_host/config_libnumbertext.h])
AC_CONFIG_HEADERS([config_host/config_locales.h])
AC_CONFIG_HEADERS([config_host/config_mpl.h])
AC_CONFIG_HEADERS([config_host/config_oox.h])
AC_CONFIG_HEADERS([config_host/config_options.h])
AC_CONFIG_HEADERS([config_host/config_options_calc.h])
AC_CONFIG_HEADERS([config_host/config_qrcodegen.h])
AC_CONFIG_HEADERS([config_host/config_typesizes.h])
AC_CONFIG_HEADERS([config_host/config_vendor.h])
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
elif test "$config_sha256" = `$SHA256SUM config_host.mk | sed "s/ .*//"`; then
    echo "Host Configuration unchanged - avoiding scp2 stamp update"
else
    echo > config_host.mk.stamp
fi

# touch the config lang timestamp file
if test ! -f config_host_lang.mk.stamp; then
    echo > config_host_lang.mk.stamp
elif test "$config_lang_sha256" = `$SHA256SUM config_host_lang.mk | sed "s/ .*//"`; then
    echo "Language Configuration unchanged - avoiding scp2 stamp update"
else
    echo > config_host_lang.mk.stamp
fi


if test \( "$STALE_MAKE" = "TRUE" -o "$HAVE_GNUMAKE_FILE_FUNC" != "TRUE" \) \
        -a "$build_os" = "cygwin"; then

cat << _EOS
****************************************************************************
WARNING:
Your make version is known to be horribly slow, and hard to debug
problems with. To get a reasonably functional make please do:

to install a pre-compiled binary make for Win32

 mkdir -p /opt/lo/bin
 cd /opt/lo/bin
 wget https://dev-www.libreoffice.org/bin/cygwin/make-4.2.1-msvc.exe
 cp make-4.2.1-msvc.exe make
 chmod +x make

to install from source:
place yourself in a working directory of you choice.

 git clone git://git.savannah.gnu.org/make.git

 [go to Start menu, open "Visual Studio 2017", click "VS2017 x86 Native Tools Command Prompt" or "VS2017 x64 Native Tools Command Prompt"]
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
if test "$HAVE_GNUMAKE_FILE_FUNC" != "TRUE"; then
    AC_MSG_ERROR([no file function found; the build will fail without it; use GNU make 4.0 or later])
fi
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
After the build of LibreOffice has finished successfully, you can immediately run LibreOffice using the command:
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
