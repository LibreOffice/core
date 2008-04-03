#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: kz $ $Date: 2008-04-03 15:24:00 $
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

PRJ=..$/..
PRJNAME=sd
TARGET=unotypesanim

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

PRIVATERDB = slideshow.rdb
UNOUCRRDB = $(SOLARBINDIR)$/$(PRIVATERDB)
UNOUCRDEP = $(UNOUCRRDB)
UNOUCROUT = $(OUT)$/inc

CPPUMAKERFLAGS += -C -X$(SOLARBINDIR)$/types.rdb

#UNOTYPES= \
#	com.sun.star.presentation.XSlideShow \
#	com.sun.star.presentation.XSlideShowView \
#	com.sun.star.presentation.XSlideShowListener \
#	com.sun.star.presentation.XShapeEventListener

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
