/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _GLSHELL_HXX
#define _GLSHELL_HXX
#include "wdocsh.hxx"

class SwGlosDocShell : public SwDocShell
{
    String          aLongName;
    String          aShortName;
    String          aGroupName;

    using SotObject::GetInterface;

protected:
    virtual sal_Bool Save();

public:

    TYPEINFO();
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

    TYPEINFO();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
