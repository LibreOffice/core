package transex3.controller;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.*;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.Runtime;
import java.util.*;

import javax.swing.ListSelectionModel;
import javax.swing.WindowConstants;
import javax.swing.event.*;

import transex3.model.*;

import java.io.*;
import javax.swing.*;
//import transex3.model.*;
public class EditorController {
    public final String[] RESTYPES = { ".src",".hrc",".xcu",".xrm",".xhp" };
    public final String   RECFILE  =   ".recommand";
    // Editor View
    static transex3.view.Editor aEditor                 = null;
    // Editor Model
    static Vector   sdfstrings                  = new Vector();
    static HashMap hashedsdfstrings         = new HashMap();
    int oldindex = 0;
    //HashMap hashedfilenames               = new HashMap();
    // Search for source Strings
    public String fetchSourceStrings( String rootdir ){

        //String outputfile = "h:\\workspace\\recommandEditor\\null2";
        File tempfile = null;

        try {
            tempfile = File.createTempFile( "receditor" , "tmp" );
        } catch (IOException e1) {
            // TODO Auto-generated catch block
            System.err.println("Can not create temp file\n");
            e1.printStackTrace();
        }

        String outputfile = tempfile.getAbsolutePath();
        try
        {
            //System.out.println("localize_sl -QQ -skip_links -e -l en-US -f "+outputfile+" -d "+rootdir);
            System.out.println("localize_sl -QQ -skip_links -e -l en-US -f "+outputfile );
            java.lang.Process aProc = Runtime.getRuntime().exec("localize_sl -QQ -skip_links -e -l en-US -f "+outputfile);

            //java.lang.Process aProc = Runtime.getRuntime().exec("localize_sl -QQ -e -l en-US -f "+outputfile+" -d "+rootdir);
            BufferedReader aBR = new BufferedReader( new InputStreamReader( aProc.getInputStream() ) );
            String line = aBR.readLine();
            while( line != null && line.length() > 0 ){
                //System.out.print( line );
                line = aBR.readLine();
            }

        } catch (IOException e) {
        // TODO Auto-generated catch block
        e.printStackTrace();
        }
        return outputfile;
    }
    // Search for recommand files
    public void findRecommandFiles( File rootdir , Vector list ){
        System.out.print(".");
        System.out.flush();
        File[] aFileArray = rootdir.listFiles();
        if( aFileArray != null ){
            for( int cnt = 0; aFileArray.length > cnt ; cnt++ ){
                if( aFileArray[ cnt ].isDirectory() && !aFileArray[ cnt ].getAbsolutePath().endsWith(".lnk") )
                    findRecommandFiles( aFileArray[ cnt ] , list);
                else if( aFileArray[ cnt ].isFile() && isRecommandFile( aFileArray[ cnt ] ) )
                    list.add( aFileArray[ cnt ]);
            }
        }
    }
    private boolean isResourceType( File aFile ){
        String filename     = aFile.getName();
        boolean isResType   = false;
        for(int cnt = 0; cnt < RESTYPES.length ; cnt++){
            if( filename.endsWith( RESTYPES[ cnt ] ) )
                    isResType = true;
        }
        return isResType;
    }
    private boolean isRecommandFile( File aFile ){
        return aFile.getName().endsWith( RECFILE );
    }
    public void clearAllRows( JTable aTable ){
        for ( int n = 0; n < aTable.getRowCount() ; n++ ){
            aTable.setValueAt( null , n , 0 );
            aTable.setValueAt( null , n , 1 );
            aTable.setValueAt( null , n , 2 );
            aTable.setValueAt( null , n , 3 );
            aTable.setValueAt( null , n , 4 );
        }
    }
    // Add all data to view
    void updateData(){
        JTable recTable =transex3.controller.EditorController.aEditor.getRectable();

        SdfString aSdfString = (SdfString) sdfstrings.get( oldindex );
        Vector newStrings = new Vector();
        for ( int n = 1; n < recTable.getRowCount() ; n++ ){
            String lang     = (String) recTable.getValueAt(n , 0 );
            String text     = (String) recTable.getValueAt(n , 1 );
            String htext    = (String) recTable.getValueAt(n , 2 );
            String qhtext   = (String) recTable.getValueAt(n , 3 );
            String ttext    = (String) recTable.getValueAt(n , 4 );
            if( lang != null && text != null ){
                //System.out.println("Data "+ lang + " " + text );
                SdfEntity aSdfEntity = new SdfEntity();
                aSdfEntity.setLangid( lang );
                aSdfEntity.setText(  text );
                aSdfEntity.setHelptext( htext );
                aSdfEntity.setQuickhelptext( qhtext );
                aSdfEntity.setTitle( ttext );
                newStrings.add( aSdfEntity );
                aSdfString.setLanguageStrings( newStrings );
            }
        }
    }

    public void initView(){
        Object[][]  sourceStringData = new Object[ sdfstrings.size() ][ 4 ];
        Object[][] firstData = new Object[100][5];
        // Set the files
        Iterator aIter = sdfstrings.iterator();
        int counter = 0;
        while( aIter.hasNext() ){
            SdfString aSdfString = (SdfString) aIter.next();
            sourceStringData[ counter ][ 0 ] = aSdfString.getSourceString().getProject()+"\\"+aSdfString.getSourceString().getSource_file();
            sourceStringData[ counter ][ 1 ] = aSdfString.getSourceString().getGid();
            sourceStringData[ counter ][ 2 ] = aSdfString.getSourceString().getLid();
            sourceStringData[ counter ][ 3 ] = aSdfString.getSourceString().getText();
            if( counter == 0 ){
                firstData[ 0 ][ 0 ] = "en-US";
                firstData[ 0 ][ 1 ] = aSdfString.getSourceString().getText();
                firstData[ 0 ][ 2 ] = aSdfString.getSourceString().getHelptext();
                firstData[ 0 ][ 3 ] = aSdfString.getSourceString().getQuickhelptext();
                firstData[ 0 ][ 4 ] = aSdfString.getSourceString().getTitle();
                aSdfString = (SdfString) sdfstrings.get( 0 );
                Vector values = aSdfString.getLanguageStrings();
                for( int n = 0; n < values.size() ; n++ )
                {
                    SdfEntity aEntity = (SdfEntity) values.get( n );
                    firstData[ n+1 ][ 0 ] = aEntity.getLangid();
                    firstData[ n+1 ][ 1 ] = aEntity.getText();
                    firstData[ n+1 ][ 2 ] = aEntity.getHelptext();
                    firstData[ n+1 ][ 3 ] = aEntity.getQuickhelptext();
                    firstData[ n+1 ][ 4 ] = aEntity.getTitle();
                }
            }
            counter++;
        }
        // Set the source srtings


        //aEditor = new transex3.view.Editor( sourceStringData , filedata.toArray() );
        aEditor = new transex3.view.Editor( sourceStringData , firstData );

        aEditor.setBounds(100,100,800,900);
        aEditor.setDefaultCloseOperation( WindowConstants.DISPOSE_ON_CLOSE );
        aEditor.setVisible(true);
        aEditor.repaint();
        aEditor.addWindowListener( new WindowAdapter(){
            public void windowClosed(WindowEvent e ){
                System.exit( 0 );
            }
        });

        aEditor.getMiExit().addActionListener( new ActionListener(){
            public void actionPerformed( ActionEvent e ){
                System.exit( 0 );
            }
        });

        aEditor.getMiSave().addActionListener( new ActionListener(){
            public void actionPerformed( ActionEvent e ){
                Iterator aIter = sdfstrings.iterator();
                String lastFile="";
                while( aIter.hasNext() )
                {
                    SdfString aSdfString = (SdfString )aIter.next();
                    if( aSdfString.getFileId().compareTo( lastFile ) !=  0 ){
                        //aSdfString.removeFile();
                    }
                    aSdfString.writeString();
                    lastFile = aSdfString.getFileId();
                }
            }
        });

        //aEditor.getRectable().putClientProperty("terminateEditOnFocusLost", Boolean.TRUE);
        aEditor.getRectable().addFocusListener( new FocusListener(){
            public void focusLost(FocusEvent e){
                //super.focusLost( e );
                //System.out.println("focus lost");
                JTable aTable = aEditor.getRectable();
                if(  aTable.getSelectedRow() != -1 && aTable.getSelectedColumn() != -1 )
                    aTable.getCellEditor( aTable.getSelectedRow(), aTable.getSelectedColumn() ).stopCellEditing();
                updateData();
            }
            public void focusGained( FocusEvent e){
                //super.focusGained( e );
                //System.out.println("focus gained");
            }
        });
        //setDefaultEditor(Object.class, new transex3.view.FocusCellEditor(new JTextField()));

        aEditor.getRectable().getModel().addTableModelListener( new TableModelListener() {
            public void tableChanged( TableModelEvent e ){
                //System.out.println( e );
            }});


        aEditor.getRectable().getSelectionModel().addListSelectionListener( new  ListSelectionListener(){
            public void valueChanged( ListSelectionEvent e ){
                JTable aTable = aEditor.getRectable();
                //if(  aTable.getSelectedRow() != -1 && aTable.getSelectedColumn() != -1 )
                    //aTable.getCellEditor( aTable.getSelectedRow(), aTable.getSelectedColumn() ).stopCellEditing();

                updateData();
            }
        });

        aEditor.getTable().setSelectionMode( ListSelectionModel.SINGLE_SELECTION );
        aEditor.getTable().getSelectionModel().addListSelectionListener( new  ListSelectionListener(){
            public void valueChanged( ListSelectionEvent e ){
                //System.out.println("Selected = " +e.getFirstIndex()+"\n");
                JTable table =transex3.controller.EditorController.aEditor.getTable();
                JTable recTable =transex3.controller.EditorController.aEditor.getRectable();
                SdfString aSdfString;
                JTable aTable = aEditor.getRectable();
                if(  aTable.getSelectedRow() != -1 && aTable.getSelectedColumn() != -1 )
                    aTable.getCellEditor( aTable.getSelectedRow(), aTable.getSelectedColumn() ).stopCellEditing();

                updateData();
                clearAllRows( recTable );

                aSdfString = (SdfString) sdfstrings.get( table.getSelectedRow() );
                recTable.setValueAt( "en-US" , 0, 0 );
                recTable.setValueAt( aSdfString.getSourceString().getText()          , 0, 1 );
                recTable.setValueAt( aSdfString.getSourceString().getHelptext()      , 0, 2 );
                recTable.setValueAt( aSdfString.getSourceString().getQuickhelptext() , 0, 3 );
                recTable.setValueAt( aSdfString.getSourceString().getTitle()         , 0, 4 );
                Vector values = aSdfString.getLanguageStrings();
                for( int n = 0; n < values.size() ; n++ )
                {
                    SdfEntity aEntity = (SdfEntity) values.get( n );
                    recTable.setValueAt( aEntity.getLangid()        , n+1 , 0 );
                    recTable.setValueAt( aEntity.getText()          , n+1 , 1 );
                    recTable.setValueAt( aEntity.getHelptext()      , n+1 , 2 );
                    recTable.setValueAt( aEntity.getQuickhelptext() , n+1 , 3 );
                    recTable.setValueAt( aEntity.getTitle()         , n+1 , 4 );
                }
                oldindex = table.getSelectedRow();
            }
        });
        //System.out.println("initView successfully");
    }
    public void initInitialStrings(){
        String rootdir = java.lang.System.getProperty("SOLARSRC");
        String sourcestringsfile = null;
        Vector recList = new Vector();
        sourcestringsfile = fetchSourceStrings( rootdir );
        //findRecommandFiles( new File( rootdir )  , recList );
        readStrings( sourcestringsfile , recList );
        File sfile = new File ( sourcestringsfile );
        sfile.delete();
        initView();
        aEditor.repaint();
    }
    // Connect recommand strings with source strings
    public void readStrings( String sourcefiles , Vector recfiles ) {
        BufferedReader aBR              = null;
        try {
            //System.out.println("DBG: sourcefiles = " +sourcefiles);
            aBR = new BufferedReader( new FileReader( sourcefiles ) );
            String current = aBR.readLine();
            SdfString aSdfString            = null;
            SdfEntity aSdfEntity            = null;
            while( current != null ){
                aSdfEntity = new SdfEntity();
                aSdfEntity.setProperties( current );
                aSdfString = new SdfString();
                aSdfString.addSourceString( aSdfEntity );
                hashedsdfstrings.put( aSdfString.getId() , aSdfString );
                //System.out.println("Put ID '"+aSdfString.getId()+"'");
                sdfstrings.add( aSdfString );
                current = aBR.readLine();

            }
            Iterator aIter=recfiles.iterator();
            File aFile;
            BufferedReader aBR2             = null;
            //System.out.println("Connecting strings");
            while( aIter.hasNext() ){
                aFile = (File) aIter.next();
                aBR2 = new BufferedReader( new FileReader( aFile ) ) ;
                String current2 = aBR2.readLine();

                while ( current2 != null ){
                    SdfEntity aEntity = new SdfEntity();
                    aEntity.setProperties( current2 );

                    if( hashedsdfstrings.containsKey( aEntity.getId() ) )
                    {
                        aSdfString = (SdfString) hashedsdfstrings.get( aEntity.getId() );
                        aSdfString.addLanguageString( aEntity );
                    }
                    else
                    {
                        System.out.println("DBG: Can't find source string '"+aEntity.getId()+"'" );
                    }
                    current2 = aBR2.readLine();
                }
            }
        } catch (FileNotFoundException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch ( IOException e){
            e.printStackTrace();
        }

    }
}
