#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2004-06-17 11:40:53 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2002
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2002 by Sun Microsystems, Inc.
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
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2002 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..

PRJNAME=basic
TARGET=testtool

# --- Settings ------------------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ------------------------------------------------------------

OBJFILES = \
    $(OBJ)$/cmdstrm.obj              \
    $(OBJ)$/cretstrm.obj             \
    $(OBJ)$/objtest.obj              \
    $(OBJ)$/tcommuni.obj

EXCEPTIONSFILES= \
    $(OBJ)$/cmdstrm.obj              \
    $(OBJ)$/objtest.obj

HXXFILES = $(INCCOM)$/keycodes.hxx			\
    $(INCCOM)$/classes.hxx			\
    $(INCCOM)$/r_cmds.hxx			\
    $(INCCOM)$/res_type.hxx			

.IF "$(GUI)" == "WNT" 
OBJFILES += $(OBJ)$/sysdir_win.obj
OBJFILES += $(OBJ)$/registry_win.obj
.ENDIF
# --- Targets ------------------------------------------------------------

.INCLUDE :  target.mk


$(OBJFILES) : $(HXXFILES)
$(HXXFILES) : $(MISC)$/xfilter.pl


$(INCCOM)$/keycodes.hxx : $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/vcl$/keycodes.hxx \
                          $(MISC)$/xfilter.pl
    +$(PERL) $(MISC)$/xfilter.pl $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)  vcl$/keycodes.hxx  $(INCCOM)$/keycodes  KEY_

$(INCCOM)$/classes.hxx :  ..$/inc$/rcontrol.hxx \
                          $(MISC)$/xfilter.pl
    +$(PERL) $(MISC)$/xfilter.pl ..$/inc  rcontrol.hxx  $(INCCOM)$/classes  M_

$(INCCOM)$/r_cmds.hxx :   ..$/inc$/rcontrol.hxx \
                          $(MISC)$/xfilter.pl
    +$(PERL) $(MISC)$/xfilter.pl ..$/inc  rcontrol.hxx  $(INCCOM)$/r_cmds  RC_

$(INCCOM)$/res_type.hxx : $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/vcl$/wintypes.hxx \
                          $(MISC)$/xfilter.pl
    +$(PERL) $(MISC)$/xfilter.pl $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)  vcl$/wintypes.hxx  $(INCCOM)$/res_type  WINDOW_



$(MISC)$/xfilter.pl : filter.pl
.IF "$(GUI)" == "UNX" || "$(USE_SHELL)"!="4nt"
       +tr  -d "\015" < filter.pl > $(MISC)$/xfilter.pl
       chmod 664 $(MISC)$/xfilter.pl
.ELSE
       +$(COPY) filter.pl $(MISC)$/xfilter.pl
       +attrib  -r $(MISC)$/xfilter.pl
.ENDIF

