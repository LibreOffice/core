/*************************************************************************
 *
 *  $RCSfile: grdocsh.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2004-01-20 10:54:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#define ITEMID_SEARCH                   SID_SEARCH_ITEM
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif

#ifndef _GLOBNAME_HXX //autogen
#include <tools/globname.hxx>
#endif

#ifndef _SO_CLSIDS_HXX //autogen
#include <sot/clsids.hxx>
#endif

#include <sfx2/fcontnr.hxx>

#pragma hdrstop

#include "app.hrc"
#include "strings.hrc"
#include "glob.hrc"

#ifndef SD_GRAPHIC_DOC_SHELL_HXX
#include "GraphicDocShell.hxx"
#endif
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#include "drawdoc.hxx"
#include "sdresid.hxx"

using namespace sd;
#define GraphicDocShell
#include "sdgslots.hxx"

namespace sd {

/*************************************************************************
|*
|* SFX-Slotmaps und -Definitionen
|*
\************************************************************************/
TYPEINIT1(GraphicDocShell, DrawDocShell);



SFX_IMPL_INTERFACE(GraphicDocShell, SfxObjectShell, SdResId(0))
{
    SFX_CHILDWINDOW_REGISTRATION(SID_SEARCH_DLG);
}

//SFX_IMPL_OBJECTFACTORY( GraphicDocShell, SFXOBJECTSHELL_STD_NORMAL, sdraw, SvGlobalName(SO3_SDRAW_CLASSID_60) )
SotFactory* GraphicDocShell::pFactory = NULL;
SotFactory * GraphicDocShell::ClassFactory()
{
    SotFactory **ppFactory = GetFactoryAdress();
    if( !*ppFactory )
    {
        *ppFactory = new SfxObjectFactory( SvGlobalName(SO3_SDRAW_CLASSID_60),
            String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "GraphicDocShell" ) ),
                                GraphicDocShell::CreateInstance );
        (*ppFactory)->PutSuperClass( SfxInPlaceObject::ClassFactory() );
    }
    return *ppFactory;
}

void * GraphicDocShell::CreateInstance( SotObject ** ppObj )
{
    GraphicDocShell * p = new GraphicDocShell();
    SfxInPlaceObject* pSfxInPlaceObject = p;
    SotObject* pBasicObj = pSfxInPlaceObject;
    if( ppObj )
        *ppObj = pBasicObj;
    return p;
}
const SotFactory * GraphicDocShell::GetSvFactory() const
{
    return ClassFactory();
}
void * GraphicDocShell::Cast( const SotFactory * pFact )
{
    void * pRet = NULL;
    if( !pFact || pFact == ClassFactory() )
        pRet = this;
    if( !pRet )
        pRet = SfxInPlaceObject::Cast( pFact );
    return pRet;
}

SfxObjectFactory* GraphicDocShell::pObjectFactory = 0;

SfxObjectShell* GraphicDocShell::CreateObject(SfxObjectCreateMode eMode)
{
    SfxObjectShell* pDoc = new GraphicDocShell(eMode);
    return pDoc;
}
SfxObjectFactory& GraphicDocShell::GetFactory() const
{
    return Factory();
}
void GraphicDocShell::RegisterFactory( USHORT nPrio )
{
    Factory().Construct(
        nPrio,
        &GraphicDocShell::CreateObject, SFXOBJECTSHELL_STD_NORMAL | SFXOBJECTSHELL_HASMENU,
        "sdraw" );
    Factory().RegisterInitFactory( &InitFactory );
    Factory().Register();
}
BOOL GraphicDocShell::DoInitNew( SvStorage *pStor )
{ return SfxObjectShell::DoInitNew(pStor); }

BOOL GraphicDocShell::DoClose()
{ return SfxInPlaceObject::DoClose(); }

BOOL GraphicDocShell::Close()
{   SvObjectRef aRef(this);
    SfxInPlaceObject::Close();
    return SfxObjectShell::Close(); }

void GraphicDocShell::ModifyChanged()
{ SfxObjectShell::ModifyChanged(); }

void GraphicDocShell::InitFactory()
{
    GraphicDocShell::Factory().SetDocumentServiceName( String( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.DrawingDocument" ) ) );
    //GraphicDocShell::Factory().GetFilterContainer()->SetDetectFilter( &SdDLL::DetectFilter );
    GraphicDocShell::Factory().RegisterMenuBar( SdResId( RID_GRAPHIC_DEFAULTMENU ) );
    GraphicDocShell::Factory().RegisterPluginMenuBar( SdResId( RID_GRAPHIC_PORTALMENU ) );
    GraphicDocShell::Factory().RegisterAccel( SdResId( RID_GRAPHIC_DEFAULTACCEL ) );
}


/*************************************************************************
|*
|* Konstruktor 1
|*
\************************************************************************/

GraphicDocShell::GraphicDocShell(SfxObjectCreateMode eMode,
                                     BOOL bDataObject,
                                     DocumentType eDocType) :
    DrawDocShell(eMode, bDataObject, eDocType)
{
    SetStyleFamily( 2 ); //CL: eigentlich SFX_STYLE_FAMILY_PARA, aber der stylist ist sch....
}

/*************************************************************************
|*
|* Konstruktor 2
|*
\************************************************************************/

GraphicDocShell::GraphicDocShell(SdDrawDocument* pDoc, SfxObjectCreateMode eMode,
                                     BOOL bDataObject,
                                     DocumentType eDocType) :
    DrawDocShell(pDoc, eMode, bDataObject, eDocType)
{
    SetStyleFamily( 2 ); //CL: eigentlich SFX_STYLE_FAMILY_PARA, aber der stylist ist sch....
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

GraphicDocShell::~GraphicDocShell()
{
}


} // end of namespace sd
