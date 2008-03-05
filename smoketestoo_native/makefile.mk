#***********************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 16:17:29 $
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

