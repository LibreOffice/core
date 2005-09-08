#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 15:39:00 $
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

PRJNAME=svtools
TARGET=filter
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/svt.pmk

# --- UNOTypes -----------------------------------------------------

UNOUCROUT=$(OUT)$/inc$/$(PRJNAME)$/$(TARGET)
INCPRE+=$(UNOUCROUT)
UNOTYPES=	\
            com.sun.star.svg.XSVGWriter							

#			com.sun.star.beans.PropertyValue					\
#			com.sun.star.beans.XPropertyAccess					\
#			com.sun.star.beans.XPropertySetInfo					\
#			com.sun.star.container.XHierarchicalNameAccess		\
#			com.sun.star.container.XIndexAccess					\
#			com.sun.star.container.XNameAccess					\
#			com.sun.star.document.XViewDataSupplier				\
#			com.sun.star.document.XExporter						\
#			com.sun.star.frame.XModel							\
#			com.sun.star.io.XActiveDataSource					\
#			com.sun.star.io.XOutputStream						\
#			com.sun.star.lang.XInitialization					\
#			com.sun.star.lang.XMultiServiceFactory				\
#			com.sun.star.lang.XServiceInfo						\
#			com.sun.star.lang.XTypeProvider						\
#			com.sun.star.ui.dialogs.XExecutableDialog			\
#			com.sun.star.ui.dialogs.ExecutableDialogResults		\
#			com.sun.star.uno.XAggregation						\
#			com.sun.star.uno.XInterface							\
#			com.sun.star.uno.XWeak								\
#			com.sun.star.util.XChangesBatch						\
#			com.sun.star.xml.sax.XDocumentHandler

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES= strings.src			\
          dlgexpor.src			\
          dlgepng.src			\
          dlgejpg.src

SLOFILES= $(SLO)$/filter.obj				\
          $(SLO)$/filter2.obj				\
          $(SLO)$/dlgexpor.obj				\
          $(SLO)$/dlgejpg.obj				\
          $(SLO)$/dlgepng.obj				\
          $(SLO)$/sgfbram.obj				\
          $(SLO)$/sgvmain.obj				\
          $(SLO)$/sgvtext.obj				\
          $(SLO)$/sgvspln.obj				\
          $(SLO)$/FilterConfigItem.obj		\
          $(SLO)$/FilterConfigCache.obj		\
          $(SLO)$/SvFilterOptionsDialog.obj

EXCEPTIONSNOOPTFILES=	$(SLO)$/filter.obj				\
                        $(SLO)$/FilterConfigItem.obj	\
                        $(SLO)$/FilterConfigCache.obj	\
                        $(SLO)$/SvFilterOptionsDialog.obj

LIB1TARGET=		$(SLB)$/$(TARGET).uno.lib
LIB1OBJFILES=	\
          $(SLO)$/dlgexpor.obj				\
          $(SLO)$/dlgejpg.obj				\
          $(SLO)$/dlgepng.obj				\
    $(SLO)$/SvFilterOptionsDialog.obj

LIB2TARGET=		$(SLB)$/$(TARGET).lib
LIB2OBJFILES=	\
    $(SLO)$/filter.obj				\
    $(SLO)$/filter2.obj				\
          $(SLO)$/sgfbram.obj				\
          $(SLO)$/sgvmain.obj				\
          $(SLO)$/sgvtext.obj				\
          $(SLO)$/sgvspln.obj				\
          $(SLO)$/FilterConfigItem.obj		\
          $(SLO)$/FilterConfigCache.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

