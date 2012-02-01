/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


import java.awt.Color;
import java.awt.Component;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.SystemColor;
import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JLabel;
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
            System.out.println("Sorry, could not locate resourecs, treecell icons will not be displayed.");
        }
    }


    public synchronized Component getTreeCellRendererComponent(JTree tree,Object value, boolean sel, boolean expanded, boolean leaf, int row, boolean hasFocus){
        try{
            bSelected = sel;
            DefaultMutableTreeNode node = (DefaultMutableTreeNode) value;
            Component rc = super.getTreeCellRendererComponent( tree, value, sel,expanded, leaf, row,hasFocus);
            String  sLabelText = (String)node.getUserObject();
            if (sLabelText != null){
                if (sLabelText.equals(XUnoFacetteNode.SCONTAINERDESCRIPTION)){
//                setIcon(m_oContainerIcon);
                } else if (sLabelText.equals(XUnoFacetteNode.SCONTENTDESCRIPTION)){
//                setIcon(m_oContentIcon);
                } else if (sLabelText.equals(XUnoFacetteNode.SINTERFACEDESCRIPTION)){
//                setIcon(m_oInterfaceIcon);
                } else if (sLabelText.equals(XUnoFacetteNode.SMETHODDESCRIPTION)){
//                setIcon(m_oMethodIcon);
                } else if (sLabelText.equals(XUnoFacetteNode.SPROPERTYDESCRIPTION)){
//                setIcon(m_oPropertyIcon);
                } else if (sLabelText.startsWith(XUnoFacetteNode.SPROPERTYINFODESCRIPTION)){
//                setIcon(m_oPropertyIcon);
                } else if (sLabelText.equals(XUnoFacetteNode.SPROPERTYVALUEDESCRIPTION)){
//                setIcon(m_oPropertyValueIcon);
                } else if (sLabelText.equals(XUnoFacetteNode.SSERVICEDESCRIPTION)){
//                setIcon(m_oServiceIcon);
                } else{
                    setText(sLabelText);
                    rc.validate();
                }
                setSize(getPreferredSize()); //fm.stringWidth(sLabelText), (int) getSize().getHeight());
                rc.validate();
//            nWidth = (int) rc.getPreferredSize().getWidth();
                doLayout();
            }
        } catch (RuntimeException e) {
            System.out.println("Sorry, icon for treecell could not be displayed.");
        }
        return this;
    }



    public void paintComponent(Graphics g) {
        FontMetrics fm = getFontMetrics(getFont());
        int x, y;
        y = fm.getAscent() + 2;
        if(getIcon() == null) {
            x = 0;
        } else {
            x = getIcon().getIconWidth() + getIconTextGap();
        }
        g.setColor(getForeground());
//         g.fillRect(x,y,x + fm.stringWidth(getText()),y);
//        System.out.println("Text: " + getText());
        super.paintComponent(g);
    }
}


