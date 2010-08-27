Name: fake-db
Version: 1.0
Release: 0
Summary: This is a dummy package
Group: dummy
License: LGPL
BuildArch: noarch
AutoReqProv: no
%define _tmppath /tmp
#BuildRoot: %{_tmppath}/%{name}-root
Provides: libgnomevfs-2.so.0
Provides: libgconf-2.so.4
Provides: libfreetype.so.6
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
