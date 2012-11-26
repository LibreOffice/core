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


#ifndef _AUTODOC_HXX
#define _AUTODOC_HXX
#include "wdocsh.hxx"

class SwGlosDocShell : public SwDocShell
{
    String          aLongName;
    String          aShortName;
    String          aGroupName;
    sal_Bool        bShow;

    using SotObject::GetInterface;

protected:
    virtual sal_Bool Save();

public:

    SFX_DECL_INTERFACE(SW_GLOSDOCSHELL)

    SwGlosDocShell( sal_Bool bNewShow = sal_True);
    virtual ~SwGlosDocShell();

    void            Execute( SfxRequest& );
    void            GetState( SfxItemSet& );
    void            SetLongName( const String& rLongName )
                        { aLongName = rLongName; }
    void            SetShortName( const String& rShortName )
                        { aShortName = rShortName; }
    void            SetGroupName( const String& rGroupName )
                        { aGroupName = rGroupName; }
    const String&   GetShortName(){return aShortName;}
};


class SwWebGlosDocShell : public SwWebDocShell
{
    String          aLongName;
    String          aShortName;
    String          aGroupName;

    using SotObject::GetInterface;

protected:
    virtual sal_Bool Save();

public:

    SFX_DECL_INTERFACE(SW_WEBGLOSDOCSHELL)

    SwWebGlosDocShell();
    virtual ~SwWebGlosDocShell();

    void            Execute( SfxRequest& );
    void            GetState( SfxItemSet& );
    void            SetLongName( const String& rLongName )
                        { aLongName = rLongName; }
    void            SetShortName( const String& rShortName )
                        { aShortName = rShortName; }
    void            SetGroupName( const String& rGroupName )
                        { aGroupName = rGroupName; }
    const String&   GetShortName(){return aShortName;}
};

#endif





















