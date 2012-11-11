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
if [ $# -lt 1 ]
then
  echo "usage:"
  echo "    make cmd cmd='$0 <idl file list>'"
  echo ""
  echo "example:"
  echo "    make cmd cmd='$0"
  echo "      offapi/com/sun/star/auth/SSOManagerFactory.idl"
  echo "      offapi/com/sun/star/auth/SSOPasswordCache.idl'"
  exit 1
fi

mkdir tmp
for i in "$@"; do
  "${OUTDIR_FOR_BUILD?}"/bin/regmerge tmp/out1.rdb /UCR \
    "${WORKDIR_FOR_BUILD?}"/UnoApiPartTarget/"${i%.idl}".urd
done
"${OUTDIR_FOR_BUILD?}"/bin/regmerge tmp/out2.rdb / \
  "${SRC_ROOT?}"/offapi/type_reference/types.rdb tmp/out1.rdb
echo "sanity check diff:"
diff <("${OUTDIR_FOR_BUILD?}"/bin/regview \
  "${SRC_ROOT?}"/offapi/type_reference/types.rdb) \
  <("${OUTDIR_FOR_BUILD?}"/bin/regview tmp/out2.rdb) && $? -le 1
mv tmp/out2.rdb "${SRC_ROOT?}"/offapi/type_reference/types.rdb
rm -r tmp
