#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 16:29:23 $
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

PRJNAME=setup_native
TARGET=install

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

UNIXTEXT= \
    $(BIN)$/langpackscript.sh \
    $(BIN)$/linuxpatchscript.sh \
    $(BIN)$/downloadscript.sh

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.IF "$(OS)" == "LINUX"

ALLTAR : $(BIN)$/install $(BIN)$/uninstall 

$(BIN)$/install: install_linux.sh
    $(TYPE) $< | tr -d "\015" > $@ ; chmod 775 $@

$(BIN)$/uninstall: uninstall_linux.sh
    $(TYPE) $< | tr -d "\015" > $@ ; chmod 775 $@

.ENDIF

.IF "$(OS)" == "SOLARIS"

ALLTAR: $(BIN)$/install $(BIN)$/uninstall $(LB)$/getuid.so.stripped

$(BIN)$/install: install_solaris.sh $(LB)$/getuid.so.stripped
    $(PERL) install_create.pl install_$(OS).sh $(LB)$/getuid.so.stripped $(BIN)$/install ; chmod 775 $(BIN)$/install

$(BIN)$/uninstall: uninstall_solaris.sh $(LB)$/getuid.so.stripped
    $(PERL) install_create.pl uninstall_$(OS).sh $(LB)$/getuid.so.stripped $(BIN)$/uninstall ; chmod 775 $(BIN)$/install

$(LB)$/getuid.so.stripped: $(LB)$/getuid.so
    @$(COPY) $< $@
    @/usr/ccs/bin/strip $@

.ENDIF

