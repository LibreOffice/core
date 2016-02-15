/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include "WPFTResMgr.hxx"

#include <tools/resmgr.hxx>

#include <vcl/svapp.hxx>

ResMgr &WPFTResMgr::GetResMgr()
{
    static ResMgr *const pResMgr(ResMgr::CreateResMgr("writerperfect", Application::GetSettings().GetUILanguageTag()));
    return *pResMgr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
