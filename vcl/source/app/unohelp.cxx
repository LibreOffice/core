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

#include <vcl/svapp.hxx>
#include <vcl/unohelp.hxx>

#include <svdata.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/CharacterClassification.hpp>
#include <com/sun/star/awt/XExtendedToolkit.hpp>
#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

using namespace ::com::sun::star;

uno::Reference < i18n::XBreakIterator > vcl::unohelper::CreateBreakIterator()
{
    uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
    return i18n::BreakIterator::create(xContext);
}

uno::Reference < i18n::XCharacterClassification > vcl::unohelper::CreateCharacterClassification()
{
    return i18n::CharacterClassification::create( comphelper::getProcessComponentContext() );
}

void vcl::unohelper::NotifyAccessibleStateEventGlobally( const css::accessibility::AccessibleEventObject& rEventObject )
{
    css::uno::Reference< css::awt::XExtendedToolkit > xExtToolkit( Application::GetVCLToolkit(), uno::UNO_QUERY );
    if ( xExtToolkit.is() )
    {
        // Only for focus events
        sal_Int16 nType = css::accessibility::AccessibleStateType::INVALID;
        rEventObject.NewValue >>= nType;
        if ( nType == css::accessibility::AccessibleStateType::FOCUSED )
            xExtToolkit->fireFocusGained( rEventObject.Source );
        else
        {
            rEventObject.OldValue >>= nType;
            if ( nType == css::accessibility::AccessibleStateType::FOCUSED )
                xExtToolkit->fireFocusLost( rEventObject.Source );
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
