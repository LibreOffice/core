#!/bin/bash

# Linux deinstallation
# No parameter required, all RPMs listed in $HOME/.RPM_OFFICEDATABASE
# will be removed.

if [ $# -ne 0 ]
then
  echo "No parameters required"
  echo "All RPMs listed in $HOME/.RPM_OFFICEDATABASE will be removed"
  exit 2
fi

RPM_DB_PATH=$HOME/.RPM_OFFICEDATABASE

# Output ...
clear
echo "#########################################"
echo "#     Deinstallation of Office RPMs     #"
echo "#########################################"
echo
echo "Path to the RPM database: " $RPM_DB_PATH
echo "RPMs to deinstall:"
echo "`rpm --dbpath $RPM_DB_PATH --query --all`"
echo "===================================================================="
echo
rpm --dbpath $RPM_DB_PATH --erase `rpm --dbpath $RPM_DB_PATH --query --all`

echo
echo "Deleting directories"
rm -rf $RPM_DB_PATH

echo
echo "Deinstallation done ..."

exit 0