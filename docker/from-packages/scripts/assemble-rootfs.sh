#!/bin/sh
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Assemble a self-contained root filesystem under /rootfs that holds Collabora
# Online and every runtime dependency the hardened base image does not already
# provide. Runs in the Debian builder stage, AFTER the packages are installed
# and configured. The result is COPY'd into the distroless runtime image.
#
# There is deliberately no hand-maintained file list: the set of files to ship
# is derived from dpkg. Installing Collabora Online pulls in its full
# dependency closure; we copy exactly what that install added to a pristine
# base image, so the list tracks packaging changes automatically.

set -eu

ROOTFS=/rootfs
PKGS_BEFORE=/tmp/pkgs.before    # pristine package set, captured before install
PKGS_ADDED=/tmp/pkgs.added
FILELIST=/tmp/rootfs.files

# Packages used only to build/configure the image (not needed at runtime), plus
# packages the base image already provides. Their files are not copied. The
# engine links its externals (openssl, libpng, libxml2, ...) statically, so the
# only system libraries we ship are the few that coolwsd genuinely links
# (pulled in via the dpkg dependency diff); these here are pure tooling:
#   libcap2-bin     - setcap, used at build time only
#   ca-certificates - the base image ships its own bundle; the jail gets its
#                     copy from systemplate (built in the builder)
#   adduser         - postinst uses it to create the 'cool' user
#   fontconfig      - postinst uses fc-cache to build the font cache that goes
#                     into systemplate; the fontconfig tools are not used at runtime
#   cpio            - postinst uses it to run coolwsd-systemplate-setup
BUILD_ONLY="libcap2-bin ca-certificates adduser fontconfig cpio locales"

# Packages pulled in transitively (mostly by the fontconfig build tool and
# ca-certificates) that are not needed at runtime, because the engine links its
# own statically bundled copies or the feature is unused. The dependency-
# closure check below warns if a shipped binary actually needs one of these.
#   libfontconfig1 libfreetype6 libpng16-16t64 libexpat1
#                   - engine builds fontconfig/freetype/libpng/expat internally
#                     (distro-config --without-system-*)
#   libbrotli1      - only the system freetype needs it; the bundled freetype
#                     is built --without-brotli
#   openssl         - the openssl CLI (pulled by ca-certificates); the engine
#                     bundles its own OpenSSL
#   libpam-cap      - the pam_cap.so module; coolwsd links libpam for the admin
#                     console but its PAM stack does not use pam_cap
#   fonts-dejavu-*  - the engine bundles its own DejaVu (external/more_fonts)
# fontconfig-config is deliberately NOT here: the engine's bundled fontconfig
# reads /etc/fonts at runtime, and systemplate copies it into the jail.
BUNDLED_OR_UNUSED="libfontconfig1 libfreetype6 libpng16-16t64 libexpat1 libbrotli1 openssl libpam-cap fonts-dejavu-core fonts-dejavu-mono"

# Shared-library sonames that are part of glibc itself, which every glibc
# base image provides. These are never shipped: the runtime must use the
# base's own libc. Everything else - libssl, zlib, libstdc++, ... - is decided
# by looking at the actual base image files in base_has(), so the list adapts
# to whatever base is used. The loader's name varies by architecture
# (ld-linux-x86-64.so.2, ld-linux-aarch64.so.1, ld64.so.2, ...). This checks
# only that a library is present, not that it is glibc-ABI-compatible with the
# base.
BASE_PROVIDES_RE='^(ld[0-9]*(-linux[a-z0-9-]*)?|libc|libc_malloc_debug|libm|libmvec|libdl|libpthread|librt|libresolv|libutil|libnsl|libnss_[a-z]+|libanl|libBrokenLocale|libthread_db)\.so'

# Debian multiarch triple of the architecture being built (x86_64-linux-gnu,
# powerpc64le-linux-gnu, ...), taken from where the builder keeps its own
# libc: uname -m does not always match the triple (ppc64le vs powerpc64le).
triple=
for d in /usr/lib/*-linux-gnu*; do
    if [ -e "$d/libc.so.6" ]; then triple=${d##*/}; break; fi
done
if [ -z "$triple" ]; then
    echo "FATAL: cannot determine the multiarch triple of this build" >&2
    exit 1
fi

# Print the real file HARDENED_ROOT holds for base-relative path $1, or
# nothing if it does not resolve to a regular file. The base's FHS paths may
# be symlinks (absolute ones into e.g. /nix/store), so follow the chain within
# HARDENED_ROOT, treating absolute link targets as rooted there.
resolve_in_target() {
    p="$1"; i=0
    while [ -L "$HARDENED_ROOT$p" ] && [ "$i" -lt 40 ]; do
        t=$(readlink "$HARDENED_ROOT$p")
        case "$t" in
            /*) p="$t" ;;
            *)  p="$(dirname "$p")/$t" ;;
        esac
        i=$((i + 1))
    done
    [ -f "$HARDENED_ROOT$p" ] && printf '%s\n' "$HARDENED_ROOT$p"
    return 0
}

echo "=== Working out which packages the Collabora install added ==="
dpkg-query -W -f '${Package}\n' | sort > /tmp/pkgs.after
comm -13 "$PKGS_BEFORE" /tmp/pkgs.after > "$PKGS_ADDED"
for p in $BUILD_ONLY $BUNDLED_OR_UNUSED; do
    sed -i "/^${p}\$/d" "$PKGS_ADDED"
done
echo "Shipping the files of these packages:"
sed 's/^/  /' "$PKGS_ADDED"

echo "=== Building the file list ==="
# dpkg -L lists files, symlinks and directories; keep files and symlinks, plus
# package-owned empty directories (populated dirs are recreated by tar from the
# files they contain).
while read -r pkg; do
    dpkg-query -L "$pkg"
done < "$PKGS_ADDED" \
    | while read -r path; do
        if [ -f "$path" ] || [ -L "$path" ]; then
            printf '%s\n' "$path"
        elif [ -d "$path" ] && [ -z "$(ls -A "$path" 2>/dev/null)" ]; then
            # keep package-owned empty dirs (deliberate placeholders such as
            # the shared-preset buckets share/autotext/common and
            # share/template/common/presnt); a files-only list would drop them
            printf '%s\n' "$path"
        fi
      done | sort -u > "$FILELIST"

# Add the generated trees that no package owns: systemplate is built by the
# coolwsd postinst, child-roots and cache are runtime working directories.
find /opt/cool >> "$FILELIST"

# Trim files that have no role at runtime in the container, to shrink the image
# and its attack surface:
#   - manuals, info, lintian and bug metadata
#   - everything under /usr/share/doc EXCEPT the copyright files, which are kept
#     for license compliance, and SBOM documents, which the sbom stage indexes
#     into the image-level SBOM
#   - the systemd unit, AppArmor profile and reverse-proxy snippets: this is a
#     distroless image started directly via coolwsd, not a service managed by
#     systemd/apparmor or fronted by a bundled nginx/apache config
#   - the legacy "lool" aliases (pre-rename compatibility)
#   - tools the daemon does not use: the build-time systemplate setup script,
#     and the convert/stress/config helpers
grep -vE \
    -e '^/usr/share/(man|info|lintian|bug)/' \
    -e '^/(lib|usr/lib)/systemd/' \
    -e '^/etc/apparmor\.d/' \
    -e '^/etc/(nginx|apache2)/' \
    -e '^/usr/bin/(loolwsd|loolconfig|loolwsd-systemplate-setup)$' \
    -e '^/usr/bin/(coolwsd-systemplate-setup|coolconvert|coolstress|coolconfig)$' \
    "$FILELIST" \
    | awk '!(/^\/usr\/share\/doc\// && !/\/copyright$/ && !/sbom[^\/]*\.json$/)' \
    > "$FILELIST.trimmed"
mv "$FILELIST.trimmed" "$FILELIST"
sort -u "$FILELIST" -o "$FILELIST"

# Debian ships the usr-merge aliases /bin, /sbin, /lib, /lib64 as symlinks to
# /usr/*, and dpkg -L can list them (our -L filter keeps them). The base image
# has these as real directories, so COPY --from would try to replace a
# directory with a symlink and fail ("cannot replace to directory ... with
# file"). Drop the bare aliases; everything real lives under /usr and merges
# cleanly.
grep -vE '^/(bin|sbin|lib|lib64)$' "$FILELIST" > "$FILELIST.nomerge"
mv "$FILELIST.nomerge" "$FILELIST"

echo "=== Resolving the runtime library closure ==="
# The package diff excludes libraries already present in the Debian builder
# base, but the hardened base carries far fewer of them (it ships libz and
# libgcc_s, but NOT libstdc++ or libcap, which every coolwsd binary needs). So
# any library our binaries link that is neither already shipped nor provided by
# the base must be pulled in explicitly by shipping the Debian package that
# owns it. "provided by the base" is decided against the base image itself
# (copied to HARDENED_ROOT), with the glibc/openssl soname regex as a floor, so
# this adapts to whatever base image is used.

# Is soname $1 provided by the base image? The regex floor covers glibc's own
# sonames; everything else must actually resolve to a file in the base image,
# wherever its layout keeps libraries.
base_has() {
    printf '%s\n' "$1" | grep -Eq "$BASE_PROVIDES_RE" && return 0
    [ -n "${HARDENED_ROOT:-}" ] || return 1
    for _dir in "/usr/lib/$triple" "/lib/$triple" /usr/lib /lib /usr/lib64 /lib64; do
        [ -n "$(resolve_in_target "$_dir/$1")" ] && return 0
    done
    return 1
}

# Resolved shared libraries needed by the shipped ELF binaries. ldd exits
# non-zero on non-ELF files, so "|| true" keeps the loop alive under set -e.
needed_libs() {
    while read -r f; do
        case "$f" in *.so | *.so.* | */bin/* | */sbin/* | */program/*) ;; *) continue ;; esac
        [ -f "$f" ] || continue
        ldd "$f" 2>/dev/null || true
    done < "$FILELIST" \
        | awk '/=>/ && $3 ~ /^\// { print $3 }' | sort -u
}

# Pull in the owning packages of needed libraries that are neither shipped nor
# base-provided, until the set is stable.
while :; do
    shipped=$(sed 's#.*/##' "$FILELIST" | sort -u)
    add_pkgs=$(
        for lib in $(needed_libs); do
            soname=${lib##*/}
            printf '%s\n' "$shipped" | grep -qxF "$soname" && continue
            base_has "$soname" && continue
            # ldd resolves to the /lib alias, but dpkg records the /usr/lib
            # path; query by soname so dpkg-query finds the owning package.
            dpkg-query -S "$soname" 2>/dev/null | head -1 | cut -d: -f1
        done | sort -u
    )
    [ -z "$add_pkgs" ] && break
    echo "Pulling in base-missing libraries from: $(echo $add_pkgs)"
    for pkg in $add_pkgs; do
        dpkg-query -L "$pkg" 2>/dev/null | while read -r p; do
            # skip gdb pretty-printer autoload scripts (libstdc++6 ships one)
            case "$p" in /usr/share/gdb/*) continue ;; esac
            { [ -f "$p" ] || [ -L "$p" ]; } && printf '%s\n' "$p"
        done
    done >> "$FILELIST"
    sort -u "$FILELIST" -o "$FILELIST"
done

# Safety net: fail the build if anything needed is still unresolved.
shipped=$(sed 's#.*/##' "$FILELIST" | sort -u)
missing=$(
    for lib in $(needed_libs); do
        soname=${lib##*/}
        printf '%s\n' "$shipped" | grep -qxF "$soname" && continue
        base_has "$soname" && continue
        printf '%s\n' "$soname"
    done | sort -u
)
if [ -n "$missing" ]; then
    echo "FATAL: libraries needed at runtime but neither shipped nor provided by the base image:" >&2
    printf '%s\n' "$missing" | sed 's/^/  /' >&2
    exit 1
fi
echo "OK: every needed library is shipped or provided by the base image."

echo "=== Copying into $ROOTFS ==="
# Re-assert the file capabilities on the binaries so the copy carries them.
setcap cap_fowner,cap_chown,cap_sys_chroot=ep /usr/bin/coolforkit-caps
setcap cap_sys_admin=ep /usr/bin/coolmount

mkdir -p "$ROOTFS"
# tar preserves permissions, symlinks, hardlinks and extended attributes
# (security.capability). Strip the leading slash so paths land under $ROOTFS.
sed 's#^/##' "$FILELIST" > "$FILELIST.rel"
tar -C / -cf - --xattrs --xattrs-include='*' --no-recursion -T "$FILELIST.rel" \
    | tar -C "$ROOTFS" -xf - --xattrs --xattrs-include='*'

# Fail now if the capabilities did not make it into the staged tree, e.g. the
# builder's filesystem does not support the security.capability xattr. (The
# survival of these across COPY --from into the final image is checked
# separately by the verify-caps stage.)
for cap_bin in usr/bin/coolforkit-caps usr/bin/coolmount; do
    if [ -z "$(getcap "$ROOTFS/$cap_bin")" ]; then
        echo "FATAL: no file capabilities on $ROOTFS/$cap_bin after staging" >&2
        exit 1
    fi
done

# /etc/passwd and /etc/group are owned by the base, so dpkg did not list our
# modified copies; install them explicitly. /etc/passwd stays group-writable so
# coolwsd can map an arbitrary (e.g. OpenShift) UID to the 'cool' user.
install -D -m 0664 /etc/passwd "$ROOTFS/etc/passwd"
install -D -m 0644 /etc/group  "$ROOTFS/etc/group"

# coolwsd writes a generated SSL certificate under /tmp/ssl; make sure a
# world-writable /tmp exists in the distroless image.
install -d -m 1777 "$ROOTFS/tmp"

# The distroless base ships no locale data, so glibc cannot provide a UTF-8
# locale; coolwsd then falls back to plain C and cannot open documents with
# non-ASCII file names. Generate a minimal C.UTF-8 into the archive glibc
# reads - the standard /usr/lib/locale/locale-archive (this base's glibc does
# not honour $LOCALE_ARCHIVE). C.UTF-8 has no localedef source of its own (it
# is a glibc built-in the Nix-built base lacks), so build it from the C locale
# with the UTF-8 charmap. The locales package (charmaps) is build-only.
echo "=== Generating C.UTF-8 locale archive ==="
rm -f /usr/lib/locale/locale-archive
localedef -i C -c -f UTF-8 C.UTF-8
install -D -m 0644 /usr/lib/locale/locale-archive "$ROOTFS/usr/lib/locale/locale-archive"

# Replace the jail's glibc loader / NSS / resolver objects and the CA trust
# store with the target (base) image's own copies, when the build provides the
# base under HARDENED_ROOT. These are dlopen'd at runtime by the in-jail
# process, which runs with the base image's libc (we ship no libc of our own),
# so they must match that libc; it also gives the jail the hardened base's
# trust store.
#
# The ZenDiS base is a Nix-built image: its FHS paths are absolute symlinks
# into /nix/store, and it keeps the libraries under /usr/lib (its /lib is
# empty) while the template uses /lib. So match by basename and resolve the
# base's object by following the symlink chain within HARDENED_ROOT, treating
# absolute link targets as rooted at HARDENED_ROOT.
if [ -n "${HARDENED_ROOT:-}" ]; then
    echo "=== Overlaying jail glibc/CA from the target image ($HARDENED_ROOT) ==="
    syst="$ROOTFS/opt/cool/systemplate"

    overlay_from_target() {  # $1 = file in the template to replace
        _b=$(basename "$1")
        _src=$(resolve_in_target "/usr/lib/$triple/$_b")
        [ -z "$_src" ] && _src=$(resolve_in_target "/lib/$triple/$_b")
        [ -z "$_src" ] && _src=$(resolve_in_target "/lib64/$_b")
        if [ -n "$_src" ]; then
            cp -f --preserve=mode,timestamps "$_src" "$1"
            echo "  overlaid ${1#"$ROOTFS"} <- ${_src#"$HARDENED_ROOT"}"
        else
            echo "  KEPT     ${1#"$ROOTFS"} (not found in target)"
        fi
    }

    find "$syst" \( -name 'ld-*' -o -name 'ld64.so*' -o -name 'libnss_*.so*' \
                    -o -name 'libresolv.so*' \) \
        -type f 2>/dev/null | while read -r f; do
        overlay_from_target "$f"
    done

    ca="$syst/etc/ssl/certs/ca-certificates.crt"
    if [ -e "$ca" ]; then
        src=$(resolve_in_target "/etc/ssl/certs/ca-certificates.crt")
        if [ -n "$src" ]; then
            cp -f --preserve=mode,timestamps "$src" "$ca"
            echo "  overlaid /etc/ssl/certs/ca-certificates.crt <- ${src#"$HARDENED_ROOT"}"
        else
            echo "  KEPT     /etc/ssl/certs/ca-certificates.crt (not found in target)"
        fi
    fi

    # The builder's CA bundle is not shipped in the rootfs because the
    # hardened base provides its own; a base chosen via the runtime_base build
    # arg may not (e.g. debian:stable-slim, used for platforms ZenDiS does not
    # publish). Ship the builder's bundle then, so coolwsd outside the jail can
    # verify the TLS certificates of WOPI hosts.
    if [ -z "$(resolve_in_target /etc/ssl/certs/ca-certificates.crt)" ]; then
        install -D -m 0644 /etc/ssl/certs/ca-certificates.crt \
            "$ROOTFS/etc/ssl/certs/ca-certificates.crt"
        echo "  shipped  /etc/ssl/certs/ca-certificates.crt from the builder (target has none)"
    fi
fi

echo "=== rootfs assembled under $ROOTFS ==="
