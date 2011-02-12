@echo off
setlocal
SET MY_ANT_HOME=e:/java150/apache-ant-1.7.0
SET JAVA_HOME=e:\java150
SET PATH=%JAVA_HOME%\bin;%PATH%;%MY_ANT_HOME%\bin;
sh -c "configure --with-system-openssl --with-system-icu --with-system-libxml --with-system-libxslt --disable-werror --with-use-shell=bash --disable-mozilla --disable-epm --disable-odk --disable-qadevooo --with-x=no --disable-fontconfig --disable-gnome-vfs --disable-gtk --with-java=yes --without-nas --without-fonts --with-ant-home=%MY_ANT_HOME%" | tee configure.log
endlocal
