#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: kz $ $Date: 2003-08-25 15:08:34 $
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

PRJNAME=bib
TARGET=bib
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE
IMGLST_SRS=$(SRS)$/bib.srs

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

CXXFILES=			bibconfig.cxx\
                    bibcont.cxx \
                    bibload.cxx \
                    bibmod.cxx  \
                    general.cxx	 \
                    framectr.cxx \
                    bibview.cxx \
                    bibbeam.cxx  \
                    toolbar.cxx	 \
                    datman.cxx	 \


SLOFILES=			$(SLO)$/formcontrolcontainer.obj \
                    $(SLO)$/loadlisteneradapter.obj \
                    $(SLO)$/bibconfig.obj \
                    $(SLO)$/bibcont.obj \
                    $(SLO)$/bibload.obj \
                    $(SLO)$/bibmod.obj \
                    $(SLO)$/general.obj \
                    $(SLO)$/framectr.obj \
                    $(SLO)$/bibview.obj \
                    $(SLO)$/bibbeam.obj  \
                    $(SLO)$/toolbar.obj  \
                    $(SLO)$/datman.obj	\


SRS1NAME=$(TARGET)
SRC1FILES=			bib.src	 \
                    datman.src \
                    sections.src \
                    menu.src	 \
                    toolbar.src

RESLIB1NAME=bib
RESLIB1SRSFILES= $(SRS)$/bib.srs
RESLIB1DEPN= bib.src bib.hrc


SHL1STDLIBS= \
        $(SVTOOLLIB) \
        $(TKLIB) \
        $(VCLLIB) \
        $(SVLLIB) \
        $(UNOTOOLSLIB) \
        $(TOOLSLIB) \
        $(DBTOOLSLIB) \
        $(COMPHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB)


#Create UNO Header files
UNOUCROUT=$(OUT)$/inc$/$(PRJNAME)
INCPRE+=$(UNOUCROUT)


UNOTYPES=\
    com.sun.star.awt.PosSize \
    com.sun.star.awt.XPointer \
    com.sun.star.awt.XControl \
    com.sun.star.awt.XControlContainer \
    com.sun.star.awt.XControlModel \
    com.sun.star.awt.XWindowPeer \
    com.sun.star.beans.PropertyAttribute \
    com.sun.star.beans.PropertyValue \
    com.sun.star.beans.PropertyValues \
    com.sun.star.form.XBoundComponent \
    com.sun.star.form.XLoadable \
    com.sun.star.form.XGridColumnFactory \
    com.sun.star.form.ListSourceType		\
    com.sun.star.form.XForm		\
    com.sun.star.form.XFormController		\
    com.sun.star.frame.XComponentLoader \
    com.sun.star.frame.XDispatchProvider \
    com.sun.star.frame.XFilterDetect\
    com.sun.star.frame.FrameSearchFlag	\
    com.sun.star.io.XPersistObject \
    com.sun.star.io.XDataInputStream \
    com.sun.star.io.XActiveDataSource \
    com.sun.star.io.XMarkableStream \
    com.sun.star.io.XActiveDataSink \
    com.sun.star.lang.XLocalizable \
    com.sun.star.lang.XServiceName \
    com.sun.star.sdb.CommandType\
    com.sun.star.sdb.XDatabaseEnvironment \
    com.sun.star.sdb.XColumn\
    com.sun.star.sdb.XSQLQueryComposerFactory\
    com.sun.star.sdbc.DataType \
    com.sun.star.sdbc.ResultSetType \
    com.sun.star.sdbc.ResultSetConcurrency \
    com.sun.star.sdbc.XConnection \
    com.sun.star.sdbc.XDataSource \
    com.sun.star.sdbc.XResultSetUpdate \
    com.sun.star.sdbcx.XTablesSupplier \
    com.sun.star.sdbcx.XColumnsSupplier \
    com.sun.star.sdbcx.XRowLocate \
    com.sun.star.sdb.XSQLQueryComposer\
    com.sun.star.sdbc.XResultSet \
    com.sun.star.sdbc.ResultSetType \
    com.sun.star.sdbc.XRowSet\
    com.sun.star.text.BibliographyDataField \
    com.sun.star.uno.XNamingService\
    com.sun.star.util.XLocalizedAliases\
    com.sun.star.util.XURLTransformer

#END

SHL1TARGET= $(TARGET)$(UPD)$(DLLPOSTFIX)
SHL1LIBS=       $(SLB)$/$(TARGET).lib
SHL1IMPLIB=		i$(TARGET)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
SHL1VERSIONMAP=exports.map
DEF1NAME=		$(SHL1TARGET)

ALL:\
    $(do_build) \
    ALLTAR

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

