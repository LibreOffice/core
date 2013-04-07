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
#ifndef DBAUI_QUERYVIEW_HXX
#define DBAUI_QUERYVIEW_HXX

#include "JoinDesignView.hxx"

namespace dbaui
{
    class OQueryController;
    class OQueryView : public OJoinDesignView
    {
    public:
        OQueryView(Window* pParent, OQueryController& _rController,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& );
        virtual ~OQueryView();

        virtual sal_Bool isCutAllowed()     = 0;
        virtual sal_Bool isPasteAllowed()   = 0;
        virtual sal_Bool isCopyAllowed()    = 0;
        virtual void copy()     = 0;
        virtual void cut()      = 0;
        virtual void paste()    = 0;
        // clears the whole query
        virtual void clear() = 0;
        // set the view readonly or not
        virtual void setReadOnly(sal_Bool _bReadOnly) = 0;
        // set the statement for representation
        virtual void setStatement(const OUString& _rsStatement) = 0;
        // returns the current sql statement
        virtual OUString getStatement() = 0;
    };
}
#endif // DBAUI_QUERYVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
