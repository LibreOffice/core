#!/bin/bash

# usage:
# make -s cmd cmd='${SRCDIR}/solenv/bin/generate-flatpak-manifest.sh master' > org.libreoffice.LibreOffice.json

set -euo pipefail

# convert FOO := BAR$(MICRO) to export FOO=BAR$MICRO
source <(sed -e's#\([^ ]\{1,\}\) := #export \1=#g' ${SRCDIR}/download.lst | sed -e 's#[)(]##g')

my_gitbranch="${1?}"
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
