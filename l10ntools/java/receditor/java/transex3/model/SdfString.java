package transex3.model;
import java.util.*;
import java.io.*;
public class SdfString {
    private SdfEntity sourceString                  = null;
    //private java.util.HashMap languageStrings     = new HashMap();
    private Vector languageList                     = new Vector();
    private String id                               = null;
    private String fileid                           = null;
    private String filename                         = null;
    private String modulename                       = null;
    private String filepath                         = null;

    /*public HashMap getLanguageStrings() {
        return languageStrings;
    }*/
    public Vector getLanguageStrings() {
        return languageList;
    }

    public void setLanguageStrings(Vector languageStrings) {
        this.languageList = languageStrings;
    }

    public void addSourceString( SdfEntity aSdfEntity )
    {
        if( id == null )
            id = aSdfEntity.getId();
        if ( fileid == null )
            fileid = aSdfEntity.getFileId();
        if( modulename == null )
            modulename = aSdfEntity.getProject();
        if( filename == null )
            filename = aSdfEntity.getSource_file();
        if( filepath == null )
            filepath = aSdfEntity.getResourcePath();
        setSourceString( aSdfEntity );
    }
    public void addLanguageString( SdfEntity aSdfEntity ){
        if( !aSdfEntity.getLangid().equals( "en-US" ) )
        {
            if( id == null )
                id = aSdfEntity.getId();
            if ( fileid == null )
                fileid = aSdfEntity.getFileId();
            if( modulename == null )
                modulename = aSdfEntity.getProject();
            if( filename == null )
                filename = aSdfEntity.getSource_file();
            if( filepath == null )
                filepath = aSdfEntity.getResourcePath();

            //if( aSdfEntity.getLangid().equals( "en-US" ) )
            //{
            //    setSourceString( aSdfEntity );
            //}
            //else
            //{
            //languageStrings.put( aSdfEntity.getLangid() , aSdfEntity );
            languageList.add( aSdfEntity );
            //}
        id = aSdfEntity.getId();
        }
    }

    public SdfEntity getSourceString() {
        return sourceString;
    }

    public void setSourceString(SdfEntity sourceString) {
        this.sourceString = sourceString;
        id = sourceString.getId();
    }
    public String getFilePath(){
        return filepath;
    }
    public String getId(){
        //return id;
        return sourceString.getId();
    }
    public String getFileId(){
        return fileid;
    }

    public String getFileName() {
        return filename;
    }

    public void setFileName(String filename) {
        this.filename = filename;
    }

    public String getModuleName() {
        return modulename;
    }

    public void setModuleName(String modulename) {
        this.modulename = modulename;
    }
    /*
    public String getRealFileName(){
        //String srcroot   = "U:\\cws01\\l10ncleanup\\SRC680\\src.m213";//java.lang.System.getProperty( "SOLARSRC");
        //String isWindows = "4nt";//java.lang.System.getProperty( "USE_SHELL");
        String srcroot   = java.lang.System.getProperty( "SOLARSRC" );
        String isWindows = java.lang.System.getProperty( "USE_SHELL" );

        //System.out.println("srcroot="+srcroot+" isWindows="+isWindows );
        //if( true ) return;
        //String relpath   = sourceString.getFileId().substring( sourceString.getFileId().lastIndexOf("\\")
        //                                                   , sourceString.getFileId().length() );
        String filename;
        if( isWindows != null && isWindows.compareTo( "4nt") == 0 )
        {
            filename = srcroot + "\\" + //sourceString.getProject() + "\\" +
                       sourceString.getFileId() +".recommand";
        }
        else
        {
            String filepart = sourceString.getFileId();
            filepart = filepart.replaceAll( "\\\\" , "/" );
            filename = srcroot + "/" + //sourceString.getProject() + "//" +
                       filepart +".recommand";
        }
        return filename;
    }*/
    public String getRealFileName(){
        String filepart = sourceString.getFileId();
        filepart = filepart.replaceAll( "\\\\" , "_" );
        String filename = "/so/ws/merge/In/" +  java.lang.System.getProperty( "WORK_STAMP" ) + "/" + filepart + ".sdf";
        return filename;
    }
    public void removeFile(){
        String filename = getRealFileName();
        File aFile = new File( filename );
        if( aFile.exists() ){
            if( ! aFile.delete() )
            {
                System.out.println("Can't delete File "+filename+"\nWrong access rights?\n");
            }
        }
    }
    public void writeString(){
        String filename = getRealFileName();
        try {
            if( languageList.size() > 0 )
            {
                System.out.print("\nWrite to "+filename );
                BufferedWriter aBW = new BufferedWriter( new FileWriter( filename , true) );
                aBW.write( sourceString + "\n" );
                Iterator aIter = languageList.iterator();
                while( aIter.hasNext() ){
                    SdfEntity aEntity = (SdfEntity)aIter.next();
                    aBW.write( sourceString.getProject()+"\t"   );
                    aBW.write( sourceString.getSource_file()+"\t"   );
                    aBW.write( sourceString.getDummy1()+"\t"    );
                    aBW.write( sourceString.getResource_type()+"\t" );
                    aBW.write( sourceString.getGid()+"\t"       );
                    aBW.write( sourceString.getLid()+"\t"       );
                    aBW.write( sourceString.getHelpid()+"\t"    );
                    aBW.write( sourceString.getPlatform()+"\t"  );
                    aBW.write( sourceString.getDummy2()+"\t"    );
                    if( aEntity.getLangid() == null )
                        aBW.write( "\t"  );
                    else
                        aBW.write( aEntity.getLangid()+"\t"    );
                    if( aEntity.getText() == null )
                        aBW.write( "\t"  );
                    else
                        aBW.write( aEntity.getText()+"\t"           );
                    if( aEntity.getHelptext() == null )
                        aBW.write( "\t"  );
                    else
                        aBW.write( aEntity.getHelptext()+"\t"       );
                    if( aEntity.getQuickhelptext() == null )
                        aBW.write( "\t"  );
                    else
                        aBW.write( aEntity.getQuickhelptext()+"\t"  );
                    if( aEntity.getTitle() == null )
                        aBW.write( "\t"  );
                    else
                        aBW.write( aEntity.getTitle()+"\t"          );
                    aBW.write( "2002-02-02 02:02:02\n"          );
                }
                aBW.close();
            }
        } catch (IOException e) {
                // TODO Auto-generated catch block
                //e.printStackTrace();
                System.out.println("\nERROR: Can't write to file '"+filename+"'\nPlease contact RE/Tooling!");
        }
    }
}
