
.IF "$(GUI)"!="UNX"
.IF "$(GUI)"!="MAC"
AWK*=awk
SED*=sed
TYPE*=type
RENAME*=ren
COPY*=copy
COPYUPDATE=/u
MKDIR*=mkdir
MKDIRHIER=$(MKDIR) 
ECHON*=echos
.IF "$(GUI)"=="OS2"
GREP*=echo which grep ?
FIND*=$(SOLARROOT)$/util$/os2$/find.exe
LS*=dir /b
.ENDIF
.IF "$(GUI)"=="WNT"
GREP*=$(ENV_TOOLS)$/grep32.exe
FIND*=$(BUILD_TOOLS)$/find.exe
LS*=$(BUILD_TOOLS)$/ls.exe
GNUCOPY*=$(BUILD_TOOLS)$/cp.exe
#wraper for solenv\bin\mkdir.pl to fix mkdir /p problem
MKDIR=+$(BUILD_TOOLS)$/mkdir.btm
TOUCH*=$(BUILD_TOOLS)$/touch.exe
PERL*=$(BUILD_TOOLS)$/perl.exe
#signing for win32 only
SIGNCODE=$(BUILD_TOOLS)$/PackSign$/signcode
CABARC=$(BUILD_TOOLS)$/PackSign$/cabarc
SIGNTOOL=$(BUILD_TOOLS)$/signtool$/signtool
.ENDIF
.IF "$(GUI)"=="WIN"
# using 32-bit tools 
GREP=$(SOLARROOT)$/util$/nt$/grep32.exe
FIND=$(SOLARROOT)$/util$/nt$/find.exe
LS=$(SOLARROOT)$/util$/nt$/ls.exe
MKDIRHIER=echo v | xcopy nul 
.ENDIF
.ELSE
SED*=sed
AWK=awk
PERL=perl
TYPE=cat
COPY=cp -f
GNUCOPY=cp
TOUCH=touch
RENAME=mv
MKDIR=mkdir
MKDIRHIER=mkdir -p
GREP=grep
FIND=find
LS=ls
ECHON=echo -n
.ENDIF
.ELSE
SED*=sed
AWK=awk
PERL=perl
TYPE=cat
COPY=cp -f
.IF "$(OS)"=="SOLARIS"
GNUCOPY=gnucp
.ELSE			# "$(OS)"=="SOLARIS"
GNUCOPY=cp
.ENDIF			# "$(OS)"=="SOLARIS"
TOUCH=touch
RENAME=mv
MKDIR=mkdir
MKDIRHIER=mkdir -p
GREP=grep
FIND=find
LS=ls
ECHON=echo -n
.ENDIF

RM+=$(RMFLAGS)

.IF "$(GUI)"=="UNX"
NULLDEV=/dev/null
.ELIF "$(GUI)"=="MAC"
NULLDEV=/dev/null
.ELSE
NULLDEV=nul
.ENDIF

