#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: hr $ $Date: 2007-06-27 15:34:45 $
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

PRJNAME=accessibility
TARGET=extended

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

SLOFILES=\
    $(SLO)$/AccessibleBrowseBoxCheckBoxCell.obj		\
    $(SLO)$/AccessibleBrowseBoxBase.obj		\
    $(SLO)$/AccessibleBrowseBox.obj			\
    $(SLO)$/AccessibleBrowseBoxTableCell.obj	\
    $(SLO)$/AccessibleBrowseBoxHeaderCell.obj	\
    $(SLO)$/AccessibleBrowseBoxTableBase.obj	\
    $(SLO)$/AccessibleBrowseBoxTable.obj		\
    $(SLO)$/AccessibleBrowseBoxHeaderBar.obj	\
    $(SLO)$/accessibleiconchoicectrl.obj		\
    $(SLO)$/accessibleiconchoicectrlentry.obj	\
    $(SLO)$/accessiblelistbox.obj			\
    $(SLO)$/accessiblelistboxentry.obj		\
    $(SLO)$/accessibletabbarbase.obj            \
    $(SLO)$/accessibletabbar.obj			\
    $(SLO)$/accessibletabbarpage.obj		\
    $(SLO)$/accessibletabbarpagelist.obj		\
    $(SLO)$/accessibletablistbox.obj		\
    $(SLO)$/accessibletablistboxtable.obj	\
    $(SLO)$/listboxaccessible.obj			\
    $(SLO)$/accessiblebrowseboxcell.obj     \
    $(SLO)$/accessibleeditbrowseboxcell.obj \
    $(SLO)$/textwindowaccessibility.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

