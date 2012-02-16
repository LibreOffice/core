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
'* short description : Exit the [Star|Open][Office|Suite][.org] and delete user directory
'*
'\*************************************************************************************

sub main
    dim sString as string

    sString = "qatesttool/global/tools/resetoffice.bas:: "
    
        'Close OOo
        try
            ' To prevent restarting of OOo, the try/catch is around this and
            ' to prevent messages about communication errors
            printlog ResetApplication
            FileExit "SynchronMode", TRUE
        catch
            warnlog sString + "Failed to close OOo."
        endcatch
        sleep 10 'To wait until OOo is realy away

    'Remove user directory
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
        catch
            warnlog sString + "Failed to delete user directory."
        endcatch
end sub

sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"
   Call GetUseFiles()
end sub

