#!/bin/bash
# This file is part of the LibreOffice project.
# ------------------------------------------------------------------
#
#    Copyright (C) 2015 Canonical Ltd.
#
#    This Source Code Form is subject to the terms of the Mozilla Public
#    License, v. 2.0. If a copy of the MPL was not distributed with this
#    file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#    Author: Bryan Quigley <bryan.quigley@canonical.com>
#
# ------------------------------------------------------------------

# This is a simple script to help get AppArmor working on different distros

INST_ROOT=$1  #Where libreoffice program folder can be found
PROFILESFROM=$2  #Where the profile files are
INSTALLTO=$3  #Where should the apparmor profiles live (to be be linked to)
INSTALL=$4  #True means try to run sudo to link (doesn't reload profiles)

#For example to get this to work on Ubuntu 15.10 with stock LibreOffice:
# ./sysui/desktop/share/apparmor.sh /usr/lib/libreoffice/ sysui/desktop/apparmor/ /mnt/store/git/libo/instdir/apparmor-testing/ true

#For example on Ubuntu 15.10, with built debs from the LibreOffice website
# At the current time you need run /opt/libreofficedev5.1/program/soffice.bin directly - splash screen doesn't work
# ./sysui/desktop/share/apparmor.sh /opt/libreofficedev5.1/ sysui/desktop/apparmor/ /mnt/store/git/libo/instdir/apparmor-testing/ true

mkdir -p $INSTALLTO

#Need to convert / to . for profile names
INST_ROOT_FORMAT=${INST_ROOT/\//}
INST_ROOT_FORMAT=${INST_ROOT_FORMAT////.}

#Need to escale / for sed
INST_ROOT_SED=${INST_ROOT////\\/}

for filename in `ls $PROFILESFROM`
do
    tourl=$INSTALLTO$INST_ROOT_FORMAT$filename
    cat $PROFILESFROM$filename | sed "s/INSTDIR-/$INST_ROOT_SED/g" > $tourl

    if [ "$INSTALL" = true ] ; then
        sudo rm /etc/apparmor.d/$INST_ROOT_FORMAT$filename
        sudo ln -s $tourl /etc/apparmor.d/$INST_ROOT_FORMAT$filename
    fi

done
