#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 16:16:56 $
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

PRJNAME := extensions
#PACKAGE := com$/sun$/star$/pgp
PACKAGE := 
TARGET  := test_com_sun_star_pgp


# --- Settings -----------------------------------------------------
.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk


# Files --------------------------------------------------------


APPLICATRDB := $(SOLARBINDIR)$/applicat.rdb
RDB := $(APPLICATRDB)

JARFILES= jurt.jar

GENJAVACLASSFILES= \
    $(CLASSDIR)$/com$/sun$/star$/beans$/PropertyValue.class				\
    $(CLASSDIR)$/com$/sun$/star$/beans$/PropertyState.class				\
    $(CLASSDIR)$/com$/sun$/star$/container$/XSet.class				\


#$(CLASSDIR)$/com$/sun$/star$/awt$/FocusEvent.class

#	$(CLASSDIR)$/com$/sun$/star$/awt$/InputEvent.class					\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/KeyEvent.class					\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/MouseEvent.class					\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/WindowEvent.class					\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/PaintEvent.class					\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/Rectangle.class					\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/XFocusListener.class				\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/XKeyListener.class				\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/XMouseListener.class				\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/XMouseMotionListener.class		\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/XWindow.class						\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/XWindowListener.class				\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/XPaintListener.class				\
#	$(CLASSDIR)$/com$/sun$/star$/bridge$/XBridge.class					\
#	$(CLASSDIR)$/com$/sun$/star$/container$/XIndexAccess.class			\
#	$(CLASSDIR)$/com$/sun$/star$/frame$/FrameAction.class				\
#	$(CLASSDIR)$/com$/sun$/star$/frame$/FrameActionEvent.class			\
#	$(CLASSDIR)$/com$/sun$/star$/frame$/XComponentLoader.class			\
#	$(CLASSDIR)$/com$/sun$/star$/frame$/XController.class				\
#	$(CLASSDIR)$/com$/sun$/star$/frame$/XFrame.class					\
#	$(CLASSDIR)$/com$/sun$/star$/frame$/XFrameActionListener.class		\
#	$(CLASSDIR)$/com$/sun$/star$/frame$/XFrames.class					\
#	$(CLASSDIR)$/com$/sun$/star$/frame$/XFramesSupplier.class			\
#	$(CLASSDIR)$/com$/sun$/star$/frame$/XModel.class					\
#	$(CLASSDIR)$/com$/sun$/star$/io$/BufferSizeExceededException.class	\
#	$(CLASSDIR)$/com$/sun$/star$/io$/NotConnectedException.class		\
#	$(CLASSDIR)$/com$/sun$/star$/io$/XInputStream.class					\
#	$(CLASSDIR)$/com$/sun$/star$/io$/XOutputStream.class				\
#	$(CLASSDIR)$/com$/sun$/star$/lang$/IndexOutOfBoundsException.class	\
#	$(CLASSDIR)$/com$/sun$/star$/lang$/XComponent.class					\
#	$(CLASSDIR)$/com$/sun$/star$/lang$/XMultiServiceFactory.class		\
#	$(CLASSDIR)$/com$/sun$/star$/lang$/XServiceInfo.class				\
#	$(CLASSDIR)$/com$/sun$/star$/text$/XSimpleText.class				\
#	$(CLASSDIR)$/com$/sun$/star$/text$/XText.class						\
#	$(CLASSDIR)$/com$/sun$/star$/text$/XTextContent.class				\
#	$(CLASSDIR)$/com$/sun$/star$/text$/XTextCursor.class				\
#	$(CLASSDIR)$/com$/sun$/star$/text$/XTextDocument.class				\
#	$(CLASSDIR)$/com$/sun$/star$/text$/XTextRange.class					\
#	$(CLASSDIR)$/com$/sun$/star$/uno$/XNamingService.class		\
#	$(CLASSDIR)$/com$/sun$/star$/uno$/Enum.class


JAVACLASSFILES= \
    $(CLASSDIR)$/$(PACKAGE)$/TestPGP.class


TYPES={$(subst,.class, $(subst,$/,.  $(subst,$(CLASSDIR)$/,-T  $(GENJAVACLASSFILES))))}
GENJAVAFILES = {$(subst,.class,.java $(subst,$/class, $(GENJAVACLASSFILES)))}
JAVAFILES= $(subst,$(CLASSDIR)$/$(PACKAGE)$/, $(subst,.class,.java $(JAVACLASSFILES))) $(GENJAVAFILES) 


# --- Targets ------------------------------------------------------


.IF "$(depend)" == ""
ALL : $(GENJAVAFILES) ALLTAR 
.ELSE
ALL: ALLDEP
.ENDIF

.INCLUDE :  target.mk

$(GENJAVAFILES) : $(RDB)
#	+javamaker -BUCR -O$(OUT) $(TYPES) $(RDB)
    +javamaker @$(mktmp -BUCR -O$(OUT) $(TYPES) $(RDB))

$(JAVACLASSFILES) : $(GENJAVAFILES)
