#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: ihi $ $Date: 2007-11-26 13:18:33 $
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
.INCLUDE : packtools.mk

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

