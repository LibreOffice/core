
#ifndef SVTOOLS_INDEXENTRYRESSOURCE_HXX
#define SVTOOLS_INDEXENTRYRESSOURCE_HXX

class IndexEntryRessourceData;

class IndexEntryRessource
{
    private:
        IndexEntryRessourceData  *mp_Data;

    public:
        IndexEntryRessource ();
        ~IndexEntryRessource ();
        const String& GetTranslation (const String& r_Algorithm);
};

#endif /* SVTOOLS_INDEXENTRYRESSOURCE_HXX */


