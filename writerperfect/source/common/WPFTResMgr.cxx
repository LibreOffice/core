/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include "WPFTResMgr.hxx"

std::locale &WPFTResLocale::GetResLocale()
{
    static std::locale loc(Translate::Create("wpt"));
    return loc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
