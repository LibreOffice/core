/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#pragma once

#include <unotools/resmgr.hxx>

inline OUString WpResId(std::string_view aId)
{
    return Translate::get(aId, Translate::Create("wpt"));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
