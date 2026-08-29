#! /usr/bin/env bash
#
# Single-command front-end for the CODA-W (Collabora Office Desktop App -
# Windows) build.
#
# The engine and the online app are two separate autoconf projects, and online
# configure needs an already-built engine (it checks instdir), so they cannot
# be configured in one pass. This script configures the engine and writes an
# orchestrator Makefile; a subsequent single "make" then builds the engine,
# runs online configure, builds the browser bundle and finally the Visual
# Studio solution.
#
# Run it - like the whole build - from Git Bash. The engine make and MSBuild
# run natively there; the steps that need a Unix toolchain (engine and online
# configure, and the browser bundle) are dispatched to an MSYS2 installation
# automatically, so no step has to be run elsewhere by hand. MSYS2 is expected
# at C:\msys64 (set MSYS2_ROOT to override) with the packages listed in
# windows/.config/user_steps.winget installed.
#
# Out-of-tree only. Run it from an empty build directory outside the source
# tree, which then holds:
#   autogen.input    (you provide - the engine configure flags)
#   Makefile         (generated here)
#   coda-config.mk   (generated here - build-specific paths)
#   engine/          (engine out-of-tree build)
#   online/          (online out-of-tree build)

set -e

# .../online/windows/coda/build/autogen.sh: the online source root is three
# levels above this script's directory.
SRC_ROOT=$(cd "$(dirname "$0")/../../.." && pwd)
BUILD_ROOT=$(pwd)

fail() { echo "autogen.sh: $*" >&2; exit 1; }

command -v cygpath >/dev/null 2>&1 || fail "cygpath not found - run this from Git Bash."

# The Unix-toolchain steps run inside MSYS2. Git Bash and MSYS2 share the
# /c/... path convention, so paths need no translation between the two.
MSYS2_ROOT=${MSYS2_ROOT:-C:/msys64}
MSYS2_BASH="$(cygpath -u "$MSYS2_ROOT")/usr/bin/bash.exe"
[ -x "$MSYS2_BASH" ] || fail "MSYS2 not found at $MSYS2_ROOT (set MSYS2_ROOT to its install directory). MSYS2 provides the Unix toolchain for the configure steps; see windows/.config/user_steps.winget."
# Not a login shell: MSYS2's /etc/profile would replace the caller's PATH with
# its own minimal one (hiding e.g. the native node and ~/bin) and point
# CONFIG_SITE at /etc/config.site, which presets the build triplet to cygwin.
# Instead keep the caller's PATH - it survives the realm hop correctly - and
# just prepend MSYS2's own bin directories so its unix tools come first.
# MSYS=winsymlinks:nativestrict: the configures create real symlinks; force it
# so the MSYS2 side does not depend on the user's environment.
run_msys2() {
    MSYSTEM=MSYS MSYS=winsymlinks:nativestrict "$MSYS2_BASH" --noprofile --norc -c "export PATH=\"/usr/local/bin:/usr/bin:/bin:/opt/bin:\$PATH\" CONFIG_SITE=/dev/null; $1"
}
run_msys2 'command -v autoconf >/dev/null && command -v aclocal >/dev/null && test -f /usr/share/aclocal/pkg.m4' \
    || fail "autoconf/automake/pkg.m4 not found in MSYS2. Install the build packages, e.g.: $MSYS2_ROOT/usr/bin/bash -lc 'pacman -S --needed autoconf automake libtool m4 make gcc bison flex gperf nasm zip unzip gettext gettext-devel patch pkgconf python python-lxml python-pip perl' (see windows/.config/user_steps.winget)."

# The engine configure flags come from an autogen.input file in the build
# directory (the Makefile keeps it in sync to drive reconfiguration), so -
# unlike a plain engine autogen.sh - they are not accepted as arguments.
[ $# -eq 0 ] || fail "pass the engine configure flags in an autogen.input file in the build directory, not as command-line arguments."

# Refuse in-source builds: the orchestrator Makefile at the build root would
# collide with the online project's own generated Makefile, and <root>/engine
# would land on the engine source tree.
case "$BUILD_ROOT/" in
    "$SRC_ROOT/"*) fail "run this from a build directory OUTSIDE the source tree ($SRC_ROOT); in-source builds are not supported." ;;
esac
case "$SRC_ROOT/" in
    "$BUILD_ROOT/"*) fail "the source tree is inside the build directory; pick a build directory that does not contain the source." ;;
esac

# The engine build (and MSBuild property quoting) does not cope with spaces in
# these paths, so reject them early with a clear message.
case "$SRC_ROOT" in *" "*) fail "the source path must not contain spaces: $SRC_ROOT" ;; esac
case "$BUILD_ROOT" in *" "*) fail "the build path must not contain spaces: $BUILD_ROOT" ;; esac

[ -f "$BUILD_ROOT/autogen.input" ] || fail "no autogen.input in $BUILD_ROOT. Create one with the engine configure flags (e.g. --with-distro=CODAWindows ...)."

# Windows (backslash) path forms, for MSBuild and cmd.exe post-build steps.
SRC_ROOT_WIN=$(cygpath -w "$SRC_ROOT")
BUILD_ROOT_WIN=$(cygpath -w "$BUILD_ROOT")

# --- Configure the engine (out-of-tree, in MSYS2) ----------------------------
# The engine build reads autogen.input from its own build dir. Seed engine/ with
# a copy of the build-root autogen.input here (it is needed before this first
# configure); the orchestrator Makefile then keeps that copy in sync on every
# make, so editing the single build-root file drives the engine's reconfigure.
# A copy, not a symlink: native make compares a symlink's own mtime, not its
# target's, so edits to the build-root file would go unnoticed through a symlink.
mkdir -p "$BUILD_ROOT/engine"
cp "$BUILD_ROOT/autogen.input" "$BUILD_ROOT/engine/autogen.input"
run_msys2 "cd '$BUILD_ROOT/engine' && '$SRC_ROOT/engine/autogen.sh'"

# --- Locate native MSBuild for the app build --------------------------------
# vswhere always lives under %ProgramFiles(x86)%. Read that from the inherited
# Windows environment: cmd.exe /c "echo %VAR%" would have its /c mangled into a
# path by the MSYS argument conversion in Git Bash.
vswhere_dir=$(printenv 'ProgramFiles(x86)')
vswhere="$(cygpath -u "$vswhere_dir")/Microsoft Visual Studio/Installer/vswhere.exe"
msbuild_win=$("$vswhere" -latest -products '*' -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' 2>/dev/null | tr -d '\r' | head -n1)
if [ -z "$msbuild_win" ]; then
    # Older vswhere without -find: derive the path from the install location.
    vsinstall=$("$vswhere" -latest -products '*' -requires Microsoft.Component.MSBuild -property installationPath 2>/dev/null | tr -d '\r' | head -n1)
    [ -n "$vsinstall" ] && msbuild_win="$vsinstall\\MSBuild\\Current\\Bin\\MSBuild.exe"
fi
[ -n "$msbuild_win" ] || fail "MSBuild.exe not found via vswhere. Install Visual Studio with the MSBuild component and the C++ workload."
MSBUILD=$(cygpath -u "$msbuild_win")

# --- Generate the build-specific config the orchestrator Makefile includes --
# Written directly (not via sed) so the backslashes in the Windows paths need
# no escaping. SOLUTION_DIR carries no trailing backslash (a trailing backslash
# would be a line continuation in make); the recipe appends it.
cat > "$BUILD_ROOT/coda-config.mk" <<EOF
# Generated by windows/coda/build/autogen.sh - do not edit.
SRC_ROOT        := $SRC_ROOT
MSYS2_BASH      := $MSYS2_BASH
MSBUILD         := $MSBUILD
SLN             := $SRC_ROOT_WIN\\windows\\coda\\CODA.sln
SOLUTION_DIR    := $BUILD_ROOT_WIN\\online\\windows\\coda
LO_PATH_WIN     := $BUILD_ROOT_WIN\\engine\\instdir
EOF

cp "$SRC_ROOT/windows/coda/build/orchestrator.mk" "$BUILD_ROOT/Makefile"

cat <<EOF

Result: engine configured in $BUILD_ROOT/engine.
Run 'make' (from Git Bash) in $BUILD_ROOT to build the engine, the browser
bundle and the app.

EOF

# vim:set shiftwidth=4 softtabstop=4 expandtab:
