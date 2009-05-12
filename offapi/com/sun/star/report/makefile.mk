#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.3 $
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

PRJ=..$/..$/..$/..

PRJNAME=offapi

TARGET=cssreport
PACKAGE=com$/sun$/star$/report

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
        modules.idl\
    XReportDefinition.idl	\
    XReportComponent.idl	\
    XReportControlModel.idl	\
    XGroups.idl				\
    XGroup.idl				\
    XFormattedField.idl		\
    XFixedText.idl			\
    XImageControl.idl		\
    XSection.idl			\
    ReportPrintOption.idl	\
    GroupKeepTogether.idl	\
    ForceNewPage.idl		\
    KeepTogether.idl		\
    XReportEngine.idl		\
    XFormatCondition.idl	\
    XReportControlFormat.idl	\
    XFunction.idl			\
    XFunctions.idl			\
    XFunctionsSupplier.idl	\
    GroupOn.idl				\
    XShape.idl				\
    XFixedLine.idl


# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
