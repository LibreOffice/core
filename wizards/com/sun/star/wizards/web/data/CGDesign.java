/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CGDesign.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:00:25 $
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
package com.sun.star.wizards.web.data;

import com.sun.star.wizards.common.*;
import org.w3c.dom.Node;


public class CGDesign extends ConfigGroup implements XMLProvider {

     public String cp_Layout;
     public String cp_Style;
     public String cp_BackgroundImage;
     public String cp_IconSet;
     //public int cp_OptimizeDisplay;

     public boolean cp_DisplayTitle;
     public boolean cp_DisplayDescription;
     public boolean cp_DisplayAuthor;
     public boolean cp_DisplayCreateDate;
     public boolean cp_DisplayUpdateDate;
     public boolean cp_DisplayFilename;
     public boolean cp_DisplayFileFormat;
     public boolean cp_DisplayFormatIcon;
     public boolean cp_DisplayPages;
     public boolean cp_DisplaySize;
     public int cp_OptimizeDisplaySize;



     private final String TRUE = "true";
     private final String FALSE = null;

     private String styleName;


     public Node createDOM(Node parent) {
         return XMLHelper.addElement(parent, "design", new String[0] ,
           new String[0]
            );

     }

    private static final Short SHORT_TRUE = new Short((short)1);
    private static final Short SHORT_FALSE = new Short((short)0);


    public short[] getStyle() {
        CGStyle style = ((CGStyle) ((CGSettings)root).cp_Styles.getElement(cp_Style));
          return new short[] { (short) ((CGSettings)root).cp_Styles.getIndexOf(style)};
    }

    public void setStyle(short[] newStyle) {
        Object o = ((CGStyle) ((CGSettings)root).cp_Styles.getElementAt(newStyle[0]));
        cp_Style =  (String)((CGSettings)root).cp_Styles.getKey(o);
    }

    public int getLayout() {
        CGLayout layout = ((CGLayout) ((CGSettings)root).cp_Layouts.getElement(cp_Layout));
        return layout.cp_Index;
    }

    public void setLayout(int layoutIndex) {
         CGLayout layout = (CGLayout)((CGSettings)root).cp_Layouts.getElementAt(layoutIndex);
         cp_Layout = (String)((CGSettings)root).cp_Layouts.getKey(layout);
    }

}

