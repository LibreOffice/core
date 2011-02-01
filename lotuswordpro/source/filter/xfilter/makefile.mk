#*************************************************************************
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: IBM Corporation
#
#   Copyright: 2008 by IBM Corporation
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=lotuswordpro
TARGET=xfilter

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk


SLOFILES=	\
        $(SLO)$/xfsaxattrlist.obj	\
        $(SLO)$/xfsaxstream.obj	\
        $(SLO)$/xffilestream.obj	\
        $(SLO)$/xffileattrlist.obj	\
        $(SLO)$/xfstylemanager.obj	\
        $(SLO)$/xfstyle.obj		\
        $(SLO)$/xftextstyle.obj	\
        $(SLO)$/xffont.obj		\
        $(SLO)$/xffontdecl.obj	\
        $(SLO)$/xfstylecont.obj	\
        $(SLO)$/xfcontentcontainer.obj	\
        $(SLO)$/xfutil.obj			\
        $(SLO)$/xfparastyle.obj		\
        $(SLO)$/xffontfactory.obj	\
        $(SLO)$/xftextcontent.obj	\
        $(SLO)$/xfparagraph.obj	\
        $(SLO)$/xftextspan.obj	\
        $(SLO)$/xfofficemeta.obj	\
        $(SLO)$/xfglobal.obj		\
        $(SLO)$/xflist.obj		\
        $(SLO)$/xftextheading.obj	\
        $(SLO)$/xfpagemaster.obj	\
        $(SLO)$/xfmasterpage.obj	\
        $(SLO)$/xfheaderstyle.obj	\
        $(SLO)$/xfsectionstyle.obj	\
        $(SLO)$/xfsection.obj	\
        $(SLO)$/xfliststyle.obj	\
        $(SLO)$/xfborders.obj	\
        $(SLO)$/xfshadow.obj		\
        $(SLO)$/xfcolor.obj		\
        $(SLO)$/xfcolumns.obj	\
        $(SLO)$/xfmargins.obj	\
        $(SLO)$/xfdatestyle.obj	\
        $(SLO)$/xftimestyle.obj	\
        $(SLO)$/xftime.obj		\
        $(SLO)$/xfdate.obj		\
        $(SLO)$/xfframe.obj		\
        $(SLO)$/xfframestyle.obj	\
        $(SLO)$/xftable.obj		\
        $(SLO)$/xfrow.obj		\
        $(SLO)$/xfcell.obj		\
        $(SLO)$/xfcolstyle.obj	\
        $(SLO)$/xfcellstyle.obj	\
        $(SLO)$/xfrowstyle.obj	\
        $(SLO)$/xftablestyle.obj	\
        $(SLO)$/xftabstyle.obj	\
        $(SLO)$/xfoutlinestyle.obj	\
        $(SLO)$/xfindex.obj		\
        $(SLO)$/xfbookmarkref.obj	\
        $(SLO)$/xfdrawobj.obj	\
        $(SLO)$/xfdrawline.obj	\
        $(SLO)$/xfdrawrect.obj	\
        $(SLO)$/xfdrawcircle.obj	\
        $(SLO)$/xfdrawellipse.obj	\
        $(SLO)$/xfdrawpolyline.obj	\
        $(SLO)$/xfdrawpolygon.obj	\
        $(SLO)$/xfdrawpath.obj	\
        $(SLO)$/xfdrawstyle.obj	\
        $(SLO)$/xfdrawlinestyle.obj	\
        $(SLO)$/xfdrawareastyle.obj	\
        $(SLO)$/xfimage.obj		\
        $(SLO)$/xfimagestyle.obj	\
        $(SLO)$/xfbase64.obj		\
        $(SLO)$/xfoleobj.obj		\
        $(SLO)$/xfbgimage.obj	\
        $(SLO)$/xfconfigmanager.obj	\
        $(SLO)$/xfnumberstyle.obj	\
        $(SLO)$/xffloatframe.obj	\
        $(SLO)$/xfentry.obj	\
        $(SLO)$/xfcrossref.obj	\
        $(SLO)$/xfdocfield.obj	\
        $(SLO)$/xfchange.obj	\
        $(SLO)$/xftextboxstyle.obj


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
