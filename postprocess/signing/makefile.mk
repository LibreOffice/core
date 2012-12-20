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

PRJ=..


PRJNAME=postprocess
TARGET=signing

.INCLUDE : settings.mk

# PFXFILE	has to be set elsewhere
# PFXPASSWORD	has to be set elsewhere

EXCLUDELIST=no_signing.txt
LOGFILE=$(MISC)$/signing_log.txt
IMAGENAMES=$(SOLARBINDIR)$/*.dll $(SOLARBINDIR)$/*.exe
TIMESTAMPURL*="http://timestamp.globalsign.com/scripts/timestamp.dll"

signing.done :
.IF "$(WINDOWS_BUILD_SIGNING)"=="TRUE"
.IF "$(COM)"=="MSC"
.IF "$(product)"=="full"
    $(PERL) signing.pl -e $(EXCLUDELIST) -l $(LOGFILE) -f $(PFXFILE) -p $(PFXPASSWORD) -t $(TIMESTAMPURL) $(IMAGENAMES) && $(TOUCH) $(MISC)$/signing.done
.ELSE  # "$(product)"=="full"
    @echo Doing nothing on non product builds ...
.ENDIF # "$(product)"=="full"
.ELSE  # "$(GUI)"=="MSC"
    @echo Nothing to do, signing is Windows \(MSC\) only.
.ENDIF # "$(GUI)"=="MSC"
.ELSE  # "$(WINDOWS_BUILD_SIGNING)"=="TRUE"
    @echo Doing nothing. Windows build signing is disabled ...
.ENDIF # "$(WINDOWS_BUILD_SIGNING)"=="TRUE"

.INCLUDE : target.mk

