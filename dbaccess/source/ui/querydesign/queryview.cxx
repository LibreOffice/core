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

#include "queryview.hxx"
#include "dbu_qry.hrc"
#include "querycontroller.hxx"

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
DBG_NAME(OQueryView)
OQueryView::OQueryView(Window* _pParent, OQueryController& _rController,const Reference< XComponentContext >& _rxContext)
    :OJoinDesignView( _pParent, _rController, _rxContext )
{
    DBG_CTOR(OQueryView,NULL);

}

OQueryView::~OQueryView()
{

    DBG_DTOR(OQueryView,NULL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
