#*******************************************************************
#*
#*    $Workfile:   unx.mk  $
#*
#*    Beschreibung      Compiler und Systemabhaengige
#*                      Entwicklungtools-Definitionen
#*                      Fuer WindowsNT-Envrionments
#*
#*    Ersterstellung    MH 9.2.96
#*    Letzte Aenderung  $Author: svesik $ $Date: 2000-10-25 14:09:29 $
#*    $Revision: 1.3 $
#*
#*    $Logfile:   T:/solar/inc/unx.mkv  $
#*
#*    Copyright (c) 1990-1996, STAR DIVISION
#*
#*******************************************************************


# --- Unix-Environment ---------------------------------------

.IF "$(GUI)" == "UNX"

# --- Ole2 Behandlung -------------

.IF "$(DONT_USE_OLE2)"!=""
OLE2=
OLE2DEF=
.ELSE
OLE2=true
OLE2DEF=-DOLE2
.ENDIF


# Dieses Define gilt fuer alle  WNT- Plattformen
# fuer Compiler, wo kein Java unterstuetzt wird, muss dies ge'undeft werden
#JAVADEF=-DSOLAR_JAVA

# --- Compiler ---

.IF "$(COM)$(OS)$(CPU)" == "C50SOLARISS"
.INCLUDE : unxsols2.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "C50SOLARISI"
.INCLUDE : unxsoli2.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "C60SOLARISS"
.INCLUDE : unxsols3.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCSOLARISS"
.INCLUDE : unxsogs.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCSCOI"
.INCLUDE : unxscoi.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)$(GLIBC)" == "GCCLINUXI"
.INCLUDE : unxlnxi.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)$(GLIBC)" == "GCCLINUXP2"
.INCLUDE : unxlngp.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)$(GLIBC)" == "GCCLINUXI2REDHAT60"
.INCLUDE : unxlngi3.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)$(GLIBC)" == "GCCLINUXP2REDHAT60" 
.INCLUDE : unxlngppc.mk 
.ENDIF 
 
.IF "$(COM)$(OS)$(CPU)" == "ACCHPUXR"
.INCLUDE : unxhpxr.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCHPUXR"
.INCLUDE : unxhpgr.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCNETBSDA"
.INCLUDE : unxbsda.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "ICCS390G"
.INCLUDE : unxmvsg.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "ICCAIXP"
.INCLUDE : unxaixp.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCNETBSDI"
.INCLUDE : unxbsdi.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCFREEBSDI"
.INCLUDE : unxfbsdi.mk
.ENDIF


.IF "$(COM)$(OS)$(CPU)" == "C730IRIXM"
.INCLUDE : unxirxm3.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "C710IRIXM"
.INCLUDE : unxirxm.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCMACOSXP"
.INCLUDE : unxmacxp.mk
.ENDIF

# --- *ix  Allgemein ---
HC=hc
HCFLAGS=
PATH_SEPERATOR*=:
.IF "$(WORK_STAMP)"!="MIX364"
CDEFS+=-D__DMAKE
.ENDIF

CDEFS+=-DUNIX

# fuer linux: bison -y -d
YACC*=yacc
YACCFLAGS*=-d -t

EXECPOST=
SCPPOST=.ins
DLLDEST=$(LB)
.ENDIF
