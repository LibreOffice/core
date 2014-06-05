#! /usr/bin/sed -f
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# first line - insert <model>
1,1i\
<model>

# last line - append </model>
$,$a\
</model>

# everywhere - replace the input line with <fasttoken>...</fasttoken>
s/\(.*\)/<fasttoken>\1<\/fasttoken>/
