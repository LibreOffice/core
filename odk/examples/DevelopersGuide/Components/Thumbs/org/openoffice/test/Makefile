#*************************************************************************
#
#  $RCSfile: Makefile,v $
#
#  $Revision: 1.4 $
#
#  last change: $Author: rt $ $Date: 2004-05-18 13:22:14 $
#
#  The Contents of this file are made available subject to the terms of
#  the BSD license.
#  
#  Copyright (c) 2003 by Sun Microsystems, Inc.
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. Neither the name of Sun Microsystems, Inc. nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
#  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
#  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
#  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#     
#**************************************************************************

# Builds the SpreadSheet examples of the Developers Guide.

PRJ=../../../../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

include ../../../thumbs.mk

IDLFILES = XImageShrink.idl

IDLFILES2 = ImageShrink.idl

PACKAGE = org/openoffice/test

GENJAVAFILES = $(patsubst %.idl,$(OUT_COMP_GEN)/$(PACKAGE)/%.java,$(IDLFILES))
GENURDFILES = $(patsubst %.idl,$(OUT_COMP_GEN)/%.urd,$(IDLFILES))
GENURDFILES += $(patsubst %.idl,$(OUT_COMP_GEN)/%.urd,$(IDLFILES2))

CLASSFILES = $(subst $(OUT_COMP_GEN),$(OUT_COMP_CLASS),$(GENJAVAFILES:.java=.class))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))


# Targets
.PHONY: ALL
ALL : \
	$(CLASSFILES)

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_GEN)/%.urd : %.idl
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(IDLC) -I. -I../../.. -I$(IDL_DIR) -O$(OUT_COMP_GEN) $^

$(OUT_COMP_GEN)/%.rdb : $(GENURDFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $(subst /,$(PS),$@)
	$(REGMERGE) $@ /UCR $(GENURDFILES)

$(OUT_COMP_GEN)/%.java : $(COMPONENT_RDB)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(JAVAMAKER) -BUCR -nD -T$(subst /,.,$(patsubst $(OUT_COMP_GEN)/%.java,%,$@)) -O$(OUT_COMP_GEN) $(COMPONENT_RDB) $(DKREGISTRYNAME)

$(CLASSFILES) : $(GENJAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(GENJAVAFILES)

.PHONY: clean
clean :
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(CLASSFILES)))
