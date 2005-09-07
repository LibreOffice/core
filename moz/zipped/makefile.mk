#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 19:59:34 $
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

PRJNAME=moz
TARGET=moz_unzip

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(PREBUILD_MOZAB)" == "" || "$(SYSTEM_MOZILLA)" == "YES" || "$(WITH_MOZILLA)" == "NO"

dummy:
    @echo "No need to unpack the prebuild mozab packages"

.ELSE # unpack mozab zips
.INCLUDE :  target.mk

.IF "$(BUILD_MOZAB)"=="TRUE"
ALLTAR:
    @echo "Does not need build mozab zipped!"
.ELSE
ALLTAR: \
    $(MISC)$/unpacked_$(TARGET)_inc \
    $(MISC)$/unpacked_$(TARGET)_lib \
    $(BIN)$/mozruntime.zip
.ENDIF
.IF "$(GUI)" == "UNX"

$(MISC)$/unpacked_$(TARGET)_lib : $(OS)$(COM)$(CPU)lib.zip $(OS)$(COM)$(CPU)runtime.zip
    +unzip -o -d $(LB) $(OS)$(COM)$(CPU)lib.zip && unzip -o -d $(LB) $(OS)$(COM)$(CPU)runtime.zip && $(TOUCH) $@
    +chmod -R 775 $(LB)

$(MISC)$/unpacked_$(TARGET)_inc : $(OS)$(COM)$(CPU)inc.zip
    +unzip -o -d $(INCCOM) $(OS)$(COM)$(CPU)inc.zip && $(TOUCH)	$@
    +chmod -R 775 $(INCCOM)

.ELSE

$(MISC)$/unpacked_$(TARGET)_lib : $(OS)$(COM)$(CPU)lib.zip
    +unzip -o -d $(LB) $(OS)$(COM)$(CPU)lib.zip && \
    $(TOUCH) $@

$(MISC)$/unpacked_$(TARGET)_inc : $(OS)$(COM)$(CPU)inc.zip
    +unzip -o -d $(INCCOM) $(OS)$(COM)$(CPU)inc.zip && $(TOUCH)	$@

.ENDIF

$(BIN)$/mozruntime.zip : $(OS)$(COM)$(CPU)runtime.zip
    +$(COPY) $(OS)$(COM)$(CPU)runtime.zip $(BIN)$/mozruntime.zip


.ENDIF	 # unpack mozab zips

