#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 16:59:06 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..

PRJNAME=SVTOOLS
TARGET=svsql

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------

UNOUCRDEP=$(SOLARBINDIR)$/applicat.rdb
UNOUCRRDB=$(SOLARBINDIR)$/applicat.rdb

# Every source directory generates headers in a own output directory to
# enable parallel building of the source directories within a project!
UNOUCROUT=$(OUT)$/inc$/$(PRJNAME)$/$(TARGET)
INCPRE+=$(UNOUCROUT)

UNOTYPES = 	com.sun.star.util.XNumberFormatsSupplier \
        com.sun.star.util.XNumberFormatter \
        com.sun.star.util.XNumberFormatTypes \
        com.sun.star.util.Date \
        com.sun.star.data.DatabaseFieldType \
        com.sun.star.data.DatabaseInfo \
        com.sun.star.data.XDatabaseDescriptor \
        com.sun.star.sdbc.XDatabaseMetaData \
        com.sun.star.lang.Locale \
        com.sun.star.beans.XPropertySet \
        com.sun.star.uno.TypeClass

SRCFILES = svsql.src

SLOFILES =  \
        $(SLO)$/sdbbison.obj		\
        $(SLO)$/sdbflex.obj			\
        $(SLO)$/sdbsql.obj \
        $(SLO)$/sdbnode.obj \
        $(SLO)$/converter.obj

EXCEPTIONSFILES=	\
        $(SLO)$/sdbbison.obj		\
        $(SLO)$/sdbsql.obj

all: \
        $(MISC)$/sdbflex.cxx \
        $(MISC)$/sdbbison.cxx \
        ALLTAR

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/sdbflex.cxx:	sdblex.l
    +flex -i -8 -PSQLyy -L -o$(MISC)$/sdbflex.cxx sdblex.l

$(MISC)$/sdbbison.cxx: sdbyacc.y
    +bison -v -d -pSQLyy -l -bsql -o$(MISC)$/sdbbison.cxx sdbyacc.y
    -$(MKDIR) $(OUT)$/inc$/svtools
    +$(COPY) $(MISC)$/sdbbison.cxx.h $(OUT)$/inc$/svtools$/sdbbison.hxx
