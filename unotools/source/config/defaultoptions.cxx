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

#include <sal/config.h>

#include <osl/file.hxx>
#include <unotools/defaultoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustrbuf.hxx>
#include <officecfg/Office/Common.hxx>

namespace SvtDefaultOptions
{

OUString GetDefaultPath( SvtPathOptions::Paths nId )
{
    SvtPathOptions aPathOpt;
    auto seqToPath = [&aPathOpt] (const css::uno::Sequence<OUString> & rSeq)
        {
            // single paths
            sal_Int32 nCount = rSeq.getLength();
            OUStringBuffer aFullPathBuf(nCount * 40);
            for ( sal_Int32 nPosition = 0; nPosition < nCount; ++nPosition )
            {
                aFullPathBuf.append(aPathOpt.SubstituteVariable( rSeq[ nPosition ] ));
                if ( nPosition < nCount-1 )
                    aFullPathBuf.append(";");
            }
            return aFullPathBuf.makeStringAndClear();
        };

    OUString aRet;
    switch (nId)
    {
        case SvtPathOptions::Paths::AddIn:
            aRet = aPathOpt.SubstituteVariable(officecfg::Office::Common::Path::Default::Addin::get()); break;
        case SvtPathOptions::Paths::AutoCorrect:
            aRet = seqToPath(officecfg::Office::Common::Path::Default::AutoCorrect::get()); break;
        case SvtPathOptions::Paths::AutoText:
            aRet = seqToPath(officecfg::Office::Common::Path::Default::AutoText::get()); break;
        case SvtPathOptions::Paths::Backup:
            aRet = aPathOpt.SubstituteVariable(officecfg::Office::Common::Path::Default::Backup::get()); break;
        case SvtPathOptions::Paths::Basic:
            aRet = seqToPath(officecfg::Office::Common::Path::Default::Basic::get()); break;
        case SvtPathOptions::Paths::Bitmap:
            aRet = aPathOpt.SubstituteVariable(officecfg::Office::Common::Path::Default::Bitmap::get()); break;
        case SvtPathOptions::Paths::Config:
            aRet = aPathOpt.SubstituteVariable(officecfg::Office::Common::Path::Default::Config::get()); break;
        case SvtPathOptions::Paths::Dictionary:
            aRet = aPathOpt.SubstituteVariable(officecfg::Office::Common::Path::Default::Dictionary::get()); break;
        case SvtPathOptions::Paths::Favorites:
            aRet = aPathOpt.SubstituteVariable(officecfg::Office::Common::Path::Default::Favorite::get()); break;
        case SvtPathOptions::Paths::Filter:
            aRet = aPathOpt.SubstituteVariable(officecfg::Office::Common::Path::Default::Filter::get()); break;
        case SvtPathOptions::Paths::Gallery:
            aRet = seqToPath(officecfg::Office::Common::Path::Default::Gallery::get()); break;
        case SvtPathOptions::Paths::Graphic:
            aRet = aPathOpt.SubstituteVariable(officecfg::Office::Common::Path::Default::Graphic::get()); break;
        case SvtPathOptions::Paths::Help:
            aRet = aPathOpt.SubstituteVariable(officecfg::Office::Common::Path::Default::Help::get()); break;
        case SvtPathOptions::Paths::Linguistic:
            aRet = aPathOpt.SubstituteVariable(officecfg::Office::Common::Path::Default::Linguistic::get()); break;
        case SvtPathOptions::Paths::Module:
            aRet = aPathOpt.SubstituteVariable(officecfg::Office::Common::Path::Default::Module::get()); break;
        case SvtPathOptions::Paths::Palette:
            aRet = aPathOpt.SubstituteVariable(officecfg::Office::Common::Path::Default::Palette::get()); break;
        case SvtPathOptions::Paths::Plugin:
            aRet = seqToPath(officecfg::Office::Common::Path::Default::Plugin::get()); break;
        case SvtPathOptions::Paths::Temp:
            aRet = aPathOpt.SubstituteVariable(officecfg::Office::Common::Path::Default::Temp::get()); break;
        case SvtPathOptions::Paths::Template:
            aRet = seqToPath(officecfg::Office::Common::Path::Default::Template::get()); break;
        case SvtPathOptions::Paths::UserConfig:
            aRet = aPathOpt.SubstituteVariable(officecfg::Office::Common::Path::Default::UserConfig::get()); break;
        case SvtPathOptions::Paths::Work:
            aRet = aPathOpt.SubstituteVariable(officecfg::Office::Common::Path::Default::Work::get()); break;
        case SvtPathOptions::Paths::Classification:
            aRet = aPathOpt.SubstituteVariable(officecfg::Office::Common::Path::Default::Classification::get()); break;
        case SvtPathOptions::Paths::DocumentTheme:
            aRet = aPathOpt.SubstituteVariable(officecfg::Office::Common::Path::Default::DocumentTheme::get()); break;
        default:
            assert(false);
    }


    if ( nId == SvtPathOptions::Paths::AddIn ||
         nId == SvtPathOptions::Paths::Filter ||
         nId == SvtPathOptions::Paths::Help ||
         nId == SvtPathOptions::Paths::Module ||
         nId == SvtPathOptions::Paths::Plugin )
    {
        OUString aTmp;
        osl::FileBase::getFileURLFromSystemPath( aRet, aTmp );
        aRet = aTmp;
    }

    return aRet;
}

} // namespace



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
