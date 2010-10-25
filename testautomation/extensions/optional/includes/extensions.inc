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
'* owner : thorsten.bosbach@oracle.com
'*
'* short description : Function test of extension manager
'*
'\************************************************************************

testcase tExtensionsFunction

    ' written for CWS jsc7, in master since SRC680m176
    ' additions for CWS sb63 - Dependency checking with versioning - documents 24-30
    ' features: - http://specs.openoffice.org/appwide/packagemanager/simple_extension_license.odt
    '           - http://specs.openoffice.org/appwide/packagemanager/extensiondependencies1.odt
    '/// The test documents are available at http://qa.openoffice.org/source/browse/qa/qatesttool/framework/basic/input/extensions/ ///'
    '/// Textdocuments used in this test and expected messages: ///'
    '/// (1/31): simpleLicense\BadDesc.oxt ///'
    '/// - BadDesc.oxt/description.xml contains data which cannot be parsed. ///'
    '/// - will not install ///'
    '/// (2/31): simpleLicense\BadNamespace.oxt ///'
    '/// - BadNamespace.oxt/description.xml contains a root element with an unsupported namespace. ///'
    '/// - will not install ///'
    '/// (3/31): simpleLicense\BadRoot.oxt ///'
    '/// - BadRoot.oxt/description.xml does not contain the root element <description>. ///'
    '/// - will not install ///'
    '/// (4/31): simpleLicense\IncompAttr1.oxt ///'
    '/// - ExtensionSoftwareLicenseAgreement dialog exists ///'
    '/// - installs and is enabled ///'
    '/// (5/31): simpleLicense\IncompAttr2.oxt ///'
    '/// - Cannot find a localized license text or a default license text. ///'
    '/// - will not install ///'
    '/// (6/31): simpleLicense\LongLic.oxt ///'
    '/// - ExtensionSoftwareLicenseAgreement dialog exists ///'
    '/// - It is a Long License ///'
    '/// - installs and is enabled ///'
    '/// (7/31): simpleLicense\MissingLic.oxt ///'
    '/// - MissingLic.oxt\registration\license_en_US.txt does not exist. ///'
    '/// - will not install ///'
    '/// (8/31): simpleLicense\MissingLicRef.oxt ///'
    '/// - Cannot find a localized license text or a default license text. ///'
    '/// - will not install ///'
    '/// (9/31): simpleLicense\NoDesc.oxt ///'
    '/// - installs and is enabled ///'
    '/// (10/31): simpleLicense\NoLangNoDefault.oxt ///'
    '/// - Cannot find a localized license text or a default license text. ///'
    '/// - will not install ///'
    '/// (11/31): simpleLicense\OnlyDefault.oxt ///'
    '/// - Cannot find a localized license text or a default license text. ///'
    '/// - will not install ///'
    '/// (12/31): simpleLicense\ShortLicense.oxt ///'
    '/// - ExtensionSoftwareLicenseAgreement dialog exists ///'
    '/// - installs and is enabled ///'
    '/// (13/31): ///'
    '/// - will not install ///'
    '/// (14/31): simpleLicense\ShortLicenseShared.oxt ///'
    '/// - ExtensionSoftwareLicenseAgreement dialog exists ///'
    '/// - installs and is enabled ///'
    '/// (15/31): dependencies\broken-dependency.oxt ///'
    '/// - broken-dependency.oxt/description.xml contains data which cannot be parsed. ///'
    '/// - will not install ///'
    '/// (16/31): dependencies\double-dependencies.oxt ///'
    '/// - UnsatisfiedDependencies dialog exists ///'
    '///     (1/2): Dependency 1 ///'
    '///     (2/2): Dependency 2 ///'
    '/// - will not install ///'
    '/// (17/31): dependencies\empty-dependencies.oxt ///'
    '/// - installs and is enabled ///'
    '/// (18/31): dependencies\funny-dependency.oxt ///'
    '/// - UnsatisfiedDependencies dialog exists ///'
    '///     (1/1): & < > ' " > ' tab	. crlf ///'
    '///           . em-dash�;. line-separator?. paragraph-separator?. xxxxxxxxxxx... ///'
    '/// - will not install ///'
    '/// (19/31): dependencies\license-dependency.oxt ///'
    '/// - UnsatisfiedDependencies dialog exists ///'
    '///     (1/1): Dependency ///'
    '/// - will not install ///'
    '/// (20/31): dependencies\many-dependencies.oxt ///'
    '/// - UnsatisfiedDependencies dialog exists ///'
    '///     (1/20): Dependency 1 ///'
    '///     (2/20): Dependency 2 ///'
    '///     (3/20): Dependency 3 ///'
    '///     (4/20): Dependency 4 ///'
    '///     (5/20): Dependency 5 ///'
    '///     (6/20): Dependency 6 ///'
    '///     (7/20): Dependency 7 ///'
    '///     (8/20): Dependency 8 ///'
    '///     (9/20): Dependency 9 ///'
    '///     (10/20): Dependency 10 ///'
    '///     (11/20): Dependency 11 ///'
    '///     (12/20): Dependency 12 ///'
    '///     (13/20): Dependency 13 ///'
    '///     (14/20): Dependency 14 ///'
    '///     (15/20): Dependency 15 ///'
    '///     (16/20): Dependency 16 ///'
    '///     (17/20): Dependency 17 ///'
    '///     (18/20): Dependency 18 ///'
    '///     (19/20): Dependency 19 ///'
    '///     (20/20): Dependency 20 ///'
    '/// - will not install ///'
    '/// (21/31): dependencies\no-dependencies.oxt ///'
    '/// - installs and is enabled ///'
    '/// (22/31): dependencies\no-description.oxt ///'
    '/// - installs and is enabled ///'
    '/// (23/31): dependencies\unknown-dependency.oxt ///'
    '/// - UnsatisfiedDependencies dialog exists ///'
    '///     (1/1): Unknown ///'
    '/// - will not install ///'
    '/// (24/31): dependencies\version21.oxt ///'
    '/// - installs and is enabled ///'
    '/// (25/31): dependencies\version10000.oxt ///'
    '/// - UnsatisfiedDependencies dialog exists ///'
    '///     (1/1): Unknown ///'
    '/// - will not install ///'
    '/// (26/31): dependencies\version21ns.oxt ///'
    '/// - installs and is enabled ///'
    '/// (27/31): dependencies\version21other.oxt ///'
    '/// - UnsatisfiedDependencies dialog exists ///'
    '///     (1/1): Unknown ///'
    '/// - will not install ///'
    '/// (25/31): dependencies\version25.oxt ///'
    '/// - installs and is enabled ///'
    '/// (25/31): dependencies\version30.oxt ///'
    '/// - installs and is enabled ///'
    '/// (25/31): dependencies\version31.oxt ///'
    '/// - UnsatisfiedDependencies dialog exists ///'
    '///     (1/1): Unknown ///'
    '/// - will not install ///'
    '/// (27/31): dependencies\version21other.oxt ///'
    '/// - UnsatisfiedDependencies dialog exists ///'
    '///     (1/1): Unknown ///'
    '/// (30/31): dependencies\versionempty.oxt ///'
    '/// - installs and is enabled ///'
    '/// (31/31): dependencies\versionnone.oxt ///'
    '/// - installs and is enabled ///'
    '/// (32/41): versions/plain.oxt
    '/// - installs and is enabled ///'
    '/// (33/41): versions/plain.oxt
    '/// - You are about to install version 1.02.4.7.0 of the extension org.openoffice.legacy.plain.oxt. That version is already installed.
    '/// - installs and is enabled ///'
    '/// (34/41): versions/version_none/plain.oxt
    '/// - You are about to install version 0 of the extension org.openoffice.legacy.plain.oxt. The newer version 1.02.4.7.0 is already installed.
    '/// - installs and is enabled ///'
    '/// (35/41): versions/version_1.2.3/plain.oxt
    '/// - You are about to install version 1.2.3 of the extension org.openoffice.legacy.plain.oxt. The older version 0 is already installed.
    '/// - installs and is enabled ///'
    '/// (36/41): versions/version_1.2.3/plain.oxt
    '/// - You are about to install version 1.2.3 of the extension org.openoffice.legacy.plain.oxt. That version is already installed.
    '/// - installs and is enabled ///'
    '/// (37/41): versions/version_0.0/plain.oxt
    '/// - You are about to install version 0.0 of the extension org.openoffice.legacy.plain.oxt. The newer version 1.2.3 is already installed.
    '/// - installs and is enabled ///'
    '/// (38/41): versions/version_1.2.4.7/plain.oxt
    '/// - You are about to install version 1.2.4.7 of the extension org.openoffice.legacy.plain.oxt. The older version 0.0 is already installed.
    '/// - installs and is enabled ///'
    '/// (39/41): versions/version_1.02.4.7.0/plain.oxt
    '/// - You are about to install version 1.02.4.7.0 of the extension org.openoffice.legacy.plain.oxt. That version is already installed.
    '/// - installs and is enabled ///'
    '/// (40/41): versions/version_1.2.3/plain.oxt
    '/// - You are about to install version 1.2.3 of the extension org.openoffice.legacy.plain.oxt. The newer version 1.02.4.7.0 is already installed.
    '/// - installs and is enabled ///'
    '/// (41/41): versions/version_1.2.15.3/plain.oxt
    '/// - You are about to install version 1.2.15.3 of the extension org.openoffice.legacy.plain.oxt. The older version 1.2.3 is already installed.

    dim j,b,i,a,x,y as integer
    dim sPath as string
    dim aFiles(50) as string
    dim aFeatures(50) as string
    dim aFileList(50) as string
    dim sTemp as string
    dim sTemp2 as string
    dim sTemp3 as string
    dim sTemp4 as string
    dim aTemp
    dim aTemp2
    dim sFirstStatus as string
    dim bFound as boolean
    dim aRemovePackages(50) as string
    dim bFoundNumber as boolean
    dim sLastVersion as string

    sLastVersion = ""
    ' want to start and work with just one document window
    b = getDocumentCount
    hFileCloseAll()
    hNewDocument()

    ' load about twenty one extension files, to test the features
    ' the list tells the filename and in encoded numbers the features
    ' a line looks like:
    ' simpleLicense\LongLic.oxt;0,2,1,0
    ' - the filename and the features are seperated by a semicolon
    ' - the numbes for the features are seperated by colons
    ' 1st number: Unsatisfied dependencies; if there are any, the number counts them; if there are none it is zero and the dialog won't come up
    ' 2nd number: License; if there is one the value is 1; it it a long one, where the scroll button needs to be used it is 2; 3 if the license should not be accepted for testing purposes, if there is no license the value is zero
    ' 3rd number: If the extension should successfully install the value is one, else zero.
    ' 4th number: If an active is expected after selecting a file for installation the value is one, else zero.
    ' 5th number: If active is expected the expected version information after selecting a file for installation the value is one, else zero and if an upgrade/downgrade is expected, the number is 2.

    ' Base path for test files
    sPath = convertPath(gTestToolPath+"extensions/optional/input/")
    ' read filelist and expected features
    ListRead(aFileList(), sPath+"files.txt")
    a = listCount(aFileList())
    printlog a
    for i = 1 to a
        aTemp = split (aFileList(i),";")
        if uBound(aTemp()) >0 then
            listAppend(aFiles(),aTemp(0))
            listAppend(aFeatures(),aTemp(1))
        endif
    next i

    ' there are some dependencys to languages...
    if iSprache = 49 then
        sTemp = aFeatures(4)
        aFeatures(4) = aFeatures(5)
        aFeatures(5) = sTemp
    else
        if iSprache <> 01 then
            aFeatures(4) = aFeatures(5)
            aFeatures(6) = aFeatures(5)
            aFeatures(12) = aFeatures(5)
            aFeatures(13) = aFeatures(5)
            aFeatures(14) = aFeatures(5)
        endif
    endif
    ' cleanup before starting; check that no extension from the list is installed right now; if remove it.
    '/// Before starting, look into extension manager, if already a file is installed, that will be used in this test ///'
    '/// Remove it. ///'
    a = listCount(aFiles())
    dim ii as integer
    ToolsPackageManager
    kontext "PackageManager"
    If PackageManager.exists(10) then
        BrowsePackages.typeKeys("<home>")
        ii = 1
        while (ii < BrowsePackages.getItemCount+1 )
            kontext "PackageManager"
            'BrowsePackages.typeKeys("+")
            'BrowsePackages.typeKeys("<down>")
            BrowsePackages.select ii
            sTemp = BrowsePackages.getItemText (ii,1)
            if right(sTemp,3) = "oxt" then
                for x = 1 to a
                    sTemp3 = aFiles(x)
                    aTemp = split(sTemp3,"\")
                    if uBound(aTemp) < 1 then
                        aTemp = split(sTemp3,"/")
                    endif
                    sTemp3 = aTemp(uBound(aTemp))
                    if sTemp = sTemp3 then
                        qaErrorLog "Package is already installed, trying to remove it: '" + sTemp + "'"
                        if remove.isEnabled then
                            remove.click
                            
                            kontext "Active"
                            if ( Active.exists( 2 ) ) then
                                Active.ok()
                            else
                                warnlog( "No confirmation dialog for Extension removal" )
                            endif
                            
                            Kontext "AddingPackages"
                            y=0
                            while AddingPackages.exists(1) AND (y<30)
                                printlog "RemovingPackages..."
                                sleep 1
                                y=y+1
                            wend
                            kontext "PackageManager"
                            BrowsePackages.typeKeys("<up>")
                            x = 100
                        else
                            warnlog "Package is already installed and can't get removed: '" + sTemp + "'"
                            ii=ii+1
                            x = 101
                        endif
                    endif
                next x
                if x < 100 then
                    ii=ii+1
                endif
            else
                ii=ii+1
            endif
        wend
        kontext "PackageManager"
        PackageManager.close
        WaitSlot( 5000 )
    endif

    ' for every test file do...
    a = listCount(aFiles())
    for i = 1 to a
        printlog "********** Test file ("+i+"/"+a+"): "+aFiles(i)
        aTemp = split (aFeatures(i),",")
        '/// call Tools - Package/Extension manager ///'
        ToolsPackageManager
        kontext "PackageManager"
        If PackageManager.exists(10) then
            '/// going to top of package list and explode every line, until the button 'Add' is enabled ///'
            '/// should be 'My Packages' ///'
            'BrowsePackages.typeKeys("<home>")
            'sTemp = BrowsePackages.getSelText
            'sTemp2 = ""
            'while (Not Add.isEnabled AND (sTemp <> sTemp2))
            '    BrowsePackages.typeKeys("+")
            '    BrowsePackages.typeKeys("<down>")
            '    sTemp2 = sTemp
            '    sTemp = BrowsePackages.getSelText
            'wend
            'if sTemp = sTemp2 then
            '    warnlog "Couldn't find a way to enable the Add button - aborting"
            '    goto endsub
            'endif
            'printlog BrowsePackages.getSelText
            '/// click button 'Add' ///'
            try
                Add.click
            catch
                warnlog "#i90870# extension manager deadlock/C++ runtimes"
                exitrestarttheoffice
                ToolsPackageManager
                kontext "PackageManager"
                Add.click
            endcatch
            '/// Add Package(s)... dialg comes up ///'
            Kontext "OeffnenDlg"
            if OeffnenDlg.exists(5) then
                '/// put filename into field ///'
                Dateiname.setText convertPath(sPath + aFiles(i))
                printlog DateiTyp.getSelText
                '/// press button 'Open' ///'
                Oeffnen.click
            else
                warnlog "Add Package(s)... dialog didn't came up"
            endif

            'Asking for whom to install; All: YES, Me: NO, nobody: Cancel
            kontext
            if active.exists(5) then
                sTemp = active.getText
                printlog "Messagebox: " + sTemp
                ' We only want to handle the installation target here which only 
                ' pops up if we have the choice (rights to write to shared layer)
                if active.getButtonCount = 3 then 
                    try
                        active.yes
                    catch
                        warnlog( "Try...catch: Failed to click Yes-button" )
                    endcatch
                else
                    printlog "No installation target message (this is probably a root-installation)" 
                endif
            endif

            '/// Progress dialog comes up ///'
            Kontext "AddingPackages"
            if AddingPackages.exists(1) then
                printlog " - AddingPackages dialog exists"
                'DialogTest(AddingPackages)
            endif

            '/// If any error happens, an active comes up, telling what is wrong ///'
            ' error is just a dialog with one button
            ' version infos have two buttons
            Kontext
            if active.exists(5) then
                sTemp = active.getText
                select case active.getbuttoncount
                case 1:
                    y = instr(sTemp, " ")
                    if cBool(aTemp(3)) then
                        printlog sTemp
                    else
                        warnlog sTemp
                    endif
                case 2:
                    x = 0
                    if cBool(aTemp(4)) then
                        printlog "Version info message available."
                    else
                        warnlog "Version info message available, but not expected"
                    endif
                    printlog sTemp
                    bFoundNumber = false
                    aTemp2 = split(sTemp," ")
                    y = uBound(aTemp2)
                    while ((NOT bFoundNumber) AND (x < y))
                        stemp2 = left(aTemp2(x),1)
                        if isNumeric(stemp2) then
                            bFoundNumber = TRUE
                            printlog  "Try to install version: " + aTemp2(x)
                        endif
                        x=x+1
                    wend
                    if NOT bFoundNumber then
                        warnlog "No version was mentioned in the version message!"
                    endif
                    bFoundNumber = false
                    while (NOT bFoundNumber AND x < y)
                        stemp2 = left(aTemp2(x),1)
                        if isNumeric(stemp2) then
                            bFoundNumber = TRUE
                        endif
                        x=x+1
                    wend
                    if aTemp(4) = "2" AND bFoundNumber then
                        printlog "Old version is: " + aTemp2(x-1)
                    elseif bFoundNumber then
                        warnlog "No old version expected: " + aTemp2(x-1)
                    elseif aTemp(4) = "2" then
                        warnlog "Old version expected, but not mentioned."
                    endif
                end select
                active.ok
            else
                if cBool(aTemp(3)) then
                    if i = 3 then
                        qaErrorLog "#i73367# "
                    else
                        warnlog "errormessage expected, but none came up."
                    endif
                endif
            endif

            '/// If there are any unsatisfied dependencies, they are shown now ///'
            Kontext "UnsatisfiedDependencies"
            if UnsatisfiedDependencies.exists(10) then
                if cBool(aTemp(0)) then
                    printlog " - UnsatisfiedDependencies dialog exists"
                else
                    warnLog "UnsatisfiedDependencies are not expected but available"
                endif
                DialogTest(UnsatisfiedDependencies)
                y = UnsatisfiedDependencyList.getItemCount
                if cInt(aTemp(0)) <> y then
                    warnlog "Difference in dependencies; expected: " + aTemp(0) + "; found: " +y
                endif
                for x = 1 to y
                    printlog "   ("+x+"/"+y+"): "+UnsatisfiedDependencyList.getItemText(x)
                next x
                '/// close dialog if available, extension will not be installed ///'
                UnsatisfiedDependencies.close
            else
                if cBool(aTemp(0)) then
                    warnLog "UnsatisfiedDependencies expected but none shown."
                endif
            endif

            '/// If there is a license, it is shown now ///'
            Kontext "ExtensionSoftwareLicenseAgreement"
            if ExtensionSoftwareLicenseAgreement.exists(10) then
                if cBool(aTemp(1)) then
                    printlog " - ExtensionSoftwareLicenseAgreement dialog exists"
                else
                    warnlog "ExtensionSoftwareLicenseAgreement are not expected but shown"
                endif
                if NOT Accept.isEnabled then
                    if cInt(aTemp(1)) = 2 then
                        printlog "   Long License"
                    else
                        errorlog "Long License"
                    endif
                else
                    if NOT cInt(aTemp(1)) = 2 then
                        printlog "Long License"
                    else
                        errorlog "Long License"
                    endif
                endif
                '/// scroll down until accept button is enabled, click accept button ///'
                LicenseText.typeKeys ("<mod1 a>",true)
                if Accept.isEnabled then
                    '/// for testing purposes click cancel one time - the extension should not get installed ///'
                    if cInt(aTemp(1)) = 3 then
                        ExtensionSoftwareLicenseAgreement.CANCEL
                    else
                        Accept.click
                    endif
                else
                    qaErrorLog "Accept not enabled.yet"
                    while NOT Accept.isEnabled
                        ScrollDown.click
                        printlog "Scrolling..."
                        sleep 1
                    wend
                    '/// for testing purposes click cancel one time - the extension should not get installed ///'
                    if cInt(aTemp(1)) = 3 then
                        ExtensionSoftwareLicenseAgreement.CANCEL
                    else
                        Accept.click
                    endif
                endif
            else
                if cBool(aTemp(1)) then
                    errorlog "ExtensionSoftwareLicenseAgreement dialog expected but not shown."
                endif
            endif

            '/// There may come up another active right now, which would be an error ///'
            Kontext
            if active.exists(5) then
                sTemp = active.getText
                if i = 7 then
                    qaerrorlog "#i67452# " + sTemp
                else
                    if i = 13 then
                        qaerrorlog "#i73307#" + stemp
                    else
                        warnlog sTemp
                    endif
                endif
                active.ok
            endif

            '/// the progress dialog is now available again ///'
            Kontext "AddingPackages"
            x=0
            while AddingPackages.exists(1) AND (x<30)
                printlog "AddingPackages..."
                sleep 1
                x=x+1
            wend

            '/// There may come up another active right now, which would be an error ///'
            Kontext
            if active.exists(5) then
                sTemp = active.getText
                warnlog sTemp
                active.ok
            endif

            ' find out, if the extension got installed
            kontext "PackageManager"
            BrowsePackages.typeKeys("<home>")
            BrowsePackages.typeKeys("+")
            y = BrowsePackages.getItemCount
            sTemp = aFiles(i)
            aTemp2 = split(sTemp,"\")
            if uBound(aTemp2) < 1 then
                aTemp2 = split(sTemp,"/")
            endif
            sTemp = aTemp2(uBound(aTemp2))
            printlog stemp
            bFound = FALSE
            for x = 1 to y
                sTemp2 = BrowsePackages.getItemText(x,1)
                printlog "("+x+"/"+y+"): "+sTemp2
                ' Mechanism to detect a status change; report it only one time
                sTemp3 = BrowsePackages.getItemText(x,3) ' enabled?
                sTemp4 = BrowsePackages.getItemText(x,2) ' version
                if sTemp3 <> "" AND sFirstStatus = "" then
                    sFirstStatus = sTemp3
                else
                    if (sFirstStatus <> sTemp3) AND (sTemp3 <> "") AND (sFirstStatus <> "alreadyshown") then
                        qaErrorlog "#i67450# " + sFirstStatus + " <> " + sTemp3
                        sFirstStatus= "alreadyshown"
                    endif
                endif
                if sTemp3 <> "" then
                    printlog "   +++ " + sTemp3
                endif
                if sTemp4 <> "" then
                    sLastVersion = sTemp4
                    printlog "   Version:" + sTemp4
                endif
                if sTemp = sTemp2 then
                    listAppend(aRemovePackages(), sTemp)
                    if cBool(aTemp(2)) then
                        printlog  "-------------------------------- Extension got installed"
                        bFound = TRUE
                    else
                        if i = 3 then
                            qaerrorLog "#i73367# "
                        else
                            warnLog "-------------------------------- Extension got installed, but shouldn't do that"
                        endif
                    endif
                endif
            next x
            if NOT cBool(aTemp(2)) and bFound then
                warnlog "not found"
            endif
            '/// Close extension manager ///'
            PackageManager.OK
            'OK / Close
        else
            warnlog "Couldn't call Tools -> Package manager - aborting test at " + i
            goto endsub
        endif
    next i

    '/// after installing all extensions, remove them ///'
    ' uninstall all components, that were installed
    a = listCount(aRemovePackages())
    ToolsPackageManager

    kontext "PackageManager"
    If PackageManager.exists(10) then

        BrowsePackages.typeKeys("<home>")
        waitslot()

        ii = 1
        while (ii < BrowsePackages.getItemCount +1)
        
            wait( 2000 ) ' Required, Extension Manager needs time to populate
            kontext "PackageManager"
            'BrowsePackages.typeKeys("+")
            'BrowsePackages.typeKeys("<down>")
            sTemp2 = sTemp
            BrowsePackages.select ii
            waitslot()
            
            sTemp = BrowsePackages.getItemText (ii,1)
            if right(sTemp,3) = "oxt" then
                for x = 1 to a
                    if sTemp = aRemovePackages(x) then
                        printlog "Package is installed, trying to remove it: '" + sTemp + "'"
                        if remove.isEnabled then
                            remove.click

                            kontext "Active"
                            if ( Active.exists( 2 ) ) then
                                Active.ok()
                            else
                                warnlog( "No confirmation for extension removal" )
                            endif

                            Kontext "AddingPackages"
                            y=0
                            while AddingPackages.exists(1) AND (y<30)
                                printlog "RemovingPackages..."
                                sleep 1
                                y=y+1
                            wend
                            kontext "PackageManager"
                            BrowsePackages.typeKeys("<up>")
                            x = 100
                        else
                            warnlog "Package can't get removed: '" + sTemp + "'"
                            ii=ii+1
                        endif
                    endif
                next x
            endif
            ii = ii+1
        wend
        kontext "PackageManager"
        hCloseDialog( PackageManager, "close" )
    else
        qaErrorLog "disabling installed extensions failed;"
    endif
    WaitSlot()
    hFileCloseAll()
endcase


