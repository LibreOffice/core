#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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



PRJ=..$/..

PRJNAME := extensions
#PACKAGE := com$/sun$/star$/pgp
PACKAGE := 
TARGET  := test_com_sun_star_pgp


# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk


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
#	javamaker -BUCR -O$(OUT) $(TYPES) $(RDB)
    javamaker @$(mktmp -BUCR -O$(OUT) $(TYPES) $(RDB))

$(JAVACLASSFILES) : $(GENJAVAFILES)
