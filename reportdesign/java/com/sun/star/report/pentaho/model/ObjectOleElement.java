/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ObjectOleElement.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:33:07 $
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
package com.sun.star.report.pentaho.model;

import java.util.Vector;

/**
 *
 * @author Ocke Janssen
 */
public class ObjectOleElement extends ReportElement{

    private String url;
    private Vector masterfields;
    private Vector detailfields;
    private String classid;

    public String getClassid() {
        return classid;
    }

    public Vector getDetailfields() {
        return detailfields;
    }

    public Vector getMasterfields() {
        return masterfields;
    }

    public ObjectOleElement()
    {
        masterfields = new Vector();
        detailfields = new Vector();
    }

    public String getUrl() {
        return url;
    }

    public void setClassId(String classid) {
        this.classid = classid;
    }
    public void setUrl(final String _url ){
        url = _url;
    }

    public void addMasterDetailFields(final String master,final String detail){
        if ( master != null ){
            masterfields.add(master);
            detailfields.add(detail == null ? master : detail);
        }
    }

}
