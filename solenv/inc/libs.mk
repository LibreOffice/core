#*************************************************************************
#
#   $RCSfile: libs.mk,v $
#
#   $Revision: 1.76 $
#
#   last change: $Author: obo $ $Date: 2004-08-12 10:50:16 $
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
LIBSMKREV!:="$$Revision: 1.76 $$"

.IF "$(COM)"=="WTC"
LIBPRE=libr
.ENDIF

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"

#
#externe libs in plattform.mk
#
AVMEDIALIB=-lavmedia$(OFFICEUPD)$(DLLPOSTFIX)
ICUINLIB=-licui18n
ICULELIB=-licule
ICUUCLIB=-licuuc
I18NUTILLIB=-li18nutil$(COMID)
.IF "$(WITH_GPC)"!="NO"
GPC3RDLIB=-lgpc
.ENDIF
SALHELPERLIB=-lsalhelper$(COMID)
XMLSCRIPTLIB =-lxcr$(OFFICEUPD)$(DLLPOSTFIX)
.INCLUDE .IGNORE : comphelper$/version.mk
COMPHELPERLIB=-lcomphelp$(COMPHLP_MAJOR)$(COMID)
CONNECTIVITYLIB=-lconnectivity
LDAPBERLIB=-lldapber
TOOLSLIBST=-latools
DVOSUTILLIBST=-ldvosutil
DUNOUTILLIBST=-ldunoutil
DVCLUTILLIBST=-ldvclutil
BPICONVLIB=-lbpiconv
OSLLIB=-losl$(OFFICEUPD)$(DLLPOSTFIX)
TOOLSLIB=-ltl$(OFFICEUPD)$(DLLPOSTFIX)
OSLLIB=
RTLLIB=
TECLIB=
CPPULIB=-lcppu
CPPUHELPERLIB=-lcppuhelper$(COMID)
.INCLUDE .IGNORE : ucbhelper$/version.mk
UCBHELPERLIB=-lucbhelper$(UCBHELPER_MAJOR)$(COMID)
REGLIB=-lreg
.INCLUDE .IGNORE : vos$/version.mk
VOSLIB=-lvos$(VOS_MAJOR)$(COMID)
XMLOFFLIB=-lxo$(OFFICEUPD)$(DLLPOSTFIX)
XMLOFFLLIB=-lxol
STORELIB=-lstore
SALLIB=-lsal
.INCLUDE .IGNORE : connectivity$/version.mk
ODBCLIB=-lodbc$(ODBC_MAJOR)
ODBCBASELIB=-lodbcbase$(ODBC_MAJOR)
DBFILELIB=-lfile$(OFFICEUPD)$(DLLPOSTFIX)
SALTLSLIB=-lsaltls$(UDK_MAJOR)
SALZIPLIB=-lsalzip$(UDK_MAJOR)
RMCXTLIB=-lrmcxt
BTSTRPLIB=-lbtstrp
TRANSEXLIB=-ltransex
IOTXLIB=
OTXLIB=-lotx_ind
OSXLIB=-losx
UNOLIB=-luno$(OFFICEUPD)$(DLLPOSTFIX)
UNOTOOLSLIB=-lutl$(OFFICEUPD)$(DLLPOSTFIX)
UCRLIB=-lucr$(OFFICEUPD)$(DLLPOSTFIX)
UASLIB=-luas$(OFFICEUPD)$(DLLPOSTFIX)
USRLIB=-lusr$(OFFICEUPD)$(DLLPOSTFIX)
SOTLIB=-lsot$(OFFICEUPD)$(DLLPOSTFIX)
ONELIB=-lone$(OFFICEUPD)$(DLLPOSTFIX)
MOZBASELIBST=$(STATIC) -lnspr4 -lxpcombase_s $(DYNAMIC)
MOZBASELIB=-lnspr4 -lxpcom
LDAPSDKLIB=-lldap50
ONELIB=
UNOLIB=
SVLIBDEPEND=$(L)$/libvcl$(OFFICEUPD)$(DLLPOSTFIX)$(DLLPOST)
SVLIB=-lvcl$(OFFICEUPD)$(DLLPOSTFIX)
ICOLIB=-lico$(OFFICEUPD)$(DLLPOSTFIX)
BSTRPLIB=-lbtstrp
VCLLIBST=-lvcl
VCLLIB=$(SVLIB)
BASEGFXLIB=-lbasegfx$(OFFICEUPD)$(DLLPOSTFIX)
CANVASTOOLSLIB=-lcanvastools$(OFFICEUPD)$(DLLPOSTFIX)
CPPCANVASLIB=-lcppcanvas$(OFFICEUPD)$(DLLPOSTFIX)
FREETYPELIB=-lfreetype
FREETYPELIBST=$(STATIC) -lfreetype $(DYNAMIC)
XPLIB=-l_xp
TKLIB=-ltk$(OFFICEUPD)$(DLLPOSTFIX)
SVTOOLLIB=-lsvt$(OFFICEUPD)$(DLLPOSTFIX)
AUTOMATIONLIB=-lsts$(OFFICEUPD)$(DLLPOSTFIX)
SVLLIB=-lsvl$(OFFICEUPD)$(DLLPOSTFIX)
SVMEMLIB=
SVUNZIPLIB=-lsvunzip
SVUNZIPDLL=-lzip$(OFFICEUPD)$(DLLPOSTFIX)
PLUGCTORLIB=-lplugctor
# PLUGCTORLIB=
SO2LIB=-lso$(OFFICEUPD)$(DLLPOSTFIX)
TKTLIB=-ltkt$(OFFICEUPD)$(DLLPOSTFIX)
SJLIB=-lj$(OFFICEUPD)$(DLLPOSTFIX)_g
GOODIESLIB=-lgo$(OFFICEUPD)$(DLLPOSTFIX)
GOODIESLLIB=-lgol$(OFFICEUPD)$(DLLPOSTFIX)
CHANELLIB=-lcha$(OFFICEUPD)$(DLLPOSTFIX)
MAILLIB=-lmail
DOCMGRLIB=-ldmg$(OFFICEUPD)$(DLLPOSTFIX)
.IF "$(BIG_GOODIES)"!=""
HELPLIB=
CHANELLIB=
.ELSE
HELPLIB=-lhlp$(OFFICEUPD)$(DLLPOSTFIX)
CHANELLIB=-lcha$(OFFICEUPD)$(DLLPOSTFIX)
.ENDIF
BASICLIB=-lsb$(OFFICEUPD)$(DLLPOSTFIX)
BASICLIGHTLIB=-lsbl$(OFFICEUPD)$(DLLPOSTFIX)
SDBLIB=-lsdb$(OFFICEUPD)$(DLLPOSTFIX)
DBTOOLSLIB=-ldbtools$(OFFICEUPD)$(DLLPOSTFIX)
HM2LIBSH=-lhmwrpdll
HM2LIBST=-lhmwrap
LINGULIB=$(HM2LIBST)
LNGLIB=-llng$(OFFICEUPD)$(DLLPOSTFIX)
EXPAT3RDLIB=-lexpat_xmlparse -lexpat_xmltok
EXPATASCII3RDLIB=-lascii_expat_xmlparse -lexpat_xmltok
.IF "$(SYSTEM_ZLIB)"=="YES"
ZLIB3RDLIB=-lz
.ELSE
ZLIB3RDLIB=-lzlib
.ENDIF
JPEG3RDLIB=-ljpeglib
NEON3RDLIB=-lneon
BERKELEYLIB=-ldb-3.2
BERKELEYCPPLIB=-ldb_cxx-3.2
CURLLIB=-lcurl
SFX2LIB=-lsfx$(OFFICEUPD)$(DLLPOSTFIX)
SFXLIB=-lsfx$(OFFICEUPD)$(DLLPOSTFIX)
SFXDEBUGLIB=
FWELIB=-lfwe$(OFFICEUPD)$(DLLPOSTFIX)
FWILIB=-lfwi$(OFFICEUPD)$(DLLPOSTFIX)
SVXLIB=-lsvx$(OFFICEUPD)$(DLLPOSTFIX)
BASCTLLIB=-lbasctl$(OFFICEUPD)$(DLLPOSTFIX)
BASICIDELIB=-lybctl
SVXLLIB=-lsvxl
CHAOSLIB=-lcnt$(OFFICEUPD)$(DLLPOSTFIX)
UUILIB=-luui$(OFFICEUPD)$(DLLPOSTFIX)
DGLIB=
SIMLIB=-lysim
SCHLIB=-lysch
SMLIB=-lysm
SBALIB=-lsbawrap
#SBALIB=-lsba$(OFFICEUPD)$(DLLPOSTFIX)
OFALIB=-lofa$(OFFICEUPD)$(DLLPOSTFIX)
SETUPLIB=-lset$(OFFICEUPD)$(DLLPOSTFIX)
SCHEDLIB=-lss$(OFFICEUPD)$(DLLPOSTFIX)
SSLLIB=-lssl$(OFFICEUPD)$(DLLPOSTFIX)
SSCLIB=-lssc$(OFFICEUPD)$(DLLPOSTFIX)
SSWLIB=-lssw$(OFFICEUPD)$(DLLPOSTFIX)
TRACERLIB=-ltrc$(OFFICEUPD)$(DLLPOSTFIX)
PAPILIB=-lpap$(OFFICEUPD)$(DLLPOSTFIX)
TRCLIB=$(TRACERLIB)
SCLIB=-lsclib
SDLIB=-lsdlib
SDLLIB=-lsdl
SWLIB=-lswlib
ISWLIB=-lsw$(OFFICEUPD)$(DLLPOSTFIX)
ISCLIB=-lsc$(OFFICEUPD)$(DLLPOSTFIX)
ISDLIB=-lsd$(OFFICEUPD)$(DLLPOSTFIX)
PKGCHKLIB=-lpkgchk$(OFFICEUPD)$(DLLPOSTFIX)
SYSSHELLLIB=-lsysshell
SSOOPTLIB=-lssoopt$(OFFICEUPD)$(DLLPOSTFIX)
JVMACCESSLIB = -ljvmaccess$(COMID)
CPPUNITLIB = -lcppunit$(DLLPOSTFIX)
XML2LIB = -lxml2
JVMFWKLIB = -ljvmfwk

# #110743#
# For BinFilters, some libs were added.
#

BFSFXLIB=-lbf_sfx
BFSVXLIB=-lbf_svx$(OFFICEUPD)$(DLLPOSTFIX)
BFSCHLIB=-lbf_ysch
BFSMLIB=-lbf_ysm
BFSCLIB=-lbf_sclib
BFSDLIB=-lbf_sdlib
BFSWLIB=-lbf_swlib
BFOFALIB=-lbf_ofa$(OFFICEUPD)$(DLLPOSTFIX)
LEGACYSMGRLIB=-llegacy_binfilters$(OFFICEUPD)$(DLLPOSTFIX)
BFXMLOFFLIB=-lbf_xo$(OFFICEUPD)$(DLLPOSTFIX)


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
AVMEDIALIB=$(LIBPRE) iavmedia.lib
ICUINLIB=icuin.lib
ICULELIB=icule.lib
ICUUCLIB=icuuc.lib
I18NUTILLIB=$(LIBPRE) ii18nutil.lib
GPC3RDLIB=gpc.lib
SALHELPERLIB=$(LIBPRE) isalhelper.lib
XMLSCRIPTLIB=ixcr.lib
COMPHELPERLIB=icomphelp.lib
CONNECTIVITYLIB=connectivity.lib
LDAPBERLIB=ldapber.lib
CPPULIB=$(LIBPRE) icppu.lib
CPPUHELPERLIB=$(LIBPRE) icppuhelper.lib
SALTLSLIB=isaltls.lib
SALZIPLIB=isalzip.lib
UCBHELPERLIB=$(LIBPRE) iucbhelper.lib
ODBCLIB=$(LIBPRE) iodbc.lib
ODBCBASELIB=$(LIBPRE) iodbcbase.lib
DBFILELIB=$(LIBPRE) ifile$(OFFICEUPD).lib
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
LDAPSDKLIB=$(LIBPRE) nsldap32v50.lib
ONELIB=
UNOLIB=
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
FWILIB=$(LIBPRE) ifwi.lib
BTSTRPLIB=$(LIBPRE) bootstrp.lib
TRANSEXLIB=$(LIBPRE) transex.lib
ICOLIB= $(LIBPRE) icom.lib
SVTOOLLIB=$(LIBPRE) svtool.lib
AUTOMATIONLIB=$(LIBPRE) ists.lib
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
CURLLIB=$(LIBPRE) libcurl.lib
CHAOSLIB=$(LIBPRE) ichaos.lib
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
ISWLIB=$(LIBPRE) _sw.lib
ISCLIB=$(LIBPRE) sci.lib
ISDLIB=$(LIBPRE) sdi.lib
SVLIB=$(LIBPRE) ivcl.lib
VCLLIB=$(SVLIB)
BASEGFXLIB=$(LIBPRE) ibasegfx.lib
CANVASTOOLSLIB=$(LIBPRE) icanvastools.lib
CPPCANVASLIB=$(LIBPRE) icppcanvas.lib
TKLIB=$(LIBPRE) itk.lib
SVLIBDEPEND=$(L)$/ivcl.lib
SVXLLIB=$(LIBPRE) svxl.lib
FREETYPELIB=freetype.lib
FREETYPELIBST=freetype.lib
PKGCHKLIB=$(LIBPRE) ipkgchk.lib
SYSSHELLLIB=sysshell.lib
SSOOPTLIB=issoopt$(OFFICEUPD)$(DLLPOSTFIX).lib
JVMACCESSLIB = $(LIBPRE) ijvmaccess.lib
CPPUNITLIB = cppunit.lib
XML2LIB = $(LIBPRE) libxml2.lib
JVMFWKLIB = $(LIBPRE) ijvmfwk.lib

# #110743#
# For BinFilters, some libs were added.
#

BFSVXLIB=$(LIBPRE) bf_svx.lib
BFSFXLIB=$(LIBPRE) bf_sfx.lib
BFSCHLIB=$(LIBPRE) bf_ysch.lib
BFSMLIB=$(LIBPRE) bf_ysm.lib
BFSCLIB=$(LIBPRE) bf_sclib.lib
BFSDLIB=$(LIBPRE) bf_sdlib.lib
BFSWLIB=$(LIBPRE) bf_swlib.lib
BFOFALIB=$(LIBPRE) bf_ofa.lib
BFXMLOFFLIB=$(LIBPRE) ibf_xo.lib
LEGACYSMGRLIB=$(LIBPRE) ilegacy_binfilters.lib

.ENDIF              # "$(GUI)"=="UNX" || "$(GUI)"=="MAC"

