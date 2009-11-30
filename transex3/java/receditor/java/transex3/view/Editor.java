package transex3.view;
import javax.swing.*;
import java.awt.*;

public class Editor extends JFrame{
    Object[]    columnnames     = { "File" , "GID" , "LID" , "String" };
    Object[]    stringcolnames  = { "Language", "Text", "Helptext" , "Quickhelptext","Title"};
    //Object[][]    data            = new Object[4][1];//{  { "a " }, { "v " }, { "v " } , { "a " } };
    JTable      table           = null;
    JTable      rectable        = null;
    JComboBox   cBox            = null;
    JMenuBar    menubar         = null;
    JMenu       filemenu        = null;
    JMenuItem   miNew           = null;
    JMenuItem   miSave          = null;
    JMenuItem   miExit          = null;
    //JButton     button            = null;

    public Editor( Object[][] tabledata , Object[][] firstdata ){
        table                   = new JTable( tabledata , columnnames );
        rectable                = new SdfTable( firstdata , stringcolnames );
        menubar                 = new JMenuBar();
        filemenu                = new JMenu("File");
        //miNew                 = new JMenuItem("New");
        miSave                  = new JMenuItem("Save");
        miExit                  = new JMenuItem("Exit");
        //button                    = new JButton("Edit");
        //filemenu.add( miNew   );
        filemenu.add( miSave  );
        filemenu.add( miExit  );
        menubar.add( filemenu );

        Container contentPane   = getContentPane();
        //contentPane.add( new ControlPanel() , BorderLayout.NORTH );
        contentPane.add( menubar , BorderLayout.NORTH );
        //JPanel aPanel = new JPanel( new FlowLayout( FlowLayout.CENTER) );
        JPanel aPanel = new JPanel( new GridLayout( 2,1 ) );
        aPanel.add( new JScrollPane( table ) );
        aPanel.add( new JScrollPane( rectable ) );
        contentPane.add( aPanel , BorderLayout.CENTER );
        //contentPane.add( button , BorderLayout.SOUTH );
        //contentPane.add( new JScrollPane( table ), BorderLayout.CENTER );
        //contentPane.add( new JScrollPane( table ), BorderLayout.SOUTH );
        //contentPane.add( new JScrollPane( rectable ), BorderLayout.SOUTH );
        //contentPane.add( new JScrollPane( rectable ), BorderLayout.SOUTH );
        this.repaint();

    }

    public JTable getRectable() {
        return rectable;
    }

    public void setRectable(JTable rectable) {
        this.rectable = rectable;
    }

    public JTable getTable() {
        return table;
    }

    public void setTable(JTable table) {
        this.table = table;
    }

    /*public JButton getButton() {
        return button;
    }

    public void setButton(JButton button) {
        this.button = button;
    }*/

    public JMenuItem getMiExit() {
        return miExit;
    }

    public void setMiExit(JMenuItem miExit) {
        this.miExit = miExit;
    }

    public JMenuItem getMiSave() {
        return miSave;
    }

    public void setMiSave(JMenuItem miSave) {
        this.miSave = miSave;
    }

    /*public void setTableData(){

    }*/

}

//class ControlPanel extends JPanel{}

