/*************************************************************************
 *
 *  $RCSfile: sourceviewconfig.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-25 17:25:12 $
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

#ifndef _SVTOOLS_SOURCEVIEWCONFIG_HXX
#include <sourceviewconfig.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX
#include "smplhint.hxx"
#endif

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;
namespace svt
{
class SourceViewConfig_Impl : public utl::ConfigItem, public SfxBroadcaster
{
private:
    OUString        m_sFontName;
    sal_Int16       m_nFontHeight;
    sal_Bool        m_bProportionalFontOnly;

    void        Load();

    static Sequence< OUString > GetPropertyNames();

public:
    SourceViewConfig_Impl();
    ~SourceViewConfig_Impl();

    virtual void    Notify( const Sequence< rtl::OUString >& aPropertyNames );
    virtual void    Commit();

    const rtl::OUString&    GetFontName() const
                                {return m_sFontName;}
    void                    SetFontName(const rtl::OUString& rName)
                                {
                                    if(rName != m_sFontName)
                                    {
                                        m_sFontName = rName;
                                        SetModified();
                                    }
                                }

    sal_Int16               GetFontHeight() const
                                {return m_nFontHeight;}
    void                    SetFontHeight(sal_Int16 nHeight)
                                {
                                    if(m_nFontHeight != nHeight)
                                    {
                                        m_nFontHeight = nHeight;
                                        SetModified();
                                    }
                                }

    sal_Bool                IsShowProportionalFontsOnly() const
                                {return m_bProportionalFontOnly;}
    void                    SetShowProportionalFontsOnly(sal_Bool bSet)
                                {
                                    if(m_bProportionalFontOnly != bSet)
                                    {
                                        m_bProportionalFontOnly = bSet;
                                        SetModified();
                                    }
                                }
};
// initialization of static members --------------------------------------
SourceViewConfig_Impl* SourceViewConfig::m_pImplConfig = 0;
sal_Int32              SourceViewConfig::m_nRefCount = 0;
namespace { struct lclMutex : public rtl::Static< ::osl::Mutex, lclMutex > {}; }
/* -----------------------------28.08.2002 16:45------------------------------

 ---------------------------------------------------------------------------*/
SourceViewConfig_Impl::SourceViewConfig_Impl() :
    ConfigItem(OUString::createFromAscii("Office.Common/Font/SourceViewFont")),
    m_nFontHeight(12),
    m_bProportionalFontOnly(sal_False)
{
    Load();
}
/* -----------------------------28.08.2002 16:45------------------------------

 ---------------------------------------------------------------------------*/
SourceViewConfig_Impl::~SourceViewConfig_Impl()
{
}
/* -----------------------------28.08.2002 16:25------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SourceViewConfig_Impl::GetPropertyNames()
{
    //this list needs exactly to mach the enum PropertyNameIndex
    static const char* aPropNames[] =
    {
        "FontName"                  // 0
        ,"FontHeight"               // 1
        ,"NonProportionalFontsOnly" // 2
    };
    const int nCount = sizeof( aPropNames ) / sizeof( const char* );
    Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    for ( int i = 0; i < nCount; i++ )
        pNames[i] = OUString::createFromAscii( aPropNames[i] );

    return aNames;
}

/*-- 28.08.2002 16:37:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SourceViewConfig_Impl::Load()
{
    Sequence< OUString > aNames = GetPropertyNames();
    Sequence< Any > aValues = GetProperties( aNames );
    EnableNotification( aNames );
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT( aValues.getLength() == aNames.getLength(), "GetProperties failed" );
    if ( aValues.getLength() == aNames.getLength() )
    {
        for ( int nProp = 0; nProp < aNames.getLength(); nProp++ )
        {
            if ( pValues[nProp].hasValue() )
            {
                switch( nProp )
                {
                    case 0:  pValues[nProp] >>= m_sFontName;         break;
                    case 1:  pValues[nProp] >>= m_nFontHeight;      break;
                    case 2:  pValues[nProp] >>= m_bProportionalFontOnly;     break;
                }
            }
        }
    }
}
/*-- 28.08.2002 16:38:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void SourceViewConfig_Impl::Notify( const Sequence< OUString >& aPropertyNames )
{
    Load();
}
/*-- 28.08.2002 16:38:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void SourceViewConfig_Impl::Commit()
{
    ClearModified();
    Sequence< OUString > aNames = GetPropertyNames();
    OUString* pNames = aNames.getArray();
    Sequence< Any > aValues( aNames.getLength() );
    Any* pValues = aValues.getArray();
    for ( int nProp = 0; nProp < aNames.getLength(); nProp++ )
    {
        switch( nProp )
        {
            case 0:  pValues[nProp] <<= m_sFontName;         break;
            case 1:  pValues[nProp] <<= m_nFontHeight;      break;
            case 2:  pValues[nProp] <<= m_bProportionalFontOnly;     break;
            default:
                DBG_ERRORFILE( "invalid index to save a user token" );
        }
    }
    PutProperties( aNames, aValues );

    //notify SfxListener
    {
        SfxSimpleHint aHint = SfxSimpleHint( SFX_HINT_DATACHANGED );
        Broadcast(aHint);
    }
}
/*-- 28.08.2002 16:32:19---------------------------------------------------

  -----------------------------------------------------------------------*/
SourceViewConfig::SourceViewConfig()
{
    {
        ::osl::MutexGuard aGuard( lclMutex::get() );
        if(!m_pImplConfig)
            m_pImplConfig = new SourceViewConfig_Impl;
         ++m_nRefCount;
    }
    StartListening( *m_pImplConfig, TRUE );
}
/*-- 28.08.2002 16:32:19---------------------------------------------------

  -----------------------------------------------------------------------*/
SourceViewConfig::~SourceViewConfig()
{
    EndListening( *m_pImplConfig, TRUE );
    ::osl::MutexGuard aGuard( lclMutex::get() );
    if( !--m_nRefCount )
    {
        if( m_pImplConfig->IsModified() )
            m_pImplConfig->Commit();
        DELETEZ( m_pImplConfig );
    }
}
/*-- 28.08.2002 16:32:19---------------------------------------------------

  -----------------------------------------------------------------------*/
const OUString&  SourceViewConfig::GetFontName() const
{
    return m_pImplConfig->GetFontName();
}
/*-- 28.08.2002 16:32:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void SourceViewConfig::SetFontName(const OUString& rName)
{
    m_pImplConfig->SetFontName(rName);
}
/*-- 28.08.2002 16:32:20---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int16 SourceViewConfig::GetFontHeight() const
{
    return m_pImplConfig->GetFontHeight();
}
/*-- 28.08.2002 16:32:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void SourceViewConfig::SetFontHeight(sal_Int16 nHeight)
{
    m_pImplConfig->SetFontHeight(nHeight);
}
/*-- 28.08.2002 16:32:20---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SourceViewConfig::IsShowProportionalFontsOnly() const
{
    return m_pImplConfig->IsShowProportionalFontsOnly();
}
/*-- 28.08.2002 16:32:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void SourceViewConfig::SetShowProportionalFontsOnly(sal_Bool bSet)
{
    m_pImplConfig->SetShowProportionalFontsOnly(bSet);
}
/* -----------------------------30.08.2002 10:40------------------------------

 ---------------------------------------------------------------------------*/
void SourceViewConfig::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    Broadcast( rHint );
}
}//namespace svt
