#!/bin/sh
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Monitor any changes in upstream git repo $1 between revisions $2 and $3 that affect files that we
# copy with our external/onlineupdate/generate-sources.sh:

set -ex -o pipefail

source=${1?}
old=${2?}
new=${3?}

files=$(grep -E '^copyto [^ ]+ [^ ]+$' external/onlineupdate/generate-sources.sh | cut -f 3 -d ' ')
git -C "${source?}" log --patch "${old?}".."${new?}" -- $files
