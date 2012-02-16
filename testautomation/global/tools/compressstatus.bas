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
'* short description : Compress local written status files for submission
'*
'\***********************************************************************

sub main
	'just run, ...
	if (gMahler AND (gMahlerLocal<>"")) then
		' just a dummy call to get gDatabasePath set; DON'T call hStatusOut !!! 
		hStatusIn("writer", "compressstatus.bas")
		call compressStatus
	else
		warnLog "There is nothing to be done - exiting"
	endif
end sub

'-------------------------------------------------------------------------

sub compressStatus
       dim sDestination as string
       dim sList(1000) as string
       dim i as integer
       dim iReturn as integer
       dim sJar as string

       sDestination = convertPath(gDatabasePath+"database/" + "mahlerlocal/")
       ' create directory beside mahlerlocal
'       sJar = convertPath(gDatabasePath+"database/" + convertDateToDatabase(now()))
       sJar = convertPath(gDatabasePath+"database/" + convertDateToDatabase(now()) + "-"+removeCharacter(convertTimeToDatabase(now()),asc(":")))
       mkdir sJar
       ' create jar file with same name as directory, beside mahlerlocal
       try
           iReturn = Shell("jar",0,"cMf " + sJar + ".jar" + " -C " + sDestination + " .",TRUE) ' wait until finished
           printlog "jar cMf " + sJar + ".jar" + " -C " + sDestination + " ."
       catch
           printlog "No program 'jar' available"
           try
                iReturn = Shell("zip",0,"-Djr " + sJar + ".jar" + " " + sDestination,TRUE) ' wait until finished
                printlog "zip -Djr " + sJar + ".jar" + " " + sDestination
           catch
                printlog "No program 'zip' available"
                iReturn = fZip(sDestination, sJar +".jar")
           endcatch
       endcatch
       if (iReturn <> 0) then
           printlog iReturn
       endif
       if (iReturn = 0) then
           printlog "Filename to submit:"
           printlog sJar + ".jar"
           ' copy files from mahlerlocal to backupdirectory with same name as jar file
           getFileList(sDestination, "*.*", sList())
           for i = 1 to listCount(sList())
                try
                    filecopy(sList(i), sJar+gPathSigne)
                catch
                    if (1=i) then warnlog "#ixxxxxx# destination file name needs to get named."
                    filecopy(sList(i), sJar+gPathSigne+DateiExtract(sList(i)))
                endcatch
                ' delete file in mahlerlocal
                kill(sList(i))
                if fileexists(sList(i)) then
                    warnlog "file couldn't get deleted! remove manually:"
                    printlog sList(i)
                endif
           next i
       endif
end sub

'-------------------------------------------------------------------------

function fZip(sDirectory as string, sZipFileName as string) as integer
'/// Zips the files in the first level of a directory into a file
'///+ The zip file hasn't to exists
'///+ Input: absolut directory path to zip
'///+ Absolut path and filename of zip file
	dim oUCB
	dim oUCB2
	dim oID
	dim oRootContent
	dim oInfo
	dim oNewStreamContent
	dim oFile
	dim oArg
    Dim aArgs(1)
    Dim oProps(0) as new com.sun.star.beans.PropertyValue
    Dim oCommand as new com.sun.star.ucb.Command
    dim lsFile(500) as string
    dim i as integer
    dim aArray
    dim sString
	
    fZip = 1
    if fileExists(sZipFileName) then
        warnlog "Can't create zip file, because it already exists: '" + sZipFileName + "'"
        exit function
    endif
    if NOT fileExists(sDirectory) then
        warnlog "Directory to zip doesn't exist: '" + sDirectory + "'"
        exit function
    else
        aArgs(0) = "Local"
        aArgs(1) = "Office"
        oUCB = CreateUnoService( "com.sun.star.ucb.UniversalContentBroker" )
        oUCB.initialize( aArgs() )
        printlog "Zip file name: '" + convertToURL(sZipFileName) + "'"
        aArray = split(convertToURL(sZipFileName), "/")
        sString = join(aArray, "%2F")
        printlog "Zip file name: '" + sString + "'"
        oID = oUCB.createContentIdentifier( "vnd.sun.star.zip://" + sString )
        oRootContent = oUCB.queryContent( oID )
        oInfo = createUnoStruct( "com.sun.star.ucb.ContentInfo" )
        oInfo.Type = "application/vnd.sun.star.zip-stream"
        oInfo.Attributes = 0
        
        ' get all files in a directory
        getFileNameList (sDirectory+"/","*.txt",lsFile())
        printlog "Going to zip Directory: '" + sDirectory + "'"
        for i = 1 to listCount(lsFile())
            printlog "Going to add: " + i + ": '" + lsFile(i) + "'"
            oNewStreamContent = oRootContent.createNewContent( oInfo )
            oProps(0).Name = "Title"
            oProps(0).Handle = -1
            oProps(0).Value = lsFile(i) ' Filename of one content file in zip
            oCommand.Name = "setPropertyValues"
            oCommand.Handle   = -1
            oCommand.Argument = oProps()
            oNewStreamContent.execute( oCommand, 0, Null )
            oUcb2 = createUnoService("com.sun.star.ucb.SimpleFileAccess")   
            oFile = oUcb2.OpenFileRead(ConvertToURL(sDirectory + "/" + lsFile(i)))
            oArg = createUnoStruct( "com.sun.star.ucb.InsertCommandArgument" )
            oArg.Data = oFile
            oArg.ReplaceExisting = false
            oCommand.Name = "insert"
            oCommand.Handle = -1
            oCommand.Argument = oArg
            oNewStreamContent.execute( oCommand, 0, Null )
        next i
    
        REM commit that package file
        oCommand.Name = "flush"
        oCommand.Handle = -1
        oCommand.Argument = 0
        
        oRootContent.execute( oCommand, 0, Null )
        fZip = 0
    endif
end function

'-------------------------------------------------------------------------

sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"
   gApplication = "WRITER"
   call GetUseFiles
end sub

'-------------------------------------------------------------------------

