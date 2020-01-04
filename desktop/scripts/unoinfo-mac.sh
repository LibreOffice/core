#!/bin/sh
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

set -e

# resolve installation directory
sd_res="$0"
while [ -h "$sd_res" ] ; do
    sd_dirname=$(dirname "$sd_res")
    cd "$sd_dirname"
    sd_basename=$(basename "$sd_res")
    sd_res=$(ls -l "$sd_basename" | sed "s/.*$sd_basename -> //g")
done
sd_dirname=$(dirname "$sd_res")
cd "$sd_dirname"
sd_prog=$(pwd)

case "$1" in
c++)
    printf '%s' "$sd_prog/../Frameworks"
    ;;
java)
    printf '0%s\0%s\0%s\0%s\0%s' \
        "$sd_prog/../Resources/java/ridl.jar" \
        "$sd_prog/../Resources/java/jurt.jar" \
        "$sd_prog/../Resources/java/juh.jar" \
        "$sd_prog/../Resources/java/unoil.jar" "$sd_prog"
    ;;
*)
    exit 1
    ;;
esac
