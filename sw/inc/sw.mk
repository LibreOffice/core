#*************************************************************************
#
#   $RCSfile: sw.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: jp $ $Date: 2000-09-28 13:02:59 $
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

# CDEFS=$(CDEFS) -DXXX

# Bereiche aktivieren

#CDEFS+=-DXML_CORE_API
CDEFS+=-DNUM_RELSPACE

# MakeDepn nicht das segdefs.hxx und die PCHs auswerten lassen
# PCHs werden nur in den ..._1st Verzeichnissen aufgenommen
EXCDEPN=segdefs.hxx
EXCDEPNPCH=core_pch.hxx;filt_pch.hxx;ui_pch.hxx
MKDEPFLAGS+=-E:$(EXCDEPN);$(EXCDEPNPCH)

.IF "$(GUI)" == "WIN" || "$(GUI)" == "WNT" || "$(GUI)" == "OS2"
CDEFS+=-DSBASIC
.ENDIF

.IF "$(GUI)" == "WIN"
CDEFS+=-DOLE -DOLE2
.ENDIF

.IF "$(compact)" != ""
CDEFS+=-DCOMPACT
.ENDIF

.IF "$(debug)" != ""
DB=cv
.ENDIF

.IF "$(prjpch)" != ""
CDEFS+=-DPRECOMPILED
.ENDIF


# ------------------------------------------------------------------
# alte defines fuer die Sourcen
# ------------------------------------------------------------------

.IF "$(GUI)" == "WIN"
CDEFS+=-DW30
.ENDIF

.IF "$(GUI)" == "OS2"
CDEFS+=-DPM2 -DPM20
RSCFLAGS+=-DPM2 -DPM20
.IF "$(debug)" != ""
# fuer OS/2 und debug groessere Pages
LIBFLAGS=/C /P512
.ENDIF
.ENDIF

.IF "$(COM)" == "BLC"
CDEFS+=-DTCPP
.ENDIF

.IF "$(COM)" == "STC"
CDEFS+=-DZTC
.ENDIF

.IF "$(COM)" == "ICC"
CDEFS+=-DCSET
.ENDIF


# ------------------------------------------------------------------------
# WINMSCI Compiler
# ------------------------------------------------------------------------
.IF "$(GUI)$(COM)" == "WINMSC"

.IF "$(optimize)" != ""
CFLAGS+=-OV9
.ENDIF

# kein -GEfs wegen dann fehlender inlinings
CFLAGSOBJGUIST=-Alfd -GA -GEd

.ENDIF # WINMSCI
# ------------------------------------------------------------------------

# ------------------------------------------------------------------------
# add uno headers to inlude path
# ------------------------------------------------------------------------
INCPRE+=$(SOLARINCDIR)$/offuh
