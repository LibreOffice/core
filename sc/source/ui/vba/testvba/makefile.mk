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
# $Revision: 1.3 $
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

PRJ=..$/..$/..$/..$/
PRJNAME=sc
TARGET=testvba

.INCLUDE : ant.mk

.IF "$(SOLAR_JAVA)"!=""

ALLTAR : PROCESSRESULTS

TESTDOCUMENTS=..$/TestDocuments
.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
TESTDOCUMENTLOGS=$(TESTDOCUMENTS)$/logs$/unix
.ELSE
TESTDOCUMENTLOGS=$(TESTDOCUMENTS)$/logs$/win
.ENDIF

OUTPUTDIR:=..$/$(TARGET)$/Logs
ANT_FLAGS+=-Dtest.documents=$(TESTDOCUMENTS)
ANT_FLAGS+=-Dtest.out=$(OUTPUTDIR)
ANT_FLAGS+=-Dtest.officepath=$(OFFICEPATH)
#UNITTEST : $(LOCAL_COMMON_OUT)$/class/TestVBA.class
PROCESSRESULTS : ANTBUILD
    $(PERL) testResults.pl  $(OUTPUTDIR) $(TESTDOCUMENTLOGS)

.ENDIF
