#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

PRJNAME=dictionaries
TARGET=dict-ca

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk
# it might be useful to have an extension wide include to set things
# like the EXTNAME variable (used for configuration processing)
# .INCLUDE :  $(PRJ)$/source$/<extension name>$/<extension_name>.pmk

# --- Files --------------------------------------------------------

# name for uniq directory
EXTENSIONNAME:=dict-ca
EXTENSION_ZIPNAME:=dict-ca

# some other targets to be done

# --- Extension packaging ------------------------------------------

# just copy:
COMPONENT_FILES= \
    $(EXTENSIONDIR)$/ca.aff \
    $(EXTENSIONDIR)$/ca.dic \
    $(EXTENSIONDIR)$/hyph_ca.dic \
    $(EXTENSIONDIR)$/th_ca_ES_v3.dat \
    $(EXTENSIONDIR)$/dict_LICENSES-en.txt \
    $(EXTENSIONDIR)$/dict_LLICENCIES-ca.txt \
    $(EXTENSIONDIR)$/hyph_release-note_ca.txt \
    $(EXTENSIONDIR)$/hyph_release-note_en.txt \
    $(EXTENSIONDIR)$/hyph_LICENSES-en.txt \
    $(EXTENSIONDIR)$/hyph_LLICENCIES-ca.txt \
    $(EXTENSIONDIR)$/LLICENCIES-ca.txt \
    $(EXTENSIONDIR)$/README_th_ca_ES_v3.txt \
    $(EXTENSIONDIR)$/th_LICENSES-en.txt \
    $(EXTENSIONDIR)$/th_LICENCES-fr.txt \
    $(EXTENSIONDIR)$/th_LICENCIAS-es.txt \
    $(EXTENSIONDIR)$/th_LLICENCIES-ca.txt \
    $(EXTENSIONDIR)$/th_release_note-ca.txt

COMPONENT_CONFIGDEST=.
COMPONENT_XCU= \
    $(EXTENSIONDIR)$/dictionaries.xcu

# disable fetching default OOo license text
CUSTOM_LICENSE=LICENSES-en.txt
# override default license destination
PACKLICS= $(EXTENSIONDIR)$/$(CUSTOM_LICENSE)

COMPONENT_UNZIP_FILES= \
    $(EXTENSIONDIR)$/th_ca_ES_v3.idx

# add own targets to packing dependencies (need to be done before
# packing the xtension
# EXTENSION_PACKDEPS=makefile.mk $(CUSTOM_LICENSE)
EXTENSION_PACKDEPS=$(COMPONENT_FILES) $(COMPONENT_UNZIP_FILES)

# global settings for extension packing
.INCLUDE : extension_pre.mk
.INCLUDE : target.mk
# global targets for extension packing
.INCLUDE : extension_post.mk

$(EXTENSIONDIR)$/th_ca_ES_v3.idx : "$(EXTENSIONDIR)$/th_ca_ES_v3.dat"
        $(PERL) $(PRJ)$/util$/th_gen_idx.pl -o $(EXTENSIONDIR)$/th_ca_ES_v3.idx <$(EXTENSIONDIR)$/th_ca_ES_v3.dat

