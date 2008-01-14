#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: ihi $ $Date: 2008-01-14 15:57:48 $
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

PRJ=..
TARGET=osxbundle

# --- Settings -------------------------------------------------

.INCLUDE :	settings.mk

# --- Files ----------------------------------------------------

.IF "$(OS)"!="MACOSX"

dummy:
        @echo "Nothing to build for OS $(OS)"

.ELSE           # "$(OS)"!="MACOSX"

BUNDLE = $(MISC)$/OpenOffice.org.app
CONTENTS = $(BUNDLE)$/Contents
VERSIONED = $(MISC)$/versioned

.IF "$(GUIBASE)"!="aqua"
scriptfiles = \
    $(CONTENTS)$/Resources$/Scripts$/main.scpt	\
    $(CONTENTS)$/Resources$/Scripts$/PostInstall.scpt
.ENDIF

plistfiles  = $(CONTENTS)$/Info.plist

nibfiles = \
    $(CONTENTS)$/Resources$/MainMenu.nib$/classes.nib   \
    $(CONTENTS)$/Resources$/MainMenu.nib$/info.nib   \
    $(CONTENTS)$/Resources$/MainMenu.nib$/keyedobjects.nib

cursorfiles = \
    $(CONTENTS)$/Resources$/cursors$/airbrush.png   \
    $(CONTENTS)$/Resources$/cursors$/ase.png   \
    $(CONTENTS)$/Resources$/cursors$/asn.png   \
    $(CONTENTS)$/Resources$/cursors$/asne.png   \
    $(CONTENTS)$/Resources$/cursors$/asns.png   \
    $(CONTENTS)$/Resources$/cursors$/asnswe.png   \
    $(CONTENTS)$/Resources$/cursors$/asnw.png   \
    $(CONTENTS)$/Resources$/cursors$/ass.png   \
    $(CONTENTS)$/Resources$/cursors$/asse.png   \
    $(CONTENTS)$/Resources$/cursors$/assw.png   \
    $(CONTENTS)$/Resources$/cursors$/asw.png   \
    $(CONTENTS)$/Resources$/cursors$/aswe.png   \
    $(CONTENTS)$/Resources$/cursors$/chain.png   \
    $(CONTENTS)$/Resources$/cursors$/chainnot.png   \
    $(CONTENTS)$/Resources$/cursors$/chart.png   \
    $(CONTENTS)$/Resources$/cursors$/copydata.png   \
    $(CONTENTS)$/Resources$/cursors$/copydlnk.png   \
    $(CONTENTS)$/Resources$/cursors$/copyf.png   \
    $(CONTENTS)$/Resources$/cursors$/copyf2.png   \
    $(CONTENTS)$/Resources$/cursors$/copyflnk.png   \
    $(CONTENTS)$/Resources$/cursors$/crook.png   \
    $(CONTENTS)$/Resources$/cursors$/crop.png   \
    $(CONTENTS)$/Resources$/cursors$/darc.png   \
    $(CONTENTS)$/Resources$/cursors$/dbezier.png   \
    $(CONTENTS)$/Resources$/cursors$/dcapt.png   \
    $(CONTENTS)$/Resources$/cursors$/dcirccut.png   \
    $(CONTENTS)$/Resources$/cursors$/dconnect.png   \
    $(CONTENTS)$/Resources$/cursors$/dellipse.png   \
    $(CONTENTS)$/Resources$/cursors$/detectiv.png   \
    $(CONTENTS)$/Resources$/cursors$/dfree.png   \
    $(CONTENTS)$/Resources$/cursors$/dline.png   \
    $(CONTENTS)$/Resources$/cursors$/dpie.png   \
    $(CONTENTS)$/Resources$/cursors$/dpolygon.png   \
    $(CONTENTS)$/Resources$/cursors$/drect.png   \
    $(CONTENTS)$/Resources$/cursors$/dtext.png   \
    $(CONTENTS)$/Resources$/cursors$/fill.png   \
    $(CONTENTS)$/Resources$/cursors$/help.png   \
    $(CONTENTS)$/Resources$/cursors$/hourglass.png   \
    $(CONTENTS)$/Resources$/cursors$/hshear.png   \
    $(CONTENTS)$/Resources$/cursors$/linkdata.png   \
    $(CONTENTS)$/Resources$/cursors$/linkf.png   \
    $(CONTENTS)$/Resources$/cursors$/magnify.png   \
    $(CONTENTS)$/Resources$/cursors$/mirror.png   \
    $(CONTENTS)$/Resources$/cursors$/movebw.png   \
    $(CONTENTS)$/Resources$/cursors$/movedata.png   \
    $(CONTENTS)$/Resources$/cursors$/movedlnk.png   \
    $(CONTENTS)$/Resources$/cursors$/movef.png   \
    $(CONTENTS)$/Resources$/cursors$/movef2.png   \
    $(CONTENTS)$/Resources$/cursors$/moveflnk.png   \
    $(CONTENTS)$/Resources$/cursors$/movept.png   \
    $(CONTENTS)$/Resources$/cursors$/neswsize.png   \
    $(CONTENTS)$/Resources$/cursors$/notallow.png   \
    $(CONTENTS)$/Resources$/cursors$/nullptr.png   \
    $(CONTENTS)$/Resources$/cursors$/nwsesize.png   \
    $(CONTENTS)$/Resources$/cursors$/pen.png   \
    $(CONTENTS)$/Resources$/cursors$/pivotcol.png   \
    $(CONTENTS)$/Resources$/cursors$/pivotdel.png   \
    $(CONTENTS)$/Resources$/cursors$/pivotfld.png   \
    $(CONTENTS)$/Resources$/cursors$/pivotrow.png   \
    $(CONTENTS)$/Resources$/cursors$/pntbrsh.png   \
    $(CONTENTS)$/Resources$/cursors$/rotate.png   \
    $(CONTENTS)$/Resources$/cursors$/tblsele.png   \
    $(CONTENTS)$/Resources$/cursors$/tblsels.png   \
    $(CONTENTS)$/Resources$/cursors$/tblselse.png   \
    $(CONTENTS)$/Resources$/cursors$/tblselsw.png   \
    $(CONTENTS)$/Resources$/cursors$/tblselw.png   \
    $(CONTENTS)$/Resources$/cursors$/timemove.png   \
    $(CONTENTS)$/Resources$/cursors$/timesize.png   \
    $(CONTENTS)$/Resources$/cursors$/vshear.png   \
    $(CONTENTS)$/Resources$/cursors$/vtext.png    

ZIPFLAGS = -r
ZIP1TARGET = osxbundle
ZIP1DIR = $(CONTENTS)
ZIP1LIST = *

.IF "$(GUIBASE)"!="aqua"
CREATOR_TYPE=OOo2
BUNDLEEXECUTABLE=droplet
.ELSE
CREATOR_TYPE=OOO2
BUNDLEEXECUTABLE=soffice.bin
.ENDIF

# --- Targets --------------------------------------------------

.INCLUDE : target.mk

$(ZIP1TARGETN) : $(scriptfiles) $(plistfiles) $(nibfiles) $(cursorfiles)

$(plistfiles) : $(scriptfiles)

.IF "$(GUIBASE)"!="aqua"
# create application bundle from apple script source
# Info.plist will be deleted to trigger copy rule of our own Info.plist  
$(CONTENTS)$/Resources$/Scripts$/main.scpt : application/main.applescript
    $(RM) -r $(BUNDLE)
    make_versioned.sh "$<" "$(VERSIONED)/$<"
    osacompile -o $(BUNDLE) -c $(CREATOR_TYPE) -s "$(VERSIONED)/$<"
    echo "APPL$(CREATOR_TYPE)" > $(CONTENTS)$/PkgInfo
    $(RM) "$(CONTENTS)$/Info.plist" "$(VERSIONED)/$<"

$(CONTENTS)$/Resources$/Scripts$/%.scpt : application/%.applescript
    make_versioned.sh "$<" "$(VERSIONED)/$<"
    osacompile -d -o "$@" "$(VERSIONED)/$<"
    $(RM) "$(VERSIONED)/$<"
.ENDIF # "$(GUIBASE)"!="aqua"

# Info.plist is just versioned and copied into the bundle	
$(CONTENTS)$/%.plist : application/%.plist
    make_versioned.sh "$<" "$(VERSIONED)/$<"
        sed "s|\%BUNDLEEXECUTABLE|${BUNDLEEXECUTABLE}|g" "$(VERSIONED)/$<" > "$@"

# nibs are just copied
$(CONTENTS)$/Resources$/MainMenu.nib$/%.nib: application$/MainMenu.nib$/%.nib
    $(MKDIRHIER) $(@:d)
    $(COPY) "$<" "$@"

# cursor files are just copied
$(CONTENTS)$/Resources$/cursors$/%.png: application$/cursors$/%.png
    $(MKDIRHIER) $(@:d)
    $(COPY) "$<" "$@"

# The InfoPlist.strings file has to be in UTF-16, thus a back-and-forth conversion
# is needed for versioning
#
# THIS CODE IS PRESERVED FOR FUTURE LOCALIZATION EFFORTS
#$(CONTENTS)$/Resources$/InfoPlist.strings : application/InfoPlist.strings	
#	iconv -f UTF-16 -t UTF-8 "$<" > "$(VERSIONED)/tmp.utf8"
#	make_versioned.sh "$(VERSIONED)/tmp.utf8" "$(VERSIONED)/$<"
#	iconv -f UTF-8 -t UTF-16 "$(VERSIONED)/$<" > "$@"
#	$(RM) "$(VERSIONED)/tmp.utf8" "$(VERSIONED)/$<"
    
.ENDIF		# "$(OS)"!="MACOSX"

