/*************************************************************************
 *
 *  $RCSfile: richtextengine.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-07 16:12:42 $
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

#ifndef FORMS_SOURCE_RICHTEXT_RICHTEXTENGINE_HXX
#include "richtextengine.hxx"
#endif

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif
#ifndef _EDITOBJ_HXX
#include <svx/editobj.hxx>
#endif
#define ITEMID_FONTHEIGHT   EE_CHAR_FONTHEIGHT
#ifndef _SVX_FHGTITEM_HXX
#include <svx/fhgtitem.hxx>
#endif
#define ITEMID_FONT         EE_CHAR_FONTHEIGHT
#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
#endif
#define ITEMID_LANGUAGE     EE_CHAR_LANGUAGE
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VCL_MAPUNIT_HXX
#include <vcl/mapunit.hxx>
#endif
#ifndef _SV_MAPMOD_HXX
#include <vcl/mapmod.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif

#include <algorithm>
#include <functional>

//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= RichTextEngine
    //====================================================================
    //--------------------------------------------------------------------
    RichTextEngine* RichTextEngine::Create()
    {
        SfxItemPool* pPool = EditEngine::CreatePool();
        pPool->FreezeIdRanges();

        RichTextEngine* pReturn = new RichTextEngine( pPool );
        OutputDevice* pOutputDevice = pReturn->GetRefDevice();
        MapMode aDeviceMapMode( pOutputDevice->GetMapMode() );

        pReturn->SetStatusEventHdl( LINK( pReturn, RichTextEngine, EditEngineStatusChanged ) );

        pPool->SetDefaultMetric( static_cast< SfxMapUnit >( aDeviceMapMode.GetMapUnit() ) );

        // defaults
        Font aFont = Application::GetSettings().GetStyleSettings().GetAppFont();
        aFont.SetName( String( RTL_CONSTASCII_USTRINGPARAM( "Times New Roman" ) ) );
        pPool->SetPoolDefaultItem( SvxFontItem( aFont.GetFamily(), aFont.GetName(), String(), aFont.GetPitch(), aFont.GetCharSet(), EE_CHAR_FONTINFO ) );

        // 12 pt font size
        MapMode aPointMapMode( MAP_POINT );
        Size a12PointSize( OutputDevice::LogicToLogic( Size( 12, 0 ), aPointMapMode, aDeviceMapMode ) );
        pPool->SetPoolDefaultItem( SvxFontHeightItem( a12PointSize.Width(), 100, EE_CHAR_FONTHEIGHT ) );

        // font languages
        SvtLinguOptions aLinguOpt;
        pPool->SetPoolDefaultItem( SvxLanguageItem( aLinguOpt.nDefaultLanguage, EE_CHAR_LANGUAGE ) );
        pPool->SetPoolDefaultItem( SvxLanguageItem( aLinguOpt.nDefaultLanguage_CJK, EE_CHAR_LANGUAGE_CJK ) );
        pPool->SetPoolDefaultItem( SvxLanguageItem( aLinguOpt.nDefaultLanguage_CTL, EE_CHAR_LANGUAGE_CTL ) );

        return pReturn;
    }

    //--------------------------------------------------------------------
    RichTextEngine* RichTextEngine::Clone()
    {
        EditTextObject* pMyText = CreateTextObject();
        OSL_ENSURE( pMyText, "RichTextEngine::Clone: CreateTextObject returned nonsense!" );

        RichTextEngine* pClone = Create();
        pClone->SetText( *pMyText );
        delete pMyText;

        return pClone;
    }

    //--------------------------------------------------------------------
    RichTextEngine::RichTextEngine( SfxItemPool* _pPool )
        :EditEngine( _pPool )
        ,m_pEnginePool( _pPool )
    {
    }

    //--------------------------------------------------------------------
    RichTextEngine::~RichTextEngine( )
    {
        delete m_pEnginePool;
    }

    //--------------------------------------------------------------------
    void RichTextEngine::registerEngineStatusListener( IEngineStatusListener* _pListener )
    {
        OSL_ENSURE( _pListener, "RichTextEngine::registerEngineStatusListener: invalid listener!" );
        if ( _pListener )
            m_aStatusListeners.push_back( _pListener );
    }

    //--------------------------------------------------------------------
    void RichTextEngine::revokeEngineStatusListener( IEngineStatusListener* _pListener )
    {
        ::std::vector< IEngineStatusListener* >::iterator aPos = ::std::find_if(
            m_aStatusListeners.begin(),
            m_aStatusListeners.end(),
            ::std::bind2nd( ::std::equal_to< IEngineStatusListener* >( ), _pListener )
        );
        OSL_ENSURE( aPos != m_aStatusListeners.end(), "RichTextEngine::revokeEngineStatusListener: listener not registered!" );
        if ( aPos != m_aStatusListeners.end() )
            m_aStatusListeners.erase( aPos );
    }

    //--------------------------------------------------------------------
    IMPL_LINK( RichTextEngine, EditEngineStatusChanged, EditStatus*, _pStatus )
    {
        for ( ::std::vector< IEngineStatusListener* >::const_iterator aLoop = m_aStatusListeners.begin();
              aLoop != m_aStatusListeners.end();
              ++aLoop
            )
            (*aLoop)->EditEngineStatusChanged( *_pStatus );
        return 0L;
    }

//........................................................................
}   // namespace frm
//........................................................................

