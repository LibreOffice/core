#!/bin/bash

# First parameter: Path to the packages
# Second parameter: Path to the local root directory
# PACKAGE_PATH=/cws/so-cwsserv05/cnpsdk/unxsoli4.pro/StarOffice/install/en-US/packages
# MY_ROOT=/export/home/is/root

if [ \( $# -ne 2 -a $# -ne 3 \) -o -z "$1" -o -z "$2" ]
then
  echo "Two (optional three) parameters required"
  echo "Usage:"
  echo "1. parameter: Path to the packages"
  echo "2. parameter: Path to the local root directory"
  echo "3. parameter (optional): \"-l\" (Creates a link \"soffice\" in $HOME)"
  exit 2
fi

PACKAGE_PATH=$1
MY_ROOT=$2

LINK="nolink"
if [ $# -eq 3 ]
then
  LINK=$3
fi

if [ ! -d $PACKAGE_PATH ]
then
  echo "$PACKAGE_PATH is not a valid directory"
  echo "Usage:"
  echo "1. parameter: Path to the packages"
  echo "2. parameter: Path to the local root directory"
  echo "3. parameter (optional): \"-l\" (Creates a link \"soffice\" in $HOME)"
  exit 2
fi

cd `dirname $0`
DIRECTORY=`pwd`

ADMINFILE=$DIRECTORY/admin
GETUID_SO=/tmp/libgetuid.so.$$
linenum=???
tail +$linenum $0 > $GETUID_SO

COREPKG=`find $PACKAGE_PATH/* -type d -prune -name "*-core-*" -print`
PKGLIST=`find $PACKAGE_PATH/* -type d -prune ! -name "*adabas*" ! -name "*j3*" ! -name "*-gnome-*" ! -name "*-cde-*" ! -name "*-core-*" -print`

if [ -z "$COREPKG" ]
then
  echo "No core package found in $PACKAGE_PATH"
  echo "Usage:"
  echo "1. parameter: Path to the packages"
  echo "2. parameter: Path to the local root directory"
  echo "3. parameter (optional): \"-l\" \(Creates a link \"soffice\" in $HOME\)"
  exit 2
fi

echo "Packages found:"
for i in $COREPKG $PKGLIST; do
  echo `basename $i`
done

# Removing old directories
rm -rf $MY_ROOT/opt
rm -rf $MY_ROOT/var

# Creating the installation directory (otherwise the user would be asked)

CORENAME=`basename $COREPKG`
INSTALLDIR=`pkgparam -d $PACKAGE_PATH $CORENAME BASEDIR`

if [ ! -d $MY_ROOT$INSTALLDIR ]
then
  mkdir -p $MY_ROOT$INSTALLDIR
fi

#Ausgaben...
clear
echo "####################################################################"
echo "#     Installation of the StarOffice packages                      #"
echo "####################################################################"
echo
echo "Path to the packages       : " $PACKAGE_PATH
echo "Path to the installation   : " $MY_ROOT

LD_PRELOAD=$GETUID_SO
export LD_PRELOAD

for i in $COREPKG $PKGLIST; do
  echo /usr/sbin/pkgadd -a $ADMINFILE -d $PACKAGE_PATH -R $MY_ROOT `basename $i`
  /usr/sbin/pkgadd -a $ADMINFILE -d $PACKAGE_PATH -R $MY_ROOT `basename $i`
done

# creating a link into the user home directory

if [ $LINK = "-l" ]
then
  echo
  echo rm -f $HOME/soffice
  rm -f $HOME/soffice
  echo ln -s $MY_ROOT$INSTALLDIR/program/soffice $HOME/soffice
  ln -s $MY_ROOT$INSTALLDIR/program/soffice $HOME/soffice
fi

rm -f $GETUID_SO

#clear
echo
echo "Installation done ..."

exit 0
