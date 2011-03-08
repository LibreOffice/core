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

#ifndef SFX2_DOCSTORAGEMODIFYLISTENER_HXX
#define SFX2_DOCSTORAGEMODIFYLISTENER_HXX

#include "sfx2/dllapi.h"

/** === begin UNO includes === **/
#include <com/sun/star/util/XModifyListener.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase1.hxx>


//........................................................................
namespace sfx2
{
//........................................................................

    //====================================================================
    //= IModifiableDocument
    //====================================================================
    /** callback for the DocumentStorageModifyListener class
    */
    class SAL_NO_VTABLE IModifiableDocument
    {
    public:
        /// indicates the root or a sub storage of the document has been modified
        virtual void storageIsModified() = 0;
    };

    //====================================================================
    //= DocumentStorageModifyListener
    //====================================================================
    typedef ::cppu::WeakImplHelper1 < ::com::sun::star::util::XModifyListener > DocumentStorageModifyListener_Base;

    class SFX2_DLLPUBLIC DocumentStorageModifyListener : public DocumentStorageModifyListener_Base
    {
        IModifiableDocument*    m_pDocument;
        ::osl::SolarMutex&      m_rMutex;

    public:
        DocumentStorageModifyListener( IModifiableDocument& _rDocument, ::osl::SolarMutex& _rMutex );

        void dispose();

        // XModifyListener
        virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        virtual ~DocumentStorageModifyListener();

    private:
        DocumentStorageModifyListener();                                                    // never implemented
        DocumentStorageModifyListener( const DocumentStorageModifyListener& );              // never implemented
        DocumentStorageModifyListener& operator=( const DocumentStorageModifyListener& );   // never implemented
    };

//........................................................................
} // namespace sfx2
//........................................................................

#endif // SFX2_DOCSTORAGEMODIFYLISTENER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
