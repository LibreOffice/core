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
'*  short description : Extension Update Test
'*
'\******************************************************************************

function hCheckForAdministratorPermissions() as boolean

    ' this function returns TRUE if the user can create files in the office
    ' program directory. This is relevant for macro security (VBA compatibility)
    ' and extension manager behavior (access to shared installations).S

    dim iFile as integer
    dim cProbeFile as string
    
    cProbeFile = convertpath( gNetzOfficePath & "program/tt_probe_file" )
    try
        iFile = FreeFile
        open cProbeFile for output as iFile
        close( iFile )
        kill( cProbeFile )
        hCheckForAdministratorPermissions() = true
        printlog( "Current user has administrator permissions" )
    catch
        hCheckForAdministratorPermissions() = false
        printlog( "Current user does not have administrator permissions" )
    endcatch

end function
