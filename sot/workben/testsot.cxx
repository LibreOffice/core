/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sot.hxx"

#include <sot/storage.hxx>

/*
 * main.
 */
int main (int argc, sal_Char **argv)
{
     SotStorageRef xStor = new SotStorage( "c:\\temp\\65254.ppt" );
/*
    SotStorageRef xStor = new SotStorage( "c:\\temp\\video.sdb" );
    SotStorageRef xStor = new SotStorage( "c:\\temp\\video.sdb" );
    SotStorageRef x2Stor = xStor->OpenSotStorage( "1117" );

    SotStorageStreamRef xStm = x2Stor->OpenSotStream( "Genres", STREAM_STD_READWRITE | STREAM_TRUNC);
    //BYTE szData[100];
    //xStm->Write( szData, 100 );
    UINT32 nSize = xStm->GetSize();
    xStm->SetSize( 0 );
    nSize = xStm->GetSize();
    x2Stor->Commit();
    xStor->Commit();
*/
    return 0;
}

