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
'* short description : g_mediaplayer.inc
'*
'*********************************************************************
' #1 g_mediaplayer
'\********************************************************************

testcase g_mediaplayer
    dim Datei$
    dim timevalue1
    dim timevalue2
    dim timevalue3
    dim timecounter, iWaitIndex, timefound as integer
   
    if iSprache <> 39 then
        Printlog "- MediaPlayer"
        if gPlatgroup = "unx" then
           QaErrorLog "Solaris / Linux: Soundcard + Java Media Framework (JMF) Media-Libraries is needed."
           goto endsub
        endif
        Datei$ = ConvertPath (convertPath (gTesttoolPath + "graphics\required\input\29secsound.wav"))
        printlog "Open New Document"
        Call hNewDocument
        sleep 2
        printlog "Check if the Media Player already exists - if so - close it"
        kontext "mPlayer"
        printlog "Close the MediaPlayer if it is open already"
        if mPlayer.exists then 
           warnlog "The MediaPlayer was visible since some earlier test."
           ToolsMediaPlayer
           printlog "Closed the Media Player"
        endif
        
        printlog "Tools - MediaPlayer"
        ToolsMediaPlayer
        sleep 2
        printlog "Open file"
        kontext "mPlayer"
        if mPlayer.exists then
           printlog "The MediaPlayer was open."
        else
            warnlog "Tools - MediaPlayer didnt work. The MediaPlayer wasnt visible. Aborting test."
            printlog "Close document"
            Call  hCloseDocument
            goto endsub
        endif
        Mopen.Click
        kontext "OeffnenDlg"
        Pfad.SetText Datei$
        Oeffnen.Click
        
        printlog "This test wont work correctly under Solaris and Linux,"
        printlog "unless some optional Media-libraries are installed."
        printlog "Therefore we warn the tester about it if we cannot open the file."
        
        iWaitIndex = 0
        do while OeffnenDlg.Exists AND iWaitIndex < 10
           sleep (1)
           iWaitIndex = iWaitIndex + 1
        loop
        kontext "Active"
        if Active.exists then
           Warnlog "Error when opening file. Last dialogue said: " + Active.GetText
           Active.OK
           kontext "Mplayer"
           ToolsMediaPlayer
           printlog "close document"
           Call hCloseDocument   
           goto endsub
        else
           kontext "Mplayer"
           if Mplayer.exists then printlog "Media Player was opened."
        endif
        
        timevalue1 = mTimeedit.GetText
        
        printlog "Stop"
        sleep 2
        mStop.Click
        printlog "Just pressed stop."
        sleep 1
        timevalue2 = mTimeedit.GetText
        sleep 2
        timevalue3 = mTimeedit.GetText
        printlog "The time was: " + mTimeedit.GetText
        if timevalue1 = timevalue2 AND timevalue2 = timevalue3 then
            if gtSYSName <> "WinXP" then '("Solaris x86" OR "Linux" OR "Solaris") then
            warnlog "The sound doesnt seem to be played at all. And since the test is run on a platform which needs extra libraries, the test ends now."
            kontext "Mplayer"
            if Mplayer.Exists(3) then
                ToolsMediaPlayer
            endif
            printlog "close document"
            Call  hCloseDocument
            goto endsub
        else
            warnlog "The sound doesnt seem to be played at all, but I will continue the test. The platform was: " + gtSYSName
        endif
        else
           if timevalue1 <> timevalue2 AND timevalue2 = timevalue3 then
              Printlog "the stop-button seemed to work. Good."
           else
              warnlog "Very strange. The first value was: " + timevalue1 + ". Second: " + timevalue2 + ". Third: " + timevalue3
           endif
        endif
        printlog "Play"
        ' Save value
        timevalue1 = mTimeedit.GetText
        ' Press 'Play'
        mPlay.Click
        sleep 2
        timevalue2 = mTimeedit.GetText
        printlog "The time was: " + mTimeedit.GetText
        if timevalue1 = timevalue2 then
           warnlog "The play-button didnt seem to work"  ' Because the time did not change.
        else
           Printlog "the play-button seemed to work. Good."
        endif
        
        printlog "Pause"
        mPlay.Click
        sleep 10
        mPause.Click
        printlog "Pause."
        timevalue1 = mTimeedit.GetText
        sleep 2
        timevalue2 = mTimeedit.GetText
        printlog "The time was: " + mTimeedit.GetText
        if timevalue1 <> timevalue2 then
           warnlog "The pause-button didnt seem to work"
        else
           Printlog "the pause-button seemed to work. Good."
        endif
        
        printlog "Time"
        printlog " Click Stop to get to the beginning."
        mStop.Click  
        sleep 1
        printlog "And make sure it's really the beginning."
        timevalue1 = mTimeedit.GetText
        if timevalue1 <> "00:00:00 / 00:00:29" then
           warnlog "Expected the time to go back to 00:00:00, but was " + timevalue1 + " instead."
            printlog "to make another try to get to the beginning again."
            mStop.Click  '
        else
           printlog "Sound stopped, time-value set to 00:00:00."
        endif
        
        printlog "We press 'Play'"
        printlog "Click play to get it running"
        mPlay.Click 
        sleep 5
        timefound = 0
        
        for timecounter = 1 to 50000
            timevalue1 = mTimeedit.GetText
            if timevalue1 = "00:00:20 / 00:00:29" then
               timecounter = 50000
               timefound = 1
            endif
            if timevalue1 = "00:00:29 / 00:00:29" then
               timecounter = 50000
               timefound = 2
            endif
        next timecounter
        
        if timefound = 1 then
           Printlog "the time-counter seemed to work. Good."
        elseif timefound = 2 then
           warnlog "Contact the test-administrator, this test needs to be rewritten."
        elseif timefound = 0 then
           warnlog "Either the time didnt change, or the test-computer is too fast."
        endif
        
        printlog "Loop"
        mLoop.Click
        
        printlog "Volume"
        
        printlog "Mute"
        mMute.Click
        sleep 1
        mMute.Click
        
        printlog "Time-slider"
        ' mTimeslider. 'Click
         '-Position slider
          'Moves to a different position in the file.
        
        printlog "Scroll-whatever"
        'mZoomlistbox.
        '-Zoom
         'Adjusts the size of the movie playback.
        
        printlog "Apply"
        mInsert.Click
        sleep 10
        
        printlog "Close the MediaPlayer via Tools - MediaPlayer"
        ToolsMediaPlayer
        
        printlog "Check if there is an object available."
        if (gApplication = "IMPRESS") then
           kontext "DocumentImpress"
        else
           kontext "DocumentDraw"
        endif
        hTypeKeys ("<escape><tab><tab><tab>")
        hOpenContextMenu()
        hMenuSelectNr(1)
        
        Kontext "TabPositionAndSize"
        Kontext
        Active.SetPage TabPositionAndSize
        Kontext "TabPositionAndSize"
        if Width.Exists then
           Printlog "Position and Size came up, object inserted correctly."
           printlog "Close dialog TabPositionAndSize with 'OK'"
           TabPositionAndSize.OK
        else
           Warnlog "Position And Size didn't come up, was there no sound-object inserted?"
        endif
        sleep 2
        printlog "close document"
        Call  hCloseDocument	
    else
        qaerrorLog "#74350# - Different strings in g_mediaplayer under Italian."
    endif
endcase