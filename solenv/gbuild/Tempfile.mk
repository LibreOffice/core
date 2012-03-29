# *************************************************************
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
# Write string to temporary file by chopping into pieces that
# fit the commandline
# parameters: filename, maxitems (for one write), string
# returns: filename
define var2file
$(strip $(1)
$(eval gb_var2file_helpervar := $$(shell printf "%s" "" > $(1) ))\
$(foreach item,$(3),$(eval gb_var2file_curblock += $(item)
    ifeq ($$(words $$(gb_var2file_curblock)),$(2)) 
        gb_var2file_helpervar := $$(shell printf "%s" "$$(gb_var2file_curblock)" >> $(1) )
        gb_var2file_curblock :=
    endif
    ))\
    $(eval gb_var2file_helpervar := $(shell echo " "$(gb_var2file_curblock) >> $(1) )
        gb_var2file_curblock :=
    ))
endef

define var2filecr
$(strip $(1)
$(foreach item,$(strip $(3)),$(eval gb_var2file_curblock += $(item)
    ifeq ($$(words $$(gb_var2file_curblock)),$(2)) 
        gb_var2file_helpervar :=$$(shell echo $$(gb_var2file_curblock) >> $(1) )
        gb_var2file_curblock :=
    endif
    )) \
	)
endef

