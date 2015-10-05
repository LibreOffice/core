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
#ifndef INCLUDED_VBAHELPER_VBADIALOGBASE_HXX
#define INCLUDED_VBAHELPER_VBADIALOGBASE_HXX

#include <ooo/vba/XDialogBase.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/frame/XModel.hpp>

typedef InheritedHelperInterfaceWeakImpl< ov::XDialogBase > VbaDialogBase_BASE;

class VBAHELPER_DLLPUBLIC VbaDialogBase : public VbaDialogBase_BASE
{
protected:
    sal_Int32 mnIndex;
        css::uno::Reference< css::frame::XModel > m_xModel;
public:
    VbaDialogBase( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::frame::XModel >& xModel, sal_Int32 nIndex ):VbaDialogBase_BASE( xParent, xContext ), mnIndex( nIndex ), m_xModel( xModel ) {}
    virtual ~VbaDialogBase() {}

    // Methods
    virtual void SAL_CALL Show() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString mapIndexToName( sal_Int32 nIndex ) = 0;
};

#endif // INCLUDED_VBAHELPER_VBADIALOGBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
