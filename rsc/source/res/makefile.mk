#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2007-11-13 14:20:28 $
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

PRJNAME=rsc
TARGET=rscres

# --- Settings ------------------------------------------------------

.INCLUDE :  settings.mk

OBJFILES=               $(OBJ)$/rscclobj.obj                         \
                        $(OBJ)$/rsccont.obj                          \
                        $(OBJ)$/rsctop.obj                           \
                        $(OBJ)$/rscrange.obj                         \
                        $(OBJ)$/rscconst.obj                         \
                        $(OBJ)$/rscflag.obj                          \
                        $(OBJ)$/rscstr.obj                           \
                        $(OBJ)$/rscall.obj                           \
                        $(OBJ)$/rscmgr.obj                           \
                        $(OBJ)$/rscclass.obj                         \
                        $(OBJ)$/rscarray.obj

CXXFILES=       rscclobj.cxx	\
            rsctop.cxx	\
            rscrange.cxx	\
            rscconst.cxx	\
            rscflag.cxx	\
            rscstr.cxx	\
            rscall.cxx	\
            rsccont.cxx	\
            rscclass.cxx	\
            rscmgr.cxx	\
            rscarray.cxx
        
.INCLUDE :  target.mk
