import com.sun.star.uno.*;

/** a helper "enumeration class" for classifying a document type
*/
public class DocumentType extends com.sun.star.uno.Enum
{
    private DocumentType( int value )
    {
        super( value );
    }

    public static DocumentType getDefault()
    {
        return WRITER;
    }

    public static final DocumentType WRITER = new DocumentType(0);
    public static final DocumentType CALC = new DocumentType(1);
    public static final DocumentType DRAWING = new DocumentType(2);
    public static final DocumentType UNKNOWN = new DocumentType(-1);

    public static DocumentType fromInt(int value)
    {
        switch(value)
        {
            case 0: return WRITER;
            case 1: return CALC;
            case 2: return DRAWING;
            default: return UNKNOWN;
        }
    }
};

