/*************************************************************************
 *
 *  $RCSfile: printoptions.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-16 10:08:37 $
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
#ifndef GCC
#pragma hdrstop
#endif

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#include "printoptions.hxx"

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_PRINT_HXX
#include <vcl/print.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

// -----------
// - statics -
// -----------

static USHORT aDPIArray[] = { 72, 96, 150, 200, 300, 600 };

#define DPI_COUNT (sizeof(aDPIArray)/sizeof(aDPIArray[0 ]))

// -----------
// - Defines -
// -----------

#define ROOTNODE_START                                  OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Common/Print/Option"))

#define PROPERTYNAME_REDUCETRANSPARENCY                 OUString(RTL_CONSTASCII_USTRINGPARAM("ReduceTransparency"))
#define PROPERTYNAME_REDUCEDTRANSPARENCYMODE            OUString(RTL_CONSTASCII_USTRINGPARAM("ReducedTransparencyMode"))
#define PROPERTYNAME_REDUCEGRADIENTS                    OUString(RTL_CONSTASCII_USTRINGPARAM("ReduceGradients"))
#define PROPERTYNAME_REDUCEDGRADIENTMODE                OUString(RTL_CONSTASCII_USTRINGPARAM("ReducedGradientMode"))
#define PROPERTYNAME_REDUCEDGRADIENTSTEPCOUNT           OUString(RTL_CONSTASCII_USTRINGPARAM("ReducedGradientStepCount"))
#define PROPERTYNAME_REDUCEBITMAPS                      OUString(RTL_CONSTASCII_USTRINGPARAM("ReduceBitmaps"))
#define PROPERTYNAME_REDUCEDBITMAPMODE                  OUString(RTL_CONSTASCII_USTRINGPARAM("ReducedBitmapMode"))
#define PROPERTYNAME_REDUCEDBITMAPRESOLUTION            OUString(RTL_CONSTASCII_USTRINGPARAM("ReducedBitmapResolution"))
#define PROPERTYNAME_REDUCEDBITMAPINCLUDESTRANSPARENCY  OUString(RTL_CONSTASCII_USTRINGPARAM("ReducedBitmapIncludesTransparency"))
#define PROPERTYNAME_CONVERTTOGREYSCALES                OUString(RTL_CONSTASCII_USTRINGPARAM("ConvertToGreyscales"))

#define PROPERTYHDL_REDUCETRANSPARENCY                  0
#define PROPERTYHDL_REDUCEDTRANSPARENCYMODE             1
#define PROPERTYHDL_REDUCEGRADIENTS                     2
#define PROPERTYHDL_REDUCEDGRADIENTMODE                 3
#define PROPERTYHDL_REDUCEDGRADIENTSTEPCOUNT            4
#define PROPERTYHDL_REDUCEBITMAPS                       5
#define PROPERTYHDL_REDUCEDBITMAPMODE                   6
#define PROPERTYHDL_REDUCEDBITMAPRESOLUTION             7
#define PROPERTYHDL_REDUCEDBITMAPINCLUDESTRANSPARENCY   8
#define PROPERTYHDL_CONVERTTOGREYSCALES                 9

#define PROPERTYCOUNT                                   10

// --------------
// - Namespaces -
// --------------

using namespace ::utl;
using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::uno;

// -----------
// - statics -
// -----------

static SvtPrintOptions_Impl*   pPrinterOptionsDataContainer = NULL;
static SvtPrintOptions_Impl*   pPrintFileOptionsDataContainer = NULL;

SvtPrintOptions_Impl*   SvtPrinterOptions::m_pStaticDataContainer = NULL;
sal_Int32               SvtPrinterOptions::m_nRefCount = 0;

SvtPrintOptions_Impl*   SvtPrintFileOptions::m_pStaticDataContainer = NULL;
sal_Int32               SvtPrintFileOptions::m_nRefCount = 0;

// ------------------------
// - SvtPrintOptions_Impl -
// ------------------------

class SvtPrintOptions_Impl : public ConfigItem
{
public:

//---------------------------------------------------------------------------------------------------------
//  constructor / destructor
//---------------------------------------------------------------------------------------------------------

     SvtPrintOptions_Impl( const OUString& rConfigRoot );
    ~SvtPrintOptions_Impl();

//---------------------------------------------------------------------------------------------------------
//  overloaded methods of baseclass
//---------------------------------------------------------------------------------------------------------

    virtual void Commit();

//---------------------------------------------------------------------------------------------------------
//  public interface
//---------------------------------------------------------------------------------------------------------

    sal_Bool    IsReduceTransparency() const { return m_bReduceTransparency; }
    sal_Int16   GetReducedTransparencyMode() const { return m_nReducedTransparencyMode; }
    sal_Bool    IsReduceGradients() const { return m_bReduceGradients; }
    sal_Int16   GetReducedGradientMode() const { return m_nReducedGradientMode; }
    sal_Int16   GetReducedGradientStepCount() const { return m_nReducedGradientStepCount; }
    sal_Bool    IsReduceBitmaps() const { return m_bReduceBitmaps; }
    sal_Int16   GetReducedBitmapMode() const { return m_nReducedBitmapMode; }
    sal_Int16   GetReducedBitmapResolution() const { return m_nReducedBitmapResolution; }
    sal_Bool    IsReducedBitmapIncludesTransparency() const { return m_bReducedBitmapIncludesTransparency; }
       sal_Bool IsConvertToGreyscales() const { return m_bConvertToGreyscales; }

    void        SetReduceTransparency( sal_Bool bState ) { m_bReduceTransparency = bState; SetModified(); }
    void        SetReducedTransparencyMode( sal_Int16 nMode ) { m_nReducedTransparencyMode = nMode; SetModified(); }
    void        SetReduceGradients( sal_Bool bState ) { m_bReduceGradients = bState; SetModified(); }
    void        SetReducedGradientMode( sal_Int16 nMode ) { m_nReducedGradientMode = nMode; SetModified(); }
    void        SetReducedGradientStepCount( sal_Int16 nStepCount ) { m_nReducedGradientStepCount = nStepCount; SetModified(); }
    void        SetReduceBitmaps( sal_Bool bState ) { m_bReduceBitmaps = bState; SetModified(); }
    void        SetReducedBitmapMode( sal_Int16 nMode ) { m_nReducedBitmapMode = nMode; SetModified(); }
    void        SetReducedBitmapResolution( sal_Int16 nResolution ) { m_nReducedBitmapResolution = nResolution; SetModified(); }
    void        SetReducedBitmapIncludesTransparency( sal_Bool bState ) { m_bReducedBitmapIncludesTransparency = bState; SetModified(); }
       void        SetConvertToGreyscales( sal_Bool bState ) { m_bConvertToGreyscales = bState; SetModified(); }

//-------------------------------------------------------------------------------------------------------------
//  private methods
//-------------------------------------------------------------------------------------------------------------

private:

    static Sequence< OUString > impl_GetPropertyNames();

//-------------------------------------------------------------------------------------------------------------
//  private member
//-------------------------------------------------------------------------------------------------------------

private:

    sal_Bool    m_bReduceTransparency;
    sal_Int16   m_nReducedTransparencyMode;
    sal_Bool    m_bReduceGradients;
    sal_Int16   m_nReducedGradientMode;
    sal_Int16   m_nReducedGradientStepCount;
    sal_Bool    m_bReduceBitmaps;
    sal_Int16   m_nReducedBitmapMode;
    sal_Int16   m_nReducedBitmapResolution;
    sal_Bool    m_bReducedBitmapIncludesTransparency;
       sal_Bool m_bConvertToGreyscales;
};

// -----------------------------------------------------------------------------

SvtPrintOptions_Impl::SvtPrintOptions_Impl( const OUString& rConfigRoot ) :
    ConfigItem( rConfigRoot ),
    m_bReduceTransparency( sal_False ),
    m_nReducedTransparencyMode( 0 ),
    m_bReduceGradients( sal_False ),
    m_nReducedGradientMode( 0 ),
    m_nReducedGradientStepCount( 64 ),
    m_bReduceBitmaps( sal_False ),
    m_nReducedBitmapMode( 1 ),
    m_nReducedBitmapResolution( 3 ),
    m_bReducedBitmapIncludesTransparency( sal_True ),
    m_bConvertToGreyscales( sal_False )
{
    Sequence< OUString >    seqNames( impl_GetPropertyNames() );
    Sequence< Any >         seqValues( GetProperties( seqNames ) );

    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtPrintOptions_Impl::SvtPrintOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to our internal member.
    sal_Int32 nPropertyCount = seqValues.getLength();
    sal_Int32 nProperty = 0;

    for( nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        DBG_ASSERT( !(seqValues[nProperty].hasValue()==sal_False), "SvtPrintOptions_Impl::SvtPrintOptions_Impl()\nInvalid property value for property detected!\n" );

        switch( nProperty )
        {
            case PROPERTYHDL_REDUCETRANSPARENCY:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "Invalid type" );
                seqValues[nProperty] >>= m_bReduceTransparency;
            }
            break;


            case PROPERTYHDL_REDUCEDTRANSPARENCYMODE:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_SHORT), "Invalid type" );
                seqValues[nProperty] >>= m_nReducedTransparencyMode;
            }
            break;

            case PROPERTYHDL_REDUCEGRADIENTS:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "Invalid type" );
                seqValues[nProperty] >>= m_bReduceGradients;
            }
            break;

            case PROPERTYHDL_REDUCEDGRADIENTMODE:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_SHORT), "Invalid type" );
                seqValues[nProperty] >>= m_nReducedGradientMode;
            }
            break;

            case PROPERTYHDL_REDUCEDGRADIENTSTEPCOUNT:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_SHORT), "Invalid type" );
                seqValues[nProperty] >>= m_nReducedGradientStepCount;
            }
            break;

            case PROPERTYHDL_REDUCEBITMAPS:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "Invalid type" );
                seqValues[nProperty] >>= m_bReduceBitmaps;
            }
            break;

            case PROPERTYHDL_REDUCEDBITMAPMODE:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_SHORT), "Invalid type" );
                seqValues[nProperty] >>= m_nReducedBitmapMode;
            }
            break;

            case PROPERTYHDL_REDUCEDBITMAPRESOLUTION:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_SHORT), "Invalid type" );
                seqValues[nProperty] >>= m_nReducedBitmapResolution;
            }
            break;

            case PROPERTYHDL_REDUCEDBITMAPINCLUDESTRANSPARENCY:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "Invalid type" );
                seqValues[nProperty] >>= m_bReducedBitmapIncludesTransparency;
            }
            break;

            case PROPERTYHDL_CONVERTTOGREYSCALES:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "Invalid type" );
                seqValues[nProperty] >>= m_bConvertToGreyscales;
            }
            break;
        }
    }
}

// -----------------------------------------------------------------------------

SvtPrintOptions_Impl::~SvtPrintOptions_Impl()
{
    if( IsModified() )
        Commit();
}

// -----------------------------------------------------------------------------

void SvtPrintOptions_Impl::Commit()
{
    Sequence< OUString >    aSeqNames( impl_GetPropertyNames() );
    Sequence< Any >         aSeqValues( aSeqNames.getLength() );

    for( sal_Int32 nProperty = 0, nCount = aSeqNames.getLength(); nProperty < nCount; ++nProperty )
    {
        switch( nProperty )
        {
            case PROPERTYHDL_REDUCETRANSPARENCY:
                aSeqValues[nProperty] <<= m_bReduceTransparency;
            break;

            case PROPERTYHDL_REDUCEDTRANSPARENCYMODE:
                aSeqValues[nProperty] <<= m_nReducedTransparencyMode;
            break;

            case PROPERTYHDL_REDUCEGRADIENTS:
                aSeqValues[nProperty] <<= m_bReduceGradients;
            break;

            case PROPERTYHDL_REDUCEDGRADIENTMODE:
                aSeqValues[nProperty] <<= m_nReducedGradientMode;
            break;

            case PROPERTYHDL_REDUCEDGRADIENTSTEPCOUNT:
                aSeqValues[nProperty] <<= m_nReducedGradientStepCount;
            break;

            case PROPERTYHDL_REDUCEBITMAPS:
                aSeqValues[nProperty] <<= m_bReduceBitmaps;
            break;

            case PROPERTYHDL_REDUCEDBITMAPMODE:
                aSeqValues[nProperty] <<= m_nReducedBitmapMode;
            break;

            case PROPERTYHDL_REDUCEDBITMAPRESOLUTION:
                aSeqValues[nProperty] <<= m_nReducedBitmapResolution;
            break;

            case PROPERTYHDL_REDUCEDBITMAPINCLUDESTRANSPARENCY:
                aSeqValues[nProperty] <<= m_bReducedBitmapIncludesTransparency;
            break;

            case PROPERTYHDL_CONVERTTOGREYSCALES:
                aSeqValues[nProperty] <<= m_bConvertToGreyscales;
            break;
        }
    }

    PutProperties( aSeqNames, aSeqValues );
}

// -----------------------------------------------------------------------------

Sequence< OUString > SvtPrintOptions_Impl::impl_GetPropertyNames()
{
    // Build static list of configuration key names.
    static const OUString pProperties[] =
    {
        PROPERTYNAME_REDUCETRANSPARENCY,
        PROPERTYNAME_REDUCEDTRANSPARENCYMODE,
        PROPERTYNAME_REDUCEGRADIENTS,
        PROPERTYNAME_REDUCEDGRADIENTMODE,
        PROPERTYNAME_REDUCEDGRADIENTSTEPCOUNT,
        PROPERTYNAME_REDUCEBITMAPS,
        PROPERTYNAME_REDUCEDBITMAPMODE,
        PROPERTYNAME_REDUCEDBITMAPRESOLUTION,
        PROPERTYNAME_REDUCEDBITMAPINCLUDESTRANSPARENCY,
        PROPERTYNAME_CONVERTTOGREYSCALES
    };

    // Initialize return sequence with these list ...
    static const Sequence< OUString > seqPropertyNames( pProperties, PROPERTYCOUNT );

    return seqPropertyNames;
}

// -----------------------
// - SvtBasePrintOptions -
// -----------------------

SvtBasePrintOptions::SvtBasePrintOptions()
{
}

// -----------------------------------------------------------------------------

SvtBasePrintOptions::~SvtBasePrintOptions()
{
}

// -----------------------------------------------------------------------------

Mutex& SvtBasePrintOptions::GetOwnStaticMutex()
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

// -----------------------------------------------------------------------------

sal_Bool SvtBasePrintOptions::IsReduceTransparency() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsReduceTransparency();
}

// -----------------------------------------------------------------------------

sal_Int16 SvtBasePrintOptions::GetReducedTransparencyMode() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetReducedTransparencyMode();
}

// -----------------------------------------------------------------------------

sal_Bool SvtBasePrintOptions::IsReduceGradients() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsReduceGradients();
}

// -----------------------------------------------------------------------------

sal_Int16 SvtBasePrintOptions::GetReducedGradientMode() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetReducedGradientMode();
}

// -----------------------------------------------------------------------------

sal_Int16 SvtBasePrintOptions::GetReducedGradientStepCount() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetReducedGradientStepCount();
}

// -----------------------------------------------------------------------------

sal_Bool SvtBasePrintOptions::IsReduceBitmaps() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsReduceBitmaps();
}

// -----------------------------------------------------------------------------

sal_Int16 SvtBasePrintOptions::GetReducedBitmapMode() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetReducedBitmapMode();
}

// -----------------------------------------------------------------------------

sal_Int16 SvtBasePrintOptions::GetReducedBitmapResolution() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetReducedBitmapResolution();
}

// -----------------------------------------------------------------------------

sal_Bool SvtBasePrintOptions::IsReducedBitmapIncludesTransparency() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsReducedBitmapIncludesTransparency();
}

// -----------------------------------------------------------------------------

sal_Bool SvtBasePrintOptions::IsConvertToGreyscales() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsConvertToGreyscales();
}

// -----------------------------------------------------------------------------

void SvtBasePrintOptions::SetReduceTransparency( sal_Bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetReduceTransparency( bState ) ;
}

// -----------------------------------------------------------------------------

void SvtBasePrintOptions::SetReducedTransparencyMode( sal_Int16 nMode )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetReducedTransparencyMode( nMode );
}

// -----------------------------------------------------------------------------

void SvtBasePrintOptions::SetReduceGradients( sal_Bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetReduceGradients( bState );
}

// -----------------------------------------------------------------------------

void SvtBasePrintOptions::SetReducedGradientMode( sal_Int16 nMode )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetReducedGradientMode( nMode );
}

// -----------------------------------------------------------------------------

void SvtBasePrintOptions::SetReducedGradientStepCount( sal_Int16 nStepCount )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetReducedGradientStepCount( nStepCount );
}

// -----------------------------------------------------------------------------

void SvtBasePrintOptions::SetReduceBitmaps( sal_Bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetReduceBitmaps( bState );
}

// -----------------------------------------------------------------------------

void SvtBasePrintOptions::SetReducedBitmapMode( sal_Int16 nMode )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetReducedBitmapMode( nMode );
}

// -----------------------------------------------------------------------------

void SvtBasePrintOptions::SetReducedBitmapResolution( sal_Int16 nResolution )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetReducedBitmapResolution( nResolution );
}

// -----------------------------------------------------------------------------

void SvtBasePrintOptions::SetReducedBitmapIncludesTransparency( sal_Bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetReducedBitmapIncludesTransparency( bState );
}

// -----------------------------------------------------------------------------

void SvtBasePrintOptions::SetConvertToGreyscales( sal_Bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetConvertToGreyscales( bState );
}

// -----------------------------------------------------------------------------

void SvtBasePrintOptions::GetPrinterOptions( PrinterOptions& rOptions ) const
{
    rOptions.SetReduceTransparency( IsReduceTransparency() );
    rOptions.SetReducedTransparencyMode( (PrinterTransparencyMode) GetReducedTransparencyMode() );
    rOptions.SetReduceGradients( IsReduceGradients() );
    rOptions.SetReducedGradientMode( (PrinterGradientMode) GetReducedGradientMode() );
    rOptions.SetReducedGradientStepCount( GetReducedGradientStepCount() );
    rOptions.SetReduceBitmaps( IsReduceBitmaps() );
    rOptions.SetReducedBitmapMode( (PrinterBitmapMode) GetReducedBitmapMode() );
    rOptions.SetReducedBitmapResolution( aDPIArray[ Min( (USHORT) GetReducedBitmapResolution(), (USHORT)( DPI_COUNT - 1 ) ) ] );
    rOptions.SetReducedBitmapIncludesTransparency( IsReducedBitmapIncludesTransparency() );
    rOptions.SetConvertToGreyscales( IsConvertToGreyscales() );
}

// -----------------------------------------------------------------------------

void SvtBasePrintOptions::SetPrinterOptions( const PrinterOptions& rOptions )
{
    SetReduceTransparency( rOptions.IsReduceTransparency() );
    SetReducedTransparencyMode( rOptions.GetReducedTransparencyMode() );
    SetReduceGradients( rOptions.IsReduceGradients() );
    SetReducedGradientMode( rOptions.GetReducedGradientMode() );
    SetReducedGradientStepCount( rOptions.GetReducedGradientStepCount() );
    SetReduceBitmaps( rOptions.IsReduceBitmaps() );
    SetReducedBitmapMode( rOptions.GetReducedBitmapMode() );
    SetReducedBitmapIncludesTransparency( rOptions.IsReducedBitmapIncludesTransparency() );
    SetConvertToGreyscales( rOptions.IsConvertToGreyscales() );

    const USHORT nDPI = rOptions.GetReducedBitmapResolution();

    if( nDPI < aDPIArray[ 0 ] )
        SetReducedBitmapResolution( 0 );
    else
    {
        for( long i = ( DPI_COUNT - 1 ); i >= 0; i-- )
        {
            if( nDPI >= aDPIArray[ i ] )
            {
                SetReducedBitmapResolution( (sal_Int16) i );
                i = -1;
            }
        }
    }
}

// ---------------------
// - SvtPrinterOptions -
// ---------------------

SvtPrinterOptions::SvtPrinterOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already!
    if( m_pStaticDataContainer == NULL )
    {
        OUString aRootPath( ROOTNODE_START );
        SetDataContainer( m_pStaticDataContainer = new SvtPrintOptions_Impl( aRootPath += OUString( RTL_CONSTASCII_USTRINGPARAM( "/Printer" ) ) ) );
        pPrinterOptionsDataContainer = m_pStaticDataContainer;
    }
}

// -----------------------------------------------------------------------------

SvtPrinterOptions::~SvtPrinterOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Decrease ouer refcount.
    --m_nRefCount;
    // If last instance was deleted ...
    // we must destroy ouer static data container!
    if( m_nRefCount <= 0 )
    {
        delete m_pStaticDataContainer;
        m_pStaticDataContainer = NULL;
        pPrinterOptionsDataContainer = NULL;
    }
}

// ---------------------
// - SvtPrintFileOptions -
// ---------------------

SvtPrintFileOptions::SvtPrintFileOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already!
    if( m_pStaticDataContainer == NULL )
    {
        OUString aRootPath( ROOTNODE_START );
        SetDataContainer( m_pStaticDataContainer = new SvtPrintOptions_Impl( aRootPath += OUString( RTL_CONSTASCII_USTRINGPARAM( "/File" ) ) ) );
        pPrintFileOptionsDataContainer = m_pStaticDataContainer;
    }
}

// -----------------------------------------------------------------------------

SvtPrintFileOptions::~SvtPrintFileOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Decrease ouer refcount.
    --m_nRefCount;
    // If last instance was deleted ...
    // we must destroy ouer static data container!
    if( m_nRefCount <= 0 )
    {
        delete m_pStaticDataContainer;
        m_pStaticDataContainer = NULL;
        pPrintFileOptionsDataContainer = NULL;
    }
}
