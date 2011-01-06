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
'* Owner : wolfram.garten@oracle.com
'*
'* short description : Tests the header-footer-function in Impress
'*
'*******************************************************************

testcase tiHeaderFooterSlide

    dim numbervalue1 as integer
    dim value2 as String
    dim value3 as String

    printlog "This part checks the first Tabpage of the Header and Footer Dialog"

    printlog "Open New document"
    Call hNewDocument		
    kontext "drawimpress"
    try
        printlog "View - Header And Footer"
        ViewHeaderAndFooter
       sleep(1)
    catch
       warnlog " Could'nt open ViewHeaderAndFooter!" 
    endcatch
    
    printlog " Testing TabHeaderFooterSlide."
    kontext "TabHeaderFooterSlide"

    printlog "Testing date and time checkbox"
    if DateTime.Exists = TRUE AND DateTimeFixed.isVisible = TRUE then 
        DateTime.UnCheck
        sleep(1)
        DateTime.Check
        sleep(1)
        
        if DateTime.IsChecked then
            printlog " DateTimeFormat-checkbox works."
        else
            warnlog " Something wrong with the DateTime-checkbox!"
        endif
        
    else
        warnlog "DateTime does not exist or is not visible."
    endif
    
    printlog "Checking if DateTime checkbox setting is remembered..."
    DateTime.Uncheck
    printlog " Date Time is unchecked now."
    Apply.Click
    ViewHeaderAndFooter
    
    if DateTime.isChecked then
        warnlog " Settings of Date Time not remembered!"
    else
        printlog " Setting of date Time is remembered."
    endif
    DateTime.Check

   printlog "Check DateTimeFixed can be checked/unchecked"
    if DateTimeFixed.Exists = TRUE AND DateTimeFixed.isVisible = TRUE then
        DateTimeFixed.Check
        sleep(1)
        
        if DateTimeFixed.isChecked = TRUE AND DateTimeVariable.IsChecked = FALSE then
            printlog " Datetimefixed-checkbox seem to work."
        else
            warnlog " Datetimefixed-checkbox isnt checked or DateTimeVariable is checked!"
        endif

    else
        warnlog " DateTimeFixed doesnt exist or is not visible!"
    endif

    printlog "Can text be inserted in the fixed-field?"
    if DateTimeFixedText.exists = TRUE AND DateTimeFixedText.isVisible = TRUE then
        DateTimeFixedText.settext "lalala"
        
        if DateTimeFixedText.gettext <> "lalala" then 
            errorlog " Could'nt write a text to the DateTimeFixedText-field!"
        else
            printlog " Text is the same."
        endif
        
    else
        warnlog " DateTimeFixedText doesnt exist or is not visible!"
    endif
      
    printlog "Checking if DateTimeFixed and Text setting is remembered..."
    DateTime.Check
    DateTimeFixed.Check
    DateTimeFixedText.settext "lalala"
    Apply.Click
    ViewHeaderAndFooter
    
    if DateTimeFixedText.gettext <> "lalala" then 
        warnlog " Text in DateTimeFixedText is not the same!"
    else
        printlog " Text was saved."
    endif
    DateTimeFixedText.settext ""

    printlog "Can I change to Variable and check it?"
    if DateTimeVariable.exists = TRUE AND DateTimeVariable.isVisible = TRUE then
        DateTimeVariable.Check
        sleep(1)
        
        if DateTimeVariable.IsChecked AND DateTimeFixed.IsChecked = FALSE then
            printlog " DateTimeVariable-checkbox works."
        else
            warnlog " Something wrong with the DateTimeVariable-checkbox or DateTimeFormat is checked!"
        endif
        
    else
        warnlog " DateTimeVariable doesnt exist or is not visible!"
    endif
    
    printlog "Is the Variable setting saved?"
    Apply.Click
    ViewHeaderAndFooter
    if DateTimeVariable.isChecked then
        printlog " Variable Setting gets saved."
    else
        warnlog " Variable setting does not get saved!"
    endif

    printlog "Checking DateTimeFormat box"
    if DateTimeFormat.exists = TRUE AND DateTimeFormat.isVisible = TRUE then
        printlog "Get the number of choices"
        numbervalue1 = DateTimeFormat.getitemcount     
        printlog " DateTimeFormat equals: " + numbervalue1
        printlog "Get the text for the choosen entry"
        value2 = DateTimeFormat.getseltext             
        printlog " Tries to get ", numbervalue1 & "."
        printlog "Select the last entry"
        DateTimeFormat.select ( numbervalue1 - 1 )
        printlog "Get the text for the last entry"
        value3 = DateTimeFormat.getseltext             
        printlog " DateTimeFormat is: " + value3
        
        if value2 = value3 then
            printlog "Check if the first and last are the same."
            errorlog " More options for Date should exist!"
            printlog "Choose the original state"
            DateTimeFormat.select value2                   
        else
            printlog " DateTimeFormat box works."
        endif
        
        printlog "Checking if settings of DateTimeFormat box get saved"
        Apply. Click
        ViewHeaderAndFooter
        if DateTimeFormat.GetSelText = value3 then
            printlog " DateTimeFormat settings get saved."
        else
            warnlog " DateTimeFormat settings did not get saved!"
        endif
        
    else
        warnlog " DateTimeFormat doesnt exist or is not visible."
    endif

    printlog "Checking DateTimeLanguage box"
    if DateTimeLanguage.exists = TRUE AND DateTimeLanguage.isVisible = TRUE then
        printlog "Get the number of choices "
        numbervalue1 = DateTimeLanguage.GetItemCount
        printlog "Choose the first entry "
        DateTimeFormat.select 1
        printlog "Get the text for the choosen entry "
        value2 = DateTimeLanguage.GetSelText
        printlog "Select the last entry "
        DateTimeLanguage.select ( numbervalue1 - 1 )
        printlog "Get the text for the last entry "
        value3 = DateTimeLanguage.GetSelText           
        
        if value2 = value3 then 
            errorlog " More options for Date should exist!"
            printlog "Check  if the first and last are the same. "
            printlog "Choose the original state "
            DateTimeLanguage.select value2                 
            value3 = DateTimeLanguage.GetSelText
            else
                printlog " DateTimeLanguage box works."
            endif
            
        printlog "Checking if language settings get saved"
        Apply.Click
        ViewHeaderAndFooter
        if DateTimeLanguage.GetSelText = value3 then
            printlog " DateTimeLanguage settings get saved."
        else
            warnlog " DateTimeLanguage do not get saved!"
        endif
        
    else
        warnlog " DateTime doesnt exist or is not visible!"
    endif

    printlog "Checking Footer checkbox "
    if Footer.exists = TRUE AND Footer.isVisible = TRUE then
        Footer.Check
        sleep(1)
        
        if Footer.IsChecked then
            printlog " Footer-checkbox works."
        else
            errorlog " Something wrong with the Footer-checkbox!"
        endif
        
    else
        warnlog " Footer checkbox doesnt exist or is not visible!"
    endif
    
    printlog "Checking if Footer setting gest saved"
    Footer.Uncheck
    Apply.Click
    ViewHeaderAndFooter
    if Footer.IsChecked then
        warnlog " Footer settings do not get saved!"
    else
        printlog " Footer settings saved."
    endif
    Footer.Check

    printlog "Checking if text can be inserted in FooterFixedText "
    if FooterFixedText.exists = TRUE AND FooterFixedText.isVisible = TRUE then
        FooterFixedText.SetText "lalala"
        
        if FooterFixedText.GetText <> "lalala" then 
            errorlog " Could'nt write a text to the FooterFixedText-field!"
        else
            printlog " FooterFixedText works."
        endif
        
    else
        warnlog " Footer doesnt exist or is not visible!"
    endif
    
    printlog "Checking if Footer text gets saved."
    Apply.Click
    ViewHeaderAndFooter
    
    if FooterFixedText.GetText <>"lalala" then
        errorlog " Text does not get saved!"
    else
        printlog " FooterFixedText works."

    endif
    FooterFixedText.SetText ""

   printlog "Checking Slidenumber checkbox"
    if Donotshowonfirstslide.exists = TRUE AND Donotshowonfirstslide.isVisible = TRUE then
        Slidenumber.Check
        sleep(1)
        
        if Slidenumber.IsChecked then
            printlog " Slidenumber-checkbox works"
        else
            warnlog " Something wrong with the slidenumber-checkbox!"
        endif
    else
        warnlog " Do not show on first slide doesnt exist or is not visible!"
    endif
    
    printlog "Checking if Slidenumber gets saved."
    Apply.Click
    ViewHeaderAndFooter
    
    if Slidenumber.IsChecked then
        printlog " Slidenumber setting is remembered."
    else
        warnlog " Slidenumber setting is not remembered!"
    endif

    printlog "Checking Do not show on first slide checkbox"
    if Donotshowonfirstslide.exists = TRUE AND Donotshowonfirstslide.isVisible = TRUE then
        Donotshowonfirstslide.UnCheck
        sleep(1)
        Donotshowonfirstslide.Check
        if Donotshowonfirstslide.IsChecked then
            printlog " Do not show on first slide-checkbox works"
        else
            errorlog " Something wrong with the Do not show on first slide-checkbox!"
        endif
    else
        warnlog " Do not show on first slide doesnt exist or is not visible!"
    endif

    printlog "Checking if Do not show on first slide is saved."
    Apply.Click
    ViewHeaderAndFooter
    
    if Donotshowonfirstslide.IsChecked then
        printlog " Do not show on first slide setting is saved."
    else
        warnlog " Do not show on first slide setting is not saved!"
    endif
        
    printlog "Closing dialog with cancel"
    sleep 1
    TabHeaderFooterSlide.Cancel
    sleep 1
    kontext "drawimpress"
    sleep 1

    printlog "Close document"
    Call hCloseDocument

endcase 'tiHeaderFooterSlide

'----------------------------------------------------------------------------

testcase tiHeaderFooterNotesHandout

    dim numbervalue1 as integer
    dim value2 as String
    dim value3 as String

    printlog "This part checks the second Tabpage of the Header and Footer Dialog for notes and handout"

    printlog "Open New document "
    Call hNewDocument		
    kontext "drawimpress"
    try
        printlog "View - Header And Footer "
        ViewHeaderAndFooter          
        sleep(1)
    catch
        printlog "Warnlog for not opening dialog"
        warnlog " Could'nt open ViewHeaderAndFooter!" 
    endcatch 'goto endcase
    kontext " TabHeaderFooterSlide"
    
    printlog " Testing TabHeaderFooterNotesHandout."
    sleep 2
    kontext
    active.setpage TabHeaderFooterNotesHandout
    sleep 2
    kontext "TabHeaderFooterNotesHandout"
    
    printlog "Checking Header checkbox "
    if Header.exists = TRUE AND Header.isVisible = TRUE then
        Header.UnCheck
        sleep(1)
        Header.Check
        sleep(1)
        
        if Header.isChecked then
            printlog " Header-checkbox works."
        else
            warnlog " Header-checkbox doesn't work!"
        endif
        
    else
        warnlog " Header doesnt exist or is not visible!"
    endif
    
    printlog "Checking if Header setting is saved."
    Header.Uncheck
    ApplyToAll.Click
    ViewHeaderAndFooter
    kontext
    active.setpage TabHeaderFooterNotesHandout
    kontext "TabHeaderFooterNotesHandout"
    
    if Header.IsChecked then
        warnlog " Header should not be checked!"
    else
        printlog " Header not checked."
        Header.Check
    endif
    
    printlog "Checking if header text can be inserted"
    if HeaderFixedText.exists = TRUE AND HeaderFixedText.isVisible = TRUE then
        HeaderFixedText.settext "lalala"
        sleep(1)
      
        if HeaderFixedText.gettext <> "lalala" then 
            warnlog " HeaderFixedText is wrong!"
        else
            printlog " HeaderFixedText works."
        endif
    else
        warnlog " HeaderFixedText doesnt exist or is not visible!"
    endif
    
    printlog "Checking if text is saved."
    ApplyToAll.Click
    ViewHeaderAndFooter
    kontext
    active.setpage TabHeaderFooterNotesHandout
    kontext "TabHeaderFooterNotesHandout"
    if HeaderFixedText.gettext <> "lalala" then
        warnlog " HeaderFixedText does not get saved!"
    else
        printlog " HeaderFixedText gets saved."
    endif
    HeaderFixedText.SetText ""

    printlog "Checking Date And Time checkbox"
    if DateTime.exists = TRUE AND DateTime.isVisible = TRUE then
        DateTime.UnCheck
        sleep(1)
        DateTime.Check
        sleep(1)
      
        if DateTime.isChecked then
            printlog " DateAndTime-checkbox works."
        else
            warnlog " DateAndTime-checkbox doesnt work!"
        endif
      
    else
        warnlog " DateTime doesnt exist or is not visible!"
    endif
    
    printlog "Checking if Date and Time setting is saved."
    DateTime.Uncheck
    ApplyToAll.Click
    ViewHeaderAndFooter
    kontext
    active.setpage TabHeaderFooterNotesHandout
    kontext "TabHeaderFooterNotesHandout"
    
    if DateTime.IsChecked then
        warnlog " DateTime is not saved!"
    else
        printlog " DateTime gets saved."
    endif
    DateTime.Check

    printlog "Check DateTimeFixed can be checked/unchecked "
    if DateTimeFixed.exists = TRUE AND DateTimeFixed.isVisible = TRUE then
    
        if DateTimeFixed.isChecked then 
            errorlog "DateTimeFixed shouldnt be checked at this point"
            DateTimeVariable.check
            sleep(1)
        else 
            printlog " DateTimeFixed not checked."
        endif
      
        if DateTimeFixed.isChecked then
            warnlog " Datetimefixed-checkbox doesnt work!"
        else
             DateTimeFixed.check
            if DateTimeFixed.isChecked then 
                printlog " Datetimefixed checkbox works."
            else
                warnlog " DateTimeFixed is not checked!"
            endif
        endif
    else
        warnlog " DateTimeFixed doesnt exist or is not visible!"
    endif

    printlog "Can text be inserted in the fixed-field? "
    if DateTimeFixedText.exists = TRUE AND DateTimeFixedText.isVisible = TRUE then
        DateTimeFixedText.settext "lalala"
      
        if DateTimeFixedText.gettext <> "lalala" then 
            errorlog " Error with writing in the DateTimeFixedText-field!"
        else
            printlog " DateTimeFixedText works."
        endif
        
        printlog "Checking if DateTimeFixedText gets saved."
        ApplyToAll.Click
        ViewHeaderAndFooter
        kontext
        active.setpage TabHeaderFooterNotesHandout
        kontext "TabHeaderFooterNotesHandout"
        
        if DateTimeFixedText.GetText <> "lalala" then
            warnlog " DateTimeFixedText does not get saved!"
        else
            printlog " DateTimeFixedText gets saved."
        endif
    
    else
        warnlog "DateTimeFixedText doesnt exist or is not visible"
    endif

    printlog "Can it be changed to Variable and checked? "
    if DateTimeVariable.exists = TRUE AND DateTimeVariable.isVisible = TRUE then
        DateTimeVariable.Check
        sleep(1)
      
        if DateTimeVariable.IsChecked AND DateTimeFixed.IsChecked = FALSE then
            printlog " DateTimeVariable-checkbox works."
        else
            warnlog " Something wrong with the DateTimeVariable-checkbox or DateTimeFormat is checked!"
        endif
      
        printlog "Checking if DateTimeVariable setting gets saved."
        ApplyToAll.Click
        ViewHeaderAndFooter
        kontext
        active.setpage TabHeaderFooterNotesHandout
        kontext "TabHeaderFooterNotesHandout"
        
        if DateTimeVariable.IsChecked then
            printlog " Variable setting gets saved."
        else
            warnlog " Variable setting does not get saved!"
        endif
        
    else
        warnlog " DateTimeVariable doesnt exist or is not visible"
    endif
   
    printlog "Checking format dropdown box"
    printlog "Get the number of choices "
    numbervalue1 = DateTimeFormat.getitemcount           
    printlog " DateTimeFormat equals: " + numbervalue1
    printlog "Get the text for the choosen entry "
    value2 = DateTimeFormat.getseltext
    printlog " Tries to get ", numbervalue1

    if DateTimeFormat.exists = TRUE AND DateTimeFormat.isVisible = TRUE then
        printlog "Select the last entry "
        DateTimeFormat.select ( numbervalue1 - 1 )
        printlog "Get the text for the last entry "
        value3 = DateTimeFormat.getseltext             
        printlog " DateTimeFormat is: " + value3
        
        if value2 = value3 then
             printlog "Check if the first and last are the same. "
            errorlog " More options for Date should exist."
            printlog "Choose the original state "
            DateTimeFormat.select value2                   
        else
            printlog " DateTimeFormat works."
        endif
        
        printlog "Checking if DateTimeFormat setting gets saved."
        ApplyToAll.Click
        ViewHeaderAndFooter
        kontext
        active.setpage TabHeaderFooterNotesHandout
        kontext "TabHeaderFooterNotesHandout"
        
        if DateTimeFormat.GetSelText = value3 then
            printlog " DateTimeFormat setting gets saved."
        else
            warnlog " DateTimeFormat setting does not get saved!"
        endif
        
    else
        warnlog "DateTimeFormat does not exist or is not visible."
    endif
   
    if DateTimeLanguage.exists = TRUE AND DateTimeLanguage.isVisible = TRUE then
        printlog "Get the number of choices "
        numbervalue1 = DateTimeLanguage.getitemcount
        printlog "Choose the first entry "
        DateTimeFormat.select 1
        printlog "Get the text for the choosen entry "
        value2 = DateTimeLanguage.getseltext
        printlog "Select the last entry "
        DateTimeLanguage.select ( numbervalue1 - 1 )
        printlog "Get the text for the last entry"
        value3 = DateTimeLanguage.getseltext           
      
        if value2 = value3 then
            printlog "Check if the first and last are the same. "
            errorlog " More options for Date should exist!"   
        else
            printlog " Footer works."
        endif
        
        printlog "Checking if DateTimeLanguage setting gets saved."
        ApplyToAll.Click
        ViewHeaderAndFooter
        kontext
        active.setpage TabHeaderFooterNotesHandout
        kontext "TabHeaderFooterNotesHandout"
        
        if DateTimeLanguage.GetSelText = value3 then
            printlog " DateTimeLanguage setting gets saved."
        else
            warnlog " DateTimeLanguage setting does not get saved!"
        endif
    else
        warnlog " DateTimeLanguage does not exist!"
    endif

        printlog "Checking Footer checkbox. "
        if Footer.exists = TRUE AND Footer.isVisible = TRUE then
            Footer.UnCheck
            sleep(1)
        else
            warnlog " Footer does not exists!"
        endif
      
        if Footer.IsChecked = TRUE then
            warnlog "Footer should NOT be checked at this point"
        else
            Footer.Check
        endif
        
        printlog "Check if Footer setting is saved."
        Footer.Uncheck
        ApplyToAll.Click
        ViewHeaderAndFooter
        kontext
        active.setpage TabHeaderFooterNotesHandout
        kontext "TabHeaderFooterNotesHandout"
        
        if Footer.IsChecked then
            warnlog " Footer should not be checked!"
        else
            printlog " Footer is not checked."
        endif
        Footer.Check
   
    printlog "Checking if Footer text can be inserted"
    if FooterFixedText.exists = TRUE AND FooterFixedText.isVisible = TRUE then
        FooterFixedText.SetText "lalala"
        sleep(1)
      
        if FooterFixedText.GetText <> "lalala" then 
            errorlog " Could'nt write a text to the FooterFixedText-field!"
        else
            printlog " FooterFixedText works."
        endif
        
    else
        warnlog " FooterFixedText does not exists!"
    endif
    
    printlog "Checking if Footer text is saved."
    ApplyToAll.Click
    ViewHeaderAndFooter
    kontext
    active.setpage TabHeaderFooterNotesHandout
    kontext "TabHeaderFooterNotesHandout"
    
    if FooterFixedText.GetText <> "lalala" then
        warnlog " FooterFixedText does not get saved!"
    else
        printlog " FooterFixedText gets saved."
    endif
    
    printlog "Checking Slidenumber checkbox"
    if Slidenumber.exists = TRUE AND Slidenumber.isVisible = TRUE then
        Slidenumber.UnCheck
        sleep(1)
        
        if Slidenumber.IsChecked = TRUE then
            warnlog " Slidenumber should NOT be checked at this point!"
        else
            Slidenumber.Check
        endif
        
        printlog "Checking if Slidenumber setting is saved."
        ApplyToAll.Click
        ViewHeaderAndFooter
        kontext
        active.setpage TabHeaderFooterNotesHandout
        kontext "TabHeaderFooterNotesHandout"
        
        if Slidenumber.IsChecked then
            printlog " Slidenumber setting get saved."
        else
            warnlog " Slidenumber setting does not get saved!"
        endif
        
    else
        warnlog " Slidenumber does'nt exist or is not visible!"
    endif

    printlog "Close dialog with cancel"
    sleep 1
    TabHeaderFooterNotesHandout.Cancel
    sleep 1
    kontext "drawimpress"
    sleep 1

   printlog "Close document "
    Call hCloseDocument

endcase 'tiHeaderFooterNotesHandout

'-------------------------------------------------------------------------------