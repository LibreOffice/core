import com.sun.star.datatransfer.*;
import com.sun.star.datatransfer.clipboard.*;
import com.sun.star.uno.Type;

//---------------------------------------
// A simple transferable containing only
// one format, unicode text
//---------------------------------------

public class TextTransferable implements XTransferable
{
    public TextTransferable(String aText)
    {
        text = aText;
    }

    // XTransferable methods

    public Object getTransferData(DataFlavor aFlavor) throws UnsupportedFlavorException
    {
        if ( !aFlavor.MimeType.equalsIgnoreCase( UNICODE_CONTENT_TYPE ) )
            throw new UnsupportedFlavorException();

        return text;
    }

    public DataFlavor[] getTransferDataFlavors()
    {
        DataFlavor[] adf = new DataFlavor[1];

        DataFlavor uniflv = new DataFlavor(
            UNICODE_CONTENT_TYPE,
            "Unicode Text",
            new Type(String.class) );

        adf[0] = uniflv;

        return adf;
    }

    public boolean isDataFlavorSupported(DataFlavor aFlavor)
    {
        return aFlavor.MimeType.equalsIgnoreCase(UNICODE_CONTENT_TYPE);
    }

// members

    private final String text;
    private final String UNICODE_CONTENT_TYPE = "text/plain;charset=utf-16";
}
