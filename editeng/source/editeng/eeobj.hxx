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

#ifndef INCLUDED_EDITENG_SOURCE_EDITENG_EEOBJ_HXX
#define INCLUDED_EDITENG_SOURCE_EDITENG_EEOBJ_HXX

#include <cppuhelper/weak.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>

#include <tools/stream.hxx>

class EditDataObject :  public css::datatransfer::XTransferable,
                        public ::cppu::OWeakObject

{
private:
    SvMemoryStream  maBinData;
    SvMemoryStream  maRTFData;
    OUString        maText;

    OUString        maOfficeBookmark;

public:
                    EditDataObject();
                    virtual ~EditDataObject();

    SvMemoryStream& GetStream() { return maBinData; }
    SvMemoryStream& GetRTFStream() { return maRTFData; }
    OUString&       GetString() { return maText; }
    OUString&       GetURL()    { return maOfficeBookmark; }


    // css::uno::XInterface
    css::uno::Any                               SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // css::datatransfer::XTransferable
    css::uno::Any SAL_CALL getTransferData( const css::datatransfer::DataFlavor& aFlavor ) throw(css::datatransfer::UnsupportedFlavorException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< css::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isDataFlavorSupported( const css::datatransfer::DataFlavor& aFlavor ) throw(css::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_EDITENG_SOURCE_EDITENG_EEOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
