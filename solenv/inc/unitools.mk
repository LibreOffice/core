#*************************************************************************
#
#   $RCSfile: unitools.mk,v $
#
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

# Common tools
TRANSEX*=transex3
ULFEX*=ulfex
XMLEX*=xmlex
XRMEX*=xrmex
CFGEX*=cfgex

ULFCONV*=ulfconv

MAKEDEPEND*=$(WRAPCMD) $(SOLARBINDIR)$/makedepend

EXECTEST:= $(PERL) -w $(SOLARENV)$/bin$/exectest.pl

SCP_CHECK_TOOL:=checkscp$E

# Not 4nt means $(GUI)==UNX or $(GUI)==WNT with tcsh
.IF "$(USE_SHELL)"!="4nt"
# iz32110: Calling a cygwin application from a non-cygwin shell requires
# backslashes to be escaped by another backslash: EES .. extra escape slash
EES:=

# iz29675: 4nt must not quote special characters, but tcsh has to.
# *NIX shells needs to use " quotes, but 4nt must not.
# EMQ .. extra meta quote (\\ at line end is \)
# USQ .. unix shell  quote
EMQ:=\\
USQ:="

NULLDEV:=/dev/null

# iz29609 helpmacro to check if file exists tcsh style
IFEXIST:=if ( -e
THEN:= )

# iz31658
CHECKZIPRESULT:=|| if ("$$status" != "12") exit $$status && echo "Nothing to update for zip"

.ELSE # "$(USE_SHELL)"!="4nt"
# (\\ at line end is \)
EES:=\\

EMQ:=
USQ:=

NULLDEV:=nul

# iz29609 helpmacro to check if file exists 4nt style
IFEXIST:=if exist
THEN:=

# iz31658
CHECKZIPRESULT:=^ iff errorlevel == 12 then ( echo Nothing to update for zip ^ set somedummyvar=%somedummyvar)

.ENDIF # "$(USE_SHELL)"!="4nt"

# Platform specific
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
DELAY=sleep
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
DELAY=delay
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
.ENDIF  "$(USE_SHELL)"!="4nt"
MKDIRHIER=$(MKDIR) 
DUMPBIN*=$(WRAPCMD) dumpbin

.ELIF "$(GUI)"=="UNX"	# "$(GUI)"=="WNT"
SED*=sed
SORT*=sort
PERL*=perl
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
.ENDIF			# "$(GUI)"=="UNX"

RM+=$(RMFLAGS)
ADJUSTVISIBILITY*:=$(WRAPCMD) adjustvisibility
CONVERT*:=$(PERL) $(SOLARENV)$/bin$/leconvert.pl

