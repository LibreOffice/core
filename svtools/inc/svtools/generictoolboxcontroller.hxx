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

#ifndef __SVTOOLS_GENERICTOOLBOXCONTROLLER_HXX_
#define __SVTOOLS_GENERICTOOLBOXCONTROLLER_HXX_

#include "svtools/svtdllapi.h"
#include <svtools/toolboxcontroller.hxx>
#include <vcl/toolbox.hxx>

namespace svt
{

struct ExecuteInfo;
class SVT_DLLPUBLIC GenericToolboxController : public svt::ToolboxController
{
    public:
        GenericToolboxController( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager,
                                  const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                                  ToolBox* pToolBox,
                                  sal_uInt16   nID,
                                  const rtl::OUString& aCommand );
        virtual ~GenericToolboxController();

        // XComponent
        virtual void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );

        // XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw (::com::sun::star::uno::RuntimeException);

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

         DECL_STATIC_LINK( GenericToolboxController, ExecuteHdl_Impl, ExecuteInfo* );

    private:
        ToolBox*    m_pToolbox;
        sal_uInt16  m_nID;
};

}

#endif // __SVTOOLS_GENERICTOOLBOXCONTROLLER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
