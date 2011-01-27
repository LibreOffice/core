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
'* owner : oliver.craemer@oracle.com
'*
'* short description : testing japanese candle strick feature in chart 
'*
'*************************************************************************************************
'*
' #1 tJapaneseCandlestickChart  'Check content.xml for attribute chart:japanese-candle-stick
'*
'\*************************************************************************************************

testcase tJapaneseCandlestickChart
    'Note: Only variant 2 and 4 using the chart:japanese-candle-stick attribute
    dim iTypeNum as INTEGER
    dim iSaxSeekIndex as INTEGER        
    dim sOutputFile as STRING
    dim sChartContentXML as STRING
    dim sUnpackedStorageDir as STRING
    dim sAttrValue as STRING
    
    for iTypeNum = 1 to 4
        if iTypeNum = 2 or iTypeNum = 4 then
            sOutputFile = gOfficePath & ConvertPath("user\work\japaneseCandlestickType" & iTypeNum & ".ods")
            sUnpackedStorageDir = gOfficePath & ConvertPath("user\work\japaneseCandlestickType") & iTypeNum
            if (dir(sUnpackedStorageDir) <> "") then
                RmDir (sUnpackedStorageDir)
            endif
            sChartContentXML = gOfficePath & ConvertPath("user\work\japaneseCandlestickType" & iTypeNum & "\Object 1\content.xml")                
            printlog "File / New / Spreadsheet"
            Call  hNewDocument                                
            Kontext "DocumentCalc"
            Kontext "RechenleisteCalc"
            if not RechenleisteCalc.isvisible then
                ViewToolbarsFormulaBar
            end if
            sleep(2)            
            printlog "Type in the 'Name Box' of the 'Formula Bar' [CTRL+A]"
            Kontext "RechenleisteCalc"
            Bereich.TypeKeys "<MOD1 A>"            
            printlog "and then 'A1:F10' to select a range"
            Bereich.TypeKeys "A1:F10"
            sleep(1)            
            printlog "Type [RETURN]"
            Bereich.TypeKeys "<RETURN>"
            sleep(2)
            Kontext "DocumentCalc"            
            printlog "In the Calc document write '=rand()[SHIFT MOD1 RETURN]'"
            DocumentCalc.typekeys("=rand()<SHIFT MOD1 RETURN>")                  
            printlog "Insert / Chart"                     
            InsertChartCalc
            sleep(2)
            Kontext "ChartType"            
            printlog "Select Type 8 'Stock Chart'"
            ChooseType.Select(8)            
            printlog "Select Type 2 or 4"
            variant.typeKeys "<HOME>"
            variant.typeKeys "<RIGHT>", (iTypeNum - 1)            
            printlog "Click FINISH button"
            Kontext "ChartWizard"
            ChartWizard.OK
            sleep(2)
            Kontext "DocumentCalc"            
            printlog "Type twice [ESCAPE]"
            DocumentCalc.TypeKeys "<ESCAPE>" , 2
            sleep(2)            
            printlog "Save document in the OpenDocument format (should be default)"            
            printlog " as gOfficePath/user/work/japaneseCandlestickType[2|4].ods"            
            if hFileSaveAsWithFilterKill (sOutputFile , "calc8") = FALSE then
                warnlog "Saving " & sOutputFile & " failed! -> Exiting test!"
                call hCloseDocument
                goto endsub
            else                
                printlog "File / Close"
                call hCloseDocument
                sleep(3)                
                printlog "Unzip or unjar the gOfficePath/user/work/japaneseCandlestickType[2|4].ods"                
                printlog "into the directory gOfficePath/user/work/japaneseCandlestickType[2|4]"               
                UnpackStorage(sOutputFile, sUnpackedStorageDir)
                printlog "Read the test documents DOM."                
                printlog "Use an XML editor like jEdit and open the"                 
                printlog "structure of the XML file gOfficePath/user/work/japaneseCandlestickType[2|4]/Object 1/content.xml"
                SAXReadFile(sChartContentXML)
                sleep(2)                        
                printlog "Set the DOM pointer on element 'office:automatic-styles'"                        
                SAXSeekElement("/")                
                printlog "The test case is passed if..."                 
                printlog "office:document-content"                
                printlog "office:automatic-styles"                
                printlog "style:style"                
                printlog "..in one of the 10 style:style elements there should be a"                                        
                printlog "style:chart-properties"                
                printlog "chart:japanese-candle-stick"                 
                printlog "with the value true"                                         
                SAXSeekElement("office:document-content")        
                SAXSeekElement("office:automatic-styles")                                                
                for iSaxSeekIndex = 1 to 10 
                    SAXSeekElement("style:style" , iSaxSeekIndex)                                                                                                                                
                    if SAXHasElement("style:chart-properties") then                            
                        SAXSeekElement("style:chart-properties")                                
                        if SaxGetAttributeName(1) = "chart:japanese-candle-stick" then
                            printlog "Found attribute 'chart:japanese-candle-stick'"                                                                            
                            sAttrValue = SAXGetAttributeValue("chart:japanese-candle-stick")
                            if lcase(sAttrValue) = "true" then
                                printlog "Expected value of 'chart:japanese-candle-stick' = true was found"
                                exit for
                            endif                    
                        endif
                        SAXSeekElement(0)
                    endif
                    SAXSeekElement(0)                            
                next iSaxSeekIndex
                SAXRelease
            endif                                    
        endif
    next iTypeNum
endcase

