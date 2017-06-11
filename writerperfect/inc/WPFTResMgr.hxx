/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef INCLUDED_WRITERPERFECT_INC_WPFTRESMGR_HXX
#define INCLUDED_WRITERPERFECT_INC_WPFTRESMGR_HXX

#include "writerperfectdllapi.h"

#include <tools/simplerm.hxx>

struct WRITERPERFECT_DLLPUBLIC WPFTResLocale
{
    static std::locale& GetResLocale();
};

inline OUString WpResId(const char* pId)
{
    return Translate::get(pId, WPFTResLocale::GetResLocale());
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
