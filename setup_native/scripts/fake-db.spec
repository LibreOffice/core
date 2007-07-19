Name: fake-db
Version: 1.0
Release: 0
Summary: This is a dummy package
Group: dummy
License: LGPL
BuildArch: noarch
AutoReqProv: no
#BuildRoot: %{_tmppath}/%{name}-root
Provides: libgnomevfs-2.so.0
Provides: libgconf-2.so.4
%description
a dummy package
#%prep
%install
mkdir -p bin
touch bin/sh
%files
%attr(0755,root,root) /bin/sh
