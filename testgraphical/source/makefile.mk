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

PRJNAME=gfxcmp
TARGET=notargetyet

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

# call with PDF=1 to use office pdf exporter instead of the XPrinter API
.IF "$(PDF)"!=""
    CREATORTYPE="-creatortype" pdf
.ELSE
    CREATORTYE="-creatortype" ps
.ENDIF

.IF "$(SHOW)"!=""
    P_SHOW=-show
.ENDIF


# call with PREPARE=1 to only create new reference files
# copy these files by hand into the corresponding directories
.IF "$(PREPARE)"!=""
    PREPAREONLY="-prepareonly" 1
.ELSE
    PREPAREONLY=
.ENDIF

.IF "$(DOCUMENTPOOL)"==""
    DOCUMENTPOOL=$PRJ$/document-pool
.ENDIF

# PERLDEBUG=-d:ptkdb
ALLTAR: selftest
#	pwd
# $(PERL) $(PERLDEBUG) compare.pl -MAJOR $(WORK_STAMP) -MINOR $(UPDMINOR) -cwsname "$(CWS_WORK_STAMP)"
#	$(PERL) $(PERLDEBUG) compare.pl -pool singletest

# $(PRJ)$/util$/makefile.pmk contains ALLTAR stuff


selftest:
    $(PERL) $(PERLDEBUG) compare.pl -creatortype pdf $(PREPAREONLY) -pool singletest -document eis-test.odt $(P_SHOW)

#
#
# The follows are demonstration targets, DO NOT DELETE
#
#

demo:
    $(PERL) $(PERLDEBUG) compare.pl $(CREATORTYPE) $(PREPAREONLY) -pool $@ $(P_SHOW)

# this test will most the time fail.
failtest:
    $(PERL) $(PERLDEBUG) compare.pl $(CREATORTYPE) $(PREPAREONLY) -force -pool demo -document CurrentTime.ods $(P_SHOW)

manual:
    $(PERL) $(PERLDEBUG) compare.pl $(CREATORTYPE) $(PREPAREONLY) -force $(P_SHOW)

# msoffice:
# 	$(PERL) $(PERLDEBUG) compare.pl -creatortype msoffice $(PREPAREONLY) -pool msoffice -document calc_cellformat_import_biff8.xls $(P_SHOW)

clean:
