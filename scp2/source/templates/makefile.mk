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

PRJ=..$/..

PRJNAME=scp2
TARGET=templates

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.INCLUDE :  target.mk

# force expanding
COMPLETELANGISO_VAR:=$(uniq $(completelangiso) $(alllangiso))
.EXPORT : COMPLETELANGISO_VAR

ALLTAR : $(INCCOM)$/alllangmodules.inc $(INCCOM)$/alllangmodules_root.inc $(INCCOM)$/alllangmodules_base.inc $(INCCOM)$/alllangmodules_calc.inc $(INCCOM)$/alllangmodules_draw.inc $(INCCOM)$/alllangmodules_impress.inc $(INCCOM)$/alllangmodules_math.inc $(INCCOM)$/alllangmodules_writer.inc $(INCCOM)$/alllangmodules_binfilter.inc $(INCCOM)$/alllangmodules_accessories_templates_root.inc $(INCCOM)$/alllangmodules_accessories_samples_root.inc $(INCCOM)$/alllangmodules_accessories_templates.inc $(INCCOM)$/alllangmodules_accessories_samples.inc $(INCCOM)$/alllangmodules_extensions_templates.inc $(INCCOM)$/alllangmodules_extensions_lightproof.inc $(INCCOM)$/allhelpmodules.inc $(INCCOM)$/allhelpmodules_root.inc

.INCLUDE .IGNORE : $(MISC)$/$(TARGET)_lang_track.mk $(MISC)$/$(TARGET)_help_track.mk
.IF "$(LAST_COMPLETELANGISO_VAR)"!="$(COMPLETELANGISO_VAR)"
PHONYTEMPL=.PHONY
.ENDIF			# "$(LAST_COMPLETELANGISO_VAR)"!="$(COMPLETELANGISO_VAR)"
$(INCCOM)$/alllangmodules%.inc $(PHONYTEMPL) : module_langpack%.sct
    @@-$(RENAME) $@ $@.tmp
    $(COMMAND_ECHO)$(PERL) -w modules.pl -i $< -o $@.tmp && $(RENAME:s/+//) $@.tmp $@
    @echo LAST_COMPLETELANGISO_VAR=$(COMPLETELANGISO_VAR) > $(MISC)$/$(TARGET)_lang_track.mk
$(INCCOM)$/allhelpmodules%.inc $(PHONYTEMPL) : module_helppack%.sct
    @@-$(RENAME) $@ $@.tmp
    $(COMMAND_ECHO)$(PERL) -w modules.pl -i $< -o $@.tmp && $(RENAME:s/+//) $@.tmp $@
    @echo LAST_COMPLETELANGISO_VAR=$(COMPLETELANGISO_VAR) > $(MISC)$/$(TARGET)_help_track.mk
