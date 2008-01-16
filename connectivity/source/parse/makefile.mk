#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.21 $
#
#   last change: $Author: ihi $ $Date: 2008-01-16 14:21:34 $
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

PRJ=..$/..

PRJNAME=connectivity
TARGET=sql
INCPRE=$(MISC)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

YACCTARGET= $(MISC)$/sqlbison.cxx
YACCFILES= sqlbison.y
YACCFLAGS=-v -d -l -pSQLyy -bsql

EXCEPTIONSFILES =  \
        $(SLO)$/PColumn.obj			\
        $(SLO)$/sqliterator.obj		\
        $(SLO)$/sqlnode.obj         \
        $(SLO)$/wrap_sqlbison.obj		\
        $(SLO)$/internalnode.obj

SLOFILES =  \
        $(EXCEPTIONSFILES)			\
        $(SLO)$/wrap_sqlflex.obj


# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/%.cxx:	%.l
    flex -i -8 -PSQLyy -L -o$(MISC)$/sqlflex.cxx sqlflex.l

$(INCCOM)$/sqlbison.hxx : $(YACCTARGET)
$(EXCEPTIONSFILES) : $(INCCOM)$/sqlbison.hxx
$(SLO)$/wrap_sqlbison.obj : $(YACCTARGET)
$(SLO)$/wrap_sqlflex.obj : $(MISC)$/sqlflex.cxx $(INCCOM)$/sqlbison.hxx
