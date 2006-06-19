
#ifndef SVTOOLS_COLLATORRESSOURCE_HXX
#define SVTOOLS_COLLATORRESSOURCE_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

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


