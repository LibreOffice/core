/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basdoc.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:54:30 $
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

#ifndef _BASDOC_HXX
#define _BASDOC_HXX

#include <svx/ifaceids.hxx>
#include <iderid.hxx>

#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif

class SfxPrinter;

class BasicDocShell: public SfxObjectShell
{
    SfxPrinter*         pPrinter;

protected:
    virtual void        FillStatusBar( StatusBar& rBar);
    virtual void    Draw( OutputDevice *, const JobSetup & rSetup,
                          USHORT nAspect = ASPECT_CONTENT );
    virtual void    FillClass( SvGlobalName * pClassName,
                               sal_uInt32 * pFormat,
                               String * pAppName,
                               String * pFullTypeName,
                               String * pShortTypeName,
                               sal_Int32 nVersion ) const;

public:
                        TYPEINFO();

                        SFX_DECL_OBJECTFACTORY();
                        SFX_DECL_INTERFACE( SVX_INTERFACE_BASIDE_DOCSH );
                        BasicDocShell( SfxObjectCreateMode eMode = SFX_CREATE_MODE_STANDARD );
                        ~BasicDocShell();

    void                Execute( SfxRequest& rReq );
    void                GetState( SfxItemSet& rSet );

    SfxPrinter*         GetPrinter( BOOL bCreate );
    void                SetPrinter( SfxPrinter* pPrinter );
};

#endif  // _BASDOC_HXX
