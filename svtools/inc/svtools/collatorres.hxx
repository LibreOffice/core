/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef SVTOOLS_COLLATORRESOURCE_HXX
#define SVTOOLS_COLLATORRESOURCE_HXX

#include "svtools/svtdllapi.h"
#include <rtl/ustring.hxx>

class CollatorResourceData;

class SVT_DLLPUBLIC CollatorResource
{
    private:

        CollatorResourceData  *mp_Data;

    public:
        CollatorResource();
        ~CollatorResource();
        const rtl::OUString& GetTranslation (const rtl::OUString& r_Algorithm);
};

#endif /* SVTOOLS_COLLATORRESOURCE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
