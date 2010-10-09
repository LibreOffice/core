'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
' DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'
' Copyright 2000, 2010 Oracle and/or its affiliates.
'
' OpenOffice.org - a multi-platform office productivity suite
'
' This file is part of OpenOffice.org.
'
' OpenOffice.org is free software: you can redistribute it and/or modify
' it under the terms of the GNU Lesser General Public License version 3
' only, as published by the Free Software Foundation.
'
' OpenOffice.org is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU Lesser General Public License version 3 for more details
' (a copy is included in the LICENSE file that accompanied this code).
'
' You should have received a copy of the GNU Lesser General Public License
' version 3 along with OpenOffice.org.  If not, see
' <http://www.openoffice.org/license.html>
' for a copy of the LGPLv3 License.
'
'/************************************************************************
'*
'* owner : marc.neumann@oracle.com
'*
'* short description : Form Filter Test Library
'*
'\***********************************************************************
dim bDatabaseOK as boolean

sub frm_FormFilter
	
	call tLoadForm
	call tFF_Navigator

end sub
'--------------------------------------------------------------------------------------
testcase tLoadForm
    '/// open the text document from dbaccess/optional/input/TT_Form_Filter.odt
    printlog "open the text document from dbaccess/optional/input/TT_Form_Filter.odt" 		
	hFileOpen (gTesttoolPath + ConvertPath("dbaccess/optional/input/TT_Form_Filter.odt"))
	sleep(1)
	Kontext "DocumentWriter"
    '/// press CTRL + F5 to get into the first textbox
    printlog "press CTRL + F5 to get into the first textbox"
	DocumentWriter.TypeKeys "<MOD1 F6>" , true
        wait(1000)
        DocumentWriter.TypeKeys "<MOD1 F5>" , true
	'sleep(1)
	'DocumentWriter.TypeKeys "<MOD1 C>" , true
	'sleep(1)
	'printlog GetClipboard
	sleep(1)
    '/// activate the form based filter
    printlog "activate the form based filter"    
	FM_FormFilter_Start
	wait(1000)
    '/// insert 2 in the first textbox
    printlog "insert 2 in the first textbox"
        DocumentWriter.TypeKeys "<MOD1 F6>" , true
        wait(1000)
        DocumentWriter.TypeKeys "<MOD1 F5>" , true
        sleep(1)
	DocumentWriter.TypeKeys "2" , true
	'DocumentWriter.TypeKeys "<RETURN>" , true
	wait(1000)
    '/// execute the form filter
    printlog "execute the form filter"    
	FM_FF_Execute
	wait(1000)
    printlog "workaround issue 102010"
    DocumentWriter.TypeKeys "<MOD1 F6>" , true
    sleep(1)
    DocumentWriter.TypeKeys "<MOD1 F5>" , true   
    sleep(1)
    '/// use tab to get in the second textbox
    printlog "use tab to get in the second textbox"    
	DocumentWriter.TypeKeys "<TAB>" , true
	wait(1000)
    '/// use CTRL + C to get the text into the Clipboard
    printlog "use CTRL + C to get the text into the Clipboard"
	DocumentWriter.TypeKeys "<MOD1 C>" , true     
	if GetClipboard <> "Value2" then
		warnlog "The field has not the right content. It have to be Test2 but it is " + GetClipboard
	endif
	wait(1000)
    '/// remove the filter
    printlog "remove the filter"
	FM_Remove_Filter
	wait(1000)
    '/// start the form filter again
    printlog "start the form filter again"
	FM_FormFilter_Start
	wait(1000)
    '/// insert into the second textbox like '%2'
    printlog "insert into the second textbox like '%2'"
        DocumentWriter.TypeKeys "<MOD1 F6>" , true
        wait(1000)
        DocumentWriter.TypeKeys "<MOD1 F5>" , true
	sleep(1)
        DocumentWriter.TypeKeys "<TAB>" , true
	sleep(1)
	DocumentWriter.TypeKeys "like '%2'" , true
	wait(1000)
    '/// execute the filter
    printlog "execute the filter"    
	FM_FF_Execute
	wait(1000)
    
    printlog "workaround issue 102010"
    DocumentWriter.TypeKeys "<MOD1 F6>" , true
    sleep(1)
    DocumentWriter.TypeKeys "<MOD1 F5>" , true   
    sleep(1)
    
    '/// use tab to get in the second textbox
    printlog "use tab to get in the second textbox" 
    DocumentWriter.TypeKeys "<TAB>" , true   
    sleep(1)
    '/// select the text in the second textbox
    printlog "select the text in the second textbox"    
	DocumentWriter.TypeKeys "<SHIFT END>" , true
	wait(1000)
    '/// press CTRL + C to get the text in the Clipboard
    printlog "press CTRL + C to get the text in the Clipboard"    
	DocumentWriter.TypeKeys "<MOD1 C>" , true
	if GetClipboard <> "Value2" then
		warnlog "#i41523# The field has not the right content. It have to be Test2 but it is " + GetClipboard
	endif
	wait(1000)
    '/// close the document
    printlog "close the document"	
	call hCloseDocument
	
endcase

'--------------------------------------------------------------------------------------
testcase tFF_Navigator
    '/// open the text document from dbaccess/optional/input/TT_Form_Filter.odt
    printlog "open the text document from dbaccess/optional/input/TT_Form_Filter.odt"
	hFileOpen ( gTesttoolPath + "dbaccess/optional/input/TT_Form_Filter.odt")
	sleep(1)
	Kontext "DocumentWriter"
    '/// press CTRL + F5 to get into the first textbox
    printlog "press CTRL + F5 to get into the first textbox"    
	DocumentWriter.TypeKeys "<MOD1 F5>" , true
	sleep(1)	
    '/// activate the form based filter
    printlog "activate the form based filter"    
	FM_FormFilter_Start
	wait(1000)
    Kontext "DocumentWriter"
    '/// press CTRL + F5 to get into the first textbox
    printlog "press CTRL + F5 to get into the first textbox"
	DocumentWriter.TypeKeys "<MOD1 F5>" , true
	wait(1000)
    '/// insert 1 into the first textbox
    printlog "insert 1 into the first textbox"    
	DocumentWriter.TypeKeys "1" , true
    '/// press return
    printlog "press return"
	DocumentWriter.TypeKeys "<RETURN>" , true
    '/// insert like '%value%' into the second textbox
    printlog "insert like '%value%' into the second textbox"
	DocumentWriter.TypeKeys "like '%value%'" , true
	DocumentWriter.TypeKeys "<RETURN>" , true
	wait(1000)
    Kontext "FilterNavigator"
    '/// open the FormFilterNavigator
    printlog "open the FormFilterNavigator"
    if( NOT FilterNavigator.exists(3)) then
	FM_FF_Navigator
    endif
    wait(1000)
    '/// check if the filter criterion is insert in the form filter navigator
    printlog "check if the filter criterion is insert in the form filter navigator"    
	FilterNavigator.TypeKeys "<END>"
	wait(1000)
    FilterNavigator.TypeKeys "<HOME>"
	wait(1000)
	FilterNavigator.TypeKeys "<DOWN>"	
	wait(1000)
	FilterNavigator.TypeKeys "<ADD>"	
	wait(1000)
	FilterNavigator.TypeKeys "<DOWN>"	
	wait(1000)
	if FilterNavigator.getSelText <> "1" then 
		warnlog "The text is not right in the filter navigator. It have to be 1 but it is " + FilterNavigator.getSelText
	endif	
	wait(1000)	
	FilterNavigator.TypeKeys "<DOWN>"	
	wait(1000)
    if iSprache = 01 then
	    if FilterNavigator.getSelText <> "LIKE '*value*'" then
            warnlog "The text is not right in the filter navigator. It have to be LIKE '*value*' but it is " + FilterNavigator.getSelText
        endif
    elseif iSprache = 33 then
        if FilterNavigator.getSelText <> "COMME '*value*'" then
            warnlog "The text is not right in the filter navigator. It have to be COMME '*value*' but it is " + FilterNavigator.getSelText
        endif
    elseif iSprache = 34 then
        if FilterNavigator.getSelText <> "COMO '*value*'" then
            warnlog "The text is not right in the filter navigator. It have to be COMO '*value*' but it is " + FilterNavigator.getSelText
        endif
    elseif iSprache = 39 then
        if FilterNavigator.getSelText <> "COME '*value*'" then
            warnlog "The text is not right in the filter navigator. It have to be COME '*value*' but it is " + FilterNavigator.getSelText
        endif
    elseif iSprache = 46 then
        if FilterNavigator.getSelText <> "SOM '*value*'" then
            warnlog "The text is not right in the filter navigator. It have to be SOM '*value*' but it is " + FilterNavigator.getSelText
        endif
    elseif iSprache = 49 then
	    if FilterNavigator.getSelText <> "WIE '*value*'" then
            warnlog "The text is not right in the filter navigator. It have to be WIE '*value*' but it is " + FilterNavigator.getSelText
        endif
    elseif iSprache = 50 then
       if FilterNavigator.getSelText <> "LIKE '*value*'" then
             warnlog "The text is not right in the filter navigator. It have to be LIKE '*value*' but it is " + FilterNavigator.getSelText
       endif
    elseif iSprache = 81 then
    
    elseif iSprache = 82 then
    
    elseif iSprache = 86 then
    
    elseif iSprache = 88 then
            
    endif
	wait(1000)	
    '/// close the document
    printlog "close the document"	
	call hCloseDocument

endcase

