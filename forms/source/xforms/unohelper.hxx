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
#ifndef _UNOHELPER_HXX
#define _UNOHELPER_HXX

#include <sal/types.h>

// forward declarations
namespace rtl { class OUString; }
namespace com { namespace sun { namespace star {
    namespace uno {
        class XInterface;
        template<class T> class Reference;
    }
    namespace beans { class XPropertySet; }
} } }


namespace xforms
{

/** instantiate a UNO service using the process global service factory */
com::sun::star::uno::Reference<com::sun::star::uno::XInterface>
    createInstance( const rtl::OUString& sServiceName );

/** copy the properties from one PropertySet into the next */
void copy( const com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& , com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& );

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
