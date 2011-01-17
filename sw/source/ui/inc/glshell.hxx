/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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





















