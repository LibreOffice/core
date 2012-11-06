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

# This script is needed in the process of generating exported
# symbols list on Mac OS X

# Please note that the awk expression expects to get the output of 'nm -gx'!
# On Panther we have to filter out symbols with a value "1f" otherwise external
# symbols will erroneously be added to the generated export symbols list file.
awk -v SYMBOLSREGEXP="^__ZTI.*$|^__ZTS.*$" '
match ($6,SYMBOLSREGEXP) > 0 &&  $6 !~ /_GLOBAL_/ { if (($2 != 1) && ( $2 != "1f" ) ) print $6 }'

