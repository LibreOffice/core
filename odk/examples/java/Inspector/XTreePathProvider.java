/*
 * xTreePathProvider.java
 *
 * Created on 12. Dezember 2006, 13:30
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

/**
 *
 * @author bc93774
 */
public interface XTreePathProvider {
    
    public XUnoNode getLastPathComponent();
        
    public XUnoNode getPathComponent(int i);
    
    public int getPathCount();          
    
    public XTreePathProvider getParentPath();
    
    public XTreePathProvider pathByAddingChild(XUnoNode _oUnoNode);    
}
