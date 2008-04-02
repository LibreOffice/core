#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: kz $ $Date: 2008-04-02 09:51:40 $
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

