
#ifndef SVTOOLS_COLLATORRESSOURCE_HXX
#define SVTOOLS_COLLATORRESSOURCE_HXX

class CollatorRessourceData;

class CollatorRessource
{
    private:

        CollatorRessourceData  *mp_Data;

    public:
                                CollatorRessource ();
                                ~CollatorRessource ();
        const String&           GetTranslation (const String& r_Algorithm);
};

#endif /* SVTOOLS_COLLATORRESSOURCE_HXX */


