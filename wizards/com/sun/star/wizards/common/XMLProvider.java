/*
 * XMLSupplier.java
 *
 * Created on 19. September 2003, 11:52
 */

package com.sun.star.wizards.common;

import org.w3c.dom.Node;
/**
 *
 * @author  rpiterman
 */
public interface XMLProvider {
    public Node createDOM(Node parent);
}
