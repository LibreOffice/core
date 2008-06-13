'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: resetoffice.bas,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-13 10:27:06 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/************************************************************************
'*
'* owner : thorsten.bosbach@sun.com
'*
'* short description : Exit the [Star|Open][Office|Suite][.org] and delete user directory
'*
'\*************************************************************************************

sub main
    dim uno
    dim ap
    dim xViewRoot
    dim apara(1) As new com.sun.star.beans.PropertyValue
    dim temp()
    dim i,x as integer
    dim sString as string
    dim fDeleteList(32000) as string
    dim sLanguage as string
    dim bError as boolean
    dim sDefaultLocale as string
    dim sDefaultLocaleCJK as string
    dim sDefaultLocaleCTL as string

    sString = "qatesttool/global/tools/resetoffice.bas:: "
    uno=hGetUnoService()
    
    'Get UI language
    try
        ap=uno.createInstance("com.sun.star.configuration.ConfigurationProvider")
        apara(0).Name="nodepath"
        apara(0).Value="/org.openoffice.Office.Linguistic/General"
        apara(1).Name="lazywrite"
        apara(1).Value=False
        xViewRoot=ap.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess",aPara())
        sLanguage = xViewRoot.getPropertyValue("UILocale")
        sDefaultLocale = xViewRoot.getPropertyValue("DefaultLocale")
        sDefaultLocaleCJK = xViewRoot.getPropertyValue("DefaultLocale_CJK")
        sDefaultLocaleCTL = xViewRoot.getPropertyValue("DefaultLocale_CTL")
        printlog "Old UI language: '" + sLanguage + "'"
        printlog "Old default locale: '" + sDefaultLocale + "'"
        printlog "Old default locale CJK: '" + sDefaultLocaleCJK + "'"
        printlog "Old default locale CTL: '" + sDefaultLocaleCTL + "'"
        xViewRoot.dispose()
        bError = FALSE
    catch
        warnlog sString + "Failed to read UI language."
        bError = TRUE
    endcatch

    if NOT bError then
        'Close OOo
        try
            ' To prevent restarting of OOo, the try/catch is around this and
            ' to prevent messages about communication errors
            printlog ResetApplication
            FileExit "SynchronMode", TRUE
            try
                ' It is no error, if this fails - so it gets its own try/catch
                kontext
                if active.exists(5) then
                    active.no 'discard changes
                endif
            catch
            endcatch
            bError = FALSE
        catch
            warnlog sString + "Failed to close OOo."
            bError = TRUE
        endcatch
        sleep 10 'To wait until OOo is realy away
    endif

    'Remove user directory
    if NOT bError then
        try
            if (right(gOfficePath,1)=gPathSigne) then
                'Dir doesn't work, is a path singe is at the end
                gOfficePath = left(gOfficePath,len(gOfficePath)-1)
            endif
            printlog "Going to delete directory: '" + gOfficePath + "'"
            if (dir(gOfficePath) = "") then
                qaErrorlog "Directory is already deleted."
            else
                rmDir (gOfficePath)
                if (dir(gOfficePath) <> "") then
                    warnlog "Directory wasn't deleted."
                endif
            endif
            bError = FALSE
        catch
            warnlog sString + "Failed to delete user directory."
            bError = TRUE
        endcatch
    endif

    'Start OOo and restore language
    'Needs only to be done, if UI language wasn't the default (!= "")
    if ((sLanguage & sDefaultLocale & sDefaultLocaleCJK & sDefaultLocaleCTL) <> "") then
        try
            hStartTheOffice
            uno=hGetUnoService()
            ap=uno.createInstance("com.sun.star.configuration.ConfigurationProvider")
            apara(0).Name="nodepath"
            apara(0).Value="/org.openoffice.Office.Linguistic/General"
            apara(1).Name="lazywrite"
            apara(1).Value=False
            xViewRoot=ap.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess",aPara())
            if (sLanguage <> "") then
                printlog "------------------------------ UI language: " + sLanguage
                xViewRoot.setPropertyValue("UILocale", sLanguage)
                xViewRoot.commitChanges()
            endif
            if (sDefaultLocale <> "") then
                printlog "------------------------------ default locale: " + sDefaultLocale
                xViewRoot.setPropertyValue("DefaultLocale", sDefaultLocale)
                xViewRoot.commitChanges()
            endif
            if (sDefaultLocaleCJK <> "") then
                printlog "------------------------------ default locale CJK: " + sDefaultLocaleCJK
                xViewRoot.setPropertyValue("DefaultLocale_CJK", sDefaultLocaleCJK)
                xViewRoot.commitChanges()
            endif
            if (sDefaultLocaleCTL <> "") then
                printlog "------------------------------ default locale CTL: " + sDefaultLocaleCTL
                xViewRoot.setPropertyValue("DefaultLocale_CTL", sDefaultLocaleCTL)
                xViewRoot.commitChanges()
            endif
            if xViewRoot.hasPendingChanges() then
                warnlog(sFileFunction+"Changes still pending...")
            endif
            xViewRoot.dispose()
            exitRestartTheOffice
        catch
            warnlog sString + "Failed to set UI language."
        endcatch
    else
        'open OOo?
    endif
    'Close OOo ?
    
    'If it still loses the language information, we have to create a file with
    'the information of the language we want to test!
    
    'Before this script is run you might need to kill the office!
    'Else you get problems with UNO access to OOo
end sub

sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"
   Call GetUseFiles()
end sub

