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

#include <sfx2/docfile.hxx>
#include <com/sun/star/plugin/PluginDescription.hpp>
#include <com/sun/star/plugin/PluginManager.hpp>
#include <com/sun/star/plugin/XPluginManager.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

#include <comphelper/processfactory.hxx>

#include "svx/pfiledlg.hxx"
#include <svx/dialogs.hrc>

#include <svx/dialmgr.hxx>
#include <list>

using namespace ::rtl;
using namespace ::com::sun::star;

sal_Char const sAudio[] = "audio";
sal_Char const sVideo[] = "video";

// Filedialog to insert Plugin-Fileformats

ErrCode SvxPluginFileDlg::Execute()
{
    return maFileDlg.Execute();
}

OUString SvxPluginFileDlg::GetPath() const
{
    return maFileDlg.GetPath();
}

SvxPluginFileDlg::SvxPluginFileDlg (Window *, sal_uInt16 nKind )
    : maFileDlg(ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, SFXWB_INSERT)
{
    // set title of the dialogwindow
    switch (nKind)
    {
        case SID_INSERT_SOUND :
        {
            maFileDlg.SetTitle(SVX_RESSTR(STR_INSERT_SOUND_TITLE));
        }
        break;
        case SID_INSERT_VIDEO :
        {
            maFileDlg.SetTitle(SVX_RESSTR(STR_INSERT_VIDEO_TITLE));
        }
        break;
    }

    // fill the filterlist of the filedialog with data of installed plugins
    uno::Reference< uno::XComponentContext >  xContext = comphelper::getProcessComponentContext();
    uno::Reference< plugin::XPluginManager >  rPluginManager( plugin::PluginManager::create(xContext) );

    const uno::Sequence<plugin::PluginDescription > aSeq( rPluginManager->getPluginDescriptions() );
    const plugin::PluginDescription* pDescription = aSeq.getConstArray();
    sal_Int32 nAnzahlPlugins = rPluginManager->getPluginDescriptions().getLength();

    std::list< OUString > aPlugNames;
    std::list< OUString > aPlugExtensions;
    std::list< OUString >::iterator j;
    std::list< OUString >::iterator k;
    std::list< OUString >::const_iterator end;

    for ( int i = 0; i < nAnzahlPlugins; i++ )
    {
        OUString aStrPlugMIMEType( pDescription[i].Mimetype );
        OUString aStrPlugName( pDescription[i].Description );
        OUString aStrPlugExtension( pDescription[i].Extension );

        aStrPlugMIMEType = aStrPlugMIMEType.toAsciiLowerCase();
        aStrPlugExtension = aStrPlugExtension.toAsciiLowerCase();

        if ( ( nKind == SID_INSERT_SOUND && aStrPlugMIMEType.indexOf( sAudio ) == 0 ) ||
             ( nKind == SID_INSERT_VIDEO && aStrPlugMIMEType.indexOf( sVideo ) == 0 ) )
        {
            // extension already in the filterlist of the filedlg ?
            bool bAlreadyExist = false;
            for ( j = aPlugExtensions.begin(), end = aPlugExtensions.end(); j != end && !bAlreadyExist; ++j )
            {
                bAlreadyExist = (j->indexOf( aStrPlugExtension ) != -1 );
            }

            if ( !bAlreadyExist )
            {
                // filterdescription already there?
                // (then append the new extension to the existing filter)
                int nfound = -1;
                 for ( j = aPlugNames.begin(),
                          k = aPlugExtensions.begin(),
                          end = aPlugNames.end();
                      j != end && nfound != 0;  )
                {
                    if ( ( nfound = j->indexOf( aStrPlugName ) ) == 0 )
                    {
                        if ( !aStrPlugExtension.isEmpty() )
                            aStrPlugExtension += ";";
                        aStrPlugExtension += *k;

                        // remove old entry, increment (iterators are invalid thereafter, thus the postincrement)
                        aPlugNames.erase(j++);
                        aPlugExtensions.erase(k++);

                        // update end iterator (which may be invalid, too!)
                        end = aPlugNames.end();
                    }
                    else
                    {
                        // next element
                        ++j; ++k;
                    }
                }

                // build filterdescription
                aStrPlugName += "  (";
                aStrPlugName += aStrPlugExtension;
                aStrPlugName += ")";

                // use a own description for the video-formate avi, mov and mpeg
                // the descriptions of these MIME-types are not very meaningful
                const sal_Char sAVI[] = "*.avi";
                const sal_Char sMOV[] = "*.mov";
                const sal_Char sMPG[] = "*.mpg";
                const sal_Char sMPE[] = "*.mpe";
                const sal_Char sMPEG[] = "*.mpeg";

                if ( aStrPlugExtension.equalsIgnoreAsciiCase( sAVI ) )
                    aStrPlugName = SVX_RESSTR( STR_INSERT_VIDEO_EXTFILTER_AVI );
                else if ( aStrPlugExtension.equalsIgnoreAsciiCase( sMOV ) )
                    aStrPlugName = SVX_RESSTR( STR_INSERT_VIDEO_EXTFILTER_MOV );
                else if ( aStrPlugExtension.indexOf( sMPG ) != -1 ||
                          aStrPlugExtension.indexOf( sMPE ) != -1 ||
                          aStrPlugExtension.indexOf( sMPEG ) != -1 )
                    aStrPlugName = SVX_RESSTR(STR_INSERT_VIDEO_EXTFILTER_MPEG);

                aPlugNames.push_back( aStrPlugName );
                aPlugExtensions.push_back( aStrPlugExtension );
            }
        }
    }

    // add filter to dialog
    for ( j = aPlugNames.begin(),
              k = aPlugExtensions.begin(),
              end = aPlugNames.end();
          j != end; ++j, ++k )
    {
        maFileDlg.AddFilter( *j, *k );
    }

    // add the All-Filter
    OUString aAllFilter( ResId( STR_EXTFILTER_ALL, DIALOG_MGR() ).toString() );
    maFileDlg.AddFilter(aAllFilter, "*.*");

    // and activate him
    maFileDlg.SetCurrentFilter( aAllFilter );
}

SvxPluginFileDlg::~SvxPluginFileDlg()
{
}

void SvxPluginFileDlg::SetContext( sfx2::FileDialogHelper::Context _eNewContext )
{
    maFileDlg.SetContext( _eNewContext );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
