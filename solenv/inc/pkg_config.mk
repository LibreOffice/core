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

.INCLUDE .IGNORE : pkgroot.mk

.IF "$(PKGCONFIG_ROOT)"!=""
PKG_CONFIG=$(PKGCONFIG_ROOT)/bin/pkg-config
.IF "$(OS)"=="SOLARIS" && "$(CPUNAME)"=="SPARC" && "$(CPU)"=="U"
PKG_CONFIG_PATH:=$(PKGCONFIG_ROOT)/lib/64/pkgconfig
.ELSE
PKG_CONFIG_PATH:=$(PKGCONFIG_ROOT)/lib/pkgconfig
.ENDIF
.EXPORT : PKG_CONFIG_PATH
PKGCONFIG_PREFIX=--define-variable=prefix=$(PKGCONFIG_ROOT)
.ELSE
PKG_CONFIG*=pkg-config
.IF "$(OS)"=="SOLARIS" && "$(CPUNAME)"=="SPARC" && "$(CPU)"=="U"
PKG_CONFIG_PATH=/usr/lib/64/pkgconfig
.EXPORT : PKG_CONFIG_PATH
.ENDIF
.ENDIF

PKGCONFIG_CFLAGS:=$(shell @$(PKG_CONFIG) $(PKGCONFIG_PREFIX) --cflags $(PKGCONFIG_MODULES))
PKGCONFIG_LIBS:=$(shell @$(PKG_CONFIG) $(PKGCONFIG_PREFIX) --libs $(PKGCONFIG_MODULES))
CFLAGS+=$(PKGCONFIG_CFLAGS)
