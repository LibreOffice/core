#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 16:43:52 $
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
PRJPCH=

PRJNAME=udkapi
TARGET=udkapi

#use_starjar=true


# --- Settings -----------------------------------------------------

.INCLUDE :  makefile.pmk

# ------------------------------------------------------------------

UNOIDLDBFILES= \
    $(UCR)$/css.db \
    $(UCR)$/cssutil.db \
    $(UCR)$/cssbeans.db \
    $(UCR)$/cssbridge.db \
    $(UCR)$/cssboleautomation.db \
    $(UCR)$/cssconnection.db\
    $(UCR)$/csscontainer.db \
    $(UCR)$/cssio.db \
    $(UCR)$/cssjava.db \
    $(UCR)$/csslang.db \
    $(UCR)$/csssec.db \
    $(UCR)$/cssloader.db \
    $(UCR)$/cssreflection.db \
    $(UCR)$/cssregistry.db \
    $(UCR)$/cssscript.db \
    $(UCR)$/csstest.db \
    $(UCR)$/cssperftest.db \
    $(UCR)$/cssbridgetest.db \
    $(UCR)$/cssuno.db \
    $(UCR)$/cssulog.db \
    $(UCR)$/csscorba.db \
    $(UCR)$/cssiop.db \
    $(UCR)$/cssiiop.db \
    $(UCR)$/cssgiop.db \
    $(UCR)$/csstask.db \
    $(UCR)$/cssuri.db

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


