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

#ifndef INCLUDED_SFX2_DOCSTORAGEMODIFYLISTENER_HXX
#define INCLUDED_SFX2_DOCSTORAGEMODIFYLISTENER_HXX

#include <sfx2/dllapi.h>

#include <com/sun/star/util/XModifyListener.hpp>

#include <cppuhelper/implbase1.hxx>

namespace comphelper { class SolarMutex; }


namespace sfx2
{



    //= IModifiableDocument

    /** callback for the DocumentStorageModifyListener class
    */
    class SAL_NO_VTABLE IModifiableDocument
    {
    public:
        /// indicates the root or a sub storage of the document has been modified
        virtual void storageIsModified() = 0;

    protected:
        ~IModifiableDocument() {}
    };


    //= DocumentStorageModifyListener

    typedef ::cppu::WeakImplHelper1 < ::com::sun::star::util::XModifyListener > DocumentStorageModifyListener_Base;

    class SFX2_DLLPUBLIC DocumentStorageModifyListener : public DocumentStorageModifyListener_Base
    {
        IModifiableDocument*    m_pDocument;
        comphelper::SolarMutex& m_rMutex;

    public:
        DocumentStorageModifyListener( IModifiableDocument& _rDocument, comphelper::SolarMutex& _rMutex );

        void dispose();

        // XModifyListener
        virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    protected:
        virtual ~DocumentStorageModifyListener();

    private:
        DocumentStorageModifyListener( const DocumentStorageModifyListener& ) = delete;
        DocumentStorageModifyListener& operator=( const DocumentStorageModifyListener& ) = delete;
    };


} // namespace sfx2


#endif // INCLUDED_SFX2_DOCSTORAGEMODIFYLISTENER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
