#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: cl $ $Date: 2001-03-14 17:35:21 $
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
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..
PRJPCH=

PRJNAME=offapi
TARGET=offapi_db


# --- Settings -----------------------------------------------------

.INCLUDE :  makefile.pmk

# ------------------------------------------------------------------

#regcheck .PHONY: 
#	$(COMSPEC) /c checkrdb.btm o:\comptest\swt614.rdb $(PRJ)$/$(OUTPATH)$/bin$/$(PRJNAME).rdb 

# ------------------------------------------------------------------

UNOIDLDBREGS= \
    $(SOLARBINDIR)$/udkapi.rdb

UNOIDLDBFILES= \
    $(UCR)$/cssawt.db \
    $(UCR)$/csschart.db \
    $(UCR)$/cssconfiguration.db \
    $(UCR)$/cssdatatransfer.db \
    $(UCR)$/cssdclipboard.db \
    $(UCR)$/cssddnd.db \
    $(UCR)$/cssdocument.db \
    $(UCR)$/cssdrawing.db \
    $(UCR)$/cssfcomp.db \
    $(UCR)$/cssform.db \
    $(UCR)$/cssformula.db \
    $(UCR)$/cssframe.db \
    $(UCR)$/cssinstallation.db \
    $(UCR)$/cssi18n.db \
    $(UCR)$/csslinguistic2.db \
    $(UCR)$/cssmozilla.db \
    $(UCR)$/csspackage.db \
    $(UCR)$/csspackages.db \
    $(UCR)$/csspgp.db \
    $(UCR)$/cssplugin.db \
    $(UCR)$/csspresentation.db \
    $(UCR)$/cssresource.db \
    $(UCR)$/csssax.db \
    $(UCR)$/cssscanner.db \
    $(UCR)$/csssdb.db \
    $(UCR)$/csssdbc.db \
    $(UCR)$/csssdbcx.db \
    $(UCR)$/csssheet.db \
    $(UCR)$/cssstyle.db \
    $(UCR)$/csstable.db \
    $(UCR)$/csstask.db \
    $(UCR)$/csstext.db \
    $(UCR)$/csstextfield.db \
    $(UCR)$/cssfieldmaster.db \
    $(UCR)$/cssucb.db \
    $(UCR)$/cssutil.db \
    $(UCR)$/cssview.db \
    $(UCR)$/csssvg.db \
    $(UCR)$/csssync.db \
    $(UCR)$/cssxml.db \
    $(UCR)$/csssetup.db \
    $(UCR)$/cssui.db \
    $(UCR)$/cssimage.db

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

