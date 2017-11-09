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

#ifndef INCLUDED_SVX_XMLEOHLP_HXX
#define INCLUDED_SVX_XMLEOHLP_HXX

#include <cppuhelper/compbase.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/document/XEmbeddedObjectResolver.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <svx/svxdllapi.h>
#include <map>
#include <memory>

enum class SvXMLEmbeddedObjectHelperMode
{
    Read, Write
};

namespace comphelper { class IEmbeddedHelper; }

class SvGlobalName;
class OutputStorageWrapper_Impl;


class SVX_DLLPUBLIC SvXMLEmbeddedObjectHelper final :
    public cppu::WeakComponentImplHelper< css::document::XEmbeddedObjectResolver, css::container::XNameAccess >
{
    ::osl::Mutex                maMutex;

    const OUString       maReplacementGraphicsContainerStorageName;
    const OUString       maReplacementGraphicsContainerStorageName60;
    OUString             maCurContainerStorageName;


    css::uno::Reference < css::embed::XStorage > mxRootStorage;  // package
    ::comphelper::IEmbeddedHelper*             mpDocPersist;
    css::uno::Reference < css::embed::XStorage > mxContainerStorage; // container sub package for
    css::uno::Reference < css::embed::XStorage > mxTempStorage;  // package
                                                // objects
    SvXMLEmbeddedObjectHelperMode       meCreateMode;
    std::unique_ptr<std::map< OUString, rtl::Reference<OutputStorageWrapper_Impl> >>
                                                 mpStreamMap;

    SVX_DLLPRIVATE bool                 ImplGetStorageNames(
                                       const OUString& rURLStr,
                                       OUString& rContainerStorageName,
                                       OUString& rObjectStorageName,
                                       bool bInternalToExternal,
                                       bool *pGraphicRepl=nullptr,
                                       bool *pOasisFormat=nullptr ) const;

    SVX_DLLPRIVATE css::uno::Reference < css::embed::XStorage > const & ImplGetContainerStorage(
                                    const OUString& rStorageName );

    SVX_DLLPRIVATE void                 ImplReadObject(
                                    const OUString& rContainerStorageName,
                                    OUString& rObjName,
                                    const SvGlobalName *pClassId,
                                    SvStream* pTemp );

    SVX_DLLPRIVATE OUString              ImplInsertEmbeddedObjectURL(
                                    const OUString& rURLStr );

    SVX_DLLPRIVATE css::uno::Reference< css::io::XInputStream > ImplGetReplacementImage(
                                const css::uno::Reference< css::embed::XEmbeddedObject >& xObj );

                                SvXMLEmbeddedObjectHelper();
                                virtual ~SvXMLEmbeddedObjectHelper() override;
    void                        Init( const css::uno::Reference < css::embed::XStorage >&,
                                      ::comphelper::IEmbeddedHelper& rDocPersist,
                                      SvXMLEmbeddedObjectHelperMode eCreateMode );

    virtual void SAL_CALL       disposing() override;

public:
                                SvXMLEmbeddedObjectHelper(
                                    ::comphelper::IEmbeddedHelper& rDocPersist,
                                    SvXMLEmbeddedObjectHelperMode eCreateMode );

    static rtl::Reference<SvXMLEmbeddedObjectHelper> Create(
                                    const css::uno::Reference < css::embed::XStorage >&,
                                    ::comphelper::IEmbeddedHelper& rDocPersist,
                                    SvXMLEmbeddedObjectHelperMode eCreateMode );
    static rtl::Reference<SvXMLEmbeddedObjectHelper>   Create(
                                    ::comphelper::IEmbeddedHelper& rDocPersist,
                                    SvXMLEmbeddedObjectHelperMode eCreateMode );

    // XEmbeddedObjectResolver
    virtual OUString SAL_CALL resolveEmbeddedObjectURL( const OUString& aURL ) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // XNameAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;


    static void splitObjectURL(const OUString& aURLNoPar,
        OUString& rContainerStorageName,
        OUString& rObjectStorageName);
};

#endif // INCLUDED_SVX_XMLEOHLP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
