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



PRJ=..$/..

PRJNAME=sysui
TARGET=solaris

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Product Version Information ----------------------------------

.INCLUDE :  ../productversion.mk

# --- Files --------------------------------------------------------
 
.IF "$(OS)"=="SOLARIS"

PKGFILES = $(BIN)$/pkg$/{$(PRODUCTLIST)}-desktop-integration.tar.gz
   
PKGDATESTRING = $(shell @date -u +%Y.%m.%d)
PKGARCH=all

FASPAC=`test -f $(SOLARBINDIR)/faspac-so.sh && echo "/sbin/sh" || echo "echo"`

.ENDIF
    
# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(OS)"=="SOLARIS"

ALLTAR : $(PKGFILES) 

# --- pkginfo ----------------------------------------------------

# Copy the pkginfo file to $(MISC) 
$(MISC)/{$(PRODUCTLIST)}/{pkginfo depend} : $$(@:f) ../productversion.mk makefile.mk
    @$(MKDIRHIER) $(@:d)
    @cat $(@:f) | tr -d "\015" | sed -e "s/%PRODUCTNAME/$(PRODUCTNAME.$(@:d:d:f)) $(PRODUCTVERSION.$(@:d:d:f))/g" -e "s/%pkgprefix/$(@:d:d:f:s/.//)/" > $@

# --- mailcap ---------------------------------

# Copy the prototype file to $(MISC)
$(MISC)/{$(PRODUCTLIST)}/mailcap : $$(@:f) ../productversion.mk
    @$(MKDIRHIER) $(@:d)
    @cat $(@:f) | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME.$(@:d:d:f))/g" -e "s_%SOURCE_$(MISC)/$(@:d:d:f)_g" > $@

# --- copyright--------------------------------

# Copy the copyright file to $(MISC) 
$(MISC)/{$(PRODUCTLIST)}/copyright : $$(@:f)
    @$(MKDIRHIER) $(@:d)
    @cat $(@:f) | tr -d "\015" > $@

# --- postinstall--------------------------------


# Copy the postinstall file to $(MISC) 
$(MISC)/{$(PRODUCTLIST)}/postinstall : $$(@:f)
    @$(MKDIRHIER) $(@:d)
    @cat $(@:f) | tr -d "\015" > $@

# --- postremove--------------------------------

# Copy the postremove file to $(MISC) 
$(MISC)/{$(PRODUCTLIST)}/postremove : $$(@:f)
    @$(MKDIRHIER) $(@:d)
    @cat $(@:f) | tr -d "\015" > $@

# --- openoffice.xml---------------------------

.IF "$(common_build)"=="TRUE"
# Copy the apacheopenoffice.xml file to $(MISC) 
#$(MISC)/$(foreach,i,$(PRODUCTLIST) $i/$i.xml) :  $(COMMONMISC)/$$(@:b)/apacheopenoffice.xml
$(MISC)/$(foreach,i,$(PRODUCTLIST) $(MISC)/$i/apacheopenoffice.xml) :  $(COMMONMISC)/$$(@:b)/apacheopenoffice.xml
    $(MKDIRHIER) $(@:d)
    echo hier hier $@
    cat $< | tr -d "\015" > $@
.ENDIF			# "$(COMMON_BUILD)"==""

# --- prototype ---------------------------------------------------

# Copy the prototype file to $(MISC)
$(MISC)/{$(PRODUCTLIST)}$/prototype : $$(@:f) ../productversion.mk makefile.mk
    @$(MKDIRHIER) $(@:d)
    cat $(@:f) | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME.$(@:d:d:f))/g" -e "s_%SOURCE_$(COMMONMISC)/$(@:d:d:f)_g" -e "s/%ICONPREFIX/$(ICONPREFIX.$(@:d:d:f))/g" > $@

# --- packaging ---------------------------------------------------

#$(PKGFILES) : $(MISC)/{$(PRODUCTLIST)}/{copyright pkginfo depend mailcap postinstall postremove} makefile.mk $(MISC)/$$(@:b:b:s/-desktop-integration//)/$$(@:b:b:s/-desktop-integration//).xml
$(PKGFILES) : $(MISC)/{$(PRODUCTLIST)}/{copyright pkginfo depend mailcap postinstall postremove} makefile.mk $(MISC)/$$(@:b:b:s/-desktop-integration//)/apacheopenoffice.xml
$(PKGFILES) : $(MISC)$/{$(PRODUCTLIST)}$/prototype
    @-$(RM) $(BIN)$/$(@:f)
    @$(MKDIRHIER) $(@:d)
    pkgmk -l 1073741824 -r . -f $(MISC)$/$(@:b:b:s/-/ /:1)$/prototype -o -d $(PKGDIR) ARCH=$(PKGARCH) VERSION=$(PKGVERSION.$(@:b:s/-/ /:1)),REV=$(PKGREV).$(PKGDATESTRING) 
.IF "$(DONTCOMPRESS)"==""
    $(FASPAC) $(SOLARBINDIR)/faspac-so.sh -a -d $(PKGDIR) $(@:b:b:s/-/ /:1:s/.//)$(PRODUCTVERSIONSHORT)-desktop-int
.ENDIF # "$(DONTCOMPRESS)"==""
    @tar -cf - -C $(PKGDIR) $(@:b:b:s/-/ /:1:s/.//)$(PRODUCTVERSIONSHORT)-desktop-int | gzip > $@
    @rm -rf $(PKGDIR)/$(@:b:b:s/-/ /:1:s/.//)$(PRODUCTVERSIONSHORT)-desktop-int

.ENDIF
