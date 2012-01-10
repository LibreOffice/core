#**************************************************************
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
#**************************************************************



$(eval $(call gb_Module_Module,svx))

$(eval $(call gb_Module_add_targets,svx,\
    Library_svx \
    Library_svxcore \
    Library_textconversiondlgs \
    AllLangResTarget_svx \
    AllLangResTarget_ofa \
    AllLangResTarget_gal \
    AllLangResTarget_about \
    AllLangResTarget_textconversiondlgs \
    Package_inc \
    Package_sdi \
))

$(eval $(call gb_Module_add_subsequentcheck_targets,svx,\
    JunitTest_svx_unoapi \
))

#todo: noopt for EnhanceCustomShapesFunctionParser.cxx on Solaris Sparc and MacOSX
#todo: -DBOOST_SPIRIT_USE_OLD_NAMESPACE only in CustomShapes ?
#todo: -DUNICODE and -D_UNICODE on WNT for source/dialog
#todo: globlmn.hrc
#todo: component file
# vim: set noet ts=4 sw=4:
