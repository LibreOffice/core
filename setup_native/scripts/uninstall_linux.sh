#!/usr/bin/env bash
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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
