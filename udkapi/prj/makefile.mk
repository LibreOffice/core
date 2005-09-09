#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 16:43:36 $
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
    +unoidl @$(mktmp -I$(PRJ) -Burd -OHout $(ALLIDLFILES:+"\n"))
    +regmerge @$(mktmp  $@ /UCR out$/{$(?:f:s/.idl/.urd/:+"\n")} )
    touch $@

