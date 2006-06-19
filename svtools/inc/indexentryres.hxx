
#ifndef SVTOOLS_INDEXENTRYRESSOURCE_HXX
#define SVTOOLS_INDEXENTRYRESSOURCE_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

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


