'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: f_java_upgrade.bas,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: jsk $ $Date: 2008-06-20 08:03:52 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'*
'/******************************************************************************
'*
'*  owner : Joerg.Skottke@Sun.Com
'*
'*  short description : Collection of all Java related testcases
'*
'\******************************************************************************

sub main

    ' This is a collection of all tests that use java somehow.
    ' It is intended to be the single test that has to be run each time a
    ' new Java version is introduced into the product.
    
    
    use "framework\optional\includes\scripting_basics.inc"     '(tScripting)
    use "framework\optional\includes\scripting_organizers.inc" '(tScriptingOrganizers)
    
    
    dim iDialog as integer ' allowed: 1 (BeanShell) and/or 2 (JavaScript)
    dim iApp as integer    ' Numeric expr. for Doc-Type: Writer, Calc, Draw...
    dim cApp as string   
    
    hDeleteUserTemplates()    
    
    ' ================== Begin Framework Tests =====================
    
    call ExitRestartTheOffice()
    
    call tUpdtWizardLetter()
    call tUpdtWizardFax()
    call tUpdtwizardAgenda()
    call tUpdtWizardWebpage()
    
    call ExitRestartTheOffice()

    call tScripting()
    call tScriptingOrganizers()
    
    call exitRestartTheOffice()
    
    ' cycle through the main applications
    for iApp = 1 to 6
        cApp = hNumericDocType( iApp )
        printlog( "" )
        printlog( iapp & ". " & cApp )
        
        ' 1 = beanshell, 2 = javascript
        for iDialog = 1 to 2
            call tScriptingOrganizers( iDialog )
        next iDialog
    next iApp
    
    ' =================== End Framework Tests ======================
    
    hCloseNavigator()
    call exitRestartTheOffice()
    
    ' ================== Begin Database Tests =====================
    
    Call sDBInit
    call db_hsqldb
    call db_JDBCMySQL
    call wiz_DatabaseWizard
    call wiz_FormWizard
    call wiz_QueryWizard
    call wiz_ReportWizard
    call wiz_TableWizard
    
    
    ' =================== End Database Tests ======================
    
    hDeleteUserTemplates() 
    call exitRestartTheOffice()

end sub

'-------------------------------------------------------------------------

sub LoadIncludeFiles

    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    
    use "global\tools\includes\optional\t_extension_manager_tools.inc"
    use "global\tools\includes\optional\t_security_tools.inc"
    use "global\tools\includes\optional\t_control_objects.inc"
    use "global\tools\includes\optional\f_docfuncs.inc"
    use "global\tools\includes\optional\t_filetools.inc"
    use "global\tools\includes\optional\t_treelist_tools.inc"
    use "global\tools\includes\optional\key_tools.inc"

    use "framework\tools\includes\scriptorganizer_tools.inc"
    use "framework\tools\includes\wizards.inc"
    use "framework\tools\includes\template_tools.inc"
    use "framework\tools\includes\javatools.inc"
    use "framework\tools\includes\performance.inc"

    use "framework\required\includes\wizard_letter.inc"
    use "framework\required\includes\wizard_agenda.inc"
    use "framework\required\includes\wizard_fax.inc"
    use "framework\required\includes\wizard_webpage.inc"

    gApplication = "WRITER"
    call GetUseFiles()
    
    ' ================== Begin Database includes =====================

    use "dbaccess/tools/dbinit.inc"
    use "dbaccess/tools/dbcreatetools.inc"
    use "dbaccess/optional/includes/db_Query.inc"
    
    use "dbaccess/optional/includes/db_hsqldb.inc"
    use "dbaccess/optional/includes/db_JDBCMySQL.inc"
    
    ' wizards
    
    use "dbaccess/optional/includes/wiz_DatabaseWizard.inc"
    use "dbaccess/optional/includes/wiz_FormWizard.inc"
    use "dbaccess/optional/includes/wiz_QueryWizard.inc"
    use "dbaccess/optional/includes/wiz_ReportWizard.inc"
    use "dbaccess/optional/includes/wiz_TableWizard.inc"
    
    ' ================== End Database includes =====================

end sub


