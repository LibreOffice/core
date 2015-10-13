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


#ifndef INCLUDED_FPICKER_SOURCE_WIN32_FOLDERPICKER_WINFOPIMPL_HXX
#define INCLUDED_FPICKER_SOURCE_WIN32_FOLDERPICKER_WINFOPIMPL_HXX

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include "MtaFop.hxx"


// forward


class CFolderPicker;

class CWinFolderPickerImpl : public CMtaFolderPicker
{
public:
    CWinFolderPickerImpl( CFolderPicker* aFolderPicker );


    // XExecutableDialog


    virtual sal_Int16 SAL_CALL execute(  )
        throw( css::uno::RuntimeException );


    // XFolderPicker


    virtual void SAL_CALL setDisplayDirectory( const OUString& aDirectory )
        throw( css::lang::IllegalArgumentException, css::uno::RuntimeException );

    virtual OUString SAL_CALL getDisplayDirectory( )
        throw( css::uno::RuntimeException );

    virtual OUString SAL_CALL getDirectory( )
        throw( css::uno::RuntimeException );

protected:
    virtual void SAL_CALL onSelChanged( const OUString& aNewPath );

private:
    CFolderPicker*  m_pFolderPicker;
    sal_Int16       m_nLastDlgResult;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
