#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: hr $ $Date: 2004-08-02 14:07:42 $
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
PRJ=..$/..$/..$/..$/..$/..

PRJNAME=officecfg
TARGET=schema

.INCLUDE :  settings.mk

# --- Targets ------------------------------------------------------

PACKAGE=org.openoffice.Office.UI

XCSFILES= \
    Controller.xcs \
    Factories.xcs \
    Commands.xcs \
    BasicIDECommands.xcs \
    BibliographyCommands.xcs \
    CalcCommands.xcs \
    ChartCommands.xcs \
    DbuCommands.xcs \
    DrawImpressCommands.xcs \
    GenericCommands.xcs \
    MathCommands.xcs \
    StartModuleCommands.xcs \
    WriterCommands.xcs       \
    WindowState.xcs \
    BasicIDEWindowState.xcs \
    BibliographyWindowState.xcs \
    CalcWindowState.xcs \
    ChartWindowState.xcs \
    DbQueryWindowState.xcs \
    DbRelationWindowState.xcs \
    DbTableWindowState.xcs \
    DrawWindowState.xcs \
    ImpressWindowState.xcs \
    MathWindowState.xcs \
    StartModuleWindowState.xcs \
    WriterWindowState.xcs \
    WriterGlobalWindowState.xcs \
    WriterWebWindowState.xcs \
    Effects.xcs

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/registry$/schema$/localtarget.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk
