#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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
