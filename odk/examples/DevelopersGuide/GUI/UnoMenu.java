import com.sun.star.awt.MenuEvent;
import com.sun.star.awt.MenuItemStyle;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.WindowAttribute;
import com.sun.star.awt.WindowClass;
import com.sun.star.awt.XMenuBar;
import com.sun.star.awt.XMenuExtended;
import com.sun.star.awt.XMenuListener;
import com.sun.star.awt.XPopupMenu;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XTopWindow;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XFramesSupplier;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

public class UnoMenu extends UnoDialogSample implements XMenuListener {
private XTopWindow mxTopWindow = null;
    
public UnoMenu(XComponentContext _xContext, XMultiComponentFactory _xMCF) {
    super(_xContext, _xMCF);
}
    
    public static void main(String args[]){
        UnoMenu oUnoMenu = null;
        XComponent xComponent = null;
        try {        
        XComponentContext xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
        if(xContext != null )
            System.out.println("Connected to a running office ...");
        XMultiComponentFactory xMCF = xContext.getServiceManager();
        oUnoMenu = new UnoMenu(xContext, xMCF);
        oUnoMenu.mxTopWindow = oUnoMenu.showTopWindow( new Rectangle(100, 100, 500, 500));   //oUnoDialogSample.m_xWindowPeer,
        oUnoMenu.addMenuBar(oUnoMenu.mxTopWindow, oUnoMenu);
        }catch( Exception ex ) {
            ex.printStackTrace(System.out);
        }    
    }


    public XPopupMenu getPopupMenu(){
    XPopupMenu xPopupMenu = null;
    try{
        // create a popup menu
        Object oPopupMenu = m_xMCF.createInstanceWithContext("stardiv.Toolkit.VCLXPopupMenu", m_xContext);
        xPopupMenu = (XPopupMenu) UnoRuntime.queryInterface(XPopupMenu.class, oPopupMenu);
        XMenuExtended xMenuExtended = (XMenuExtended) UnoRuntime.queryInterface(XMenuExtended.class, xPopupMenu);

        xPopupMenu.insertItem((short) 0, "~First Entry", MenuItemStyle.AUTOCHECK, (short) 0);
        xPopupMenu.insertItem((short) 1, "First ~Radio Entry", (short) (MenuItemStyle.RADIOCHECK + MenuItemStyle.AUTOCHECK), (short) 1);
        xPopupMenu.insertItem((short) 2, "~Second Radio Entry", (short) (MenuItemStyle.RADIOCHECK + MenuItemStyle.AUTOCHECK), (short) 2);
        xPopupMenu.insertItem((short) 3, "~Third RadioEntry",(short) (MenuItemStyle.RADIOCHECK + MenuItemStyle.AUTOCHECK), (short) 3);
        xPopupMenu.insertSeparator((short)4);
        xPopupMenu.insertItem((short) 4, "F~ifth Entry", (short) (MenuItemStyle.CHECKABLE + MenuItemStyle.AUTOCHECK), (short) 5);
        xPopupMenu.insertItem((short) 5, "~Fourth Entry", (short) (MenuItemStyle.CHECKABLE + MenuItemStyle.AUTOCHECK), (short) 6);
        xPopupMenu.enableItem((short) 1, false);
        xPopupMenu.insertItem((short) 6, "~Sixth Entry", (short) 0, (short) 7);
        xPopupMenu.insertItem((short) 7, "~Close Dialog", (short) 0, (short) 8);
        xPopupMenu.checkItem((short) 2, true);
        xPopupMenu.addMenuListener(this);
    }catch( Exception e ) {
        throw new java.lang.RuntimeException("cannot happen...");    
    }
        return xPopupMenu;
    }


    public void addMenuBar(XTopWindow _xTopWindow, XMenuListener _xMenuListener){
    try{
        // create a menubar at the global MultiComponentFactory...
        Object oMenuBar = m_xMCF.createInstanceWithContext("stardiv.Toolkit.VCLXMenuBar", m_xContext);
        // add the menu items...
        XMenuBar xMenuBar = (XMenuBar) UnoRuntime.queryInterface(XMenuBar.class, oMenuBar);
        xMenuBar.insertItem((short) 0, "~First MenuBar Item", com.sun.star.awt.MenuItemStyle.AUTOCHECK, (short) 0);
        xMenuBar.insertItem((short) 1, "~Second MenuBar Item", com.sun.star.awt.MenuItemStyle.AUTOCHECK, (short) 1);
        xMenuBar.setPopupMenu((short) 0, getPopupMenu());
        xMenuBar.addMenuListener(_xMenuListener);
        _xTopWindow.setMenuBar(xMenuBar);
    }catch( Exception e ) {
        throw new java.lang.RuntimeException("cannot happen...");    
    }}

    protected void closeDialog(){
        XComponent xComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, mxTopWindow);
        if (xComponent != null){
            xComponent.dispose();
        }

        // to ensure that the Java application terminates
        System.exit( 0 );    
    }

    public XTopWindow showTopWindow( Rectangle _aRectangle){
    XTopWindow xTopWindow = null;
    try {
        // The Toolkit is the creator of all windows...
        Object oToolkit = m_xMCF.createInstanceWithContext("com.sun.star.awt.Toolkit", m_xContext);
        XToolkit xToolkit = (XToolkit) UnoRuntime.queryInterface(XToolkit.class, oToolkit);

        // set up a window description and create the window. A parent window is always necessary for this...
        com.sun.star.awt.WindowDescriptor aWindowDescriptor = new com.sun.star.awt.WindowDescriptor();
        // a TopWindow is contains a title bar and is able to inlude menus...
        aWindowDescriptor.Type = WindowClass.TOP; 
        // specify the position and height of the window on the parent window
        aWindowDescriptor.Bounds = _aRectangle;
        // set the window attributes...
        aWindowDescriptor.WindowAttributes = WindowAttribute.SHOW + WindowAttribute.MOVEABLE + WindowAttribute.SIZEABLE + WindowAttribute.CLOSEABLE;

        // create the window...
        XWindowPeer xWindowPeer = xToolkit.createWindow(aWindowDescriptor);
        XWindow xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, xWindowPeer);

        // create a frame and initialize it with the created window...
        Object oFrame = m_xMCF.createInstanceWithContext("com.sun.star.frame.Frame", m_xContext);
        m_xFrame = (XFrame) UnoRuntime.queryInterface(XFrame.class, oFrame);

        Object oDesktop = m_xMCF.createInstanceWithContext("com.sun.star.frame.Desktop", m_xContext); 
        XFramesSupplier xFramesSupplier = (XFramesSupplier) UnoRuntime.queryInterface(XFramesSupplier.class, oDesktop);
        m_xFrame.setCreator(xFramesSupplier);
        // get the XTopWindow interface..
        xTopWindow = (XTopWindow) UnoRuntime.queryInterface(XTopWindow.class, xWindow);
    } catch (com.sun.star.lang.IllegalArgumentException ex) {
        ex.printStackTrace();
    } catch (com.sun.star.uno.Exception ex) {
        ex.printStackTrace();
    }
        return xTopWindow;
    }

    public void addMenuBar(XWindow _xWindow){
        XTopWindow xTopWindow = (XTopWindow) UnoRuntime.queryInterface(XTopWindow.class, _xWindow);
        addMenuBar(xTopWindow, this);
    }

    public void select(MenuEvent menuEvent){
        // find out which menu item has been triggered,
        // by getting the menu-id...
        switch (menuEvent.MenuId){
            case 0:
                // add your menu-item-specific code here:
                break;
            case 1:
                // add your menu-item-specific code here:
                break;
            case 7:
                closeDialog();
            default:
                //..
        }
    }

    public void highlight(MenuEvent menuEvent) {
        int i = 0;
    }

    public void deactivate(MenuEvent menuEvent) {
        int i = 0;    }

    public void activate(MenuEvent menuEvent) {
        int i = 0;
    }

}
