#*************************************************************************
#
#   $RCSfile: libs.mk,v $
#
#   $Revision: 1.32 $
#
#   last change: $Author: obo $ $Date: 2001-07-25 14:42:42 $
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

.IF "$(COM)"=="WTC"
LIBPRE=libr
.ENDIF

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"

#
#externe libs in plattform.mk
#

GPC3RDLIB=-lgpc
SALHELPERLIB=-lsalhelper$(UDK_MAJOR)$(COM)
XMLSCRIPTLIB =-lxcr$(UPD)$(DLLPOSTFIX)
COMPHELPERLIB=-lcomphelp2
CONNECTIVITYLIB=-lconnectivity
LDAPBERLIB=-lldapber
TOOLSLIBST=-latools
DVOSUTILLIBST=-ldvosutil
DUNOUTILLIBST=-ldunoutil
DVCLUTILLIBST=-ldvclutil
BPICONVLIB=-lbpiconv
OSLLIB=-losl$(UPD)$(DLLPOSTFIX)
TOOLSLIB=-ltl$(UPD)$(DLLPOSTFIX)
OSLLIB=
RTLLIB=
TECLIB=
CPPULIB=-lcppu
CPPUHELPERLIB=-lcppuhelper$(UDK_MAJOR)$(COM)
.INCLUDE .IGNORE : ucbhelper$/version.mk
UCBHELPERLIB=-lucbhelper$(UCBHELPER_MAJOR)$(COM)
REGLIB=-lreg
.INCLUDE .IGNORE : vos$/version.mk
VOSLIB=-lvos$(VOS_MAJOR)$(COM)
XMLOFFLIB=-lxo$(UPD)$(DLLPOSTFIX)
XMLOFFLLIB=-lxol
STORELIB=-lstore
SALLIB=-lsal
.INCLUDE .IGNORE : connectivity$/version.mk
ODBCLIB=-lodbc$(ODBC_MAJOR)
ODBCBASELIB=-lodbcbase$(ODBC_MAJOR)
DBFILELIB=-lfile$(UPD)$(DLLPOSTFIX)
SALTLSLIB=-lsaltls$(UDK_MAJOR)
SALZIPLIB=-lsalzip$(UDK_MAJOR)
RMCXTLIB=-lrmcxt
BTSTRPLIB=-lbtstrp
IOTXLIB=
OTXLIB=-lotx_ind
OSXLIB=-losx
UNOLIB=-luno$(UPD)$(DLLPOSTFIX)
UNOTOOLSLIB=-lutl$(UPD)$(DLLPOSTFIX)
UCRLIB=-lucr$(UPD)$(DLLPOSTFIX)
UASLIB=-luas$(UPD)$(DLLPOSTFIX)
USRLIB=-lusr$(UPD)$(DLLPOSTFIX)
SOTLIB=-lsot$(UPD)$(DLLPOSTFIX)
ONELIB=-lone$(UPD)$(DLLPOSTFIX)
MOZBASELIBST=$(STATIC) -lnspr4 -lxpcombase_s $(DYNAMIC)
MOZBASELIB=-lnspr4 -lxpcom
ONELIB=
UNOLIB=
S2ULIB=-lstu$(UPD)$(DLLPOSTFIX)
SVLIBDEPEND=$(L)$/libvcl$(UPD)$(DLLPOSTFIX)$(DLLPOST)
SVLIB=-lvcl$(UPD)$(DLLPOSTFIX)
ICOLIB=-lico$(UPD)$(DLLPOSTFIX)
BSTRPLIB=-lbtstrp
VCLLIBST=-lvcl
VCLLIB=$(SVLIB)
FREETYPELIB=-lfreetype
FREETYPELIBST=$(STATIC) -lfreetype $(DYNAMIC)
XPLIB=-l_xp
TKLIB=-ltk$(UPD)$(DLLPOSTFIX)
SVTOOLLIB=-lsvt$(UPD)$(DLLPOSTFIX)
AUTOMATIONLIB=-lsts$(UPD)$(DLLPOSTFIX)
SVLLIB=-lsvl$(UPD)$(DLLPOSTFIX)
SVMEMLIB=
SVUNZIPLIB=-lsvunzip
SVUNZIPDLL=-lzip$(UPD)$(DLLPOSTFIX)
PLUGCTORLIB=-lplugctor
# PLUGCTORLIB=
INETLIB=-lni$(UPD)$(DLLPOSTFIX)
INETLIBSH=-linetdll
INETLIBST=-linetwrap
SO2LIB=-lso$(UPD)$(DLLPOSTFIX)
TKTLIB=-ltkt$(UPD)$(DLLPOSTFIX)
SJLIB=-lj$(UPD)$(DLLPOSTFIX)_g
GOODIESLIB=-lgo$(UPD)$(DLLPOSTFIX)
GOODIESLLIB=-lgol$(UPD)$(DLLPOSTFIX)
CHANELLIB=-lcha$(UPD)$(DLLPOSTFIX)
MAILLIB=-lmail
DOCMGRLIB=-ldmg$(UPD)$(DLLPOSTFIX)
.IF "$(BIG_GOODIES)"!=""
HELPLIB=
CHANELLIB=
.ELSE
HELPLIB=-lhlp$(UPD)$(DLLPOSTFIX)
CHANELLIB=-lcha$(UPD)$(DLLPOSTFIX)
.ENDIF
BASICLIB=-lsb$(UPD)$(DLLPOSTFIX)
BASICLIGHTLIB=-lsbl$(UPD)$(DLLPOSTFIX)
SDBLIB=-lsdb$(UPD)$(DLLPOSTFIX)
DBTOOLSLIB=-ldbtools$(DBTOOLS_MAJOR)
SYNCACCLIB=-lsyncacc1
HM2LIBSH=-lhmwrpdll
HM2LIBST=-lhmwrap
LINGULIB=$(HM2LIBST)
LNGLIB=-llng$(UPD)$(DLLPOSTFIX)
EXPAT3RDLIB=-lexpat_xmlparse -lexpat_xmltok
EXPATASCII3RDLIB=-lascii_expat_xmlparse -lexpat_xmltok
ZLIB3RDLIB=-lzlib
JPEG3RDLIB=-ljpeglib
NEON3RDLIB=-lneon
BERKELEYLIB=-ldb-3.2
BERKELEYCPPLIB=-ldb_cxx-3.2
SFX2LIB=-lsfx$(UPD)$(DLLPOSTFIX)
SFXLIB=-lsfx$(UPD)$(DLLPOSTFIX)
SFXDEBUGLIB=
FWELIB=-lfwe$(UPD)$(DLLPOSTFIX)
SVXLIB=-lsvx$(UPD)$(DLLPOSTFIX)
BASCTLLIB=-lbasctl$(UPD)$(DLLPOSTFIX)
BASICIDELIB=-lybctl
SVXLLIB=-lsvxl
.IF "$(OLD_CHAOS)"==""
CHAOSLIB=-lcnt$(UPD)$(DLLPOSTFIX)
.ENDIF
UUILIB=-luui$(UPD)$(DLLPOSTFIX)
DGLIB=
SIMLIB=-lysim
SCHLIB=-lysch
SMLIB=-lysm
SBALIB=-lsbawrap
#SBALIB=-lsba$(UPD)$(DLLPOSTFIX)
OFALIB=-lofa$(UPD)$(DLLPOSTFIX)
SETUPLIB=-lset$(UPD)$(DLLPOSTFIX)
SCHEDLIB=-lss$(UPD)$(DLLPOSTFIX)
SSLLIB=-lssl$(UPD)$(DLLPOSTFIX)
SSCLIB=-lssc$(UPD)$(DLLPOSTFIX)
SSWLIB=-lssw$(UPD)$(DLLPOSTFIX)
TRACERLIB=-ltrc$(UPD)$(DLLPOSTFIX)
PAPILIB=-lpap$(UPD)$(DLLPOSTFIX)
TRCLIB=$(TRACERLIB)
SCLIB=-lsclib
SDLIB=-lsdlib
SDLLIB=-lsdl
SWLIB=-lswlib

#
# USED_%NAME%_LIBS
# Variablen, in denen in gueltiger Reihenfolge ALLE Libraries,
# die unterhalb von %NAME% liegen, zusammengefasst werden
#

# Libraries
USED_OSL_LIBS =
USED_VOS_LIBS =		$(OSLLIB)
USED_UNO_LIBS =		$(VOSLIB) $(OSLLIB)
USED_TOOLS_LIBS =
USED_SOT_LIBS = 	$(TOOLSLIB)
USED_VCL_LIBS =		$(SOTLIB) $(TOOLSLIB) $(USED_UNO_LIBS)

# Applikationen
USED_BOOTSTRP_LIBS= $(TOOLSLIB)
USED_RCLIENT_LIBS =	$(VCLLIB) $(SOTLIB) $(TOOLSLIB) $(UNOLIB) \
                    $(USED_UNO_LIBS)

.ELSE				# "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
GPC3RDLIB=gpc.lib
SALHELPERLIB=$(LIBPRE) isalhelper.lib
XMLSCRIPTLIB=ixcr.lib
COMPHELPERLIB=icomphelp2.lib
CONNECTIVITYLIB=connectivity.lib
LDAPBERLIB=ldapber.lib
CPPULIB=$(LIBPRE) icppu.lib
CPPUHELPERLIB=$(LIBPRE) icppuhelper.lib
SALTLSLIB=isaltls.lib
SALZIPLIB=isalzip.lib
UCBHELPERLIB=$(LIBPRE) iucbhelper.lib
ODBCLIB=$(LIBPRE) iodbc.lib
ODBCBASELIB=$(LIBPRE) iodbcbase.lib
DBFILELIB=$(LIBPRE) ifile$(UPD).lib
TOOLSLIB=$(LIBPRE)itools.lib
TOOLSLIBST=$(LIBPRE) atools.lib
DVOSUTILLIBST=$(LIBPRE) dvosutil.lib
DUNOUTILLIBST=$(LIBPRE) dunoutil.lib
DVCLUTILLIBST=$(LIBPRE) dvclutil.lib
BPICONVLIB=$(LIBPRE) bpiconv.lib
SALLIB=$(LIBPRE) isal.lib
OSLLIB=
RTLLIB=
TECLIB=
VOSLIB=$(LIBPRE) ivos.lib
UNOLIB=$(LIBPRE) iuno.lib
UNOTOOLSLIB=$(LIBPRE) iutl.lib
RMCXTLIB=$(LIBPRE) irmcxt.lib
XMLOFFLIB=$(LIBPRE) ixo.lib
XMLOFFLLIB=$(LIBPRE) xol.lib
STORELIB=$(LIBPRE) istore.lib
IOTXLIB=$(LIBPRE) iotx.lib
OTXLIB=$(LIBPRE) otx_ind.lib
OSXLIB=$(LIBPRE) osx.lib
REGLIB=$(LIBPRE) ireg.lib
UCRLIB=$(LIBPRE) ucr.lib
UASLIB=$(LIBPRE) uas.lib
EXTLIB=$(LIBPRE) iext.lib
USRLIB=$(LIBPRE) usr.lib
SOTLIB=$(LIBPRE) sot.lib
ONELIB=$(LIBPRE) one.lib
MOZBASELIBST=$(LIBPRE) nspr4_s.lib $(LIBPRE) xpcombase_s.lib
MOZBASELIB=$(LIBPRE) nspr4.lib $(LIBPRE) xpcom.lib
ONELIB=
UNOLIB=
S2ULIB=$(LIBPRE) stu.lib
PAPILIB=$(LIBPRE) ipap.lib
TRACERLIB=$(LIBPRE) itrc.lib
TRCLIB=$(TRACERLIB)
SETUPLIB=$(LIBPRE) iset.lib
SCHEDLIB=$(LIBPRE) ss.lib
SSCLIB=$(LIBPRE) ssc.lib
SSLLIB=$(LIBPRE) ssl.lib
SSWLIB=$(LIBPRE) ssw.lib
SVLIBDEPEND=$(L)$/sv.lib
SFX2LIB=$(LIBPRE) sfx.lib
SFXLIB=$(SFX2LIB)
FWELIB=$(LIBPRE) ifwe.lib
BTSTRPLIB=$(LIBPRE) bootstrp.lib
ICOLIB= $(LIBPRE) icom.lib
SVTOOLLIB=$(LIBPRE) svtool.lib
AUTOMATIONLIB=$(LIBPRE) automation.lib
SVLLIB=$(LIBPRE) isvl.lib
PLUGCTORLIB=$(LIBPRE) plugctor.lib
PLUGAPPLIB=$(LIBPRE) plugapp.lib
SVMEMLIB=$(LIBPRE) svmem.lib
SVUNZIPLIB=$(LIBPRE) svunzip.lib
SVUNZIPDLL=$(LIBPRE) unzipdll.lib
GOODIESLIB=$(LIBPRE) igo.lib
GOODIESLLIB=$(LIBPRE) igol.lib
MAILLIB=$(LIBPRE) mail.lib
DOCMGRLIB=$(LIBPRE) docmgr.lib
.IF "$(BIG_GOODIES)"!=""
HELPLIB=
CHANELLIB=
.ELSE
HELPLIB=$(LIBPRE) hlp.lib
CHANELLIB=$(LIBPRE) ich.lib
.ENDIF
INETLIBSH=$(LIBPRE) inetdll.lib
INETLIBST=$(LIBPRE) inetwrap.lib
BASICLIB=$(LIBPRE) basic.lib
BASICLIGHTLIB=$(LIBPRE) basicl.lib
SO2LIB=$(LIBPRE) so2.lib
TKTLIB=$(LIBPRE) tkt.lib
SJLIB=$(LIBPRE) sj.lib
SVXLIB=$(LIBPRE) svx.lib
SVXLIB+=$(LIBPRE) dl.lib
BASCTLLIB=$(LIBPRE) basctl.lib
BASICIDELIB=ybctl.lib
SVXLLIB=$(LIBPRE) svxl.lib
SDBLIB=$(LIBPRE) sdb.lib
DBTOOLSLIB=$(LIBPRE) idbtools.lib
SYNCACCLIB=$(LIBPRE) isyncacc1.lib
HM2LIBSH=$(LIBPRE) hmwrpdll.lib
HM2LIBST=$(LIBPRE) hmwrap.lib
LINGULIB=$(HM2LIBST)
LNGLIB=$(LIBPRE) ilng.lib
EXPAT3RDLIB=$(LIBPRE) expat_xmltok.lib $(LIBPRE) expat_xmlparse.lib
EXPATASCII3RDLIB=$(LIBPRE) expat_xmltok.lib $(LIBPRE) ascii_expat_xmlparse.lib
ZLIB3RDLIB=$(LIBPRE) zlib.lib
JPEG3RDLIB=$(LIBPRE) jpeglib.lib
NEON3RDLIB=$(LIBPRE) neon.lib
BERKELEYLIB=$(LIBPRE) libdb32.lib
BERKELEYCPPLIB=
.IF "$(OLD_CHAOS)"==""
CHAOSLIB=$(LIBPRE) ichaos.lib
.ENDIF
UUILIB=$(LIBPRE) iuui.lib
DGLIB=
SIMLIB=$(LIBPRE) ysim.lib
SCHLIB=$(LIBPRE) ysch.lib
SMLIB=$(LIBPRE) ysm.lib
SBALIB=$(LIBPRE) sbawrap.lib
OFALIB=$(LIBPRE) aofa.lib
SCLIB=$(LIBPRE) sclib.lib
SDLIB=$(LIBPRE) sdlib.lib
SDLLIB=$(LIBPRE) sdl.lib
SWLIB=$(LIBPRE) swlib.lib
SVLIB=$(LIBPRE) ivcl.lib
VCLLIB=$(SVLIB)
TKLIB=$(LIBPRE) itk.lib
SVLIBDEPEND=$(L)$/ivcl.lib
SVXLLIB=$(LIBPRE) svxl.lib
FREETYPELIB=freetype.lib
FREETYPELIBST=freetype.lib

.ENDIF              # "$(GUI)"=="UNX" || "$(GUI)"=="MAC"

