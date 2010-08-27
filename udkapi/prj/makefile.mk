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

PRJ=.
PRJNAME:=udkapi

# --- Settings -----------------------------------------------------

OUT!:=$(PRJ)$/out

# ------------------------------------------------------------------

INCLUDES= \
    com$/sun$/star$/container$/makefile.mk \
    com$/sun$/star$/io$/makefile.mk \
    com$/sun$/star$/reflection$/makefile.mk \
    com$/sun$/star$/beans$/makefile.mk \
    com$/sun$/star$/lang$/makefile.mk \
    com$/sun$/star$/uno$/makefile.mk \
    com$/sun$/star$/uno$/util$/logging$/makefile.mk \
    com$/sun$/star$/corba$/makefile.mk			\
    com$/sun$/star$/corba$/iop$/makefile.mk 	\
    com$/sun$/star$/corba$/giop$/makefile.mk 	\
    com$/sun$/star$/corba$/iiop$/makefile.mk 	\
    com$/sun$/star$/script$/makefile.mk \
    com$/sun$/star$/test$/makefile.mk \
    com$/sun$/star$/registry$/makefile.mk \
    com$/sun$/star$/loader$/makefile.mk \
    com$/sun$/star$/bridge$/makefile.mk 

.INCLUDE: $(INCLUDES)

out$/$(PRJNAME).rdb:: $(ALLIDLFILES)
    unoidl @$(mktmp -I$(PRJ) -Burd -OHout $(ALLIDLFILES:+"\n"))
    regmerge @$(mktmp  $@ /UCR out$/{$(?:f:s/.idl/.urd/:+"\n")} )
    touch $@

