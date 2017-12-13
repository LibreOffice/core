/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef INCLUDED_WRITERPERFECT_INC_WPFTRESMGR_HXX
#define INCLUDED_WRITERPERFECT_INC_WPFTRESMGR_HXX

#include "writerperfectdllapi.h"

#include <unotools/resmgr.hxx>

inline OUString WpResId(const char *pId)
{
    return Translate::get(pId, Translate::Create("wpt"));
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
