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