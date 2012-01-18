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
'*
'* short description :
'*
'\******************************************************************************

testcase tToolsOptionsMeasurementUnit (sApplication as string)
    dim i as integer
    dim iCount as integer
    dim iCount2 as integer
    dim sUnitOptions as string
    dim sUnitDialog as string

    hNewDocument
    sleep 1
    ToolsOptions
    hToolsOptions (sApplication,"General")
    iCount2 = Masseinheit.GetSelIndex
    printlog "current measurement unit is: " & iCount2
    iCount = Masseinheit.GetItemCount

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    printlog "----------------------"

    printlog "Take Measurement Units from Graphics "
    printlog "(1) reference from options; (2) BMP export dialog; (3) Format->3D Effects->Geometry"
    ' In graphics are some more; beside the general one from writer we see large ones: m, km, Miles, foot
    for i=1 to iCount
        ToolsOptions
        hToolsOptions (sApplication,"General")
        Masseinheit.select i
        sUnitOptions = GetMeasUnit(Tabulatorabstand.getText)  ' (1)
        printlog "(" + i + "/" + iCount + "): '" + Masseinheit.getSelText + "' - " + sUnitOptions + "      (1)"
        Kontext "ExtrasOptionenDlg"
        ExtrasOptionenDlg.OK
        Format3D_Effects
        Kontext "Drei_D_Effekte"
        Geometrie.Click
        sleep 1
        if (getMeasUnit(Tiefe.getText) <> sUnitOptions) then
             printlog "#109705# Measurement Unit is different for Options:'" + sUnitOptions + "' and Depth:'" + getMeasUnit(Tiefe.getText) + "' (1) <> (3)"
        else
             printlog "Measurement Unit is same for Options:'" + sUnitOptions + "' and Depth:'" + getMeasUnit(Tiefe.getText) + "'      (3)"
        endif
        Drei_D_Effekte.close
        FormatParagraph
        Kontext
        Messagebox.SetPage TabEinzuegeUndAbstaende
        kontext "TabEinzuegeUndAbstaende"
        sleep 1
        if (getMeasUnit(Vonlinks.getText) <> sUnitOptions) then
             warnlog "#109705# Measurement Unit is different for Options:'" + sUnitOptions + "' and Vonlinks:'" + getMeasUnit(Vonlinks.getText) + "'  Paragraph"
        else
             printlog "Measurement Unit is same for Options:'" + sUnitOptions + "' and Vonlinks:'" + getMeasUnit(Vonlinks.getText) + "' Paragraph"
        endif
        TabEinzuegeUndAbstaende.cancel
    next i

    printlog "Setting back measurement to " & iCount2
    ToolsOptions
    hToolsOptions (sApplication,"General")
    Masseinheit.Select(iCount2)
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    hCloseDocument
endcase 'tToolsOptionsMeasurementUnit 
