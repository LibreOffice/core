#!/bin/bash

# Basic argument checks
if [ $# -lt 2 ]
then
  echo "usage:"
  echo "    $0 <module name> <service name list>"
  echo ""
  echo "example:"
  echo "    $0 auth SSOPasswordCache SSOManagerFactory"
  exit 1
fi

# Move to dir script
cd "`dirname "$0"`"

# Load env vars
source ../config_host.mk 2> /dev/null

# Start the work
FOLDER=`pwd`
FOLDER=`basename $FOLDER`
MODULE=$1
shift
FILES=`echo $@ | sed -e "s#\([a-zA-Z0-9]*\)#$OUTDIR/UnoApiPartTarget/$FOLDER/com/sun/star/$MODULE/\\1.urd#g"`

cd ..
mkdir tmp
LD_LIBRARY_PATH=$OUTDIR/lib $OUTDIR/bin/regmerge tmp/out1.rdb /UCR $FILES
LD_LIBRARY_PATH=$OUTDIR/lib  $OUTDIR/bin/regmerge tmp/out2.rdb / $FOLDER/type_reference/types.rdb tmp/out1.rdb
diff <(LD_LIBRARY_PATH=$OUTDIR/lib $OUTDIR/bin/regview $FOLDER/type_reference/types.rdb)  <(LD_LIBRARY_PATH=$OUTDIR/lib $OUTDIR/bin/regview tmp/out2.rdb) # sanity check
mv tmp/out2.rdb offapi/type_reference/types.rdb
rm -rf tmp
cd $FOLDER
