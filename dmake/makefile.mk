# 			//// Makefile for DMAKE. \\\\
# The target system is characterized by the following macros imported from
# the environment.
#
#	OS	      - gives the class of operating system
#	OSRELEASE     - optionally specifies the particular release of the OS
#	OSENVIRONMENT - optionally specifies the environment under which the
#                       OS and OSENVIRONMENT are running.
#
# For valid values for the above macros consult the readme/* files or type
# 'make' by itself to get a summary of what is available.

# First target in the makefile, do this so that targets declared in the
# included files are never marked as being the first *default* target.
first : all ;

#Enable keeping of state for future compiles
.KEEP_STATE *:= _state.mk

# Define $(PUBLIC)
_osenv   := $(OSENVIRONMENT)$/
_osre    := $(OSRELEASE)$/$(!null,$(OSENVIRONMENT) $(_osenv))
ENVDIR    = $(OS)$/$(!null,$(OSRELEASE) $(_osre))
PUBLIC    = $(ENVDIR)public.h
MKCONFIG := startup/config.mk

# Define the source files
SRC =\
    infer.c make.c stat.c expand.c dmstring.c hash.c dag.c dmake.c\
    path.c imacs.c sysintf.c parse.c getinp.c quit.c state.c\
    dmdump.c macparse.c rulparse.c percent.c function.c

# Common Include files.
HDR = dmake.h extern.h struct.h vextern.h patchlvl.h version.h

# Define the TARGET we are making, and where the OBJECT files go.
OBJDIR := objects
TARGET  = dmake$E
CFLAGS += $(SWITCHAR)I.

# Meta rule for making .o's from .c's (give our own so we can move object
# to objects directory in a portable, compiler independent way)
# Define it before the .INCLUDE so that different OS combinations can redefine
# it.
%$O : %.c
.IF $(SHELL) == mpw
    %$(CC) $(CFLAGS) -o :$(OBJDIR:s,/,:,):$@ $<
.ELSE
    %$(CC) $(SWITCHAR)c $(CFLAGS) $<
.IF $(SHELL) != $(COMSPEC)
    mv $(@:f) $(OBJDIR)
.ELSE
    +copy $(@:f) $(OBJDIR)
    +del $(@:f)
.ENDIF
.ENDIF

# Pull in the proper configuration files, based on the value of OS.
.INCLUDE : $(OS)/config.mk
.INCLUDE : dbug/dbug.mk

# Set the .SOURCE targets so that we look for things in the right place.
.SOURCE.c :^ .NULL
.SOURCE.h :^ .NULL
.SOURCE$O :^ $(OBJDIR)
.PRECIOUS : $(HDR)

# Must come after the above INCLUDE so that it gets ALL objects.
OBJECTS	:= {$(ASRC:b) $(SRC:b)}$O

# The main target, make sure the objects directory exists first.
# LDARGS is defined in config.mk file of each OS/OSRELEASE combination.
all        : $(TARGET) $(MKCONFIG);
$(TARGET)  : $(OBJDIR)
$(TARGET)  : $(OBJECTS);$(LD) $(LDARGS)

# Use this for install targets
.IF $(SHELL) == mpw
    $(MKCONFIG) : template.mk
    duplicate :$(<:s,/,:,) $@
.ELSE
    $(MKCONFIG) : template.mk
    $(eq,$(SHELL),$(COMSPEC) +copy cp) $< $@
.ENDIF

# how to make public.h
public .PHONY : $(PUBLIC);
$(PUBLIC) .GROUP .NOSTATE: $(SRC)
    genpub -n DMAKE $< >$@
#	drcsclean ./rcsclean.awk $@ > /dev/null

# Other obvious targets...
.IF $(SHELL) == mpw
    $(OBJDIR):;-newfolder $@
.ELSE
    $(OBJDIR):;-$(eq,$(SHELL),$(COMSPEC) +md mkdir) $@
.ENDIF

# remaining dependencies should be automatically generated
sysintf$O  : sysintf.h
ruletab$O  : startup.h  #khc 01NOV90 - dependency was missing
$(OBJECTS) : $(HDR)

clean:;+- $(RM:f) -rf dmake$E dbdmake$E objects* _*state*.mk startup/config.mk

# Rules for making the manual pages.
man .SETDIR=man : dmake.nc dmake.uue ;
dmake.nc : dmake.p ; scriptfix < $< > $@
dmake.p  : dmake.tf; groff -man -Tascii $< > $@
dmake.uue : dmake.p
    compress -b 12 dmake.p
    mv dmake.p.Z dmake.Z
    uuencode dmake.Z dmake.Z >dmake.uue
    /bin/rm -f dmake.Z

template.mk ".SETDIR=$(ENVDIR)" .USESHELL .MKDIR : $$(TMD)startup/template.mk
    cat $< |\
    sed -e 's/xxOSxx/$(OS)/' |\
    sed -e 's/xxOSRELEASExx/$(OSRELEASE)/' |\
    sed -e 's/xxOSENVIRONMENTxx/$(OSENVIRONMENT)/' > $@

#--------------------------------------------------------------------------
# Make the various archives for shipping the thing around.
#
archives : zip tar
    $(RM) -rf src-list dmake

zip  .PHONY : dmake.zip ;
shar .PHONY : dmake.shar;
tar  .PHONY : dmake.tar;

dmake.zip  : dir-copy
    zip -r $(@:b) $(@:b)

dmake.shar : dir-copy
    find dmake -type f -print >src-list
    xshar -vc -o$@ -L40 `cat src-list`

dmake.tar : dir-copy
    tar cf $@ dmake

dir-copy .PHONY : src-list
[
    echo 'tmp.tar .SILENT :$$(ALLSRC) ;tar -cf tmp.tar $$(ALLSRC)' >> $<
    $(MAKECMD) -f $< tmp.tar
    mkdir dmake
    cd dmake
    tar xf ../tmp.tar; chmod -R u+rw .
    cd ..
    /bin/rm -f tmp.tar
]

src-list : clean
    echo 'MAXLINELENGTH := 65536' > $@
    echo 'ALLSRC = \' >>$@
    find . -type f -print |\
    sed -e 's/,v//'\
        -e 's/$$/\\/'\
        -e 's/^\.\// /'|\
    sort -u |\
    grep -v tst | grep -v $@ | grep -v license |\
    grep -v CVS | grep -v RCS |\
    grep -v '\.zip' | grep -v '\.tar'| grep -v '\.shar' >> $@
    echo ' readme/license.txt' >> $@

#--------------------------------------------------------------------------
# This section can be used to make the necessary script files so that dmake
# can be bootstrapped.
#
#	dmake scripts	-- makes all the script files at once.
#
FIX-SH  = $(SH:s,fix/,,)
FIX95-SH  = $(SH:s,fix95nt/,,)
SH      = $(SH_n:s/c40d/cd/:s/c50d/cd/:s/c51d/cd/:s/c60d/cd/)
SH_n    = $(@:s/swp-/-/:s,-,/,:s/scripts/${SCRIPTFILE}/)
MS      = MAKESTARTUP=startup/startup.mk
FS     := "SHELL := $(SHELL)" "SHELLFLAGS := -ce"
SET-TMP:= TMPDIR:=/tmp

scripts: unix-scripts\
     atari-tos-scripts apple-mac-scripts qssl-qnx-scripts\
     msdos-scripts win95-scripts os2-scripts;

# To add a new environment for UNIX, simply create the appropriate entry
# in the style below for the macro which contains the OS, OSRELEASE and
# OSENVIRONMENT flags.  Then add the entry as a recipe line for the target
# unix-scripts.
#
unix-bsd43-scripts-flags = OS=unix OSRELEASE=bsd43  OSENVIRONMENT=
unix-linux-gnu-scripts-flags  = OS=unix OSRELEASE=linux  OSENVIRONMENT=gnu
unix-solaris-scripts-flags = OS=unix OSRELEASE=solaris OSENVIRONMENT=
unix-solaris-gnu-scripts-flags = OS=unix OSRELEASE=solaris OSENVIRONMENT=gnu
unix-bsd43-uw-scripts-flags= OS=unix OSRELEASE=bsd43  OSENVIRONMENT=uw
unix-bsd43-vf-scripts-flags= OS=unix OSRELEASE=bsd43  OSENVIRONMENT=vf
unix-sysvr4-scripts-flags  = OS=unix OSRELEASE=sysvr4 OSENVIRONMENT=
unix-sysvr3-scripts-flags  = OS=unix OSRELEASE=sysvr3 OSENVIRONMENT=
unix-sysvr3-pwd-scripts-flags  = OS=unix OSRELEASE=sysvr3 OSENVIRONMENT=pwd
unix-xenix-scripts-flags  = OS=unix OSRELEASE=xenix OSENVIRONMENT=
unix-xenix-pwd-scripts-flags  = OS=unix OSRELEASE=xenix OSENVIRONMENT=pwd
unix-sysvr1-scripts-flags  = OS=unix OSRELEASE=sysvr1 OSENVIRONMENT=
unix-386ix-scripts-flags   = OS=unix OSRELEASE=386ix  OSENVIRONMENT=
unix-coherent-ver40-scripts-flags= OS=unix OSRELEASE=coherent OSENVIRONMENT=ver40
unix-coherent-ver42-scripts-flags= OS=unix OSRELEASE=coherent OSENVIRONMENT=ver42
unix-macosx-gnu-scripts-flags  = OS=unix OSRELEASE=macosx OSENVIRONMENT=gnu
qssl--scripts-flags        = OS=qssl OSRELEASE=       OSENVIRONMENT=
tos--scripts-flags         = OS=tos  OSRELEASE=       OSENVIRONMENT=
mac--scripts-flags         = OS=mac  OSRELEASE=       OSENVIRONMENT=

unix-scripts .SWAP : clean
    $(MAKE) SCRIPTFILE=make.sh unix-bsd43-scripts
    $(MAKE) SCRIPTFILE=make.sh unix-bsd43-uw-scripts
    $(MAKE) SCRIPTFILE=make.sh unix-bsd43-vf-scripts
    $(MAKE) SCRIPTFILE=make.sh unix-linux-gnu-scripts
    $(MAKE) SCRIPTFILE=make.sh unix-solaris-scripts
    $(MAKE) SCRIPTFILE=make.sh unix-solaris-gnu-scripts
    $(MAKE) SCRIPTFILE=make.sh unix-sysvr4-scripts
    $(MAKE) SCRIPTFILE=make.sh unix-sysvr3-scripts
    $(MAKE) SCRIPTFILE=make.sh unix-sysvr3-pwd-scripts
    $(MAKE) SCRIPTFILE=make.sh unix-xenix-scripts
    $(MAKE) SCRIPTFILE=make.sh unix-xenix-pwd-scripts
    $(MAKE) SCRIPTFILE=make.sh unix-sysvr1-scripts
    $(MAKE) SCRIPTFILE=make.sh unix-386ix-scripts
    $(MAKE) SCRIPTFILE=make.sh unix-coherent-ver40-scripts
    $(MAKE) SCRIPTFILE=make.sh unix-coherent-ver42-scripts
    $(MAKE) SCRIPTFILE=make.sh unix-macosx-gnu-scripts

atari-tos-scripts .SWAP : clean
    $(MAKE) SCRIPTFILE=make.sh tos--scripts

qssl-qnx-scripts .SWAP : clean
    $(MAKE) SCRIPTFILE=make.sh qssl--scripts

apple-mac-scripts .SWAP : clean
    $(MAKE) SCRIPTFILE=make.sh mac--scripts

unix-%-scripts .SWAP :
    $(MAKECMD) -su $($@-flags) .KEEP_STATE:= $(FS) public template.mk
    $(MAKECMD) -ns .KEEP_STATE:= $(MS) $($@-flags) >/tmp/dmscr
    dfold </tmp/dmscr >$(SH)

qssl-%-scripts .SWAP :
    $(MAKECMD) -su $($@-flags) .KEEP_STATE:= $(FS) public template.mk
    $(MAKECMD) -ns .KEEP_STATE:= $(MS) $($@-flags) >/tmp/dmscr
    dfold </tmp/dmscr >$(SH)

tos-%-scripts .SWAP :
    $(MAKECMD) -su $($@-flags) .KEEP_STATE:= $(FS) public template.mk
    $(MAKECMD) -ns .KEEP_STATE:= $(MS) $($@-flags) >/tmp/dmscr
    dfold </tmp/dmscr >$(SH)

mac-%-scripts .SWAP :
    $(MAKECMD) -su $($@-flags) .KEEP_STATE:= $(FS) public template.mk
    $(MAKECMD) -ns .KEEP_STATE:= $(MS) $($@-flags) >$(SH)
    sed 's/ mac\/\(.*\)$$/ :mac:\1/' <$(SH) | dfold >/tmp/dmscr
    /bin/mv /tmp/dmscr $(SH)

# We make the standard dos scripts here, but we have to go and fix up the
# mkXX.bat file since it contains names of temporary files for the response
# files required by the linker.  We need to also construct the response file
# contents.  These two functions are performed by the fix-msdos-%-scripts
# meta-target.
#
# To add a new DOS environment just do what is described for adding a new
# unix environment, and then make certain that the fix-msdos-%-scripts target
# performs the correct function for the new environment.
msdos-cf = OS=msdos
win95-cf = OS=win95
winnt-cf = OS=winnt

msdos-borland-tcc20swp-scripts-flags = $(msdos-cf) OSRELEASE=borland OSENVIRONMENT=tcc20
msdos-borland-bcc30-scripts-flags = $(msdos-cf) OSRELEASE=borland OSENVIRONMENT=bcc30 SWAP=n
msdos-borland-bcc30swp-scripts-flags = $(msdos-cf) OSRELEASE=borland OSENVIRONMENT=bcc30
msdos-borland-bcc40swp-scripts-flags = $(msdos-cf) OSRELEASE=borland OSENVIRONMENT=bcc40
msdos-borland-bcc45swp-scripts-flags = $(msdos-cf) OSRELEASE=borland OSENVIRONMENT=bcc45
msdos-borland-bcc50swp-scripts-flags = $(msdos-cf) OSRELEASE=borland OSENVIRONMENT=bcc50
msdos-borland-bcc32-scripts-flags = $(msdos-cf) OSRELEASE=borland OSENVIRONMENT=bcc32 SWAP=n
msdos-microsft-msc51-scripts-flags= $(msdos-cf) OSRELEASE=microsft SWAP=n MSC_VER=5.1 OSENVIRONMENT=msc51
msdos-microsft-msc51swp-scripts-flags = $(msdos-cf) OSRELEASE=microsft MSC_VER=5.1 OSENVIRONMENT=msc51
msdos-microsft-msc60-scripts-flags= $(msdos-cf) OSRELEASE=microsft SWAP=n MSC_VER=6.0 OSENVIRONMENT=msc60
msdos-microsft-msc60swp-scripts-flags = $(msdos-cf) OSRELEASE=microsft MSC_VER=6.0 OSENVIRONMENT=msc60
msdos-zortech-scripts-flags= $(msdos-cf) OSRELEASE=zortech SWAP=n OSENVIRONMENT=
msdos-zortechswp-scripts-flags= $(msdos-cf) OSRELEASE=zortech OSENVIRONMENT=
win95-borland-bcc50-scripts-flags = $(win95-cf) OSRELEASE=borland OSENVIRONMENT=bcc50 SWAP=n
win95-microsft-vpp40-scripts-flags = $(win95-cf) OSRELEASE=microsft OSENVIRONMENT=vpp40 SWAP=n

winnt-borland-bcc50-scripts-flags = $(winnt-cf) OSRELEASE=borland OSENVIRONMENT=bcc50 SWAP=n
winnt-microsft-vpp40-scripts-flags = $(winnt-cf) OSRELEASE=microsft OSENVIRONMENT=vpp40 SWAP=n 


msdos-scripts: clean\
           msdos-borland-turbo-scripts\
           msdos-borland-c++-scripts\
           msdos-microsoft-scripts;

win95-scripts: clean\
           win95-borland-c++-scripts\
           win95-microsft-vc++-scripts;

winnt-scripts: clean\
           winnt-borland-c++-scripts\
           winnt-microsft-vc++-scripts;

msdos-borland-turbo-scripts .SWAP :
    $(MAKECMD) SCRIPTFILE=mkswp.bat msdos-borland-tcc20swp-scripts

msdos-borland-c++-scripts .SWAP :! 30 40 45 50
    $(MAKECMD) SCRIPTFILE=mkswp.bat msdos-borland-bcc$?swp-scripts

msdos-microsoft-scripts .SWAP :! 51 60
    $(MAKECMD) SCRIPTFILE=mk.bat msdos-microsft-msc$?-scripts
    $(MAKECMD) SCRIPTFILE=mkswp.bat msdos-microsft-msc$?swp-scripts

msdos-zortech-scripts .SWAP :
    $(MAKECMD) SCRIPTFILE=mk.bat msdos-zortech-scripts
    $(MAKECMD) SCRIPTFILE=mkswp.bat msdos-zortechswp-scripts

win95-borland-c++-scripts .SWAP :! 50
    $(MAKECMD) SCRIPTFILE=mk.bat win95-borland-bcc$?-scripts

win95-microsft-vc++-scripts .SWAP :! 40
    $(MAKECMD) SCRIPTFILE=mk.bat win95-microsft-vpp$?-scripts

winnt-borland-c++-scripts .SWAP :! 50
    $(MAKECMD) SCRIPTFILE=mk.cmd winnt-borland-bcc$?-scripts

winnt-microsft-vc++-scripts .SWAP :! 40
    $(MAKECMD) SCRIPTFILE=mk.cmd winnt-microsft-vpp$?-scripts

msdos-%-scripts .SWAP .SILENT:
    $(MAKECMD) -su $($@-flags) .KEEP_STATE:= $(FS) public template.mk
    $(MAKECMD) -ns DIRSEPSTR:=$(DIRSEPSTR) SHELL=command.com COMSPEC=command.com .KEEP_STATE:= $(MS) $($@-flags) >$(SH)
    $(MAKECMD) -s $(MAKEMACROS) $(MS) $($@-flags) $(SET-TMP) fix-msdos-$*-scripts

win95-borland-%-scripts .SWAP .SILENT:
    $(MAKECMD) -u $($@-flags) .KEEP_STATE:= $(FS) public template.mk
    $(MAKECMD) -n DIRSEPSTR:=$(DIRSEPSTR) SHELL=command.com COMSPEC=command.com .KEEP_STATE:= $(MS) $($@-flags) >$(SH)
    $(MAKECMD) -s $(MAKEMACROS) $(MS) $($@-flags) $(SET-TMP) fix-win95-borland-$*-scripts

win95-microsft-%-scripts .SWAP .SILENT:
    $(MAKECMD) -su $($@-flags) .KEEP_STATE:= $(FS) public template.mk
    $(MAKECMD) -ns DIRSEPSTR:=$(DIRSEPSTR) SHELL=command.com COMSPEC=command.com .KEEP_STATE:= $(MS) $($@-flags) >$(SH)
    $(MAKECMD) -s $(MAKEMACROS) $(MS) $($@-flags) $(SET-TMP) fix95nt-win95-microsft-$*-scripts

winnt-borland-%-scripts .SWAP .SILENT:
    $(MAKECMD) -su $($@-flags) .KEEP_STATE:= $(FS) public template.mk
    $(MAKECMD) -ns DIRSEPSTR:=$(DIRSEPSTR) SHELL=cmd.exe COMSPEC=cmd.exe .KEEP_STATE:= $(MS) $($@-flags) >$(SH)
    $(MAKECMD) -s $(MAKEMACROS) $(MS) $($@-flags) $(SET-TMP) fix-winnt-borland-$*-scripts

winnt-microsft-%-scripts .SWAP .SILENT:
    $(MAKECMD) -su $($@-flags) .KEEP_STATE:= $(FS) public template.mk
    $(MAKECMD) -ns DIRSEPSTR:=$(DIRSEPSTR) SHELL=cmd.exe COMSPEC=cmd.exe .KEEP_STATE:= $(MS) $($@-flags) >$(SH)
    $(MAKECMD) -s $(MAKEMACROS) $(MS) $($@-flags) $(SET-TMP) fix95nt-winnt-microsft-$*-scripts


# We make the standard OS/2 scripts here, but we have to go and fix up the
# mkXX.cmd file since it contains names of temporary files for the response
# files required by the linker.  We need to also construct the response file
# contents.  These two functions are performed by the fix-msdos-%-scripts
# meta-target.
#
# To add a new OS/2 environment just do what is described for adding a new
# unix environment, and then make certain that the fix-msdos-%-scripts target
# performs the correct function for the new environment.
os2-cf = OS=os2
os2-ibm-icc-scripts-flags= $(os2-cf) OSRELEASE=ibm OSENVIRONMENT=icc

os2-scripts: clean os2-ibm-scripts;

os2-ibm-scripts .SWAP :! icc
    $(MAKECMD) SCRIPTFILE=mk.cmd os2-ibm-$?-scripts

os2-%-scripts .SWAP :
    $(MAKECMD) -su $($@-flags) .KEEP_STATE:= $(FS) public template.mk
    $(MAKECMD) -ns DIRSEPSTR:=$(DIRSEPSTR) SHELL=cmd.exe COMSPEC=cmd.exe .KEEP_STATE:= $(MS) SWITCHAR:=- $($@-flags) >$(SH)
    $(MAKECMD) -s $(MAKEMACROS) $(MS) $($@-flags) $(SET-TMP) fix-os2-$*-scripts
    cat $(SH) | sed -e 's, -, /,g' >tmp-out
    mv tmp-out $(SH)

# Signify NULL targets for the various Compiler versions.
icc 30 40 45 50 51 60 .PHONY:;

# Go over the created script file and make sure all the '/' that are in
# filenames are '\', and make sure the final link command line looks
# reasonable.
MAPOBJ   = obj$(SWAP:s/y/swp/:s/n//).rsp
MAPLIB   = lib$(SWAP:s/y/swp/:s/n//).rsp
OBJRSP   = $(SH:s,fix/,,:s,${SCRIPTFILE},${MAPOBJ},)
LIBRSP   = $(SH:s,fix/,,:s,${SCRIPTFILE},${MAPLIB},)
OBJRSP95 = $(SH:s,fix95nt/,,:s,${SCRIPTFILE},${MAPOBJ},)
LIBRSP95 = $(SH:s,fix95nt/,,:s,${SCRIPTFILE},${MAPLIB},)
DOSOBJ   = $(CSTARTUP) $(OBJDIR)/{$(OBJECTS)}

# Use group recipes, as SHELL has an invalid value in some platform instances.
fix-%-scripts .GROUP:
    tac $(FIX-SH) >tmp-sh-r
    tail +3 tmp-sh-r | sed -e 's,/,\\,g' >tmp-out
    tac tmp-out|\
    sed -e 's,\\nologo,/nologo,g' >$(FIX-SH)
    head -2 tmp-sh-r |\
    sed -e 's,\\tmp\\mkA[a-zA-Z0-9]*,$(OBJRSP:s,/,\\),'\
        -e 's,\\tmp\\mkB[a-zA-Z0-9]*,$(LIBRSP:s,/,\\),'\
        -e 's,\\tmp\\mk[0-9]*a[a-z]*,$(OBJRSP:s,/,\\),'\
        -e 's,\\tmp\\mk[0-9]*b[a-z]*,$(LIBRSP:s,/,\\),'\
        -e 's,/,\\,g'\
        -e 's,\\nologo,/nologo,g'\
        -e 's,-,/,g' |\
    tac >>$(FIX-SH)
    rm -f tmp-sh-r tmp-out
    mv <+$(DOSOBJ:s,/,\\,:t"+\n")\n+> $(OBJRSP)
    mv <+$(LDLIBS:s,/,\\,:t"+\n")\n+> $(LIBRSP)

# Use group recipes, as SHELL has an invalid value in some platform instances.
fix95nt-%-scripts .GROUP:
    tac $(FIX95-SH) >tmp-sh-r
    tail +3 tmp-sh-r | sed -e 's,/,\\,g' >tmp-out
    tac tmp-out|\
    sed -e 's,\\nologo,/nologo,g' >$(FIX95-SH)
    head -2 tmp-sh-r |\
    sed -e 's,\\tmp\\mkA[a-zA-Z0-9]*,$(OBJRSP95:s,/,\\),'\
        -e 's,\\tmp\\mkB[a-zA-Z0-9]*,$(LIBRSP95:s,/,\\),'\
        -e 's,\\tmp\\mk[0-9]*a[a-z]*,$(OBJRSP95:s,/,\\),'\
        -e 's,\\tmp\\mk[0-9]*b[a-z]*,$(LIBRSP95:s,/,\\),'\
        -e 's,/,\\,g'\
        -e 's,\\nologo,/nologo,g'\
        -e 's,-,/,g' |\
    tac >>$(FIX95-SH)
    rm -f tmp-sh-r tmp-out
    mv <+$(DOSOBJ:s,/,\\,:t"\n")\n+> $(OBJRSP95)
    mv <+$(LDLIBS:s,/,\\,:t"\n")\n+> $(LIBRSP95)
