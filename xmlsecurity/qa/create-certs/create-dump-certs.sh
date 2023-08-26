#!/usr/bin/env bash
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This script generates 100 certificates (modifiable) for cases of testing UI latency for instance.
#
# Basically, it creates certificates with sender same as receiver following User [number] pattern.
#

for ((i=1; i<=100; i++))
do
    gpg --batch --gen-key <<EOF
        %no-protection
        Key-Type: RSA
        Key-Length: 2048
        Subkey-Type: RSA
        Subkey-Length: 2048
        Name-Real: User $i
        Name-Email: user$i@example.com
        Expire-Date: 1y
        %commit
EOF

    gpg --armor --export user$i@example.com > user$i.asc

    echo "Generated certificate for User $i"
done
