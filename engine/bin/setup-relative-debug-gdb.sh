#!/bin/bash

# Install (or print) a gdb hook for builds configured with relative debug
# paths (--enable-relative-debug-paths, which --enable-ccache-shared turns
# on for you).  With those builds the recorded compilation directory is ".",
# so gdb resolves split-dwarf .dwo files and source relative to its own
# working directory.  The hook watches for the first instdir/program/ object
# to load and cd's gdb into that build's engine root, so the relative paths
# resolve from any working directory and from any worktree, with nothing to
# register per tree.
#
# To debug a running kit, use the kitattach command this block also defines.
# "kitattach PID" reads /proc/PID/exe to find that kit's engine root, cd's
# there, sets sysroot to / for the jail, then attaches.  The cd happens
# before the attach, so the kit binary's own split-dwarf externals resolve
# on the first symbol read.  The new_objfile hook alone cannot help here: it
# fires only after the attach has read those externals.
#
# Usage: setup-relative-debug-gdb.sh [--install | --print]
#   --install   append the hook to your gdb init file, once (idempotent)
#   --print     write the hook to stdout for you to paste yourself
#
# --install picks the file gdb actually reads: ~/.config/gdb/gdbinit when it
# exists (recent gdb prefers it and ignores ~/.gdbinit once it does), else
# ~/.gdbinit, else a fresh ~/.config/gdb/gdbinit.  Set GDBINIT to override.

set -euo pipefail

readonly BEGIN_MARK="# engine relative-debug-paths hook -- begin (delete this block to disable)"
readonly END_MARK="# engine relative-debug-paths hook -- end"
readonly VERSION_MARK="# relative-debug-paths hook version:"

# Bump on any change to the emitted block.  --install compares this against
# the version recorded in an already-installed block and replaces it when
# they differ, so an old block does not linger silently.  A block with no
# version line predates versioning and is always replaced.
readonly HOOK_VERSION=1

print_hook() {
    cat <<EOF
$BEGIN_MARK
$VERSION_MARK $HOOK_VERSION
# cd into the engine root of a loaded instdir/program/ binary so its
# relative .dwo and source paths resolve from any working directory.
python
import gdb
import os
_co_done = [False]
def _co_engine_cd(event):
    if _co_done[0]:
        return
    name = getattr(getattr(event, "new_objfile", None), "filename", "") or ""
    i = name.find("/instdir/program/")
    if i < 0:
        return
    _co_done[0] = True
    gdb.execute("cd " + name[:i], to_string=True)
gdb.events.new_objfile.connect(_co_engine_cd)

# Engine root that resolves a binary's relative .dwo paths: the prefix before
# instdir/program/ for an installed binary, otherwise the engine/ sibling of
# the binary's own directory (the forkit sits at the monorepo top level).
def _co_kit_root(exe):
    if "/instdir/program/" in exe:
        return exe.split("/instdir/program/")[0]
    d = os.path.dirname(exe)
    return d + "/engine" if os.path.isdir(d + "/engine/workdir") else d
class _CoKitAttach(gdb.Command):
    """kitattach PID: cd to the kit's engine root, set sysroot to / for the jail, then attach."""
    def __init__(self):
        super(_CoKitAttach, self).__init__("kitattach", gdb.COMMAND_RUNNING)
    def invoke(self, arg, from_tty):
        argv = gdb.string_to_argv(arg)
        if len(argv) != 1:
            raise gdb.GdbError("usage: kitattach PID")
        pid = argv[0]
        exe = os.path.realpath("/proc/%s/exe" % pid)
        if not os.path.exists(exe):
            raise gdb.GdbError("kitattach: no running process " + pid)
        gdb.execute("cd " + _co_kit_root(exe), to_string=True)
        gdb.execute("set sysroot /")
        gdb.execute("attach " + pid)
_CoKitAttach()
end
$END_MARK
EOF
}

target_file() {
    if [ -n "${GDBINIT:-}" ]; then
        printf '%s\n' "$GDBINIT"
        return
    fi
    local xdg="${XDG_CONFIG_HOME:-$HOME/.config}/gdb/gdbinit"
    if [ -f "$xdg" ]; then
        printf '%s\n' "$xdg"
    elif [ -f "$HOME/.gdbinit" ]; then
        printf '%s\n' "$HOME/.gdbinit"
    else
        printf '%s\n' "$xdg"
    fi
}

installed_version() {
    # version recorded in the block, or empty if absent (pre-versioning)
    sed -n "s/^${VERSION_MARK} //p" "$1" | head -1
}

remove_block() {
    # print the file with the begin..end block (and one blank line ahead of
    # it) dropped, so a replacement appends cleanly
    awk -v b="$BEGIN_MARK" -v e="$END_MARK" '
        index($0, b) { if (prev_blank) hold = ""; skip = 1 }
        !skip {
            if (hold != "") { print hold; hold = "" }
            if ($0 == "") { hold = $0; prev_blank = 1 } else { print; prev_blank = 0 }
        }
        index($0, e) { skip = 0 }
        END { if (hold != "") print hold }
    ' "$1"
}

install_hook() {
    local f action installed
    f="$(target_file)"
    if [ -f "$f" ] && grep -qF "$BEGIN_MARK" "$f"; then
        installed="$(installed_version "$f")"
        if [ "$installed" = "$HOOK_VERSION" ]; then
            printf 'hook already current (version %s) in %s, nothing to do\n' "$HOOK_VERSION" "$f"
            return
        fi
        local tmp
        tmp="$(mktemp)"
        remove_block "$f" > "$tmp"
        cat "$tmp" > "$f"
        rm -f "$tmp"
        action="updated ${installed:-unversioned} to version $HOOK_VERSION in"
    else
        action="installed version $HOOK_VERSION into"
    fi
    mkdir -p "$(dirname "$f")"
    {
        printf '\n'
        print_hook
    } >> "$f"
    printf '%s %s\n' "$action" "$f"
    if command -v gdb >/dev/null 2>&1; then
        if gdb -batch -ex 'python print("_co_engine_cd" in globals())' -ex quit 2>/dev/null | grep -qx True; then
            printf 'gdb picked it up (kitattach PID is now available)\n'
        else
            printf 'warning: gdb did not pick up the hook, check that %s is the file it reads\n' "$f" >&2
        fi
    fi
}

case "${1:-}" in
    --install) install_hook ;;
    --print)   print_hook ;;
    *)
        printf 'usage: %s [--install | --print]\n' "$(basename "$0")" >&2
        exit 1
        ;;
esac
