/*
 * XTreeControlProvider.java
 *
 * Created on 12. Dezember 2006, 11:42
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

/**
 *
 * @author bc93774
 */
public interface XTreeControlProvider {

    public static String sTitle = "User defined";

    public String enableFilterElements(UnoNode _oUnoNode);

    public void setSourceCode(String _sSourceCode);

    public Object inspect(java.lang.Object _oUserDefinedObject, String _sTitle) throws com.sun.star.uno.RuntimeException;

    public void nodeInserted(UnoNode _oParentNode, UnoNode _oChildNode, int index);

    public void nodeChanged(UnoNode _oNode);

    public boolean setNodeVisible(Object node, boolean v);

    public UnoNode getSelectedNode();

    public XTreePathProvider getSelectedPath();

    public void expandPath(XTreePathProvider xTreePathProvider) throws java.lang.ClassCastException;

    public void addTreeExpandListener();

    public void setMaximumOfProgressBar();

    public void addInspectorPane(InspectorPane _oInspectorPane);

    public boolean isPropertyNode(UnoNode _oUnoNode);

    public boolean isMethodNode(UnoNode _oUnoNode);

    public boolean isObjectNode(UnoNode _oUnoNode);

    public boolean isFacetteNode(UnoNode _oUnoNode);

    public boolean isServiceNode(UnoNode _oUnoNode);

    public boolean isInterfaceNode(UnoNode _oUnoNode);

//    public void setProgressValue(int _nValue, String _sTitle);

//    public void hideProgressPanel();

//    public SwingTreeControlProvider.ProgressDisplay getProgressDisplay();
}
