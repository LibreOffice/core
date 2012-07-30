/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef SVTOOLS_INDEXENTRYRESOURCE_HXX
#define SVTOOLS_INDEXENTRYRESOURCE_HXX

#include "svtools/svtdllapi.h"
#include <rtl/ustring.hxx>

class IndexEntryResourceData;

class SVT_DLLPUBLIC IndexEntryResource
{
    private:
        IndexEntryResourceData  *mp_Data;

    public:
        IndexEntryResource ();
        ~IndexEntryResource ();
        const rtl::OUString& GetTranslation (const rtl::OUString& r_Algorithm);
};

#endif /* SVTOOLS_INDEXENTRYRESOURCE_HXX */


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
