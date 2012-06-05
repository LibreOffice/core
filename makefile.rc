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
# *************************************************************

TARGET=OpenOffice.org
MAKEFILERC=true

# 
# build targets
# 

build_all : build_instsetoo_native

all .PHONY : build_all

check_modules .PHONY :
	@+echo Checking module list
	@+perl $(SOLARENV)$/bin$/build.pl --checkmodules

# Only build when all modules available
build_instsetoo_native .SETDIR=instsetoo_native/prj : check_modules
	@+perl $(SOLARENV)$/bin$/build.pl --all $(PROFULLSWITCH)

depend .SETDIR=instsetoo_native/prj : check_modules
	@+perl $(SOLARENV)$/bin$/build.pl --all $(PROFULLSWITCH) depend=t


# 
# bootstrap target
# 

bootstrap .PHONY :
	@bootstrap


distclean .PHONY: clean
	-rm config.cache
	-rm config.log
.IF "$(BUILD_DMAKE)"!="NO"
	-$(GNUMAKE) -C dmake distclean
.ENDIF


clean .PHONY:
	-rm -rf */$(INPATH)
	-rm -rf solver/*/$(INPATH)
.IF "$(ADDITIONAL_REPOSITORIES)"!=""
	-rm -rf $(foreach,f,$(ADDITIONAL_REPOSITORIES) $f/*/$(INPATH))
.ENDIF
.IF "$(BUILD_DMAKE)"!="NO"
	-echo cleaning up dmake...
	-$(GNUMAKE) -C dmake clean
.ENDIF

# 
# configure target
# 

configure .PHONY SETDIR=. :
	@configure


# 
# install target
# 

install .PHONY :
	@test "$(DESTDIR)$(prefix)" != "" || (echo Usage: make install [DESTDIR=DIR] prefix=DIR; exit 2)
	cp -p instsetoo_native/$(INPATH)/bin/* $(SOLARVER)/$(INPATH)/bin
	cd instsetoo_native/util \
	    && OUT=$(INPATH)\
	    LOCAL_OUT=$(INPATH)\
	    LOCAL_COMMON_OUT=$(INPATH)\
            DEFAULT_TO_ENGLISH_FOR_PACKING=1\
	    PYTHONPATH=$(SOLARVER)/$(INPATH)/bin:$(SOLARVER)/$(INPATH)/lib:$(PYTHONPATH) \
	    $(PERL) $(SOLARENV)/bin/make_installer.pl -f openoffice.lst -l $(WITH_LANG:f:t",") -p OpenOffice -buildid 0 -simple $(DESTDIR)$(prefix)

#*************************************************************************

