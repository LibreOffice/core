/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optionsdrawinglayer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 15:40:01 $
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
#ifndef GCC
#pragma hdrstop
#endif

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#ifndef INCLUDED_SVTOOLS_OPTIONSDRAWINGLAYER_HXX
#include "optionsdrawinglayer.hxx"
#endif

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
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

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

using namespace ::utl                   ;
using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

#define ROOTNODE_START                  OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Common/Drawinglayer"   ))
#define DEFAULT_OVERLAYBUFFER           sal_True
#define DEFAULT_PAINTBUFFER             sal_True
#define DEFAULT_STRIPE_COLOR_A          0
#define DEFAULT_STRIPE_COLOR_B          16581375
#define DEFAULT_STRIPE_LENGTH           4

#define PROPERTYNAME_OVERLAYBUFFER      OUString(RTL_CONSTASCII_USTRINGPARAM("OverlayBuffer"    ))
#define PROPERTYNAME_PAINTBUFFER        OUString(RTL_CONSTASCII_USTRINGPARAM("PaintBuffer"      ))
#define PROPERTYNAME_STRIPE_COLOR_A     OUString(RTL_CONSTASCII_USTRINGPARAM("StripeColorA"     ))
#define PROPERTYNAME_STRIPE_COLOR_B     OUString(RTL_CONSTASCII_USTRINGPARAM("StripeColorB"     ))
#define PROPERTYNAME_STRIPE_LENGTH      OUString(RTL_CONSTASCII_USTRINGPARAM("StripeLength"     ))

#define PROPERTYHANDLE_OVERLAYBUFFER    0
#define PROPERTYHANDLE_PAINTBUFFER      1
#define PROPERTYHANDLE_STRIPE_COLOR_A   2
#define PROPERTYHANDLE_STRIPE_COLOR_B   3
#define PROPERTYHANDLE_STRIPE_LENGTH    4

#define PROPERTYCOUNT                   5

class SvtOptionsDrawinglayer_Impl : public ConfigItem
{
public:

//---------------------------------------------------------------------------------------------------------
//  constructor / destructor
//---------------------------------------------------------------------------------------------------------

     SvtOptionsDrawinglayer_Impl();
    ~SvtOptionsDrawinglayer_Impl();

//---------------------------------------------------------------------------------------------------------
//  overloaded methods of baseclass
//---------------------------------------------------------------------------------------------------------

    virtual void Commit();

//---------------------------------------------------------------------------------------------------------
//  public interface
//---------------------------------------------------------------------------------------------------------

    sal_Bool    IsOverlayBuffer() const;
    sal_Bool    IsPaintBuffer() const;
    Color       GetStripeColorA() const;
    Color       GetStripeColorB() const;
    sal_uInt16  GetStripeLength() const;

    void        SetOverlayBuffer( sal_Bool bState );
    void        SetPaintBuffer( sal_Bool bState );
    void        SetStripeColorA( Color aColor );
    void        SetStripeColorB( Color aColor );
    void        SetStripeLength( sal_uInt16 nLength );

//-------------------------------------------------------------------------------------------------------------
//  private methods
//-------------------------------------------------------------------------------------------------------------

private:

    static Sequence< OUString > impl_GetPropertyNames();

//-------------------------------------------------------------------------------------------------------------
//  private member
//-------------------------------------------------------------------------------------------------------------

private:

        sal_Bool    m_bOverlayBuffer;
        sal_Bool    m_bPaintBuffer;
        Color       m_bStripeColorA;
        Color       m_bStripeColorB;
        sal_uInt16  m_nStripeLength;
};

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl() :
    ConfigItem( ROOTNODE_START  ),
    m_bOverlayBuffer( DEFAULT_OVERLAYBUFFER ),
    m_bPaintBuffer( DEFAULT_PAINTBUFFER ),
    m_bStripeColorA(Color(DEFAULT_STRIPE_COLOR_A)),
    m_bStripeColorB(Color(DEFAULT_STRIPE_COLOR_B)),
    m_nStripeLength(DEFAULT_STRIPE_LENGTH)
{
    Sequence< OUString >    seqNames( impl_GetPropertyNames() );
    Sequence< Any >         seqValues   = GetProperties( seqNames ) ;

    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to ouer internal member.
    sal_Int32 nPropertyCount = seqValues.getLength();
    sal_Int32 nProperty = 0;

    for( nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        DBG_ASSERT( !(seqValues[nProperty].hasValue()==sal_False), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nInvalid property value for property detected!\n" );

        switch( nProperty )
        {
            case PROPERTYHANDLE_OVERLAYBUFFER:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\OverlayBuffer\"?" );
                seqValues[nProperty] >>= m_bOverlayBuffer;
            }
            break;

            case PROPERTYHANDLE_PAINTBUFFER:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\PaintBuffer\"?" );
                seqValues[nProperty] >>= m_bPaintBuffer;
            }
            break;

            case PROPERTYHANDLE_STRIPE_COLOR_A:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\StripeColorA\"?" );
                sal_Int32 nValue;
                seqValues[nProperty] >>= nValue;
                m_bStripeColorA = nValue;
            }
            break;

            case PROPERTYHANDLE_STRIPE_COLOR_B:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\StripeColorB\"?" );
                sal_Int32 nValue;
                seqValues[nProperty] >>= nValue;
                m_bStripeColorB = nValue;
            }
            break;

            case PROPERTYHANDLE_STRIPE_LENGTH:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_SHORT), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\StripeLength\"?" );
                seqValues[nProperty] >>= m_nStripeLength;
            }
            break;
        }
    }
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtOptionsDrawinglayer_Impl::~SvtOptionsDrawinglayer_Impl()
{
    if( IsModified() )
        Commit();
}

//*****************************************************************************************************************
//  Commit
//*****************************************************************************************************************
void SvtOptionsDrawinglayer_Impl::Commit()
{
    Sequence< OUString >    aSeqNames( impl_GetPropertyNames() );
    Sequence< Any >         aSeqValues( aSeqNames.getLength() );

    for( sal_Int32 nProperty = 0, nCount = aSeqNames.getLength(); nProperty < nCount; ++nProperty )
    {
        switch( nProperty )
        {
            case PROPERTYHANDLE_OVERLAYBUFFER:
                aSeqValues[nProperty] <<= m_bOverlayBuffer;
            break;

            case PROPERTYHANDLE_PAINTBUFFER:
                aSeqValues[nProperty] <<= m_bPaintBuffer;
            break;

            case PROPERTYHANDLE_STRIPE_COLOR_A:
                aSeqValues[nProperty] <<= m_bStripeColorA.GetColor();
            break;

            case PROPERTYHANDLE_STRIPE_COLOR_B:
                aSeqValues[nProperty] <<= m_bStripeColorB.GetColor();
            break;

            case PROPERTYHANDLE_STRIPE_LENGTH:
                aSeqValues[nProperty] <<= m_nStripeLength;
            break;
        }
    }

    PutProperties( aSeqNames, aSeqValues );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtOptionsDrawinglayer_Impl::IsOverlayBuffer() const
{
    return m_bOverlayBuffer;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtOptionsDrawinglayer_Impl::IsPaintBuffer() const
{
    return m_bPaintBuffer;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Color SvtOptionsDrawinglayer_Impl::GetStripeColorA() const
{
    return m_bStripeColorA;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Color SvtOptionsDrawinglayer_Impl::GetStripeColorB() const
{
    return m_bStripeColorB;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_uInt16 SvtOptionsDrawinglayer_Impl::GetStripeLength() const
{
    return m_nStripeLength;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtOptionsDrawinglayer_Impl::SetOverlayBuffer( sal_Bool bState )
{
    if(m_bOverlayBuffer != bState)
    {
        m_bOverlayBuffer = bState;
        SetModified();
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtOptionsDrawinglayer_Impl::SetPaintBuffer( sal_Bool bState )
{
    if(m_bPaintBuffer != bState)
    {
        m_bPaintBuffer = bState;
        SetModified();
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtOptionsDrawinglayer_Impl::SetStripeColorA( Color aColor )
{
    if(m_bStripeColorA != aColor)
    {
        m_bStripeColorA = aColor;
        SetModified();
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtOptionsDrawinglayer_Impl::SetStripeColorB( Color aColor )
{
    if(m_bStripeColorB != aColor)
    {
        m_bStripeColorB = aColor;
        SetModified();
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtOptionsDrawinglayer_Impl::SetStripeLength( sal_uInt16 nLength )
{
    if(m_nStripeLength != nLength)
    {
        m_nStripeLength = nLength;
        SetModified();
    }
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Sequence< OUString > SvtOptionsDrawinglayer_Impl::impl_GetPropertyNames()
{
    // Build static list of configuration key names.
    static const OUString pProperties[] =
    {
        PROPERTYNAME_OVERLAYBUFFER      ,
        PROPERTYNAME_PAINTBUFFER        ,
        PROPERTYNAME_STRIPE_COLOR_A     ,
        PROPERTYNAME_STRIPE_COLOR_B     ,
        PROPERTYNAME_STRIPE_LENGTH
    };
    // Initialize return sequence with these list ...
    static const Sequence< OUString > seqPropertyNames( pProperties, PROPERTYCOUNT );
    // ... and return it.
    return seqPropertyNames;
}

//*****************************************************************************************************************
//  initialize static member
//  DON'T DO IT IN YOUR HEADER!
//  see definition for further informations
//*****************************************************************************************************************
SvtOptionsDrawinglayer_Impl* SvtOptionsDrawinglayer::m_pDataContainer = NULL;
sal_Int32 SvtOptionsDrawinglayer::m_nRefCount = 0;

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtOptionsDrawinglayer::SvtOptionsDrawinglayer()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already!
    if( m_pDataContainer == NULL )
    {
        m_pDataContainer = new SvtOptionsDrawinglayer_Impl();
    }
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtOptionsDrawinglayer::~SvtOptionsDrawinglayer()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Decrease ouer refcount.
    --m_nRefCount;
    // If last instance was deleted ...
    // we must destroy ouer static data container!
    if( m_nRefCount <= 0 )
    {
        delete m_pDataContainer;
        m_pDataContainer = NULL;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtOptionsDrawinglayer::IsOverlayBuffer() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsOverlayBuffer();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtOptionsDrawinglayer::IsPaintBuffer() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsPaintBuffer();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Color SvtOptionsDrawinglayer::GetStripeColorA() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetStripeColorA();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Color SvtOptionsDrawinglayer::GetStripeColorB() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetStripeColorB();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_uInt16 SvtOptionsDrawinglayer::GetStripeLength() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetStripeLength();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtOptionsDrawinglayer::SetOverlayBuffer( sal_Bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetOverlayBuffer( bState );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtOptionsDrawinglayer::SetPaintBuffer( sal_Bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetPaintBuffer( bState );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtOptionsDrawinglayer::SetStripeColorA( Color aColor )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetStripeColorA( aColor );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtOptionsDrawinglayer::SetStripeColorB( Color aColor )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetStripeColorB( aColor );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtOptionsDrawinglayer::SetStripeLength( sal_uInt16 nLength )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetStripeLength( nLength );
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Mutex& SvtOptionsDrawinglayer::GetOwnStaticMutex()
{
    // Initialize static mutex only for one time!
    static Mutex* pMutex = NULL;
    // If these method first called (Mutex not already exist!) ...
    if( pMutex == NULL )
    {
        // ... we must create a new one. Protect follow code with the global mutex -
        // It must be - we create a static variable!
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        // We must check our pointer again - because it can be that another instance of ouer class will be fastr then these!
        if( pMutex == NULL )
        {
            // Create the new mutex and set it for return on static variable.
            static Mutex aMutex;
            pMutex = &aMutex;
        }
    }
    // Return new created or already existing mutex object.
    return *pMutex;
}

// eof
