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
BUILD_ONLY="libcap2-bin ca-certificates adduser fontconfig cpio"

# Shared libraries the base image is known to provide (glibc and openssl). A
# needed library matching this is considered covered even if we do not ship it.
# Everything else (zlib, fontconfig, freetype, ...) must be shipped.
BASE_LIB_RE='/(ld-linux-x86-64|ld-linux|libc|libm|libdl|libpthread|librt|libresolv|libutil|libnsl|libnss_[a-z]+|libcrypt|libssl|libcrypto)\.so'

echo "=== Working out which packages the Collabora install added ==="
dpkg-query -W -f '${Package}\n' | sort > /tmp/pkgs.after
comm -13 "$PKGS_BEFORE" /tmp/pkgs.after > "$PKGS_ADDED"
for p in $BUILD_ONLY; do
    sed -i "/^${p}\$/d" "$PKGS_ADDED"
done
echo "Shipping the files of these packages:"
sed 's/^/  /' "$PKGS_ADDED"

echo "=== Building the file list ==="
# dpkg -L lists files, symlinks and directories; keep only files and symlinks.
while read -r pkg; do
    dpkg-query -L "$pkg"
done < "$PKGS_ADDED" \
    | while read -r path; do
        if [ -f "$path" ] || [ -L "$path" ]; then
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
#     for license compliance
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
    | awk '!(/^\/usr\/share\/doc\// && !/\/copyright$/)' \
    > "$FILELIST.trimmed"
mv "$FILELIST.trimmed" "$FILELIST"
sort -u "$FILELIST" -o "$FILELIST"

echo "=== Verifying the dependency closure ==="
# Builder-side ldd resolves against the builder (which has everything), so it
# cannot tell us a library is missing from the base image. Instead, flag any
# needed library that is neither shipped nor known to be base-provided; those
# are the ones to double-check against the base image.
uncovered=$(
    while read -r f; do
        case "$f" in
            *.so | *.so.* | */bin/* | */sbin/* | */program/*) ;;
            *) continue ;;
        esac
        ldd "$f" 2>/dev/null
    done < "$FILELIST" \
        | awk '/=>/ && $3 ~ /^\// { print $3 }' \
        | sort -u \
        | while read -r lib; do
            grep -qxF "$lib" "$FILELIST" && continue
            printf '%s\n' "$lib" | grep -Eq "$BASE_LIB_RE" && continue
            printf '%s\n' "$lib"
          done
)
if [ -n "$uncovered" ]; then
    echo "WARNING: needed libraries that are neither shipped nor known base-provided:"
    printf '%s\n' "$uncovered" | sed 's/^/  /'
    echo "Confirm the base image provides them; otherwise ship their package."
else
    echo "OK: every needed library is shipped or provided by the base image."
fi

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

# Source the jail's glibc loader / NSS / resolver objects and the CA trust
# store from the target (base) image instead of the Debian builder, when the
# build provides it via HARDENED_ROOT. These are dlopen'd at runtime by the
# in-jail process, which runs with the base image's libc (we ship no libc of
# our own), so they must match that libc; sourcing them here also lets the jail
# use the hardened base's libraries and trust store. Each file is overlaid only
# if the target image carries it at the same path; otherwise the builder's copy
# is kept (and logged), never removed.
if [ -n "${HARDENED_ROOT:-}" ]; then
    echo "=== Overlaying jail glibc/CA from the target image ($HARDENED_ROOT) ==="
    syst="$ROOTFS/opt/cool/systemplate"
    find "$syst" \( -name 'ld-*' -o -name 'libnss_*.so*' -o -name 'libresolv.so*' \) \
        -type f 2>/dev/null | while read -r f; do
        rel=${f#"$syst"/}
        if [ -e "$HARDENED_ROOT/$rel" ]; then
            cp -a -L "$HARDENED_ROOT/$rel" "$f"
            echo "  overlaid /$rel"
        else
            echo "  KEPT     /$rel (not provided by the target image)"
        fi
    done
    ca="etc/ssl/certs/ca-certificates.crt"
    if [ -e "$syst/$ca" ]; then
        if [ -e "$HARDENED_ROOT/$ca" ]; then
            cp -a -L "$HARDENED_ROOT/$ca" "$syst/$ca"
            echo "  overlaid /$ca"
        else
            echo "  KEPT     /$ca (not provided by the target image)"
        fi
    fi
fi

echo "=== rootfs assembled under $ROOTFS ==="
