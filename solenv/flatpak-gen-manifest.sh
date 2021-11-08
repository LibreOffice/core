#!/bin/bash

# usage:
# make -s cmd cmd='${SRCDIR}/solenv/flatpak-gen-manifest.sh' > org.libreoffice.LibreOffice.json

set -euo pipefail

my_gitbranch=master
subst="-e s!@BRANCH@!${my_gitbranch?}!"

subst="${subst} $(
    < ${SRCDIR}/solenv/flatpak-manifest.in \
    sed ${subst} | \
    grep -o '@[A-Z0-9_]*@' | while read var; do
        temp=${var:1:-1}
        echo -n " -e s/${var}/${!temp}/"
    done
)"

exec sed ${subst} < "${SRCDIR}"/solenv/flatpak-manifest.in
