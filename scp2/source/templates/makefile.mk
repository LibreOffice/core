#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:38:09 $
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

PRJNAME=scp2
TARGET=templates

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.INCLUDE :  target.mk

# force expanding
COMPLETELANGISO_VAR:=$(uniq $(completelangiso) $(alllangiso))
.EXPORT : COMPLETELANGISO_VAR

ALLTAR : $(INCCOM)$/alllangmodules.inc $(INCCOM)$/alllangmodules_root.inc $(INCCOM)$/alllangmodules_base.inc $(INCCOM)$/alllangmodules_calc.inc $(INCCOM)$/alllangmodules_draw.inc $(INCCOM)$/alllangmodules_impress.inc $(INCCOM)$/alllangmodules_math.inc $(INCCOM)$/alllangmodules_writer.inc $(INCCOM)$/alllangmodules_onlineupdate.inc

.PHONY $(INCCOM)$/alllangmodules.inc:
    @@-$(RENAME) $@ $@.tmp
    $(PERL) -w modules.pl -i $(PRJ)$/source$/templates$/module_langpack.sct -o $@.tmp && $(RENAME:s/+//) $@.tmp $@

.PHONY $(INCCOM)$/alllangmodules_root.inc:
    @@-$(RENAME) $@ $@.tmp
    $(PERL) -w modules.pl -i $(PRJ)$/source$/templates$/module_langpack_root.sct -o $@.tmp && $(RENAME:s/+//) $@.tmp $@

.PHONY $(INCCOM)$/alllangmodules_base.inc:
    @@-$(RENAME) $@ $@.tmp
    $(PERL) -w modules.pl -i $(PRJ)$/source$/templates$/module_langpack_base.sct -o $@.tmp && $(RENAME:s/+//) $@.tmp $@

.PHONY $(INCCOM)$/alllangmodules_calc.inc:
    @@-$(RENAME) $@ $@.tmp
    $(PERL) -w modules.pl -i $(PRJ)$/source$/templates$/module_langpack_calc.sct -o $@.tmp && $(RENAME:s/+//) $@.tmp $@

.PHONY $(INCCOM)$/alllangmodules_draw.inc:
    @@-$(RENAME) $@ $@.tmp
    $(PERL) -w modules.pl -i $(PRJ)$/source$/templates$/module_langpack_draw.sct -o $@.tmp && $(RENAME:s/+//) $@.tmp $@

.PHONY $(INCCOM)$/alllangmodules_impress.inc:
    @@-$(RENAME) $@ $@.tmp
    $(PERL) -w modules.pl -i $(PRJ)$/source$/templates$/module_langpack_impress.sct -o $@.tmp && $(RENAME:s/+//) $@.tmp $@

.PHONY $(INCCOM)$/alllangmodules_math.inc:
    @@-$(RENAME) $@ $@.tmp
    $(PERL) -w modules.pl -i $(PRJ)$/source$/templates$/module_langpack_math.sct -o $@.tmp && $(RENAME:s/+//) $@.tmp $@

.PHONY $(INCCOM)$/alllangmodules_writer.inc:
    @@-$(RENAME) $@ $@.tmp
    $(PERL) -w modules.pl -i $(PRJ)$/source$/templates$/module_langpack_writer.sct -o $@.tmp && $(RENAME:s/+//) $@.tmp $@

.PHONY $(INCCOM)$/alllangmodules_onlineupdate.inc:
    @@-$(RENAME) $@ $@.tmp
    $(PERL) -w modules.pl -i $(PRJ)$/source$/templates$/module_langpack_onlineupdate.sct -o $@.tmp && $(RENAME:s/+//) $@.tmp $@
