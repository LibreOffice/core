/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: glshell.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:01:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
protected:
    virtual BOOL Save();

public:

    using SotObject::GetInterface;

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
protected:
    virtual BOOL Save();

public:

    using SotObject::GetInterface;

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





















