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
# Define additional OS/2 specific macros.
#

# Process release-specific refinements, if any.
# YD .INCLUDE .NOINFER .IGNORE : $(INCFILENAME:d)$(OSRELEASE)$/macros.mk

# Directory entries are case incensitive
.DIRCACHERESPCASE *:= no

# tmp is already imported - use it!
TMPDIR *= $(TMP)

# Applicable suffix definitions
A *:= .lib	# Libraries
E *:= .exe	# Executables
F *:= .for	# Fortran
O *:= .obj	# Objects
P *:= .pas	# Pascal
S *:= .asm	# Assembler sources
V *:= 		# RCS suffix

.MKSARGS         *:= yes
RM               *=  rm
MV	         *=  mv
DIVFILE          *=  $(TMPFILE:s,/,${__.DIVSEP-sh-${USESHELL}})
__.DIVSEP-sh-yes !:= \\\
__.DIVSEP-sh-no  !:= \\

NAMEMAX		=	256

