#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************

LIBSMKREV!:="$$Revision: 1.134.2.3 $$"

.IF ("$(GUI)"=="UNX" || "$(COM)"=="GCC")

#
#externe libs in plattform.mk
#
.IF "$(GUI)$(COM)"=="WNTGCC"
AWTLIB*=$(JAVA_HOME)/lib/jawt.lib
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
AWTLIB*=-ljawt
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
AVMEDIALIB=-lavmedia$(DLLPOSTFIX)
.IF "$(GUI)$(COM)"=="WNTGCC"
.INCLUDE .IGNORE : icuversion.mk
ICUINLIB=-licuin$(ICU_MAJOR)$(ICU_MINOR)
ICULELIB=-licule$(ICU_MAJOR)$(ICU_MINOR)
ICUUCLIB=-licuuc$(ICU_MAJOR)$(ICU_MINOR)
ICUDATALIB=-licudt$(ICU_MAJOR)$(ICU_MINOR)
.ELIF "$(GUI)"=="OS2"
ICUINLIB=-licuin
ICULELIB=-licule
ICUUCLIB=-licuuc
ICUDATALIB=-licudt
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
ICUINLIB=-licui18n
ICULELIB=-licule
ICUUCLIB=-licuuc
ICUDATALIB=-licudata
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
.IF "$(GUI)"=="OS2"
I18NUTILLIB=-li18nutil
.ELSE
I18NUTILLIB=-li18nutil$(COMID)
.ENDIF
.INCLUDE .IGNORE : i18npool/version.mk
I18NISOLANGLIB=-li18nisolang$(ISOLANG_MAJOR)$(COMID)
I18NPAPERLIB=-li18npaper$(DLLPOSTFIX)
.IF "$(GUI)$(COM)"=="WNTGCC"
SALHELPERLIB=-lsalhelper$(UDK_MAJOR)$(COMID)
.ELIF "$(GUI)"=="OS2"
SALHELPERLIB=-lsalhelp$(UDK_MAJOR)
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
SALHELPERLIB=-luno_salhelper$(COMID)
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
XMLSCRIPTLIB =-lxcr$(DLLPOSTFIX)
COMPHELPERLIB=-lcomphelp$(COMID)
CONNECTIVITYLIB=-lconnectivity
LDAPBERLIB=-lldapber
TOOLSLIBST=-latools
BPICONVLIB=-lbpiconv
TOOLSLIB=-ltl$(DLLPOSTFIX)
.IF "$(GUI)$(COM)"=="WNTGCC"
CPPULIB=-lcppu$(UDK_MAJOR)
CPPUHELPERLIB=-lcppuhelper$(UDK_MAJOR)$(COMID)
.ELIF "$(GUI)"=="OS2"
CPPULIB=-lcppu$(UDK_MAJOR)
CPPUHELPERLIB=-lcppuh$(UDK_MAJOR)
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
CPPULIB=-luno_cppu
CPPUHELPERLIB=-luno_cppuhelper$(COMID)
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
.INCLUDE .IGNORE : ucbhelper/version.mk
.IF "$(GUI)"=="OS2"
UCBHELPERLIB=-lucbh$(UCBHELPER_MAJOR)
.ELSE
UCBHELPERLIB=-lucbhelper$(UCBHELPER_MAJOR)$(COMID)
.ENDIF
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
.IF "$(GUI)$(COM)"=="WNTGCC" || "$(GUI)"=="OS2"
REGLIB=-lreg$(UDK_MAJOR)
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
REGLIB=-lreg
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
.INCLUDE .IGNORE : vos/version.mk
VOSLIB=-lvos$(VOS_MAJOR)$(COMID)
XMLOFFLIB=-lxo$(DLLPOSTFIX)
XMLOFFLLIB=-lxol
.IF "$(GUI)$(COM)"=="WNTGCC" || "$(GUI)"=="OS2"
STORELIB=-lstore$(UDK_MAJOR)
SALLIB=-lsal$(UDK_MAJOR)
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
STORELIB=-lstore
SALLIB=-luno_sal
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
.INCLUDE .IGNORE : connectivity/version.mk
ODBCLIB=-lodbc$(DLLPOSTFIX)
ODBCBASELIB=-lodbcbase$(DLLPOSTFIX)
DBFILELIB=-lfile$(DLLPOSTFIX)
.IF "$(GUI)$(COM)"=="WNTGCC" || "$(GUI)"=="OS2"
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
.IF "$(GUI)$(COM)"=="WNTGCC"
MOZBASELIBST=$(STATIC) -lnspr4_s -lxpcombase_s
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
MOZBASELIBST=$(STATIC) -lnspr4 -lxpcombase_s $(DYNAMIC)
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
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
OOXLIB=-loox$(DLLPOSTFIX)
BASEGFXLIB=-lbasegfx$(DLLPOSTFIX)
DRAWINGLAYERLIB=-ldrawinglayer$(DLLPOSTFIX)
SVGIOLIB=-lsvgio$(DLLPOSTFIX)
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
.IF "$(SOLAR_JAVA)" != "" && "$(JDK)" != "gcj" && "$(OS)" != "MACOSX" && "$(OS)" != "OS2"
#i34482# Blackdown/Sun jdk is in the libsearch patch and has a libjpeg :-(
.IF "$(OS)" == "FREEBSD"
JPEG3RDLIB=/usr/local/lib/libjpeg.so
.ELIF "$(CPUNAME)" == "X86_64" || "$(CPUNAME)" == "S390X" || "$(CPUNAME)" == "POWERPC64"
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
.IF "$(SYSTEM_NEON)" == "YES"
NEON3RDLIB=-lneon
.ELIF "$(GUI)$(COM)"=="WNTGCC"
NEON3RDLIB=-lneon
.ELIF "$(OS)" == "MACOSX"
NEON3RDLIB=$(SOLARLIBDIR)/libneon.dylib
.ELSE
NEON3RDLIB=-lneon
.ENDIF
CURLLIB=-lcurl
SFX2LIB=-lsfx$(DLLPOSTFIX)
SFXLIB=-lsfx$(DLLPOSTFIX)
EGGTRAYLIB=-leggtray$(DLLPOSTFIX)
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
.IF "$(GUI)$(COM)"=="WNTGCC" || "$(GUI)"=="OS2"
JVMACCESSLIB = -ljvmaccess$(UDK_MAJOR)$(COMID)
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
JVMACCESSLIB = -ljvmaccess$(COMID)
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
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
.IF "$(GUI)$(COM)"=="WNTGCC" || "$(GUI)"=="OS2"
JVMFWKLIB = -ljvmfwk$(UDK_MAJOR)
.ELSE			# "$(GUI)$(COM)"=="WNTGCC"
JVMFWKLIB = -ljvmfwk
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
.IF "$(SYSTEM_REDLAND)"=="YES"
REDLANDLIB=$(REDLAND_LIBS)
.ELSE
REDLANDLIB=-lrdf
.ENDIF


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
HNJLIB*=-lhyphen
MYSPELLLIB=-lmyspell
COSVLIB=-lcosv
UDMLIB=-ludm
ULINGULIB=-lulingu
.IF "$(SYSTEM_HUNSPELL)" == "YES"
HUNSPELLLIB=$(HUNSPELL_LIBS)
.ELSE
HUNSPELLLIB=-lhunspell-1.3
.ENDIF
.IF "$(SYSTEM_MYTHES)" == "YES"
MYTHESLIB=$(MYTHES_LIBS)
.ELIF "$(GUI)" == "OS2"
MYTHESLIB=-lmythes
.ELSE
MYTHESLIB=-lmythes-1.2
.ENDIF
PYUNOLIB=-lpyuno
COINMPLIBS=-lCoinMP -lCoinUtils -lClp -lCbc -lOsi -lOsiClp -lCgl -lCbcSolver
INTERNAL_APR_LIBS=-lapr-1
INTERNAL_APR_UTIL_LIBS=-laprutil-1
INTERNAL_SERF_LIBS=-lserf-1
SOFFICELIB=-lsofficeapp
UNOPKGAPPLIB=-lunopkgapp
TESTLIB=-ltest
XMLREADERLIB=-lxmlreader

.ELSE				# ("$(GUI)"=="UNX" || "$(COM)"=="GCC")

AWTLIB*=jawt.lib
AVMEDIALIB=iavmedia.lib
ICUINLIB=icuin.lib
ICULELIB=icule.lib
ICUUCLIB=icuuc.lib
ICUDATALIB=icudata.lib
I18NUTILLIB=ii18nutil.lib
I18NISOLANGLIB=ii18nisolang.lib
I18NPAPERLIB=ii18npaper.lib
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
SJLIB=sj.lib
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
EXPAT3RDLIB=expat_xmltok.lib expat_xmlparse.lib
EXPATASCII3RDLIB=expat_xmltok.lib ascii_expat_xmlparse.lib
ZLIB3RDLIB=zlib.lib
JPEG3RDLIB=jpeglib.lib
NEON3RDLIB=ineon.lib
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
OOXLIB=ioox.lib
BASEGFXLIB=ibasegfx.lib
DRAWINGLAYERLIB=idrawinglayer.lib
SVGIOLIB=isvgio.lib
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
XSLTLIB = libxslt.lib $(LIBXML2LIB)
CPPUNITLIB = icppunit_dll.lib
REDLANDLIB = librdf.lib

JVMFWKLIB = ijvmfwk.lib

SABLOT3RDLIB= $(LIBPRE) sablot.lib
APP3RDLIB= $(LIBPRE) app.lib
SAMPLE3RDLIB= $(LIBPRE) sample.lib
HNJLIB*=libhnj.lib
MYSPELLLIB= $(LIBPRE) myspell.lib
COSVLIB= $(LIBPRE) cosv.lib
UDMLIB= $(LIBPRE) udm.lib
ULINGULIB=$(LIBPRE) libulingu.lib
.IF "$(SYSTEM_HUNSPELL)" == "YES"
HUNSPELLLIB=$(HUNSPELL_LIBS)
.ELSE
HUNSPELLLIB=$(LIBPRE) libhunspell.lib
.ENDIF
MYTHESLIB=libmythes.lib
PYUNOLIB=ipyuno.lib
COINMPLIBS=CoinMP.lib
INTERNAL_APR_LIBS=libapr-1.lib
INTERNAL_APR_UTIL_LIBS=iapr-util.lib
INTERNAL_SERF_LIBS=iserf.lib
SOFFICELIB=isofficeapp.lib
UNOPKGAPPLIB=iunopkgapp.lib
TESTLIB=itest.lib
XMLREADERLIB=ixmlreader.lib

.ENDIF              # ("$(GUI)"=="UNX" || "$(COM)"=="GCC")
