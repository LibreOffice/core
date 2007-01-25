#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: obo $ $Date: 2007-01-25 13:23:55 $
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

PRJ := ..$/..$/..
PRJNAME := testtools
TARGET := bridgetest_idl

.INCLUDE: settings.mk
.INCLUDE: target.mk

ALLTAR: $(MISC)$/$(TARGET).cppumaker.done $(MISC)$/$(TARGET).javamaker.done

$(MISC)$/$(TARGET).cppumaker.done: $(BIN)$/bridgetest.rdb
    $(CPPUMAKER) -O$(INCCOM) -BUCR $< -X$(SOLARBINDIR)/types.rdb
    $(TOUCH) $@

$(MISC)$/$(TARGET).javamaker.done: $(BIN)$/bridgetest.rdb
    $(JAVAMAKER) -O$(CLASSDIR) -BUCR -nD -X$(SOLARBINDIR)/types.rdb $<
    $(TOUCH) $@

$(BIN)$/bridgetest.rdb: bridgetest.idl
    $(IDLC) -O$(MISC)$/$(TARGET) -I$(SOLARIDLDIR) -cid -we $<
    - $(RM) $@
    $(REGMERGE) $@ /UCR $(MISC)$/$(TARGET)$/bridgetest.urd

.IF "$(GUI)" == "WNT"

CLIMAKERFLAGS =
.IF "$(debug)" != ""
CLIMAKERFLAGS += --verbose
.ENDIF

ALLTAR: $(MISC)$/$(TARGET).cppumaker.done \
    $(MISC)$/$(TARGET).javamaker.done \
    $(BIN)$/cli_types_bridgetest.dll 

$(BIN)$/cli_types_bridgetest.dll: $(BIN)$/bridgetest.rdb
    $(CLIMAKER) $(CLIMAKERFLAGS) --out $@ -r $(SOLARBINDIR)$/cli_types.dll \
        -X $(SOLARBINDIR)$/types.rdb $< 
    $(TOUCH) $@

.ENDIF # GUI, WNT
