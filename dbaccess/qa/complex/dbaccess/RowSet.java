/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RowSet.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 12:01:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
package complex.dbaccess;

import com.sun.star.awt.XWindow;
import com.sun.star.frame.*;
// import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCloseable;
import com.sun.star.beans.*;
import com.sun.star.lang.*;
import com.sun.star.sdbcx.*;
import com.sun.star.sdbc.*;
import com.sun.star.sdb.*;
import com.sun.star.container.*;
import com.sun.star.lang.XMultiServiceFactory;

import complexlib.ComplexTestCase;

import java.io.PrintWriter;
import java.io.File;
import java.util.Random;

import util.utils;
import util.dbg;


public class RowSet extends ComplexTestCase {

    static final int MAX_TABLE_ROWS = 100;
    static final int MAX_FETCH_ROWS = 10;

    XConnection con;
    String tempFileName;
    XDataSource xDoc;

    class CloneThread implements Runnable
    {
        XResultSet clone;
        XRow row;
        int id;
        public CloneThread(XResultSet _clone,int _id) throws java.lang.Exception {
            clone = _clone;
            row = (XRow)UnoRuntime.queryInterface(XRow.class,clone);
            id = _id;
        }
        public void run()
        {
            try{
                clone.beforeFirst();
                for ( int i = 0; i < MAX_TABLE_ROWS && !clone.isAfterLast(); ++i ){
                    boolean move = clone.next();
                    if ( move ){
                        int pos = clone.getRow();
                        int val = row.getInt(1);
                        log.println("Clone Move(" + id +") Pos: " + pos + " Val: " + val);
                        testPosition(clone,row,"Clone Move(" + id +")");
                        int pos2 = clone.getRow();
                        log.println("Clone GetR(" + id +") Pos: " + pos2 + " Val: " + val);
                        assure("CloneThread wrong position: " + i + " Pos1: " + pos + " Pos2: " + pos2,pos == pos2);
                    }
                }
            }catch(AssureException e){
            }catch(Exception e){
                assure("CloneThread(" + id + ") failed: " + e,false);
            }
        }
    }

    private void createTestCase(){
        if ( xDoc != null )
            return;

        tempFileName = new String();
        try{
            String str = File.createTempFile("testdb",".odb").getCanonicalPath();
            str = str.replaceAll(" ","%20");
            str = "file:///" +str;
            tempFileName = str.replace('\\','/');
           //tempFileName = java.net.URI.create(str).getPath();
        }catch(java.io.IOException e){

        }
        try{

                XOfficeDatabaseDocument xDoc = (XOfficeDatabaseDocument)UnoRuntime.queryInterface(XOfficeDatabaseDocument.class,
                                                ((XMultiServiceFactory)param.getMSF()).createInstance("com.sun.star.sdb.OfficeDatabaseDocument"));

            /*    xDoc = (XDataSource)UnoRuntime.queryInterface(XDataSource.class,
                                                ((XMultiServiceFactory)param.getMSF()).createInstance("com.sun.star.sdb.OfficeDatabaseDocument"));
            */
                XDataSource xDs = xDoc.getDataSource();
                //XDataSource xDs = xDoc;
                XPropertySet xProp = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class,xDs);
                xProp.setPropertyValue("URL", "sdbc:embedded:hsqldb");
                XStorable xStore = (XStorable)UnoRuntime.queryInterface(XStorable.class,xDoc);

                xStore.storeAsURL(tempFileName,new PropertyValue[]{});

                con = xDs.getConnection(new String(),new String());
                createStruture(con);
        }catch(AssureException e){
        }catch(Exception e){
            assure("Exception catched: " + e.getMessage(),false);
        }
    }


    public String[] getTestMethodNames() {
        return new String[] { "testRowSet" ,"testRowSetEvents" };
    }

    public String getTestObjectName() {
        return "RowSet";
    }

    private XRowSet createRowSet() throws java.lang.Exception {
        XRowSet rowset = (XRowSet)UnoRuntime.queryInterface(XRowSet.class,
                                                            ((XMultiServiceFactory)param.getMSF()).createInstance("com.sun.star.sdb.RowSet"));
        XPropertySet rowProp = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class,rowset);
        rowProp.setPropertyValue("Command","TEST1");
        rowProp.setPropertyValue("CommandType",new Integer(CommandType.TABLE));
        rowProp.setPropertyValue("ActiveConnection",con);
        //rowProp.setPropertyValue("FetchSize",new Integer(MAX_TABLE_ROWS));

        rowset.execute();
        return rowset;
    }
    public void testRowSet()  throws java.lang.Exception {

        log.println("testing testRowSet");
        createTestCase();

        XRowSet rowset = createRowSet();
        XResultSet rowSet = (XResultSet)UnoRuntime.queryInterface(XResultSet.class,rowset);
        XRow rowRow = (XRow)UnoRuntime.queryInterface(XRow.class,rowset);

        // 1st test
        rowSet.beforeFirst();
        test1(rowSet,rowRow);

        // 2nd test
        test2(rowSet,rowRow);

        // 3rd test
        {
            XResultSetAccess rowAcc = (XResultSetAccess)UnoRuntime.queryInterface(XResultSetAccess.class,rowset);
            XResultSet clone = rowAcc.createResultSet();
            test3(clone,rowSet);
        }
        // 4th test
        test4(rowSet);

        // test5(rowSet);
    }
    void createStruture(XConnection con) throws java.lang.Exception {
        {
            XStatement stmt = con.createStatement();
            stmt.execute("CREATE TABLE \"TEST1\" (\"ID\" integer not null primary key, \"col2\" varchar(50) )");
        }
        XPreparedStatement prep = con.prepareStatement("INSERT INTO \"TEST1\" values (?,?)");
        XParameters para = (XParameters)UnoRuntime.queryInterface(XParameters.class,prep);
        for(int i=1 ; i <= MAX_TABLE_ROWS ; ++i){
            para.setInt(1, i );
            para.setString(2, "Test" + i);
            prep.executeUpdate();
        }
    }

    void testPosition(XResultSet rowSet,XRow rowRow,String errorMsg) throws java.lang.Exception {
        int val = rowRow.getInt(1);
        int pos = rowSet.getRow();
        assure(errorMsg + " Value is not identical: " + pos + " (Pos) != " + val + " (Val)",val == pos);
    }
    void test1(XResultSet rowSet,XRow rowRow){
        try{
            // 1st test
            int i=1;
            while(rowSet.next()){
                int val = rowRow.getInt(1);
                int pos = rowSet.getRow();
                assure("Value is not identical: " + i + " != " + val,val == i);
                assure("Row is not identical: " + i + " != " + pos,pos == i);
                ++i;
            }
        }catch(AssureException e){
        }catch(Exception e){
            assure("test1 failed: " + e,false);
        }
    }
    void test2(XResultSet rowSet,XRow rowRow){
        try{
            for(int i = 1 ; i <= MAX_FETCH_ROWS ; ++i){
                int calcPos = (MAX_TABLE_ROWS % i) + 1;
                if ( calcPos != 0 && rowSet.absolute(calcPos) ){
                    testPosition(rowSet,rowRow,"test2");
                }
            }
        }catch(AssureException e){
        }catch(Exception e){
            assure("test2 failed: " + e,false);
    }
    }
    void test3(XResultSet clone,XResultSet rowSet){
        try{
            XRow rowRow = (XRow)UnoRuntime.queryInterface(XRow.class,rowSet);
            XRow cloneRow = (XRow)UnoRuntime.queryInterface(XRow.class,clone);
            for(int i = 1 ; i <= MAX_FETCH_ROWS ; ++i){
                int calcPos = (MAX_TABLE_ROWS % i) + 1;
                if ( calcPos != 0 && clone.absolute(calcPos) ){
                    int val = cloneRow.getInt(1);
                    int pos = clone.getRow();
                    assure("Value is not identical: " + pos + " != " + val,val == pos);
                    test2(rowSet,rowRow);
                    test2(clone,cloneRow);
                }
            }
        }catch(AssureException e){
        }catch(Exception e){
            assure("test3 failed: " + e,false);
    }
    }
    void test4(XResultSet rowSet){
        try{
            XResultSetAccess rowAcc = (XResultSetAccess)UnoRuntime.queryInterface(XResultSetAccess.class,rowSet);

            XRow rowRow = (XRow)UnoRuntime.queryInterface(XRow.class,rowSet);
            rowSet.beforeFirst();

            for(int i = 1 ; i <= MAX_TABLE_ROWS ; ++i){
                rowSet.next();
                XResultSet clone = rowAcc.createResultSet();
                XRow cloneRow = (XRow)UnoRuntime.queryInterface(XRow.class,clone);
                int calcPos = MAX_TABLE_ROWS - 1;
                if ( calcPos != 0 && clone.absolute(calcPos) ){
                    int val = cloneRow.getInt(1);
                    int pos = clone.getRow();
                    assure("Value is not identical: " + pos + " != " + val,val == pos);
                    val = rowRow.getInt(1);
                    pos = rowSet.getRow();
                    assure("Value is not identical: " + pos + " != " + val,val == pos && val == i);
                }
            }
        }catch(AssureException e){
        }catch(Exception e){
            assure("test4 failed: " + e,false);
    }
    }

     void test5(XResultSet rowSet){
         log.println("testing Thread");
        try{
            XResultSetAccess rowAcc = (XResultSetAccess)UnoRuntime.queryInterface(XResultSetAccess.class,rowSet);

            XRow rowRow = (XRow)UnoRuntime.queryInterface(XRow.class,rowSet);
            rowSet.beforeFirst();
            Thread t1 = new Thread(new CloneThread( rowAcc.createResultSet(),1));
            t1.start();
            System.out.println("Start Thread 1");
/*
            Thread t2 = new Thread(new CloneThread( rowAcc.createResultSet(),2));
            t2.start();
            System.out.println("Start Thread 2");
 */
            Thread t3 = new Thread(new CloneThread( rowSet,3));
            t3.start();
            System.out.println("Start Thread 3");
            t1.join();
//            t2.join();
            t3.join();
        }catch(AssureException e){
        }catch(Exception e){
            e.printStackTrace();
            assure("test5 failed: " + e,false);
        }
    }
    public void testRowSetEvents() throws java.lang.Exception {
        log.println("testing RowSet Events");
        createTestCase();
        // first we create our RowSet object
        RowSetEventListener pRow = new RowSetEventListener(this);

        XRowSet rowset = createRowSet();
        XResultSet resSet = (XResultSet)UnoRuntime.queryInterface(XResultSet.class,rowset);
        XRow rowRow = (XRow)UnoRuntime.queryInterface(XRow.class,rowset);
        XColumnsSupplier colSup = (XColumnsSupplier)UnoRuntime.queryInterface(XColumnsSupplier.class,rowset);
        XPropertySet col = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class,colSup.getColumns().getByName("ID"));
        col.addPropertyChangeListener("Value", pRow);
        XPropertySet set = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class,resSet);
        set.addPropertyChangeListener("IsModified", pRow);
        set.addPropertyChangeListener("IsNew", pRow);
        set.addPropertyChangeListener("IsRowCountFinal", pRow);
        set.addPropertyChangeListener("RowCount", pRow);

        XRowSetApproveBroadcaster xApBroad = (XRowSetApproveBroadcaster)UnoRuntime.queryInterface(XRowSetApproveBroadcaster.class,resSet);
        xApBroad.addRowSetApproveListener(pRow);
        rowset.addRowSetListener(pRow);

        // do some movements to check if we got all notifications

        Class cResSet = java.lang.Class.forName("com.sun.star.sdbc.XResultSet");
        boolean moves[] = new boolean[9];
        for( int i = 0; i < moves.length; ++i)
            moves[i] = false;
        moves[RowSetEventListener.APPROVE_CURSOR_MOVE] = true;
        moves[RowSetEventListener.COLUMN_VALUE] = true;
        moves[RowSetEventListener.CURSOR_MOVED] = true;
        moves[RowSetEventListener.IS_ROW_COUNT_FINAL] = true;
        moves[RowSetEventListener.ROW_COUNT] = true;

        testCursorMove(resSet,cResSet.getMethod("afterLast",null),pRow,moves,null);

        moves[RowSetEventListener.IS_ROW_COUNT_FINAL] = false;
        moves[RowSetEventListener.ROW_COUNT] = false;
        testCursorMove(resSet,cResSet.getMethod("next",null),pRow,moves,null);
        testCursorMove(resSet,cResSet.getMethod("next",null),pRow,moves,null);
        testCursorMove(resSet,cResSet.getMethod("next",null),pRow,moves,null);
        testCursorMove(resSet,cResSet.getMethod("last",null),pRow,moves,null);
        testCursorMove(resSet,cResSet.getMethod("next",null),pRow,moves,null);
        testCursorMove(resSet,cResSet.getMethod("first",null),pRow,moves,null);
        testCursorMove(resSet,cResSet.getMethod("previous",null),pRow,moves,null);
        testCursorMove(resSet,cResSet.getMethod("next",null),pRow,moves,null);
        moves[RowSetEventListener.IS_MODIFIED] = true;
        XRowUpdate updRow = (XRowUpdate)UnoRuntime.queryInterface(XRowUpdate.class,resSet);
        updRow.updateString(2,"Test21");
        testCursorMove(resSet,cResSet.getMethod("next",null),pRow,moves,null);

        moves[RowSetEventListener.IS_MODIFIED] = false;
        Class cupd = java.lang.Class.forName("com.sun.star.sdbc.XResultSetUpdate");
        XResultSetUpdate upd = (XResultSetUpdate)UnoRuntime.queryInterface(XResultSetUpdate.class,resSet);
        testCursorMove(upd,cupd.getMethod("moveToInsertRow",null),pRow,moves,null);

        updRow.updateInt(1, MAX_TABLE_ROWS + 2);
        updRow.updateString(2, "HHHH");
        moves[RowSetEventListener.APPROVE_CURSOR_MOVE] = false;
        moves[RowSetEventListener.CURSOR_MOVED] = false;
        moves[RowSetEventListener.IS_MODIFIED] = true;
        moves[RowSetEventListener.IS_NEW] = true;
        moves[RowSetEventListener.ROW_COUNT] = true;
        moves[RowSetEventListener.APPROVE_ROW_CHANGE] = true;
        moves[RowSetEventListener.ROW_CHANGED] = true;

        testCursorMove(upd,cupd.getMethod("insertRow",null),pRow,moves,null);

        moves[RowSetEventListener.IS_NEW] = false;
        moves[RowSetEventListener.ROW_COUNT] = false;
        resSet.first();
        updRow.updateInt(1, MAX_TABLE_ROWS + 3);
        updRow.updateString(2, "__");
        testCursorMove(upd,cupd.getMethod("updateRow",null),pRow,moves,null);

        moves[RowSetEventListener.IS_NEW] = true;
        moves[RowSetEventListener.ROW_COUNT] = true;
        resSet.first();
        testCursorMove(upd,cupd.getMethod("deleteRow",null),pRow,moves,null);

        moves[RowSetEventListener.IS_NEW] = false;
        moves[RowSetEventListener.COLUMN_VALUE] = true;
        moves[RowSetEventListener.ROW_COUNT] = false;
        resSet.first();
        updRow.updateString(2,"Test21");
        testCursorMove(resSet,cResSet.getMethod("refreshRow",null),pRow,moves,null);

        resSet.first();
        updRow.updateString(2,"Test21");
        testCursorMove(upd,cupd.getMethod("cancelRowUpdates",null),pRow,moves,null);

        for( int i = 0; i < moves.length; ++i)
            moves[i] = false;
        moves[RowSetEventListener.APPROVE_CURSOR_MOVE] = true;
        moves[RowSetEventListener.COLUMN_VALUE] = true;
        moves[RowSetEventListener.CURSOR_MOVED] = true;

        Class cloc = java.lang.Class.forName("com.sun.star.sdbcx.XRowLocate");
        XRowLocate loc = (XRowLocate)UnoRuntime.queryInterface(XRowLocate.class,resSet);
        resSet.first();
        Object bookmark = loc.getBookmark();
        resSet.next();
        Object temp[] = new Object[1];
        temp[0] = bookmark;
        Class ctemp[] = new Class[1];
        ctemp[0] = Object.class;
        testCursorMove(loc,cloc.getMethod("moveToBookmark",ctemp),pRow,moves,temp);

        Object temp2[] = new Object[2];
        temp2[0] = bookmark;
        temp2[1] = new Integer(1);
        Class ctemp2[] = new Class[2];
        ctemp2[0] = Object.class;
        ctemp2[1] = int.class;
        testCursorMove(loc,cloc.getMethod("moveRelativeToBookmark",ctemp2),pRow,moves,temp2);

        for( int i = 0; i < moves.length; ++i)
            moves[i] = false;
        moves[RowSetEventListener.APPROVE_ROW_CHANGE] = true;
        moves[RowSetEventListener.ROW_CHANGED] = true;
        moves[RowSetEventListener.ROW_COUNT] = true;
        Class cdelRows = java.lang.Class.forName("com.sun.star.sdbcx.XDeleteRows");
        ctemp[0] = Object[].class;
        XDeleteRows delRows = (XDeleteRows)UnoRuntime.queryInterface(XDeleteRows.class,resSet);
        Object bookmarks[] = new Object[5];
        resSet.first();
        for ( int i = 0; i < bookmarks.length ; ++i ){
            resSet.next();
            bookmarks[i] = loc.getBookmark();
        }

        temp[0] = bookmarks;
        testCursorMove(delRows,cdelRows.getMethod("deleteRows",ctemp),pRow,moves,temp);

        // now destroy the RowSet
        XComponent xComp = (XComponent)UnoRuntime.queryInterface(XComponent.class,resSet);
        xComp.dispose();
    }
    private void testCursorMove(Object res
                                ,java.lang.reflect.Method _method
                                , RowSetEventListener _evt
                                , boolean _must[]
                                , Object args[]) throws java.lang.Exception {
        _evt.clearCalling();
        _method.invoke(res,args);

        log.println("testing events for " + _method.getName());
        int calling[] = _evt.getCalling();
        int pos = 1;
        assure("Callings are not in the correct order for APPROVE_CURSOR_MOVE " ,
                ( !_must[RowSetEventListener.APPROVE_CURSOR_MOVE] || calling[RowSetEventListener.APPROVE_CURSOR_MOVE] == -1) || calling[RowSetEventListener.APPROVE_CURSOR_MOVE] == pos++ );
        assure("Callings are not in the correct order for APPROVE_ROW_CHANGE" ,
                ( !_must[ RowSetEventListener.APPROVE_ROW_CHANGE] || calling[RowSetEventListener.APPROVE_ROW_CHANGE] == -1) || calling[RowSetEventListener.APPROVE_ROW_CHANGE] == pos++);
        assure("Callings are not in the correct order for COLUMN_VALUE" ,
                ( !_must[RowSetEventListener.COLUMN_VALUE] || calling[RowSetEventListener.COLUMN_VALUE] == -1) || calling[RowSetEventListener.COLUMN_VALUE] == pos++);
        assure("Callings are not in the correct order for CURSOR_MOVED" ,
                ( !_must[RowSetEventListener.CURSOR_MOVED] || calling[RowSetEventListener.CURSOR_MOVED] == -1) || calling[RowSetEventListener.CURSOR_MOVED] == pos++);
        assure("Callings are not in the correct order for ROW_CHANGED" ,
                ( !_must[ RowSetEventListener.ROW_CHANGED] || calling[RowSetEventListener.ROW_CHANGED] == -1) || calling[RowSetEventListener.ROW_CHANGED] == pos++);
        assure("Callings are not in the correct order for IS_MODIFIED" ,
                ( !_must[ RowSetEventListener.IS_MODIFIED] || calling[RowSetEventListener.IS_MODIFIED] == -1) || calling[RowSetEventListener.IS_MODIFIED] == pos++);
        assure("Callings are not in the correct order for IS_NEW" ,
                ( !_must[ RowSetEventListener.IS_NEW] || calling[RowSetEventListener.IS_NEW] == -1) || calling[RowSetEventListener.IS_NEW] == pos++);
        assure("Callings are not in the correct order for ROW_COUNT" ,
                ( !_must[ RowSetEventListener.ROW_COUNT] || calling[RowSetEventListener.ROW_COUNT] == -1) || calling[RowSetEventListener.ROW_COUNT] == pos++);
        assure("Callings are not in the correct order for IS_ROW_COUNT_FINAL" ,
                ( !_must[ RowSetEventListener.IS_ROW_COUNT_FINAL] || calling[RowSetEventListener.IS_ROW_COUNT_FINAL] == -1) || calling[RowSetEventListener.IS_ROW_COUNT_FINAL] == pos++);

        _evt.clearCalling();
    }

    protected void finalize() throws Throwable {
        ((XComponent)UnoRuntime.queryInterface(XComponent.class,con)).dispose();
        ((XComponent)UnoRuntime.queryInterface(XComponent.class,xDoc)).dispose();

        try{
            File file = new File(tempFileName);
            file.delete();
        } catch(Exception e){
            assure("Exception catched: " + e.getMessage(),false);
        }
        super.finalize();
    }
}
