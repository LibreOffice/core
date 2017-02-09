/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <unotools/printwarningoptions.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include "itemholder1.hxx"

//  namespaces

using namespace ::utl;
using namespace ::osl;
using namespace ::com::sun::star::uno;

#define ROOTNODE_START                  "Office.Common/Print"

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

//  constructor / destructor

     SvtPrintWarningOptions_Impl();
    virtual ~SvtPrintWarningOptions_Impl() override;

//  override methods of baseclass

    virtual void    Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;

//  public interface

    bool    IsPaperSize() const { return m_bPaperSize; }
    bool    IsPaperOrientation() const { return m_bPaperOrientation; }
    bool    IsTransparency() const { return m_bTransparency; }
    bool    IsModifyDocumentOnPrintingAllowed() const { return m_bModifyDocumentOnPrintingAllowed; }

    void        SetPaperSize( bool bState ) { m_bPaperSize = bState; SetModified(); }
    void        SetPaperOrientation( bool bState ) { m_bPaperOrientation = bState; SetModified(); }
    void        SetTransparency( bool bState ) { m_bTransparency = bState; SetModified(); }
    void        SetModifyDocumentOnPrintingAllowed( bool bState ) { m_bModifyDocumentOnPrintingAllowed = bState; SetModified(); }

//  private methods

private:

    virtual void ImplCommit() override;

    static Sequence< OUString > impl_GetPropertyNames();

//  private member

private:

    bool    m_bPaperSize;
    bool    m_bPaperOrientation;
    bool    m_bNotFound;
    bool    m_bTransparency;
    bool    m_bModifyDocumentOnPrintingAllowed;
};

//  constructor

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

    // Copy values from list in right order to our internal member.
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

//  destructor

SvtPrintWarningOptions_Impl::~SvtPrintWarningOptions_Impl()
{
    assert(!IsModified()); // should have been committed
}

//  Commit

void SvtPrintWarningOptions_Impl::ImplCommit()
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

//  private method

Sequence< OUString > SvtPrintWarningOptions_Impl::impl_GetPropertyNames()
{
    // Build list of configuration key names.
    const OUString pProperties[] =
    {
        PROPERTYNAME_PAPERSIZE,
        PROPERTYNAME_PAPERORIENTATION,
        PROPERTYNAME_NOTFOUND,
        PROPERTYNAME_TRANSPARENCY,
        PROPERTYNAME_PRINTINGMODIFIESDOCUMENT
    };

    // Initialize return sequence with these list ...
    const Sequence< OUString > seqPropertyNames( pProperties, PROPERTYCOUNT );

    return seqPropertyNames;
}

static std::weak_ptr<SvtPrintWarningOptions_Impl> g_pPrintWarningOptions;

SvtPrintWarningOptions::SvtPrintWarningOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );

    m_pImpl = g_pPrintWarningOptions.lock();
    if( !m_pImpl )
    {
        m_pImpl = std::make_shared<SvtPrintWarningOptions_Impl>();
        g_pPrintWarningOptions = m_pImpl;
        ItemHolder1::holdConfigItem(EItem::PrintWarningOptions);
    }
}

SvtPrintWarningOptions::~SvtPrintWarningOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetOwnStaticMutex() );

    m_pImpl.reset();
}

//  public method

bool SvtPrintWarningOptions::IsPaperSize() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->IsPaperSize();
}

//  public method

bool SvtPrintWarningOptions::IsPaperOrientation() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->IsPaperOrientation();
}

//  public method

bool SvtPrintWarningOptions::IsTransparency() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->IsTransparency();
}

//  public method

void SvtPrintWarningOptions::SetPaperSize( bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pImpl->SetPaperSize( bState );
}

//  public method

void SvtPrintWarningOptions::SetPaperOrientation( bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pImpl->SetPaperOrientation( bState );
}

//  public method

void SvtPrintWarningOptions::SetTransparency( bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pImpl->SetTransparency( bState );
}

bool SvtPrintWarningOptions::IsModifyDocumentOnPrintingAllowed() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->IsModifyDocumentOnPrintingAllowed();
}

void SvtPrintWarningOptions::SetModifyDocumentOnPrintingAllowed( bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pImpl->SetModifyDocumentOnPrintingAllowed( bState );
}

namespace
{
    class thePrintWarningOptionsMutex : public rtl::Static<osl::Mutex, thePrintWarningOptionsMutex>{};
}

//  private method

Mutex& SvtPrintWarningOptions::GetOwnStaticMutex()
{
    return thePrintWarningOptionsMutex::get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
