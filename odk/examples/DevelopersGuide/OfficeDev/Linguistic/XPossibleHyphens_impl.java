import com.sun.star.lang.Locale;

public class XPossibleHyphens_impl implements
    com.sun.star.linguistic2.XPossibleHyphens
{
    String              aWord;
    String              aHyphWord;
    short[]             aOrigHyphenPos;
    Locale              aLang;

    public XPossibleHyphens_impl(
            String      aWord,
            Locale      aLang,
            String      aHyphWord,
            short[]     aOrigHyphenPos)
    {
        this.aWord = aWord;
        this.aLang = aLang;
        this.aHyphWord = aHyphWord;
        this.aOrigHyphenPos = aOrigHyphenPos;

        //!! none of these cases should ever occur!
        //!! values provided only for safety
        if (this.aWord == null)
            this.aWord = new String();
        if (this.aLang == null)
            this.aLang = new Locale();
        if (this.aHyphWord == null)
            this.aHyphWord = new String();

        // having no hyphenation positions is OK though.
        // still for safety an empty existing array has to be provided.
        if (this.aOrigHyphenPos == null)
            this.aOrigHyphenPos = new short[]{};
    }

    // XPossibleHyphens
    public String getWord() throws com.sun.star.uno.RuntimeException
    {
        return aWord;
    }

    public Locale getLocale() throws com.sun.star.uno.RuntimeException
    {
        return aLang;
    }
    public String getPossibleHyphens() throws com.sun.star.uno.RuntimeException
    {
        return aHyphWord;
    }
    public short[] getHyphenationPositions() throws com.sun.star.uno.RuntimeException
    {
        return aOrigHyphenPos;
    }
};
