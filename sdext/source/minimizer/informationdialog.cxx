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


#include "informationdialog.hxx"
#include <com/sun/star/awt/PushButtonType.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <rtl/ustrbuf.hxx>
#include <sal/macros.h>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;

static OUString ImpValueOfInMB( sal_Int64 rVal )
{
    double fVal( static_cast<double>( rVal ) );
    fVal /= ( 1 << 20 );
    fVal += 0.05;
    OUStringBuffer aVal( OUString::number( fVal ) );
    sal_Int32 nX( aVal.indexOf( '.' ) );
    if ( nX > 0 )
        aVal.setLength( nX + 2 );
    return aVal.makeStringAndClear();
}

void InformationDialog::InitDialog()
{
    set_title(getString(STR_SUN_OPTIMIZATION_WIZARD2));

    sal_Int64 nSource = mnSourceSize;
    sal_Int64 nDest   = mnDestSize;

    PPPOptimizerTokenEnum eInfoString( STR_INFO_SECONDARY_1 );
    if ( mnSourceSize )
    {
        if ( mnDestSize )
            eInfoString = STR_INFO_SECONDARY_1;
        else
        {
            eInfoString = STR_INFO_SECONDARY_2;
            nDest = mnApproxSize;
        }
    }
    else if ( mnDestSize )
        eInfoString = STR_INFO_SECONDARY_3;
    else
    {
        eInfoString = STR_INFO_SECONDARY_4;
        nDest = mnApproxSize;
    }

    OUString aTitle;
    if ( !maSaveAsURL.isEmpty() )
    {
        Reference< XURLTransformer > xURLTransformer( URLTransformer::create(mxContext) );
        util::URL aURL, aPresentationURL;
        aURL.Complete = maSaveAsURL;
        xURLTransformer->parseSmart( aURL, OUString() );

        static constexpr OUString sFileProtocol( u"file:///"_ustr );
        aPresentationURL.Complete = sFileProtocol + aURL.Name;
        aTitle = xURLTransformer->getPresentation( aPresentationURL, false );

        if ( aTitle.match( sFileProtocol ) )
            aTitle = aTitle.replaceAt( 0, sFileProtocol.getLength(), u"" );
    }

    OUString sPrimary( getString( STR_INFO_PRIMARY ) );
    OUString sSecondary( getString( eInfoString ) );
    static constexpr OUString aOldSizePlaceholder( u"%OLDFILESIZE"_ustr  );
    static constexpr OUString aNewSizePlaceholder( u"%NEWFILESIZE"_ustr  );
    const OUString aTitlePlaceholder( !aTitle.isEmpty() ? OUString("%TITLE"  )
                                                         : OUString("'%TITLE'") );

    sal_Int32 i = sSecondary.indexOf( aOldSizePlaceholder );
    if ( i >= 0 )
        sSecondary = sSecondary.replaceAt( i, aOldSizePlaceholder.getLength(), ImpValueOfInMB( nSource ) );

    sal_Int32 j = sSecondary.indexOf( aNewSizePlaceholder );
    if ( j >= 0 )
        sSecondary = sSecondary.replaceAt( j, aNewSizePlaceholder.getLength(), ImpValueOfInMB( nDest ) );

    sal_Int32 k = sPrimary.indexOf( aTitlePlaceholder );
    if ( k >= 0 )
        sPrimary = sPrimary.replaceAt( k, aTitlePlaceholder.getLength(), aTitle );

    set_primary_text(sPrimary);
    set_secondary_text(sSecondary);
    mxCheckBox->set_visible(!maSaveAsURL.isEmpty());
    mxCheckBox->set_active(mrbOpenNewDocument);
}

InformationDialog::InformationDialog(const Reference< XComponentContext > &rxContext, const Reference<XWindow>& rxDialogParent,
                                     const OUString& rSaveAsURL, bool& rbOpenNewDocument,
                                     sal_Int64 rSourceSize, sal_Int64 rDestSize, sal_Int64 rApproxSize)
    : MessageDialogController(Application::GetFrameWeld(rxDialogParent), "modules/simpress/ui/pminfodialog.ui", "PMInfoDialog", "ask")
    , ConfigurationAccess(rxContext)
    , mxCheckBox(m_xBuilder->weld_check_button("ask"))
    , mnSourceSize(rSourceSize)
    , mnDestSize(rDestSize)
    , mnApproxSize(rApproxSize)
    , mrbOpenNewDocument(rbOpenNewDocument)
    , maSaveAsURL(rSaveAsURL)
{
    InitDialog();
}

InformationDialog::~InformationDialog()
{
}

void InformationDialog::execute()
{
    run();
    if (!maSaveAsURL.isEmpty())
        mrbOpenNewDocument = mxCheckBox->get_active();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
