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

ADMINFILE=$DIRECTORY/admin
GETUID_SO=/tmp/getuid.so.$$
linenum=???
tail +$linenum $0 > $GETUID_SO

PKGLIST=`pkginfo -R $MY_ROOT | cut -f 2 -d ' ' | grep -v core`
COREPKG=`pkginfo -R $MY_ROOT | cut -f 2 -d ' ' | grep core`

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

LD_PRELOAD=$GETUID_SO
export LD_PRELOAD

for i in $PKGLIST $COREPKG; do
  echo /usr/sbin/pkgrm -a $ADMINFILE -R $MY_ROOT $i
  /usr/sbin/pkgrm -a $ADMINFILE -R $MY_ROOT $i
done

# Removing old root directory, very dangerous!
# rm -rf $MY_ROOT

# removing library in temp directory
rm -f $GETUID_SO

echo
echo "Deinstallation done..."

exit 0
