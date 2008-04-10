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
# $Revision: 1.10 $
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
TARGET=rebase

.INCLUDE : settings.mk

.INCLUDE : target.mk

STARTADDRESS=0x68000000
BASEADDRESSES=$(MISC)$/coffbase.txt
EXCLUDELIST=no_rebase.txt
LOGFILE=$(MISC)$/rebase_log.txt
IMAGENAMES=$(SOLARBINDIR)$/*.dll $(SOLARBINDIR)$/so$/*.dll

ALLTAR : $(BASEADDRESSES)

$(BASEADDRESSES) .PHONY :
.IF "$(GUI)"=="WNT"
.IF "$(product)"=="full"
    $(PERL) rebase.pl -C $(BASEADDRESSES) -b $(STARTADDRESS) -d -e 10000 -l $(LOGFILE) -m $(MISC) -v -R $(SOLARBINDIR) -N $(EXCLUDELIST) $(IMAGENAMES)
.ELSE	# "$(product)"=="full"
    @echo Doing nothing on non product builds ...
.ENDIF	# "$(product)"=="full"
.ELSE	# "$(GUI)"=="WNT"
    @echo Nothing to do, 'rebase' is windows only.
.ENDIF


