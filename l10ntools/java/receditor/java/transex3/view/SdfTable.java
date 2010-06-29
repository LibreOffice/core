package transex3.view;

import javax.swing.JTable;

class SdfTable extends JTable{
    //private String tableId;
    public SdfTable( Object[][] obj1 , Object[] obj2){
        super(obj1,obj2);
    }
    //@Override
    public boolean isCellEditable(int row, int col) {
        if( row == 0 && col == 0 || row == 0 && col == 1 || row == 0 && col == 2 || row == 0 && col == 3 || row == 0 && col == 4 )
            return false;
        else
            return true;
    }
    /*public String getTableId() {
        return tableId;
    }
    public void setTableId(String tableId) {
        this.tableId = tableId;
    }*/

}