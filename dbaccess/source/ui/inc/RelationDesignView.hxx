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

#ifndef DBAUI_RELATIONDESIGNVIEW_HXX
#define DBAUI_RELATIONDESIGNVIEW_HXX

#include "JoinDesignView.hxx"
#include <vector>
#include "QEnumTypes.hxx"
#include "RelationTableView.hxx"

namespace dbaui
{
    class ORelationController;

    class ORelationDesignView : public OJoinDesignView
    {
    public:
        ORelationDesignView(Window* pParent, ORelationController& _rController,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& );
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
