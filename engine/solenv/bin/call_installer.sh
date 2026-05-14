#!/bin/sh

# unpack parameters. The first colon-separated field of the target name
# (the module family - "openoffice" / "ooohelppack") used to feed the
# Windows MSI template path; that pipeline is gone, so the field is now
# unused and parsing skips it.
VERBOSITY=$1; shift
LANG=$(echo "$@" | cut -d ':' -f 2)
PRODNAME=$(echo "$@" | cut -d ':' -f 3)
EXTENSION=$(echo "$@" | cut -d ':' -f 4)
PKGFORMAT=$(echo "$@" | cut -d ':' -f 5)
STRIP=$(echo "$@" | cut -d ':' -f 6)

# need to hack buildid?
if [ "${PKGFORMAT}${LIBO_VERSION_PATCH}" = "deb0" ] || \
   [ "${PKGFORMAT}${LIBO_VERSION_PATCH}" = "rpm0" ] ; then
    LIBO_VERSION_PATCH=1
fi

# switch to verbose?
if [ "${VERBOSITY}" = "-verbose" ] ; then
    set -x
fi

# need to strip?
if [ "${STRIP}" = "strip" ] ; then
    export ENABLE_STRIP=1
fi

# shellcheck disable=SC2086
# shellcheck disable=SC2154
${PERL} -w "${SRCDIR}"/solenv/bin/make_installer.pl \
    -f "${BUILDDIR}"/instsetoo_native/util/openoffice.lst \
    -l "${LANG}" \
    -p "${PRODUCTNAME_WITHOUT_SPACES}${PRODNAME}" \
    -u "${instsetoo_OUT}" \
    -packer "${COMPRESSIONTOOL}" \
    -buildid "${LIBO_VERSION_PATCH}" \
    ${EXTENSION:+"$EXTENSION"} \
    -format "${PKGFORMAT}" \
    "${VERBOSITY}"
