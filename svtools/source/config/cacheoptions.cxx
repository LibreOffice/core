/*************************************************************************
 *
 *  $RCSfile: cacheoptions.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: ka $ $Date: 2001-04-12 14:59:15 $
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

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#include "cacheoptions.hxx"

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

using namespace ::utl;
using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::uno;

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

#define ROOTNODE_START                      OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Common/Cache"  ))
#define DEFAULT_WRITEROLE                   20
#define DEFAULT_DRAWINGOLE                  20
#define DEFAULT_GRFMGR_TOTALSIZE            10000000
#define DEFAULT_GRFMGR_OBJECTSIZE           2400000

#define PROPERTYNAME_WRITEROLE              OUString(RTL_CONSTASCII_USTRINGPARAM("Writer/OLE_Objects"))
#define PROPERTYNAME_DRAWINGOLE             OUString(RTL_CONSTASCII_USTRINGPARAM("DrawingEngine/OLE_Objects"))
#define PROPERTYNAME_GRFMGR_TOTALSIZE       OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicManager/TotalCacheSize"))
#define PROPERTYNAME_GRFMGR_OBJECTSIZE      OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicManager/ObjectCacheSize"))

#define PROPERTYHANDLE_WRITEROLE            0
#define PROPERTYHANDLE_DRAWINGOLE           1
#define PROPERTYHANDLE_GRFMGR_TOTALSIZE     2
#define PROPERTYHANDLE_GRFMGR_OBJECTSIZE    3

#define PROPERTYCOUNT                       4

class SvtCacheOptions_Impl : public ConfigItem
{
public:

//---------------------------------------------------------------------------------------------------------
//  constructor / destructor
//---------------------------------------------------------------------------------------------------------

     SvtCacheOptions_Impl();
    ~SvtCacheOptions_Impl();

//---------------------------------------------------------------------------------------------------------
//  overloaded methods of baseclass
//---------------------------------------------------------------------------------------------------------

    virtual void    Commit();

//---------------------------------------------------------------------------------------------------------
//  public interface
//---------------------------------------------------------------------------------------------------------

    sal_Int32       GetWriterOLE_Objects() const;
    sal_Int32       GetDrawingEngineOLE_Objects() const;
    sal_Int32       GetGraphicManagerTotalCacheSize() const;
    sal_Int32       GetGraphicManagerObjectCacheSize() const;

    void            SetWriterOLE_Objects( sal_Int32 nObjects );
    void            SetDrawingEngineOLE_Objects( sal_Int32 nObjects );
    void            SetGraphicManagerTotalCacheSize( sal_Int32 nTotalCacheSize );
    void            SetGraphicManagerObjectCacheSize( sal_Int32 nObjectCacheSize );

//-------------------------------------------------------------------------------------------------------------
//  private methods
//-------------------------------------------------------------------------------------------------------------

private:

    static Sequence< OUString > impl_GetPropertyNames();

//-------------------------------------------------------------------------------------------------------------
//  private member
//-------------------------------------------------------------------------------------------------------------

private:

        sal_Int32   mnWriterOLE;
        sal_Int32   mnDrawingOLE;
        sal_Int32   mnGrfMgrTotalSize;
        sal_Int32   mnGrfMgrObjectSize;
};

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtCacheOptions_Impl::SvtCacheOptions_Impl() :
    ConfigItem( ROOTNODE_START  ),
    mnWriterOLE( DEFAULT_WRITEROLE ),
    mnDrawingOLE( DEFAULT_DRAWINGOLE ),
    mnGrfMgrTotalSize( DEFAULT_GRFMGR_TOTALSIZE ),
    mnGrfMgrObjectSize( DEFAULT_GRFMGR_OBJECTSIZE )
{
    Sequence< OUString >    seqNames( impl_GetPropertyNames() );
    Sequence< Any >         seqValues   = GetProperties( seqNames ) ;

    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtCacheOptions_Impl::SvtCacheOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to ouer internal member.
    sal_Int32 nPropertyCount = seqValues.getLength();
    sal_Int32 nProperty = 0;

    for( nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        DBG_ASSERT( !(seqValues[nProperty].hasValue()==sal_False), "SvtCacheOptions_Impl::SvtCacheOptions_Impl()\nInvalid property value for property detected!\n" );

        switch( nProperty )
        {
            case PROPERTYHANDLE_WRITEROLE:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtCacheOptions_Impl::SvtCacheOptions_Impl()\nWho has changed the value type of \"Office.Common\\Cache\\Writer\\OLE_Objects\"?" );
                seqValues[nProperty] >>= mnWriterOLE;
            }
            break;

            case PROPERTYHANDLE_DRAWINGOLE:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtCacheOptions_Impl::SvtCacheOptions_Impl()\nWho has changed the value type of \"Office.Common\\Cache\\DrawingEngine\\OLE_Objects\"?" );
                seqValues[nProperty] >>= mnDrawingOLE;
            }
            break;

            case PROPERTYHANDLE_GRFMGR_TOTALSIZE:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtCacheOptions_Impl::SvtCacheOptions_Impl()\nWho has changed the value type of \"Office.Common\\Cache\\GraphicManager\\TotalCacheSize\"?" );
                seqValues[nProperty] >>= mnGrfMgrTotalSize;
            }
            break;

            case PROPERTYHANDLE_GRFMGR_OBJECTSIZE:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtCacheOptions_Impl::SvtCacheOptions_Impl()\nWho has changed the value type of \"Office.Common\\Cache\\GraphicManager\\ObjectCacheSize\"?" );
                seqValues[nProperty] >>= mnGrfMgrObjectSize;
            }
            break;
        }
    }
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtCacheOptions_Impl::~SvtCacheOptions_Impl()
{
    if( IsModified() )
        Commit();
}

//*****************************************************************************************************************
//  Commit
//*****************************************************************************************************************
void SvtCacheOptions_Impl::Commit()
{
    Sequence< OUString >    aSeqNames( impl_GetPropertyNames() );
    Sequence< Any >         aSeqValues( aSeqNames.getLength() );

    for( sal_Int32 nProperty = 0, nCount = aSeqNames.getLength(); nProperty < nCount; ++nProperty )
    {
        switch( nProperty )
        {
            case PROPERTYHANDLE_WRITEROLE:
                aSeqValues[nProperty] <<= mnWriterOLE;
            break;

            case PROPERTYHANDLE_DRAWINGOLE:
                aSeqValues[nProperty] <<= mnDrawingOLE;
            break;

            case PROPERTYHANDLE_GRFMGR_TOTALSIZE:
                aSeqValues[nProperty] <<= mnGrfMgrTotalSize;
            break;

            case PROPERTYHANDLE_GRFMGR_OBJECTSIZE:
                aSeqValues[nProperty] <<= mnGrfMgrObjectSize;
            break;
        }
    }

    PutProperties( aSeqNames, aSeqValues );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Int32 SvtCacheOptions_Impl::GetWriterOLE_Objects() const
{
    return mnWriterOLE;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Int32 SvtCacheOptions_Impl::GetDrawingEngineOLE_Objects() const
{
    return mnDrawingOLE;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Int32 SvtCacheOptions_Impl::GetGraphicManagerTotalCacheSize() const
{
    return mnGrfMgrTotalSize;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Int32 SvtCacheOptions_Impl::GetGraphicManagerObjectCacheSize() const
{
    return mnGrfMgrObjectSize;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtCacheOptions_Impl::SetWriterOLE_Objects( sal_Int32 nWriterOLE )
{
    mnWriterOLE = nWriterOLE;
    SetModified();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtCacheOptions_Impl::SetDrawingEngineOLE_Objects( sal_Int32 nDrawingOLE )
{
    mnDrawingOLE = nDrawingOLE;
    SetModified();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtCacheOptions_Impl::SetGraphicManagerTotalCacheSize( sal_Int32 nGrfMgrTotalSize )
{
    mnGrfMgrTotalSize = nGrfMgrTotalSize;
    SetModified();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtCacheOptions_Impl::SetGraphicManagerObjectCacheSize( sal_Int32 nGrfMgrObjectSize )
{
    mnGrfMgrObjectSize = nGrfMgrObjectSize;
    SetModified();
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Sequence< OUString > SvtCacheOptions_Impl::impl_GetPropertyNames()
{
    // Build static list of configuration key names.
    static const OUString pProperties[] =
    {
        PROPERTYNAME_WRITEROLE,
        PROPERTYNAME_DRAWINGOLE,
        PROPERTYNAME_GRFMGR_TOTALSIZE,
        PROPERTYNAME_GRFMGR_OBJECTSIZE
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
SvtCacheOptions_Impl*   SvtCacheOptions::m_pDataContainer = NULL;
sal_Int32               SvtCacheOptions::m_nRefCount = 0;

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtCacheOptions::SvtCacheOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already!
    if( m_pDataContainer == NULL )
    {
        m_pDataContainer = new SvtCacheOptions_Impl();
    }
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtCacheOptions::~SvtCacheOptions()
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
sal_Int32 SvtCacheOptions::GetWriterOLE_Objects() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetWriterOLE_Objects();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Int32 SvtCacheOptions::GetDrawingEngineOLE_Objects() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetDrawingEngineOLE_Objects();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Int32 SvtCacheOptions::GetGraphicManagerTotalCacheSize() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetGraphicManagerTotalCacheSize();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Int32 SvtCacheOptions::GetGraphicManagerObjectCacheSize() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetGraphicManagerObjectCacheSize();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtCacheOptions::SetWriterOLE_Objects( sal_Int32 nWriterOLE )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetWriterOLE_Objects( nWriterOLE );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtCacheOptions::SetDrawingEngineOLE_Objects( sal_Int32 nDrawingOLE )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetDrawingEngineOLE_Objects( nDrawingOLE );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtCacheOptions::SetGraphicManagerTotalCacheSize( sal_Int32 nGrfMgrTotalSize )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetGraphicManagerTotalCacheSize( nGrfMgrTotalSize );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtCacheOptions::SetGraphicManagerObjectCacheSize( sal_Int32 nGrfMgrObjectSize )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetGraphicManagerObjectCacheSize( nGrfMgrObjectSize );
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Mutex& SvtCacheOptions::GetOwnStaticMutex()
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
