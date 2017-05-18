/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef INCLUDED_WRITERPERFECT_INC_WPFTRESMGR_HXX
#define INCLUDED_WRITERPERFECT_INC_WPFTRESMGR_HXX

#include "writerperfectdllapi.h"

#include <tools/resid.hxx>

struct WRITERPERFECT_DLLPUBLIC WPFTResMgr
{
    static ResMgr &GetResMgr();
};

inline OUString WpResId(sal_uInt16 nId)
{
    return ResId(nId, WPFTResMgr::GetResMgr());
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
