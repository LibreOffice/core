import com.sun.star.lang.Locale;

public class XHyphenatedWord_impl implements
    com.sun.star.linguistic2.XHyphenatedWord
{
    String     aWord;
    String     aHyphenatedWord;
    short      nHyphenPos;
    short      nHyphenationPos;
    Locale     aLang;
    boolean    bIsAltSpelling;

    public XHyphenatedWord_impl(
            String      aWord,
            Locale      aLang,
            short       nHyphenationPos,
            String      aHyphenatedWord,
            short       nHyphenPos )
    {
        this.aWord = aWord;
        this.aLang = aLang;
        this.nHyphenationPos = nHyphenationPos;
        this.aHyphenatedWord = aHyphenatedWord;
        this.nHyphenPos = nHyphenPos;
        this.bIsAltSpelling = (aWord != aHyphenatedWord);

        //!! none of these cases should ever occur!
        //!! values provided only for safety
        if (this.aWord == null)
            this.aWord = new String();
        if (this.aLang == null)
            this.aLang = new Locale();
        if (this.aHyphenatedWord == null)
            this.aHyphenatedWord = new String();
    }


    // XHyphenatedWord
    public String getWord() throws com.sun.star.uno.RuntimeException
    {
        return aWord;
    }
    public Locale getLocale() throws com.sun.star.uno.RuntimeException
    {
        return aLang;
    }
    public short getHyphenationPos() throws com.sun.star.uno.RuntimeException
    {
        return nHyphenationPos;
    }
    public String getHyphenatedWord() throws com.sun.star.uno.RuntimeException
    {
        return aHyphenatedWord;
    }
    public short getHyphenPos() throws com.sun.star.uno.RuntimeException
    {
        return nHyphenPos;
    }
    public boolean isAlternativeSpelling() throws com.sun.star.uno.RuntimeException
    {
        return bIsAltSpelling;
    }
};
