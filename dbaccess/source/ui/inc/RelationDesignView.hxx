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

#ifndef DBAUI_RELATIONDESIGNVIEW_HXX
#define DBAUI_RELATIONDESIGNVIEW_HXX

#include "JoinDesignView.hxx"
#ifndef _VECTOR_
#include <vector>
#endif
#include <tools/string.hxx>
#include "QEnumTypes.hxx"
#include "RelationTableView.hxx"

namespace dbaui
{
    class OAddTableDlg;
    class OTableConnection;
    class ORelationTableConnectionData;
    class OConnectionLineData;
    class ORelationController;

    class ORelationDesignView : public OJoinDesignView
    {
    public:
        ORelationDesignView(Window* pParent, ORelationController& _rController,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
        virtual ~ORelationDesignView();

        // set the statement for representation
        /// late construction
        virtual void Construct();
        virtual void initialize();


        virtual long PreNotify( NotifyEvent& rNEvt );
        virtual void GetFocus();
    };
}
#endif // DBAUI_RELATIONDESIGNVIEW_HXX



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
