#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: obo $ $Date: 2007-01-25 13:09:36 $
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
PRJ=..$/..

PRJNAME=sc
TARGET=autil


# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

$(BIN)$/addin.zip : \
    $(MISC)$/rot.lst \
    $(MISC)$/dfa.lst
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
    $(TYPE) $(MISC)$/rot.lst | tr -s " " "\n" | zip -@ -u -j -ll $(BIN)$/addin.zip $(CHECKZIPRESULT)
    $(TYPE) $(MISC)$/dfa.lst | tr -s " " "\n" | zip -@ -u -j -ll $(BIN)$/addin.zip $(CHECKZIPRESULT)
    chmod +rw $(BIN)$/addin.zip
.ELSE
    $(TYPE) $< | zip -@ -u -j $(BIN)$/addin.zip $(CHECKZIPRESULT)
.ENDIF

ALL: \
    $(BIN)$/addin.zip \
    ALLTAR

.INCLUDE: target.mk

