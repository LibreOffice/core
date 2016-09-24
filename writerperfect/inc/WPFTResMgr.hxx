/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef INCLUDED_WRITERPERFECT_INC_WPFTRESMGR_HXX
#define INCLUDED_WRITERPERFECT_INC_WPFTRESMGR_HXX

#include "writerperfectdllapi.h"

#include <tools/resid.hxx>

struct WRITERPERFECT_DLLPUBLIC WPFTResMgr
{
    static ResMgr &GetResMgr();
};

#define WPFT_RES(i) ResId((i), WPFTResMgr::GetResMgr())
#define WPFT_RESSTR(i) WPFT_RES(i).toString()

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
