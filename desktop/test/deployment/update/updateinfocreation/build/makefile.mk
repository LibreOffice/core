#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: ihi $ $Date: 2007-11-22 15:53:16 $
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

PRJ		= ..$/..$/..$/..$/..
PRJNAME = updateinfo
PACKAGE = com$/sun$/star$/comp$/smoketest
TARGET  = com_sun_star_comp_smoketest

no_common_build_zip:=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

JARFILES = ridl.jar jurt.jar unoil.jar juh.jar


JARTARGET = TestExtension.jar
JARCOMPRESS 	= TRUE
CUSTOMMANIFESTFILE = MANIFEST.MF

ZIP1TARGET=updateinfo
ZIP1LIST=*
ZIPFLAGS=-r
ZIP1DIR=$(MISC)$/$(TARGET)
ZIP1EXT=.oxt

EXTUPDATEINFO_NAME=org.openoffice.extensions.testarea.desktop.updateinfo.update.xml
EXTUPDATEINFO_SOURCE=description.xml
EXTUPDATEINFO_URLS = http://extensions.openoffice.org/testarea/desktop/updateinfocreation/update/updateinfo.oxt
# --- Files --------------------------------------------------------

COPY_OXT_MANIFEST:= $(MISC)$/$(TARGET)$/META-INF$/manifest.xml
JAVAFILES = TestExtension.java

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

$(JARTARGETN) : $(MISC)$/$(TARGET).javamaker.done

$(JAVACLASSFILES) : $(MISC)$/$(TARGET).javamaker.done

$(MISC)$/$(TARGET).javamaker.done: $(BIN)$/TestExtension.rdb
    $(JAVAMAKER) -O$(CLASSDIR) -BUCR -nD -X$(SOLARBINDIR)/types.rdb $<
    $(TOUCH) $@

$(BIN)$/TestExtension.rdb: TestExtension.idl
    $(IDLC) -O$(MISC) -I$(SOLARIDLDIR) -cid -we $<
    +-$(RM) $@
    $(REGMERGE) $@ /UCR $(MISC)$/TestExtension.urd

$(MISC)$/$(ZIP1TARGET).createdir :
    +$(MKDIRHIER) $(MISC)$/$(TARGET)$/META-INF >& $(NULLDEV) && $(TOUCH) $@

$(MISC)$/$(TARGET)_resort : manifest.xml $(JARTARGETN) $(MISC)$/$(ZIP1TARGET).createdir $(BIN)$/TestExtension.rdb description.xml
    $(GNUCOPY) -u manifest.xml $(MISC)$/$(TARGET)$/META-INF$/manifest.xml
    $(GNUCOPY) -u $(JARTARGETN) $(MISC)$/$(TARGET)$/$(JARTARGET)
    $(GNUCOPY) -u $(BIN)$/TestExtension.rdb $(MISC)$/$(TARGET)$/TestExtension.rdb
    $(GNUCOPY) -u description.xml $(MISC)$/$(TARGET)$/description.xml	
    $(TOUCH) $@

$(ZIP1TARGETN) : $(MISC)$/$(TARGET)_resort $(MISC)$/$(ZIP1TARGET).createdir

