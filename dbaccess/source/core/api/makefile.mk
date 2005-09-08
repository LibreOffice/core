#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 10:06:47 $
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

PRJ=..$/..$/..
PRJINC=$(PRJ)$/source
PRJNAME=dbaccess
TARGET=api

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Files -------------------------------------
SLOFILES=	\
        $(SLO)$/FilteredContainer.obj			\
        $(SLO)$/TableDeco.obj					\
        $(SLO)$/RowSetCacheIterator.obj			\
        $(SLO)$/RowSet.obj						\
        $(SLO)$/column.obj						\
        $(SLO)$/RowSetBase.obj					\
        $(SLO)$/CRowSetDataColumn.obj			\
        $(SLO)$/CRowSetColumn.obj				\
        $(SLO)$/querycomposer.obj				\
        $(SLO)$/CIndexes.obj					\
        $(SLO)$/CKey.obj						\
        $(SLO)$/CKeys.obj						\
        $(SLO)$/CKeyColumns.obj					\
        $(SLO)$/BookmarkSet.obj					\
        $(SLO)$/KeySet.obj						\
        $(SLO)$/StaticSet.obj					\
        $(SLO)$/CacheSet.obj					\
        $(SLO)$/RowSetCache.obj					\
        $(SLO)$/definitioncolumn.obj			\
        $(SLO)$/resultcolumn.obj				\
        $(SLO)$/datacolumn.obj					\
        $(SLO)$/resultset.obj					\
        $(SLO)$/statement.obj					\
        $(SLO)$/preparedstatement.obj			\
        $(SLO)$/callablestatement.obj			\
        $(SLO)$/query.obj						\
        $(SLO)$/querycontainer.obj				\
        $(SLO)$/table.obj						\
        $(SLO)$/viewcontainer.obj				\
        $(SLO)$/tablecontainer.obj				\
        $(SLO)$/querydescriptor.obj				\
        $(SLO)$/SingleSelectQueryComposer.obj	\
        $(SLO)$/HelperCollections.obj			\
        $(SLO)$/datasettings.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk

