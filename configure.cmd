@echo off
setlocal
SET JAVA_HOME=d:\os2\java160
SET MY_ANT_HOME=d:/os2/java160/apache-ant-1.7.0
SET PATH=%JAVA_HOME%\bin;%PATH%;%MY_ANT_HOME%\bin;
rem set grep=grep.exe

set config_shell=sh
set perl_sh_dir=/usr/bin

rem  --with-system-icu --disable-epm
sh ./configure --disable-binfilter --with-system-zlib --with-system-jpeg --without-stlport --with-system-icu --with-system-curl --with-system-python --with-system-openssl --with-system-libxml --with-system-libxslt --disable-werror --disable-mozilla --disable-odk --with-x=no --disable-fontconfig --disable-gnome-vfs --disable-gtk --with-java=yes --disable-gcjaot --without-fonts --with-ant-home=%MY_ANT_HOME% --without-junit | tee configure.log
endlocal
