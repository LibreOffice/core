#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: vg $ $Date: 2007-12-07 11:15:40 $
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
