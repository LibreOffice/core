#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1 $
#
#   last change: $Author: mi $ $Date: 2004-09-06 15:11:38 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., September, 2004
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2004 by Sun Microsystems, Inc.
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
#   Copyright: 2004 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ	= ..$/..$/..$/..$/..
PRJNAME = beans
TARGET  = officebean
PACKAGE = com$/sun$/star$/comc$/beans

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk

.IF "$(OS)"=="MACOSX"

dummy:
    @echo "Nothing to build for OS $(OS)"

.ELSE		# "$(OS)"=="MACOSX"

JARFILES 		= sandbox.jar ridl.jar unoil.jar jurt.jar juh.jar
JAVAFILES		= $(subst,$(CLASSDIR)$/$(PACKAGE)$/, $(subst,.class,.java $(JAVACLASSFILES)))

JARCLASSDIRS	= $(PACKAGE)
JARTARGET		= $(TARGET).jar
JARCOMPRESS 	= TRUE

# --- Files --------------------------------------------------------

JAVACLASSFILES=\
    $(CLASSDIR)$/$(PACKAGE)$/OfficeConnection.class \
    $(CLASSDIR)$/$(PACKAGE)$/OfficeWindow.class \
    $(CLASSDIR)$/$(PACKAGE)$/ContainerFactory.class \
    $(CLASSDIR)$/$(PACKAGE)$/JavaWindowPeerFake.class \
    $(CLASSDIR)$/$(PACKAGE)$/NativeConnection.class \
    $(CLASSDIR)$/$(PACKAGE)$/NativeService.class \
    $(CLASSDIR)$/$(PACKAGE)$/LocalOfficeConnection.class \
    $(CLASSDIR)$/$(PACKAGE)$/LocalOfficeWindow.class \
    $(CLASSDIR)$/$(PACKAGE)$/OutputStreamToXOutputStreamAdapter.class \
    $(CLASSDIR)$/$(PACKAGE)$/InputStreamToXInputStreamAdapter.class \
    $(CLASSDIR)$/$(PACKAGE)$/XOutputStreamToByteArrayAdapter.class \
    $(CLASSDIR)$/$(PACKAGE)$/ByteArrayToXInputStreamAdapter.class \
    $(CLASSDIR)$/$(PACKAGE)$/InvalidArgumentException.class \
    $(CLASSDIR)$/$(PACKAGE)$/HasConnectionException.class \
    $(CLASSDIR)$/$(PACKAGE)$/NoConnectionException.class \
    $(CLASSDIR)$/$(PACKAGE)$/SystemWindowException.class \
    $(CLASSDIR)$/$(PACKAGE)$/NoDocumentException.class \
    $(CLASSDIR)$/$(PACKAGE)$/OfficeDocument.class \
    $(CLASSDIR)$/$(PACKAGE)$/Controller.class \
    $(CLASSDIR)$/$(PACKAGE)$/Frame.class \
    $(CLASSDIR)$/$(PACKAGE)$/Wrapper.class \
    $(CLASSDIR)$/$(PACKAGE)$/OOoBeanInterface.class \
    $(CLASSDIR)$/$(PACKAGE)$/OOoBean.class 

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
