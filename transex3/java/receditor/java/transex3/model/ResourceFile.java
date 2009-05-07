package transex3.model;

import java.util.*;

public class ResourceFile {
    Vector      sdfStrings      = new Vector();
    HashMap     sdfHashMap      = new HashMap();
    String filepathid           = null;
    String modulename           = null;
    String filename             = null;


    public String getModuleName(){
        return modulename;
    }
    public String getFilePath(){
        return filepathid;
    }
    public String getFileName(){
        return filename;
    }
/*  public List readSoureStrings( java.io.File aSdfFile ){
        List sdfList=null;
        return sdfList;
    };*/
    public void addString( SdfString aSdfstring ){
        sdfStrings.add( aSdfstring );
        sdfHashMap.put( aSdfstring.getFileId() , aSdfstring );
        if( filepathid == null )
            filepathid = aSdfstring.getFilePath();
        if( modulename == null )
            modulename = aSdfstring.getModuleName();
        if( filename == null )
            filename = aSdfstring.getFileName();
    }


    public void ParseString( String aSourceString ){
        //sourceString          = new SdfEntity();
        SdfEntity aSdfEntity    = new SdfEntity();
        aSdfEntity.setProperties( aSourceString );
        SdfString sdfstring     = null;
        if( sdfHashMap.containsKey( aSdfEntity.getFileId() ) ){
            sdfstring = (SdfString) sdfHashMap.get( aSdfEntity.getFileId() );
        }
        else
        {
            sdfstring = new SdfString();
            addString( sdfstring );
        }
        sdfstring.addLanguageString( aSdfEntity );


    }
    /*public void ParseSdfFile( java.util.Vector aSdfList ){
        ListIterator aLI = aSdfList.listIterator();
        String current;
        String[] splitted;
        SdfEntity aSdfEntity;
        SdfString aSdfString = new SdfString();
        while( aLI.hasNext() ){
            aSdfEntity = new SdfEntity();
            aSdfEntity.setProperties( (String) aLI.next() );
            SdfString aString;

            if( sdfHashMap.containsKey( aSdfEntity.getFileId() ) )
                aString = (SdfString) sdfHashMap.get( aSdfEntity.getFileId() );
            else
            {
                aString = new SdfString();
                addString( aString );
            }
            aString.addLanguageString( aSdfEntity );
        }

    }*/
}
