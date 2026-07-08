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
# run natively there; the steps that need a Unix environment (engine and online
# configure, and the browser bundle) are dispatched to WSL automatically, so no
# step has to be run under wsl by hand.
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
command -v wsl.exe >/dev/null 2>&1 || fail "wsl.exe not found - WSL is required for the engine and online configure steps."

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

# Path forms we need: Windows (backslash, for MSBuild and cmd.exe post-build
# steps) and WSL (/mnt/..., for the steps dispatched into WSL).
SRC_ROOT_WIN=$(cygpath -w "$SRC_ROOT")
BUILD_ROOT_WIN=$(cygpath -w "$BUILD_ROOT")
# wslpath must get a forward-slash Windows path (cygpath -m): a backslash path
# passed through wsl.exe has its backslashes stripped (C:\lo -> C:lo).
SRC_ROOT_WSL=$(wsl.exe wslpath -u "$(cygpath -m "$SRC_ROOT")" | tr -d '\r')
BUILD_ROOT_WSL=$(wsl.exe wslpath -u "$(cygpath -m "$BUILD_ROOT")" | tr -d '\r')

# --- Configure the engine (out-of-tree, in WSL) -----------------------------
# The engine build reads autogen.input from its own build dir. Seed engine/ with
# a copy of the build-root autogen.input here (it is needed before this first
# configure); the orchestrator Makefile then keeps that copy in sync on every
# make, so editing the single build-root file drives the engine's reconfigure.
# A copy, not a symlink: native make compares a symlink's own mtime, not its
# target's, so edits to the build-root file would go unnoticed through a symlink.
mkdir -p "$BUILD_ROOT/engine"
cp "$BUILD_ROOT/autogen.input" "$BUILD_ROOT/engine/autogen.input"
wsl.exe --exec bash -c "cd '$BUILD_ROOT_WSL/engine' && '$SRC_ROOT_WSL/engine/autogen.sh'"

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
SRC_ROOT_WSL    := $SRC_ROOT_WSL
ENGINE_BUILD_WSL := $BUILD_ROOT_WSL/engine
ONLINE_BUILD_WSL := $BUILD_ROOT_WSL/online
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
