#*************************************************************************
#
#   $RCSfile: tg_moz.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: hr $ $Date: 2003-04-28 16:45:02 $
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


.IF "$(EXPORTS)"!=""
EXPORT_HEADER=export_mozilla_header
.ENDIF

.IF "$(XPIDLSRCS)"!=""
XPIDL_TYPELIB_MODULE=$(MISC)$/$(XPIDL_MODULE).xpt
XPIDLHDR:=$(XPIDLSRCS:s/.idl/.h/)
XPIDLXPT:=$(XPIDLSRCS:s/.idl/.xpt/)
EXTRA_MOZ_TARGET+= $(foreach,i,$(XPIDLHDR) $(INCCOM)$/$i)
EXTRA_MOZ_TARGET+= $(foreach,i,$(XPIDLXPT) $(MISC)$/$i)

EXTRA_MOZ_TARGET+= $(XPIDL_TYPELIB_MODULE)
.ENDIF

ALLMOZ:=$(EXTRA_MOZ_TARGET) $(DO_XPIDL) $(GEN_PRCPUCFG) $(EXPORT_HEADER)

$(INCCOM)$/%.h : %.idl
    +$(COPY) $< $(INCCOM)
    +$(XPIDL) -m header -w -I $(SOLARIDLDIR)$/mozilla -I$(INCCOM) -I . -o $(INCCOM)$/$* $<

$(MISC)$/%.xpt : %.idl
    +$(XPIDL) -m typelib -w -I $(SOLARIDLDIR)$/mozilla -I$(INCCOM) -I . -o $(MISC)$/$* $<

.INCLUDE : target.mk

.IF "$(GUI)"=="WNT"
CPUCFG=_winnt.cfg
.ENDIF

.IF "$(OS)"=="SOLARIS"
CPUCFG=_solaris32.cfg
.ENDIF

.IF "$(OS)"=="LINUX"
CPUCFG=_linux.cfg
.ENDIF

.IF "$(GEN_PRCPUCFG)"!=""
$(GEN_PRCPUCFG): $(PRJ)$/pr$/include$/md$/$(CPUCFG) 
    @+$(COPY) $(PRJ)$/pr$/include$/md$/$(CPUCFG) $@
.ENDIF

.IF "$(EXPORTS)"!=""
$(EXPORT_HEADER):
    @+$(COPY) $(EXPORTS) $(INCCOM)
.ENDIF

.IF "$(XPIDLSRCS)"!=""
$(XPIDL_TYPELIB_MODULE): $(foreach,i,$(XPIDLXPT) $(MISC)$/$i)
    +$(XPIDL_LINK) $(MISC)$/$(XPIDL_MODULE).xpt $^
    @+-mkdir $(BIN)$/components
    @+$(COPY) $@ $(BIN)$/components
.ENDIF
