#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kso $ $Date: 2000-10-11 07:55:31 $
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

PROJECTPCH=items
PROJECTPCHSOURCE=items

PRJNAME=SVTOOLS
TARGET=items
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

UNOUCRDEP=$(SOLARBINDIR)$/applicat.rdb
UNOUCRRDB=$(SOLARBINDIR)$/applicat.rdb
# Every source directory generates headers in a own output directory to
# enable parallel building of the source directories within a project!
UNOUCROUT=$(OUT)$/inc$/$(PRJNAME)$/$(TARGET)
INCPRE+=$(UNOUCROUT)

UNOTYPES=\
    com.sun.star.awt.Point \
    com.sun.star.awt.Rectangle \
    com.sun.star.awt.Size \
    com.sun.star.beans.PropertyAttribute \
    com.sun.star.beans.PropertyState \
    com.sun.star.beans.XPropertySetInfo \
    com.sun.star.container.XNameAccess \
    com.sun.star.container.XNameContainer \
    com.sun.star.io.XActiveDataControl \
    com.sun.star.io.XActiveDataSource \
    com.sun.star.io.XOutputStream \
    com.sun.star.lang.IllegalArgumentException \
    com.sun.star.lang.XMultiServiceFactory\
    com.sun.star.lang.XTypeProvider\
    com.sun.star.lang.XServiceInfo\
    com.sun.star.lang.XUnoTunnel\
    com.sun.star.ucb.TransferResult \
    com.sun.star.uno.TypeClass \
    com.sun.star.uno.XAggregation \
    com.sun.star.uno.XInterface \
    com.sun.star.uno.XWeak \
    com.sun.star.util.DateTime \
    com.sun.star.util.DateTimeRange

# --- Files --------------------------------------------------------

.IF "$(header)" == ""

LIB2TARGET=$(SLB)$/items1.lib
LIB2OBJFILES=\
    $(SLO)$/bintitem.obj	\
    $(SLO)$/cenumitm.obj	\
    $(SLO)$/cintitem.obj	\
    $(SLO)$/cntwall.obj	\
    $(SLO)$/cstitem.obj	\
    $(SLO)$/ctypeitm.obj	\
    $(SLO)$/custritm.obj	\
    $(SLO)$/dateitem.obj	\
    $(SLO)$/dtritem.obj	\
    $(SLO)$/frqitem.obj	\
    $(SLO)$/itemiter.obj	\
    $(SLO)$/itempool.obj	\
    $(SLO)$/itemprop.obj	\
    $(SLO)$/itemset.obj	\
    $(SLO)$/lckbitem.obj	\
    $(SLO)$/poolio.obj	\
    $(SLO)$/poolitem.obj	\
    $(SLO)$/sfontitm.obj	\
    $(SLO)$/sitem.obj	    \
    $(SLO)$/slstitm.obj	\
    $(SLO)$/tfrmitem.obj	\
    $(SLO)$/tresitem.obj	\
    $(SLO)$/whiter.obj

LIB3TARGET=$(SLB)$/items2.lib
LIB3OBJFILES=\
    $(SLO)$/aeitem.obj	\
    $(SLO)$/args.obj	\
    $(SLO)$/compat.obj	\
    $(SLO)$/eitem.obj	\
    $(SLO)$/flagitem.obj	\
    $(SLO)$/intitem.obj	\
    $(SLO)$/itemdel.obj	\
    $(SLO)$/macitem.obj	\
    $(SLO)$/poolcach.obj	\
    $(SLO)$/ptitem.obj	\
    $(SLO)$/rectitem.obj	\
    $(SLO)$/rngitem.obj	\
    $(SLO)$/stritem.obj	\
    $(SLO)$/style.obj	\
    $(SLO)$/szitem.obj	\
    $(SLO)$/wallitem.obj

SLOFILES=\
    $(LIB2OBJFILES)	\
    $(LIB3OBJFILES)

.ENDIF

SRCFILES=\
    cstitem.src

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
