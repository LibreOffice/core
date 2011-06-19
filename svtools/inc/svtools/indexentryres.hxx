/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef SVTOOLS_INDEXENTRYRESSOURCE_HXX
#define SVTOOLS_INDEXENTRYRESSOURCE_HXX

#include "svtools/svtdllapi.h"
#include <tools/string.hxx>

class IndexEntryRessourceData;

class SVT_DLLPUBLIC IndexEntryRessource
{
    private:
        IndexEntryRessourceData  *mp_Data;

    public:
        IndexEntryRessource ();
        ~IndexEntryRessource ();
        const String& GetTranslation (const String& r_Algorithm);
};

#endif /* SVTOOLS_INDEXENTRYRESSOURCE_HXX */


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
