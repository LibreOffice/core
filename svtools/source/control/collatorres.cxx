#include <svtdata.hxx>
#include <svtools.hrc>


#ifndef SVTOOLS_COLLATORRESSOURCE_HXX
#include <collatorres.hxx>
#endif

// -------------------------------------------------------------------------
//
//  wrapper for locale specific translations data of collator algorithm
//
// -------------------------------------------------------------------------

class CollatorRessourceData
{
    friend class CollatorRessource;

    private: /* data */

        String          ma_Name;
        String          ma_Translation;

    private: /* member functions */

                        CollatorRessourceData ()
                            {}
    public:
                        CollatorRessourceData (
                            const String &r_Algorithm, const String &r_Translation)
                                 : ma_Name (r_Algorithm), ma_Translation (r_Translation)
                            {}

        const String&   GetAlgorithm () const
                            { return ma_Name; }

        const String&   GetTranslation () const
                            { return ma_Translation; }

                        ~CollatorRessourceData ()
                            {}

        CollatorRessourceData&
                        operator= (CollatorRessourceData& r_From)
                            {
                                ma_Name         = r_From.GetAlgorithm();
                                ma_Translation  = r_From.GetTranslation();
                                return *this;
                            }
};

// -------------------------------------------------------------------------
//
//  implementation of the collator-algorithm-name translation
//
// -------------------------------------------------------------------------

CollatorRessource::CollatorRessource()
{
    mp_Data = new CollatorRessourceData[7];

    #define ASCSTR(str) String(RTL_CONSTASCII_USTRINGPARAM(str))
    #define RESSTR(rid) String(SvtResId(rid))

    mp_Data[0] = CollatorRessourceData (ASCSTR("charset"),
                                             RESSTR(STR_SVT_COLLATE_CHARSET));
    mp_Data[1] = CollatorRessourceData (ASCSTR("dictionary"),
                                             RESSTR(STR_SVT_COLLATE_DICTIONARY));
    mp_Data[2] = CollatorRessourceData (ASCSTR("normal"),
                                             RESSTR(STR_SVT_COLLATE_NORMAL));
    mp_Data[3] = CollatorRessourceData (ASCSTR("pinyin"),
                                             RESSTR(STR_SVT_COLLATE_PINYIN));
    mp_Data[4] = CollatorRessourceData (ASCSTR("radical"),
                                             RESSTR(STR_SVT_COLLATE_RADICAL));
    mp_Data[5] = CollatorRessourceData (ASCSTR("stroke"),
                                             RESSTR(STR_SVT_COLLATE_STROKE));
    mp_Data[6] = CollatorRessourceData (ASCSTR("zhuyin"),
                                             RESSTR(STR_SVT_COLLATE_ZHUYIN));
}

CollatorRessource::~CollatorRessource()
{
    delete mp_Data;
}

const String&
CollatorRessource::GetTranslation (const String &r_Algorithm)
{
    xub_StrLen nIndex = r_Algorithm.Search('.');
    String aLocaleFreeAlgorithm;

    if (nIndex == STRING_NOTFOUND)
    {
        aLocaleFreeAlgorithm = r_Algorithm;
    }
    else
    {
        nIndex += 1;
        aLocaleFreeAlgorithm = String(r_Algorithm, nIndex, r_Algorithm.Len() - nIndex);
    }

    for (sal_uInt32 i = 0; i < (sizeof(mp_Data) / sizeof(mp_Data[0])); i++)
    {
        if (aLocaleFreeAlgorithm == mp_Data[i].GetAlgorithm())
            return mp_Data[i].GetTranslation();
    }

    return r_Algorithm;
}

