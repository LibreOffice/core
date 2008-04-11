#*************************************************************************
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

PRJNAME=cairo
TARGET=cairo_unzip

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(SYSTEM_CAIRO)" == "YES"

dummy:
    @echo "Nothing to do, using system cairo."

.ELIF  "$(ENABLE_CAIRO)" == ""

dummy:
    @echo "Nothing to do (Cairo not enabled)."

.ELIF  "$(OS)" != "MACOSX" && "$(GUI)" != "WNT"

dummy:
    @echo "Cairo prebuilt not supported on this platform."

.ELSE # unpack prebuild cairo libraries
.INCLUDE :  target.mk

ALLTAR: \
    $(MISC)$/unpacked_$(TARGET)_inc \
    $(MISC)$/unpacked_$(TARGET)_lib 

.IF "$(OS)" == "MACOSX"

# On Mac OS X the Cairo prebuilt binary is for native (Quartz) cairo only.
# Intel library is MACOSXGCCI
#
CAIROURL=http://porting.openoffice.org/mac/prebuilt
CAIROINC=cairo-$(OS)$(COM)$(CPU)-inc.tar.gz
CAIROLIB=cairo-$(OS)$(COM)$(CPU)-lib.tar.gz

$(MISC)$/unpacked_$(TARGET)_lib : $(CAIROLIB)
    $(COPY) $< $(MISC)$/$<
    cd $(LB) && tar xvzf ..$/misc$/$<
    $(TOUCH) $@
    chmod -R 775 $(LB)

$(MISC)$/unpacked_$(TARGET)_inc : $(CAIROINC)
    $(COPY) $< $(MISC)$/$<
    cd $(INCCOM) && tar xvzf ..$/misc$/$<
    $(TOUCH)	$@
    chmod -R 775 $(INCCOM)


.ELIF "$(GUI)" == "WNT"

CAIROURL=http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies
CAIROINC=cairo-dev-1.4.8-win32-only.zip
CAIROLIB=cairo-1.4.8-win32-only.zip

$(MISC)$/unpacked_$(TARGET)_lib : $(CAIROLIB) 
    unzip -o -d $(MISC)$/cairo$/ $<                          && \
    $(COPY) -r $(MISC)$/cairo$/bin$/*cairo*.* $(LB)$/        && \
    $(TOUCH) $@

$(MISC)$/unpacked_$(TARGET)_inc : $(CAIROINC)
    unzip -o -d $(MISC)$/cairo$/ $<                          && \
    $(COPY) -r $(MISC)$/cairo$/include$/cairo $(INCCOM)$/    && \
    $(COPY) -r $(MISC)$/cairo$/lib$/*cairo*.* $(LB)$/   	 && \
    $(TOUCH)	$@

.ENDIF # WNT

$(CAIROLIB) : 
        curl $(CAIROURL)/$(CAIROLIB) > $@

$(CAIROINC) : 
        curl $(CAIROURL)/$(CAIROINC) > $@


.ENDIF	 # unpack prebuild cairo libraries

