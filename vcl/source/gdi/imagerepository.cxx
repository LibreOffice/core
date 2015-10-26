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

#include <vcl/bitmapex.hxx>
#include <vcl/imagerepository.hxx>
#include <vcl/implimagetree.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

namespace vcl
{
    bool ImageRepository::loadImage( const OUString& _rName, BitmapEx& _out_rImage, bool _bSearchLanguageDependent, bool loadMissing )
    {
        OUString sIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();

        return ImplImageTree::get().loadImage( _rName, sIconTheme, _out_rImage, _bSearchLanguageDependent, loadMissing );
    }

    bool ImageRepository::loadDefaultImage( BitmapEx& _out_rImage)
    {
        OUString sIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();
        return ImplImageTree::get().loadDefaultImage( sIconTheme,_out_rImage);
    }

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
