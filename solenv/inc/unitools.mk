#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: unitools.mk,v $
#
#   $Revision: 1.56 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 16:34:39 $
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

# Common tools - move this to the end / consolidate
TRANSEX*=transex3
ULFEX*=ulfex
XMLEX*=xmlex
XRMEX*=xrmex
CFGEX*=cfgex
XSLTPROC*=xsltproc

ULFCONV*=ulfconv

MAKEDEPEND*=$(SOLARBINDIR)$/makedepend

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

# iz29609 helpmacro to check if file exists
.IF "$(USE_SHELL)"=="bash"
IFEXIST:=if test -e
THEN:= ; then
FI:= ; fi
PIPEERROR=2>&1 |
.ELSE
IFEXIST:=if ( -e
THEN:= )
FI:=
PIPEERROR=|&
.ENDIF

# iz31658
.IF "$(USE_SHELL)"=="bash"
CHECKZIPRESULT:=|| if test "$$?" != "12" && "$$?" != "1" ; then exit $$? ; fi && echo "Nothing to update for zip"
.ELSE
CHECKZIPRESULT:=|| if ("$$status" != "12" && "$$status" != "1") exit $$status && echo "Nothing to update for zip"
.ENDIF

.ELSE # "$(USE_SHELL)"!="4nt"
# (\\ at line end is \)
EES:=\\

EMQ:=
USQ:=

NULLDEV:=nul

# iz29609 helpmacro to check if file exists 4nt style
IFEXIST:=+if exist
THEN:=
FI:=
PIPEERROR=|&

# iz31658
CHECKZIPRESULT:=^ iff errorlevel == 12 .and. errorlevel == 12 then ( echo Nothing to update for zip ^ set somedummyvar=%somedummyvar)

# try the same for 4nt "copy /u"
CHECKCOPYURESULT:=^ iff errorlevel == 2 then ( echo Nothing to update for copy ^ set somedummyvar=%somedummyvar)

# tell makedepend to write windows native format
MKDEPFLAGS=-n

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
ECHON=echo -n
ECHONL=echo
FIND*=find
GNUCOPY*=cp
GNUMAKE*=make
GREP*=grep
LS*=ls
PERL*:=perl
.EXPORT : PERL
RENAME*=mv
TOUCH*=touch
TYPE*=cat
TRANSEX*=transex3
ULFEX*=ulfex
XMLEX*=xmlex
XRMEX*=xrmex
CFGEX*=cfgex
.ELSE			# "$(USE_SHELL)"!="4nt"
CDD=+cdd
COPY*=+copy
COPYRECURSE=/s
COPYUPDATE=/u
DELAY*=+delay
ECHON*=+echos
ECHONL=+echo.
FIND*=$(BUILD_TOOLS)$/find.exe
.IF "$(use_cygcp)"!=""
GNUCOPY=$(BUILD_TOOLS)$/gnucp.exe
.ENDIF			# "$(use_cygcp)"!=""
GNUCOPY*=$(BUILD_TOOLS)$/cp.exe
GNUMAKE*=$(BUILD_TOOLS)$/gnumake.exe
GREP*=$(BUILD_TOOLS)$/grep.exe
LS*=$(BUILD_TOOLS)$/ls.exe
#wraper for solenv\bin\mkdir.pl to fix mkdir /p problem
PERL*:=+call perl5.btm
.EXPORT : PERL
RENAME*=+ren
TOUCH*=$(PERL) $(SOLARENV)$/bin$/touch.pl
TYPE*=+type
XARGS*=tr -d "\015" | xargs
4nt_force_shell:=+
.ENDIF  "$(USE_SHELL)"!="4nt"
DUMPBIN*=dumpbin

.ELIF "$(GUI)"=="UNX"	# "$(GUI)"=="WNT"
SED*=sed
SORT*=sort
PERL*=perl
.EXPORT : PERL
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
GREP=grep
FIND=find
LS=ls
ECHON=echo -n
ECHONL=echo
.ENDIF			# "$(GUI)"=="UNX"

# (Global) Set if not set before
DELAY*=sleep
MKDIR*=mkdir$E
MKDIRHIER*=mkdir$E -p
XARGS*=xargs

RM+=$(RMFLAGS)
ADJUSTVISIBILITY*:=adjustvisibility
CONVERT*:=$(PERL) $(SOLARENV)$/bin$/leconvert.pl
EXECTEST := $(PERL) -w $(SOLARENV)$/bin$/exectest.pl
GCCINSTLIB:=$(PERL) -w $(SOLARENV)$/bin$/gccinstlib.pl
