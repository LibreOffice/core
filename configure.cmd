@echo off
rem *************************************************************
rem  
rem  Licensed to the Apache Software Foundation (ASF) under one
rem  or more contributor license agreements.  See the NOTICE file
rem  distributed with this work for additional information
rem  regarding copyright ownership.  The ASF licenses this file
rem  to you under the Apache License, Version 2.0 (the
rem  "License")rem you may not use this file except in compliance
rem  with the License.  You may obtain a copy of the License at
rem  
rem    http://www.apache.org/licenses/LICENSE-2.0
rem  
rem  Unless required by applicable law or agreed to in writing,
rem  software distributed under the License is distributed on an
rem  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
rem  KIND, either express or implied.  See the License for the
rem  specific language governing permissions and limitations
rem  under the License.
rem  
rem *************************************************************
setlocal
SET JAVA_HOME=e:\os2\java160
SET MY_ANT_HOME=e:/os2/java160/apache-ant-1.7.0
SET PATH=%JAVA_HOME%\bin;%PATH%;%MY_ANT_HOME%\bin;

set config_shell=sh
set perl_sh_dir=%UNIXROOT%/usr/bin

SET CFG_OPT=
SET CFG_OPT=%CFG_OPT% --prefix=/@unixroot/usr --with-system-zlib --with-system-jpeg
SET CFG_OPT=%CFG_OPT% --without-stlport --with-system-icu --with-system-curl
SET CFG_OPT=%CFG_OPT% --with-system-python --with-system-openssl --with-system-libxml
SET CFG_OPT=%CFG_OPT% --with-system-libxslt --disable-werror
SET CFG_OPT=%CFG_OPT% --disable-odk --with-x=no --disable-fontconfig --disable-gnome-vfs
SET CFG_OPT=%CFG_OPT% --disable-gtk --with-java=yes --disable-gcjaot --without-fonts
SET CFG_OPT=%CFG_OPT% --with-ant-home=%MY_ANT_HOME% --without-junit
SET CFG_OPT=%CFG_OPT% --with-system-apr --with-system-apr-util --with-system-serf
SET CFG_OPT=%CFG_OPT% --enable-bundled-dictionaries --enable-category-b --with-system-nss
SET CFG_OPT=%CFG_OPT% --disable-unit-tests
SET CFG_OPT=%CFG_OPT% --disable-online-update
SET CFG_OPT=%CFG_OPT% --with-system-expat
SET CFG_OPT=%CFG_OPT% --with-lang="de es fr it nl ru"

sh ./configure %CFG_OPT% | tee configure.log

REM auto* breaks PATH for perl, workaround
perl set_soenv

endlocal
