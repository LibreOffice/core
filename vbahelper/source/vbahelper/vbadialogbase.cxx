/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include <vbahelper/vbadialogbase.hxx>
#include <vbahelper/vbahelper.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

sal_Bool SAL_CALL VbaDialogBase::Show() throw ( uno::RuntimeException )
{
    rtl::OUString aURL;
    sal_Bool bSuccess = sal_False;
    if ( m_xModel.is() )
    {
        aURL = mapIndexToName( mnIndex );
        if( aURL.getLength() == 0  )
            throw uno::RuntimeException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " Unable to open the specified dialog " ) ),
                uno::Reference< XInterface > () );

        uno::Sequence< beans::PropertyValue > dispatchProps(0);
        if ( aURL.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(".uno:PrinterSetup")) )
        {
            dispatchProps.realloc(1);
            dispatchProps[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "VBADialogResultRequest" ) );
            dispatchProps[0].Value <<= (sal_Bool) sal_True;
        }

        VBADispatchListener *pNotificationListener = new VBADispatchListener();
        uno::Reference< frame::XDispatchResultListener > rListener = pNotificationListener;
        dispatchRequests( m_xModel, aURL, dispatchProps, rListener, sal_False );

        bSuccess = pNotificationListener->getState();
        uno::Any aResult = pNotificationListener->getResult();
        if ( bSuccess )
        {
            if ( aResult.getValueTypeClass() == uno::TypeClass_BOOLEAN )
                aResult >>= bSuccess;
        }
    }
    return bSuccess;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
