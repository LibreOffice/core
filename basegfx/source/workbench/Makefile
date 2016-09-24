#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

# Testbuild

#test : bezierclip.cxx convexhull.cxx
#	g++ -Wall -g \
#	-I. -I. -I../inc -I./inc -I./unx/inc -I./unxlngi4/inc -I. -I/develop4/update/SRX644/unxlngi4/inc.m4/stl -I/develop4/update/SRX644/unxlngi4/inc.m4/external -I/develop4/update/SRX644/unxlngi4/inc.m4 -I/develop4/update/SRX644/src.m4/solenv/unxlngi4/inc  -I/net/grande/develop6/update/dev/gcc_3.0.1_linux_libc2.11_turbolinux/include -I/develop4/update/SRX644/src.m4/solenv/inc -I/develop4/update/SRX644/unxlngi4/inc.m4/stl -I/net/grande.germany/develop6/update/dev/gcc_3.0.1_linux_libc2.11_turbolinux/redhat60/usr/include -I/net/grande.germany/develop6/update/dev/gcc_3.0.1_linux_libc2.11_turbolinux/redhat60/usr/include/X11 -I/develop4/update/SRX644/src.m4/res -I/net/grande/develop6/update/dev/Linux_JDK_1.4.0/include -I/net/grande/develop6/update/dev/Linux_JDK_1.4.0/include/linux -I. -I./res -I. \
#	-include preinclude.h -D_USE_NAMESPACE -DGLIBC=2 -D_USE_NAMESPACE=1 -D_DEBUG_RUNTIME \
#	bezierclip.cxx convexhull.cxx -o bezierclip

prog : bezierclip.cxx convexhull.cxx
	g++ -Wall -g bezierclip.cxx convexhull.cxx -o bezierclip

test : testconvexhull.cxx
	g++ -Wall -g testconvexhull.cxx -o testhull

.cxx.o:
	g++ -c $(LOCALDEFINES) $(CCFLAGS) $<
