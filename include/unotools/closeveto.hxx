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

#ifndef UNOTOOLS_CLOSEVETO_HXX
#define UNOTOOLS_CLOSEVETO_HXX

#include "unotools/unotoolsdllapi.h"

#include <com/sun/star/uno/XInterface.hpp>

#include <boost/scoped_ptr.hpp>

//......................................................................................................................
namespace utl
{
//......................................................................................................................

    //==================================================================================================================
    //= CloseVeto
    //==================================================================================================================
    struct CloseVeto_Data;
    /** will add a XCloseListener to a given component, and veto its closing as long as the <code>CloseVeto</code>
        instance is alive.

        If closing has been requested and vetoed while the <code>CloseVeto</code> instance is alive, and the ownership
        went to the <code>CloseVeto</code> instance, then it will close the component in its dtor.
    */
    class UNOTOOLS_DLLPUBLIC CloseVeto
    {
    public:
        CloseVeto( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& i_closeable );
        ~CloseVeto();

    private:
        ::boost::scoped_ptr< CloseVeto_Data >   m_pData;
    };

//......................................................................................................................
} // namespace dbaui
//......................................................................................................................

#endif // UNOTOOLS_CLOSEVETO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
