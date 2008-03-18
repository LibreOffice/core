#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: libs.mk,v $
#
#   $Revision: 1.130 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:08:10 $
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
LIBSMKREV!:="$$Revision: 1.130 $$"

.IF "$(GUI)"=="UNX" || "$(COM)"=="GCC"

#
#externe libs in plattform.mk
#
.IF "$(GUI)$(COM)"=="WNTGCC"
AWTLIB*=$(JAVA_HOME)$/lib$/jawt.lib
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
AWTLIB*=-ljawt
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
AVMEDIALIB=-lavmedia$(DLLPOSTFIX)
.IF "$(GUI)$(COM)"=="WNTGCC"
.INCLUDE .IGNORE : icuversion.mk
ICUINLIB=-licuin$(ICU_MAJOR)$(ICU_MINOR)
ICULELIB=-licule$(ICU_MAJOR)$(ICU_MINOR)
ICUUCLIB=-licuuc$(ICU_MAJOR)$(ICU_MINOR)
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
ICUINLIB=-licui18n
ICULELIB=-licule
ICUUCLIB=-licuuc
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
I18NUTILLIB=-li18nutil$(COMID)
.INCLUDE .IGNORE : i18npool$/version.mk
I18NISOLANGLIB=-li18nisolang$(ISOLANG_MAJOR)$(COMID)
.IF "$(WITH_GPC)"!="NO"
GPC3RDLIB=-lgpc
.ENDIF
.IF "$(GUI)$(COM)"=="WNTGCC"
SALHELPERLIB=-lsalhelper$(UDK_MAJOR)$(COMID)
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
SALHELPERLIB=-luno_salhelper$(COMID)
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
XMLSCRIPTLIB =-lxcr$(DLLPOSTFIX)
.INCLUDE .IGNORE : comphelper$/version.mk
COMPHELPERLIB=-lcomphelp$(COMPHLP_MAJOR)$(COMID)
CONNECTIVITYLIB=-lconnectivity
LDAPBERLIB=-lldapber
TOOLSLIBST=-latools
BPICONVLIB=-lbpiconv
TOOLSLIB=-ltl$(DLLPOSTFIX)
.IF "$(GUI)$(COM)"=="WNTGCC"
CPPULIB=-lcppu$(UDK_MAJOR)
CPPUHELPERLIB=-lcppuhelper$(UDK_MAJOR)$(COMID)
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
CPPULIB=-luno_cppu
CPPUHELPERLIB=-luno_cppuhelper$(COMID)
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
.INCLUDE .IGNORE : ucbhelper$/version.mk
UCBHELPERLIB=-lucbhelper$(UCBHELPER_MAJOR)$(COMID)
.IF "$(SYSTEM_OPENSSL)" == "YES"
OPENSSLLIB=$(OPENSSL_LIBS)
OPENSSLLIBST=$(STATIC) $(OPENSSL_LIBS) $(DYNAMIC)
.ELSE           # "$(SYSTEM_OPENSSL)" == "YES
OPENSSLLIB=-lssl -lcrypto
OPENSSLLIBST=$(STATIC) -lssl -lcrypto $(DYNAMIC)
.ENDIF          # "$(SYSTEM_OPENSSL)" == "YES"
.IF "$(GUI)$(COM)"=="WNTGCC"
REGLIB=-lreg$(UDK_MAJOR)
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
REGLIB=-lreg
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
.INCLUDE .IGNORE : vos$/version.mk
VOSLIB=-lvos$(VOS_MAJOR)$(COMID)
XMLOFFLIB=-lxo$(DLLPOSTFIX)
XMLOFFLLIB=-lxol
.IF "$(GUI)$(COM)"=="WNTGCC"
STORELIB=-lstore$(UDK_MAJOR)
SALLIB=-lsal$(UDK_MAJOR)
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
STORELIB=-lstore
SALLIB=-luno_sal
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
.INCLUDE .IGNORE : connectivity$/version.mk
ODBCLIB=-lodbc$(ODBC_MAJOR)
ODBCBASELIB=-lodbcbase$(ODBC_MAJOR)
DBFILELIB=-lfile$(DLLPOSTFIX)
.IF "$(GUI)$(COM)"=="WNTGCC"
RMCXTLIB=-lrmcxt$(UDK_MAJOR)
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
RMCXTLIB=-lrmcxt
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
BTSTRPLIB=-lbtstrp
BTSTRPDTLIB=-lbootstrpdt$(DLLPOSTFIX)
SOLDEPLIB=-lsoldep$(DLLPOSTFIX)
TRANSEXLIB=-ltransex
OTXLIB=-lotx_ind
OSXLIB=-losx
UNOTOOLSLIB=-lutl$(DLLPOSTFIX)
SOTLIB=-lsot$(DLLPOSTFIX)
MOZBASELIBST=$(STATIC) -lnspr4 -lxpcombase_s $(DYNAMIC)
MOZBASELIB=-lnspr4 -lxpcom
.IF "$(GUI)$(COM)"=="WNTGCC"
LDAPSDKLIB=-lnsldap32v50
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
.IF "$(WITH_OPENLDAP)" == "YES"
LDAPSDKLIB=-lldap
.ELSE
LDAPSDKLIB=-lldap50
.ENDIF
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
ICOLIB=-lico$(DLLPOSTFIX)
VCLLIB=-lvcl$(DLLPOSTFIX)
BASEGFXLIB=-lbasegfx$(DLLPOSTFIX)
BASEBMPLIB=-lbasebmp$(DLLPOSTFIX)
CANVASTOOLSLIB=-lcanvastools$(DLLPOSTFIX)
CPPCANVASLIB=-lcppcanvas$(DLLPOSTFIX)
.IF "$(SYSTEM_AGG)" == "YES"
AGGLIB=-lagg
.ELSE
AGGLIB=-lagg$(DLLPOSTFIX)
.ENDIF
.IF "$(SYSTEM_FREETYPE)"=="YES"
FREETYPELIB=$(FREETYPE_LIBS)
FREETYPELIBST=$(FREETYPE_LIBS)
.ELSE
FREETYPELIB=-lfreetype
FREETYPELIBST=$(STATIC) -lfreetype $(DYNAMIC)
.ENDIF
PSPLIB=-lpsp$(DLLPOSTFIX)
TKLIB=-ltk$(DLLPOSTFIX)
LAYOUTLIB=-ltklayout$(DLLPOSTFIX)
SVTOOLLIB=-lsvt$(DLLPOSTFIX)
.IF "$(GUI)$(COM)"=="WNTGCC"
XMLSECLIB=-lxmlsec1-1
XMLSECLIB-NSS=-lxmlsec1-nss-1
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
XMLSECLIB=-lxmlsec1
XMLSECLIB-NSS=-lxmlsec1-nss
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
.IF "$(GUI)$(COM)"=="WNTGCC"
LIBXML2LIB=-lxml2-2
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
.IF "$(SYSTEM_LIBXML)"=="YES"
LIBXML2LIB=$(LIBXML_LIBS)
.ELSE
LIBXML2LIB=-lxml2
.ENDIF
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
NSS3LIB=-lnss3
NSPR4LIB=-lnspr4
PLC4LIB=-lplc4
NSSCRYPTOLIBS=$(LIBXML2LIB) $(XMLSECLIB) $(XMLSECLIB-NSS) $(NSS3LIB) $(NSPR4LIB) $(PLC4LIB)
.IF "$(GUI)$(COM)"=="WNTGCC"
XMLSECLIB-MS=-lxmlsec1-mscrypto-1
MSCRYPTOLIBS=$(LIBXML2LIB) $(XMLSECLIB) $(XMLSECLIB-MS) $(CRYPT32LIB) $(ADVAPI32LIB)
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
BROOKERLIB=-lbrooker$(DLLPOSTFIX)
SIMPLECMLIB=-lsimplecm$(DLLPOSTFIX)
COMMUNILIB=-lcommuni$(DLLPOSTFIX)
BTCOMMUNILIB=-lbtcommuni$(DLLPOSTFIX)
AUTOMATIONLIB=-lsts$(DLLPOSTFIX)
SVLLIB=-lsvl$(DLLPOSTFIX)
TKTLIB=-ltkt$(DLLPOSTFIX)
SJLIB=-lj$(DLLPOSTFIX)_g
GOODIESLIB=-lgo$(DLLPOSTFIX)
SAXLIB=-lsax$(DLLPOSTFIX)
MAILLIB=-lmail
DOCMGRLIB=-ldmg$(DLLPOSTFIX)
BASICLIB=-lsb$(DLLPOSTFIX)
DBTOOLSLIB=-ldbtools$(DLLPOSTFIX)
HM2LIBSH=-lhmwrpdll
HM2LIBST=-lhmwrap
LINGULIB=$(HM2LIBST)
LNGLIB=-llng$(DLLPOSTFIX)
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
.IF "$(SYSTEM_NEON)" == "YES" || "$(GUI)$(COM)"=="WNTGCC"
NEON3RDLIB=-lneon
.ELIF "$(OS)" == "MACOSX"
NEON3RDLIB=$(SOLARLIBDIR)$/libneon.a
.ELSE
NEON3RDLIB=$(STATIC) -lneon $(DYNAMIC)
.ENDIF
.IF "$(GUI)$(COM)"=="WNTGCC"
BERKELEYLIB=-ldb42
BERKELEYCPPLIB=-ldb_cxx42
CURLLIB=-lcurl-3
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
.IF "$(SYSTEM_DB)" == "YES"
BERKELEYLIB=-ldb
BERKELEYCPPLIB=-ldb_cxx
.ELSE
BERKELEYLIB=-ldb-4.2
BERKELEYCPPLIB=-ldb_cxx-4.2
.ENDIF
CURLLIB=-lcurl
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
SFX2LIB=-lsfx$(DLLPOSTFIX)
SFXLIB=-lsfx$(DLLPOSTFIX)
EGGTRAYLIB=-leggtray$(DLLPOSTFIX)
SFXDEBUGLIB=
FWELIB=-lfwe$(DLLPOSTFIX)
FWILIB=-lfwi$(DLLPOSTFIX)
SVXLIB=-lsvx$(DLLPOSTFIX)
BASCTLLIB=-lbasctl$(DLLPOSTFIX)
BASICIDELIB=-lybctl
SVXLLIB=-lsvxl
CHAOSLIB=-lcnt$(DLLPOSTFIX)
UUILIB=-luui$(DLLPOSTFIX)
DGLIB=
SCHLIB=-lysch
SMLIB=-lysm
OFALIB=-lofa$(DLLPOSTFIX)
PRXLIB=-llprx2$(DLLPOSTFIX)
PAPILIB=-lpap$(DLLPOSTFIX)
SCLIB=-lsclib
SDLIB=-lsdlib
SDLLIB=-lsdl
SWLIB=-lswlib
ISWLIB=-lsw$(DLLPOSTFIX)
ISCLIB=-lsc$(DLLPOSTFIX)
ISDLIB=-lsd$(DLLPOSTFIX)
PKGCHKLIB=-lpkgchk$(DLLPOSTFIX)
HELPLINKERLIB=-lhelplinker$(DLLPOSTFIX)
SYSSHELLLIB=-lsysshell
.IF "$(GUI)$(COM)"=="WNTGCC"
JVMACCESSLIB = -ljvmaccess$(UDK_MAJOR)$(COMID)
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
JVMACCESSLIB = -ljvmaccess$(COMID)
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
CPPUNITLIB = -lcppunit$(DLLPOSTFIX)
.IF "$(GUI)$(COM)"=="WNTGCC"
XSLTLIB=-lxslt-1 $(ZLIB3RD) $(LIBXML2LIB)
JVMFWKLIB = -ljvmfwk$(UDK_MAJOR)
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
.IF "$(SYSTEM_LIBXSLT)"=="YES"
XSLTLIB=$(LIBXSLT_LIBS)
.ELSE
XSLTLIB=-lxslt $(ZLIB3RD) $(LIBXML2LIB)
.ENDIF
JVMFWKLIB = -ljvmfwk
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"

# #110743#
# For BinFilters, some libs were added.
#

BFSVXLIB=-lbf_svx$(DLLPOSTFIX)
BFSCHLIB=-lbf_ysch
BFSMLIB=-lbf_ysm
BFSCLIB=-lbf_sclib
BFSDLIB=-lbf_sdlib
BFSWLIB=-lbf_swlib
BFOFALIB=-lbf_ofa$(DLLPOSTFIX)
LEGACYSMGRLIB=-llegacy_binfilters$(DLLPOSTFIX)
BFXMLOFFLIB=-lbf_xo$(DLLPOSTFIX)
BFGOODIESLIB=-lbf_go$(DLLPOSTFIX)
BFBASICLIB=-lbf_sb$(DLLPOSTFIX)
BFSO3LIB=-lbf_so$(DLLPOSTFIX)
BFSVTOOLLIB=-lbf_svt$(DLLPOSTFIX)

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
USED_RCLIENT_LIBS =	$(VCLLIB) $(SOTLIB) $(TOOLSLIB) \
                    $(USED_UNO_LIBS)

SABLOT3RDLIB=-lsablot
APP3RDLIB=-lapp
SAMPLE3RDLIB=-lsample
HNJLIB=-lhnj
MYSPELLLIB=-lmyspell
COSVLIB=-lcosv
UDMLIB=-ludm
HUNSPELLLIB*=-lhunspell
ULINGULIB=-lulingu
MYTHESLIB=-lmythes
PYUNOLIB=-lpyuno
LPSOLVELIB=-llpsolve55
SOFFICELIB=-lsoffice
UNOPKGLIB=-lunopkg

.ELSE				# "$(GUI)"=="UNX"
AWTLIB*=jawt.lib
AVMEDIALIB=iavmedia.lib
ICUINLIB=icuin.lib
ICULELIB=icule.lib
ICUUCLIB=icuuc.lib
I18NUTILLIB=ii18nutil.lib
I18NISOLANGLIB=ii18nisolang.lib
.IF "$(WITH_GPC)"!="NO"
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
OPENSSLLIB=ssleay32.lib libeay32.lib
ODBCLIB=iodbc.lib
ODBCBASELIB=iodbcbase.lib
DBFILELIB=ifile.lib
TOOLSLIB=itools.lib
TOOLSLIBST=atools.lib
BPICONVLIB=bpiconv.lib
SALLIB=isal.lib
VOSLIB=ivos.lib
UNOTOOLSLIB=iutl.lib
RMCXTLIB=irmcxt.lib
XMLOFFLIB=ixo.lib
XMLOFFLLIB=xol.lib
STORELIB=istore.lib
OTXLIB=otx_ind.lib
OSXLIB=osx.lib
REGLIB=ireg.lib
EXTLIB=iext.lib
SOTLIB=sot.lib
MOZBASELIBST=nspr4_s.lib xpcombase_s.lib
MOZBASELIB=nspr4.lib xpcom.lib
LDAPSDKLIB=nsldap32v50.lib
PAPILIB=ipap.lib
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
PLUGAPPLIB=plugapp.lib
GOODIESLIB=igo.lib
SAXLIB=isax.lib
MAILLIB=mail.lib
DOCMGRLIB=docmgr.lib
BASICLIB=basic.lib
TKTLIB=tkt.lib
SJLIB=sj.lib
SVXLIB=isvx.lib
BASCTLLIB=basctl.lib
BASICIDELIB=ybctl.lib
SVXLLIB=svxl.lib
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
SCHLIB=ysch.lib
SMLIB=ysm.lib
OFALIB=aofa.lib
SCLIB=sclib.lib
SDLIB=sdlib.lib
SDLLIB=sdl.lib
SWLIB=swlib.lib
PRXLIB=ilprx2.lib
ISWLIB=_sw.lib
ISCLIB=sci.lib
ISDLIB=sdi.lib
VCLLIB=ivcl.lib
BASEGFXLIB=ibasegfx.lib
BASEBMPLIB=ibasebmp.lib
CANVASTOOLSLIB=icanvastools.lib
CPPCANVASLIB=icppcanvas.lib
AGGLIB=iagg.lib
PSPLIB=apsp.lib
TKLIB=itk.lib
LAYOUTLIB=itklayout.lib
SVXLLIB=svxl.lib
FREETYPELIB=freetype.lib
FREETYPELIBST=freetype.lib
PKGCHKLIB=ipkgchk.lib
HELPLINKERLIB=ihelplinker.lib
SYSSHELLLIB=sysshell.lib
JVMACCESSLIB = ijvmaccess.lib
CPPUNITLIB = cppunit.lib
XSLTLIB = libxslt.lib $(ZLIB3RD) $(LIBXML2LIB)

JVMFWKLIB = ijvmfwk.lib

# #110743#
# For BinFilters, some libs were added.
#

BFSVXLIB=bf_svx.lib
BFSCHLIB=bf_ysch.lib
BFSMLIB=bf_ysm.lib
BFSCLIB=bf_sclib.lib
BFSDLIB=bf_sdlib.lib
BFSWLIB=bf_swlib.lib
BFOFALIB=bf_ofa.lib
BFXMLOFFLIB=ibf_xo.lib
BFGOODIESLIB=bf_go.lib
BFBASICLIB=bf_sb.lib
BFSO3LIB=bf_so.lib
LEGACYSMGRLIB=ilegacy_binfilters.lib
BFSVTOOLLIB=bf_svt.lib

SABLOT3RDLIB= $(LIBPRE) sablot.lib
APP3RDLIB= $(LIBPRE) app.lib
SAMPLE3RDLIB= $(LIBPRE) sample.lib
HNJLIB= libhnj.lib
MYSPELLLIB= $(LIBPRE) myspell.lib
COSVLIB= $(LIBPRE) cosv.lib
UDMLIB= $(LIBPRE) udm.lib
HUNSPELLLIB*=hunspell.lib
ULINGULIB=$(LIBPRE) libulingu.lib
MYTHESLIB=libmythes.lib
PYUNOLIB=ipyuno.lib
LPSOLVELIB=lpsolve55.lib
SOFFICELIB=isoffice.lib
UNOPKGLIB=iunopkg.lib

.ENDIF              # "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
