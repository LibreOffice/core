#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: fs $ $Date: 2001-03-21 15:43:21 $
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

PRJNAME=pcr
TARGET=pcr
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

#Create UNO Header files
UNOUCROUT=$(OUT)$/inc$/$(PRJNAME)
INCPRE+=$(UNOUCROUT)

UNOUCRDEP=	$(SOLARBINDIR)$/applicat.rdb
UNOUCRRDB=	$(SOLARBINDIR)$/applicat.rdb

UNOTYPES=\
    com.sun.star.frame.XController				\
    com.sun.star.frame.XFrame					\
    com.sun.star.beans.XPropertyState			\
    com.sun.star.beans.XPropertySet				\
    com.sun.star.beans.XMultiPropertySet		\
    com.sun.star.beans.XFastPropertySet			\
    com.sun.star.beans.XIntrospectionAccess		\
    com.sun.star.beans.XIntrospection			\
    com.sun.star.beans.Property					\
    com.sun.star.beans.PropertyConcept			\
    com.sun.star.beans.PropertyAttribute		\
    com.sun.star.form.ListSourceType			\
    com.sun.star.form.XForm						\
    com.sun.star.form.XFormController			\
    com.sun.star.form.XGridColumnFactory		\
    com.sun.star.form.FormComponentType			\
    com.sun.star.script.XEventAttacherManager	\
    com.sun.star.lang.XInitialization			\
    com.sun.star.lang.Locale					\
    com.sun.star.lang.XMultiServiceFactory		\
    com.sun.star.container.XChild				\
    com.sun.star.i18n.NumberFormatCode			\
    com.sun.star.i18n.XLocaleData				\
    com.sun.star.i18n.LocaleItem				\
    com.sun.star.i18n.reservedWords				\
    com.sun.star.util.XTextSearch				\
    com.sun.star.util.XNumberFormatsSupplier	\
    com.sun.star.util.XNumberFormatTypes		\
    com.sun.star.lang.XUnoTunnel				\
    com.sun.star.util.URL						\
    com.sun.star.connection.XConnection			\
    com.sun.star.lang.Locale					\
    com.sun.star.sdbc.XResultSet				\
    com.sun.star.sdbc.XRowSet					\
    com.sun.star.sdbc.XPreparedStatement		\
    com.sun.star.sdbc.XDataSource				\
    com.sun.star.script.XTypeConverter			\
    com.sun.star.sdbcx.XTablesSupplier			\
    com.sun.star.sdbcx.XColumnsSupplier			\
    com.sun.star.sdb.XQueriesSupplier			\
    com.sun.star.awt.FontWeight					\
    com.sun.star.awt.FontUnderline				\
    com.sun.star.awt.FontStrikeout				\
    com.sun.star.awt.FontSlant					\
    com.sun.star.uno.XNamingService				\
    com.sun.star.frame.XController				\
    com.sun.star.awt.XVclWindowPeer				\
    com.sun.star.awt.XLayoutConstrains			\
    com.sun.star.awt.XView						\
    com.sun.star.awt.XPointer					\
    com.sun.star.awt.XGraphics					\
    com.sun.star.awt.XVclContainerListener		\
    com.sun.star.awt.XKeyListener				\
    com.sun.star.awt.XMouseListener				\
    com.sun.star.awt.XMouseMotionListener		\
    com.sun.star.awt.XPaintListener				\
    com.sun.star.awt.XTopWindowListener			\
    com.sun.star.awt.XTextListener				\
    com.sun.star.awt.XActionListener			\
    com.sun.star.awt.XItemListener				\
    com.sun.star.container.XContainerListener	\
    com.sun.star.awt.XSpinListener				\
    com.sun.star.awt.XAdjustmentListener		\
    com.sun.star.awt.XMenuListener				\


CXXFILES=			fontdialog.cxx	\
                    pcrservices.cxx	\
                    browserline.cxx	\
                    selectlabeldialog.cxx	\
                    formcontroller.cxx	\
                    formmetadata.cxx	\
                    formbrowsertools.cxx	\
                    standardcontrol.cxx	\
                    usercontrol.cxx	\
                    commoncontrol.cxx	\
                    browserpage.cxx	\
                    browserlistbox.cxx	\
                    propertyeditor.cxx	\
                    formstrings.cxx	\
                    pcrstrings.cxx	\
                    browserview.cxx	\
                    propcontroller.cxx	\
                    pcrcommon.cxx	\
                    modulepcr.cxx

SLOFILES=			$(SLO)$/fontdialog.obj	\
                    $(SLO)$/pcrservices.obj	\
                    $(SLO)$/browserline.obj	\
                    $(SLO)$/selectlabeldialog.obj	\
                    $(SLO)$/formcontroller.obj	\
                    $(SLO)$/formmetadata.obj	\
                    $(SLO)$/formbrowsertools.obj	\
                    $(SLO)$/standardcontrol.obj	\
                    $(SLO)$/usercontrol.obj	\
                    $(SLO)$/commoncontrol.obj	\
                    $(SLO)$/browserpage.obj	\
                    $(SLO)$/browserlistbox.obj	\
                    $(SLO)$/propertyeditor.obj	\
                    $(SLO)$/formstrings.obj	\
                    $(SLO)$/pcrstrings.obj	\
                    $(SLO)$/browserview.obj	\
                    $(SLO)$/propcontroller.obj	\
                    $(SLO)$/pcrcommon.obj	\
                    $(SLO)$/modulepcr.obj


SRCFILES=			propres.src	 \
                    formres.src

RESLIB1NAME=pcr
RESLIB1SRSFILES= $(SRS)/pcr.srs
#RESLIB1DEPN= pcr.src pcr.hrc

IMGLST_SRS=$(SRS)$/pcr.srs

SHL1TARGET= $(TARGET)$(UPD)$(DLLPOSTFIX)
SHL1VERSIONMAP= $(TARGET).map

SHL1STDLIBS= \
        $(CPPULIB) 	 \
        $(CPPUHELPERLIB) 	 \
        $(COMPHELPERLIB) 	 \
        $(VOSLIB) 	 \
        $(TOOLSLIB)  \
        $(SO2LIB)	\
        $(VCLLIB)    \
        $(TKLIB)	\
        $(SVTOOLLIB)\
        $(SVLLIB)	\
        $(OSLLIB)	 \
        $(SALLIB)	\
        $(UNOTOOLSLIB)	\
        $(SOTLIB)	\
        $(UCBHELPERLIB)	\
        $(SVXLIB)	\
        $(SFXLIB)	\
        $(DBTOOLSLIB)	\
        $(OFALIB)


SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1IMPLIB=		i$(TARGET)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)
#DEF1DEPN=		$(MISC)$/$(SHL1TARGET).flt
#DEFLIB1NAME=	$(TARGET)
DEF1EXPORTFILE=	exports.dxp



# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

#$(MISC)$/$(SHL1TARGET).flt: makefile.mk
#	@echo ------------------------------
#	@echo Making: $@
#   @echo _Impl		>$@
#	@echo _real		>>$@
#	@echo _TI1		>>$@
#	@echo _TI2		>>$@
#	@echo WEP		>>$@
#	@echo LIBMAIN	>>$@
#	@echo LibMain	>>$@

