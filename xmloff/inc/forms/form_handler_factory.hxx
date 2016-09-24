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

#ifndef INCLUDED_XMLOFF_INC_FORMS_FORM_HANDLER_FACTORY_HXX
#define INCLUDED_XMLOFF_INC_FORMS_FORM_HANDLER_FACTORY_HXX

#include "forms/property_handler.hxx"

namespace xmloff
{

    //= FormHandlerFactory
    class FormHandlerFactory
    {
    public:
        static PPropertyHandler getFormPropertyHandler( const PropertyId i_propertyId );
    };

} // namespace xmloff

#endif // INCLUDED_XMLOFF_INC_FORMS_FORM_HANDLER_FACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
