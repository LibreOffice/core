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

#ifndef BASCTL_DLGEDFAC_HXX
#define BASCTL_DLGEDFAC_HXX

#include <svx/svdobj.hxx>
#include <com/sun/star/frame/XModel.hpp>

namespace basctl
{

//============================================================================
// DlgEdFactory
//============================================================================

class DlgEdFactory
{
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > mxModel;
public:
    DlgEdFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel );
    ~DlgEdFactory();

    DECL_LINK( MakeObject, SdrObjFactory * );
};

} // namespace basctl

#endif // BASCTL_DLGEDFAC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
