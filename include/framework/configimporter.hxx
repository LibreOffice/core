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

#ifndef INCLUDED_FRAMEWORK_CONFIGIMPORTER_HXX
#define INCLUDED_FRAMEWORK_CONFIGIMPORTER_HXX

#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/ui/XUIConfigurationManager2.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <rtl/ustring.hxx>
#include <framework/fwedllapi.h>

namespace framework
{
    class FWE_DLLPUBLIC UIConfigurationImporterOOo1x
    {
        public:
            static bool ImportCustomToolbars(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager2 >& rContainerFactory,
                        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > >& rSeqContainer,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rToolbarStorage );
    };

}

#endif // INCLUDED_FRAMEWORK_CONFIGIMPORTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
