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

Name: fake-db
Version: 1.0
Release: 0
Summary: This is a dummy package
Group: dummy
License: LGPLv3 with MPLv2 on ALv2
BuildArch: noarch
AutoReqProv: no
%define _tmppath /tmp
#BuildRoot: %{_tmppath}/%{name}-root
Provides: libgnomevfs-2.so.0
Provides: libgnomevfs-2.so.0()(64bit)
Provides: libfreetype.so.6
Provides: libfreetype.so.6()(64bit)
Provides: /bin/sh
Provides: /bin/basename
Provides: /bin/cat
Provides: /bin/cp
Provides: /bin/gawk
Provides: /bin/grep
Provides: /bin/ln
Provides: /bin/ls
Provides: /bin/mkdir
Provides: /bin/mv
Provides: /bin/pwd
Provides: /bin/rm
Provides: /bin/sed
Provides: /bin/sort
Provides: /bin/touch
Provides: /usr/bin/cut
Provides: /usr/bin/dirname
Provides: /usr/bin/expr
Provides: /usr/bin/find
Provides: /usr/bin/tail
Provides: /usr/bin/tr
Provides: /usr/bin/wc
%description
a dummy package
%files
