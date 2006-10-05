#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: tg_java.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: kz $ $Date: 2006-10-05 16:21:42 $
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



$(CLASSDIR)$/com$/sun$/star$/upd$/$(VERSIONINFOFILE)_updversion.class .PHONY: $(MISC)$/java$/com$/sun$/star$/upd$/$(VERSIONINFOFILE)_updversion.java 

$(MISC)$/java$/com$/sun$/star$/upd$/$(VERSIONINFOFILE)_updversion.java .PHONY:
    @echo ------------------------------
    @echo Making: $@
    @-$(MKDIRHIER) $(MISC)$/java$/com$/sun$/star$/upd
.IF "$(GUI)"=="UNX"
    @echo package com.sun.star.upd\; > $@
    @echo 'public class $(VERSIONINFOFILE)_updversion { public static String Version = "$(RSCREVISION)";}' >> $@
.ELSE			# "$(GUI)"=="UNX"
    @echo package com.sun.star.upd; > $@
    @echo public class $(VERSIONINFOFILE)_updversion { public static String Version = "$(RSCREVISION)";} >> $@
.ENDIF			# "$(GUI)"=="UNX"

.IF "$(JAVATARGET)"!=""
.IF "$(PACKAGE)"!=""
$(CLASSDIR)$/$(IDLPACKAGE)$/%.class .NOINFER .IGNORE : %.java
#	echo $@
    +-$(RM) $(JAVATARGET) >& $(NULLDEV)
.ELSE			# "$(PACKAGE)"!=""
$(CLASSDIR)$/%.class .NOINFER .IGNORE : %.java
#	echo $@
    +-$(RM) $(JAVATARGET) >& $(NULLDEV)
.ENDIF			# "$(PACKAGE)"!=""

$(JAVATARGET) :	$(JAVAFILES) $(JAVACLASSFILES) 
.IF "$(JAVARESPONSE)"!=""
    $(JAVAC) @<<
    $(JAVACPS) $(CLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) $(JAVAFILES)
    <<keep
.ELSE			# "$(JAVARESPONSE)"!=""
.IF "$(use_jdep)"!=""
    $(JAVAC) -depend $(JAVACPS) $(CLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) $(JAVAFILES)
.ELSE			# "$(use_jdep)"!=""
.IF "$(javauno)"!=""
.IF "$(JAVAFILES:d)"==""
    $(JAVAC) $(JAVACPS) $(CLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) $(JAVAFILES)
.ELSE			# "$(JAVAFILES:d)"==""
    @+$(TOUCH) $(INPATH)_$(VCSID)_a_dummy.java >& $(NULLDEV)
    $(JAVAC) $(JAVACPS) $(CLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) .$/*.java $(uniq $(JAVAFILES:d:+"*.java"))
    @+-$(RM) $(INPATH)_$(VCSID)_a_dummy.java >& $(NULLDEV)
.ENDIF			# "$(JAVAFILES:d)"==""
.ELSE			# "$(javauno)"!=""
.IF "$(USE_SHELL)"=="4nt"
    $(JAVAC) $(JAVACPS) $(CLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) $(JAVAFILES)
.ELSE			# "$(USE_SHELL)"=="4nt"
    $(JAVAC) $(JAVACPS) "$(CLASSPATH)" -d $(CLASSDIR) $(JAVAFLAGS) $(JAVAFILES)
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(javauno)"!=""
.ENDIF			# "$(use_jdep)"!=""
.ENDIF			# "$(JAVARESPONSE)"!=""
.IF "$(UPDATER)"!=""
.IF "$(OS)$(CPU)"=="SOLARISS"
    +-find $(CLASSDIR) -type d -user $(USER) \! -perm -5 -print | xargs chmod a+r $$1 >& $(NULLDEV)
.ENDIF
.ENDIF
    @echo > $@

.ENDIF			# "$(JAVATARGET)"!=""

