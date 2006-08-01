#!/bin/bash

# Linux deinstallation
# No parameter required, all RPMs listed in $HOME/.RPM_OFFICEDATABASE
# will be removed.

if [ $# -ne 1 ]
then
  echo
  echo "Usage:" $0 "<office-installation-dir>"
  echo "    <inst-destination-dir>: directory where the office to be removed is installed"
  echo
  exit 2
fi

#RPM_DB_PATH=$HOME/.RPM_DATABASE
RPM_DB_PATH=$1/.RPM_DATABASE

RPMLIST=`rpm --dbpath $RPM_DB_PATH --query --all`

# Output ...
clear
echo "#########################################"
echo "#     Deinstallation of Office RPMs     #"
echo "#########################################"
echo
echo "Path to the RPM database: " $RPM_DB_PATH
echo "RPMs to deinstall:"
echo "$RPMLIST"
echo "===================================================================="
echo

# Restore original bootstraprc
mv -f $1/program/bootstraprc.orig $1/program/bootstraprc

rpm --dbpath $RPM_DB_PATH --erase $RPMLIST || exit 2

echo "Deleting directories"
rm -rf $RPM_DB_PATH

echo
echo "Deinstallation done ..."

exit 0
