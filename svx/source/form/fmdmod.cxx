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
#include "fmservs.hxx"
#include <fmobj.hxx>
#include <svx/unoshape.hxx>
#include <comphelper/processfactory.hxx>
#include <svx/fmglob.hxx>

using namespace ::svxform;


::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL SvxFmMSFactory::createInstance(const OUString& rServiceSpecifier) throw( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xRet;
    if ( rServiceSpecifier.startsWith( "com.sun.star.form.component." ) )
    {
        css::uno::Reference<css::uno::XComponentContext> xContext = comphelper::getProcessComponentContext();
        xRet = xContext->getServiceManager()->createInstanceWithContext(rServiceSpecifier, xContext);
    }
    else if ( rServiceSpecifier == "com.sun.star.drawing.ControlShape" )
    {
        SdrObject* pObj = new FmFormObj();
        xRet = static_cast<cppu::OWeakObject*>(static_cast<SvxShape_UnoImplHelper*>(new SvxShapeControl(pObj)));
    }
    if (!xRet.is())
        xRet = SvxUnoDrawMSFactory::createInstance(rServiceSpecifier);
    return xRet;
}


// Encapsulation violation: We *know* that
// SvxUnoDrawMSFactory::createInstanceWithArguments() always throws.

SAL_WNOUNREACHABLE_CODE_PUSH

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL  SvxFmMSFactory::createInstanceWithArguments(const OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments) throw( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception )
{
    return SvxUnoDrawMSFactory::createInstanceWithArguments(ServiceSpecifier, Arguments );
}

SAL_WNOUNREACHABLE_CODE_POP


::com::sun::star::uno::Sequence< OUString > SAL_CALL SvxFmMSFactory::getAvailableServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
    static const OUStringLiteral aSvxComponentServiceNameList[] =
    {
        OUStringLiteral(FM_SUN_COMPONENT_TEXTFIELD),
        OUStringLiteral(FM_SUN_COMPONENT_FORM),
        OUStringLiteral(FM_SUN_COMPONENT_LISTBOX),
        OUStringLiteral(FM_SUN_COMPONENT_COMBOBOX),
        OUStringLiteral(FM_SUN_COMPONENT_RADIOBUTTON),
        OUStringLiteral(FM_SUN_COMPONENT_GROUPBOX),
        OUStringLiteral(FM_SUN_COMPONENT_FIXEDTEXT),
        OUStringLiteral(FM_SUN_COMPONENT_COMMANDBUTTON),
        OUStringLiteral(FM_SUN_COMPONENT_CHECKBOX),
        OUStringLiteral(FM_SUN_COMPONENT_GRIDCONTROL),
        OUStringLiteral(FM_SUN_COMPONENT_IMAGEBUTTON),
        OUStringLiteral(FM_SUN_COMPONENT_FILECONTROL),
        OUStringLiteral(FM_SUN_COMPONENT_TIMEFIELD),
        OUStringLiteral(FM_SUN_COMPONENT_DATEFIELD),
        OUStringLiteral(FM_SUN_COMPONENT_NUMERICFIELD),
        OUStringLiteral(FM_SUN_COMPONENT_CURRENCYFIELD),
        OUStringLiteral(FM_SUN_COMPONENT_PATTERNFIELD),
        OUStringLiteral(FM_SUN_COMPONENT_HIDDENCONTROL),
        OUStringLiteral(FM_SUN_COMPONENT_IMAGECONTROL)
    };

    static const sal_uInt16 nSvxComponentServiceNameListCount = SAL_N_ELEMENTS(aSvxComponentServiceNameList);

    ::com::sun::star::uno::Sequence< OUString > aSeq( nSvxComponentServiceNameListCount );
    OUString* pStrings = aSeq.getArray();
    for( sal_uInt16 nIdx = 0; nIdx < nSvxComponentServiceNameListCount; nIdx++ )
        pStrings[nIdx] = aSvxComponentServiceNameList[nIdx];

    ::com::sun::star::uno::Sequence< OUString > aParentSeq( SvxUnoDrawMSFactory::getAvailableServiceNames() );
    return concatServiceNames( aParentSeq, aSeq );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
