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
# Define additional MSDOS specific settings.
#

# --- Set Wrapper command ---
# Provide a macro that can be used to access the wrapper and to avoid
# hardcoding the program name everywhere
GUWCMD*=guw.exe

# This is a no-op for native W32 dmake
.WINPATH !:= yes

# Directory cache configuration.
.DIRCACHE  *:= no

# Applicable suffix definitions
E *:= .exe	# Executables

# This is hopefully not used. Only in: dmake/msdos/spawn.c  
#  .MKSARGS         *:= yes
   DIVFILE          *=  $(TMPFILE:s,/,${__.DIVSEP-sh-${USESHELL}})
   RM               *=  $/bin$/rm
   MV	            *=  $/bin$/mv
   __.DIVSEP-sh-yes *:= \\\
   __.DIVSEP-sh-no  *:= \\
   DIRSEPSTR :=/

.EXPORT : GUWCMD

# Does not respect case of filenames.
.DIRCACHERESPCASE := no
NAMEMAX	=	256

