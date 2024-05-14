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

#include "vbaradiobutton.hxx"
#include "vbanewfont.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaRadioButton::ScVbaRadioButton( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, std::unique_ptr<ov::AbstractGeometryAttributes> pGeomHelper )
    : RadioButtonImpl_BASE( xParent, xContext, xControl, xModel, std::move(pGeomHelper) )
{
}

// Attributes
OUString SAL_CALL
ScVbaRadioButton::getCaption()
{
    OUString Label;
    m_xProps->getPropertyValue( u"Label"_ustr ) >>= Label;
    return Label;
}

void SAL_CALL
ScVbaRadioButton::setCaption( const OUString& _caption )
{
    m_xProps->setPropertyValue( u"Label"_ustr, uno::Any( _caption ) );
}

uno::Any SAL_CALL
ScVbaRadioButton::getValue()
{
    sal_Int16 nValue = -1;
    m_xProps->getPropertyValue( u"State"_ustr ) >>= nValue;
    if( nValue != 0 )
        nValue = -1;
//    return uno::makeAny( nValue );
// I must be missing something MSO says value should be -1 if selected, 0 if not
// selected
    return uno::Any( nValue == -1 );

}

void SAL_CALL
ScVbaRadioButton::setValue( const uno::Any& _value )
{
    sal_Int16 nValue = 0;
    sal_Int16 nOldValue = 0;
    m_xProps->getPropertyValue( u"State"_ustr ) >>= nOldValue;

    if( !( _value >>= nValue ) )
    {
        bool bValue = false;
        _value >>= bValue;
        if ( bValue )
            nValue = -1;
    }

    if( nValue == -1)
        nValue = 1;
    m_xProps->setPropertyValue( u"State"_ustr, uno::Any( nValue ) );
    if ( nValue != nOldValue )
    {
        fireChangeEvent();
        // In Excel, only when the radio button is checked, the click event is fired.
        if ( nValue != 0 )
        {
            fireClickEvent();
        }
    }
}

uno::Reference< msforms::XNewFont > SAL_CALL ScVbaRadioButton::getFont()
{
    return new VbaNewFont( m_xProps );
}

OUString
ScVbaRadioButton::getServiceImplName()
{
    return u"ScVbaRadioButton"_ustr;
}

uno::Sequence< OUString >
ScVbaRadioButton::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.msforms.RadioButton"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
