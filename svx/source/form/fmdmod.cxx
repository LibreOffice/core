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

#include <sal/macros.h>
#include <svx/fmdmod.hxx>
#include <fmservs.hxx>
#include <fmobj.hxx>
#include <svx/unoshape.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

using namespace ::svxform;


::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL SvxFmMSFactory::createInstance(const OUString& rServiceSpecifier)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xRet;

    if ( rServiceSpecifier.startsWith( "com.sun.star.form.component." ) )
    {
        css::uno::Reference<css::uno::XComponentContext> xContext = comphelper::getProcessComponentContext();
        xRet = xContext->getServiceManager()->createInstanceWithContext(rServiceSpecifier, xContext);
    }
    else if ( rServiceSpecifier == "com.sun.star.drawing.ControlShape" )
    {
        SdrModel& rTargetModel(getSdrModelFromUnoModel());
        SdrObject* pObj = new FmFormObj(rTargetModel);
        xRet = static_cast<cppu::OWeakObject*>(static_cast<SvxShape_UnoImplHelper*>(new SvxShapeControl(pObj)));
    }

    if (!xRet.is())
    {
        xRet = SvxUnoDrawMSFactory::createInstance(rServiceSpecifier);
    }

    return xRet;
}


::com::sun::star::uno::Sequence< OUString > SAL_CALL SvxFmMSFactory::getAvailableServiceNames()
{
    static const OUStringLiteral aSvxComponentServiceNameList[] =
    {
        u"" FM_SUN_COMPONENT_TEXTFIELD,
        u"" FM_SUN_COMPONENT_FORM,
        u"" FM_SUN_COMPONENT_LISTBOX,
        u"" FM_SUN_COMPONENT_COMBOBOX,
        u"" FM_SUN_COMPONENT_RADIOBUTTON,
        u"" FM_SUN_COMPONENT_GROUPBOX,
        u"" FM_SUN_COMPONENT_FIXEDTEXT,
        u"" FM_SUN_COMPONENT_COMMANDBUTTON,
        u"" FM_SUN_COMPONENT_CHECKBOX,
        u"" FM_SUN_COMPONENT_GRIDCONTROL,
        u"" FM_SUN_COMPONENT_IMAGEBUTTON,
        u"" FM_SUN_COMPONENT_FILECONTROL,
        u"" FM_SUN_COMPONENT_TIMEFIELD,
        u"" FM_SUN_COMPONENT_DATEFIELD,
        u"" FM_SUN_COMPONENT_NUMERICFIELD,
        u"" FM_SUN_COMPONENT_CURRENCYFIELD,
        u"" FM_SUN_COMPONENT_PATTERNFIELD,
        u"" FM_SUN_COMPONENT_HIDDENCONTROL,
        u"" FM_SUN_COMPONENT_IMAGECONTROL
    };

    static const sal_uInt16 nSvxComponentServiceNameListCount = SAL_N_ELEMENTS(aSvxComponentServiceNameList);

    auto aSeq( comphelper::arrayToSequence< OUString >(aSvxComponentServiceNameList, nSvxComponentServiceNameListCount) );

    ::com::sun::star::uno::Sequence< OUString > aParentSeq( SvxUnoDrawMSFactory::getAvailableServiceNames() );
    return comphelper::concatSequences( aParentSeq, aSeq );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
