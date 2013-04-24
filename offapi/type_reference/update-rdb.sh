#!/bin/bash
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

set -e

# Basic argument checks
if [ $# -lt 2 ]
then
  echo "usage:"
  echo "    make cmd cmd='$0 <rdb file> <idl file list>'"
  echo ""
  echo "example:"
  echo "    make cmd cmd='$0"
  echo "      offapi/type_reference/offapi.rdb"
  echo "      offapi/com/sun/star/auth/SSOManagerFactory.idl"
  echo "      offapi/com/sun/star/auth/SSOPasswordCache.idl'"
  exit 1
fi

rdb=${1?}
shift
mkdir tmp
for i in "$@"; do
  "${OUTDIR_FOR_BUILD?}"/bin/regmerge -v tmp/out1.rdb /UCR \
    "${WORKDIR_FOR_BUILD?}"/UnoApiPartTarget/"${i%.idl}".urd
done
"${OUTDIR_FOR_BUILD?}"/bin/regmerge -v tmp/out2.rdb / "${SRC_ROOT?}"/"${rdb?}" \
  tmp/out1.rdb
echo "sanity check diff:"
diff <("${OUTDIR_FOR_BUILD?}"/bin/regview "${SRC_ROOT?}"/"${rdb?}") \
  <("${OUTDIR_FOR_BUILD?}"/bin/regview tmp/out2.rdb) && $? -le 1
mv tmp/out2.rdb "${SRC_ROOT?}"/"${rdb?}"
rm -r tmp
