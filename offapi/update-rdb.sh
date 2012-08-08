#!/bin/bash

if [ $# -lt 2 ]
then
  echo "usage:"
  echo "    $0 <module name> <service name list>"
  echo ""
  echo "example:"
  echo "    $0 auth SSOPasswordCache SSOManagerFactory"
  exit 1
fi

FOLDER=$(basename `pwd`)
MODULE=$1
shift
FILES=$(echo $@ | sed -e "s#\([a-zA-Z0-9]*\)#workdir/unxlngi6.pro/UnoApiPartTarget/$FOLDER/com/sun/star/$MODULE/\\1.urd#g")

cd ../
mkdir tmp
LD_LIBRARY_PATH=solver/unxlngi6.pro/lib solver/unxlngi6.pro/bin/regmerge tmp/out1.rdb /UCR $FILES
LD_LIBRARY_PATH=solver/unxlngi6.pro/lib  solver/unxlngi6.pro/bin/regmerge tmp/out2.rdb / $FOLDER/type_reference/types.rdb tmp/out1.rdb
diff <(LD_LIBRARY_PATH=solver/unxlngi6.pro/lib solver/unxlngi6.pro/bin/regview $FOLDER/type_reference/types.rdb)  <(LD_LIBRARY_PATH=solver/unxlngi6.pro/lib solver/unxlngi6.pro/bin/regview tmp/out2.rdb) # sanity check
mv tmp/out2.rdb offapi/type_reference/types.rdb
rm -rf tmp
cd $FOLDER
