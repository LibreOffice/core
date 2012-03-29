# *************************************************************
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
# *************************************************************
These Files belong to the project at:
http://wiki.services.openoffice.org/wiki/Mnemonics_Localisation

' Raffaella Braconi and her l10n team defined these reference files;
' If there is a difference, it is a bug, that has to get defined by her.
' Or checked on the wiki page.

It belongs to the test qa/qatesttool/framework/level1/f_lvl1_menuentries.bas
Which generates the files for a jet unknown language.

Mnemonics Localisation
To try to solve the problem of duplicate or unusual mnemonics in the product the StarOffice QA team has created a testtool script which is able to detect changes in the mnemonics menu entries which may erroneously occur during translation.
The menu items (first level) should have - as far as possible - the same mnemonics for both consistency and usability reasons.
Automating the mnemonic testing it means that initially we would need to invest some time in verifying and, if necessary fix, the mnemonics in the main menus. Based on that information the script would be able to tell us if and what has been changed from version to version.
Thorsten B. from the StarOffice QA team, has now created text documents containing the menu entries with the mnemonics for
de, en-us, es, fr, hu, it, nl, pl, pt-br, ru, sv
based on the OOC680m0 build. 
...
