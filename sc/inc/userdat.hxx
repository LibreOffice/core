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

#ifndef SC_USERDAT_HXX
#define SC_USERDAT_HXX

#include <svx/svdobj.hxx>
#include <svtools/imap.hxx>
#include "global.hxx"
#include "address.hxx"

//-------------------------------------------------------------------------
class SdrObjFactory;

//-------------------------------------------------------------------------

#define SC_DRAWLAYER 0x30334353     // Inventor: "SC30"

// Object-Ids fuer UserData
#define SC_UD_OBJDATA       1
#define SC_UD_IMAPDATA      2
#define SC_UD_MACRODATA     3

//-------------------------------------------------------------------------

class ScDrawObjFactory
{
private:
    DECL_LINK( MakeUserData, SdrObjFactory * );

public:
    ScDrawObjFactory();
   ~ScDrawObjFactory();
};

//-------------------------------------------------------------------------

class ScDrawObjData : public SdrObjUserData
{
public:
    ScAddress           maStart;
    ScAddress           maEnd;
    bool                mbNote;

    explicit            ScDrawObjData();

private:
    virtual ScDrawObjData* Clone( SdrObject* pObj ) const;
};

//-------------------------------------------------------------------------

class ScIMapInfo : public SdrObjUserData
{
    ImageMap        aImageMap;

public:
                    ScIMapInfo();
                    ScIMapInfo( const ImageMap& rImageMap );
                    ScIMapInfo( const ScIMapInfo& rIMapInfo );
    virtual         ~ScIMapInfo();

    virtual SdrObjUserData* Clone( SdrObject* pObj ) const;

    void    SetImageMap( const ImageMap& rIMap )    { aImageMap = rIMap; }
    const ImageMap& GetImageMap() const             { return aImageMap; }
};

//-------------------------------------------------------------------------

class ScMacroInfo : public SdrObjUserData
{
public:
                    ScMacroInfo();
    virtual         ~ScMacroInfo();

    virtual SdrObjUserData* Clone( SdrObject* pObj ) const;

    void            SetMacro( const rtl::OUString& rMacro ) { maMacro = rMacro; }
    const rtl::OUString& GetMacro() const { return maMacro; }

#ifdef ISSUE66550_HLINK_FOR_SHAPES
    void            SetHlink( const rtl::OUString& rHlink ) { maHlink = rHlink; }
    const rtl::OUString& GetHlink() const { return maHlink; }
#endif

private:
    rtl::OUString   maMacro;
#ifdef ISSUE66550_HLINK_FOR_SHAPES
    rtl::OUString   maHlink;
#endif
};

//-------------------------------------------------------------------------

#endif


