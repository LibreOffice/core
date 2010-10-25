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
'* owner : wolfram.garten@oracle.com
'*
'* short description : update and resouce test for numberformatter
'*
'\***********************************************************************

'///Setting Variables..

    dim sSeperatorCurr  as string        ' which kind of Separator is used
    dim iCategoryCount  as integer       ' number of Category entries
    dim sCategoryText   as string        ' names of Category entries
    dim sFormatText     as string        ' variable for format string text
    dim sFormatCode     as string        ' variable for format code text line
    dim iLanguageCount  as integer       ' number of language entries
    dim sLanguageText   as string        ' name sof language entries
    dim iCount          as integer       ' count variable for/next
    dim sCell           as string        ' Cell-Content
 
'-------------------------------------------------------------------------------

'///Setting Preconditions for tests
sub sNumberformatterPreconditions

    printlog "Setting up some preconditions for starting the testcases now.."

    select case gApplication
            case "WRITER"   :
                                '/// Case Writer:
                                Kontext "DocumentWriter"
                                printlog "Application is " & gApplication
                                '/// Press CTRL+F12 to insert via shortcut a table
                                DocumentWriter.TypeKeys "<MOD1 F12>", true
                                Kontext "TabelleEinfuegenWriter"
                                '/// Leave the insert table dialog with OK 
                                TabelleEinfuegenWriter.OK
                                printlog "Table inserted"
                                Kontext "DocumentWriter"
                                '/// Type 0 in the first cell
                                DocumentWriter.TypeKeys "0", true
                                Call sNumberformatterOpenDialog 'opening Dialog
    
            case "CALC"   :
                                '/// Case Calc:
                                '/// Calc with Chart in inplace mode:
                                Kontext "DokumentChart"
                                    if DokumentChart.Exists(1) then
                                    printlog "Application is " & gApplication & " with inserted Chart."
                                    DokumentChart.TypeKeys "<TAB>",2,true 'selecting data serie
                                    '/// Opening Number Formatter Dialog using Menu
                                    DokumentChart.UseMenu
                                    hMenuSelectNr(5)
                                    hMenuSelectNr(1)
                                    Kontext
                                    '/// Selecting correct Tab page
                                    active.SetPage TabDataSeriesLabels
                                    Kontext "TabDataSeriesLabels"
                                    AsPercentage.Check
                                    PercentageFormat.Click
                                    Kontext "TabChartAxisNumbers"
                                    Sourceformat.uncheck
                                    '/// fLocaleString function gets English from t_locale_strings1.inc
                                    Language.Select (fLocaleString ("LocaleNumFormLanEng"))

                                else
                                '/// Calc without Chart:
                                Kontext "DocumentCalc" 'just Calc without Chart
                                printlog "Application is " & gApplication
                                '/// Inserting 0 in first cell
                                DocumentCalc.TypeKeys "0", true
                                DocumentCalc.TypeKeys "<RETURN>", true
                                DocumentCalc.TypeKeys "<UP>", true
                                Call sNumberformatterOpenDialog 'opening Dialog
                            endif
   
            case else       : warnlog "This testcase can only be tested in Writer,Calc or Chart!"
        end select
end sub

'-------------------------------------------------------------------------------

sub sNumberformatterOpenDialog

select case gApplication
            case "WRITER"   :   
                                '/// Case Writer:
                                Kontext "DocumentWriter"
                                '///Open Context Menu 
                                DocumentWriter.TypeKeys "<SHIFT F10>", true
                                sleep (1)
                                '///Open Number Formatter
                                DocumentWriter.TypeKeys "<DOWN>",11, true
                                sleep (1)
                                DocumentWriter.TypeKeys "<RETURN>", true
                                sleep (1)
                                printlog "Opening number formatter dialog."
                                Kontext "TabChartAxisNumbers"
                                Language.Select (fLocaleString ("LocaleNumFormLanEng"))
                            
            case "CALC"     :   
                                '/// Calc with Chart in inplace mode
                                Kontext "DokumentChart"
                                if DokumentChart.Exists(1) then
                                    printlog "Application is " & gApplication & " with inserted Chart."
                                    '/// Opening Numberformatter using menu
                                    DocumentCalc.TypeKeys "<TAB>",2,true 'selecting data serie
                                    DocumentCalc.UseMenu
                                    hMenuSelectNr(5)
                                    hMenuSelectNr(1)
                                    Kontext "TabDataSeriesLabels"
                                    AsPercentage.Check
                                    PercentageFormat.Click
                                    Kontext "TabChartAxisNumbers"
                                    Sourceformat.uncheck
                                    Language.Select (fLocaleString ("LocaleNumFormLanEng"))

                                else
                                    '/// Calc
                                    Kontext "DocumentCalc"
                                    '///Open Context Menu
                                    DocumentCalc.TypeKeys "<SHIFT F10>", true
                                    '///Open Number Format
                                    DocumentCalc.TypeKeys "<DOWN>",3, true
                                    DocumentCalc.TypeKeys "<RETURN>", true
                                    Kontext
                                    Active.SetPage TabChartAxisNumbers
                                    printlog "Opening number formatter dialog."
                                    Kontext "TabChartAxisNumbers"
                                    '/// fLocaleString function gets English from t_locale_strings1.inc
                                    Language.Select (fLocaleString ("LocaleNumFormLanEng"))
                                endif
                            
            case else       :   warnlog "This testcase can only be tested in Writer and Calc!"
            end select
end sub

'------------------------------------------------------------------------------

testcase tNumberformatterPre
    '/// Setting some preconditions for the testcases:
    '/// Setting Measuring to cm by calling fSetMeasurementToCM
    Call fSetMeasurementToCM
    
    '///Getting the decimal separator with GetDecimalSeperator
    printlog "looking for used Separator..."
    sSeperatorCurr = GetDecimalSeperator
endcase

'------------------------------------------------------------------------------

testcase tNumberformatter

    
    'This part is only for testing a testrun with Chart and can be deleted when checked in!!!
    'Call hNewDocument
    'DocumentCalc.UseMenu
    'hMenuSelectNr(4)
    'hMenuSelectNr(17)
    'hMenuSelectNr(1)
    'Kontext "OLEObjektInsert"
    'OLEObjektInsert.OK
    'printlog "Chart inserted"
    'Kontext "DokumentChart"

    'Check this one out if above is checked in:
    '/// Calling new document
    Call hNewDocument
    
    '/// Calling Subroutine for getting preconditions
    Call sNumberformatterPreconditions
    
    Kontext "TabChartAxisNumbers"
    '/// Checking if Category entries in dialog are correct
    iCategoryCount = Category.GetItemCount    
    '/// In the Category section list box 11 entries should be available.
    if iCategoryCount = 11 then
        printlog "All 11 entries present."
    else    
        warnlog "Warning: Only " & iCategoryCount & " listed!"
    endif
        
    'Checking if listbox comes up when switching category to Currency
    '/// Select Currency in the Category section list box (entry 5)
    Category.Select(5)
    '/// In the Format section 
    '/// A drop down list box should be visible
    if FormatCurrency.IsVisible then
        printlog "FormatCurrency available"
    else
        warnlog "Drop Down listbox did not come up"
    endif
    
    'Checking if ZahlenFormat fits to selected Category
    '/// The content should match to the category currency
    FormatString.Select(1) 'selecting entry
    sFormatText = FormatString.GetSelText 'getting text into variable
    'comparing
    '/// Comparing with given value "-$1,234"
    if sFormatText = "-$1,234" then 
        printlog "Text fitting to Format: " & sFormatText
    else
        warnlog "Text not fitting to Format: " & sFormatText
    endif
        
    '/// Select Percent in the Category section list box
    Category.Select(4)
    
    '/// In the Format section 
    '/// No drop down list box should be visible
    'Checking if NO listbox comes up when switching category to Percent
    if FormatCurrency.IsVisible then
        warnlog "FormatCurrency should not be available!"
    else
        printlog "Drop Down listbox did not come up."
    endif
    
    '/// The content should match to the category percent
    'Checking if ZahlenFormat fits to selected Category
    FormatString.Select(1)
    sFormatText = FormatString.GetSelText 
    if sFormatText = "-13%" then
        printlog "Text fitting to Format, " & sFormatText
    else
        warnlog "Text not fitting to Format: " & sFormatText
    endif
    
    '/// Set the category to All 
    Category.Select(1)
    
    '/// Choose a language Lithuanian from the drop down list box for Language
    Language.Select (fLocaleString ("LocaleNumFormLanLith"))
    
    '/// In the Category list box the entry should Number should be selected
    sCategoryText = Category.GetSelIndex
    if sCategoryText = 3 then '3 = number entry in Category
        printlog sCategoryText & " is selected."
    else
        warnlog "Wrong entry selected:" & sCategoryText & "!"
    endif
    
    '/// In the Format list box 
    '/// the entry Bendras should be selected
    sFormatText = FormatString.GetSelText
    if sFormatText = "Bendras" then
        printlog sFormatText & " is selected."
    else
        warnlog "Wrong entry in Format box selected: " & sformatText & "!"
    endif
    
    '/// Select the entry -1,234 (or -1.234 depending on the locale used) 
    FormatString.Select(4)
    
    '/// Does the checkbox Thousands separator get taged ? 
    if ThousandsSeparator.IsChecked then
        printlog "Thousands Separator checked."
    else
        warnlog "Warning! Thousands Separator does not get checked!"
    endif
    
    '/// Changed the format code to #,##0 (or #.##0) ?
    sFormatCode = FormatCode.GetText
    if sFormatCode = "#.##0" then 
        sFormatCode = "#,##0"
        printlog "FormatCode changed to " & sFormatCode & "."
    else
        sFormatCode = "#.##0"
        printlog "FormatCode changed to " & sFormatCode & "."
    endif
    
    '/// Set the Language back to Default (English (USA) and choose the category Currency.
    '/// Gets the string "English" from t_locale_Strings1.inc in the correct language and sets the 
    '/// language box to it
    Language.Select (fLocaleString ("LocaleNumFormLanEng"))
    printlog "Language set to Default."
    Category.Select (5)
    '/// Category set to Currency
    printlog "Category set to Currency."
    
    '/// Is the fourth entry in the format string box selected ? 
    if FormatString.GetSelIndex = 4 then
        printlog "Fourth entry selected, good."
    else
        warnlog "Warning! Not the fourth entry selected!"
    endif
    
    '/// Does the Decimal Places Spinfield change to 2? 
    if DecimalPlaces.GetText = "2" then
        printlog "Decimal places = 2, good."
    else
        warnlog "Warning: Decimal places is not 2!"
    endif
    
    '/// are the check boxes Negative numbers red and Thousands separator tagged ? 
    if NegativNumbersRed.IsChecked then
        printlog "Negative numbers red is checked."
    else
        warnlog "Warning: Negative numbers red is NOT checked!"
    endif
    
     if ThousandsSeparator.IsChecked then
        printlog "ThousandsSeparator is checked."
    else
        warnlog "Warning: ThousandsSeparator is NOT checked!"
    endif
    
    '/// Change the Language field under Format to $ English (USA) if it is not set to this value yet.
    if FormatCurrency.GetSelText = "$ " & (fLocaleString ("LocaleNumFormLanEng")) then
        printlog "Format already set to English."
    else
        printlog "Setting Format to English..."
        '/// Getting correct L10N string from global/t_local_strings1.inc
        FormatCurrency.Select "$ " & (fLocaleString ("LocaleNumFormLanEng"))
    endif
    
    '/// Compares the Format String match this one : [$$-409]#,##0.00;[RED]-[$$-409]#,##0.00 
    sFormatCode = FormatCode.GetText
    if sFormatCode = "[$$-409]#,##0.00;[RED]-[$$-409]#,##0.00" then
        printlog "Format Code is correct: " & sFormatCode
    else
        warnlog "Format Code wrong: " & sFormatCode & "!"
    endif
    
    '/// Change the Language Dropdown list box to show Dutch (Netherlands). 
    printlog "Selecting Dutch..."
    Language.Select (fLocaleString ("LocaleNumFormLanDutch"))
    
    '/// Has the content of the format Listbox changed to show also Euro and Netherlands old fl currency values ?   
    FormatString.Select (15)
    sFormatText = FormatString.GetSelText
    '/// Checking for the string "fl 1.234-"
    if sFormatText = "fl 1.234-" then
        printlog "Format String also shows " & sFormatText
    else
        warnlog "Format String does not show fl 1.234- but " & sFormatText
    endif
    
    '/// Reset the language to Default and change the Category to Date
    Language.Select (fLocaleString ("LocaleNumFormLanEng"))
    printlog "Language set to English."
    Category.Select (6)
    printlog "Category set to Currency."
    
    '/// Checking if the Options Decimal places, leading zeroes, negative numnbers red, 
    '/// thousands seperator and so on are disabled ? 
    if DecimalPlaces.isEnabled = False AND LeadingZeroes.isEnabled = False AND NegativNumbersRed.isEnabled = False AND ThousandsSeparator.isEnabled = False then
        printlog "Options disabled."
    else
        warnlog "Not all options disabled!"
    endif
    
    '/// Checking the Date formats in the format section ?  
    sFormatText = FormatString.GetSelText
    if sFormatText = "12/31/99" then
        printlog "Format is correctly set to Date, " & sFormatText
    else
        warnlog "Format NOT correct: " & sFormatText
    endif
    
    '/// Change the Category to Time. Does the Format List box show time formats ?  
     Category.Select (7)
     printlog "Category Time selected."
     FormatString.Select(1)
     sFormatText = FormatString.GetSelText
      if sFormatText = "13:37" then
        printlog "Format is correctly set to time, " & sFormatText
    else
        warnlog "Format NOT correct: " & sFormatText
    endif
     
    '/// Change the Category to Scientific. Are there two formats shown ? 
     Category.Select (8)
     printlog "Category Scientific selected."
     sFormatText = FormatString.GetSelText
      if FormatString.GetSelText = "-1.23E+003" then
        printlog "Format is correctly set to Scientific, " & sFormatText
    else
        warnlog "Format NOT correct: " & sFormatText
    endif
    
    '/// Change to the Category Fraction. Are there Fraction formats shown ? 
     Category.Select (9)
     printlog "Category Fraction selected."
     sFormatText = FormatString.GetSelText
      if sFormatText = "-1234 1/8" then
        printlog "Format is correctly set to Fraction, " & sFormatText
    else
        warnlog "Format NOT correct: " & sFormatText
    endif
    
    '/// Change to Boolean Value. Is there one entry showing in the format section saying TRUE ? 
     Category.Select (10)
     printlog "Category Boolean Value selected."
      if FormatString.GetSelText = "TRUE" then
        printlog "Format is correctly set to Boolean Value."
    else
        warnlog "Format NOT correct!"
    endif
    
    '/// Change to Category Text. Is there an @-sign shown in the Format section ? 
     Category.Select (11)
     printlog "Category Text selected."
      if FormatString.GetSelText = "@" then
        printlog "Format is correctly set to Text."
    else
        warnlog "Format NOT correct!"
    endif
    
    '/// Closing Number Formatter Dialog
    printlog "Closing number formatter dialog..."
    if TabChartAxisNumbers.exists(1) then
        TabChartAxisNumbers.Cancel
    else 
        printlog "No dialog left, ok."
    endif
    
    '/// Closing Data Series Dialog
    Kontext "TabDataSeriesLabels"
    printlog "Closing data series dialog..."
    if TabDataSeriesLabels.exists(1) then
        TabDataSeriesLabels.Cancel
    else 
        printlog "No dialog left, ok."
    endif
    '/// Closing document
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------

testcase tUserDefinedNumberformatter
    '/// Open an new doc for writer and calc
    '/// for chart this should be disabled
    Call hNewDocument 'opening new doc
    
    '/// Start with the step 2. under preconditions
    Call sNumberformatterPreconditions '/Calling Subroutine for getting preconditions
      
    '/// Change to Category User-defined
    Kontext "TabChartAxisNumbers"
    Language.Select (fLocaleString ("LocaleNumFormLanEng"))
    Category.Select(2)
    printlog "Selecting Category user-defined."
    
    if AddButton.IsEnabled then
        warnlog "Button should be disabled!"
    else
        printlog "Button disabled before typing, good."
    endif
    
    '/// Enter the following Format string into the Format code line : [~jewish]yyyy-mm-dd and click on the green confirmation hook.
    FormatCode.SetText "[~jewish]yyyy-mm-dd"
    printlog "Entering text in Format Code line..."
    
    '/// Did this Icon change from grey to green as you began typing ? 
    if AddButton.IsEnabled then
        AddButton.Click
        printlog "Add Button clicked."
    else
        warnlog "AddButton is not enabled."
    endif
    
    '/// Changed the Category from User-defined to Date ? 
    if Category.GetSelIndex = 6 then
        printlog "Category changed to Date, correct."
    else
        warnlog "Category must change to Date."
    endif
    
    '/// Changed the Format Preview from 05/18/03 to 5663-02-21 ? 
    QAErrorLog "#i84085#-Number formatter: Preview box cannot be accessed by testool"

    '/// Change back to User-defined and select the Format entered previously.
    Category.Select(2)
    printlog "Changed to User-defined."
    FormatString.Select(1)
    'qaerrorlog "#i83551#-Bad focus behaviour in number formatter dialog."
    '/// Click on the little note button next to the format code line. Can you enter a comment now into a newly visible line under the format code line ? 
    EditCommentButton.Click
    if Comment.IsEnabled then
        printlog "Comment Field made writable."
        Comment.SetText "Test-Text"
    else
        warnlog "Comment Field not made writable!"
    endif
    
    '/// Can you close this line by hitting the note button again and is the comment kept ?
    EditCommentButton.Click
     
    '/// Is the third Icon in the Format Code line (the X) colored Red ? 
    if RemoveButton.IsEnabled then
        printlog "Remove button is active."
    else
        warnlog "Remove button not active!"
    endif
    
    '/// Click on the Red X-Icon. Did the Category change to Date and the Format to the default one for the given locale (the third one from top for Englisch (USA)) ? 
    RemoveButton.Click
    '/// Change back to the User-defined category. Is the previously created Number format still available ? 
    Category.Select(2)
     
    '/// Closing Number Formatter Dialog
    printlog "Closing number formatter dialog..."
    if TabChartAxisNumbers.exists(1) then
        TabChartAxisNumbers.Cancel
    else 
        printlog "No dialog left, ok."
    endif
    
    '/// Closing Data Series Dialog
    Kontext "TabDataSeriesLabels"
    printlog "Closing data series dialog..."
    if TabDataSeriesLabels.exists(1) then
        TabDataSeriesLabels.Cancel
    else 
        printlog "No dialog left, ok."
    endif
    '/// Closing Document
    Call hCloseDocument
endcase 

'-------------------------------------------------------------------------------

testcase tButtonsNumberformatter
    printlog "Testing some buttons now..."    
    '/// Opening new document
    Call hNewDocument 'opening new doc
    printlog "Setting some preconditions"
    Call sNumberformatterPreconditions '/Calling Subroutine for getting preconditions
    '/// Number format opened
    Kontext "TabChartAxisNumbers"
    '/// Change to Category Date
    Category.Select (6)
    printlog "Changing to date..."
        
    '/// Click on Cancel. Does the Number formatter dialog get closed ? 
    TabChartAxisNumbers.Cancel 'canceling Dialog
    if TabChartAxisNumbers.Exists(1) then
        warnlog "Dialog should have been closed now."
    else
        printlog "Ok, Dialog closed."
    endif
    
    '/// Open the Number formatter dialog again. Press the ESC key on the Keyboard. Does the dialog close ? 
    Call sNumberformatterOpenDialog 'opening Dialog
    printlog "testing closing with ESC."
    Kontext "TabChartAxisNumbers"
    if TabChartAxisNumbers.Exists(1) then
        printlog "Dialog should be open now."
    else
        warnlog "Warning, Dialog not opened."
    endif
    Kontext "TabChartAxisNumbers"
    TabChartAxisNumbers.TypeKeys "<ESCAPE>", true 'using Escape to close the Dialog
    sleep (1)
    if TabchartAxisNumbers.Exists(1) then
        warnlog "ESC should have closed the dialog..."
    else 
        printlog "Fine, Dialog closed."
    endif
    
    '/// Open the Number formatter dialog again. Change to Category Date. Click on Ok. 
    '/// Does the dialog close and shows the cell you have formatted the date 12/30/99 ?
    Call sNumberformatterOpenDialog 'opening Dialog
    printlog "Assigning date.."
    Kontext "TabChartAxisNumbers"
    Category.Select (6) 'category date
    TabChartAxisNumbers.OK 'closing dialog
    if TabChartAxisNumbers.Exists(1) then 'checking if dialog is closed
        warnlog "OK should have closed the dialog..."
    else 
        printlog "Fine, Dialog closed with ok."
    endif
    
    select case gApplication 'opening Dialog again
        case "WRITER"   :Kontext "DocumentWriter"
                         EditSelectAll
        case "CALC"     :Kontext "DocumentCalc"
        case "CHART"    :printlog "Chart"
        case else       :warnlog "This onyl runs in Writer and Calc!"
                        exit sub
    end select

    EditCopy 'getting content into clipboard
    '/// Copying for comparing the content
    sleep(2)
    sCell = GetClipboardtext 'clipboard into variable
    if sCell = "12/30/99" then
        printlog "Cell is formattted correctly, " & sCell
    else
        warnlog "Formatting is wrong: " & sCell
    endif
    
    select case gApplication 'for selection in Writer, different context menu
        case "WRITER"   :Kontext "DocumentWriter"
                         DocumentWriter.TypeKeys "<ESCAPE>" 'deselecting
        case "CALC"     :Kontext "DocumentCalc"
        case "CHART"    :printlog "Chart"
        case else       :warnlog "This onyl runs in Writer and Calc!"
                        exit sub
    end select
    
    '/// Open the Number formatter dialog again. Change to Category Time. Press the RETURN Key. 
    '/// Does the dialog close and shows the cell you have formatted the time 12:00:00 AM ? 
    Call sNumberformatterOpenDialog 'opening Dialog
    Kontext "TabChartAxisNumbers"
    Category.Select (7) 'selecting Time
    TabChartAxisNumbers.TypeKeys "<RETURN>", true
    if TabChartAxisNumbers.Exists(1) then
        warnlog "Return should have closed the dialog..."
    else 
        printlog "Fine, Dialog closed with ok."
    endif
    
    '/// Select case gApplication for selection in Writer, selecting to get content into clipboard
    select case gApplication 'opening Dialog again
        case "WRITER"   :Kontext "DocumentWriter"
                         EditSelectAll
        case "CALC"     :Kontext "DocumentCalc"
        case "CHART"    :printlog "Chart"
        case else       :warnlog "This onyl runs in Writer and Calc!"
                        exit sub
    end select
    
    EditCopy 
    '/// Getting content into clipboard to compare
    sleep(2)
    sCell = GetClipboardtext 'clipboard into variable
    if sCell = "12:00:00 AM" then
        printlog "Cell is formattted correctly: " & sCell
    else
        warnlog "Formatting is wrong: " & sCell
    endif

    '/// Open the Number formatter dialog again. Change to Category Percent. 
    '/// The Format area shows Percent formats. Click on the Reset Button. Does the Category changes back to Time ? 
    
    'Resetting status for getting correct context menu
    select case gApplication 'for selection in Writer, different context menu
        case "WRITER"   :Kontext "DocumentWriter"
                         DocumentWriter.TypeKeys "<ESCAPE>" 'deselecting
        case "CALC"     :Kontext "DocumentCalc"
        case "CHART"    :printlog "Chart"
        case else       :warnlog "This onyl runs in Writer and Calc!"
                        exit sub
    end select
    
    Call sNumberformatterOpenDialog 'opening Dialog
    Kontext "TabChartAxisNumbers"
    Category.Select (4) 'selecting percent
    printlog "Selecting percent."
    FormatString.Select (1)
    sFormatText = FormatString.GetSelText
    if sFormatText = "-13%" then 'checking if Percent is set in format box
        printlog "Fine, switched to percent, " & sFormatText
    else
        warnlog "Format Box shows wrong value, " & sFormatText
    endif
    if gApplication ="CALC" then 'checking Application for using Reset button
        Zurueck.Click
        if Category.GetSelIndex = 7.then
            printlog "Reset to Time."
        else 
            warnlog "Reset did not succeed!"
        endif
    else
        Category.Select (7) 'setting the Category for other Application
    endif
    
    TabChartAxisNumbers.OK 'closing dialog
    printlog "Dialog closed."
    
    '/// Open the Number formatter dialog again. Change to Category Percent. 
    '/// The Format area shows Percent formats. Press the Keys ALT+R . Does the Category changes back to Time ? 
    Call sNumberformatterOpenDialog 'opening Dialog
    Kontext "TabChartAxisNumbers"
    Category.Select (4) 'selecting percent
    FormatString.Select (1)
    sFormatText = FormatString.GetSelText
    if sFormatText = "-13%" then 'checking entry of Format box
        printlog "Fine, switched to percent, " & sFormatText
    else
        warnlog "Format Box shows wrong value: " & sFormatText
    endif
    
    if gApplication = "CALC" then
        TabChartAxisNumbers.TypeKeys "<MOD2 R>", true 'using ALT+R to reset back from percent to time
        if Category.GetSelIndex = 7 then
            printlog "ALT-R changed back to Time."
        else
            warnlog "ALT-R did not change Category back to time."
        endif
    else
        Category.Select (7) 'setting back to Time manually, no Reset button
    endif
     if TabChartAxisNumbers.Exists(1) then
        TabChartAxisNumbers.Cancel
    else 
        printlog "No dialog left, ok."
    endif

    '///Closing Number Formatter Dialog
    printlog "Closing number formatter dialog..."
    if TabChartAxisNumbers.exists(1) then
        TabChartAxisNumbers.Cancel
    else 
        printlog "No dialog left, ok."
    endif
    
    '///Closing Data Series Dialog
    Kontext "TabDataSeriesLabels"
    printlog "Closing data series dialog..."
    if TabDataSeriesLabels.exists(1) then
        TabDataSeriesLabels.Cancel
    else 
        printlog "No dialog left, ok."
    endif
    Call hCloseDocument
endcase
'   
'\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
'
sub sNumberformatterGetLanguage

'/// When adapting the test for a new language you will need this script to get the names out of the language dropdown box
'/// This routine is to be run isloated from the rest, the found languages from the dropdownbox willbe written into
'/// The result file. Simply copy'n'paste the needed words into global/tools/inc/t_locale_strings1.inc
'/// The test needs to be run once after the office has bben set to the needed language
'/// This sub routine is only for getting a list of the used languages out of the Numberformatter/Language.
'/// The list depends upon the used locale and is written into the result file
'/// This is not part of the normal test run. This is for getting new languages into t_locale_strings1.inc 

    Call hNewDocument 'new doc
    Kontext "DocumentWriter"
    '///Insert a table///
    DocumentWriter.TypeKeys "<MOD1 F12>", true
    Kontext "TabelleEinfuegenWriter"
    TabelleEinfuegenWriter.OK 'insert table
    printlog "Table inserted"
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "0", true 'insert value
    Kontext "DocumentWriter"
    '///Open Context Menu///
    DocumentWriter.TypeKeys "<SHIFT F10>", true 'Context menu
    '///Open Number Format///
    DocumentWriter.TypeKeys "<DOWN>",11, true
    DocumentWriter.TypeKeys "<RETURN>", true
    printlog "Opening number formatter dialog."
    Kontext "TabChartAxisNumbers"
    
    iLanguageCount = Language.GetItemCount 'getting number of entries
    for iCount = 1 to iLanguageCount
        Language.Select (iCount)
        sLanguageText = Language.GetSelText
        printlog sLanguageText 'printing entries
    next iCount
    
    TabChartAxisNumbers.Close 'closing dialog
    Call hCloseDocument
end sub

'\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
