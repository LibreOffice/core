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



#ifndef _SD_IMAPINFO_HXX
#define _SD_IMAPINFO_HXX

#include "sdiocmpt.hxx"
#include "glob.hxx"
#include <svx/svdobj.hxx>
#include <svtools/imap.hxx>


/*************************************************************************
|*
|*
|*
\************************************************************************/

class SdIMapInfo : public SdrObjUserData, public SfxListener
{

    ImageMap        aImageMap;

public:
                    SdIMapInfo() :
                        SdrObjUserData( SdUDInventor, SD_IMAPINFO_ID ) {};

                    SdIMapInfo( const ImageMap& rImageMap ) :
                        SdrObjUserData( SdUDInventor, SD_IMAPINFO_ID ),
                        aImageMap( rImageMap ) {};

                    SdIMapInfo( const SdIMapInfo& rIMapInfo ) :
                        SdrObjUserData( SdUDInventor, SD_IMAPINFO_ID ),
                        SfxListener(),
                        aImageMap( rIMapInfo.aImageMap ) {};

    virtual         ~SdIMapInfo() {};

    virtual SdrObjUserData* Clone( SdrObject* ) const { return new SdIMapInfo( *this ); }

    void            SetImageMap( const ImageMap& rIMap ) { aImageMap = rIMap; }
    const ImageMap& GetImageMap() const { return aImageMap; }
};

#endif      // _SD_IMAPINFO_HXX


