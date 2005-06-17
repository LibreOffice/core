#!/bin/bash

# First parameter: Root path that will be removed
# MY_ROOT=/export/home/is/root

if [ $# -ne 1 -o -z "$1" ]
then
  echo "One parameter required"
  echo "Usage:"
  echo "1. parameter: Path to the local root directory"
  echo "All packages in local database will be removed!"
  exit 2
fi

MY_ROOT=$1

cd `dirname $0`
DIRECTORY=`pwd`

GETUID_SO=/tmp/getuid.so.$$
linenum=???
tail +$linenum `basename $0` > $GETUID_SO

PKGLIST=`pkginfo -R $MY_ROOT | cut -f 2 -d ' ' | grep -v core`
COREPKG=`pkginfo -R $MY_ROOT | cut -f 2 -d ' ' | grep core`
COREPKG01=`pkginfo -R $MY_ROOT | cut -f 2 -d ' ' | grep core01`

echo "#############################################"
echo "#     Deinstallation of Office packages     #"
echo "#############################################"
echo
echo "Path to the root directory :  " $MY_ROOT
echo
echo "Packages to deinstall:"
for i in $PKGLIST $COREPKG; do
  echo $i
done

INSTALL_DIR=$MY_ROOT`pkginfo -R $MY_ROOT -r $COREPKG01`

# Restore original bootstraprc
mv -f $INSTALL_DIR/program/bootstraprc.orig $INSTALL_DIR/program/bootstraprc

for i in $PKGLIST $COREPKG; do
  LD_PRELOAD=$GETUID_SO /usr/sbin/pkgrm -n -R $MY_ROOT $i
done

# Removing old root directory, very dangerous!
# rm -rf $MY_ROOT

# removing library in temp directory
rm -f $GETUID_SO

echo
echo "Deinstallation done..."

exit 0
