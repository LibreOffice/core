#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.19 $
#
#   last change: $Author: obo $ $Date: 2008-03-25 15:21:38 $
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

PRJ=..$/..$/..
PRJNAME=xmlhelp
TARGET=chelp

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE


# --- Settings ---------------------------------------------------------

.INCLUDE: settings.mk

CFLAGS +=  -DHAVE_EXPAT_H

.IF "$(SYSTEM_DB)" == "YES"
CFLAGS+=-DSYSTEM_DB -I$(DB_INCLUDES)
.ENDIF

.IF "$(SYSTEM_LIBXML)" == "YES"
CFLAGS+= $(LIBXML_CFLAGS)
.ELSE
LIBXMLINCDIR=external$/libxml
CFLAGS+= -I$(SOLARINCDIR)$/$(LIBXMLINCDIR)
.ENDIF

.IF "$(SYSTEM_LIBXSLT)" == "YES"
CFLAGS+= $(LIBXSLT_CFLAGS)
.ELSE
LIBXSLTINCDIR=external$/libxslt
CFLAGS+= -I$(SOLARINCDIR)$/$(LIBXSLTINCDIR)
.ENDIF

.IF "$(GUI)"=="WNT"
CFLAGS+=-GR
.ENDIF

# --- General -----------------------------------------------------

SLOFILES=\
    $(SLO)$/services.obj    	   \
    $(SLO)$/provider.obj    	   \
    $(SLO)$/content.obj     	   \
    $(SLO)$/resultset.obj     	   \
    $(SLO)$/resultsetbase.obj      \
    $(SLO)$/resultsetforroot.obj   \
    $(SLO)$/resultsetforquery.obj  \
    $(SLO)$/contentcaps.obj        \
    $(SLO)$/urlparameter.obj       \
    $(SLO)$/inputstream.obj        \
    $(SLO)$/databases.obj          \
    $(SLO)$/db.obj		           \
    $(SLO)$/bufferedinputstream.obj

# --- Targets ----------------------------------------------------------

.INCLUDE: target.mk

