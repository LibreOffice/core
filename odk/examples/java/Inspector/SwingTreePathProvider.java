import javax.swing.tree.TreePath;


public class SwingTreePathProvider implements XTreePathProvider {
    TreePath m_aTreePath;

    /** Creates a new instance of TreePathProvider */
    public SwingTreePathProvider(TreePath _aTreePath) {
        m_aTreePath = _aTreePath;
    }


    public UnoNode getLastPathComponent(){
        return (UnoNode) m_aTreePath.getLastPathComponent();
    }


    public UnoNode getPathComponent(int i){
        return (UnoNode) m_aTreePath.getPathComponent(i);
    }

    public int getPathCount(){
        return m_aTreePath.getPathCount();
    }


    public XTreePathProvider getParentPath(){
        return new SwingTreePathProvider(m_aTreePath.getParentPath());
    }


    public XTreePathProvider pathByAddingChild(UnoNode _oUnoNode){
        TreePath aTreePath = m_aTreePath.pathByAddingChild(_oUnoNode);
        return new SwingTreePathProvider(aTreePath);
    }

    public TreePath getSwingTreePath(){
        return m_aTreePath;
    }
}
