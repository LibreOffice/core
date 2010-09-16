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
'*******************************************************************
'*
' #1 MakeTextSquare             :Inserts a text frame with content
' #1 MakeRectangle              :Inserts a Rectangle
' #1 MakeCircle                 :Inserts a circle if not Writer or Calc
' #1 Make3dObject               :Inserts a 3DObject if not Writer or Calc
' #1 MakeCurveObject            :Inserts Curve Object if not Writer or Calc
' #1 MakeLineObject             :Inserts line object if not writer or calc
' #1 MakeConnectorObject        :Inserts Connector object if not wirter or calc
' #1 MakeFormattedTextLine      :Insert a formatted text line
' #1 Formfunctions              :Inserts a push button
' #1 Objectplugin               :Inserting a Plugin
'*
'\*******************************************************************

sub MakeTextSquare

    printlog "---------   Testing: TextSquare   ---------"
    Call hTextRahmenErstellen ("This is a text-frame",15,20,40,30)
    gMouseClick 10,10
    SetKontextApplication

end sub
'-------------------------------------------------------------------------------
sub MakeRectangle

    printlog "we run rectangle"
    Call hRechteckErstellen (15,15,30,30)

end sub

'-------------------------------------------------------------------------------
testcase MakeCircle

    printlog "Create Circle"
    if gApplication = "WRITER" then
        Warnlog "Cant make circle in this application"
        goto endsub
    end if
    if gApplication = "CALC" then
        Warnlog "Cant make circle in this application"
        goto endsub
    end if
    printlog "we run Circle"
    Kontext "Toolbar"
    Ellipsen.Click
    sleep 1
    gMouseDown (15,15)
    gMouseMove (15,15,30,30)
    gMouseUp (30,30)
    sleep 1

endcase

'-------------------------------------------------------------------------------
testcase Make3dObject

    printlog "Create 3dObject"
    if (Ucase(gApplication) = "CALC") then
        warnlog "Cant make circle in this application"
        goto endsub
    end if
    if (Ucase(gApplication) = "WRITER") then
        warnlog "Cant make circle in this application"
        goto endsub
    end if

    printlog "we run 3d-Object"
    kontext "Toolbar"
    if Drei_D_Objekte.exists then
        Drei_D_Objekte.tearoff
        Kontext "ThreeDObjectsbar"
        sleep 1
        Wuerfel.click
        gMouseDown (15,15)
        gMouseMove (15,15,35,35)
        gMouseUp (35,35)
        Kontext "ThreeDObjectsbar"
        ThreeDObjectsbar.close
    else
        if (gApplication = "DRAW") then
            sleep 1
            hMenuselectNr (1)
            sleep 1
            hMenuItemCheck (11)
            sleep 2
            Drei_D_Objekte.tearoff
            Kontext "ThreeDObjectsbar"
            sleep 1
            Wuerfel.click
            sleep 1
            gMouseDown (15,15)
            gMouseMove (15,15,35,35)
            gMouseUp (35,35)
            Kontext "ThreeDObjectsbar"
            ThreeDObjectsbar.close
            sleep 2
            kontext "Toolbar"
            sleep 1
            hMenuselectNr (1)
            sleep 1
            hMenuItemUnCheck (11)
            sleep 1
        else
            sleep 1
            hMenuselectNr (1)
            sleep 1
            hMenuItemCheck (11)
            sleep 2
            Drei_D_Objekte.tearoff
            Kontext "ThreeDObjectsbar"
            sleep 1
            Wuerfel.click
            sleep 1
            gMouseDown (15,15)
            gMouseMove (15,15,35,35)
            gMouseUp (35,35)
            Kontext "ThreeDObjectsbar"
            ThreeDObjectsbar.close
            sleep 2
            kontext "Toolbar"
            sleep 1
            hMenuselectNr (1)
            sleep 1
            hMenuItemUnCheck (11)
            sleep 1
        endif
    end if

endcase

'-------------------------------------------------------------------------------
testcase MakeCurveObject

    if (Ucase(gApplication) = "WRITER") then
        warnlog "Cant make circle in this application"
        goto endsub
    end if
    if (Ucase(gApplication) = "CALC") then
        warnlog "Cant make circle in this application"
        goto endsub
    end if

    printlog "we run Curve-object"
    kontext "Toolbar"
    Toolbar.OpenContextMenu
    sleep 1
    hMenuselectNr (1)
    sleep 1
    hMenuItemCheck (7)
    sleep 1
    Kurven.Click
    sleep 1
    gMouseDown (10,25)
    gMouseMove (10,25,30,35)
    gMouseUp (30,35)
    gMouseClick 90,90
    sleep 1
    kontext "Toolbar"
    Toolbar.OpenContextMenu
    sleep 1
    hMenuselectNr (1)
    sleep 1
    hMenuItemUnCheck (7)
    sleep 1

endcase

'-------------------------------------------------------------------------------
testcase MakeLineObject

    if gApplication = "WRITER" then
        warnlog "Currently no support for line-object in this application"
        goto endsub
    end if
    if gApplication = "CALC" then
        warnlog "Currently no support for line-object in this application"
        goto endsub
    end if
    printlog "we run Line-object"
    sleep 1
    Kontext "Toolbar"
    Linien.TearOff
    Kontext "Arrowshapes"
    sleep 1
    Leftarrow.Click
    sleep 1
    SetKontextApplication
    gMouseDown (10,15)
    gMouseMove 10,15,20,27
    gMouseUp (20,27)
    sleep 1
    Kontext "Arrowshapes"
    Arrowshapes.Close

endcase

'-------------------------------------------------------------------------------
testcase MakeConnectorObject

    if gApplication = "WRITER" then
        warnlog "Cant make connector in this application"
        goto endsub
    end if
    if gApplication = "CALC" then
        warnlog "Cant make connector in this application"
        goto endsub
    end if
    printlog "we run Connector-object"
    if gApplication = "DRAW" then
        sleep 1
        kontext "Toolbar"
        sleep 1
        Verbinder.TearOff    ' Insert connector
        kontext "Connectorsbar"
        sleep 1
        Verbinder.Click
        Connectorsbar.Close
        sleep 1
        SetKontextApplication
        gMouseDown (10,17)
        gMouseMove (10,17,30,37)
        gMouseUp (30,37)
    end if
    if gApplication = "IMPRESS" then
        kontext "Toolbar"
        sleep 1
        Verbinder.TearOff   ' Insert connector
        kontext "Connectorsbar"
        sleep 1
        Verbinder.Click
        Connectorsbar.Close
        sleep 1
        SetKontextApplication
        gMouseDown (10,17)
        gMouseMove (10,17,30,37)
        gMouseUp (30,37)
    end if

endcase

'-------------------------------------------------------------------------------
sub MakeFormattedTextLine

    dim Zufall, iWaitIndex as integer
    Kontext "TextObjectbar"
    if TextObjectbar.Exists then
        printlog "   TextObjectbar.Exists = " + TextObjectbar.Exists
    else
        ViewToolbarsTextFormatting
        sleep (2)
    end if

    hTypeKeys "This is a text which we will format in some different ways."
    hTypeKeys "<SHIFT HOME>" 'Marked the inserted text. (= now ready for formatting)
    Kontext "TextObjectbar"
    Printlog "   - choose random font"
    randomize
    Zufall=((20*Rnd)+1)
    'Schriftart.GetItemcount
    Text1 = Schriftart.GetSelText
    Printlog "   -Font: '" + Schriftart.GetSelText + "' selected."

    Kontext "TextObjectbar"
    sleep (2)
    Printlog "   - Change size of Font"
    Schriftgroesse.Select (Zufall)
    Printlog "   - Fontsize set to '" + Schriftgroesse.GetSelText + "'."
    Text2 = Schriftgroesse.GetSelText
    Kontext "TextObjectbar"
    Fett.Click
    sleep (2)
    Printlog "   - Font attribute set to bold."
    Text4 = Fett.GetState(2)

    Kontext "TextObjectbar"
    Kursiv.Click
    sleep (2)
    Printlog "   - Font attribute set to cursive."
    Text5 = Kursiv.GetState(2)

    Kontext "TextObjectbar"
    Unterstrichen.Click
    sleep (2)
    Printlog "   - Font attribute set to underlined."
    Text6 = Unterstrichen.GetState(2)

    Kontext "TextObjectbar"
    Blocksatz.Click
    sleep (2)
    Printlog "   - Text set to Justified."
    Text7 = Blocksatz.GetState(2)

    SetKontextApplication

end sub 'MakeFormattedTextLine

'--------------------------------------------------------------------------------------------------------
sub Formfunctions
    gMouseMove2 (50, 20)
    if (Ucase(gApplication) = "IMPRESS") then
        kontext "Toolbar"
        sleep 1
        Toolbar.OpenContextMenu ' enable forms button in menuebar
        sleep 1
        hMenuselectNr (6)
        sleep 1
        hMenuItemCheck (14)
        sleep 1
        WL_WRITER_Formular.TearOff  ' use fromsbutton
    else
        kontext "Toolbar"
        sleep 1
        Toolbar.OpenContextMenu ' enable forms button in menuebar
        sleep 1
        hMenuselectNr (5)
        sleep 1
        hMenuItemCheck (14)
        sleep 1
        WL_WRITER_Formular.TearOff  ' use fromsbutton
    endif
    printlog "  enabled 'forms' button and clicked it"
    sleep 1
    kontext "FormControls"
    if (Pushbutton.IsEnabled = FALSE) then
        DesigneModus.click            ' use forms menue, go into design mode
    endif
    Pushbutton.Click              ' use a pushbutton
    Sleep 1
    gMouseDown (50,20)
    gMouseMove (50, 20,70, 40)' create a pushbutton
    gMouseUp (70,40)

    FormatControl
    Kontext "ControlPropertiesDialog"
    sleep 1
    ControlPropertiesDialog.Close
    formatform
    Kontext "ControlPropertiesDialog"
    sleep 1
    ControlPropertiesDialog.Close

    sleep 5 ' needed, otherwise the context menue will get closed...

    if (Ucase(gApplication) = "IMPRESS") then
        kontext "Toolbar"
        Toolbar.OpenContextMenu ' disable forms button in menuebar
        sleep 1
        hMenuselectNr (9)
    else
        kontext "Toolbar"
        Toolbar.OpenContextMenu ' disable forms button in menuebar
        sleep 1
        hMenuselectNr (8)
    endif
end sub

'--------------------------------------------------------------------------------------------------------
sub Objectplugin

    printlog "Insert Object-plugin"
    'Call hNewDocument

    InsertObjectPlugin
    Kontext "PlugInEinfuegen"
    'DialogTest ( PlugInEinfuegen)
    Durchsuchen.click
    Kontext "OeffnenDlg"
    'Call DialogTest ( OeffnenDlg )
    if OeffnenDlg.exists (5) then
        OeffnenDlg.Cancel
    else
        warnlog "Open file dialog didn't come up"
    endif
    sleep 5
    Kontext "PlugInEinfuegen"
    if PlugInEinfuegen.exists then
        DateiUrl.SetText (ConvertPath ( gTesttoolpath + "graphics\required\input\sample.mov" ))
        printlog "Type something into the option field"
        'Optionen               so3:MultiLineEdit:MD_INSERT_OBJECT_PLUGIN:ED_PLUGINS_OPTIONS
        Optionen.SetText "Fiddler's Green"
        sleep 1
        Optionen.TypeKeys "<HOME>"
        Optionen.TypeKeys "<SHIFT><END>"
        Optionen.TypeKeys "<delete>"
        PlugInEinfuegen.Ok
    else
        warnlog "Insert plugin isn't visible"
    endif
    sleep (5)
    kontext "Messagebox"
    if Messagebox.exists (5) then
        warnlog "Messagebox: " + Messagebox.gettext
        Messagebox.ok
    endif
end sub

'--------------------------------------------------------------------------------------------------------
