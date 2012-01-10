#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
    $(BIN)$/register_extensions \
    $(BIN)$/deregister_extensions \
    $(BIN)$/unpack_update.sh \
    $(BIN)$/update.sh \
    $(BIN)$/downloadscript.sh

.IF "$(OS)" == "SOLARIS" || "$(OS)" == "LINUX"
UNIXTEXT+= $(BIN)$/stclient_wrapper.sh
.ENDIF

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
