#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: libs.mk,v $
#
#   $Revision: 1.105 $
#
#   last change: $Author: kz $ $Date: 2006-11-08 12:06:46 $
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
LIBSMKREV!:="$$Revision: 1.105 $$"

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"

#
#externe libs in plattform.mk
#
AWTLIB*=-ljawt
AVMEDIALIB=-lavmedia$(OFFICEUPD)$(DLLPOSTFIX)
ICUINLIB=-licui18n
ICULELIB=-licule
ICUUCLIB=-licuuc
I18NUTILLIB=-li18nutil$(COMID)
.INCLUDE .IGNORE : i18npool$/version.mk
I18NISOLANGLIB=-li18nisolang$(ISOLANG_MAJOR)$(COMID)
.IF "$(WITH_GPC)"!="NO"
GPC3RDLIB=-lgpc
.ENDIF
SALHELPERLIB=-luno_salhelper$(COMID)
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
TOOLSLIB=-ltl$(OFFICEUPD)$(DLLPOSTFIX)
# make sure some ancient libs are empty
OSLLIB=
RTLLIB=
TECLIB=
CPPULIB=-luno_cppu
CPPUHELPERLIB=-luno_cppuhelper$(COMID)
.INCLUDE .IGNORE : ucbhelper$/version.mk
UCBHELPERLIB=-lucbhelper$(UCBHELPER_MAJOR)$(COMID)
REGLIB=-lreg
.INCLUDE .IGNORE : vos$/version.mk
VOSLIB=-lvos$(VOS_MAJOR)$(COMID)
XMLOFFLIB=-lxo$(OFFICEUPD)$(DLLPOSTFIX)
XMLOFFLLIB=-lxol
STORELIB=-lstore
SALLIB=-luno_sal
.INCLUDE .IGNORE : connectivity$/version.mk
ODBCLIB=-lodbc$(ODBC_MAJOR)
ODBCBASELIB=-lodbcbase$(ODBC_MAJOR)
DBFILELIB=-lfile$(OFFICEUPD)$(DLLPOSTFIX)
RMCXTLIB=-lrmcxt
BTSTRPLIB=-lbtstrp
BTSTRPDTLIB=-lbootstrpdt$(OFFICEUPD)$(DLLPOSTFIX)
SOLDEPLIB=-lsoldep$(OFFICEUPD)$(DLLPOSTFIX)
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
.IF "$(WITH_OPENLDAP)" == "YES"
LDAPSDKLIB=-lldap
.ELSE
LDAPSDKLIB=-lldap50
.ENDIF
ONELIB=
UNOLIB=
SVLIB=-lvcl$(OFFICEUPD)$(DLLPOSTFIX)
ICOLIB=-lico$(OFFICEUPD)$(DLLPOSTFIX)
VCLLIBST=-lvcl
VCLLIB=$(SVLIB)
BASEGFXLIB=-lbasegfx$(OFFICEUPD)$(DLLPOSTFIX)
BASEBMPLIB=-lbasebmp$(OFFICEUPD)$(DLLPOSTFIX)
CANVASTOOLSLIB=-lcanvastools$(OFFICEUPD)$(DLLPOSTFIX)
CPPCANVASLIB=-lcppcanvas$(OFFICEUPD)$(DLLPOSTFIX)
.IF "$(SYSTEM_AGG)" == "YES"
AGGLIB=-lagg
.ELSE
AGGLIB=-lagg$(OFFICEUPD)$(DLLPOSTFIX)
.ENDIF
.IF "$(SYSTEM_FREETYPE)"=="YES"
FREETYPELIB=$(FREETYPE_LIBS)
FREETYPELIBST=$(FREETYPE_LIBS)
.ELSE
FREETYPELIB=-lfreetype
FREETYPELIBST=$(STATIC) -lfreetype $(DYNAMIC)
.ENDIF
XPLIB=-l_xp
TKLIB=-ltk$(OFFICEUPD)$(DLLPOSTFIX)
SVTOOLLIB=-lsvt$(OFFICEUPD)$(DLLPOSTFIX)
XMLSECLIB=-lxmlsec1
XMLSECLIB-NSS=-lxmlsec1-nss
LIBXML2LIB=-lxml2
NSS3LIB=-lnss3
NSPR4LIB=-lnspr4
PLC4LIB=-lplc4
NSSCRYPTOLIBS=$(LIBXML2LIB) $(XMLSECLIB) $(XMLSECLIB-NSS) $(NSS3LIB) $(NSPR4LIB) $(PLC4LIB)
BROOKERLIB=-lbrooker$(OFFICEUPD)$(DLLPOSTFIX)
SIMPLECMLIB=-lsimplecm$(OFFICEUPD)$(DLLPOSTFIX)
COMMUNILIB=-lcommuni$(OFFICEUPD)$(DLLPOSTFIX)
BTCOMMUNILIB=-lbtcommuni$(OFFICEUPD)$(DLLPOSTFIX)
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
MAILLIB=-lmail
DOCMGRLIB=-ldmg$(OFFICEUPD)$(DLLPOSTFIX)
BASICLIB=-lsb$(OFFICEUPD)$(DLLPOSTFIX)
BASICLIGHTLIB=-lsbl$(OFFICEUPD)$(DLLPOSTFIX)
SDBLIB=-lsdb$(OFFICEUPD)$(DLLPOSTFIX)
DBTOOLSLIB=-ldbtools$(OFFICEUPD)$(DLLPOSTFIX)
HM2LIBSH=-lhmwrpdll
HM2LIBST=-lhmwrap
LINGULIB=$(HM2LIBST)
LNGLIB=-llng$(OFFICEUPD)$(DLLPOSTFIX)
.IF "$(SYSTEM_EXPAT)"=="YES"
EXPAT3RDLIB=-lexpat
EXPATASCII3RDLIB=-lexpat
.ELSE
EXPAT3RDLIB=-lexpat_xmlparse -lexpat_xmltok
EXPATASCII3RDLIB=-lascii_expat_xmlparse -lexpat_xmltok
.ENDIF
.IF "$(SYSTEM_ZLIB)"=="YES"
ZLIB3RDLIB=-lz
.ELSE
ZLIB3RDLIB=-lzlib
.ENDIF
.IF "$(SYSTEM_JPEG)"=="YES"
.IF "$(SOLAR_JAVA)" != "" && "$(JDK)" != "gcj" && "$(OS)" != "MACOSX"
#i34482# Blackdown/Sun jdk is in the libsearch patch and has a libjpeg :-(
.IF "$(OS)" == "FREEBSD"
JPEG3RDLIB=/usr/local/lib/libjpeg.so
.ELIF "$(CPUNAME)" == "X86_64"
JPEG3RDLIB=/usr/lib64/libjpeg.so
.ELSE
JPEG3RDLIB=/usr/lib/libjpeg.so
.ENDIF
.ELSE
JPEG3RDLIB=-ljpeg
.ENDIF
.ELSE
JPEG3RDLIB=-ljpeglib
.ENDIF
NEON3RDLIB=-lneon
.IF "$(SYSTEM_DB)" == "YES"
BERKELEYLIB=-ldb
BERKELEYCPPLIB=-ldb_cxx
.ELSE
BERKELEYLIB=-ldb-4.2
BERKELEYCPPLIB=-ldb_cxx-4.2
.ENDIF
CURLLIB=-lcurl
SFX2LIB=-lsfx$(OFFICEUPD)$(DLLPOSTFIX)
SFXLIB=-lsfx$(OFFICEUPD)$(DLLPOSTFIX)
EGGTRAYLIB=-leggtray$(OFFICEUPD)$(DLLPOSTFIX)
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
PRXLIB=-llprx2$(OFFICEUPD)$(DLLPOSTFIX)
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
.IF "$(SYSTEM_LIBXML)"=="YES"
XML2LIB=$(LIBXML_LIBS)
.ELSE
XML2LIB=-lxml2
.ENDIF
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
AWTLIB*=jawt.lib
AVMEDIALIB=iavmedia.lib
ICUINLIB=icuin.lib
ICULELIB=icule.lib
ICUUCLIB=icuuc.lib
I18NUTILLIB=ii18nutil.lib
I18NISOLANGLIB=ii18nisolang.lib
.IF "$(GPC)"!="NO"
GPC3RDLIB=gpc.lib
.ENDIF
SALHELPERLIB=isalhelper.lib
XMLSCRIPTLIB=ixcr.lib
COMPHELPERLIB=icomphelp.lib
CONNECTIVITYLIB=connectivity.lib
LDAPBERLIB=ldapber.lib
CPPULIB=icppu.lib
CPPUHELPERLIB=icppuhelper.lib
UCBHELPERLIB=iucbhelper.lib
ODBCLIB=iodbc.lib
ODBCBASELIB=iodbcbase.lib
DBFILELIB=ifile$(OFFICEUPD).lib
TOOLSLIB=itools.lib
TOOLSLIBST=atools.lib
DVOSUTILLIBST=dvosutil.lib
DUNOUTILLIBST=dunoutil.lib
DVCLUTILLIBST=dvclutil.lib
BPICONVLIB=bpiconv.lib
SALLIB=isal.lib
# make sure some ancient libs are empty
OSLLIB=
RTLLIB=
TECLIB=
VOSLIB=ivos.lib
UNOLIB=iuno.lib
UNOTOOLSLIB=iutl.lib
RMCXTLIB=irmcxt.lib
XMLOFFLIB=ixo.lib
XMLOFFLLIB=xol.lib
STORELIB=istore.lib
IOTXLIB=iotx.lib
OTXLIB=otx_ind.lib
OSXLIB=osx.lib
REGLIB=ireg.lib
UCRLIB=ucr.lib
UASLIB=uas.lib
EXTLIB=iext.lib
USRLIB=usr.lib
SOTLIB=sot.lib
ONELIB=one.lib
MOZBASELIBST=nspr4_s.lib xpcombase_s.lib
MOZBASELIB=nspr4.lib xpcom.lib
LDAPSDKLIB=nsldap32v50.lib
ONELIB=
UNOLIB=
PAPILIB=ipap.lib
TRACERLIB=itrc.lib
TRCLIB=$(TRACERLIB)
SETUPLIB=iset.lib
SCHEDLIB=ss.lib
SSCLIB=ssc.lib
SSLLIB=ssl.lib
SSWLIB=ssw.lib
SFX2LIB=sfx.lib
SFXLIB=$(SFX2LIB)
FWELIB=ifwe.lib
FWILIB=ifwi.lib
BTSTRPLIB=btstrp.lib
BTSTRPDTLIB=bootstrpdt.lib
SOLDEPLIB=soldep.lib
TRANSEXLIB=transex.lib
ICOLIB=icom.lib
SVTOOLLIB=svtool.lib
XMLSECLIB=libxmlsec.lib
XMLSECLIB-MS=libxmlsec-mscrypto.lib
XMLSECLIB-NSS=libxmlsec-nss.lib
LIBXML2LIB=libxml2.lib
NSS3LIB=nss3.lib
NSPR4LIB=nspr4.lib
PLC4LIB=plc4.lib
NSSCRYPTOLIBS=$(LIBXML2LIB) $(XMLSECLIB) $(XMLSECLIB-NSS) $(NSS3LIB) $(NSPR4LIB) $(PLC4LIB)
MSCRYPTOLIBS=$(LIBXML2LIB) $(XMLSECLIB) $(XMLSECLIB-MS) crypt32.lib advapi32.lib
BROOKERLIB=ibrooker.lib
SIMPLECMLIB=isimplecm.lib
COMMUNILIB=icommuni.lib
BTCOMMUNILIB=ibtcommuni.lib
AUTOMATIONLIB=ists.lib
SVLLIB=isvl.lib
PLUGCTORLIB=plugctor.lib
PLUGAPPLIB=plugapp.lib
SVMEMLIB=svmem.lib
SVUNZIPLIB=svunzip.lib
SVUNZIPDLL=unzipdll.lib
GOODIESLIB=igo.lib
MAILLIB=mail.lib
DOCMGRLIB=docmgr.lib
BASICLIB=basic.lib
BASICLIGHTLIB=basicl.lib
SO2LIB=so2.lib
TKTLIB=tkt.lib
SJLIB=sj.lib
SVXLIB=isvx.lib
BASCTLLIB=basctl.lib
BASICIDELIB=ybctl.lib
SVXLLIB=svxl.lib
SDBLIB=sdb.lib
DBTOOLSLIB=idbtools.lib
HM2LIBSH=hmwrpdll.lib
HM2LIBST=hmwrap.lib
LINGULIB=$(HM2LIBST)
LNGLIB=ilng.lib
EXPAT3RDLIB=expat_xmltok.lib expat_xmlparse.lib
EXPATASCII3RDLIB=expat_xmltok.lib ascii_expat_xmlparse.lib
ZLIB3RDLIB=zlib.lib
JPEG3RDLIB=jpeglib.lib
NEON3RDLIB=neon.lib
BERKELEYLIB=libdb42.lib
BERKELEYCPPLIB=
CURLLIB=libcurl.lib
CHAOSLIB=ichaos.lib
UUILIB=iuui.lib
DGLIB=
SIMLIB=ysim.lib
SCHLIB=ysch.lib
SMLIB=ysm.lib
SBALIB=sbawrap.lib
OFALIB=aofa.lib
SCLIB=sclib.lib
SDLIB=sdlib.lib
SDLLIB=sdl.lib
SWLIB=swlib.lib
PRXLIB=ilprx2.lib
ISWLIB=_sw.lib
ISCLIB=sci.lib
ISDLIB=sdi.lib
SVLIB=ivcl.lib
VCLLIB=$(SVLIB)
BASEGFXLIB=ibasegfx.lib
BASEBMPLIB=ibasebmp.lib
CANVASTOOLSLIB=icanvastools.lib
CPPCANVASLIB=icppcanvas.lib
AGGLIB=iagg.lib
TKLIB=itk.lib
SVXLLIB=svxl.lib
FREETYPELIB=freetype.lib
FREETYPELIBST=freetype.lib
PKGCHKLIB=ipkgchk.lib
SYSSHELLLIB=sysshell.lib
SSOOPTLIB=issoopt$(OFFICEUPD)$(DLLPOSTFIX).lib
JVMACCESSLIB = ijvmaccess.lib
CPPUNITLIB = cppunit.lib
XML2LIB = libxml2.lib
JVMFWKLIB = ijvmfwk.lib

# #110743#
# For BinFilters, some libs were added.
#

BFSVXLIB=bf_svx.lib
BFSFXLIB=bf_sfx.lib
BFSCHLIB=bf_ysch.lib
BFSMLIB=bf_ysm.lib
BFSCLIB=bf_sclib.lib
BFSDLIB=bf_sdlib.lib
BFSWLIB=bf_swlib.lib
BFOFALIB=bf_ofa.lib
BFXMLOFFLIB=ibf_xo.lib
LEGACYSMGRLIB=ilegacy_binfilters.lib

.ENDIF              # "$(GUI)"=="UNX" || "$(GUI)"=="MAC"

