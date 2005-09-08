/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XDataPilotFieldGrouping.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:46:10 $
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

package ifc.sheet;

import com.sun.star.container.XNameAccess;
import com.sun.star.sheet.DataPilotFieldGroupBy;
import com.sun.star.sheet.DataPilotFieldGroupInfo;
import com.sun.star.sheet.XDataPilotField;
import com.sun.star.sheet.XDataPilotFieldGrouping;
import com.sun.star.uno.UnoRuntime;
import lib.MultiMethodTest;

/**
 *
 * @author sw93809
 */
public class _XDataPilotFieldGrouping extends MultiMethodTest
{
    public XDataPilotFieldGrouping oObj = null;

    public void _createNameGroup() {
        boolean result = true;
        try {
            XDataPilotField xDataPilotField = (XDataPilotField) UnoRuntime.queryInterface(XDataPilotField.class, oObj);
            XNameAccess xNameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, xDataPilotField.getItems ());
            String[] elements = xNameAccess.getElementNames ();
            oObj.createNameGroup(elements);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception while checking createNameGroup"+e);
            result = false;
        }
        tRes.tested ("createNameGroup()",result);
    }

    public void _createDateGroup() {
        boolean result = true;
        try {
            DataPilotFieldGroupInfo aInfo = new DataPilotFieldGroupInfo();
            aInfo.GroupBy = DataPilotFieldGroupBy.MONTHS;
            aInfo.HasDateValues = true;
            oObj.createDateGroup(aInfo);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception while checking createDateGroup"+e);
            result = false;
        }
        tRes.tested ("createDateGroup()",result);
    }
}
