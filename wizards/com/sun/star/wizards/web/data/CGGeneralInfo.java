/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CGGeneralInfo.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:01:28 $
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
 ************************************************************************/package com.sun.star.wizards.web.data;

import com.sun.star.util.DateTime;
import com.sun.star.wizards.common.*;
import org.w3c.dom.Node;


public class CGGeneralInfo     extends ConfigGroup implements XMLProvider {
    public String           cp_Title;
    public String           cp_Description;
    public String           cp_Author;
    public int              cp_CreationDate;
    public int              cp_UpdateDate;
    public String           cp_Email;
    public String           cp_Copyright;

    public Node createDOM(Node parent) {
        return XMLHelper.addElement(parent,"general-info",
          new String[] {"title","author","description","creation-date","update-date","email","copyright"},
          new String[] {cp_Title, cp_Author, cp_Description, String.valueOf(cp_CreationDate), String.valueOf(cp_UpdateDate),cp_Email, cp_Copyright}
        );
    }

    public Integer getCreationDate() {
        if (cp_CreationDate == 0)
            cp_CreationDate = currentDate();
        return new Integer(cp_CreationDate);
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

    public void setUpdateDate(Integer i) {
        cp_UpdateDate = i.intValue();
    }

    private int currentDate() {
        DateTime dt = JavaTools.getDateTime(System.currentTimeMillis());
        //System.out.println();
        return dt.Day + dt.Month * 100 + dt.Year * 10000;
    }
}
