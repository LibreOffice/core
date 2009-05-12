#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: unosdk.mk,v $
#
# $Revision: 1.3 $
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

PPATH=inc

FILES= \
    $(PPATH)\rtl\wstring 	\
    $(PPATH)\rtl\wstring.h 	\
    $(PPATH)\rtl\wstring.hxx \
    $(PPATH)\rtl\string.h 	\
    $(PPATH)\rtl\string.hxx 	\
    $(PPATH)\rtl\wstrbuf.hxx \
    $(PPATH)\rtl\strbuf.hxx 	\
    $(PPATH)\rtl\locale.h 	\
    $(PPATH)\rtl\locale.hxx \
    $(PPATH)\rtl\char.h 		\
    $(PPATH)\rtl\char.hxx 	\
    $(PPATH)\rtl\macros.hxx \
    $(PPATH)\rtl\object.h 	\
    $(PPATH)\rtl\rb.h 

unosdk: $(FILES)
     docpp -H -m -f  -u -d ..\..\doc\sal\rtl $(FILES)
