#*************************************************************************
#*
#*  $RCSfile: makefile.mk,v $
#*
#*  $Revision: 1.12 $
#*
#*  last change: $Author: rt $ $Date: 2000-11-22 15:07:58 $
#*
#*  The Contents of this file are made available subject to the terms of
#*  either of the following licenses
#*
#*         - GNU Lesser General Public License Version 2.1
#*         - Sun Industry Standards Source License Version 1.1
#*
#*  Sun Microsystems Inc., October, 2000
#*
#*  GNU Lesser General Public License Version 2.1
#*  =============================================
#*  Copyright 2000 by Sun Microsystems, Inc.
#*  901 San Antonio Road, Palo Alto, CA 94303, USA
#*
#*  This library is free software; you can redistribute it and/or
#*  modify it under the terms of the GNU Lesser General Public
#*  License version 2.1, as published by the Free Software Foundation.
#*
#*  This library is distributed in the hope that it will be useful,
#*  but WITHOUT ANY WARRANTY; without even the implied warranty of
#*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#*  Lesser General Public License for more details.
#*
#*  You should have received a copy of the GNU Lesser General Public
#*  License along with this library; if not, write to the Free Software
#*  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#*  MA  02111-1307  USA
#*
#*
#*  Sun Industry Standards Source License Version 1.1
#*  =================================================
#*  The contents of this file are subject to the Sun Industry Standards
#*  Source License Version 1.1 (the "License"); You may not use this file
#*  except in compliance with the License. You may obtain a copy of the
#*  License at http://www.openoffice.org/license.html.
#*
#*  Software provided under this License is provided on an "AS IS" basis,
#*  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#*  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#*  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#*  See the License for the specific provisions governing your rights and
#*  obligations concerning the Software.
#*
#*  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#*
#*  Copyright: 2000 by Sun Microsystems, Inc.
#*
#*  All Rights Reserved.
#*
#*  Contributor(s): _______________________________________
#*
#*
#*************************************************************************

PRJ=..

PRJNAME=svtools
TARGET=svtool
RESTARGET=svt
RESTARGETSIMPLE=svs
VERSION=$(UPD)
GEN_HID=TRUE
ENABLE_EXCEPTIONS=TRUE
# --- Settings -----------------------------------------------------

.INCLUDE :	svpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	sv.mk

USE_LDUMP2=TRUE

.IF "$(COM)"=="ICC"
LINKFLAGS+=/SEGMENTS:1024 /PACKD:32768
.ENDIF

# --- Allgemein ----------------------------------------------------

LIB1TARGET= $(LB)$/svmem.lib
LIB1ARCHIV=  $(LB)$/libsvarray.a
LIB1FILES=	$(LB)$/svarray.lib

LIB2TARGET=   $(LB)$/filearch.lib
LIB2OBJFILES= $(OBJ)$/filearch.obj

.IF "$(GUI)"!="UNX"
LIB3TARGET= $(LB)$/svtool.lib
LIB3FILES=	$(LB)$/_svt.lib
LIB3OBJFILES=\
    $(OBJ)$/rtfkey2.obj
.ENDIF

.IF "$(GUI)"!="UNX"
LIB4TARGET= $(LB)$/isvl.lib
LIB4FILES=	$(LB)$/_isvl.lib
LIB4OBJFILES=\
    $(OBJ)$/htmlkey2.obj
.ENDIF

LIB7TARGET= $(SLB)$/svt.lib
#LIB7ARCHIV= $(LB)$/libsvt$(UPD)$(DLLPOSTFIX).a
LIB7FILES=	\
        $(SLB)$/misc2.lib		\
        $(SLB)$/items2.lib		\
        $(SLB)$/browse.lib		\
        $(SLB)$/ctrl.lib		\
        $(SLB)$/dialogs.lib 	\
        $(SLB)$/edit.lib		\
        $(SLB)$/unoiface.lib	\
        $(SLB)$/filter.lib		\
        $(SLB)$/igif.lib		\
        $(SLB)$/ipng.lib		\
        $(SLB)$/jpeg.lib		\
        $(SLB)$/ixpm.lib		\
        $(SLB)$/ixbm.lib		\
        $(SLB)$/sbx.lib 		\
        $(SLB)$/numbers.lib 	\
        $(SLB)$/wmf.lib 		\
        $(SLB)$/undo.lib		\
        $(SLB)$/urlobj.lib		\
        $(SLB)$/plugapp.lib 	\
        $(SLB)$/svcontnr.lib	\
        $(SLB)$/svhtml2.lib


.IF "$(GUI)" == "OS2"
LIB7FILES+= 	$(SLB)$/eaimp.lib
.ENDIF

.IF "$(GUI)" == "UNX"
LIB7FILES+= 	$(SLB)$/eaimp.lib
.ENDIF

LIB7FILES+= \
            $(SLB)$/svdde.lib

LIB8TARGET= $(SLB)$/svl.lib
#LIB8ARCHIV= $(LB)$/libsvl$(UPD)$(DLLPOSTFIX).a
LIB8OBJFILES= $(SLO)$/svtdata.obj
LIB8FILES=	\
        $(SLB)$/config.lib	\
        $(SLB)$/filerec.lib \
        $(SLB)$/items1.lib	\
        $(SLB)$/misc1.lib	\
        $(SLB)$/notify.lib	\
        $(SLB)$/svarray.lib \
        $(SLB)$/svhtml1.lib \
        $(SLB)$/svrtf.lib	\
        $(SLB)$/svsql.lib	\
        $(SLB)$/cmdparse.lib

# Bilden der dyn. Resourcen-DLL --------------------------------------------------

RESLIB1NAME=	$(RESTARGET)
RESLIB1SRSFILES=$(SRS)$/filter.srs		\
        $(SRS)$/misc.srs		\
        $(SRS)$/sbx.srs 		\
        $(SRS)$/ctrl.srs		\
        $(SRS)$/items.srs		\
        $(SRS)$/dialogs.srs \
        $(SRS)$/plugapp.srs \
        $(SRS)$/uno.srs
RESLIB2NAME=	$(RESTARGETSIMPLE)
RESLIB2SRSFILES=\
    $(SRS)$/mediatyp.srs

# Bilden der DLL --------------------------------------------------

SHL1TARGET= svt$(VERSION)$(DLLPOSTFIX)
SHL1IMPLIB= _svt
SHL1STDLIBS=$(TOOLSLIB)			\
            $(VOSLIB)			\
            $(CPPULIB)			\
            $(CPPUHELPERLIB)	\
            $(RTLLIB)			\
            $(SVLIB)			\
            $(TKLIB)			\
            $(SVLLIB)			\
            $(SALLIB)			\
            $(UNOTOOLSLIB)		\
            $(COMPHELPERLIB)		\
            $(UCBHELPERLIB)


.IF "$(GUI)"=="WIN"
SHL1STDLIBS+= \
        $(LIBPRE) ddeml.lib \
            $(LIBPRE) shell.lib 	\
            $(LIBPRE) ver.lib
.ENDIF

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= \
        $(LIBCIMT) \
        $(LIBPRE) advapi32.lib	\
        $(LIBPRE) gdi32.lib
.ENDIF

.IF "$(VCL)" != ""
SHL1STDLIBS+= \
            $(SOTLIB)
.ENDIF

SHL1STDLIBS += $(JPEG3RDLIB)

#SHL1DEPNU=$(LB)$/isvl.lib
#SHL1DEPNU=$(SHL2TARGETN)
SHL1LIBS= \
        $(SLB)$/svt.lib 	\
        $(LB)$/svmem.lib

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME =svt
DEF1DES 	=SvTools

.IF "$(GUI)"=="WNT"
DEF1EXPORT1 = component_writeInfo
DEF1EXPORT2 = component_getFactory
.ELSE
.IF "$(COM)"=="ICC"
DEF1EXPORT1 = component_writeInfo
DEF1EXPORT2 = component_getFactory
.ELSE
DEF1EXPORT1 = _component_writeInfo
DEF1EXPORT2 = _component_getFactory
.ENDIF
.ENDIF

# --- svtools lite --------------------------------------------------

SHL2TARGET= svl$(VERSION)$(DLLPOSTFIX)
SHL2IMPLIB= _isvl
SHL1OBJS=$(SLO)$/svtdata.obj
SHL2STDLIBS=$(TOOLSLIB) 	\
        $(VOSLIB)			\
        $(CPPULIB)			\
        $(COMPHELPERLIB)		\
        $(CPPUHELPERLIB)	\
        $(RTLLIB)			\
        $(SALLIB)			\
        $(UNOTOOLSLIB)

.IF "$(GUI)"=="WNT"
SHL2STDLIBS+= \
        $(LIBCIMT) \
            $(LIBPRE) advapi32.lib	\
            $(LIBPRE) gdi32.lib
.ENDIF

SHL2LIBS=	$(SLB)$/svl.lib

SHL2DEF=	$(MISC)$/$(SHL2TARGET).def

DEF2NAME=	$(SHL2TARGET)
DEF2DEPN=	$(MISC)$/$(SHL2TARGET).flt $(SLB)$/svl.lib
DEFLIB2NAME=svl
DEF2DES =SvTools lite

# --- Targets ------------------------------------------------------

.IF "$(GUI)"=="UNX"
SVTTARGETS= $(LB)$/lib$(SHL2TARGET)$(DLLPOST) $(LB)$/lib$(SHL1TARGET)$(DLLPOST)
.ELSE
SVTTARGETS= $(LB)$/isvl.lib \
            $(BIN)$/$(SHL2TARGET)$(DLLPOST) $(BIN)$/$(SHL1TARGET)$(DLLPOST)
.ENDIF

ALL: $(SLB)$/svl.lib \
    $(SLB)$/svt.lib \
    $(MISC)$/$(SHL2TARGET).flt \
    $(MISC)$/$(SHL1TARGET).flt \
    $(MISC)$/$(SHL2TARGET).def \
    $(MISC)$/$(SHL1TARGET).def \
    $(SVTTARGETS) \
    ALLTAR

.INCLUDE :	target.mk

# --- Svtools-Control-Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo AUTO_CODE > $@
    @echo BrowserDataWin >> $@
    @echo BrowserColumn >> $@
    @echo ButtonFrame >> $@
    @echo CreateLoader >> $@
    @echo DdeString >> $@
    @echo DlgSource >> $@
    @echo DlgExport >> $@
    @echo EditWindow >> $@
    @echo FileEntry >> $@
    @echo GIFLZWDecompressor >> $@
    @echo GIFReader >> $@
    @echo PNGReader >> $@
    @echo ImpDetect >> $@
    @echo ImpDraw >> $@
    @echo ImpGet >> $@
    @echo ImpPut >>$@
    @echo ImpSv >> $@
    @echo JPEGReader >> $@
    @echo WMFReader >> $@
    @echo EnhWMFReader >> $@
    @echo WMFWriter >> $@
    @echo LinkStub >> $@
    @echo MultiTextLineInfo >> $@
    @echo Regexpr >> $@
    @echo RemoteControlServer >> $@
    @echo RemoteControlService >> $@
    @echo RetStream >> $@
    @echo SCmdStream >> $@
    @echo SbxArrayRef >> $@
    @echo SbxBasicFormater >> $@
    @echo SbxRes >> $@
    @echo SbxVariableRef >> $@
    @echo SfxListUndoAction >> $@
    @echo SfxPointerEntry >> $@
    @echo SfxUINT32s >> $@
    @echo SfxULongRangeItem >> $@
    @echo SfxULongRanges >> $@
    @echo SfxULongRangesItem >> $@
    @echo SgfFontLst >> $@
    @echo SgfFontOne >> $@
    @echo StatementCommand >> $@
    @echo StatementControl >> $@
    @echo StatementFlow >> $@
    @echo StatementList >> $@
    @echo StatementSlot >> $@
    @echo SVDDE_MISC_CODE >> $@
    @echo SvInterfaceClient >> $@
    @echo SvImp >> $@
    @echo SvStringLockBytes >> $@
    @echo svtools >> $@
    @echo SVTOOLS_FILTER >> $@
    @echo SVTOOLS_CODE >> $@
    @echo SVTOOLS_FILTER4 >> $@
    @echo SVTOOLS_FILTER_CODE >> $@
    @echo ValueSetItem >> $@
    @echo WMFRecord >> $@
    @echo WriteRecord >> $@
    @echo XPMReader >> $@
    @echo XBMReader >> $@
    @echo _grow >> $@
    @echo _ImplINetHistoryLoadTimer >> $@
    @echo _ImplINetHistoryStoreTimer >> $@
    @echo sRTF_>>$@
    @echo sHTML_>>$@
    @echo unnamed>>$@
.IF "$(GUI)"=="WNT" || "$(GUI)"=="WIN"
    @echo WEP>>$@
    @echo LIBMAIN>>$@
    @echo LibMain>>$@
.ENDIF
.IF "$(COM)"=="MSC"
    @echo ??_7>>$@
    @echo ??_8>>$@
    @echo ??_C>>$@
    @echo ??_E>>$@
    @echo ??_F>>$@
    @echo ??_G>>$@
    @echo ??_H>>$@
    @echo ??_I>>$@
    @echo 0Imp>>$@
    @echo Impl@@>>$@
    @echo Imp@@>>$@
    @echo __CT>>$@
    @echo _STL>>$@
.ENDIF

$(MISC)$/$(SHL2TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo AUTO_CODE > $@
    @echo BrowserDataWin >> $@
    @echo BrowserColumn >> $@
    @echo ButtonFrame >> $@
    @echo CreateLoader >> $@
    @echo DdeString >> $@
    @echo DlgSource >> $@
    @echo DlgExport >> $@
    @echo EditWindow >> $@
    @echo FileEntry >> $@
    @echo GIFLZWDecompressor >> $@
    @echo GIFReader >> $@
    @echo PNGReader >> $@
    @echo ImpDetect >> $@
    @echo ImpDraw >> $@
    @echo ImpGet >> $@
    @echo ImpPut >>$@
    @echo ImpSvNum >>$@
    @echo JPEGReader >> $@
    @echo WMFReader >> $@
    @echo EnhWMFReader >> $@
    @echo WMFWriter >> $@
    @echo LinkStub >> $@
    @echo MultiTextLineInfo >> $@
    @echo Regexpr >> $@
    @echo RemoteControlServer >> $@
    @echo RemoteControlService >> $@
    @echo RetStream >> $@
    @echo SCmdStream >> $@
    @echo SbxArrayRef >> $@
    @echo SbxBasicFormater >> $@
    @echo SbxRes >> $@
    @echo SbxVariableRef >> $@
    @echo SfxListUndoAction >> $@
    @echo SfxPointerEntry >> $@
    @echo SfxUINT32s >> $@
    @echo SfxULongRangeItem >> $@
    @echo SfxULongRanges >> $@
    @echo SfxULongRangesItem >> $@
    @echo SgfFontLst >> $@
    @echo SgfFontOne >> $@
    @echo StatementCommand >> $@
    @echo StatementControl >> $@
    @echo StatementFlow >> $@
    @echo StatementList >> $@
    @echo StatementSlot >> $@
    @echo SVDDE_MISC_CODE >> $@
    @echo SvInterfaceClient >> $@
    @echo SvImp >> $@
    @echo SvStringLockBytes >> $@
    @echo svtools >> $@
    @echo SVTOOLS_FILTER >> $@
    @echo SVTOOLS_CODE >> $@
    @echo SVTOOLS_FILTER4 >> $@
    @echo SVTOOLS_FILTER_CODE >> $@
    @echo ValueSetItem >> $@
    @echo WMFRecord >> $@
    @echo WriteRecord >> $@
    @echo XPMReader >> $@
    @echo XBMReader >> $@
    @echo _grow >> $@
    @echo _ImplINetHistoryLoadTimer >> $@
    @echo _ImplINetHistoryStoreTimer >> $@
    @echo sRTF_>>$@
    @echo sHTML_>>$@
    @echo SdbSqlScanner>>$@
    @echo Sdbyy_scan>>$@
    @echo SqlInternalNode>>$@
    @echo unnamed>>$@
.IF "$(GUI)"=="WNT" || "$(GUI)"=="WIN"
    @echo WEP>>$@
    @echo LIBMAIN>>$@
    @echo LibMain>>$@
.ENDIF
.IF "$(COM)"=="MSC"
    @echo ??_7>>$@
    @echo ??_8>>$@
    @echo ??_C>>$@
    @echo ??_E>>$@
    @echo ??_F>>$@
    @echo ??_G>>$@
    @echo ??_H>>$@
    @echo ??_I>>$@
    @echo 0Imp>>$@
    @echo Impl@@>>$@
    @echo Imp@@>>$@
    @echo __CT>>$@
    @echo _STL>>$@
.ENDIF

