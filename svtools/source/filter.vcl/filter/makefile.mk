#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: sj $ $Date: 2001-02-28 13:47:07 $
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

PRJ=..$/..$/..

PRJNAME=SVTOOLS
TARGET=filter
DEPTARGET=vfilter
VERSION=$(UPD)

# --- Settings -----------------------------------------------------

.IF "$(VCL)" != ""

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

.IF "$(GUI)"=="WIN"
LINKFLAGS=$(LINKFLAGS) /PACKC:32768
.ENDIF

# --- UNOTypes -----------------------------------------------------

UNOUCRDEP=$(SOLARBINDIR)$/applicat.rdb
UNOUCRRDB=$(SOLARBINDIR)$/applicat.rdb
UNOUCROUT=$(OUT)$/inc$/$(PRJNAME)$/$(TARGET)
INCPRE+=$(UNOUCROUT)
UNOTYPES=\
    com.sun.star.uno.TypeClass							\
    com.sun.star.uno.XInterface							\
    com.sun.star.uno.XWeak								\
    com.sun.star.uno.XAggregation						\
    com.sun.star.lang.XTypeProvider						\
    com.sun.star.lang.XMultiServiceFactory				\
    com.sun.star.io.XActiveDataSource					\
    com.sun.star.io.XOutputStream						\
    com.sun.star.svg.XSVGWriter							\
    com.sun.star.xml.sax.XDocumentHandler				


# --- Files --------------------------------------------------------

CXXFILES= filter.cxx          \
          filter2.cxx		  \
          dlgexpor.cxx		  \
          dlgejpg.cxx		  \
          sgfbram.cxx         \
          sgvmain.cxx         \
          sgvtext.cxx         \
          sgvspln.cxx         \
          FilterConfigItem.cxx\
          FilterConfigCache.cxx	

SRCFILES= strings.src		  \
          dlgexpor.src		  \
          dlgejpg.src

SLOFILES= $(SLO)$/filter.obj   \
          $(SLO)$/filter2.obj  \
          $(SLO)$/dlgexpor.obj \
          $(SLO)$/dlgejpg.obj  \
            $(SLO)$/sgfbram.obj  \
          $(SLO)$/sgvmain.obj  \
          $(SLO)$/sgvtext.obj  \
          $(SLO)$/sgvspln.obj  \
          $(SLO)$/FilterConfigItem.obj	\
          $(SLO)$/FilterConfigCache.obj

EXCEPTIONSNOOPTFILES=	$(SLO)$/filter.obj				\
                        $(SLO)$/FilterConfigItem.obj	\
                        $(SLO)$/FilterConfigCache.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

.ELSE

dummy:
    @+echo VCL not set. nothing to do!

.ENDIF # VCL

