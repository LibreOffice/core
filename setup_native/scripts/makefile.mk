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

PRJNAME=setup_native
TARGET=install

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.IF "$(L10N_framework)"==""
# --- Files --------------------------------------------------------

UNIXTEXT= \
    $(BIN)$/langpackscript.sh \
    $(BIN)$/linuxpatchscript.sh \
    $(BIN)$/javaloader.sh \
    $(BIN)$/update.sh \
    $(BIN)$/downloadscript.sh

.IF "$(ENABLE_ONLINE_UPDATE)" == "TRUE"
UNIXTEXT += $(BIN)$/unpack_update.sh
.END

NOARCH=$(BIN)$/noarch
FAKEDB=$(NOARCH)/fake-db-1.0-0.noarch.rpm
FAKEDBROOT=$(COMMONMISC)/$(TARGET)/fake-db-root

# --- Targets ------------------------------------------------------

.ENDIF # L10N_framework
.INCLUDE :	target.mk
.IF "$(L10N_framework)"==""
.IF "$(OS)" == "SOLARIS" || ( "$(OS)" == "LINUX" && "$(PKGFORMAT)"!="$(PKGFORMAT:s/rpm//)" )

ALLTAR: $(BIN)$/install $(BIN)$/uninstall

$(BIN)$/install: install_$(OS:l).sh 
    $(PERL) install_create.pl $& $@
    -chmod 775 $@

.ENDIF

.IF "$(OS)" == "LINUX"
.IF "$(PKGFORMAT)"!="$(PKGFORMAT:s/rpm//)"

$(FAKEDB) : fake-db.spec 
    $(MKDIRHIER) $(FAKEDBROOT)
    $(RPM) --define "_builddir $(shell @cd $(FAKEDBROOT) && pwd)" --define "_rpmdir $(shell @cd $(BIN) && pwd)" -bb $<
    chmod g+w $(NOARCH)

$(BIN)$/install: $(FAKEDB)
.ENDIF          # "$(PKGFORMAT)"!="$(PKGFORMAT:s/rpm//)"

$(BIN)$/uninstall: uninstall_linux.sh
    $(TYPE) $< | tr -d "\015" > $@
    -chmod 775 $@

.ENDIF          # "$(OS)" == "LINUX"

.IF "$(OS)" == "SOLARIS"

$(BIN)$/install: $(LB)$/getuid.so.stripped

$(BIN)$/uninstall: uninstall_solaris.sh $(LB)$/getuid.so.stripped
    $(PERL) install_create.pl $<  $@
    -chmod 775 $@

$(LB)$/getuid.so.stripped: $(LB)$/getuid.so
    @$(COPY) $< $@
    @/usr/ccs/bin/strip $@

.ENDIF

.ENDIF # L10N_framework
