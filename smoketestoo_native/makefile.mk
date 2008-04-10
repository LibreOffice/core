#***********************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
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

PRJ=.

PRJNAME=test10
TARGET=t1

.INCLUDE : settings.mk

.IF "$(OS)$(PROEXT)"=="LINUX.pro_never"
SHL1TARGET= genstats
SLOFILES=       $(SLO)$/genstats.obj
SHL1OBJS=   $(SLOFILES)

#
# Java related 
#

JARFILES    =   jcommon-0.7.1.jar \
                jfreechart-0.9.4.jar
JARS2COPY    =  $(CLASSDIR)$/jcommon-0.7.1.jar \
                $(CLASSDIR)$/jfreechart-0.9.4.jar

JAVACLASSFILES=$(BIN)$/Graphics.class
.ENDIF

STAR_REGISTRY=
.EXPORT : STAR_REGISTRY
.EXPORT : PERL
.EXPORT : DMAKE_WORK_DIR

.INCLUDE :      target.mk   

ALLTAR : make_test


make_test:
.IF $(NOREMOVE)
    @$(PERL) smoketest.pl -nr $(LAST_MINOR)
.ELSE
    @$(PERL) smoketest.pl $(LAST_MINOR)
.ENDIF

noremove:
    @$(PERL) smoketest.pl -nr $(LAST_MINOR)

