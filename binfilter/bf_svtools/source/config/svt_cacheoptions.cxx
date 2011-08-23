/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove

//_________________________________________________________________________________________________________________
//	includes
//_________________________________________________________________________________________________________________

#include "cacheoptions.hxx"

#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

//_________________________________________________________________________________________________________________
//	namespaces
//_________________________________________________________________________________________________________________

using namespace ::utl;
using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::uno;

namespace binfilter
{

//_________________________________________________________________________________________________________________
//	const
//_________________________________________________________________________________________________________________

#define	ROOTNODE_START			   			OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Common/Cache"	))
#define	DEFAULT_WRITEROLE					20
#define	DEFAULT_DRAWINGOLE					20
#define	DEFAULT_GRFMGR_TOTALSIZE			10000000
#define	DEFAULT_GRFMGR_OBJECTSIZE			2400000
#define	DEFAULT_GRFMGR_OBJECTRELEASE	    600
                                            
#define	PROPERTYNAME_WRITEROLE				OUString(RTL_CONSTASCII_USTRINGPARAM("Writer/OLE_Objects"))
#define	PROPERTYNAME_DRAWINGOLE				OUString(RTL_CONSTASCII_USTRINGPARAM("DrawingEngine/OLE_Objects"))
#define	PROPERTYNAME_GRFMGR_TOTALSIZE		OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicManager/TotalCacheSize"))
#define	PROPERTYNAME_GRFMGR_OBJECTSIZE		OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicManager/ObjectCacheSize"))
#define PROPERTYNAME_GRFMGR_OBJECTRELEASE   OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicManager/ObjectReleaseTime"))

#define	PROPERTYHANDLE_WRITEROLE			0
#define	PROPERTYHANDLE_DRAWINGOLE			1
#define	PROPERTYHANDLE_GRFMGR_TOTALSIZE		2
#define	PROPERTYHANDLE_GRFMGR_OBJECTSIZE	3
#define PROPERTYHANDLE_GRFMGR_OBJECTRELEASE 4

#define	PROPERTYCOUNT						5

class SvtCacheOptions_Impl : public ConfigItem
{
public:

//---------------------------------------------------------------------------------------------------------
//	constructor / destructor
//---------------------------------------------------------------------------------------------------------

     SvtCacheOptions_Impl();
    ~SvtCacheOptions_Impl();

//---------------------------------------------------------------------------------------------------------
//	overloaded methods of baseclass
//---------------------------------------------------------------------------------------------------------

    virtual void	Commit();
    void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& );

//---------------------------------------------------------------------------------------------------------
//	public interface
//---------------------------------------------------------------------------------------------------------

    sal_Int32		GetDrawingEngineOLE_Objects() const;

//-------------------------------------------------------------------------------------------------------------
//	private methods
//-------------------------------------------------------------------------------------------------------------

private:

    static Sequence< OUString > impl_GetPropertyNames();

//-------------------------------------------------------------------------------------------------------------
//	private member
//-------------------------------------------------------------------------------------------------------------

private:

        sal_Int32	mnWriterOLE;
        sal_Int32	mnDrawingOLE;
        sal_Int32	mnGrfMgrTotalSize;
        sal_Int32	mnGrfMgrObjectSize;
        sal_Int32   mnGrfMgrObjectRelease;
};

//_________________________________________________________________________________________________________________
//	definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//	constructor
//*****************************************************************************************************************
SvtCacheOptions_Impl::SvtCacheOptions_Impl() :
    ConfigItem( ROOTNODE_START	),
    mnWriterOLE( DEFAULT_WRITEROLE ),
    mnDrawingOLE( DEFAULT_DRAWINGOLE ),
    mnGrfMgrTotalSize( DEFAULT_GRFMGR_TOTALSIZE ),
    mnGrfMgrObjectSize( DEFAULT_GRFMGR_OBJECTSIZE ),
    mnGrfMgrObjectRelease( DEFAULT_GRFMGR_OBJECTRELEASE )
{
    Sequence< OUString >	seqNames( impl_GetPropertyNames() );
    Sequence< Any >			seqValues	= GetProperties( seqNames )	;

    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtCacheOptions_Impl::SvtCacheOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to ouer internal member.
    sal_Int32 nPropertyCount = seqValues.getLength();
    sal_Int32 nProperty	= 0;
    
    for( nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        if( seqValues[ nProperty ].hasValue() )
        {
            switch( nProperty )
            {
                case PROPERTYHANDLE_WRITEROLE:
                {
                    if( seqValues[ nProperty ].getValueTypeClass() == TypeClass_LONG )
                        seqValues[nProperty] >>= mnWriterOLE;
                }
                break;

                case PROPERTYHANDLE_DRAWINGOLE:
                {
                    if( seqValues[ nProperty ].getValueTypeClass() == TypeClass_LONG )
                        seqValues[nProperty] >>= mnDrawingOLE;
                }
                break;

                case PROPERTYHANDLE_GRFMGR_TOTALSIZE:
                {
                    if( seqValues[ nProperty ].getValueTypeClass() == TypeClass_LONG )
                        seqValues[nProperty] >>= mnGrfMgrTotalSize;
                }
                break;

                case PROPERTYHANDLE_GRFMGR_OBJECTSIZE:
                {
                    if( seqValues[ nProperty ].getValueTypeClass() == TypeClass_LONG )
                        seqValues[nProperty] >>= mnGrfMgrObjectSize;
                }
                break;

                case PROPERTYHANDLE_GRFMGR_OBJECTRELEASE:
                {
                    if( seqValues[ nProperty ].getValueTypeClass() == TypeClass_LONG )
                        seqValues[nProperty] >>= mnGrfMgrObjectRelease;
                }
                break;
            }
        }
    }
}

//*****************************************************************************************************************
//	destructor
//*****************************************************************************************************************
SvtCacheOptions_Impl::~SvtCacheOptions_Impl()
{
    if( IsModified() )
        Commit();
}

//*****************************************************************************************************************
//	Commit
//*****************************************************************************************************************
void SvtCacheOptions_Impl::Commit()
{
    Sequence< OUString >	aSeqNames( impl_GetPropertyNames() );
    Sequence< Any >			aSeqValues( aSeqNames.getLength() );
    
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

            case PROPERTYHANDLE_GRFMGR_OBJECTRELEASE:
                aSeqValues[nProperty] <<= mnGrfMgrObjectRelease;
            break;
        }
    }

    PutProperties( aSeqNames, aSeqValues );
}

    void SvtCacheOptions_Impl::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

//*****************************************************************************************************************
//	public method
//*****************************************************************************************************************
sal_Int32 SvtCacheOptions_Impl::GetDrawingEngineOLE_Objects() const
{
    return mnDrawingOLE;
}

//*****************************************************************************************************************
//	private method
//*****************************************************************************************************************
Sequence< OUString > SvtCacheOptions_Impl::impl_GetPropertyNames()
{
    // Build static list of configuration key names.
    static const OUString pProperties[] =
    {
        PROPERTYNAME_WRITEROLE,
        PROPERTYNAME_DRAWINGOLE,
        PROPERTYNAME_GRFMGR_TOTALSIZE,
        PROPERTYNAME_GRFMGR_OBJECTSIZE,
        PROPERTYNAME_GRFMGR_OBJECTRELEASE
    };
    // Initialize return sequence with these list ...
    static const Sequence< OUString > seqPropertyNames( pProperties, PROPERTYCOUNT );
    // ... and return it.
    return seqPropertyNames;
}

//*****************************************************************************************************************
//	initialize static member
//	DON'T DO IT IN YOUR HEADER!
//	see definition for further informations
//*****************************************************************************************************************
SvtCacheOptions_Impl*	SvtCacheOptions::m_pDataContainer = NULL;
sal_Int32				SvtCacheOptions::m_nRefCount = 0;

//*****************************************************************************************************************
//	constructor
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
//	destructor
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
//	public method
//*****************************************************************************************************************
sal_Int32 SvtCacheOptions::GetDrawingEngineOLE_Objects() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetDrawingEngineOLE_Objects();
}

//*****************************************************************************************************************
//	private method
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
