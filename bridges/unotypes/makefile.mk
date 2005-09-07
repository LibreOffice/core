#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 23:03:06 $
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
PRJ=..

PRJNAME=bridges
TARGET=unotypes

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

#-------------------------------------------------------------------

CPPUMAKERFLAGS += -C
UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb 
UNOUCROUT=$(OUT)$/inc

UNOTYPES = \
    com.sun.star.bridge.XProtocolProperties		\
    com.sun.star.corba.giop.MessageHeader_1_2	\
    com.sun.star.corba.giop.MsgType_1_1		\
    com.sun.star.corba.iop.ServiceContextList	\
    com.sun.star.corba.iop.ProfileIdGroup		\
    com.sun.star.corba.iiop.ProfileBody_1_1		\
    com.sun.star.corba.LogicalThreadID		\
    com.sun.star.corba.iop.ServiceIdGroup		\
    com.sun.star.corba.giop.ReplyHeader_1_2		\
    com.sun.star.corba.giop.RequestHeader_1_2	\
    com.sun.star.corba.TCKind			\
    com.sun.star.corba.ObjectKey			\
    com.sun.star.uno.XInterface			\
    com.sun.star.lang.DisposedException		\
    com.sun.star.uno.TypeClass

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

