/*************************************************************************
 *
 *  $RCSfile: CGDesign.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $  $Date: 2004-05-19 13:16:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 */

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
          return new short[] { (short) style.cp_Index };
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

