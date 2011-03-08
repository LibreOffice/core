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

INSTALLDIR=$1

# Check for old style .RPM_OFFICEDATABASE first
if [ -d ${INSTALLDIR}/.RPM_OFFICEDATABASE ]; then
  RPM_DB_PATH=${INSTALLDIR}/.RPM_OFFICEDATABASE
else
  RPM_DB_PATH=${INSTALLDIR}/.RPM_DATABASE
fi

# the RPM_DB_PATH must be absolute
if [ ! "${RPM_DB_PATH:0:1}" = "/" ]; then
  RPM_DB_PATH=`cd ${RPM_DB_PATH}; pwd`
fi

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

echo "Removing RPM database ..."
rm -rf $RPM_DB_PATH

echo
echo "Deinstallation done."

exit 0
