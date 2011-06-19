/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef SVTOOLS_COLLATORRESSOURCE_HXX
#define SVTOOLS_COLLATORRESSOURCE_HXX

#include "svtools/svtdllapi.h"
#include <tools/string.hxx>

class CollatorRessourceData;

class SVT_DLLPUBLIC CollatorRessource
{
    private:

        CollatorRessourceData  *mp_Data;

    public:
                                CollatorRessource ();
                                ~CollatorRessource ();
        const String&           GetTranslation (const String& r_Algorithm);
};

#endif /* SVTOOLS_COLLATORRESSOURCE_HXX */


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
