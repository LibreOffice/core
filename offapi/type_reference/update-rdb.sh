#!/bin/bash

# Basic argument checks
if [ $# -lt 1 ]
then
  echo "usage:"
  echo "    $0 <idl file list>"
  echo ""
  echo "example:"
  echo "    $0 \\"
  echo "      offapi/com/sun/star/auth/SSOManagerFactory.idl \\"
  echo "      offapi/com/sun/star/auth/SSOPasswordCache.idl"
  exit 1
fi

# Load env vars
source "$(dirname "$0")"/../../config_host.mk 2>/dev/null

set -e
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
