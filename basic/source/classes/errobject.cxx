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
#include "precompiled_basic.hxx"
#include "errobject.hxx"

#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/script/XDefaultProperty.hpp>
#include "sbintern.hxx"
#include "runtime.hxx"

using namespace ::com::sun::star;
using namespace ::ooo;

typedef ::cppu::WeakImplHelper2< vba::XErrObject, script::XDefaultProperty > ErrObjectImpl_BASE;

class ErrObject : public ErrObjectImpl_BASE
{
    rtl::OUString m_sHelpFile;
    rtl::OUString m_sSource;
    rtl::OUString m_sDescription;
    sal_Int32 m_nNumber;
    sal_Int32 m_nHelpContext;

public:
    ErrObject();
    ~ErrObject();
    // Attributes
    virtual ::sal_Int32 SAL_CALL getNumber() throw (uno::RuntimeException);
    virtual void SAL_CALL setNumber( ::sal_Int32 _number ) throw (uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getHelpContext() throw (uno::RuntimeException);
    virtual void SAL_CALL setHelpContext( ::sal_Int32 _helpcontext ) throw (uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getHelpFile() throw (uno::RuntimeException);
    virtual void SAL_CALL setHelpFile( const ::rtl::OUString& _helpfile ) throw (uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getDescription() throw (uno::RuntimeException);
    virtual void SAL_CALL setDescription( const ::rtl::OUString& _description ) throw (uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getSource() throw (uno::RuntimeException);
    virtual void SAL_CALL setSource( const ::rtl::OUString& _source ) throw (uno::RuntimeException);

    // Methods
    virtual void SAL_CALL Clear(  ) throw (uno::RuntimeException);
    virtual void SAL_CALL Raise( const uno::Any& Number, const uno::Any& Source, const uno::Any& Description, const uno::Any& HelpFile, const uno::Any& HelpContext ) throw (uno::RuntimeException);
    // XDefaultProperty
    virtual ::rtl::OUString SAL_CALL getDefaultPropertyName(  ) throw (uno::RuntimeException);

    // Helper method
    void setData( const uno::Any& Number, const uno::Any& Source, const uno::Any& Description,
        const uno::Any& HelpFile, const uno::Any& HelpContext ) throw (uno::RuntimeException);
};


ErrObject::~ErrObject()
{
}

ErrObject::ErrObject() : m_nNumber(0), m_nHelpContext(0)
{
}

sal_Int32 SAL_CALL
ErrObject::getNumber() throw (uno::RuntimeException)
{
    return m_nNumber;
}

void SAL_CALL
ErrObject::setNumber( ::sal_Int32 _number ) throw (uno::RuntimeException)
{
    pINST->setErrorVB( _number, String() );
    ::rtl::OUString _description = pINST->GetErrorMsg();
    setData( uno::makeAny( _number ), uno::Any(), uno::makeAny( _description ), uno::Any(), uno::Any() );
}

::sal_Int32 SAL_CALL
ErrObject::getHelpContext() throw (uno::RuntimeException)
{
    return m_nHelpContext;
}
void SAL_CALL
ErrObject::setHelpContext( ::sal_Int32 _helpcontext ) throw (uno::RuntimeException)
{
    m_nHelpContext = _helpcontext;
}

::rtl::OUString SAL_CALL
ErrObject::getHelpFile() throw (uno::RuntimeException)
{
    return m_sHelpFile;
}

void SAL_CALL
ErrObject::setHelpFile( const ::rtl::OUString& _helpfile ) throw (uno::RuntimeException)
{
    m_sHelpFile = _helpfile;
}

::rtl::OUString SAL_CALL
ErrObject::getDescription() throw (uno::RuntimeException)
{
    return m_sDescription;
}

void SAL_CALL
ErrObject::setDescription( const ::rtl::OUString& _description ) throw (uno::RuntimeException)
{
    m_sDescription = _description;
}

::rtl::OUString SAL_CALL
ErrObject::getSource() throw (uno::RuntimeException)
{
    return m_sSource;
}

void SAL_CALL
ErrObject::setSource( const ::rtl::OUString& _source ) throw (uno::RuntimeException)
{
    m_sSource = _source;
}

// Methods
void SAL_CALL
ErrObject::Clear(  ) throw (uno::RuntimeException)
{
    m_sHelpFile = rtl::OUString();
    m_sSource = m_sHelpFile;
    m_sDescription = m_sSource;
    m_nNumber = 0;
    m_nHelpContext = 0;
}

void SAL_CALL
ErrObject::Raise( const uno::Any& Number, const uno::Any& Source, const uno::Any& Description, const uno::Any& HelpFile, const uno::Any& HelpContext ) throw (uno::RuntimeException)
{
    setData( Number, Source, Description, HelpFile, HelpContext );
    if ( m_nNumber )
        pINST->ErrorVB( m_nNumber, m_sDescription );
}

// XDefaultProperty
::rtl::OUString SAL_CALL
ErrObject::getDefaultPropertyName(  ) throw (uno::RuntimeException)
{
    static rtl::OUString sDfltPropName( RTL_CONSTASCII_USTRINGPARAM("Number") );
    return sDfltPropName;
}

void ErrObject::setData( const uno::Any& Number, const uno::Any& Source, const uno::Any& Description, const uno::Any& HelpFile, const uno::Any& HelpContext )
    throw (uno::RuntimeException)
{
    if ( !Number.hasValue() )
        throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Missing Required Paramater")), uno::Reference< uno::XInterface >() );
    Number >>= m_nNumber;
    Description >>= m_sDescription;
    Source >>= m_sSource;
    HelpFile >>= m_sHelpFile;
    HelpContext >>= m_nHelpContext;
}

// SbxErrObject
SbxErrObject::SbxErrObject( const String& rName, const Any& rUnoObj )
    : SbUnoObject( rName, rUnoObj )
    , m_pErrObject( NULL )
{
    OSL_TRACE("SbxErrObject::SbxErrObject ctor");
    rUnoObj >>= m_xErr;
    if ( m_xErr.is() )
    {
        SetDfltProperty( uno::Reference< script::XDefaultProperty >( m_xErr, uno::UNO_QUERY_THROW )->getDefaultPropertyName() ) ;
        m_pErrObject = static_cast< ErrObject* >( m_xErr.get() );
    }
}

SbxErrObject::~SbxErrObject()
{
    OSL_TRACE("SbxErrObject::~SbxErrObject dtor");
}

uno::Reference< vba::XErrObject >
SbxErrObject::getUnoErrObject()
{
    SbxVariable* pVar = getErrObject();
    SbxErrObject* pGlobErr = static_cast< SbxErrObject* >(  pVar );
    return pGlobErr->m_xErr;
}

SbxVariableRef
SbxErrObject::getErrObject()
{
    static SbxVariableRef pGlobErr = new SbxErrObject( String(  RTL_CONSTASCII_USTRINGPARAM("Err")), uno::makeAny( uno::Reference< vba::XErrObject >( new ErrObject() ) ) );
    return pGlobErr;
}

void SbxErrObject::setNumberAndDescription( ::sal_Int32 _number, const ::rtl::OUString& _description )
    throw (uno::RuntimeException)
{
    if( m_pErrObject != NULL )
        m_pErrObject->setData( uno::makeAny( _number ), uno::Any(), uno::makeAny( _description ), uno::Any(), uno::Any() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
