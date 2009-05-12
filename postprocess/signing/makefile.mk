#***********************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.3 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..


PRJNAME=postprocess
TARGET=signing

.INCLUDE : settings.mk

# PFXFILE	has to be set elsewhere
# PFXPASSWORD	has to be set elsewhere

EXCLUDELIST=no_signing.txt
LOGFILE=$(MISC)$/signing_log.txt
IMAGENAMES=$(SOLARBINDIR)$/*.dll $(SOLARBINDIR)$/so$/*.dll $(SOLARBINDIR)$/*.exe $(SOLARBINDIR)$/so$/*.exe
TIMESTAMPURL*="http://timestamp.verisign.com/scripts/timstamp.dll"

signing.done :
.IF "$(VISTA_SIGNING)"!=""
.IF "$(COM)"=="MSC"
.IF "$(product)"=="full"
    $(PERL) signing.pl -e $(EXCLUDELIST) -f $(PFXFILE) -p $(PFXPASSWORD) -t $(TIMESTAMPURL) $(IMAGENAMES) && $(TOUCH) $(MISC)$/signing.done
.ELSE  # "$(product)"=="full"
    @echo Doing nothing on non product builds ...
.ENDIF # "$(product)"=="full"
.ELSE  # "$(GUI)"=="MSC"
    @echo Nothing to do, signing is Windows \(MSC\) only.
.ENDIF # "$(GUI)"=="MSC"
.ELSE  # "$(VISTA_SIGNING)"!=""
    @echo Doing nothing. To switch on signing set VISTA_SIGNING=TRUE ...
.ENDIF # "$(VISTA_SIGNING)"!=""

.INCLUDE : target.mk

