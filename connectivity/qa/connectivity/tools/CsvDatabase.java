package connectivity.tools;

import com.sun.star.lang.XMultiServiceFactory;

public class CsvDatabase extends FlatFileDatabase
{
    // --------------------------------------------------------------------------------------------------------
    public CsvDatabase( final XMultiServiceFactory i_orb ) throws Exception
    {
        super( i_orb, "flat" );
    }

    // --------------------------------------------------------------------------------------------------------
    protected CsvDatabase( final XMultiServiceFactory i_orb, final String i_existingDocumentURL ) throws Exception
    {
        super( i_orb, i_existingDocumentURL, "flat" );
    }
}
