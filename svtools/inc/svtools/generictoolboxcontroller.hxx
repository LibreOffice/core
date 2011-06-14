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

#ifndef __SVTOOLS_GENERICTOOLBOXCONTROLLER_HXX_
#define __SVTOOLS_GENERICTOOLBOXCONTROLLER_HXX_

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

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
