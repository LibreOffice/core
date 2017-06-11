/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include "WPFTResMgr.hxx"

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

std::locale &WPFTResLocale::GetResLocale()
{
    static std::locale loc(Translate::Create("wpt", Application::GetSettings().GetUILanguageTag()));
    return loc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
