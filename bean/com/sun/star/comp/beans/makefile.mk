#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 22:03:34 $
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

PRJ	= ..$/..$/..$/..$/..
PRJNAME = beans
TARGET  = com_sun_star_comp_beans
PACKAGE = com$/sun$/star$/comp$/beans

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk

.IF "$(OS)"=="MACOSX"

dummy:
    @echo "Nothing to build for OS $(OS)"

.ELSE		# "$(OS)"=="MACOSX"

JARFILES 		= sandbox.jar ridl.jar unoil.jar jurt.jar juh.jar

# --- Sources --------------------------------------------------------

JAVAFILES=  \
    ContainerFactory.java \
    Controller.java \
    Frame.java \
    HasConnectionException.java \
    InvalidArgumentException.java \
    JavaWindowPeerFake.java \
    LocalOfficeConnection.java \
    LocalOfficeWindow.java \
    NativeConnection.java \
    NativeService.java \
    NoConnectionException.java \
    NoDocumentException.java \
    OfficeConnection.java \
    OfficeDocument.java \
    OfficeWindow.java \
    OOoBean.java \
    SystemWindowException.java \
    Wrapper.java \
    CallWatchThread.java

JAVACLASSFILES= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
