/*************************************************************************
 *
 *  $RCSfile: CGGeneralInfo.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $  $Date: 2004-05-19 13:17:20 $
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

import com.sun.star.util.DateTime;
import com.sun.star.wizards.common.*;
import org.w3c.dom.Node;


public class CGGeneralInfo     extends ConfigGroup implements XMLProvider {
    public String           cp_Title;
    public String           cp_Description;
    public String           cp_Icon;
    public String           cp_Author;
    public String           cp_Keywords;
    public int              cp_CreationDate;
    public int              cp_UpdateDate;
    public int              cp_RevisitAfter;
    public String           cp_Email;
    public String           cp_Copyright;

    public Node createDOM(Node parent) {
        return XMLHelper.addElement(parent,"general-info",
          new String[] {"title","author","description","keywords","creation-date","update-date","revisit-after","email","copyright"},
          new String[] {cp_Title, cp_Author, cp_Description, cp_Keywords, String.valueOf(cp_CreationDate), String.valueOf(cp_UpdateDate),String.valueOf(cp_RevisitAfter),cp_Email, cp_Copyright}
        );
    }

    public Integer getCreationDate() {
        if (cp_CreationDate == 0)
            cp_CreationDate = currentDate();
        return new Integer(cp_CreationDate);
    }

    public String getIcon() {
        return cp_Icon;
    }

    public Integer getUpdateDate() {
        if (cp_UpdateDate== 0)
            cp_UpdateDate = currentDate();
        return new Integer(cp_UpdateDate);
    }

    public void setCreationDate(Integer i) {
        //System.out.println(i);
        cp_CreationDate = i.intValue();
    }

    public void setIcon(String string) {
        cp_Icon = string;
    }


    public void setUpdateDate(Integer i) {
        cp_UpdateDate = i.intValue();
    }

    private int currentDate() {
        DateTime dt = JavaTools.getDateTime(System.currentTimeMillis());
        //System.out.println();
        return dt.Day + dt.Month * 100 + dt.Year * 10000;
    }
}
