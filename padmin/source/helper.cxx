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

#include <unistd.h>
#include <helper.hxx>
#include <osl/file.hxx>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>
#include <tools/config.hxx>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/XControlAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <unotools/confignode.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <rtl/ustrbuf.hxx>


using namespace osl;
using namespace padmin;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ui::dialogs;


/*
 *  PaResId
 */

ResId padmin::PaResId( sal_uInt32 nId )
{
    static ResMgr* pPaResMgr = NULL;
    if( ! pPaResMgr )
    {
        LanguageTag aLanguageTag( LANGUAGE_SYSTEM);

        utl::OConfigurationNode aNode =
            utl::OConfigurationTreeRoot::tryCreateWithComponentContext(
                    comphelper::getProcessComponentContext(),
                    OUString("org.openoffice.Setup/L10N") );
        if ( aNode.isValid() )
        {
            OUString aLoc;
            Any aValue = aNode.getNodeValue( OUString("ooLocale") );
            if( aValue >>= aLoc )
            {
                aLanguageTag.reset( aLoc);
            }
        }
        pPaResMgr = ResMgr::SearchCreateResMgr( "spa", aLanguageTag );
        AllSettings aSettings = Application::GetSettings();
        aSettings.SetUILanguageTag( aLanguageTag );
        Application::SetSettings( aSettings );
    }
    return ResId( nId, *pPaResMgr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
