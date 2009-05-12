#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: extension_tmpl.mk,v $
#
# $Revision: 1.3.56.1 $
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

############################################
#
# sample makefile for extension packing
#
# NOTE: not all of this is required or useful
# for every specific case
#
############################################

PRJ=..$/..

PRJNAME=my_project
TARGET=some_unique_target

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk
# it might be useful to have an extension wide include to set things
# like the EXTNAME variable (used for configuration processing)
# .INCLUDE :  $(PRJ)$/source$/<extension name>$/<extension_name>.pmk

# --- Files --------------------------------------------------------

# name for uniq directory
EXTENSIONNAME:=MyExtension
EXTENSION_ZIPNAME:=MyExtension

.IF "$(ENABLE_MYEXTENSION)" != "YES"
@all:
    @echo "$(EXTENSIONNAME) - extension disabled."
.ENDIF
.IF "$(SOLAR_JAVA)"==""
@all:
    @echo "$(EXTENSIONNAME) - extension requires java."
.ENDIF


# some other targets to be done



# --- Extension packaging ------------------------------------------

# these variables are optional and defaulted to the current directory

DESCRIPTION_SRC:=config$/description.xml
MANIFEST_SRC:=config$/manifest.xml
COMPONENT_CONFIGDIR:=config

# ------------------------------------------------------------------

# optional: generated list of .xcu files contained in the extension
COMPONENT_MANIFEST_GENERIC:=TRUE
COMPONENT_MANIFEST_SEARCHDIR:=registry


# variables to trigger predifined targets
# just copy:
COMPONENT_FILES= \
    $(EXTENSIONDIR)$/xMyExtension$(EXECPOST) \
    $(EXTENSIONDIR)$/some_local.html

# localized configuration files
COMPONENT_MERGED_XCU= \
    $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/Addons.xcu \
    $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/extension$/MyExtension.xcu

# other configuration files
COMPONENT_XCU= \
    $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/Data.xcu

# location of configurationfiles inside extension,
# "." for flat .xcu files
#COMPONENT_CONFIGDEST=.

# native libraries
COMPONENT_LIBRARIES= \
    $(EXTENSIONDIR)$/$(SHL1TARGET)$(DLLPOST)

# jar files
COMPONENT_JARFILES = \
    $(EXTENSIONDIR)$/MyExtension.jar

# disable fetching default OOo license text
#CUSTOM_LICENSE=my_license.txt
# override default license destination
#PACKLICS= $(EXTENSIONDIR)$/registration$/$(CUSTOM_LICENSE)

# -------------------------------
#  variables for own targets specific to this extension; no common
#  target available...
#
CONVERTER_FILE= \
    $(EXTENSIONDIR)$/xMyExtension$(EXECPOST) \

COMPONENT_DIALOGS= \
    $(EXTENSIONDIR)$/basic$/Module1.xba \
    $(EXTENSIONDIR)$/basic$/TargetChooser.xdl \
    $(EXTENSIONDIR)$/basic$/dialog.xlb \
    $(EXTENSIONDIR)$/basic$/impress.png \
    $(EXTENSIONDIR)$/basic$/script.xlb \
    $(EXTENSIONDIR)$/basic$/writer.png

# add own targets to packing dependencies (need to be done before
# packing the xtension
EXTENSION_PACKDEPS=$(CONVERTER_FILE)  $(COMPONENT_DIALOGS) makefile.mk $(CUSTOM_LICENSE)

# global settings for extension packing
.INCLUDE : extension_pre.mk
.INCLUDE : target.mk
# global targets for extension packing
.INCLUDE : extension_post.mk

# own targets 
$(CONVERTER_FILE) : $(SOLARBINDIR)$/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(COMPONENT_DIALOGS) : dialogs$/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(CUSTOM_LICENSE) : my_license.txt
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

