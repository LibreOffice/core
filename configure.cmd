@echo off
setlocal
SET ANT_HOME=C:\PROGRAMMI\jsdk142\apache-ant-1.7.0
SET JAVA_HOME=C:\PROGRAMMI\jsdk142
SET PATH=%JAVA_HOME%\bin;%PATH%;%ANT_HOME%\bin;
sh -c "configure --disable-werror --with-use-shell=4nt --disable-mozilla --disable-epm --disable-odk --disable-qadevooo --with-x=no --disable-fontconfig --disable-gnome-vfs --disable-gtk --with-java=yes --disable-gcjaot --without-nas --without-fonts" | tee configure.log
endlocal
