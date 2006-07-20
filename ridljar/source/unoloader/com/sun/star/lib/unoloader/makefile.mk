#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: kz $ $Date: 2006-07-20 16:06:08 $
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

PRJ := ..$/..$/..$/..$/..$/..$/..
PRJNAME := ridljar

TARGET := unoloader_cssl_unoloader
PACKAGE := com$/sun$/star$/lib$/unoloader

.INCLUDE: settings.mk

# Use a non-default CLASSDIR, so that the util/makefile.mk can use
# JARCLASSDIRS=com and nevertheless not include this package in ridl.jar:
CLASSDIR != $(OUT)$/class$/unoloader

JAVAFILES = \
    UnoClassLoader.java \
    UnoLoader.java

JARTARGET = unoloader.jar
JARCLASSDIRS = $(PACKAGE)
CUSTOMMANIFESTFILE = manifest

.INCLUDE: target.mk

.IF "$(SOLAR_JAVA)" != ""

.IF "$(depend)" == ""
$(JAVACLASSFILES): $(MISC)$/$(TARGET).mkdir.done
.ENDIF

$(MISC)$/$(TARGET).mkdir.done .ERRREMOVE:
    $(MKDIRHIER) $(CLASSDIR)
    $(TOUCH) $@

.ENDIF
