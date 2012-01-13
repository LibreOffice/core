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

************
' #1 tEditImageMap
' #1 tEditImageMapProperties
'\*************************************************************************************

testcase tEditImageMap
    printlog "new document"
   Call  hNewDocument
   printlog "edit image map"
   EditImageMap                  
   Kontext "ImageMapEditor"
   sleep 1
   if ImageMapEditor.Exists (2) then         
   printlog "ImageMap editor exists"
        printlog "test dialog controls"
      DialogTest ( ImageMapEditor )
      try
        ImageMapEditor.Close
        Printlog "ImageMap closed"
      catch
        EditImageMap
        Printlog "ImageMap closed using the edit-menu entry"
      endcatch
   else
      sleep 2
      DialogTest ( ImageMapEditor )
      try
        ImageMapEditor.Close
        Printlog "ImageMap closed"
      catch
        EditImageMap
        Printlog "ImageMap closed using the edit-menu entry"
      endcatch
   end if
   printlog "close document"
   Call  hCloseDocument             
   
endcase 'tEditImageMap

'-------------------------------------------------------------------------------

testcase tEditImageMapProperties

    qaerrorlog "test not yet ready."
    goto endsub
    printlog "new document"
    Call  hNewDocument
    printlog "Insert a graphic and select it"
    InsertGraphicsFromFile               
    sleep (1)
    Kontext "GrafikEinfuegenDlg"
    sleep (2)
    Dateiname.SetText ConvertPath (gTesttoolPath + "graphics\required\input\SaveAsPicture\SaveAsPicture.pcx")
    sleep (2)
    Oeffnen.Click
    sleep (2)
    printlog "Open the image map dialog"
    EditImageMap
    Kontext "ImageMapEditor"
    sleep (1)
    if ImageMapEditor.Exists (2) then
        printlog "ImageMap editor exists"
    else
        warnlog "Something is wrong, the Imagemap didn't show up."
    end if
    printlog "Insert an 'image map' object"
    Ellipse.Click
    Dokument.MouseDown 50,50
    Dokument.MouseMove 50,50,10,10
    Dokument.MouseUp 10,10

    printlog "Click on the properties icon to open up the properties for the Object"
    Eigenschaften.Click
    kontext "ImageMapHyperlink"
    sleep (1)
    if ImageMapHyperlink.Exists (2) then
        printlog "ImageMapHyperlink (properties) editor exists"
    else
        warnlog "Something is wrong, the ImageMapHyperlink (properties) didn't show up."
    end if
    printlog "Write some text in each field"
    URL.SetText "http://www.cnn.com"
    Frame.SetText "http://www.framed.com"
    Objectname.SetText "This is a strange name: Huckillerry Bohaahw"
    Alternativetext.SetText "Alternative, the only way to fly..."
    Description.SetText "Just a Foney-text... or whatever it's called."
    printlog "Close the Properties-Dialogue with Ok"
    ImageMapHyperlink.Ok
    kontext "ImageMapEditor"
    Zuweisen.Click
    sleep (1)
    ImageMapEditor.Close

    if gApplication = "DRAW" then
        kontext "DocumentDraw"
    else
        kontext "DocumentImpress"
    end if

    EditImageMap
    printlog "Check if what we wrote in the Properites-Dialogue still is visible."
    Kontext "ImageMapEditor"
    sleep (1)
    if ImageMapEditor.Exists (2) then   
        printlog "ImageMap editor exists"
    else
        warnlog "Something is wrong, the ImageMap Editor didn't show up."
    end if

    Dokument.TypeKeys "<TAB>"
    sleep (1)
    printlog "Click on the properties icon to open up the properties for the Object"
    Eigenschaften.Click                                 
    kontext "ImageMapHyperlink"
    sleep (1)
    if ImageMapHyperlink.Exists (2) then
        printlog "ImageMapHyperlink (properties) editor exists"
    else
        warnlog "Something is wrong, the ImageMapHyperlink (properties) didn't show up."
    end if

    printlog "Check if the entries we set before has remained the same, with one exception: cnn.com should now have a slash after it. "
    if URL.GetText <> "http://www.cnn.com/" then warnlog "URL should have been: http://www.cnn.com/ . But was " + URL.GetText
'        if Frame.GetSelText <> "http://www.framed.com" then warnlog "Frame should have been: http://www.framed.com. But was " + Frame.GetSelText
    if Objectname.GetText <> "This is a strange name: Huckillerry Bohaahw"  then warnlog "Objectname. should have been: This is a strange name: Huckillerry Bohaahw. But was " + Objectname.GetText
    if Alternativetext.GetText <> "Alternative, the only way to fly..." then warnlog "Alternativetext should have been: . But was " + Alternativetext.GetText
    if Description.GetText <> "Just a Foney-text... or whatever it's called." then warnlog "Description. should have been: . But was " + Description.GetText
    printlog "Close the Properties-Dialogue"
    ImageMapHyperlink.Ok
    sleep (1)

    kontext "ImageMapEditor"
    Zuweisen.Click
    sleep (1)
    ImageMapEditor.Close
    sleep (1)
    printlog "close document"
    Call  hCloseDocument
   
endcase 'tEditImageMapProperties
'------------------------------------------------------------------------------------------------------------------------------------
