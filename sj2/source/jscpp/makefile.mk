#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 20:13:36 $
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

PRJNAME=sj2
TARGET=jscpp
# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

UNOTYPES= \
    com.sun.star.uno.Exception	\
    com.sun.star.uno.XInterface	\
    com.sun.star.uno.TypeClass	\
    com.sun.star.awt.XControl	\
    com.sun.star.lang.XMultiServiceFactory	\
    com.sun.star.java.XJavaVM				



CXXFILES=	\
    sjapplet.cxx							\
    sjapplet_impl.cxx 


#.IF "$(GUI)"=="UNX"
#CXXFILES += widget.cxx
#.ENDIF

SLOFILES=	\
    $(SLO)$/sjapplet.obj						\
    $(SLO)$/sjapplet_impl.obj


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.INCLUDE :  $(PRJ)$/util$/target.pmk

