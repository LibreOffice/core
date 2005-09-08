#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 17:43:33 $
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

PRJ = ..$/..$/..$/..$/..$/..$/..
PRJNAME = MyPersistObjectImpl
TARGET  = MyPersistObjectImpl
PACKAGE = com$/sun$/star$/cmp

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES        = ridl.jar unoil.jar jurt.jar juh.jar jut.jar java_uno.jar
JAVAFILES       = MyPersistObject.java
JAVACLASSFILES  = $(CLASSDIR)$/$(PACKAGE)$/MyPersistObject.class

#----- make a jar from compiled files ------------------------------

MAXLINELENGTH = 100000

JARCLASSDIRS    = com/sun/star/cmp
JARTARGET       = $(TARGET).jar
JARCOMPRESS 	= TRUE
CUSTOMMANIFESTFILE = manifest


# --- Files --------------------------------------------------------

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALL : \
    ALLTAR
.ELSE
ALL: 	ALLDEP
.ENDIF

.INCLUDE :  target.mk

