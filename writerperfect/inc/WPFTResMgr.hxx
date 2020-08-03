/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#pragma once

#include <unotools/resmgr.hxx>

inline OUString WpResId(const char* pId) { return Translate::get(pId, Translate::Create("wpt")); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
