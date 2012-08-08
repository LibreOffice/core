#!/bin/bash

# Make sure we are called thru MAKE: need some env
if [ -z "$MAKEFLAGS" ]
then
  # cd to dir script
  cd "`dirname "$0"`"
  # get script relative to LO root dir
  FOLDER=`pwd`
  SCRIPT=`basename $FOLDER`/`basename $0`
  # Move to LO root dir
  cd ..
  # Re-launch the command properly
  ARGS="$SCRIPT $@"
  make cmd cmd="$ARGS"
  exit $?
fi

if [ $# -lt 2 ]
then
  echo "usage:"
  echo "    make cmd cmd=\"$0 <module name> <service name list>\""
  echo ""
  echo "example:"
  echo "    make cmd cmd=\"$0 auth SSOPasswordCache SSOManagerFactory\""
  exit 1
fi

FOLDER=`dirname $0`
MODULE=$1
shift
FILES=`echo $@ | sed -e "s#\([a-zA-Z0-9]*\)#workdir/unxlngi6.pro/UnoApiPartTarget/$FOLDER/com/sun/star/$MODULE/\\1.urd#g"`

mkdir tmp
LD_LIBRARY_PATH=$OUTDIR/lib $OUTDIR/bin/regmerge tmp/out1.rdb /UCR $FILES
LD_LIBRARY_PATH=$OUTDIR/lib  $OUTDIR/bin/regmerge tmp/out2.rdb / $FOLDER/type_reference/types.rdb tmp/out1.rdb
diff <(LD_LIBRARY_PATH=$OUTDIR/lib $OUTDIR/bin/regview $FOLDER/type_reference/types.rdb)  <(LD_LIBRARY_PATH=$OUTDIR/lib $OUTDIR/bin/regview tmp/out2.rdb) # sanity check
mv tmp/out2.rdb offapi/type_reference/types.rdb
rm -rf tmp
