package connectivity.tools;

import com.sun.star.lang.XMultiServiceFactory;

public class DbaseDatabase extends FlatFileDatabase
{
    // --------------------------------------------------------------------------------------------------------
    public DbaseDatabase( final XMultiServiceFactory i_orb ) throws Exception
    {
        super( i_orb, "dbase" );
    }

    // --------------------------------------------------------------------------------------------------------
    protected DbaseDatabase( final XMultiServiceFactory i_orb, final String i_existingDocumentURL ) throws Exception
    {
        super( i_orb, i_existingDocumentURL, "dbase" );
    }
}
