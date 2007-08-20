#***********************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: ihi $ $Date: 2007-08-20 15:43:45 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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

