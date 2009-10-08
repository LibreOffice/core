#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: unitools.mk,v $
#
# $Revision: 1.59 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

# Common tools - move this to the end / consolidate
TRANSEX*=$(AUGMENT_LIBRARY_PATH) transex3
ULFEX*=$(AUGMENT_LIBRARY_PATH) ulfex
XMLEX*=$(AUGMENT_LIBRARY_PATH) xmlex
XRMEX*=$(AUGMENT_LIBRARY_PATH) xrmex
CFGEX*=$(AUGMENT_LIBRARY_PATH) cfgex
XSLTPROC*=$(AUGMENT_LIBRARY_PATH) xsltproc

ULFCONV*=$(AUGMENT_LIBRARY_PATH) ulfconv

MAKEDEPEND*=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)$/makedepend

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
CHECKZIPRESULT:=|| ret=$$?; if [[ "$$ret" != "12" && "$$ret" != "1" ]] ; then exit $$ret ; fi && echo "Nothing to update for zip"
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
#MKDEPFLAGS=-n

.ENDIF # "$(USE_SHELL)"!="4nt"

# Platform specific
.IF "$(GUI)"=="WNT"
AWK*=awk
SORT*=sort
SED*=sed
GNUPATCH*=patch
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
FLIPCMD*=slfl.pl
GNUCOPY*=cp
GNUMAKE*=make
GREP*=grep
LS*=ls
PERL*:=perl
.EXPORT : PERL
RENAME*=mv
TOUCH*=touch
TYPE*=cat
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
MKDIRHIER=+mkdir /sn
#wraper for solenv\bin\mkdir.pl to fix mkdir /p problem
PERL*:=+call perl5.btm
.EXPORT : PERL
RENAME*=+ren
RMDIR:=$(PERL) $(SOLARENV)$/bin$/rmdir.pl
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
GNUTAR*=gtar
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
.ELIF "$(GUI)"=="OS2"
SED*=sed
SORT*=sort
PERL*=perl
TYPE=cat
CDD=@cd
COPY*=$(SHELL) /c copy /b
COPYRECURSE=/s
COPYUPDATE=/u
DELAY=sleep
ECHON*=echos
ECHONL=+echo.
AWK*=awk
GNUCOPY*=cp
GNUPATCH*=gnupatch
GNUMAKE*=make
TOUCH=touch /c
#YD rename doesn't work across different drives!
RENAME=mv
MKDIR=+md
MKDIRHIER=+md /s
GREP=grep
FIND=find
LS=ls
DUMPBIN=echo
4nt_force_shell:=+
.ENDIF			# "$(GUI)"=="UNX"

# (Global) Set if not set before
DELAY*=sleep
MKDIR*=mkdir$E
MKDIRHIER*=mkdir$E -p
RMDIR*=rmdir
XARGS*=xargs
GNUTAR*:=tar
TAR*:=tar

RM+=$(RMFLAGS)
ADJUSTVISIBILITY*=$(AUGMENT_LIBRARY_PATH) adjustvisibility
CONVERT*:=$(PERL) $(SOLARENV)$/bin$/leconvert.pl
EXECTEST := $(PERL) -w $(SOLARENV)$/bin$/exectest.pl
GCCINSTLIB:=$(PERL) -w $(SOLARENV)$/bin$/gccinstlib.pl

# The dmake $(PWD) variable and the tcsh pwd command both apparantly produce
# paths with symlinks resolved, while the bash pwd command by default produces
# paths with unresolved symlinks, so that computing PATH_IN_MODULE in
# settings.mk would fail without the -P flag to the bash pwd command:
.IF "$(USE_SHELL)" == "bash"
PWDFLAGS = -P
.ENDIF
