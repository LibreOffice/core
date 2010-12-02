#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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
TRANSEX*=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/transex3
ULFEX*=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/ulfex
XMLEX*=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/xmlex
XRMEX*=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/xrmex
CFGEX*=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/cfgex
AUTODOC*=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/autodoc
LOCALIZE_SL*=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/localize_sl
GSICHECK*=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/gsicheck

.IF "$(SYSTEM_LIBXSLT)"!="YES"
XSLTPROC*=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/xsltproc
.ELSE			# "$(SYSTEM_LIBXSLT)"!="YES"
XSLTPROC*=$(AUGMENT_LIBRARY_PATH) xsltproc
.ENDIF			# "$(SYSTEM_LIBXSLT)"!="YES"

ULFCONV*=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/ulfconv

MAKEDEPEND*=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/makedepend

SCP_CHECK_TOOL:=checkscp$E

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
IFEXIST:=if [ -f 
IFNOTEXIST:= if ! test -f
THEN:= ] ; then
FI:= ; fi
PIPEERROR=2>&1 |

# iz31658
CHECKZIPRESULT:=|| ret=$$?; if [[ "$$ret" != "12" && "$$ret" != "1" ]] ; then exit $$ret ; fi && echo "Nothing to update for zip"

# Platform specific
.IF "$(GUI)"=="WNT"
AWK*=awk
SORT*=sort
SED*=sed
GNUPATCH*=patch
# change drive and directory
CDD=cd
# expect cygwin tools to exist
COPY*=cp
COPYRECURSE=-r
DEREFERENCE=-L
COPYUPDATE=-u
ECHON=echo -n
ECHONL=echo
FIND*=find
FLIPCMD*=$(PERL) $(SOLARENV)/bin/slfl.pl
GNUCOPY*=cp
GNUMAKE*=make
GREP*=grep
LS*=ls
PERL*:=perl
.EXPORT : PERL
RENAME*=mv
TOUCH*=touch
TYPE*=cat
DUMPBIN*=dumpbin

.ELIF "$(GUI)"=="UNX"	# "$(GUI)"=="WNT"
SED*=sed
SORT*=sort
PERL*=perl
.EXPORT : PERL
TYPE=cat
CDD=cd
COPY=cp -f
.IF "$(OS)"=="MACOSX"
COPYRECURSE=-R
.ELSE #"$(OS)"=="MACOSX"
COPYRECURSE=-r
.ENDIF
.IF "$(OS)"=="SOLARIS"
AWK*=nawk
GNUCOPY*=gnucp
GNUPATCH*=gnupatch
GNUTAR*=/usr/sfw/bin/gtar
DEREFERENCE=
.ELSE			# "$(OS)"=="SOLARIS"
AWK*=awk
# this is not true, as BSD does not default to a GNU cp, but BSD cp
# so in dmake makefiles one should be restricted to use the subset
# of features that both BSD and GNU make support.
# as the GNU make build system requires GNUCOPY to really be a GNU copy
# we work around that for now by not setting GNUCOPY in sdev300.ini
# for fbsd for now, but for all other platforms it is set.
GNUCOPY*=cp
GNUPATCH*=patch
DEREFERENCE=-L
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

RM+=$(RMFLAGS)
ADJUSTVISIBILITY*=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/adjustvisibility
CONVERT*:=$(PERL) $(SOLARENV)/bin/leconvert.pl
EXECTEST := $(PERL) -w $(SOLARENV)/bin/exectest.pl
GCCINSTLIB:=$(PERL) -w $(SOLARENV)/bin/gccinstlib.pl

# The dmake $(PWD) apparantly produces paths with symlinks resolved, while the
# bash pwd command by default produces paths with unresolved symlinks, so that
# computing PATH_IN_MODULE in settings.mk would fail without the -P flag to the
# bash pwd command:
PWDFLAGS = -P
