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

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ=..

PRJNAME=binfilter
TARGET=bf_util

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

# --- Files -------------------------------------
.IF "$(L10N_framework)"==""
.IF "$(GUI)"=="UNX"
LOCALLIBDIR=$(LB)
.ELSE          # "$(GUI)"=="UNX"
LOCALLIBDIR=$(BIN)
.ENDIF          # "$(GUI)"=="UNX"

RDBNAMES=		\
    bf_xo		\
    bf_sw		\
    bf_sc		\
    bf_sd		\
    bf_sm		\
    bf_sch		\
    bf_frm		\
    bf_svx		\
    bf_svt		\
    bf_wrapper

RDBLIBS=$(foreach,i,$(strip $(RDBNAMES)) $(LOCALLIBDIR)$/$(DLLPRE)$i$(DLLPOSTFIX)$(DLLPOST))
.ENDIF

# --- Targets ----------------------------------

.INCLUDE : target.mk
.IF "$(L10N_framework)"==""
ALLTAR : $(BIN)$/legacy_binfilters.rdb

.IF "$(OS)" == "WNT"
my_OOO_BIN_PATH = BRAND_BASE_DIR
.ELSE
my_OOO_BIN_PATH = OOO_BASE_DIR
.ENDIF

$(BIN)$/legacy_binfilters.rdb : $(RDBLIBS)
    @@-$(RM) $@
    cd $(LOCALLIBDIR) && $(REGCOMP) -register -r ..$/bin$/$(@:f) -wop=vnd.sun.star.expand:$(EMQ)$$$(my_OOO_BIN_PATH)/program/ $(foreach,i,$(RDBLIBS) -c $(subst,$(LOCALLIBDIR)$/,./ $i))

.ELSE
pseudo:

.ENDIF
