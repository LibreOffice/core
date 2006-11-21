package convwatch;

import java.sql.Connection;
import java.util.StringTokenizer;
// import convwatch.DBHelper;

public class DB extends DBHelper
{
    private static String m_sReferenceVersion;
    private static String m_sCurrentVersion;
    private static String m_sDocumentPool;
    private static String m_sEnvironment;

    public static void startFile(String _sDBInfoString,
                                 String _sFilename)
        {
            if (_sDBInfoString == null) return;
            fillVariables(_sDBInfoString);
            updatestate_currentdocs(_sFilename, "started");
        }

    public static void ref_finishedFile(String _sDBInfoString,
                                    String _sFilename)
        {
            if (_sDBInfoString == null) return;
            fillVariables(_sDBInfoString);
            updatestate_currentdocs(_sFilename, "ref-finished");
        }

    public static void ref_failedFile(String _sDBInfoString,
                                      String _sFilename)
        {
            if (_sDBInfoString == null) return;
            fillVariables(_sDBInfoString);
            updatestate_currentdocs(_sFilename, "ref-failed");
        }


    public static void finishedFile(String _sDBInfoString,
                                    String _sFilename)
        {
            if (_sDBInfoString == null) return;
            fillVariables(_sDBInfoString);
            updatestate_currentdocs(_sFilename, "PASSED-OK");
        }

    public static void failedFile(String _sDBInfoString,
                                  String _sFilename)
        {
            if (_sDBInfoString == null) return;
            fillVariables(_sDBInfoString);
            updatestate_currentdocs(_sFilename, "PASSED-FAILED");
        }

    public static void reallyfailedFile(String _sDBInfoString,
                                  String _sFilename)
        {
            if (_sDBInfoString == null) return;
            fillVariables(_sDBInfoString);
            updatestate_currentdocs(_sFilename, "FAILED-FAILED");
        }

    public static void writeToDB(String _sDBInfoString,
                                 String _sFilename,
                                 String _sBasename,
                                 String _sFileFormat,
                                 String _sBuildID,
                                 String _sReferenceType,
                                 int _nResolution )
        {
            if (_sDBInfoString == null) return;
            fillVariables(_sDBInfoString);

        }

    private static String getEnvironment()
        {
            if (System.getProperty("os.name").toLowerCase().startsWith("windows"))
            {
                return "wntmsci";
            }
            else if (System.getProperty("os.name").toLowerCase().startsWith("solaris") &&
                System.getProperty("os.arch").equals("x86"))
            {
                return "unxsoli";
            }
            else if (System.getProperty("os.name").toLowerCase().startsWith("solaris") &&
                System.getProperty("os.arch").equals("sparc"))
            {
                return "unxsols";
            }
            else if (System.getProperty("os.name").toLowerCase().startsWith("linux") &&
                     System.getProperty("os.arch").equals("i386"))
            {
                return "unxlngi";
            }
            else
            {
                GlobalLogWriter.get().println("Unknown environment.");
                GlobalLogWriter.get().println("os.name := " + System.getProperty("os.name").toLowerCase());
                GlobalLogWriter.get().println("os.arch := " + System.getProperty("os.arch"));
                return "";
            }
        }

    private static void fillVariables(String _sInfo)
        {
            m_sEnvironment = getEnvironment();

            StringTokenizer aTokenizer = new StringTokenizer(_sInfo,",",false);
            while (aTokenizer.hasMoreTokens())
            {
                String sPart = aTokenizer.nextToken();
                if (sPart.startsWith("p:"))
                {
                    m_sReferenceVersion = sPart.substring(2);
                    GlobalLogWriter.get().println("Found previous version: " + m_sReferenceVersion);
                }
                else if (sPart.startsWith("c:"))
                {
                    m_sCurrentVersion = sPart.substring(2);
                    GlobalLogWriter.get().println("Found current version: " + m_sCurrentVersion);
                }
                else if (sPart.startsWith("d:"))
                {
                    m_sDocumentPool = sPart.substring(2);
                    GlobalLogWriter.get().println("Found documentpool version: " + m_sDocumentPool);
                }
                else
                {
                }
            }
        }

    public static void insertinto_file(String _sFilename,
                                       String _sBasename,
                                       String _sFileFormat,
                                       String _sBuildID,
                                       String _sReferenceType,
                                       int _nResolution )
        {
            Connection aCon = new ShareConnection().getConnection();

            String sFilename = _sFilename.replace('\\', '/');

            String sDeleteOld = "DELETE FROM file WHERE filename = " + Quote(sFilename);
            ExecSQL(aCon, sDeleteOld);

            String sValueLine = "type, filename, basename, fileformat, buildid, resolution, date";
            StringBuffer aDataLine = new StringBuffer();
            aDataLine.append( Quote(_sReferenceType) ) . append( sComma ) .
                append( Quote( sFilename) )   . append( sComma ) .
                append( Quote( _sBasename) )   . append( sComma ) .
                append( Quote( _sFileFormat) ) . append( sComma ) .
                append( Quote( _sBuildID) )    . append( sComma ) .
                append( _nResolution)    . append( sComma ) .
                append( Quote( today() ) );

            SQLinsertValues(aCon, "file", sValueLine, aDataLine.toString());
        }

    public static void updatestate_currentdocs(String _sFilename,
                                               String _sState)
        {
            Connection aCon = new ShareConnection().getConnection();

            String sFilename = _sFilename.replace('\\', '/');

            // String sDeleteOld = "DELETE FROM file WHERE filename = " + Quote(sFilename);
            // ExecSQL(aCon, sDeleteOld);

            String sSet = "state=" + Quote(_sState);
            String sWhere = getWhere() + sAND + "name=" + Quote(sFilename);
            SQLupdateValue( aCon, "currentdocs", sSet, sWhere );
        }

    public static String getWhere()
        {
            StringBuffer aWhere = new StringBuffer();
            // WHERE environment='' and referenceversion='' and currentversion='' and documentpool=''
            aWhere.append( "environment" ). append(sEqual) . append(Quote(m_sEnvironment)) .
                append(sAND) .
                append( "referenceversion" ). append(sEqual) . append(Quote(m_sReferenceVersion)) .
                append(sAND) .
                append( "currentversion" ). append(sEqual) . append(Quote(m_sCurrentVersion)) .
                append(sAND) .
                append( "documentpool" ). append(sEqual) . append(Quote(m_sDocumentPool));
            return aWhere.toString();
        }

    public static void filesRemove(String _sDBInfoString)
        {
            if (_sDBInfoString == null) return;
            fillVariables(_sDBInfoString);

            Connection aCon = new ShareConnection().getConnection();

            String sDeleteSQL = "DELETE FROM currentdocs WHERE " + getWhere();

            ExecSQL(aCon, sDeleteSQL);
        }

    public static void fileInsert(String _sDBInfoString,
                                  String _sFilename,
                                  String _sRemovePath)
        {
            if (_sDBInfoString == null) return;
            fillVariables(_sDBInfoString);

            String sFilename = _sFilename.replace('\\', '/');

            Connection aCon = new ShareConnection().getConnection();

            String sValueLine = "environment, referenceversion, currentversion, documentpool, name, state";
            StringBuffer aDataLine = new StringBuffer();
            aDataLine.append( Quote(m_sEnvironment) ) . append( sComma ) .
                append( Quote( m_sReferenceVersion) )   . append( sComma ) .
                append( Quote( m_sCurrentVersion) )   . append( sComma ) .
                append( Quote( m_sDocumentPool) ) . append( sComma ) .
                append( Quote( sFilename) )    . append( sComma ) .
                append( Quote( "undone"));

            SQLinsertValues(aCon, "currentdocs", sValueLine, aDataLine.toString());
        }

    // public static void insertinto_file(String _sFilename, String _sFileFormat, String _sBuildID)
    //     {
    //         Connection aCon = new ShareConnection().getConnection();
    //
    //         String sValueLine = "type, filename, fileformat, buildid, date";
    //         StringBuffer aDataLine = new StringBuffer();
    //         aDataLine.append( "1" )            . append( sComma ) .
    //             append( Quote( _sFilename) )   . append( sComma ) .
    //             append( Quote( _sFileFormat) ) . append( sComma ) .
    //             append( Quote( _sBuildID) )    . append( sComma ) .
    //             append( Quote( today() ) );
    //
    //         SQLinsertValues(aCon, "file", sValueLine, aDataLine.toString());
    //     }

    public static void main( String[] args )
        {

            String _sFilename = "";
            String _sFileFormat = "";
            String _sBuildID = "";

            // insertinto_file("c:\temp\test.txt", "test", "txt", "nix", "", 0);
            fillVariables("p:m128,c:m134,d:demo");
        }
}
