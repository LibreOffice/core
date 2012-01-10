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




$(MISC)/%.cxx : %.y
    @echo "Making:   " $(@:f)
    $(COMMAND_ECHO)tr -d "\015" < $< > $(MISC)/stripped_$<
    $(BISON) $(YACCFLAGS) -o $(YACCTARGET) $(MISC)/stripped_$<
# removing -f switch - avoid getting nothing when copying a file to itself
    @@-$(COPY:s/-f//) $@.h $(INCCOM)/$(@:b).hxx
    @@-$(COPY:s/-f//) $(@:d)/$(@:b).hxx $(INCCOM)/$(@:b).hxx
# fail on not existing .hxx
    $(COMMAND_ECHO)$(TYPE) $(INCCOM)/$(@:b).hxx >& $(NULLDEV)

$(INCCOM)/yy%.cxx : %.y
    @echo "Making:   " $(@:f)
    $(COMMAND_ECHO)tr -d "\015" < $< > $(MISC)/stripped_$<
    $(COMMAND_ECHO)$(BISON) $(YACCFLAGS) -o $(YACCTARGET) $(MISC)/stripped_$<
# removing -f switch - avoid getting nothing when copying a file to itself
    @@-$(COPY:s/-f//) $@.h $(INCCOM)/$(@:b).hxx
    @@-$(COPY:s/-f//) $(@:d)/$(@:b).hxx $(INCCOM)/$(@:b).hxx
# fail on not existing .hxx
    $(COMMAND_ECHO)$(TYPE) $(INCCOM)/$(@:b).hxx >& $(NULLDEV)

