#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.6 $
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
PRJ=..

PRJNAME=bridges
TARGET=unotypes

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

#-------------------------------------------------------------------

CPPUMAKERFLAGS += -C
UNOUCRDEP=$(BUILDSOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(BUILDSOLARBINDIR)$/udkapi.rdb 
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

