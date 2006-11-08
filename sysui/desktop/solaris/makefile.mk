#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: kz $ $Date: 2006-11-08 11:55:50 $
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

PRJ=..$/..

PRJNAME=sysui
TARGET=solaris

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Product Version Information ----------------------------------

.INCLUDE :  ../productversion.mk

# --- Files --------------------------------------------------------

MIMELIST = \
    text \
    text-template \
    spreadsheet \
    spreadsheet-template \
    drawing \
    drawing-template \
    presentation \
    presentation-template \
    formula \
    master-document \
    oasis-text \
    oasis-text-template \
    oasis-spreadsheet \
    oasis-spreadsheet-template \
    oasis-drawing \
    oasis-drawing-template \
    oasis-presentation \
    oasis-presentation-template \
    oasis-formula \
    oasis-master-document \
    oasis-web-template \
    oasis-database

GNOMEMIMEDEPN = ../mimetypes/{$(MIMELIST)}.keys 
  
.IF "$(OS)"=="SOLARIS"

PKGFLAGFILE = $(MISC)$/$(TARGET).flag 
PKGDEPN = \
    $(MISC)/$(TARGET)/openoffice.applications \
    $(MISC)/$(TARGET)/openoffice.mime \
    $(MISC)/$(TARGET)/openoffice.keys \
    $(MISC)/$(TARGET)/printeradmin.sh \
    $(MISC)/$(TARGET)/openoffice.sh \
    $(MISC)/$(TARGET)/space \
    $(MISC)/$(TARGET)/depend \
    $(MISC)/$(TARGET)/pkginfo \
    $(MISC)/$(TARGET)/mailcap \
    $(MISC)/$(TARGET)/postinstall \
    $(MISC)/$(TARGET)/prototype \
    $(MISC)/$(TARGET)/checkinstall	

PKGDIR  = $(shell cd $(BIN); pwd)

PKGNAME = $(shell sed -n -e s/PKG=//p pkginfo)
PKGFILE = $(BIN)$/pkg$/openoffice.org-desktop-integration.tar.gz
   
PKGDATESTRING = $(shell date -u +%Y.%m.%d)
PKGARCH=sparc,i386

.IF "$(WITH_LANG)"!=""
ULFDIR = $(COMMONMISC)$/desktopshare
.ELSE			# "$(WITH_LANG)"!=""
ULFDIR:=..$/share
.ENDIF			# "$(WITH_LANG)"!=""

FASPAC=`test -f $(SOLARBINDIR)/faspac-so.sh && echo "/sbin/sh" || echo "echo"`

.ENDIF
    
# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(OS)"=="SOLARIS"

ALLTAR : $(PKGFILE) 

# --- mime types ---------------------------------------------------

$(MISC)/$(TARGET)/openoffice.keys : $(GNOMEMIMEDEPN) ../productversion.mk ../share/brand.pl ../share/translate.pl $(ULFDIR)/documents.ulf
    @$(MKDIRHIER) $(@:d)
    @echo Creating GNOME .keys file ..
    @echo ---------------------------------
    @$(PERL) ../share/brand.pl -p $(PRODUCTNAME) -u $(UNIXFILENAME) --iconprefix "$(UNIXFILENAME)-" $(GNOMEMIMEDEPN) $(MISC)/$(TARGET)
    @$(PERL) ../share/translate.pl -p $(PRODUCTNAME) -d $(MISC)/$(TARGET) --ext "keys" --key "description"  $(ULFDIR)/documents.ulf
    @cat $(MISC)/$(TARGET)/{$(MIMELIST)}.keys > $@

$(MISC)/$(TARGET)/openoffice.mime : ../mimetypes/openoffice.mime
    @$(MKDIRHIER) $(@:d)
    @echo Creating GNOME .mime file ..
    @echo ---------------------------------
    @cat $< | tr -d "\015" > $@

$(MISC)/$(TARGET)/openoffice.applications : ../productversion.mk ../mimetypes/openoffice.applications
    @$(MKDIRHIER) $(@:d)
    @echo Creating GNOME .applications file ..
    @echo ---------------------------------
    @cat ../mimetypes/openoffice.applications | tr -d "\015" | sed -e "s/openoffice/$(UNIXFILENAME)/" -e "s/%PRODUCTNAME/$(LONGPRODUCTNAME)/" > $@

# --- pkginfo ----------------------------------------------------

# Copy the pkginfo file to $(MISC) 
$(MISC)/$(TARGET)/{pkginfo depend} : $$(@:f) ../productversion.mk
    @$(MKDIRHIER) $(@:d)
    @cat $(@:f) | tr -d "\015" | sed -e "s/%PRODUCTNAME/$(LONGPRODUCTNAME)/g" > $@

# --- prototype ---------------------------------------------------

# Copy the prototype file to $(MISC)
$(MISC)/$(TARGET)/prototype : $$(@:f) ../productversion.mk $(MISC)/cde/$(UNIXFILENAME).flag
    @$(MKDIRHIER) $(@:d)
    @cat $(@:f) | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME)/g" -e "s_%SOURCE_$(MISC)/$(TARGET)_g" > $@
    @pkgproto $(MISC)/cde/types=usr/dt/appconfig/types | awk '{ printf "%s %s %s 0%d%d root bin\n",$$1, $$2, $$3, $$4/100, $$4%10*11 }' >> $@

# --- space, postinstall & mailcap ---------------------------------

# Copy the prototype file to $(MISC)
$(MISC)/$(TARGET)/{space postinstall mailcap} : $$(@:f) ../productversion.mk
    @$(MKDIRHIER) $(@:d)
    @cat $(@:f) | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME)/g" -e "s_%SOURCE_$(MISC)/$(TARGET)_g" > $@

# --- checkinstall -----------------------------------------------

# Copy the checkinstall file to $(MISC) 
$(MISC)/$(TARGET)/checkinstall : $$(@:f)
    @$(MKDIRHIER) $(@:d)
    @cat $(@:f) | tr -d "\015" > $@

# --- office launch scripts --------------------------------------

# Copy the office launch scripts to $(MISC) 
$(MISC)/$(TARGET)/{openoffice.sh printeradmin.sh} : ../share/$$(@:f)
        @$(MKDIRHIER) $(@:d)
        @cat $< | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME)/g" > $@

# --- packaging ---------------------------------------------------

$(PKGFILE) : $(PKGDEPN) makefile.mk
    @-$(RM) $(BIN)$/$(PKGNAME).tar.gz
    @$(MKDIRHIER) $(@:d)
    @pkgmk -r . -f $(MISC)/$(TARGET)/prototype -o -d $(BIN) ARCH=$(PKGARCH) VERSION=$(PKGVERSION),REV=$(PKGREV).$(PKGDATESTRING)
    @$(FASPAC) $(SOLARBINDIR)/faspac-so.sh -a -d $(BIN) $(PKGNAME) 
    @tar -cf - -C $(BIN) $(PKGNAME) | gzip > $@
    @rm -rf $(BIN)/$(PKGNAME)

.ENDIF
