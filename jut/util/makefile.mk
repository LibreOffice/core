#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 19:21:35 $
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

PRJ		= ..
PRJNAME	= jut
TARGET	= jut

# --- Settings -----------------------------------------------------

.INCLUDE: makefile.pmk

JARCLASSDIRS	= com
JARTARGET		= $(TARGET).jar
JARCOMPRESS 	= TRUE
JARCLASSPATH = $(JARFILES)
CUSTOMMANIFESTFILE = manifest

# Special work necessary for building java reference with javadoc.
# The source of puplic APIs must be delivered and used later in the
# odk module.
ZIP1TARGET=$(TARGET)_src
ZIP1FLAGS=-u -r
ZIP1DIR=$(PRJ)
ZIP1LIST=com -x "*makefile.mk"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
