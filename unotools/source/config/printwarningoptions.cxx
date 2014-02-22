/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <unotools/printwarningoptions.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <itemholder1.hxx>





using namespace ::utl                   ;
using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;

#define ROOTNODE_START                  OUString("Office.Common/Print")

#define PROPERTYNAME_PAPERSIZE          OUString("Warning/PaperSize")
#define PROPERTYNAME_PAPERORIENTATION   OUString("Warning/PaperOrientation")
#define PROPERTYNAME_NOTFOUND           OUString("Warning/NotFound")
#define PROPERTYNAME_TRANSPARENCY       OUString("Warning/Transparency")
#define PROPERTYNAME_PRINTINGMODIFIESDOCUMENT  OUString("PrintingModifiesDocument")

#define PROPERTYHANDLE_PAPERSIZE        0
#define PROPERTYHANDLE_PAPERORIENTATION 1
#define PROPERTYHANDLE_NOTFOUND         2
#define PROPERTYHANDLE_TRANSPARENCY     3
#define PROPERTYHDL_PRINTINGMODIFIESDOCUMENT            4

#define PROPERTYCOUNT                   5

class SvtPrintWarningOptions_Impl : public ConfigItem
{
public:





     SvtPrintWarningOptions_Impl();
    ~SvtPrintWarningOptions_Impl();





    virtual void Commit();
    virtual void    Notify( const com::sun::star::uno::Sequence< OUString >& aPropertyNames );





    bool    IsPaperSize() const { return m_bPaperSize; }
    bool    IsPaperOrientation() const { return m_bPaperOrientation; }
    bool    IsNotFound() const { return m_bNotFound; }
    bool    IsTransparency() const { return m_bTransparency; }
    bool    IsModifyDocumentOnPrintingAllowed() const { return m_bModifyDocumentOnPrintingAllowed; }

    void        SetPaperSize( bool bState ) { m_bPaperSize = bState; SetModified(); }
    void        SetPaperOrientation( bool bState ) { m_bPaperOrientation = bState; SetModified(); }
    void        SetNotFound( bool bState ) { m_bNotFound = bState; SetModified(); }
    void        SetTransparency( bool bState ) { m_bTransparency = bState; SetModified(); }
    void        SetModifyDocumentOnPrintingAllowed( bool bState ) { m_bModifyDocumentOnPrintingAllowed = bState; SetModified(); }





private:

    static Sequence< OUString > impl_GetPropertyNames();





private:

    bool    m_bPaperSize;
    bool    m_bPaperOrientation;
    bool    m_bNotFound;
    bool    m_bTransparency;
    bool    m_bModifyDocumentOnPrintingAllowed;
};




SvtPrintWarningOptions_Impl::SvtPrintWarningOptions_Impl() :
    ConfigItem( ROOTNODE_START  ),
    m_bPaperSize( false ),
    m_bPaperOrientation( false ),
    m_bNotFound( false ),
    m_bTransparency( true ),
    m_bModifyDocumentOnPrintingAllowed( true )
{
    Sequence< OUString >    seqNames( impl_GetPropertyNames() );
    Sequence< Any >         seqValues( GetProperties( seqNames ) );

    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtPrintWarningOptions_Impl::SvtPrintWarningOptions_Impl()\nI miss some values of configuration keys!\n" );

    
    sal_Int32 nPropertyCount = seqValues.getLength();
    sal_Int32 nProperty = 0;

    for( nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        DBG_ASSERT( seqValues[nProperty].hasValue(), "SvtPrintWarningOptions_Impl::SvtPrintWarningOptions_Impl()\nInvalid property value for property detected!\n" );

        switch( nProperty )
        {
            case PROPERTYHANDLE_PAPERSIZE:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "Invalid type" );
                seqValues[nProperty] >>= m_bPaperSize;
            }
            break;

            case PROPERTYHANDLE_PAPERORIENTATION:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "Invalid type" );
                seqValues[nProperty] >>= m_bPaperOrientation;
            }
            break;

            case PROPERTYHANDLE_NOTFOUND:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "Invalid type" );
                seqValues[nProperty] >>= m_bNotFound;
            }
            break;

            case PROPERTYHANDLE_TRANSPARENCY:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "Invalid type" );
                seqValues[nProperty] >>= m_bTransparency;
            }
            break;
            case PROPERTYHDL_PRINTINGMODIFIESDOCUMENT:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "Invalid type" );
                seqValues[nProperty] >>= m_bModifyDocumentOnPrintingAllowed;
            }
            break;

        }
    }
}




SvtPrintWarningOptions_Impl::~SvtPrintWarningOptions_Impl()
{
    if( IsModified() )
        Commit();
}




void SvtPrintWarningOptions_Impl::Commit()
{
    Sequence< OUString >    aSeqNames( impl_GetPropertyNames() );
    Sequence< Any >         aSeqValues( aSeqNames.getLength() );

    for( sal_Int32 nProperty = 0, nCount = aSeqNames.getLength(); nProperty < nCount; ++nProperty )
    {
        switch( nProperty )
        {
            case PROPERTYHANDLE_PAPERSIZE:
                aSeqValues[nProperty] <<= m_bPaperSize;
            break;

            case PROPERTYHANDLE_PAPERORIENTATION:
                aSeqValues[nProperty] <<= m_bPaperOrientation;
            break;

            case PROPERTYHANDLE_NOTFOUND:
                aSeqValues[nProperty] <<= m_bNotFound;
            break;

            case PROPERTYHANDLE_TRANSPARENCY:
                aSeqValues[nProperty] <<= m_bTransparency;
            break;
            case PROPERTYHDL_PRINTINGMODIFIESDOCUMENT:
                aSeqValues[nProperty] <<= m_bModifyDocumentOnPrintingAllowed;
            break;
        }
    }

    PutProperties( aSeqNames, aSeqValues );
}

void SvtPrintWarningOptions_Impl::Notify( const Sequence< OUString >&  )
{
}




Sequence< OUString > SvtPrintWarningOptions_Impl::impl_GetPropertyNames()
{
    
    const OUString pProperties[] =
    {
        PROPERTYNAME_PAPERSIZE,
        PROPERTYNAME_PAPERORIENTATION,
        PROPERTYNAME_NOTFOUND,
        PROPERTYNAME_TRANSPARENCY,
        PROPERTYNAME_PRINTINGMODIFIESDOCUMENT
    };

    
    const Sequence< OUString > seqPropertyNames( pProperties, PROPERTYCOUNT );

    return seqPropertyNames;
}






SvtPrintWarningOptions_Impl*    SvtPrintWarningOptions::m_pDataContainer = NULL;
sal_Int32                       SvtPrintWarningOptions::m_nRefCount = 0;




SvtPrintWarningOptions::SvtPrintWarningOptions()
{
    
    MutexGuard aGuard( GetOwnStaticMutex() );
    
    ++m_nRefCount;
    
    if( m_pDataContainer == NULL )
    {
        m_pDataContainer = new SvtPrintWarningOptions_Impl();
        ItemHolder1::holdConfigItem(E_PRINTWARNINGOPTIONS);
    }
}




SvtPrintWarningOptions::~SvtPrintWarningOptions()
{
    
    MutexGuard aGuard( GetOwnStaticMutex() );
    
    --m_nRefCount;
    
    
    if( m_nRefCount <= 0 )
    {
        delete m_pDataContainer;
        m_pDataContainer = NULL;
    }
}




bool SvtPrintWarningOptions::IsPaperSize() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsPaperSize();
}




bool SvtPrintWarningOptions::IsPaperOrientation() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsPaperOrientation();
}




bool SvtPrintWarningOptions::IsTransparency() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsTransparency();
}




void SvtPrintWarningOptions::SetPaperSize( bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetPaperSize( bState );
}




void SvtPrintWarningOptions::SetPaperOrientation( bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetPaperOrientation( bState );
}




void SvtPrintWarningOptions::SetTransparency( bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetTransparency( bState );
}


bool SvtPrintWarningOptions::IsModifyDocumentOnPrintingAllowed() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsModifyDocumentOnPrintingAllowed();
}



void SvtPrintWarningOptions::SetModifyDocumentOnPrintingAllowed( bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetModifyDocumentOnPrintingAllowed( bState ) ;
}

namespace
{
    class thePrintWarningOptionsMutex : public rtl::Static<osl::Mutex, thePrintWarningOptionsMutex>{};
}




Mutex& SvtPrintWarningOptions::GetOwnStaticMutex()
{
    return thePrintWarningOptionsMutex::get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
