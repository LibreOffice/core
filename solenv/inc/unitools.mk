#*************************************************************************
#
#   $RCSfile: unitools.mk,v $
#
#   $Revision: 1.29 $
#
#   last change: $Author: hjs $ $Date: 2004-07-16 13:14:42 $
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
.IF "$(GUI)"=="WNT"
AWK*=awk
SORT*=sort
SED*=sed
.IF "$(USE_SHELL)"!="4nt"
# change drive and directory
CDD=cd
# expect cygwin tools to exist
COPY*=cp
COPYRECURSE=-r
COPYUPDATE=-u
ECHON=echo -n
ECHONL=echo
FIND*=find
GNUCOPY*=cp
GNUMAKE*=make
GREP*=grep
LS*=ls
MKDIR*=mkdir.pl
PERL*:=perl
RENAME*=mv
TOUCH*=touch
TYPE*=cat
.ELSE			# "$(USE_SHELL)"!="4nt"
CDD=cdd
COPY*=copy
COPYRECURSE=/s
COPYUPDATE=/u
ECHON*=echos
ECHONL=echo.
FIND*=$(BUILD_TOOLS)$/find.exe
.IF "$(use_cygcp)"!=""
GNUCOPY=$(BUILD_TOOLS)$/gnucp.exe
.ENDIF			# "$(use_cygcp)"!=""
GNUCOPY*=$(BUILD_TOOLS)$/cp.exe
GNUMAKE*=$(ENV_TOOLS)$/gnumake.exe
GREP*=$(ENV_TOOLS)$/grep32.exe
LS*=$(BUILD_TOOLS)$/ls.exe
#wraper for solenv\bin\mkdir.pl to fix mkdir /p problem
MKDIR=+$(SOLARENV)$/bin$/mkdir.btm
PERL*:=call perl5.btm
RENAME*=ren
TOUCH*=$(BUILD_TOOLS)$/touch.exe
TYPE*=type
TRANSEX*=transex3
ULFEX*=ulfex
XMLEX*=xmlex
XRMEX*=xrmex
.ENDIF  "$(USE_SHELL)"!="4nt"
MKDIRHIER=$(MKDIR) 
SCP_CHECK_TOOL=checkscp.exe

.ELIF "$(GUI)"=="UNX"	# "$(GUI)"=="WNT"
SED*=sed
SORT*=sort
PERL=perl
TYPE=cat
CDD=cd
COPY=cp -f
COPYRECURSE=-r
.IF "$(OS)"=="SOLARIS"
AWK*=nawk
GNUCOPY*=gnucp
GNUPATCH*=gnupatch
.ELSE			# "$(OS)"=="SOLARIS"
AWK*=awk
GNUCOPY*=cp
GNUPATCH*=patch
.ENDIF			# "$(OS)"=="SOLARIS"
.IF "$(OS)"=="LINUX" || "$(OS)"=="MACOSX"
GNUMAKE*=make
.ELSE			# "$(OS)"=="LINUX" || "$(OS)"=="MACOSX"
GNUMAKE*=gmake
.ENDIF			# "$(OS)"=="LINUX" || "$(OS)"=="MACOSX"
TOUCH=touch
RENAME=mv
MKDIR=mkdir
MKDIRHIER=mkdir -p
GREP=grep
FIND=find
LS=ls
ECHON=echo -n
ECHONL=echo
TRANSEX*=transex3
ULFEX*=ulfex
XMLEX*=xmlex
XRMEX*=xrmex
.ENDIF			# "$(GUI)"=="UNX"

MAKEDEPEND*=$(WRAPCMD) makedepend

RM+=$(RMFLAGS)

.IF "$(GUI)"=="UNX"
SCP_CHECK_TOOL=checkscp
NULLDEV=/dev/null
.ELIF "$(GUI)"=="WNT"
.IF "$(USE_SHELL)"!="4nt"
NULLDEV=/dev/null
.ELSE
NULLDEV=nul
.ENDIF
.ENDIF

EXECTEST = $(PERL) -w $(SOLARENV)$/bin$/exectest.pl
