/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/
import java.awt.Component;
import java.awt.Graphics;
import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JTree;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeCellRenderer;



public class UnoTreeRenderer extends DefaultTreeCellRenderer{
    private Icon m_oMethodIcon;
    private Icon m_oPropertyIcon;
    private Icon m_oContainerIcon;
    private Icon m_oContentIcon;
    private Icon m_oServiceIcon;
    private Icon m_oInterfaceIcon;
    private Icon m_oPropertyValueIcon;
    private boolean bSelected;
    private int nWidth = 0;


    /** Creates a new instance of UnoTreeRenderer */
    public UnoTreeRenderer(){
        super();
        try {

            final ClassLoader loader = ClassLoader.getSystemClassLoader();
            m_oMethodIcon = new ImageIcon(loader.getResource("images/methods_16.png"));
            m_oPropertyIcon = new ImageIcon("images/properties_16.png");
            m_oPropertyValueIcon = new ImageIcon("images/properties_16.png");
            m_oContainerIcon = new ImageIcon("images/containers_16.png");
            m_oServiceIcon = new ImageIcon("images/services_16.png");
            m_oInterfaceIcon = new ImageIcon("images/interfaces_16.png");
            m_oContentIcon = new ImageIcon("images/content_16.png");
        } catch (RuntimeException e) {
            System.out.println("Sorry, could not locate resources, treecell icons will not be displayed.");
        }
    }


    @Override
    public synchronized Component getTreeCellRendererComponent(JTree tree,Object value, boolean sel, boolean expanded, boolean leaf, int row, boolean hasFocus){
        try{
            bSelected = sel;
            DefaultMutableTreeNode node = (DefaultMutableTreeNode) value;
            Component rc = super.getTreeCellRendererComponent( tree, value, sel,expanded, leaf, row,hasFocus);
            String  sLabelText = (String)node.getUserObject();
            if (sLabelText != null){
                if (sLabelText.equals(XUnoFacetteNode.SCONTAINERDESCRIPTION)){
                } else if (sLabelText.equals(XUnoFacetteNode.SCONTENTDESCRIPTION)){
                } else if (sLabelText.equals(XUnoFacetteNode.SINTERFACEDESCRIPTION)){
                } else if (sLabelText.equals(XUnoFacetteNode.SMETHODDESCRIPTION)){
                } else if (sLabelText.equals(XUnoFacetteNode.SPROPERTYDESCRIPTION)){
                } else if (sLabelText.startsWith(XUnoFacetteNode.SPROPERTYINFODESCRIPTION)){
                } else if (sLabelText.equals(XUnoFacetteNode.SPROPERTYVALUEDESCRIPTION)){
                } else if (sLabelText.equals(XUnoFacetteNode.SSERVICEDESCRIPTION)){
                } else{
                    setText(sLabelText);
                    rc.validate();
                }
                setSize(getPreferredSize());
                rc.validate();
                doLayout();
            }
        } catch (RuntimeException e) {
            System.out.println("Sorry, icon for treecell could not be displayed.");
        }
        return this;
    }



    @Override
    public void paintComponent(Graphics g) {
        int x;
        if(getIcon() == null) {
            x = 0;
        } else {
            x = getIcon().getIconWidth() + getIconTextGap();
        }
        g.setColor(getForeground());
        super.paintComponent(g);
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
