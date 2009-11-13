#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: tg_moz.mk,v $
#
# $Revision: 1.8 $
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


.IF "$(EXPORTS)"!=""
EXPORT_HEADER=export_mozilla_header
.ENDIF

.IF "$(XPIDLSRCS)"!=""
XPIDL_TYPELIB_MODULE=$(MISC)/$(XPIDL_MODULE).xpt
XPIDLHDR:=$(XPIDLSRCS:s/.idl/.h/)
XPIDLXPT:=$(XPIDLSRCS:s/.idl/.xpt/)
EXTRA_MOZ_TARGET+= $(foreach,i,$(XPIDLHDR) $(INCCOM)/$i)
EXTRA_MOZ_TARGET+= $(foreach,i,$(XPIDLXPT) $(MISC)/$i)

EXTRA_MOZ_TARGET+= $(XPIDL_TYPELIB_MODULE)
.ENDIF

ALLMOZ:=$(EXTRA_MOZ_TARGET) $(DO_XPIDL) $(GEN_PRCPUCFG) $(EXPORT_HEADER)

$(INCCOM)/%.h : %.idl
    $(COMMAND_ECHO)$(COPY) $< $(INCCOM)
    $(COMMAND_ECHO)$(XPIDL) -m header -w -I $(SOLARIDLDIR)/mozilla -I$(INCCOM) -I . -o $(INCCOM)/$* $<

$(MISC)/%.xpt : %.idl
    $(COMMAND_ECHO)$(XPIDL) -m typelib -w -I $(SOLARIDLDIR)/mozilla -I$(INCCOM) -I . -o $(MISC)/$* $<

.INCLUDE : target.mk

.IF "$(GUI)"=="WNT"
CPUCFG=_winnt.cfg
.ENDIF

.IF "$(OS)"=="SOLARIS"
CPUCFG=_solaris32.cfg
.ENDIF

.IF "$(OS)"=="LINUX"
CPUCFG=_linux.cfg
.ENDIF

.IF "$(OS)"=="FREEBSD"
CPUCFG=_freebsd.cfg
.ENDIF

.IF "$(OS)"=="NETBSD"
CPUCFG=_netbsd.cfg
.ENDIF

.IF "$(GEN_PRCPUCFG)"!=""
$(GEN_PRCPUCFG): $(PRJ)/pr/include/md/$(CPUCFG) 
    @$(COPY) $(PRJ)/pr/include/md/$(CPUCFG) $@
.ENDIF

.IF "$(EXPORTS)"!=""
$(EXPORT_HEADER):
    @$(COPY) $(EXPORTS) $(INCCOM)
.ENDIF

.IF "$(XPIDLSRCS)"!=""
$(XPIDL_TYPELIB_MODULE): $(foreach,i,$(XPIDLXPT) $(MISC)/$i)
    $(COMMAND_ECHO)$(XPIDL_LINK) $(MISC)/$(XPIDL_MODULE).xpt $^
    @-mkdir $(BIN)/components
    @$(COPY) $@ $(BIN)/components
.ENDIF
