#!/bin/bash

# First parameter: Path to the RPM packages
# Second parameter: Path to the installation
# MY_OFFICE=/home/abc/opt/staroffice8-ea
# PACKAGE_PATH=/cws/instset_native/unxlngi5.pro/StarOffice/install/en-US/RPMS/

if [ \( $# -ne 2 -a $# -ne 3 \) -o -z "$1" -o -z "$2" ]
then
  echo "Two (optional three) parameters required"
  echo "Usage:"
  echo "1. parameter: Path to the RPM packages"
  echo "2. parameter: Path to the installation"
  echo "3. parameter (optional): \"-l\" (Creates a link \"soffice\" in $HOME)"
  exit 2
fi

PACKAGE_PATH=$1
MY_OFFICE=$2

LINK="nolink"
if [ $# -eq 3 ]
then
  LINK=$3
fi

if [ ! -d $PACKAGE_PATH ]
then
  echo "$PACKAGE_PATH is not a valid directory"
  echo "Usage:"
  echo "1. parameter: Path to the RPM packages"
  echo "2. parameter: Path to the installation"
  echo "3. parameter (optional): \"-l\" (Creates a link \"soffice\" in $HOME)"
  exit 2
fi

RPM_DB_PATH=$HOME/.RPM_OFFICEDATABASE

# Removing old directories
rm -rf $MY_OFFICE
rm -rf $RPM_DB_PATH

# Output ...
clear
echo "####################################################################"
echo "#     Installation of StarOffice RPMs                              #"
echo "####################################################################"
echo
echo "Path to the RPM database:       " $RPM_DB_PATH
echo "Path to the RPM packages:       " $PACKAGE_PATH
echo "Path to the installation:       " $MY_OFFICE

# Creating directories
mkdir $RPM_DB_PATH
chmod 700 $RPM_DB_PATH

# Creating RPM database and initializing
rpm --initdb --dbpath $RPM_DB_PATH

RPMLIST=`find $PACKAGE_PATH -type f -name "*.rpm" ! -name "*-core-*" ! -name "*-menus-*" ! -name "adabas*" ! -name "j2re*" -print`
CORERPM=`find $PACKAGE_PATH -type f -name "*.rpm" -name "*-core-*" -print`
PREFIX=`rpm -qlp $CORERPM | head -n1`

if [ -z "$CORERPM" ]
then
  echo "No core RPM found in $PACKAGE_PATH"
  echo "Usage:"
  echo "1. parameter: Path to the RPM packages"
  echo "2. parameter: Path to the installation"
  echo "3. parameter (optional): \"-l\" \(Creates a link \"soffice\" in $HOME\)"
  exit 2
fi

echo "Installing the RPMs"
for i in $CORERPM; do
  echo `basename $i`
done
for i in $RPMLIST; do
  echo `basename $i`
done

rpm --install --nodeps -vh --relocate $PREFIX=$MY_OFFICE --dbpath $RPM_DB_PATH $CORERPM
rpm --install --nodeps -vh --relocate $PREFIX=$MY_OFFICE --dbpath $RPM_DB_PATH $RPMLIST

# creating a link into the user home directory

if [ $LINK = "-l" ]
then
  echo
  echo rm -f $HOME/soffice
  rm -f $HOME/soffice
  echo ln -s $MY_OFFICE/program/soffice $HOME/soffice
  ln -s $MY_OFFICE/program/soffice $HOME/soffice
fi

#clear
echo
echo "Installation done ..."

exit 0