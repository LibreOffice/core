

.IF "$(JAVACLASSFILES:s/DEFINED//)$(javauno)"!=""

#$(JAVACLASSFILES):	$(JAVAFILES)
#.IF "$(JAVARESPONSE)"!=""
#	$(JAVAC) @<<
#	$(JAVACPS) $(CLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) $(JAVAFILES)
#	<<keep
#.ELSE
##	$(JAVAC) -depend $(JAVACPS) $(CLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) $(JAVAFILES)
#.ENDIF
#

$(CLASSDIR)$/com$/sun$/star$/upd$/$(VERSIONINFOFILE)_updversion.class .PHONY: $(MISC)$/java$/com$/sun$/star$/upd$/$(VERSIONINFOFILE)_updversion.java 

$(MISC)$/java$/com$/sun$/star$/upd$/$(VERSIONINFOFILE)_updversion.java .PHONY:
    @echo ------------------------------
    @echo Making: $@
    @+-$(MKDIRHIER) $(MISC)$/java$/com$/sun$/star$/upd
.IF "$(GUI)"=="WNT"
.IF "$(UPDATER)"!=""
    +tolodir $(MISC)$
#	+tolodir $(MISC)$/java
.ENDIF
.ENDIF
.IF "$(GUI)"=="UNX"
    @+echo package com.sun.star.upd\; > $@
    @+echo 'public class $(VERSIONINFOFILE)_updversion { public static String Version = "$(RSCREVISION)";}' >> $@
.ELSE
    @+echo package com.sun.star.upd; > $@
    @+echo public class $(VERSIONINFOFILE)_updversion { public static String Version = "$(RSCREVISION)";} >> $@
.ENDIF

.IF "$(JAVATARGET)"!=""
.IF "$(PACKAGE)"!=""
$(CLASSDIR)$/$(IDLPACKAGE)$/%.class .NOINFER .IGNORE .SILENT : $(JAVAFILES)
#	+echo $@
    @+-$(RM) $(JAVATARGET) >& $(NULLDEV)
.ELSE
%.class .NOINFER .IGNORE .SILENT : $(JAVAFILES)
#	+echo $@
    @+-$(RM) $(JAVATARGET) >& $(NULLDEV)
.ENDIF
# $(JAVACLASSFILES)  : $(JAVAFILES)
# 	@$(RM) $(JAVATARGET) >& $(NULLDEV)

$(JAVATARGET) :	$(JAVAFILES) $(JAVACLASSFILES) 
.IF "$(JAVARESPONSE)"!=""
    $(JAVAC) @<<
    $(JAVACPS) $(CLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) $(JAVAFILES)
    <<keep
.ELSE
.IF "$(use_jdep)"!=""
    $(JAVAC) -depend $(JAVACPS) $(CLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) $(JAVAFILES)
.ELSE
.IF "$(javauno)"!=""
.IF "$(JAVAFILES:d)"==""
    $(JAVAC) $(JAVACPS) $(CLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) $(JAVAFILES)
.ELSE
    @+$(TOUCH) $(INPATH)_$(VCSID)_a_dummy.java >& $(NULLDEV)
    $(JAVAC) $(JAVACPS) $(CLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) .$/*.java $(uniq $(JAVAFILES:d:+"*.java"))
    @+-$(RM) $(INPATH)_$(VCSID)_a_dummy.java >& $(NULLDEV)
.ENDIF
.ELSE
    $(JAVAC) $(JAVACPS) $(CLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) $(JAVAFILES)
.ENDIF
.ENDIF
.ENDIF
.IF "$(UPDATER)"!=""
.IF "$(OS)$(CPU)"=="SOLARISS"
    +-find $(CLASSDIR) -type d -user $(USER) \! -perm -5 -print | xargs chmod a+r $$1 >& $(NULLDEV)
.ENDIF
.ENDIF
    @+echo > $@

.ENDIF
.ENDIF

