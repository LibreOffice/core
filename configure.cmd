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
SET JAVA_HOME=d:\os2\java160
SET MY_ANT_HOME=d:/os2/java160/apache-ant-1.7.0
SET PATH=%JAVA_HOME%\bin;%PATH%;%MY_ANT_HOME%\bin;

set config_shell=sh
set perl_sh_dir=/usr/bin

sh ./configure --prefix=/@unixroot/usr --with-system-zlib --with-system-jpeg --without-stlport --with-system-icu --with-system-curl --with-system-python --with-system-openssl --with-system-libxml --with-system-libxslt --disable-werror --disable-mozilla --disable-odk --with-x=no --disable-fontconfig --disable-gnome-vfs --disable-gtk --with-java=yes --disable-gcjaot --without-fonts --with-ant-home=%MY_ANT_HOME% --without-junit --with-system-apr --with-system-apr-util --with-system-serf --with-system-coinmp | tee configure.log
endlocal
