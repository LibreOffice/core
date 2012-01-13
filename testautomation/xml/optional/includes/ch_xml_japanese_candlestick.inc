'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************

***********************
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

