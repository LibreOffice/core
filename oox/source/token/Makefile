###############################################################
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
###############################################################



# generate source and header files from text files ----------------------------

SRCDIR_TOKEN := $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
WORKDIR_TOKEN := .

include $(GBUILDDIR)/Output.mk
include $(GBUILDDIR)/BuildDirs.mk
include $(GBUILDDIR)/Helper.mk

# XML namespace identifiers and names

$(WORKDIR_TOKEN)/namespaces.hxx : $(SRCDIR_TOKEN)/namespaces.hxx.head $(WORKDIR_TOKEN)/namespaceids.inc $(SRCDIR_TOKEN)/namespaces.hxx.tail
	$(call gb_Output_announce,$@,build,CAT,1)
	$(call gb_Helper_abbreviate_dirs,cat $^ > $@)

$(WORKDIR_TOKEN)/namespaceids.inc : $(SRCDIR_TOKEN)/namespaces.txt $(SRCDIR_TOKEN)/namespaces.pl
	$(call gb_Output_announce,$@,build,PRL,1)
	$(call gb_Helper_abbreviate_dirs, \
		mkdir -p $(dir $@) && $(PERL) $(SRCDIR_TOKEN)/namespaces.pl 1 < $< > $@ \
	)

$(WORKDIR_TOKEN)/namespacenames.inc : $(SRCDIR_TOKEN)/namespaces.txt $(SRCDIR_TOKEN)/namespaces.pl
	$(call gb_Output_announce,$@,build,PRL,1)
	$(call gb_Helper_abbreviate_dirs, \
		mkdir -p $(dir $@) && $(PERL) $(SRCDIR_TOKEN)/namespaces.pl 2 < $< > $@ \
	)

$(WORKDIR_TOKEN)/namespaces.txt : $(SRCDIR_TOKEN)/namespaces.txt $(SRCDIR_TOKEN)/namespaces.pl
	$(call gb_Output_announce,$@,build,PRL,1)
	$(call gb_Helper_abbreviate_dirs, \
		mkdir -p $(dir $@) && $(PERL) $(SRCDIR_TOKEN)/namespaces.pl 3 < $< > $@ \
	)

# XML token identifiers and names

$(WORKDIR_TOKEN)/tokens.hxx : $(SRCDIR_TOKEN)/tokens.hxx.head $(WORKDIR_TOKEN)/tokenids.inc $(SRCDIR_TOKEN)/tokens.hxx.tail
	$(call gb_Output_announce,$@,build,CAT,1)
	$(call gb_Helper_abbreviate_dirs,cat $^ > $@)

$(WORKDIR_TOKEN)/tokenids.inc : $(SRCDIR_TOKEN)/tokens.txt $(SRCDIR_TOKEN)/tokens.pl
	$(call gb_Output_announce,$@,build,PRL,1)
	$(call gb_Helper_abbreviate_dirs, \
		mkdir -p $(dir $@) && $(PERL) $(SRCDIR_TOKEN)/tokens.pl 1 < $< > $@ \
	)

$(WORKDIR_TOKEN)/tokennames.inc : $(SRCDIR_TOKEN)/tokens.txt $(SRCDIR_TOKEN)/tokens.pl
	$(call gb_Output_announce,$@,build,PRL,1)
	$(call gb_Helper_abbreviate_dirs, \
		mkdir -p $(dir $@) && $(PERL) $(SRCDIR_TOKEN)/tokens.pl 2 < $< > $@ \
	)

$(WORKDIR_TOKEN)/tokenhash.inc : $(WORKDIR_TOKEN)/tokenhash.gperf
	$(call gb_Output_announce,$@,build,GPF,3)
	$(call gb_Helper_abbreviate_dirs, \
		gperf $< | $(gb_AWK) -- '{ if ($$0 !~ /^#line/){ gsub("\\(char\\*\\)0", "0", $$0); print; } }' > $@ \
	)

$(WORKDIR_TOKEN)/tokenhash.gperf : $(SRCDIR_TOKEN)/tokens.txt $(SRCDIR_TOKEN)/tokens.pl
	$(call gb_Output_announce,$@,build,PRL,1)
	$(call gb_Helper_abbreviate_dirs, \
		mkdir -p $(dir $@) && $(PERL) $(SRCDIR_TOKEN)/tokens.pl 3 < $< > $@ \
	)

# UNO property identifiers and names

$(WORKDIR_TOKEN)/properties.hxx : $(SRCDIR_TOKEN)/properties.hxx.head $(WORKDIR_TOKEN)/propertyids.inc $(SRCDIR_TOKEN)/properties.hxx.tail
	$(call gb_Output_announce,$@,build,CAT,1)
	$(call gb_Helper_abbreviate_dirs,cat $^ > $@)

$(WORKDIR_TOKEN)/propertyids.inc : $(SRCDIR_TOKEN)/properties.txt $(SRCDIR_TOKEN)/properties.pl
	$(call gb_Output_announce,$@,build,PRL,1)
	$(call gb_Helper_abbreviate_dirs, \
		mkdir -p $(dir $@) && $(PERL) $(SRCDIR_TOKEN)/properties.pl 1 < $< > $@ \
	)

$(WORKDIR_TOKEN)/propertynames.inc : $(SRCDIR_TOKEN)/properties.txt $(SRCDIR_TOKEN)/properties.pl
	$(call gb_Output_announce,$@,build,PRL,1)
	$(call gb_Helper_abbreviate_dirs, \
		mkdir -p $(dir $@) && $(PERL) $(SRCDIR_TOKEN)/properties.pl 2 < $< > $@ \
	)

.DEFAULT_GOAL:=all
.PHONY: all
all : \
		namespaces.hxx \
		namespaceids.inc \
		namespacenames.inc \
		namespaces.txt \
		tokens.hxx \
		tokenids.inc \
		tokennames.inc \
		tokenhash.inc \
		tokenhash.gperf \
		properties.hxx \
		propertyids.inc \
		propertynames.inc \


# vim: set noet sw=4 ts=4:
