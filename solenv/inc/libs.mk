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

.IF ("$(GUI)"=="UNX" || "$(COM)"=="GCC")

# workaround for MinGW to get the same DLL names as with MSC
# e.g., uno_salhelper3gcc3.dll (vs. uno_salhelpergcc3.dll)
.IF "$(GUI)" == "WNT"
COMID_WITH_VERSION = $(UDK_MAJOR)$(COMID)
.ELSE
COMID_WITH_VERSION = $(COMID)
.ENDIF

.IF "$(GUI)" == "WNT"
ODMA_LIB_LIB=-lodma_lib
.ENDIF

#
#externe libs in plattform.mk
#
AWTLIB*=-ljawt
AVMEDIALIB=-lavmedia$(DLLPOSTFIX)
.IF "$(GUI)$(COM)"=="WNTGCC" && "$(SYSTEM_ICU)"!="YES"
ICUINLIB=-licuin$(ICU_MAJOR)$(ICU_MINOR)
ICULELIB=-licule$(ICU_MAJOR)$(ICU_MINOR)
ICUUCLIB=-licuuc$(ICU_MAJOR)$(ICU_MINOR)
ICUDATALIB=-licudt$(ICU_MAJOR)$(ICU_MINOR)
.ELIF "$(OS)"=="ANDROID"
ICUINLIB=-licui18nlo
ICULELIB=-liculelo
ICUUCLIB=-licuuclo
ICUDATALIB=-licudatalo
.ELSE
ICUINLIB=-licui18n
ICULELIB=-licule
ICUUCLIB=-licuuc
ICUDATALIB=-licudata
.ENDIF
I18NUTILLIB=-li18nutil$(COMID)
I18NISOLANGLIB=-li18nisolang1$(COMID)
I18NREGEXPLIB=-li18nregexp$(DLLPOSTFIX)
SALHELPERLIB=-luno_salhelper$(COMID_WITH_VERSION)
XMLSCRIPTLIB =-lxmlscript$(DLLPOSTFIX)
COMPHELPERLIB=-lcomphelp$(COMID)
CONNECTIVITYLIB=-lconnectivity
LDAPBERLIB=-lldapber
TOOLSLIBST=-latools
BPICONVLIB=-lbpiconv
TOOLSLIB=-ltl$(DLLPOSTFIX)
CPPULIB=-luno_cppu
CPPUHELPERLIB=-luno_cppuhelper$(COMID_WITH_VERSION)
UCBHELPERLIB=-lucbhelper4$(COMID)
.IF "$(SYSTEM_OPENSSL)" == "YES"
OPENSSLLIB=$(OPENSSL_LIBS)
OPENSSLLIBST=$(STATIC) $(OPENSSL_LIBS) $(DYNAMIC)
.ELSE           # "$(SYSTEM_OPENSSL)" == "YES
OPENSSLLIB=-lssl -lcrypto
.IF "$(GUI)$(COM)"=="WNTGCC"
OPENSSLLIBST=-lssl_static -lcrypto_static
.ELSE          # "$(GUI)$(COM)"=="WNTGCC"
OPENSSLLIBST=$(STATIC) -lssl -lcrypto $(DYNAMIC)
.ENDIF          # "$(GUI)$(COM)"=="WNTGCC"
.ENDIF          # "$(SYSTEM_OPENSSL)" == "YES"
REGLIB=-lreg
XMLOFFLIB=-lxo$(DLLPOSTFIX)
XMLOFFLLIB=-lxol
STORELIB=-lstore
SALLIB=-luno_sal
ODBCLIB=-lodbc$(DLLPOSTFIX)
ODBCBASELIB=-lodbcbase$(DLLPOSTFIX)
DBFILELIB=-lfile$(DLLPOSTFIX)
.IF "$(GUI)$(COM)"=="WNTGCC"
RMCXTLIB=-lrmcxt$(UDK_MAJOR)
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
RMCXTLIB=-lrmcxt
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
BTSTRPLIB=-lbtstrp
BTSTRPDTLIB=-lbootstrpdt$(DLLPOSTFIX)
TRANSEXLIB=-ltransex
OTXLIB=-lotx_ind
OSXLIB=-losx
UNOTOOLSLIB=-lutl$(DLLPOSTFIX)
SOTLIB=-lsot$(DLLPOSTFIX)
.IF "$(GUI)$(COM)"=="WNTGCC"
MOZBASELIBST=$(STATIC) -lnspr4_s -lxpcombase_s
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
MOZBASELIBST=$(STATIC) -lnspr4 -lxpcombase_s $(DYNAMIC)
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
MOZBASELIB=-lnspr4 -lxpcom
.IF "$(GUI)$(COM)"=="WNTGCC"
LDAPSDKLIB=-lwldap32
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
LDAPSDKLIB=-lldap
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"

ICOLIB=-lico$(DLLPOSTFIX)
VCLLIB=-lvcl$(DLLPOSTFIX)
OOXLIB=-loox$(DLLPOSTFIX)
BASEGFXLIB=-lbasegfx$(DLLPOSTFIX)
DRAWINGLAYERLIB=-ldrawinglayer$(DLLPOSTFIX)
BASEBMPLIB=-lbasebmp$(DLLPOSTFIX)
CANVASTOOLSLIB=-lcanvastools$(DLLPOSTFIX)
CPPCANVASLIB=-lcppcanvas$(DLLPOSTFIX)
FORLIB=-lfor$(DLLPOSTFIX)
FORUILIB=-lforui$(DLLPOSTFIX)

.IF "$(SYSTEM_AGG)" == "YES"
AGGLIB=-lagg
.ELSE
AGGLIB=-lagg$(DLLPOSTFIX)
.ENDIF
FREETYPE_LIBS*=-lfreetype
FREETYPELIB=$(FREETYPE_LIBS)
TKLIB=-ltk$(DLLPOSTFIX)
LAYOUTLIB=-ltklayout$(DLLPOSTFIX)
SVTOOLLIB=-lsvt$(DLLPOSTFIX)
XMLSECLIB=-lxmlsec1
XMLSECLIB-NSS=-lxmlsec1-nss
.IF "$(SYSTEM_LIBXML)"=="YES"
LIBXML2LIB=$(LIBXML_LIBS)
.ELSE
LIBXML2LIB=-lxml2
.ENDIF
NSS3LIB=-lnss3
NSPR4LIB=-lnspr4
PLC4LIB=-lplc4
NSSCRYPTOLIBS=$(XMLSECLIB-NSS) $(XMLSECLIB) $(LIBXML2LIB) $(NSS3LIB) $(NSPR4LIB) $(PLC4LIB)
.IF "$(GUI)$(COM)"=="WNTGCC"
XMLSECLIB-MS=-lxmlsec1-mscrypto
MSCRYPTOLIBS=$(XMLSECLIB) $(XMLSECLIB-MS) $(LIBXML2LIB) $(CRYPT32LIB) $(ADVAPI32LIB)
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
BROOKERLIB=-lbrooker$(DLLPOSTFIX)
SIMPLECMLIB=-lsimplecm$(DLLPOSTFIX)
COMMUNILIB=-lcommuni$(DLLPOSTFIX)
BTCOMMUNILIB=-lbtcommuni$(DLLPOSTFIX)
AUTOMATIONLIB=-lsts$(DLLPOSTFIX)
SVLLIB=-lsvl$(DLLPOSTFIX)
TKTLIB=-ltkt$(DLLPOSTFIX)
SAXLIB=-lsax$(DLLPOSTFIX)
MAILLIB=-lmail
DOCMGRLIB=-ldmg$(DLLPOSTFIX)
BASICLIB=-lsb$(DLLPOSTFIX)
VBAHELPERLIB=-lvbahelper$(DLLPOSTFIX)
DBTOOLSLIB=-ldbtools$(DLLPOSTFIX)
HM2LIBSH=-lhmwrpdll
HM2LIBST=-lhmwrap
LINGULIB=$(HM2LIBST)
LNGLIB=-llng$(DLLPOSTFIX)
.IF "$(SYSTEM_CLUCENE)"=="YES"
CLUCENELIB=$(CLUCENE_LIBS)
.ELSE
CLUCENELIB=-lclucene
.ENDIF
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
.IF "$(SYSTEM_MINIZIP)"=="YES"
MINIZIP3RDLIB=$(MINIZIP_LIBS)
.ELSE
# internally, minizip is included in zlib
MINIZIP3RDLIB=
.ENDIF
.IF "$(SYSTEM_JPEG)"=="YES"
JPEG3RDLIB=-ljpeg
.ELSE
JPEG3RDLIB=-ljpeglib
.ENDIF
.IF "$(SYSTEM_NEON)" == "YES"
NEON3RDLIB=-lneon
.ELIF "$(GUI)$(COM)"=="WNTGCC"
NEON3RDLIB=-lneon
.ELIF "$(OS)" == "MACOSX"
NEON3RDLIB=$(SOLARLIBDIR)/libneon.dylib
.ELSE
NEON3RDLIB=-lneon
.ENDIF
.IF "$(SYSTEM_DB)" == "YES"
BERKELEYLIB=-l$(DB_LIB)
.ELIF "$(GUI)$(COM)"=="WNTGCC"
BERKELEYLIB=-ldb47
.ELSE
BERKELEYLIB=-ldb-4.7
.ENDIF
CURLLIB=-lcurl
SFX2LIB=-lsfx$(DLLPOSTFIX)
SFXLIB=-lsfx$(DLLPOSTFIX)
SFXDEBUGLIB=
FWELIB=-lfwe$(DLLPOSTFIX)
FWILIB=-lfwi$(DLLPOSTFIX)
SVXCORELIB=-lsvxcore$(DLLPOSTFIX)
MSFILTERLIB=-lmsfilter$(DLLPOSTFIX)
SVXLIB=-lsvx$(DLLPOSTFIX)
EDITENGLIB=-lediteng$(DLLPOSTFIX)
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
PACKAGE2LIB=-lpackage2
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
JVMACCESSLIB = -ljvmaccess$(COMID_WITH_VERSION)
.IF "$(SYSTEM_CPPUNIT)"=="YES"
CPPUNITLIB = $(CPPUNIT_LIBS)
.ELSE
CPPUNITLIB = -lcppunit
.ENDIF
.IF "$(SYSTEM_LIBXSLT)"=="YES"
XSLTLIB=$(LIBXSLT_LIBS)
.ELSE
XSLTLIB=-lxslt $(LIBXML2LIB)
.ENDIF
JVMFWKLIB = -ljvmfwk
.IF "$(SYSTEM_REDLAND)"=="YES"
REDLANDLIB=$(REDLAND_LIBS)
.ELSE
REDLANDLIB=-lrdf
.ENDIF

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
USED_UNO_LIBS =		 $(OSLLIB)
USED_TOOLS_LIBS =
USED_SOT_LIBS = 	$(TOOLSLIB)
USED_VCL_LIBS =		$(SOTLIB) $(TOOLSLIB) $(USED_UNO_LIBS)

# Applikationen
USED_BOOTSTRP_LIBS= $(TOOLSLIB)
USED_RCLIENT_LIBS =	$(VCLLIB) $(SOTLIB) $(TOOLSLIB) \
                    $(USED_UNO_LIBS)

APP3RDLIB=-lapp
SAMPLE3RDLIB=-lsample
HNJLIB*=-lhyphen
MYSPELLLIB=-lmyspell
ULINGULIB=-lulingu
.IF "$(SYSTEM_HUNSPELL)" == "YES"
HUNSPELLLIB=$(HUNSPELL_LIBS)
.ELSE
HUNSPELLLIB=-lhunspell-1.3
.ENDIF
.IF "$(SYSTEM_MYTHES)" == "YES"
MYTHESLIB=$(MYTHES_LIBS)
.ELSE
MYTHESLIB=-lmythes-1.2
.ENDIF
PYUNOLIB=-lpyuno
LPSOLVELIB=-llpsolve55
SOFFICELIB=-lsofficeapp
UNOPKGAPPLIB=-lunopkgapp
TESTLIB=-ltest
UNOTESTLIB=-lunotest
XMLREADERLIB=-lxmlreader

.IF "$(OS)" == "IOS"

# As we build static libraries for iOS, we append to each library its
# dependencies, recursively. Start from the bottom of the dependency
# stack. This is experimental and obviously incomplete.

REGLIB+=$(STORELIB)

BOOTSTRAPUNOLIB=$(SOLARLIBDIR)/bootstrap.uno.a
BOOTSTRAPUNOLIB+=$(SALHELPERLIB) $(REGLIB) $(XMLREADERLIB)

GCC3UNOLIB=-lgcc3_uno

SALLIB+=-lsal_textenc

CPPULIB+=$(GCC3UNOLIB)

CPPUHELPERLIB+=$(BOOTSTRAPUNOLIB)

UCBHELPERLIB+=$(SALHELPERLIB)

UNOTOOLSLIB+=$(I18NISOLANGLIB)
UNOTOOLSLIB+=$(UCBHELPERLIB)

TOOLSLIB+=$(BASEGFXLIB)
TOOLSLIB+=$(ZLIB3RDLIB)

VCLLIB+=$(ICUUCLIB)

.ENDIF

.IF "$(OS)" == "ANDROID"

# Some of the external libraries get built as static libraries for
# Android, mostly by accident, because we haven't bothered fixing up
# their configury to realize that it would work to build shared
# libraries.

CURLLIB+=$(ZLIB3RDLIB)

.ENDIF

.ELSE				# ("$(GUI)"=="UNX" || "$(COM)"=="GCC")

ODMA_LIB_LIB=odma_lib.lib
AWTLIB*=jawt.lib
AVMEDIALIB=iavmedia.lib
ICUINLIB=icuin.lib
ICULELIB=icule.lib
ICUUCLIB=icuuc.lib
ICUDATALIB=icudt.lib
I18NUTILLIB=ii18nutil.lib
I18NISOLANGLIB=ii18nisolang.lib
I18NREGEXPLIB=ii18nregexp.lib
SALHELPERLIB=isalhelper.lib
XMLSCRIPTLIB=ixmlscript.lib
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
UNOTOOLSLIB=iutl.lib
RMCXTLIB=irmcxt.lib
XMLOFFLIB=ixo.lib
XMLOFFLLIB=xol.lib
STORELIB=istore.lib
OTXLIB=otx_ind.lib
OSXLIB=osx.lib
REGLIB=ireg.lib
EXTLIB=iext.lib
SOTLIB=isot.lib
MOZBASELIBST=nspr4_s.lib xpcombase_s.lib
MOZBASELIB=nspr4.lib xpcom.lib
LDAPSDKLIB=wldap32.lib
PAPILIB=ipap.lib
SFX2LIB=sfx.lib
SFXLIB=$(SFX2LIB)
FWELIB=ifwe.lib
FWILIB=ifwi.lib
BTSTRPLIB=btstrp.lib
BTSTRPDTLIB=bootstrpdt.lib
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
NSSCRYPTOLIBS=$(XMLSECLIB-NSS) $(XMLSECLIB) $(LIBXML2LIB) $(NSS3LIB) $(NSPR4LIB) $(PLC4LIB)
MSCRYPTOLIBS=$(XMLSECLIB-MS) $(XMLSECLIB) $(LIBXML2LIB) crypt32.lib advapi32.lib
BROOKERLIB=ibrooker.lib
SIMPLECMLIB=isimplecm.lib
COMMUNILIB=icommuni.lib
BTCOMMUNILIB=ibtcommuni.lib
AUTOMATIONLIB=ists.lib
SVLLIB=isvl.lib
PLUGAPPLIB=plugapp.lib
SAXLIB=isax.lib
MAILLIB=mail.lib
DOCMGRLIB=docmgr.lib
BASICLIB=basic.lib
VBAHELPERLIB=vbahelper.lib
TKTLIB=tkt.lib
SVXCORELIB=isvxcore.lib
MSFILTERLIB=imsfilter.lib
SVXLIB=isvx.lib
EDITENGLIB=iediteng.lib
BASCTLLIB=basctl.lib
BASICIDELIB=ybctl.lib
SVXLLIB=svxl.lib
DBTOOLSLIB=idbtools.lib
HM2LIBSH=hmwrpdll.lib
HM2LIBST=hmwrap.lib
LINGULIB=$(HM2LIBST)
LNGLIB=ilng.lib
CLUCENELIB=iclucene.lib
EXPAT3RDLIB=expat_xmltok.lib expat_xmlparse.lib
EXPATASCII3RDLIB=expat_xmltok.lib ascii_expat_xmlparse.lib
ZLIB3RDLIB=zlib.lib
JPEG3RDLIB=jpeglib.lib
NEON3RDLIB=ineon.lib
BERKELEYLIB=libdb47.lib
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
PACKAGE2LIB=ipackage.lib
ISWLIB=_sw.lib
ISCLIB=sci.lib
ISDLIB=sdi.lib
VCLLIB=ivcl.lib
OOXLIB=ioox.lib
BASEGFXLIB=ibasegfx.lib
DRAWINGLAYERLIB=idrawinglayer.lib
BASEBMPLIB=ibasebmp.lib
CANVASTOOLSLIB=icanvastools.lib
CPPCANVASLIB=icppcanvas.lib
FORLIB=ifor.lib
FORUILIB=iforui.lib
AGGLIB=iagg.lib
TKLIB=itk.lib
LAYOUTLIB=itklayout.lib
SVXLLIB=svxl.lib
FREETYPELIB=freetype.lib
PKGCHKLIB=ipkgchk.lib
HELPLINKERLIB=ihelplinker.lib
JVMACCESSLIB = ijvmaccess.lib
CPPUNITLIB = icppunit_dll.lib
XSLTLIB = libxslt.lib $(LIBXML2LIB)
REDLANDLIB = librdf.lib

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

APP3RDLIB= $(LIBPRE) app.lib
SAMPLE3RDLIB= $(LIBPRE) sample.lib
HNJLIB*=libhnj.lib
MYSPELLLIB= $(LIBPRE) myspell.lib
ULINGULIB=$(LIBPRE) libulingu.lib
.IF "$(SYSTEM_HUNSPELL)" == "YES"
HUNSPELLLIB=$(HUNSPELL_LIBS)
.ELSE
HUNSPELLLIB=$(LIBPRE) libhunspell.lib
.ENDIF
MYTHESLIB=libmythes.lib
PYUNOLIB=ipyuno.lib
LPSOLVELIB=lpsolve55.lib
SOFFICELIB=isofficeapp.lib
UNOPKGAPPLIB=iunopkgapp.lib
TESTLIB=itest.lib
UNOTESTLIB=iunotest.lib
BOOSTTHREADLIB=boostthread.lib
XMLREADERLIB=ixmlreader.lib

.ENDIF              # ("$(GUI)"=="UNX" || "$(COM)"=="GCC")
