/*************************************************************************
 *
 *  $RCSfile: accessibilityoptions.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: os $ $Date: 2002-08-28 12:19:42 $
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

#pragma hdrstop

#ifndef INCLUDED_SVTOOLS_ACCESSIBILITYOPTIONS_HXX
#include "accessibilityoptions.hxx"
#endif

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX
#include "smplhint.hxx"
#endif

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

// class SvtAccessibilityOptions_Impl ---------------------------------------------

class SvtAccessibilityOptions_Impl : public utl::ConfigItem, public SfxBroadcaster
{
private:
    sal_Bool        m_bForDrawings;
    sal_Bool        m_bForBorders;
    sal_Bool        m_bForPagePreviews;
    sal_Bool        m_bHelpTipsDisappear;
    sal_Bool        m_bAllowAnimatedGraphics;
    sal_Bool        m_bAllowAnimatedText;
    sal_Bool        m_bAutomaticFontColor;
    sal_Bool        m_bSystemFont;
    sal_Int16       m_nHelpTipSeconds;
    sal_Bool        m_bTextSelectionInReadonly;

    typedef sal_Bool SvtAccessibilityOptions_Impl:: *BoolPtr;

    sal_Bool    GetToken( BoolPtr pPtr ) const;
    void        SetToken( BoolPtr pPtr, sal_Bool bSet );
    void        Load();

    //this list needs exactly to mach the listet properties in GetPropertyNames
    enum PropertyNameIndex { DRAWINGS=0
                            , BORDERS
                            , PAGEPREVIEWS
                            , HELPTIPSDISAPPEAR
                            , HELPTIPSECONDS
                            , ALLOWANIMATEDGRAPHICS
                            , ALLOWANIMATEDTEXT
                            , AUTOMATICFONTCOLOR
                            , SYSTEMFONT
                            , TEXTSELECTION
                        };

    static Sequence< OUString > GetPropertyNames();

public:
    SvtAccessibilityOptions_Impl();

    virtual void    Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    virtual void    Commit();

    sal_Bool    GetIsForDrawings() const
                    {return GetToken( &SvtAccessibilityOptions_Impl::m_bForDrawings );}
    sal_Bool    GetIsForBorders() const
                    {return GetToken( &SvtAccessibilityOptions_Impl::m_bForBorders );}
    sal_Bool    GetIsForPagePreviews() const
                    {return GetToken( &SvtAccessibilityOptions_Impl::m_bForPagePreviews );}
    sal_Bool    GetIsHelpTipsDisappear() const
                    {return GetToken( &SvtAccessibilityOptions_Impl::m_bHelpTipsDisappear );}
    sal_Bool    GetIsAllowAnimatedGraphics() const
                    {return GetToken( &SvtAccessibilityOptions_Impl::m_bAllowAnimatedGraphics );}
    sal_Bool    GetIsAllowAnimatedText() const
                    {return GetToken( &SvtAccessibilityOptions_Impl::m_bAllowAnimatedText );}
    sal_Bool    GetIsAutomaticFontColor() const
                    {return GetToken( &SvtAccessibilityOptions_Impl::m_bAutomaticFontColor );}
    sal_Bool    GetIsSystemFont() const
                    {return GetToken( &SvtAccessibilityOptions_Impl::m_bSystemFont );}
    sal_Int16   GetHelpTipSeconds() const
                    {return m_nHelpTipSeconds;}
    sal_Bool    IsSelectionInReadonly() const
                    {return m_bTextSelectionInReadonly;}

    void        SetIsForDrawings(sal_Bool bSet)
                    { SetToken( &SvtAccessibilityOptions_Impl::m_bForDrawings, bSet ); }
    void        SetIsForBorders(sal_Bool bSet)
                    { SetToken( &SvtAccessibilityOptions_Impl::m_bForBorders, bSet ); }
    void        SetIsForPagePreviews(sal_Bool bSet)
                    { SetToken( &SvtAccessibilityOptions_Impl::m_bForPagePreviews, bSet ); }
    void        SetIsHelpTipsDisappear(sal_Bool bSet)
                    { SetToken( &SvtAccessibilityOptions_Impl::m_bHelpTipsDisappear, bSet ); }
    void        SetIsAllowAnimatedGraphics(sal_Bool bSet)
                    { SetToken( &SvtAccessibilityOptions_Impl::m_bAllowAnimatedGraphics, bSet ); }
    void        SetIsAllowAnimatedText(sal_Bool bSet)
                    { SetToken( &SvtAccessibilityOptions_Impl::m_bAllowAnimatedText, bSet ); }
    void        SetIsAutomaticFontColor(sal_Bool bSet)
                    { SetToken( &SvtAccessibilityOptions_Impl::m_bAutomaticFontColor, bSet ); }
    void        SetIsSystemFont(sal_Bool bSet)
                    { SetToken( &SvtAccessibilityOptions_Impl::m_bSystemFont, bSet ); }
    void        SetHelpTipSeconds(sal_Int16 nSet)
                    {
                        if(m_nHelpTipSeconds!=nSet)
                        {
                            m_nHelpTipSeconds=nSet;
                            SetModified();
                        }
                    }
    void        SetSelectionInReadonly(sal_Bool bSet)
                    {SetToken( &SvtAccessibilityOptions_Impl::m_bTextSelectionInReadonly, bSet);}
};

// initialization of static members --------------------------------------

SvtAccessibilityOptions_Impl* volatile  SvtAccessibilityOptions::sm_pSingleImplConfig =NULL;
sal_Int32                     volatile  SvtAccessibilityOptions::sm_nAccessibilityRefCount(0);
::osl::Mutex                            SvtAccessibilityOptions::m_aSingletonMutex;

// functions -------------------------------------------------------------

Sequence< OUString > SvtAccessibilityOptions_Impl::GetPropertyNames()
{
    //this list needs exactly to mach the enum PropertyNameIndex
    static const char* aPropNames[] =
    {
        "IsForDrawings"             // DRAWINGS
        ,"IsForBorders"             // BORDERS
        ,"IsForPagePreviews"        // PAGEPREVIEWS
        ,"IsHelpTipsDisappear"      // HELPTIPSDISAPPEAR
        ,"HelpTipSeconds"           // HELPTIPSECONDS
        ,"IsAllowAnimatedGraphics"  // ALLOWANIMATEDGRAPHICS
        ,"IsAllowAnimatedText"      // ALLOWANIMATEDTEXT
        ,"IsAutomaticFontColor"     // AUTOMATICFONTCOLOR
        ,"IsSystemFont"             // SYSTEMFONT
        ,"IsSelectionInReadonly"    // TEXTSELECTION
    };
    const int nCount = sizeof( aPropNames ) / sizeof( const char* );
    Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    for ( int i = 0; i < nCount; i++ )
        pNames[i] = OUString::createFromAscii( aPropNames[i] );

    return aNames;
}

// -----------------------------------------------------------------------
// class SvtAccessibilityOptions_Impl ---------------------------------------------

SvtAccessibilityOptions_Impl::SvtAccessibilityOptions_Impl() :
    ConfigItem( OUString::createFromAscii("Office.Common/Accessibility") )
{
    Load();
}
// -----------------------------------------------------------------------
void SvtAccessibilityOptions_Impl::Load()
{
    Sequence< OUString > aNames = GetPropertyNames();
    Sequence< Any > aValues = GetProperties( aNames );
    EnableNotification( aNames );
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT( aValues.getLength() == aNames.getLength(), "GetProperties failed" );
    if ( aValues.getLength() == aNames.getLength() )
    {
        sal_Bool bTemp;

        for ( int nProp = 0; nProp < aNames.getLength(); nProp++ )
        {
            if ( pValues[nProp].hasValue() )
            {
                if ( pValues[nProp] >>= bTemp )
                {
                    switch ( PropertyNameIndex(nProp) )
                    {
                        case DRAWINGS:              m_bForDrawings = bTemp;         break;
                        case BORDERS:               m_bForBorders = bTemp;         break;
                        case PAGEPREVIEWS:          m_bForPagePreviews = bTemp;     break;
                        case HELPTIPSDISAPPEAR:     m_bHelpTipsDisappear = bTemp;   break;
                        case ALLOWANIMATEDGRAPHICS: m_bAllowAnimatedGraphics = bTemp; break;
                        case ALLOWANIMATEDTEXT:     m_bAllowAnimatedText = bTemp;   break;
                        case AUTOMATICFONTCOLOR:    m_bAutomaticFontColor = bTemp;  break;
                        case SYSTEMFONT:            m_bSystemFont = bTemp;          break;
                        case TEXTSELECTION:         m_bTextSelectionInReadonly = bTemp; break;
                        default:
                            DBG_ERRORFILE( "invalid index to load a user token" );
                    }
                }
                else
                {
                    sal_Int16 nTemp;
                    if( pValues[nProp] >>= nTemp )
                    {
                        if(PropertyNameIndex(nProp)==HELPTIPSECONDS)//this is an integer and not a bool
                            m_nHelpTipSeconds=nTemp;
                        else
                        {
                            DBG_ERRORFILE( "sal_Int16 any type not matched with property name" );
                        }
                    }
                    else
                    {
                        DBG_ERRORFILE( "Wrong any type" );
                    }
                }
            }
        }
    }
}
// -----------------------------------------------------------------------

void SvtAccessibilityOptions_Impl::Commit()
{
    ClearModified();
    Sequence< OUString > aNames = GetPropertyNames();
    OUString* pNames = aNames.getArray();
    Sequence< Any > aValues( aNames.getLength() );
    Any* pValues = aValues.getArray();
    sal_Bool bTemp=false;
    for ( int nProp = 0; nProp < aNames.getLength(); nProp++ )
    {
        switch ( PropertyNameIndex(nProp) )
        {
            case DRAWINGS:              bTemp = m_bForDrawings;         break;
            case BORDERS:               bTemp = m_bForBorders;         break;
            case PAGEPREVIEWS:          bTemp = m_bForPagePreviews;     break;
            case HELPTIPSDISAPPEAR:     bTemp = m_bHelpTipsDisappear;   break;
            case HELPTIPSECONDS:        pValues[nProp] <<= m_nHelpTipSeconds; continue;//this is an integer and not a bool
            case ALLOWANIMATEDGRAPHICS: bTemp = m_bAllowAnimatedGraphics; break;
            case ALLOWANIMATEDTEXT:     bTemp = m_bAllowAnimatedText;   break;
            case AUTOMATICFONTCOLOR:    bTemp = m_bAutomaticFontColor;  break;
            case SYSTEMFONT:            bTemp = m_bSystemFont;          break;
            case TEXTSELECTION:         bTemp = m_bTextSelectionInReadonly; break;
            default:
                DBG_ERRORFILE( "invalid index to save a user token" );
        }
        pValues[nProp] <<= bTemp;
    }
    PutProperties( aNames, aValues );

    //notify SfxListener
    {
        SfxSimpleHint aHint = SfxSimpleHint( SFX_HINT_ACCESSIBILITY_CHANGED );
        Broadcast(aHint);
    }
}

// -----------------------------------------------------------------------

void SvtAccessibilityOptions_Impl::Notify( const Sequence<rtl::OUString>& aPropertyNames )
{
    Load();
}

// -----------------------------------------------------------------------

sal_Bool SvtAccessibilityOptions_Impl::GetToken( BoolPtr pPtr ) const
{
    return this->*pPtr;
}

void SvtAccessibilityOptions_Impl::SetToken( BoolPtr pPtr, sal_Bool bSet )
{
    if(this->*pPtr != bSet)
    {
        this->*pPtr = bSet;
        SetModified();
    }
}

// -----------------------------------------------------------------------
// class SvtAccessibilityOptions --------------------------------------------------

SvtAccessibilityOptions::SvtAccessibilityOptions()
{
    {
        ::osl::MutexGuard aGuard( m_aSingletonMutex );
        if(!sm_pSingleImplConfig)
            sm_pSingleImplConfig = new SvtAccessibilityOptions_Impl;
         ++sm_nAccessibilityRefCount;
    }
    StartListening( *sm_pSingleImplConfig, TRUE );
}

// -----------------------------------------------------------------------

SvtAccessibilityOptions::~SvtAccessibilityOptions()
{
    EndListening( *sm_pSingleImplConfig, TRUE );
    ::osl::MutexGuard aGuard( m_aSingletonMutex );
    if( !--sm_nAccessibilityRefCount )
    {
        if( sm_pSingleImplConfig->IsModified() )
            sm_pSingleImplConfig->Commit();
        DELETEZ( sm_pSingleImplConfig );
    }
}

// -----------------------------------------------------------------------

void SvtAccessibilityOptions::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    Broadcast( rHint );
}

// -----------------------------------------------------------------------

sal_Bool SvtAccessibilityOptions::IsModified() const
{
    return sm_pSingleImplConfig->IsModified();
}
void SvtAccessibilityOptions::Commit()
{
    sm_pSingleImplConfig->Commit();
}

// -----------------------------------------------------------------------

sal_Bool SvtAccessibilityOptions::GetIsForDrawings() const
{
    return sm_pSingleImplConfig->GetIsForDrawings();
}
sal_Bool SvtAccessibilityOptions::GetIsForBorders() const
{
    return sm_pSingleImplConfig->GetIsForBorders();
}
sal_Bool SvtAccessibilityOptions::GetIsForPagePreviews() const
{
    return sm_pSingleImplConfig->GetIsForPagePreviews();
}
sal_Bool SvtAccessibilityOptions::GetIsHelpTipsDisappear() const
{
    return sm_pSingleImplConfig->GetIsHelpTipsDisappear();
}
sal_Bool SvtAccessibilityOptions::GetIsAllowAnimatedGraphics() const
{
    return sm_pSingleImplConfig->GetIsAllowAnimatedGraphics();
}
sal_Bool SvtAccessibilityOptions::GetIsAllowAnimatedText() const
{
    return sm_pSingleImplConfig->GetIsAllowAnimatedText();
}
sal_Bool SvtAccessibilityOptions::GetIsAutomaticFontColor() const
{
    return sm_pSingleImplConfig->GetIsAutomaticFontColor();
}
sal_Bool SvtAccessibilityOptions::GetIsSystemFont() const
{
    return sm_pSingleImplConfig->GetIsSystemFont();
}
sal_Int16 SvtAccessibilityOptions::GetHelpTipSeconds() const
{
    return sm_pSingleImplConfig->GetHelpTipSeconds();
}
sal_Bool SvtAccessibilityOptions::IsSelectionInReadonly() const
{
    return sm_pSingleImplConfig->IsSelectionInReadonly();
}

// -----------------------------------------------------------------------

void SvtAccessibilityOptions::SetIsForDrawings(sal_Bool bSet)
{
    sm_pSingleImplConfig->SetIsForDrawings(bSet);
}
void SvtAccessibilityOptions::SetIsForBorders(sal_Bool bSet)
{
    sm_pSingleImplConfig->SetIsForBorders(bSet);
}
void SvtAccessibilityOptions::SetIsForPagePreviews(sal_Bool bSet)
{
    sm_pSingleImplConfig->SetIsForPagePreviews(bSet);
}
void SvtAccessibilityOptions::SetIsHelpTipsDisappear(sal_Bool bSet)
{
    sm_pSingleImplConfig->SetIsHelpTipsDisappear(bSet);
}
void SvtAccessibilityOptions::SetIsAllowAnimatedGraphics(sal_Bool bSet)
{
    sm_pSingleImplConfig->SetIsAllowAnimatedGraphics(bSet);
}
void SvtAccessibilityOptions::SetIsAllowAnimatedText(sal_Bool bSet)
{
    sm_pSingleImplConfig->SetIsAllowAnimatedText(bSet);
}
void SvtAccessibilityOptions::SetIsAutomaticFontColor(sal_Bool bSet)
{
    sm_pSingleImplConfig->SetIsAutomaticFontColor(bSet);
}
void SvtAccessibilityOptions::SetIsSystemFont(sal_Bool bSet)
{
    sm_pSingleImplConfig->SetIsSystemFont(bSet);
}
void SvtAccessibilityOptions::SetHelpTipSeconds(sal_Int16 nSet)
{
    sm_pSingleImplConfig->SetHelpTipSeconds(nSet);
}
void SvtAccessibilityOptions::SetSelectionInReadonly(sal_Bool bSet)
{
    sm_pSingleImplConfig->SetSelectionInReadonly(bSet);
}

// -----------------------------------------------------------------------

