import com.sun.star.lang.Locale;

public class XMeaning_impl implements
    com.sun.star.linguistic2.XMeaning
{
    String      aMeaning;
    String[]    aSynonyms;

    public XMeaning_impl ( String aMeaning, String[] aSynonyms )
    {
        this.aMeaning   = aMeaning;
        this.aSynonyms  = aSynonyms;

        //!! none of these cases should ever occur!
        //!! values provided only for safety
        if (this.aMeaning == null)
            this.aMeaning = new String();

        // a meaning without synonyms may be OK though.
        // still for safety an empty existing array has to be provided.
        if (this.aSynonyms == null)
            this.aSynonyms = new String[]{};
    }

    // XMeaning
    public String getMeaning() throws com.sun.star.uno.RuntimeException
    {
        return aMeaning;
    }
    public String[] querySynonyms() throws com.sun.star.uno.RuntimeException
    {
        return aSynonyms;
    }
};

