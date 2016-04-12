/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0
 */


#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>

#define CPPUNIT_ASSERT_MESSAGE

using namespace com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

class CheckIndeterminateState
{
public:
    CheckIndeterminateState()
    {
    }

    void testCheckIndeterminateState(css::uno::Reference<lang::XComponent> mxComponent)
    {
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    css::uno::Reference<text::XText> text(textDocument->getText(), uno::UNO_QUERY_THROW);
    css::uno::Reference<css::frame::XModel> aModel( textDocument, uno::UNO_QUERY_THROW );

    css::uno::Reference<css::frame::XController> xController(aModel->getCurrentController());

    text->setString("normal");
    css::uno::Reference<css::text::XTextRange> end( text->getEnd(), uno::UNO_QUERY_THROW);
    end->setString("bold");

    css::uno::Reference< beans::XPropertySet> ended( end, uno::UNO_QUERY_THROW);
    ended-> setPropertyValue( "CharWeight", uno::makeAny(::com::sun::star::awt::FontWeight::BOLD));

    css::uno::Reference<text::XTextViewCursorSupplier> controller(xController, uno::UNO_QUERY_THROW);
    controller->getViewCursor()->gotoRange(text, false);


    uno::Reference< frame::XFrame > xFrame( xController->getFrame(), uno::UNO_SET_THROW);
    uno::Reference< awt::XWindow > xWindow( xFrame->getContainerWindow(), uno::UNO_SET_THROW);

    css::uno::Reference<css::accessibility::XAccessibleContext> xAccessibleContext;

    xAccessibleContext -> getAccessibleStateSet() -> contains ( AccessibleStateType::EDITABLE);

    if (xAccessibleContext->getAccessibleRole() == AccessibleRole::TOGGLE_BUTTON){
        css::uno::Reference<css::accessibility::XAccessibleContext> oContext;
        css::uno::Reference<css::accessibility::XAccessibleStateSet> oSet( oContext->getAccessibleStateSet());
        CPPUNIT_ASSERT_MESSAGE("The INDETERMINATE state is not set." , oSet->contains(AccessibleStateType::INDETERMINATE));
    }

}
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */