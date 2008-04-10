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
# $Revision: 1.18 $
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

PRJNAME=setup_native
TARGET=install

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

UNIXTEXT= \
    $(BIN)$/langpackscript.sh \
    $(BIN)$/linuxpatchscript.sh \
    $(BIN)$/javaloader.sh \
    $(BIN)$/register_extensions \
    $(BIN)$/deregister_extensions \
    $(BIN)$/unpack_update.sh \
    $(BIN)$/update.sh \
    $(BIN)$/downloadscript.sh

.IF "$(OS)" == "SOLARIS" || "$(OS)" == "LINUX"
UNIXTEXT+= $(BIN)$/stclient_wrapper.sh
.ENDIF

FAKEDB=$(BIN)$/noarch/fake-db-1.0-0.noarch.rpm
FAKEDBROOT=$(COMMONMISC)/$(TARGET)/fake-db-root

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.IF "$(OS)" == "SOLARIS" || "$(OS)" == "LINUX"

ALLTAR: $(BIN)$/install $(BIN)$/uninstall

$(BIN)$/install: install_$(OS:l).sh 
    $(PERL) install_create.pl $& $@
    -chmod 775 $@

.ENDIF

.IF "$(OS)" == "LINUX"

$(FAKEDB) : fake-db.spec 
    $(MKDIRHIER) $(FAKEDBROOT)
    $(RPM) --define "_builddir $(shell @cd $(FAKEDBROOT) && pwd)" --define "_rpmdir $(BIN)" -bb $<

$(BIN)$/install: $(FAKEDB)

$(BIN)$/uninstall: uninstall_linux.sh
    $(TYPE) $< | tr -d "\015" > $@
    -chmod 775 $@

.ENDIF

.IF "$(OS)" == "SOLARIS"

$(BIN)$/install: $(LB)$/getuid.so.stripped

$(BIN)$/uninstall: uninstall_solaris.sh $(LB)$/getuid.so.stripped
    $(PERL) install_create.pl $<  $@
    -chmod 775 $@

$(LB)$/getuid.so.stripped: $(LB)$/getuid.so
    @$(COPY) $< $@
    @/usr/ccs/bin/strip $@

.ENDIF

